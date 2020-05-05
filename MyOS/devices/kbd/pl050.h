#ifndef KMI_PL050_H
#define KMI_PL050_H

/******** KBD register byte offsets; for char *base *****/
#define KCNTL 0x00 // 7-6- 5(0=AT)4=RxIntEn 3=TxIntEn
#define KSTAT 0x04 // 7-6=TxE 5=TxBusy 4=RXFull 3=RxBusy
#define KDATA 0x08 // data register;
#define KCLK 0x0C  // clock divisor register; (not used)
#define KISTA 0x10 // interrupt status register;(not used)

#define MAX_KBD_CHAR_BUFFER_SIZE 128

typedef volatile struct kbd
{                               // base = 0x10006000
    u8 *base;                 // base address of KBD, as char *
    u8 buf[MAX_KBD_CHAR_BUFFER_SIZE];              // input buffer
    u32 head, tail, data, room; // control variables    
} KBD;

extern volatile KBD kbd; // KBD data structure

void kbd_init();
void kbd_handler();
int kgets(char s[]);

#endif