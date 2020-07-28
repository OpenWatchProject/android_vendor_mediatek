typedef struct DpBlitStream_Param
{
    uint32_t          srcWidth;
    uint32_t          srcHeight;
    DP_COLOR_ENUM     srcColorFormat;
    DP_PROFILE_ENUM   srcProfile;
    DpInterlaceFormat srcField;
    DpRect            srcCrop;
    DpSecure          srcSecure;
    bool              srcDoFlush;

    int32_t           rotate;
    int               flip;

    uint32_t          dstWidth;
    uint32_t          dstHeight;
    DP_COLOR_ENUM     dstColorFormat;
    DP_PROFILE_ENUM   dstProfile;
    DpInterlaceFormat dstField;
    DpRect            dstROI;
    DpSecure          dstSecure;
    bool              dstDoFlush;
}DpBlitStream_Param;
