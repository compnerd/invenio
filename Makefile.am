# vim: set ft=make ts=8 :

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

INCLUDES = -I src

bin_PROGRAMS = src/invenio/invenio
noinst_LTLIBRARIES = src/lash/libash.la

src_lash_libash_la_CFLAGS = $(GTK_CFLAGS) $(X11_CFLAGS)
src_lash_libash_la_LIBADD = $(GTK_LIBS) $(X11_LIBS)
src_lash_libash_la_SOURCES = src/lash/lash.c \
			     src/lash/lash.h \
			     $(NULL)

src_invenio_invenio_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(TRACKER_CFLAGS)
src_invenio_invenio_LDADD = $(GTK_LIBS) $(GLIB_LIBS) $(TRACKER_LIBS)
src_invenio_invenio_SOURCES = src/invenio/invenio.c               \
			      src/invenio/invenio-category.c      \
			      src/invenio/invenio-category.h      \
			      src/invenio/invenio-query.c         \
			      src/invenio/invenio-query.h         \
			      src/invenio/invenio-query-result.c  \
			      src/invenio/invenio-query-result.h  \
			      src/invenio/invenio-search-window.c \
			      src/invenio/invenio-search-window.h \
			      src/invenio/invenio-status-icon.c   \
			      src/invenio/invenio-status-icon.h   \
			      $(NULL)

MAINTAINERCLEANFILES = aclocal.m4 configure Makefile.in

