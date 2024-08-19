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
kernel_parse_bootinfo(struct bootinfo* bootinfo) {
  char* s;
  struct bootinfo_module* module;

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

static void
kernel_remap(void) {
  status_t res;

  /* remap kernel entry (if present) as RW- */
  res = vmk_flag_range(KERNEL_VADDR_START, KERNEL_TEXT_START, VM_FLAG_READ | VM_FLAG_WRITABLE);
  ASSERT(res == SUCCESS);

  /* remap kernel text as R-X */
  res = vmk_flag_range(KERNEL_TEXT_START, KERNEL_TEXT_END, VM_FLAG_READ | VM_FLAG_EXECUTABLE);
  ASSERT(res == SUCCESS);

  /* remap kernel rodata as R-- */
  res = vmk_flag_range(KERNEL_RODATA_START, KERNEL_RODATA_END, VM_FLAG_READ);
  ASSERT(res == SUCCESS);

  /* remap kernel bss and data as RW- */
  res = vmk_flag_range(KERNEL_DATA_START, KERNEL_DATA_END, VM_FLAG_READ | VM_FLAG_WRITABLE);
  ASSERT(res == SUCCESS);
}

void
kernel_main(
    struct bootinfo* bootinfo,
    addr_t bootinfo_data_start,
    addr_t bootinfo_data_end) {
  kcfg.bootinfo = bootinfo;
  pm_init();
  vm_init();
  mm_init();
  kernel_remap();
  isr_init();
  ASSERT(arch_init(bootinfo) == SUCCESS);
  int_enable();
  kernel_parse_bootinfo(bootinfo);
  ASSERT(0 && "Hello from MIMIK!");
}
