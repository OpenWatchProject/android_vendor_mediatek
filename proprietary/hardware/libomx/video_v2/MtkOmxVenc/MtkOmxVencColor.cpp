#include <signal.h>
#include <cutils/log.h>

#include <utils/Trace.h>
#include <utils/AndroidThreads.h>

#include "osal_utils.h"
#include "MtkOmxVenc.h"

#include <cutils/properties.h>
#include <sched.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVenc"

#include "OMX_IndexExt.h"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG

#define MTK_OMX_H263_ENCODER    "OMX.MTK.VIDEO.ENCODER.H263"
#define MTK_OMX_MPEG4_ENCODER   "OMX.MTK.VIDEO.ENCODER.MPEG4"
#define MTK_OMX_AVC_ENCODER     "OMX.MTK.VIDEO.ENCODER.AVC"
#define MTK_OMX_HEVC_ENCODER    "OMX.MTK.VIDEO.ENCODER.HEVC"
#define MTK_OMX_AVC_SEC_ENCODER "OMX.MTK.VIDEO.ENCODER.AVC.secure"
#define MTK_OMX_VP8_ENCODER     "OMX.MTK.VIDEO.ENCODER.VPX"

#define H264_MAX_BS_SIZE    1024*1024
#define HEVC_MAX_BS_SIZE    1024*1024
#define MP4_MAX_BS_SIZE     1024*1024
#define VP8_MAX_BS_SIZE     1024*1024

// Morris Yang 20120214 add for live effect recording [
#ifdef ANDROID_ICS
#include <ui/Rect.h>
//#include <ui/android_native_buffer.h> // for ICS
#include <android/native_window.h> // for JB
// #include <media/stagefright/HardwareAPI.h> // for ICS
#include <HardwareAPI.h> // for JB
//#include <media/stagefright/MetadataBufferType.h> // for ICS
#include <MetadataBufferType.h> // for JB

#include <hardware/gralloc.h>
#include <ui/gralloc_extra.h>
#endif
// ]
#define OMX_CHECK_DUMMY
#include "../../../omx/core/src/MtkOmxCore.h"

#include <poll.h>

#include <utils/Trace.h>
//#include <utils/AndroidThreads.h>

////////for fence in M0/////////////
#include <ui/Fence.h>
#include <media/IOMX.h>
///////////////////end///////////

const char *PixelFormatToString(unsigned int nPixelFormat, bool metamode);

int MtkOmxVenc::colorConvert(OMX_U8 *aInputBuf, OMX_U32 aInputSize, OMX_U8 *aOutputBuf, OMX_U32 aOutputSize)
{
    //config color convert src
    uint32_t srcWidth       = mInputPortDef.format.video.nFrameWidth;
    uint32_t srcHeight      = mInputPortDef.format.video.nFrameHeight;
    uint32_t srcStride      = 0;
    uint32_t srcUVStride    = 0;
    uint32_t srcSliceHeight = mInputPortDef.format.video.nSliceHeight;
    DpColorFormat srcFormat = DP_COLOR_RGBA8888;
    DpSecure srcSecure = DP_SECURE_NONE;



    DpRect srcRoi;
    srcRoi.x = 0;
    srcRoi.y = 0;
    srcRoi.w = mInputPortDef.format.video.nFrameWidth;
    srcRoi.h = mInputPortDef.format.video.nFrameHeight;

    uint8_t *srcYUVbufArray[3];
    uint8_t *srcYUVMVAbufArray[3];
    unsigned int srcYUVbufSizeArray[3];
    int ret = 0;

    if (mStoreMetaDataInBuffers)//meta mode
    {
        int ionfd, size, hSecHandle = 0;
        OMX_U32 _handle = 0;
        GetMetaHandleFromBufferPtr(aInputBuf, &_handle);

        //get ion fd
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ION_FD, &ionfd);//get ion fd
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);//get real size
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_STRIDE, &srcStride);//get real stride
        switch (mInputMetaDataFormat)
        {
            case HAL_PIXEL_FORMAT_YV12:
                srcUVStride = VENC_ROUND_N(srcWidth >> 1, 16);
                srcWidth = srcStride;//width need be the same with stride...
                srcFormat = DP_COLOR_YV12;
                break;
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcWidth << 2;  //RGBX is a 1-plane data
                srcFormat = DP_COLOR_RGBA8888;
                break;
            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcWidth << 2;  //BGRA is a 1-plane data
                srcFormat = DP_COLOR_BGRA8888;
                break;
            case HAL_PIXEL_FORMAT_RGB_888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcWidth * 3;   //RGB is a 1-plane data
                srcFormat = DP_COLOR_RGB888;
                break;
            default:
                MTK_OMX_LOGE("Unsupport CC PIXEL FORMAT: %d, %s", mInputMetaDataFormat,
                             PixelFormatToString(mInputMetaDataFormat, mStoreMetaDataInBuffers));
                break;
        }

        if (mIsSecureSrc)
        {
            srcSecure = DP_SECURE;
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);

            //ret = mBlitStream.setSrcBuffer((void**)&hSecHandle, &size, 1);
            ret = mBlitStream.setSrcBuffer((void *)hSecHandle, size); //new way, need test
        }
        else
        {
            if (mInputMetaDataFormat == HAL_PIXEL_FORMAT_YV12)  // 3 plane
            {
                uint32_t size_array[3];
                size_array[0] = srcStride * srcSliceHeight;
                size_array[1] = size_array[0] / 4;
                size_array[2] = size_array[0] / 4;
                ret = mBlitStream.setSrcBuffer((int)ionfd, size_array, 3); //set ion src
            }
            else
            {
                ret = mBlitStream.setSrcBuffer((int)ionfd, (uint32_t *)&size, 1); //set ion src
            }
            if (true == mDrawStripe)
            {
                drawRGBStripe((void *)_handle, srcWidth, srcHeight, size);
            }
        }
    }
    else//not meta mode
    {
        srcStride = mInputPortDef.format.video.nStride;
        //int size = srcStride * srcHeight;
        int size = aInputSize;  //size should be aInputSize
        VBufInfo srcInfo;
        int srcSzList[3] = {0};

        switch (mInputPortDef.format.video.eColorFormat)
        {
            case OMX_COLOR_Format16bitRGB565:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcStride * 2;
                srcFormat = DP_COLOR_RGB565;
                //size = size * 2;
                ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_Format24bitRGB888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcStride * 3;
                srcFormat = DP_COLOR_RGB888;
                //size = size * 3;
                ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_Format32bitARGB8888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcStride * 4;
                srcFormat = DP_COLOR_RGBA8888;
                //size = size * 4;
                ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_Format32bitBGRA8888:
                srcWidth = srcStride;//width need be the same with stride...
                srcStride = srcStride * 4;
                srcFormat = DP_COLOR_BGRA8888;
                //size = size * 4;
                ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_FormatYUV420Planar:
            case OMX_COLOR_FormatYUV420Flexible:
                srcUVStride = srcWidth >> 1;
                srcWidth = srcStride;//width need be the same with stride...
                srcFormat = DP_COLOR_I420;
                //size = size + (srcUVStride * srcHeight);
                {
                    mInputMVAMgr->getOmxInfoFromVA((void *)aInputBuf, &srcInfo);
                    srcSzList[0] = srcStride * srcSliceHeight;
                    srcSzList[1] = srcUVStride * srcSliceHeight / 2;
                    srcSzList[2] = srcUVStride * srcSliceHeight / 2;
                    ret = mBlitStream.setSrcBuffer((int32_t)srcInfo.iIonFd, (uint32_t *)srcSzList, (uint32_t)3);

                    MTK_OMX_LOGD("[Debug] CC DP_COLOR_I420 src %d sz[0] %d sz[1] %d sz[2] %d",
                        srcInfo.iIonFd, srcSzList[0], srcSzList[1], srcSzList[2]);
                }
                //ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
                break;
            case OMX_COLOR_FormatYUV420SemiPlanar:
                srcUVStride = srcWidth;
                srcWidth = srcStride;//width need be the same with stride...
                srcFormat = DP_COLOR_NV12;
                {
                    mInputMVAMgr->getOmxInfoFromVA((void *)aInputBuf, &srcInfo);
                    srcSzList[0] = srcUVStride * srcSliceHeight;
                    srcSzList[1] = srcUVStride * srcSliceHeight / 2;
                    srcSzList[2] = 0;
                    ret = mBlitStream.setSrcBuffer((int32_t)srcInfo.iIonFd, (uint32_t *)srcSzList, (uint32_t)2);

                    MTK_OMX_LOGD("[Debug] CC DP_COLOR_NV12 src %d sz[0] %d sz[1] %d",
                        srcInfo.iIonFd, srcSzList[0], srcSzList[1]);
                }
                break;
            default:
                MTK_OMX_LOGE("Unsupport CC FORMAT: %d", mInputPortDef.format.video.eColorFormat);
                break;
        }

        //ret = mBlitStream.setSrcBuffer((OMX_U8 *)aInputBuf, size);
    }

    if (ret < 0)
    {
        MTK_OMX_LOGE("mBlitStream.setSrcBuffer fail: %d", ret);
        return -1;
    }

    MTK_OMX_LOGD_ENG("cc src: w=%u, h=%u, s=%u, uvs=%u, f=0x%x, sec=%u",
                 srcWidth, srcHeight, srcStride, srcUVStride, srcFormat, srcSecure);
    //flush src due to 82 some mpeg4 enc problem
    ret = mBlitStream.setSrcConfig((int32_t)srcWidth, (int32_t)srcHeight, (int32_t)srcStride, (int32_t)srcUVStride,
                             srcFormat, DP_PROFILE_BT601, eInterlace_None, &srcRoi, srcSecure, true);
    if(ret < 0)
    {
       MTK_OMX_LOGE("mBlitStream.setSrcConfig fail: %d", ret);
       return -1;
    }

    if (mRotationType.nRotation == 90 || mRotationType.nRotation == 180 || mRotationType.nRotation == 270)
    {
        mBlitStream.setRotate(mRotationType.nRotation);
        MTK_OMX_LOGD_ENG("cc set rotation %d", mRotationType.nRotation);
    }

    //config color convert dst
    //the output setting of color convert and drv setting shoult be the same
    uint32_t dstWidth       = mEncDrvSetting.u4Width;
    uint32_t dstHeight      = mEncDrvSetting.u4Height;
    uint32_t dstStride      = mEncDrvSetting.u4BufWidth;
    uint32_t dstUVStride    = dstStride;



    if (VENC_DRV_YUV_FORMAT_YV12 == mEncDrvSetting.eVEncFormat)
    {
        dstUVStride = VENC_ROUND_N(dstWidth >> 1, 16);
    }
    else if (VENC_DRV_YUV_FORMAT_NV12 == mEncDrvSetting.eVEncFormat  || VENC_DRV_YUV_FORMAT_NV21 == mEncDrvSetting.eVEncFormat)
    {
        dstUVStride = dstStride;
    }
    else
    {
        //I420
        if ((dstWidth >> 1) & 0x07)
        {
            //uv width not 8-align
            dstUVStride = VENC_ROUND_N(dstWidth >> 1, 8);
        }
        else
        {
            dstUVStride = dstWidth >> 1;
        }
    }
    uint32_t dstSliceHeight = mEncDrvSetting.u4BufHeight;

    DpColorFormat dstFormat = dpFormatMap(mEncDrvSetting.eVEncFormat);
    DpSecure dstSecure = DP_SECURE_NONE;

    DpRect dstRoi;
    dstRoi.x = 0;
    dstRoi.y = 0;
    dstRoi.w = mEncDrvSetting.u4Width;
    dstRoi.h = mEncDrvSetting.u4Height;

    uint8_t *dstYUVbufArray[3];
    uint8_t *dstYUVMVAbufArray[3];
    unsigned int dstYUVbufSizeArray[3];

    if (mIsSecureSrc)//secure buffer dst MUST be meta mode
    {
        int size, hSecHandle;
        OMX_U32 _handle = 0;
        GetMetaHandleFromBufferPtr(aOutputBuf, &_handle);

        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);

        dstSecure = DP_SECURE;
        //ret = mBlitStream.setDstBuffer((void **)&dstAddr, dstYUVbufSizeArray, 3);
        ret = mBlitStream.setDstBuffer((void *)hSecHandle, size);//new way, need test
    }
    else//normal buffer
    {
        VBufInfo dstInfo;
        mCnvtMVAMgr->getOmxInfoFromVA((void *)aOutputBuf, &dstInfo);
        setYUVMVAForCC((void *)aOutputBuf, NULL, dstStride, dstUVStride, dstSliceHeight,
                       dstYUVbufArray, dstYUVMVAbufArray, dstYUVbufSizeArray);
        ret = mBlitStream.setDstBuffer(dstInfo.iIonFd, (unsigned int *)dstYUVbufSizeArray, 3);
    }

    if (ret < 0)
    {
        MTK_OMX_LOGE("mBlitStream.setDstBuffer fail: %d", ret);
        return -1;
    }

    MTK_OMX_LOGD_ENG("cc dst: w=%u, h=%u, s=%u, uvs=%u, f=0x%x, sec=%u",
                 dstWidth, dstHeight, dstStride, dstUVStride, dstFormat, dstSecure);
    ret = mBlitStream.setDstConfig((int32_t)dstWidth, (int32_t)dstHeight, (int32_t)dstStride, (int32_t)dstUVStride,
                             dstFormat, DP_PROFILE_BT601, eInterlace_None, &dstRoi, dstSecure, false);
    if(ret < 0)
    {
       MTK_OMX_LOGE("mBlitStream.setDstConfig fail: %d", ret);
       return -1;
    }

    dumpCCInput(aInputBuf, aInputSize);

    //mBlitStream.invalidate();//trigger convert
#ifdef MTK_QOS_SUPPORT
    struct timeval target_end;
    gettimeofday(&target_end, NULL);
    target_end.tv_usec += 1600;
    if (target_end.tv_usec > 1000000)
    {
        target_end.tv_sec++;
        target_end.tv_usec -= 1000000;
    }
    if (DP_STATUS_RETURN_SUCCESS != mBlitStream.invalidate(&target_end))//trigger convert
#else
    if (DP_STATUS_RETURN_SUCCESS != mBlitStream.invalidate())//trigger convert
#endif
    {
        MTK_OMX_LOGE("DpBlitStream invalidate failed");
    }

    dumpCCOutput(aOutputBuf, aOutputSize);

    return 1;
}

void MtkOmxVenc::dumpCCInput(OMX_U8 *aInputBuf, OMX_U32 aInputSize)
{
    //config color convert src
    uint32_t srcWidth       = mInputPortDef.format.video.nFrameWidth;
    uint32_t srcHeight      = mInputPortDef.format.video.nFrameHeight;

    char name[128];

    if (OMX_TRUE == mDumpColorConvertFrame && mFrameCount < (VAL_UINT32_T)mDumpCCNum)
    {
        if (mStoreMetaDataInBuffers)//meta mode
        {
            int ionfd, size, hSecHandle = 0;
            OMX_U32 _handle = 0;
            GetMetaHandleFromBufferPtr(aInputBuf, &_handle);

            //get ion fd
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ION_FD, &ionfd);//get ion fd
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);//get real size
            if(size <= 0)
            {
                MTK_OMX_LOGE("%s, line %d, invalide size[0x%x]\n", __FUNCTION__, __LINE__,size);
            }
            switch (mInputMetaDataFormat)
            {
                case HAL_PIXEL_FORMAT_YV12:
                    if (mIsSecureSrc)
                    {
                        sprintf(name, "/sdcard/inputbc_%ux%u_%d_r%u_s.yuv", srcWidth, srcHeight,
                                gettid(), mReconfigCount);
                    }
                    else
                    {
                        if (OMX_FALSE == mDumpCts) {
                            sprintf(name, "/sdcard/inputbc_%ux%u_%d_r%u.yuv", srcWidth, srcHeight,
                                    gettid(), mReconfigCount);
                        } else {
                            sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.yuv", srcWidth, srcHeight,
                                    gettid());
                        }
                    }
                    break;
                case HAL_PIXEL_FORMAT_RGBA_8888:
                case HAL_PIXEL_FORMAT_RGBX_8888:
                case HAL_PIXEL_FORMAT_BGRA_8888:
                case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
                case HAL_PIXEL_FORMAT_RGB_888:
                    if (mIsSecureSrc)
                    {
                        sprintf(name, "/sdcard/inputbc_%u_%u_%d_%d_r%u_s.rgb", srcWidth, srcHeight,
                                4, gettid(), mReconfigCount);
                    }
                    else
                    {
                        if (OMX_FALSE == mDumpCts) {
                            sprintf(name, "/sdcard/inputbc_%u_%u_%d_%d_r%u.rgb", srcWidth, srcHeight,
                                    4, gettid(), mReconfigCount);
                        } else {
                            sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.rgb", srcWidth, srcHeight,
                                    gettid());
                        }
                    }
                    break;
                default:
                    MTK_OMX_LOGE("Unsupport CC PIXEL FORMAT: %d, %s", mInputMetaDataFormat,
                                 PixelFormatToString(mInputMetaDataFormat, mStoreMetaDataInBuffers));
                    break;
            }

            if (mIsSecureSrc)
            {
                gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);

                sprintf(name, "/sdcard/input_%u_%u_%d_s.rgb", srcWidth, srcHeight, gettid());
                size = srcWidth * srcHeight * 4;
                dumpSecBuffer(name, hSecHandle, size);
            }
            else
            {
                VENC_DRV_PARAM_FRM_BUF_T    tmpFrm;
                if (mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)_handle, &tmpFrm) < 0)
                {
                    mInputMVAMgr->newOmxMVAwithHndl((void *)_handle, NULL);
                    mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)_handle, &tmpFrm);
                }
                dumpBuffer(name, (unsigned char *)tmpFrm.rFrmBufAddr.u4VA, size);
            }
        }
        else//not meta mode
        {
            switch (mInputPortDef.format.video.eColorFormat)
            {
                case OMX_COLOR_Format16bitRGB565:
                case OMX_COLOR_Format24bitRGB888:
                case OMX_COLOR_Format32bitARGB8888:
                case OMX_COLOR_Format32bitBGRA8888:
                    if (OMX_FALSE == mDumpCts) {
                        sprintf(name, "/sdcard/inputbc_%u_%u_%d_%d_r%u.rgb", srcWidth, srcHeight,
                                4, gettid(), mReconfigCount);
                    } else {
                        sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.rgb", srcWidth, srcHeight,
                                gettid());
                    }
                    dumpBuffer(name, aInputBuf, aInputSize);
                    break;
                case OMX_COLOR_FormatYUV420Planar:
                case OMX_COLOR_FormatYUV420Flexible:
                case OMX_MTK_COLOR_FormatYV12:
                    if (OMX_FALSE == mDumpCts) {
                        sprintf(name, "/sdcard/inputbc_%ux%u_%d_r%u.yuv", srcWidth, srcHeight,
                                gettid(), mReconfigCount);
                    } else {
                        sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.yuv", srcWidth, srcHeight,
                                gettid());
                    }
                    dumpBuffer(name, aInputBuf, srcWidth * srcHeight * 3 / 2);
                    break;
                default:
                    if (OMX_FALSE == mDumpCts) {
                        sprintf(name, "/sdcard/inputbc_%ux%u_%d_r%u.yuv", srcWidth, srcHeight,
                                gettid(), mReconfigCount);
                    } else {
                        sprintf(name, "/sdcard/vdump/Venc_inputbc_%u_%u_t%05d.yuv", srcWidth, srcHeight,
                                gettid());
                    }
                    MTK_OMX_LOGE("Unsupport CC FORMAT: %d", mInputPortDef.format.video.eColorFormat);
                    break;
            }
            //dumpBuffer(name, aInputBuf, aInputSize);
        }
    }
}

void MtkOmxVenc::dumpCCOutput(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize)
{
    (void)(aOutputSize);
    //the output setting of color convert and drv setting shoult be the same
    uint32_t dstWidth       = mEncDrvSetting.u4Width;
    uint32_t dstHeight      = mEncDrvSetting.u4Height;
    uint32_t dstStride      = mEncDrvSetting.u4BufWidth;
    uint32_t dstUVStride    = 0;
    uint32_t dstSliceHeight = mEncDrvSetting.u4BufHeight;

    if (OMX_TRUE == mDumpColorConvertFrame && mFrameCount < (VAL_UINT32_T)mDumpCCNum)
    {
        char name[128];
        unsigned int uYSize = dstWidth * dstHeight;
        unsigned int uUVSize = uYSize >> 2;
        unsigned char *uYAddr = aOutputBuf;
        unsigned char *u2ndPlaneAddr = NULL;
        unsigned char *u3rdPlaneAddr = NULL;

        if (VENC_DRV_YUV_FORMAT_YV12 == mEncDrvSetting.eVEncFormat)
        {
            dstUVStride = VENC_ROUND_N(dstWidth >> 1, 16);
        }
        else if (VENC_DRV_YUV_FORMAT_NV12 == mEncDrvSetting.eVEncFormat
            || VENC_DRV_YUV_FORMAT_NV21 == mEncDrvSetting.eVEncFormat)
        {
            dstUVStride = dstStride;
        }
        else
        {
            //I420
            if ((dstWidth >> 1) & 0x07)
            {
                //uv width not 8-align
                dstUVStride = VENC_ROUND_N(dstWidth >> 1, 8);
            }
            else
            {
                dstUVStride = dstWidth >> 1;
            }
        }

        if (mIsSecureSrc)
        {
            int hSecHandle = 0;
            OMX_U32 _handle = 0;
            GetMetaHandleFromBufferPtr(aOutputBuf, &_handle);
            gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &hSecHandle);

            sprintf(name, "/sdcard/input_%u_%u_%d_s.yuv", dstWidth, dstHeight, gettid());
            dumpSecBuffer(name, hSecHandle, dstStride * dstSliceHeight + dstUVStride * dstSliceHeight);
        }
        else
        {
            if (OMX_FALSE == mDumpCts) {
                sprintf(name, "/sdcard/inputac_%u_%u_%u_%d_%d.yuv", dstWidth, dstHeight, uYSize, uUVSize, gettid());
            } else {
                sprintf(name, "/sdcard/vdump/Venc_inputac_%u_%u_t%05d.yuv", dstWidth, dstHeight, gettid());
            }
            dumpBuffer(name, uYAddr, dstStride * dstSliceHeight + dstUVStride * dstSliceHeight);
        }
    }
    return;
}

OMX_BOOL MtkOmxVenc::NeedConversion()
{
    IN_FUNC();
    if (mInputScalingMode)
    {
        OUT_FUNC();
        return OMX_TRUE;
    }
    if (mEncDrvSetting.u4Width != mInputPortDef.format.video.nFrameWidth
        || mEncDrvSetting.u4Height != mInputPortDef.format.video.nFrameHeight)
    {
        MTK_OMX_LOGD_ENG("NeedConversion for resize input(w: %d h:%d) setdriver(w:%d h:%d)\n",mInputPortDef.format.video.nFrameWidth
             ,mInputPortDef.format.video.nFrameHeight, mEncDrvSetting.u4Width, mEncDrvSetting.u4Height);
        return OMX_TRUE;
    }

    if (mRotationType.nRotation == 90 || mRotationType.nRotation == 180 || mRotationType.nRotation == 270 )
    {
        MTK_OMX_LOGD_ENG("NeedConversion for rotate %d degree", mRotationType.nRotation);
        return OMX_TRUE;
    }
    if (OMX_FALSE == mStoreMetaDataInBuffers)
    {
        switch (mInputPortDef.format.video.eColorFormat)
        {
            case OMX_COLOR_Format16bitRGB565:
            case OMX_COLOR_Format24bitRGB888:
            case OMX_COLOR_Format32bitARGB8888:
            case OMX_COLOR_Format32bitBGRA8888:
                OUT_FUNC(); return OMX_TRUE;
            case OMX_COLOR_FormatYUV420Planar:
            case OMX_MTK_COLOR_FormatYV12:
            case OMX_COLOR_FormatYUV420SemiPlanar:
            case OMX_COLOR_FormatYUV420Flexible:
                if (mInputPortDef.format.video.nStride & 0x0f
                    //|| mInputPortDef.format.video.nSliceHeight & 0x0f
                    )
                {
                    //if stride not 16-align
                    OUT_FUNC(); return OMX_TRUE;
                }
                else
                {
                    OUT_FUNC(); return OMX_FALSE;
                }
            default:
                OUT_FUNC(); return OMX_FALSE;
        }
    }
    else //mStoreMetaDataInBuffers
    {
        switch (mInputMetaDataFormat)
        {
            case HAL_PIXEL_FORMAT_YV12:
                OUT_FUNC(); return OMX_FALSE;
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            case HAL_PIXEL_FORMAT_RGB_888:
                OUT_FUNC(); return OMX_TRUE;
            default:
                OUT_FUNC(); return OMX_FALSE;
        }
    }
    //MTK_OMX_LOGD("@@@ NeedConversion bNeedConversion %x, eColorFormat %x ",
    //bNeedConversion, mInputPortFormat.eColorFormat);
    OUT_FUNC(); return OMX_FALSE;
}

OMX_BOOL MtkOmxVenc::DescribeFlexibleColorFormat(DescribeColorFormatParams *params)
{
    MediaImage &imageInfo = params->sMediaImage;
    memset(&imageInfo, 0, sizeof(imageInfo));

    imageInfo.mType = MediaImage::MEDIA_IMAGE_TYPE_UNKNOWN;
    imageInfo.mNumPlanes = 0;

    const OMX_COLOR_FORMATTYPE fmt = params->eColorFormat;
    imageInfo.mWidth = params->nFrameWidth;
    imageInfo.mHeight = params->nFrameHeight;

    MTK_OMX_LOGD_ENG("DescribeFlexibleColorFormat %d fmt %x, W/H(%d, %d), WS/HS(%d, %d), (%d, %d)", sizeof(size_t), fmt, imageInfo.mWidth, imageInfo.mHeight,
        params->nStride, params->nSliceHeight, mOutputPortDef.format.video.nStride, mOutputPortDef.format.video.nSliceHeight);

    // only supporting YUV420
    if (fmt != OMX_COLOR_FormatYUV420Planar &&
        fmt != OMX_COLOR_FormatYUV420PackedPlanar &&
        fmt != OMX_COLOR_FormatYUV420SemiPlanar &&
        fmt != HAL_PIXEL_FORMAT_I420 &&
        fmt != OMX_COLOR_FormatYUV420Flexible &&
        fmt != OMX_COLOR_FormatYUV420PackedSemiPlanar)
    {
        ALOGW("do not know color format 0x%x = %d", fmt, fmt);
        return OMX_FALSE;
    }

    // set-up YUV format
    imageInfo.mType = MediaImage::MEDIA_IMAGE_TYPE_YUV;
    imageInfo.mNumPlanes = 3;
    imageInfo.mBitDepth = 8;
    imageInfo.mPlane[MediaImage::Y].mOffset = 0;
    imageInfo.mPlane[MediaImage::Y].mColInc = 1;
    imageInfo.mPlane[MediaImage::Y].mRowInc = params->nFrameWidth;
    imageInfo.mPlane[MediaImage::Y].mHorizSubsampling = 1;
    imageInfo.mPlane[MediaImage::Y].mVertSubsampling = 1;

    switch (fmt)
    {
        case OMX_COLOR_FormatYUV420Planar: // used for YV12
        case OMX_COLOR_FormatYUV420PackedPlanar:
        case OMX_COLOR_FormatYUV420Flexible:
        case HAL_PIXEL_FORMAT_I420:
            imageInfo.mPlane[MediaImage::U].mOffset = params->nFrameWidth * params->nFrameHeight;
            imageInfo.mPlane[MediaImage::U].mColInc = 1;
            imageInfo.mPlane[MediaImage::U].mRowInc = params->nFrameWidth / 2;
            imageInfo.mPlane[MediaImage::U].mHorizSubsampling = 2;
            imageInfo.mPlane[MediaImage::U].mVertSubsampling = 2;

            imageInfo.mPlane[MediaImage::V].mOffset = imageInfo.mPlane[MediaImage::U].mOffset
                                            + (params->nFrameWidth * params->nFrameHeight / 4);
            imageInfo.mPlane[MediaImage::V].mColInc = 1;
            imageInfo.mPlane[MediaImage::V].mRowInc = params->nFrameWidth / 2;
            imageInfo.mPlane[MediaImage::V].mHorizSubsampling = 2;
            imageInfo.mPlane[MediaImage::V].mVertSubsampling = 2;
            break;

        case OMX_COLOR_FormatYUV420SemiPlanar:
            // FIXME: NV21 for sw-encoder, NV12 for decoder and hw-encoder
        case OMX_COLOR_FormatYUV420PackedSemiPlanar:
            // NV12
            imageInfo.mPlane[MediaImage::U].mOffset = params->nStride * params->nSliceHeight;
            imageInfo.mPlane[MediaImage::U].mColInc = 2;
            imageInfo.mPlane[MediaImage::U].mRowInc = params->nStride;
            imageInfo.mPlane[MediaImage::U].mHorizSubsampling = 2;
            imageInfo.mPlane[MediaImage::U].mVertSubsampling = 2;

            imageInfo.mPlane[MediaImage::V].mOffset = imageInfo.mPlane[MediaImage::U].mOffset + 1;
            imageInfo.mPlane[MediaImage::V].mColInc = 2;
            imageInfo.mPlane[MediaImage::V].mRowInc = params->nStride;
            imageInfo.mPlane[MediaImage::V].mHorizSubsampling = 2;
            imageInfo.mPlane[MediaImage::V].mVertSubsampling = 2;
            break;

        default:
            MTK_OMX_LOGE("default %x", fmt);
    }
    return OMX_TRUE;
}


VENC_DRV_YUV_FORMAT_T MtkOmxVenc::CheckFormatToDrv()
{
    VENC_DRV_YUV_FORMAT_T ret = VENC_DRV_YUV_FORMAT_YV12;

    if (mStoreMetaDataInBuffers)//if meta mode
    {
        switch (mInputMetaDataFormat)
        {
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            case HAL_PIXEL_FORMAT_RGB_888:
                //drv format is the format after color converting
                ret = VENC_DRV_YUV_FORMAT_YV12;
                break;
            case HAL_PIXEL_FORMAT_YV12:
                //only support YV12 (16/16/16) right now
                ret = VENC_DRV_YUV_FORMAT_YV12;
                break;
            default:
                MTK_OMX_LOGD("unsupported format:0x%x %s", mInputMetaDataFormat,
                             PixelFormatToString(mInputMetaDataFormat, mStoreMetaDataInBuffers));
                ret = VENC_DRV_YUV_FORMAT_YV12;
                break;
        }
    }
    else
    {
        switch (mInputPortFormat.eColorFormat)
        {
            case OMX_COLOR_FormatYUV420Planar:
            case OMX_COLOR_FormatYUV420Flexible:
                ret = VENC_DRV_YUV_FORMAT_420;
                break;

            case OMX_MTK_COLOR_FormatYV12:
                ret = VENC_DRV_YUV_FORMAT_YV12;
                break;

            case OMX_COLOR_FormatAndroidOpaque:
                //should not be here, metaMode MUST on when format is AndroidQpaque...
                ret = VENC_DRV_YUV_FORMAT_420;
                break;

            // Gary Wu add for MediaCodec encode with input data format is RGB
            case OMX_COLOR_Format16bitRGB565:
            case OMX_COLOR_Format24bitRGB888:
            case OMX_COLOR_Format32bitARGB8888:
            case OMX_COLOR_Format32bitBGRA8888:
                ret = VENC_DRV_YUV_FORMAT_420;
                break;

            case OMX_COLOR_FormatYUV420SemiPlanar:
                ret = VENC_DRV_YUV_FORMAT_NV12;
                break;

            default:
                MTK_OMX_LOGE("[ERROR][EncSettingCodec] ColorFormat = %d, not supported ?\n",
                             mInputPortFormat.eColorFormat);
                ret = VENC_DRV_YUV_FORMAT_NONE;
                break;
        }
    }
    MTK_OMX_LOGD("[EncSettingCodec] Input Format = 0x%x, ColorFormat = 0x%x\n", mInputPortFormat.eColorFormat, ret);
    return ret;
}

void MtkOmxVenc::DrawYUVStripeLine(OMX_U8 *pYUV)
{
    unsigned char *pY, *pY2, *pU, *pU2, *pV, *pV2;
    OMX_U32 iDrawWidth = mOutputPortDef.format.video.nFrameWidth;
    OMX_U32 iDrawHeight = mOutputPortDef.format.video.nFrameHeight;
    pY = (unsigned char *)pYUV;

    //if( OMX_ErrorNotReady == WFD_DrawYUVStripeLine(&pY, &pU, &pV, iDrawWidth, iDrawHeight) )
    {
        pU = pY + VENC_ROUND_N(iDrawWidth, 16) * VENC_ROUND_N(iDrawHeight, 16);
        pV = pU + ((VENC_ROUND_N((iDrawWidth >> 1), 16) * VENC_ROUND_N(iDrawHeight, 16) >> 1));
    }

    unsigned int i;
    for (i = 0; i <= (iDrawHeight - 4) / 100; ++i)
    {
        pY2 = pY + 100 * i * VENC_ROUND_N(iDrawWidth, 16);
        pU2 = pU + 50 * i * VENC_ROUND_N((iDrawWidth >> 1), 16);
        pV2 = pV + 50 * i * VENC_ROUND_N((iDrawWidth >> 1), 16);
        //MTK_OMX_LOGE("%d %d %d", 100*i*VENC_ROUND_N(iDrawWidth, 16), 50*i*VENC_ROUND_N((iDrawWidth>>1), 16),
        //50*i*VENC_ROUND_N((iDrawWidth>>1), 16));
        for (int j = 0; j < 4; ++j)
        {
            memset(pY2, 0, iDrawWidth);
            pY2 += VENC_ROUND_N(iDrawWidth, 16);
            if (j & 1)
            {
                memset(pU2, 0, iDrawWidth >> 1);
                memset(pV2, 0, iDrawWidth >> 1);
                pU2 += VENC_ROUND_N((iDrawWidth >> 1), 16);
                pV2 += VENC_ROUND_N((iDrawWidth >> 1), 16);
            }
        }
    }
}

uint32_t MtkOmxVenc::CheckOpaqueFormat(OMX_BUFFERHEADERTYPE *pInputBuf)
{
    OMX_U8 *aInputBuf = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U32 _handle = 0;
    unsigned int format = 0;

    if (pInputBuf->nFilledLen == 0 || OMX_TRUE != mStoreMetaDataInBuffers)
    {
        //if buffer is empty or not meta mode
        //if ((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (0xCDCDCDCD == (OMX_U32)_handle))
        return 0;
    }
    else
    {
        GetMetaHandleFromBufferPtr(aInputBuf, &_handle);
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_FORMAT, &format);
        if (format != mInputMetaDataFormat)
        {
            MTK_OMX_LOGD("CheckOpaqueFormat: 0x%x %s -> 0x%x %s",
                mInputMetaDataFormat, PixelFormatToString(mInputMetaDataFormat, mStoreMetaDataInBuffers),
                format, PixelFormatToString(format, mStoreMetaDataInBuffers));
        }

        return (uint32_t)format;
    }
}

DpColorFormat MtkOmxVenc::dpFormatMap(VENC_DRV_YUV_FORMAT_T format)
{
    switch (format)
    {
        case VENC_DRV_YUV_FORMAT_YV12:
            return DP_COLOR_YV12;
        case VENC_DRV_YUV_FORMAT_420:
            return DP_COLOR_I420;
        case VENC_DRV_YUV_FORMAT_NV12:
            return DP_COLOR_NV12;
        default:
            MTK_OMX_LOGE("Unsupport drv Format to CC: %d", format);
            return DP_COLOR_YV12;
    }
}

OMX_U32 MtkOmxVenc::getInputBufferSizeByFormat(OMX_VIDEO_PORTDEFINITIONTYPE video)
{
    OMX_U32 mBufferTmpSize = (VENC_ROUND_N(video.nStride, 32) * VENC_ROUND_N(video.nSliceHeight, 32) * 3 >> 1);
    OMX_U32 mBytesperPixel = 2;
    OMX_U32 mNeedDivideTwo = 0;
    switch (mInputPortDef.format.video.eColorFormat)
    {
        case OMX_COLOR_FormatYUV420Planar:
        case OMX_COLOR_FormatYUV420Flexible:
        case OMX_MTK_COLOR_FormatYV12:
        case OMX_COLOR_FormatAndroidOpaque:
        case OMX_COLOR_FormatYUV420SemiPlanar:
        {
            mBytesperPixel = 3;
            mNeedDivideTwo = 1;
        }
        break;
        case OMX_COLOR_Format32bitBGRA8888:
        case OMX_COLOR_Format32bitARGB8888:
        {
            mBytesperPixel = 4;
        }
        break;
        case OMX_COLOR_Format16bitRGB565:
        case OMX_COLOR_Format16bitBGR565:
        {
            mBytesperPixel = 2;
        }
        break;
        case OMX_COLOR_Format24bitRGB888:
        case OMX_COLOR_Format24bitBGR888:
        {
            mBytesperPixel = 3;
        }
        break;
        default:
        {
            //2 bytes per pixel
            mBytesperPixel = 3;
            mNeedDivideTwo = 1;
        }
        break;
    }

    if (mNeedDivideTwo)
    {
        mBufferTmpSize = (VENC_ROUND_N(video.nStride, 32) * VENC_ROUND_N(video.nSliceHeight, 32) * mBytesperPixel >> 1); // this
    }
    else
    {
        mBufferTmpSize = (VENC_ROUND_N(video.nStride, 32) * VENC_ROUND_N(video.nSliceHeight, 32) * mBytesperPixel);
    }

    OMX_U32 ret = mBufferTmpSize;

    // +((ALIGN(pix_fmt_mp->width, 16) * 2) * 16)
    //OMX_U32 ret = mBufferTmpSize + (VENC_ROUND_N(video.nStride, 16)*2*16);

    MTK_OMX_LOGD("getInputBufferSizeByFormat: %d", ret);
    return ret;
}

int MtkOmxVenc::drawRGBStripe(void *handle, int imgWidth, int imgHeight, int size)
{
    VENC_DRV_PARAM_FRM_BUF_T    tmpBuf;
    if (mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)handle, &tmpBuf) < 0)
    {
        mInputMVAMgr->newOmxMVAwithHndl((void *)handle, NULL);
        mInputMVAMgr->getOmxMVAFromHndlToVencFrm((void *)handle, &tmpBuf);
    }
    unsigned char *in = (unsigned char *)tmpBuf.rFrmBufAddr.u4VA;
    MTK_OMX_LOGD("!!!!! in addr=0x%x, size = %d, format=0x%x (%s)", in, size, mInputMetaDataFormat, PixelFormatToString(mInputMetaDataFormat, mStoreMetaDataInBuffers));
    int y, x;
    for (y = 0; y < imgHeight; ++y)
    {
        int xShift = mFrameCount % (imgWidth - 32);
        for (x = 0; x < 16; ++x)
        {
            in[(y * imgWidth * 4) + ((xShift + x) * 4)] = 0xff;
            in[(y * imgWidth * 4) + ((xShift + x) * 4) + 1] = 0x00;
            in[(y * imgWidth * 4) + ((xShift + x) * 4) + 2] = 0x00;
            in[(y * imgWidth * 4) + ((xShift + x) * 4) + 3] = 0x00;
        }
    }
    return 1;
}

int MtkOmxVenc::setYUVMVAForCC(void *va, void *mva, int stride, int uvStride, int sliceHeight,
                               uint8_t *vaArray[], uint8_t *mvaArray[], unsigned int sizeArray[],
                               int format/*0*/)
{
    // format : 0=I420, 1=NV12
    vaArray[0] = (uint8_t *)va;
    vaArray[1] = vaArray[0] + (stride * sliceHeight);
    if (0 == format) {
        vaArray[2] = vaArray[1] + (uvStride * (sliceHeight >> 1));
    }
    else {
        vaArray[2] = 0;
    }

    sizeArray[0] = stride * sliceHeight;
    if (0 == format) {
        sizeArray[1] = uvStride * (sliceHeight >> 1);
        sizeArray[2] = sizeArray[1];
    }
    else {
        sizeArray[1] = stride * sliceHeight;
        sizeArray[2] = 0;
    }

    mvaArray[0] = (uint8_t *)mva;
    mvaArray[1] = mvaArray[0] + (stride * sliceHeight);
    if (0 == format) {
        mvaArray[2] = mvaArray[1] + (uvStride * (sliceHeight >> 1));
    }
    else {
        mvaArray[2] = 0;
    }

    MTK_OMX_LOGD_ENG("sizeArray %d, %d, %d, vaArray 0x%x, 0x%x, 0x%x",
                 sizeArray[0], sizeArray[1], sizeArray[2],
                 (unsigned int)vaArray[0], (unsigned int)vaArray[1], (unsigned int)mvaArray[2]);

    return 1;
}

bool MtkOmxVenc::supportNV12(void)
{
    //right now only AVC/HEVC VENC and HW solution support NV12
    if(MTK_VENC_CODEC_ID_AVC == mCodecId ||\
       MTK_VENC_CODEC_ID_HEVC == mCodecId)
    {
        return true;
    }
    return false;
}

const char* PixelFormatToString(unsigned int nPixelFormat, bool metamode)
{
    if(metamode)
    {
        switch (nPixelFormat)
        {
            case HAL_PIXEL_FORMAT_RGBA_8888:
                return "HAL_PIXEL_FORMAT_RGBA_8888";
            case HAL_PIXEL_FORMAT_RGBX_8888:
                return "HAL_PIXEL_FORMAT_RGBX_8888";
            case HAL_PIXEL_FORMAT_RGB_888:
                return "HAL_PIXEL_FORMAT_RGB_888";
            case HAL_PIXEL_FORMAT_RGB_565:
                return "HAL_PIXEL_FORMAT_RGB_565";
            case HAL_PIXEL_FORMAT_BGRA_8888:
                return "HAL_PIXEL_FORMAT_BGRA_8888";
            case HAL_PIXEL_FORMAT_YV12:
                return "HAL_PIXEL_FORMAT_YV12";
            default:
                return "Unknown Pixel Format";
        }
    }
    else
    {
        switch (nPixelFormat)
        {
            case OMX_COLOR_FormatYUV420Flexible:
                return "OMX_COLOR_FormatYUV420Flexible";
            case OMX_COLOR_FormatYUV420PackedPlanar:
                return "OMX_COLOR_FormatYUV420PackedPlanar";
            case OMX_COLOR_FormatAndroidOpaque:
                return "OMX_COLOR_FormatAndroidOpaque";
            case OMX_COLOR_Format16bitRGB565:
                return "OMX_COLOR_Format16bitRGB565";
            case OMX_COLOR_Format24bitRGB888:
                return "OMX_COLOR_Format24bitRGB888";
            case OMX_COLOR_Format32bitARGB8888:
                return "OMX_COLOR_Format32bitARGB8888";
            case OMX_COLOR_Format32bitBGRA8888:
                return "OMX_COLOR_Format32bitBGRA8888";
            case OMX_COLOR_FormatYUV420Planar:
                return "OMX_COLOR_FormatYUV420Planar";
            case OMX_MTK_COLOR_FormatYV12:
                return "OMX_MTK_COLOR_FormatYV12";
            case OMX_COLOR_FormatYUV420SemiPlanar:
                return "OMX_COLOR_FormatYUV420SemiPlanar";
            default:
                return "Unknown OMX Format";
        }
    }
}

void MtkOmxVenc::setBufferBlack(OMX_U8* buffer, int size)
{
    memset(((OMX_U8 *)buffer) + size, 0x80, size / 2);
    memset((OMX_U8 *)buffer, 0x10, size);
}
