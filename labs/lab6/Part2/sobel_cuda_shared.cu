// Tristan Barber & Jarrell Reynolds

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <cuda_runtime.h>

#define WIDTH  5000
#define HEIGHT 5000

uint8_t h_input [HEIGHT][WIDTH];
uint8_t h_output[HEIGHT][WIDTH];

typedef double ttype;

ttype tdiff(struct timespec a, struct timespec b) {
    return (b.tv_sec  - a.tv_sec ) +
           (b.tv_nsec - a.tv_nsec) / 1E9;
}

struct timespec now_ts() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t;
}

/* grid(WIDTH*HEIGHT/1000, 1, 1)  block(1000, 1, 1) */
__global__ void sobel_1d_shared(const uint8_t * __restrict__ in, uint8_t * __restrict__ out, int width, int height) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    int r = tid / width;
    int c = tid % width;
    int tx = threadIdx.x;
    int bw = blockDim.x;
    int smem_w = bw + 2;

    extern __shared__ uint8_t smem[];

    smem[0 * smem_w + (tx + 1)] = (r - 1 >= 0 && c >= 0 && c < width) ? in[(r-1)*width + c] : 0;
    smem[1 * smem_w + (tx + 1)] = (r >= 0 && r < height && c >= 0 && c < width) ? in[r*width + c] : 0;
    smem[2 * smem_w + (tx + 1)] = (r + 1 < height && c >= 0 && c < width) ? in[(r+1)*width + c] : 0;

    if (tx == 0) {
        smem[0 * smem_w + 0] = (r - 1 >= 0 && c - 1 >= 0) ? in[(r-1)*width + (c-1)] : 0;
        smem[1 * smem_w + 0] = (r >= 0 && r < height && c - 1 >= 0) ? in[r*width + (c-1)] : 0;
        smem[2 * smem_w + 0] = (r + 1 < height && c - 1 >= 0) ? in[(r+1)*width + (c-1)] : 0;
    }

    if (tx == bw - 1) {
        smem[0 * smem_w + (bw + 1)] = (r - 1 >= 0 && c + 1 < width) ? in[(r-1)*width + (c+1)] : 0;
        smem[1 * smem_w + (bw + 1)] = (r >= 0 && r < height && c + 1 < width) ? in[r*width + (c+1)] : 0;
        smem[2 * smem_w + (bw + 1)] = (r + 1 < height && c + 1 < width) ? in[(r+1)*width + (c+1)] : 0;
    }

    __syncthreads();

    if (tid >= width * height) return;
    if (r == 0 || r == height - 1 || c == 0 || c == width - 1) {
        out[tid] = 0;
        return;
    }

    int sc = tx + 1;
    int p1 = smem[0 * smem_w + (sc - 1)];
    int p2 = smem[0 * smem_w + sc];
    int p3 = smem[0 * smem_w + (sc + 1)];
    int q1 = smem[1 * smem_w + (sc - 1)];
    int q3 = smem[1 * smem_w + (sc + 1)];
    int r1 = smem[2 * smem_w + (sc - 1)];
    int r2 = smem[2 * smem_w + sc];
    int r3 = smem[2 * smem_w + (sc + 1)];

    int qx = (p1 - r1) + 2*(p2 - r2) + (p3 - r3);
    int qy = (p1 - p3) + 2*(q1 - q3) + (r1 - r3);
    int mag = abs(qx) + abs(qy);
    if (mag > 255) mag = 255;

    out[tid] = (uint8_t)mag;
}

/* grid(WIDTH/100, HEIGHT/10, 1)  block(100, 10, 1) */
__global__ void sobel_2d_shared(const uint8_t * __restrict__ in, uint8_t * __restrict__ out, int width, int height) {
    int c = blockDim.x * blockIdx.x + threadIdx.x;
    int r = blockDim.y * blockIdx.y + threadIdx.y;
    int tx = threadIdx.x;
    int ty = threadIdx.y;
    int bw = blockDim.x;
    int bh = blockDim.y;
    int smem_w = bw + 2;

    extern __shared__ uint8_t smem[];

    smem[(ty + 1) * smem_w + (tx + 1)] = (r >= 0 && r < height && c >= 0 && c < width) ? in[r*width + c] : 0;

    if (ty == 0)
        smem[0 * smem_w + (tx + 1)] = (r - 1 >= 0 && c >= 0 && c < width) ? in[(r-1)*width + c] : 0;
    if (ty == bh - 1)
        smem[(bh + 1) * smem_w + (tx + 1)] = (r + 1 < height && c >= 0 && c < width) ? in[(r+1)*width + c] : 0;
    if (tx == 0)
        smem[(ty + 1) * smem_w + 0] = (r >= 0 && r < height && c - 1 >= 0) ? in[r*width + (c-1)] : 0;
    if (tx == bw - 1)
        smem[(ty + 1) * smem_w + (bw + 1)] = (r >= 0 && r < height && c + 1 < width) ? in[r*width + (c+1)] : 0;

    if (tx == 0 && ty == 0)
        smem[0 * smem_w + 0] = (r - 1 >= 0 && c - 1 >= 0) ? in[(r-1)*width + (c-1)] : 0;
    if (tx == bw - 1 && ty == 0)
        smem[0 * smem_w + (bw + 1)] = (r - 1 >= 0 && c + 1 < width) ? in[(r-1)*width + (c+1)] : 0;
    if (tx == 0 && ty == bh - 1)
        smem[(bh + 1) * smem_w + 0] = (r + 1 < height && c - 1 >= 0) ? in[(r+1)*width + (c-1)] : 0;
    if (tx == bw - 1 && ty == bh - 1)
        smem[(bh + 1) * smem_w + (bw + 1)] = (r + 1 < height && c + 1 < width) ? in[(r+1)*width + (c+1)] : 0;

    __syncthreads();

    if (r >= height || c >= width) return;
    if (r == 0 || r == height - 1 || c == 0 || c == width - 1) {
        out[r*width + c] = 0;
        return;
    }

    int sr = ty + 1;
    int sc = tx + 1;
    int p1 = smem[(sr - 1) * smem_w + (sc - 1)];
    int p2 = smem[(sr - 1) * smem_w + sc];
    int p3 = smem[(sr - 1) * smem_w + (sc + 1)];
    int q1 = smem[sr * smem_w + (sc - 1)];
    int q3 = smem[sr * smem_w + (sc + 1)];
    int r1 = smem[(sr + 1) * smem_w + (sc - 1)];
    int r2 = smem[(sr + 1) * smem_w + sc];
    int r3 = smem[(sr + 1) * smem_w + (sc + 1)];

    int qx = (p1 - r1) + 2*(p2 - r2) + (p3 - r3);
    int qy = (p1 - p3) + 2*(q1 - q3) + (r1 - r3);
    int mag = abs(qx) + abs(qy);
    if (mag > 255) mag = 255;

    out[r*width + c] = (uint8_t)mag;
}

void read_image(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("Error opening input file"); exit(1); }
    for (int r = 0; r < HEIGHT; r++)
        for (int c = 0; c < WIDTH; c++) {
            int val;
            fscanf(fp, "%d", &val);
            h_input[r][c] = (uint8_t)val;
        }
    fclose(fp);
}

void write_image(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { perror("Error opening output file"); exit(1); }
    for (int r = 0; r < HEIGHT; r++) {
        for (int c = 0; c < WIDTH; c++)
            fprintf(fp, "%d ", h_output[r][c]);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void run_config(const char *label, const uint8_t *d_in, uint8_t *d_out, int config, dim3 grid, dim3 block) {

    size_t smem_bytes;
    if (config == 1)
        smem_bytes = 3 * (block.x + 2) * sizeof(uint8_t);
    else
        smem_bytes = (block.y + 2) * (block.x + 2) * sizeof(uint8_t);

    cudaMemset(d_out, 0, (size_t)WIDTH * HEIGHT);

    if (config == 1)
        sobel_1d_shared<<<grid, block, smem_bytes>>>(d_in, d_out, WIDTH, HEIGHT);
    else
        sobel_2d_shared<<<grid, block, smem_bytes>>>(d_in, d_out, WIDTH, HEIGHT);

    cudaDeviceSynchronize();

    cudaMemcpy(h_output, d_out, (size_t)WIDTH * HEIGHT, cudaMemcpyDeviceToHost);

    printf("  grid=(%u,%u,%u)  block=(%u,%u,%u)\n\n", grid.x, grid.y, grid.z, block.x, block.y, block.z);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s input.txt <config>\n", argv[0]);
        printf("  config: 1  ->  1-D grid(WIDTH*HEIGHT/1000, 1, 1) block(1000, 1, 1)\n");
        printf("          2  ->  2-D grid(WIDTH/100, HEIGHT/10, 1)  block(100, 10, 1)\n");
        return 1;
    }

    int config = atoi(argv[2]);
    if (config != 1 && config != 2) {
        fprintf(stderr, "Error: config must be 1 or 2\n");
        return 1;
    }

    struct timespec prog_start, prog_end;
    prog_start = now_ts();

    read_image(argv[1]);

    uint8_t *d_in  = NULL;
    uint8_t *d_out = NULL;
    size_t   sz    = (size_t)WIDTH * HEIGHT;

    cudaMalloc(&d_in,  sz);
    cudaMalloc(&d_out, sz);
    cudaMemcpy(d_in, h_input, sz, cudaMemcpyHostToDevice);

    if (config == 1) {
        dim3 dimBlock(1000, 1, 1);
        dim3 dimGrid(WIDTH * HEIGHT / dimBlock.x, 1, 1);

        run_config("Config 1: 1-D grid(WIDTH*HEIGHT/1000,1,1) block(1000,1,1)", d_in, d_out, 1, dimGrid, dimBlock);
        write_image("output_shared_1.txt");
    } else {
        dim3 dimBlock(100, 10, 1);
        dim3 dimGrid(WIDTH / dimBlock.x, HEIGHT / dimBlock.y, 1);

        run_config("Config 2: 2-D grid(WIDTH/100,HEIGHT/10,1) block(100,10,1)", d_in, d_out, 2, dimGrid, dimBlock);
        write_image("output_shared_2.txt");
    }

    cudaFree(d_in);
    cudaFree(d_out);

    prog_end = now_ts();
    printf("\nTotal program wall-clock time: %.8f sec\n\n", tdiff(prog_start, prog_end));

    return 0;
}
