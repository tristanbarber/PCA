/* Tristan Barber */
/*lab3_part2.c */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

typedef double ttype;

ttype tdiff(struct timespec a, struct timespec b) {
   ttype dt = (( b.tv_sec - a.tv_sec ) + ( b.tv_nsec - a.tv_nsec ) / 1E9);
   return dt;
}

struct timespec now() {
   struct timespec t;
   clock_gettime(CLOCK_REALTIME, &t);
   return t;
}

#define MASTER 0
#define FROM_MASTER 1
#define FROM_WORKER 2

int main(int argc, char *argv[]) {
    int numtasks, taskid, numworkers;
    int source, dest, rows, averow, extra, offset;
    int i, j, k;

    struct timespec begin, end;
    double time_spent;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    int NRA = atoi(argv[1]);
    int NCA_RB = atoi(argv[2]);
    int NCB = atoi(argv[3]);

    if (numtasks < 2) {
        if (taskid == MASTER) {
            printf("Need at least two MPI tasks.\n");
        }
        MPI_Finalize();
    }

    numworkers = numtasks - 1;

    int A[NRA][NCA_RB];
    int B[NCA_RB][NCB];
    int C[NRA][NCB];

/************************ MASTER ************************/
    if (taskid == MASTER) {

        printf("Matrix A: %d x %d\n", NRA, NCA_RB);
        printf("Matrix B: %d x %d\n", NCA_RB, NCB);
        printf("MPI tasks: %d\n", numtasks);
        printf("OpenMP threads per process: %d\n\n", omp_get_max_threads());

        #pragma omp parallel for default(none) shared(A,NRA,NCA_RB) private(i,j)
        for (i = 0; i < NRA; i++) {
            for (j = 0; j < NCA_RB; j++) {
                A[i][j] = i + j;
            }
        }

        #pragma omp parallel for default(none) shared(B,NCA_RB,NCB) private(i,j)
        for (i = 0; i < NCA_RB; i++) {
            for (j = 0; j < NCB; j++) {
                B[i][j] = i - j;
            }
        }

        printf ("Contents of matrix A:\n");
        for (i=0; i<NRA; i++) {
            for (j=0; j<NCA_RB; j++) {
                printf("%d\t", A[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        printf ("Contents of matrix B:\n");
        for (i=0; i<NCA_RB; i++) {
            for (j=0; j<NCB; j++) {
                printf("%d\t", B[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        averow = NRA / numworkers;
        extra  = NRA % numworkers;
        offset = 0;

        begin = now();

        for (dest = 1; dest <= numworkers; dest++) {
            rows = (dest <= extra) ? averow + 1 : averow;

            MPI_Send(&offset, 1, MPI_INT, dest, FROM_MASTER, MPI_COMM_WORLD);
            MPI_Send(&rows, 1, MPI_INT, dest, FROM_MASTER, MPI_COMM_WORLD);
            MPI_Send(&A[offset][0], rows * NCA_RB, MPI_INT, dest, FROM_MASTER, MPI_COMM_WORLD);
            MPI_Send(&B, NCA_RB * NCB, MPI_INT, dest, FROM_MASTER, MPI_COMM_WORLD);

            offset += rows;
        }

        for (i = 1; i <= numworkers; i++) {
            source = i;

            MPI_Recv(&offset, 1, MPI_INT, source, FROM_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&rows, 1, MPI_INT, source, FROM_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&C[offset][0], rows * NCB, MPI_INT, source, FROM_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        end = now();
        time_spent = tdiff(begin, end);

        /* Print results */
        printf("******************************************************\n");
        printf("Result Matrix:\n");
        for (i=0; i<NRA; i++)
        {
            printf("\n");
            for (j=0; j<NCB; j++)
                printf("%d\t", C[i][j]);
        }
        printf ("\n\n");

        printf("Total time: %.6f seconds\n", time_spent);
    }

/************************ WORKERS ************************/
    if (taskid > MASTER) {

        MPI_Recv(&offset, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&rows, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&A, rows * NCA_RB, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&B, NCA_RB * NCB, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        #pragma omp parallel for default(none) shared(A,B,C,rows,NCA_RB,NCB) private(i,j,k)
        for (i = 0; i < rows; i++) {
            for (j = 0; j < NCB; j++) {
                C[i][j] = 0;
                for (k = 0; k < NCA_RB; k++) {
                    C[i][j] += A[i][k] * B[k][j];
                }
            }
        }

        MPI_Send(&offset, 1, MPI_INT, MASTER, FROM_WORKER, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, MASTER, FROM_WORKER, MPI_COMM_WORLD);
        MPI_Send(&C, rows * NCB, MPI_INT, MASTER, FROM_WORKER, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
