#include <stdio.h>
#include <stdlib.h>
#include "MM.h"
void dump_buffer(unsigned char *buf, int size) {
    int i;

    for (i = 0; i < size; i++) {
        printf("%02x ", buf[i]);
        if (i % 16 == 15) {
            printf("\n");
        }
    }
    printf("\n");
}

void fill_buffer(unsigned char *buf, int size) {
    int i;

    for (i = 0; i < size; i++) {
        buf[i] = i;
    }
}

int main() {
    MEM_Init();
    //Dump
    printf("\n\nDump:\n");
    unsigned char* p1 = MEM_Allocate(10); fill_buffer(p1, 10);
    unsigned char* p2 = MEM_Allocate(8); fill_buffer(p2, 9);
    unsigned char* p3 = MEM_Allocate(16); fill_buffer(p3, 5);
    MEM_Dump();

    //Reallocate
    printf("\n\nReallocate:\n");
    MEM_Reallocate(p1, 6);
    //MEMMNG_M_Reallocate(p2, 10);
    MEM_Reallocate(p3, 20);
    MEM_Dump();

    dump_buffer(p1, 6);
    dump_buffer(p2, 8);
    dump_buffer(p3, 20);

    //Free
    printf("\n\nFree:\n");
    MEM_Free(p1);
    MEM_Free(p2);
    MEM_Free(p3);
    MEM_Dump();

    #ifdef __debug
    //Storage
    handle_t hs = MEM_S_Open(64);
    unsigned char* p4 = MEM_S_Allocate(hs, 50);
    fill_buffer(p4, 50);
    unsigned char* p5 = MEM_S_Allocate(hs, 56);
    fill_buffer(p5, 56);
    unsigned char* p6 = MEM_S_Allocate(hs, 13);
    fill_buffer(p6, 13);
    MEM_Dump();
    MEM_S_Close(hs);
    #endif // __debug

    return 0;
}
