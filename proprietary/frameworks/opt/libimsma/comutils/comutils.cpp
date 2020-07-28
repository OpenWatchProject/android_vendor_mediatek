
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   Rotate.cpp
 *
 * Project:
 * --------
 *
 *
 * Description:
 * ------------
 *   do video buffer rotation
 *
 * Author:
 * -------
 *   Qian Dong
 *
 ****************************************************************************/
#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "[VT][comutils]"
#include "comutils.h"
#include <OMX_IVCommon.h>
#include <cutils/properties.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/base64.h>
#include "media/stagefright/foundation/avc_utils.h"
#include <utils/Vector.h>
#include <utils/Trace.h>
#include <OMX_Video.h>
#include <OMX_VideoExt.h>
#include <media/stagefright/MediaDefs.h>
#include <ctype.h>
#include <media/stagefright/foundation/ABitReader.h>
#include <system/graphics.h>
#include <utils/Timers.h>


#if USING_MDP_PRE_PREOCESS

#ifdef USING_MDP_BY_HIDL
#include <ion/ion.h>
#include <ion_copy.h>
//#include <linux/ion_drv.h>
//#include <linux/mtk_ion.h>
#include <vendor/mediatek/hardware/mms/1.1/IMms.h>
#include "DpColorFormat.h"

using ::vendor::mediatek::hardware::mms::V1_1::IMms;
using ::vendor::mediatek::hardware::mms::V1_1::HwMDPParam;
using ::vendor::mediatek::hardware::mms::V1_1::DpRect;

#else
#include "DpBlitStream.h"
#endif
#else
#include "DpColorFormat.h" //for compile
#endif



#define MEM_ALIGN_32 32
#define ROUND_2(X)     ((X + 0x1) & (~0x1))
#define ROUND_8(X)     ((X + 0x7) & (~0x7))
#define ROUND_16(X)     ((X + 0xF) & (~0xF))
#define ROUND_32(X)     ((X + 0x1F) & (~0x1F))
#define YUV_SIZE(W,H)   (W * H * 3 >> 1)
#define RGB888_SIZE(W,H)   (W * H * 3)
#define ARGB888_SIZE(W,H)   (W * H * 4)
enum {
    UV_STRIDE_16_8_8,
    UV_STRIDE_16_16_16,
    UV_STRIDE_other,
};


namespace android
{
static void printBinary(uint8_t *ptrBS, int iLen)
{
    int i;
    char tmp[1024];
    char *ptr = tmp;

    for(i = 0; i < iLen; ++i) {
        sprintf(ptr, "%02x", ptrBS[i]);
        ptr += 2;
    }

    *ptr = '\0';
    ALOGI("[genParameterSets] bssize(%d), bs(%s)", iLen, tmp);
}


int32_t  RemovePreventionByte(uint8_t *profile_tier_level, uint8_t size)
{
    int32_t pos = 0;
    int32_t leftSize = size ;

    for(; pos < size - 2;) {
        if(profile_tier_level[pos] == 0x00 &&
                profile_tier_level[pos + 1] == 0x00 &&
                profile_tier_level[pos + 2] == 0x03) {
            memmove(profile_tier_level + pos + 2, profile_tier_level + pos + 3, size - (pos + 3));
            pos += 2;
            leftSize--;//remove 1 0x03
        } else {
            pos++;
        }
    }

    VT_LOGD("Size %d --> leftSize %d",size,leftSize);
    return leftSize;
}

DpColorFormat OmxColorToDpColor(int32_t omx_color_format)
{
    DpColorFormat colorFormat;

    switch(omx_color_format) {
    case OMX_COLOR_FormatVendorMTKYUV:
        colorFormat = eNV12_BLK;
        break;
    case OMX_COLOR_FormatVendorMTKYUV_FCM:
        colorFormat = eNV12_BLK_FCM;
        break;
    case OMX_COLOR_FormatYUV420Planar:
        colorFormat = eYUV_420_3P;
        break;
    case OMX_MTK_COLOR_FormatYV12://HAL3
        colorFormat = eYV12;
        break;
    case OMX_COLOR_Format24bitRGB888:
        colorFormat = eRGB888 ;
        break;
    case OMX_COLOR_Format32bitARGB8888:
        colorFormat = eARGB8888  ;
        break;
    case OMX_COLOR_Format32BitRGBA8888://hAL1
        colorFormat = eRGBA8888 ;
        break;
    case OMX_COLOR_FormatYUV420SemiPlanar://NV21
        colorFormat = eNV21 ;
        break;	
    default:
        colorFormat = eYUV_420_3P;
        VT_LOGE("[Warning] Cannot find color mapping !!");
        break;
    }

    return colorFormat;
}

const char *PixelFormatToString(int32_t pixelFormat)
{
    switch(pixelFormat) {
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
    case HAL_PIXEL_FORMAT_YCrCb_420_SP://NV21
        return "HAL_PIXEL_FORMAT_YCrCb_420_SP ";
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
        return "HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED";
    default:
        return "Unknown Pixel Format";
    }
}
const char *OmxFormatToString(int32_t omxFormat)
{
    switch((OMX_COLOR_FORMATTYPE) omxFormat) {
    case OMX_MTK_COLOR_FormatYV12:
        return "OMX_MTK_COLOR_FormatYV12";
    case OMX_COLOR_FormatYUV420SemiPlanar:
        return "OMX_COLOR_FormatYUV420SemiPlanar";
    case OMX_COLOR_Format24bitRGB888:
        return "OMX_COLOR_Format24bitRGB888";
    case OMX_COLOR_FormatAndroidOpaque:
        return "OMX_COLOR_FormatAndroidOpaque";
    case OMX_COLOR_Format32bitARGB8888:
        return "OMX_COLOR_Format32bitARGB8888";
    case OMX_COLOR_Format32BitRGBA8888:
        return "OMX_COLOR_Format32BitRGBA8888";
    case OMX_COLOR_Format32bitBGRA8888:
        return "OMX_COLOR_Format32bitBGRA8888";
    default:
        return "Unknown OMX Format";
    }
}

int32_t PixelForamt2ColorFomat(int32_t pixelFormat)
{
    int32_t  colorFormat;

    switch(pixelFormat) {
    case HAL_PIXEL_FORMAT_YV12://0x32315659
        colorFormat =  OMX_MTK_COLOR_FormatYV12;//0x7F000200,
        break;

    case HAL_PIXEL_FORMAT_YCrCb_420_SP://17
        colorFormat =  OMX_COLOR_FormatYUV420SemiPlanar;//  NV21 17  
        break;

    case HAL_PIXEL_FORMAT_RGB_888:
        colorFormat =  OMX_COLOR_Format24bitRGB888;
        break;

    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:// = 0x22,TODO qian
        colorFormat =  OMX_COLOR_FormatAndroidOpaque;
        break;

    case HAL_PIXEL_FORMAT_RGBA_8888://0x1
        colorFormat =  OMX_COLOR_Format32BitRGBA8888;//TODO 0x7F00A000
        break;

    default :
        colorFormat = OMX_COLOR_FormatAndroidOpaque;
        break;
    }

    //CHECK(colorFormat == OMX_MTK_COLOR_FormatYV12);
    return colorFormat;
}
int32_t getEncoderInPutFormat()
{
    /*
    OMX_COLOR_FormatYUV420SemiPlanar = 11
    OMX_MTK_COLOR_FormatYV12 = 0x7F000200
    char value[PROPERTY_VALUE_MAX];
    if (property_get("persist.vendor.radio.vilte.enc.format", value, NULL)) {//resolution change interval
    format = atoi(value);
    VT_LOGI("[ID=%d]format %s",mMultiInstanceID,OmxFormatToString(format));
    }
    */
    return OMX_MTK_COLOR_FormatYV12;//OMX_COLOR_FormatYUV420SemiPlanar
}


/*
default 0
ro_type : 0 -->rotate and resize with the same ratio  as the input buffer, fill the middle of the output buffer with black edge on left-right
ro_type : 1 -->crop input buffer with the same ratio as output, rotate and resize to fully fill the output buffer, loose some input buffer content
ro_type : 2 -->just rotate and resize the input buffer ,and fully fill the output buffer,distorte the picture content
ro_type : 3 -->ratate width changed W and H
*/
/*int32_t vilte_ion_va_mva(unsigned long va,int32_t size,uint32_t* mva){

	struct ion_sys_data sys_data;
	struct ion_custom_data custom_data;
	int ion_fd;
	ion_fd = ion_open();
	if (ion_fd < 0){
	      ALOGE("fd invliad \n");
		return -1;
	}
	sys_data.sys_cmd = ION_SYS_GET_VA2MVA;
	sys_data.get_phys_param.va = va;
	sys_data.get_phys_param.len = size;
	//sys_data.get_phys_param.phy_addr=mva;

	custom_data.cmd = ION_CMD_SYSTEM;
	custom_data.arg = (unsigned long)&sys_data;

	ioctl(ion_fd, ION_IOC_CUSTOM, &custom_data);
	*mva = sys_data.get_phys_param.phy_addr;
	ALOGI("va 0x%lx, size %d,phy_addr 0x%x,mav 0x%x",va,size,sys_data.get_phys_param.phy_addr,*mva);

	ion_close(ion_fd);
	return 0;
}*/
#if USING_MDP_PRE_PREOCESS
#if USING_MDP_BY_HIDL
int vilte_ion_va_mva(unsigned int ion_fd,unsigned long va, unsigned int size, unsigned int *mva,int *handleToBeFree)
{
   // unsigned int ion_fd;
    int ion_user_handle;

	int ret = 0;
	ion_sys_data_t sys_data;
	struct ion_mm_data mm_data;
	struct ion_custom_data custom_data;

	/*ion_fd = ion_open();
	if (ion_fd < 0) {
		ALOGE("ion_open(%d) fail\n", ion_fd);
		return ret;
	}*/

	ret = ion_alloc(ion_fd, size, va, ION_HEAP_MULTIMEDIA_MAP_MVA_MASK, 3, &ion_user_handle);
	if (ret < 0) {
		ALOGE("ion_alloc fail\n");
		return ret;
	}

	mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
	mm_data.config_buffer_param.eModuleID = 5;
	mm_data.config_buffer_param.coherent = 0;
	mm_data.config_buffer_param.security = 0;
	mm_data.config_buffer_param.handle = ion_user_handle;

	custom_data.cmd = ION_CMD_MULTIMEDIA;
	custom_data.arg = (unsigned long)&mm_data;
	ret = ioctl(ion_fd, ION_IOC_CUSTOM, &custom_data);
	if (ret < 0) {
		ALOGE("ion config buffer fail\n");
		return ret;
	}

	sys_data.sys_cmd = ION_SYS_GET_PHYS;
	sys_data.get_phys_param.handle = ion_user_handle;
	sys_data.get_phys_param.len = size;

	custom_data.cmd = ION_CMD_SYSTEM;
	custom_data.arg = (unsigned long)&sys_data;

	ret = ioctl(ion_fd, ION_IOC_CUSTOM, &custom_data);
	if (ret < 0) {
		ALOGE("ion get phys fail\n");
		return ret;
	}
	*mva = sys_data.get_phys_param.phy_addr;
	VT_LOGD("va 0x%lx, size %d,phy_addr 0x%x,mav 0x%x %d",va,size,sys_data.get_phys_param.phy_addr,*mva,ion_user_handle);

   //ion_free(ion_fd, ion_user_handle);
   // ion_close(ion_fd);
	*handleToBeFree = ion_user_handle;
	return 0;
}

#endif

int64_t rotateBuffer(uint8_t *input,uint8_t *output, RotateInfo *info,int32_t *outFillLen)
{

    int64_t startTimeUs = systemTime(SYSTEM_TIME_MONOTONIC) / 1000ll;

    //set internal parameters
    int32_t srcWidth,  srcHeight, tarWidth,  tarHeight,degree,srcFormat,tarFormat,ro_type;
    int32_t srcBufWidth,  srcBufHeight, tarBufWidth,  tarBufHeight;

    srcWidth = info->mSrcWidth;
    srcHeight = info->mSrcHeight;
    tarWidth = info->mTargetWidth;
    tarHeight = info->mTargetHeight;
    degree = info->mRotateDegree;
    srcFormat = info->mSrcColorFormat;
    tarFormat = info->mTargetColorFormat;
    ro_type = info->mRotateType;

    srcBufWidth = info->mSrcBufWidth;
    srcBufHeight = info->mSrcBufHeight;
    tarBufWidth = info->mTargetBufWidth;
    tarBufHeight = info->mTargetBufHeight;

    ATRACE_CALL();
    VT_LOGD("input %p,s-w %d,s-h %d,s-F %s, t-w %d,t-h %d,t-bw %d,t-bh %d,t-F %s,output %p, degree %d,ro_type %d *outFillLen %d",
            input,  srcWidth,  srcHeight, OmxFormatToString(srcFormat), tarWidth,  tarHeight,  tarBufWidth,  tarBufHeight,OmxFormatToString(tarFormat),output,degree,ro_type,*outFillLen);

#ifdef USING_MDP_BY_HIDL

	sp<IMms> IMms_service = IMms::tryGetService();
	if (IMms_service == nullptr)
	{
		ALOGE("cannot find IMms_service!");
		return -1;
	}
	HwMDPParam mpdParams;
	memset(&mpdParams,0,sizeof(HwMDPParam));

#endif
    if(ro_type == ROT_A_DEGREE_NOT_KEEP_RATIO && (degree == 90 || degree == 270)) {
        //CHECK(info->mSrcWidth  >= info->mTargetHeight);
        //CHECK(info->mSrcHeight >=  info->mTargetWidth);
        CHECK(tarBufWidth  >= tarWidth);
        CHECK(tarBufHeight  >= tarHeight);
    } else {
        CHECK(info->mSrcWidth  == info->mTargetWidth);
        CHECK(info->mSrcHeight== info->mTargetHeight);
    }

    char value[PROPERTY_VALUE_MAX];

    if(property_get("vendor.vt.ro.degree", value, NULL)) {
        degree= atoi(value);
    }

   
    //config buffers

    uint8_t   *srcYUVbuf_va = NULL;
    uint8_t   *dstYUVbuf_va = NULL;

    uint32_t srcUvStrideMode = UV_STRIDE_16_8_8;
    uint32_t tarUvStrideMode = UV_STRIDE_16_8_8;

    //planes
    uint8_t *srcYUVbufArray[3];
    unsigned int srcYUVbufSizeArray[3];
    uint8_t *dstYUVbufArray[3];
    unsigned int dstYUVbufSizeArray[3];
    uint32_t srcNumPlanes = 2;
    uint32_t tarNumPlanes = 2;


    uint32_t  srcWStride = srcWidth;
    uint32_t  srcHStride = srcHeight;
    uint32_t srcBufferSize = 0;

    uint32_t  tarWStride = tarWidth;
    uint32_t  tarHStride = tarHeight;

    srcYUVbuf_va =   input;
    dstYUVbuf_va = output;
    uint32_t dstBufferSize = 0;


    if(srcFormat == OMX_COLOR_FormatVendorMTKYUV
            || srcFormat == OMX_COLOR_FormatVendorMTKYUV_FCM) {
        srcWStride =ROUND_16(srcWidth);
        srcHStride = ROUND_32(srcHeight);
        srcBufferSize = ROUND_32(YUV_SIZE(srcWStride, srcHStride));
        srcYUVbufArray[0] = srcYUVbuf_va;      // Y
        srcYUVbufArray[1] = srcYUVbuf_va + srcWStride * srcHStride;  // C
        srcYUVbufSizeArray[0] = srcWStride * srcHStride;
        srcYUVbufSizeArray[1] = srcWStride * srcHStride / 2;
        srcNumPlanes = 2;
        srcUvStrideMode = UV_STRIDE_16_8_8;

    } else if(srcFormat == OMX_COLOR_FormatYUV420Planar) {
        srcWStride =ROUND_16(srcWidth);
        srcHStride = ROUND_16(srcHeight);
        srcBufferSize = ROUND_32(YUV_SIZE(srcWStride, srcHStride));
        srcYUVbufArray[0] = srcYUVbuf_va;
        srcYUVbufArray[1] = srcYUVbuf_va + (srcWStride * srcHStride);
        srcYUVbufArray[2] = srcYUVbufArray[1] + (srcWStride * srcHStride) / 4;
        srcYUVbufSizeArray[0] = srcWStride * srcHStride;
        srcYUVbufSizeArray[1] = (srcWStride * srcHStride) / 4;
        srcYUVbufSizeArray[2] = (srcWStride * srcHStride) / 4;
        srcNumPlanes = 3;
        srcUvStrideMode = UV_STRIDE_16_8_8;
    } else if(srcFormat == OMX_MTK_COLOR_FormatYV12) {
        srcWStride =ROUND_16(srcWidth);
        srcHStride = ROUND_16(srcHeight);
        srcYUVbufSizeArray[0] =  srcWStride * srcHStride;
        srcYUVbufSizeArray[1] = ROUND_16(srcWStride / 2) * (srcHStride / 2);
        srcYUVbufSizeArray[2] = ROUND_16(srcWStride / 2) * (srcHStride / 2);
        srcBufferSize = srcYUVbufSizeArray[0] + srcYUVbufSizeArray[1] + srcYUVbufSizeArray[2];
        srcYUVbufArray[0] = srcYUVbuf_va;
        srcYUVbufArray[1] = srcYUVbuf_va + srcYUVbufSizeArray[0];
        srcYUVbufArray[2] = srcYUVbufArray[1] + srcYUVbufSizeArray[1];
        srcNumPlanes = 3;
        srcUvStrideMode = UV_STRIDE_16_16_16;


    } else if(srcFormat == OMX_COLOR_FormatYUV420SemiPlanar) {
        srcWStride =ROUND_16(srcWidth);
        srcHStride = ROUND_16(srcHeight);
        srcYUVbufSizeArray[0] =  srcWStride * srcHStride;
        srcYUVbufSizeArray[1] = (srcWStride * srcHStride) /2;

        srcBufferSize = srcYUVbufSizeArray[0] + srcYUVbufSizeArray[1] ;
        srcYUVbufArray[0] = srcYUVbuf_va;
        srcYUVbufArray[1] = srcYUVbuf_va + srcYUVbufSizeArray[0];
        srcNumPlanes = 2;
        srcUvStrideMode = UV_STRIDE_16_8_8;


    } else if(srcFormat == OMX_COLOR_Format32BitRGBA8888 || srcFormat == OMX_COLOR_Format32bitARGB8888) {
        srcWStride =ROUND_16(srcWidth);
        srcHStride = ROUND_16(srcHeight);
        srcYUVbufSizeArray[0] =  srcWStride * srcHStride*4;
        srcBufferSize = srcWStride * srcHStride*4;
        srcYUVbufArray[0] = srcYUVbuf_va;
        srcNumPlanes = 1;

    } else {
        VT_LOGE("ERROR not supported color format: srcFormat(%s)", OmxFormatToString(srcFormat));
    }

    if(tarFormat == OMX_MTK_COLOR_FormatYV12) {
        tarWStride =ROUND_16(tarWidth);
        tarHStride = ROUND_16(tarHeight);
        dstYUVbufSizeArray[0] =  tarWStride * tarHStride;
        dstYUVbufSizeArray[1] =  ROUND_16(tarWStride / 2) * (tarHStride / 2);
        dstYUVbufSizeArray[2] =  ROUND_16(tarWStride / 2) * (tarHStride / 2);
        dstYUVbufArray[0] = dstYUVbuf_va;
        dstYUVbufArray[1] = dstYUVbuf_va + dstYUVbufSizeArray[0];
        dstYUVbufArray[2] = dstYUVbufArray[1] + dstYUVbufSizeArray[1];

        // some chip unable to flush all, so we shouldn't touch this buffer
        //memset(dstYUVbuf_va, 0x10, dstYUVbufSizeArray[0]);
        //memset(dstYUVbuf_va + dstYUVbufSizeArray[0], 128, dstYUVbufSizeArray[1]+dstYUVbufSizeArray[2]);
        dstBufferSize = dstYUVbufSizeArray[0] + dstYUVbufSizeArray[1] + dstYUVbufSizeArray[2];

        tarNumPlanes = 3;
        tarUvStrideMode = UV_STRIDE_16_16_16;


    } else if(tarFormat == OMX_COLOR_FormatYUV420SemiPlanar) {

        tarWStride =ROUND_16(tarWidth);
        tarHStride = ROUND_16(tarHeight);
        dstYUVbufSizeArray[0] =  tarWStride * tarHStride;
        dstYUVbufSizeArray[1] = (tarWStride * tarHStride) /2;
        dstYUVbufArray[0] = dstYUVbuf_va;
        dstYUVbufArray[1] = dstYUVbuf_va + dstYUVbufSizeArray[0];

        // some chip unable to flush all, so we shouldn't touch this buffer
        //memset(dstYUVbuf_va, 0x10, dstYUVbufSizeArray[0]);
        //memset(dstYUVbuf_va + dstYUVbufSizeArray[0], 128, dstYUVbufSizeArray[1]);
        dstBufferSize = dstYUVbufSizeArray[0] + dstYUVbufSizeArray[1];

        tarNumPlanes = 2;
        tarUvStrideMode = UV_STRIDE_16_8_8;

    } else {
        VT_LOGE("ERROR not supported color format: tarFormat(%s)", OmxFormatToString(tarFormat));
    }


    *outFillLen = dstBufferSize;

    
    DpColorFormat srcColorFormat = OmxColorToDpColor(srcFormat);
    DpColorFormat tarColorFormat = OmxColorToDpColor(tarFormat);
   

    //config mdp src and dst parameters
    DpRect srcRoi;
    srcRoi.x = 0;
    srcRoi.y = 0;
    srcRoi.w = ROUND_2(srcWidth);
    srcRoi.h = ROUND_2(srcHeight);

#if USING_MDP_BY_HIDL
	unsigned int ion_fd;
	ion_fd = ion_open();
	if (ion_fd < 0) {
		ALOGE("ion_open(%d) fail\n", ion_fd);
		return ion_fd;
	} 
	int src_ion_handle_tobe_free[srcNumPlanes] ;
	for(uint32_t i =0 ; i < srcNumPlanes; i++){
		uint32_t  mva = 0 ;
		src_ion_handle_tobe_free [i] = 0;
		unsigned long va = (unsigned long)(srcYUVbufArray[i]);
		vilte_ion_va_mva(ion_fd,va,srcYUVbufSizeArray[i],&mva,&(src_ion_handle_tobe_free[i]));
		mpdParams.src_MVAList[i] =  mva;
		mpdParams.src_sizeList[i] = (uint32_t)(srcYUVbufSizeArray[i]);
	}
	mpdParams.src_planeNumber = srcNumPlanes;
#else
	DpBlitStream blitStream;
 	int32_t bliterr = DP_STATUS_RETURN_SUCCESS;
	bliterr = blitStream.setSrcBuffer((void **) srcYUVbufArray, (unsigned int *) srcYUVbufSizeArray, srcNumPlanes);
#endif

	

    unsigned int yPitch = ((srcWStride) * DP_COLOR_BITS_PER_PIXEL(srcColorFormat)) >> 3;
    unsigned int uvPitch = 0;

    switch(srcUvStrideMode) {
    case UV_STRIDE_16_16_16:
        uvPitch = (ROUND_16(srcWStride / 2) * DP_COLOR_BITS_PER_PIXEL(srcColorFormat)) >> 3;
        break;

    case UV_STRIDE_16_8_8:
        uvPitch = ((srcWStride / 2) * DP_COLOR_BITS_PER_PIXEL(srcColorFormat)) >> 3;
        break;

    default:  // use 16_8_8
        uvPitch = ((srcWStride / 2) * DP_COLOR_BITS_PER_PIXEL(srcColorFormat)) >> 3;
        break;
    }

    if(ro_type == ROT_KEEP_RATIO_WITH_CROP) {    //add crop ,should reest the parameters

        int32_t x =  ROUND_2(srcHeight*srcHeight/srcWidth);
        int32_t y =  srcHeight;
        srcRoi.x =ROUND_2((srcWidth - x) /2);
        srcRoi.y = 0;
        srcRoi.w = ROUND_2(x);
        srcRoi.h =  ROUND_2(y);
    }

    if(srcColorFormat == eARGB8888 || srcColorFormat == eRGBA8888) {       
#if USING_MDP_BY_HIDL        
        mpdParams.src_width = srcWidth;mpdParams.src_height = srcHeight;
	 mpdParams.src_format = srcColorFormat;
	 mpdParams.src_rect.x = srcRoi.x;mpdParams.src_rect.y = srcRoi.y;mpdParams.src_rect.w = srcRoi.w;mpdParams.src_rect.h = srcRoi.h;
#else
	 bliterr = blitStream.setSrcConfig(srcWidth, srcHeight,srcColorFormat,  eInterlace_None, &srcRoi);
#endif
    } else {
        
#if USING_MDP_BY_HIDL    
        mpdParams.src_width = srcWidth;mpdParams.src_height = srcHeight;
	 mpdParams.src_format = srcColorFormat; mpdParams.src_yPitch = yPitch;mpdParams.src_uvPitch = uvPitch;//mpdParams.src_profile = 0;//DP_PROFILE_BT601
	 mpdParams.src_rect.x = srcRoi.x;mpdParams.src_rect.y = srcRoi.y;mpdParams.src_rect.w = srcRoi.w;mpdParams.src_rect.h = srcRoi.h;
#else
	bliterr = blitStream.setSrcConfig(srcWidth, srcHeight, yPitch, uvPitch, srcColorFormat, DP_PROFILE_BT601, eInterlace_None, &srcRoi);
#endif
	}


    VT_LOGV("@ s-w %d  s-h%d srcConfig x %d y %d w %d h %d, yPitch%d,uvPitch%d,s-F %d",
            srcWidth,srcHeight,srcRoi.x, srcRoi.y, srcRoi.w, srcRoi.h,yPitch,uvPitch,srcColorFormat);

    //config dst mdp parameters
    DpRect dstRoi;
    dstRoi.x = 0;
    dstRoi.y = 0;
    dstRoi.w = ROUND_2(tarWidth);
    dstRoi.h =  ROUND_2(tarHeight);
    yPitch = ((tarWStride) * DP_COLOR_BITS_PER_PIXEL(tarColorFormat)) >> 3;

    switch(tarUvStrideMode) {
    case UV_STRIDE_16_16_16:
        uvPitch = (ROUND_16(tarWStride / 2) * DP_COLOR_BITS_PER_PIXEL(tarColorFormat)) >> 3;
        break;

    case UV_STRIDE_16_8_8:
        uvPitch = ((tarWStride / 2) * DP_COLOR_BITS_PER_PIXEL(tarColorFormat)) >> 3;
        break;

    default:  // use 16_8_8
        uvPitch = ((tarWStride / 2) * DP_COLOR_BITS_PER_PIXEL(tarColorFormat)) >> 3;
        break;
    }

    uint32_t  resizeW=tarWidth;
    uint32_t  resizeH=tarHeight;

    //rest for type 0
    if(ro_type ==ROT_KEEP_RATIO_WITH_BLACK_EDGE  && (degree % 90 ==0)) {

        resizeW =  ROUND_2(tarHeight*tarHeight/tarWidth);
        resizeH =  tarHeight;
        dstRoi.x =ROUND_2((tarWidth - resizeW) /2);
        dstRoi.y = 0;
        dstRoi.w = resizeW;
        dstRoi.h =  resizeH;
    }

    VT_LOGV("@ t-w %d  t-h%d ,setDstBuffer+setDstConfig x %d y %d w %d h %d,rotate degree %d,yPitch%d,uvPitch%d,t-F %d",
            tarWidth,   tarHeight,dstRoi.x, dstRoi.y, dstRoi.w, dstRoi.h,degree,yPitch,uvPitch,tarColorFormat);
   
#if USING_MDP_BY_HIDL
     int dst_ion_handle_tobe_free[tarNumPlanes];
     for(uint32_t i =0 ; i < tarNumPlanes; i++){
		uint32_t mva = 0;
		dst_ion_handle_tobe_free[i] = 0;
		unsigned long va =  (unsigned long)(dstYUVbufArray[i]);
		vilte_ion_va_mva(ion_fd, va,dstYUVbufSizeArray[i],&mva,&(dst_ion_handle_tobe_free[i]));
		mpdParams.dst_MVAList[i] =   mva;
		mpdParams.dst_sizeList[i] = (uint32_t)(dstYUVbufSizeArray[i]);
 	}
	mpdParams.dst_planeNumber = tarNumPlanes;

	mpdParams.dst_width = resizeW;mpdParams.dst_height = resizeH;
	mpdParams.dst_format = tarColorFormat; mpdParams.dst_yPitch = yPitch;mpdParams.dst_uvPitch = uvPitch;//mpdParams.dst_profile = 0;//DP_PROFILE_BT601
	mpdParams.dst_rect.x = dstRoi.x;mpdParams.dst_rect.y = dstRoi.y;mpdParams.dst_rect.w = dstRoi.w;mpdParams.dst_rect.h = dstRoi.h;
	mpdParams.rotation = degree;
	IMms_service->mdp_run(mpdParams) ;

	 //free mva handle and close ion_fd
	for(uint32_t i =0 ; i < srcNumPlanes; i++){
		VT_LOGV("src  ion_free_handle(%d)  \n", src_ion_handle_tobe_free[i]);
		ion_free(ion_fd, src_ion_handle_tobe_free[i]);
	}

	for(uint32_t i =0 ; i < tarNumPlanes; i++){
		VT_LOGV("dst  ion_free_handle(%d)  \n", src_ion_handle_tobe_free[i]);
	 	ion_free(ion_fd, dst_ion_handle_tobe_free[i]);  
	}
      ion_close(ion_fd);
#else
	bliterr = blitStream.setDstBuffer((void **) dstYUVbufArray, (unsigned int *) dstYUVbufSizeArray, tarNumPlanes);
	bliterr = blitStream.setDstConfig(resizeW, resizeH, yPitch, uvPitch, tarColorFormat, DP_PROFILE_BT601, eInterlace_None, &dstRoi);
	bliterr = blitStream.setRotate(degree);  
	bliterr = blitStream.invalidate();
	
#endif
	
  
    int64_t endTimeUs = systemTime(SYSTEM_TIME_MONOTONIC) / 1000ll;
    int64_t useTimeMs = (endTimeUs-startTimeUs) /1000ll;
    VT_LOGV("[profile]rotate use timeMs %lld ms  ", (long long) useTimeMs);

    char dump[PROPERTY_VALUE_MAX];
    bool dumpYUV = false;

    if(property_get("vendor.vt.ro.dump", dump, NULL)) {
        dumpYUV = atoi(dump);
    }

    if(dumpYUV) {
        char buf[255];

        if(srcFormat == OMX_COLOR_Format32bitARGB8888 || srcFormat == OMX_COLOR_Format32BitRGBA8888) {
            sprintf(buf, "/sdcard/in%d_%d.rgb",srcWidth,srcHeight);
            FILE *fp = fopen(buf, "ab");

            if(fp) {
                fwrite((void *) input, 1, ARGB888_SIZE(srcWidth, srcHeight), fp);
                fclose(fp);
            }


        } else {
            sprintf(buf, "/sdcard/in%d_%d.yuv",srcWidth,srcHeight);
            FILE *fp = fopen(buf, "ab");

            if(fp) {
                fwrite((void *) input, 1, YUV_SIZE(srcWidth, srcHeight), fp);
                fclose(fp);
            }
        }

        sprintf(buf, "/sdcard/outBuf%d_%d.yuv",tarBufWidth,tarBufHeight);
        FILE *fp = fopen(buf, "ab");

        if(fp) {
            fwrite((void *) output, 1, YUV_SIZE(tarBufHeight, tarBufWidth), fp);
            fclose(fp);
        }

        sprintf(buf, "/sdcard/outVid%d_%d.yuv",tarWStride,tarHStride);
        fp = fopen(buf, "ab");

        if(fp) {
            fwrite((void *) output, 1, YUV_SIZE(tarWStride, tarHStride), fp);
            fclose(fp);
        }
    }
    //useTimeMs = (bliterr < 0)?bliterr:useTimeMs;
    return useTimeMs;

}
#else
int64_t rotateBuffer(uint8_t *input,uint8_t *output, RotateInfo *info,int32_t *outFillLen)
{

    VT_LOGE("should not called !!!:input %p,output %p, info %p *outFillLen %d",
            	 input,output,info,*outFillLen);
	return -1;
}

#endif

void mtk_scaling_list(int32_t sizeOfScalingList, ABitReader *br)
{
    int lastScale = 8;
    int nextScale = 8;

    for(int j = 0; j < sizeOfScalingList; j++) {
        if(nextScale != 0) {
            int32_t delta_scale = parseSE(br);
            nextScale = (lastScale + delta_scale + 256) % 256;
        }

        lastScale = (nextScale == 0) ? lastScale : nextScale;
    }
}

void mtk_parse_seq_scaling_matrix_present(ABitReader *br)
{
    for(int i = 0; i < 8; i++) {
        uint32_t seq_scaling_list_present_flag =  br->getBits(1);
        ALOGV("seq_scaling_list_presetn_flag :%d", seq_scaling_list_present_flag);

        if(seq_scaling_list_present_flag) {
            if(i < 6)
                mtk_scaling_list(16, br);
            else
                mtk_scaling_list(64, br);
        }
    }
}


void MTKFindAVCSPSInfo(
    uint8_t *seqParamSet, size_t size, struct MtkSPSInfo *pSPSInfo)
{
    if(pSPSInfo == NULL) {
        ALOGE("pSPSInfo == NULL");
        return ;
    }

    ABitReader br(seqParamSet + 1, size - 1);

    unsigned profile_idc = br.getBits(8);
    pSPSInfo->profile = profile_idc;
    br.skipBits(8);

    pSPSInfo->level = br.getBits(8);
    parseUE(&br);    // seq_parameter_set_id

    unsigned chroma_format_idc = 1;  // 4:2:0 chroma format

    if(profile_idc == 100 || profile_idc == 110
            || profile_idc == 122 || profile_idc == 244
            || profile_idc == 44 || profile_idc == 83 || profile_idc == 86) {
        chroma_format_idc = parseUE(&br);

        if(chroma_format_idc == 3) {
            br.skipBits(1);    // residual_colour_transform_flag
        }

        parseUE(&br);    // bit_depth_luma_minus8
        parseUE(&br);    // bit_depth_chroma_minus8
        br.skipBits(1);    // qpprime_y_zero_transform_bypass_flag

        // CHECK_EQ(br.getBits(1), 0u);  // seq_scaling_matrix_present_flag
        if(br.getBits(1) != 0) {
            ALOGW("seq_scaling_matrix_present_flag != 0");
            mtk_parse_seq_scaling_matrix_present(&br);
        }
    }

    parseUE(&br);    // log2_max_frame_num_minus4
    unsigned pic_order_cnt_type = parseUE(&br);

    if(pic_order_cnt_type == 0) {
        parseUE(&br);    // log2_max_pic_order_cnt_lsb_minus4
    } else if(pic_order_cnt_type == 1) {
        // offset_for_non_ref_pic, offset_for_top_to_bottom_field and
        // offset_for_ref_frame are technically se(v), but since we are
        // just skipping over them the midpoint does not matter.

        br.getBits(1);    // delta_pic_order_always_zero_flag
        parseUE(&br);    // offset_for_non_ref_pic
        parseUE(&br);    // offset_for_top_to_bottom_field

        unsigned num_ref_frames_in_pic_order_cnt_cycle = parseUE(&br);

        for(unsigned i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; ++i) {
            parseUE(&br);    // offset_for_ref_frame
        }
    }

    parseUE(&br);    // num_ref_frames
    br.getBits(1);    // gaps_in_frame_num_value_allowed_flag

    unsigned pic_width_in_mbs_minus1 = parseUE(&br);
    unsigned pic_height_in_map_units_minus1 = parseUE(&br);
    unsigned frame_mbs_only_flag = br.getBits(1);

    pSPSInfo->width = pic_width_in_mbs_minus1 * 16 + 16;

    pSPSInfo->height = (2 - frame_mbs_only_flag)
                       * (pic_height_in_map_units_minus1 * 16 + 16);

    if(!frame_mbs_only_flag) {
        br.getBits(1);    // mb_adaptive_frame_field_flag
    }

    br.getBits(1);    // direct_8x8_inference_flag

    if(br.getBits(1)) {       // frame_cropping_flag
        unsigned frame_crop_left_offset = parseUE(&br);
        unsigned frame_crop_right_offset = parseUE(&br);
        unsigned frame_crop_top_offset = parseUE(&br);
        unsigned frame_crop_bottom_offset = parseUE(&br);

        unsigned cropUnitX, cropUnitY;

        if(chroma_format_idc == 0  /* monochrome */) {
            cropUnitX = 1;
            cropUnitY = 2 - frame_mbs_only_flag;
        } else {
            unsigned subWidthC = (chroma_format_idc == 3) ? 1 : 2;
            unsigned subHeightC = (chroma_format_idc == 1) ? 2 : 1;

            cropUnitX = subWidthC;
            cropUnitY = subHeightC * (2 - frame_mbs_only_flag);
        }

        ALOGV("frame_crop = (%u, %u, %u, %u), cropUnitX = %u, cropUnitY = %u",
              frame_crop_left_offset, frame_crop_right_offset,
              frame_crop_top_offset, frame_crop_bottom_offset,
              cropUnitX, cropUnitY);

        pSPSInfo->width -=
            (frame_crop_left_offset + frame_crop_right_offset) * cropUnitX;
        pSPSInfo->height -=
            (frame_crop_top_offset + frame_crop_bottom_offset) * cropUnitY;
    }

    return ;
}


sp<ABuffer> MakeAVCCodecSpecificData(
    const char *params,
    int32_t *profile,int32_t *level,
    int32_t *width, int32_t *height, int32_t *sarWidth, int32_t *sarHeight)
{

    AString val(params);
    Vector<sp<ABuffer> > paramSets;

    size_t numSeqParameterSets = 0;
    size_t totalSeqParameterSetSize = 0;
    size_t numPicParameterSets = 0;
    size_t totalPicParameterSetSize = 0;
    *profile = 0;
    *level = 0;

    VT_LOGD("params : %s width=%d height=%d sarWidth=%d sarHeight=%d",
            val.c_str(), *width, *height, *sarWidth, *sarHeight);

    //VT_LOGD("params : %s",val.c_str());

    size_t start = 0;
    int loopCount = 0;

    for(;;) {
        bool parseErr = false;
        ssize_t commaPos = val.find(",", start);
        size_t end = (commaPos < 0) ? val.size() : commaPos;

        AString nalString(val, start, end - start);
        VT_LOGD("nalString : %s,start %zu",nalString.c_str(),start);

        sp<ABuffer> nal = decodeBase64(nalString);

        if(nal == NULL) {
            VT_LOGI("params error 1 @ start %zu",start);
            return NULL;
        }

        //maybe have some tuncate SPS in CMCC...
        if(loopCount++ == 0 && nal->size() < 10) {
            VT_LOGE("find SPS nal_size too little %zu",nal->size());
            return NULL;
        }

        int32_t leftSize = RemovePreventionByte(nal->data(), nal->size());
        nal->setRange(0, leftSize);


        if(nal->size() <= 0) {
            VT_LOGE("params error 2 @ start %zu",start);
            parseErr = true;
        }

        if(nal->size() >=65535u) {
            VT_LOGE("params error 3 @ start %zu",start);
            parseErr = true;
        }

        if(parseErr) {
            if(numSeqParameterSets == 0) {
                VT_LOGE("can not find right sps, return NULL");
                return NULL;
            } else {
                VT_LOGE("find %zu sps, %zu pps,",numSeqParameterSets,numPicParameterSets);
                break;
            }
        }

        uint8_t nalType = nal->data() [0] & 0x1f;

        if(numSeqParameterSets == 0) {
            CHECK_EQ((unsigned) nalType, 7u);      //sps
        } else if(numPicParameterSets > 0) {
            CHECK_EQ((unsigned) nalType, 8u);      //pps
        }

        if(nalType == 7) {
            ++numSeqParameterSets;
            totalSeqParameterSetSize += nal->size();
        } else  {
            CHECK_EQ((unsigned) nalType, 8u);
            ++numPicParameterSets;
            totalPicParameterSetSize += nal->size();
        }

        paramSets.push(nal);

        if(commaPos < 0) {
            break;
        }

        start = commaPos + 1;
    }

    CHECK_LT(numSeqParameterSets, 32u);
    CHECK_LE(numPicParameterSets, 255u);

    size_t csdSize =
        1 + 3 + 1 + 1 /*configurationVersion +profilelevel +lengthSizes */
        + 2 * numSeqParameterSets/*size field*/ + totalSeqParameterSetSize
        + 1 + 2 * numPicParameterSets/*size field*/ + totalPicParameterSetSize;

    sp<ABuffer> csd = new ABuffer(csdSize);
    uint8_t *out = csd->data();

    *out++ = 0x01;  // configurationVersion

    sp<ABuffer> nal = paramSets.editItemAt(0);
    memcpy(out, nal->data() + 1, 3);    // profile/level... memcpy(out, profilelevel, 3);
    VT_LOGD("P L %p", (nal->data() + 1));
    out += 3;

    *out++ = (0x3f << 2) | 1;   // lengthSize == 2 bytes
    *out++ = 0xe0 | numSeqParameterSets;


    for(size_t i = 0; i < numSeqParameterSets; ++i) {
        sp<ABuffer> nal = paramSets.editItemAt(i);


        *out++ = nal->size() >> 8;
        *out++ = nal->size() & 0xff;

        memcpy(out, nal->data(), nal->size());

        out += nal->size();

        if(i == 0) {
            FindAVCDimensions(nal, width, height);
            MtkSPSInfo  spsInf;
            MTKFindAVCSPSInfo(nal->data(),nal->size(),&spsInf);
            VT_LOGD("W %d H %d P %d L %d",spsInf.width,spsInf.height,spsInf.profile,spsInf.level);
            *profile = spsInf.profile;
            *level = spsInf.level;
        }
    }

    *out++ = numPicParameterSets;

    for(size_t i = 0; i < numPicParameterSets; ++i) {
        sp<ABuffer> nal = paramSets.editItemAt(i + numSeqParameterSets);

        *out++ = nal->size() >> 8;
        *out++ = nal->size() & 0xff;

        memcpy(out, nal->data(), nal->size());

        out += nal->size();
    }

    // hexdump(csd->data(), csd->size());

    return csd;
}
uint16_t U16_AT(const uint8_t *ptr)
{
    return ptr[0] << 8 | ptr[1];
}
sp<ABuffer> parseAVCCodecSpecificData(const uint8_t *data, size_t size)
{
    const uint8_t *ptr =  data;
    sp<ABuffer> csd = new ABuffer(1024);
    static const uint8_t kNALStartCode[4] = { 0x00, 0x00, 0x00, 0x01 };

    // verify minimum size and configurationVersion == 1.
    if(size < 7 || ptr[0] != 1) {
        CHECK(0);
    }


    // There is decodable content out there that fails the following
    // assertion, let's be lenient for now...
    // CHECK((ptr[4] >> 2) == 0x3f);  // reserved

    //size_t lengthSize = 1 + (ptr[4] & 3);

    // commented out check below as H264_QVGA_500_NO_AUDIO.3gp
    // violates it...
    // CHECK((ptr[5] >> 5) == 7);  // reserved

    size_t numSeqParameterSets = ptr[5] & 31;

    ptr += 6;
    size -= 6;

    uint32_t totalSize = 0;

    for(size_t i = 0; i < numSeqParameterSets; ++i) {
        if(size < 2) {
            CHECK(0);
        }

        size_t length = U16_AT(ptr);

        ptr += 2;
        size -= 2;

        if(size < length) {
            CHECK(0);
        }


        memcpy(csd->data() +totalSize, kNALStartCode, 4);
        memcpy(csd->data() + 4 + totalSize, ptr,length);
        totalSize += length + 4;

        csd->setRange(0, totalSize);

        ptr += length;
        size -= length;
    }

    if(size < 1) {
        CHECK(0);
    }

    size_t numPictureParameterSets = *ptr;
    ++ptr;
    --size;

    for(size_t i = 0; i < numPictureParameterSets; ++i) {
        if(size < 2) {
            CHECK(0);
        }

        size_t length = U16_AT(ptr);

        ptr += 2;
        size -= 2;

        if(size < length) {
            CHECK(0);
        }

        memcpy(csd->data() +totalSize, kNALStartCode, 4);
        memcpy(csd->data() + 4 + totalSize, ptr,length);
        totalSize += length + 4;
        csd->setRange(0, totalSize);


        ptr += length;
        size -= length;
    }

    return csd;
}
void dumpFileToPath(const char* path,const sp<ABuffer> &buffer,bool appendStartCode)
{
    FILE *fp = fopen(path, "ab");
    uint8_t startCodec[4] = {0,0,0,1};

    if(fp) {
        if(appendStartCode) {
            fwrite((void *) startCodec, 1, 4, fp);
        }

        fwrite((void *) buffer->data(), 1, buffer->size(), fp);
        fclose(fp);
    }

}
void dumpRawYUVToPath(const char* path,void * data,int32_t size)
{
    FILE *fp = fopen(path, "ab");

    if(fp) {
        fwrite(data, 1,  size , fp);
        fclose(fp);
    }

}
/*
00 00 00
Profile 00 Level

Profile valid value
Profile Value
Baseline    66
Main    77
High    100

Level
10  1       (supports only QCIF format and below with 380160 samples/sec)
11  1.1    (CIF and below. 768000 samples/sec)
12  1.2    (CIF and below. 1536000 samples/sec)
13  1.3    (CIF and below. 3041280 samples/sec)
20  2       (CIF and below. 3041280 samples/sec)
21  2.1    (Supports HHR formats. Enables Interlace support. 5068800 samples/sec)
22  2.2    (Supports SD/4CIF formats. Enables Interlace support. 5184000 samples/sec)
30  3       (Supports SD/4CIF formats. Enables Interlace support. 10368000 samples/sec)
31  3.1    (Supports 720p HD format. Enables Interlace support. 27648000 samples/sec)

typedef enum OMX_VIDEO_HEVCPROFILETYPE {
    OMX_VIDEO_HEVCProfileUnknown = 0x0,
    OMX_VIDEO_HEVCProfileMain    = 0x1,
    OMX_VIDEO_HEVCProfileMain10  = 0x2,
    OMX_VIDEO_HEVCProfileMax     = 0x7FFFFFFF
} OMX_VIDEO_HEVCPROFILETYPE;


typedef enum OMX_VIDEO_HEVCLEVELTYPE {
    OMX_VIDEO_HEVCLevelUnknown    = 0x0,
    OMX_VIDEO_HEVCMainTierLevel1  = 0x1,
    OMX_VIDEO_HEVCHighTierLevel1  = 0x2,
    OMX_VIDEO_HEVCMainTierLevel2  = 0x4,
    OMX_VIDEO_HEVCHighTierLevel2  = 0x8,
    OMX_VIDEO_HEVCMainTierLevel21 = 0x10,
    OMX_VIDEO_HEVCHighTierLevel21 = 0x20,
    OMX_VIDEO_HEVCMainTierLevel3  = 0x40,
    OMX_VIDEO_HEVCHighTierLevel3  = 0x80,
    OMX_VIDEO_HEVCMainTierLevel31 = 0x100,
    OMX_VIDEO_HEVCHighTierLevel31 = 0x200,
    OMX_VIDEO_HEVCMainTierLevel4  = 0x400,
    OMX_VIDEO_HEVCHighTierLevel4  = 0x800,
    OMX_VIDEO_HEVCMainTierLevel41 = 0x1000,
    OMX_VIDEO_HEVCHighTierLevel41 = 0x2000,
    OMX_VIDEO_HEVCMainTierLevel5  = 0x4000,
    OMX_VIDEO_HEVCHighTierLevel5  = 0x8000,
    OMX_VIDEO_HEVCMainTierLevel51 = 0x10000,
    OMX_VIDEO_HEVCHighTierLevel51 = 0x20000,
    OMX_VIDEO_HEVCMainTierLevel52 = 0x40000,
    OMX_VIDEO_HEVCHighTierLevel52 = 0x80000,
    OMX_VIDEO_HEVCMainTierLevel6  = 0x100000,
    OMX_VIDEO_HEVCHighTierLevel6  = 0x200000,
    OMX_VIDEO_HEVCMainTierLevel61 = 0x400000,
    OMX_VIDEO_HEVCHighTierLevel61 = 0x800000,
    OMX_VIDEO_HEVCMainTierLevel62 = 0x1000000,
    OMX_VIDEO_HEVCHighTierLevel62 = 0x2000000,
    OMX_VIDEO_HEVCHighTiermax     = 0x7FFFFFFF
} OMX_VIDEO_HEVCLEVELTYPE;


Level   level_id    OMX_VIDEO_HEVCLEVELTYPE
1.0 30  OMX_VIDEO_HEVCMainTierLevel1
2.0 60  OMX_VIDEO_HEVCMainTierLevel2
2.1 63  OMX_VIDEO_HEVCMainTierLevel21
3   90  OMX_VIDEO_HEVCMainTierLevel3
3.1 93  OMX_VIDEO_HEVCMainTierLevel31
4.0 120 OMX_VIDEO_HEVCMainTierLevel4
4.1 123 OMX_VIDEO_HEVCMainTierLevel41
5.0 150 OMX_VIDEO_HEVCMainTierLevel5
5.1 153 OMX_VIDEO_HEVCMainTierLevel51
5.2 156 OMX_VIDEO_HEVCMainTierLevel52
6.0 180 OMX_VIDEO_HEVCMainTierLevel6
6.1 183 OMX_VIDEO_HEVCMainTierLevel61
6.2 186 OMX_VIDEO_HEVCMainTierLevel62
level_id = Level * 30
0 < level_id < 255



*/
//TODO, add HEVC related value
int32_t convertToOMXProfile(const char* mimetype,int32_t profile)
{
    bool isH264 = (!strcasecmp(mimetype, MEDIA_MIMETYPE_VIDEO_AVC));
    bool isHEVC= (!strcasecmp(mimetype, MEDIA_MIMETYPE_VIDEO_HEVC));

    if(isH264) {
        if(profile ==  66) {
            return  OMX_VIDEO_AVCProfileBaseline;
        } else if(profile == 77) {
            return  OMX_VIDEO_AVCProfileMain;
        } else if(profile == 100) {
            return  OMX_VIDEO_AVCProfileHigh;
        }
    }

    if(isHEVC) {
        if(profile ==  1) {
            return  OMX_VIDEO_HEVCProfileMain;
        } else if(profile == 2) {
            return  OMX_VIDEO_HEVCProfileMain10;
        } else if(profile == 0x7FFFFFFF) {
            return  OMX_VIDEO_HEVCProfileMax;
        }
    }

    VT_LOGE("not support profile");
    return -1;
}
int32_t convertToOMXLevel(const char* mimetype,int32_t profile,int32_t level)
{
    bool isH264 = (!strcasecmp(mimetype, MEDIA_MIMETYPE_VIDEO_AVC));
    bool isHEVC= (!strcasecmp(mimetype, MEDIA_MIMETYPE_VIDEO_HEVC));

    if(isH264) {
        if(level == 10) {
            return  OMX_VIDEO_AVCLevel1;
        } else if(level == 11) {
            return  OMX_VIDEO_AVCLevel11;
        } else if(level == 12) {
            return  OMX_VIDEO_AVCLevel12;
        } else if(level == 13) {
            return  OMX_VIDEO_AVCLevel13;
        } else if(level == 20) {
            return  OMX_VIDEO_AVCLevel2;
        } else if(level == 21) {
            return  OMX_VIDEO_AVCLevel21;
        } else if(level == 22) {
            return  OMX_VIDEO_AVCLevel22;
        } else if(level == 30) {
            return  OMX_VIDEO_AVCLevel3;
        } else if(level == 31) {
            return  OMX_VIDEO_AVCLevel31;
        } else if(level == 32) {
            return  OMX_VIDEO_AVCLevel32;
        } else if(level == 40) {
            return  OMX_VIDEO_AVCLevel4;
        } else if(level == 41) {
            return  OMX_VIDEO_AVCLevel41;
        } else if(level == 42) {
            return  OMX_VIDEO_AVCLevel42;
        } else if(level == 50) {
            return  OMX_VIDEO_AVCLevel5;
        } else if(level == 51) {
            return  OMX_VIDEO_AVCLevel51;
        } else if(level == 52) {
            return  OMX_VIDEO_AVCLevel52;
        }
    }

    int32_t OMXProfile = convertToOMXProfile(mimetype,profile);

    if(isHEVC && OMXProfile == OMX_VIDEO_HEVCProfileMain) {
        if(level == 30) {
            return  OMX_VIDEO_HEVCMainTierLevel1;
        } else if(level == 60) {
            return  OMX_VIDEO_HEVCMainTierLevel2;
        } else if(level == 63) {
            return  OMX_VIDEO_HEVCMainTierLevel21;
        } else if(level == 90) {
            return  OMX_VIDEO_HEVCMainTierLevel3;
        } else if(level == 93) {
            return  OMX_VIDEO_HEVCMainTierLevel31;
        } else if(level == 120) {
            return  OMX_VIDEO_HEVCMainTierLevel4;
        } else if(level == 123) {
            return  OMX_VIDEO_HEVCMainTierLevel41;
        } else if(level == 150) {
            return  OMX_VIDEO_HEVCMainTierLevel5;
        } else if(level == 153) {
            return  OMX_VIDEO_HEVCMainTierLevel51;
        } else if(level == 156) {
            return  OMX_VIDEO_HEVCMainTierLevel52;
        } else if(level == 180) {
            return  OMX_VIDEO_HEVCMainTierLevel6;
        } else if(level == 183) {
            return  OMX_VIDEO_HEVCMainTierLevel61;
        } else if(level == 186) {
            return  OMX_VIDEO_HEVCMainTierLevel62;
        }
    }

    VT_LOGE("not support level");
    return -1;
}
//CCW:counter-clockwise
int32_t convertToCCWRotationDegree(bool isCcw, int32_t degree)
{
    if(isCcw) return degree;

    return 360-degree;
}
//CCW:clockwise
int32_t convertToCWRotationDegree(bool isCw, int32_t degree)
{
    if(isCw) return degree;

    return 360-degree;
}

sp<ABuffer> MakeHEVCCodecSpecificData(
    const char *params, int32_t *width, int32_t *height)
{
    VT_LOGD("params: %s",params);


    AString val(params);
    Vector<sp<ABuffer> > paramSets;
    size_t numVPS = 0;
    size_t numSPS = 0;
    size_t numPPS = 0;
    size_t totalparamSetsSize = 0;

    const uint8_t VPS_NAL_TYPE = 32;
    const uint8_t SPS_NAL_TYPE = 33;
    const uint8_t PPS_NAL_TYPE = 34;
    static const uint8_t kNALStartCode[4] = { 0x00, 0x00, 0x00, 0x01 };

    VT_LOGD("val : %s",val.c_str());

    size_t start = 0;
    int loopCount = 0;

    for(;;) {
        bool parseErr = false;
        ssize_t commaPos = val.find(",", start);
        size_t end = (commaPos < 0) ? val.size() : commaPos;

        AString nalString(val, start, end - start);
        VT_LOGD("nalString : %s,start %zu",nalString.c_str(),start);

        sp<ABuffer> nal = decodeBase64(nalString);

        if(nal == NULL) {
            VT_LOGI("params error 1 @ start %zu",start);
            return NULL;
        }

        //maybe have some tuncate SPS in CMCC...
        if(loopCount++ == 0 && nal->size() < 10) {
            VT_LOGE("find SPS nal_size too little %zu",nal->size());
            return NULL;
        }

        printBinary(nal->data(), nal->size());

        sp<ABuffer> nalWoPreventionByte = ABuffer::CreateAsCopy(nal->data(),nal->size());
        RemovePreventionByte(nalWoPreventionByte->data(), nalWoPreventionByte->size());

        if(nalWoPreventionByte == NULL) {
            VT_LOGI("params error 1 @ start %zu",start);
            parseErr = true;
        }

        if(nalWoPreventionByte->size() <= 0) {
            VT_LOGE("params error 2 @ start %zu",start);
            parseErr = true;
        }

        if(nalWoPreventionByte->size() >=65535u) {
            VT_LOGE("params error 3 @ start %zu",start);
            parseErr = true;
        }

        if(parseErr) {
            if(numVPS == 0 &&  numSPS == 0 &&  numVPS == 0) {
                VT_LOGE("can not find any valid vps/sps/pps, return NULL");
                return NULL;
            } else if(numSPS == 0 &&  numVPS == 0) {
                VT_LOGE("can not find any valid /sps/pps, return NULL");
                return NULL;
            } else {
                VT_LOGE("find %zu numVPS, %zu numSPS,, %zu numVPS",numVPS,numSPS,numVPS);
                break;
            }
        }

        printBinary(nalWoPreventionByte->data(), nalWoPreventionByte->size());
        uint8_t nalType = (nalWoPreventionByte->data() [0] >> 1) & 0x3f;
        VT_LOGD("nalType= %d",nalType);

        if(nalType == VPS_NAL_TYPE) {
            numVPS++;
        } else if(nalType == SPS_NAL_TYPE) {
            numSPS++;
        } else if(nalType == PPS_NAL_TYPE) {
            numPPS++;
        }

        paramSets.push(nal);
        totalparamSetsSize += nal->size();

        if(nalType == SPS_NAL_TYPE) {
            FindHEVCWH(nalWoPreventionByte, width, height);
        }

        if(commaPos < 0) {
            break;
        }

        start = commaPos + 1;
    }

    sp<ABuffer> accessUnit = new ABuffer(totalparamSetsSize + paramSets.size() *4);
    uint32_t shiftSize = 0;

    for(size_t i = 0; i < paramSets.size(); i++) {
        sp<ABuffer> tmpNal = paramSets.editItemAt(i);

        memcpy(accessUnit->data() +shiftSize , kNALStartCode,4);
        memcpy(accessUnit->data() +shiftSize+4 , tmpNal->data(),tmpNal->size());
        shiftSize += tmpNal->size() +4;
    }

    CHECK(shiftSize == (totalparamSetsSize + 4*paramSets.size()));

    //now we can direct return the accessUnit
    ALOGI("found HEVC codec config W %d H%d",*width, *height);
    printBinary(accessUnit->data(), accessUnit->size());
    return accessUnit;




    //extract all NAL info
    /*
    const uint8_t *data = accessUnit->data();
    size_t size = accessUnit->size();
    size_t numOfParamSets = 0;

    // find vps,only choose the first vps,
    // need check whether need sent all the vps to decoder
    sp<ABuffer> videoParamSet = FindHEVCNAL(data, size, VPS_NAL_TYPE, NULL);
    if (videoParamSet == NULL) {
        ALOGW("no vps found !!!");
        // return NULL;
    } else {
        numOfParamSets++;
        ALOGI("find vps, size =%zu", videoParamSet->size());
    }

    // find sps,only choose the first sps,
    // need check whether need sent all the sps to decoder
    sp<ABuffer> seqParamSet = FindHEVCNAL(data, size, SPS_NAL_TYPE, NULL);
    if (seqParamSet == NULL) {
        ALOGW("no sps found !!!");
        return NULL;
    } else {
        numOfParamSets++;
        ALOGI("find sps, size =%zu", seqParamSet->size());
    }


    //int32_t width, height;
    FindHEVCDimensions(seqParamSet, &width, &height);

    // find pps,only choose the first pps,
    // need check whether need sent all the pps to decoder
    size_t stopOffset;
    sp<ABuffer> picParamSet = FindHEVCNAL(data, size, PPS_NAL_TYPE, &stopOffset);
    if (picParamSet == NULL) {
        ALOGW("no sps found !!!");
        return NULL;
    } else {
        numOfParamSets++;
        ALOGI("find pps, size =%zu", picParamSet->size());
    }

    int32_t numbOfArrays = numOfParamSets;
    int32_t paramSetSize = 0;

    // only save one vps,sps,pps in codecConfig data
    if (videoParamSet != NULL) {
        paramSetSize += 1 + 2 + 2 + videoParamSet->size();
    }
    if (seqParamSet != NULL) {
        paramSetSize += 1 + 2 + 2 + seqParamSet->size();
    }
    if (picParamSet != NULL) {
        paramSetSize += 1 + 2 + 2 + picParamSet->size();
    }
    size_t csdSize =
        1 + 1 + 4 + 6 + 1 + 2 + 1 + 1 + 1 + 1 + 2 + 1
        + 1 + paramSetSize;
    ALOGI("MakeHEVCCodecSpecificData codec config data size =%zu", csdSize);
    sp<ABuffer> csd = new ABuffer(csdSize);
    uint8_t *out = csd->data();

    *out++ = 0x01;  // configurationVersion

    //opy profile_tier_leve info in sps, containing
     // 1 byte:general_profile_space(2),general_tier_flag(1),general_profile_idc(5)
    //  4 bytes: general_profile_compatibility_flags, 6 bytes: general_constraint_indicator_flags
    //  1 byte:general_level_idc

    memcpy(out, seqParamSet->data() + 3, 1 + 4 + 6 + 1);

    uint8_t profile = out[0] & 0x1f;
    uint8_t level = out[11];


    out += 1 + 4 + 6 + 1;

    *out++ = 0xf0;  // reserved(1111b) + min_spatial_segmentation_idc(4)
    *out++ = 0x00;  // min_spatial_segmentation_idc(8)
    *out++ = 0xfc;  // reserved(6bits,111111b) + parallelismType(2)(0=unknow,1=slices,2=tiles,3=WPP)
    *out++ = 0xfd;  // reserved(6bits,111111b)+chromaFormat(2)(0=monochrome, 1=4:2:0, 2=4:2:2, 3=4:4:4)

    *out++ = 0xf8;  // reserved(5bits,11111b) + bitDepthLumaMinus8(3)
    *out++ = 0xf8;  // reserved(5bits,11111b) + bitDepthChromaMinus8(3)

    uint16_t avgFrameRate = 0;
    // avgFrameRate (16bits,in units of frames/256 seconds,0 indicates an unspecified average frame rate)
    *out++ = avgFrameRate >> 8;
    *out++ = avgFrameRate & 0xff;

    // constantFrameRate(2bits,0=not be of constant frame rate),numTemporalLayers(3bits),temporalIdNested(1bits),
    *out++ = 0x03;
    // lengthSizeMinusOne(2bits)

    *out++ = numbOfArrays;  // numOfArrays

    if (videoParamSet != NULL) {
        *out++ = 0x3f & VPS_NAL_TYPE;  // array_completeness(1bit)+reserved(1bit,0)+NAL_unit_type(6bits)

        // num of vps
        uint16_t numNalus = 1;
        *out++ = numNalus >> 8;
        *out++ =  numNalus & 0xff;

        // vps nal length
        *out++ = videoParamSet->size() >> 8;
        *out++ = videoParamSet->size() & 0xff;

        memcpy(out, videoParamSet->data(), videoParamSet->size());
        out += videoParamSet->size();
    }

    if (seqParamSet != NULL) {
        *out++ = 0x3f & SPS_NAL_TYPE;  // array_completeness(1bit)+reserved(1bit,0)+NAL_unit_type(6bits)

        // num of sps
        uint16_t numNalus = 1;
        *out++ = numNalus >> 8;
        *out++ = numNalus & 0xff;

        // sps nal length
        *out++ = seqParamSet->size() >> 8;
        *out++ = seqParamSet->size() & 0xff;

            memcpy(out, seqParamSet->data(), seqParamSet->size());
            out += seqParamSet->size();
        }
        if (picParamSet != NULL) {
            *out++ = 0x3f & PPS_NAL_TYPE;  // array_completeness(1bit)+reserved(1bit,0)+NAL_unit_type(6bits)

            // num of pps
            uint16_t numNalus = 1;
            *out++ = numNalus >> 8;
            *out++ = numNalus & 0xff;

            // pps nal length
            *out++ = picParamSet->size() >> 8;
            *out++ = picParamSet->size() & 0xff;

            memcpy(out, picParamSet->data(), picParamSet->size());
            // no need add out offset
        }


        ALOGI("found HEVC codec config W %d H%d,  profile%d  level %d",
                width, height, profile, level);

        return csd; */
}

void FindHEVCWH(const sp<ABuffer> &seqParamSet, int32_t *width, int32_t *height)
{
    ALOGI("FindHEVCWH ++");
    ABitReader br(seqParamSet->data() + 2, seqParamSet->size() - 1);

    br.skipBits(4);    // sps_video_parameter_set_id;
    unsigned sps_max_sub_layers_minus1 = br.getBits(3);
    ALOGI("sps_max_sub_layers_minus1 =%d", sps_max_sub_layers_minus1);
    br.skipBits(1);    // sps_temporal_id_nesting_flag;

    /*-----------profile_tier_level start-----------------------*/

    br.skipBits(3);    // general_profile_spac, general_tier_flag

    unsigned general_profile_idc = br.getBits(5);
    ALOGI("general_profile_idc =%d", general_profile_idc);
    br.skipBits(32);    // general_profile_compatibility_flag

    br.skipBits(48);    // general_constraint_indicator_flags

    unsigned general_level_idc = br.getBits(8);
    ALOGI("general_level_idc =%d", general_level_idc);

    uint8_t sub_layer_profile_present_flag[sps_max_sub_layers_minus1];
    uint8_t sub_layer_level_present_flag[sps_max_sub_layers_minus1];

    for(int i = 0; (unsigned) i < sps_max_sub_layers_minus1; i++) {
        sub_layer_profile_present_flag[i] = br.getBits(1);
        sub_layer_level_present_flag[i] = br.getBits(1);
    }

    if(sps_max_sub_layers_minus1 > 0) {
        for(int j = sps_max_sub_layers_minus1; j < 8; j++) {
            br.skipBits(2);
        }
    }

    for(int i = 0; (unsigned) i < sps_max_sub_layers_minus1; i++) {
        if(sub_layer_profile_present_flag[i]) {
            br.skipBits(2);    // sub_layer_profile_space
            br.skipBits(1);    // sub_layer_tier_flag
            br.skipBits(5);    // sub_layer_profile_idc
            br.skipBits(32);    // sub_layer_profile_compatibility_flag
            br.skipBits(48);    // sub_layer_constraint_indicator_flags
        }

        if(sub_layer_level_present_flag[i]) {
            br.skipBits(8);    // sub_layer_level_idc
        }
    }

    /*-----------profile_tier_level done-----------------------*/

    parseUE(&br);    // sps_seq_parameter_set_id
    unsigned chroma_format_idc, separate_colour_plane_flag;
    chroma_format_idc = parseUE(&br);
    ALOGI("chroma_format_idc=%d", chroma_format_idc);

    if(chroma_format_idc == 3) {
        separate_colour_plane_flag = br.getBits(1);
    }

    int32_t pic_width_in_luma_samples = parseUE(&br);
    int32_t pic_height_in_luma_samples = parseUE(&br);
    ALOGI("pic_width_in_luma_samples =%d", pic_width_in_luma_samples);
    ALOGI("pic_height_in_luma_samples =%d", pic_height_in_luma_samples);

    *width = pic_width_in_luma_samples;
    *height = pic_height_in_luma_samples;
    uint8_t conformance_window_flag = br.getBits(1);
    ALOGI("conformance_window_flag =%d", conformance_window_flag);

    if(conformance_window_flag) {
        unsigned conf_win_left_offset = parseUE(&br);
        unsigned conf_win_right_offset = parseUE(&br);
        unsigned conf_win_top_offset = parseUE(&br);
        unsigned conf_win_bottom_offset = parseUE(&br);

        *width -= conf_win_left_offset + conf_win_right_offset* 2;
        *height -= conf_win_top_offset + conf_win_bottom_offset* 2;

        ALOGI("frame_crop = (%u, %u, %u, %u)",
              conf_win_left_offset, conf_win_right_offset,
              conf_win_top_offset, conf_win_bottom_offset);
    }

    unsigned bit_depth_luma_minus8 = parseUE(&br);
    unsigned bit_depth_chroma_minus8 = parseUE(&br);
    ALOGI("bit_depth_luma_minus8 =%u, bit_depth_chroma_minus8 =%u", bit_depth_luma_minus8, bit_depth_chroma_minus8);
    ALOGI("FindHEVCWH --");
}

}
