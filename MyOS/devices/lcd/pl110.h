#ifndef PL110_LCD_H
#define PL110_LCD_H

#include "types.h"
#include "versatilepb.h"

//#define PL110_LCD_BASE VERSATILEPB_PL110_LCD_BASE

#define LCD_TIMING0 0x0
#define LCD_TIMING1 0x4
#define LCD_TIMING2 0x8
#define LCD_TIMING3 0xC
#define LCD_UPBASE 0x10
#define LCD_IMSC 0x18


u32 fbuf_init(u32 PL110_LCD_BASE, u32 OSC);

#endif