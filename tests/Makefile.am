ACLOCAL_AMFLAGS = -I autotools
AM_CXXFLAGS =  -DGTEST_HAS_PTHREAD=0 -I$(top_srcdir)/tests/ -I(top_srcdir)/include/
AM_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_ASIO_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_SIGNALS_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_DATE_TIME_LIB)
#check_PROGRAMS = gtest_all_test
#dnl TESTS_ENVIRONMENT = env GTEST_OUTPUT=xml:results/
#dnl TESTS = gtest_all_test
#gtest_all_test_SOURCES = gtest_all_test.cpp gtest-varstorage_test.cpp gtest-all.cc gtest_main.cc
#noinst_HEADERS = gtest.h
clean-local:
	-rm -rf results
