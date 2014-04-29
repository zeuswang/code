include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libsutils.la
libsutils_la_SOURCES=utils.cpp memcached_utils.cpp doc_filter.cpp doc_filter.h
libsutils_la_LIBADD=-L$(top_srcdir)/_lib/ -lssplatform -lpthread -lmemcached -lencoding
libsutils_la_CXXFLAGS=-I/usr/include/thrift/ -I. -I/usr/include/thrift/fb303 -DSS_LOG_DEFAULT_MODULE=ods_utils_util

AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
cpheadfiles: 
	cp *.h $(top_srcdir)/utils/

bin_SCRIPTS = cpheadfiles
