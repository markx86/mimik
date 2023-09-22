OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

KERNEL_SPACE = 0xFFFFFFFF80000000;

SECTIONS {
	. = 0x0000000000000000;
	
	.multiboot.data : {
		*(.multiboot.data)
	}

	.multiboot.text : {
		*(.multiboot.text)
	}

	. = KERNEL_SPACE;
	
	__kernel_start = .;

	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_SPACE) {
		*(.text)
	}

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_SPACE) {
		*(.rodata)
	}

	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_SPACE) {
		*(.data)
	}

	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_SPACE) {
		*(COMMON)
		*(.bss)
	}

	/DISCARD/ : {
		*(.note.*)
	}
	
	__kernel_end = .;
}