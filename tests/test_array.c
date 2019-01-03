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

    {
        DALEC_Array_descriptor d = { .comm = MPI_COMM_WORLD,
                                     .type = MPI_DOUBLE,
                                     .ndim = 1,
                                     .dims = {1000},
                                     .blks = {0},
                                     .name = "test array 1" };
        DALEC_Array_handle h;
        DALEC_Create_array(&d, &h);
        DALEC_Destroy_array(&h);
    }

    MPI_Barrier(MPI_COMM_WORLD); fflush(stdout); fflush(stderr); MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) printf("==========================\n");
    {
        DALEC_Array_descriptor d = { .comm = MPI_COMM_WORLD,
                                     .type = MPI_DOUBLE,
                                     .ndim = 1,
                                     .dims = {1000},
                                     .blks = {70},
                                     .name = "test array 2" };
        DALEC_Array_handle h;
        DALEC_Create_array(&d, &h);
        DALEC_Destroy_array(&h);
    }

    MPI_Barrier(MPI_COMM_WORLD); fflush(stdout); fflush(stderr); MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) printf("==========================\n");
    {
        DALEC_Array_descriptor d = { .comm = MPI_COMM_WORLD,
                                     .type = MPI_DOUBLE,
                                     .ndim = 1,
                                     .dims = {1000},
                                     .blks = {25},
                                     .name = "test array 3" };
        DALEC_Array_handle h;
        DALEC_Create_array(&d, &h);
        DALEC_Destroy_array(&h);
    }

    DALEC_Finalize();
    MPI_Finalize();

    return 0;
}
