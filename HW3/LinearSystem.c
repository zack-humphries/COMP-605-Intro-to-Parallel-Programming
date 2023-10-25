#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <sys/resource.h>

double **A;
double *b, *x;
int n;
double ** T;
double ** base;
int thread_count;

int sizeT = 100;
int sizebase;

struct rusage usage_sr, usage_sc, usage_pr, usage_pc, usage_psr, usage_pdr, usage_pgr, usage_prh, usage_pch;
struct timeval start, end;

double total_time;

double randfrom(double min, double max);

void row_oriented_gaussian_elimination(double** A, double* b, double* x, int n);
void column_oriented_gaussian_elimination(double** A, double* b, double* x, int n);
void parallel_row_oriented_gaussian_elimination(double** A, double* b, double* x, int n);
void parallel_static_row_oriented_gaussian_elimination(double** A, double* b, double* x, int n);
void parallel_column_oriented_gaussian_elimination(double** A, double* b, double* x, int n);
void parallel_dynamic_row_oriented_gaussian_elimination(double** A, double* b, double* x, int n);
void parallel_guided_row_oriented_gaussian_elimination(double** A, double* b, double* x, int n);


void make_random_triangular(int n);
void free_A(int n);
void make_A(int n);
void make_T(int sizeT);
void make_base(int sizebase);
void Kroneckerproduct(int n, int sizeT);
void include_I(int n);

int main(int argc, char* argv[]){

    thread_count = strtol (argv[1], NULL, 10);

    n = strtol (argv[2], NULL, 10);

    if (!n){
        printf("Did not input a size (n) for matrix");
        exit(0);
    }

    sizebase = (int) ceil(n/sizeT);

    printf("Starting Constructing Random Upper Triangle Matrix with %i variables\n", n);

    make_random_triangular(n);

    b = malloc(n * sizeof(b[0]));
    x = malloc(n * sizeof(x[0]));

    for (int i = 0; i<n; i++){
        b[i] = i+1;
    }

    for(int row = 0; row<n; row++){
        x[row] = b[row];
    }

    // for (int i = 0; i<n; i++){
    //     for (int j = 0; j<n; j++){
    //         printf("%f  ", A[i][j]);
    //     }
    //     printf("\n\n");

    // }

    printf("Starting...\n");
    printf("Serial Row-Oriented Gaussian Elimination\n");

    getrusage(RUSAGE_SELF, &usage_sr);
    start = usage_sr.ru_utime;
    row_oriented_gaussian_elimination(A, b, x, n);
    getrusage(RUSAGE_SELF, &usage_sr);
    end = usage_sr.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Serial Row-Oriented Gaussian Elimination\nTotal Time: %0.6f s\n", total_time);

    memcpy(x, b, n*sizeof(double));

    printf("\n");
    printf("Serial Column-Oriented Gaussian Elimination\n");


    getrusage(RUSAGE_SELF, &usage_sc);
    start = usage_sc.ru_utime;
    column_oriented_gaussian_elimination(A, b, x, n);
    getrusage(RUSAGE_SELF, &usage_sc);
    end = usage_sc.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Serial Column-Oriented Gaussian Elimination\nTotal Time: %0.6f s\n", total_time);

    memcpy(x, b, n*sizeof(double));

    printf("\n");
    printf("Parallel Row-Oriented Gaussian Elimination\n");
    
    getrusage(RUSAGE_SELF, &usage_pr);
    start = usage_pr.ru_utime;
    parallel_row_oriented_gaussian_elimination(A, b, x, n);
    getrusage(RUSAGE_SELF, &usage_pr);
    end = usage_pr.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Parallel Row-Oriented Gaussian Elimination\nTotal Time: %0.6f s\n", total_time);


    memcpy(x, b, n*sizeof(double));

    printf("\n");
    printf("Parallel Column-Oriented Gaussian Elimination\n");


    getrusage(RUSAGE_SELF, &usage_pc);
    start = usage_pc.ru_utime;
    parallel_column_oriented_gaussian_elimination(A, b, x, n);
    end = usage_pc.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Parallel Column-Oriented Gaussian Elimination\nTotal Time: %0.6f s\n", total_time);

    memcpy(x, b, n*sizeof(double));

    printf("\n");
    printf("Parallel Static Row-Oriented Gaussian Elimination\n");
    
    getrusage(RUSAGE_SELF, &usage_psr);
    start = usage_psr.ru_utime;
    parallel_static_row_oriented_gaussian_elimination(A, b, x, n);
    end = usage_psr.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Parallel Static Row-Oriented Gaussian Elimination\nTotal Time: %0.6f s\n", total_time);


    memcpy(x, b, n*sizeof(double));
    printf("\n");
    printf("Parallel Dynamic Row-Oriented Gaussian Elimination\n");

    getrusage(RUSAGE_SELF, &usage_pdr);
    start = usage_pdr.ru_utime;
    parallel_dynamic_row_oriented_gaussian_elimination(A, b, x, n);
    end = usage_pdr.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Parallel Dynamic Row-Oriented Gaussian Elimination\nTotal Time: %0.6f s\n", total_time);


    memcpy(x, b, n*sizeof(double));
    printf("\n");
    printf("Parallel Guided Row-Oriented Gaussian Elimination\n");

    getrusage(RUSAGE_SELF, &usage_pgr);
    start = usage_pgr.ru_utime;
    parallel_guided_row_oriented_gaussian_elimination(A, b, x, n);
    end = usage_pgr.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Parallel Guided Row-Oriented Gaussian Elimination\nTotal Time: %0.6f s\n", total_time);

    memcpy(x, b, n*sizeof(double));
    free_A(n);

    printf("\n");
    printf("Starting Constructing A for Problem 2)h...\n");

    make_A(n);

    printf("Done Constructing A with %i variables. T and I each have %i variables. \n", n, sizeT);

    printf("\n");
    printf("Parallel Row-Oriented Gaussian Elimination\n");
    
    getrusage(RUSAGE_SELF, &usage_prh);
    start = usage_prh.ru_utime;
    parallel_row_oriented_gaussian_elimination(A, b, x, n);
    getrusage(RUSAGE_SELF, &usage_prh);
    end = usage_prh.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Parallel Row-Oriented Gaussian Elimination\nTotal Time: %0.6f s\n", total_time);


    memcpy(x, b, n*sizeof(double));

    printf("\n");
    printf("Parallel Column-Oriented Gaussian Elimination\n");


    getrusage(RUSAGE_SELF, &usage_pch);
    start = usage_pch.ru_utime;
    parallel_column_oriented_gaussian_elimination(A, b, x, n);
    end = usage_pch.ru_utime;
    total_time = 1.0*(end.tv_sec - start.tv_sec) + 0.000001*(end.tv_usec - start.tv_usec);
    printf("Done with Parallel Column-Oriented Gaussian Elimination\nTotal Time: %0.6f s\n", total_time);

    return 0;
}


void row_oriented_gaussian_elimination(double** A, double* b, double* x, int n){

    for(int row = n-1; row >= 0; row--){
        x[row] = b[row];

        for (int col = row + 1; col < n; col++){
            x[row] -= ((double)A[row][col])*x[col];
        }


    x[row] /= (double)A[row][row];
    }
}

void parallel_row_oriented_gaussian_elimination(double** A, double* b, double* x, int n){

    int row, col;

    int thread_count = omp_get_num_threads();

#   pragma omp parallel num_threads(thread_count) default(none) shared(A,x,b,n, row) private(col)
    for(row = n-1; row >= 0; row--){
        x[row] = b[row];

#       pragma omp for
        for (col = row + 1; col < n; col++){
            x[row] -= (A[row][col])*x[col];
        }


    x[row] /= A[row][row];
    }
}

void parallel_static_row_oriented_gaussian_elimination(double** A, double* b, double* x, int n){

    int row, col;

    int thread_count = omp_get_num_threads();

    double temp;


#   pragma omp parallel num_threads(thread_count) default(none) shared(A,x,b,n, row, thread_count, temp) private(col) 
    for(row = n-1; row >= 0; row--){
        x[row] = b[row];

        temp = x[row];

#       pragma omp for reduction(-: temp) schedule(static, thread_count)
        for (col = row + 1; col < n; col++){
            temp -= (A[row][col])*x[col];
        }

    x[row] = temp;

    x[row] /= A[row][row];
    }
}

void parallel_dynamic_row_oriented_gaussian_elimination(double** A, double* b, double* x, int n){

    int row, col;

    int thread_count = omp_get_num_threads();

    double temp;


#   pragma omp parallel num_threads(thread_count) default(none) shared(A,x,b,n, row, thread_count, temp) private(col) 
    for(row = n-1; row >= 0; row--){
        x[row] = b[row];

        temp = x[row];


#       pragma omp for reduction(-: temp) schedule(dynamic)
        for (col = row + 1; col < n; col++){
            temp -= (A[row][col])*x[col];
        }

    x[row] = temp;

    x[row] /= A[row][row];
    }
}

void parallel_guided_row_oriented_gaussian_elimination(double** A, double* b, double* x, int n){

    int row, col;

    int thread_count = omp_get_num_threads();

    double temp;


#   pragma omp parallel num_threads(thread_count) default(none) shared(A,x,b,n, row, thread_count, temp, sizeT) private(col) 
    for(row = n-1; row >= 0; row--){
        x[row] = b[row];

        temp = x[row];


#       pragma omp for reduction(-: temp) schedule(guided, sizeT)
        for (col = row + 1; col < n; col++){
            temp -= (A[row][col])*x[col];
        }

    x[row] = temp;

    x[row] /= A[row][row];
    }
}


void column_oriented_gaussian_elimination(double** A, double* b, double* x, int n){

    for(int row = 0; row<n; row++){
        x[row] = b[row];
    }

    for(int col = n-1; col >= 0; col--){
        x[col] /= ((double)A[col][col]);

        for (int row = 0; row < col; row++){
            x[row] -= ((double)A[row][col])*x[col];
        }
    }
}

void parallel_column_oriented_gaussian_elimination(double** A, double* b, double* x, int n){

    int row, col;

    int thread_count = omp_get_num_threads();

    for(row = 0; row<n; row++){
        x[row] = b[row];
    }

#   pragma omp parallel num_threads(thread_count) default(none) shared(A,x,n,col) private(row)
    for(col = n-1; col >= 0; col--){
        x[col] /= (A[col][col]);

#       pragma omp for
        for (row = 0; row < col; row++){
            x[row] -= (A[row][col])*x[col];
        }

    }
}


void make_random_triangular(int n){


    A = malloc(n * sizeof(A[0]));
    for(int i = 0; i < n; i++)
    {
        A[i] = malloc(n * sizeof(A[0][0]));
    }

    int j;

#   pragma omp parallel for num_threads(thread_count) private(j)
    for(int i = 0; i < n; i++)
    {
        for(j = 0; j < n; j++){
            if (i>j){
                A[i][j] = 0.0;
            } else {
                A[i][j] = randfrom(-100.0 , 100.0);
            }
        }
    }


}

void free_A(int n){


    for(int i = 0; i < n; i++)
    {
        free(A[i]);
    }



}

void make_A(int n){

    A = malloc(n * sizeof(A[0]));
    for(int i = 0; i < n; i++)
    {
        A[i] = malloc(n * sizeof(A[0][0]));
    }

    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++){
            A[i][j] = 0;
        }
    }


    make_base(sizebase);

    // for (int i = 0; i<sizebase; i++){
    //     for (int j = 0; j<sizebase; j++){
    //         printf("%d  ", base[i][j]);
    //     }
    //     printf("\n");

    // }

    make_T(sizeT);

    // for (int i = 0; i<sizeT; i++){
    //     for (int j = 0; j<sizeT; j++){
    //         printf("%d  ", T[i][j]);
    //     }
    //     printf("\n");

    // }


    Kroneckerproduct(sizebase, sizeT);

    include_I(n);


}

void make_T(int sizeT){

    T = malloc(sizeT * sizeof(T[0]));
    for(int i = 0; i < sizeT; i++)
    {
        T[i] = malloc(sizeT * sizeof(T[0][0]));
    }


    for(int i = 0; i < sizeT; i++){
        for (int j = 0; j< sizeT; j++){
            T[i][j] = 0;
        }
        T[i][i] = -4;

        if(i>0){
            T[i][i-1] = 1;
        }

        if(i < sizeT-1){
            T[i][i+1] = 1;
        }
    }

}

void make_base(int sizebase){

    base = malloc(sizebase * sizeof(base[0]));
    for(int i = 0; i < sizebase; i++)
    {
        base[i] = malloc(sizebase * sizeof(base[0][0]));
    }

    for(int i = 0; i < sizebase; i++){
        for (int j = 0; j< sizebase; j++){
            base[i][j] = 0;
        }
        base[i][i] = 1;
    }

}

void Kroneckerproduct(int sizebase, int sizeT)
{
 
 
    // i loops till rowa
    for (int i = 0; i < sizebase; i++) {
 
        // k loops till rowb
        for (int j = 0; j < sizebase; j++) {
 
            // j loops till cola
            for (int k = 0; k < sizeT; k++) {
 
                // l loops till colb
                for (int l = 0; l < sizeT; l++) {
 
                    // Each element of matrix A is
                    // multiplied by whole Matrix B
                    // resp and stored as Matrix C
                    A[(i * sizeT) + k][(j * sizeT) + l]= base[i][j] * T[k][l];
                }
            }
        }
    }
}


void include_I(int n){

    for (int i = 0; i< (n-sizeT); i++){
        A[i][sizeT+i] = 1;
        A[sizeT+i][i] = 1;
    }

}

double randfrom(double min, double max) 
{
    double range = (max - min); 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}