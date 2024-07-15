
VALGRIND_memcheck_FLAGS = --leak-check=full --track-origins=yes
@VALGRIND_CHECK_RULES@

TESTS = hashset_test
check_PROGRAMS = hashset_test

hashset_test_SOURCES = hashset_test.c $(top_srcdir)/src/hashset.h
hashset_test_CFLAGS = @CHECK_CFLAGS@ -I$(top_srcdir)/vector/src
hashset_test_LDADD = $(top_builddir)/src/libhashset.la $(top_builddir)/vector/src/libvector.la @CHECK_LIBS@

debug-hashset-test: ../src/libhashset.la hashset_test
	LD_LIBRARY_PATH=../src/.libs:../vector/src/.libs:/usr/local/lib CK_FORK=no gdb -tui .libs/hashset_test
