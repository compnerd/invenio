dnl Copyright 2010 Saleem Abdulrasool <compnerd@compnerd.org>
dnl vim: set sw=4 sts=4 et fdm=marker fmr={{{,}}} :

dnl {{{ program, version
AC_PREREQ(2.59)
AC_INIT(invenio, 0.0.1, compnerd@compnerd.org)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(m4)
AM_CONFIG_HEADER(config/config.h)
AM_INIT_AUTOMAKE([subdir-objects dist-bzip2])
dnl }}}

dnl {{{ basic toolchain checks
AC_PROG_CC_C99
AC_PROG_CC_C_O
AC_PROG_INSTALL
dnl }}}

dnl {{{ library checks
GTK_REQUIRED=2.18

PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= $GTK_REQUIRED])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(GDK, [gdk-2.0])
AC_SUBST(GDK_CFLAGS)
AC_SUBST(GDK_LIBS)
dnl }}}

dnl {{{ output
AM_SILENT_RULES(yes)
AC_OUTPUT(Makefile)
dnl }}}

