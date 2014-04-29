include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libstimer.la
libstimer_la_SOURCES=stimer.cpp
libstimer_la_LIBADD=-levent -lpthread 

#AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
cpheadfiles: 
	cp stimer.h $(top_srcdir)/utils/

bin_SCRIPTS = cpheadfiles

#bin_PROGRAMS+=test
#test_SOURCES=test.cpp
#test_LDADD=-lstimer
