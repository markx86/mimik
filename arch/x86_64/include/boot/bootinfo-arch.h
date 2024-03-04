#ifndef MIMIK_BOOTINFO_ARCH_H
#define MIMIK_BOOTINFO_ARCH_H

#include <acpi.h>

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

struct bootinfo_arch {
  struct bootinfo_acpi acpi;
};

#endif
