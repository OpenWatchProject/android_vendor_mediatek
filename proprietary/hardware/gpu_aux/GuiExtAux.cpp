
#include <GuiExtAux.h>
#include <graphics_mtk_defs.h>
#include <gralloc1_mtk_defs.h>
#include <string.h>
#include <hardware/gralloc.h>
#include <ui/gralloc_extra.h>
//#include <DpBlitStream.h>
#include <sync/sync.h>
#include <DpAsyncBlitStream2.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <cutils/properties.h>
#ifdef MTK_GPUD_SUPPORT
#include <gpud/gpud_api.h>
#endif
#include <aux/aux_gralloc.h>
#include <aux/aux_mapper.h>

#include "utils.h"
#include "mtk_queue.h"
#include "aux/GpuAux.h"

using namespace android;

DP_PROFILE_ENUM gInitColorSpace = DP_PROFILE_BT601;

#define SUPPORT_MM_BINDER 1

#if SUPPORT_MM_BINDER
#include <cutils/native_handle.h>
#include <vendor/mediatek/hardware/mms/1.5/IMms.h>
using ::vendor::mediatek::hardware::mms::V1_5::IMms;
using ::vendor::mediatek::hardware::mms::V1_5::HwMDPParamAUX;
using ::vendor::mediatek::hardware::mms::V1_5::MMS_ORIENTATION_ENUM;
using ::vendor::mediatek::hardware::mms::V1_2::MMS_PROFILE_ENUM;
#endif

int guiExtIsMultipleDisplayScene(void)
{
#define LIB_FULL_NAME "libgui_ext.so"
#define FUNC_FULL_NAME "guiExtIsMultipleDisplays"

    typedef status_t (*FNguiExtIsMultipleDisplays)(int32_t *const);

    static int inited;
    static FNguiExtIsMultipleDisplays pfn;
    static void *handle;

    if (inited == 0) {
        const char *err_str = NULL;

        inited = 1; /* inited */

        handle = dlopen(LIB_FULL_NAME, RTLD_NOW);
        if (handle == NULL) {
            MTK_LOGE("dlopen " LIB_FULL_NAME " failed");
        }

        dlerror(); /* clear error */
        pfn = reinterpret_cast<FNguiExtIsMultipleDisplays>(dlsym(handle, FUNC_FULL_NAME));

        if ((err_str = dlerror()) || pfn == NULL) {
            MTK_LOGE("dlsym " FUNC_FULL_NAME " failed, [%s]", err_str);
        } else {
            inited = 2;    /* init success */
        }
    }

    int ret = 0;
    int is_multipledisplay = 0;

    if (inited == 2) {
        ret = pfn(&is_multipledisplay);
        if (ret) {
            MTK_LOGE("call " LIB_FULL_NAME " - " FUNC_FULL_NAME " ret error: %d", ret);
        }
    }

    return (is_multipledisplay > 0);
}

struct GuiExtAuxBufferItem {
public:
    android_native_buffer_t *mSrcBuffer;
    sp<GPUAUXBuffer>         mMTKBuffer;

    GuiExtAuxBufferItem() :
        mSrcBuffer(NULL),
        mMTKBuffer(NULL)
    {}

    ~GuiExtAuxBufferItem()
    {
        mMTKBuffer = NULL;
    }
};

struct GuiExtAuxBufferQueue {
public:
    Mutex                 mMutex;

    GPUAUXBufferQueue     mMTKQueue;
    GuiExtAuxBufferItem  *mSlots;

    int                   width;
    int                   height;
    int                   format;
    int                   num_slots;

    DpAsyncBlitStream2   *bltStream2;
#if SUPPORT_MM_BINDER
    sp<IMms>              mmservice;
#endif
    bool                  oldScltmEnable;

    GuiExtAuxBufferQueue(int _width, int _height, int _format, int _num_slots) :
        mMTKQueue(_num_slots),
        mSlots(new GuiExtAuxBufferItem[_num_slots]),
        width(_width),
        height(_height),
        format(_format),
        num_slots(_num_slots),
        bltStream2(NULL),
#if SUPPORT_MM_BINDER
        mmservice(0),
#endif
        oldScltmEnable(0)
    {}

    ~GuiExtAuxBufferQueue()
    {
        delete [] mSlots;
        if (bltStream2) {
            delete bltStream2;
        }
    }
};

/* // unused function, mark it.
static buffer_handle_t guiExtAuxGetBufferHandle(android_native_buffer_t *buffer)
{
    if ((buffer->common.magic   == ANDROID_NATIVE_BUFFER_MAGIC) &&
        (buffer->common.version == sizeof(android_native_buffer_t))) {
        return buffer->handle;
    }
    return 0;
}
*/

void guiExtAuxSetWidthHieghtFromSrcBuffer(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t *src_buffer)
{
    GuiExtAuxSetSize(bq, src_buffer->width, src_buffer->height);
}

static int guiExtAuxIsUIPQFormat(int format)
{
    if (
        (format == HAL_PIXEL_FORMAT_RGBX_8888) ||
        (format == HAL_PIXEL_FORMAT_RGBA_8888) ||
        (format == HAL_PIXEL_FORMAT_RGB_888)   ||
        (format == HAL_PIXEL_FORMAT_RGB_565)   ||
        0) {
        return 1;
    }

    return 0;
}

/*
 * return -1, use the defalut value.
 * reutrn corresponding profile, otherwise.
 */
#if GPUD_GRALLOC_VERSION_MAJOR >= 2
static int guiExtAuxGetYUVColorSpace(AuxBufferInfo *info, DP_PROFILE_ENUM *ret, bool is_src)
#else
static int guiExtAuxGetYUVColorSpace(GPUAUXBufferInfo_t *info, DP_PROFILE_ENUM *ret, bool is_src)
#endif
{
    unsigned int yuv_info = info->status & GRALLOC_EXTRA_MASK_YUV_COLORSPACE;
#ifdef MTK_GPUD_SUPPORT
    GPUD_API_ENTRY(AuxHackAUXImageYUVColorSpace, &yuv_info);
#endif
    switch (yuv_info) {
        case GRALLOC_EXTRA_BIT_YUV_BT601_NARROW:
            *ret = DP_PROFILE_BT601;
            gInitColorSpace = *ret;
            return 0;
        case GRALLOC_EXTRA_BIT_YUV_BT601_FULL:
            *ret = DP_PROFILE_FULL_BT601;
            gInitColorSpace = *ret;
            return 0;
        case GRALLOC_EXTRA_BIT_YUV_BT709_NARROW:
            *ret = DP_PROFILE_BT709;
            gInitColorSpace = *ret;
            return 0;
        case GRALLOC_EXTRA_BIT_YUV_BT709_FULL:
            // OVL not support bt709 full output
            *ret = DP_PROFILE_FULL_BT709;
            if(is_src)
                    *ret = DP_PROFILE_FULL_BT709;
            else
                    *ret = DP_PROFILE_BT709;
            gInitColorSpace = *ret;
            return 0;
        case GRALLOC_EXTRA_BIT_YUV_BT2020_NARROW:
            // Need to do the mapping since MDP doesn't support BT2020
            if(is_src)
                *ret = DP_PROFILE_BT2020;
            else
                *ret = DP_PROFILE_BT709;
            gInitColorSpace = *ret;
            return 0;
        case GRALLOC_EXTRA_BIT_YUV_BT2020_FULL:
            // Need to do the mapping since MDP doesn't support BT2020
            if(is_src)
                *ret = DP_PROFILE_FULL_BT2020;
            else
                *ret = DP_PROFILE_BT709;
            gInitColorSpace = *ret;
            return 0;
    }

    /* UI PQ should use JPEG. */
    if (guiExtAuxIsUIPQFormat(info->format)) {
        *ret = DP_PROFILE_JPEG;
        gInitColorSpace = *ret;
        return 0;
    }

    *ret = gInitColorSpace;
    return -1;
}

static void guiExtAuxSetYUVColorSpace(android_native_buffer_t *buffer, DP_PROFILE_ENUM value)
{
    int mask = GRALLOC_EXTRA_MASK_YUV_COLORSPACE;
    int bit = 0;

    switch (value) {
        case DP_PROFILE_BT601:
            bit = GRALLOC_EXTRA_BIT_YUV_BT601_NARROW;
            break;
        case DP_PROFILE_FULL_BT601:
            bit = GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            break;
        case DP_PROFILE_BT709:
            bit = GRALLOC_EXTRA_BIT_YUV_BT709_NARROW;
            break;
        case DP_PROFILE_FULL_BT709:
            bit = GRALLOC_EXTRA_BIT_YUV_BT709_FULL;
            break;

        default:
            return;
    }

#if GPUD_GRALLOC_VERSION_MAJOR >= 2
    AuxGralloc::getInstance().setBufferSFInfo(buffer, mask, bit);
#else
    MTKGralloc::getInstance()->setBufferPara(buffer, mask, bit);
#endif
}

static void guiExtAuxDokickConversion(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t *src_buffer)
{
    /* update the w/h for output buffer */
    guiExtAuxSetWidthHieghtFromSrcBuffer(bq, src_buffer);

    int bufSlot = -1;
    int fence_fd = -1;

    GuiExtAuxDequeueBuffer(bq, &bufSlot, &fence_fd);

    if (fence_fd >= 0) {
        close(fence_fd);
        fence_fd = -1;
    }

    GuiExtAuxQueueBuffer(bq, bufSlot, -1);
}

static GuiExtAuxBufferItemHandle guiExtAuxFindItem(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t *buffer)
{
    for (int i = 0; i < bq->num_slots; ++i) {
        android_native_buffer_t *b = GuiExtAuxRequestBuffer(bq, i);

        if (b == buffer) {
            return &bq->mSlots[i];
        }
    }

    return NULL;
}

extern "C" { /* begin of extern "C" */

    int GuiExtAuxIsSupportFormat(android_native_buffer_t *anb)
    {
        if (anb != NULL) {
            int format = anb->format;
#ifdef MTK_GPUD_SUPPORT
            int hack_format = 0;
            GPUD_API_ENTRY(AuxHackAUXImageSupportFormat, &hack_format);
#endif
            if ((format == HAL_PIXEL_FORMAT_I420) ||
                (format == HAL_PIXEL_FORMAT_YCbCr_420_888) ||
                (format == HAL_PIXEL_FORMAT_NV12_BLK) ||
                (format == HAL_PIXEL_FORMAT_NV12_BLK_FCM) ||
                (format == HAL_PIXEL_FORMAT_YUV_PRIVATE) ||
                (format == HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H) ||
                (format == HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V) ||
                (format == HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H_JUMP) ||
                (format == HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V_JUMP) ||
                (format == HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT) ||
                (format == HAL_PIXEL_FORMAT_UFO) ||
                (format == HAL_PIXEL_FORMAT_UFO_AUO) ||
                (format == HAL_PIXEL_FORMAT_UFO_10BIT_H) ||
                (format == HAL_PIXEL_FORMAT_UFO_10BIT_V) ||
                (format == HAL_PIXEL_FORMAT_UFO_10BIT_H_JUMP) ||
                (format == HAL_PIXEL_FORMAT_UFO_10BIT_V_JUMP) ||
                (format == HAL_PIXEL_FORMAT_YCbCr_422_I) ||
#ifdef MTK_GPUD_SUPPORT
                (format == hack_format) ||
#endif
                0) {
                return 1;
            }
        }

        return 0;
    }

    /*
     * This interface is to support UI PQ format.
     * We don't want to change GuiExtAuxIsSupportFormat behavior that may
     * cause in-use DDK failed.
     */
    int GuiExtAuxIsUIPQFormat(android_native_buffer_t *anb)
    {
        if (anb != NULL) {
            int format = anb->format;

            if (guiExtAuxIsUIPQFormat(format)) {
                return 1;
            }
        }

        return 0;
    }

    unsigned int GuiExtAuxGetStatus(android_native_buffer_t* anb, int StatusVer)
    {
        GRALLOC_EXTRA_RESULT result = GRALLOC_EXTRA_OK;
        int ion_fd = -1;
        result = gralloc_extra_query(anb->handle, GRALLOC_EXTRA_GET_ION_FD, &ion_fd);

        if ((GRALLOC_EXTRA_OK == result) && (-1 != ion_fd))
        {
            gralloc_extra_ion_sf_info_t sf_info;
            result = gralloc_extra_query(anb->handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
            if (GRALLOC_EXTRA_OK == result)
            {
                if(StatusVer == 1)
                {
                    return sf_info.status;
                }else if(StatusVer == 2)
                {
                    return sf_info.status2;
                }
            }
        }
        return 0;
    }

    int GuiExtAuxCheckAuxPath(android_native_buffer_t *anb, void *gpu_info)
    {
        AuxGPUInfo *local_gpu_info = (AuxGPUInfo *)gpu_info;
        if (anb) {
            /* UIPQ condition */
            if (GuiExtAuxIsUIPQFormat(anb) &&
                ((GuiExtAuxGetStatus(anb, 2 /*status2*/) & GRALLOC_EXTRA_MASK2_UI_PQ) == GRALLOC_EXTRA_BIT2_UI_PQ_ON))
            {
#ifdef MTK_GPUD_SUPPORT
                char condition[100];
                snprintf(condition, 100, "Fmt=%x && UI_PQ_ON, ", anb->format);
                GPUD_API_ENTRY(AuxShowAUXPath, condition, PATH_AUX_UIPQ);
#endif
                return PATH_AUX_UIPQ;
            }

#ifdef MTK_PQ_VIDEO_WHITELIST_SUPPORT
            /* SCLTM condition */
            ge_pq_scltm_info_t pq_scltm_info;
            gralloc_extra_query(anb->handle, GRALLOC_EXTRA_GET_PQ_SCLTM_INFO, &pq_scltm_info);
            if (GuiExtAuxIsUIPQFormat(anb) &&
                pq_scltm_info.scltmEnable &&
                (!(GuiExtAuxGetStatus(anb, 2 /*status2*/) & GRALLOC_EXTRA_MASK2_BUF_COMPRESSION_STATUS)))
            {
#ifdef MTK_GPUD_SUPPORT
                char condition[100];
                snprintf(condition, 100, "Fmt=%x && scltm Enable && Not Compressed, ", anb->format);
                GPUD_API_ENTRY(AuxShowAUXPath, condition, PATH_AUX_UIPQ);
#endif
                // SCLTM need to set output to rgb
                return PATH_AUX_UIPQ;
            }
#endif

#ifdef MTK_GAMEPQ_SUPPORT
            /* PQ_MIRA condition */
            ge_pq_mira_vision_info_t pq_mira_vision_info;
            gralloc_extra_query(anb->handle, GRALLOC_EXTRA_GET_PQ_MIRA_VISION_INFO, (void *)&pq_mira_vision_info);

            if (GuiExtAuxIsUIPQFormat(anb) && (0 < pq_mira_vision_info.inc_count))
            {
#ifdef MTK_GPUD_SUPPORT
                char condition[100];
                snprintf(condition, 100, "Fmt=0x%x && 0 < pq_mira_vision_info.inc_count, ", anb->format);
                GPUD_API_ENTRY(AuxShowAUXPath, condition, PATH_AUX_UIPQ);
#endif
                return PATH_AUX_UIPQ;
            }
#endif
            /* All in Video for PQ */
            if (local_gpu_info->is_vedio_PQ_support)
            {
                if ((GuiExtAuxGetStatus(anb, 1 /*status*/) & GRALLOC_EXTRA_MASK_TYPE) == GRALLOC_EXTRA_BIT_TYPE_VIDEO)
                {
                    return PATH_AUX_VIDEO;
                }
            }

            /* Normal Video AUX condition */
            if (GuiExtAuxIsSupportFormat(anb))
            {
                if (local_gpu_info->vendor == GPU_MALI)
                {
                    /*
                    * MTK: Mali handled the HAL_PIXEL_FORMAT_YCbCr_420_888 as MALI_GRALLOC_FORMAT_INTERNAL_NV12 in both of the
                    * gralloc and ddk.
                    * In general cases, the HAL_PIXEL_FORMAT_YCbCr_420_888 with specific usages will be transfered in Gralloc Extra,
                    * and if not, here we just bypass the AUX handling if the input src buffer is HAL_PIXEL_FORMAT_YCbCr_420_888
                    * and let it use the Mali's native handling.
                    */
                    if (anb->format != HAL_PIXEL_FORMAT_YCbCr_420_888)
                    {
#ifdef MTK_GPUD_SUPPORT
                        char condition[100];
                        snprintf(condition, 100, "Fmt=0x%x && GPU_MALI && Fmt != HAL_PIXEL_FORMAT_YCbCr_420_888, ", anb->format);
                        GPUD_API_ENTRY(AuxShowAUXPath, condition, PATH_AUX_VIDEO);
#endif
                        return PATH_AUX_VIDEO;
                    }
                }
                else // GPU_IMG
                {
                    if (local_gpu_info->is_nv21_blk_support)
                    {
                        /*
                         * If GPU support NV12_BLK format, we will let GPU deal with it and not entering the AUX.
                         */
                        if ((anb->format != HAL_PIXEL_FORMAT_NV12_BLK) &&
                            (anb->format != HAL_PIXEL_FORMAT_NV12_BLK_FCM) &&
                            (anb->format != HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H) &&
                            (anb->format != HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V)
                            )
                        {
#ifdef MTK_GPUD_SUPPORT
                            char condition[100];
                            snprintf(condition, 100, "Fmt=0x%x && GPU_IMG && NV21_BLK_SUPPORT && Fmt != HAL_PIXEL_FORMAT_NV12_BLKxxx, ", anb->format);
                            GPUD_API_ENTRY(AuxShowAUXPath, condition, PATH_AUX_VIDEO);
#endif
                            return PATH_AUX_VIDEO;
                        }
                    }
                    else
                    {
#ifdef MTK_GPUD_SUPPORT
                            char condition[100];
                            snprintf(condition, 100, "Fmt=0x%x && GPU_IMG && !NV21_BLK_SUPPORT, ", anb->format);
                            GPUD_API_ENTRY(AuxShowAUXPath, condition, PATH_AUX_VIDEO);
#endif
                        return PATH_AUX_VIDEO;
                    }
                }
            }
        }
#ifdef MTK_GPUD_SUPPORT
        char condition[100];
        if (anb)
        {
            snprintf(condition, 100, "Fmt=0x%x, ", anb->format);
        }
        else
        {
            snprintf(condition, 100, "Fmt=None, ");
        }
        GPUD_API_ENTRY(AuxShowAUXPath, condition, PATH_NO_AUX);
#endif
        return PATH_NO_AUX;
    }

    int GuiExtAuxIsSupportPQWhiteList()
    {
        //Since ddk can't get mtk config option, use this function instead
#if defined(MTK_PQ_VIDEO_WHITELIST_SUPPORT)
        return 1;
#endif
        return 0;
    }

    static int guiExtIsSurfaceFlinger();

    GuiExtAuxBufferQueueHandle GuiExtAuxCreateBufferQueue(int width, int height, int output_format, int num_max_buffer)
    {
        GuiExtAuxBufferQueueHandle bq;

        if ((output_format != HAL_PIXEL_FORMAT_YV12) &&
            (output_format != HAL_PIXEL_FORMAT_YCrCb_420_SP) &&
            (output_format != HAL_PIXEL_FORMAT_RGBA_8888) &&
            (output_format != HAL_PIXEL_FORMAT_RGB_888)) {
            MTK_LOGE("Unsupported color format %d", output_format);
            return NULL;
        }

        if (num_max_buffer < 1) {
            num_max_buffer = 2;
            MTK_LOGE("num_max_buffer(%d) < 1", num_max_buffer);
        }

        bq = new GuiExtAuxBufferQueue(width, height, output_format, num_max_buffer);

        if (bq == NULL) {
            MTK_LOGE("GPU_AUX_createBufferQueue allocate fail, out of memory");
            return NULL;
        }

#if SUPPORT_MM_BINDER
        if (guiExtIsSurfaceFlinger()) {
#endif
            int ret = 0;
            bq->bltStream2 = new DpAsyncBlitStream2();
            ret = bq->bltStream2->setUser(DP_BLIT_GPU);
            if (DP_STATUS_RETURN_SUCCESS != ret) {
                MTK_LOGE("bltStream2 setUser fail %d", ret);
                return NULL;
            }
#if SUPPORT_MM_BINDER
        } else {
            bq->mmservice = IMms::getService();
            if (bq->mmservice == nullptr) {
                MTK_LOGE("Cannot get mms_hidl service");
                return NULL;
            }
        }
#endif

        return bq;

    }

    void GuiExtAuxDestroyBufferQueue(GuiExtAuxBufferQueueHandle bq)
    {
        delete bq;
    }

    void GuiExtAuxSetSize(GuiExtAuxBufferQueueHandle bq, int width, int height)
    {
        bq->width = width;
        bq->height = height;
    }

    void GuiExtAuxSetName(GuiExtAuxBufferQueueHandle bq, const char *name)
    {
        bq->mMTKQueue.setConsumerName(name);
    }

    void GuiExtAuxKickConversion(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t *src_buffer)
    {
        guiExtAuxDokickConversion(bq, src_buffer);
    }

    static int guiExtIsSurfaceFlinger()
    {
        static int bInitAppName = -1;

        if (bInitAppName == -1) {
            char path[PATH_MAX];
            char cmdline[PATH_MAX] = "";
            FILE *file;

            snprintf(path, PATH_MAX, "/proc/%d/cmdline", getpid());
            file = fopen(path, "r");
            if (file) {
                fgets(cmdline, sizeof(cmdline) - 1, file);
                fclose(file);
            } else {
                /* Open file fail, should never happen. What can we do?  */
                MTK_LOGE("open [%s] fail.", path);
            }
            bInitAppName = (strcmp(cmdline, "/system/bin/surfaceflinger") == 0);
        }

        return bInitAppName;
    }

    void GuiExtSetDstColorSpace(android_native_buffer_t *dst_buffer, android_native_buffer_t *src_buffer)
    {
#if GPUD_GRALLOC_VERSION_MAJOR >= 2
        AuxBufferInfo src_info = AuxGralloc::getInstance().getBufferInfo(src_buffer);
#else
        GPUAUXBufferInfo_t src_info = MTKGralloc::getInstance()->getBufferInfo(src_buffer);
#endif

        DP_PROFILE_ENUM out_dp_profile;
        if (guiExtAuxGetYUVColorSpace(&src_info, &out_dp_profile, false) != 0) {
            MTK_LOGW("src does not specify the COLORSPACE, use default: narrow-range");
        }

        guiExtAuxSetYUVColorSpace(dst_buffer, out_dp_profile);
    }

    static int PrivateFormat2HALFormat(int status)
    {
#undef GEN_CASE
#define GEN_CASE(f) case GRALLOC_EXTRA_BIT_CM_##f: return HAL_PIXEL_FORMAT_##f

        switch (status & GRALLOC_EXTRA_MASK_CM) {
                GEN_CASE(YV12);
                GEN_CASE(I420);
                GEN_CASE(NV12_BLK);
                GEN_CASE(NV12_BLK_FCM);
                GEN_CASE(NV12_BLK_10BIT_H);
                GEN_CASE(NV12_BLK_10BIT_V);
                GEN_CASE(YUYV);
                GEN_CASE(NV12);
        }
        return -1;
    }

/* // unused function, mark it.
    static DpColorFormat PrivateFormat2MDPFormat(int status)
    {
#undef GEN_CASE
#define GEN_CASE(f) case GRALLOC_EXTRA_BIT_CM_##f: return DP_COLOR_##f
        switch (status & GRALLOC_EXTRA_MASK_CM) {
                GEN_CASE(YV12);
                GEN_CASE(I420);
        }
        return (DpColorFormat) - 1;
    }
*/

#if SUPPORT_MM_BINDER
    static MMS_PROFILE_ENUM DpToMmsEnum(DP_PROFILE_ENUM dp_enum)
    {
        if (dp_enum == DP_PROFILE_BT601) {
            return MMS_PROFILE_ENUM::MMS_PROFILE_BT601;
        } else if (dp_enum == DP_PROFILE_BT709) {
            return MMS_PROFILE_ENUM::MMS_PROFILE_BT709;
        } else if (dp_enum == DP_PROFILE_JPEG) {
            return MMS_PROFILE_ENUM::MMS_PROFILE_JPEG;
        }


        return MMS_PROFILE_ENUM::MMS_PROFILE_BT601;
    }
#endif

    int GuiExtAuxDoConversionIfNeed(GuiExtAuxBufferQueueHandle bq, android_native_buffer_t *dst_buffer, android_native_buffer_t *src_buffer, int crop_width, int crop_height)
    {
        ATRACE_CALL();

#ifdef MTK_GPUD_SUPPORT
        AUXImageInfo src_image_info, dst_image_info;
        AUXDpPqParamInfo gpud_dppq_param;

        src_image_info.height_align = -1;
        src_image_info.width_align = -1;
        src_image_info.uv_align = -1;
        src_image_info.is_omx_align = false;

        dst_image_info.height_align = -1;
        dst_image_info.width_align = -1;
        dst_image_info.uv_align = -1;
        dst_image_info.is_omx_align = false;
#endif

        int err = AUX_CONVERSION_SUCCESS;
        void *src_yp;

        int lockret;

#if SUPPORT_MM_BINDER
        struct HwMDPParamAUX hwMDPParamAUX;
        native_handle_t* srcHdl = NULL;
        native_handle_t* dstHdl = NULL;
        memset(&hwMDPParamAUX, 0, sizeof(hwMDPParamAUX));
#endif

        ge_pq_scltm_info_t pq_scltm_info;
#if defined(MTK_PQ_VIDEO_WHITELIST_SUPPORT)
        gralloc_extra_query(src_buffer->handle, GRALLOC_EXTRA_GET_PQ_SCLTM_INFO, &pq_scltm_info);
        if (!pq_scltm_info.scltmEnable) {
            pq_scltm_info.scltmPosition = 0;
        } else if (pq_scltm_info.scltmPosition < 1) {
            pq_scltm_info.scltmPosition = 1;
        } else if (pq_scltm_info.scltmPosition > 1024) {
            pq_scltm_info.scltmPosition = 1024;
        }

        // === for debug
        {
            char value[PROPERTY_VALUE_MAX];
            int position;

            property_get("debug.scltm.position", value, "-1");
            position = atoi(value);
            if (position >= 0 && position <= 1024) {
                pq_scltm_info.scltmEnable = 1;
                pq_scltm_info.scltmPosition = position;
            }
            if (bq->oldScltmEnable != pq_scltm_info.scltmEnable) {
                bq->oldScltmEnable = pq_scltm_info.scltmEnable;
                ALOGE("GPU Aux, pq_scltm_info.scltmEnable %d, pq_scltm_info.scltmPosition %d, pq_scltm_info.scltmOrientation %d\n",
                    pq_scltm_info.scltmEnable, pq_scltm_info.scltmPosition, pq_scltm_info.scltmOrientation);
            }
        }
        // ===
#else
        pq_scltm_info.scltmEnable = 0;
        pq_scltm_info.scltmPosition = 0;
        pq_scltm_info.scltmOrientation = 0;
        pq_scltm_info.scltmTableIndex = 0;
#endif

        DpColorFormat dp_out_fmt;
        int width_even;
        int height_even;
        DpAsyncBlitStream2 &bltStream2 = *bq->bltStream2;
        uint32_t job = 0;
        int32_t dp_fence = 0;
        int ret = 0;

        unsigned int src_offset[2] = {0, 0};
        unsigned int src_size[3];
        unsigned int dst_size[3];
        DpRect src_roi;
        DpRect dst_roi;

        DpColorFormat dp_in_fmt;
        int src_width = 0;
        int src_height = 0;

#if GPUD_GRALLOC_VERSION_MAJOR >= 2
        AuxBufferInfo src_info = AuxGralloc::getInstance().getBufferInfo(src_buffer);
        AuxBufferInfo dst_info = AuxGralloc::getInstance().getBufferInfo(dst_buffer);
#else
        GPUAUXBufferInfo_t src_info = MTKGralloc::getInstance()->getBufferInfo(src_buffer);
        GPUAUXBufferInfo_t dst_info = MTKGralloc::getInstance()->getBufferInfo(dst_buffer);
#endif

        GuiExtAuxBufferItemHandle hnd;

        struct timeval target_end;

        // pre-rotation
        int prot = 0;

        if (src_info.err != 0 || dst_info.err != 0) {
            MTK_LOGE("retrive info fail: src:%p dst:%p", src_buffer, dst_buffer);
            return AUX_CONVERSION_GET_BUFFER_INFO_FAIL;
        }

        if (src_info.usage & (GRALLOC1_PRODUCER_USAGE_PROTECTED | GRALLOC1_USAGE_SECURE)) {
            MTK_LOGV("skip, cannot convert protect / secure buffer");
            return AUX_CONVERSION_SKIP_PROTECT_BUFFER;
        }

        if (!pq_scltm_info.scltmEnable && guiExtIsSurfaceFlinger()) {
            if ((src_info.status & GRALLOC_EXTRA_MASK_AUX_DIRTY) == 0) {
                if (guiExtAuxIsUIPQFormat(src_info.format)) {
                    /* Launch an app when enable UI PQ, the start window no pq flag,
                          but the 1st frame of the app has PQ, which will lead to distortion.
                          Add this restriction to let the start window has PQ. */
                    if ((src_info.status & GRALLOC_EXTRA_MASK_TYPE) == GRALLOC_EXTRA_BIT_TYPE_GPU) {
                        //MTK_LOGV("rgb, src_info.status %d, return", src_info.status);
                        return AUX_CONVERSION_SKIP_NOT_DIRTY;
                    }
                } else {
                    /* conversion already be done. */
                    //MTK_LOGV("src_info.status %d, return", src_info.status);
                    ge_hdr_info_t hdr_info;
                    gralloc_extra_query(src_buffer->handle, GRALLOC_EXTRA_GET_HDR_INFO, &hdr_info);
                    // Determin u4TransformCharacter in HDR info, if equal 16
                    if (hdr_info.u4TransformCharacter != 16) {
                        return AUX_CONVERSION_SKIP_NOT_DIRTY;
                    }
                }
            }

            /* clear AUX dirty bit */
            gralloc_extra_ion_sf_info_t sf_info;

            gralloc_extra_query(src_buffer->handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
            gralloc_extra_sf_set_status(&sf_info, GRALLOC_EXTRA_MASK_AUX_DIRTY, 0);
            gralloc_extra_perform(src_buffer->handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
        }

        hnd = guiExtAuxFindItem(bq, dst_buffer);

        if (NULL != hnd) {
            hnd->mSrcBuffer = src_buffer;
        }

        Mutex::Autolock l(bq->mMutex);

        /* set SRC config */
        {
            int src_format = 0;
            int src_plane_num;
            int src_y_stride = 0;
            int src_uv_stride = 0;
            int src_size_luma = 0;
            int src_size_chroma = 0;
            int src_no_pq = 0;

            if ((src_info.width  <= 0) ||
                (src_info.height <= 0) ||
                (src_info.stride <= 0)) {
                MTK_LOGE("Invalid buffer width %d, height %d, or stride %d", src_info.width, src_info.height, src_info.stride);
                return AUX_CONVERSION_INVALIDE_DIMENSION;
            }

            gralloc_extra_ion_sf_info_t sf_info;
            gralloc_extra_query(src_buffer->handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

            src_format = src_info.format;

            if (
                (src_info.format == HAL_PIXEL_FORMAT_YUV_PRIVATE) ||
                (src_info.format == HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT) ||
                (src_info.format == HAL_PIXEL_FORMAT_YCbCr_420_888) ||
                0) {
                src_format = PrivateFormat2HALFormat(src_info.status);

                if (src_format == -1) {
                    MTK_LOGE("unexpected format for variable format: 0x%x, status: 0x%x",
                             src_info.format,
                             (int)(src_info.status & GRALLOC_EXTRA_MASK_CM));
                }
            }

#define VIDEOBUFFER_ISVALID(v) (!!((v)&0x80000000))
#define VIDEOBUFFER_YALIGN(v) ({int t=((v)&0x7FFFFFFF)>>25;t==0?1:(t<<1);})
#define VIDEOBUFFER_CALIGN(v) ({int t=((v)&0x01FFFFFF)>>19;t==0?1:(t<<1);})
#define VIDEOBUFFER_HALIGN(v) ({int t=((v)&0x0007FFFF)>>13;t==0?1:(t<<1);})

#ifndef DP_COLOR_420_BLKP_UFO_AUO
#define DP_COLOR_420_BLKP_UFO_AUO DP_COLOR_420_BLKP_UFO
#endif
#ifndef DP_COLOR_420_BLKP_UFO_10_H_JUMP
#define DP_COLOR_420_BLKP_UFO_10_H_JUMP DP_COLOR_420_BLKP_UFO_10_H
#endif
#ifndef DP_COLOR_420_BLKP_UFO_10_V_JUMP
#define DP_COLOR_420_BLKP_UFO_10_V_JUMP DP_COLOR_420_BLKP_UFO_10_V
#endif
#ifndef DP_COLOR_420_BLKP_10_H_JUMP
#define DP_COLOR_420_BLKP_10_H_JUMP DP_COLOR_420_BLKP_10_H
#endif
#ifndef DP_COLOR_420_BLKP_10_V_JUMP
#define DP_COLOR_420_BLKP_10_V_JUMP DP_COLOR_420_BLKP_10_V
#endif

            //MTK_LOGE("debug format 0x%x, 0x%x wh, %dx%d", src_info.format, src_format, src_info.width, src_info.height);
            //MTK_LOGE("debug swh, %dx%d, 0x%08x a:v%d h%d y%d c%d",
            //    src_info.stride, src_info.vertical_stride,
            //    src_info.videobuffer_status,
            //    VIDEOBUFFER_ISVALID(src_info.videobuffer_status),
            //    VIDEOBUFFER_HALIGN(src_info.videobuffer_status),
            //    VIDEOBUFFER_YALIGN(src_info.videobuffer_status),
            //    VIDEOBUFFER_CALIGN(src_info.videobuffer_status)
            //    );

            switch (src_format) {
                case HAL_PIXEL_FORMAT_I420:
                    src_width     = src_info.width;

                    if (VIDEOBUFFER_ISVALID(src_info.videobuffer_status)) {
                        src_height    = ALIGN(src_info.height, VIDEOBUFFER_HALIGN(src_info.videobuffer_status));
                        src_y_stride  = ALIGN(src_width, VIDEOBUFFER_YALIGN(src_info.videobuffer_status));
                        src_uv_stride = ALIGN(src_y_stride / 2, VIDEOBUFFER_CALIGN(src_info.videobuffer_status));
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.height_align = VIDEOBUFFER_HALIGN(src_info.videobuffer_status);
                        src_image_info.width_align = VIDEOBUFFER_YALIGN(src_info.videobuffer_status);
                        src_image_info.uv_align = VIDEOBUFFER_CALIGN(src_info.videobuffer_status);
                        src_image_info.is_omx_align = true;
#endif
                    } else {
                        src_height    = src_info.height;
                        src_y_stride  = src_info.stride;
                        src_uv_stride = src_y_stride / 2;
                    }

                    src_plane_num = 3;
                    src_size_luma   = src_height * src_y_stride;
                    src_size_chroma = src_height * src_uv_stride / 2;
                    src_offset[0]   = src_size_luma;
                    src_offset[1]   = src_size_luma + src_size_chroma;
                    src_size[0]     = src_size_luma;
                    src_size[1]     = src_size_chroma;
                    src_size[2]     = src_size_chroma;
                    dp_in_fmt       = DP_COLOR_I420;
                    break;

                case HAL_PIXEL_FORMAT_YV12:
                    src_width     = src_info.width;

                    if (VIDEOBUFFER_ISVALID(src_info.videobuffer_status)) {
                        src_height    = ALIGN(src_info.height, VIDEOBUFFER_HALIGN(src_info.videobuffer_status));
                        src_y_stride  = ALIGN(src_width, VIDEOBUFFER_YALIGN(src_info.videobuffer_status));
                        src_uv_stride = ALIGN(src_y_stride / 2, VIDEOBUFFER_CALIGN(src_info.videobuffer_status));
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.height_align = VIDEOBUFFER_HALIGN(src_info.videobuffer_status);
                        src_image_info.width_align = VIDEOBUFFER_YALIGN(src_info.videobuffer_status);
                        src_image_info.uv_align = VIDEOBUFFER_CALIGN(src_info.videobuffer_status);
                        src_image_info.is_omx_align = true;
#endif
                    } else {
                        src_height    = src_info.height;
                        src_y_stride  = src_info.stride;
                        src_uv_stride = ALIGN(src_y_stride / 2, 16);
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.uv_align = 16;
#endif
                    }

                    src_plane_num = 3;
                    src_size_luma   = src_height * src_y_stride;
                    src_size_chroma = src_height * src_uv_stride / 2;
                    src_offset[0]   = src_size_luma;
                    src_offset[1]   = src_size_luma + src_size_chroma;
                    src_size[0]     = src_size_luma;
                    src_size[1]     = src_size_chroma;
                    src_size[2]     = src_size_chroma;
                    dp_in_fmt       = DP_COLOR_YV12;
                    break;

                case HAL_PIXEL_FORMAT_NV12:
                    src_width     = src_info.width;
                    if (VIDEOBUFFER_ISVALID(src_info.videobuffer_status)) {
                        src_height = ALIGN(src_info.height, VIDEOBUFFER_HALIGN(src_info.videobuffer_status));
                        src_y_stride  = ALIGN(src_width, VIDEOBUFFER_YALIGN(src_info.videobuffer_status));
                        src_uv_stride = ALIGN(src_y_stride, VIDEOBUFFER_CALIGN(src_info.videobuffer_status));
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.height_align = VIDEOBUFFER_HALIGN(src_info.videobuffer_status);
                        src_image_info.width_align = VIDEOBUFFER_YALIGN(src_info.videobuffer_status);
                        src_image_info.uv_align = VIDEOBUFFER_CALIGN(src_info.videobuffer_status);
                        src_image_info.is_omx_align = true;
#endif
                    } else {
                        src_height = src_info.height;
                        src_y_stride  = ALIGN(src_info.stride, 16);
                        src_uv_stride = ALIGN(src_y_stride, 8);
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = 16;
                        src_image_info.uv_align = 8;
#endif
                    }

                    src_plane_num = 2;
                    src_size_luma   = src_height * src_y_stride;
                    src_size_chroma = src_height * src_uv_stride;
                    src_offset[0]   = src_size_luma;
                    src_size[0]     = src_size_luma;
                    src_size[1]     = src_size_chroma;
                    dp_in_fmt       = DP_COLOR_NV12;
                    break;

                case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H:
                case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H_JUMP:
                    if (VIDEOBUFFER_ISVALID(src_info.videobuffer_status)) {
                        src_width  = ALIGN(src_info.width, VIDEOBUFFER_YALIGN(src_info.videobuffer_status));
                        src_height = ALIGN(src_info.height, VIDEOBUFFER_HALIGN(src_info.videobuffer_status));
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = VIDEOBUFFER_YALIGN(src_info.videobuffer_status);
                        src_image_info.height_align = VIDEOBUFFER_HALIGN(src_info.videobuffer_status);
                        src_image_info.is_omx_align = true;
#endif
                    } else {
                        src_width  = ALIGN(src_info.width, 16);
                        src_height = ALIGN(src_info.height, 32);
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = 16;
                        src_image_info.height_align = 32;
#endif
                    }
                    src_y_stride  = src_width * 40;
                    src_uv_stride = src_y_stride / 2;

                    src_plane_num = 2;
                    // total bytes for 10 bit format
                    src_size_luma   = ALIGN(((src_width * src_height) * 5) >> 2, 512);
                    src_offset[0]   = src_size_luma;
                    src_size[0]     = src_size_luma;
                    src_size[1]     = src_size_luma >> 1;
                    src_size[2]     = 0;

                    dp_in_fmt       = (src_format == HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H) ? DP_COLOR_420_BLKP_10_H : DP_COLOR_420_BLKP_10_H_JUMP;
                    break;

                case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V:
                case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V_JUMP:
                    if (VIDEOBUFFER_ISVALID(src_info.videobuffer_status)) {
                        src_width  = ALIGN(src_info.width, VIDEOBUFFER_YALIGN(src_info.videobuffer_status));
                        src_height = ALIGN(src_info.height, VIDEOBUFFER_HALIGN(src_info.videobuffer_status));
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = VIDEOBUFFER_YALIGN(src_info.videobuffer_status);
                        src_image_info.height_align = VIDEOBUFFER_HALIGN(src_info.videobuffer_status);
                        src_image_info.is_omx_align = true;
#endif
                    } else {
                        src_width  = ALIGN(src_info.width, 16);
                        src_height = ALIGN(src_info.height, 32);
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = 16;
                        src_image_info.height_align = 32;
#endif
                    }
                    src_y_stride  = src_width * 40;
                    src_uv_stride = src_y_stride / 2;

                    src_plane_num = 2;
                    // total bytes for 10 bit format
                    src_size_luma   = ALIGN(((src_width * src_height) * 5) >> 2, 512);
                    src_offset[0]   = src_size_luma;
                    src_size[0]     = src_size_luma;
                    src_size[1]     = src_size_luma >> 1;
                    src_size[2]     = 0;
                    dp_in_fmt       = (src_format == HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V) ? DP_COLOR_420_BLKP_10_V : DP_COLOR_420_BLKP_10_V_JUMP;
                    break;

                case HAL_PIXEL_FORMAT_NV12_BLK:
                    if (VIDEOBUFFER_ISVALID(src_info.videobuffer_status)) {
                        src_width  = ALIGN(src_info.width, VIDEOBUFFER_YALIGN(src_info.videobuffer_status));
                        src_height = ALIGN(src_info.height, VIDEOBUFFER_HALIGN(src_info.videobuffer_status));
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = VIDEOBUFFER_YALIGN(src_info.videobuffer_status);
                        src_image_info.height_align = VIDEOBUFFER_HALIGN(src_info.videobuffer_status);
                        src_image_info.is_omx_align = true;
#endif
                    } else {
                        src_width  = ALIGN(src_info.width, 16);
                        src_height = ALIGN(src_info.height, 32);
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = 16;
                        src_image_info.height_align = 32;
#endif
                    }
                    src_y_stride  = src_width * 32;
                    src_uv_stride = src_y_stride / 2;

                    src_plane_num = 2;
                    src_size_luma   = src_width * src_height;
                    src_size_chroma = src_width * src_height / 2;
                    src_offset[0]   = src_size_luma;
                    src_size[0]     = src_size_luma;
                    src_size[1]     = src_size_chroma;
                    dp_in_fmt       = DP_COLOR_420_BLKP;
                    break;

                case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
                    src_width     = ALIGN(src_info.width, 16);
                    src_height    = ALIGN(src_info.height, 32);
#ifdef MTK_GPUD_SUPPORT
                    src_image_info.width_align = 16;
                    src_image_info.height_align = 32;
#endif
                    src_y_stride  = src_width * 32;
                    src_uv_stride = src_y_stride / 2;

                    src_plane_num = 2;
                    src_size_luma   = src_width * src_height;
                    src_size_chroma = src_width * src_height / 2;
                    src_offset[0]   = src_size_luma;
                    src_size[0]     = src_size_luma;
                    src_size[1]     = src_size_chroma;
                    dp_in_fmt       = DP_COLOR_420_BLKI;
                    break;

                case HAL_PIXEL_FORMAT_UFO:
                case HAL_PIXEL_FORMAT_UFO_AUO:
                    if (VIDEOBUFFER_ISVALID(src_info.videobuffer_status)) {
                        src_width   = ALIGN(src_info.width, VIDEOBUFFER_YALIGN(src_info.videobuffer_status));
                        src_height  = ALIGN(src_info.height, VIDEOBUFFER_HALIGN(src_info.videobuffer_status));
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = VIDEOBUFFER_YALIGN(src_info.videobuffer_status);
                        src_image_info.height_align = VIDEOBUFFER_HALIGN(src_info.videobuffer_status);
                        src_image_info.is_omx_align = true;
#endif
                    } else {
                        src_width   = src_info.stride;
                        src_height  = src_info.vertical_stride;
                    }
                    src_y_stride  = src_width * 32;
                    src_uv_stride = src_y_stride / 2;

                    src_plane_num = 2;
                    {
                        int pic_size_y_bs = ALIGN(src_width * src_height, 512);

                        src_offset[0] = pic_size_y_bs;

                        src_size[0] = pic_size_y_bs;;
                        src_size[1] = pic_size_y_bs;
                        src_size[2] = 0;
                    }

                    dp_in_fmt       = (src_format == HAL_PIXEL_FORMAT_UFO) ? DP_COLOR_420_BLKP_UFO : DP_COLOR_420_BLKP_UFO_AUO;
                    break;

                case HAL_PIXEL_FORMAT_UFO_10BIT_H:
                case HAL_PIXEL_FORMAT_UFO_10BIT_H_JUMP:
                    if (VIDEOBUFFER_ISVALID(src_info.videobuffer_status)) {
                        src_width   = ALIGN(src_info.width, VIDEOBUFFER_YALIGN(src_info.videobuffer_status));
                        src_height  = ALIGN(src_info.height, VIDEOBUFFER_HALIGN(src_info.videobuffer_status));
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = VIDEOBUFFER_YALIGN(src_info.videobuffer_status);
                        src_image_info.height_align = VIDEOBUFFER_HALIGN(src_info.videobuffer_status);
                        src_image_info.is_omx_align = true;
#endif
                    } else {
                        src_width   = src_info.stride;
                        src_height  = src_info.vertical_stride;
                    }
                    src_y_stride  = src_width * 40;
                    src_uv_stride = src_y_stride / 2;

                    src_plane_num = 2;
                    src_size_luma   = ((src_width * src_height) * 5) >> 2; //total bytes for 10 bit format
                    src_offset[0] = src_size_luma;
                    src_size[0] = src_size_luma;
                    src_size[1] = src_size_luma >> 1;
                    src_size[2] = 0;

                    dp_in_fmt       = (src_format == HAL_PIXEL_FORMAT_UFO_10BIT_H) ? DP_COLOR_420_BLKP_UFO_10_H : DP_COLOR_420_BLKP_UFO_10_H_JUMP;
                    break;

                case HAL_PIXEL_FORMAT_UFO_10BIT_V:
                case HAL_PIXEL_FORMAT_UFO_10BIT_V_JUMP:
                    if (VIDEOBUFFER_ISVALID(src_info.videobuffer_status)) {
                        src_width   = ALIGN(src_info.width, VIDEOBUFFER_YALIGN(src_info.videobuffer_status));
                        src_height  = ALIGN(src_info.height, VIDEOBUFFER_HALIGN(src_info.videobuffer_status));
#ifdef MTK_GPUD_SUPPORT
                        src_image_info.width_align = VIDEOBUFFER_YALIGN(src_info.videobuffer_status);
                        src_image_info.height_align = VIDEOBUFFER_HALIGN(src_info.videobuffer_status);
                        src_image_info.is_omx_align = true;
#endif
                    } else {
                        src_width   = src_info.stride;
                        src_height  = src_info.vertical_stride;
                    }
                    src_y_stride  = src_width * 40;
                    src_uv_stride = src_y_stride / 2;

                    src_plane_num = 2;
                    src_size_luma   = ((src_width * src_height) * 5) >> 2; //total bytes for 10 bit format
                    src_offset[0] = src_size_luma;
                    src_size[0] = src_size_luma;
                    src_size[1] = src_size_luma >> 1;
                    src_size[2] = 0;

                    dp_in_fmt       = (src_format == HAL_PIXEL_FORMAT_UFO_10BIT_V) ? DP_COLOR_420_BLKP_UFO_10_V : DP_COLOR_420_BLKP_UFO_10_V_JUMP;
                    break;

                case HAL_PIXEL_FORMAT_YCbCr_422_I:
                    src_width     = src_info.width;
                    src_height    = src_info.height;
                    src_y_stride    = src_info.width * 2;
                    src_uv_stride   = 0;

                    src_plane_num = 1;
                    src_size_luma   = src_info.width * src_info.height * 2;
                    src_size[0]     = src_size_luma;
                    dp_in_fmt       = DP_COLOR_YUYV;
                    src_no_pq = 1;

                    /* pre-rotation */
                    gralloc_extra_query(src_buffer->handle, GRALLOC_EXTRA_GET_ORIENTATION, &prot);
                    if ((prot & HAL_TRANSFORM_ROT_90) == HAL_TRANSFORM_ROT_90 ||
                        (prot & HAL_TRANSFORM_ROT_270) == HAL_TRANSFORM_ROT_270) {
                        int hStride2;
                        gralloc_extra_query(src_buffer->handle, GRALLOC_EXTRA_GET_BYTE_2ND_STRIDE, &hStride2);

                        //MTK_LOGE("Aux debug, --90,270 prot = 0x%x ,hStride2 = %d", prot ,hStride2);
                        src_y_stride    = hStride2 * 2;
                        src_width     = src_info.height;
                        src_height    = src_info.width;
                        src_size_luma = hStride2 * src_height * 2;
                        src_size[0]     = src_size_luma;
                    } else {
                        //MTK_LOGE("Aux debug, --other, prot = 0x%x ,hStride2 = %d", prot ,hStride2);
                        src_width     = src_info.width;
                        src_height    = src_info.height;
                    }
                    break;

                case HAL_PIXEL_FORMAT_RGBX_8888:
                case HAL_PIXEL_FORMAT_RGBA_8888:
                    src_width     = src_info.width;
                    src_height    = src_info.height;
                    src_y_stride  = src_info.stride * 4;
                    src_plane_num = 1;
                    src_size[0]     = src_y_stride * src_height;
                    src_size[1]     = 0;
                    src_size[2]     = 0;

                    dp_in_fmt     = DP_COLOR_RGBX8888;
                    break;

                case HAL_PIXEL_FORMAT_RGB_888:
                    src_width     = src_info.width;
                    src_height    = src_info.height;
                    src_y_stride  = src_info.stride * 3;
                    src_plane_num = 1;
                    src_size[0]     = src_y_stride * src_height;
                    src_size[1]     = 0;
                    src_size[2]     = 0;

                    dp_in_fmt     = DP_COLOR_RGB888;
                    break;

                case HAL_PIXEL_FORMAT_RGB_565:
                    src_width     = src_info.width;
                    src_height    = src_info.height;
                    src_y_stride  = src_info.stride * 2;
                    src_plane_num = 1;
                    src_size[0]     = src_y_stride * src_height;
                    src_size[1]     = 0;
                    src_size[2]     = 0;

                    dp_in_fmt     = DP_COLOR_RGB565;
                    break;

                default:
                    MTK_LOGE("src buffer format not support 0x%x, 0x%x", src_info.format, src_format);
                    return AUX_CONVERSION_SRC_FORMAT_NOT_SUPPORT;
            }

            DpPqParam dppq_param;
#ifdef MTK_PQ_VIDEO_WHITELIST_SUPPORT
            if (pq_scltm_info.scltmEnable) {
                dppq_param.enable = true;
            } else {
                dppq_param.enable = PQ_ULTRARES_EN | PQ_COLOR_EN | PQ_DYN_CONTRAST_EN;
            }
#else
            dppq_param.enable = true;
#endif
            dppq_param.scenario = MEDIA_VIDEO;
            dppq_param.u.video.userScenario = INFO_GPU;
            dppq_param.u.video.id = src_info.pool_id;
            bool dp_flush_cache = false;

            /* For UI PQ, we are using the number 0xFFFFFF00 */
            if (guiExtAuxIsUIPQFormat(src_format)) {
                // if src buffer is not from cpu, we can skip flush
                if ((src_info.usage & GRALLOC_USAGE_SW_WRITE_MASK) &&
                    (sf_info.status & GRALLOC_EXTRA_MASK_TYPE) == GRALLOC_EXTRA_BIT_TYPE_VIDEO &&
                    (sf_info.status & GRALLOC_EXTRA_MASK_FLUSH) == GRALLOC_EXTRA_BIT_FLUSH) {
                    dppq_param.u.video.id = 0xFFFFFF00;
                    dp_flush_cache = true;
                }
            }

            dppq_param.u.video.timeStamp = src_info.timestamp;
            dppq_param.u.video.grallocExtraHandle = src_buffer->handle;
            dppq_param.u.video.isHDR2SDR = 0;
            dppq_param.u.video.paramTable = pq_scltm_info.scltmTableIndex;

#ifdef MTK_GAMEPQ_SUPPORT
            /* PQ_MIRA feature */
            ge_pq_mira_vision_info_t pq_mira_vision_info;
            gralloc_extra_query(src_buffer->handle, GRALLOC_EXTRA_GET_PQ_MIRA_VISION_INFO, (void *)&pq_mira_vision_info);
            if (0 < pq_mira_vision_info.inc_count)
            {
                //MTK_LOGE("[PQ_MIRA] set dppq_param.u.video.videoScenario = INFO_GAME");
                dppq_param.u.video.videoScenario = INFO_GAME;
                dppq_param.u.video.userScenario = INFO_GPU;
            }
#endif

            /* No need to do PQ for video screen shot. */
            if (GuiExtAuxIsSupportFormat(src_buffer) &&
                ((src_info.status2 & GRALLOC_EXTRA_MASK2_VIDEO_PQ) == GRALLOC_EXTRA_BIT2_VIDEO_PQ_OFF)) {
                src_no_pq = 1;
            }
            if (!guiExtIsSurfaceFlinger()) {
                // if source is not surfaceflinger, PQ is no need
                src_no_pq = 1;
            }

#if SUPPORT_MM_BINDER
            if (guiExtIsSurfaceFlinger()) {
#endif
            ret = bltStream2.createJob(job, dp_fence);
            if (ret != 0 || dp_fence < 0) {
                MTK_LOGE("bltStream2 createJob fail %d, dp_fence %d", ret, dp_fence);
                return AUX_CONVERSION_CREATE_JOB_FAIL;
            }
            bltStream2.setConfigBegin(job, pq_scltm_info.scltmPosition, pq_scltm_info.scltmOrientation);

            if (!src_no_pq) {
                bltStream2.setPQParameter(0, dppq_param);
            }

            if (src_info.ion_fd >= 0) {
                bltStream2.setSrcBuffer(src_info.ion_fd, src_size, src_plane_num);
            } else {
#if GPUD_GRALLOC_VERSION_MAJOR >= 2
                lockret = AuxBufferMapper::getInstance().lockBuffer(src_buffer, &src_yp);
#else
                lockret = MTKGralloc::getInstance()->lockBuffer(src_buffer, MTKGralloc::getInstance()->getUsage(), &src_yp);
#endif

                if (0 != lockret) {
                    MTK_LOGE("lock src buffer fail");
                    return AUX_CONVERSION_LOCK_SRC_BUFFER_FAIL;
                }

                uintptr_t *src_addr[3] = {0, 0, 0};
                src_addr[0] = (uintptr_t *)src_yp;
                src_addr[1] = src_addr[0] + src_offset[0];
                src_addr[2] = src_addr[0] + src_offset[1];
                bltStream2.setSrcBuffer((void **)src_addr, src_size, src_plane_num);
            }
#if SUPPORT_MM_BINDER
            } else {
                if (src_info.ion_fd >= 0) {
                    srcHdl = native_handle_create(1, 0);
                    srcHdl->data[0] = src_info.ion_fd;
                }
            }
#endif

            DpRect src_roi;
            src_roi.x = 0;
            src_roi.y = 0;
            if (crop_width > 1 && crop_height > 1) {
                src_roi.w = crop_width;
                src_roi.h = crop_height;
                dst_roi.w = crop_width;
                dst_roi.h = crop_height;
            } else {
                src_roi.w = src_info.width;
                src_roi.h = src_info.height;
                dst_roi.w = dst_info.width;
                dst_roi.h = dst_info.height;
            }

            if ((prot & HAL_TRANSFORM_ROT_90) == HAL_TRANSFORM_ROT_90 ||
                (prot & HAL_TRANSFORM_ROT_270) == HAL_TRANSFORM_ROT_270) {
                int temp = src_roi.w;
                src_roi.w = src_roi.h;
                src_roi.h = temp;
            }

            DP_PROFILE_ENUM src_dp_profile;
            guiExtAuxGetYUVColorSpace(&src_info, &src_dp_profile, true);

#if SUPPORT_MM_BINDER
            if (guiExtIsSurfaceFlinger()) {
#endif
            bltStream2.setSrcConfig(src_width, src_height, src_y_stride, src_uv_stride,
                                               dp_in_fmt, src_dp_profile, eInterlace_None, DP_SECURE_NONE, dp_flush_cache);
            bltStream2.setSrcCrop(0, src_roi);
#if SUPPORT_MM_BINDER
            }
#endif

#ifdef MTK_GPUD_SUPPORT
            gpud_dppq_param.enable = dppq_param.enable;
            gpud_dppq_param.scenario = dppq_param.scenario;
            gpud_dppq_param.video_paramTable = dppq_param.u.video.paramTable;
            gpud_dppq_param.video_id = dppq_param.u.video.id;
            gpud_dppq_param.video_timeStamp = dppq_param.u.video.timeStamp;
            gpud_dppq_param.video_isHDR2SDR = dppq_param.u.video.isHDR2SDR;

            // scltm info
            gpud_dppq_param.scltmEnable = pq_scltm_info.scltmEnable;
            gpud_dppq_param.scltmPosition = pq_scltm_info.scltmPosition;
            gpud_dppq_param.scltmOrientation = pq_scltm_info.scltmOrientation;
            gpud_dppq_param.scltmTableIndex = pq_scltm_info.scltmTableIndex;

            src_image_info.specifier = GPUD_DUMP_SPECIFIER_AUXIMAGE_READ;
            src_image_info.ion_fd = src_info.ion_fd;
            src_image_info.format = src_format;
            src_image_info.DPFormat = dp_in_fmt;
            src_image_info.width = src_width;
            src_image_info.height = src_height;
            src_image_info.stride = src_info.stride;
            src_image_info.y_stride = src_y_stride;
            src_image_info.uv_stride = src_uv_stride;
            src_image_info.size = src_size[0] + src_size[1] + src_size[2];
            src_image_info.yuv_info = src_dp_profile;
            src_image_info.yuv_info_orig = src_info.status & GRALLOC_EXTRA_MASK_YUV_COLORSPACE;
#endif

#if SUPPORT_MM_BINDER
            hwMDPParamAUX.src_width = src_width;
            hwMDPParamAUX.src_height = src_height;
            hwMDPParamAUX.src_yPitch = src_y_stride;
            hwMDPParamAUX.src_uvPitch = src_uv_stride;
            hwMDPParamAUX.src_format = dp_in_fmt;
            //hwMDPParamAUX.src_compressed = compressed;
            hwMDPParamAUX.src_profile = DpToMmsEnum(src_dp_profile);
            hwMDPParamAUX.src_rect.x = src_roi.x;
            hwMDPParamAUX.src_rect.y = src_roi.y;
            hwMDPParamAUX.src_rect.w = src_roi.w;
            hwMDPParamAUX.src_rect.h = src_roi.h;
            hwMDPParamAUX.inputHandle = srcHdl;
            hwMDPParamAUX.src_sizeList[0] = src_size[0];
            hwMDPParamAUX.src_sizeList[1] = src_size[1];
            hwMDPParamAUX.src_sizeList[2] = src_size[2];
            hwMDPParamAUX.src_planeNumber = src_plane_num;
#endif
        }

        /* set DST config */
        {
            int dst_plane_num;
            int dst_stride;
            int dst_pitch_uv;

            switch (dst_info.format) {
                case HAL_PIXEL_FORMAT_YV12:
                    dst_plane_num = 3;
                    dp_out_fmt = DP_COLOR_YV12;
                    dst_stride =  dst_info.stride;
                    dst_pitch_uv = ALIGN((dst_stride / 2), 16);
#ifdef MTK_GPUD_SUPPORT
                    dst_image_info.uv_align = 16;
#endif
                    dst_size[0] = dst_stride * dst_info.height;
                    dst_size[1] = dst_pitch_uv * (dst_info.height / 2);
                    dst_size[2] = dst_pitch_uv * (dst_info.height / 2);
                    break;

                case HAL_PIXEL_FORMAT_YCrCb_420_SP:
                    dst_plane_num = 2;
                    dp_out_fmt = DP_COLOR_NV21;
                    dst_stride =  dst_info.stride;
                    dst_pitch_uv = dst_stride;
                    dst_size[0] = dst_stride * dst_info.height;
                    dst_size[1] = dst_pitch_uv * (dst_info.height / 2);
                    break;

                case HAL_PIXEL_FORMAT_RGBA_8888:
                    dst_plane_num = 1;
                    dp_out_fmt = DP_COLOR_RGBX8888;
                    dst_stride = dst_info.stride * 4;
                    dst_pitch_uv = 0;
                    dst_size[0] = dst_stride * dst_info.height;
                    break;

                case HAL_PIXEL_FORMAT_RGB_888:
                    dst_plane_num = 1;
                    dp_out_fmt = DP_COLOR_RGB888;
                    dst_stride = ALIGN((dst_info.stride * 3), 64);
                    dst_pitch_uv = 0;
                    dst_size[0] = dst_stride * dst_info.height;
                    break;

                default:
                    if (src_info.ion_fd < 0) {
#if GPUD_GRALLOC_VERSION_MAJOR >= 2
                        AuxBufferMapper::getInstance().unlockBuffer(src_buffer);
#else
                        MTKGralloc::getInstance()->unlockBuffer(src_buffer);
#endif
                    }
                    if (srcHdl) {
                        native_handle_delete(srcHdl);
                    }
                    MTK_LOGE("Unsupported dst color format %d\n", dst_info.format);
                    return AUX_CONVERSION_DST_FORMAT_NOT_SUPPORT;
            }

            if (dst_info.ion_fd >= 0) {
#if SUPPORT_MM_BINDER
            if (guiExtIsSurfaceFlinger()) {
#endif
                bltStream2.setDstBuffer(0, dst_info.ion_fd, dst_size, dst_plane_num);
#if SUPPORT_MM_BINDER
            } else {
                dstHdl = native_handle_create(1, 0);
                dstHdl->data[0] = dst_info.ion_fd;
            }
#endif
            } else {
                MTK_LOGE("dst is not a ion buffer");
#if GPUD_GRALLOC_VERSION_MAJOR >= 2
                AuxBufferMapper::getInstance().unlockBuffer(src_buffer);
#else
                MTKGralloc::getInstance()->unlockBuffer(src_buffer);
#endif
                if (srcHdl) {
                    native_handle_delete(srcHdl);
                }
                return AUX_CONVERSION_DST_NOT_ION_BUFFER;
            }

            /* Make sure the w and h are even numbers. */
            width_even = (dst_roi.w % 2) ? dst_roi.w - 1 : dst_roi.w;
            height_even = (dst_roi.h  % 2) ? dst_roi.h  - 1 : dst_roi.h ;

            dst_roi.x = 0;
            dst_roi.y = 0;
            dst_roi.w = width_even;
            dst_roi.h = height_even;

            DP_PROFILE_ENUM out_dp_profile;
            guiExtAuxGetYUVColorSpace(&dst_info, &out_dp_profile, false);

#if SUPPORT_MM_BINDER
            if (guiExtIsSurfaceFlinger()) {
#endif
            bltStream2.setDstConfig(0, dst_roi.w, dst_roi.h, dst_stride, dst_pitch_uv,
                                   dp_out_fmt, out_dp_profile, eInterlace_None, &dst_roi, DP_SECURE_NONE, false);

            if (HAL_TRANSFORM_ROT_90 == (prot & HAL_TRANSFORM_ROT_90)) {
                MTK_LOGD("Aux debug, ++90  prot = 0x%x \n", prot);
                if (HAL_TRANSFORM_FLIP_H == (prot & HAL_TRANSFORM_FLIP_H)) {
                    //MTK_LOGD("Aux debug, ++90 ,1, prot = 0x%x \n", prot );
                    bltStream2.setOrientation(0, DpAsyncBlitStream::ROT_90 | DpAsyncBlitStream::FLIP_V);
                } else if (HAL_TRANSFORM_FLIP_V == (prot & HAL_TRANSFORM_FLIP_V)) {
                    //MTK_LOGD("Aux debug, ++90 ,2, prot = 0x%x \n", prot );
                    bltStream2.setOrientation(0, DpAsyncBlitStream::ROT_90 | DpAsyncBlitStream::FLIP_H);
                } else {
                    //MTK_LOGD("Aux debug, ++90 ,3, prot = 0x%x \n", prot );
                    bltStream2.setOrientation(0, DpAsyncBlitStream::ROT_270); ///(DpAsyncBlitStream::ROT_90 | DpAsyncBlitStream::FLIP_V); (DpAsyncBlitStream::ROT_90 | DpAsyncBlitStream::FLIP_H);
                }
            } else if (HAL_TRANSFORM_ROT_270 == (prot & HAL_TRANSFORM_ROT_270)) {
                MTK_LOGD("Aux debug, ++270  prot = 0x%x \n", prot);
                bltStream2.setOrientation(0, DpAsyncBlitStream::ROT_90); ///(DpAsyncBlitStream::ROT_270 | DpAsyncBlitStream::FLIP_V); (DpAsyncBlitStream::ROT_90 | DpAsyncBlitStream::FLIP_H);
            } else if (prot == 0) {
                //MTK_LOGD("Aux debug, ++0,prot = 0x%x \n", prot );
                bltStream2.setOrientation(0, DpAsyncBlitStream::ROT_0);///(DpAsyncBlitStream::ROT_90 | DpAsyncBlitStream::FLIP_V); (DpAsyncBlitStream::ROT_90 | DpAsyncBlitStream::FLIP_H);
            }
#if SUPPORT_MM_BINDER
            } else {
                if (HAL_TRANSFORM_ROT_90 == (prot & HAL_TRANSFORM_ROT_90)) {
                    if (HAL_TRANSFORM_FLIP_H == (prot & HAL_TRANSFORM_FLIP_H)) {
                        hwMDPParamAUX.transform = MMS_ORIENTATION_ENUM::MMS_ROT_90 | MMS_ORIENTATION_ENUM::MMS_FLIP_V;
                    } else if (HAL_TRANSFORM_FLIP_V == (prot & HAL_TRANSFORM_FLIP_V)) {
                        hwMDPParamAUX.transform = MMS_ORIENTATION_ENUM::MMS_ROT_90 | MMS_ORIENTATION_ENUM::MMS_FLIP_H;
                    } else {
                        hwMDPParamAUX.transform = static_cast<uint32_t>(MMS_ORIENTATION_ENUM::MMS_ROT_270);
                    }
                } else if (HAL_TRANSFORM_ROT_270 == (prot & HAL_TRANSFORM_ROT_270)) {
                    hwMDPParamAUX.transform = static_cast<uint32_t>(MMS_ORIENTATION_ENUM::MMS_ROT_90);
                } else if (prot == 0) {
                    hwMDPParamAUX.transform = static_cast<uint32_t>(MMS_ORIENTATION_ENUM::MMS_ROT_0);
                }
            }
#endif

#ifdef MTK_GPUD_SUPPORT
            dst_image_info.specifier = GPUD_DUMP_SPECIFIER_AUXIMAGE_DRAW;
            dst_image_info.ion_fd = dst_info.ion_fd;
            dst_image_info.format = dst_info.format;
            dst_image_info.DPFormat = dp_out_fmt;
            dst_image_info.width = dst_roi.w;
            dst_image_info.height = dst_roi.h;
            dst_image_info.stride = dst_info.stride;
            dst_image_info.y_stride = dst_stride;
            dst_image_info.uv_stride = dst_pitch_uv;
            dst_image_info.size = dst_size[0] + dst_size[1] + dst_size[2];
            dst_image_info.yuv_info = out_dp_profile;
            dst_image_info.yuv_info_orig = dst_info.status & GRALLOC_EXTRA_MASK_YUV_COLORSPACE;
#endif

#if SUPPORT_MM_BINDER
            hwMDPParamAUX.dst_width = dst_roi.w;
            hwMDPParamAUX.dst_height = dst_roi.h;
            hwMDPParamAUX.dst_yPitch = dst_stride;
            hwMDPParamAUX.dst_uvPitch = dst_pitch_uv;
            hwMDPParamAUX.dst_format = dp_out_fmt;
            //hwMDPParamAUX.dst_compressed = 0;
            hwMDPParamAUX.dst_profile = DpToMmsEnum(out_dp_profile);
            hwMDPParamAUX.dst_rect.x = dst_roi.x;
            hwMDPParamAUX.dst_rect.y = dst_roi.y;
            hwMDPParamAUX.dst_rect.w = dst_roi.w;
            hwMDPParamAUX.dst_rect.h = dst_roi.h;
            hwMDPParamAUX.outputHandle = dstHdl;
            hwMDPParamAUX.dst_sizeList[0] = dst_size[0];
            hwMDPParamAUX.dst_sizeList[1] = dst_size[1];
            hwMDPParamAUX.dst_sizeList[2] = dst_size[2];
            hwMDPParamAUX.dst_planeNumber = dst_plane_num;
#endif
        }

        gettimeofday(&target_end, NULL);
        target_end.tv_usec += 1000;
        if (target_end.tv_usec > 1000000) {
            target_end.tv_sec++;
            target_end.tv_usec -= 1000000;
        }

#if SUPPORT_MM_BINDER
        if (guiExtIsSurfaceFlinger()) {
#endif
        bltStream2.setConfigEnd();
#if SUPPORT_MM_BINDER
        } else {
            hwMDPParamAUX.pid = getpid();
            //hwMDPParamAUX.user_id = 0;
            hwMDPParamAUX.end_time.sec = target_end.tv_sec;
            hwMDPParamAUX.end_time.usec = target_end.tv_usec;
        }
#endif

#ifdef MTK_GPUD_SUPPORT
        GPUD_API_ENTRY(AuxDumpAUXImageSrc, src_image_info, src_buffer, gpud_dppq_param);
#endif

#if SUPPORT_MM_BINDER
        if (guiExtIsSurfaceFlinger()) {
#endif
        /* bltStream2 HW do not support odd width/height
         * src_width/src_height are actual width/height and may be odd number.
         * the memory gralloc allocated are all align even number,
         * so we can bypass this check function
         * we only print log for debug
         */
        static bool aux_check_hw_support = false;
        if (!bltStream2.queryHWSupport(src_width, src_height, width_even, height_even,
                pq_scltm_info.scltmOrientation, dp_in_fmt, dp_out_fmt) &&
            !aux_check_hw_support) {
            MTK_LOGD("warning: queryHWSupport failed, src_width %d, src_height %d, width_even %d, height_even %d",
                    src_width, src_height, width_even, height_even);
            aux_check_hw_support = true;
        }
        MTK_ATRACE_BEGIN("bltStream2.invalidate()");
        if (bltStream2.invalidate(&target_end) != DP_STATUS_RETURN_SUCCESS) {
            MTK_LOGE("DpAsyncBlitStream2 invalidate failed");
            err = AUX_CONVERSION_INVALIDATE_FAIL;
        }

        sync_wait(dp_fence, -1);
        close(dp_fence);
        MTK_ATRACE_END();
#if SUPPORT_MM_BINDER
        } else {
            if (bq->mmservice != NULL) {
                auto ret = bq->mmservice->BlitStreamAUX(hwMDPParamAUX);
                if (!ret.isOk() || ret < 0) {
                    MTK_LOGE("mmservice->BlitStreamAUX failed");
                }
            }
            if (srcHdl) {
                native_handle_delete(srcHdl);
            }
            if (dstHdl) {
                native_handle_delete(dstHdl);
            }
        }
#endif


        if (!(src_info.ion_fd >= 0)) {
#if GPUD_GRALLOC_VERSION_MAJOR >= 2
            AuxBufferMapper::getInstance().unlockBuffer(src_buffer);
#else
            MTKGralloc::getInstance()->unlockBuffer(src_buffer);
#endif
        }

#ifdef MTK_GPUD_SUPPORT
        GPUD_API_ENTRY(AuxDumpAUXImageDst, dst_image_info, dst_buffer);
#endif

        return err;
    }

    int GuiExtAuxAcquireBuffer(GuiExtAuxBufferQueueHandle bq, int *bufSlot, int *fence_fd)
    {
        status_t err = 0;

        android_native_buffer_t *pBuffer = NULL;

        err = bq->mMTKQueue.acquireBuffer(bufSlot, &pBuffer, fence_fd);

        if (err != NO_ERROR) {
            MTK_LOGE("acquireBuffer fail(%d)", err);
            return -err;
        }

        GuiExtAuxBufferItemHandle auxitem = &bq->mSlots[*bufSlot];

        if (pBuffer) {
            auxitem->mMTKBuffer = (GPUAUXBuffer *) pBuffer;
            if (auxitem->mMTKBuffer->getNativeBuffer()->format <= 0) {
                MTK_LOGE("The format(0x%x) of buffer(%p) is not supported.", pBuffer->format, pBuffer);
            }
        }
        auxitem->mSrcBuffer = 0;

        return err;
    }

    int GuiExtAuxReleaseBuffer(GuiExtAuxBufferQueueHandle bq, int bufSlot, int fence_fd)
    {
        status_t err = 0;

        if (fence_fd >= 0) {
            close(fence_fd);
            fence_fd = -1;
        }

        err = bq->mMTKQueue.releaseBuffer(bufSlot);

        return err;
    }

    int GuiExtAuxDequeueBuffer(GuiExtAuxBufferQueueHandle bq, int *bufSlot, int *fence_fd)
    {
        ATRACE_CALL();

        int buf = -1;
        const int reqW = bq->width;
        const int reqH = bq->height;
        const int reqF = bq->format;

        status_t err;

        if (reqW == -1 || reqH == -1) {
            MTK_LOGE("please call setSize() beforce dequeueBuffer().");
            return -1;
        }

        android_native_buffer_t *pBuffer = NULL;

        err = bq->mMTKQueue.dequeueBuffer(reqW, reqH, reqF, &buf, &pBuffer, fence_fd);

        if (err == 0) {
            GuiExtAuxBufferItemHandle auxitem = &bq->mSlots[buf];

            auxitem->mMTKBuffer = (GPUAUXBuffer *)pBuffer;
            if (auxitem->mMTKBuffer->getNativeBuffer()->format <= 0) {
                MTK_LOGE("The format(0x%x) of buffer(%p) is not supported.", pBuffer->format, pBuffer);
            }

            *bufSlot = buf;
        } else {
            MTK_LOGE("error %d", err);
        }

        return err;
    }

    int GuiExtAuxQueueBuffer(GuiExtAuxBufferQueueHandle bq, int bufSlot, int fence_fd)
    {
        ATRACE_CALL();

        if (fence_fd >= 0) {
            close(fence_fd);
            fence_fd = -1;
        }

        status_t err = bq->mMTKQueue.queueBuffer(bufSlot);

        return err;
    }

    android_native_buffer_t *GuiExtAuxRequestBuffer(GuiExtAuxBufferQueueHandle bq, int bufSlot)
    {
        return bq->mSlots[bufSlot].mMTKBuffer->getNativeBuffer();
    }

} /* end of extern "C" */
