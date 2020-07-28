#ifndef RSC_TESTCOMMON_H
#define RSC_TESTCOMMON_H
#include <mtkcam/def/common.h>

MUINT32 rsc_comp_roi_mem_with_file(char* pGolden, short mode, MUINTPTR base_addr, int width, int height, MUINT32 stride,
                                int start_x, int start_y, int end_x, int end_y, MUINTPTR mask_base_addr, 
                                MUINT32 mask_size, MUINT32 mask_stride);
                                
#endif
