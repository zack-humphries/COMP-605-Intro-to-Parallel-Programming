#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int thread_count;                                               /*global variable : accessable to all threads */
double pi_estimate;
long long number_in_circle, number_of_tosses;
int k;


int main ( int argc , char *argv[])
{                                              
    int my_rank;
    double starttime, endtime;


    MPI_Init(&argc, &argv);                             // Initialize MPI
    MPI_Comm_size(MPI_COMM_WORLD, &thread_count);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0){printf("Enter Exponent (d) for Number of Tosses (number_of_tossses = 10^k)...\n");}

    if (my_rank == 0){
        scanf("%i", &k);
        printf("10^(%i) tosses, using %i processors...\n", k, thread_count);
    }
    
    MPI_Bcast(&k, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);         // Broadcast exponent (k) to all processes

    if (k <= 2){printf("\nNot enough tosses\n"); return 0;}
    else {
        number_of_tosses = 1LL; 
        for (int i = 1; i <= k; i++){number_of_tosses = number_of_tosses * 10LL;}     // Establishes number of tosses as a long long

        if (my_rank == 0){printf("\nEstimating Pi through %Li tosses with Parallel Code\n", number_of_tosses);}
        starttime = MPI_Wtime();

        /* Split up number of tosses by process*/
        long long my_number_in_circle = 0;
        long long my_n = (long long) floor(number_of_tosses/thread_count);
        long long remainder = (number_of_tosses)-(my_n*thread_count);
        long long min, max;

        if (my_rank == 0){
            min = 1;
            max = min + my_n + remainder;
        } else {
            min = (my_n * my_rank) + remainder + 1;
            max = min + my_n;
        }
        /* Monte Carlo Pi Estimation Process*/
        for(long long toss = min; toss < max; toss++)
        {
            double x = (double)rand() / ((double)RAND_MAX/(2.0))-1.0;
            double y = (double)rand() / ((double)RAND_MAX/(2.0))-1.0;
            double distance_squared = x*x + y*y ;
            if ( distance_squared <= 1.0 ) my_number_in_circle++;
        }

        /* Sum all processes's my_number_in_circle to total of number_in_circle */
        MPI_Reduce(&my_number_in_circle, &number_in_circle, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    endtime = MPI_Wtime();
    

    if (my_rank == 0){
        printf("\nTotal Number in Circle: %lli\n", number_in_circle);
        pi_estimate = (double) (((long double) 4.0) * (((long double)number_in_circle) /(long double)number_of_tosses));
        printf("Pi Estimation: %f\n", (pi_estimate));
        printf("Parallel Process took %f seconds\n",endtime-starttime);
    }
    MPI_Finalize();

    

    /* Start Serial Process */
    if (my_rank == 0){printf("\nEstimating Pi through %Li tosses with Parallel Code\n", number_of_tosses);}
    long long serial_number_in_circle = 0;

    double s_starttime, s_endtime;
    s_starttime = MPI_Wtime();
    for(long long toss = 1; toss < number_of_tosses; toss++)
    {
        double x = (double)rand() / ((double)RAND_MAX/(2.0))-1.0;
        double y = (double)rand() / ((double)RAND_MAX/(2.0))-1.0;
        double distance_squared = x*x + y*y ;
        if ( distance_squared <= 1.0 ) serial_number_in_circle++;
    }
    s_endtime = MPI_Wtime();

    if (my_rank == 0){printf("\nSerial Process took %f seconds\n", s_endtime - s_starttime);}

    /* Speedup and Efficiency Calculation */
    if (my_rank == 0){printf("Speedup is %f\n", (s_endtime - s_starttime)/((endtime - starttime)));}
    if (my_rank == 0){printf("Efficiency is %f\n", (s_endtime - s_starttime)/(thread_count* (endtime - starttime)));}


    return 0;
}