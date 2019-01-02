/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#ifndef _DALEC_H_
#define _DALEC_H_

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
/* this only fails if the C compiler lies about having full C11 support */
#include <stdnoreturn.h>
#define DALEC_NORETURN_PREFIX noreturn
#define DALEC_NORETURN_SUFFIX
#elif defined (__GNUC__) && (__GNUC__ >= 3)
/* this attribute was introduced in GCC 2.5 */
#define DALEC_NORETURN_PREFIX
#define DALEC_NORETURN_SUFFIX __attribute__((noreturn))
#endif

#include <mpi.h>

enum {
    DALEC_SUCCESS = 0,
    DALEC_INPUT_ERROR = 1,
    DALEC_ERROR_MPI_LIBRARY = 2,
    DALEC_ERROR_MPI_USAGE = 4
} dalec_rc_t;

/* This becomes part of the ABI.  If this is undesirable, we can instead
 * use pointers for dims and blocksizes, but this will add overhead
 * because every use will have to dereference a heap pointer. */

#define DALEC_ARRAY_MAX_DIM 4

typedef struct DALEC_Array_descriptor {
    MPI_Comm comm;
    MPI_Datatype type;
    int ndim;
    size_t dims[DALEC_ARRAY_MAX_DIM];
    size_t blocksizes[DALEC_ARRAY_MAX_DIM];
    char * name;
} DALEC_Array_descriptor;

typedef struct DALEC_Array_handle {
    MPI_Win win;
    MPI_Datatype type;
    int ndim;
    size_t dims[DALEC_ARRAY_MAX_DIM];
    size_t blocksizes[DALEC_ARRAY_MAX_DIM];
#if 0
    int win_keyval;
#endif
} DALEC_Array_handle;

#ifndef _GENERATE_DALEC_PUBLIC_API_
#define _GENERATE_DALEC_PUBLIC_API_

#define DALEC_PUBLIC_API
#include "dalec_api.h"
#undef DALEC_PUBLIC_API

#endif /* _GENERATE_DALEC_PUBLIC_API_ */

#ifndef _GENERATE_DALEC_PROFILE_API_
#define _GENERATE_DALEC_PROFILE_API_

#define DALEC_PROFILE_API
#include "dalec_api.h"

#endif /* _GENERATE_DALEC_PROFILE_API_ */

#endif /* _DALEC_H_ */
