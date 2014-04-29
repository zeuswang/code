include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libqdb_utils.la
libqdb_utils_la_SOURCES=qdb_daemon_client.cpp mpage.cpp
libqdb_utils_la_LIBADD=-L$(top_srcdir)/_lib/ -lACE -loffdb -lsnet -lpthread -lstimer -levent -lsutils -lsstring
libqdb_utils_la_CPPFLAGS=$(AM_CPPFLAGS) -DSS_DOCID_BITS=256 -ggdb
AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
cpheadfiles: 
	cp *.h $(top_srcdir)/utils/

bin_SCRIPTS = cpheadfiles
