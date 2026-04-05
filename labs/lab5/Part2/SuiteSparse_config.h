#ifndef SUITESPARSE_CONFIG_H
#define SUITESPARSE_CONFIG_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <limits.h>

#define SuiteSparse_malloc(n, size) malloc((n) * (size))
#define SuiteSparse_free(ptr) free(ptr)
#define SUITESPARSE_PRINTF(params) printf params

static inline void *SuiteSparse_realloc
(
    size_t nnew,
    size_t nold,
    size_t size,
    void *p,
    int *ok
)
{
    (void) nold;
    void *pnew = realloc(p, nnew * size);
    if (ok != NULL)
    {
        *ok = (pnew != NULL);
    }
    return pnew;
}

#define SUITESPARSE_VER_CODE(main,sub) (((main) * 1000) + (sub))
#define SUITESPARSE__VERCODE(main,sub,patch) (((main) * 1000000) + ((sub) * 1000) + (patch))
#define SUITESPARSE__VERSION SUITESPARSE__VERCODE(7,11,0)

#endif
