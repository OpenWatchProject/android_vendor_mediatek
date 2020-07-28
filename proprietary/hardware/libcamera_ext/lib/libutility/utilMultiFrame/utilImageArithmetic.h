#ifndef _UTIL_IMAGEARITHMETIC_H_
#define _UTIL_IMAGEARITHMETIC_H_

#include "MTKUtilCommon.h"

#define SUBSAMPLE_D                     (4)       //Sub-smaple ratio in both direction
#define MOTION_CALCULATED_PIXELS        (10)

typedef struct UTIL_IMAGE_SAD_STRUCT
{
    UTIL_CLIP_IMAGE_STRUCT src1;
    UTIL_BASE_IMAGE_STRUCT src2;
    MINT32 sub_w;
    MINT32 sub_h;
    MINT16 saturation_value;
} UTIL_IMAGE_SAD_STRUCT, *P_UTIL_IMAGE_SAD_STRUCT;


UTIL_ERRCODE_ENUM ImageSubstract(const UTIL_BASE_IMAGE_STRUCT* A, const UTIL_BASE_IMAGE_STRUCT* B, MINT32 residue, UTIL_BASE_IMAGE_STRUCT* C);
UTIL_ERRCODE_ENUM ImageAdd(const UTIL_BASE_IMAGE_STRUCT* A, const UTIL_BASE_IMAGE_STRUCT* B, MINT32 residue, UTIL_BASE_IMAGE_STRUCT* C);
UTIL_ERRCODE_ENUM utilImageSad(P_UTIL_IMAGE_SAD_STRUCT sad_data, MUINT32 *sum, MUINT32 *count);



#endif /* _UTIL_IMAGEARITHMETIC_H_ */

