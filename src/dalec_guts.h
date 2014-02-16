/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#ifndef HAVE_DALEC_GUTS_H
#define HAVE_DALEC_GUTS_H

#include <dalec.h>
#include <dalecconf.h>

#if   HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

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
  int           verbose;                /* DALEC should produce extra status output                             */
} global_state_t;


/* Global data */

extern global_state_t DALECI_GLOBAL_STATE;

/* Utility functions */

char *DALECI_Getenv(char *varname);
int   DALECI_Getenv_bool(char *varname, int default_value);
int   DALECI_Getenv_int(char *varname, int default_value);

#endif /* HAVE_DALEC_GUTS_H */
