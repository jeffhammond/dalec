#
# Copyright (C) 2014. See COPYRIGHT in top-level directory.
#

check_PROGRAMS += tests/test_hello            \
		  tests/test_assert           \
		  tests/test_array            \
                  # end

TESTS          += tests/test_hello            \
		  tests/test_array	      \
                  # end

XFAIL_TESTS    += tests/test_assert           \
                  # end

tests_test_assert_LDADD = libdalec.la
tests_test_array_LDADD = libdalec.la
tests_test_hello_LDADD = libdalec.la
