#ifndef MIMIK_ALIGN_H
#define MIMIK_ALIGN_H

#define ALIGNUP(val, align)   (((val) + ((align)-1)) & (size_t) ~((align)-1))
#define ALIGNDOWN(val, align) ((val) & (size_t) ~((align)-1))

#endif
