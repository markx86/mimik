#ifndef MIMIK_BOOTINFO_H
#define MIMIK_BOOTINFO_H

#include <acpi.h>

struct bootinfo_module {
  struct bootinfo_module* next;
  addr_t start_address;
  addr_t end_address;
  char cmdline[];
};

enum bootinfo_mem_segment_type {
  BOOTINFO_MEM_SEGMENT_TYPE_RESERVED = 0,
  BOOTINFO_MEM_SEGMENT_TYPE_AVAILABLE = 1,
};

struct bootinfo_mem_segment {
  union {
    enum bootinfo_mem_segment_type type;
    uint64_t force8;
  };
  addr_t addr;
  size_t length;
};

struct bootinfo_mem_map {
  size_t entries;
  struct bootinfo_mem_segment* segments;
};

struct bootinfo_acpi {
  union {
    enum acpi_type type;
    uint64_t force8;
  };
  union {
    struct acpi_rsdp rsdp;
    struct acpi_xsdp xsdp;
  };
};

struct bootinfo {
  char* cmdline;
  struct bootinfo_module* modules;
  struct bootinfo_mem_map mem_map;
  struct bootinfo_acpi acpi;
};

#endif
