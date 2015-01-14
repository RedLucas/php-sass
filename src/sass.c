#include "sass_options.h"
#include "sass_functions.h"

int sass_php_count(HashTable* ht) {
	HashPosition position;
	zval **data = NULL;

	int count = 0;
	// Iterating all the key and values in the context
	for (zend_hash_internal_pointer_reset_ex(ht, &position);
		 zend_hash_get_current_data_ex(ht, (void**) &data, &position) == SUCCESS;
		 zend_hash_move_forward_ex(ht, &position)) {
		 count++;
	}
	return count;
}

struct Sass_Import** sass_php_importer(const char* url, const char* prev, void* cookie) {
	struct Sass_Import** list = sass_make_import_list(2);
	const char* local = "local { color: green; }";
	const char* remote = "remote { color: red; }";
	list[0] = sass_make_import_entry("/tmp/styles.scss", strdup(local), 0);
	list[1] = sass_make_import_entry("http://www.example.com", strdup(remote), 0);
	return list;
}

union Sass_Value* sass_report_error(const char* s_error) {
	zend_throw_exception(zend_exception_get_default(TSRMLS_C), s_error, 0 TSRMLS_CC);
	return sass_make_error(s_error);
}

void sass_to_php(union Sass_Value* psv_arg, zval* pzv_arg) {
	if(sass_value_is_null(psv_arg)) {
		ZVAL_NULL(pzv_arg);
	}
	else if (sass_value_is_number(psv_arg)) {
		ZVAL_DOUBLE(pzv_arg, sass_number_get_value(psv_arg));
	}
	else if (sass_value_is_string(psv_arg)) {
		ZVAL_STRING(pzv_arg, sass_string_get_value(psv_arg), true);
	}
	else if (sass_value_is_boolean(psv_arg)) {
		if(sass_boolean_get_value(psv_arg)) {
			ZVAL_TRUE(pzv_arg);
		}
		else {
			ZVAL_FALSE(pzv_arg);
		}
	}
	else if (sass_value_is_color(psv_arg)) {
		char color[24];
		snprintf(color, sizeof color, "#%.2x%.2x%.2x", 
			(int)sass_color_get_r(psv_arg), 
			(int)sass_color_get_g(psv_arg), 
			(int)sass_color_get_b(psv_arg));
		ZVAL_STRING(pzv_arg, color, true);
	}
	else if (sass_value_is_list(psv_arg)) {

		size_t count = sass_list_get_length(psv_arg);

		// Initlialize the php arg as an array
		array_init_size(pzv_arg, count);

		int i;
		
		for(i = 0; i < count; i++) {
			zval* pzv_array_item = NULL;
			MAKE_STD_ZVAL(pzv_array_item);

			// Conver the list item to php
			sass_to_php(sass_list_get_value(psv_arg, i), pzv_array_item);

			// Add the array item to the array
			add_next_index_zval(pzv_arg, pzv_array_item);
		}
	}
	else if (sass_value_is_map(psv_arg)) {
		size_t count = sass_map_get_length(psv_arg);

		// Initlialize the php arg as an array
		array_init_size(pzv_arg, count);

		int i;
		
		for(i = 0; i < count; i++) {
			zval* pzv_array_item = NULL;
			MAKE_STD_ZVAL(pzv_array_item);

			// Conver the list item to php
			sass_to_php(sass_map_get_value(psv_arg, i), pzv_array_item);

			// Add the array item to the array
			add_assoc_zval(pzv_arg, sass_string_get_value(sass_map_get_key(psv_arg, i)), pzv_array_item);
		}
	}
}

union Sass_Value* convert_php_to_list(zval* pzv_val) {
	int count = sass_php_count(Z_ARRVAL_P(pzv_val));
	union Sass_Value** values = safe_emalloc(sizeof(union Sass_Value*), count, 0);

	HashTable* ht = Z_ARRVAL_P(pzv_val);
	HashPosition position;
	zval **data = NULL;

	int real_count = 0;
	// Iterating all the key and values in the context
	for (zend_hash_internal_pointer_reset_ex(ht, &position);
		 zend_hash_get_current_data_ex(ht, (void**) &data, &position) == SUCCESS;
		 zend_hash_move_forward_ex(ht, &position)) {

		 char *key = NULL;
		 uint  klen;
		 ulong index;

		 // Only require the string key, all the index key will be ignored
		 if (zend_hash_get_current_key_ex(ht, &key, &klen, &index, 0, &position) == HASH_KEY_IS_LONG) {
			 values[real_count] = php_to_sass(*data);
			 real_count++;
		 }
	}

	// Create the map
	union Sass_Value* psv_list = sass_make_list(real_count, SASS_SPACE);

	int i;
	for(i = 0; i < real_count; i++) {
		sass_list_set_value(psv_list, i, values[i]);
	}

	efree(values);

	return psv_list;
}

union Sass_Value* convert_php_to_map(zval* pzv_val) {
	int count = sass_php_count(Z_OBJPROP_P(pzv_val));
	const char** keys = emalloc((count + 1) * sizeof(const char*));
	union Sass_Value** values = emalloc((count + 1) * sizeof(union Sass_Value*));

	HashTable* ht = Z_OBJPROP_P(pzv_val);
	HashPosition position;
	zval **data = NULL;

	int real_count = 0;
	// Iterating all the key and values in the context
	for (zend_hash_internal_pointer_reset_ex(ht, &position);
		 zend_hash_get_current_data_ex(ht, (void**) &data, &position) == SUCCESS;
		 zend_hash_move_forward_ex(ht, &position)) {

		 char *key = NULL;
		 uint  klen;
		 ulong index;

		 // Only require the string key, all the index key will be ignored
		 if (zend_hash_get_current_key_ex(ht, &key, &klen, &index, 0, &position) == HASH_KEY_IS_STRING) {
			 keys[real_count] = key;
			 values[real_count] = php_to_sass(*data);
			 real_count++;
		 }
	}

	// Create the map
	union Sass_Value* psv_map = sass_make_map(real_count);

	int i;
	for(i = 0; i < real_count; i++) {
		sass_map_set_key(psv_map, i, sass_make_string(keys[i]));
		sass_map_set_value(psv_map, i, values[i]);
	}

	efree(keys);
	efree(values);

	return psv_map;
}

union Sass_Value* php_to_sass(zval* pzv_arg) {
	switch(Z_TYPE_P(pzv_arg)) {
	case IS_LONG:
		return sass_make_number(Z_LVAL_P(pzv_arg), "");
	case IS_DOUBLE:
		return sass_make_number(Z_DVAL_P(pzv_arg), "");
	case IS_BOOL:
		return sass_make_boolean(Z_BVAL_P(pzv_arg));
	case IS_ARRAY:
		// Will treat array as list and object as map
		return convert_php_to_list(pzv_arg);
	case IS_OBJECT:
		return convert_php_to_map(pzv_arg);
	case IS_STRING:
		return sass_make_string(Z_STRVAL_P(pzv_arg));
	}
	return sass_make_null();
}

union Sass_Value* sass_php_call(const char* s_func, const union Sass_Value* psv_args) {
	size_t args_count = sass_list_get_length(psv_args) - 1; // Skip the first arg of function name

	// Copy the function name to php variable
    zval* pzv_function_name = NULL;
    MAKE_STD_ZVAL(pzv_function_name);
    ZVAL_STRING(pzv_function_name, s_func, true);

    zend_uint argc = sass_list_get_length(psv_args) - 1;

    // Initialize the paramter array
    zval** ppzv_params = (zval**) emalloc(argc * sizeof(zval*));
    zval *pzv_php_ret_val = NULL;

	// Initialize the return php value
	MAKE_STD_ZVAL(pzv_php_ret_val);

	int i;
	// Setup the input parameters
	for(i = 0; i < argc; i++) {
		// Initialize the php value
		zval* pzv_val = NULL;
		MAKE_STD_ZVAL(pzv_val);

		// Getting sass value
		union Sass_Value* psv_val = sass_list_get_value(psv_args, i + 1);
		
		// Convert it to php
		sass_to_php(psv_val, pzv_val);

		// Adding it to the php args
		ppzv_params[i] = pzv_val;
	}

	union Sass_Value* psv_ret = NULL;

    if (call_user_function(EG(function_table), NULL, pzv_function_name, pzv_php_ret_val, argc, ppzv_params TSRMLS_CC) == SUCCESS) {
		psv_ret = php_to_sass(pzv_php_ret_val);
	}

    // Destroy all the php parameter variables
    for(i = 0; i < argc; i++) {
		zval_ptr_dtor(&ppzv_params[i]);
    }

	// Free the args list
    efree(ppzv_params);

    // Destroy the php return variable
    zval_ptr_dtor(&pzv_php_ret_val);

    // Destroy the php function name variable
    zval_ptr_dtor(&pzv_function_name);

	if(psv_ret)
		return psv_ret;

	char buff[256];
	sprintf(buff, "Failed to call php function %s!", s_func);
	return sass_report_error(buff);
}

/**
 * Set all the options in the php hashtable and set them into Sass options.
 *
 * @args
 * 		sass_options: The options struct
 * 		php_options: The php hashtable
 */
void sass_set_options(struct Sass_Options* pso_options, zval* pzv_options) {
	HashTable* ht = Z_ARRVAL_P(pzv_options);
    HashPosition position;
    zval **data = NULL;

    // Iterating all the key and values in the context
    for (zend_hash_internal_pointer_reset_ex(ht, &position);
         zend_hash_get_current_data_ex(ht, (void**) &data, &position) == SUCCESS;
         zend_hash_move_forward_ex(ht, &position)) {

         char *key = NULL;
         uint  klen;
         ulong index;

         if (zend_hash_get_current_key_ex(ht, &key, &klen, &index, 0, &position) == HASH_KEY_IS_STRING) {
			 sass_set_option(pso_options, key, *data);
         }
    }


	// Adding the extend functions.
	
	// allocate a custom function caller
	Sass_C_Function_Callback fn_php = sass_make_function("php($func...)", call_fn_php, NULL);
	Sass_C_Function_Callback fn_str_get = sass_make_function("str-get($str, $index)", call_fn_str_get, NULL);
	Sass_C_Function_Callback fn_pow = sass_make_function("pow($i, $n)", call_fn_pow, NULL);
	Sass_C_Function_Callback fn_gettype = sass_make_function("gettype($i)", call_fn_gettype, NULL);
	Sass_C_Function_Callback fn_remove_nth = sass_make_function("remove-nth($l, $i)", call_fn_remove_nth, NULL);
	Sass_C_Function_Callback fn_list_end = sass_make_function("list-end($i)", call_fn_list_end, NULL);
	Sass_C_Function_Callback fn_list_splice = sass_make_function("list-splice($list, $offset:0, $count:0, $list_append:null)", call_fn_list_splice, NULL);

	// create list of all custom functions
	Sass_C_Function_List fn_list = sass_make_function_list(7);
	sass_function_set_list_entry(fn_list, 0, fn_php);
	sass_function_set_list_entry(fn_list, 1, fn_str_get);
	sass_function_set_list_entry(fn_list, 2, fn_pow);
	sass_function_set_list_entry(fn_list, 3, fn_gettype);
	sass_function_set_list_entry(fn_list, 4, fn_remove_nth);
	sass_function_set_list_entry(fn_list, 5, fn_list_end);
	sass_function_set_list_entry(fn_list, 6, fn_list_splice);
	sass_option_set_c_functions(pso_options, fn_list);

	// Adding the php stream importers
	// sass_option_set_importer(pso_options, sass_make_importer(sass_php_importer, NULL));
}

/**
 * Compile the input.
 *
 * @args
 * 		input: The input, for file compile, it is the file name, for string compile, it is the string
 * 		type: The type of the compile, support file or data
 * 		options: The php hashtable for all the options
 * 		error: The error string
 */
const char* sass_compile_context(char* s_input, const char* s_type, zval* pzv_options, zval* psv_error) {

	const char* ret = NULL;

	if(strcmp(s_type, SASS_TYPE_FILE) == 0) {
		// Initialize the context
		struct Sass_File_Context* psfc_file_ctx = sass_make_file_context(s_input);
		struct Sass_Context* psc_ctx = sass_file_context_get_context(psfc_file_ctx);
		struct Sass_Options* pso_ctx_opt = sass_context_get_options(psc_ctx);

		// Set the options from php
		sass_set_options(pso_ctx_opt, pzv_options);
		
		// Set it back to sass
		sass_file_context_set_options(psfc_file_ctx, pso_ctx_opt);

		// Do the compile
		int status = sass_compile_file_context(psfc_file_ctx);

		if(status) {
			// Set the error message
			ZVAL_STRING(psv_error, sass_context_get_error_message(psc_ctx), true);
		}
		else {
			ret = sass_context_get_output_string(psc_ctx);
		}

		// Clean the context
		sass_delete_file_context(psfc_file_ctx);

	}
	else if(strcmp(s_type, SASS_TYPE_DATA) == 0) {
		struct Sass_Data_Context* psdc_data_ctx = sass_make_data_context(s_input);
		struct Sass_Context* psc_ctx = sass_data_context_get_context(psdc_data_ctx);
		struct Sass_Options* pso_ctx_opt = sass_context_get_options(psc_ctx);

		sass_set_options(pso_ctx_opt, pzv_options);

		// Set it back to sass
		sass_data_context_set_options(psdc_data_ctx, pso_ctx_opt);

		// Do the compile
		int status = sass_compile_data_context(psdc_data_ctx);
		
		if(status) {
			// Set the error message
			ZVAL_STRING(psv_error, sass_context_get_error_message(psc_ctx), true);
		}
		else {
			ret = sass_context_get_output_string(psc_ctx);
		}
		// Clean the context
		sass_delete_data_context(psdc_data_ctx);
	}

	if(ret)
		return ret;

	return NULL;
}

void sass_set_option(struct Sass_Options* pso_options, const char* s_name, zval* pzv_option) {
	if(strcmp(s_name, SASS_OPTION_PRECISION) == 0) {
		sass_option_set_precision(pso_options, Z_LVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_OUTPUT_STYLE) == 0) {
		sass_option_set_output_style(pso_options, Z_LVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_COMMENTS) == 0) {
		sass_option_set_source_comments(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_MAP_EMBED) == 0) {
		sass_option_set_source_map_embed(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_MAP_CONTENTS) == 0) {
		sass_option_set_source_map_contents(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_OMIT_SOURCE_MAP_URL) == 0) {
		sass_option_set_omit_source_map_url(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_IS_INDENTED_SYNTAX_SRC) == 0) {
		sass_option_set_is_indented_syntax_src(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_INDENT) == 0) {
		sass_option_set_indent(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_LINEFEED) == 0) {
		sass_option_set_linefeed(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_INPUT_PATH) == 0) {
		sass_option_set_input_path(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_OUTPUT_PATH) == 0) {
		sass_option_set_output_path(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_IMAGE_PATH) == 0) {
		sass_option_set_image_path(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_INCLUDE_PATH) == 0) {
		sass_option_set_include_path(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_MAP_FILE) == 0) {
		sass_option_set_source_map_file(pso_options, Z_STRVAL_P(pzv_option));
	}
}

/*******************************************************************************
 *
 *  Function sass_compile
 *
 *  This function will run the sass compiler using the options
 *
 *  @version 1.0
 *  @args
 *
 *******************************************************************************/
PHP_FUNCTION(sass_compile) {
	char* s_type = NULL;
	int type_len;
	char* s_input = NULL;
	int input_len = 0;
	zval* pzv_options = NULL;
	zval* pzv_error = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssAz", &s_type, &type_len,
			   	&s_input, &input_len,
				&pzv_options, &pzv_error) == SUCCESS) {
		const char* s_ret = sass_compile_context(s_input, s_type, pzv_options, pzv_error);

		if(s_ret)
			RETURN_STRING(s_ret, true);

		RETURN_FALSE;
	}
	RETURN_FALSE;
}


/*******************************************************************************
 *
 *  Function sass_version
 *
 *  This function will return the version of the libsass
 *
 *  @version 1.0
 *
 *******************************************************************************/
PHP_FUNCTION(sass_version) {
	RETURN_STRING(libsass_version(), true);
}

static zend_function_entry sass_functions[] = {
    PHP_FE(sass_compile, NULL)   
    PHP_FE(sass_version, NULL)   
    {NULL, NULL, NULL}         
};
  
zend_module_entry sass_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,    
#endif
    PHP_SASS_EXTNAME,         
    sass_functions,           
    NULL, // No initialize functions
	NULL, // No deinitialize functions
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_SASS_VERSION,         
#endif
    STANDARD_MODULE_PROPERTIES 
};
  
#ifdef COMPILE_DL_SASS
ZEND_GET_MODULE(sass)
#endif

