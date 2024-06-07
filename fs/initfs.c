#include <fs/initfs.h>
#include <util/compiler.h>
#include <mem/str.h>
#include <mem/mem.h>
#include <mm/pm.h>
#include <mm/vm.h>
#include <assert.h>

enum type_flag {
  TYPE_FLAG_FILE = 1,
  TYPE_FLAG_HARDLINK,
  TYPE_FLAG_CHARDEV,
  TYPE_FLAG_BLOCKDEV,
  TYPE_FLAG_DIRECTORY,
  TYPE_FLAG_PIPE
};

struct PACKED ustar_header {
  char file_name[100];
  char file_mode[8];
  char owner_uid[8];
  char owner_gid[8];
  char file_size[12];
  char last_modification_time[12];
  char checksum[8];
  char type_flag;
  char linked_file_name[100];
  char ustar_string[6];
  char ustar_version[2];
  char owner_user_name[32];
  char owner_group_name[32];
  char device_major_number[8];
  char device_minor_number[8];
  char file_name_prefix[155];
  char reserved[12];
};

static inline size_t
oct_str_to_num(const char* str, size_t len) {
  size_t num = 0;
  while (--len > 0) {
    num <<= 3;
    ASSERT(*str >= '0' && *str <= '7');
    num += (size_t)(*(str++) - '0');
  }
  return num;
}

struct initfs
initfs_from_module(struct bootinfo_module* mod) {
  struct initfs fs;
  fs.phys_start = mod->start_address;
  fs.phys_end = mod->end_address;
  fs.virt_start = 0;
  pm_try_lock_range(fs.phys_start, fs.phys_end);
  vm_kmap_range(fs.phys_start, fs.phys_end, &fs.virt_start, 0);
  fs.virt_end = fs.virt_start + (fs.phys_end - fs.phys_start);
  return fs;
}

void
initfs_release(struct initfs* fs) {
  ASSERT(fs->virt_start != 0);
  vm_kunmap_range(fs->virt_start, fs->virt_end);
  pm_try_release_range(fs->phys_start, fs->phys_end);
  mem_set(fs, 0, sizeof(*fs));
}

status_t
initfs_lookup(
    struct initfs* fs,
    const char* path,
    ptr_t* file_ptr,
    size_t* file_size) {
  struct ustar_header* hdr = (struct ustar_header*)fs->virt_start;
  ASSERT(hdr != NULL);
  ASSERT(file_ptr != NULL);
  ASSERT(file_size != NULL);
  ASSERT(path != NULL);
  ASSERT(str_length(path) < sizeof(hdr->file_name));
  while ((addr_t)hdr < fs->virt_end) {
    size_t sz;
    ASSERT(str_equal(hdr->ustar_string, "ustar"));
    ASSERT(str_nequal(hdr->ustar_version, "00", 2));
    sz = oct_str_to_num(hdr->file_size, sizeof(hdr->file_size));
    if (str_equal(hdr->file_name, path)) {
      *file_size = sz;
      *file_ptr = hdr + 1;
      return SUCCESS;
    }
    hdr = (struct ustar_header*)ALIGNUP((addr_t)(hdr + 1) + sz, sizeof(*hdr));
  }
  return -ENOENT;
}
