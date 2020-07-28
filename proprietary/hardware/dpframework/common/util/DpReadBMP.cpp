#include "DpReadBMP.h"
#include "DpColorConv.h"
#include "DpLogger.h"

#define UTIL_MAX_READ_BMP_WIDTH         (10000)
#define UTIL_MAX_READ_BMP_HEIGHT        (10000)
#define UTIL_MAX_TEMP_BUFFER_SIZE       (1U * 1024 * 1024 * 1024)
#define UTIL_LENGTH_PAD(length, base)   ((base == 0)? 0: (((length + base - 1) / base) * base))

static DP_STATUS_ENUM decodeBMP(const char *pFileName,
                                uint8_t    *pAlphaBuf,
                                uint8_t    *pRedBuf,
                                uint8_t    *pGreenBuf,
                                uint8_t    *pBlueBuf,
                                int32_t    buffSize,
                                int32_t    *pWidth,
                                int32_t    *pHeight)
{
    FILE    *pFile;
    int32_t offset;
    int32_t width;
    int32_t height;
    int16_t depth;
    int32_t padded;
    uint32_t tempSize;
    uint8_t *pTemp;
    uint8_t *pLine;
    int32_t yCoord;
    int32_t xCoord;

    pFile = fopen(pFileName, "rb");
    if (NULL == pFile)
    {
        DPLOGE("cannot open file %s to read\n", pFileName);
        return DP_STATUS_INVALID_FILE;
    }

    if ((fgetc(pFile) != 'B') || (fgetc(pFile) != 'M'))
    {
        fclose(pFile);
        DPLOGE("%s is not a bitmap file\n", pFileName);
        return DP_STATUS_INVALID_FILE;
    }

    fseek(pFile, 8, 1);

    fread(&offset, sizeof(int), 1, pFile);
    fseek(pFile, 4, 1);

    fread(&width,  sizeof(int32_t), 1, pFile);
    fread(&height, sizeof(int32_t), 1, pFile);

    if ((width > UTIL_MAX_READ_BMP_WIDTH) ||
        (height > UTIL_MAX_READ_BMP_HEIGHT))
    {
        fclose(pFile);
        DPLOGE("bmp width(%d) or height(%d) are too large to read\n", width, height);
        return DP_STATUS_INVALID_PARAX;
    }

    if (buffSize < (width * height))
    {
        fclose(pFile);
        DPLOGE("bmp width(%d) and height(%d) are not matched with buffer size(%d)\n", width, height, buffSize);
        return DP_STATUS_INVALID_PARAX;
    }

    fseek(pFile, 2, 1);

    fread(&depth, sizeof(int16_t), 1,  pFile);

    if ((24 != depth) && (32 != depth))
    {
        fclose(pFile);
        DPLOGE("bmp pixel size %d-bit is not supported\n", depth);
        return DP_STATUS_INVALID_PARAX;
    }

    // Skip remain header bytes
    fseek(pFile, 24, 1);

    padded = (4 - ((width * (depth >> 3)) % 4)) & 3;
    tempSize = width * height * (depth >> 3) + padded * height;
    if (tempSize > UTIL_MAX_TEMP_BUFFER_SIZE)
    {
        fclose(pFile);
        DPLOGE("bmp size(%d) is too large to read\n", tempSize);
        return DP_STATUS_INVALID_PARAX;
    }

    pTemp = (uint8_t*)malloc(tempSize);
    if (NULL == pTemp)
    {
        fclose(pFile);
        DPLOGE("Allocate buffer failed\n");
        return DP_STATUS_OUT_OF_MEMORY;
    }

    fread(pTemp, 1, tempSize, pFile);

    pLine = pTemp;
    for (yCoord = width * (height - 1); yCoord >= 0; yCoord -= width)
    {
        for (xCoord = 0; xCoord < width; xCoord++)
        {
            pBlueBuf[yCoord + xCoord]  = *pLine++;
            pGreenBuf[yCoord + xCoord] = *pLine++;
            pRedBuf[yCoord + xCoord]   = *pLine++;

            if (32 == depth)
            {
                pAlphaBuf[yCoord + xCoord] = *pLine++;
            }
            else
            {
                pAlphaBuf[yCoord + xCoord] = 0xFF;
            }
        }

        // Skip padded bytes
        pLine += padded;
    }

    if (NULL != pWidth)
    {
        *pWidth = width;
    }

    if (NULL != pHeight)
    {
        *pHeight = height;
    }

    free(pTemp);

    if (NULL != pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


static void horizontal_down_sample(uint8_t *pSource,
                                   int32_t srcWidth,
                                   int32_t srcHeight,
                                   uint8_t *pTarget,
                                   int32_t *pOutWidth,
                                   int32_t *pOutHeight)
{
    uint8_t  *pTemp;
    uint8_t  *pBase;
    int32_t  index;
    uint32_t pixel[2];

    pBase = (uint8_t*)malloc((srcWidth >> 1) * srcHeight);
    assert(NULL != pBase);

    pTemp = pBase;
    for(index = 0;  index < ((srcWidth >> 1) * srcHeight); index++)
    {
        pixel[0] = pSource[index * 2 + 0];
        pixel[1] = pSource[index * 2 + 1];
        *pTemp++ = (pixel[0] + pixel[1]) >> 1;
    }

    memcpy(pTarget, pBase, (srcWidth >> 1) * srcHeight);

    *pOutWidth  = srcWidth >> 1;
    *pOutHeight = srcHeight;

    free(pBase);
}


static void vertical_down_sample(uint8_t *pSource,
                                 int32_t srcWidth,
                                 int32_t srcHeight,
                                 uint8_t *pTarget,
                                 int32_t *pOutWidth,
                                 int32_t *pOutHeight)
{
    uint8_t *pBase;
    uint8_t *pTemp;
    int32_t  index;
    int32_t  inner;
    uint32_t pixel[2];

    pBase = (uint8_t*)malloc(srcWidth * (srcHeight >> 1));
    assert(NULL != pBase);

    pTemp = pBase;
    for(index = 0; index < (srcHeight / 2);  index++)
    {
        for(inner = 0; inner < srcWidth; inner++)
        {
            pixel[0] = pSource[(index * 2 + 0) * srcWidth + inner];
            pixel[1] = pSource[(index * 2 + 1) * srcWidth + inner];
            *pTemp++ = (pixel[0] + pixel[1]) >> 1;
        }
    }

    memcpy(pTarget, pBase, srcWidth * (srcHeight >> 1));

    *pOutWidth  = srcWidth;
    *pOutHeight = srcHeight >> 1;

    free(pBase);
}


static void RGB888_to_YUV444(uint8_t *pRedBuf,
                             uint8_t *pGreenBuf,
                             uint8_t *pBlueBuf,
                             int32_t width,
                             int32_t height,
                             uint8_t *pYTarBuf,
                             uint8_t *pUTarBuf,
                             uint8_t *pVTarBuf)
{
    int32_t  index;
    int32_t  red;
    int32_t  green;
    int32_t  blue;
    int32_t  YVal;
    int32_t  UVal;
    int32_t  VVal;

    for(index = 0;  index < (width * height); index++)
    {
        red   = pRedBuf[index];
        green = pGreenBuf[index];
        blue  = pBlueBuf[index];

        YVal  = RGB888_TO_YUV_Y(red, green, blue);
        UVal  = RGB888_TO_YUV_U(red, green, blue);
        VVal  = RGB888_TO_YUV_V(red, green, blue);

        pYTarBuf[index] = (uint8_t)YVal;
        pUTarBuf[index] = (uint8_t)UVal;
        pVTarBuf[index] = (uint8_t)VVal;
    }
}


static void merge_UV_channel(uint8_t *pSrcBuf0,
                             uint8_t *pSrcBuf1,
                             int32_t srcWidth,
                             int32_t srcHeight,
                             int32_t order,
                             uint8_t *pOutBuf,
                             int32_t *pOutWidth,
                             int32_t *pOutHeight)
{
    uint8_t *pTemp;
    int32_t index;

    pTemp = (uint8_t*)malloc(srcWidth * srcHeight * 2);
    if (NULL == pTemp)
    {
        DPLOGE("Allocate temp buffer failed\n");
        assert(0);
    }

    for(index = 0; index < (srcWidth * srcHeight); index++)
    {
        if (0 == order)
        {
            pTemp[index * 2 + 1] = pSrcBuf1[index];
            pTemp[index * 2 + 0] = pSrcBuf0[index];
        }
        else
        {
            pTemp[index * 2 + 1] = pSrcBuf0[index];
            pTemp[index * 2 + 0] = pSrcBuf1[index];
        }
    }

    memcpy(pOutBuf, pTemp, srcWidth * srcHeight * 2);

    *pOutWidth  = srcWidth;
    *pOutHeight = srcHeight;

    free(pTemp);
}


static void conv_scan_to_block(uint8_t  *pSourceBuf,
                               int32_t  sourceWidth,
                               int32_t  sourceHeight,
                               int32_t  blockWidth,
                               int32_t  blockHeight,
                               int32_t  bytesPerPixel,
                               uint8_t  *pTargetBuf,
                               int32_t  *pOutWidth,
                               int32_t  *pOutHeight)
{
    uint8_t  *pTemp;
    int32_t  index;
    int32_t  inner;
    int32_t  count;
    uint32_t st[2];
    uint32_t dt[2];
    uint32_t dx[2];
    uint32_t dy[2];
    uint32_t srcIndex;
    uint32_t dstIndex;
    uint32_t dstWidth;
    uint32_t dstHeight;

    dstWidth  = UTIL_LENGTH_PAD(sourceWidth,  blockWidth);
    dstHeight = UTIL_LENGTH_PAD(sourceHeight, blockHeight);

    pTemp = (uint8_t*)malloc(dstWidth * dstHeight * bytesPerPixel * sizeof(uint8_t));
    assert(NULL != pTemp);

    memset(pTemp, 0x0, dstWidth * dstHeight * bytesPerPixel);

    for(index = 0; index < sourceHeight; index++)
    {
        st[0] = index * sourceWidth;

        dy[0] = index / blockHeight;
        dy[1] = index % blockHeight;

        dt[0] = dy[0] * blockHeight * dstWidth +
                dy[1] * blockWidth;

        for(inner = 0; inner < sourceWidth; inner++)
        {
            st[1] = (st[0] + inner) * bytesPerPixel;

            dx[0] = inner / blockWidth;
            dx[1] = inner % blockWidth;

            dt[1] = (dt[0] + dx[0] * blockWidth * blockHeight + dx[1]) * bytesPerPixel;

            for(count = 0; count < bytesPerPixel; count++)
            {
                srcIndex = st[1] + count;
                dstIndex = dt[1] + count;

                pTemp[dstIndex] = pSourceBuf[srcIndex];
            }
        }
    }

    memcpy(pTargetBuf, pTemp, dstWidth * dstHeight * bytesPerPixel);

    *pOutWidth  = dstWidth;
    *pOutHeight = dstHeight;

    free(pTemp);
}


static void conv_frame_to_field(uint8_t  *pSourceBuf,
                                int32_t  sourceWidth,
                                int32_t  sourceHeight,
                                int32_t  blockWidth,
                                int32_t  blockHeight,
                                int32_t  bytesPerPixel)
{
    uint8_t  *pTemp;
    int32_t s_0    = 0;
    int32_t s_1    = 0;
    int32_t s_2    = 0;
    int32_t m      = 0;
    int32_t n      = 0;
    int32_t i      = 0;
    int32_t j      = 0;
    int32_t b      = 0;
    uint32_t st[2] = {0};
    uint32_t dt[2] = {0};
    uint32_t dy[2] = {0};
    uint32_t si    = 0;
    uint32_t di    = 0;

    s_0 = sourceWidth * sourceHeight;
    s_1 = blockWidth  * blockHeight;
    s_2 = s_1 * bytesPerPixel;

    pTemp = (uint8_t*)malloc(s_2);
    assert(NULL != pTemp);

    memset(pTemp, 0, s_2);

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

                    pTemp[di] = pSourceBuf[si];
                }
            }
        }

        for(i = 0; i < s_2; i++)
        {
            di = j * bytesPerPixel + i;
            pSourceBuf[di] = pTemp[i];
        }
    }

    free(pTemp);
}


static void conv_block_to_compact(uint8_t  *pSourceBuf,
                                  int32_t  sourceWidth,
                                  int32_t  sourceHeight,
                                  int32_t  channelsPerPixel,
                                  int32_t  compactWidth,
                                  int32_t  compactHeight,
                                  int32_t  bitsPerChannel,
                                  int32_t  tile,
                                  uint8_t  *pTargetBuf)
{
    uint8_t  *pTemp;
    int32_t  srcSize = 0;
    int32_t  dstSize = 0;
    int32_t  msb     = 0;
    int32_t  lsb     = 0;
    uint8_t  mask    = 0;
    int32_t  inner   = 0;
    int32_t st       = 0;
    int32_t dt       = 0;
    uint32_t si;
    uint32_t di[2];
    uint32_t dl[2];
    uint32_t dx;
    uint32_t dy;

    srcSize = sourceWidth * sourceHeight * channelsPerPixel;
    dstSize = (srcSize * bitsPerChannel + 7) / 8;
    msb     = compactWidth * compactHeight;
    lsb     = (msb * (bitsPerChannel - 8) + 7) / 8;
    mask    = (uint8_t)(((int32_t)1 << (bitsPerChannel - 8)) - 1) << (16 - bitsPerChannel);

    pTemp = (uint8_t*)malloc(dstSize * sizeof(uint8_t));
    assert(NULL != pTemp);

    memset(pTemp, 0x0, dstSize);

    for (st = 0; st < srcSize; st += msb)
    {
        for (inner = 0; inner < msb; inner++)
        {
            si = st + inner;
            di[0] = dt + lsb + inner;

            if (0 == tile)
            {
                di[1] = (msb - inner - 1) * (bitsPerChannel - 8);
            }
            else
            {
                dy = inner / compactWidth;
                dx = inner % compactWidth;
                di[1] = dx * compactHeight + dy;

                di[1] = (msb - di[1] - 1) * (bitsPerChannel - 8);
            }

            dl[0] = di[1] / 8 + dt;
            dl[1] = di[1] % 8;

            pTemp[di[0]] = pSourceBuf[si];
            pTemp[dl[0]] |= (pSourceBuf[si] & mask) >> dl[1];
        }

        dt += lsb + msb;
    }

    memcpy(pTargetBuf, pTemp, dstSize);

    free(pTemp);
}


DP_STATUS_ENUM utilReadBMP(const char    *pSrcFile,
                           uint8_t       *pOutBuf0,
                           uint8_t       *pOutBuf1,
                           uint8_t       *pOutBuf2,
                           DpColorFormat outFormat,
                           int32_t       bufWidth,
                           int32_t       bufHeight,
                           int32_t       bufPitch)
{
    uint8_t  *pSrcBufA;
    uint8_t  *pSrcBuf0;
    uint8_t  *pSrcBuf1;
    uint8_t  *pSrcBuf2;
    int32_t  srcWidth;
    int32_t  srcHeight;
    int32_t  UVWidth;
    int32_t  UVHeight;
    int32_t  index;

    pSrcBufA = (uint8_t*)malloc(bufWidth * bufHeight * sizeof(uint8_t));
    if (NULL == pSrcBufA)
    {
        DPLOGE("Allocate source buffer A failed\n");
        return DP_STATUS_OUT_OF_MEMORY;
    }

    pSrcBuf0 = (uint8_t*)malloc(bufWidth * bufHeight * sizeof(uint8_t));
    if (NULL == pSrcBuf0)
    {
        DPLOGE("Allocate source buffer 0 failed\n");
        return DP_STATUS_OUT_OF_MEMORY;
    }

    // May be used to merge U&V pixels
    pSrcBuf1 = (uint8_t*)malloc(bufWidth * bufHeight * sizeof(uint8_t) * 2);
    if (NULL == pSrcBuf1)
    {
        DPLOGE("Allocate source buffer 1 failed\n");
        return DP_STATUS_OUT_OF_MEMORY;
    }

    pSrcBuf2 = (uint8_t*)malloc(bufWidth * bufHeight * sizeof(uint8_t));
    if (NULL == pSrcBuf2)
    {
        DPLOGE("Allocate source buffer 2 failed\n");
        return DP_STATUS_OUT_OF_MEMORY;
    }

    decodeBMP(pSrcFile,
              pSrcBufA,
              pSrcBuf0,
              pSrcBuf1,
              pSrcBuf2,
              bufWidth * bufHeight,
              &srcWidth,
              &srcHeight);

    if (srcWidth != bufWidth)
    {
        DPLOGE("Incorrect buffer width\n");
        return DP_STATUS_INVALID_PARAX;
    }

    if (srcHeight != bufHeight)
    {
        DPLOGE("Incorrect buffer height\n");
        return DP_STATUS_INVALID_PARAX;
    }

    UVWidth  = 0;
    UVHeight = 0;

    switch(outFormat)
    {
        case DP_COLOR_GREY:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            break;
        case DP_COLOR_RGB565:
            for (index = 0; index < (srcWidth * srcHeight); index++)
            {
                ((uint16_t*)pOutBuf0)[index] = RGB888_TO_RGB565(pSrcBuf0[index], pSrcBuf1[index], pSrcBuf2[index]);
            }
            break;
        case DP_COLOR_BGR565:
            for (index = 0; index < (srcWidth * srcHeight); index++)
            {
                ((uint16_t*)pOutBuf0)[index] = RGB888_TO_BGR565(pSrcBuf0[index], pSrcBuf1[index], pSrcBuf2[index]);
            }
            break;
        case DP_COLOR_RGB888:
            for (index = 0; index < (srcWidth * srcHeight); index++)
            {
                pOutBuf0[index * 3 + 0] = pSrcBuf0[index];
                pOutBuf0[index * 3 + 1] = pSrcBuf1[index];
                pOutBuf0[index * 3 + 2] = pSrcBuf2[index];
            }
            break;
        case DP_COLOR_BGR888:
            for (index = 0; index < (srcWidth * srcHeight); index++)
            {
                pOutBuf0[index * 3 + 0] = pSrcBuf2[index];
                pOutBuf0[index * 3 + 1] = pSrcBuf1[index];
                pOutBuf0[index * 3 + 2] = pSrcBuf0[index];
            }
            break;
        case DP_COLOR_ARGB8888:
        //case DP_COLOR_XRGB8888:
            for (index = 0; index < (srcWidth * srcHeight); index++)
            {
                pOutBuf0[index * 4 + 0] = pSrcBufA[index];
                pOutBuf0[index * 4 + 1] = pSrcBuf0[index];
                pOutBuf0[index * 4 + 2] = pSrcBuf1[index];
                pOutBuf0[index * 4 + 3] = pSrcBuf2[index];
            }
            break;
        case DP_COLOR_ABGR8888:
        //case DP_COLOR_XBGR8888:
            for (index = 0; index < (srcWidth * srcHeight); index++)
            {
                pOutBuf0[index * 4 + 0] = pSrcBufA[index];
                pOutBuf0[index * 4 + 1] = pSrcBuf2[index];
                pOutBuf0[index * 4 + 2] = pSrcBuf1[index];
                pOutBuf0[index * 4 + 3] = pSrcBuf0[index];
            }
            break;
        case DP_COLOR_RGBA8888:
        //case DP_COLOR_RGBX8888:
            for (index = 0; index < (srcWidth * srcHeight); index++)
            {
                pOutBuf0[index * 4 + 3] = pSrcBufA[index];
                pOutBuf0[index * 4 + 2] = pSrcBuf2[index];
                pOutBuf0[index * 4 + 1] = pSrcBuf1[index];
                pOutBuf0[index * 4 + 0] = pSrcBuf0[index];
            }
            break;
        case DP_COLOR_BGRA8888:
        //case DP_COLOR_BGRX8888:
            for (index = 0; index < (srcWidth * srcHeight); index++)
            {
                pOutBuf0[index * 4 + 3] = pSrcBufA[index];
                pOutBuf0[index * 4 + 2] = pSrcBuf0[index];
                pOutBuf0[index * 4 + 1] = pSrcBuf1[index];
                pOutBuf0[index * 4 + 0] = pSrcBuf2[index];
            }
            break;
        case DP_COLOR_I420:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf1,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf1,
                                 &UVWidth,
                                 &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                  srcWidth,
                                  srcHeight,
                                  pSrcBuf2,
                                  &UVWidth,
                                  &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf2,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf2,
                                 &UVWidth,
                                 &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf1, pSrcBuf1, UVWidth  * UVHeight);
            memcpy(pOutBuf2, pSrcBuf2, UVWidth  * UVHeight);
            break;
        case DP_COLOR_YV12:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf1,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf1,
                                 &UVWidth,
                                 &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                  srcWidth,
                                  srcHeight,
                                  pSrcBuf2,
                                  &UVWidth,
                                  &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf2,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf2,
                                 &UVWidth,
                                 &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf2, pSrcBuf1, UVWidth  * UVHeight);
            memcpy(pOutBuf1, pSrcBuf2, UVWidth  * UVHeight);
            break;
        case DP_COLOR_420_BLKP:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf1,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf1,
                                 &UVWidth,
                                 &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf2,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf2,
                                 &UVWidth,
                                 &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            // Convert Y pixels into block
            conv_scan_to_block(pSrcBuf0,
                               srcWidth,
                               srcHeight,
                               16,
                               32,
                               1,
                               pOutBuf0,
                               &srcWidth,
                               &srcHeight);

            // Convert U&V pixels into block
            conv_scan_to_block(pSrcBuf1,
                               UVWidth,
                               UVHeight,
                               8,
                               16,
                               2,
                               pOutBuf1,
                               &UVWidth,
                               &UVHeight);
            break;
        case DP_COLOR_420_BLKI:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf1,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf1,
                                 &UVWidth,
                                 &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf2,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf2,
                                 &UVWidth,
                                 &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            // Convert Y pixels into block
            conv_scan_to_block(pSrcBuf0,
                               srcWidth,
                               srcHeight,
                               16,
                               32,
                               1,
                               pOutBuf0,
                               &srcWidth,
                               &srcHeight);

            conv_frame_to_field(pOutBuf0,
                                srcWidth,
                                srcHeight,
                                16,
                                32,
                                1);

            // Convert U&V pixels into block
            conv_scan_to_block(pSrcBuf1,
                               UVWidth,
                               UVHeight,
                               8,
                               16,
                               2,
                               pOutBuf1,
                               &UVWidth,
                               &UVHeight);

            conv_frame_to_field(pOutBuf1,
                                UVWidth,
                                UVHeight,
                                8,
                                16,
                                2);
            break;
        case DP_COLOR_420_BLKP_10_H:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf1,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf1,
                                 &UVWidth,
                                 &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf2,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf2,
                                 &UVWidth,
                                 &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            // Convert Y pixels into block
            conv_scan_to_block(pSrcBuf0,
                               srcWidth,
                               srcHeight,
                               16,
                               32,
                               1,
                               pOutBuf0,
                               &srcWidth,
                               &srcHeight);

            conv_block_to_compact(pOutBuf0,
                                  srcWidth,
                                  srcHeight,
                                  1,
                                  16,
                                  4,
                                  10,
                                  0,
                                  pOutBuf0);

            // Convert U&V pixels into block
            conv_scan_to_block(pSrcBuf1,
                               UVWidth,
                               UVHeight,
                               8,
                               16,
                               2,
                               pOutBuf1,
                               &UVWidth,
                               &UVHeight);

            conv_block_to_compact(pOutBuf1,
                                  UVWidth,
                                  UVHeight,
                                  2,
                                  16,
                                  4,
                                  10,
                                  0,
                                  pOutBuf1);
            break;
        case DP_COLOR_420_BLKP_10_V:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf1,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf1,
                                 &UVWidth,
                                 &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf2,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf2,
                                 &UVWidth,
                                 &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            // Convert Y pixels into block
            conv_scan_to_block(pSrcBuf0,
                               srcWidth,
                               srcHeight,
                               16,
                               32,
                               1,
                               pOutBuf0,
                               &srcWidth,
                               &srcHeight);

            conv_block_to_compact(pOutBuf0,
                                  srcWidth,
                                  srcHeight,
                                  1,
                                  16,
                                  4,
                                  10,
                                  1,
                                  pOutBuf0);

            // Convert U&V pixels into block
            conv_scan_to_block(pSrcBuf1,
                               UVWidth,
                               UVHeight,
                               8,
                               16,
                               2,
                               pOutBuf1,
                               &UVWidth,
                               &UVHeight);

            conv_block_to_compact(pOutBuf1,
                                  UVWidth,
                                  UVHeight,
                                  2,
                                  16,
                                  4,
                                  10,
                                  1,
                                  pOutBuf1);
            break;
        case DP_COLOR_NV12:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf1,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf1,
                                 &UVWidth,
                                 &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                  srcWidth,
                                  srcHeight,
                                  pSrcBuf2,
                                  &UVWidth,
                                  &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf2,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf2,
                                 &UVWidth,
                                 &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf1, pSrcBuf1, UVWidth  * UVHeight * 2);
            break;
        case DP_COLOR_NV21:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf1,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf1,
                                 &UVWidth,
                                 &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                  srcWidth,
                                  srcHeight,
                                  pSrcBuf2,
                                  &UVWidth,
                                  &UVHeight);

            // Down sample in vertical direction
            vertical_down_sample(pSrcBuf2,
                                 UVWidth,
                                 UVHeight,
                                 pSrcBuf2,
                                 &UVWidth,
                                 &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             1,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf1, pSrcBuf1, UVWidth  * UVHeight * 2);
            break;
        case DP_COLOR_I422:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf1, pSrcBuf1, UVWidth  * UVHeight);
            memcpy(pOutBuf2, pSrcBuf2, UVWidth  * UVHeight);
            break;
        case DP_COLOR_YV16:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf2, pSrcBuf1, UVWidth  * UVHeight);
            memcpy(pOutBuf1, pSrcBuf2, UVWidth  * UVHeight);
            break;
        case DP_COLOR_422_BLKP:
             // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            // Interleave Y with U&V pixels
            merge_UV_channel(pSrcBuf0,
                             pSrcBuf1,
                             srcWidth,
                             srcHeight,
                             1,
                             pOutBuf0,
                             &srcWidth,
                             &srcHeight);

            // Convert Y with U&V pixels into block
            conv_scan_to_block(pOutBuf0,
                               srcWidth,
                               srcHeight,
                               8,
                               32,
                               2,
                               pOutBuf0,
                               &srcWidth,
                               &srcHeight);
            break;
        case DP_COLOR_UYVY:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf0,
                             pSrcBuf1,
                             srcWidth,
                             srcHeight,
                             1,
                             pOutBuf0,
                             &srcWidth,
                             &srcHeight);
            break;
       case DP_COLOR_VYUY:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             1,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf0,
                             pSrcBuf1,
                             srcWidth,
                             srcHeight,
                             1,
                             pOutBuf0,
                             &srcWidth,
                             &srcHeight);
            break;
        case DP_COLOR_YUYV:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf0,
                             pSrcBuf1,
                             srcWidth,
                             srcHeight,
                             0,
                             pOutBuf0,
                             &srcWidth,
                             &srcHeight);
            break;
        case DP_COLOR_YVYU:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             1,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf0,
                             pSrcBuf1,
                             srcWidth,
                             srcHeight,
                             0,
                             pOutBuf0,
                             &srcWidth,
                             &srcHeight);
            break;
        case DP_COLOR_NV16:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf1, pSrcBuf1, UVWidth  * UVHeight * 2);
            break;
        case DP_COLOR_NV61:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf1,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf1,
                                   &UVWidth,
                                   &UVHeight);

            // Down sample in horizontal direction
            horizontal_down_sample(pSrcBuf2,
                                   srcWidth,
                                   srcHeight,
                                   pSrcBuf2,
                                   &UVWidth,
                                   &UVHeight);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             UVWidth,
                             UVHeight,
                             1,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf1, pSrcBuf1, UVWidth  * UVHeight * 2);
            break;
        case DP_COLOR_I444:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf1, pSrcBuf1, bufWidth * bufHeight);
            memcpy(pOutBuf2, pSrcBuf2, bufWidth * bufHeight);
            break;
        case DP_COLOR_NV24:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf1, pSrcBuf1, UVWidth  * UVHeight * 2);
            break;
        case DP_COLOR_NV42:
            // Convert RGB888 to YUV444
            RGB888_to_YUV444(pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             pSrcBuf0,
                             pSrcBuf1,
                             pSrcBuf2);

            // Interleave U&V pixels
            merge_UV_channel(pSrcBuf1,
                             pSrcBuf2,
                             srcWidth,
                             srcHeight,
                             0,
                             pSrcBuf1,
                             &UVWidth,
                             &UVHeight);

            memcpy(pOutBuf0, pSrcBuf0, bufWidth * bufHeight);
            memcpy(pOutBuf1, pSrcBuf1, UVWidth  * UVHeight * 2);
            break;
        default:
            DPLOGE("DpReadBMP: Unsupported color format\n");
            assert(0);
            break;
    }

    free(pSrcBufA);
    free(pSrcBuf0);
    free(pSrcBuf1);
    free(pSrcBuf2);

    return DP_STATUS_RETURN_SUCCESS;
}
