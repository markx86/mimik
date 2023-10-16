uppercase = $(shell echo $(1) | tr 'a-z' 'A-Z')

BUILDTYPE = DEBUG
VERSION = 0.0.1
ARCH = $(shell uname -m)

BUILDDIR = $(abspath ./build)
SOURCEDIR = $(abspath .)
RELARCHDIR = arch/$(ARCH)
ARCHDIR = $(SOURCEDIR)/$(RELARCHDIR)
TARGET = $(BUILDDIR)/mimik-$(VERSION)-$(ARCH)
CSOURCES = $(shell find $(SOURCEDIR) -type f -name '*.c' -not -path '$(SOURCEDIR)/arch/*')
SSOURCES = $(shell find $(SOURCEDIR) -type f -name '*.S' -not -path '$(SOURCEDIR)/arch/*')

CC = gcc
LD = ld
DBG = gdb
VM = qemu-system-$(ARCH)

DEFINES = \
	-DMIMIK_ARCH_$(call uppercase, $(ARCH))
CCFLAGS = \
	-ffreestanding 							\
	-mno-red-zone 							\
	-Wall 									\
	-Werror 								\
	-O0										\
	-mcmodel=kernel							\
	-I$(SOURCEDIR)/include					\
	-I$(SOURCEDIR)/include/arch/$(ARCH)
LDFLAGS = -nostdlib -z noexecstack
VMFLAGS = \
	-cpu qemu64								\
	-smp cpus=1,cores=2,threads=2,maxcpus=4	\
	-machine q35							\
	-m 128M									\
	-vga std								\
	-kernel $(TARGET)
VMDBGFLAGS = -s -S -d guest_errors,cpu_reset,int -no-reboot -no-shutdown
DBGFLAGS = \
	-ex "target remote localhost:1234"			\
	-ex "set confirm off"						\
	-ex "set step-mode on"						\
	-ex "set disassembly-flavor intel"			\
	-ex "set disassemble-next-instruction on"	\
	-ex "add-symbol-file $(TARGET)"

include $(ARCHDIR)/arch.mk

ifeq ($(BUILDTYPE),DEBUG)
CCFLAGS += -ggdb
endif

OBJECTS = \
	$(patsubst $(SOURCEDIR)/%.c,$(BUILDDIR)/%.o,$(CSOURCES)) \
	$(patsubst $(SOURCEDIR)/%.S,$(BUILDDIR)/%.o,$(SSOURCES))

.PHONY: run debug clean

$(TARGET): $(BUILDDIR)/$(LDS) $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -T$^ -o $@

run: $(TARGET)
	$(VM) $(VMFLAGS)

debug: $(TARGET)
	$(VM) $(VMFLAGS) $(VMDBGFLAGS) &
	$(DBG) $(DBGFLAGS)

clean:
	rm -rf $(BUILDDIR)

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(DEFINES) -c $< -o $@
	
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(DEFINES) -c $< -o $@

$(BUILDDIR)/$(LDS): $(SOURCEDIR)/$(LDS)
	@mkdir -p $(dir $@)
	$(CC) -E -P -x c $(DEFINES) $< > $@
