#
# Copyright (C) 2014. See COPYRIGHT in top-level directory.
#

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_srcdir)/src

lib_LTLIBRARIES = libdalec.la

# Needed to connect with the GA build system
noinst_LTLIBRARIES = libdaleci.la

libdalec_la_SOURCES = src/init_finalize.c 

libdalec_la_LDFLAGS = -version-info $(libdalec_abi_version)

libdaleci_la_SOURCES = $(libdalec_la_SOURCES)
libdaleci_la_LDFLAGS = $(libdalec_abi_version)

include_HEADERS = src/dalec.h

bin_PROGRAMS =
check_PROGRAMS = 
TESTS = 
XFAIL_TESTS = 

MPIEXEC = mpiexec -n 2
TESTS_ENVIRONMENT = $(MPIEXEC)

include benchmarks/Makefile.mk
include tests/Makefile.mk

.PHONY: checkprogs
checkprogs: $(check_PROGRAMS)
