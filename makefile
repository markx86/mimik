uppercase = $(shell echo $(1) | tr 'a-z' 'A-Z')

BUILDTYPE = DEBUG
VERSION = 0.0.1
ARCH = $(shell uname -m)

BUILDDIR = $(abspath ./build)
SOURCEDIR = $(abspath .)
ARCHDIR = $(SOURCEDIR)/arch/$(ARCH)
TARGET = $(BUILDDIR)/mimik-$(VERSION)-$(ARCH)
CSOURCES = $(shell find $(SOURCEDIR) -type f -name '*.c' -not -path '$(SOURCEDIR)/arch/*')
SSOURCES = $(shell find $(SOURCEDIR) -type f -name '*.S' -not -path '$(SOURCEDIR)/arch/*')

CC = gcc
LD = ld
DBG = gdb
VM = qemu-system-$(ARCH)

CCFLAGS = \
	-ffreestanding 							\
	-mno-red-zone 							\
	-Wall 									\
	-Werror 								\
	-I$(SOURCEDIR)/include					\
	-DMIMIK_ARCH_$(call uppercase, $(ARCH))
LDFLAGS = -nostdlib
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

include $(ARCHDIR)/config.mk

ifeq ($(BUILDTYPE),DEBUG)
CCFLAGS += -g
else
CCFLAGS += -O2
endif

OBJECTS = \
	$(patsubst $(SOURCEDIR)/%.c,$(BUILDDIR)/%.o,$(CSOURCES)) \
	$(patsubst $(SOURCEDIR)/%.S,$(BUILDDIR)/%.o,$(SSOURCES))

.PHONY: run debug clean

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -T$(LDS) -o $@ $^

run: $(TARGET)
	$(VM) $(VMFLAGS)

debug: $(TARGET)
	$(VM) $(VMFLAGS) $(VMDBGFLAGS) &
	$(DBG) $(DBGFLAGS)

clean:
	rm -rf $(BUILDDIR)

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -c $< -o $@
	
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -c $< -o $@
