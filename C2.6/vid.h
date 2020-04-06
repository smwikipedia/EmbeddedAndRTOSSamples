#ifndef VID_H
#define VID_H
#include "types.h"
#define WIDTH 640 // default to VGA mode
extern volatile u32 *fb;

/* 
Use the nm on the image.o to check the actual symobl names exported.
If you are converting a "image.bmp" to "image.o", the symbol name will be "_binary_image_bmp_start".
Note that the file suffix will also be part of the exported symbol name.
*/
extern char _binary_image_bmp_start;

u32 fbuf_init();
void show_bmp(u8 *p, u32 start_row, u32 start_col);

#endif