#ifndef VID_H
#define VID_H
#include "types.h"
#define WIDTH 640 // default to VGA mode
extern volatile u32 *fb;

u32 fbuf_init();

#endif