#include "types.h"
#include "vid.h"

volatile u32 *fb;


u32 fbuf_init()
{
    fb = (u32 *)(0x200000); //frame buffer starts at 2M
    /* for 640x480 VGA */
    *(volatile u32 *)(0x1000001c) = 0x00002C77;
    *(volatile u32 *)(0x10120000) = 0x3F1F3F9C;
    *(volatile u32 *)(0x10120004) = 0x090B61DF;
    *(volatile u32 *)(0x10120008) = 0x067F1800;

    /* for 800x600 SVGA */
    // *(volatile u32 *)(0x1000001c) = 0x00002CAC;
    // *(volatile u32 *)(0x10120000) = 0x1313A4C4;
    // *(volatile u32 *)(0x10120004) = 0x0505F6F7;
    // *(volatile u32 *)(0x10120008) = 0x071F1800;

    *(volatile u32 *)(0x10120010) = 0x200000; //fbuf
    *(volatile u32 *)(0x10120018) = 0x82B;
}

