dnl config.m4 for extension avahi

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(avahi, for avahi support,
dnl Make sure that the comment is aligned:
dnl [  --with-avahi             Include avahi support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(avahi, whether to enable avahi support,
dnl Make sure that the comment is aligned:
[  --enable-avahi          Enable avahi support], no)

if test "$PHP_AVAHI" != "no"; then
  dnl Write more examples of tests here...

  dnl # get library FOO build options from pkg-config output
  dnl AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  dnl AC_MSG_CHECKING(for libfoo)
  dnl if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists foo; then
  dnl   if $PKG_CONFIG foo --atleast-version 1.2.3; then
  dnl     LIBFOO_CFLAGS=\`$PKG_CONFIG foo --cflags\`
  dnl     LIBFOO_LIBDIR=\`$PKG_CONFIG foo --libs\`
  dnl     LIBFOO_VERSON=\`$PKG_CONFIG foo --modversion\`
  dnl     AC_MSG_RESULT(from pkgconfig: version $LIBFOO_VERSON)
  dnl   else
  dnl     AC_MSG_ERROR(system libfoo is too old: version 1.2.3 required)
  dnl   fi
  dnl else
  dnl   AC_MSG_ERROR(pkg-config not found)
  dnl fi
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBDIR, AVAHI_SHARED_LIBADD)
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)

  dnl # --with-avahi -> check with-path
  SEARCH_PATH="/usr"
  SEARCH_FOR="/include/avahi-common/simple-watch.h"
  if test -r $PHP_AVAHI/$SEARCH_FOR; then # path given as parameter
    AVAHI_DIR=$PHP_AVAHI
  else # search default path list
    AC_MSG_CHECKING([for avahi files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        AVAHI_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$AVAHI_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the avahi distribution])
  fi

  dnl # --with-avahi -> add include path
  PHP_ADD_INCLUDE($AVAHI_DIR/include)

  dnl # --with-avahi -> check for lib and symbol presence
  LIBNAME=avahi-common
  LIBSYMBOL=avahi_string_list_get_service_cookie

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $AVAHI_DIR/$PHP_LIBDIR, AVAHI_SHARED_LIBADD)
    AC_DEFINE(HAVE_AVAHILIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong avahi lib version or lib not found])
  ],[
    -L$AVAHI_DIR/$PHP_LIBDIR -lm
  ])

  dnl # --with-avahi -> check with-path
  SEARCH_PATH="/usr"
  SEARCH_FOR="/include/avahi-client/client.h"
  if test -r $PHP_AVAHI/$SEARCH_FOR; then # path given as parameter
    AVAHI_DIR=$PHP_AVAHI
  else # search default path list
    AC_MSG_CHECKING([for avahi files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        AVAHI_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$AVAHI_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the avahi distribution])
  fi

  dnl # --with-avahi -> add include path
  PHP_ADD_INCLUDE($AVAHI_DIR/include)

  dnl # --with-avahi -> check for lib and symbol presence
  LIBNAME=avahi-client
  LIBSYMBOL=avahi_client_new

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $AVAHI_DIR/$PHP_LIBDIR, AVAHI_SHARED_LIBADD)
    AC_DEFINE(HAVE_AVAHILIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong avahi lib version or lib not found])
  ],[
    -L$AVAHI_DIR/$PHP_LIBDIR -lm
  ])

  PHP_SUBST(AVAHI_SHARED_LIBADD)

  dnl # In case of no dependencies
  AC_DEFINE(HAVE_AVAHI, 1, [ Have avahi support ])

  PHP_NEW_EXTENSION(avahi, [avahi.c avahi-data.c], $ext_shared)
fi
