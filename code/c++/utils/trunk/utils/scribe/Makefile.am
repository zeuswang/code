include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libscribe_proxy.la
libscribe_proxy_la_SOURCES=scribe_proxy.cpp scribe_handler.cpp gen-cpp/scribe_types.cpp  gen-cpp/scribe.cpp
libscribe_proxy_la_LIBADD=-lpthread -lssplatform -lthrift -lfb303 -lthriftnb -levent
libscribe_proxy_la_CXXFLAGS=-I/usr/include/thrift/ -I. -I/usr/include/thrift/fb303
libscribe_proxy_la_CPPFLAGS=$(AM_CPPFLAGS)  -DSS_LOG_DEFAULT_MODULE=ods_utils
AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
#cpheadfiles: 
#	cp tcp_asyn_connect.h $(top_srcdir)/utils/

#bin_SCRIPTS = cpheadfiles
