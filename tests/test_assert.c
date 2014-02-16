/*
 * Copyright (C) 2010. See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <mpi.h>
#include <dalec.h>

int main(int argc, char ** argv) {
  MPI_Init(&argc, &argv);
  DALEC_Initialize();
  assert(0);
  DALEC_Finalize();
  MPI_Finalize();
  return 0;
}
