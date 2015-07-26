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

#include "php_safeexec.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

static int le_safeexec;
/* True global resources - no need for thread safety here */

ZEND_DECLARE_MODULE_GLOBALS(safeexec)

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("safeexec.dissallow_sudo_command", "1", PHP_INI_ALL, OnUpdateBool, dissallow_sudo_command, zend_safeexec_globals, safeexec_globals)
    STD_PHP_INI_ENTRY("safeexec.dissallow_all_expect_php", "0", PHP_INI_ALL, OnUpdateBool, dissallow_all_expect_php, zend_safeexec_globals, safeexec_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_safeexec_init_globals
 */
static void php_safeexec_init_globals(zend_safeexec_globals *safeexec_globals)
{
	safeexec_globals->dissallow_sudo_command = '1';
	safeexec_globals->dissallow_all_expect_php = '0';
}
/* }}} */

static struct safeexec_overridden_fucs /* {{{ */ {
	php_func exec;
	php_func system;
	php_func passthru;
	php_func shell_exec;
	php_func proc_open;
	php_func popen;
} safeexec_origin_funcs;

#define SAFEEXEC_O_FUNC(m) (safeexec_origin_funcs.m)
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_safeexec_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_safeexec_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "safeexec", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

static unsigned char safeexec_exec(INTERNAL_FUNCTION_PARAMETERS, int mode) /* {{{ */
{
	char *cmd, cwd;
	int cmd_len, cwd_len;
	zval *ret_code=NULL, *ret_array=NULL;
	zval *pipes;
	zval *environment = NULL;
	zval *other_options = NULL;
	int ret;

	if (mode) {
		if(mode == 4)
		{
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "saz|s!a!a!", &cmd,
						&cmd_len, &ret_code, &pipes, &cwd, &cwd_len, &environment,
						&other_options) == FAILURE) {
				return '0';
			}
		} else if (mode == 5) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ps", &cmd, &cmd_len, &mode, &cwd_len) == FAILURE) {
				return '0';
			}
		} else {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z/", &cmd, &cmd_len, &ret_code) == FAILURE) {
				return '0';
			}
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z/z/", &cmd, &cmd_len, &ret_array, &ret_code) == FAILURE) {
			return '0';
		}
	}

	if(SAFEEXEC_G(dissallow_sudo_command))
	{
		char *searchSudoWord;
		searchSudoWord = strstr(cmd, "sudo ");

		 if (searchSudoWord != NULL)
		 {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot execute %s command. Sudo command is not allowed with safeexec extension", cmd);
			return '0';
		 }

		 searchSudoWord = strstr(cmd, "su ");

		 if (searchSudoWord != NULL)
		 {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot execute %s command. Su command is not allowed with safeexec extension", cmd);
			return '0';
		 }
	}

	if(SAFEEXEC_G(dissallow_all_expect_php))
	{
		char *searchCommand = "php ";
		int lenpre = strlen(searchCommand),
	        lenstr = strlen(cmd);
	    unsigned char isCommandStartsWithPhp = '0';

	    if(lenstr >= lenpre)
	    {
	    	if(strncmp(searchCommand, cmd, lenpre) == 0)
	    	{
	    		isCommandStartsWithPhp	= '1';
	    	}
	    }

	    if(isCommandStartsWithPhp == '0')
	    {
	    	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot execute %s command. Only PHP commands allowed with safeexec extension", cmd);
	    	return '0';
	    }
	}

	return '1';

}
/* }}} */

static void safeexec_override_func(char *name, uint len, php_func handler, php_func *stash TSRMLS_DC) /* {{{ */ {
	zend_function *func;
	if (zend_hash_find(CG(function_table), name, len, (void **)&func) == SUCCESS) {
		if (stash) {
			*stash = func->internal_function.handler;
		}
		func->internal_function.handler = handler;
	}
}
/* }}} */

static void safeexec_override_functions(TSRMLS_D) /* {{{ */ {
	char fn_exec[] = "exec";
	char fn_system[] = "system";
	char fn_passthru[] = "passthru";
	char fn_shell_exec[] = "shell_exec";
	char fn_proc_open[] = "proc_open";
	char fn_popen[] = "popen";

	safeexec_override_func(fn_exec, sizeof(fn_exec), PHP_FN(safeexec_exec), &SAFEEXEC_O_FUNC(exec) TSRMLS_CC);
	safeexec_override_func(fn_system, sizeof(fn_system), PHP_FN(safeexec_system), &SAFEEXEC_O_FUNC(system) TSRMLS_CC);
	safeexec_override_func(fn_passthru, sizeof(fn_passthru), PHP_FN(safeexec_passthru), &SAFEEXEC_O_FUNC(passthru) TSRMLS_CC);
	safeexec_override_func(fn_shell_exec, sizeof(fn_shell_exec), PHP_FN(safeexec_shell_exec), &SAFEEXEC_O_FUNC(shell_exec) TSRMLS_CC);
	safeexec_override_func(fn_proc_open, sizeof(fn_proc_open), PHP_FN(safeexec_proc_open), &SAFEEXEC_O_FUNC(proc_open) TSRMLS_CC);
	safeexec_override_func(fn_popen, sizeof(fn_popen), PHP_FN(safeexec_popen), &SAFEEXEC_O_FUNC(popen) TSRMLS_CC);
}
/* }}} */

/* {{{ proto string exec(string command [, array &output [, int &return_value]])
   Execute an external program */
PHP_FUNCTION(safeexec_exec)
{
	if(safeexec_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) == '1')
	{
		SAFEEXEC_O_FUNC(exec)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	} else {
		return;
	}

}
/* }}} */

/* {{{ proto int system(string command [, int &return_value])
   Execute an external program */
PHP_FUNCTION(safeexec_system)
{
	if(safeexec_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1) == '1')
	{
		SAFEEXEC_O_FUNC(system)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	} else {
		return;
	}

}
/* }}} */

/* {{{ proto void passthru(string command [, int &return_value])
   Execute an external program */
PHP_FUNCTION(safeexec_passthru)
{
	if(safeexec_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3) == '1')
	{
		SAFEEXEC_O_FUNC(passthru)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	} else {
		return;
	}
}
/* }}} */

/* {{{ proto string shell_exec(string cmd)
   Execute command via shell and return complete output as string */
PHP_FUNCTION(safeexec_shell_exec)
{
	if(safeexec_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1) == '1')
	{
		SAFEEXEC_O_FUNC(shell_exec)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	} else {
		return;
	}
}
/* }}} */

/* {{{ proto resource proc_open(string command, array descriptorspec, array &pipes [, string cwd [, array env [, array other_options]]])
   Run a process with more control over it's file descriptors */
PHP_FUNCTION(safeexec_proc_open)
{
	if(safeexec_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU, 4) == '1')
	{
		SAFEEXEC_O_FUNC(proc_open)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	} else {
		return;
	}
}
/* }}} */

/* {{{ proto resource popen(string command, string mode)
   Execute a command and open either a read or a write pipe to it */
PHP_FUNCTION(safeexec_popen)
{
	if(safeexec_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU, 5) == '1')
	{
		SAFEEXEC_O_FUNC(popen)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	} else {
		return;
	}
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(safeexec)
{
	/* If you have INI entries, uncomment these lines */
	REGISTER_INI_ENTRIES();

	safeexec_override_functions(TSRMLS_C);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(safeexec)
{
	/* uncomment this line if you have INI entries*/
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(safeexec)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(safeexec)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(safeexec)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Safeexec support", "enabled");
	php_info_print_table_row(2, "Safeexec version", PHP_SAFEEXEC_VERSION);
	php_info_print_table_row(2, "Author", "Ilker Ozcan");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ safeexec_functions[]
 *
 * Every user visible function must have an entry in safeexec_functions[].
 */
const zend_function_entry safeexec_functions[] = {
	PHP_FE(confirm_safeexec_compiled,	NULL)		/* For testing, remove later. */
	/* PHP_FE_END	* Must be the last line in safeexec_functions[] */
	{NULL, NULL, NULL}
};
/* }}} */

/** {{{ module depends
 */
#if ZEND_MODULE_API_NO >= 20050922
zend_module_dep safeexec_deps[] = {
	ZEND_MOD_CONFLICTS("xdebug")
	{NULL, NULL, NULL}
};
#endif
/* }}} */

/* {{{ safeexec_module_entry
 */
zend_module_entry safeexec_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
	STANDARD_MODULE_HEADER_EX, NULL,
	safeexec_deps,
#else
	STANDARD_MODULE_HEADER,
#endif
	"safeexec",
	safeexec_functions,
	PHP_MINIT(safeexec),
	PHP_MSHUTDOWN(safeexec),
	PHP_RINIT(safeexec),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(safeexec),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(safeexec),
	PHP_SAFEEXEC_VERSION,
	PHP_MODULE_GLOBALS(safeexec),
	NULL,    /* GINIT */
	NULL,    /* GSHUTDOWN */
	NULL,    /* RPOSTSHUTDOWN */
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_SAFEEXEC
ZEND_GET_MODULE(safeexec)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
