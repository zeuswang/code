include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libsxml.la
libsxml_la_SOURCES=sxml.h sxml.cpp
libsxml_la_LIBADD=-lxml2

AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
cpheadfiles: 
	cp sxml.h $(top_srcdir)/utils/

bin_SCRIPTS = cpheadfiles
