/*
 * Copyright (C) 2014. See COPYRIGHT in top-level directory.
 */

#ifdef NAMESPACE
#undef NAMESPACE
#endif

#if defined(DALEC_PUBLIC_API)
#define NAMESPACE(x) DALEC_##x
#elif defined(DALEC_PROFILE_API)
#define NAMESPACE(x) PDALEC_##x
#endif

int   NAMESPACE(Initialize)(MPI_Comm comm);
int   NAMESPACE(Finalize)(void);

DALEC_NORETURN_PREFIX void  NAMESPACE(Error)(const char *msg, int code) DALEC_NORETURN_SUFFIX;

int   NAMESPACE(Create_array)(const DALEC_Array_descriptor *, DALEC_Array_handle *);
int   NAMESPACE(Destroy_array)(DALEC_Array_handle *);

#undef NAMESPACE
