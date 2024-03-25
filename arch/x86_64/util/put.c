#include <log/put.h>
#include <cpu/io.h>
#include <util/compiler.h>
#include <types.h>

#define COM 0x03f8 /* COM1 port address */

#define RBR (COM + 0)
#define THR (COM + 0)
#define DLL (COM + 0)
#define IER (COM + 1)
#define DLM (COM + 1)
#define IIR (COM + 2)
#define FCR (COM + 2)
#define LCR (COM + 3)
#define MCR (COM + 4)
#define LSR (COM + 5)
#define MSR (COM + 6)
#define SCR (COM + 7)

#define BAUD_MAX 115200
#define BAUD_TGT 115200
#define DIVISOR (BAUD_MAX / BAUD_TGT)

enum data_bits {
  DATA_BITS_5 = 0b00,
  DATA_BITS_6 = 0b01,
  DATA_BITS_7 = 0b10,
  DATA_BITS_8 = 0b11
};

enum stop_bits {
  STOP_BITS_1 = 0b0,
  STOP_BITS_2 = 0b1
};

enum parity {
  PARITY_NONE = 0b000,
  PARITY_ODD = 0b001,
  PARITY_EVEN = 0b011,
  PARITY_HIGH = 0b101,
  PARITY_LOW = 0b111
};

union lcr {
  struct {
    uint8_t data_bits : 2;
    uint8_t stop_bit : 1;
    uint8_t parity : 3;
    uint8_t break_signal_enable : 1;
    uint8_t dlab : 1;
  };
  uint8_t raw;
};

enum fifo_dma_mode {
  FIFO_DMA_MODE_0 = 0,
  FIFO_DMA_MODE_1 = 1
};

enum fifo_size {
  FIFO_SIZE_1B = 0b00,
  FIFO_SIZE_4B = 0b01,
  FIFO_SIZE_8B = 0b10,
  FIFO_SIZE_14B = 0b11
};

union fcr {
  struct {
    uint8_t enable_fifo : 1;
    uint8_t clear_rx_fifo : 1;
    uint8_t clear_tx_fifo : 1;
    uint8_t fifo_dma_mode : 1;
    uint8_t reserved : 2;
    uint8_t fifo_size : 2;
  };
  uint8_t raw;
};

union lsr {
  struct {
    uint8_t data_available : 1;
    uint8_t overrun_error : 1;
    uint8_t parity_error : 1;
    uint8_t framing_error : 1;
    uint8_t break_signal_received : 1;
    uint8_t thr_is_empty : 1;
    uint8_t thr_is_empty_and_idle : 1;
    uint8_t erroneous_data_in_fifo : 1;
  };
  uint8_t raw;
};

static bool_t serial_initialized = FALSE;

static void
init_serial(void) {
  union lcr lcr;
  union fcr fcr;

  lcr.raw = io_inb(LCR);
  lcr.dlab = TRUE;
  io_outb(LCR, lcr.raw);

  /* set baud rate to BAUD_TGT */
  io_outb(DLL, DIVISOR & 0xff);
  io_outb(DLM, (DIVISOR >> 8) & 0xff);

  lcr.data_bits = DATA_BITS_8;
  lcr.stop_bit = STOP_BITS_1;
  lcr.parity = PARITY_EVEN;
  lcr.break_signal_enable = FALSE; /* break signal disabled */
  lcr.dlab = FALSE;                /* disable DLAB */
  io_outb(LCR, lcr.raw);

  fcr.raw = io_inb(FCR);
  fcr.enable_fifo = TRUE;
  fcr.clear_rx_fifo = TRUE;
  fcr.clear_tx_fifo = TRUE;
  fcr.fifo_dma_mode = FIFO_DMA_MODE_0;
  fcr.fifo_size = FIFO_SIZE_14B;
  io_outb(FCR, fcr.raw);

  serial_initialized = TRUE;
}

void
putc(char c) {
  union lsr lsr;
  if (unlikely(!serial_initialized))
    init_serial();
  do
    lsr.raw = io_inb(LSR);
  while (!lsr.thr_is_empty_and_idle);
  io_outb(THR, (uint8_t)c);
}
