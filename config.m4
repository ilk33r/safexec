dnl $Id$
dnl config.m4 for extension safeexec

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(safeexec, for safeexec support,
dnl Make sure that the comment is aligned:
[  --with-safeexec             Include safeexec support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(safeexec, whether to enable safeexec support,
dnl Make sure that the comment is aligned:
[  --enable-safeexec           Enable safeexec support])

if test "$PHP_SAFEEXEC" != "no"; then
  PHP_NEW_EXTENSION(safeexec, safeexec.c, $ext_shared)
fi