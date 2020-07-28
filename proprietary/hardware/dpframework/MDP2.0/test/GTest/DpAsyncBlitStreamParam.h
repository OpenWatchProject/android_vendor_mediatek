#include "DpAsyncBlitStream.h"

typedef struct DpAsyncBlitStream_Param
{
    uint32_t                   srcWidth;
    uint32_t                   srcHeight;
    DP_COLOR_ENUM              srcColorFormat;
    DP_PROFILE_ENUM            srcProfile;
    DpInterlaceFormat          srcField;
    DpRect                     srcCrop[ISP_MAX_OUTPUT_PORT_NUM];
    DpSecure                   srcSecure;
    bool                       srcDoFlush;

    int32_t                    rotate[ISP_MAX_OUTPUT_PORT_NUM];
    int                        flip[ISP_MAX_OUTPUT_PORT_NUM];

    bool                       indexEnable[ISP_MAX_OUTPUT_PORT_NUM];
    uint32_t                   dstWidth[ISP_MAX_OUTPUT_PORT_NUM];
    uint32_t                   dstHeight[ISP_MAX_OUTPUT_PORT_NUM];
    DP_COLOR_ENUM              dstColorFormat[ISP_MAX_OUTPUT_PORT_NUM];
    DP_PROFILE_ENUM            dstProfile[ISP_MAX_OUTPUT_PORT_NUM];
    DpInterlaceFormat          dstField[ISP_MAX_OUTPUT_PORT_NUM];
    DpRect                     dstROI[ISP_MAX_OUTPUT_PORT_NUM];
    DpSecure                   dstSecure[ISP_MAX_OUTPUT_PORT_NUM];
    bool                       dstDoFlush[ISP_MAX_OUTPUT_PORT_NUM];
}DpIspStream_Param;
