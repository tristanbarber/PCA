/* Tristan Barber */
/*lab3_part1.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

/* ttype: type to use for representing time */
typedef double ttype;

/* Find the time difference. */
ttype tdiff(struct timespec a, struct timespec b) {
   ttype dt = (( b.tv_sec - a.tv_sec ) + ( b.tv_nsec - a.tv_nsec ) / 1E9);
   return dt;
}

/* Return the current time. */
struct timespec now() {
   struct timespec t;
   clock_gettime(CLOCK_REALTIME, &t);
   return t;
}

int main (int argc, char *argv[])
{
    int NRA = atoi(argv[1]);
    int NCA = atoi(argv[2]);
    int NRB = NCA;
    int NCB = atoi(argv[3]);
    int A[NRA][NCA];
    int B[NRB][NCB];
    int C[NRA][NCB];
    int numthreads,          /* number of threads */
        taskid,              /* thread identifier */
        i, j, k;             /* misc */

    struct timespec begin, end;
    double time_spent;

    printf ("\n");
    printf("Matrix A: #rows %d; #cols %d\n", NRA, NCA);
    printf("Matrix B: #rows %d; #cols %d\n", NRB, NCB);
    printf ("\n");

    #pragma omp parallel private(taskid,i,j,k) shared(A,B,C)
    {
        taskid = omp_get_thread_num();

        #pragma omp master
        {
            numthreads = omp_get_num_threads();
            printf("Using %d threads.\n", numthreads);
            printf("Initializing arrays...\n\n");

            for (i=0; i<NRA; i++) {
                for (j=0; j<NCA; j++) {
                    A[i][j]= i+j;
                }
            }

            printf ("Contents of matrix A:\n");
            for (i=0; i<NRA; i++) {
                for (j=0; j<NCA; j++) {
                    printf("%d\t", A[i][j]);
                }
                printf("\n");
            }
            printf("\n");

            for (i=0; i<NRB; i++) {
                for (j=0; j<NCB; j++) {
                    B[i][j]= i-j;
                }
            }

            printf ("Contents of matrix B:\n");
            for (i=0; i<NRB; i++) {
                for (j=0; j<NCB; j++) {
                    printf("%d\t", B[i][j]);
                }
                printf("\n");
            }
            printf("\n");

            begin = now();
        }

        #pragma omp barrier

        #pragma omp for
        for (i=0; i<NRA; i++) {
            for (j=0; j<NCB; j++) {
                C[i][j] = 0;
                for (k=0; k<NCA; k++) {
                    C[i][j] = C[i][j] + A[i][k] * B[k][j];
                }
            }
        }

        #pragma omp barrier

        #pragma omp master
        {
            end = now();
        }
    }

    time_spent = tdiff(begin, end);

    printf("******************************************************\n");
    printf("Result Matrix:\n");

    for (i=0; i<NRA; i++) {
        for (j=0; j<NCB; j++) {
            printf("%d\t", C[i][j]);
        }
        printf("\n");
    }

    printf("******************************************************\n");
    printf ("\n");
    printf("total time: %.8f sec\n", time_spent);
    printf ("\n");

    return 0;
    }
