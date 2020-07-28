typedef struct DpFragStream_Param
{
    uint32_t          srcWidth;
    uint32_t          srcHeight;
    DP_COLOR_ENUM     srcColorFormat;
    int32_t           MCUXSize;
    int32_t           MCUYSize;
    DpRect            srcCrop;

    int32_t           rotate;
    int               flip;

    uint32_t          dstWidth;
    uint32_t          dstHeight;
    DP_COLOR_ENUM     dstColorFormat;
    DpRect            dstROI;
    bool              waitBuf;
}DpFragStream_Param;
