#include <stdio.h>
#include <stdlib.h>
#include "MM.h"

static void dumpBuffer(unsigned char *buf, size_t size) {
    size_t i;
    for (i = 0; i < size; i++) {
        printf("%02x ", buf[i]);
        if (i % 16 == 15) {
            printf("\n");
        }
    }
    printf("\n");
}

static void fillBuffer(unsigned char* buff, size_t size) {
    size_t i;
    for (i = 8; i < size; i++) {
        buff[i] = 1 + i;
    }
    for (i = 0; i < (size>8?8:size); i++) {
        buff[i] = 0xFF;
    }
}

int main() {

    MEM_setstream();
    //Check
    printf("\n\nDump:\n");
    unsigned char* p1 = MEM_malloc(10); fillBuffer(p1, 10);
    unsigned char* p2 = MEM_malloc(8); fillBuffer(p2, 9);
    unsigned char* p3 = MEM_malloc(16); fillBuffer(p3, 5);
    MEM_check_all_blocks();

    //Reallocate
    printf("\n\nReallocate:\n");
    p1 = MEM_realloc(p1, 6);
    //MEMMNG_M_Reallocate(p2, 10);
    p3 = MEM_realloc(p3, 20);
    MEM_dump_blocks();

    MEM_print_block(p1);
    dumpBuffer(p1, 6);
    dumpBuffer(p2, 8);
    dumpBuffer(p3, 20);

    //Free
    printf("\n\nFree:\n");
    MEM_free(p1);
    MEM_free(p2);
    MEM_free(p3);

    MEM_dump_blocks();

    #ifdef __debug
    //Storage

    handle_t hs = MEM_open_storage(30);
    unsigned char* p4 = MEM_storage_malloc(hs, 50);
    fillBuffer(p4, 50);
    unsigned char* p5 = MEM_storage_malloc(hs, 56);
    fillBuffer(p5, 56);
    unsigned char* p6 = MEM_storage_malloc(hs, 13);
    fillBuffer(p6, 13);
    MEM_dump_blocks();

    MEM_dispose_storage(hs);
    #endif // __debug

    printf("[END]\n");
    return 0;
}
