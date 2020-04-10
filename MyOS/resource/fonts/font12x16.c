#include "types.h"
#include "display.h"

extern char _font12x16_start;


void InitializeFontContext12x16()
{
    gDisplayContext.font_bitmap_width = 8;
    gDisplayContext.font_bitmap_height = 24;    
    gDisplayContext.font_bitmap_size = 24; // in bytes
    
    gDisplayContext.font_display_width = 12; // the display size of a font block
    gDisplayContext.font_display_height = 16;

    gDisplayContext.v_font_space = 0;
    gDisplayContext.h_font_space = 0;

    dcharX = dchar12x16;
    undcharX = undchar12x16;
}


/*
Only for 12*16 font
*/
void dchar12x16(u8 c, u32 x, u32 y) // display a char at scree pixel location (x, y)
{
    u8* p = (u8*)(&_font12x16_start + (c - 32) * gDisplayContext.font_bitmap_size);
    u32 half = 0;
    for(u32 i=0; i<gDisplayContext.font_bitmap_height; i++) // 24
    {
        half = (i<12)? 1 : 2; // 1 for upper half, 2 for lower half. this depends on the font bitmap implementation        
        u32 ii = (i<12)?i:(i-12);
        for(u32 j=0;j<gDisplayContext.font_bitmap_width; j++) // 8
        {
            u32 xx = x + ii;
            u32 yy = y + half*8 - 1 - j;
            u8 *scan_line = p + i;
            u8 bit_test = (u8)1 << (7-j);
            if(*scan_line & bit_test)
            {
                setpix(xx, yy);
            }
            else
            {
                clrpix(xx, yy);
            }            
        }
    }
}

void undchar12x16(u8 c, u32 x, u32 y) // erase a char at scree location pixel location (x, y)
{
    u8* p = (u8*)(&_font12x16_start + (c - 32) * gDisplayContext.font_bitmap_size);
    u32 half = 0;
    for(u32 i=0; i<gDisplayContext.font_bitmap_height; i++) // 24
    {
        half = (i<12)? 1 : 2; // 1 for upper half, 2 for lower half. this depends on the font bitmap implementation        
        u32 ii = (i<12)?i:(i-12);
        for(u32 j=0;j<gDisplayContext.font_bitmap_width; j++) // 8
        {
            u32 xx = x + ii;
            u32 yy = y + half*8 - j;
            u8 *scan_line = p + i;
            u8 bit_test = (u8)1 << (7-j);
            if(*scan_line & bit_test)
            {
                clrpix(xx, yy);
            }
        }
    } 
}


