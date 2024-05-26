
VALGRIND_memcheck_FLAGS = --leak-check=full --track-origins=yes
@VALGRIND_CHECK_RULES@

TESTS = hashset_test
check_PROGRAMS = hashset_test

hashset_test_SOURCES = hashset_test.c $(top_builddir)/src/hashset.h
hashset_test_CFLAGS = @CHECK_CFLAGS@ -I../vector/src
hashset_test_LDADD = $(top_builddir)/src/libhashset.la $(top_builddir)/vector/src/libvector.la @CHECK_LIBS@

