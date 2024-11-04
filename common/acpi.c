#include <common/acpi.h>
#include <mm/mm.h>
#include <mem/mem.h>
#include <mem/str.h>
#include <mem/layout.h>
#include <log/log.h>
#include <assert.h>
#include <kernel.h>

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

static struct acpi_sdt_header* root = NULL;
static struct acpi_sdt_header** tbls = NULL;
static const char* known_tbls[] = {
  [ACPI_TABLE_MADT] = "APIC",
  [ACPI_TABLE_FADT] = "FACP",
  [ACPI_TABLE_MCFG] = "MCFG",
  [ACPI_TABLE_RSDT] = "RSDT",
  [ACPI_TABLE_SSDT] = "SSDT",
  [ACPI_TABLE_XSDT] = "XSDT"};

static bool_t
checksum_matches(ptr_t ptr, size_t sz) {
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
acpi_get_known_table(enum acpi_table table) {
  if (table >= ACPI_TABLE_MAX)
    return NULL;
  return acpi_get_table(known_tbls[table]);
}

ptr_t
acpi_get_table(const char* sig) {
  struct acpi_sdt_header** tbl = tbls;
  ASSERT(tbls != NULL);
  if (sig == NULL)
    return NULL;
  if (!str_nlength(sig, 4))
    return NULL;
  while (*tbl != NULL) {
    if (str_nequal((*tbl)->signature, sig, 4)) {
      if (checksum_matches(*tbl, (*tbl)->length))
        return *tbl;
      LOGWARNING("corrupted table %4s", sig);
      return NULL;
    }
    ++tbl;
  }
  return NULL;
}

static status_t
acpi_init_generic(
    struct acpi_rsdp* rsdp,
    addr_t sdt_address,
    uint8_t revision) {
  if (!checksum_matches(rsdp, sizeof(*rsdp)))
    return -EINVAL;
  if (!signature_matches(rsdp->signature))
    return -EINVAL;
  if (rsdp->revision != revision)
    return -EINVAL;
  root = (struct acpi_sdt_header*)layout_paddr_to_vaddr(sdt_address);
  return SUCCESS;
}

static void
map_n_tables(size_t n_tbls, size_t addr_size) {
  size_t i, tbls_sz;
  addr_t addr_mask;
  uint64_t* paddr;
  addr_mask = ~(((addr_t)-1) << (addr_size << 3));
  tbls_sz = sizeof(struct acpi_sdt_header*) * (n_tbls + 1);
  tbls = mm_alloc(tbls_sz);
  mem_set(tbls, 0, tbls_sz);
  paddr = (uint64_t*)(root + 1);
  for (i = 0; i < n_tbls; ++i) {
    tbls[i] =
        (struct acpi_sdt_header*)layout_paddr_to_vaddr(*paddr & addr_mask);
    paddr = (uint64_t*)((addr_t)paddr + addr_size);
  }
  tbls[n_tbls] = NULL;
}

static status_t
acpi_init_rsdp(struct acpi_rsdp* rsdp) {
  size_t n_tbls;
  status_t res = acpi_init_generic(rsdp, rsdp->rsdt_address, 0);
  if (ISERROR(res))
    return res;
  if (!str_nequal(root->signature, known_tbls[ACPI_TABLE_RSDT], 4))
    return -EINVAL;
  if (!checksum_matches(root, root->length))
    return -EINTEGRITY;
  /* map all the acpi tables in the RSDT */
  n_tbls = (root->length - sizeof(*root)) >> 2;
  map_n_tables(n_tbls, sizeof(uint32_t));
  return SUCCESS;
}

static status_t
acpi_init_xsdp(struct acpi_xsdp* xsdp) {
  size_t n_tbls;
  status_t res = acpi_init_generic(&xsdp->rsdp, xsdp->xsdt_address, 2);
  if (ISERROR(res))
    return res;
  if (!str_nequal(root->signature, known_tbls[ACPI_TABLE_XSDT], 4))
    return -EINVAL;
  if (!checksum_matches(root, root->length))
    return -EINTEGRITY;
  /* map all the acpi tables in the XSDT */
  n_tbls = (root->length - sizeof(struct acpi_sdt_header)) >> 3;
  map_n_tables(n_tbls, sizeof(uint64_t));
  return SUCCESS;
}

status_t
acpi_init(void) {
  status_t res;
  struct acpi_sdt_header** tbl;
  struct bootinfo_acpi* acpi = &K.bootinfo->arch.acpi;
  switch (acpi->type) {
    case ACPI_TYPE_RSDP:
      res = acpi_init_rsdp(&acpi->rsdp);
      break;
    case ACPI_TYPE_XSDP:
      res = acpi_init_xsdp(&acpi->xsdp);
      break;
    default:
      return -ENOTIMPL;
  }
  if (ISERROR(res))
    return res;
  ASSERT(root != NULL);
  ASSERT(tbls != NULL);
  for (tbl = tbls; *tbl != NULL; ++tbl)
    LOGTRACE("found acpi table %4s", (*tbl)->signature);
  return SUCCESS;
}
