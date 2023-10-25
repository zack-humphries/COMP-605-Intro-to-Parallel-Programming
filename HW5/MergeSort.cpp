#include <iostream>
#include <cstdlib>
#include <random>
#include <upcxx/upcxx.hpp>
#include <cmath>

using namespace std;

void merge_arrays(long int local_size, long int * local_array, long int * temp_dest_array, long int * new_array);
void sort(long int arr[], long int n);
void swap(long int* xp, long int* yp);
void swap_arrays(long int* xp, long int* yp);

int main(int argc , char *argv[]){

    // Initialize upcxx
    upcxx:: init();

    int k = 0;

    if (upcxx::rank_me() == 0){
        char *output;
        k = strtol(argv[1], &output, 10);
        cout << "Size 2^"
             << k
             << " array, using "
             << upcxx::rank_n()
             << "processes...\n";
    }

    // Broadcast k to all threads
    k = upcxx::broadcast(k, 0).wait();
    upcxx::barrier();

    // get array_size = 2^k without having to use pow()
    long int array_size = 1;
    for (int i = 1; i <= k; i++){array_size = array_size * 2LL;}

    srand(upcxx::rank_me());

    // Initialize random values for each thread's array
    long int local_array_size = array_size/upcxx::rank_n();
    long int * local_array_inital = (long int*)malloc(local_array_size * sizeof(long double));
    for (long int i = 0; i < local_array_size; i++){
        local_array_inital[i] = rand();
    }

    // Sort local array
    sort(local_array_inital, local_array_size);



    // Print out sorted local array
    for (int tid = 0; tid <upcxx::rank_n(); tid++) {
        cout << "Thread " << tid << ":\n";
        if (tid == upcxx::rank_me()) {
            for (long int i = 0; i < local_array_size; i++){
                cout << local_array_inital[i] << ", ";
            }
            cout << "\n\n";
        }
    }

    //long int * local_array_ptr = local_array_inital;

    // Set up shared global pointer array where each element of the array has a pointer that points to the thread's local array address
    //auto local_arrayPartition = upcxx::new_array<int64_t>(upcxx::rank_me());
    //upcxx::dist_object<upcxx::global_ptr<int64_t>> partitions(local_arrayPartition);
    //*local_arrayPartition.local() = *local_array_ptr;
    //upcxx::barrier();

    upcxx::dist_object<upcxx::global_ptr<int64_t>> local_array_ptr(upcxx::new_array<int64_t>(upcxx::rank_n()));
    long int ** array_ptr = local_array_ptr->local();

    long int ** local_array = (long int**) malloc(sizeof(long int*));
    *(local_array) = (long int*) malloc(local_array_size * sizeof(long int*));

    *(local_array) = local_array_inital;

    array_ptr = local_array;


    int p = k;

    for(int level = 0; level <= p; level++){
        long int level_2 = static_cast<int64_t>(powl(2,(level)));
        long int level_2_plus1 = static_cast<int64_t>(powl(2,(level+1)));

        if ((upcxx::rank_me()) % level_2_plus1 == 0){       // If thread will get another thread's array address to merge and sort

            // Call the address and store it as a temp address
            int call_indx = upcxx::rank_me() + level_2;                                                         
            auto partition = local_array_ptr.fetch(call_indx).wait();

            long int * temp_dest_array = upcxx::rget(partition).wait();

            // Initialize temp array with other thread's array
            //long int * temp_dest_array = (long int*) malloc(local_array_size * level_2 * sizeof(long int));
            //temp_dest_array = *(temp_dest_ptr);

            // Make a new array to store merged and sorted values
            long int * new_array = (long int*) malloc(local_array_size * level_2_plus1 * sizeof(long int));
            merge_arrays((local_array_size*level_2), *(local_array), temp_dest_array, new_array);
            sort(new_array, local_array_size * level_2_plus1);

            // Only need new_array with sorted values
            free(temp_dest_array);
            free(local_array);

            // Initialize new local_array that will store new_array's array
            long int ** local_array = (long int**) malloc(sizeof(long int));
            *(local_array) = (long int*) malloc(local_array_size* level_2_plus1 * sizeof(long int));

            //memcpy(local_array, new_array, local_array_size* level_2_plus1 * sizeof(long int));

            // Swap addresses to arrays so local_array now has new merged and sorted array, and free new_array
            long int * temp = *(local_array);
            *(local_array) = new_array;
            new_array = temp;

            //swap_arrays(local_array, new_array);
            free(new_array);
            free(temp);

            // Update pointer to local array on global pointer array
            // local_array_ptr = local_array;
            // *local_arrayPartition.local() = local_array_ptr;
            upcxx::barrier();
        }

        upcxx::barrier();
    }

    if (upcxx::rank_me() == 0){
        cout << "Final Sorted Array:\n";
        for (int i = 0; i < array_size; i++){
            cout << *(local_array)[i] << ", ";
        }
    }
    
    
    upcxx::finalize();


}

void merge_arrays(long int local_size, long int * local_array, long int * temp_dest_array, long int * new_array){
    long int count = 0;
    for (long int i = 0; i < local_size; i++){
        *(new_array + count) = *(local_array + i);
        count++;
        *(new_array + count) = *(local_array + i);
        count++;
    }
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

void swap_arrays(long int* arr1, long int* arr2)
{
    long int * temp_arr1 = arr1;
    long int * temp_arr2 = arr2;
    long int * temp = temp_arr1;

    temp_arr1 = temp_arr2;
    temp_arr2 = temp;

    arr1 = temp_arr1;
    arr2 = temp_arr2;
}

