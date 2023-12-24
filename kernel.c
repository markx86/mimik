#include <kernel.h>
#include <mm/pm.h>
#include <mm/vm.h>

struct kernel_config kcfg;

static void
parse_bootinfo(struct bootinfo* bootinfo, addr_t* free_mem_ptr) {
  kcfg.bootinfo = bootinfo;

  /* Parse cmdline. */
  for (char* s = bootinfo->cmdline; *s != '\0'; s++) {
  }

  struct bootinfo_module* module;
  /* Find modules. */
  for (module = bootinfo->modules; module != NULL; module = module->next) {
    if (module->end_address > *free_mem_ptr) {
      *free_mem_ptr = module->end_address;
    }
  }
}

void
kernel_main(
    struct bootinfo* bootinfo,
    addr_t bootinfo_data_start,
    addr_t bootinfo_data_end) {
  addr_t free_mem_ptr = bootinfo_data_end;
  parse_bootinfo(bootinfo, &free_mem_ptr);
  pm_init(free_mem_ptr);
  vm_init();
}
