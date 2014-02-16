/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include <dalec.h>

/* -- begin weak symbols block -- */
#if defined(HAVE_PRAGMA_WEAK)
#  pragma weak DALEC_Init = PDALEC_Init
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#  pragma _HP_SECONDARY_DEF PDALEC_Init DALEC_Init
#elif defined(HAVE_PRAGMA_CRI_DUP)
#  pragma _CRI duplicate DALEC_Init as PDALEC_Init
#endif
/* -- end weak symbols block -- */

/** Initialize DALEC.  MPI must be initialized before this can be called.  It
  * invalid to make DALEC calls before initialization.  Collective on the world
  * group.
  *
  * @return            Zero on success
  */
int PDALEC_Initialize(void)
{
    /* Check for MPI initialization */
    {
        int mpi_is_init, mpi_is_fin;
        MPI_Initialized(&mpi_is_init);
        MPI_Finalized(&mpi_is_fin);
        if (!mpi_is_init || mpi_is_fin)
            DALECI_Error("MPI must be initialized before calling DALEC_Initialize");
    }

    return 0;
}

/* -- begin weak symbols block -- */
#if defined(HAVE_PRAGMA_WEAK)
#  pragma weak DALEC_Finalize = PDALEC_Finalize
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#  pragma _HP_SECONDARY_DEF PDALEC_Finalize DALEC_Finalize
#elif defined(HAVE_PRAGMA_CRI_DUP)
#  pragma _CRI duplicate DALEC_Finalize as PDALEC_Finalize
#endif
/* -- end weak symbols block -- */

/** Finalize DALEC.  Must be called before MPI is finalized.  DALEC calls are
  * not valid after finalization.  Collective on world group.
  *
  * @return            Zero on success
  */
int PDALEC_Finalize(void)
{

    return 0;
}

