#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

int thread_count;                                               /*global variable : accessable to all threads */
long long n;
long double a,b;
int flag;
long double summation;
long double trapezoidal_integral;


int main (void)
{
    
    n = 100;
    a = 1;
    b = exp(4);

    thread_count = 2;
    
    for (long rank = 0; rank < thread_count; rank++){
    
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

        for(int k = min; k < max; k++)
        {
            my_summation += log(a + (k *((b-a)/n)));
            printf("%d -> %f : %f\n", k, ((double) (a + (k *((b-a)/n)))), my_summation);
        }

        printf("Thread %ld of %d has ended going from range [%ld, %ld]. Summation is %lf\n" , my_rank , thread_count, min, max, my_summation);

    }

}