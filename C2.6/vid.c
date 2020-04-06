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

/*
start_row and start_col is the screen postion to display the image.
*/
void show_bmp(u8 *p, u32 start_row, u32 start_col)
{
    u32 h, w, pixel, rsize, i, j;
    u8 r, g, b;
    u8* pp;
    u32 *q =(u32 *)(p+14); //skip over 14-byte file header
    w = *(q+1); // how many cols in pixel
    h = *(q+2); // how may rows in pixel
    p += 54; // p points to pixels in image

    //BMP image is upside down, each row is a multiple of 4 bytes
    rsize = 4*((3*w+3)/4); //multiple of 4
    p += (h-1) * rsize; // last row of pixels is the fist row of the image.
    for(i = start_row; i<start_row + h; i++)
    {
        pp=p;
        for(j=start_col;j<start_col+w;j++)
        {
            b=*pp;
            g=*(pp+1);
            r=*(pp+2);
            pixel = (b<<16)|(g<<8)|r;
            fb[i*WIDTH + j]=pixel;
            pp +=3; //advance pp to next pixel
        }
        p -=rsize; // to preceding row
    }
}

