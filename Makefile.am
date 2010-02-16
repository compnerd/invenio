# vim: set ft=make ts=8 :

INCLUDES = -I src

bin_PROGRAMS = src/invenio/invenio

src_invenio_invenio_CFLAGS = $(GTK_CFLAGS) $(GDK_CFLAGS)
src_invenio_invenio_LDADD = $(GTK_LIBS)
src_invenio_invenio_SOURCES = src/invenio/invenio.c               \
			      src/invenio/invenio-search-window.c \
			      src/invenio/invenio-search-window.h \
			      src/invenio/invenio-status-icon.c   \
			      src/invenio/invenio-status-icon.h   \
			      $(NULL)

MAINTAINERCLEANFILES = aclocal.m4 configure Makefile.in

