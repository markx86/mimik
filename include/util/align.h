#ifndef MIMIK_ALIGN_H
#define MIMIK_ALIGN_H

#define ALIGNUP(val, align) (((val) + ((align)-1)) & ~(align - 1))
#define ALIGNDOWN(val, align) ((val) & ~((align)-1))

#endif
