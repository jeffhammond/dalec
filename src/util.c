/*
 * Copyright (C) 2010. See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include <dalec.h>
#include <dalec_internals.h>
#include <debug.h>
#include <gmr.h>


/** Fatal error, print the message and abort the program with the provided
  * error code.
  */
void DALEC_Error(char *msg, int code) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    fprintf(stderr, "[%d] DALEC Error: %s\n", rank, msg);
    fflush(NULL);

    MPI_Abort(DALEC_GROUP_WORLD.comm, code);

    return;
}

/** Retrieve the value of a boolean environment variable.  */
int DALECI_Getenv_bool(char *varname, int default_value) {
  char *var = getenv(varname);

  if (var == NULL)
    return default_value;

  if (var[0] == 'T' || var[0] == 't' || var[0] == '1' || var[0] == 'y' || var[0] == 'Y')
    return 1;

  else
    return 0;
}


/** Retrieve the value of a environment variable.
  */
char *DALECI_Getenv(char *varname) {
  return getenv(varname);
}


/** Retrieve the value of an integer environment variable.
  */
int DALECI_Getenv_int(char *varname, int default_value) {
  char *var = getenv(varname);
  if (var)
    return atoi(var);
  else
    return default_value;
}
