#include <iostream>
#include <cstdlib>
#include <random>
#include <upcxx/upcxx.hpp>
#include <cmath>
#include <time.h>

using namespace std;

void merge_arrays(long int local_size, long int * local_array, long int * temp_dest_array, long int * new_array);
void sort(long int arr[], long int n);
void swap(long int* xp, long int* yp);
void swap_arrays(long int* xp, long int* yp);

int main(int argc , char *argv[]){

    // Initialize upcxx
    upcxx:: init();

    int k = 0;
    clock_t t;

    if (upcxx::rank_me() == 0){
        char *output;
        k = strtol(argv[1], &output, 10);
        cout << "Size 2^"
             << k
             << " array, using "
             << upcxx::rank_n()
             << " threads...\n";
        t = clock();
    }

    // Broadcast k to all threads
    k = upcxx::broadcast(k, 0).wait();

    // get array_size = 2^k without having to use pow()
    long int array_size = 1;
    for (int i = 1; i <= k; i++){array_size = array_size * 2LL;}

    srand(upcxx::rank_me()+1);

    // Initialize random values for each thread's array
    long int local_array_size = array_size/upcxx::rank_n();


    // Initialize global pointer to distributed array for other threads to access
    upcxx::dist_object<upcxx::global_ptr<int64_t>> local_array_global(upcxx::new_array<int64_t>(array_size));
    long int * local_array = local_array_global->local();

    // Insert random values for local array
    for (long int i = 0; i < local_array_size; i++){
        local_array[i] = rand();
    }

    // Sort local array
    sort(local_array, local_array_size);


    // Set up a flag 
    auto flagPartition = upcxx::new_<int>(upcxx::rank_me());
    upcxx::dist_object<upcxx::global_ptr<int>> flag_partitions(flagPartition);

    // Print out sorted local array for thread 0
    int printcount = 0;
    if (upcxx::rank_me()==0){
        cout << "Thread " << 0 << ":\n";
            for (long int i = 0; i < local_array_size; i++){
                cout << local_array[i] << ", ";
            }
        cout << "\n\n";
        *flagPartition.local() = 1;
        printcount = 1;
    } else {
        *flagPartition.local() = 0;
    }

    if (upcxx::rank_me()==0){
        t = clock();
    }

    // Have each thread print out sorted local array by calling previous thread's flag and checking to see if done printing
    for (int tid = 1; tid < upcxx::rank_n(); tid++) {
        auto autoFlagPartition = flag_partitions.fetch(tid-1).wait();
        int flag = upcxx::rget(autoFlagPartition).wait();
        upcxx::barrier();
        if (flag == 1 && printcount != 1) {
            cout << "Thread " << upcxx::rank_me() << ":\n";
            for (long int i = 0; i < local_array_size; i++){
                cout << local_array[i] << ", ";
            }
            cout << "\n\n";
            *flagPartition.local() = 1;
            printcount = 1;
        }
    upcxx::barrier();
    }


    int p = static_cast<int>(log2(upcxx::rank_n()));

    for(int level = 1; level <= p; level++){
        long int level_2 = static_cast<int64_t>(powl(2,(level)));
        long int level_2_plus1 = static_cast<int64_t>(powl(2,(level+1)));
        long int level_2_minus1 = static_cast<int64_t>(powl(2,(level-1)));

        // if (upcxx::rank_me() == 0){
        //     cout << "Level: " << level << "\n";
        // }

        if ((upcxx::rank_me()) % level_2 == 0){       // If thread will get another thread's array address to merge and sort

            // Call the address and store it as a temp address
            int call_indx = upcxx::rank_me() + level_2_minus1;  
            upcxx::global_ptr<int64_t> global_temp_array = local_array_global.fetch(call_indx).wait(); 
            long int * temp_dest_array = global_temp_array.local();
            
            //cout << "Thread " << upcxx::rank_me() << " recieved from Thread " << call_indx << "\n";

            // Make a new array to store merge between local and temp array
            // long int * new_array = (long int*) malloc(array_size * sizeof(long int));

            // long int count = 0;
            // for (long int i = 0; i < (local_array_size * level_2_minus1); i++){
            //     *(new_array + count) = *(local_array + i);
            //     count++;
            //     *(new_array + count) = *(temp_dest_array + i);
            //     count++;
            // }

            // Merge arrays
            for (long int i = (local_array_size * level_2_minus1); i < (local_array_size * level_2); i++){
                *(local_array + i) = *(temp_dest_array + i);
            }

            // Sort Arrays
            sort(local_array, local_array_size * level_2);

        }
        upcxx::barrier();
    }

    // Serial Times: See serial_times.txt
    double serial_times[5] = {0.0053, 0.0076, 0.0102, 0.0146, 0.0254};

    if (upcxx::rank_me() == 0){

        t = (100*(clock()) - (100*t));
        double total_time = ((static_cast<double>(t))/CLOCKS_PER_SEC);
        cout << "\nTotal time: " << total_time << " ms\n";

        cout << "Speedup is " << (serial_times[k-5]/total_time) << "\n"; 
        cout << "Efficiency is " << ((serial_times[k-5])/(total_time*upcxx::rank_n())) << "\n\n";
        
        cout << "Final Sorted Array:\n";
        for (int i = 0; i < array_size; i++){
            cout << local_array[i] << ", ";
        }
        cout << "\n\n";

    }
    
    
    upcxx::finalize();


}

void sort(long int arr[], long int n)
{
    int i, j, min_idx;

  
    // One by one move boundary of unsorted subarray
    for (i = 0; i < n - 1; i++) {
  
        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (arr[j] < arr[min_idx])
                min_idx = j;
  
        // Swap the found minimum element
        // with the first element
        swap(&arr[min_idx], &arr[i]);
    }
}

void swap(long int* xp, long int* yp)
{
    long int temp = *xp;
    *xp = *yp;
    *yp = temp;
}
