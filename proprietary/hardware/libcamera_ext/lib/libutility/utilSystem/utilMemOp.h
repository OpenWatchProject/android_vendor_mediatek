#ifndef _UTIL_MEM_OP_H_
#define _UTIL_MEM_OP_H_

#include "MTKUtilCommon.h"

MINT32 vmemcpy(void *p_dst, void *p_src, MUINT32 size);
MINT32 vmemset(void *_Dst, MINT32 Value, MUINT32 Size);

#endif /* _UTIL_MEM_OP_H_ */
