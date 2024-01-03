#ifndef MIMIK_TYPES_H
#define MIMIK_TYPES_H

#include <errors.h>

typedef char int8_t;
#define INT8_MIN 0x80
#define INT8_MAX (~INT8_MIN)

typedef short int16_t;
#define INT16_MIN 0x8000
#define INT16_MAX (~INT16_MIN)

typedef int int32_t;
#define INT32_MIN 0x80000000
#define INT32_MAX (~INT32_MIN)

typedef long int64_t;
#define INT64_MIN 0x8000000000000000
#define INT64_MAX (~INT64_MIN)

typedef unsigned char uint8_t;
#define UINT8_MIN 0
#define UINT8_MAX 0xFF

typedef unsigned short uint16_t;
#define UINT16_MIN 0
#define UINT16_MAX 0xFFFF

typedef unsigned int uint32_t;
#define UINT32_MIN 0
#define UINT32_MAX 0xFFFFFFFF

typedef unsigned long uint64_t;
#define UINT64_MIN 0
#define UINT64_MAX 0xFFFFFFFFFFFFFFFF

typedef int64_t ssize_t;
#define SSIZE_MIN -1
#define SSIZE_MAX INT64_MAX

typedef uint64_t size_t;
#define SIZE_MIN 0
#define SIZE_MAX UINT64_MAX

typedef uint64_t addr_t;
#define ADDR_MIN 0
#define ADDR_MAX UINT64_MAX

typedef void* ptr_t;
#define NULL ((ptr_t)0)

typedef uint8_t bool_t;
#define TRUE 1
#define FALSE 0

typedef int64_t status_t;
#define SUCCESS 0
#define ISERROR(e) ((e) < 0)

#endif
