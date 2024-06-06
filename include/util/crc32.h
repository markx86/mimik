#ifndef MIMIK_CRC32_H
#define MIMIK_CRC32_H

#include <util/crc32-arch.h>

/* if there's not platfrom specific crc32c implementation defined  */
#ifndef ARCH_CRC32C
#error "No default crc32c implementation defined"
#else
#undef ARCH_CRC32C
#endif

#endif
