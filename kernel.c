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
find_free_mem_ptr(struct bootinfo* bootinfo, addr_t* free_mem_ptr) {
  for (struct bootinfo_module* module = bootinfo->modules; module != NULL;
       module = module->next) {
    if (module->end_address > *free_mem_ptr)
      *free_mem_ptr = module->end_address;
  }
  LOGTRACE("free mem @ %p", *free_mem_ptr);
}

static void
parse_bootinfo(struct bootinfo* bootinfo) {
  kcfg.bootinfo = bootinfo;

  /* Parse cmdline. */
  for (char* s = bootinfo->cmdline; *s != '\0'; s++) {
  }

  /* Find modules. */
  for (struct bootinfo_module* module = bootinfo->modules; module != NULL;
       module = module->next) {
    kcfg.fs = initfs_from_module(module);
    /* TODO: support other modules */
    break;
  }
}

void
kernel_main(
    struct bootinfo* bootinfo,
    addr_t bootinfo_data_start,
    addr_t bootinfo_data_end) {
  addr_t free_mem_ptr = bootinfo_data_end;
  find_free_mem_ptr(bootinfo, &free_mem_ptr);
  pm_init(free_mem_ptr);
  vm_init();
  mm_init();
  ASSERT(arch_init(bootinfo) == SUCCESS);
  isr_init();
	/* TODO: initialize PIC and mask all interrupts */
  int_enable();
  parse_bootinfo(bootinfo);
  ASSERT(0 && "Hello from MIMIK!");
}
