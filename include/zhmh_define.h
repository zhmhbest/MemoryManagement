/* Keywords :
define_private_function     call_private_function
define_protected_function   call_protected_function
define_public_function
define_interface_function
handle_t
define_interface
expose_block
expose_interface
define_handle
implement_handle
implement_interface
bind_expose
bind_interface
*/

//■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■
//■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■ ■■
//■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#ifndef __ZHMH_C_DEFINE_H_HEAD
#define __ZHMH_C_DEFINE_H_HEAD

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

/*!
 * 通用编译环境
 */
#if !defined(__WINDOWS__) && ( \
    defined(MSC_VCR) || defined(_MSC_VCR) || \
    defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__) ||  \
    defined(WIN64) || defined(_WIN64) || defined(__WIN64) || defined(__WIN64__)     \
)
    #define __WINDOWS__
#endif

#if !defined(__SYSENV32__) && ( __SIZEOF_POINTER__ == 4 )
    #define __SYSENV32__
#endif

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■


#define get_composed_function_name(fun, l_mark, r_mark) l_mark##fun##r_mark

/**
 * private : 仅允许在.c文件内部使用。
 */
#define call_private_function(FUN)              get_composed_function_name(FUN, __private$, $private)
#define define_private_function(RET, FUN)       static RET call_private_function(FUN)

/**
 * protected : 对外暴露但不提供给用户直接使用。
 */
#define call_protected_function(FUN)            get_composed_function_name(FUN, __protected$, $protected)
#define define_protected_function(RET, FUN)     RET call_protected_function(FUN)

/**
 * public : 提供给用户直接使用。
 */
#define define_public_function(RET, FUN)        extern RET FUN

/**
 * interface : 结构体内定义的函数指针。
 */
#define define_interface_function(RET, FUN)     RET (*FUN)

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■

typedef void* handle_t;

#define define_interface    typedef struct
#define define_handle       typedef struct

#define expose_block(NAME)          extern handle_t NAME [2]
#define expose_interface(NAME)      extern handle_t NAME

#define implement_handle(HANDLE_TYPE)       static HANDLE_TYPE    __Living_Handle
#define implement_interface(INTERFACE_TYPE) static INTERFACE_TYPE __Living_Methods =

#define bind_expose(NAME)       handle_t NAME[2] = { &__Living_Handle, &__Living_Methods }
#define bind_interface(NAME)    handle_t NAME = &__Living_Methods;

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#endif // __ZHMH_C_DEFINE_H_HEAD
