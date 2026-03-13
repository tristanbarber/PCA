#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>

#define WIDTH  5000
#define HEIGHT 5000

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

void read_image(const char *filename, uint8_t *img) {
    FILE *fp = fopen(filename, "r");

    if (!fp) {
        perror("Error opening input file");
        exit(1);
    }

    for (int r = 0; r < HEIGHT; r++) {
        for (int c = 0; c < WIDTH; c++) {
            int val;
            fscanf(fp, "%d", &val);
            img[r * WIDTH + c] = (uint8_t)val;
        }
    }
    fclose(fp);
}

void write_image(const char *filename, uint8_t *img) {
    FILE *fp = fopen(filename, "w");

    if (!fp) {
        perror("Error opening output file");
        exit(1);
    }
    for (int r = 0; r < HEIGHT; r++) {
        for (int c = 0; c < WIDTH; c++) {
            fprintf(fp, "%d ", img[r * WIDTH + c]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void sobel(uint8_t *local_in, uint8_t *local_out, int local_rows) {
    #pragma omp parallel for schedule(static)
    for (int r = 1; r < local_rows - 1; r++) {
        for (int c = 1; c < WIDTH - 1; c++) {
            int p1 = local_in[(r-1)*WIDTH + c-1];
            int p2 = local_in[(r-1)*WIDTH + c];
            int p3 = local_in[(r-1)*WIDTH + c+1];
            int q1 = local_in[(r  )*WIDTH + c-1];
            int q3 = local_in[(r  )*WIDTH + c+1];
            int s1 = local_in[(r+1)*WIDTH + c-1];
            int s2 = local_in[(r+1)*WIDTH + c];
            int s3 = local_in[(r+1)*WIDTH + c+1];

            int qx = (p1 - s1) + 2*(p2 - s2) + (p3 - s3);
            int qy = (p1 - p3) + 2*(q1 - q3) + (s1 - s3);

            int mag = abs(qx) + abs(qy);

            if (mag > 255) {
                mag = 255;
            }

            local_out[r*WIDTH + c] = (uint8_t)mag;
        }
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    struct timespec begin, end;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (argc != 2) {
        if (rank == 0) printf("Usage: %s input.txt\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    // setting up scatterv & gatherv sendcounts and displs based on lab3,
    // with extra rows above and below
    int sendcounts[nprocs], displs[nprocs];
    int base_rows = (HEIGHT - 2) / nprocs, rem = (HEIGHT - 2) % nprocs;
    int displs_tracker = 0;
    for (int i = 0; i < nprocs; i++) {
        sendcounts[i] = WIDTH * (base_rows + 2);
        if (rem-- > 0) sendcounts[i] += WIDTH;
        displs[i] = displs_tracker;
        displs_tracker += sendcounts[i] - 2 * WIDTH;
    }
    int local_rows = sendcounts[rank] / WIDTH;

    int recvcounts[nprocs], rdispls[nprocs];
    int rdispls_tracker = 0;
    for (int i = 0; i < nprocs; i++) {
        recvcounts[i] = sendcounts[i] - 2 * WIDTH;
        rdispls[i] = rdispls_tracker;
        rdispls_tracker += recvcounts[i];
    }

    uint8_t *input  = NULL;
    uint8_t *output = NULL;
    if (rank == 0) {
        input  = malloc(HEIGHT * WIDTH);
        output = malloc(HEIGHT * WIDTH);
        read_image(argv[1], input);
    }

    uint8_t *local_in  = calloc(local_rows * WIDTH, 1);
    uint8_t *local_out = calloc(local_rows * WIDTH, 1);

    begin = now();

    MPI_Scatterv(input, sendcounts, displs, MPI_UINT8_T, local_in, sendcounts[rank], MPI_UINT8_T, 0, MPI_COMM_WORLD);

    sobel(local_in, local_out, local_rows);

    MPI_Gatherv(local_out + WIDTH, recvcounts[rank], MPI_UINT8_T, output, recvcounts, rdispls, MPI_UINT8_T, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        write_image("output.txt", output);
        end = now();

        printf("Sobel edge detection complete.\n");
        printf("total time: %.8f sec\n", tdiff(begin, end));

        free(input);
        free(output);
    }

    free(local_in);
    free(local_out);
    MPI_Finalize();
    return 0;
}
