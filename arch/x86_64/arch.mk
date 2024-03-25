BOOTDIR=$(ARCHDIR)/boot

BOOTPROTOCOL ?= multiboot2

# Architecture specific implementation
SOURCES += $(call findsources, $(ARCHDIR), -not -path '$(BOOTDIR)')
# Boot protocol sources
SOURCES += $(call findsources, $(BOOTDIR)/$(BOOTPROTOCOL))

LDS = $(RELARCHDIR)/boot/$(BOOTPROTOCOL)/link.ld
DEFINES += \
	-DMIMIK_BOOTPROTOCOL_$(call uppercase, $(BOOTPROTOCOL)) \
	-DHIGHER_HALF=0xffffffff80000000
