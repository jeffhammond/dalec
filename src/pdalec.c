/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

/* If no weak symbols support */
#if !defined(HAVE_PRAGMA_WEAK) && !defined(HAVE_PRAGMA_HP_SEC_DEF) && !defined(HAVE_PRAGMA_CRI_DUP)

#include "dalec.h"

#pragma weak DALEC_Initialize
int DALEC_Initialize(void) {
  return PDALEC_Initialize();
}

#pragma weak DALEC_Finalize
int DALEC_Finalize(void) {
  return PDALEC_Finalize();
}

#endif
