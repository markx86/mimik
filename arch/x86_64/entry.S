#ifdef MIMIK_BOOTPROTOCOL_MULTIBOOT2

#define ASM_FILE
#include "multiboot2.h"

.section .multiboot.text, "ax"
.global _start
_start:
	cmp $MULTIBOOT2_BOOTLOADER_MAGIC, %eax
	jne .fail
	jmp _start
.fail:
	jmp .fail

#endif
