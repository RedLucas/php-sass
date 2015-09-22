#ifndef PHP_SASS
#define PHP_SASS

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define PHP_SASS_VERSION "0.0.2"
#define PHP_SASS_EXTNAME "sass"
#define COUNT_NORMAL      0
#define COUNT_RECURSIVE   1

#include <stdio.h>
#include <string.h>
#include <php.h>
#include <sass.h>
#include <Zend/zend_exceptions.h>
#include <ext/standard/php_array.h>
#include "ext/standard/info.h"

#define SASS_TYPE_FILE "file"
#define SASS_TYPE_DATA "data"

#define SASS_FUNCTION(name, def) Sass_Function_Entry \
	fn_##name = sass_make_function(def, name, NULL); \
	sass_function_set_list_entry(fn_list, i++, fn_##name);
	

bool sass_compile_context(char* s_input, const char* s_type, zval* pzv_options, zval* pzv_ret, zval* pzv_error);
union Sass_Value* sass_php_call(const char* s_func, const union Sass_Value* psv_args);
union Sass_Value* sass_report_error(const char* s_error);
void sass_to_php(union Sass_Value* psv_arg, zval* pzv_arg);
union Sass_Value* php_to_sass(zval* pzv_arg);
union Sass_Value* sass_dup_value(union Sass_Value* psv_v);
void sass_set_options(struct Sass_Options* pso_options, zval* pzv_options);
void sass_set_option(struct Sass_Options* pso_options, const char* s_name, zval* pzv_option);
int sass_php_count(HashTable* ht);
bool sass_check_args(const char* s_args, int count, const union Sass_Value* psv_args_list);

static PHP_MINFO_FUNCTION(sass);
PHP_FUNCTION(sass_version);
PHP_FUNCTION(sass_compile);
PHP_FUNCTION(sass_is_complete);

#define phpext_clips_ptr &sass_module_entry

#endif
