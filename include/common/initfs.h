#ifndef MIMIK_INITFS_H
#define MIMIK_INITFS_H

#include <boot/bootinfo.h>

struct initfs {
  addr_t virt_start;
  addr_t virt_end;
  addr_t phys_start;
  addr_t phys_end;
};

struct initfs initfs_from_module(struct bootinfo_module* mod);
void initfs_release(struct initfs* fs);
status_t initfs_lookup(
    struct initfs* fs,
    const char* path,
    ptr_t* file_ptr,
    size_t* file_size);

#endif
