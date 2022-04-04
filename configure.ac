AC_PREREQ([2.69])
AC_INIT([Epanet],[2.4],[https://github.com/jburguete/EPANET])

# Checks for operative systems
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AC_MSG_NOTICE([Build: $build])
AC_MSG_NOTICE([Host: $host])
AC_MSG_NOTICE([Target: $target])
if test "x$win" = "x1"; then
	AC_SUBST(EXE, ".exe")
	AC_SUBST(SO, ".dll")
else
	AC_SUBST(SO, ".so")
fi

# Checks for programs
AC_PROG_CC([egcc gcc clang])
AC_MSG_NOTICE([Compiler: $CC])
AC_PROG_MAKE_SET
AC_LANG([C])
PKG_PROG_PKG_CONFIG
compiler=$CC

# Checking gprof flag
prof=0
AC_ARG_WITH([gprof], AS_HELP_STRING([--with-gprof], [Build with gprof]))
if test "x$with_gprof" = "xyes"; then
	CC="$CC -pg"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [])], [prof=1], )
	CC=$compiler
	if test "x$prof" = "x0"; then
		AC_MSG_NOTICE([checking for -pg... no])
	else
		AC_MSG_NOTICE([checking for -pg... yes])
	fi
fi
if test "x$prof" = "x0"; then
	AC_SUBST(GPROF, "-g")
else
	AC_SUBST(GPROF, "-pg")
fi

# Checking -march=native compiler flag
AC_ARG_WITH([native], AS_HELP_STRING([--with-native],
	[Build with native optimization]))
if test "x$with_native" = "xyes"; then
	CC="$CC -march=native"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [])], [arch=1], [arch=0])
	CC=$compiler
	if test "x$arch" = "x0"; then
		AC_MSG_NOTICE([checking for -march=native... no])
		AC_SUBST(ARCH, "")
	else
		AC_MSG_NOTICE([checking for -march=native... yes])
		AC_SUBST(ARCH, "-march=native")
	fi
fi

# Checking -flto compiler flag
CC="$CC -flto"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [])], [lto=1], [lto=0])
CC=$compiler
if test "x$lto" = "x0"; then
	AC_MSG_NOTICE([checking for -flto... no])
	AC_SUBST(LTO, "")
else
	AC_MSG_NOTICE([checking for -flto... yes])
	AC_SUBST(LTO, "-flto")
fi

# Checking PGO optimiztion
AC_ARG_WITH([pgo], AS_HELP_STRING([--with-pgo], [Build with PGO optimization]))
if test "x$with_pgo" = "xyes"; then
	CC="$CC -fprofile-generate"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [])], [pgo=1], [pgo=0])
	CC=$compiler
	if test "x$pgo" = "x0"; then
		AC_MSG_NOTICE("checking for PGO... no")
		AC_SUBST(PGO, "0")
	else
		AC_MSG_NOTICE("checking for PGO... yes")
		AC_SUBST(PGO, "1")
		if test "x$compiler" = "xclang"; then
			AC_SUBST(PGO_GENERATE, "-fprofile-generate")
			AC_SUBST(PGO_USE, "-fprofile-use=\$(BIN)")
      AC_SUBST(PGO_MERGE,
               "llvm-profdata merge -output=default.profdata default_*.profraw")
		else
			AC_SUBST(PGO_GENERATE, "-fprofile-generate")
			AC_SUBST(PGO_USE, "-fprofile-use -fprofile-correction")
		fi
	fi
fi

# Check libs
PKG_CHECK_MODULES([GLIB], [glib-2.0])

# Final output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
