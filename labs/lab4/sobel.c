#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WIDTH 5000
#define HEIGHT 5000

uint8_t input[HEIGHT][WIDTH];
uint8_t output[HEIGHT][WIDTH];

void read_image(const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        perror("Error opening input file");
        exit(1);
    }

    for (int r = 0; r < HEIGHT; r++)
    {
        for (int c = 0; c < WIDTH; c++)
        {
            int val;
            fscanf(fp, "%d", &val);
            input[r][c] = (uint8_t)val;
        }
    }

    fclose(fp);
}

void write_image(const char *filename)
{
    FILE *fp = fopen(filename, "w");

    if (!fp)
    {
        perror("Error opening output file");
        exit(1);
    }

    for (int r = 0; r < HEIGHT; r++)
    {
        for (int c = 0; c < WIDTH; c++)
        {
            fprintf(fp, "%d ", output[r][c]);
        }

        fprintf(fp, "\n");
    }

    fclose(fp);
}

void sobel()
{
    // skip the outer borders like lab 3
    for (int r = 1; r < HEIGHT - 1; r++) {
        for (int c = 1; c < WIDTH - 1; c++) {
            int p1 = input[r-1][c-1];
            int p2 = input[r-1][c];
            int p3 = input[r-1][c+1];
            int q1 = input[r][c-1];
            int q2 = input[r][c];
            int q3 = input[r][c+1];
            int r1 = input[r+1][c-1];
            int r2 = input[r+1][c];
            int r3 = input[r+1][c+1];

            int qx = (p1 - r1) + 2*(p2 - r2) + (p3 - r3);
            int qy = (p1 - p3) + 2*(q1 - q3) + (r1 - r3);
            int magnitude = abs(qx) + abs(qy);

            // clamp results to 8-bit
            if (magnitude > 255)
                magnitude = 255;

            output[r][c] = (uint8_t)magnitude;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s input.txt\n", argv[0]);
        return 1;
    }

    read_image(argv[1]);

    sobel();

    write_image("output.txt");

    printf("Sobel edge detection complete.\n");

    return 0;
}
