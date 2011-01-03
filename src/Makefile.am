SUBDIRS = 
bin_PROGRAMS = tinjac

tinjac_SOURCES = main.cpp crontabs.cpp log.cpp
tinjac_LDADD = $(BOOST_LDFLAGS) $(BOOST_ASIO_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_SIGNALS_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_DATE_TIME_LIB)
tinjac_CXXFLAGS = -I$(top_srcdir)/include/ -I/usr/include/squirrel/

EXTRA_DIST = 
noinst_HEADERS = 
