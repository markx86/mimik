CSOURCES += $(shell find $(ARCHDIR) -name '*.c')
SSOURCES += $(shell find $(ARCHDIR) -name '*.S')

BOOTPROTOCOL ?= multiboot2

LDS = $(RELARCHDIR)/$(BOOTPROTOCOL)/link.ld
DEFINES += \
	-DMIMIK_BOOTPROTOCOL_$(call uppercase, $(BOOTPROTOCOL)) \
	-DHIGHER_HALF=0xFFFFFFFF80000000