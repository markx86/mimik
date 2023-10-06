.section .entry.bss, "aw", @nobits
.align 4096
table:
  .skip 4096

.section .entry.data, "aw", @progbits
/* shut the compiler up */
kernel_vaddr_start:
  .quad _kernel_vaddr_start

.section .entry.text, "ax"
.code32

#define ENTRY_FLAG_PRESENT (1 << 0)
#define ENTRY_FLAG_READWRITE (1 << 1)
#define ENTRY_FLAGS     \
  (ENTRY_FLAG_PRESENT | \
  ENTRY_FLAG_READWRITE)
#define ENTRY_ADDR_MASK 0xFFFFFFFFFF000

#define SIZE_4KB 0x1000

.global setup_paging
setup_paging:
  /* compute page table indices */
  movl (kernel_vaddr_start), %eax
  movl (kernel_vaddr_start+4), %edx
  /* pt index */
  shrdl $12, %edx, %eax
  movl %eax, %ecx
  andl $0x1FF, %ecx
  pushl %ecx
  /* pd index */
  shrdl $9, %edx, %eax
  movl %eax, %ecx
  andl $0x1FF, %ecx
  pushl %ecx
  /* pdp index */
  shrdl $9, %edx, %eax
  movl %eax, %ecx
  andl $0x1FF, %ecx
  pushl %ecx
  /* pml4 index */
  shrdl $9, %edx, %eax
  movl %eax, %ecx
  andl $0x1FF, %ecx
  pushl %ecx
  
  /*
   * for simplicity we'll with a recursive map of the kernel
   * the pml4, pdp and pd entries will have it's physical address
   * pointing back to `table`
   */
  movl $table, %eax
  andl $ENTRY_ADDR_MASK, %eax
  orl $ENTRY_FLAGS, %eax
  /* set the pml4 entry */
  popl %edi
  call get_entry
  movl %eax, (%edi)
  /* set the pdp entry */
  popl %edi
  call get_entry
  movl %eax, (%edi)
  /* set the pd entry */
  popl %edi
  call get_entry
  movl %eax, (%edi)

  /* store map start address */
  movl $_kernel_paddr_start, %ebx
  /* compute kernel size in pages */
  movl $_kernel_paddr_end, %ecx
  subl %ebx, %ecx
  addl $0xFFF, %ecx
  andl $~0xFFF, %ecx
  shrl $12, %ecx
  /* get first kernel page directory entry */
  popl %edi
  call get_entry
0:
  /* map kernel */
  movl %ebx, %eax
  andl $ENTRY_ADDR_MASK, %eax
  orl $ENTRY_FLAGS, %eax
  movl %eax, (%edi)
  addl $8, %edi
  addl $SIZE_4KB, %ebx
  loop 0b
  /* set pml4 */
  movl $table, %eax
  andl $ENTRY_ADDR_MASK, %eax
  movl %eax, %cr3
  ret

get_entry:
  shl $3, %edi
  addl $table, %edi
  ret
