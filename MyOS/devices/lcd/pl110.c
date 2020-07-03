#include "types.h"
#include "display.h"
#include "pl110.h"

/*
LCD frame buffer init
*/
u32 fbuf_init(u32 PL110_LCD_BASE, u32 OSC)
{
    fb = (u32 *)(0x200000); //frame buffer starts at 2M, decided by the board spec

    InitializeFontContext12x16();

    /* for 640x480 VGA */
    // *(volatile u32 *)(VERSATILEPB_OSC1) = 0x00002C77;
    // *(volatile u32 *)(PL110_LCD_BASE + LCD_TIMING0) = 0x3F1F3F9C;
    // *(volatile u32 *)(PL110_LCD_BASE + LCD_TIMING1) = 0x090B61DF;
    // *(volatile u32 *)(PL110_LCD_BASE + LCD_TIMING2) = 0x067F1800;
    // gDisplayContext.screen_width = 640;
    // gDisplayContext.screen_height = 480;

    /* for 800x600 SVGA */
    *(volatile u32 *)(OSC) = 0x00002CAC;
    *(volatile u32 *)(PL110_LCD_BASE + LCD_TIMING0) = 0x1313A4C4;
    *(volatile u32 *)(PL110_LCD_BASE + LCD_TIMING1) = 0x0505F6F7;
    *(volatile u32 *)(PL110_LCD_BASE + LCD_TIMING2) = 0x071F1800;
    gDisplayContext.screen_width = 800;
    gDisplayContext.screen_height = 600;


    gDisplayContext.cursor_row = 1;
    gDisplayContext.cursor_col = 1;
    gDisplayContext.max_col = gDisplayContext.screen_width / (gDisplayContext.font_display_width + gDisplayContext.h_font_space);
    gDisplayContext.max_row = gDisplayContext.screen_height / (gDisplayContext.font_display_height + gDisplayContext.v_font_space);
    gDisplayContext.cursor = '_';

    *(volatile u32 *)(PL110_LCD_BASE + LCD_UPBASE) = 0x200000; //fbuf, should be consistent with the globl variable "fb"
    *(volatile u32 *)(PL110_LCD_BASE + LCD_IMSC) = 0x82B;
}