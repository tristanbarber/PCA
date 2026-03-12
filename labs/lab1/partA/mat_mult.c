/* Tristan Barber */
/*mat_mult.c */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NRA 60                 /* number of rows in matrix A */
#define NCA 12                 /* number of columns in matrix A */
#define NRB 12                 /* number of rows in matrix B, should equal to NCA */
#define NCB 10                  /* number of columns in matrix B */
#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

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
   int numtasks,              /* number of tasks in partition */
       taskid,                /* a task identifier */
       numworkers,            /* number of worker tasks */
       source,                /* task id of message source */
       dest,                  /* task id of message destination */
       mtype,                 /* message type */
       rows,                  /* rows of matrix A sent to each worker */
       averow, extra, offset, /* used to determine rows sent to each worker */
       i, j, k, rc;           /* misc */
   int A[NRA][NCA],          /* matrix A to be multiplied */
       B[NRB][NCB],           /* matrix B to be multiplied */
       C[NRA][NCB];           /* result matrix C */
   MPI_Status status;

   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
   MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

   if (numtasks < 2 ) {
      printf("Need at least two MPI tasks. Quitting...\n");
      MPI_Abort(MPI_COMM_WORLD, rc);
      exit(1);
   }

   numworkers = numtasks-1;

   //clock_t begin, end;
   struct timespec begin, end;
   double time_spent;
   double wcalcTime_max = 0;
   double wcalcTime_sum = 0;
   double *wcalcTime_each = malloc(numworkers * sizeof(double));

   /**************************** master task ************************************/
   if (taskid == MASTER)
   {
      // Do some basic error checking
      if (NRB != NCA) {
        printf ("Matrix a column size must equal matrix b row size.\n");
        return 1;
      }

      printf("mpi_mm has started with %d tasks.\n",numtasks);
      printf("Matrix A: #rows %d; #cols %d\n", NRA, NCA);
      printf("Matrix B: #rows %d; #cols %d\n", NRB, NCB);
      printf ("\n");

      printf("Initializing arrays...\n");
      for (i=0; i<NRA; i++)
         for (j=0; j<NCA; j++)
            A[i][j]= i+j;

      printf (" Contents of matrix A\n");
      for (i=0; i<NRA; i++) {
        for (j=0; j<NCA; j++)
        printf("%d\t", A[i][j]);
        printf("\n");
      }

      for (i=0; i<NRB; i++)
         for (j=0; j<NCB; j++)
            B[i][j]= i-j;

      printf (" Contents of matrix B\n");
      for (i=0; i<NRB; i++) {
        for (j=0; j<NCB; j++)
        printf("%d\t", B[i][j]);
        printf("\n");
        printf("\n");
      }


      /* Send matrix data to the worker tasks */
      averow = NRA/numworkers;
      extra = NRA%numworkers;
      offset = 0;
      mtype = FROM_MASTER;

	    //begin = clock();
	    begin = now();

      for (dest=1; dest<=numworkers; dest++)
      {
         rows = (dest <= extra) ? averow+1 : averow;
         printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
         MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&A[offset][0], rows*NCA, MPI_INT, dest, mtype,
                   MPI_COMM_WORLD);
         MPI_Send(&B, NRB*NCB, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         offset = offset + rows;
      }

      /* Receive results from worker tasks */
      mtype = FROM_WORKER;
      for (i=1; i<=numworkers; i++)
      {
         source = i;
         MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&C[offset][0], rows*NCB, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&wcalcTime_each[i - 1], 1, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
         printf("Received results from task %d\n",source);
      }

        //end = clock();
        end = now();
        time_spent = tdiff(begin, end);

      /* Print results */
      printf ("\n");
      printf("******************************************************\n");
      printf("Result Matrix:\n");
      for (i=0; i<NRA; i++)
      {
         printf("\n");
         for (j=0; j<NCB; j++)
            printf("%d\t", C[i][j]);
      }

      printf("\n******************************************************\n");
      printf ("\n");
      for(int i=1; i<=numworkers; i++){
        printf("wcalcTime worker %d: %.8f sec\n", i, wcalcTime_each[i - 1]);
        wcalcTime_sum += wcalcTime_each[i - 1];
        if(wcalcTime_each[i - 1] > wcalcTime_max) {
            wcalcTime_max = wcalcTime_each[i - 1];
        }
      }
      printf ("\n");

      printf("maximum wcalcTime for single worker: %.8f sec\n", wcalcTime_max);
      printf ("\n");

      printf("sum of wcalcTime for all workers: %.8f sec\n", wcalcTime_sum);
      printf ("\n");

      printf("total time: %.8f sec\n", time_spent);
      printf ("\n");
   }


/**************************** worker task ************************************/
   if (taskid > MASTER)
   {
      mtype = FROM_MASTER;
      MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&A, rows*NCA, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&B, NRB*NCB, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);

      begin = now();

      for (i=0; i<rows; i++)
         for (j=0; j<NCB; j++)
         {
           C[i][j] = 0;
           for (k=0; k<NCA; k++)
              C[i][j] = C[i][j] + A[i][k] * B[k][j];
         }

      end = now();
      time_spent = tdiff(begin, end);

      mtype = FROM_WORKER;
      MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&C, rows*NCB, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&time_spent, 1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
   }
   MPI_Finalize();

   free(wcalcTime_each);
}
