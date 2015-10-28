#ifndef PHP_SASS_FUNCTIONS
#define PHP_SASS_FUNCTIONS

#include "sass.h"
#include <math.h>

union Sass_Value* call_fn_php(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_str_get(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_list_start(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_list_end(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_list_reverse(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_pow(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_gettype(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_remove_nth(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_list_splice(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_list_set(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_strip_unit(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_assert(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 
union Sass_Value* call_fn_convert_unit(const union Sass_Value* psv_args, Sass_Function_Entry cb, struct Sass_Compiler* options); 

#endif
