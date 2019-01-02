/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#ifndef _DALEC_H_
#define _DALEC_H_

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ == 201112L)
#include <stdnoreturn.h>
#define DALEC_NORETURN_PREFIX noreturn
#define DALEC_NORETURN_SUFFIX
#elif defined (__GNUC__)
#define DALEC_NORETURN_PREFIX
#define DALEC_NORETURN_SUFFIX __attribute__((noreturn))
#endif

#include <mpi.h>

#define DALECI_CHECK_MPI(rc) do { \
                                  \
} while(0);

enum {
    DALEC_SUCCESS = 0,
    DALEC_INPUT_ERROR = 1,
    DALEC_ERROR_MPI_LIBRARY = 2,
    DALEC_ERROR_MPI_USAGE = 4
} dalec_rc_t;

#define DALEC_ARRAY_MAX_DIM 3

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
} DALEC_Array_handle;

/* Public API */

int   DALEC_Initialize(MPI_Comm comm);
int   DALEC_Finalize(void);

DALEC_NORETURN_PREFIX void  DALEC_Error(const char *msg, int code) DALEC_NORETURN_SUFFIX;

int   DALEC_Create_array(const DALEC_Array_descriptor *, DALEC_Array_handle *);
int   DALEC_Destroy_array(DALEC_Array_handle *);

/* PDALEC -- Profiling Interface */

int   PDALEC_Initialize(MPI_Comm comm);
int   PDALEC_Finalize(void);

DALEC_NORETURN_PREFIX void PDALEC_Error(const char *msg, int code) DALEC_NORETURN_SUFFIX;

int   PDALEC_Create_array(const DALEC_Array_descriptor *, DALEC_Array_handle *);
int   PDALEC_Destroy_array(DALEC_Array_handle *);

#endif /* _DALEC_H_ */
