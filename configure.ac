AC_INIT([fbpager],[0.1.6])
AC_CONFIG_SRCDIR([src/main.cc])
AC_PREREQ([2.69])
AM_INIT_AUTOMAKE


AC_LANG([C++])
 
dnl Locate required external software
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
LT_INIT


dnl Check for system header files
# AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h signal.h sys/select.h sys/signal.h sys/stat.h sys/time.h sys/types.h sys/wait.h)

AC_CHECK_HEADERS(sstream, ,
        [ AC_CHECK_HEADERS(strstream,,
            [AC_MSG_ERROR([Your libstdc++ doesn't have the sstream or strstream classes])]
        )]
)

# AC_HEADER_TIME
dnl Determine the return type of signal handlers
AC_DEFINE_UNQUOTED([RETSIGTYPE],[$ac_cv_type_signal],[Define as the return type of signal handlers
		    (`int' or `void').])


AC_CHECK_FUNCS(sigaction strcasestr)

dnl Check for X headers and libraries
AC_PATH_X
AC_PATH_XTRA

test x$no_x = "xyes" && AC_MSG_ERROR([fbpager requires the X Window System libraries and headers.])

test x$x_includes = "x" && x_includes="/usr/include"
test x$x_libraries = "x" && x_libraries="/usr/lib"

CFLAGS="$CFLAGS $X_CFLAGS"
CXXFLAGS="$CXXFLAGS $X_CFLAGS"
LIBS="$LIBS $X_LIBS"
LDFLAGS="$LDFLAGS $LIBS $X_PRE_LIBS"

dnl Check for required functions in -lX11
AC_CHECK_LIB(X11, XOpenDisplay,
  LIBS="$LIBS -lX11",
  AC_MSG_ERROR([Could not find XOpenDisplay in -lX11.])
)

LIBS="$LIBS $X_EXTRA_LIBS"

Xext_lib=""


dnl Check whether to include debugging code
DEBUG=""
AC_MSG_CHECKING([whether to include verbose debugging code])
AC_ARG_ENABLE(debug,
  [  --enable-debug          include verbose debugging code [default=no]],
  if test x$enableval = "xyes"; then
    AC_MSG_RESULT([yes])
    DEBUG="-DDEBUG -fno-inline"
  else
    AC_MSG_RESULT([no])
  fi,
  AC_MSG_RESULT([no])
)
AC_SUBST(DEBUG)
CXXFLAGS="$CXXFLAGS $DEBUG"

AC_MSG_CHECKING([whether to have Xrender (transparent) support])
AC_ARG_ENABLE(
  xrender,
[  --enable-xrender        Xrender (transparent) support [default=yes]],
  if test x$enableval = "xyes"; then
	AC_MSG_RESULT([yes])
	AC_CHECK_LIB(Xrender, XRenderCreatePicture, 
		AC_DEFINE(HAVE_XRENDER, 1, "Xrender support")
		LIBS="$LIBS -lXrender")
  else
    AC_MSG_RESULT([no])	
  fi,  
  AC_MSG_RESULT([yes])
  AC_CHECK_LIB(Xrender, XRenderCreatePicture, 
	AC_DEFINE(HAVE_XRENDER, 1, "Xrender support")
	LIBS="$LIBS -lXrender")
)

dnl Print results
AC_MSG_RESULT([])
AC_MSG_RESULT([	$PACKAGE version $VERSION configured successfully.])
AC_MSG_RESULT([])
AC_MSG_RESULT([Using '$prefix' for installation.])
AC_MSG_RESULT([Using '$CXX' for C++ compiler.])
AC_MSG_RESULT([Building with '$CXXFLAGS' for C++ compiler flags.])
AC_MSG_RESULT([Building with '$LIBS' for linker flags.])
AC_MSG_RESULT([])

dnl Output files
AM_CONFIG_HEADER(config.h)

AC_CONFIG_FILES([Makefile
version.h
src/Makefile
src/FbTk/Makefile
])
AC_OUTPUT
