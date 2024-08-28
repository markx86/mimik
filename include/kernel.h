#ifndef MIMIK_KERNEL_H
#define MIMIK_KERNEL_H

#include <common/initfs.h>
#include <boot/bootinfo.h>
#include <log/log.h>

struct kernel_singleton {
  struct {
    uint32_t pm : 1;
    uint32_t vm : 1;
    uint32_t mm : 1;
    uint32_t reserved : 29;
  } subsys_init;
  struct bootinfo* bootinfo;
  struct initfs fs;
};

extern struct kernel_singleton K;

#endif
