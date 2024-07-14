noinst_LTLIBRARIES = libhashset_funcs.la
lib_LTLIBRARIES = libhashset.la libhashset_static.la

libhashset_funcs_la_SOURCES = hashset.c hash.c hashset.h
libhashset_funcs_la_CFLAGS = -I$(top_srcdir)/vector/src/
libhashset_funcs_la_LDFLAGS = -L$(top_builddir)/vector/src/

libhashset_la_SOURCES =
libhashset_la_LDFLAGS = -shared
libhashset_la_CFLAGS =
libhashset_la_LIBADD = libhashset_funcs.la $(top_builddir)/vector/src/libvector.la

libhashset_static_la_SOURCES =
libhashset_static_la_LDFLAGS = -static
libhashset_static_la_CFLAGS =
libhashset_static_la_LIBADD = libhashset_funcs.la $(top_builddir)/vector/src/libvector_static.la

include_HEADERS = hashset.h hash.h bitset.h
