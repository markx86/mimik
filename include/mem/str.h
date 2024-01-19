#ifndef MIMIK_STR_H
#define MIMIK_STR_H

#include <types.h>

size_t str_length(const char* str);
bool_t str_equal(const char* this, const char* other);
bool_t str_nequal(const char* this, const char* other, size_t n);

#endif
