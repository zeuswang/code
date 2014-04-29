include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libsconfig.la
libsconfig_la_SOURCES=sconfig.cpp sconfig.h
libsconfig_la_LIBADD=-lACE -lssplatform
libsconfig_la_CPPFLAGS=$(AM_CPPFLAGS)  -DSS_LOG_DEFAULT_MODULE=ods_utils

AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
cpheadfiles: 
	cp sconfig.h $(top_srcdir)/utils/

bin_SCRIPTS = cpheadfiles
