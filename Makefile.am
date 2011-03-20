# vim: set ft=make sw=4 sts=8 ts=8 :

INCLUDES = -I src

desktopdir = $(datadir)/applications
desktop_in_files = data/invenio.desktop.in

bin_PROGRAMS = src/invenio/invenio
noinst_PROGRAMS = src/invenio-preferences/invenio-preferences
noinst_LTLIBRARIES = src/lash/libash.la src/libinvenio/libinvenio.la
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

src_lash_libash_la_CFLAGS = $(GTK_CFLAGS) $(X11_CFLAGS)
src_lash_libash_la_LIBADD = $(GTK_LIBS) $(X11_LIBS)
src_lash_libash_la_SOURCES = src/lash/lash.c \
			     src/lash/lash.h \
			     $(NULL)

src_libinvenio_libinvenio_la_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS)
src_libinvenio_libinvenio_la_LIBADD = $(GTK_LIBS) $(GLIB_LIBS)
src_libinvenio_libinvenio_la_SOURCES = src/libinvenio/invenio-category.c        \
				       src/libinvenio/invenio-category.h        \
				       src/libinvenio/invenio-configuration.c   \
				       src/libinvenio/invenio-configuration.h   \
				       $(NULL)

src_invenio_invenio_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(TRACKER_CFLAGS) $(LIBWNCK_CFLAGS) -DWNCK_I_KNOW_THIS_IS_UNSTABLE
src_invenio_invenio_LDADD = $(GTK_LIBS) $(GLIB_LIBS) $(TRACKER_LIBS) $(LIBWNCK_LIBS) src/lash/libash.la src/libinvenio/libinvenio.la
src_invenio_invenio_SOURCES = src/invenio/invenio.c               \
			      src/invenio/invenio-query.c         \
			      src/invenio/invenio-query.h         \
			      src/invenio/invenio-query-result.c  \
			      src/invenio/invenio-query-result.h  \
			      src/invenio/invenio-search-window.c \
			      src/invenio/invenio-search-window.h \
			      src/invenio/invenio-status-icon.c   \
			      src/invenio/invenio-status-icon.h   \
			      $(NULL)

src_invenio_preferences_invenio_preferences_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS)
src_invenio_preferences_invenio_preferences_LDADD = $(GTK_LIBS) $(GLIB_LIBS) src/libinvenio/libinvenio.la
src_invenio_preferences_invenio_preferences_SOURCES = src/invenio-preferences/invenio-preferences.c            \
						      src/invenio-preferences/invenio-preferences-dialog.c     \
						      src/invenio-preferences/invenio-preferences-dialog.h     \
						      $(NULL)

MAINTAINERCLEANFILES = aclocal.m4 configure Makefile.in

maintainer-clean-local:
	-rm -fr config
	-rm -fr m4

.PHONY: maintainer-clean-local

