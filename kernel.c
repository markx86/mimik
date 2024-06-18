#include <kernel.h>
#include <mm/pm.h>
#include <mm/vm.h>
#include <mm/mm.h>
#include <cpu/int.h>
#include <cpu/isr.h>
#include <arch.h>
#include <assert.h>

struct kernel_config kcfg;

static void
parse_bootinfo(struct bootinfo* bootinfo) {
  char* s;
  struct bootinfo_module* module;

  kcfg.bootinfo = bootinfo;

  /* Parse cmdline. */
  for (s = bootinfo->cmdline; *s != '\0'; s++) {
  }

  /* Find modules. */
  for (module = bootinfo->modules; module != NULL; module = module->next) {
    kcfg.fs = initfs_from_module(module);
    /* TODO: support other modules */
    break;
  }
}

void remap_kernel(void) {
  status_t res;

  /* remap kernel entry (if present) as RW- */
  res = vm_kflag_range(KERNEL_VADDR_START, KERNEL_TEXT_START, VM_FLAG_READ | VM_FLAG_WRITABLE);
  ASSERT(res == SUCCESS);

  /* remap kernel text as R-X */
  res = vm_kflag_range(KERNEL_TEXT_START, KERNEL_TEXT_END, VM_FLAG_READ | VM_FLAG_EXECUTABLE);
  ASSERT(res == SUCCESS);

  /* remap kernel rodata as R-- */
  res = vm_kflag_range(KERNEL_RODATA_START, KERNEL_RODATA_END, VM_FLAG_READ);
  ASSERT(res == SUCCESS);

  /* remap kernel bss and data as RW- */
  res = vm_kflag_range(KERNEL_DATA_START, KERNEL_DATA_END, VM_FLAG_READ | VM_FLAG_WRITABLE);
  ASSERT(res == SUCCESS);
}

void
kernel_main(
    struct bootinfo* bootinfo,
    addr_t bootinfo_data_start,
    addr_t bootinfo_data_end) {
  pm_init();
  vm_init();
  mm_init();
  remap_kernel();
  isr_init();
  ASSERT(arch_init(bootinfo) == SUCCESS);
  /* TODO: initialize PIC and mask all interrupts */
  int_enable();
  parse_bootinfo(bootinfo);
  ASSERT(0 && "Hello from MIMIK!");
}
