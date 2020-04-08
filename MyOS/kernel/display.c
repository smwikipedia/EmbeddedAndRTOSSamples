#include "types.h"
#include "display.h"


volatile u32 *fb;
DisplayContext gDisplayContext = {0};

dchar_ptr dcharX;
dchar_ptr undcharX;


u32 fbuf_init()
{
    fb = (u32 *)(0x200000); //frame buffer starts at 2M, decided by the board spec

    InitializeFontContext12x16();

    /* for 640x480 VGA */
    // *(volatile u32 *)(0x1000001c) = 0x00002C77;
    // *(volatile u32 *)(0x10120000) = 0x3F1F3F9C;
    // *(volatile u32 *)(0x10120004) = 0x090B61DF;
    // *(volatile u32 *)(0x10120008) = 0x067F1800;
    // gDisplayContext.screen_width = 640;
    // gDisplayContext.screen_height = 480;

    /* for 800x600 SVGA */
    *(volatile u32 *)(0x1000001c) = 0x00002CAC;
    *(volatile u32 *)(0x10120000) = 0x1313A4C4;
    *(volatile u32 *)(0x10120004) = 0x0505F6F7;
    *(volatile u32 *)(0x10120008) = 0x071F1800;
    gDisplayContext.screen_width = 800;
    gDisplayContext.screen_height = 600;


    gDisplayContext.cursor_row = 1;
    gDisplayContext.cursor_col = 1;
    gDisplayContext.max_col = gDisplayContext.screen_width / (gDisplayContext.font_display_width + gDisplayContext.h_font_space);
    gDisplayContext.max_row = gDisplayContext.screen_height / (gDisplayContext.font_display_height + gDisplayContext.v_font_space);

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
            pixel = (b<<16)|(g<<8)|r; // 24-bit color BMP
            fb[i*gDisplayContext.screen_width + j]=pixel; // in the frame buffer, a pixel occupies 32-bit.
            pp +=3; //advance pp to next pixel
        }
        p -=rsize; // to preceding row
    }
}


/*
Text Mode APIs in terms of column and row.
Here are the common APIs regardless of font bitmap styles.
The font-specific APIs live in their respective files, such as char12x16.c
*/

void clrpix(u32 x, u32 y) // clear pixel at screen pixel location (x, y)
{
    u32 pix = y*gDisplayContext.screen_width + x;
    fb[pix] = 0x00000000;
}

void setpix(u32 x, u32 y) // set pixel at screen pixel location (x, y)
{
    u32 pix = y*gDisplayContext.screen_width + x;
    fb[pix] = 0x0000FF00; // GREEN
}


void scrollup() // scroll UP one row (the hard way)
{
    if(gDisplayContext.cursor_row == 1) return;

    u32 font_block_height = gDisplayContext.font_display_height + gDisplayContext.v_font_space;
    u32 row_block_size = gDisplayContext.screen_width * font_block_height;
    u32 end = (gDisplayContext.max_row - 1) * row_block_size;

    //move all rows one row up.
    for(u32 i=0; i<=end; i++)
    {
        fb[i]=fb[i + gDisplayContext.screen_width * font_block_height];
    }

    //clear the last row
    for(u32 i=end+1; i<= end + row_block_size; i++)
    {
        fb[i]=0x00000000;
    }    
}

void scrolldown() // scroll DOWN one row (the hard way)
{

}


void kpchar(u8 c, u32 row, u32 col) // print char at screen char location (row, col)
{
    u32 x, y;
    x = (col-1) * (gDisplayContext.font_display_width + gDisplayContext.h_font_space);
    y = (row-1) * (gDisplayContext.font_display_height + gDisplayContext.v_font_space);
    dcharX(c, x, y);
}

void unkpchar(u8 c, u32 row, u32 col) // erase char c at screen char location (row, col)
{
    u32 x, y;
    x = (col-1) * (gDisplayContext.font_display_width + gDisplayContext.h_font_space);
    y = (row-1) * (gDisplayContext.font_display_height + gDisplayContext.v_font_space);
    undcharX(c, x, y);    
}

void erasechar(u32 row, u32 col) // erase any char at screen char location (row, col)
{
    u32 x, y;
    x = (col-1) * (gDisplayContext.font_display_width + gDisplayContext.h_font_space);
    y = (row-1) * (gDisplayContext.font_display_height + gDisplayContext.v_font_space);

    for(u32 i=0; i<gDisplayContext.font_display_height; i++) // 16
    {
        for(u32 j=0;j<gDisplayContext.font_display_width; j++) // 10
        {
            u32 xx = x + j;
            u32 yy = y + i;
            clrpix(xx, yy);
        }
    }      
}

void clrcursor() // clear cursor at current (row, col)
{
    erasechar(gDisplayContext.cursor_row, gDisplayContext.cursor_col);
}

/*
Print char at current cursor location.
And then move cursor to the next available location.
The cursor movement can depends on 'c' for '\n', 'r', and '\b'
The cursor always stays at a printable location.
*/
void putcursor(u8 c) // put cursor (row, col)
{
    kpchar(c, gDisplayContext.cursor_row, gDisplayContext.cursor_col);

    if(gDisplayContext.cursor_col == gDisplayContext.max_col)
    {
        gDisplayContext.cursor_col = 1;
        if(gDisplayContext.cursor_row == gDisplayContext.max_row)
        {
            scrollup();            
        }
        else
        {
            gDisplayContext.cursor_row++;
        }
    }
    else
    {
        gDisplayContext.cursor_col++;
    }
}


