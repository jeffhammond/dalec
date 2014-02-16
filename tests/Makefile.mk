#
# Copyright (C) 2014. See COPYRIGHT in top-level directory.
#

check_PROGRAMS += tests/test_rmw_fadd         \
                  # end

TESTS          += tests/test_rmw_fadd         \
                  # end

XFAIL_TESTS    += tests/test_assert           \
                  # end

tests_test_assert_LDADD = libarmci.la
tests_test_rmw_fadd_LDADD = libarmci.la
