#include "MM.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

define_private_function(void, mem_log_err)
(MemoryManagement* handle, const char* filename, int line, const char* message) {
    fprintf(handle->err_stream, "Memory:%s failed in %s at %d.\n", message, filename, line);
}

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

define_private_function(int, mem_check_mark)
(MemoryHeader* header) {
    int i;
    unsigned char* dat;

    dat = (unsigned char*)header + sizeof(MemoryHeader) - sizeof(MemoryMark);
    for(i=0; i<sizeof(MemoryMark); i++) {
        if(dat[i] != 0x66) return -1;
    }

    dat = (unsigned char*)header + sizeof(MemoryHeader) + (header->size);
    for(i=0; i<sizeof(MemoryMark); i++) {
        if(dat[i] != 0x99) return -2;
    }

    return 0;
}

define_private_function(void, mem_print_check_mark)
(MemoryManagement* handle, MemoryHeader* header) {
    int r = call_private_function(mem_check_mark)(header);
    if(r < 0) {
        fprintf(handle->err_stream,
            "%s Line=%Id Size=%Id\n\tOverflow=%d\n",
                header->name,
                header->line,
                header->size,
                r
        );
    }
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

define_private_function(void, mem_print_data)
(MemoryManagement* handle, MemoryHeader* header) {
    unsigned char* dat = (unsigned char*)header + sizeof(MemoryHeader);
    int i = 0;
    for (;; i++) {
        if(i < header->size) {
            if (i % 8 == 0) {
                fprintf(handle->log_stream, "\n\t");
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
define_private_function(void, init)
(handle_t __H, FILE* log, FILE* err) {
    handle->log_stream = log;
    handle->err_stream = err;
    handle->lnk_header = NULL;
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

    void*  his_ptr;
    MemoryHeader his_header;
    //his_header.size

    if(his_ptr_usr) {
        his_ptr = (char*)his_ptr_usr - sizeof(MemoryHeader);
        call_private_function(mem_print_check_mark) (handle, (MemoryHeader*)his_ptr);
        his_header = *((MemoryHeader*)his_ptr);
        call_private_function(mem_chain_d) (handle, (MemoryHeader*)his_ptr);
    } else {
        his_ptr = NULL;
        his_header.size = 0;
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
    }
    //Final
    MemoryHeader* header = (MemoryHeader*)new_ptr;
    if(NULL==his_ptr) {
        call_private_function(mem_init_data) (header, filename, line, user_size);
        call_private_function(mem_chain_a) (handle, header);
    } else {
        *header = his_header;
        header->size = user_size;
        call_private_function(mem_chain_r)   (handle, header);
        call_private_function(mem_init_tail) (header, user_size);
    }
    //Format
    if (user_size > his_header.size) {
        memset((char*)new_ptr + sizeof(MemoryHeader) + his_header.size,
               0xCC,
               (user_size - his_header.size));
    }
    return new_ptr;
}

//! free
define_private_function(void, free)
(handle_t __H, void* ptr) {
    if(NULL==ptr) return;
    MemoryHeader* header = (MemoryHeader*)( (char*)ptr - sizeof(MemoryHeader) );
    call_private_function(mem_print_check_mark)(handle, header);
    call_private_function(mem_chain_d)(handle, header);
    free(header);
}

//! dump
define_private_function(void, dump)
(handle_t __H) {
    size_t index = 0;
    MemoryHeader* it_header = handle->lnk_header;
    while(it_header) {
        fprintf(handle->log_stream, "--------------------------\n");
        //Print Message
        fprintf(handle->log_stream,
            "[%04Id] %s\n\tLine=%Id, Size=%Id, Overflow=%d\n",
                index,
                it_header->name,
                it_header->line,
                it_header->size,
                call_private_function(mem_check_mark)(it_header)
        );
        //Print Data
        call_private_function(mem_print_data)(handle, it_header);
        it_header = it_header->next; index++;
    }
    fprintf(handle->log_stream, "--------------------------\n\n");
}

#undef handle

//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■
//■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■ ■■■■■■■■

implement_handle(MemoryManagement) = {NULL, NULL, NULL};

implement_interface(MEMMNG_Methods) {
    call_private_function(init),
    call_private_function(allocate),
    call_private_function(reallocate),
    call_private_function(free),
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
        MemoryPage* new_page = (MemoryPage*)
            call_private_function(allocate) (storage->managment, filename, line,
                sizeof(MemoryPage) + CellSize * (alloc_cell_num - 1) //因为自带一个
            );
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
