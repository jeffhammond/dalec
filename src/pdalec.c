/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

/* If no weak symbols support */
#if !defined(HAVE_PRAGMA_WEAK) && !defined(HAVE_PRAGMA_HP_SEC_DEF) && !defined(HAVE_PRAGMA_CRI_DUP) && !defined(HAVE_WEAK_ATTRIBUTE)

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
}

#pragma weak DALEC_Create_array
int DALEC_Create_array(const DALEC_Array_descriptor * d, DALEC_Array_handle * h) {
    return PDALEC_Create_array(d, h);
}

#pragma weak DALEC_Destroy_array
int DALEC_Destroy_array(DALEC_Array_handle * h) {
    return PDALEC_Destroy_array(h);
}

#endif
