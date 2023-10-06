#ifndef MIMIK_ACPI_H
#define MIMIK_ACPI_H

#include <types.h>

enum acpi_type {
  MIMIK_ACPI_TYPE_NONE = 0,
  MIMIK_ACPI_TYPE_RSDP = 1,
  MIMIK_ACPI_TYPE_XSDP = 2,
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

#endif
