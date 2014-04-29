include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libpoolbuf_allocator.la
libpoolbuf_allocator_la_SOURCES=poolbuf_allocator.cpp poolbuf_allocator.hpp \
		PoolBufAhoCorasick.h PoolBufAhoCorasick.cpp PoolBufMatch.h PoolBufMatch.cpp
libpoolbuf_allocator_la_LIBADD=
libpoolbuf_allocator_la_CXXFLAGS=

bin_PROGRAMS+=poolbufallocatortest
poolbufallocatortest_SOURCES=poolbuf_allocator.hpp testmain.cpp
poolbufallocatortest_LDADD=libpoolbuf_allocator.la
poolbufallocatortest_CXXFLAGS=

AM_LDFLAGS+=-version-number 3:1:$(SVNVERSION)
#cpheadfiles: 
#	cp tcp_asyn_connect.h $(top_srcdir)/utils/

#bin_SCRIPTS = cpheadfiles
