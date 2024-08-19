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
		*(.entry.bss)
	}

	.entry.text ALIGN(4K) : {
		*(.multiboot.text)
		*(.entry.text)
	}

	. += HIGHER_HALF;
	
	.text ALIGN(4K) : AT(ADDR(.text) - HIGHER_HALF) {
		_kernel_text_start = .;
		*(.text)
		_kernel_text_end = .;
	}

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - HIGHER_HALF) {
		_kernel_rodata_start = .;
		*(.rodata)
		_kernel_rodata_end = .;
	}

	.data ALIGN(4K) : AT(ADDR(.data) - HIGHER_HALF) {
		_kernel_data_start = .;
		*(.data)
	}

	.bss : AT(ADDR(.bss) - HIGHER_HALF) {
		*(.bss)
		_kernel_data_end = .;
	}

	_kernel_paddr_end = . - HIGHER_HALF;
	_kernel_vaddr_end = .;

	_bootinfo_data = ALIGN(8) - HIGHER_HALF;
}
