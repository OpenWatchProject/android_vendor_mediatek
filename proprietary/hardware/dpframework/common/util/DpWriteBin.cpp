#include "DpWriteBin.h"
#include "DpPortAdapt.h"
#include "DpLogger.h"
#include "errno.h"

DP_STATUS_ENUM utilWriteBin(const char    *pDstFile,
                            void          *pOutBuf0,
                            void          *pOutBuf1,
                            void          *pOutBuf2,
                            DpColorFormat srcFormat,
                            int32_t       srcWidth,
                            int32_t       srcHeight,
                            int32_t       srcYPitch,
                            int32_t       srcUVPitch)
{
    FILE    *pFile;
    int32_t plane;
    int32_t UVSize;

    if ((void*)-1L == pOutBuf0)
    {
        DPLOGE("invalid buffer %p to read\n", pOutBuf0);
        return DP_STATUS_INVALID_PARAX;
    }

    pFile = fopen(pDstFile, "wb");
    if (NULL == pFile)
    {
        DPLOGE("cannot create file %s to write: %s\n", pDstFile, strerror(errno));
        return DP_STATUS_INVALID_FILE;
    }

    plane = DP_COLOR_GET_PLANE_COUNT(srcFormat);
    if (DP_COLOR_GET_BLOCK_MODE(srcFormat))
    {
        if (1 == plane)
        {
            fwrite(pOutBuf0, DP_COLOR_GET_MIN_Y_SIZE(srcFormat, srcWidth, srcHeight), 1, pFile);
        }
        else if (2 == plane)
        {
            fwrite(pOutBuf0, DP_COLOR_GET_MIN_Y_SIZE(srcFormat, srcWidth, srcHeight), 1, pFile);
            fwrite(pOutBuf1, DP_COLOR_GET_MIN_UV_SIZE(srcFormat, srcWidth, srcHeight), 1, pFile);
        }
    }
    else
    {
        if (1 == plane)
        {
            fwrite(pOutBuf0, srcYPitch * srcHeight, 1, pFile);
        }
        else if (2 == plane)
        {
            fwrite(pOutBuf0, srcYPitch  * srcHeight, 1, pFile);

            UVSize = (srcUVPitch * srcHeight) >> DP_COLOR_GET_V_SUBSAMPLE(srcFormat);

            fwrite(pOutBuf1, UVSize, 1, pFile);
        }
        else if (3 == plane)
        {
            fwrite(pOutBuf0, srcYPitch  * srcHeight, 1, pFile);

            UVSize = (srcUVPitch * srcHeight) >> DP_COLOR_GET_V_SUBSAMPLE(srcFormat);

            fwrite(pOutBuf1, UVSize, 1, pFile);
            fwrite(pOutBuf2, UVSize, 1, pFile);
        }
    }

    if (NULL != pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    return DP_STATUS_RETURN_SUCCESS;
}
