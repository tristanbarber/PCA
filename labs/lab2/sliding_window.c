#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ttype: type to use for representing time */
typedef double ttype;

/* function prototypes */
ttype           tdiff(struct timespec a, struct timespec b);
struct timespec now(void);

/* global structs for tracking program time */
struct timespec begin, end;

void initialize_data(int N, int A[][N]);
int *distribute_data(int N, int A[][N]);
int *mask_operation(int *recv_buff, int N);
void collect_results(int *updated_buff, int N, int **Ap);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int N = atof(argv[1]);
    int A[N][N];
    int *Ap = NULL;
    begin = now();

    initialize_data(N, A);
    int *temp1 = distribute_data(N, A);
    int *temp2 = mask_operation(temp1, N);
    collect_results(temp2, N, &Ap);
    MPI_Finalize();

    free(temp1);
    free(temp2);
    free(Ap);
    return 0;
}

void initialize_data(int N, int A[][N]) {
    int rank, size;     // for storing this process' rank, and the number of processes
    int i, j;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(1);

    // initialize and print original array
    if (0 == rank) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                A[i][j] = rand() % 256;
            }
        }

        printf ("\n");
        printf ("Initial %d X %d data array:", N, N);
        printf ("\n");

        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                printf ("%d\t", A[i][j]);
            }
            printf ("\n");
        }
        printf ("\n");
    }
}

int *distribute_data(int N, int A[][N]) {
    int rank, size;
    int displs_tracker = 0;
    int i, j;
    int k = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *sendcounts = malloc(sizeof(int)*size);
    int *displs = malloc(sizeof(int)*size);

    int base_rows = (N - 2) / size;
    int rem = (N - 2) % size;

    for (i = 0; i < size; i++) {
        sendcounts[i] = N * (base_rows + 2);

        if (rem > 0) {
            sendcounts[i] += N;
            rem--;
        }

        displs[i] = displs_tracker;
        displs_tracker += sendcounts[i] - 2*N;
    }

    // print calculated displacements for each process
    if (0 == rank) {
        printf ("\n");
        for (i = 0; i < size; i++) {
            printf("scatterv: sendcounts[%d] = %d\tdispls[%d] = %d\n", i, sendcounts[i], i, displs[i]);
        }
        printf ("\n");
    }

    int *rec_buf = malloc(sizeof(int)*sendcounts[rank]);

    MPI_Scatterv(A, sendcounts, displs, MPI_INT, rec_buf, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

    // print what each process received
    printf("\nRank %d partial matrix:\n", rank);
    for (i = 0; i < sendcounts[rank]; i++) {
        if(i > 0 && i % N == 0) {
            printf("\n");
        }

        printf("%d\t", rec_buf[i]);
    }
    printf("\n\n");

    free(sendcounts);
    free(displs);

    return rec_buf;
}

int *mask_operation(int *recv_buff, int N) {
    int rank, size;
    int a, b, c, d, e, f, g, h, i, j, k;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int base_rows = (N - 2) / size;
    int rem = (N - 2) % size;
    int local_rows = base_rows + (rank < rem ? 1 : 0);
    int rows = local_rows + 2;
    int elements = rows * N;

    int *updated_buff = calloc(rows * N, sizeof(int));

    // first copy recv_buff into updated_buff (so we have data for the edge rows + columns)
    for(j = 0; j < rows; j++) {
        for(k = 0; k < N; k++) {
            updated_buff[j * N + k] = recv_buff[j * N + k];
        }
    }

    // do the image processing
    for(j = 1; j < rows - 1; j++) {
        for(k = 1; k < N - 1; k++) {
            a = recv_buff[(j-1)*N + (k-1)];
            b = recv_buff[(j-1)*N + k];
            c = recv_buff[(j-1)*N + (k+1)];
            d = recv_buff[j*N + (k-1)];
            e = recv_buff[j*N + k];
            f = recv_buff[j*N + (k+1)];
            g = recv_buff[(j+1)*N +(k-1)];
            h = recv_buff[(j+1)*N + k];
            i = recv_buff[(j+1)*N + (k+1)];
            updated_buff[j * N + k] = (a + b + c + d + 2*e + f + g + h + i)/10;
        }
    }

    // print updated buff output
    printf("Rank %d matrix after mask operation:", rank);
    for (i = 0; i < elements; i++) {
        if(i % N == 0) {
            printf("\n");
        }

        printf("%d\t", updated_buff[i]);
    }
    printf("\n\n");

    return updated_buff;
}

void collect_results(int *updated_buff, int N, int **Ap) {
    int rank, size;
    int i;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int base_rows = (N - 2) / size;
    int rem = (N - 2) % size;

    int *sendcounts = malloc(sizeof(int) * size);
    int *displs     = malloc(sizeof(int) * size);

    int offset = 0;

    // sendcounts and displacements
    for (i = 0; i < size; i++) {

        int local_rows = base_rows + (i < rem ? 1 : 0);

        if (i == 0) {
            sendcounts[i] = (local_rows + 1) * N;
        } else if (i == size - 1) {
            sendcounts[i] = (local_rows + 1) * N;
        } else {
            sendcounts[i] = local_rows * N;
        }

        displs[i] = offset;
        offset += sendcounts[i];
    }

    if (rank == 0) {
        *Ap = malloc(N * N * sizeof(int));
    }

    int *sendptr;

    if (rank == 0) {
        sendptr = &updated_buff[0];
    } else {
        sendptr = &updated_buff[N];
    }

    MPI_Gatherv(sendptr, sendcounts[rank], MPI_INT, *Ap, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    end = now();

    struct timespec *begins_buf = NULL;
    int *time_counts = malloc(size * sizeof(int));
    int *time_displs = malloc(size * sizeof(int));

    for (i = 0; i < size; i++) {
        time_counts[i] = sizeof(struct timespec);
        time_displs[i] = i * sizeof(struct timespec);
    }

    if (rank == 0) {
        begins_buf = malloc(size * sizeof(struct timespec));
    }

    MPI_Gatherv(&begin,
                sizeof(struct timespec),
                MPI_BYTE,
                begins_buf,
                time_counts,
                time_displs,
                MPI_BYTE,
                0,
                MPI_COMM_WORLD);

    if (rank == 0) {

        double time_spent = tdiff(begins_buf[0], end);

        for (i = 1; i < size; i++) {
            double candidate = tdiff(begins_buf[i], end);
            if (candidate > time_spent)
                time_spent = candidate;
        }

        printf("Final processed %d x %d matrix:\n", N, N);

        for (i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d\t", (*Ap)[i * N + j]);
            }
            printf("\n");
        }

        printf("\ntotal time: %.8f sec\n\n", time_spent);

        free(begins_buf);
    }

    free(sendcounts);
    free(displs);
    free(time_counts);
    free(time_displs);
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
