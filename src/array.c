/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#include <dalec.h>
#include <dalec_guts.h>
#include <debug.h>

#if 0
typedef {
    MPI_Comm comm;
    MPI_Datatype type;
    int ndim;
    size_t dims[];
    size_t blocksizes[];
    char * name;
} DALEC_Array_descriptor;

typedef MPI_Win DALEC_Array_handle;

int   DALEC_Create_array(const DALEC_Array_descriptor *, DALEC_Array_handle *);
int   DALEC_Destroy_array(DALEC_Array_handle *);
#endif

/* -- begin weak symbols block -- */
#if defined(HAVE_PRAGMA_WEAK)
#  pragma weak DALEC_Create_array = PDALEC_Create_array
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#  pragma _HP_SECONDARY_DEF PDALEC_Create_array DALEC_Create_array
#elif defined(HAVE_PRAGMA_CRI_DUP)
#  pragma _CRI duplicate DALEC_Create_array as PDALEC_Create_array
#endif
/* -- end weak symbols block -- */

/** 
  *
  * @return            Zero on success
  */
int PDALEC_Create_array(const DALEC_Array_descriptor * d, DALEC_Array_handle * h);
{
    {
        /* determine if the datatype is a predefined one */
        int num_integers, num_addresses, num_datatypes, combiner;
        int rc = MPI_Type_get_envelope(d->type, &num_integers, &num_addresses, &num_datatypes, &combiner);
        DALECI_CHECK_MPI(rc);
        if (combiner != MPI_COMBINER_NAMED) {
            DALEC_Error("input datatype is not a predefined MPI datatype!", DALEC_INPUT_ERROR);
            return DALEC_INPUT_ERROR;
        }
    }

    {
        int type_size;
        {
            int rc = MPI_Type_size(d->type, &type_size);
            DALECI_CHECK_MPI(rc);
        }

        MPI_Aint win_size = 0;
        void * baseptr = NULL;
        {
            int rc = MPI_Win_allocate(win_size, type_size, MPI_INFO_NULL, d->comm, &baseptr, &(h->win));
            DALECI_CHECK_MPI(rc);
        }
    }
    return DALEC_SUCCESS;
}

/* -- begin weak symbols block -- */
#if defined(HAVE_PRAGMA_WEAK)
#  pragma weak DALEC_Destroy_array = PDALEC_Destroy_array
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#  pragma _HP_SECONDARY_DEF PDALEC_Destroy_array DALEC_Destroy_array
#elif defined(HAVE_PRAGMA_CRI_DUP)
#  pragma _CRI duplicate DALEC_Destroy_array as PDALEC_Destroy_array
#endif
/* -- end weak symbols block -- */

/** 
  *
  * @return            Zero on success
  */
int PDALEC_Destroy_array(DALEC_Array_handle * h);
{
    return DALEC_SUCCESS;
}

