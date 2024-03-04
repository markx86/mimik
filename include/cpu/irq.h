#ifndef MIMIK_IRQ_H
#define MIMIK_IRQ_H

enum irq_type {
	IRQ_TIMER
};

void irq_mask(enum irq_type type);

#endif
