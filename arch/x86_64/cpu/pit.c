#include <cpu/pit.h>
#include <cpu/io.h>
#include <cpu/isr.h>
#include <cpu/pic.h>
#include <assert.h>

#define PIT_CH0 0x40
#define PIT_CH1 0x41
#define PIT_CH2 0x42
#define PIT_CMD 0x43

#define PIT_COUNTER_BINARY 0
#define PIT_COUNTER_BCD    1

#define PIT_MODE_TERMCNT  (0 << 1)
#define PIT_MODE_ONESHOT  (1 << 1)
#define PIT_MODE_RATEGEN  (2 << 1)
#define PIT_MODE_SQRWAVE  (3 << 1)
#define PIT_MODE_SWSTROBE (4 << 1)
#define PIT_MODE_HWSTROBE (5 << 1)

#define PIT_ACCESS_LATCHCNT (1 << 4)
#define PIT_ACCESS_LOBYTE   (2 << 4)
#define PIT_ACCESS_HIBYTE   (3 << 4)
#define PIT_ACCESS_LOTHENHI (4 << 4)

#define PIT_CHANNEL_CH0 (0 << 6)
#define PIT_CHANNEL_CH1 (1 << 6)
#define PIT_CHANNEL_CH2 (2 << 6)

#define PIT_FREQ 1193182

#define PIT_IRQ 32

static status_t
pit_handler(struct isr_frame* isr_frame) {
  ASSERT(isr_frame->isr_n == PIT_IRQ);
  return SUCCESS;
}

void
pit_init(void) {
  io_outb(
      PIT_CMD,
      PIT_CHANNEL_CH0 | PIT_ACCESS_LOTHENHI | PIT_MODE_RATEGEN |
          PIT_COUNTER_BINARY);
  IOWAIT();
  isr_register(PIT_IRQ, &pit_handler, NULL);
  pit_set_period_ms(10);
  pic_unmask(PIT_IRQ);
}

void
pit_set_period_us(time_t us) {
  uint16_t div = (uint16_t)(PIT_FREQ / us);
  ASSERT(div > 1);
  io_outb(PIT_CHANNEL_CH0, div & 0xff);
  io_outb(PIT_CHANNEL_CH0, div >> 8);
}
