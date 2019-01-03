/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#ifndef HAVE_DALEC_GUTS_H
#define HAVE_DALEC_GUTS_H

#include "dalecconf.h"
#include "dalec.h"
#include "debug.h"

#if   HAVE_STDIO_H
#include <stdio.h>
#endif

#if   HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if   HAVE_STRING_H
#include <string.h>
#endif

#if   HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#if HAVE_STDATOMIC_H
#  include <stdatomic.h>
#else
#error C11 atomics are required for now.
#endif

#if HAVE_STDBOOL_H
#  include <stdbool.h>
#else
#error C11 boolean is required for now.
#endif

#define DALECI_QUOTE_STRING(A) #A

/* Likely/Unlikely macros borrowed from MPICH: */

/* These likely/unlikely macros provide static branch prediction hints to the
 * compiler, if such hints are available.  Simply wrap the relevant expression in
 * the macro, like this:
 *
 * if (unlikely(ptr == NULL)) {
 *     // ... some unlikely code path ...
 * }
 *
 * They should be used sparingly, especially in upper-level code.  It's easy to
 * incorrectly estimate branching likelihood, while the compiler can often do a
 * decent job if left to its own devices.
 *
 * These macros are not namespaced because the namespacing is cumbersome.
 */
/* safety guard for now, add a configure check in the future */
#if ( defined(__GNUC__) && (__GNUC__ >= 3) ) || defined(__IBMC__) || defined(__INTEL_COMPILER) || defined(__clang__)
#  define unlikely(x_) __builtin_expect(!!(x_),0)
#  define likely(x_)   __builtin_expect(!!(x_),1)
#else
#  define unlikely(x_) (x_)
#  define likely(x_)   (x_)
#endif

/* Internal types */

enum DALECI_Op_e { DALECI_OP_PUT, DALECI_OP_GET, DALECI_OP_ACC };

typedef struct {
    atomic_int    alive;                /* DALEC has been initialized but not finalized */
    int           verbose;              /* DALEC should produce extra status output     */
    MPI_Comm      mpi_comm;             /* MPI communicator from user (duped)           */
    int           mpi_thread_level;     /* MPI thread level                             */
} dalec_global_state_t;

/* Global data */

extern dalec_global_state_t DALECI_GLOBAL_STATE;

/* Utility functions */

int    DALECI_Check_MPI(const char * dalecfn, const char * mpifn, int mpirc);
char * DALECI_Getenv(const char *varname);
int    DALECI_Getenv_bool(const char *varname, int default_value);
int    DALECI_Getenv_int(const char *varname, int default_value);

#endif /* HAVE_DALEC_GUTS_H */
