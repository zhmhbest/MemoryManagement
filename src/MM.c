#include "MM.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

//!MemoryManagement Private Structure

typedef struct {
    uintptr_t mark;
} MemoryMark;

typedef struct __$MemoryHeader MemoryHeader;
struct __$MemoryHeader {
    size_t      size;
    size_t      line;
    const char* name; //file name
    MemoryHeader*       prev;
    MemoryHeader*       next;
    MemoryMark          mark;
};

typedef struct {
    uintptr_t mark;
} MemoryTail;

//■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■ ■■■

//!MemoryManagement Handle
define_handle {
    FILE*           log_stream;
    FILE*           err_stream;
    MemoryHeader*   lnk_header;
} MemoryManagement;

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■

//!MemoryManagement Private Methods

define_private_function(inline void, mem_init_tail)
(void* header, size_t user_size){
    void* tail = (char*)header + sizeof(MemoryHeader) + user_size;
    memset(tail, 0x99, sizeof(MemoryMark));
}

define_private_function(inline void, mem_init_data)
(MemoryHeader* header, const char* filename, size_t line, size_t user_size){
    header->size = user_size;
    header->line = line;
    header->name = filename;

    memset(&(header->mark), 0x66, sizeof(MemoryMark));
    call_private_function(mem_init_tail) (header, user_size);
}

//! interface
define_private_function(int, mem_check_mark)
(void* header) {
    int i;
    unsigned char* dat;
    dat = (unsigned char*)header + sizeof(MemoryHeader) - sizeof(MemoryMark);
    for(i=0; i<sizeof(MemoryMark); i++) {
        if(dat[i] != 0x66) return -1;
    }
    dat = (unsigned char*)header + sizeof(MemoryHeader) + (((MemoryHeader*)header)->size);
    /*for(i=0; i<sizeof(MemoryMark); i++) {
        printf("%x\n", dat[i]);
    }*/
    for(i=0; i<sizeof(MemoryMark); i++) {
        if(dat[i] != 0x99) return -2;
    }
    return 0;
}

define_private_function(void, mem_chain_a)
(MemoryManagement* handle, MemoryHeader* header) {
    if(handle->lnk_header) {
        handle->lnk_header->prev = header;
    }
    header->prev = NULL;
    header->next = handle->lnk_header;
    handle->lnk_header = header;
}

define_private_function(void, mem_chain_r)
(MemoryManagement* handle, MemoryHeader* header) {
    if (header->prev) {
        header->prev->next = header;
    } else {
        handle->lnk_header = header;
    }
    if (header->next) {
        header->next->prev = header;
    }
}

define_private_function(void, mem_chain_d)
(MemoryManagement* handle, MemoryHeader* header) {
    if (header->prev) {
        header->prev->next = header->next;
    } else {
        handle->lnk_header = header->next;
    }
    if (header->next) {
        header->next->prev = header->prev;
    }
}

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■

//! log error
define_private_function(void, mem_log_err)
(MemoryManagement* handle, const char* filename, size_t line, const char* message) {
    fprintf(handle->err_stream,
        #ifdef __SYSENV32__
            "Error: %s in %s at %d.\n"
        #else
            "Error: %s in %s at %Id.\n"
        #endif // __SYSENV32__
        , message, filename, line);
}

//! log check mark
define_private_function(void, mem_log_check_mark)
(MemoryManagement* handle, MemoryHeader* header) {
    static const char* err_msg[] = {"Overflow at tail", "Overflow at head"};
    int result = call_private_function(mem_check_mark)(header);
    if( 0!=result ) {
        call_private_function(mem_log_err)
            (handle, (header)->name, (header)->line, err_msg[result+2]);
    }
}

//! print header
define_private_function(void, mem_print_header)
(MemoryManagement* handle, MemoryHeader* header) {
    fprintf(handle->log_stream,
        #ifdef __SYSENV32__
            "%s at %d, Size=%d\n"
        #else
            "%s at %Id, Size=%Id\n"
        #endif // __SYSENV32__
        ,
        header->name,
        header->line,
        header->size
    );
}

//! print data
define_private_function(void, mem_print_data)
(MemoryManagement* handle, MemoryHeader* header) {
    unsigned char* dat = (unsigned char*)header + sizeof(MemoryHeader);
    size_t i = 0;
    for (;; i++) {
        if(i < header->size) {
            if (i % 8 == 0) {
                fprintf(handle->log_stream,
                #ifdef __SYSENV32__
                    "\n\t [%04d] "
                #else
                    "\n\t [%04Id] "
                #endif // __SYSENV32__
                , (i/8));
            }
            fprintf(handle->log_stream, "%02x ", dat[i]);
        } else {
            fprintf(handle->log_stream, "\n\n");
            break;
        }
    }
}

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■

//!MemoryManagement Interface Methods
#define handle ( (MemoryManagement*)__H )

//! init
define_private_function(void, setstream)
(handle_t __H, FILE* log, FILE* err) {
    handle->log_stream = log;
    handle->err_stream = err;
    //handle->lnk_header = NULL;
}

//! allocate
define_private_function(void*, allocate)
(handle_t __H, const char* filename, size_t line, size_t user_size) {
    size_t alloc_size = sizeof(MemoryHeader) + user_size + sizeof(MemoryTail);
    //Allocate
    void* ptr = malloc(alloc_size);
    if (NULL==ptr) {
        call_private_function(mem_log_err) (handle, filename, line, "malloc");
    }
    memset((char*)ptr + sizeof(MemoryHeader), 0xCC, user_size);
    MemoryHeader* header = (MemoryHeader*)ptr;
    call_private_function(mem_init_data) (header, filename, line, user_size);
    call_private_function(mem_chain_a) (handle, header);
    return (char*)ptr + sizeof(MemoryHeader);
}

//! reallocate
define_private_function(void*, reallocate)
(handle_t __H, const char* filename, size_t line, void* his_ptr_usr, size_t user_size) {

    size_t alloc_size = sizeof(MemoryHeader) + user_size + sizeof(MemoryTail);

    void*        his_ptr;
    MemoryHeader his_tmp;
    if(NULL==his_ptr_usr) {
        his_ptr = NULL;
        his_tmp.size = 0;
    } else {
        his_ptr = (char*)his_ptr_usr - sizeof(MemoryHeader);
        his_tmp = *((MemoryHeader*)his_ptr);
        call_private_function(mem_log_check_mark) (handle, (MemoryHeader*)his_ptr);
        call_private_function(mem_chain_d) (handle, (MemoryHeader*)his_ptr);
    }

    //ReAllocate
    void* new_ptr = realloc(his_ptr, alloc_size);
    if(NULL==new_ptr) {
        if(NULL==his_ptr) {
            call_private_function(mem_log_err) (handle, filename, line, "realloc(malloc)");
        } else {
            call_private_function(mem_log_err) (handle, filename, line, "realloc");
            free(his_ptr);
        }
        return NULL;
    }
    #ifdef __debug
        printf("[debug] pointer is old : %s\n", (his_ptr==new_ptr)?"True":"False");
    #endif // __debug

    //Final
    MemoryHeader* header = (MemoryHeader*)new_ptr;
    if(NULL==his_ptr) {
        call_private_function(mem_init_data) (header, filename, line, user_size);
        call_private_function(mem_chain_a) (handle, header);
    } else {
        *header = his_tmp;
        header->size = user_size;
        call_private_function(mem_chain_r)   (handle, header);
        call_private_function(mem_init_tail) (header, user_size);
    }

    //Format
    if (user_size > his_tmp.size) {
        memset(
            (char*)new_ptr + sizeof(MemoryHeader) + his_tmp.size,
            0xCC,
            (user_size - his_tmp.size)
        );
    }

    return new_ptr + sizeof(MemoryHeader);
}

//! free
define_private_function(void, free)
(handle_t __H, void* ptr) {
    if(NULL==ptr) return;
    MemoryHeader* header = (MemoryHeader*)( (char*)ptr - sizeof(MemoryHeader) );
    #ifdef __debug
        printf("[debug]free: %Id\n", header->size);
    #endif // __debug
    call_private_function(mem_log_check_mark)(handle, header);
    call_private_function(mem_chain_d)(handle, header);
    free(header);
}

//! check all
define_private_function(void, check_all)
(handle_t __H) {
    MemoryHeader* it_header = handle->lnk_header;
    while(it_header) {
        call_private_function(mem_log_check_mark)(handle, it_header);
        it_header = it_header->next;
    }
}

//! print block
define_private_function(void, print_block)
(handle_t __H, const char* filename, size_t line, void* ptr) {
    MemoryHeader* header = (MemoryHeader*)( (char*)ptr - sizeof(MemoryHeader) );
    fprintf(handle->log_stream,
        #ifdef __SYSENV32__
            "%s at %d\n"
        #else
            "%s at %Id\n"
        #endif // __SYSENV32__
        , filename, line
    );
    call_private_function(mem_print_header)(handle, header);
    call_private_function(mem_log_check_mark)(handle, header);
    call_private_function(mem_print_data)(handle, header);
}

//! dump all
define_private_function(void, dump)
(handle_t __H) {

    int index = 0;
    MemoryHeader* it_header = handle->lnk_header;
    while(it_header) {
        fprintf(handle->log_stream, "[%04d] ", index);
        call_private_function(mem_print_header)(handle, it_header);
        call_private_function(mem_log_check_mark)(handle, it_header);
        call_private_function(mem_print_data)(handle, it_header);
        it_header = it_header->next; index++;
    }
    fprintf(handle->log_stream, "\n\n");
}

#undef handle

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■

implement_handle(MemoryManagement) = {NULL, NULL, NULL};

implement_interface(MEMMNG_Methods) {
    call_private_function(setstream),
    call_private_function(allocate),
    call_private_function(reallocate),
    call_private_function(free),
    call_private_function(mem_check_mark),
    call_private_function(check_all),
    call_private_function(print_block),
    call_private_function(dump)
};

bind_expose(MEMMNG);

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
#ifdef __debug

typedef struct {
    union dummy {
        intptr_t    l;
        uintptr_t   u;
        double      d;
        void*       p;
    };
} Cell;
#define CellSize sizeof(Cell)

typedef struct __$MemoryPage MemoryPage;
struct __$MemoryPage {
	size_t              cell_use;   // 已使用的内存块个数
	size_t              cell_max;   // 内存块个数
	MemoryPage*         next;
	Cell                cell[1];    // （根据情况可以越界）
};

typedef struct {
    MemoryManagement*   managment;
	MemoryPage*         page_list;  //内存页面
	size_t              page_size;	//Current页面Cell大小 current_page_size
} MemoryStorage;

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■

define_private_function(handle_t, storage_open)
(handle_t __H, const char* filename, size_t line, size_t page_size) {

	MemoryStorage* storage =
        call_private_function(allocate) (__H, filename, line, sizeof(MemoryStorage));
    #ifdef __debug
        printf("[debug] Open Storage!\n");
    #endif // __debug
    storage->managment = (MemoryManagement*)__H;
    storage->page_list = NULL;
    storage->page_size = page_size;

	return storage; //__H
}

#define getmax(a, b)    ( (a > b) ? a : b )
define_private_function(void*, storage_allocate)
(handle_t __SH, const char* filename, size_t line, size_t size) {
    MemoryStorage* storage = (MemoryStorage*)__SH;
	void* ptr;

    // 计算所需的内存块个数
	size_t cell_need = 1 + ( (size - 1) / CellSize );

	if(
        storage->page_list != NULL &&
        (storage->page_list->cell_use + cell_need) < (storage->page_list->cell_max)
    ) {
		// 存储器的内存页中还有空闲的内存块
		ptr = &(storage->page_list->cell[storage->page_list->cell_use]);
		((storage->page_list)->cell_use) += cell_need;
	} else {
		// 存储器中没有空闲的内存块
		size_t alloc_cell_num = getmax(cell_need, storage->page_size);
		size_t alloc_byte_num = sizeof(MemoryPage) + CellSize * (alloc_cell_num - 1); //自带一个Cell
		#ifdef __debug
            printf("alloc cell=%Id, byte=%Id\n", alloc_cell_num, alloc_byte_num);
		#endif // __debug
        MemoryPage* new_page = (MemoryPage*)
            call_private_function(allocate) (storage->managment, filename, line, alloc_byte_num);

        //插入链表
		new_page->next = storage->page_list;
		new_page->cell_max = alloc_cell_num;
		storage->page_list = new_page;

		ptr = &(new_page->cell[0]);
		new_page->cell_use = cell_need;
	}
	return ptr;
}

define_private_function(void, storage_close)
(handle_t __SH) {
    MemoryStorage* storage = (MemoryStorage*)__SH;
	MemoryPage* temp;
	while (storage->page_list) {
		temp = storage->page_list->next;
		call_private_function(free)(storage->managment, storage->page_list);
		storage->page_list = temp;
	}
	call_private_function(free)(storage->managment, storage);
    #ifdef __debug
        printf("[debug] Close Storage!\n");
    #endif // __debug
}

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■

//implement_interface
static MEMMNG_Storage_Methods __Living_Methods2 = {
    call_private_function(storage_open),
    call_private_function(storage_allocate),
    call_private_function(storage_close)
};

//bind_interface
handle_t MEMMNG_Storage = &__Living_Methods2;

#endif // __debug
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
