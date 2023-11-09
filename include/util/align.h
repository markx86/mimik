#ifndef MIMIK_ALIGN_H
#define MIMIK_ALIGN_H

#define ALIGN_UP(addr, align) (((addr) + ((align)-1)) & ~(align-1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align)-1))

#endif
