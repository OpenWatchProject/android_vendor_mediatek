/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "MfbDrv"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <cutils/properties.h>  // For property_get().
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <cutils/atomic.h>

#include "camera_mfb.h"
#include <mtkcam/def/common.h>
#include <mtkcam/drv/def/mfbcommon.h>
#include <mfb_drv.h>
#include <mfbunittest.h>
#include <tpipe_config.h> //temp marked by Ryan
#include <imem_drv.h>

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG
#include "log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#include <cutils/properties.h>
DECLARE_DBG_LOG_VARIABLE(mfb_drv);


// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        BASE_LOG_VRB(fmt, ##arg);
#define LOG_DBG(fmt, arg...)        do { if (g_isMFBLogEnable) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        BASE_LOG_INF(fmt, ##arg);
#define LOG_WRN(fmt, arg...)        BASE_LOG_WRN(fmt, ##arg);
#define LOG_ERR(fmt, arg...)        BASE_LOG_ERR(fmt, ##arg);
#define LOG_AST(cond, fmt, arg...)  BASE_LOG_AST(cond, fmt, ##arg);
MBOOL g_isMFBLogEnable = MFALSE;
unsigned int g_tpipe_no = 0;
unsigned int g_workaround_height = 0;
#if 1
DipIMemDrv *g_MfbImemDrv = NULL;
IMEM_BUF_INFO g_MfbTileBuffer;
IMEM_BUF_INFO Mfb_Imem_Buf_Info;
MUINT8   *g_MfbImem_pLogVir=NULL;
MINT32   g_MfbImem_MemID;
MUINT32  g_MfbImem_Size;
MUINT32  Mfb_Imem_AllocNum = 0;
#endif
ISP_TPIPE_CONFIG_STRUCT pTpipeCfgInfo;
ISP_TPIPE_DESCRIPTOR_STRUCT  tpipeDesc; // for tpipe tmp buffer
#define TPIPE_BLD_SIZE 4800 // Max 25 tiles: 192x25
MINT32 configTPIPE(NSCam::NSIoPipe::MFBConfig* pMfbConfig, SrzCfg* pSrz_cfg);
MINT32 configSRZ5(SrzCfg* pSrz_cfg, SRZSizeInfo *SRZ5Info);
MFBERRCODE MFB_ErrorCheck(NSCam::NSIoPipe::MFBConfig* pMfbConfig);

class MfbDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    MfbDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
        if  (0==mIdx)
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
        }
        else
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms] [last-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000, (float)(i4EndUs-mi4LastUs)/1000);
        }
        mi4LastUs = i4EndUs;

        //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};

MINT32              MfbDrvImp::m_Fd = -1;
volatile MINT32     MfbDrvImp::m_UserCnt = 0;
MFB_DRV_RW_MODE     MfbDrvImp::m_regRWMode = MFB_DRV_R_ONLY;
MUINT32*            MfbDrvImp::m_pMfbHwRegAddr = NULL;
android::Mutex      MfbDrvImp::m_MfbInitMutex;

char                MfbDrvImp::m_UserName[MAX_USER_NUMBER][32] =
{
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
    {"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},{"\0"},
};

MfbDrvImp::MfbDrvImp()
{
    DBG_LOG_CONFIG(drv, mfb_drv);
    LOG_VRB("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    m_pMfbHwRegAddr = NULL;
}

template<>
EngineDrv<MFBConfig>* EngineDrv<MFBConfig>::createInstance()
{
    return MfbDrvImp::getInstance();
}

static MfbDrvImp singleton;

EngineDrv<MFBConfig>* MfbDrvImp::getInstance()
{
    singleton.ctl_tbl = mfb_ctl;

    LOG_DBG("singleton[0x%p].",static_cast<void *>(&singleton));

    return &singleton;
}

MINT32 MFB_Imem_alloc(MUINT32 size,MINT32 *memId,MUINT8 **vAddr,MUINTPTR *pAddr)
{
    if ( NULL == g_MfbImemDrv ) {
        g_MfbImemDrv = DipIMemDrv::createInstance();
        g_MfbImemDrv->init();
    }
    //
    Mfb_Imem_Buf_Info.size = size;
    Mfb_Imem_Buf_Info.useNoncache = 1;
    if (g_MfbImemDrv->allocVirtBuf(&Mfb_Imem_Buf_Info) != 0)
        LOG_ERR("Imem Allocate Virtual Buffer Fail!");
    *memId = Mfb_Imem_Buf_Info.memID;
    *vAddr = (MUINT8 *)Mfb_Imem_Buf_Info.virtAddr;
    //
    if (g_MfbImemDrv->mapPhyAddr(&Mfb_Imem_Buf_Info) != 0)
        LOG_ERR("Imem Map Physical Address Fail!");
    *pAddr = (MUINTPTR)Mfb_Imem_Buf_Info.phyAddr;

    g_MfbTileBuffer.virtAddr=(MUINTPTR)g_MfbImem_pLogVir;
    g_MfbTileBuffer.memID=g_MfbImem_MemID;
    g_MfbTileBuffer.size=g_MfbImem_Size;

    Mfb_Imem_AllocNum ++;

    /*MY_LOGD("Mfb_Imem_AllocNum(%d)\n",Mfb_Imem_AllocNum);*/
    /*MY_LOGD("vAddr(0x%p) pAddr(0x%p) Mfb_Imem_AllocNum(%d)\n",*vAddr,*pAddr,Mfb_Imem_AllocNum);*/

    return 0;
}

MINT32 MFB_Imem_free(MUINT8 *vAddr, MUINTPTR phyAddr,MUINT32 size,MINT32 memId)
{
    Mfb_Imem_Buf_Info.size = size;
    Mfb_Imem_Buf_Info.memID = memId;
    Mfb_Imem_Buf_Info.virtAddr = (MUINTPTR)vAddr;
    Mfb_Imem_Buf_Info.phyAddr = (MUINTPTR)phyAddr;
    //

    if(g_MfbImemDrv)
    {
        if(g_MfbImemDrv->unmapPhyAddr(&Mfb_Imem_Buf_Info))
            LOG_ERR("Imem Unmap Physical Address Fail!");

        if(g_MfbImemDrv->freeVirtBuf(&Mfb_Imem_Buf_Info))
            LOG_ERR("Imem Free Virtual Buffer Fail!");

        if(Mfb_Imem_AllocNum)
        {
            Mfb_Imem_AllocNum--;
            if(Mfb_Imem_AllocNum==0)
            {
                g_MfbImemDrv->uninit();
                g_MfbImemDrv = NULL;
            }
        }
    }
    else
    {
        LOG_ERR("No MFB ImemDrv!");
    }

    /*MY_LOGD("Mfb_Imem_AllocNum(%d)\n",Mfb_Imem_AllocNum);*/

    return 0;
}

void MFB_dumpTPipeCfgInfo(ISP_TPIPE_CONFIG_STRUCT *pTpipeCfgInfo)
{
    LOG_INF("mfb_en: %d\n",                 pTpipeCfgInfo->top.mfb_en);
    LOG_INF("mfbi_en: %d\n",                pTpipeCfgInfo->top.mfbi_en);
    LOG_INF("mfbi_b_en: %d\n",              pTpipeCfgInfo->top.mfbi_b_en);
    LOG_INF("mfb3i_en: %d\n",               pTpipeCfgInfo->top.mfb3i_en);
    LOG_INF("mfb4i_en: %d\n",               pTpipeCfgInfo->top.mfb4i_en);
    LOG_INF("mfbo_en: %d\n",                pTpipeCfgInfo->top.mfbo_en);
    LOG_INF("mfbo_b_en: %d\n",              pTpipeCfgInfo->top.mfbo_b_en);
    LOG_INF("mfb2o_en: %d\n",               pTpipeCfgInfo->top.mfb2o_en);
    LOG_INF("mfb_srz_en: %d\n",             pTpipeCfgInfo->top.mfb_srz_en);
    LOG_INF("bld_mbd_wt_en: %d\n",          pTpipeCfgInfo->mfb.bld_mbd_wt_en);
    LOG_INF("mfbi_stride: %d\n",            pTpipeCfgInfo->mfbi.mfbi_stride);
    LOG_INF("mfbi_v_flip_en: %d\n",         pTpipeCfgInfo->mfbi.mfbi_v_flip_en);
    LOG_INF("mfbi_b_stride: %d\n",          pTpipeCfgInfo->mfbi_b.mfbi_b_stride);
    LOG_INF("mfbi_b_v_flip_en: %d\n",       pTpipeCfgInfo->mfbi_b.mfbi_b_v_flip_en);
    LOG_INF("mfb3i_stride: %d\n",           pTpipeCfgInfo->mfb3i.mfb3i_stride);
    LOG_INF("mfb3i_v_flip_en: %d\n",        pTpipeCfgInfo->mfb3i.mfb3i_v_flip_en);
    LOG_INF("mfb4i_stride: %d\n",           pTpipeCfgInfo->mfb4i.mfb4i_stride);
    LOG_INF("mfb4i_v_flip_en: %d\n",        pTpipeCfgInfo->mfb4i.mfb4i_v_flip_en);
    LOG_INF("mfb4i_xsize: %d\n",            pTpipeCfgInfo->mfb4i.mfb4i_xsize);
    LOG_INF("mfb4i_ysize: %d\n",            pTpipeCfgInfo->mfb4i.mfb4i_ysize);
    LOG_INF("mfbo_stride: %d\n",            pTpipeCfgInfo->mfbo.mfbo_stride);
    LOG_INF("mfbo_xoffset: %d\n",           pTpipeCfgInfo->mfbo.mfbo_xoffset);
    LOG_INF("mfbo_xsize: %d\n",             pTpipeCfgInfo->mfbo.mfbo_xsize);
    LOG_INF("mfbo_yoffset: %d\n",           pTpipeCfgInfo->mfbo.mfbo_yoffset);
    LOG_INF("mfbo_ysize: %d\n",             pTpipeCfgInfo->mfbo.mfbo_ysize);
    LOG_INF("mfbo_b_stride: %d\n",          pTpipeCfgInfo->mfbo_b.mfbo_b_stride);
    LOG_INF("mfb2o_stride: %d\n",           pTpipeCfgInfo->mfb2o.mfb2o_stride);
    LOG_INF("mfb2o_xoffset: %d\n",          pTpipeCfgInfo->mfb2o.mfb2o_xoffset);
    LOG_INF("mfb2o_xsize: %d\n",            pTpipeCfgInfo->mfb2o.mfb2o_xsize);
    LOG_INF("mfb2o_yoffset: %d\n",          pTpipeCfgInfo->mfb2o.mfb2o_yoffset);
    LOG_INF("mfb2o_ysize: %d\n",            pTpipeCfgInfo->mfb2o.mfb2o_ysize);
    LOG_INF("srz_input_crop_width: %d\n",   pTpipeCfgInfo->mfb_srz.srz_input_crop_width);
    LOG_INF("srz_input_crop_height: %d\n",  pTpipeCfgInfo->mfb_srz.srz_input_crop_height);
    LOG_INF("srz_output_width: %d\n",       pTpipeCfgInfo->mfb_srz.srz_output_width);
    LOG_INF("srz_output_height: %d\n",      pTpipeCfgInfo->mfb_srz.srz_output_height);
    LOG_INF("srz_luma_horizontal_integer_offset: %d\n",     pTpipeCfgInfo->mfb_srz.srz_luma_horizontal_integer_offset);
    LOG_INF("srz_luma_horizontal_subpixel_offset: %d\n",    pTpipeCfgInfo->mfb_srz.srz_luma_horizontal_subpixel_offset);
    LOG_INF("srz_luma_vertical_integer_offset: %d\n",       pTpipeCfgInfo->mfb_srz.srz_luma_vertical_integer_offset);
    LOG_INF("srz_luma_vertical_subpixel_offset: %d\n",      pTpipeCfgInfo->mfb_srz.srz_luma_vertical_subpixel_offset);
    LOG_INF("srz_horizontal_coeff_step: %d\n",              pTpipeCfgInfo->mfb_srz.srz_horizontal_coeff_step);
    LOG_INF("srz_vertical_coeff_step: %d\n",                pTpipeCfgInfo->mfb_srz.srz_vertical_coeff_step);
    LOG_INF("crsp_xoffset: %d\n",           pTpipeCfgInfo->mfb_crsp.crsp_xoffset);
    LOG_INF("crsp_yoffset: %d\n",           pTpipeCfgInfo->mfb_crsp.crsp_yoffset);
    LOG_INF("crsp_ystep: %d\n",             pTpipeCfgInfo->mfb_crsp.crsp_ystep);
    LOG_INF("mfb_y_pak_yuv_bit: %d\n",      pTpipeCfgInfo->mfb_y_pak.yuv_bit);
    LOG_INF("mfb_y_pak_yuv_dng: %d\n",      pTpipeCfgInfo->mfb_y_pak.yuv_dng);
    LOG_INF("mfb_c_pak_yuv_bit: %d\n",      pTpipeCfgInfo->mfb_c_pak.yuv_bit);
    LOG_INF("mfb_c_pak_yuv_dng: %d\n",      pTpipeCfgInfo->mfb_c_pak.yuv_dng);
    LOG_INF("mfb_y_unp_yuv_bit: %d\n",      pTpipeCfgInfo->mfb_y_unp.yuv_bit);
    LOG_INF("mfb_y_unp_yuv_dng: %d\n",      pTpipeCfgInfo->mfb_y_unp.yuv_dng);
    LOG_INF("mfb_c_unp_yuv_bit: %d\n",      pTpipeCfgInfo->mfb_c_unp.yuv_bit);
    LOG_INF("mfb_c_unp_yuv_dng: %d\n",      pTpipeCfgInfo->mfb_c_unp.yuv_dng);
    LOG_INF("log_en: %d\n",                 pTpipeCfgInfo->sw.log_en);
    LOG_INF("src_width_mfb: %d\n",          pTpipeCfgInfo->sw.src_width_mfb);
    LOG_INF("src_height_mfb: %d\n",         pTpipeCfgInfo->sw.src_height_mfb);
    LOG_INF("tpipe_irq_mode: %d\n",         pTpipeCfgInfo->sw.tpipe_irq_mode);
    LOG_INF("tpipe_width_mfb: %d\n",        pTpipeCfgInfo->sw.tpipe_width_mfb);
    LOG_INF("tpipe_height_mfb: %d\n",       pTpipeCfgInfo->sw.tpipe_height_mfb);
    LOG_INF("tpipe_sel_mode: 0x%x\n",       pTpipeCfgInfo->sw.tpipe_sel_mode);
}

void MFB_dumpTPipeBuffer(unsigned int *startAddr)
{
    for (int word_num = 0; word_num < 10; word_num++) {
            LOG_INF("[%02d]:0x%08x 0x%08x 0x%08x 0x%08x\n",
                word_num*4,
                *((unsigned int *)(startAddr + 4*word_num)),
                *((unsigned int *)(startAddr + 4*word_num+1)),
                *((unsigned int *)(startAddr + 4*word_num+2)),
                *((unsigned int *)(startAddr + 4*word_num+3)));
    }
    LOG_INF("\n");
}

MINT32 MFB_Check4Align(unsigned int x)
{
    if ((x & 0x3) == 0) {
        return 0;
    } else {
        return 1;
    }
}

MINT32 MFB_Check16Align(unsigned int x)
{
    if ((x & 0xF) == 0) {
        return 0;
    } else {
        return 1;
    }
}

MFBERRCODE MFB_ErrorCheck(NSCam::NSIoPipe::MFBConfig* pMfbConfig)
{
    if (pMfbConfig->Mfb_inbuf_baseFrame != 0x0) {
        if (pMfbConfig->Mfb_inbuf_baseFrame->getBufPA(0) == 0x0) {
            LOG_INF("BaseFrame: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (pMfbConfig->Mfb_inbuf_baseFrame->getBufPA(1) == 0x0) {
            LOG_INF("BaseFrame: Doesn't have PA(1)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (MFB_Check4Align(pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(0)) != 0) {
            LOG_INF("BaseFrame: Stride is not 4 alignment!");
            return MFBERR_BUFFER_PARAM;
        }
    }
    else {
        LOG_INF("No base frame!\n");
        return MFBERR_BUFFER_EXIST;
    }

    if (pMfbConfig->Mfb_inbuf_refFrame != 0x0) {
        if (pMfbConfig->Mfb_inbuf_refFrame->getBufPA(0) == 0x0) {
            LOG_INF("RefFrame: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (pMfbConfig->Mfb_inbuf_refFrame->getBufPA(1) == 0x0) {
            LOG_INF("RefFrame: Doesn't have PA(1)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (MFB_Check16Align(pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0)) != 0) {
            LOG_INF("RefFrame: Stride is not 16 alignment!");
            return MFBERR_BUFFER_PARAM;
        }
    }
    else {
        LOG_INF("No ref Frame!\n");
        return MFBERR_BUFFER_EXIST;
    }

    if (pMfbConfig->Mfb_inbuf_blendWeight != 0x0) {
        if (pMfbConfig->Mfb_inbuf_blendWeight->getBufPA(0) == 0x0) {
            LOG_INF("WeightTable: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
    }
    else {
        LOG_INF("No blend weight table\n");
        //bld mode = 0 => no need weight table input
        if (pMfbConfig->Mfb_bldmode != 0) {
            return MFBERR_BUFFER_EXIST;
        }
    }

    if (pMfbConfig->Mfb_inbuf_confidenceMap != 0x0) {
        if (pMfbConfig->Mfb_inbuf_confidenceMap->getBufPA(0) == 0x0) {
            LOG_INF("ConfidenceMap: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
    }
    else {
        LOG_INF("No confidence map\n");
        //srz en = 1 => need confidence map input
        if (pMfbConfig->Mfb_srzEn != 0) {
            return MFBERR_BUFFER_EXIST;
        }
    }

    if (pMfbConfig->Mfb_inbuf_mv != 0x0) {
        if (pMfbConfig->Mfb_inbuf_mv->getBufPA(0) == 0x0) {
            LOG_INF("Motion: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (MFB_Check16Align(pMfbConfig->Mfb_inbuf_mv->getBufStridesInBytes(0)) != 0) {
            LOG_INF("Motion: Stride is not 16 alignment!");
            return MFBERR_BUFFER_PARAM;
        }
    }
    else {
        LOG_INF("No motion vector\n");
        return MFBERR_BUFFER_EXIST;
    }

    if (pMfbConfig->Mfb_outbuf_frame != 0x0) {
        if (pMfbConfig->Mfb_outbuf_frame->getBufPA(0) == 0x0) {
            LOG_INF("OutputFrame: Doesn't have PA(0)!\n");
            return MFBERR_BUFFER_EXIST;
        }
        if (pMfbConfig->Mfb_outbuf_frame->getBufPA(1) == 0x0) {
            LOG_INF("OutputFrame: Doesn't have PA(1)!\n");
            return MFBERR_BUFFER_EXIST;
        }
    }
    else {
        LOG_INF("No output frame!\n");
        return MFBERR_BUFFER_EXIST;
    }

    if (pMfbConfig->Mfb_outbuf_blendWeight != 0x0) {
        if (pMfbConfig->Mfb_outbuf_blendWeight->getBufPA(0) == 0x0) {
            LOG_INF("OutputWeightTable: Doesn't have PA(0)!\n");
            return MFBERR_BUFFER_EXIST;
        }
    }
    else {
        LOG_INF("No output blend weight table\n");
        return MFBERR_BUFFER_EXIST;
    }

    return MFBERR_NOERROR;
}

MBOOL MfbDrvImp::init(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 resetModule;
    int32_t old_cnt;
    //
    android::Mutex::Autolock lock(m_MfbInitMutex);
    //
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.mfb.debug.enable", value, "0");
    g_isMFBLogEnable = atoi(value);

    if( (userName==NULL) || (strlen(userName)<1) || (strlen(userName) >= MAX_USER_NAME_SIZE))
    {
        LOG_ERR("Plz add userName if you want to use mfb driver\n");
        return MFALSE;
    }
    //
    LOG_INF("+,m_UserCnt(%d), curUser(%s).", this->m_UserCnt,userName);
    //
    if(this->m_UserCnt > 0)
    {
        if(this->m_UserCnt < MAX_USER_NUMBER){
            strncpy((char*)this->m_UserName[this->m_UserCnt], userName, MAX_USER_NAME_SIZE - 1);
            old_cnt = android_atomic_inc(&m_UserCnt);
            LOG_INF(" - X. m_UserCnt: %d.", this->m_UserCnt);
            return Result;
        }
        else{
            LOG_ERR("m_userCnt is over upper bound\n");
            return MFALSE;
        }
    }

    // Open mfb device
    this->m_Fd = open(MFB_DRV_DEV_NAME, O_RDWR);
    if (this->m_Fd < 0)    // 1st time open failed.
    {
        LOG_ERR("MFB kernel 1st open fail, errno(%d):%s.", errno, strerror(errno));
        // Try again, using "Read Only".
        this->m_Fd = open(MFB_DRV_DEV_NAME, O_RDONLY);
        if (this->m_Fd < 0) // 2nd time open failed.
        {
            LOG_ERR("MFB kernel 2nd open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        else
            this->m_regRWMode=MFB_DRV_R_ONLY;
    }
    else    // 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
        // fd opened only once at the very 1st init
        m_pMfbHwRegAddr = (MUINT32 *) mmap(0, MFB_REG_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, MFB_BASE_HW);

        if(m_pMfbHwRegAddr == MAP_FAILED)
        {
            LOG_ERR("MFB mmap fail, errno(%d):%s", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
        this->m_regRWMode=MFB_DRV_RW_MMAP;
    }

    g_MfbImem_Size = TPIPE_BLD_SIZE;
    MFB_Imem_alloc(g_MfbImem_Size, &g_MfbImem_MemID, &g_MfbImem_pLogVir, &g_MfbTileBuffer.phyAddr);

    if(ioctl(this->m_Fd, MFB_RESET, NULL) < 0){
        LOG_ERR("MFB Reset fail !!\n");
        Result = MFALSE;
        goto EXIT;
    }

    //
    strncpy((char*)this->m_UserName[this->m_UserCnt], userName, MAX_USER_NAME_SIZE - 1);
    old_cnt = android_atomic_inc(&m_UserCnt);

EXIT:

    //
    if (!Result)    // If some init step goes wrong.
    {
        if(this->m_Fd >= 0)
        {
            // unmap to avoid memory leakage
            munmap(m_pMfbHwRegAddr, MFB_REG_RANGE);

            close(this->m_Fd);
            this->m_Fd = -1;
            LOG_INF("close mfb device Fd");
        }
    }

    LOG_DBG("-,ret: %d. mInitCount:(%d),m_pMfbHwRegAddr(0x%p)", Result, this->m_UserCnt, (void*)m_pMfbHwRegAddr);
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL MfbDrvImp::uninit(const char* userName)
{
    MBOOL Result = MTRUE;
    MUINT32 bMatch = 0;
    int32_t old_cnt;
    //
    android::Mutex::Autolock lock(m_MfbInitMutex);
    //
    if((userName==NULL) || (strlen(userName)<1) || (strlen(userName) >= MAX_USER_NAME_SIZE))
    {
        LOG_ERR("Plz add userName if you want to uninit mfb driver\n");
        return MFALSE;
    }
    //
    LOG_INF("-,m_UserCnt(%d),curUser(%s)", this->m_UserCnt,userName);

    //
    if(this->m_UserCnt <= 0)
    {
        LOG_ERR("no more user in MfbDrv , curUser(%s)",userName);
        goto EXIT;
    }

    for(MUINT32 i=0;i<MAX_USER_NUMBER;i++){
        if(strcmp((const char*)this->m_UserName[i],userName) == 0){
            bMatch = i+1;   //avoid match at the very 1st
            break;
        }
    }

    if(!bMatch){
        LOG_ERR("no matching username:%s\n",userName);
        for(MUINT32 i=0;i<MAX_USER_NUMBER;i+=4)
           LOG_ERR("current user: %s, %s, %s, %s\n",this->m_UserName[i],this->m_UserName[i+1],this->m_UserName[i+2],this->m_UserName[i+3]);
        return MFALSE;
    }
    else
        this->m_UserName[bMatch-1][0] = '\0';

    MFB_Imem_free((MUINT8 *)g_MfbTileBuffer.virtAddr, g_MfbTileBuffer.phyAddr, g_MfbTileBuffer.size, g_MfbTileBuffer.memID);

    // More than one user
    old_cnt = android_atomic_dec(&m_UserCnt);

    if(this->m_UserCnt > 0)    // If there are still users, exit.
        goto EXIT;

    if(m_pMfbHwRegAddr != MAP_FAILED){
         munmap(m_pMfbHwRegAddr, MFB_REG_RANGE);
    }


    //
    if(this->m_Fd >= 0)
    {
        close(this->m_Fd);
        this->m_Fd = -1;
        this->m_regRWMode=MFB_DRV_R_ONLY;
    }

    //
EXIT:

    LOG_INF(" - X. ret: %d. m_UserCnt: %d.", Result, this->m_UserCnt);

    if(this->m_UserCnt!= 0){
        LOG_INF("current working user:\n");
        for(MUINT32 i=0;i<MAX_USER_NUMBER;i+=8)
            LOG_INF("current user: %s, %s, %s, %s, %s, %s, %s, %s\n"    \
            ,this->m_UserName[i],this->m_UserName[i+1],this->m_UserName[i+2],this->m_UserName[i+3]  \
            ,this->m_UserName[i+4],this->m_UserName[i+5],this->m_UserName[i+6],this->m_UserName[i+7]);
    }
    return Result;
}

#define FD_CHK()({\
    MINT32 Ret=0;\
    if(this->m_Fd < 0){\
        LOG_ERR("no mfb device\n");\
        Ret = -1;\
    }\
    Ret;\
})

MBOOL MfbDrvImp::waitIrq(MFB_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 Ret = 0;
    MUINT32 OrgTimeOut;
    MFB_IRQ_CLEAR_ENUM OrgClr;
    MfbDbgTimer dbgTmr("waitIrq");
    MUINT32 Ta=0,Tb=0;
    MFB_WAIT_IRQ_STRUCT waitirq;
    LOG_DBG(" - E. Status(0x%08x),Timeout(%d).\n",pWaitIrq->Status, pWaitIrq->Timeout);
    if(-1 == FD_CHK()){
        return MFALSE;
    }
    waitirq.Clear=pWaitIrq->Clear;
    waitirq.Type=pWaitIrq->Type;
    waitirq.Status=pWaitIrq->Status;
    waitirq.Timeout=pWaitIrq->Timeout;
    waitirq.UserKey=pWaitIrq->UserKey;
    waitirq.ProcessID=pWaitIrq->ProcessID;
    waitirq.bDumpReg=pWaitIrq->bDumpReg;

    while( waitirq.Timeout > 0 )//receive restart system call again
    {
        Ta=dbgTmr.getUs();
        Ret = ioctl(this->m_Fd,MFB_WAIT_IRQ,&waitirq);
        Tb=dbgTmr.getUs();
        if( Ret== (-SIG_ERESTARTSYS) )
        {
            waitirq.Timeout=waitirq.Timeout - ((Tb-Ta)/1000);
            LOG_INF("ERESTARTSYS,Type(%d),Status(0x%08x),Timeout(%d us)\n",waitirq.Type, waitirq.Status, waitirq.Timeout);
        }
        else
        {
            break;
        }
    }

    if(Ret < 0) {
        LOG_ERR("MFB_WAIT_IRQ fail(%d). Wait Status(0x%08x), Timeout(%d).\n", Ret,  pWaitIrq->Status, pWaitIrq->Timeout);
        return MFALSE;
    }

    return MTRUE;
}

MBOOL MfbDrvImp::clearIrq(MFB_CLEAR_IRQ_STRUCT* pClearIrq)
{
    MINT32 Ret;
    MFB_CLEAR_IRQ_STRUCT clear;
    //
    LOG_DBG(" - E. user(%d), Status(%d)\n",pClearIrq->UserKey, pClearIrq->Status);
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    memcpy(&clear, pClearIrq, sizeof(MFB_CLEAR_IRQ_STRUCT));

    Ret = ioctl(this->m_Fd,MFB_CLEAR_IRQ,&clear);
    if(Ret < 0)
    {
        LOG_ERR("MFB_CLEAR_IRQ fail(%d)\n",Ret);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL MfbDrvImp::waitFrameDone(unsigned int Status, MINT32 timeoutMs)
{
    MFB_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = MFB_IRQ_WAIT_CLEAR;
    WaitIrq.Type = MFB_IRQ_TYPE_INT_MFB_ST;
    WaitIrq.Status = Status;
    WaitIrq.Timeout = timeoutMs;
    WaitIrq.UserKey = 0x0; //Driver Key
    WaitIrq.ProcessID = 0x0;
    WaitIrq.bDumpReg = 0x1;

    DRV_TRACE_BEGIN("MFB waitIrq");

    if (MTRUE == waitIrq(&WaitIrq))
    {
        if (Status & MFB_INT_ST)
        {
            LOG_INF("MFB Wait Interupt Frame Done Success!!\n");
        }
        DRV_TRACE_END();

        return MTRUE;
    }
    else
    {
        if (Status & MFB_INT_ST)
        {
            LOG_INF("MFB Wait Interupt Frame Done Fail!!\n");
            for (int qq = 0; qq <= 8; qq ++) {
                MFB_dumpTPipeBuffer((unsigned int *)g_MfbTileBuffer.virtAddr + 40 * qq);
            }
        }
        DRV_TRACE_END();
        return MFALSE;
    }
}

MINT32 configSRZ5(SrzCfg *pSrz_cfg, SRZSizeInfo SRZ5Info)
{
    MUINT32 srz_ctrl = 0x13; // srz_vert_en = 1, srz_hori_en = 1

    pSrz_cfg->ctrl = srz_ctrl;

    pSrz_cfg->inout_size.out_w = SRZ5Info.out_w;
    pSrz_cfg->inout_size.out_h = g_workaround_height;
    pSrz_cfg->crop.x = SRZ5Info.crop_x;
    pSrz_cfg->crop.y = SRZ5Info.crop_y;
    pSrz_cfg->crop.w = SRZ5Info.crop_w;
    pSrz_cfg->crop.h = SRZ5Info.crop_h;
    pSrz_cfg->crop.floatX = ((SRZ5Info.crop_floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
    pSrz_cfg->crop.floatY = ((SRZ5Info.crop_floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
    pSrz_cfg->h_step = ((SRZ5Info.crop_w-1)*32768+(((((SRZ5Info.out_w+31)>>5)<<5)-1)>>1) )/((((SRZ5Info.out_w+31)>>5)<<5)-1);
    pSrz_cfg->v_step = ((SRZ5Info.crop_h-1)*32768+(((((g_workaround_height+31)>>5)<<5)-1)>>1) )/((((g_workaround_height+31)>>5)<<5)-1);
    pSrz_cfg->inout_size.in_w = ((((long long)pSrz_cfg->h_step*(SRZ5Info.out_w-1))+16384)>>15)+1;
    pSrz_cfg->inout_size.in_h = ((((long long)pSrz_cfg->v_step*(g_workaround_height-1))+16384)>>15)+1;

    LOG_INF("out_w: %d, out_h: %d, in_w: %d, in_h: %d, h_step: %d, v_step: %d\n",
        pSrz_cfg->inout_size.out_w, pSrz_cfg->inout_size.out_h, pSrz_cfg->inout_size.in_w, pSrz_cfg->inout_size.in_h, pSrz_cfg->h_step, pSrz_cfg->v_step);
/*
//for TC_00
pSrz_cfg->h_step = 2304;
pSrz_cfg->v_step = 2048;

//for TC_01
pSrz_cfg->h_step = 1046;
pSrz_cfg->v_step = 2048;

//for TC_02
pSrz_cfg->h_step = 981;
pSrz_cfg->v_step = 1024;

//for TC_03
pSrz_cfg->h_step = 2304;
pSrz_cfg->v_step = 2048;

//for TC_04
pSrz_cfg->h_step = 2304;
pSrz_cfg->v_step = 2048;
*/
    return 0;
}

MINT32 configTPIPE(NSCam::NSIoPipe::MFBConfig* pMfbConfig, SrzCfg pSrz_cfg, MFB_DRV_DMA_EN_STRUCT mfb_dma_en)
{
    LOG_DBG("configTPIPE Start\n");
#if 1
    char* pWorkingBuffer =  NULL;
    MUINT32 tpipeWorkingSize = 300000;
    pWorkingBuffer = new char[tpipeWorkingSize];

    //pTpipeCfgInfo = (ISP_TPIPE_CONFIG_STRUCT *)gTileDesc.virtAddr;
    //pTpipeCfgInfo = (ISP_TPIPE_CONFIG_STRUCT *)(pMfbConfig->Mfb_inbuf_tdri->getBufVA(0)+256*100);

    pTpipeCfgInfo.top.mfb_en = 1;
    pTpipeCfgInfo.top.mfbi_en = mfb_dma_en.mfbi_en;
    pTpipeCfgInfo.top.mfbi_b_en = mfb_dma_en.mfbi_b_en;

    pTpipeCfgInfo.top.mfb3i_en = mfb_dma_en.mfb3i_en;
    pTpipeCfgInfo.top.mfb4i_en = mfb_dma_en.mfb4i_en;
    pTpipeCfgInfo.top.mfbo_en = mfb_dma_en.mfbo_en;
    pTpipeCfgInfo.top.mfbo_b_en = mfb_dma_en.mfbo_b_en;
    pTpipeCfgInfo.top.mfb2o_en = mfb_dma_en.mfb2o_en;

    pTpipeCfgInfo.top.mfb_srz_en = pMfbConfig->Mfb_srzEn;

    pTpipeCfgInfo.mfb.bld_mbd_wt_en = (pMfbConfig->Mfb_tuningBuf[0] & 0x00000040) >> 6;

    if (mfb_dma_en.mfbi_en == 1 && pMfbConfig->Mfb_inbuf_baseFrame != 0x0) {
        pTpipeCfgInfo.mfbi.mfbi_stride = pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(0);
        pTpipeCfgInfo.mfbi.mfbi_v_flip_en = 0;
    }

    if (mfb_dma_en.mfbi_b_en == 1 && pMfbConfig->Mfb_inbuf_baseFrame != 0x0) {
        pTpipeCfgInfo.mfbi_b.mfbi_b_stride = pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(1);
        pTpipeCfgInfo.mfbi_b.mfbi_b_v_flip_en = 0;
    }

    //if (mfb_dma_en.mfb2i_en == 1 && pMfbConfig->Mfb_inbuf_refFrame != 0x0) {
    //    pTpipeCfgInfo.mfb2i.mfb2i_stride = pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0);
    //    pTpipeCfgInfo.mfb2i.mfb2i_v_flip_en = 0;
    //}

    //if (mfb_dma_en.mfb2i_b_en == 1 && pMfbConfig->Mfb_inbuf_refFrame != 0x0) {
    //    pTpipeCfgInfo.mfb2bi.mfb2bi_stride = pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(1);
    //    pTpipeCfgInfo.mfb2bi.mfb2bi_v_flip_en = 0;
    //}

    if (mfb_dma_en.mfb3i_en == 1 && pMfbConfig->Mfb_inbuf_blendWeight != 0x0) {
        pTpipeCfgInfo.mfb3i.mfb3i_stride = pMfbConfig->Mfb_inbuf_blendWeight->getBufStridesInBytes(0);
        pTpipeCfgInfo.mfb3i.mfb3i_v_flip_en = 0;
    }

    if (mfb_dma_en.mfb4i_en == 1 && pMfbConfig->Mfb_inbuf_confidenceMap != 0x0) {
        pTpipeCfgInfo.mfb4i.mfb4i_stride = pMfbConfig->Mfb_inbuf_confidenceMap->getBufStridesInBytes(0);
        pTpipeCfgInfo.mfb4i.mfb4i_v_flip_en = 0;
        pTpipeCfgInfo.mfb4i.mfb4i_xsize = pMfbConfig->Mfb_inbuf_confidenceMap->getBufStridesInBytes(0) - 1;
        pTpipeCfgInfo.mfb4i.mfb4i_ysize = pMfbConfig->Mfb_inbuf_confidenceMap->getImgSize().h - 1;
    }

    if (mfb_dma_en.mfbo_en == 1 && pMfbConfig->Mfb_outbuf_frame != 0x0) {
        pTpipeCfgInfo.mfbo.mfbo_stride = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(0);
        pTpipeCfgInfo.mfbo.mfbo_xoffset = 0;
        pTpipeCfgInfo.mfbo.mfbo_xsize = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(0) - 1;
        pTpipeCfgInfo.mfbo.mfbo_yoffset = 0;
        pTpipeCfgInfo.mfbo.mfbo_ysize = g_workaround_height - 1;
    }

    if (mfb_dma_en.mfbo_b_en == 1 && pMfbConfig->Mfb_outbuf_frame != 0x0) {
        pTpipeCfgInfo.mfbo_b.mfbo_b_stride = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(1);
    }

    if (mfb_dma_en.mfb2o_en == 1 && pMfbConfig->Mfb_outbuf_blendWeight!= 0x0) {
        pTpipeCfgInfo.mfb2o.mfb2o_stride = pMfbConfig->Mfb_outbuf_blendWeight->getBufStridesInBytes(0);
        pTpipeCfgInfo.mfb2o.mfb2o_xoffset = 0;
        pTpipeCfgInfo.mfb2o.mfb2o_xsize = pMfbConfig->Mfb_outbuf_blendWeight->getBufStridesInBytes(0) - 1;
        pTpipeCfgInfo.mfb2o.mfb2o_yoffset = 0;
        pTpipeCfgInfo.mfb2o.mfb2o_ysize = g_workaround_height - 1;
    }

    pTpipeCfgInfo.mfb_srz.srz_input_crop_width = pSrz_cfg.inout_size.in_w;
    pTpipeCfgInfo.mfb_srz.srz_input_crop_height = pSrz_cfg.inout_size.in_h;
    pTpipeCfgInfo.mfb_srz.srz_output_width = pSrz_cfg.inout_size.out_w;
    pTpipeCfgInfo.mfb_srz.srz_output_height = pSrz_cfg.inout_size.out_h;
    pTpipeCfgInfo.mfb_srz.srz_luma_horizontal_integer_offset = 0;//pSrz_cfg.crop.x;
    pTpipeCfgInfo.mfb_srz.srz_luma_horizontal_subpixel_offset = 0;//pSrz_cfg.crop.floatX;
    pTpipeCfgInfo.mfb_srz.srz_luma_vertical_integer_offset = 0;//pSrz_cfg.crop.y;
    pTpipeCfgInfo.mfb_srz.srz_luma_vertical_subpixel_offset = 0;//pSrz_cfg.crop.floatY;
    pTpipeCfgInfo.mfb_srz.srz_horizontal_coeff_step = pSrz_cfg.h_step;
    pTpipeCfgInfo.mfb_srz.srz_vertical_coeff_step = pSrz_cfg.v_step;

    pTpipeCfgInfo.mfb_crsp.crsp_xoffset = 0x0;
    pTpipeCfgInfo.mfb_crsp.crsp_yoffset = 0x1;
    pTpipeCfgInfo.mfb_crsp.crsp_ystep = 0x4;

    if (pMfbConfig->Mfb_format_bit == FMT_4202P_10BIT_UNPAK)
    {
        pTpipeCfgInfo.mfb_y_pak.yuv_bit = 1;
        pTpipeCfgInfo.mfb_y_pak.yuv_dng = 1;
        pTpipeCfgInfo.mfb_c_pak.yuv_bit = 1;
        pTpipeCfgInfo.mfb_c_pak.yuv_dng = 1;
        pTpipeCfgInfo.mfb_y_unp.yuv_bit = 1;
        pTpipeCfgInfo.mfb_y_unp.yuv_dng = 1;
        pTpipeCfgInfo.mfb_c_unp.yuv_bit = 1;
        pTpipeCfgInfo.mfb_c_unp.yuv_dng = 1;
    }
    else if (pMfbConfig->Mfb_format_bit == FMT_4202P_10BIT_PAK)
    {
        pTpipeCfgInfo.mfb_y_pak.yuv_bit = 1;
        pTpipeCfgInfo.mfb_y_pak.yuv_dng = 0;
        pTpipeCfgInfo.mfb_c_pak.yuv_bit = 1;
        pTpipeCfgInfo.mfb_c_pak.yuv_dng = 0;
        pTpipeCfgInfo.mfb_y_unp.yuv_bit = 1;
        pTpipeCfgInfo.mfb_y_unp.yuv_dng = 0;
        pTpipeCfgInfo.mfb_c_unp.yuv_bit = 1;
        pTpipeCfgInfo.mfb_c_unp.yuv_dng = 0;
    }
    else if (pMfbConfig->Mfb_format_bit == FMT_4202P_8BIT)
    {
        pTpipeCfgInfo.mfb_y_pak.yuv_bit = 0;
        pTpipeCfgInfo.mfb_y_pak.yuv_dng = 0;
        pTpipeCfgInfo.mfb_c_pak.yuv_bit = 0;
        pTpipeCfgInfo.mfb_c_pak.yuv_dng = 0;
        pTpipeCfgInfo.mfb_y_unp.yuv_bit = 0;
        pTpipeCfgInfo.mfb_y_unp.yuv_dng = 0;
        pTpipeCfgInfo.mfb_c_unp.yuv_bit = 0;
        pTpipeCfgInfo.mfb_c_unp.yuv_dng = 0;
    }

    pTpipeCfgInfo.sw.log_en = 0;
    pTpipeCfgInfo.sw.src_width_mfb = pMfbConfig->Mfb_inbuf_baseFrame->getImgSize().w;
    pTpipeCfgInfo.sw.src_height_mfb = g_workaround_height;
    pTpipeCfgInfo.sw.tpipe_irq_mode = 0; // IRQ - 0: Frame Stop, 2: Tile Stop
    pTpipeCfgInfo.sw.tpipe_width_mfb = 0;
    pTpipeCfgInfo.sw.tpipe_height_mfb = 8192;
    pTpipeCfgInfo.sw.tpipe_sel_mode = 0x100000;

    LOG_DBG("tpipe_main_platform start, tdri_PA: 0x%lx, tdri_VA: 0x%lx\n", g_MfbTileBuffer.phyAddr, g_MfbTileBuffer.virtAddr);
    LOG_DBG("tpipe_main_platform start, working_buf_VA: 0x%lx, working_buf_size: %d\n", (unsigned long)pWorkingBuffer, tpipeWorkingSize);

    tpipe_main_platform(&pTpipeCfgInfo, &tpipeDesc, (char *)pWorkingBuffer, tpipeWorkingSize, NULL, NULL);

    g_tpipe_no = tpipeDesc.used_tpipe_no;

    memcpy((unsigned long *)g_MfbTileBuffer.virtAddr, tpipeDesc.tpipe_config_mfb, TPIPE_BLD_SIZE);

    if (g_isMFBLogEnable) {
        MFB_dumpTPipeCfgInfo(&pTpipeCfgInfo);
        for (int qq = 0; qq <= 8; qq ++) {
            MFB_dumpTPipeBuffer((unsigned int *)g_MfbTileBuffer.virtAddr + 40 * qq);
        }
    }

    delete[] pWorkingBuffer;
    pWorkingBuffer = NULL;

    //Use non-cache memory, so don't need to flush memory
    //m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &tpipeDesc);
#endif

    LOG_DBG("tpipe_main_platform end\n");

    return 0;
}

#define BYPASS_REG (0)
MBOOL MfbDrvImp::enque(vector<NSCam::NSIoPipe::MFBConfig>& MfbConfigVec)
{
    LOG_DBG("MFB Start Enque!!\n");
    MINT32 Ret;
    MUINT32 min;
    unsigned int num = MfbConfigVec.size();
    NSCam::NSIoPipe::MFBConfig* pMfbConfig;
    MFB_Request mfb_Request;
    MFB_Config mfb_Config[_SUPPORT_MAX_MFB_FRAME_REQUEST_];
    memset(&mfb_Config, 0, sizeof(mfb_Config));
    SrzCfg pSrz_cfg;
    memset(&pSrz_cfg, 0, sizeof(pSrz_cfg));
    MFB_DRV_DMA_EN_STRUCT mfb_dma_en;
    memset(&mfb_dma_en, 0, sizeof(MFB_DRV_DMA_EN_STRUCT));
    MFBERRCODE ErrCode = MFBERR_NOERROR;

    for (unsigned int i=0; i<MfbConfigVec.size(); i++)
    {
        pMfbConfig = &(MfbConfigVec.at(i));
        if (pMfbConfig == 0x0) {
            LOG_INF("No MFBConfig!\n");
        }
        LOG_INF("MFB ConfigVec[%d] Enque, BLD Mode: %d, SRZ En: %d\n", i, pMfbConfig->Mfb_bldmode, pMfbConfig->Mfb_srzEn);

        ErrCode = MFB_ErrorCheck(pMfbConfig);
        if (ErrCode != MFBERR_NOERROR) {
            LOG_INF("MFB Driver Check Error! ERRCDOE (%d)\n", ErrCode);
            abort();
        }

        if (pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0) != 0) {
            g_workaround_height = pMfbConfig->Mfb_inbuf_refFrame->getBufSizeInBytes(0)
                              / pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0);
            LOG_INF("MFB height = %d\n", g_workaround_height);
        }
        else {
            LOG_INF("Abnormal RefFrame, BufSize: %d, BufferStrides: %d\n",
                pMfbConfig->Mfb_inbuf_refFrame->getBufSizeInBytes(0), pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0));
        }

        mfb_Config[i].MFBDMA_MFBI_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_baseFrame->getBufPA(0);
        mfb_Config[i].MFBDMA_MFBI_STRIDE = (pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(0)) | (0x3 << 20);
        mfb_Config[i].MFBDMA_MFBI_YSIZE = g_workaround_height - 1;
        mfb_dma_en.mfbi_en = 1;

        mfb_Config[i].MFBDMA_MFBI_B_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_baseFrame->getBufPA(1);
        mfb_Config[i].MFBDMA_MFBI_B_STRIDE = (pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(1)) | (0x3 << 20);
        mfb_Config[i].MFBDMA_MFBI_B_YSIZE = g_workaround_height * 0.5 - 1;
        mfb_dma_en.mfbi_b_en = 1;

        mfb_Config[i].MFBDMA_MFB2I_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_refFrame->getBufPA(0);
        mfb_Config[i].MFBDMA_MFB2I_STRIDE = (pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0)) | (0x3 << 20);
        mfb_Config[i].MFBDMA_MFB2I_YSIZE = g_workaround_height - 1;
        mfb_dma_en.mfb2i_en = 1;

        mfb_Config[i].MFBDMA_MFB2I_B_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_refFrame->getBufPA(1);
        mfb_Config[i].MFBDMA_MFB2I_B_STRIDE = (pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(1)) | (0x3 << 20);
        mfb_Config[i].MFBDMA_MFB2I_B_YSIZE = g_workaround_height * 0.5 - 1;
        mfb_dma_en.mfb2i_b_en = 1;


        if (pMfbConfig->Mfb_bypassOMC != 0) {
            mfb_Config[i].OMC_TOP = ((pMfbConfig->Mfb_format_bit & 0xFF) << 4) | ((pMfbConfig->Mfb_omc_bicub & 0x1) << 3) | 0x1;
        } else {
            mfb_Config[i].OMC_TOP = ((pMfbConfig->Mfb_format_bit & 0xFF) << 4) | ((pMfbConfig->Mfb_omc_bicub & 0x1) << 3);
        }
        mfb_Config[i].OMC_FRAME_SIZE = ((pMfbConfig->Mfb_inbuf_refFrame->getImgSize().w & 0xFFFF) << 16) |
                                       (g_workaround_height & 0xFFFF);
        mfb_Config[i].OMC_TILE_EDGE = 0xF;
        mfb_Config[i].OMCC_OMC_C_ADDR_GEN_BASE_ADDR_0 = (unsigned int)pMfbConfig->Mfb_inbuf_refFrame->getBufPA(0);
        mfb_Config[i].OMCC_OMC_C_ADDR_GEN_STRIDE_0 = pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0);
        mfb_Config[i].OMCC_OMC_C_ADDR_GEN_BASE_ADDR_1 = (unsigned int)pMfbConfig->Mfb_inbuf_refFrame->getBufPA(1);
        mfb_Config[i].OMCC_OMC_C_ADDR_GEN_STRIDE_1 = pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(1);
        mfb_Config[i].OMCC_OMC_C_CTL_FMT_SEL = 0x10000 | (pMfbConfig->Mfb_format_bit & 0xFF << 8);

        if (pMfbConfig->Mfb_inbuf_blendWeight != 0x0) {
            if (pMfbConfig->Mfb_inbuf_blendWeight->getBufPA(0) != 0x0) {
                mfb_Config[i].MFBDMA_MFB3I_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_blendWeight->getBufPA(0);
                mfb_Config[i].MFBDMA_MFB3I_STRIDE = pMfbConfig->Mfb_inbuf_blendWeight->getBufStridesInBytes(0);
                mfb_Config[i].MFBDMA_MFB3I_YSIZE = g_workaround_height - 1;
                mfb_dma_en.mfb3i_en = 1;
            } else {
                mfb_dma_en.mfb3i_en = 0;
            }
        } else {
        }

        if (pMfbConfig->Mfb_inbuf_confidenceMap != 0x0) {
            if (pMfbConfig->Mfb_inbuf_confidenceMap->getBufPA(0) != 0x0) {
                mfb_Config[i].MFBDMA_MFB4I_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_confidenceMap->getBufPA(0);
                mfb_Config[i].MFBDMA_MFB4I_STRIDE = pMfbConfig->Mfb_inbuf_confidenceMap->getBufStridesInBytes(0);
                mfb_Config[i].MFBDMA_MFB4I_YSIZE = pMfbConfig->Mfb_inbuf_confidenceMap->getImgSize().h - 1;
                mfb_dma_en.mfb4i_en = 1;
            } else {
                mfb_dma_en.mfb4i_en = 0;
            }
        } else {
        }

        mfb_Config[i].OMC_MV_RDMA_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_mv->getBufPA(0);
        mfb_Config[i].OMC_MV_RDMA_STRIDE = ((pMfbConfig->Mfb_inbuf_refFrame->getImgSize().w >> 4 << 2) + 0xF) & (~0xF);
        //mfb_Config[i].OMC_MV_RDMA_STRIDE = pMfbConfig->Mfb_inbuf_mv->getBufStridesInBytes(0);

        mfb_Config[i].MFBDMA_MFBO_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_outbuf_frame->getBufPA(0);
        mfb_Config[i].MFBDMA_MFBO_STRIDE = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(0);
        mfb_Config[i].MFBDMA_MFBO_YSIZE = g_workaround_height - 1;
        mfb_dma_en.mfbo_en = 1;

        mfb_Config[i].MFBDMA_MFBO_B_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_outbuf_frame->getBufPA(1);
        mfb_Config[i].MFBDMA_MFBO_B_STRIDE = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(1);
        mfb_Config[i].MFBDMA_MFBO_B_YSIZE = g_workaround_height * 0.5 - 1;
        mfb_dma_en.mfbo_b_en = 1;

        mfb_Config[i].MFBDMA_MFB2O_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_outbuf_blendWeight->getBufPA(0);
        mfb_Config[i].MFBDMA_MFB2O_STRIDE = pMfbConfig->Mfb_outbuf_blendWeight->getBufStridesInBytes(0);
        mfb_Config[i].MFBDMA_MFB2O_YSIZE = g_workaround_height - 1;
        mfb_dma_en.mfb2o_en = 1;

#define MFB_TUNABLE
#ifdef MFB_TUNABLE
        LOG_DBG("MFB Set Tuning Regs!!\n");
        if (pMfbConfig->Mfb_bldmode == 0) {
            mfb_Config[i].MFB_CON = (pMfbConfig->Mfb_tuningBuf[0] & ~(1UL) & 0x0001FFF1) | 0x0;
        } else if (pMfbConfig->Mfb_bldmode == 1) {
            mfb_Config[i].MFB_CON = (pMfbConfig->Mfb_tuningBuf[0] & ~(1UL) & 0x0001FFF1) | 0x1;
        } else if (pMfbConfig->Mfb_bldmode == 2) {
            mfb_Config[i].MFB_CON = (pMfbConfig->Mfb_tuningBuf[0] & ~(1UL) & 0x0001FFF1) | 0x6;
        }
        mfb_Config[i].MFB_LL_CON1 = pMfbConfig->Mfb_tuningBuf[1];
        mfb_Config[i].MFB_LL_CON2 = pMfbConfig->Mfb_tuningBuf[2];
        mfb_Config[i].MFB_EDGE = 0xF;
        mfb_Config[i].MFB_LL_CON5 = pMfbConfig->Mfb_tuningBuf[6];
        mfb_Config[i].MFB_LL_CON6 = pMfbConfig->Mfb_tuningBuf[7];
        mfb_Config[i].MFB_LL_CON7 = pMfbConfig->Mfb_tuningBuf[8];
        mfb_Config[i].MFB_LL_CON8 = pMfbConfig->Mfb_tuningBuf[9];
        mfb_Config[i].MFB_LL_CON9 = pMfbConfig->Mfb_tuningBuf[10];
        mfb_Config[i].MFB_LL_CON10 = pMfbConfig->Mfb_tuningBuf[11];
        mfb_Config[i].MFB_MBD_CON0 = pMfbConfig->Mfb_tuningBuf[12];
        mfb_Config[i].MFB_MBD_CON1 = pMfbConfig->Mfb_tuningBuf[13];
        mfb_Config[i].MFB_MBD_CON2 = pMfbConfig->Mfb_tuningBuf[14];
        mfb_Config[i].MFB_MBD_CON3 = pMfbConfig->Mfb_tuningBuf[15];
        mfb_Config[i].MFB_MBD_CON4 = pMfbConfig->Mfb_tuningBuf[16];
        mfb_Config[i].MFB_MBD_CON5 = pMfbConfig->Mfb_tuningBuf[17];
        mfb_Config[i].MFB_MBD_CON6 = pMfbConfig->Mfb_tuningBuf[18];
        mfb_Config[i].MFB_MBD_CON7 = pMfbConfig->Mfb_tuningBuf[19];
        mfb_Config[i].MFB_MBD_CON8 = pMfbConfig->Mfb_tuningBuf[20];
        mfb_Config[i].MFB_MBD_CON9 = pMfbConfig->Mfb_tuningBuf[21];
        mfb_Config[i].MFB_MBD_CON10 = pMfbConfig->Mfb_tuningBuf[22];
        LOG_DBG("MFB Set Tuning Regs Done\n");
        LOG_DBG("MFB_CON: 0x%08x\n",       mfb_Config[i].MFB_CON);
        LOG_DBG("MFB_LL_CON1: 0x%08x\n",   mfb_Config[i].MFB_LL_CON1);
        LOG_DBG("MFB_LL_CON2: 0x%08x\n",   mfb_Config[i].MFB_LL_CON2);
        LOG_DBG("MFB_EDGE: 0x%08x\n",      mfb_Config[i].MFB_EDGE);
        LOG_DBG("MFB_LL_CON5: 0x%08x\n",   mfb_Config[i].MFB_LL_CON5);
        LOG_DBG("MFB_LL_CON6: 0x%08x\n",   mfb_Config[i].MFB_LL_CON6);
        LOG_DBG("MFB_LL_CON7: 0x%08x\n",   mfb_Config[i].MFB_LL_CON7);
        LOG_DBG("MFB_LL_CON8: 0x%08x\n",   mfb_Config[i].MFB_LL_CON8);
        LOG_DBG("MFB_LL_CON9: 0x%08x\n",   mfb_Config[i].MFB_LL_CON9);
        LOG_DBG("MFB_LL_CON10: 0x%08x\n",  mfb_Config[i].MFB_LL_CON10);
        LOG_DBG("MFB_MBD_CON0: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON0);
        LOG_DBG("MFB_MBD_CON1: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON1);
        LOG_DBG("MFB_MBD_CON2: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON2);
        LOG_DBG("MFB_MBD_CON3: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON3);
        LOG_DBG("MFB_MBD_CON4: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON4);
        LOG_DBG("MFB_MBD_CON5: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON5);
        LOG_DBG("MFB_MBD_CON6: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON6);
        LOG_DBG("MFB_MBD_CON7: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON7);
        LOG_DBG("MFB_MBD_CON8: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON8);
        LOG_DBG("MFB_MBD_CON9: 0x%08x\n",  mfb_Config[i].MFB_MBD_CON9);
        LOG_DBG("MFB_MBD_CON10: 0x%08x\n", mfb_Config[i].MFB_MBD_CON10);
#endif

        if (configSRZ5(&pSrz_cfg, pMfbConfig->Mfb_srz5Info) != 0)
        {
            LOG_ERR("MFB SRZ5 CONFIG FAIL\n");
        }
        mfb_Config[i].SRZ_CONTROL = pSrz_cfg.ctrl;
        mfb_Config[i].SRZ_IN_IMG = pSrz_cfg.inout_size.in_w | (pSrz_cfg.inout_size.in_h << 16);
        mfb_Config[i].SRZ_OUT_IMG = pSrz_cfg.inout_size.out_w | (pSrz_cfg.inout_size.out_h << 16);
        mfb_Config[i].SRZ_HORI_STEP = pSrz_cfg.h_step;
        mfb_Config[i].SRZ_VERT_STEP = pSrz_cfg.v_step;
        mfb_Config[i].SRZ_HORI_INT_OFST = 0;//pSrz_cfg.crop.x;
        mfb_Config[i].SRZ_HORI_SUB_OFST = 0;//pSrz_cfg.crop.floatX;
        mfb_Config[i].SRZ_VERT_INT_OFST = 0;//pSrz_cfg.crop.y;
        mfb_Config[i].SRZ_VERT_SUB_OFST = 0;//pSrz_cfg.crop.floatY;
        LOG_DBG("MFB Set SRZ Done, SRZ Ctrl: 0x%x\n", pSrz_cfg.ctrl);

        // [4:4]: C02 Interpolation Mode is enabled in Verification, [3:0]: C02_TPIPE_EDGE
        mfb_Config[i].C02_CON = 0x1F;

        mfb_Config[i].CRSP_CTRL = 0x80000002; // 422 to 420, fixed configuration
        mfb_Config[i].CRSP_OUT_IMG = (pMfbConfig->Mfb_inbuf_baseFrame->getImgSize().w) | (g_workaround_height << 16);
        mfb_Config[i].CRSP_STEP_OFST = (0x1 << 24) | (0x4 << 16) | (0x2);

        mfb_Config[i].MFB_MFB_TOP_CFG0 = pMfbConfig->Mfb_srzEn << 2 | 0x3;

        mfb_Config[i].MFB_MFB_TOP_CFG2 = mfb_dma_en.mfbi_en
                                   //| mfb_dma_en.mfb2i_en << 1
                                   | mfb_dma_en.mfb3i_en << 2
                                   | mfb_dma_en.mfb4i_en << 3
                                   | mfb_dma_en.mfbo_en << 4
                                   | mfb_dma_en.mfb2o_en << 5
                                   | mfb_dma_en.mfbi_b_en << 6
                                   //| mfb_dma_en.mfb2i_b_en << 7
                                   | mfb_dma_en.mfbo_b_en << 8;

        if (pMfbConfig->Mfb_format_bit == FMT_4202P_10BIT_UNPAK)
        {
            mfb_Config[i].PAK_CONT_Y = 0x000A1110;
            mfb_Config[i].PAK_CONT_C = 0x000A1100;
            mfb_Config[i].UNP_CONT_Y = 0x00010110;
            mfb_Config[i].UNP_CONT_C = 0x00010010;
        }
        else if (pMfbConfig->Mfb_format_bit == FMT_4202P_10BIT_PAK)
        {
            mfb_Config[i].PAK_CONT_Y = 0x000A0110;
            mfb_Config[i].PAK_CONT_C = 0x000A0100;
            mfb_Config[i].UNP_CONT_Y = 0x00000110;
            mfb_Config[i].UNP_CONT_C = 0x00000010;
        }
        else if (pMfbConfig->Mfb_format_bit == FMT_4202P_8BIT)
        {
            mfb_Config[i].PAK_CONT_Y = 0x000A0010;
            mfb_Config[i].PAK_CONT_C = 0x000A0000;
            mfb_Config[i].UNP_CONT_Y = 0x00000000;
            mfb_Config[i].UNP_CONT_C = 0x00000000;
        }

        //mfb_Config[i].MFBDMA_TDRI_BASE_ADDR = pMfbConfig->Mfb_inbuf_tdri->getBufPA(0);
        if (configTPIPE(pMfbConfig, pSrz_cfg, mfb_dma_en) != 0)
        {
            LOG_ERR("MFB TPIPE CONFIG FAIL\n");
        }
        mfb_Config[i].MFBDMA_TDRI_BASE_ADDR = g_MfbTileBuffer.phyAddr;
        mfb_Config[i].MFBDMA_TDRI_XSIZE = 160 - 1;
        mfb_Config[i].TPIPE_NO = g_tpipe_no;

        //LOG_DBG("TDRI WORKING BUFFER VA: 0x%lx, MFB VA: 0x%lx\n", (unsigned long)tpipeDesc.tpipe_config_bld, pMfbConfig->Mfb_inbuf_tdri->getBufVA(0));
        LOG_DBG("TDRI WORKING BUFFER VA: 0x%lx, MFB VA: 0x%lx\n", (unsigned long)tpipeDesc.tpipe_config_mfb, g_MfbTileBuffer.virtAddr);

        //for (int g = 0; g < 100; g++)
        //LOG_DBG("TDRI BUFFER[%d]:0x%08x 0x%08x 0x%08x 0x%08x\n", g, *((unsigned int *)(pMfbConfig->Mfb_inbuf_tdri->getBufVA(0) + 4*g)), *((unsigned int *)(pMfbConfig->Mfb_inbuf_tdri->getBufVA(0) + 4*g+1)), *((unsigned int *)(pMfbConfig->Mfb_inbuf_tdri->getBufVA(0) + 4*g+2)), *((unsigned int *)(pMfbConfig->Mfb_inbuf_tdri->getBufVA(0) + 4*g+3)));

        mfb_Config[i].MFBDMA_SPECIAL_FUN_EN = 0x000000FF;

        LOG_INF("MFB_CON: 0x%08x, MFB_TOP_CFG0: 0x%08x, MFB_TOP_CFG2: 0x%08x, PhyAddr: MFBI: 0x%08x, MFBI_B: 0x%08x, MFB2I: 0x%08x, MFB2I_B: 0x%08x, MFB3I: 0x%08x, MFB4I: 0x%08x, MFBO: 0x%08x, MFBO_B: 0x%08x, MFB2O: 0x%08x, TDRI: 0x%08x\n",
            mfb_Config[i].MFB_CON,
            mfb_Config[i].MFB_MFB_TOP_CFG0,
            mfb_Config[i].MFB_MFB_TOP_CFG2,
            mfb_Config[i].MFBDMA_MFBI_BASE_ADDR,
            mfb_Config[i].MFBDMA_MFBI_B_BASE_ADDR,
            mfb_Config[i].MFBDMA_MFB2I_BASE_ADDR,
            mfb_Config[i].MFBDMA_MFB2I_B_BASE_ADDR,
            mfb_Config[i].MFBDMA_MFB3I_BASE_ADDR,
            mfb_Config[i].MFBDMA_MFB4I_BASE_ADDR,
            mfb_Config[i].MFBDMA_MFBO_BASE_ADDR,
            mfb_Config[i].MFBDMA_MFBO_B_BASE_ADDR,
            mfb_Config[i].MFBDMA_MFB2O_BASE_ADDR,
            mfb_Config[i].MFBDMA_TDRI_BASE_ADDR);

        if (g_isMFBLogEnable == 1) {
            mfb_Config[i].USERDUMP_EN = 1;
        } else {
            mfb_Config[i].USERDUMP_EN = 0;
        }
    }

    mfb_Request.m_ReqNum = num;
    mfb_Request.m_pMfbConfig = mfb_Config;

    LOG_INF("Start to trigger, ReqNum = %d\n", mfb_Request.m_ReqNum);

    Ret = ioctl(this->m_Fd,MFB_ENQUE_REQ,&mfb_Request);
    if(Ret < 0)
    {
        LOG_ERR("MFB_ENQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }

    return MTRUE;
}

MBOOL MfbDrvImp::deque(vector<NSCam::NSIoPipe::MFBConfig>& MfbConfigVec)
{
    MINT32 Ret;
    NSCam::NSIoPipe::MFBConfig MfbConfig;
    MFB_Request mfb_Request;
    MFB_Config mfb_Config[_SUPPORT_MAX_MFB_FRAME_REQUEST_];
    mfb_Request.m_pMfbConfig = mfb_Config;

    Ret = ioctl(this->m_Fd,MFB_DEQUE_REQ,&mfb_Request);
    if(Ret < 0)
    {
        LOG_ERR("MFB_DEQUE_REQ fail(%d)\n", Ret);
        return MFALSE;
    }
    LOG_DBG("dequeMFB num:%d\n", mfb_Request.m_ReqNum);
    for (unsigned int i=0; i< mfb_Request.m_ReqNum; i++)
    {
        /* TODO: Engine-specific statistics for feature use */
        #if 0
            MfbConfig.feedback.reg1 = 0xFFFFFFFF;
            MfbConfig.feedback.reg2 = 0xFFFFFFFF;
        #endif
        MfbConfigVec.push_back(MfbConfig);
    }

    return MTRUE;
}

MUINT32 MfbDrvImp::readReg(MUINT32 Addr,MINT32 caller)
{
    MINT32 Ret=0;
    MUINT32 value=0x0;
    MUINT32 legal_range = MFB_REG_RANGE;
    (void) caller;

    LOG_DBG("+,Mfb_read:Addr(0x%x)\n",Addr);
    android::Mutex::Autolock lock(this->MfbRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return 1;
    }

    if(this->m_regRWMode==MFB_DRV_RW_MMAP){
        if(Addr >= legal_range){
            LOG_ERR("over range(0x%x)\n",Addr);
            return 0;
        }
        value = this->m_pMfbHwRegAddr[(Addr>>2)];
    }
    else{
        MFB_REG_IO_STRUCT MfbRegIo;
        MFB_DRV_REG_IO_STRUCT RegIo;
        //RegIo.module = this->m_HWmodule;
        RegIo.Addr = Addr;
        MfbRegIo.pData = (MFB_REG_STRUCT*)&RegIo;
        MfbRegIo.Count = 1;

        Ret = ioctl(this->m_Fd, MFB_READ_REGISTER, &MfbRegIo);
        if(Ret < 0)
        {
            LOG_ERR("MFB_READ via IO fail(%d)", Ret);
            return value;
        }
        value=RegIo.Data;
    }
    LOG_DBG("-,Mfb_read:(0x%x,0x%x)",Addr,value);
    return value;
}

//-----------------------------------------------------------------------------
MBOOL MfbDrvImp::readRegs(MFB_DRV_REG_IO_STRUCT* pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    MUINT32 legal_range = MFB_REG_RANGE;
    (void) caller;

    android::Mutex::Autolock lock(this->MfbRegMutex);
    //(void)caller;
    if((-1 == FD_CHK()) || (NULL == pRegIo)){
        return MFALSE;
    }

    if(this->m_regRWMode == MFB_DRV_RW_MMAP){
        unsigned int i;
        for (i=0; i<Count; i++)
        {
            if(pRegIo[i].Addr >= legal_range)
            {
                LOG_ERR("over range,bypass_0x%x\n",pRegIo[i].Addr);
            }
            else
            {
                pRegIo[i].Data = this->m_pMfbHwRegAddr[(pRegIo[i].Addr>>2)];
            }
        }
    }
    else{
        MFB_REG_IO_STRUCT MfbRegIo;
        //pRegIo->module = this->m_HWmodule;
        MfbRegIo.pData = (MFB_REG_STRUCT*)pRegIo;
        MfbRegIo.Count = Count;


        Ret = ioctl(this->m_Fd, MFB_READ_REGISTER, &MfbRegIo);
        if(Ret < 0)
        {
            LOG_ERR("MFB_READ via IO fail(%d)", Ret);
            return MFALSE;
        }
    }

    LOG_DBG("Mfb_reads_Cnt(%d): 0x%x_0x%x", Count, pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;
}

MBOOL MfbDrvImp::writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller)
{
    MINT32 ret=0;
    MUINT32 legal_range = MFB_REG_RANGE;
    (void) caller;

    LOG_DBG("Mfb_write:m_regRWMode(0x%x),(0x%x,0x%lx)",this->m_regRWMode,Addr,Data);
    android::Mutex::Autolock lock(this->MfbRegMutex);
    //(void)caller;
    if(-1 == FD_CHK()){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case MFB_DRV_RW_MMAP:
            if(Addr >= legal_range){
                LOG_ERR("over range(0x%x)\n",Addr);
                return MFALSE;
            }
            this->m_pMfbHwRegAddr[(Addr>>2)] = Data;
            break;
        case MFB_DRV_RW_IOCTL:
            MFB_REG_IO_STRUCT MfbRegIo;
            MFB_DRV_REG_IO_STRUCT RegIo;
            //RegIo.module = this->m_HWmodule;
            RegIo.Addr = Addr;
            RegIo.Data = Data;
            MfbRegIo.pData = (MFB_REG_STRUCT*)&RegIo;
            MfbRegIo.Count = 1;
            ret = ioctl(this->m_Fd, MFB_WRITE_REGISTER, &MfbRegIo);
            if(ret < 0){
                LOG_ERR("MFB_WRITE via IO fail(%d)", ret);
                return MFALSE;
            }
            break;
        case MFB_DRV_R_ONLY:
            LOG_ERR("MFB Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    //
    //release mutex in order to read back for DBG log
    this->MfbRegMutex.unlock();
    //
    return MTRUE;
}

MBOOL MfbDrvImp::writeRegs(MFB_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller)
{
    MINT32 Ret;
    unsigned int i=0;
    MUINT32 legal_range = MFB_REG_RANGE;
    (void) caller;

    android::Mutex::Autolock lock(this->MfbRegMutex);
    if(-1 == FD_CHK() || (NULL == pRegIo)){
        return MFALSE;
    }

    switch(this->m_regRWMode){
        case MFB_DRV_RW_IOCTL:
            MFB_REG_IO_STRUCT MfbRegIo;
            //pRegIo->module = this->m_HWmodule;
            MfbRegIo.pData = (MFB_REG_STRUCT*)pRegIo;
            MfbRegIo.Count = Count;

            Ret = ioctl(this->m_Fd, MFB_WRITE_REGISTER, &MfbRegIo);
            if(Ret < 0){
                LOG_ERR("MFB_WRITE via IO fail(%d)",Ret);
                return MFALSE;
            }
            break;
        case MFB_DRV_RW_MMAP:
            //if(this->m_HWmodule >= CAM_MAX )
            //    legal_range = DIP_BASE_RANGE_SPECIAL;
            do{
                if(pRegIo[i].Addr >= legal_range){
                    LOG_ERR("mmap over range,bypass_0x%x\n",pRegIo[i].Addr);
                    i = Count;
                }
                else
                    this->m_pMfbHwRegAddr[(pRegIo[i].Addr>>2)] = pRegIo[i].Data;
            }while(++i<Count);
            break;
        case MFB_DRV_R_ONLY:
            LOG_ERR("MFB Read Only");
            return MFALSE;
            break;
        default:
            LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
            return MFALSE;
            break;
    }
    LOG_DBG("Mfb_writes(%d):0x%x_0x%x\n",Count,pRegIo[0].Addr,pRegIo[0].Data);
    return MTRUE;

}

MUINT32 MfbDrvImp::getRWMode(void)
{
    return this->m_regRWMode;
}

MBOOL MfbDrvImp::setRWMode(MFB_DRV_RW_MODE rwMode)
{
    if(rwMode > MFB_DRV_R_ONLY)
    {
        LOG_ERR("no this reg operation mode(0x%x)",this->m_regRWMode);
        return MFALSE;
    }

    this->m_regRWMode = rwMode;
    return MTRUE;
}

