#ifndef MIMIK_LAYOUT_H
#define MIMIK_LAYOUT_H

#define _(x) extern char _kernel_##x##_start[0], _kernel_##x##_end[0]

_(paddr);
_(vaddr);
_(text);
_(rodata);
_(data);

#undef _

#define LAYOUT_HIGHER_HALF HIGHER_HALF

#define LAYOUT_PADDR_START ((addr_t)_kernel_paddr_start)
#define LAYOUT_PADDR_END   ((addr_t)_kernel_paddr_end)

#define LAYOUT_VADDR_START ((addr_t)_kernel_vaddr_start)
#define LAYOUT_VADDR_END   ((addr_t)_kernel_vaddr_end)

#define LAYOUT_TEXT_START ((addr_t)_kernel_text_start)
#define LAYOUT_TEXT_END   ((addr_t)_kernel_text_end)

#define LAYOUT_RODATA_START ((addr_t)_kernel_rodata_start)
#define LAYOUT_RODATA_END   ((addr_t)_kernel_rodata_end)

#define LAYOUT_DATA_START ((addr_t)_kernel_data_start)
#define LAYOUT_DATA_END   ((addr_t)_kernel_data_end)

#define LAYOUT_HEAP_START 0xffff800000000000

#endif
