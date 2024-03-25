.section .entry.bss, "aw", @nobits

#define SIZE_4KB 0x1000
#define DEFINEPT(name) \
  .align SIZE_4KB      ;\
  name:                ;\
    .skip SIZE_4KB

DEFINEPT(pml4)
DEFINEPT(pdp)
DEFINEPT(pd)
DEFINEPT(pt)

.section .entry.data, "aw", @progbits
/* shut the compiler up */
kernel_vaddr_start:
  .quad _kernel_vaddr_start

.section .entry.text, "ax"
.code32

#define ENTRY_FLAG_PRESENT (1 << 0)
#define ENTRY_FLAG_WRITABLE (1 << 1)
#define ENTRY_FLAGS     \
  (ENTRY_FLAG_PRESENT | \
  ENTRY_FLAG_WRITABLE)

.global setup_paging
setup_paging:
  /* zero out page tables */
  movl $pml4, %edi
  call zero_pt
  movl $pdp, %edi
  call zero_pt
  movl $pd, %edi
  call zero_pt
  movl $pt, %edi
  call zero_pt

  /* compute page table indices */
  movl (kernel_vaddr_start), %eax
  movl (kernel_vaddr_start+4), %edx
  /* pt index */
  shrdl $12, %edx, %eax
  movl %eax, %ecx
  andl $0x1ff, %ecx
  pushl %ecx
  /* pd index */
  shrdl $9, %edx, %eax
  movl %eax, %ecx
  andl $0x1ff, %ecx
  pushl %ecx
  /* pdp index */
  shrdl $9, %edx, %eax
  movl %eax, %ecx
  andl $0x1ff, %ecx
  pushl %ecx
  /* pml4 index */
  shrdl $9, %edx, %eax
  movl %eax, %ecx
  andl $0x1ff, %ecx
  pushl %ecx

  /*
   * recursive map the lower half of the kernel
   * NOTE: pml4[0] and pt[0] have to be free
   */
  movl $(pt + ENTRY_FLAGS), %eax
  movl %eax, (pml4)
  movl %eax, (pt)

  /* set pml4 entry */
  popl %edi
  movl $pml4, %esi
  call get_entry
  movl $(pdp + ENTRY_FLAGS), (%edi)
  /* set pdp entry */
  popl %edi
  movl $pdp, %esi
  call get_entry
  movl $(pd + ENTRY_FLAGS), (%edi)
  /* set pd entry */
  popl %edi
  movl $pd, %esi
  call get_entry
  movl $(pt + ENTRY_FLAGS), (%edi)
  /* store map start address */
  movl $_kernel_paddr_start, %edx
  andl $~0xfff, %edx
  /*
   * compute kernel size in pages
   * NOTE: %ebx contains the bootinfo end address,
   *       which *should* be greater than _kernel_paddr_end
   */
  movl %ebx, %ecx
  addl $0xfff, %ecx
  andl $~0xfff, %ecx
  subl %edx, %ecx
  shrl $12, %ecx
  /* get first kernel page directory entry */
  popl %edi
  movl $pt, %esi
  call get_entry
0:
  /* map pages */
  movl %edx, %eax
  orl $ENTRY_FLAGS, %eax
  movl %eax, (%edi)
  addl $SIZE_4KB, %edx
  addl $8, %edi
  loop 0b
  /* set pml4 */
  movl $pml4, %eax
  movl %eax, %cr3
  ret

get_entry:
  shl $3, %edi
  addl %esi, %edi
  ret

zero_pt:
  movl $(SIZE_4KB >> 2), %ecx
0:
  movl $0, (%edi)
  addl $4, %edi
  loop 0b
  ret
