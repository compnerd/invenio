dnl Copyright 2010 Saleem Abdulrasool <compnerd@compnerd.org>
dnl vim: set sw=4 sts=4 et fdm=marker fmr={{{,}}} :

dnl {{{ program, version
AC_PREREQ(2.59)
AC_INIT(invenio, 0.0.1, compnerd@compnerd.org)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(m4)
AM_CONFIG_HEADER(config/config.h)
AM_INIT_AUTOMAKE([1.11.1 subdir-objects dist-bzip2])
dnl }}}

dnl {{{ basic toolchain checks
AC_PROG_CC_C99
AC_PROG_CC_C_O
AC_PROG_INSTALL

LT_INIT([disable-shared])
dnl }}}

dnl {{{ library checks
X11_REQUIRED=1.2
GTK_REQUIRED=2.18
GLIB_REQUIRED=2.18
TRACKER_REQUIRED=0.7.25

PKG_CHECK_MODULES(X11, [x11 >= $X11_REQUIRED])
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= $GTK_REQUIRED gdk-2.0 >= $GTK_REQUIRED])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= $GLIB_REQUIRED gio-2.0 >= $GLIB_REQUIRED])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(TRACKER, [tracker-client-0.7 >= $TRACKER_REQUIRED])
AC_SUBST(TRACKER_CFLAGS)
AC_SUBST(TRACKER_LIBS)
dnl }}}

dnl {{{ output
AM_SILENT_RULES(yes)
AC_OUTPUT(Makefile)
dnl }}}

