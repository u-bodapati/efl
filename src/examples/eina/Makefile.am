MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib/efl \
-I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/efl/interfaces \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina

if HAVE_WINDOWS
AM_CPPFLAGS += \
-I$(top_srcdir)/src/lib/evil \
-I$(top_builddir)/src/lib/evil
endif

LDADD = $(top_builddir)/src/lib/eina/libeina.la @EINA_LDFLAGS@

SRCS = \
eina_accessor_01.c \
eina_array_01.c \
eina_array_02.c \
eina_error_01.c \
eina_file_01.c \
eina_file_02.c \
eina_hash_01.c \
eina_hash_02.c \
eina_hash_03.c \
eina_hash_04.c \
eina_hash_05.c \
eina_hash_06.c \
eina_hash_07.c \
eina_hash_08.c \
eina_iterator_01.c \
eina_list_01.c \
eina_list_02.c \
eina_list_03.c \
eina_list_04.c \
eina_log_01.c \
eina_log_02.c \
eina_log_03.c \
eina_inlist_01.c \
eina_inlist_02.c \
eina_inlist_03.c \
eina_str_01.c \
eina_strbuf_01.c \
eina_stringshare_01.c \
eina_tmpstr_01.c \
eina_tiler_01.c \
eina_simple_xml_parser_01.c \
eina_value_01.c \
eina_value_02.c \
eina_value_03.c \
eina_inarray_01.c \
eina_inarray_02.c \
eina_inarray_03.c \
eina_magic_01.c \
eina_xattr_01.c \
eina_xattr_02.c

EXTRA_PROGRAMS = \
eina_accessor_01 \
eina_array_01 \
eina_array_02 \
eina_error_01 \
eina_file_01 \
eina_file_02 \
eina_hash_01 \
eina_hash_02 \
eina_hash_03 \
eina_hash_04 \
eina_hash_05 \
eina_hash_06 \
eina_hash_07 \
eina_hash_08 \
eina_iterator_01 \
eina_list_01 \
eina_list_02 \
eina_list_03 \
eina_list_04 \
eina_log_01 \
eina_log_02 \
eina_log_03 \
eina_inlist_01 \
eina_inlist_02 \
eina_inlist_03 \
eina_str_01 \
eina_strbuf_01 \
eina_stringshare_01 \
eina_tmpstr_01 \
eina_magic_01 \
eina_simple_xml_parser_01 \
eina_value_01 \
eina_value_02 \
eina_value_03 \
eina_inarray_01 \
eina_inarray_02 \
eina_inarray_03 \
eina_xattr_01 \
eina_xattr_02

EXTRA_PROGRAMS += eina_tiler_01

eina_tiler_01_CPPFLAGS = \
-I$(top_srcdir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/evas \
-I$(top_srcdir)/src/lib/emile \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/efl/interfaces \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/emile \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_evas \
@EINA_CFLAGS@

eina_tiler_01_LDADD = \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/emile/libemile.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/evas/libevas.la \
@EINA_LDFLAGS@

DATA_FILES = addr_book.txt chat.xml Makefile.examples

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	$(MKDIR_P) $(DESTDIR)$(datadir)/eina/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(DESTDIR)$(datadir)/eina/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/eina/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
