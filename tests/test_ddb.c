/*
 * Copyright (C) 2018. See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <dalec.h>

void ddb(ssize_t ndims, ssize_t ardims[], ssize_t npes, ssize_t blk[], ssize_t pedims[]);
void ddb_ex(ssize_t ndims, ssize_t ardims[], ssize_t npes, double threshold, ssize_t blk[], ssize_t pedims[]);
void ddb_h1(ssize_t ndims, ssize_t ardims[], ssize_t npes, double threshold, ssize_t blk[], ssize_t pedims[]);
void ddb_h2(ssize_t ndims, ssize_t ardims[], ssize_t npes, double threshold, ssize_t bias, ssize_t blk[], ssize_t pedims[]);

int main(int argc, char ** argv)
{
    int rank, nproc;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (rank == 0) {

        int np  = (argc>1) ? atoi(argv[1]) :   4;
        int dim = (argc>2) ? atoi(argv[2]) : 100;
        int blk = (argc>3) ? atoi(argv[3]) :  10;

        for (int i=1; i<=DALEC_ARRAY_MAX_DIM; i++) {

            int ndim = i;

            ssize_t dims[DALEC_ARRAY_MAX_DIM] = {0};
            for (int j=0; j<ndim; j++) dims[j] = dim;

            ssize_t blks[DALEC_ARRAY_MAX_DIM] = {0};
            for (int j=0; j<ndim; j++) blks[j] = blk;

            ssize_t outs[DALEC_ARRAY_MAX_DIM] = {0};

            ddb_h2(ndim, dims, np, 0.2, 0 /* bias */, blks, outs);

            printf("ndim = %d\n", ndim);
            for (int j=0; j<ndim; j++) printf("dims[%d] = %zd\n", j, dims[j]);
            for (int j=0; j<ndim; j++) printf("blks[%d] = %zd\n", j, blks[j]);
            for (int j=0; j<ndim; j++) printf("outs[%d] = %zd\n", j, outs[j]);
        }
    }

    MPI_Finalize();

    return 0;
}
