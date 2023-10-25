#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <string.h>

int thread_count, k;
long long n;

void merge(int arr1[], int arr2[], int * merged_array, long long n);
void sort(int arr[], long long n);
void swap(int* xp, int* yp);

int count = 0;


int main ( int argc , char *argv[])
{                                              /*use long in case of a 64−bit system */
    int my_rank;

    MPI_Init(&argc, &argv);
    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD, &thread_count);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0){printf("Enter Exponent (k) for Number of Array Elements (n = 2^k)...\n");}

    if (my_rank == 0){
        scanf("%i", &k);
        printf("2^(%i) array elements, using %i processors...\n", k, thread_count);
    }

    MPI_Bcast(&k, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);         // Broadcast exponent (k) to all processes

    n = 1LL;
    for (int i = 1; i <= k; i++){n = n * 2LL;}


    long long my_n = (long long) (n/thread_count);
    long long min, max;

    if (my_rank == 0){
        min = 0LL;
        max = min + my_n;
    } else {
        min = (my_n * my_rank);
        max = min + my_n;
    }

    long long size_of_array = max-min;

    int * local_array;

    //local_array = malloc(size_of_array * sizeof(int));

    local_array = malloc(n * sizeof(int));

    srand((int)rand()/(my_rank+1));

    for (long long i = 0; i < size_of_array; i++){
        local_array[i] = rand();
    }

    sort(local_array, size_of_array);


    int process_n = (int) log2(thread_count);


    for (int level = 0; level < process_n; level++){
        for (int indx = 0; indx < ((int) thread_count); indx += (int) pow(2, level)){
            int dest;

            if(indx % (((int) pow(2, (level+1)))) == 0){
                dest = indx;
            } else {
                dest = indx - (int) pow(2, level);
            }

            long long size_of_local_array, size_of_new_array;
            int * temp;
            int * temp_local;

            size_of_local_array = size_of_array;

            if ((my_rank == indx) && (dest != indx)){
                MPI_Send(&size_of_local_array, 1, MPI_LONG_LONG, dest, 1, MPI_COMM_WORLD);
                MPI_Send(local_array, size_of_local_array, MPI_INT, dest, 2, MPI_COMM_WORLD);
                printf("Level %i Index %i: Sent to %i\n", level, indx, dest);
                free(local_array);
            }

            if ((my_rank == dest) && (dest != indx)){
                MPI_Recv(&size_of_new_array, sizeof(int), MPI_LONG_LONG, indx, 1, MPI_COMM_WORLD, &status);
                temp = malloc(n * sizeof(local_array[0]));
                MPI_Recv(temp, n, MPI_INT, indx, 2, MPI_COMM_WORLD, &status);
                printf("Level %i Dest %i: Recieved from %i\n", level, dest, indx);

                long long size_of_temp_local = size_of_local_array + size_of_new_array;

                temp_local = malloc(n * sizeof(int));

                int count_local = 0;
                int count_new = 0;

                for (int i = 0; i < size_of_temp_local; i++){
                    if (i%2 == 0){
                        temp_local[i] = local_array[count_local];
                        count_local++;
                    } else{
                        temp_local[i] = temp[count_new];
                        count_new++;
                    }
                }

                sort(temp_local, size_of_temp_local);

                memcpy(local_array, temp_local, n*sizeof(int));

                free(temp_local);
                free(temp);

                size_of_array = size_of_temp_local;        

            }

                
        }

    }

    if (my_rank == 0){
        for (int i = 0; i < n; i++){
            printf("%i,\n", local_array[i]);
        }
    }

    MPI_Finalize();

    return 0;

}

void merge(int arr1[], int arr2[], int * merged_array, long long n){

    for (int i = 0; i < n; i++){
        merged_array[i] = arr1[i];
        printf("%i, ", merged_array[i]);
        i++;
        merged_array[i] = arr2[i];
        printf("%i, ", merged_array[i]);
    }

    printf("\n Done Merge\n");

}

void sort(int arr[], long long n)
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

void swap(int* xp, int* yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}