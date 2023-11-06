OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS {
	. = 0x0000000000010000;
	
	_kernel_paddr_start = .;
	_kernel_vaddr_start = . + HIGHER_HALF;
	
	.entry.data : {
		KEEP(*(.multiboot.data))
		*(.entry.data)
	}

	.entry.bss : {
		*(.multiboot.bss)
		*(.entry.bss)
	}

	.entry.text ALIGN(4K) : {
		*(.multiboot.text)
		*(.entry.text)
	}

	. += HIGHER_HALF;
	
	.text : AT(ADDR(.text) - HIGHER_HALF) {
		*(.text)
	}

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - HIGHER_HALF) {
		*(.rodata)
	}

	.data : AT(ADDR(.data) - HIGHER_HALF) {
		*(.data)
	}

	.bss : AT(ADDR(.bss) - HIGHER_HALF) {
		*(.bss)
	}

	_kernel_paddr_end = . - HIGHER_HALF;
	_kernel_vaddr_end = .;

	_bootinfo_data = ALIGN(8) - HIGHER_HALF;
}
