#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>


int thread_count;                                               /*global variable : accessable to all threads */
long long number_of_tosses;
double pi_estimate;
int flag;
long long number_in_circle;

void *MonteCarlo ( void *rank );                                    /*Thread function */

int main ( int argc , char *argv[])
{
    long thread;                                               /*use long in case of a 64−bit system */
    pthread_t*thread_handles ;

    number_of_tosses = strtol( argv[1] , NULL, 10) ; //232792560; /*5354228880 232792560 2520*11*13;*/

    thread_count = strtol( argv[2] , NULL, 10) ;                /*# of threads (command line) */

    if (number_of_tosses < thread_count){
        printf("Error: Number_of_Tosses is less than Thread_Count. Enter Number_of_Tosses first and then Thread_Count second.");
        exit(-1);
    }

    flag = 0;

    thread_handles = malloc ( thread_count*sizeof(pthread_t));

    for(thread = 0; thread < thread_count ; thread++){
        pthread_create(&thread_handles[thread] , NULL, MonteCarlo, (void*) thread);
    }

    for( thread = 0; thread < thread_count ; thread++){
        pthread_join ( thread_handles [ thread ] , NULL);
    }

    free(thread_handles);

    pi_estimate = (4.0* number_in_circle / ( (double) number_of_tosses));

    printf("Using Busy-Waiting... \nPi Estimate: %f\n" , pi_estimate);

    return 0;
}

void *MonteCarlo (void *rank)
{
    long my_rank = (long) rank;                              /*use long in case 64−bit system */

    printf("Thread %ld of %d has started\n" , my_rank , thread_count );

    long long my_number_in_circle = 0;

    long long my_n = (long long) floor(number_of_tosses/thread_count);
    long long remainder = (number_of_tosses)-(my_n*thread_count);
    long long min;
    long long max;

    if (my_rank == 0){
        min = 1;
        max = min + my_n + remainder;
    } else {
        min = (my_n * my_rank) + remainder + 1;
        max = min + my_n;
    }

    for(int toss = min; toss < max; toss++)
    {
        double x = (double)rand() / ((double)RAND_MAX/(2.0))-1.0;
        double y = (double)rand() / ((double)RAND_MAX/(2.0))-1.0;
        double distance_squared = x*x + y*y ;
        if ( distance_squared <= 1.0 ) my_number_in_circle++;
    }

    while(flag != my_rank);
    number_in_circle += my_number_in_circle;
    flag++;

    printf("Thread %ld of %d has ended\n" , my_rank , thread_count );

    return NULL;
}