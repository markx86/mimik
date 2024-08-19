BOOTDIR = $(ARCHDIR)/boot

BOOTPROTOCOL ?= multiboot2

# Architecture specific implementation
SOURCES += $(call findsources, $(ARCHDIR), -not -path '$(BOOTDIR)')
# Boot protocol sources
SOURCES += $(call findsources, $(BOOTDIR)/$(BOOTPROTOCOL))

LDS = $(RELARCHDIR)/boot/$(BOOTPROTOCOL)/link.ld
DEFINES += \
	-DMIMIK_BOOTPROTOCOL_$(call uppercase, $(BOOTPROTOCOL)) \
	-DHIGHER_HALF=0xffffffff80000000

CCFLAGS += \
	-mstack-protector-guard=global
# TODO: use per thread stack protector
#	-mstack-protector-guard=tls				\
#	-mstack-protector-guard-reg=gs			\
#	-mstack-protector-guard-offset=0
