/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#include <dalec_guts.h>
#include <debug.h>

/* Set the default debugging message classes to enable.
 */
unsigned DEBUG_CATS_ENABLED = 
    DEBUG_CAT_NONE;
    // DEBUG_CAT_ALL;
    // DEBUG_CAT_ALLOC;
    // DEBUG_CAT_ALLOC | DEBUG_CAT_MEM_REGION;
    // DEBUG_CAT_MUTEX;
    // DEBUG_CAT_GROUPS;


/** Print an assertion failure message and abort the program.
  */
void DALECI_Assert_fail(const char *expr, const char *msg, const char *file, int line, const char *func) {
  int rank;
  MPI_Comm_rank(DALECI_GLOBAL_STATE.mpi_comm, &rank);

  if (msg == NULL)
    fprintf(stderr, "[%d] DALEC assert fail in %s() [%s:%d]: \"%s\"\n", rank, func, file, line, expr);
  else
    fprintf(stderr, "[%d] DALEC assert fail in %s() [%s:%d]: \"%s\"\n"
                    "[%d] Message: \"%s\"\n", rank, func, file, line, expr, rank, msg);

#if HAVE_EXECINFO_H
  {
#include <execinfo.h>

    const int SIZE = 100;
    int    j, nframes;
    void  *frames[SIZE];
    char **symbols;

    nframes = backtrace(frames, SIZE);
    symbols = backtrace_symbols(frames, nframes);

    if (symbols == NULL)
      perror("Backtrace failure");

    fprintf(stderr, "[%d] Backtrace:\n", rank);
    for (j = 0; j < nframes; j++)
      fprintf(stderr, "[%d]  %2d - %s\n", rank, nframes-j-1, symbols[j]);

    free(symbols);
  }
#endif

  fflush(NULL);
  {
    double stall = MPI_Wtime();
    while (MPI_Wtime() - stall < 1) ;
  }
  MPI_Abort(DALECI_GLOBAL_STATE.mpi_comm, -1);
}


/** Print a debugging message.
  */
void DALECI_Dbg_print_impl(const char *func, const char *format, ...) {
  va_list etc;
  int  disp;
  char string[500];

  int rank;
  MPI_Comm_rank(DALECI_GLOBAL_STATE.mpi_comm, &rank);

  disp  = 0;
  disp += snprintf(string, 500, "[%d] %s: ", rank, func);
  va_start(etc, format);
  disp += vsnprintf(string+disp, 500-disp, format, etc);
  va_end(etc);

  fprintf(stderr, "%s", string);
}


/** Print an DALEC warning message.
  */
void DALECI_Warning(const char *fmt, ...) {
  va_list etc;
  int  disp;
  char string[500];

  int rank;
  MPI_Comm_rank(DALECI_GLOBAL_STATE.mpi_comm, &rank);

  disp  = 0;
  disp += snprintf(string, 500, "[%d] DALEC Warning: ", rank);
  va_start(etc, fmt);
  disp += vsnprintf(string+disp, 500-disp, fmt, etc);
  va_end(etc);

  fprintf(stderr, "%s", string);
  fflush(NULL);
}

/** Raise an internal fatal DALEC error.
  *
  * @param[in] file Current file name (__FILE__)
  * @param[in] line Current line numeber (__LINE__)
  * @param[in] func Current function name (__func__)
  * @param[in] msg  Message to be printed
  * @param[in] code Exit error code
  */
void DALECI_Error_impl(const char *file, const int line, const char *func, const char *msg, ...) {
  va_list ap;
  int  disp;
  char string[500];

  int rank;
  MPI_Comm_rank(DALECI_GLOBAL_STATE.mpi_comm, &rank);

  disp  = 0;
  va_start(ap, msg);
  disp += vsnprintf(string, 500, msg, ap);
  va_end(ap);

  fprintf(stderr, "[%d] DALEC Internal error in %s (%s:%d)\n[%d] Message: %s\n", rank,
      func, file, line, rank, string);
  MPI_Abort(DALECI_GLOBAL_STATE.mpi_comm, 100);
}

