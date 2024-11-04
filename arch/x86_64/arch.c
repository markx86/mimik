#include <arch.h>
#include <cpu/gdt.h>
#include <cpu/pic.h>
#include <cpu/pit.h>
#include <cpu/cpuid.h>
#include <assert.h>

static void
check_and_enable_features(void) {
  struct cpuid_regs r;

  cpuid(CPUID_STDFN_00000001, &r);
  ASSERT(r.edx & (1 << 24)); /* check for FXSAVE/FXRSTOR instructions support */
  /* NOTE: we're going to assume every processor x86 comes after 2008
     and supports all the legacy SIMD instructions */
  ASSERT(r.edx & (1 << 25)); /* check for SSE1   support */
  ASSERT(r.edx & (1 << 26)); /* check for SSE2   support */
  ASSERT(r.ecx & (1 << 0));  /* check for SSE3   support */
  ASSERT(r.ecx & (1 << 9));  /* check for SSSE3  support */
  ASSERT(r.ecx & (1 << 19)); /* check for SSE4.1 support */
  ASSERT(r.ecx & (1 << 20)); /* check for SSE4.2 support */
  /* enable legacy SIMD instructions */
  ASM(
      /* set CR4.OSFXSR = 1 */
      "mov %%cr4, %%rax;"
      "or $(1 << 9), %%rax;"
      "mov %%rax, %%cr4;"
      /* set CR0.EM = 0 and CR0.MP = 1 */
      "mov %%cr0, %%rax;"
      "and $~(1 << 2), %%rax;"
      "or $(1 << 1), %%rax;"
      "mov %%rax, %%cr0"
      :
      :
      : "rax");

  cpuid(CPUID_EXTFN_80000001, &r);
  /* NOTE: we'll need NX bit support and SYSCALL/SYSRET support */
  ASSERT(r.edx & (1 << 20)); /* check for NX bit support */
  ASSERT(r.edx & (1 << 11)); /* check for SYSCALL/SYSRET instructions support */
}

status_t
arch_init(void) {
  gdt_load();
  check_and_enable_features();
  acpi_init();
  pic_init();
  pit_init();
  return SUCCESS;
}
