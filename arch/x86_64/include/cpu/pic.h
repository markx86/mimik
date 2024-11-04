#ifndef MIMIK_PIC_H
#define MIMIK_PIC_H

#include <boot/bootinfo.h>

void pic_ack(uint8_t irq);
void pic_mask(uint8_t irq);
void pic_unmask(uint8_t irq);
status_t pic_init(void);

#endif
