/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#include <dalec.h>
#include <dalec_guts.h>
#include <debug.h>

dalec_global_state_t DALECI_GLOBAL_STATE = { 0 };

/* -- begin weak symbols block -- */
#if defined(HAVE_PRAGMA_WEAK)
#  pragma weak DALEC_Init = PDALEC_Init
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#  pragma _HP_SECONDARY_DEF PDALEC_Init DALEC_Init
#elif defined(HAVE_PRAGMA_CRI_DUP)
#  pragma _CRI duplicate DALEC_Init as PDALEC_Init
#elif defined(HAVE_WEAK_ATTRIBUTE)
int DALEC_Initialize(MPI_Comm user_comm) __attribute__ ((weak, alias("PDALEC_Init")));
#else
#define DALEC_Initialize PDALEC_Initialize
#endif
/* -- end weak symbols block -- */

/** Initialize DALEC.  MPI must be initialized before this can be called.  It
  * invalid to make DALEC calls before initialization.  Collective on the world
  * group.
  *
  * @return            Zero on success
  */
int DALEC_Initialize(MPI_Comm user_comm)
{
    int dalec_alive = atomic_fetch_sub_explicit(&(DALECI_GLOBAL_STATE.alive),
                                                1,memory_order_seq_cst);
    if (dalec_alive == 0) {
        /* Initialize, since this is the first call to this function. */
        int mpi_is_init, mpi_is_fin;
        MPI_Initialized(&mpi_is_init);
        MPI_Finalized(&mpi_is_fin);
        if (!mpi_is_init || mpi_is_fin) {
            DALECI_Warning("MPI must be active when calling DALEC_Initialize");
            return DALEC_ERROR_MPI_USAGE;
        }

        /* Always dupe the user communicator for internal usage. */
        /* Do not abort on MPI failure, let user handle if MPI does not abort. */
        int rc = MPI_Comm_dup(user_comm, &DALECI_GLOBAL_STATE.mpi_comm);
        return DALECI_Check_MPI("DALEC_Initialize", "MPI_Comm_dup", rc);

        /* Determine what level of threading MPI supports. */
        int mpi_thread_level;
        MPI_Query_thread(&mpi_thread_level);
        DALECI_GLOBAL_STATE.mpi_thread_level;

    } else {
        /* Library has already been initialized. */
        return DALEC_SUCCESS;
    }
}

/* -- begin weak symbols block -- */
#if defined(HAVE_PRAGMA_WEAK)
#  pragma weak DALEC_Finalize = PDALEC_Finalize
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#  pragma _HP_SECONDARY_DEF PDALEC_Finalize DALEC_Finalize
#elif defined(HAVE_PRAGMA_CRI_DUP)
#  pragma _CRI duplicate DALEC_Finalize as PDALEC_Finalize
#elif defined(HAVE_WEAK_ATTRIBUTE)
int DALEC_Finalize(void) __attribute__ ((weak, alias("PDALEC_Finalize")));
#else
#define DALEC_Finalize PDALEC_Finalize
#endif
/* -- end weak symbols block -- */

/** Finalize DALEC.  Must be called before MPI is finalized.  DALEC calls are
  * not valid after finalization.  Collective on world group.
  *
  * @return            Zero on success
  */
int DALEC_Finalize(void)
{
    int dalec_alive = atomic_fetch_sub_explicit(&(DALECI_GLOBAL_STATE.alive),
                                                1,memory_order_seq_cst);

    if (dalec_alive == 1) {
        /* Check for MPI initialization */
        int mpi_is_init, mpi_is_fin;
        MPI_Initialized(&mpi_is_init);
        MPI_Finalized(&mpi_is_fin);

        /* Free communicator if possible and return */
        if (!mpi_is_init || mpi_is_fin) {
            DALECI_Warning("MPI must be active when calling DALEC_Finalize");
            return DALEC_ERROR_MPI_USAGE;
        } else {
            int rc = MPI_Comm_free(&DALECI_GLOBAL_STATE.mpi_comm);
            return DALECI_Check_MPI("DALEC_Finalize", "MPI_Comm_free", rc);
        }
    } else {
        /* Library is still active. */
        return DALEC_SUCCESS;
    }
}

