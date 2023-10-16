BOOTDIR=$(ARCHDIR)/boot

BOOTPROTOCOL ?= multiboot2

# Architecture specific implementation
CSOURCES += $(shell find $(ARCHDIR) -name '*.c' -not -path "$(BOOTDIR)")
SSOURCES += $(shell find $(ARCHDIR) -name '*.S' -not -path "$(BOOTDIR)")
# Boot protocol sources
CSOURCES += $(shell find $(BOOTDIR)/$(BOOTPROTOCOL) -name '*.c')
SSOURCES += $(shell find $(BOOTDIR)/$(BOOTPROTOCOL) -name '*.S')

LDS = $(RELARCHDIR)/boot/$(BOOTPROTOCOL)/link.ld
DEFINES += \
	-DMIMIK_BOOTPROTOCOL_$(call uppercase, $(BOOTPROTOCOL)) \
	-DHIGHER_HALF=0xFFFFFFFF80000000