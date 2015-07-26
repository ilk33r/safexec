/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Ilker Ozcan <iletisim@ilkerozcan.com.tr>                     |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_SAFEEXEC_H
#define PHP_SAFEEXEC_H

#ifdef PHP_WIN32
#	define PHP_SAFEEXEC_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_SAFEEXEC_API __attribute__ ((visibility("default")))
#else
#	define PHP_SAFEEXEC_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"


#define PHP_SAFEEXEC_VERSION "0.1.0" /* Replace with version number for your extension */

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

*/
ZEND_BEGIN_MODULE_GLOBALS(safeexec)
	zend_bool dissallow_sudo_command;
	zend_bool dissallow_all_expect_php;
ZEND_END_MODULE_GLOBALS(safeexec)

typedef void (*php_func)(INTERNAL_FUNCTION_PARAMETERS);

/* In every utility function you add that needs to use variables 
   in php_safeexec_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as SAFEEXEC_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define SAFEEXEC_G(v) TSRMG(safeexec_globals_id, zend_safeexec_globals *, v)
#else
#define SAFEEXEC_G(v) (safeexec_globals.v)
#endif

extern zend_module_entry safeexec_module_entry;
#define phpext_safeexec_ptr &safeexec_module_entry

PHP_FUNCTION(safeexec_exec);
PHP_FUNCTION(safeexec_system);
PHP_FUNCTION(safeexec_passthru);
PHP_FUNCTION(safeexec_shell_exec);
PHP_FUNCTION(safeexec_proc_open);
PHP_FUNCTION(safeexec_popen);

#endif	/* PHP_SAFEEXEC_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
