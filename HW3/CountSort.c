#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <sys/resource.h>


int *arr, *a_serial, *a_parallel, *a_qsort;
int n;

struct rusage usage_s, usage_p, usage_q;
struct timeval start, end;

double total_time;

int thread_count;


void Count_sort(int* a, int n);
void Count_sort_parallel(int* a, int n);
int comparator (const void * p1, const void * p2);



int main(int argc, char* argv[]){

    thread_count = strtol (argv[1], NULL, 10);

    n = 50000;

    arr = malloc(n*sizeof(int));
    a_serial = malloc(n*sizeof(int));
    a_parallel = malloc(n*sizeof(int));
    a_qsort = malloc(n*sizeof(int));

    for (int i = 0; i < n; i++){
        arr[i] = (rand() % (n));
    }

    memcpy(a_serial, arr, n*sizeof(int));
    memcpy(a_parallel, arr, n*sizeof(int));
    memcpy(a_qsort, arr, n*sizeof(int));

    printf("Starting... \n");

    getrusage(RUSAGE_SELF, &usage_s);
    start = usage_s.ru_utime;
    Count_sort(a_serial,n);
    getrusage(RUSAGE_SELF, &usage_s);
    end = usage_s.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Serial Count Sort\nTotal Time: %0.6f s\n", total_time);

    getrusage(RUSAGE_SELF, &usage_p);
    start = usage_p.ru_utime;
    Count_sort_parallel(a_parallel, n);
    getrusage(RUSAGE_SELF, &usage_p);
    end = usage_p.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Parallel Count Sort\nTotal Time: %0.6f s\n", total_time);

    getrusage(RUSAGE_SELF, &usage_q);
    start = usage_q.ru_utime;
    qsort(arr, n, sizeof(int), comparator);
    getrusage(RUSAGE_SELF, &usage_q);
    end = usage_q.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with qsort\nTotal Time: %0.6f s\n", total_time);

}

void Count_sort(int* a, int n){
    int i, j, count;

    int* temp = malloc(n*sizeof(int));

    for (i = 0; i< n; i++){
        count = 0;

        for(j = 0; j<n; j++){
            if (a[j] < a[i]){
                count++;
            } else if (a[j]==a[i] && j<i){
                count++;
            }
        }
        temp[count] = a[i];
    }

    memcpy(a, temp, n*sizeof(int));

    free(temp);

}


void Count_sort_parallel(int* a, int n){
    
    int i, j;

    int* temp = malloc(n*sizeof(int));

#   pragma omp parallel for num_threads(thread_count) default(none) shared(a,n, temp, thread_count) private(i,j) schedule(guided, thread_count)
    for (i=0; i < n; i++){
        int count = 0;

        for(j = 0; j < n; j++){
            if (a[j] < a[i]){
                count++;
            } else if (a[j] == a[i] && j < i){
                count++;
            }
        }
        temp[count] = a[i];
        
    }

    memcpy(a, temp, n*sizeof(int));

    free(temp);

}

int comparator (const void * p1, const void * p2)
{
  return (*(int*)p1 - *(int*)p2);
}