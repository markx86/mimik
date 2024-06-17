#ifndef MIMIK_PIT_H
#define MIMIK_PIT_H

typedef unsigned long long time_t;

void pit_init(void);
void pit_set_period_us(time_t us);

static inline void
pit_set_period_ms(time_t ms) {
  pit_set_period_us(ms * 1000);
}

#endif
