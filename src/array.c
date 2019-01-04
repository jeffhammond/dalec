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
#else
#define DALEC_Create_array PDALEC_Create_array
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

    /* check for really stupid inputs */
    if (d==NULL || h==NULL) {
        DALECI_Error("d (%p) or h (%p) is a null pointer", d, h);
        return DALEC_INPUT_ERROR;
    }

    const int ndim = d->ndim;
    const MPI_Comm comm = d->comm;

    /* print arguments (debug mode) */
    if (DEBUG_CAT_ENABLED(DEBUG_CAT_ARGS)) {
#if defined(OPEN_MPI)
#define HANDLE_FORMAT "%p"
#elif defined(MPICH)
#define HANDLE_FORMAT "%d"
#else
#error Unknown MPI handle format (i.e. not MPICH or Open-MPI)
#endif
        DALECI_Dbg_print(DEBUG_CAT_ARGS, "MPI_Comm     = " HANDLE_FORMAT "\n", comm);
        DALECI_Dbg_print(DEBUG_CAT_ARGS, "MPI_Datatype = " HANDLE_FORMAT "\n", d->type);

        DALECI_Dbg_print(DEBUG_CAT_ARGS, "ndim = %d\n", ndim);

        const size_t * ptrs[2] = {d->dims, d->blks};
        const char   * name[2] = {"dims", "blks"};
        for (int j=0; j<2; j++) {
            int total = 0;
            char buf[DALEC_ARRAY_MAX_DIM * 16] = {0};
            total += snprintf(&buf[total], sizeof(buf), "{");
            for (int i=0; i<ndim; i++) {
                total += snprintf(&buf[total], sizeof(buf), "%zu", ptrs[j][i]);
                if (i < (ndim-1)) {
                    total += snprintf(&buf[total], sizeof(buf), ",");
                }
            }
            total += snprintf(&buf[total], sizeof(buf), "}");
            DALECI_Dbg_print(DEBUG_CAT_ARGS, "%s = %s\n", name[j], buf);
        }
        DALECI_Dbg_print(DEBUG_CAT_ARGS, "name = %s\n", d->name);
#undef HANDLE_FORMAT
    }

    /* check argument validity */
    {
        if (ndim < 1) {
            DALECI_Error("ndim (%d) less than 1 is nonsensical", ndim);
            return DALEC_INPUT_ERROR;
        } else if (ndim > DALEC_ARRAY_MAX_DIM) {
            DALECI_Error("ndim (%d) exceeds DALEC_ARRAY_MAX_DIM (%d)", ndim, DALEC_ARRAY_MAX_DIM);
            return DALEC_INPUT_ERROR;
        }

        for (int i=0; i<ndim; i++) {
            const size_t dim = d->dims[i];
            const size_t blk = d->blks[i];
            if (dim < 1) {
                DALECI_Error("dims[%d] = %zu < 1", i, dim);
                return DALEC_INPUT_ERROR;
            }
            if (blk > dim) {
                DALECI_Error("blks[%d] (%zu) > dims[%d] (%zu)", i, blk, i, dim);
                return DALEC_INPUT_ERROR;
            }
        }

        /* check to make sure all calling processes gave the same arguments */

#define DALEC_ARGS_COUNT 2+4*DALEC_ARRAY_MAX_DIM

        int64_t args[DALEC_ARGS_COUNT] = {0};
        args[0] =  ndim;
        args[1] = -ndim;
        for (int i=0; i<ndim; i++) {
            const size_t dim = d->dims[i];
            const size_t blk = d->blks[i];
            args[2+4*i+0] =  dim;
            args[2+4*i+1] = -dim;
            args[2+4*i+2] =  blk;
            args[2+4*i+3] = -blk;
        }

        int rc = MPI_Allreduce(MPI_IN_PLACE, args, DALEC_ARGS_COUNT, MPI_INT64_T, MPI_MAX, comm);
        DALECI_Check_MPI(FCNAME, "MPI_Reduce", rc);

#undef DALEC_ARGS_COUNT

        if (args[0] != -args[1]) {
            DALECI_Error("ndim (%d) is not constant across ranks", ndim);
            return DALEC_INPUT_ERROR;
        }
        for (int i=0; i<ndim; i++) {
            if (args[2+4*i+0] != -args[2+4*i+1]) {
                DALECI_Error("dims[%d] (%zu) is not constant across ranks", i, d->dims[i]);
                return DALEC_INPUT_ERROR;
            }
            if (args[2+4*i+2] != -args[2+4*i+3]) {
                DALECI_Error("blks[%d] (%zu) is not constant across ranks", i, d->blks[i]);
                return DALEC_INPUT_ERROR;
            }
        }
    }

    /* determine if the datatype is a predefined one */
    {
        int num_integers, num_addresses, num_datatypes, combiner;
        rc = MPI_Type_get_envelope(d->type, &num_integers, &num_addresses, &num_datatypes, &combiner);
        DALECI_Check_MPI(FCNAME, "MPI_Type_get_envelope", rc);
        if (combiner != MPI_COMBINER_NAMED) {
            DALECI_Error("input datatype is not a predefined MPI datatype!");
            return DALEC_INPUT_ERROR;
        }
    }

    /* allocate the window for this array */
    {
        MPI_Aint win_size = 0;

        int type_size = 0;
        rc = MPI_Type_size(d->type, &type_size);
        DALECI_Check_MPI(FCNAME, "MPI_Type_size", rc);

        int np = 1, me = 0;
        MPI_Comm_size(comm, &np);
        MPI_Comm_rank(comm, &me);

        /* start with specialization and generalize later... */
        DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "ndim = %d\n", ndim);
        if (ndim==1) {

            const size_t dim = d->dims[0];
            const size_t blk = d->blks[0];

            DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "dims[1] = {%d}\n", dim);
            DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "blks[1] = {%d}\n", blk);

            /* blk = 0 means we get to decide */
            if (blk == 0) {
                win_size = (dim/np);
                if (dim%np > me) win_size++;
                DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "win_size = %zu\n", win_size);
            } else {
                const size_t num_full_blocks    = (dim/blk);
                const size_t partial_block_size = (dim%blk);

                DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "num_full_blocks    = %zu\n", num_full_blocks);
                DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "partial_block_size = %zu\n", partial_block_size);

                const size_t num_all_blocks  = num_full_blocks + (partial_block_size>1);
                const size_t full_blk_per_pe = (num_full_blocks/np) + (num_full_blocks%np > 0);
                const size_t all_blk_per_pe = (num_all_blocks/np) + (num_all_blocks%np > 0);

                DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "num_all_blocks  = %zu\n", num_all_blocks);
                DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "full_blk_per_pe = %zu\n", full_blk_per_pe);
                DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "all_blk_per_pe  = %zu\n", all_blk_per_pe);

#if 0
                win_size = blk_per_pe * blk;
                DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "win_size   = %zu\n", win_size);
                if (me < num_blocks%np - 1) win_size += blk;
                DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "win_size   = %zu\n", win_size);
                if (me == num_blocks%np) win_size += (dim - blk*(num_blocks/np + num_blocks%np - 1));
                DALECI_Dbg_print(DEBUG_CAT_ARRAY_DIST, "win_size   = %zu\n", win_size);
#endif
            }
        }

        void * baseptr = NULL;
        rc = MPI_Win_allocate(win_size, type_size, MPI_INFO_NULL, comm, &baseptr, &(h->win));
        DALECI_Check_MPI(FCNAME, "MPI_Win_allocate", rc);
    }

    /* if array is named, assign to window */
    {
        if (d->name != NULL) {
            rc = MPI_Win_set_name(h->win, d->name);
            DALECI_Check_MPI(FCNAME, "MPI_Win_set_name", rc);
        }
    }

#if 0
    /* capture array properties in window attributes */
    {
        rc = MPI_Win_create_keyval(MPI_WIN_NULL_COPY_FN, MPI_WIN_NULL_DELETE_FN, &(h->win_keyval), NULL);
        DALECI_Check_MPI(FCNAME, "MPI_Win_create_keyval", rc);


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
#elif defined(HAVE_WEAK_ATTRIBUTE)
int DALEC_Destroy_array(DALEC_Array_handle * h) __attribute__ ((weak, alias("PDALEC_Destroy_array")));
#else
#define DALEC_Destroy_array PDALEC_Destroy_array
#endif
/* -- end weak symbols block -- */

#undef FUNCNAME
#define FUNCNAME DALEC_Destroy_array
#undef FNAME
#define FCNAME DALECI_QUOTE_STRING(FUNCNAME)

/** 
  *
  * @return            Zero on success
  */
int DALEC_Destroy_array(DALEC_Array_handle * h)
{
    int rc; /* MPI return code */

    rc = MPI_Win_free(&(h->win));
    DALECI_Check_MPI(FCNAME, "MPI_Win_free", rc);

    return DALEC_SUCCESS;
}

