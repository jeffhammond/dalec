/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

/* If no weak symbols support */
#if !defined(HAVE_PRAGMA_WEAK) && !defined(HAVE_PRAGMA_HP_SEC_DEF) && !defined(HAVE_PRAGMA_CRI_DUP)

#include <dalec.h>

#pragma weak DALEC_Initialize
int DALEC_Initialize(MPI_Comm comm) {
  return PDALEC_Initialize(comm);
}

#pragma weak DALEC_Finalize
int DALEC_Finalize(void) {
  return PDALEC_Finalize();
}

#pragma weak DALEC_Error
void DALEC_Error(const char *msg, int code) {
  PDALEC_Error(msg, code);
  return;
}

#endif
