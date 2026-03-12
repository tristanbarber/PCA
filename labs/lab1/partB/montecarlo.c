/* Tristan Barber */
/* montecarlo.c */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MASTER       0   /* taskid of first task */
#define FROM_MASTER  1   /* setting a message type */
#define FROM_WORKER  2   /* setting a message type */

/* ttype: type to use for representing time */
typedef double ttype;

/* function prototypes */
ttype           tdiff(struct timespec a, struct timespec b);
struct timespec now(void);
void            init_rand_seed(void);
double          estimate_g(double lower_bound, double upper_bound, long long int N);
void            collect_results(double *result);

/* global structs for tracking program time */
struct timespec begin, end;


int main(int argc, char **argv) {
	/* start begin at the beginning of the program, as explained in class */
	begin = now();

	double result = 0.0;

	MPI_Init(&argc, &argv);

	float lower_bound = atof(argv[1]);
	float upper_bound = atof(argv[2]);
	long long int N   = atof(argv[3]);

	init_rand_seed();	/* using srand() */
	result = estimate_g(lower_bound, upper_bound, N);
	collect_results(&result);

	MPI_Finalize();
	return 0;
}


void init_rand_seed(void) {
    /* originally I just used the taskid to seed srand but using the current time allows for a different estimate between two runs of the same job script setup */
	int taskid;
    struct timespec ts;

	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    ts = now();

    srand((unsigned int)(ts.tv_nsec ^ ts.tv_sec ^ taskid));
}


double estimate_g(double lower_bound, double upper_bound, long long int N) {
	int numtasks;
	int taskid;
	int partialN;
	int aveN;
	int extra;
	int i;

	double x;
	double estimate = 0.0;

	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    /* this debug print is commented out so that it doesn't impact the timing calculation */
	// printf("task %d: N = %lld, numtasks = %d\n", taskid, N, numtasks);

	aveN = N / numtasks;
	extra = N % numtasks;
	partialN = (taskid < extra) ? aveN + 1 : aveN;

    /* this debug print is commented out so that it doesn't impact the timing calculation */
	// printf("task %d has a partialN of %d\n", taskid, partialN);

	begin = now();

	for (i = 0; i < partialN; i++) {
		x = lower_bound + (upper_bound - lower_bound) * ((double)rand() / RAND_MAX);

		estimate += (8 * sqrt(2 * M_PI)) / (exp((2 * x) * (2 * x)));
	}

	return ((upper_bound - lower_bound) / N) * estimate;
}


void collect_results(double *result) {
	int numtasks;
	int taskid;
	int mtype;
	int i;

	MPI_Status status;

	double worker_result;
	double sum = 0.0;
	double time_spent = 0.0;

	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	/**************************** master task ************************************/
	if (taskid == MASTER) {
		sum += *result;
		mtype = FROM_WORKER;

		for (i = 1; i < numtasks; i++) {
			MPI_Recv(&worker_result, 1, MPI_DOUBLE, i, mtype, MPI_COMM_WORLD, &status);
            sum += worker_result;
		}

		end = now();

        printf("master (task %d) result: %.8f\n", taskid, *result);

		/* algorithm to find the earliest begin for program timing calculation */
		for (i = 1; i < numtasks; i++) {
			struct timespec temp;

			MPI_Recv(&temp, sizeof(struct timespec), MPI_BYTE, i, mtype, MPI_COMM_WORLD, &status);

			if (tdiff(temp, end) > time_spent) {
				time_spent = tdiff(temp, end);
			}
		}

		printf("\n");
		printf("the monte carlo estimate is: %.8f\n", sum);
		printf("total time: %.8f sec\n", time_spent);
		printf("\n");
	}

	/**************************** worker task ************************************/
	if (taskid > MASTER) {
		mtype = FROM_WORKER;

		MPI_Send(result, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);

        printf("worker %d result: %.8f\n", taskid, *result);

		MPI_Send(&begin, sizeof(struct timespec), MPI_BYTE, MASTER, mtype, MPI_COMM_WORLD);
	}
}


/* Find the time difference */
ttype tdiff(struct timespec a, struct timespec b) {
	return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1E9;
}


/* Return the current time */
struct timespec now(void) {
	struct timespec t;

	clock_gettime(CLOCK_REALTIME, &t);
	return t;
}
