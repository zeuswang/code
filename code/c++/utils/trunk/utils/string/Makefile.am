include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libsstring.la
libsstring_la_SOURCES=sstring.cpp
libsstring_la_LIBADD=

#AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
cpheadfiles: 
	cp sstring.h $(top_srcdir)/utils/

bin_SCRIPTS = cpheadfiles
