/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#ifndef _DALEC_H_
#define _DALEC_H_

#include <mpi.h>

int   DALEC_Initialize(void);
int   DALEC_Finalize(void);

/* PDALEC -- Profiling Interface */

int     PDALEC_Initialize(void);
int     PDALEC_Finalize(void);

#endif /* _DALEC_H_ */
