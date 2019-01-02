/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <mpi.h>
#include <dalec.h>

int main(int argc, char ** argv) {

    int rank, nproc;

    MPI_Init(&argc, &argv);
    DALEC_Initialize(MPI_COMM_WORLD);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (rank == 0) printf("Starting DALEC array test with %d processes\n", nproc);



    DALEC_Finalize();
    MPI_Finalize();

    return 0;
}
