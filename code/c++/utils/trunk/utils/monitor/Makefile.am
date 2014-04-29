include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libsmonitor.la
libsmonitor_la_SOURCES=monitor.cpp
libsmonitor_la_LIBADD=-L$(top_srcdir)/_lib/ -lssplatform -lpthread -lscribe_proxy -lsutils
libsmonitor_la_CXXFLAGS=-I/usr/include/thrift/ -I. -I/usr/include/thrift/fb303 -ggdb

AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
cpheadfiles: 
	cp monitor.h $(top_srcdir)/utils/

bin_SCRIPTS = cpheadfiles
