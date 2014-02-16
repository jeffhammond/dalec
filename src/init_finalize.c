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
int PDALEC_Init(void) {
  char *var;

  /* GA/TCGMSG end up calling DALEC_Init() multiple times. */
  if (DALECI_GLOBAL_STATE.init_count > 0) {
    DALECI_GLOBAL_STATE.init_count++;
    return 0;
  }

  /* Check for MPI initialization */
  {
    int mpi_is_init, mpi_is_fin;
    MPI_Initialized(&mpi_is_init);
    MPI_Finalized(&mpi_is_fin);
    if (!mpi_is_init || mpi_is_fin) 
      DALECI_Error("MPI must be initialized before calling DALEC_Init");
  }

  /* Set defaults */
#ifdef DALEC_GROUP
  DALECI_GLOBAL_STATE.noncollective_groups = 1;
#endif
#ifdef NO_SEATBELTS
  DALECI_GLOBAL_STATE.iov_checks           = 0;
#endif

  /* Check for debugging flags */

  DALECI_GLOBAL_STATE.debug_alloc          = DALECI_Getenv_bool("DALEC_DEBUG_ALLOC", 0);
  {
	int junk;
	junk = DALECI_Getenv_bool("DALEC_FLUSH_BARRIERS", -1);
    if (junk != -1)
      DALECI_Warning("DALEC_FLUSH_BARRIERS is deprecated.  Use DALEC_SYNC_AT_BARRIERS instead. \n");
  }
  DALECI_GLOBAL_STATE.verbose              = DALECI_Getenv_bool("DALEC_VERBOSE", 0);

  /* Group formation options */

  DALECI_GLOBAL_STATE.cache_rank_translation=DALECI_Getenv_bool("DALEC_CACHE_RANK_TRANSLATION", 1);
  if (DALECI_Getenv("DALEC_NONCOLLECTIVE_GROUPS"))
    DALECI_GLOBAL_STATE.noncollective_groups = DALECI_Getenv_bool("DALEC_NONCOLLECTIVE_GROUPS", 0);

  /* Check for IOV flags */

  DALECI_GLOBAL_STATE.iov_checks           = DALECI_Getenv_bool("DALEC_IOV_CHECKS", 0);
  DALECI_GLOBAL_STATE.iov_batched_limit    = DALECI_Getenv_int("DALEC_IOV_BATCHED_LIMIT", 0);

  if (DALECI_GLOBAL_STATE.iov_batched_limit < 0) {
    DALECI_Warning("Ignoring invalid value for DALEC_IOV_BATCHED_LIMIT (%d)\n", DALECI_GLOBAL_STATE.iov_batched_limit);
    DALECI_GLOBAL_STATE.iov_batched_limit = 0;
  }

  var = DALECI_Getenv("DALEC_IOV_METHOD");

  /* DIRECT leads to addr=NULL errors when DALEC_{GetV,PutV} are used */
  DALECI_GLOBAL_STATE.iov_method = DALECI_IOV_DIRECT;

  if (var != NULL) {
    if (strcmp(var, "AUTO") == 0)
      DALECI_GLOBAL_STATE.iov_method = DALECI_IOV_AUTO;
    else if (strcmp(var, "CONSRV") == 0)
      DALECI_GLOBAL_STATE.iov_method = DALECI_IOV_CONSRV;
    else if (strcmp(var, "BATCHED") == 0)
      DALECI_GLOBAL_STATE.iov_method = DALECI_IOV_BATCHED;
    else if (strcmp(var, "DIRECT") == 0)
      DALECI_GLOBAL_STATE.iov_method = DALECI_IOV_DIRECT;
    else if (DALEC_GROUP_WORLD.rank == 0)
      DALECI_Warning("Ignoring unknown value for DALEC_IOV_METHOD (%s)\n", var);
  }

  /* Check for Strided flags */

  var = DALECI_Getenv("DALEC_STRIDED_METHOD");

  DALECI_GLOBAL_STATE.strided_method = DALECI_STRIDED_DIRECT;

  if (var != NULL) {
    if (strcmp(var, "IOV") == 0)
      DALECI_GLOBAL_STATE.strided_method = DALECI_STRIDED_IOV;
    else if (strcmp(var, "DIRECT") == 0)
      DALECI_GLOBAL_STATE.strided_method = DALECI_STRIDED_DIRECT;
    else if (DALEC_GROUP_WORLD.rank == 0)
      DALECI_Warning("Ignoring unknown value for DALEC_STRIDED_METHOD (%s)\n", var);
  }

  /* Shared buffer handling method */

  var = DALECI_Getenv("DALEC_SHR_BUF_METHOD");

  DALECI_GLOBAL_STATE.shr_buf_method = DALECI_SHR_BUF_COPY;

  if (var != NULL) {
    if (strcmp(var, "COPY") == 0)
      DALECI_GLOBAL_STATE.shr_buf_method = DALECI_SHR_BUF_COPY;
    else if (strcmp(var, "NOGUARD") == 0)
      DALECI_GLOBAL_STATE.shr_buf_method = DALECI_SHR_BUF_NOGUARD;
    else if (DALEC_GROUP_WORLD.rank == 0)
      DALECI_Warning("Ignoring unknown value for DALEC_SHR_BUF_METHOD (%s)\n", var);
  }

  /* Setup groups and communicators */

  MPI_Comm_dup(MPI_COMM_WORLD, &DALEC_GROUP_WORLD.comm);
  DALECI_Group_init_from_comm(&DALEC_GROUP_WORLD);
  DALEC_GROUP_DEFAULT = DALEC_GROUP_WORLD;

  /* Create GOP operators */

  MPI_Op_create(DALECI_Absmin_op, 1 /* commute */, &MPI_ABSMIN_OP);
  MPI_Op_create(DALECI_Absmax_op, 1 /* commute */, &MPI_ABSMAX_OP);

  MPI_Op_create(DALECI_Msg_sel_min_op, 1 /* commute */, &MPI_SELMIN_OP);
  MPI_Op_create(DALECI_Msg_sel_max_op, 1 /* commute */, &MPI_SELMAX_OP);

  DALECI_GLOBAL_STATE.init_count++;

  if (DALECI_GLOBAL_STATE.verbose) {
    if (DALEC_GROUP_WORLD.rank == 0) {
      int major, minor;

      MPI_Get_version(&major, &minor);

      printf("DALEC-MPI initialized with %d process%s, MPI v%d.%d\n", DALEC_GROUP_WORLD.size, DALEC_GROUP_WORLD.size > 1 ? "es":"", major, minor);
#ifdef NO_SEATBELTS
      printf("  NO_SEATBELTS           = ENABLED\n");
#endif
      printf("  STRIDED_METHOD         = %s\n", DALECI_Strided_methods_str[DALECI_GLOBAL_STATE.strided_method]);
      printf("  IOV_METHOD             = %s\n", DALECI_Iov_methods_str[DALECI_GLOBAL_STATE.iov_method]);

      if (   DALECI_GLOBAL_STATE.iov_method == DALECI_IOV_BATCHED
          || DALECI_GLOBAL_STATE.iov_method == DALECI_IOV_AUTO)
      {
        if (DALECI_GLOBAL_STATE.iov_batched_limit > 0)
          printf("  IOV_BATCHED_LIMIT      = %d\n", DALECI_GLOBAL_STATE.iov_batched_limit);
        else
          printf("  IOV_BATCHED_LIMIT      = UNLIMITED\n");
      }

      printf("  IOV_CHECKS             = %s\n", DALECI_GLOBAL_STATE.iov_checks             ? "TRUE" : "FALSE");
      printf("  SHR_BUF_METHOD         = %s\n", DALECI_Shr_buf_methods_str[DALECI_GLOBAL_STATE.shr_buf_method]);
      printf("  NONCOLLECTIVE_GROUPS   = %s\n", DALECI_GLOBAL_STATE.noncollective_groups   ? "TRUE" : "FALSE");
      printf("  CACHE_RANK_TRANSLATION = %s\n", DALECI_GLOBAL_STATE.cache_rank_translation ? "TRUE" : "FALSE");
      printf("  DEBUG_ALLOC            = %s\n", DALECI_GLOBAL_STATE.debug_alloc            ? "TRUE" : "FALSE");
      printf("\n");
      fflush(NULL);
    }

    MPI_Barrier(DALEC_GROUP_WORLD.comm);
  }

  return 0;
}


/* -- begin weak symbols block -- */
#if defined(HAVE_PRAGMA_WEAK)
#  pragma weak DALEC_Init_args = PDALEC_Init_args
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#  pragma _HP_SECONDARY_DEF PDALEC_Init_args DALEC_Init_args
#elif defined(HAVE_PRAGMA_CRI_DUP)
#  pragma _CRI duplicate DALEC_Init_args as PDALEC_Init_args
#endif
/* -- end weak symbols block -- */

/** Initialize DALEC.  MPI must be initialized before this can be called.  It
  * is invalid to make DALEC calls before initialization.  Collective on the
  * world group.
  *
  * @param[inout] argc Command line argument count
  * @param[inout] argv Command line arguments
  * @return            Zero on success
  */
int PDALEC_Init_args(int *argc, char ***argv) {
  return PDALEC_Init();
}


/* -- begin weak symbols block -- */
#if defined(HAVE_PRAGMA_WEAK)
#  pragma weak DALEC_Initialized = PDALEC_Initialized
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#  pragma _HP_SECONDARY_DEF PDALEC_Initialized DALEC_Initialized
#elif defined(HAVE_PRAGMA_CRI_DUP)
#  pragma _CRI duplicate DALEC_Initialized as PDALEC_Initialized
#endif
/* -- end weak symbols block -- */

/** Check if DALEC has been initialized.
  *
  * @return Non-zero if DALEC has been initialized.
  */
int PDALEC_Initialized(void) {
  return DALECI_GLOBAL_STATE.init_count > 0;
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
int PDALEC_Finalize(void) {
  int nfreed;

  /* GA/TCGMSG end up calling DALEC_Finalize() multiple times. */
  if (DALECI_GLOBAL_STATE.init_count == 0) {
    return 0;
  }

  DALECI_GLOBAL_STATE.init_count--;

  /* Only finalize on the last matching call */
  if (DALECI_GLOBAL_STATE.init_count > 0) {
    return 0;
  }

  nfreed = gmr_destroy_all();

  if (nfreed > 0 && DALEC_GROUP_WORLD.rank == 0)
    DALECI_Warning("Freed %d leaked allocations\n", nfreed);

  /* Free GOP operators */

  MPI_Op_free(&MPI_ABSMIN_OP);
  MPI_Op_free(&MPI_ABSMAX_OP);

  MPI_Op_free(&MPI_SELMIN_OP);
  MPI_Op_free(&MPI_SELMAX_OP);

  DALEC_Cleanup();

  DALEC_Group_free(&DALEC_GROUP_WORLD);

  return 0;
}


/** Cleaup DALEC resources.  Call finalize instead.
  */
void DALEC_Cleanup(void) {
  return;
}

