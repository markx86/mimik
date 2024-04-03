#include <common/acpi.h>
#include <mm/vm.h>
#include <assert.h>

struct acpi_sdt_header {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
};

static struct acpi_sdt_header* root;

static bool_t
checksum_matches(void* ptr, size_t sz) {
  uint8_t c, *p = ptr;
  for (c = 0; sz > 0; --sz)
    c += *(p++);
  return c == 0;
}

static bool_t
signature_matches(char* sig) {
  size_t i;
  char target[] = "RSD PTR ";
  for (i = 0; i < sizeof(target) - 1; ++i) {
    if (sig[i] != target[i])
      return FALSE;
  }
  return TRUE;
}

ptr_t
acpi_get_table(enum acpi_table table) {
  return NULL;
}

static status_t
acpi_init_rsdp(struct acpi_rsdp* rsdp) {
  status_t res;
  if (!checksum_matches(rsdp, sizeof(struct acpi_rsdp)))
    return -EINVAL;
  if (!signature_matches(rsdp->signature))
    return -EINVAL;
  if (rsdp->revision != 0)
    return -EINVAL;
  res = vm_kmap_bytes(
      rsdp->rsdt_address,
      sizeof(struct acpi_sdt_header),
      (addr_t*)&root,
      0);
  if (ISERROR(res))
    return res;
  if (root->length > PAGE_SIZE - ((addr_t)root & 0xfff)) {
    res = vm_kmap_bytes(rsdp->rsdt_address, root->length, (addr_t*)&root, 0);
    if (ISERROR(res))
      return res;
  }
  return SUCCESS;
}

static status_t
acpi_init_xsdp(struct acpi_xsdp* xsdp) {
  status_t res;
  if (!checksum_matches(xsdp, sizeof(struct acpi_xsdp)))
    return -EINVAL;
  if (!signature_matches(xsdp->rsdp.signature))
    return -EINVAL;
  if (xsdp->rsdp.revision != 2)
    return -EINVAL;
  res = vm_kmap_page(xsdp->xsdt_address, (addr_t*)&root, 0);
  if (ISERROR(res))
    return res;
  if (root->length > PAGE_SIZE - ((addr_t)root & 0xfff)) {
    res = vm_kmap_pages(
        xsdp->xsdt_address,
        PAGES(root->length),
        (addr_t*)&root,
        0);
    if (ISERROR(res))
      return res;
  }
  return -ENOTIMPL;
}

status_t
acpi_init(struct bootinfo_acpi* acpi) {
  root = NULL;
  switch (acpi->type) {
    case ACPI_TYPE_RSDP:
      return acpi_init_rsdp(&acpi->rsdp);
    case ACPI_TYPE_XSDP:
      return acpi_init_xsdp(&acpi->xsdp);
    default:
      return -ENOTIMPL;
  }
}
