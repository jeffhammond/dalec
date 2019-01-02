/*
 * Copyright (C) 2010. See COPYRIGHT in top-level directory.
 */

#include <dalec.h>
#include <dalec_guts.h>
#include <debug.h>

/** Fatal error, print the message and abort the program with the provided
  * error code.
  */
void DALEC_Error(const char *msg, int code)
{
    int rank;
    MPI_Comm_rank(DALECI_GLOBAL_STATE.mpi_comm, &rank);

    fprintf(stderr, "[%d] DALEC Error: %s\n", rank, msg);
    fflush(NULL);

    /* MPI_Abort does not have noreturn declaration but C abort does,
     * so even though abort() is unreachable, it ensures tools understand
     * the behavior of this function. */
    MPI_Abort(DALECI_GLOBAL_STATE.mpi_comm, code);
    abort();
}

/** MPI error message
  * error code.
  */
int DALECI_Check_MPI(const char * dfn, const char * mpifn, int mpirc)
{
    if (mpirc==MPI_SUCCESS) {
        return DALEC_SUCCESS;
    } else {
        int rank;
        MPI_Comm_rank(DALECI_GLOBAL_STATE.mpi_comm, &rank);

        int len;
        char errmsg[MPI_MAX_ERROR_STRING];
        MPI_Error_string(mpirc, errmsg, &len);

        DALECI_Warning("%d: %s -> %s:\n %s \n", rank, dfn, mpifn, errmsg);

        return DALEC_ERROR_MPI_LIBRARY;
    }
}

/** Retrieve the value of a boolean environment variable.  */
int DALECI_Getenv_bool(const char *varname, int default_value)
{
  char *var = getenv(varname);

  if (var == NULL) {
    return default_value;
  } else if (var[0] == 'T' || var[0] == 't' || var[0] == '1' || var[0] == 'y' || var[0] == 'Y') {
    return 1;
  } else {
    return 0;
  }
}


/** Retrieve the value of a environment variable.
  */
 char *DALECI_Getenv(const char *varname)
{
  return getenv(varname);
}


/** Retrieve the value of an integer environment variable.
  */
int DALECI_Getenv_int(const char *varname, int default_value)
{
  char *var = getenv(varname);
  if (var) {
    return atoi(var);
  } else {
    return default_value;
  }
}
