#include <cpu/pic.h>
#include <cpu/io.h>
#include <cpu/cpuid.h>
#include <assert.h>

/* 8259 PIC defines */
#define PIC1 0x20
#define PIC2 0xa0

#define PIC1_COMM (PIC1 + 0)
#define PIC2_COMM (PIC2 + 0)
#define PIC1_DATA (PIC1 + 1)
#define PIC2_DATA (PIC1 + 1)

#define PIC_EOI     0x20
#define PIC_DISABLE 0xff

#define PIC_ICW1_ICW4      0x01
#define PIC_ICW1_SINGLE    0x02
#define PIC_ICW1_INTERVAL4 0x04
#define PIC_ICW1_LEVEL     0x08
#define PIC_ICW1_INIT      0x10

#define PIC_ICW4_8086      0x01
#define PIC_ICW4_AUTO      0x02
#define PIC_ICW4_BUFSLAVE  0x08
#define PIC_ICW4_BUFMASTER 0x0c
#define PIC_ICW4_SFNM      0x10

static bool_t use_new_pic;

bool_t
has_apic(struct bootinfo_arch* bootinfo) {
  struct cpuid_regs regs;
  cpuid(CPUID_STDFN_00000001, &regs);
  return (regs.edx & (1 << 9)) == 0;
}

status_t
new_pic_init(struct bootinfo_arch* bootinfo) {
  /* disable 8259 PICs by masking all interrupts */
  io_outb(PIC1_DATA, PIC_DISABLE);
  io_outb(PIC2_DATA, PIC_DISABLE);
  /* TODO: init APIC */
  TODO("apic initialization");
  return SUCCESS;
}

static void
old_pic_remap(uint8_t off1, uint8_t off2) {
  uint8_t pic1_mask, pic2_mask;

  /* save old interrupt masks */
  pic1_mask = io_inb(PIC1_DATA);
  pic2_mask = io_inb(PIC2_DATA);

  io_outb(PIC1_COMM, PIC_ICW1_INIT | PIC_ICW1_ICW4);
  IOWAIT();
  io_outb(PIC2_COMM, PIC_ICW1_INIT | PIC_ICW1_ICW4);
  IOWAIT();

  io_outb(PIC1_DATA, off1);
  IOWAIT();
  io_outb(PIC2_DATA, off2);
  IOWAIT();

  io_outb(PIC1_DATA, 1 << 2);
  IOWAIT();
  io_outb(PIC2_DATA, 1 << 1);
  IOWAIT();

  io_outb(PIC1_DATA, PIC_ICW4_8086);
  IOWAIT();
  io_outb(PIC2_DATA, PIC_ICW4_8086);
  IOWAIT();

  /* restore old interrupt masks */
  io_outb(PIC1_DATA, pic1_mask);
  io_outb(PIC2_DATA, pic2_mask);
}

static void
old_pic_mask(uint8_t irq) {
  uint8_t value, port = irq < 8 ? PIC1_DATA : PIC2_DATA;
  irq &= 7;
  value = io_inb(port) | (uint8_t)(1 << irq);
  io_outb(port, value);
}

static void
old_pic_unmask(uint8_t irq) {
  uint8_t value, port = irq < 8 ? PIC1_DATA : PIC2_DATA;
  irq &= 7;
  value = io_inb(port) & (uint8_t) ~(1 << irq);
  io_outb(port, value);
}

static void
old_pic_ack(uint8_t irq) {
  if (irq >= 8)
    io_outb(PIC2_COMM, PIC_EOI);
  io_outb(PIC1_COMM, PIC_EOI);
}

static status_t
old_pic_init(void) {
  old_pic_remap(0x10, 0x18);
  /* mask all interrupts */
  io_outb(PIC1_DATA, PIC_DISABLE);
  io_outb(PIC2_DATA, PIC_DISABLE);
  return SUCCESS;
}

void
pic_ack(uint8_t irq) {
  if (use_new_pic)
    TODO();
  else
    old_pic_ack(irq);
}

void
pic_mask(uint8_t irq) {
  if (use_new_pic)
    TODO();
  else
    old_pic_mask(irq);
}

void
pic_unmask(uint8_t irq) {
  if (use_new_pic)
    TODO();
  else
    old_pic_unmask(irq);
}

status_t
pic_init(struct bootinfo_arch* bootinfo) {
  use_new_pic = has_apic(bootinfo);
  return use_new_pic ? new_pic_init(bootinfo) : old_pic_init();
}
