dnl configure.ac for ccze
dnl Copyright (C) 2002 Gergely Nagy <algernon@bonehunter.rulez.org
dnl
dnl Process this file with autoconf to produce a configure script.
dnl
dnl This file is part of ccze.
dnl
dnl ccze is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl ccze is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

dnl **********************
dnl * Boilerplate
dnl **********************
AC_PREREQ(2.49d)
AC_INIT([ccze], [0.1-snapshot], [algernon@bonehunter.rulez.org])
AC_CONFIG_SRCDIR(src/ccze.c)

AC_CONFIG_HEADERS(system.h)

AC_CANONICAL_HOST
AC_SUBST(host_os)

VERSION="$PACKAGE_VERSION"

dnl **********************
dnl * Programs
dnl **********************
AC_CHECK_TOOL(CC, gcc)

WFLAGS_3X=""
AC_MSG_CHECKING(if ${CC} is gcc 3.x)
GCCVER=$(${CC} -dumpversion)
case ${GCCVER} in
	3.*)
		WFLAGS_3X='${WFLAGS_3X}'
		AC_MSG_RESULT(yes)
		;;
	*)
		AC_MSG_RESULT(no)
		;;
esac
AC_SUBST(WFLAGS_3X)

AC_PROG_CC
AC_PROG_INSTALL

dnl **********************
dnl * Headers
dnl **********************
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([netdb.h])

dnl **********************
dnl * Typedefs & co
dnl **********************
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL

dnl **********************
dnl * Library functions
dnl **********************
AC_FUNC_MALLOC
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memset strcasecmp strchr strdup strndup strstr])

AC_CHECK_LIB(pcre, pcre_exec, [], [AC_ERROR(PCRE library not found)])
AC_CHECK_LIB(ncurses, initscr, [], [AC_ERROR(NCurses library not found)])
AC_CHECK_LIB(dl, dlopen, [], [AC_ERROR(dlopen() not found)])

dnl **********************
dnl * Output
dnl **********************
AC_CONFIG_FILES([
Rules.mk
Makefile
src/Makefile
])

AC_OUTPUT
