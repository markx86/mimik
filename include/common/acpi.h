#ifndef MIMIK_ACPI_H
#define MIMIK_ACPI_H

#include <types.h>

enum acpi_type {
  ACPI_TYPE_NONE = 0,
  ACPI_TYPE_RSDP = 1,
  ACPI_TYPE_XSDP = 2,
};

struct acpi_rsdp {
  char signature[8];
  uint8_t checksum;
  char oem_id[6];
  uint8_t revision;
  uint32_t rsdt_address;
};

struct acpi_xsdp {
  struct acpi_rsdp rsdp;
  uint32_t length;
  uint64_t xsdt_address;
  uint8_t extended_checksum;
  uint8_t reserved[3];
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

enum acpi_table {
  ACPI_MADT,
};

ptr_t acpi_get_table(enum acpi_table table);
status_t acpi_init(struct bootinfo_acpi* acpi);

#endif
