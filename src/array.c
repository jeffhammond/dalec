/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#include <dalec.h>
#include <dalec_guts.h>
#include <debug.h>

/* -- Begin Profiling Symbol Block for routine DALEC_Create_array */
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak DALEC_Create_array = PDALEC_Create_array
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PDALEC_Create_array  DALEC_Create_array
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate DALEC_Create_array as PDALEC_Create_array
#elif defined(HAVE_WEAK_ATTRIBUTE)
int DALEC_Create_array(const DALEC_Array_descriptor * d, DALEC_Array_handle * h) __attribute__ ((weak, alias("PDALEC_Create_array")));
#endif
/* -- End Profiling Symbol Block */

#undef FUNCNAME
#define FUNCNAME DALEC_Create_array
#undef FNAME
#define FCNAME DALECI_QUOTE_STRING(FUNCNAME)

/** 
  *
  * @return            Zero on success
  */
int DALEC_Create_array(const DALEC_Array_descriptor * d, DALEC_Array_handle * h)
{
    int rc; /* MPI return code */

#if 0
    typedef {
        MPI_Comm comm;
        MPI_Datatype type;
        int ndim;
        size_t dims[DALEC_ARRAY_MAX_DIM];
        size_t blocksizes[DALEC_ARRAY_MAX_DIM];
        char * name;
    } DALEC_Array_descriptor;
#endif

    /* determine if the datatype is a predefined one */
    {
        int num_integers, num_addresses, num_datatypes, combiner;
        rc = MPI_Type_get_envelope(d->type, &num_integers, &num_addresses, &num_datatypes, &combiner);
        DALECI_Check_MPI("DALEC_Create_array", "MPI_Type_get_envelope", rc);
        if (combiner != MPI_COMBINER_NAMED) {
            DALECI_Error("input datatype is not a predefined MPI datatype!");
            return DALEC_INPUT_ERROR;
        }
    }

    /* allocate the window for this array */
    {
        int type_size;
        rc = MPI_Type_size(d->type, &type_size);
        DALECI_Check_MPI("DALEC_Create_array", "MPI_Type_size", rc);

        MPI_Aint win_size = 0;
        void * baseptr = NULL;
        rc = MPI_Win_allocate(win_size, type_size, MPI_INFO_NULL, d->comm, &baseptr, &(h->win));
        DALECI_Check_MPI("DALEC_Create_array", "MPI_Win_allocate", rc);
    }

    /* if array is named, assign to window */
    {
        if (d->name != NULL) {
            rc = MPI_Win_set_name(h->win, d->name);
            DALECI_Check_MPI("DALEC_Create_array", "MPI_Win_set_name", rc);
        }
    }

#if 0
    /* capture array properties in window attributes */
    {
        rc = MPI_Win_create_keyval(MPI_WIN_NULL_COPY_FN, MPI_WIN_NULL_DELETE_FN, &(h->win_keyval), NULL);
        DALECI_Check_MPI("DALEC_Create_array", "MPI_Win_create_keyval", rc);


    }
#endif

    return DALEC_SUCCESS;
}

/* -- begin weak symbols block -- */
#if defined(HAVE_PRAGMA_WEAK)
#  pragma weak DALEC_Destroy_array = DALEC_Destroy_array
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#  pragma _HP_SECONDARY_DEF DALEC_Destroy_array DALEC_Destroy_array
#elif defined(HAVE_PRAGMA_CRI_DUP)
#  pragma _CRI duplicate DALEC_Destroy_array as DALEC_Destroy_array
#endif
/* -- end weak symbols block -- */

/** 
  *
  * @return            Zero on success
  */
int DALEC_Destroy_array(DALEC_Array_handle * h)
{
    return DALEC_SUCCESS;
}

