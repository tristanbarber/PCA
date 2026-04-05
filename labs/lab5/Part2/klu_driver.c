#include <stdio.h>
#include <stdlib.h>
#include "klu.h"

#define N 2000

int main(void)
{
    int n = N;
    int nnz = 3 * n - 2;

    int *Ap = malloc((n + 1) * sizeof(int));
    int *Ai = malloc(nnz * sizeof(int));
    double *Ax = malloc(nnz * sizeof(double));

    if (!Ap || !Ai || !Ax)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    int p = 0;
    Ap[0] = 0;

    for (int j = 0; j < n; j++)
    {
        if (j > 0)
        {
            Ai[p] = j - 1;
            Ax[p] = -1.0;
            p++;
        }

        Ai[p] = j;
        Ax[p] = 4.0;
        p++;

        if (j < n - 1)
        {
            Ai[p] = j + 1;
            Ax[p] = -1.0;
            p++;
        }

        Ap[j + 1] = p;
    }

    klu_common Common;
    klu_defaults(&Common);

    klu_symbolic *Symbolic = NULL;
    klu_numeric *Numeric = NULL;

    for (int iter = 0; iter < 200; iter++)
    {
        Symbolic = klu_analyze(n, Ap, Ai, &Common);
        if (Symbolic == NULL)
        {
            fprintf(stderr, "klu_analyze failed at iter %d, status=%d\n", iter, Common.status);
            free(Ap);
            free(Ai);
            free(Ax);
            return 1;
        }

        Numeric = klu_factor(Ap, Ai, Ax, Symbolic, &Common);
        if (Numeric == NULL)
        {
            fprintf(stderr, "klu_factor failed at iter %d, status=%d\n", iter, Common.status);
            klu_free_symbolic(&Symbolic, &Common);
            free(Ap);
            free(Ai);
            free(Ax);
            return 1;
        }

        klu_free_numeric(&Numeric, &Common);
        klu_free_symbolic(&Symbolic, &Common);
    }

    printf("Done\n");

    free(Ap);
    free(Ai);
    free(Ax);

    return 0;
}
