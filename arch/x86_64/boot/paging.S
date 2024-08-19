.section .entry.text, "ax"
.code32

#define SIZE_4KB 0x1000

#define ENTRY_FLAG_PRESENT (1 << 0)
#define ENTRY_FLAG_WRITABLE (1 << 1)
#define ENTRY_FLAGS     \
  (ENTRY_FLAG_PRESENT | \
  ENTRY_FLAG_WRITABLE)

/*
 * NOTE: this function will work so long as:
 *       1. the kernel can fit within a pt (max. 2MB)
 *       2. the pt and pd indices for the higher half are zero
 */
.global setup_paging
setup_paging:
  /*
   * NOTE: %ebx contains the bootinfo end address,
   *       which *should* be greater than _kernel_paddr_end
   */
  movl %ebx, %edi
  /* align end paddr to next page */
  addl $0xfff, %edi
  andl $~0xfff, %edi
  /* zero out page tables */
  call next_cleared_pt
  pushl %edi /* pdp  paddr */
  call next_cleared_pt
  pushl %edi /* pd   paddr */
  call next_cleared_pt
  pushl %edi /* pt   paddr */
  call next_cleared_pt

  /* store map start paddr */
  movl $_kernel_paddr_start, %edx
  andl $~0xfff, %edx
  /* compute the number of pages we should map */
  leal SIZE_4KB(%edi), %ecx
  addl $0xfff, %ecx
  andl $~0xfff, %ecx
  subl %edx, %ecx
  shrl $12, %ecx
  /* save the pointer to pt */
  pushl %edi
  /* save start paddr into %esi */
  movl %edx, %esi
  /* compute start page index in the pt */
  shrl $12, %edx
  movl %edx, %eax
  andl $0x1ff, %eax
  leal (%edi,%eax,8), %edi
  /* map pages */
  leal ENTRY_FLAGS(%esi), %eax
0:
  stosl
  addl $4, %edi
  addl $SIZE_4KB, %eax
  loop 0b

  movl $3, %ecx
  /* get the pt paddr */
  popl %esi
1:
  /* get the next pt paddr */
  popl %edi
  /* compute the pd index */
  shrl $9, %edx
  movl %edx, %eax
  andl $0x1ff, %eax
  leal (%edi,%eax,8), %edi
  /* generate pt entry into pd */
  orl $ENTRY_FLAGS, %esi
  movl %esi, (%edi)
  /* set %esi to this pt paddr */
  andl $~0xfff, %edi
  movl %edi, %esi
  loop 1b

  /* set pml4 */
  movl %esi, %cr3
  ret

next_cleared_pt:
  movl $(SIZE_4KB >> 2), %ecx
  xorl %eax, %eax
  rep stosl
  ret

.code64

.global map_higher_half
map_higher_half:
  /* get the current pml4 paddr */
  movq %cr3, %rax
  andq $~0xfff, %rax
  /* %r8 (and %r10) holds the higher half vaddr */
  movq $_kernel_vaddr_start, %r8
  shldq $16, %r8, %r10
  shlq $16, %r8
  /* %r9 (and %r11) holds the lower half vaddr */
  movq $_kernel_paddr_start, %r9
  shldq $16, %r9, %r11
  shlq $16, %r9
  movq $3, %rcx
0:
  /* get next pt index for higher half vaddr */
  shldq $9, %r8, %r10
  shlq $9, %r8
  andq $0x1ff, %r10
  /* compute pointer to destination slot in pt */
  leaq (%rax,%r10,8), %rdi
  /* get next pt index for lower half vaddr */
  shldq $9, %r9, %r11
  shlq $9, %r9
  andq $0x1ff, %r11
  /* compute pointer to source slot in pt */
  leaq (%rax,%r11,8), %rsi
  /* get the next pt paddr */
  movq (%rsi), %rax
  andq $~0xfff, %rax
  /* copy the entry over */
  movsq
  loop 0b
  ret

