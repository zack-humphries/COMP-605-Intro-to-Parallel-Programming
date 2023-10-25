#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>
//#include <C:/Users/14048/Desktop/SDSU/Spring 2023/COMP 605/HW2/resource.h>


int thread_count;                                               /*global variable : accessable to all threads */
long long n;
long double a,b;
int flag;
double summation;
double trapezoidal_integral;

void *Trapezoidal ( void *rank );                                    /*Thread function */

int main ( int argc , char *argv[])
{
    struct rusage usage;
    struct timeval start, end;
    getrusage(RUSAGE_SELF, &usage);
    start = usage.ru_utime;
    
    long thread;                                               /*use long in case of a 64−bit system */
    pthread_t*thread_handles ;

    thread_count = strtol( argv[1] , NULL, 10) ;                /*# of threads (command line) */

    n = 1024;
    a = 1;
    b = exp(4);

    flag = 0;

    thread_handles = malloc ( thread_count*sizeof(pthread_t));

    for(thread = 0; thread < thread_count ; thread++){
        pthread_create(&thread_handles[thread] , NULL, Trapezoidal, (void*) thread);
    }

    for( thread = 0; thread < thread_count ; thread++){
        pthread_join ( thread_handles [ thread ] , NULL);
    }

    free(thread_handles);

    trapezoidal_integral = ((b-a)/n) * (  (log(a)/2)  + summation + (log(b)/2) );

    printf("Trapezoidal Integral of f(x) = %f\n" , trapezoidal_integral);

    getrusage(RUSAGE_SELF, &usage);
    end = usage.ru_utime;

    printf("Started at: %is\n", start.tv_usec);
    printf("Ended at: %is\n", end.tv_usec);

    double total_time = 0.000001*(end.tv_usec - start.tv_usec)

    printf("Total Time: %0.6f s\n", total_time);

    return 0;
}

void *Trapezoidal (void *rank)
{
    long my_rank = (long) rank;                              /*use long in case 64−bit system */

    printf("Thread %ld of %d has started\n" , my_rank , thread_count );

    double my_summation = 0;

    long long my_n = (long long) floor((n-1)/thread_count);
    long long remainder = (n-1)-(my_n*thread_count);
    long long min;
    long long max;

    if (my_rank == 0){
        min = 1;
        max = min + my_n + remainder;
    } else {
        min = (my_n * my_rank) + remainder + 1;
        max = min + my_n;
    }

    for(long long k = min; k < max; k++)
    {
        my_summation += log(a + (k *((b-a)/n)));
    }

    while(flag != my_rank);
    summation += my_summation;
    printf("Thread %ld of %d has ended going from range [%ld, %ld]. Summation is %lf\n" , my_rank , thread_count, min, max-1, my_summation);
    flag++;

    return NULL;
}