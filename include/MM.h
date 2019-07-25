
#ifndef __ZHMH_PUBLIC_MEMMNG_H_INCLUDED
#define __ZHMH_PUBLIC_MEMMNG_H_INCLUDED

#include "zhmh_define.h"
#include <stdio.h>

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

define_interface {
    define_interface_function(void, init)
        (handle_t, FILE* log, FILE* err);

    define_interface_function(void*, allocate)
        (handle_t handle, const char* filename, size_t line, size_t size);

    define_interface_function(void*, reallocate)
        (handle_t handle, const char* filename, size_t line, void* pointer, size_t size);

    define_interface_function(void, free)
        (handle_t handle, void* pointer);

    define_interface_function(void, dump)
        (handle_t handle);

} MEMMNG_Methods;

expose_block(MEMMNG);

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
#ifdef __debug

#define MEM_Init()                  ((MEMMNG_Methods*)MEMMNG[1])->init(MEMMNG[0], stdout, stderr)
#define MEM_Allocate(SIZE)          ((MEMMNG_Methods*)MEMMNG[1])->allocate(MEMMNG[0], __FILE__, __LINE__, SIZE)
#define MEM_Reallocate(PTR, SIZE)   ((MEMMNG_Methods*)MEMMNG[1])->reallocate(MEMMNG[0], __FILE__, __LINE__, PTR, SIZE)
#define MEM_Free(PTR)               ((MEMMNG_Methods*)MEMMNG[1])->free(MEMMNG[0], PTR)
#define MEM_Dump()                  ((MEMMNG_Methods*)MEMMNG[1])->dump(MEMMNG[0])

#else

#define MEM_Init()
#define MEM_Allocate(SIZE)          malloc(SIZE)
#define MEM_Reallocate(PTR, SIZE)   realloc(PTR, SIZE)
#define MEM_Free(PTR)               free(PTR)
#define MEM_Dump()

#endif // __debug
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
#ifdef __debug

define_interface {
    define_interface_function(handle_t, open)
        (handle_t, const char* filename, size_t line, size_t page_size);

    define_interface_function(void*, allocate)
        (handle_t, const char* filename, size_t line, size_t size);

    define_interface_function(void, close)
        (handle_t);
} MEMMNG_Storage_Methods;

expose_interface(MEMMNG_Storage);

#define MEM_S_Open(PAGE_SIZE)           ((MEMMNG_Storage_Methods*)MEMMNG_Storage)->open (MEMMNG[0], __FILE__, __LINE__, PAGE_SIZE)
#define MEM_S_Allocate(STORAGE_H, SIZE) ((MEMMNG_Storage_Methods*)MEMMNG_Storage)->allocate (STORAGE_H, __FILE__, __LINE__, SIZE)
#define MEM_S_Close(STORAGE_H)          ((MEMMNG_Storage_Methods*)MEMMNG_Storage)->close (STORAGE_H)

#endif // __debug
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#endif // __ZHMH_PUBLIC_MEMMNG_H_INCLUDED
