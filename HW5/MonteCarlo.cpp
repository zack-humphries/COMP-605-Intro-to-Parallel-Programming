#include <iostream>
#include <cstdlib>
#include <random>
#include <upcxx/upcxx.hpp>
#include <cmath>
#include <time.h>

using namespace std;

int main(int argc , char *argv[]){


    // Initialize upcxx
    upcxx:: init();

    int k = 0;
    clock_t t;

    if (upcxx::rank_me() == 0){
        char *output;
        k = strtol(argv[1], &output, 10);
        cout <<"10^("
             << k
             << ") tosses, using "
             << upcxx::rank_n()
             << " processes...\n";
             t = clock();
    }
    

    // broadcast a to all nodes
    k = upcxx::broadcast(k, 0).wait();

    long int number_of_tosses = 1;

    // get number_of_tosses = 2^k without having to use pow()
    for (int i = 1; i <= k; i++){number_of_tosses = number_of_tosses * 10LL;}

    srand(upcxx::rank_me());

    

    // Calculate thread's pi/4 estimate
    long int my_number_in_circle = 0;
    for(long int toss = 0; toss < static_cast<int64_t>(number_of_tosses/upcxx::rank_n()); toss++)
    {
        double x = (static_cast<double> (rand()) / static_cast<double> (RAND_MAX/(2.0)))-1.0;
        double y = (static_cast<double>(rand()) / static_cast<double> (RAND_MAX/(2.0)))-1.0;
        double distance_squared = x*x + y*y ;
        if ( distance_squared <= 1.0 ) my_number_in_circle++;
    }


    // Set up distributed object which allows all threads to put in their pi/4 estimate
    auto my_number_in_circlePartition = upcxx::new_<int64_t>(upcxx::rank_me());
    upcxx::dist_object<upcxx::global_ptr<int64_t>> partitions(my_number_in_circlePartition);
    *my_number_in_circlePartition.local() = my_number_in_circle;
    upcxx::barrier();

    // Serial Times: See serial_times.txt
    double serial_times[5] = {0.004361, 0.041752, 0.416791, 4.16155, 41.599};

    // Host thread combines and prints out pi estimate, speedup, & efficiency
    long int number_in_circle = 0;
    if (upcxx::rank_me() == 0) {
        for (int i = 0; i < upcxx::rank_n(); i++) {
            // Grabs pi/4 estimate
            auto partition = partitions.fetch(i).wait();
            long int num_circle = upcxx::rget(partition).wait();
            cout << "Thread " << i << ": " << num_circle << "\n";
            number_in_circle = number_in_circle + num_circle;
        }

        long double pi_estimate = static_cast<long double> ((static_cast<long double> (4.0)) * ((static_cast<long double> (number_in_circle)) /static_cast<long double> (number_of_tosses)));
        cout << "Using " << number_of_tosses << " tosses, "
             << number_in_circle << " landed in circle "
             << "Pi estimate is " << pi_estimate << "\n";

        t = clock() - t;
        double total_time = ((static_cast<double>(t))/CLOCKS_PER_SEC);
        cout << "Total time: " << total_time << " seconds\n";

        cout << "Speedup is " << (serial_times[k-5]/total_time) << "\n"; 
        cout << "Efficiency is " << ((serial_times[k-5])/(total_time*upcxx::rank_n())) << "\n";   
    }

    upcxx::finalize();
}