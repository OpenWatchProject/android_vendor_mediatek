#ifndef __DP_WRITE_BMP_H__
#define __DP_WRITE_BMP_H__

#include "DpDataType.h"

DP_STATUS_ENUM utilWriteBMP(const char    *pFileName,
                            void          *pPixels0,
                            void          *pPixels1,
                            void          *pPixels2,
                            DpColorFormat colorType,
                            int32_t       srcWidth,
                            int32_t       srcHeight,
                            int32_t       srcYPitch,
                            int32_t       srcUVPitch);

#endif  // __DP_WRITE_BMP_H__
