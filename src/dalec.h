/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#ifndef _DALEC_H_
#define _DALEC_H_

#include <mpi.h>

enum {
    DALEC_SUCCESS = 0,
    DALEC_ERROR_MPI_USAGE = 1,
    DALEC_ERROR_MPI_LIBRARY = 2
} dalec_rc_t;

int   DALEC_Initialize(MPI_Comm comm);
int   DALEC_Finalize(void);
void  DALEC_Error(const char *msg, int code);

/* PDALEC -- Profiling Interface */

int     PDALEC_Initialize(MPI_Comm comm);
int     PDALEC_Finalize(void);
void    PDALEC_Error(const char *msg, int code);

#endif /* _DALEC_H_ */
