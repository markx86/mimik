uppercase = $(shell echo $(1) | tr 'a-z' 'A-Z')
findsources = $(shell find $(1) -type f -name '*.S' -or -name '*.c' $(2))

BUILDTYPE = DEBUG
VERSION = 0.0.1
ARCH = $(shell uname -m)

BUILDDIR = $(abspath ./build)
SOURCEDIR = $(abspath .)
RELARCHDIR = arch/$(ARCH)
ARCHDIR = $(SOURCEDIR)/$(RELARCHDIR)

TARGET = $(BUILDDIR)/mimik-$(VERSION)-$(ARCH)
SOURCES = $(call findsources, $(SOURCEDIR), -not -path '$(SOURCEDIR)/arch/*')

CC = clang
LD = ld.lld
GDB = gdb
QEMU = qemu-system-$(ARCH)
CLANGFMT = clang-format

DEFINES = \
	-DMIMIK_ARCH_$(call uppercase, $(ARCH))
CCFLAGS = \
	-ffreestanding 							\
	-fstack-protector						\
	-fpic									\
	-nostdinc								\
	-Wall 									\
	-Wextra									\
	-Wconversion							\
	-O0										\
	-std=c99								\
	-I$(SOURCEDIR)/include					\
	-I$(SOURCEDIR)/arch/$(ARCH)/include
# TODO: figure out why the fuck it needs --no-relax
LDFLAGS = \
	-nostdlib	\
	--no-pie	\
	--no-relax
QEMUFLAGS = \
	-cpu qemu64,+ssse3,+sse4.1,+sse4.2		\
	-smp cpus=1,cores=2,threads=2,maxcpus=4	\
	-machine q35							\
	-m 128M									\
	-vga std								\
	-serial stdio							\
	-kernel $(TARGET)						\
	-initrd $(SOURCEDIR)/initrd.tar
QEMUGDBFLAGS = -s -S -d guest_errors,cpu_reset,int -no-reboot -no-shutdown
GDBFLAGS = \
	-ex "target remote localhost:1234"			\
	-ex "set confirm off"						\
	-ex "set step-mode on"						\
	-ex "set disassembly-flavor intel"			\
	-ex "set disassemble-next-instruction on"	\
	-ex "add-symbol-file $(TARGET)"

include $(ARCHDIR)/arch.mk

ifeq ($(BUILDTYPE),DEBUG)
CCFLAGS += -ggdb
else
CCFLAGS += -Werror
endif

OBJECTS = $(patsubst $(SOURCEDIR)/%.c,$(BUILDDIR)/%_c.o,$(SOURCES))
OBJECTS := $(patsubst $(SOURCEDIR)/%.S,$(BUILDDIR)/%_S.o,$(OBJECTS))

.PHONY: run debug clean fmt

$(TARGET): $(BUILDDIR)/$(LDS) $(OBJECTS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -T$^ -o $@

run: $(TARGET)
	$(QEMU) $(QEMUFLAGS)

debug: $(TARGET)
	$(QEMU) $(QEMUFLAGS) $(QEMUGDBFLAGS) &
	$(GDB) $(GDBFLAGS)

clean:
	rm -rf $(BUILDDIR)

fmt:
	$(CLANGFMT) -i $(shell find $(SOURCEDIR) -name '*.c' -or -name '*.h' -type f)

$(BUILDDIR)/%_c.o: $(SOURCEDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(DEFINES) -c $< -o $@
	
$(BUILDDIR)/%_S.o: $(SOURCEDIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(DEFINES) -c $< -o $@

$(BUILDDIR)/$(LDS): $(SOURCEDIR)/$(LDS)
	@mkdir -p $(dir $@)
	$(CC) -E -P -x c $(DEFINES) $< > $@
