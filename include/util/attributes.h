#ifndef MIMIK_ATTRIBUTES_H
#define MIMIK_ATTRIBUTES_H

#define attribute(x) __attribute__((x))

#define PACKED attribute(packed)
#define ALIGNED(s) attribute(aligned(s))

#endif
