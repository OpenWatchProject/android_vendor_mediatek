#include "DpWriteBMP.h"
#include "DpColorConv.h"
#include "DpLogger.h"
#include "errno.h"

void conv_block_to_scan(uint8_t  *pSourceBuf,
                        uint32_t blockWidth,
                        uint32_t blockHeight,
                        uint32_t bytesPerPixel,
                        uint32_t outputWidth,
                        uint32_t outputHeight,
                        uint8_t  *pTargetBuf)
{
    uint8_t  *pTemp;
    uint32_t index;
    uint32_t inner;
    uint32_t count;
    uint32_t st[2];
    uint32_t dt[2];
    uint32_t dx[2];
    uint32_t dy[2];
    uint32_t srcIndex;
    uint32_t dstIndex;

    pTemp = (uint8_t*)malloc(outputWidth * outputHeight * bytesPerPixel * sizeof(uint8_t));
    assert(NULL != pTemp);

    memset(pTemp, 0x0, outputWidth * outputHeight * bytesPerPixel);

    for(index = 0; index < outputHeight; index++)
    {
        st[0] = index * outputWidth;

        dy[0] = index / blockHeight;
        dy[1] = index % blockHeight;

        dt[0] = dy[0] * blockHeight * outputWidth +
                dy[1] * blockWidth;

        for(inner = 0; inner < outputWidth; inner++)
        {
            st[1] = (st[0] + inner) * bytesPerPixel;

            dx[0] = inner / blockWidth;
            dx[1] = inner % blockWidth;

            dt[1] = (dt[0] + dx[0] * blockWidth * blockHeight + dx[1]) * bytesPerPixel;

            for(count = 0; count < bytesPerPixel; count++)
            {
                srcIndex = st[1] + count;
                dstIndex = dt[1] + count;

                pTemp[srcIndex] = pSourceBuf[dstIndex];;
            }
        }
    }

    memcpy(pTargetBuf, pTemp, outputWidth * outputHeight * bytesPerPixel);

    free(pTemp);
}


void conv_field_to_frame(uint8_t  *pSourceBuf,
                         uint32_t sourceWidth,
                         uint32_t sourceHeight,
                         uint32_t blockWidth,
                         uint32_t blockHeight,
                         uint32_t bytesPerPixel,
                         uint8_t  *pTargetBuf)
{
    uint8_t  *pBlock;
    uint32_t s_0   = 0;
    uint32_t s_1   = 0;
    uint32_t s_2   = 0;
    uint32_t m     = 0;
    uint32_t n     = 0;
    uint32_t j     = 0;
    uint32_t b     = 0;
    uint32_t st[2] = {0};
    uint32_t dt[2] = {0};
    uint32_t dy[2] = {0};
    uint32_t si    = 0;
    uint32_t di    = 0;

    s_0 = sourceWidth * sourceHeight;
    s_1 = blockWidth  * blockHeight;
    s_2 = s_0 * bytesPerPixel;

    pBlock = pSourceBuf;
    for(j=0; j < s_0; j = j + s_1)
    {
        for(n = 0; n < blockHeight; n++)
        {
            st[0] = j + n * blockWidth;

            dy[0] = n % 2;
            dy[1] = n / 2;
            dt[0] = (dy[0] * (blockHeight / 2) + dy[1]) * blockWidth;

            for(m = 0; m < blockWidth; m++)
            {
                st[1] = (st[0] + m) * bytesPerPixel;
                dt[1] = (dt[0] + m) * bytesPerPixel;

                for(b = 0; b < bytesPerPixel; b++)
                {
                    si = st[1] + b;
                    di = dt[1] + b;

                    pTargetBuf[si] = pBlock[di];
                }
            }
        }

        pBlock += (blockHeight * blockWidth * bytesPerPixel);
    }
}


void conv_compact_to_block(uint8_t  *pSourceBuf,
                           uint32_t sourceWidth,
                           uint32_t sourceHeight,
                           uint32_t channelsPerPixel,
                           uint32_t compactWidth,
                           uint32_t compactHeight,
                           uint32_t bitsPerChannel,
                           uint8_t  *pTargetBuf)
{
    uint32_t dstSize = 0;
    uint32_t srcSize = 0;
    uint32_t msb     = 0;
    uint32_t lsb     = 0;
    uint32_t inner   = 0;
    uint32_t st      = 0;
    uint32_t dt      = 0;
    uint32_t si;
    uint32_t di;

    dstSize = sourceWidth * sourceHeight * channelsPerPixel;
    srcSize = (dstSize * bitsPerChannel + 7) / 8;
    msb     = compactWidth * compactHeight;
    lsb     = (msb * (bitsPerChannel - 8) + 7) / 8;

    for (st = 0; st < srcSize; st += lsb + msb)
    {
        for (inner = 0; inner < msb; inner++)
        {
            si = st + lsb + inner;
            di = dt + inner;

            pTargetBuf[di] = pSourceBuf[si];
        }

        dt += msb;
    }
}


DP_STATUS_ENUM utilWriteBMP(const char    *pFileName,
                            void          *pInput0,
                            void          *pInput1,
                            void          *pInput2,
                            DpColorFormat colorType,
                            int32_t       srcWidth,
                            int32_t       srcHeight,
                            int32_t       srcYPitch,
                            int32_t       srcUVPitch)
{
    FILE    *pFile;
    uint8_t *pPixels0;
    uint8_t *pPixels1;
    uint8_t *pPixels2;
    int32_t temp;
    int32_t offset;
    int32_t depth;
    int32_t padded;
    int32_t index;
    int32_t yCoord;
    int32_t xCoord;
    int32_t YValue0;
    int32_t YValue1;
    int32_t UValue;
    int32_t VValue;
    int32_t RValue;
    int32_t GValue;
    int32_t BValue;
    uint8_t *pYTemp;
    uint8_t *pUVTemp;

    if ((void*)-1L == pInput0)
    {
        DPLOGE("invalid buffer %p to read\n", pInput0);
        return DP_STATUS_INVALID_PARAX;
    }

    pFile = fopen(pFileName, "wb");
    if (NULL == pFile)
    {
        DPLOGE("cannot create file %s to write: %s\n", pFileName, strerror(errno));
        return DP_STATUS_INVALID_FILE;
    }

    // Setup input buffer
    pPixels0 = (uint8_t*)pInput0;
    pPixels1 = (uint8_t*)pInput1;
    pPixels2 = (uint8_t*)pInput2;

    offset = 0x36;
    depth  = (DP_COLOR_GET_HW_FORMAT(colorType) == 2 || DP_COLOR_GET_HW_FORMAT(colorType) == 3) ? 32 : 24;
    padded = (4 - ((srcWidth * (depth >> 3)) % 4)) & 3;

    fputc('B', pFile);
    fputc('M', pFile);

    temp = srcWidth * srcHeight * (depth >> 3) +
           padded * srcHeight + offset;
    fwrite(&temp, sizeof(int32_t), 1, pFile);  // file size

    fputc(0x00, pFile);
    fputc(0x00, pFile);
    fputc(0x00, pFile);
    fputc(0x00, pFile);

    fwrite(&(offset), sizeof(int32_t), 1, pFile);

    //offset - already read bits : 54 - 14 =40
    fputc(0x28, pFile);  // header size
    fputc(0x00, pFile);
    fputc(0x00, pFile);
    fputc(0x00, pFile);

    //srcWidth and srcHeight
    fwrite(&(srcWidth), sizeof(int), 1, pFile);
    fwrite(&(srcHeight), sizeof(int), 1, pFile);

    //reserved
    fputc(0x01, pFile);
    fputc(0x00, pFile);

    //24 or 32 bit bitmap
    fwrite(&(depth), sizeof(int16_t), 1, pFile);

    fputc(0x00, pFile);
    fputc(0x00, pFile);
    fputc(0x00, pFile);
    fputc(0x00, pFile);

    //total pixel value = w * h * (depth >> 3)
    temp = srcWidth * srcHeight * (depth >> 3) + padded * srcHeight;
    fwrite(&temp, sizeof(int32_t), 1, pFile);

    for(index = 16;  index > 0;  index--)
    {
        fputc(0x00, pFile);
    }

    pYTemp  = NULL;
    pUVTemp = NULL;

    // write pixel value
    switch (colorType)
    {
        case DP_COLOR_BAYER8:
        case DP_COLOR_BAYER10:
        case DP_COLOR_BAYER12:
            // Not supported yet.
            break;
        case DP_COLOR_RGB565:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = *((uint16_t*)&(pPixels0[yCoord * srcYPitch + xCoord * 2]));

                    RValue = RGB565_TO_RGB888_R(YValue0);
                    GValue = RGB565_TO_RGB888_G(YValue0);
                    BValue = RGB565_TO_RGB888_B(YValue0);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_BGR565:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = *((uint16_t*)&(pPixels0[yCoord * srcYPitch + xCoord * 2]));

                    RValue = BGR565_TO_RGB888_R(YValue0);
                    GValue = BGR565_TO_RGB888_G(YValue0);
                    BValue = BGR565_TO_RGB888_B(YValue0);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_RGB888:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 3 + 2], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 3 + 1], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 3 + 0], pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_BGR888:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 3 + 0], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 3 + 1], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 3 + 2], pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_RGBA8888:
        //case DP_COLOR_RGBX8888:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 2], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 1], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 0], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 3], pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_ARGB8888:
        //case DP_COLOR_XRGB8888:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 3], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 2], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 1], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 0], pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_ABGR8888:
        //case DP_COLOR_XBGR8888:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 1], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 2], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 3], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 0], pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_BGRA8888:
        //case DP_COLOR_BGRX8888:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 0], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 1], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 2], pFile);
                    fputc(pPixels0[yCoord * srcYPitch + xCoord * 4 + 3], pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_I420:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[(yCoord >> 0) * srcYPitch  + (xCoord >> 0)];
                    UValue  = pPixels1[(yCoord >> 1) * srcUVPitch + (xCoord >> 1)];
                    VValue  = pPixels2[(yCoord >> 1) * srcUVPitch + (xCoord >> 1)];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_YV12:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[(yCoord >> 0) * srcYPitch  + (xCoord >> 0)];
                    UValue  = pPixels2[(yCoord >> 1) * srcUVPitch + (xCoord >> 1)];
                    VValue  = pPixels1[(yCoord >> 1) * srcUVPitch + (xCoord >> 1)];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_NV12:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[(yCoord >> 0) * srcYPitch  + (xCoord >> 0) + 0];
                    UValue  = pPixels1[(yCoord >> 1) * srcUVPitch + (xCoord >> 1) * 2 + 0];
                    VValue  = pPixels1[(yCoord >> 1) * srcUVPitch + (xCoord >> 1) * 2 + 1];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_NV21:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[(yCoord >> 0) * srcYPitch  + (xCoord >> 0) + 0];
                    VValue  = pPixels1[(yCoord >> 1) * srcUVPitch + (xCoord >> 1) * 2 + 0];
                    UValue  = pPixels1[(yCoord >> 1) * srcUVPitch + (xCoord >> 1) * 2 + 1];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
         case DP_COLOR_I422:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch  + (xCoord >> 0)];
                    UValue  = pPixels1[yCoord * srcUVPitch + (xCoord >> 1)];
                    VValue  = pPixels2[yCoord * srcUVPitch + (xCoord >> 1)];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_YV16:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch  + (xCoord >> 0)];
                    UValue  = pPixels2[yCoord * srcUVPitch + (xCoord >> 1)];
                    VValue  = pPixels1[yCoord * srcUVPitch + (xCoord >> 1)];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_NV16:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch  + (xCoord >> 0) + 0];
                    UValue  = pPixels1[yCoord * srcUVPitch + (xCoord >> 1) * 2 + 0];
                    VValue  = pPixels1[yCoord * srcUVPitch + (xCoord >> 1) * 2 + 1];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_NV61:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch  + (xCoord >> 0) + 0];
                    VValue  = pPixels1[yCoord * srcUVPitch + (xCoord >> 1) * 2 + 0];
                    UValue  = pPixels1[yCoord * srcUVPitch + (xCoord >> 1) * 2 + 1];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_YUYV:
            pYTemp = (uint8_t*)malloc(srcWidth * srcHeight * sizeof(uint8_t) * 3);
            assert(NULL != pYTemp);

            for (yCoord = 0; yCoord <= (srcHeight - 1); yCoord += 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord += 2)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch + xCoord * 2 + 0];
                    UValue  = pPixels0[yCoord * srcYPitch + xCoord * 2 + 1];
                    YValue1 = pPixels0[yCoord * srcYPitch + xCoord * 2 + 2];
                    VValue  = pPixels0[yCoord * srcYPitch + xCoord * 2 + 3];

                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 5] = YValue1; //Y1
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 4] = UValue;  //U0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 3] = VValue;  //V0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 2] = YValue0; //Y0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 1] = UValue;  //U0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 0] = VValue;  //V0
                }
            }

            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 2];
                    UValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 1];
                    VValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 0];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_YVYU:
            pYTemp = (uint8_t*)malloc(srcWidth * srcHeight * sizeof(uint8_t) * 3);
            assert(NULL != pYTemp);

            for (yCoord = 0; yCoord <= (srcHeight - 1); yCoord += 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord += 2)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch + xCoord * 2 + 0];
                    VValue  = pPixels0[yCoord * srcYPitch + xCoord * 2 + 1];
                    YValue1 = pPixels0[yCoord * srcYPitch + xCoord * 2 + 2];
                    UValue  = pPixels0[yCoord * srcYPitch + xCoord * 2 + 3];

                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 5] = YValue1; //Y1
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 4] = UValue;  //U0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 3] = VValue;  //V0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 2] = YValue0; //Y0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 1] = UValue;  //U0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 0] = VValue;  //V0
                }
            }

            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 2];
                    UValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 1];
                    VValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 0];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_UYVY:
            pYTemp = (uint8_t*)malloc(srcWidth * srcHeight * sizeof(uint8_t) * 3);
            assert(NULL != pYTemp);

            for (yCoord = 0; yCoord <= (srcHeight - 1); yCoord += 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord += 2)
                {
                    UValue  = pPixels0[yCoord * srcYPitch + xCoord * 2 + 0];
                    YValue0 = pPixels0[yCoord * srcYPitch + xCoord * 2 + 1];
                    VValue  = pPixels0[yCoord * srcYPitch + xCoord * 2 + 2];
                    YValue1 = pPixels0[yCoord * srcYPitch + xCoord * 2 + 3];

                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 5] = YValue1; //Y1
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 4] = UValue;  //U0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 3] = VValue;  //V0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 2] = YValue0; //Y0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 1] = UValue;  //U0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 0] = VValue;  //V0
                }
            }

            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 2];
                    UValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 1];
                    VValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 0];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_VYUY:
            pYTemp = (uint8_t*)malloc(srcWidth * srcHeight * sizeof(uint8_t) * 3);
            assert(NULL != pYTemp);

            for (yCoord = 0; yCoord <= (srcHeight - 1); yCoord += 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord += 2)
                {
                    VValue  = pPixels0[yCoord * srcYPitch + xCoord * 2 + 0];
                    YValue0 = pPixels0[yCoord * srcYPitch + xCoord * 2 + 1];
                    UValue  = pPixels0[yCoord * srcYPitch + xCoord * 2 + 2];
                    YValue1 = pPixels0[yCoord * srcYPitch + xCoord * 2 + 3];

                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 5] = YValue1; //Y1
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 4] = UValue;  //U0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 3] = VValue;  //V0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 2] = YValue0; //Y0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 1] = UValue;  //U0
                    pYTemp[(yCoord * srcWidth + xCoord) * 3 + 0] = VValue;  //V0
                }
            }

            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 2];
                    UValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 1];
                    VValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 0];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_I444:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch + xCoord];
                    UValue  = pPixels1[yCoord * srcYPitch + xCoord];
                    VValue  = pPixels2[yCoord * srcYPitch + xCoord];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_NV24:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch  + xCoord + 0];
                    UValue  = pPixels1[yCoord * srcUVPitch + xCoord * 2 + 0];
                    VValue  = pPixels1[yCoord * srcUVPitch + xCoord * 2 + 1];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_NV42:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch  + xCoord + 0];
                    VValue  = pPixels1[yCoord * srcUVPitch + xCoord * 2 + 0];
                    UValue  = pPixels1[yCoord * srcUVPitch + xCoord * 2 + 1];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_IYU2:
            for (yCoord = srcYPitch * (srcHeight - 1); yCoord >= 0; yCoord -= srcYPitch)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    UValue  = pPixels0[(yCoord + xCoord * 3) + 1];
                    VValue  = pPixels0[(yCoord + xCoord * 3) + 0];
                    YValue0 = pPixels0[(yCoord + xCoord * 3) + 2];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_GREY:
            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pPixels0[yCoord * srcYPitch + xCoord];
                    UValue  = 128;
                    VValue  = 128;

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_420_BLKP:
            pYTemp = (uint8_t*)malloc(srcWidth * srcHeight * sizeof(uint8_t));
            assert(NULL != pYTemp);

            pUVTemp = (uint8_t*)malloc(srcWidth * (srcHeight >> 1) * sizeof(uint8_t));
            assert(NULL != pUVTemp);

            conv_block_to_scan(pPixels0,
                               16,
                               32,
                               1,
                               srcWidth,
                               srcHeight,
                               pYTemp);

            conv_block_to_scan(pPixels1,
                               8,
                               16,
                               2,
                               srcWidth  >> 1,
                               srcHeight >> 1,
                               pUVTemp);

            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pYTemp[(yCoord >> 0)  * srcWidth + (xCoord >> 0) + 0];
                    UValue  = pUVTemp[(yCoord >> 1) * srcWidth + (xCoord >> 1) * 2 + 0];
                    VValue  = pUVTemp[(yCoord >> 1) * srcWidth + (xCoord >> 1) * 2 + 1];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_420_BLKI:
            pYTemp = (uint8_t*)malloc(srcWidth * srcHeight * sizeof(uint8_t));
            assert(NULL != pYTemp);

            pUVTemp = (uint8_t*)malloc(srcWidth * (srcHeight >> 1) * sizeof(uint8_t));
            assert(NULL != pUVTemp);

            conv_field_to_frame(pPixels0,
                                srcWidth,
                                srcHeight,
                                16,
                                32,
                                1,
                                pYTemp);

            conv_block_to_scan(pYTemp,
                               16,
                               32,
                               1,
                               srcWidth,
                               srcHeight,
                               pYTemp);

            conv_field_to_frame(pPixels1,
                                srcWidth  >> 1,
                                srcHeight >> 1,
                                8,
                                16,
                                2,
                                pUVTemp);

            conv_block_to_scan(pUVTemp,
                               8,
                               16,
                               2,
                               srcWidth  >> 1,
                               srcHeight >> 1,
                               pUVTemp);

            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pYTemp[(yCoord >> 0)  * srcWidth + (xCoord >> 0) + 0];
                    UValue  = pUVTemp[(yCoord >> 1) * srcWidth + (xCoord >> 1) * 2 + 0];
                    VValue  = pUVTemp[(yCoord >> 1) * srcWidth + (xCoord >> 1) * 2 + 1];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_422_BLKP:
            pUVTemp = (uint8_t*)malloc(srcWidth * srcHeight * sizeof(uint8_t) * 2);
            assert(NULL != pUVTemp);

            conv_block_to_scan(pPixels0,
                               8,
                               32,
                               2,
                               srcWidth,
                               srcHeight,
                               pUVTemp);

            pYTemp = (uint8_t*)malloc(srcWidth * srcHeight * sizeof(uint8_t) * 3);
            assert(NULL != pYTemp);

            for(index = 0; index < (srcWidth * srcHeight); index += 2)
            {
                UValue  = pUVTemp[index * 2 + 0];
                YValue0 = pUVTemp[index * 2 + 1];
                VValue  = pUVTemp[index * 2 + 2];
                YValue1 = pUVTemp[index * 2 + 3];

                pYTemp[index * 3 + 5] = YValue1; //Y1
                pYTemp[index * 3 + 4] = UValue;  //U0
                pYTemp[index * 3 + 3] = VValue;  //V0
                pYTemp[index * 3 + 2] = YValue0; //Y0
                pYTemp[index * 3 + 1] = UValue;  //U0
                pYTemp[index * 3 + 0] = VValue;  //V0
            }

            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 2];
                    UValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 1];
                    VValue  = pYTemp[(yCoord * srcWidth + xCoord) * 3 + 0];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        case DP_COLOR_420_BLKP_10_H:
        case DP_COLOR_420_BLKP_10_V:
            pYTemp = (uint8_t*)malloc(srcWidth * srcHeight * sizeof(uint8_t));
            assert(NULL != pYTemp);

            pUVTemp = (uint8_t*)malloc(srcWidth * (srcHeight >> 1) * sizeof(uint8_t));
            assert(NULL != pUVTemp);

            conv_compact_to_block(pPixels0,
                                  srcWidth,
                                  srcHeight,
                                  1,
                                  16,
                                  4,
                                  10,
                                  pYTemp);

            conv_block_to_scan(pYTemp,
                               16,
                               32,
                               1,
                               srcWidth,
                               srcHeight,
                               pYTemp);

            conv_compact_to_block(pPixels1,
                                  srcWidth  >> 1,
                                  srcHeight >> 1,
                                  2,
                                  16,
                                  4,
                                  10,
                                  pUVTemp);

            conv_block_to_scan(pUVTemp,
                               8,
                               16,
                               2,
                               srcWidth  >> 1,
                               srcHeight >> 1,
                               pUVTemp);

            for (yCoord = (srcHeight - 1); yCoord >= 0; yCoord -= 1)
            {
                for (xCoord = 0; xCoord < srcWidth; xCoord++)
                {
                    YValue0 = pYTemp[(yCoord >> 0)  * srcWidth + (xCoord >> 0) + 0];
                    UValue  = pUVTemp[(yCoord >> 1) * srcWidth + (xCoord >> 1) * 2 + 0];
                    VValue  = pUVTemp[(yCoord >> 1) * srcWidth + (xCoord >> 1) * 2 + 1];

                    RValue = YUV_TO_RGB888_R(YValue0, UValue, VValue);
                    GValue = YUV_TO_RGB888_G(YValue0, UValue, VValue);
                    BValue = YUV_TO_RGB888_B(YValue0, UValue, VValue);

                    fputc(BValue, pFile);
                    fputc(GValue, pFile);
                    fputc(RValue, pFile);
                }

                for(index = padded;  index > 0;  index--)
                {
                    fputc(0x00, pFile);
                }
            }
            break;
        default:
            DPLOGE("DpWriteBMP: Unsupported color format\n");
            break;
    }

    if (NULL != pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    free(pYTemp);
    free(pUVTemp);

    return DP_STATUS_RETURN_SUCCESS;
}
