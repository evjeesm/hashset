noinst_LTLIBRARIES = libhashset_funcs.la
lib_LTLIBRARIES = libhashset.la libhashset_static.la

libhashset_funcs_la_SOURCES = hashset.c hash.c hashset.h
libhashset_funcs_la_CFLAGS = -I../vector/src/
libhashset_funcs_la_LDFLAGS = -L../vector/src/.libs/

libhashset_la_SOURCES =
libhashset_la_LDFLAGS = -shared
libhashset_la_CFLAGS =
libhashset_la_LIBADD = libhashset_funcs.la ../vector/src/libvector.la

libhashset_static_la_SOURCES =
libhashset_static_la_LDFLAGS = -static
libhashset_static_la_CFLAGS =
libhashset_static_la_LIBADD = libhashset_funcs.la ../vector/src/libvector_static.la
