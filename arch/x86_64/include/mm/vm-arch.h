#ifndef MIMIK_VM_ARCH_H
#define MIMIK_VM_ARCH_H

#ifndef MIMIK_VM_H
#error "Do not include vm-arch.h directly. Include vm.h instead."
#endif

#include <types.h>
#include <util/compiler.h>

typedef uint64_t pte_t;

#define PT_LEVELS 4
#define PT_LENGTH 512
#define PT_SIZE   PAGE_SIZE

#define PTE_BIT_PRESENT   0
#define PTE_BIT_WRITE     1
#define PTE_BIT_USER      2
#define PTE_BIT_NOEXECUTE 63

#define VADDR_PAGE_IDX_BITS 9

static inline void
pte_set_paddr(pte_t* pte, addr_t paddr) {
  *pte = (*pte & 0xfff0000000000fff) | (paddr & 0x000ffffffffff000);
}

static inline addr_t
pte_get_paddr(pte_t pte) {
  return pte & 0x000ffffffffff000;
}

static inline addr_t
vm_get_current_root_pt_paddr(void) {
  addr_t paddr;
  ASM("movq %%cr3, %0" : "=r"(paddr));
  return paddr;
}

static inline void
vm_invalidate_page(addr_t vaddr) {
  ASM("invlpg (%0)" : : "r"(vaddr));
}

static inline void
vm_flush_tlb(void) {
  ASM("movq %%cr3, %%rax;"
      "movq %%rax, %%rax" : : : "rax");
}

static inline addr_t
vm_get_fault_address(void) {
  register addr_t fault;
  ASM("movq %%cr2, %0" : "=r"(fault));
  return fault;
}

#endif
