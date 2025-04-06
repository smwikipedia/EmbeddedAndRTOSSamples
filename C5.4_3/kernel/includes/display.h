#ifndef DISPLAY_H
#define DISPLAY_H
#include "types.h"
#include "font12x16.h"

typedef
void (*dchar_ptr) (u8 c, u32 x, u32 y);

typedef struct
{
    u32 screen_width;
    u32 screen_height;
    
    u32 font_bitmap_width;
    u32 font_bitmap_height;
    u32 font_bitmap_size;

    u32 font_display_width;
    u32 font_display_height;
    u32 v_font_space; // vertical space between font 
    u32 h_font_space; // horizontal space between font
    
    u32 current_pixel_x; // current pixel postion x
    u32 current_pixel_y; // current pixel postion y
    u32 cursor_row; // current char postion row
    u32 cursor_col; // current char postion col

    u32 max_row;
    u32 max_col;

    u8 cursor;
}DisplayContext;

extern DisplayContext gDisplayContext;
extern dchar_ptr dcharX;
extern dchar_ptr undcharX;

extern volatile u32 *fb;

/* 
Use the nm on the image.o to check the actual symobl names exported.
If you are converting a "image.bmp" to "image.o", the symbol name will be "_binary_image_bmp_start".
Note that the file suffix will also be part of the exported symbol name.
*/
extern char _binary___resource_images_image_bmp_start;

void show_bmp(u8 *p, u32 start_row, u32 start_col);

void clrpix(u32 x, u32 y);
void setpix(u32 x, u32 y);

void kpchar(u8 c, u32 row, u32 col);
void unkpchar(u8 c, u32 row, u32 col);
void erasechar(u32 row, u32 col);
void scrollup();
void putcursor(u8 c);
void clrcursor();
void kputc(u8 c);
void kprintf(u8* fmt, ...);
void kprintf1(u8* fmt, u8 x1, u8* x2, i32 x3, i32 x4, i32 x5);

// //Display a char at row, col.
// void DisplayChar(u8 c, u32 row, u32 col);
// //Get current text mode screen resolution in row by col.
// void GetScreenResolutionTextMode(u32 *row, u32 *col);
// //Scroll up n rows.
// void ScrollUp(u32 n);







#endif