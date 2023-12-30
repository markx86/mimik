#ifndef MIMIK_ALIGN_H
#define MIMIK_ALIGN_H

#define ALIGN_UP(val, align) (((val) + ((align)-1)) & ~(align - 1))
#define ALIGN_DOWN(val, align) ((val) & ~((align)-1))

#endif
