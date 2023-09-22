CSOURCES += $(shell find $(ARCHDIR) -name '*.c')
SSOURCES += $(shell find $(ARCHDIR) -name '*.S')

BOOTPROTOCOL ?= multiboot2

LDS = $(ARCHDIR)/link_$(BOOTPROTOCOL).ld
CCFLAGS += -DMIMIK_BOOTPROTOCOL_$(call uppercase, $(BOOTPROTOCOL))
