include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libddbclient.la
libddbclient_la_SOURCES=topology.cpp sync_client.cpp sconnection.cpp \
		   http_protocol_adapter.cpp connection_server.cpp connection_client.cpp
libddbclient_la_LIBADD=-L$(top_srcdir)/_lib/ -lsxml -lstimer -lsutils -lssplatform -lpthread -lsstring -loffdb -levent
libddbclient_la_CPPFLAGS=$(AM_CPPFLAGS) -DSS_DOCID_BITS=256 -ggdb
AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
cpheadfiles: 
	cp *.h $(top_srcdir)/utils/

bin_SCRIPTS = cpheadfiles
