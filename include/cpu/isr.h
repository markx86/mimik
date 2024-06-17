#ifndef MIMIK_ISR_H
#define MIMIK_ISR_H

struct isr_frame;

#include <types.h>
#include <cpu/isr-arch.h>

typedef status_t (*isr_t)(struct isr_frame*);

void isr_init(void);
void isr_register(int irq, isr_t handler, isr_t* old_handler);

#endif
