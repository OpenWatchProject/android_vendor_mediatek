#ifndef __DP_READ_BIN_H__
#define __DP_READ_BIN_H__

#include "DpDataType.h"

DP_STATUS_ENUM utilWriteBin(const char    *pDstFile,
                            void          *pOutBuf0,
                            void          *pOutBuf1,
                            void          *pOutBuf2,
                            DpColorFormat srcFormat,
                            int32_t       srcWidth,
                            int32_t       srcHeight,
                            int32_t       srcYPitch,
                            int32_t       srcUVPitch);

#endif  // __DP_WRITE_BIN_H__
