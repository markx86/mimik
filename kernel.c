#include <kernel.h>
#include <mm/pm.h>
#include <mm/vm.h>
#include <mm/mm.h>
#include <mem/mem.h>
#include <mem/layout.h>
#include <cpu/int.h>
#include <cpu/isr.h>
#include <arch.h>
#include <assert.h>

struct kernel_singleton K;

static void
kernel_parse_bootinfo(void) {
  char* s;
  struct bootinfo_module* module;

  /* Parse cmdline. */
  for (s = K.bootinfo->cmdline; *s != '\0'; s++) {
  }

  /* Find modules. */
  for (module = K.bootinfo->modules; module != NULL; module = module->next) {
    K.fs = initfs_from_module(module);
    /* TODO: support other modules */
    break;
  }
}

static void
kernel_remap(void) {
  status_t res;

  /* remap kernel entry (if present) as RW- */
  res = vmk_map_edit_flags_range(
      LAYOUT_KERNEL_VADDR_START,
      LAYOUT_TEXT_START,
      VM_FLAG_READ | VM_FLAG_WRITE);
  ASSERT(res == SUCCESS);

  /* remap kernel text as R-X */
  res = vmk_map_edit_flags_range(
      LAYOUT_TEXT_START,
      LAYOUT_TEXT_END,
      VM_FLAG_READ | VM_FLAG_EXEC);
  ASSERT(res == SUCCESS);

  /* remap kernel rodata as R-- */
  res = vmk_map_edit_flags_range(
      LAYOUT_RODATA_START,
      LAYOUT_RODATA_END,
      VM_FLAG_READ);
  ASSERT(res == SUCCESS);

  /* remap kernel bss and data as RW- */
  res = vmk_map_edit_flags_range(
      LAYOUT_DATA_START,
      LAYOUT_DATA_END,
      VM_FLAG_READ | VM_FLAG_WRITE);
  ASSERT(res == SUCCESS);

  /* remap bootinfo data as R-- */
  res = vmk_map_edit_flags_range(
      K.bootinfo->data_vaddr_start,
      K.bootinfo->data_vaddr_end,
      VM_FLAG_READ);
  ASSERT(res == SUCCESS);

  /* remap entry sections as R-- */
  res = vmk_map_edit_flags_range(
      LAYOUT_ENTRY_PADDR_START + HIGHER_HALF,
      LAYOUT_ENTRY_PADDR_END + HIGHER_HALF,
      VM_FLAG_READ
  );
  ASSERT(res == SUCCESS);

  /* unmap everything after K.bootinfo->data_vaddr_end */
  /* FIXME: do not use a hardcoded value if 1000, do this better */
  vmk_unmap_pages(PAGEALIGNUP(K.bootinfo->data_vaddr_end), 1000);
}

static void
kernel_init_singleton(struct bootinfo* bootinfo) {
  mem_set(&K, 0, sizeof(K));
  K.bootinfo = bootinfo;
}

void
kernel_main(struct bootinfo* bootinfo) {
  kernel_init_singleton(bootinfo);
  isr_init();
  pm_init();
  vm_init();
  mm_init();
  kernel_remap();
  ASSERT(arch_init() == SUCCESS);
  int_enable();
  kernel_parse_bootinfo();
  K.inited.kernel = TRUE;
  ASSERT(0 && "Hello from MIMIK!");
}
