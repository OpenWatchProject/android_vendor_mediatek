#ifndef __DP_READ_BMP_H__
#define __DP_READ_BMP_H__

#include "DpDataType.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

DP_STATUS_ENUM utilReadBMP(const char    *pSrcFile,
                           uint8_t       *pOutBuf0,
                           uint8_t       *pOutBuf1,
                           uint8_t       *pOutBuf2,
                           DpColorFormat outFormat,
                           int32_t       bufWidth,
                           int32_t       bufHeight,
                           int32_t       bufPitch);

//#ifdef __cplusplus
//}
//#endif

#endif  // __DP_READ_BMP_H__
