#include "Mms.h"
#include "DpBlitStream.h"
#include "DpAsyncBlitStream.h"
#include "DpDataType.h"
#include <ui/gralloc_extra.h>
#include <linux/mtk_ion.h>
#include <linux/ion.h>
#include <include/ion.h>
#include <sync/sync.h>

#ifdef MMS_SUPPORT_JPG_ENC
#include <enc/jpeg_hal.h>
#ifdef MTK_M4U_SUPPORT
#include <m4u_lib.h>
#else
#include "mt_iommu_port.h"
#endif
#include <sys/mman.h>
#endif

#include <linux/ion_drv.h>
#include <ion/ion.h>
#include <ion.h>
#include <cutils/log.h>
#include <system/graphics-base-v1.0.h>
#include <hardware/gralloc.h>
#include <memory>
#include <graphics_mtk_defs.h>


#define JPGENC_PORT_READ 1
#define JPGENC_PORT_WRITE 0

#define UNUSED(expr) do { (void)(expr); } while (0)

#undef LOG_TAG
#define LOG_TAG "MMServer"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace mms {
namespace V1_5 {
namespace implementation {

Mms::Mms() :
    mAsyncBlitStream(new DpAsyncBlitStream()) {
    mAsyncBlitStream->setUser(DP_BLIT_GPU);
    ALOGD("new mmservice\n");
}

Mms::~Mms() {
    delete mAsyncBlitStream;
}

DP_COLOR_ENUM mapColorFormat(int32_t copybit_colorformat)
{
    switch(copybit_colorformat)
    {
    case HAL_PIXEL_FORMAT_RGBA_8888:    return DP_COLOR_RGBA8888;
    case HAL_PIXEL_FORMAT_RGBX_8888:      return DP_COLOR_RGBA8888;
    case HAL_PIXEL_FORMAT_RGB_888:    return DP_COLOR_RGB888;
    case HAL_PIXEL_FORMAT_RGB_565:     return DP_COLOR_RGB565;
    case HAL_PIXEL_FORMAT_BGRA_8888:      return DP_COLOR_BGRA8888;
    case HAL_PIXEL_FORMAT_YCBCR_422_SP:       return DP_COLOR_NV16; //NV16
    case HAL_PIXEL_FORMAT_YCRCB_420_SP:        return DP_COLOR_NV21; //NV21
    case HAL_PIXEL_FORMAT_YCBCR_422_I:      return DP_COLOR_YUYV; //YUY2
    case HAL_PIXEL_FORMAT_RGBA_FP16:      return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_RAW16:      return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_BLOB:  return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:  return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_YCBCR_420_888: return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_RAW_OPAQUE:     return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_RAW10:       return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_RAW12: return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_RGBA_1010102: return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_Y8: return DP_COLOR_GREY;
    case HAL_PIXEL_FORMAT_Y16: return DP_COLOR_UNKNOWN;
    case HAL_PIXEL_FORMAT_YV12: return DP_COLOR_YV12;
    case HAL_PIXEL_FORMAT_I420: return DP_COLOR_I420;
    default:  return DP_COLOR_UNKNOWN;
    }
};

int32_t va_to_mva(int32_t ion_fd, pid_t pid, unsigned long va, uint32_t size, uint32_t *mva, int32_t *handleToBeFree)
{
    int32_t ion_user_handle = 0;
    int32_t ret = 0;
    ion_sys_data_t sys_data;
    struct ion_mm_data mm_data;
    struct ion_custom_data custom_data;
    unsigned long flags;

    flags = (pid << 4) | 3;

    ret = ion_alloc(ion_fd, size, va, ION_HEAP_MULTIMEDIA_MAP_MVA_MASK, flags, &ion_user_handle);
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

    *handleToBeFree = ion_user_handle;

    return 0;
}

// Methods from ::vendor::mediatek::hardware::mms::V1_0::IMms follow.
Return<bool> Mms::isCopybitSupportedOutFormat(int32_t format) {

    DpBlitStream  *pStream;
    int32_t default_value = 10;
    DP_COLOR_ENUM colorFormat;
    bool result;

    colorFormat = mapColorFormat(format);
    if (colorFormat == DP_COLOR_UNKNOWN)
    {
        ALOGD("copybit: Not support colorformat = %d\n", format);
        return bool {false};
    }
    pStream = new DpBlitStream();
    if (pStream == NULL)
    {
        ALOGE("copybit: new DpBlitStream fail\n");
        return int32_t {DP_STATUS_UNKNOWN_ERROR};
    }
    result = pStream->queryHWSupport(default_value, default_value, default_value, default_value, 0, colorFormat, colorFormat);
    ALOGD("copybit: Does the colorformat support? %d\n", result);

    delete pStream;

    return bool {result};
}

Return<int32_t> Mms::getCopybitDefaultOutFormat() {

    /*HAL_PIXEL_FORMAT_YCrCb_420_SP = 0x11 NV21*/
    ALOGD("copybit: Default OutFormat = %d\n", HAL_PIXEL_FORMAT_YCRCB_420_SP);
    return int32_t {HAL_PIXEL_FORMAT_YCRCB_420_SP};
}

Return<int32_t> Mms::getCopybitOutUsage(uint32_t usage) {

    /* buffer will be used by the 2D hardware blitter */
    /*GRALLOC_USAGE_HW_2D    = 0x00000400U*/
    UNUSED(usage);
    ALOGD("copybit:  gralloc usage = GRALLOC_USAGE_HW_2D\n");
    return int32_t {GRALLOC_USAGE_HW_2D};
}

Return<int32_t> Mms::copybit(const ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam& copybitparam) {

    DP_STATUS_ENUM status;
    uint32_t ret;
    int32_t i = 0;
    uint32_t          inputWidth = copybitparam.inputWidth;
    uint32_t          inputHeight = copybitparam.inputHeight;
    uint32_t          outputWidth = copybitparam.outputWidth;
    uint32_t          outputHeight = copybitparam.outputHeight;
    uint32_t          inputSize[3];
    uint32_t          outputSize[3];
    int32_t           ionInputFD;
    int32_t           ionOutputFD;
    DP_COLOR_ENUM     inputFormat = mapColorFormat(copybitparam.inputFormat);
    DP_COLOR_ENUM     outputFormat= mapColorFormat(copybitparam.outputFormat);
    uint32_t          inPlanes = DP_COLOR_GET_PLANE_COUNT(inputFormat);
    uint32_t          outPlanes = DP_COLOR_GET_PLANE_COUNT(outputFormat);

    ALOGD("cobybit: Start\n");

    if (inputFormat == DP_COLOR_UNKNOWN || outputFormat == DP_COLOR_UNKNOWN)
    {
        ALOGE("copybit: Not support inputFormat = %d or outputFormat = %d \n", inputFormat, outputFormat);
        return int32_t {DP_STATUS_INVALID_PARAX};
    }

    ALOGD("copybit: inputWidth = %d, inputHeight = %d, outputWidth = %d, outputHeight = %d\n", inputWidth, inputHeight, outputWidth, outputHeight);

    std::unique_ptr<DpBlitStream> pStream (new DpBlitStream());

    if (!pStream.get())
    {
        ALOGE("copybit: new DpBlitStream fail\n");
        return int32_t {DP_STATUS_UNKNOWN_ERROR};
    }

    /*input*/
    ret = gralloc_extra_query(copybitparam.inputHandle, GRALLOC_EXTRA_GET_ION_FD, &ionInputFD);
    if (ret < 0)
    {
        ALOGE("copybit: gralloc_extra_query failed\n");
    }

    if (!copybitparam.src_yPitch)
    {
        pStream->setSrcConfig(inputWidth, inputHeight, inputFormat);
        copybit_Bufsize(inputSize, inputFormat, inPlanes, inputWidth, inputHeight);
    }
    else
    {
        pStream->setSrcConfig(inputWidth, inputHeight, copybitparam.src_yPitch, copybitparam.src_uvPitch, inputFormat);
        for (i = 0; i < 3; i++)
        {
            inputSize[i] = copybitparam.src_sizeList[i];
        }
    }

    status = pStream->setSrcBuffer(ionInputFD, inputSize, inPlanes);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("copybit: setSrcBuffer fail with error = %d\n", status);
        return int32_t {status};
    }

    /*rotate & flip*/
    pStream->setRotate(copybitparam.rotation);
    pStream->setFlip(copybitparam.flipMirror);

    /*out*/
    ret = gralloc_extra_query(copybitparam.outputHandle, GRALLOC_EXTRA_GET_ION_FD, &ionOutputFD);
    if (ret < 0)
    {
        ALOGE("copybit: gralloc_extra_query failed\n");
    }

    if (!copybitparam.dst_yPitch)
    {
        pStream->setDstConfig(outputWidth, outputHeight, outputFormat);
        copybit_Bufsize(outputSize, outputFormat, outPlanes, outputWidth, outputHeight);
    }
    else
    {
        pStream->setDstConfig(outputWidth, outputHeight, copybitparam.dst_yPitch, copybitparam.dst_uvPitch, outputFormat);
        for (i = 0; i < 3; i++)
        {
            outputSize[i] = copybitparam.dst_sizeList[i];
        }
    }

    pStream->setDstBuffer(ionOutputFD, outputSize, outPlanes);

    /*invalidate*/
    ALOGD("cobybit: Invalidate start\n");
    status = pStream->invalidate();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("copybit: Invalidate fail with error = %d\n", status);
        return int32_t {status};
    }

    ALOGD("cobybit: End\n");

    return int32_t {status};
}

Return<void> Mms::jpegencode(const ::vendor::mediatek::hardware::mms::V1_0::HwJpegEncoderConfig& cfg, jpegencode_cb _hidl_cb) {
#ifdef MMS_SUPPORT_JPG_ENC
    ALOGD("Mms: JpgENC: Start!!!\n");		//UT Debug Log
	JUINT32 width = cfg.width;
	JUINT32 height = cfg.height;
	JUINT32 quality = cfg.level;
	JpgEncHal::EncFormat encFormat = JpgEncHal::kENC_NV12_Format;
	JUINT32 stride = 0;

	ALOGD("Mms: JpgENC: width: %d, height: %d, quality: %d\n", width, height, quality);		//UT Debug Log

	if (quality > 100)
	{
		ALOGE("Mms: JpgENC: level NOT support!!!\n");
		_hidl_cb(0, 0);		//no encode
		return Void();
	}

	ALOGD("Mms: JpgENC: format: 0x%x\n", cfg.format);		//UT Debug Log
	//MTK: support 1 plane(YUY2), 2 plane(NV21,NV12)
	switch (cfg.format)
	{
		case HAL_PIXEL_FORMAT_YCbCr_422_I:
			encFormat = JpgEncHal::kENC_YUY2_Format;
			ALOGD("Mms: JpgENC: format: HAL_PIXEL_FORMAT_YCbCr_422_I\n");		//UT Debug Log
			break;
		case HAL_PIXEL_FORMAT_YCbCr_422_SP:
			encFormat = JpgEncHal::kENC_NV12_Format;
			break;
		case HAL_PIXEL_FORMAT_YCrCb_420_SP:
			encFormat = JpgEncHal::kENC_NV21_Format;
			break;
		case HAL_PIXEL_FORMAT_YV12:
			encFormat = JpgEncHal::kENC_YV12_Format;
			//break;
		default:
			ALOGE("Mms: JpgENC: Format NOT support!!!\n");
			_hidl_cb(0, 0);		//no encode
			return Void();
	}
	//HAL_PIXEL_FORMAT_YCbCr_422_I	/* 422 format, 1 plane (YUYV) */
	//HAL_PIXEL_FORMAT_YCrCb_420_SP	/* 420 format, 2 plane (Y),(VU) */
	//HAL_PIXEL_FORMAT_YCbCr_422_SP	/* 422 format, 2 plane (Y),(UV) */
	//HAL_PIXEL_FORMAT_YV12			/* 420 format, 3 plane (Y),(V),(U) */

	if (NULL == cfg.input)
	{
		ALOGE("Mms: JpgENC: Input buffer is EMPTY!\n");
		_hidl_cb(0, 0);		//no encode
		return Void();
	}
	else if (NULL == cfg.output)
	{
		ALOGE("Mms: JpgENC: Output buffer is EMPTY!\n");
		_hidl_cb(0, 0);		//no encode
		return Void();
	}

	JINT32 ionInputFD = -1, ionInputFD2 = -1;
    JINT32 ionOutputFD = -1;

	JUINT32 srcBufferSize = 0;
	JUINT32 srcChromaBufferSize = 0;

	JUINT32 dstBuffersize = 0;

	JINT32 ion_client;
	ion_client = mt_ion_open("MmsJpgEnc");
	ALOGD("Mms: JpgENC: ion_client: %d\n", ion_client);		//UT Debug Log

        if (ion_client<0)
        {
            ALOGE("Mms: JpgENC: invalid ion handle!\n");
	    _hidl_cb(0, 0);		//no encode
	    return Void();
        }

	ion_user_handle_t src_hnd = -1;
	gralloc_extra_query(cfg.input, GRALLOC_EXTRA_GET_ION_FD, &ionInputFD);
	ALOGD("Mms: JpgENC: Get ionInputFD: %d\n", ionInputFD);		//UT Debug Log

	if (ion_import(ion_client, ionInputFD, &src_hnd))
	{
		ALOGE("Mms: JpgENC: ion_import FAILED!, client(%d), FD(%d)\n", ion_client, ionInputFD);
		src_hnd = -1;
	}
	ALOGD("Mms: JpgENC[ion_import]: ion_client: %d, ionInputFD: %d, src_hnd: %p\n", ion_client, ionInputFD, src_hnd);		//UT Debug Log

	JUINT32 srcPAddr = NULL;
	JUINT32 srcChromaPAddr = NULL;
	if (!mmsJpeg_query_buffer(ion_client, src_hnd, &srcPAddr, &srcBufferSize, JPGENC_PORT_READ))
	{
		if (src_hnd != -1 && ion_free(ion_client, src_hnd))
		{
			ALOGE("Mms: JpgENC: ion free: client(%d), src handle(%p)\n", ion_client, src_hnd);
			src_hnd = -1;
		}

		close(ion_client);

		ALOGD("Mms: JpgENC: End (query src PA failed)!!!\n");
		_hidl_cb(0, 0);		//no encode
		return Void();
	}

	ion_user_handle_t dst_hnd = -1;
	gralloc_extra_query(cfg.output, GRALLOC_EXTRA_GET_ION_FD, &ionOutputFD);
	ALOGD("Mms: JpgENC: Get ionOutputFD: %d\n", ionOutputFD);		//UT Debug Log

	if (ion_import(ion_client, ionOutputFD, &dst_hnd))
	{
		ALOGE("Mms: JpgENC: ion_import FAILED!, client(%d), FD(%d)\n", ion_client, ionOutputFD);
		dst_hnd = -1;
	}
	ALOGD("Mms: JpgENC[ion_import]: ion_client: %d, ionOutputFD: %d, dst_hnd: %p\n", ion_client, ionOutputFD, dst_hnd);		//UT Debug Log

	JUINT32 dstPAddr = NULL;
	if (!mmsJpeg_query_buffer(ion_client, dst_hnd, &dstPAddr, &dstBuffersize, JPGENC_PORT_WRITE))
	{
		if (dst_hnd != -1 && ion_free(ion_client, dst_hnd))
		{
			ALOGE("Mms: JpgENC: ion free: client(%d), dst handle(%p)\n", ion_client, dst_hnd);
			dst_hnd = -1;
		}

		if (src_hnd != -1 && ion_free(ion_client, src_hnd))
		{
			ALOGE("Mms: JpgENC: ion free: client(%d), src handle(%p)\n", ion_client, src_hnd);
			src_hnd = -1;
		}

		close(ion_client);

		ALOGD("Mms: JpgENC: End (query dst PA failed)!!!\n");
		_hidl_cb(0, 0);		//no encode
		return Void();
	}

	if (encFormat == JpgEncHal::kENC_YUY2_Format)
	{/* 1 plane */
		srcBufferSize = width * height * 2;
		srcChromaBufferSize = 0;
		stride = width * 2;
		ionInputFD2 = -1;
	}
	else
	{/* 2 planes */
		srcBufferSize = width * height;
		srcChromaBufferSize = (width / 2) * (height / 2) * 2;
		stride = width;
		srcChromaPAddr = srcPAddr + srcBufferSize;
		ionInputFD2 = ionInputFD;
	}

	JUINT32 encSize = 0;

	JINT32 shared_InputFD = -1, shared_InputFD2 = -1;
	JINT32 shared_OutputFD = -1;
	void* srcVAddr = NULL;
	void* srcChromaVAddr = NULL;
	void* dstVAddr = NULL;

	JpgEncHal* mmsJpgEncoder = new JpgEncHal();

	if(!mmsJpgEncoder->LevelLock(JpgEncHal::JPEG_ENC_LOCK_HW_FIRST))
    {
        ALOGE("Mms: JpgENC: can't lock jpeg encode resource\n");
        delete mmsJpgEncoder;
		_hidl_cb(0, 0);		//no encode
        goto QuitCloseIon;
    }

	ALOGD("Mms: JpgENC[setEncSize]: width: %d, height: %d, format: 0x%x\n", width, height, encFormat);		//UT Debug Log
	mmsJpgEncoder->setEncSize(width, height, encFormat);

	ALOGD("Mms: JpgENC[setQuality]: quality: %d\n", quality);		//UT Debug Log
	mmsJpgEncoder->setQuality(quality);

	//mmsJpgEncoder->enableSOI();

	//ION
	ALOGD("Mms: JpgENC[setIonMode]: ion mode(1)\n");		//UT Debug Log
	mmsJpgEncoder->setIonMode(1);

	ALOGD("Mms: JpgENC[setSrcPAddr]: srcPAddr: 0x%x, srcChromaPAddr: 0x%x\n", srcPAddr, srcChromaPAddr);		//UT Debug Log
	mmsJpgEncoder->setSrcPAddr((void *)(unsigned long)srcPAddr, (void *)(unsigned long)srcChromaPAddr);
	ALOGD("Mms: JpgENC[setSrcBufSize]: stride: %d, srcBufferSize: %d, srcChromaBufferSize: %d\n", stride, srcBufferSize, srcChromaBufferSize);		//UT Debug Log
	mmsJpgEncoder->setSrcBufSize(stride, srcBufferSize, srcChromaBufferSize);

	//Src VA
	if (src_hnd != -1 && ion_share(ion_client, src_hnd, &shared_InputFD))
	{
		ALOGE("Mms: JpgENC: ion shared FAILED!, src handle(%p)\n", src_hnd);
		shared_InputFD = -1;

		_hidl_cb(0, 0);
		goto QuitCloseEncoder;
	}
	shared_InputFD2 = shared_InputFD;

	if (!mmsJpeg_getVA(shared_InputFD, (srcBufferSize+srcChromaBufferSize), &srcVAddr))
	{
		_hidl_cb(0, 0);
		goto QuitCloseEncoder;
	}
	srcChromaVAddr = (void *)((unsigned long)srcVAddr + srcBufferSize);

	ALOGD("Mms: JpgENC[setSrcAddr]: srcVAddr: %p, srcChromaVAddr: %p\n", srcVAddr, srcChromaVAddr);		//UT Debug Log
	mmsJpgEncoder->setSrcAddr(srcVAddr, srcChromaVAddr);

	/*
	if (encFormat != JpgEncHal::kENC_YV12_Format)
	{
		mmsJpgEncoder->setSrcAddr(,);
		mmsJpgEncoder->setSrcBufSize(,,);
	}
	else
	{
		mmsJpgEncoder->setSrcAddr(,,);
		mmsJpgEncoder->setSrcBufSize(,,,,);
	}
	*/

	//ION
	ALOGD("Mms: JpgENC[setSrcFD]: SrcFD: %d, SrcFD2: %d\n", shared_InputFD, shared_InputFD2);		//UT Debug Log
	mmsJpgEncoder->setSrcFD(shared_InputFD, shared_InputFD2);

	ALOGD("Mms: JpgENC[setDstPAddr]: dstPAddr: 0x%x\n", dstPAddr);		//UT Debug Log
	mmsJpgEncoder->setDstPAddr((void *)(unsigned long)dstPAddr);
	ALOGD("Mms: JpgENC[setDstSize]: dstBuffersize: %d\n", dstBuffersize);		//UT Debug Log
    mmsJpgEncoder->setDstSize(dstBuffersize);

	//Dst VA
	if (dst_hnd != -1 && ion_share(ion_client, dst_hnd, &shared_OutputFD))
	{
		ALOGE("Mms: JpgENC: ion shared FAILED!, dst handle(%p)\n", dst_hnd);
		shared_OutputFD = -1;

		_hidl_cb(0, 0);
		goto QuitCloseEncoder;
	}

	if(!mmsJpeg_getVA(shared_OutputFD, dstBuffersize, &dstVAddr))
	{
		_hidl_cb(0, 0);
		goto QuitCloseEncoder;
	}

	ALOGD("Mms: JpgENC[setDstAddr]: dstVAddr: %p\n", dstVAddr);		//UT Debug Log
	mmsJpgEncoder->setDstAddr(dstVAddr);

	//ION
	ALOGD("Mms: JpgENC[setDstFD]: DstFD: %d\n", shared_OutputFD);		//UT Debug Log
	mmsJpgEncoder->setDstFD(shared_OutputFD);

    if(!mmsJpgEncoder->start(&encSize))
    {
        ALOGE("Mms: JpgENC: trigger Encoder FAILED!!\n");
		_hidl_cb(0, 0);		//no encode
        goto QuitCloseEncoder;
    }

	ALOGD("Mms: JpgENC: Enc Done, Size = %d\n", encSize);
	_hidl_cb(encSize, 1);		//encode done

QuitCloseEncoder:
    mmsJpgEncoder->unlock();
	ALOGD("Mms: JpgENC: unlock done!\n");		//UT Debug Log

    delete mmsJpgEncoder;
    mmsJpgEncoder = NULL;

QuitCloseIon:
	mmsJpeg_freeVA((srcBufferSize+srcChromaBufferSize), &srcVAddr);
	mmsJpeg_freeVA(dstBuffersize, &dstVAddr);

	if (dst_hnd != -1 && ion_free(ion_client, dst_hnd))
	{
		ALOGE("Mms: JpgENC: ion free: client(%d), dst handle(%p)\n", ion_client, dst_hnd);
		dst_hnd = -1;
	}
	if (shared_OutputFD != -1 && ion_share_close(ion_client, shared_OutputFD))
	{
		ALOGE("Mms: JpgENC: ion share close: client(%d), shared FD(%d)\n", ion_client, shared_OutputFD);
		shared_OutputFD = -1;
	}

	if (src_hnd != -1 && ion_free(ion_client, src_hnd))
	{
		ALOGE("Mms: JpgENC: ion free: client(%d), src handle(%p)\n", ion_client, src_hnd);
		src_hnd = -1;
	}
	if (shared_InputFD != -1 && ion_share_close(ion_client, shared_InputFD))
	{
		ALOGE("Mms: JpgENC: ion share close: client(%d), shared FD(%d)\n", ion_client, shared_InputFD);
		shared_InputFD = -1;
	}

	close(ion_client);
	ALOGD("Mms: JpgENC: free ION client!\n");

	ALOGD("Mms: JpgENC: End!!!\n");		//UT Debug Log
#endif
	return Void();
}

Return<int32_t> Mms::jpegOpenClient(uint32_t width, uint32_t height) {
#ifdef MMS_SUPPORT_JPG_ENC
    ALOGD("Mms: JpgENC: jpegOpenClient!!!\n");
#endif
    return int32_t {};
}

Return<int32_t> Mms::jpegCloseClient() {
#ifdef MMS_SUPPORT_JPG_ENC
    ALOGD("Mms: JpgENC: jpegCloseClient!!!\n");
#endif
    return int32_t {};
}
// Methods from ::vendor::mediatek::hardware::mms::V1_1::IMms follow.

Return<int32_t> Mms::mdp_run(const ::vendor::mediatek::hardware::mms::V1_1::HwMDPParam& mdpparam) {

    DP_STATUS_ENUM status;
    int32_t i;
    int32_t ret = 0;

    std::unique_ptr<DpBlitStream> pStream (new DpBlitStream());

    if (!pStream.get())
    {
        ALOGE("Mms: new DpBlitStream fail\n");
        return int32_t {DP_STATUS_UNKNOWN_ERROR};
    }

    int32_t    s_IONHandle = -1;
    int32_t    src_ion_handle_tobe_free[mdpparam.src_planeNumber];
    int32_t    dst_ion_handle_tobe_free[mdpparam.dst_planeNumber];

    if (mdpparam.pid)
    {
        /* va -> mva */
        s_IONHandle = mt_ion_open("MmsIonHandler");
        if (s_IONHandle < 0) {
            ALOGE("Mms: ion_open(%d) fail\n", s_IONHandle);
            return s_IONHandle;
        }
    }

    uint64_t src_pVAddr[3];
    uint32_t src_pMVAddr[3];
    uint64_t dst_pVAddr[3];
    uint32_t dst_pMVAddr[3];
    uint32_t src_pSizeList[3];
    uint32_t dst_pSizeList[3];

    /* set src crop*/
    ::DpRect       SrcRect;
    SrcRect.x = mdpparam.src_rect.x;
    SrcRect.y = mdpparam.src_rect.y;
    SrcRect.w = mdpparam.src_rect.w;
    SrcRect.h = mdpparam.src_rect.h;

    /* set dst roi*/
    ::DpRect       DstRect;
    DstRect.x = mdpparam.dst_rect.x;
    DstRect.y = mdpparam.dst_rect.y;
    DstRect.w = mdpparam.dst_rect.w;
    DstRect.h = mdpparam.dst_rect.h;

    DP_COLOR_ENUM     inputFormat = (DP_COLOR_ENUM)mdpparam.src_format;
    DP_COLOR_ENUM     outputFormat = (DP_COLOR_ENUM)mdpparam.dst_format;

    /* set src & dst sizeList*/
    for (i = 0; i < 3; i++)
    {
        src_pSizeList[i] = mdpparam.src_sizeList[i];
        dst_pSizeList[i] = mdpparam.dst_sizeList[i];
    }

    /* src config*/
    if (!mdpparam.src_yPitch)
    {
        pStream->setSrcConfig(mdpparam.src_width, mdpparam.src_height, inputFormat, eInterlace_None, &SrcRect);
    }
    else
    {
        pStream->setSrcConfig(mdpparam.src_width, mdpparam.src_height, mdpparam.src_yPitch, mdpparam.src_uvPitch, inputFormat, DP_PROFILE_BT601, eInterlace_None, &SrcRect);
    }

    /* src buffer*/
    if (mdpparam.pid)
    {
        /* va + pid -> mva */
        for (i =0 ; i < mdpparam.src_planeNumber; i++)
        {
            uint32_t  mva = 0 ;
            src_ion_handle_tobe_free[i] = 0;
            src_pVAddr[i] = mdpparam.src_VAList[i];
            unsigned long va = (unsigned long)(src_pVAddr[i]);
            ret = va_to_mva(s_IONHandle, (pid_t)mdpparam.pid, va, src_pSizeList[i], &mva ,&(src_ion_handle_tobe_free[i]));
            if (ret < 0)
            {
                ALOGE("Mms: transform src va to mva failed (%d)\n", ret);
            }
            src_pMVAddr[i] =  mva;
        }
    }
    else
    {
        /* mva */
        for (i = 0; i < mdpparam.src_planeNumber; i++)
        {
            src_pMVAddr[i] = mdpparam.src_MVAList[i];
        }
    }

    status = pStream->setSrcBuffer(NULL, (void**)&src_pMVAddr, src_pSizeList, mdpparam.src_planeNumber);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("Mms: setSrcBuffer fail with error = %d\n", status);
        return int32_t {status};
    }

    /* dst config*/
    if (!mdpparam.dst_yPitch)
    {
        pStream->setDstConfig(mdpparam.dst_width, mdpparam.dst_height, outputFormat, eInterlace_None, &DstRect);
    }
    else
    {
        pStream->setDstConfig(mdpparam.dst_width, mdpparam.dst_height, mdpparam.dst_yPitch, mdpparam.dst_uvPitch, outputFormat, DP_PROFILE_BT601, eInterlace_None, &DstRect);
    }

    /* dst buffer*/
    if (mdpparam.pid)
    {
        /* va + pid -> mva */
        for (i =0 ; i < mdpparam.dst_planeNumber; i++)
        {
            uint32_t  mva = 0 ;
            dst_ion_handle_tobe_free[i] = 0;
            dst_pVAddr[i] = mdpparam.dst_VAList[i];
            unsigned long va = (unsigned long)(dst_pVAddr[i]);
            ret = va_to_mva(s_IONHandle, (pid_t)mdpparam.pid, va, dst_pSizeList[i], &mva ,&(dst_ion_handle_tobe_free[i]));
            if (ret < 0)
            {
                ALOGE("Mms: transform dst va to mva failed (%d)\n", ret);
            }
            dst_pMVAddr[i] =  mva;
        }
    }
    else
    {
        /* mva */
        for (i = 0; i < mdpparam.dst_planeNumber; i++)
        {
            dst_pMVAddr[i] = mdpparam.dst_MVAList[i];
        }
    }

    pStream->setDstBuffer(NULL, (void**)&dst_pMVAddr, dst_pSizeList, mdpparam.dst_planeNumber);

    /* roation*/
    pStream->setRotate(mdpparam.rotation);

    /* flip*/
    pStream->setFlip(mdpparam.flip);

    status = pStream->invalidate();

    if (mdpparam.pid)
    {
        //free mva handle and close ion_fd
        for(i =0 ; i < mdpparam.src_planeNumber; i++)
        {
            if (ion_free(s_IONHandle, src_ion_handle_tobe_free[i]))
            {
                ALOGE("Mms: free src memory failed\n");
            }
        }

        for(i =0 ; i < mdpparam.dst_planeNumber; i++)
        {
            if (ion_free(s_IONHandle, dst_ion_handle_tobe_free[i]))
            {
                ALOGE("Mms: free dst memory failed\n");
            }
        }
        ion_close(s_IONHandle);
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("Mms: Invalidate fail with error = %d\n", status);
    }
    return int32_t {status};
}

// Methods from ::vendor::mediatek::hardware::mms::V1_2::IMms follow.
Return<int32_t> Mms::BlitStream(const ::vendor::mediatek::hardware::mms::V1_2::MDPParam& mdpparam) {

    DP_STATUS_ENUM status;
    int32_t i;
    int32_t ret = 0;

    std::unique_ptr<DpBlitStream> pStream (new DpBlitStream());

    if (!pStream.get())
    {
        ALOGE("Mms: new DpBlitStream fail\n");
        return int32_t {DP_STATUS_UNKNOWN_ERROR};
    }

    int32_t    s_IONHandle = -1;
    int32_t    src_ion_handle_tobe_free[mdpparam.src_planeNumber];
    int32_t    dst_ion_handle_tobe_free[mdpparam.dst_planeNumber];

    if (mdpparam.pid)
    {
        /* va -> mva */
        s_IONHandle = mt_ion_open("MmsIonHandler");
        if (s_IONHandle < 0) {
            ALOGE("Mms: ion_open(%d) fail\n", s_IONHandle);
            return s_IONHandle;
        }
    }

    uint64_t src_pVAddr[3];
    uint32_t src_pMVAddr[3];
    uint64_t dst_pVAddr[3];
    uint32_t dst_pMVAddr[3];
    uint32_t src_pSizeList[3];
    uint32_t dst_pSizeList[3];

    /* set src crop*/
    ::DpRect       SrcRect;
    SrcRect.x = mdpparam.src_rect.x;
    SrcRect.y = mdpparam.src_rect.y;
    SrcRect.w = mdpparam.src_rect.w;
    SrcRect.h = mdpparam.src_rect.h;

    /* set dst roi*/
    ::DpRect       DstRect;
    DstRect.x = mdpparam.dst_rect.x;
    DstRect.y = mdpparam.dst_rect.y;
    DstRect.w = mdpparam.dst_rect.w;
    DstRect.h = mdpparam.dst_rect.h;

    DP_COLOR_ENUM     inputFormat = (DP_COLOR_ENUM)mdpparam.src_format;
    DP_COLOR_ENUM     outputFormat = (DP_COLOR_ENUM)mdpparam.dst_format;

    DP_PROFILE_ENUM src_prof = (DP_PROFILE_ENUM)static_cast<int32_t>(mdpparam.src_profile);
    DP_PROFILE_ENUM dst_prof = (DP_PROFILE_ENUM)static_cast<int32_t>(mdpparam.dst_profile);

    /* set src & dst sizeList*/
    for (i = 0; i < 3; i++)
    {
        src_pSizeList[i] = mdpparam.src_sizeList[i];
        dst_pSizeList[i] = mdpparam.dst_sizeList[i];
    }

    /* src config*/
    if (!mdpparam.src_yPitch)
    {
        pStream->setSrcConfig(mdpparam.src_width, mdpparam.src_height, inputFormat, eInterlace_None, &SrcRect);
    }
    else
    {
        pStream->setSrcConfig(mdpparam.src_width, mdpparam.src_height, mdpparam.src_yPitch, mdpparam.src_uvPitch, inputFormat, src_prof, eInterlace_None, &SrcRect);
    }

    /* src buffer*/
    if (mdpparam.pid)
    {
        /* va + pid -> mva */
        for (i =0 ; i < mdpparam.src_planeNumber; i++)
        {
            uint32_t  mva = 0 ;
            src_ion_handle_tobe_free[i] = 0;
            src_pVAddr[i] = mdpparam.src_VAList[i];
            unsigned long va = (unsigned long)(src_pVAddr[i]);
            ret = va_to_mva(s_IONHandle, (pid_t)mdpparam.pid, va, src_pSizeList[i], &mva ,&(src_ion_handle_tobe_free[i]));
            if (ret < 0)
            {
                ALOGE("Mms: transform src va to mva failed (%d)\n", ret);
            }
            src_pMVAddr[i] =  mva;
        }
    }
    else
    {
        /* mva */
        for (i = 0; i < mdpparam.src_planeNumber; i++)
        {
            src_pMVAddr[i] = mdpparam.src_MVAList[i];
        }
    }

    status = pStream->setSrcBuffer(NULL, (void**)&src_pMVAddr, src_pSizeList, mdpparam.src_planeNumber);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("Mms: setSrcBuffer fail with error = %d\n", status);
        return int32_t {status};
    }

    /* dst config*/
    if (!mdpparam.dst_yPitch)
    {
        pStream->setDstConfig(mdpparam.dst_width, mdpparam.dst_height, outputFormat, eInterlace_None, &DstRect);
    }
    else
    {
        pStream->setDstConfig(mdpparam.dst_width, mdpparam.dst_height, mdpparam.dst_yPitch, mdpparam.dst_uvPitch, outputFormat, dst_prof, eInterlace_None, &DstRect);
    }

    /* dst buffer*/
    if (mdpparam.pid)
    {
        /* va + pid -> mva */
        for (i =0 ; i < mdpparam.dst_planeNumber; i++)
        {
            uint32_t  mva = 0 ;
            dst_ion_handle_tobe_free[i] = 0;
            dst_pVAddr[i] = mdpparam.dst_VAList[i];
            unsigned long va = (unsigned long)(dst_pVAddr[i]);
            ret = va_to_mva(s_IONHandle, (pid_t)mdpparam.pid, va, dst_pSizeList[i], &mva ,&(dst_ion_handle_tobe_free[i]));
            if (ret < 0)
            {
                ALOGE("Mms: transform dst va to mva failed (%d)\n", ret);
            }
            dst_pMVAddr[i] =  mva;
        }
    }
    else
    {
        /* mva */
        for (i = 0; i < mdpparam.dst_planeNumber; i++)
        {
            dst_pMVAddr[i] = mdpparam.dst_MVAList[i];
        }
    }

    pStream->setDstBuffer(NULL, (void**)&dst_pMVAddr, dst_pSizeList, mdpparam.dst_planeNumber);

    /* roation*/
    pStream->setRotate(mdpparam.rotation);

    /* flip*/
    pStream->setFlip(mdpparam.flip);

    /* pq */
    if (mdpparam.pq_param.enable)
    {
        ::DpPqParam    pqparam;
        pqparam.enable = mdpparam.pq_param.enable;
        pqparam.scenario = (DP_MEDIA_TYPE_ENUM)static_cast<int32_t>(mdpparam.pq_param.scenario);;
        if (pqparam.scenario == MEDIA_PICTURE)
            pqparam.u.image.iso = mdpparam.pq_param.iso;
        else
            ALOGE("Mms: unexpected pq scenario = %d\n", mdpparam.pq_param.scenario);
        pStream->setPQParameter(pqparam);
    }

    status = pStream->invalidate();

    if (mdpparam.pid)
    {
        //free mva handle and close ion_fd
        for(i =0 ; i < mdpparam.src_planeNumber; i++)
        {
            if (ion_free(s_IONHandle, src_ion_handle_tobe_free[i]))
            {
                ALOGE("Mms: free src memory failed\n");
            }
        }

        for(i =0 ; i < mdpparam.dst_planeNumber; i++)
        {
            if (ion_free(s_IONHandle, dst_ion_handle_tobe_free[i]))
            {
                ALOGE("Mms: free dst memory failed\n");
            }
        }
        ion_close(s_IONHandle);
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("Mms: Invalidate fail with error = %d\n", status);
    }
    return int32_t {status};
}

// Methods from ::vendor::mediatek::hardware::mms::V1_3::IMms follow.
Return<int32_t> Mms::BlitStreamFD(const ::vendor::mediatek::hardware::mms::V1_3::MDPParamFD& mdpparamfd) {

    DP_STATUS_ENUM status;
    int32_t i;
    int32_t ret = 0;

    std::unique_ptr<DpBlitStream> pStream (new DpBlitStream());

    if (!pStream.get())
    {
        ALOGE("Mms: new DpBlitStream fail\n");
        return int32_t {DP_STATUS_UNKNOWN_ERROR};
    }

    int32_t    s_IONHandle = -1;
    int32_t    src_ion_handle_tobe_free[mdpparamfd.src_planeNumber];
    int32_t    dst_ion_handle_tobe_free[mdpparamfd.dst_planeNumber];

    if (mdpparamfd.pid && !(mdpparamfd.inputHandle && mdpparamfd.outputHandle))
    {
        /* va -> mva */
        s_IONHandle = mt_ion_open("MmsIonHandler");
        if (s_IONHandle < 0) {
            ALOGE("Mms: ion_open(%d) fail\n", s_IONHandle);
            return s_IONHandle;
        }
    }

    uint64_t src_pVAddr[3];
    uint32_t src_pMVAddr[3];
    uint64_t dst_pVAddr[3];
    uint32_t dst_pMVAddr[3];
    uint32_t src_pSizeList[3];
    uint32_t dst_pSizeList[3];

    /* set src crop*/
    ::DpRect       SrcRect;
    SrcRect.x = mdpparamfd.src_rect.x;
    SrcRect.y = mdpparamfd.src_rect.y;
    SrcRect.w = mdpparamfd.src_rect.w;
    SrcRect.h = mdpparamfd.src_rect.h;

    /* set dst roi*/
    ::DpRect       DstRect;
    DstRect.x = mdpparamfd.dst_rect.x;
    DstRect.y = mdpparamfd.dst_rect.y;
    DstRect.w = mdpparamfd.dst_rect.w;
    DstRect.h = mdpparamfd.dst_rect.h;

    DP_COLOR_ENUM     inputFormat = (DP_COLOR_ENUM)mdpparamfd.src_format;
    DP_COLOR_ENUM     outputFormat = (DP_COLOR_ENUM)mdpparamfd.dst_format;

    DP_PROFILE_ENUM src_prof = (DP_PROFILE_ENUM)static_cast<int32_t>(mdpparamfd.src_profile);
    DP_PROFILE_ENUM dst_prof = (DP_PROFILE_ENUM)static_cast<int32_t>(mdpparamfd.dst_profile);

    /* set src & dst sizeList*/
    for (i = 0; i < 3; i++)
    {
        src_pSizeList[i] = mdpparamfd.src_sizeList[i];
        dst_pSizeList[i] = mdpparamfd.dst_sizeList[i];
    }

    /* src config*/
    if (!mdpparamfd.src_yPitch)
    {
        pStream->setSrcConfig(mdpparamfd.src_width, mdpparamfd.src_height, inputFormat, eInterlace_None, &SrcRect);
    }
    else
    {
        pStream->setSrcConfig(mdpparamfd.src_width, mdpparamfd.src_height, mdpparamfd.src_yPitch, mdpparamfd.src_uvPitch, inputFormat, src_prof, eInterlace_None, &SrcRect);
    }

    /* src buffer*/
    if (!mdpparamfd.inputHandle)
    {
        if (mdpparamfd.pid)
        {
            /* va + pid -> mva */
            for (i =0 ; i < mdpparamfd.src_planeNumber; i++)
            {
                uint32_t  mva = 0 ;
                src_ion_handle_tobe_free[i] = 0;
                src_pVAddr[i] = mdpparamfd.src_VAList[i];
                unsigned long va = (unsigned long)(src_pVAddr[i]);
                ret = va_to_mva(s_IONHandle, (pid_t)mdpparamfd.pid, va, src_pSizeList[i], &mva ,&(src_ion_handle_tobe_free[i]));
                if (ret < 0)
                {
                    ALOGE("Mms: transform src va to mva failed (%d)\n", ret);
                }
                src_pMVAddr[i] =  mva;
            }
        }
        else
        {
            /* mva */
            for (i = 0; i < mdpparamfd.src_planeNumber; i++)
            {
                src_pMVAddr[i] = mdpparamfd.src_MVAList[i];
            }
        }

        status = pStream->setSrcBuffer(NULL, (void**)&src_pMVAddr, src_pSizeList, mdpparamfd.src_planeNumber);
    }
    else
    {
        /* FD */
        status = pStream->setSrcBuffer((mdpparamfd.inputHandle)->data[0], src_pSizeList, mdpparamfd.src_planeNumber);
    }
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("Mms: setSrcBuffer fail with error = %d\n", status);
        return int32_t {status};
    }

    /* dst config*/
    if (!mdpparamfd.dst_yPitch)
    {
        pStream->setDstConfig(mdpparamfd.dst_width, mdpparamfd.dst_height, outputFormat, eInterlace_None, &DstRect);
    }
    else
    {
        pStream->setDstConfig(mdpparamfd.dst_width, mdpparamfd.dst_height, mdpparamfd.dst_yPitch, mdpparamfd.dst_uvPitch, outputFormat, dst_prof, eInterlace_None, &DstRect);
    }

    /* dst buffer*/
    if (!mdpparamfd.outputHandle)
    {
        if (mdpparamfd.pid)
        {
            /* va + pid -> mva */
            for (i =0 ; i < mdpparamfd.dst_planeNumber; i++)
            {
                uint32_t  mva = 0 ;
                dst_ion_handle_tobe_free[i] = 0;
                dst_pVAddr[i] = mdpparamfd.dst_VAList[i];
                unsigned long va = (unsigned long)(dst_pVAddr[i]);
                ret = va_to_mva(s_IONHandle, (pid_t)mdpparamfd.pid, va, dst_pSizeList[i], &mva ,&(dst_ion_handle_tobe_free[i]));
                if (ret < 0)
                {
                    ALOGE("Mms: transform dst va to mva failed (%d)\n", ret);
                }
                dst_pMVAddr[i] =  mva;
            }
        }
        else
        {
            /* mva */
            for (i = 0; i < mdpparamfd.dst_planeNumber; i++)
            {
                dst_pMVAddr[i] = mdpparamfd.dst_MVAList[i];
            }
        }

        pStream->setDstBuffer(NULL, (void**)&dst_pMVAddr, dst_pSizeList, mdpparamfd.dst_planeNumber);
    }
    else
    {
        /* FD */
        pStream->setDstBuffer((mdpparamfd.outputHandle)->data[0], dst_pSizeList, mdpparamfd.dst_planeNumber);
    }

    /* roation*/
    pStream->setRotate(mdpparamfd.rotation);

    /* flip*/
    pStream->setFlip(mdpparamfd.flip);

    /* pq */
    if (mdpparamfd.pq_param.enable)
    {
        ::DpPqParam    pqparam;
        pqparam.enable = mdpparamfd.pq_param.enable;
        pqparam.scenario = (DP_MEDIA_TYPE_ENUM)static_cast<int32_t>(mdpparamfd.pq_param.scenario);;
        if (pqparam.scenario == MEDIA_PICTURE)
            pqparam.u.image.iso = mdpparamfd.pq_param.iso;
        else
            ALOGE("Mms: unexpected pq scenario = %d\n", mdpparamfd.pq_param.scenario);
        pStream->setPQParameter(pqparam);
    }

    status = pStream->invalidate();

    if (mdpparamfd.pid)
    {
        //free mva handle and close ion_fd
        if (!mdpparamfd.inputHandle)
        {
            for(i =0 ; i < mdpparamfd.src_planeNumber; i++)
            {
                if (ion_free(s_IONHandle, src_ion_handle_tobe_free[i]))
                {
                    ALOGE("Mms: free src memory failed\n");
                }
            }
        }

        if (!mdpparamfd.outputHandle)
        {
            for(i =0 ; i < mdpparamfd.dst_planeNumber; i++)
            {
                if (ion_free(s_IONHandle, dst_ion_handle_tobe_free[i]))
                {
                    ALOGE("Mms: free dst memory failed\n");
                }
            }
        }

        if (!(mdpparamfd.inputHandle && mdpparamfd.outputHandle))
        {
            ion_close(s_IONHandle);
        }
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("Mms: Invalidate fail with error = %d\n", status);
    }
    return int32_t {status};
}

Return<int32_t> Mms::copybitprofile(const ::vendor::mediatek::hardware::mms::V1_4::HwCopybitParamProfile& copybitparamprofile) {

    DP_STATUS_ENUM status;
    uint32_t ret;
    int32_t i = 0;
    uint32_t          inputWidth = copybitparamprofile.inputWidth;
    uint32_t          inputHeight = copybitparamprofile.inputHeight;
    uint32_t          outputWidth = copybitparamprofile.outputWidth;
    uint32_t          outputHeight = copybitparamprofile.outputHeight;
    uint32_t          inputSize[3];
    uint32_t          outputSize[3];
    int32_t           ionInputFD;
    int32_t           ionOutputFD;
    DP_COLOR_ENUM     inputFormat = mapColorFormat(copybitparamprofile.inputFormat);
    DP_PROFILE_ENUM   src_prof = (DP_PROFILE_ENUM)static_cast<int32_t>(copybitparamprofile.src_profile);
    DP_COLOR_ENUM     outputFormat= mapColorFormat(copybitparamprofile.outputFormat);
    DP_PROFILE_ENUM   dst_prof = (DP_PROFILE_ENUM)static_cast<int32_t>(copybitparamprofile.dst_profile);
    uint32_t          inPlanes = DP_COLOR_GET_PLANE_COUNT(inputFormat);
    uint32_t          outPlanes = DP_COLOR_GET_PLANE_COUNT(outputFormat);

    ALOGD("cobybit: Start\n");

    if (inputFormat == DP_COLOR_UNKNOWN || outputFormat == DP_COLOR_UNKNOWN)
    {
        ALOGE("copybit: Not support inputFormat = %d or outputFormat = %d \n", inputFormat, outputFormat);
        return int32_t {DP_STATUS_INVALID_PARAX};
    }

    ALOGD("copybit: inputWidth = %d, inputHeight = %d, outputWidth = %d, outputHeight = %d\n", inputWidth, inputHeight, outputWidth, outputHeight);

    std::unique_ptr<DpBlitStream> pStream (new DpBlitStream());

    if (!pStream.get())
    {
        ALOGE("copybit: new DpBlitStream fail\n");
        return int32_t {DP_STATUS_UNKNOWN_ERROR};
    }

    /*input*/
    ret = gralloc_extra_query(copybitparamprofile.inputHandle, GRALLOC_EXTRA_GET_ION_FD, &ionInputFD);
    if (ret < 0)
    {
        ALOGE("copybit: gralloc_extra_query failed\n");
    }

    if (!copybitparamprofile.src_yPitch)
    {
        pStream->setSrcConfig(inputWidth, inputHeight, inputFormat);
        copybit_Bufsize(inputSize, inputFormat, inPlanes, inputWidth, inputHeight);
    }
    else
    {
        pStream->setSrcConfig(inputWidth, inputHeight, copybitparamprofile.src_yPitch, copybitparamprofile.src_uvPitch, inputFormat, src_prof);
        for (i = 0; i < 3; i++)
        {
            inputSize[i] = copybitparamprofile.src_sizeList[i];
        }
    }

    status = pStream->setSrcBuffer(ionInputFD, inputSize, inPlanes);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("copybit: setSrcBuffer fail with error = %d\n", status);
        return int32_t {status};
    }

    /*rotate & flip*/
    pStream->setRotate(copybitparamprofile.rotation);
    pStream->setFlip(copybitparamprofile.flipMirror);

    /*out*/
    ret = gralloc_extra_query(copybitparamprofile.outputHandle, GRALLOC_EXTRA_GET_ION_FD, &ionOutputFD);
    if (ret < 0)
    {
        ALOGE("copybit: gralloc_extra_query failed\n");
    }

    if (!copybitparamprofile.dst_yPitch)
    {
        pStream->setDstConfig(outputWidth, outputHeight, outputFormat);
        copybit_Bufsize(outputSize, outputFormat, outPlanes, outputWidth, outputHeight);
    }
    else
    {
        pStream->setDstConfig(outputWidth, outputHeight, copybitparamprofile.dst_yPitch, copybitparamprofile.dst_uvPitch, outputFormat, dst_prof);
        for (i = 0; i < 3; i++)
        {
            outputSize[i] = copybitparamprofile.dst_sizeList[i];
        }
    }

    pStream->setDstBuffer(ionOutputFD, outputSize, outPlanes);

    /*invalidate*/
    ALOGD("cobybit: Invalidate start\n");
    status = pStream->invalidate();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("copybit: Invalidate fail with error = %d\n", status);
        return int32_t {status};
    }

    ALOGD("cobybit: End\n");

    return int32_t {status};
}

Return<int32_t> Mms::BlitStreamAUX(const ::vendor::mediatek::hardware::mms::V1_5::HwMDPParamAUX& mdpparam) {
    DP_STATUS_ENUM status;
    int32_t i;

    Mutex::Autolock autoLock(mLock);
    DpAsyncBlitStream *pStream = mAsyncBlitStream;

    uint32_t src_pSizeList[3];
    uint32_t dst_pSizeList[3];

    /* set src crop*/
    ::DpRect       SrcRect;
    SrcRect.x = mdpparam.src_rect.x;
    SrcRect.y = mdpparam.src_rect.y;
    SrcRect.w = mdpparam.src_rect.w;
    SrcRect.h = mdpparam.src_rect.h;

    /* set dst roi*/
    ::DpRect       DstRect;
    DstRect.x = mdpparam.dst_rect.x;
    DstRect.y = mdpparam.dst_rect.y;
    DstRect.w = mdpparam.dst_rect.w;
    DstRect.h = mdpparam.dst_rect.h;

    DP_COLOR_ENUM     inputFormat = (DP_COLOR_ENUM)mdpparam.src_format;
    DP_COLOR_ENUM     outputFormat = (DP_COLOR_ENUM)mdpparam.dst_format;

    DP_PROFILE_ENUM src_prof = (DP_PROFILE_ENUM)static_cast<int32_t>(mdpparam.src_profile);
    DP_PROFILE_ENUM dst_prof = (DP_PROFILE_ENUM)static_cast<int32_t>(mdpparam.dst_profile);

    /* set src & dst sizeList*/
    for (i = 0; i < 3; i++)
    {
        src_pSizeList[i] = mdpparam.src_sizeList[i];
        dst_pSizeList[i] = mdpparam.dst_sizeList[i];
    }

    uint32_t job = 0;
    int32_t dp_fence = -1;
    status = pStream->createJob(job, dp_fence);
    if (DP_STATUS_RETURN_SUCCESS != status || dp_fence < 0) {
        ALOGE("Mms: createJob fail with error = %d, dp_fence %d\n", status, dp_fence);
        return int32_t {status};
    }
    pStream->setConfigBegin(job);

    /* src config*/
    if (!mdpparam.src_yPitch)
    {
        pStream->setSrcConfig(mdpparam.src_width, mdpparam.src_height, inputFormat, eInterlace_None);
    }
    else
    {
        pStream->setSrcConfig(mdpparam.src_width, mdpparam.src_height, mdpparam.src_yPitch, mdpparam.src_uvPitch,
            inputFormat, src_prof, eInterlace_None, DP_SECURE_NONE, false);
    }
    pStream->setSrcCrop(0, SrcRect);

    /* src buffer*/
    status = pStream->setSrcBuffer((mdpparam.inputHandle)->data[0], src_pSizeList, mdpparam.src_planeNumber);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("Mms: setSrcBuffer fail with error = %d\n", status);
        //return int32_t {status};
    }

    /* dst config*/
    if (!mdpparam.dst_yPitch)
    {
        pStream->setDstConfig(0, mdpparam.dst_width, mdpparam.dst_height, outputFormat, eInterlace_None, &DstRect);
    }
    else
    {
        pStream->setDstConfig(0, mdpparam.dst_width, mdpparam.dst_height, mdpparam.dst_yPitch, mdpparam.dst_uvPitch,
            outputFormat, dst_prof, eInterlace_None, &DstRect, DP_SECURE_NONE, false);
    }

    /* dst buffer*/
    status = pStream->setDstBuffer(0, (mdpparam.outputHandle)->data[0], dst_pSizeList, mdpparam.dst_planeNumber);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("Mms: setDstBuffer fail with error = %d\n", status);
        //return int32_t {status};
    }

    if (!mdpparam.transform)
    {
        /* roation*/
        pStream->setRotate(0, mdpparam.rotation);

        /* flip*/
        pStream->setFlip(0, mdpparam.flip);
    }
    else
    {
        /* orientation */
        pStream->setOrientation(0, mdpparam.transform);
    }

    /* pq */

    pStream->setConfigEnd();

    struct timeval target_end;
    target_end.tv_sec = mdpparam.end_time.sec;
    target_end.tv_usec = mdpparam.end_time.usec;
    status = pStream->invalidate(&target_end);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        ALOGE("Mms: Invalidate fail with error = %d\n", status);
        delete pStream;
        // Otherwise, the API cannot be recovered.
        mAsyncBlitStream = new DpAsyncBlitStream();
        mAsyncBlitStream->setUser(DP_BLIT_GPU);
    }
    else
    {
        sync_wait(dp_fence, -1);
    }
    close(dp_fence);
    return int32_t {status};
}

// Minimum Y pitch that is acceptable by HW
#define DP_COLOR_GET_MIN_Y_PITCH(color, width)                                              \
    (((DP_COLOR_BITS_PER_PIXEL(color) * width) + 4) >> 3)

// Minimum UV pitch that is acceptable by HW
#define DP_COLOR_GET_MIN_UV_PITCH(color, width)                                             \
    ((1 == DP_COLOR_GET_PLANE_COUNT(color))? 0:                                             \
     (((0 == DP_COLOR_IS_UV_COPLANE(color)) || (1 == DP_COLOR_GET_BLOCK_MODE(color)))?      \
      (DP_COLOR_GET_MIN_Y_PITCH(color, width)  >> DP_COLOR_GET_H_SUBSAMPLE(color)):         \
      ((DP_COLOR_GET_MIN_Y_PITCH(color, width) >> DP_COLOR_GET_H_SUBSAMPLE(color)) * 2)))

// Minimum Y plane size that is necessary in buffer
#define DP_COLOR_GET_MIN_Y_SIZE(color, width, height)                                       \
    ((1 == DP_COLOR_GET_BLOCK_MODE(color))?                                                 \
     (((DP_COLOR_BITS_PER_PIXEL(color) * width) >> 8) * height):                            \
     (DP_COLOR_GET_MIN_Y_PITCH(color, width) * height))

// Minimum UV plane size that is necessary in buffer
#define DP_COLOR_GET_MIN_UV_SIZE(color, width, height)                                                  \
    (((1 < DP_COLOR_GET_PLANE_COUNT(color)) && (1 == DP_COLOR_GET_BLOCK_MODE(color)))?                  \
     (((DP_COLOR_BITS_PER_PIXEL(color) * width) >> 8) * (height >> DP_COLOR_GET_V_SUBSAMPLE(color))):   \
     (DP_COLOR_GET_MIN_UV_PITCH(color, width) * (height >> DP_COLOR_GET_V_SUBSAMPLE(color))))


void copybit_Bufsize(uint32_t *size, int32_t colorFormat, int32_t planes, uint32_t width, uint32_t height)
{
    switch(planes)
    {
        case 3:
            size[2] = DP_COLOR_GET_MIN_UV_SIZE((DP_COLOR_ENUM)colorFormat, width, height);
        case 2:
            size[1] = DP_COLOR_GET_MIN_UV_SIZE((DP_COLOR_ENUM)colorFormat, width, height);
        case 1:
            size[0] = DP_COLOR_GET_MIN_Y_SIZE((DP_COLOR_ENUM)colorFormat, width, height);
            break;
        default:
            size[0] = 0;
        }
}

#ifdef MMS_SUPPORT_JPG_ENC
bool mmsJpeg_query_buffer(JINT32 ion_client, ion_user_handle_t handle, JUINT32 *PAddr, JUINT32 *bufferSize, JINT32 MODULE_ID_ENUM)
{
	if (handle == -1 || ion_client < 0)
	{
		ALOGE("Mms: JpgENC: BAD ion client(%d), ion handle(%p)!!!\n", ion_client, handle);
		return 0;
	}

	bool ret = 0;
	//config module to get physical address
	struct ion_mm_data mm_data;

	mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = handle;
    mm_data.config_buffer_param.eModuleID = MODULE_ID_ENUM;
    mm_data.config_buffer_param.security = 0;
    mm_data.config_buffer_param.coherent = 0;

	if( ion_custom_ioctl(ion_client, ION_CMD_MULTIMEDIA, &mm_data) )
	{
		ALOGE("Mms: JpgENC: config buffer FAILED(%d)!!!\n", MODULE_ID_ENUM);
		PAddr = NULL;
		bufferSize = 0;
		return ret;
	}

	//get physical address
	struct ion_sys_data sys_data;

	sys_data.sys_cmd = ION_SYS_GET_PHYS;
	sys_data.get_phys_param.handle = handle;

	ALOGD("Mms: JpgENC: ion_client: %d, handle: %d, CMD: 0x%x, PAddr: 0x%x, Data Len: %d\n"
		, ion_client, sys_data.get_phys_param.handle, sys_data.sys_cmd, *PAddr, *bufferSize);		//UT Debug Log

	if( ion_custom_ioctl(ion_client, ION_CMD_SYSTEM, &sys_data) )
	{
		ALOGE("Mms: JpgENC: get buffer phys addr FAILED!!!\n");
		PAddr = NULL;
		bufferSize = 0;
	}
	else
	{
		*PAddr = sys_data.get_phys_param.phy_addr;
		*bufferSize = sys_data.get_phys_param.len;
		ALOGD("Mms: JpgENC: PAddr: 0x%x, Data Len: %d\n", *PAddr, *bufferSize);		//UT Debug Log
		ret = 1;
	}
	return ret;
}

bool mmsJpeg_getVA(JINT32 bufFD, JUINT32 size, void **VA)
{
	//get virtual address
	*VA = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, bufFD, NULL);
	if ((*VA == NULL) || (*VA == (void*)(-1)))
	{
		ALOGE("Mms: MMAP FAILED!!! FD: %d, VA: %lx, size: %x\n", bufFD, (unsigned long)*VA, size);
		*VA = NULL;
		return false;
	}

	return true;
}

void mmsJpeg_freeVA(JUINT32 size, void **VA)
{
	if (*VA != NULL)
		munmap(*VA, size);
	*VA = NULL;

	return;
}

#endif

// Methods from ::android::hidl::base::V1_0::IBase follow.

IMms* HIDL_FETCH_IMms(const char* /* name */) {
    return new Mms();
}
//
}  // namespace implementation
}  // namespace V1_5
}  // namespace mms
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
