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
#define LOG_TAG "ICcuCtrlLtm"

#include <stdlib.h>
#include <sys/mman.h>
#include <cutils/properties.h>  // For property_get().

#include "ccu_log.h"
#include "ccu_ctrl.h"
#include "ccu_drvutil.h"
#include "ccu_ctrl_ltm.h"

namespace NSCcuIf {



/**************************************************************************
* Globals
**************************************************************************/
EXTERN_DBG_LOG_VARIABLE(ccu_drv);
// static CcuCtrlLtm _ccuCtrlLtmSingleton[CCU_CAM_TG_CNT];
static CcuCtrlLtm _ccuCtrlLtmSingleton[IMGSENSOR_SENSOR_IDX_MAX_NUM];

/*******************************************************************************
* Class Function
********************************************************************************/
ICcuCtrlLtm *ICcuCtrlLtm::getInstance(uint32_t sensorIdx, ESensorDev_T sensorDev)
{
    char value[100] = {'\0'};
    uint32_t ltm_ctrl_mode;
    property_get("vendor.ccu_ltm_ctrl.mode", value, "1");
    ltm_ctrl_mode = atoi(value);

    if(ltm_ctrl_mode == 0)
    {
        LOG_DBG("Adb command force disable LTM(%d)\n", ltm_ctrl_mode);
        return NULL;
    }

    ICcuCtrlLtm *mCcuLtmCtrl = NULL;
    mCcuLtmCtrl = (ICcuCtrlLtm *)&_ccuCtrlLtmSingleton[sensorIdx];
    return mCcuLtmCtrl;
}

/*******************************************************************************
* Public Functions
********************************************************************************/
int CcuCtrlLtm::init(uint32_t sensorIdx, ESensorDev_T sensorDev)
{
	LOG_DBG("+:%s\n",__FUNCTION__);

	int ret = AbsCcuCtrlBase::init(sensorIdx, sensorDev);
	if(ret != CCU_CTRL_SUCCEED)
	{
		return ret;
	}

    struct ccu_ltm_instance_mapping_handle instance_mapping;
    ccuControl(MSG_TO_CCU_LTM_GET_INT_IDX, NULL, &instance_mapping);
    m_ccuInstanceIdx = instance_mapping.instance_idx;
    LOG_DBG("LTM shared buffer init: si(%d) ii(%d)\n", sensorIdx, m_ccuInstanceIdx);

    CcuLtmBufferList *ltmBufferList = (CcuLtmBufferList *)m_pBufferList;
    volatile struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();

    LOG_DBG("LTM shared buffer, sharedBufMap(%p), ltmBufferList(%p)\n", sharedBufMap, ltmBufferList);
    //init ridx as -1, to prevent getting garbage data in first read
    sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].ltm_info_widx = -1;
    sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].ltm_info_ridx = -1;
    sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].hlr_info_widx = -1;
    sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].hlr_info_ridx = -1;

    //fill-up ltm algo output mva
    for(int i = 0 ; i < CCU_LTM_DBG_RING_SIZE ; i++)
    {
        sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].ltm_info_exif_addr[i] = ltmBufferList->ltm_info_exif[i].getMva();
        LOG_DBG("Set ltm_info_exif[%d](0x%x)", i, sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].ltm_info_exif_addr[i]);
    }

    for(int i = 0 ; i < CCU_LTM_DBG_RING_SIZE ; i++)
    {
        sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].ltm_info_isp_addr[i] = ltmBufferList->ltm_info_isp[i].getMva();
        LOG_DBG("Set ltm_info_isp[%d](0x%x)", i, sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].ltm_info_isp_addr[i]);
    }

    for(int i = 0 ; i < CCU_LTM_DBG_RING_SIZE ; i++)
    {
        sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].hlr_info_exif_addr[i] = ltmBufferList->hlr_info_exif[i].getMva();
        LOG_DBG("Set hlr_info_exif[%d](0x%x)", i, sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].hlr_info_exif_addr[i]);
    }

    for(int i = 0 ; i < CCU_LTM_DBG_RING_SIZE ; i++)
    {
        sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].hlr_info_isp_addr[i] = ltmBufferList->hlr_info_isp[i].getMva();
        LOG_DBG("Set hlr_info_isp[%d](0x%x)", i, sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].hlr_info_isp_addr[i]);
    }
	//TODO: Imp this func.
    LOG_DBG("-:%s\n",__FUNCTION__);
    return CCU_CTRL_SUCCEED;
}

int CcuCtrlLtm::uninit()
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    int ret = AbsCcuCtrlBase::uninit();
    if(ret != CCU_CTRL_SUCCEED)
    {
        LOG_DBG("%s: %d\n",__FUNCTION__, ret);
    }
    LOG_DBG("-:%s\n",__FUNCTION__);
    return ret;
}

void CcuCtrlLtm::destroyInstance(void)
{
	LOG_DBG("+:%s\n",__FUNCTION__);
	delete this;
	LOG_DBG("-:%s\n",__FUNCTION__);
}

// Get Ltm Exif data
uint32_t CcuCtrlLtm::getLtmInfoExif(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_ltm_debug_info)
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    if(!m_initialized)
    {
        LOG_DBG("Ltm instance not initialized\n");
        return 0;
    }
    //Step1: Get data address from shared memory
    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();
    uint32_t ridx = sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].ltm_info_ridx;
    if((ridx < 0) || (ridx >= CCU_LTM_DBG_RING_SIZE))
    {
        LOG_DBG("ridx(%d) not ready\n", ridx);
        return 0;
    }
    CcuLtmBufferList *ltmBufferList = (CcuLtmBufferList *)m_pBufferList;
    struct ccu_ltm_algo_data *ltmOutPtr = NULL;

    ltmOutPtr = (struct ccu_ltm_algo_data *)ltmBufferList->ltm_info_exif[ridx].getVa();

    if(ltmOutPtr == NULL)
    {
        LOG_DBG("Ltm shared buffer pointer null\n");
        return 0;
    }

    LOG_DBG("fidx(%d), ridx(%d), mva(0x%x), va(0x%x)\n",
        ltmOutPtr->frame_idx, ridx, ltmBufferList->ltm_info_exif[ridx].getMva(), ltmBufferList->ltm_info_exif[ridx].getVa());
    fillUpLtmDebugInfo(ccu_ltm_debug_info, ltmOutPtr);

	LOG_DBG("-:%s\n",__FUNCTION__);
    return ltmOutPtr->frame_idx;
}

// Get Ltm curve, ct...
uint32_t CcuCtrlLtm::getLtmInfoIsp(struct ccu_ltm_info_isp *ccu_ltm_debug_info)
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    int nvram_addr_match = 0;

    //Step1: Get data address from shared memory
    if(!m_initialized)
    {
        LOG_DBG("Ltm instance not initialized\n");
        return 0;
    }

    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();
    uint32_t ridx = sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].ltm_info_ridx;
    if((ridx < 0) || (ridx >= CCU_LTM_DBG_RING_SIZE))
    {
        LOG_DBG("ridx(%d) not ready\n", ridx);
        return 0;
    }
    CcuLtmBufferList *ltmBufferList = (CcuLtmBufferList *)m_pBufferList;
    struct ccu_ltm_info_isp *isp_info_ptr = NULL;

    isp_info_ptr = (struct ccu_ltm_info_isp *)ltmBufferList->ltm_info_isp[ridx].getVa();

    if(isp_info_ptr == NULL)
    {
        LOG_DBG("Ltm shared buffer pointer null\n");
        return 0;
    }
    memcpy(ccu_ltm_debug_info, isp_info_ptr, sizeof(struct ccu_ltm_info_isp));

    for(int i = 0 ; i < CCU_LTMSO_RING_SIZE ; i++)
    {
        if(ccu_ltm_debug_info->ltmso_buffer_addr == ltmBufferList->ltmso_ring_buf[i].getMva())
        {
            nvram_addr_match = i;
            break;
        }
    }

    ccu_ltm_debug_info->ltmso_buffer_addr_va = (void *)ltmBufferList->ltmso_ring_buf[nvram_addr_match].getVa();

    LOG_DBG("fIdx(%d), rIdx(%d), mva(0x%x), va(0x%x)\n",
        ccu_ltm_debug_info->frame_idx, ridx,
        ltmBufferList->ltmso_ring_buf[nvram_addr_match].getMva(), ltmBufferList->ltmso_ring_buf[nvram_addr_match].getVa());

    LOG_DBG("-:%s\n",__FUNCTION__);
    return isp_info_ptr->frame_idx;
}

// Get Ltm Exif data
uint32_t CcuCtrlLtm::getHlrInfoExif(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_hlr_debug_info)
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    if(!m_initialized)
    {
        LOG_DBG("Ltm instance not initialized\n");
        return 0;
    }

    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();
    uint32_t ridx = sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].hlr_info_ridx;
    if((ridx < 0) || (ridx >= CCU_LTM_DBG_RING_SIZE))
    {
        LOG_DBG("ridx(%d) not ready\n", ridx);
        return 0;
    }
    CcuLtmBufferList *ltmBufferList = (CcuLtmBufferList *)m_pBufferList;
    struct ccu_hlr_algo_data *hlr_info_exif_va = NULL;

    hlr_info_exif_va = (struct ccu_hlr_algo_data *)ltmBufferList->hlr_info_exif[ridx].getVa();

    if(hlr_info_exif_va == NULL)
    {
        LOG_DBG("Ltm shared buffer pointer null\n");
        return 0;
    }
    LOG_DBG("fidx(%d), ridx(%d), mva(0x%x), va(0x%x)\n",
        hlr_info_exif_va->frame_idx, ridx, ltmBufferList->hlr_info_exif[ridx].getMva(), ltmBufferList->hlr_info_exif[ridx].getVa());
    fillUpHlrDebugInfo(ccu_hlr_debug_info, hlr_info_exif_va);

    LOG_DBG("-:%s\n",__FUNCTION__);
    return hlr_info_exif_va->frame_idx;
}

// Get Hlr hw setting
uint32_t CcuCtrlLtm::getHlrInfoIsp(struct ccu_hlr_info_isp *ccu_hlr_debug_info)
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    if(!m_initialized)
    {
        LOG_DBG("Ltm instance not initialized\n");
        return 0;
    }

    struct shared_buf_map *sharedBufMap = m_pDrvCcu->getSharedBufMap();
    uint32_t ridx = sharedBufMap->ltm_debug_info_handle[m_ccuInstanceIdx].hlr_info_ridx;
    if((ridx < 0) || (ridx >= CCU_LTM_DBG_RING_SIZE))
    {
        LOG_DBG("ridx(%d) not ready\n", ridx);
        return 0;
    }
    CcuLtmBufferList *ltmBufferList = (CcuLtmBufferList *)m_pBufferList;
    struct ccu_hlr_info_isp *isp_info_ptr = NULL;

    isp_info_ptr = (struct ccu_hlr_info_isp *)ltmBufferList->hlr_info_isp[ridx].getVa();

    if(isp_info_ptr == NULL)
    {
        LOG_DBG("Ltm shared buffer pointer null\n");
        return 0;
    }
    memcpy(ccu_hlr_debug_info, isp_info_ptr, sizeof(struct ccu_hlr_info_isp));

    LOG_DBG("fIdx(%d), rIdx(%d), mva(0x%x), va(0x%x)\n",
        isp_info_ptr->frame_idx, ridx, ltmBufferList->hlr_info_isp[ridx].getMva(), ltmBufferList->hlr_info_isp[ridx].getVa());

    LOG_DBG("-:%s\n",__FUNCTION__);
    return isp_info_ptr->frame_idx;
}

bool CcuCtrlLtm::ccuCtrlPreprocess(uint32_t msgId, void *inDataPtr, void *inDataBuf)
{
    bool result = true;
    switch(msgId)
    {
        case MSG_TO_CCU_LTM_INIT:
        {
            result = ccuCtrlPreprocess_LtmInit(inDataPtr, inDataBuf);
            break;
        }
        case MSG_TO_CCU_LTM_PERFRAME_CTRL:
        {
            result = ccuCtrlPreprocess_LtmPerframeCtrl(inDataPtr, inDataBuf);
            break;
        }
        default : break;
    }
    return result;
}

bool CcuCtrlLtm::ccuCtrlPreprocess_LtmInit(void *inDataPtr, void *inDataBuf)
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    CcuLtmBufferList *ltmBufferList = (CcuLtmBufferList *)m_pBufferList;
    struct ccu_ltm_init_data_in *in_ptr = (struct ccu_ltm_init_data_in *)inDataPtr;
    struct ccu_ltm_init_data_in *out_ptr = (struct ccu_ltm_init_data_in *)inDataBuf;

    // Overwrite LTMSO ring buffer
    for(int i = 0 ; i < CCU_LTMSO_RING_SIZE ; i++)
    {
        out_ptr->ltmso_ring_buf_addr[i] = ltmBufferList->ltmso_ring_buf[i].getMva();
        LOG_INF_MUST("LTMSO ring buffer[%d]:(%x)\n", i, ltmBufferList->ltmso_ring_buf[i].getMva());
    }

    LOG_INF_MUST("LTM(NVRAM addr(0x%x), size(%d), num(%d))\n", in_ptr->ltm_nvram_addr, in_ptr->ltm_nvram_size, in_ptr->ltm_nvram_num);
    LOG_INF_MUST("HLR(NVRAM addr(0x%x), size(%d), num(%d))\n", in_ptr->hlr_nvram_addr, in_ptr->hlr_nvram_size, in_ptr->hlr_nvram_num);
    LOG_INF_MUST("HLR(NVRAM MVA addr(%x))\n", ltmBufferList->CtrlMsgBufs[MSG_TO_CCU_LTM_PERFRAME_CTRL]);

    // // Check nvram size match
    // if((in_ptr->ltm_nvram_size/in_ptr->ltm_nvram_num) != CCU_LTM_NVRAM_SIZE)
    // {
    //     LOG_DBG("LTM nvram size mismatch, AP(%d)/num(%d), CCU(%d)\n", in_ptr->ltm_nvram_size, in_ptr->ltm_nvram_num, CCU_LTM_NVRAM_SIZE);
    // }
    // else
    // {
    //     // Overwrite LTM nvram buffer
    //     memcpy(ltmBufferList->ltm_nvram.getVa(), in_ptr->ltm_nvram_addr, in_ptr->ltm_nvram_size);
    //     out_ptr->ltm_nvram_addr_ccu = (uint32_t)ltmBufferList->ltm_nvram.getMva();
    //     LOG_INF_MUST("LTM nvram buf(%x), size(%d), num(%d)\n", ltmBufferList->ltm_nvram.getMva(), in_ptr->ltm_nvram_size, in_ptr->ltm_nvram_num);
    // }
    // if((in_ptr->hlr_nvram_size/in_ptr->hlr_nvram_num) != CCU_HLR_NVRAM_SIZE)
    // {
    //     LOG_DBG("HLR nvram size mismatch, AP(%d), CCU(%d)\n", in_ptr->hlr_nvram_size, CCU_HLR_NVRAM_SIZE);
    // }
    // else
    // {
    //     // Overwrite HLR nvram buffer
    //     memcpy(ltmBufferList->hlr_nvram.getVa(), in_ptr->hlr_nvram_addr, in_ptr->hlr_nvram_size);
    //     out_ptr->hlr_nvram_addr_ccu = (uint32_t)ltmBufferList->hlr_nvram.getMva();
    //     LOG_INF_MUST("HLR nvram buf(%x), size(%d), num(%d)\n", ltmBufferList->hlr_nvram.getMva(), in_ptr->hlr_nvram_size, in_ptr->hlr_nvram_num);
    // }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return true;
}

bool CcuCtrlLtm::ccuCtrlPreprocess_LtmPerframeCtrl(void *inDataPtr, void *inDataBuf)
{
    LOG_DBG("+:%s\n",__FUNCTION__);
    struct ccu_ltm_perframe_data_in *inpt = (struct ccu_ltm_perframe_data_in *)inDataPtr;
    struct ccu_ltm_perframe_data_in *oupt = (struct ccu_ltm_perframe_data_in *)inDataBuf;
    // char value[100] = {'\0'};
    // uint32_t ltm_ctrl_mode;
    // property_get("vendor.ccu_ltm_ctrl.mode", value, "1");
    // ltm_ctrl_mode = atoi(value);
    //0: Disable, 1: Enable & Fix Curve, 2: Enable & Normal mode.
    // if(inpt->ltm_en)
        // oupt->ltm_en = ltm_ctrl_mode;

    LOG_DBG("LTM mode: %d\n", inpt->ltm_en);
    LOG_DBG("LTM magic_number(%d), request_number(%d)\n", inpt->magic_number, inpt->request_number);
    LOG_DBG("-:%s\n",__FUNCTION__);
    return true;
}



void CcuCtrlLtm::fillUpLtmDebugInfo(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_ltm_debug_info, ccu_ltm_algo_data *p_ltm_algo_data)
{
    int i, j;   
    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_ChipVersion, p_ltm_algo_data->chip_version);
    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_MainVersion, p_ltm_algo_data->main_version);
    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_SubVersion, p_ltm_algo_data->sub_version);
    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_SystemVersion, p_ltm_algo_data->system_version);
    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_LV, p_ltm_algo_data->real_light_value_x10);
    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_STABLE, p_ltm_algo_data->ltm_stable_cnt);
    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_LTMS_CT, p_ltm_algo_data->ltm_ltmso_ct);

    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_GLOBAL_CT, p_ltm_algo_data->ltm_global_clipping_thd);
    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_GLOBAL_TP, p_ltm_algo_data->ltm_global_turning_point);
    setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_GLOBAL_MP, p_ltm_algo_data->ltm_global_middle_point);
    
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_NUM, p_ltm_algo_data->face_num);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_PROTECT_NUM, p_ltm_algo_data->protect_face_num);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_CHECK_FLAG, p_ltm_algo_data->face_check_flag);
	for (i=0;i<CCU_LTM_MAX_FACE_AREAS;i++)
	{
		setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_AREA_0_LEFT+i, p_ltm_algo_data->fd_area[i].i4_left);
		setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_AREA_0_RIGHT+i, p_ltm_algo_data->fd_area[i].i4_right);
		setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_AREA_0_TOP+i, p_ltm_algo_data->fd_area[i].i4_top);
		setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_AREA_0_BOTTOM+i, p_ltm_algo_data->fd_area[i].i4_bottom);
		setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_BRIGHT_TONE_PROTECT_VALUE0+i, p_ltm_algo_data->face_bright_tone_protect_value[i]);
	}
		
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_SMOOTH_STATE, p_ltm_algo_data->ltm_face_smooth_type);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_NON_ROBUST_CNT, p_ltm_algo_data->non_robust_face_count);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_LAST_ROBUST_FACE_TP, p_ltm_algo_data->last_robust_max_face_turning_point);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_LTM_LOCK_RESULT, p_ltm_algo_data->ltm_smooth_lock_result);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_AE_STABLE, p_ltm_algo_data->face_ae_stable);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_AE_EN, p_ltm_algo_data->enable_face_ae);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_STATE_FROM_AE, p_ltm_algo_data->face_state_from_ae);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_ROBUST_CNT, p_ltm_algo_data->face_robust_count);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_ROBUST_TRUST_CNT, p_ltm_algo_data->face_robust_trust_count);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_LOST_AE_LOCK_MAX_CNT, p_ltm_algo_data->face_lost_lock_max_count);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_LOST_AE_TS_FRAME_NUM, p_ltm_algo_data->face_lost_temporal_smooth_frame_num);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_AE_NON_ROBUST_TIMEOUT, p_ltm_algo_data->non_robust_face_ae_converge_timeout);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_AE_NON_ROUBST_LOCK_FLAG, p_ltm_algo_data->non_robust_face_timeout_lock_ae);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_SPATIAL_BG_TO_FACE_TPDIFF_X1, p_ltm_algo_data->background_to_face_tp_diff_x[0]);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_SPATIAL_BG_TO_FACE_TPDIFF_X2, p_ltm_algo_data->background_to_face_tp_diff_x[1]);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_SPATIAL_BG_TO_FACE_WEIGHT_Y1, p_ltm_algo_data->background_to_face_weighting_y[0]);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_SPATIAL_BG_TO_FACE_WEIGHT_Y2, p_ltm_algo_data->background_to_face_weighting_y[1]);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_SPATIAL_FACE_TO_BG_TPDIFF_X1, p_ltm_algo_data->face_to_background_tp_diff_x[0]);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_SPATIAL_FACE_TO_BG_TPDIFF_X2, p_ltm_algo_data->face_to_background_tp_diff_x[1]);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_SPATIAL_FACE_TO_BG_WEIGHT_Y1, p_ltm_algo_data->face_to_background_weighting_y[0]);
	setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_SPATIAL_FACE_TO_BG_WEIGHT_Y2, p_ltm_algo_data->face_to_background_weighting_y[1]);
	for (i=0;i<LTM_HEIGHT;i++)
	{
		for (j=0;j<LTM_WIDTH;j++)
		{
			setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_LOCAL_TP_RESULT_0_0 + (i*LTM_WIDTH+j), p_ltm_algo_data->ltm_local_turning_point_result[i][j]);
			setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_LOCAL_CT_RESULT_0_0 + (i*LTM_WIDTH+j), p_ltm_algo_data->ltm_local_clipping_thd_result[i][j]);
			setDebugTag(ccu_ltm_debug_info, NSIspExifDebug::SW_LTM_FACE_MAP_0_0 + (i*LTM_WIDTH+j), p_ltm_algo_data->face_alpha_map[i][j]);
		}
	}
}

void CcuCtrlLtm::setDebugTag(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_ltm_debug_info, unsigned int index, int value)
{
    ccu_ltm_debug_info->tags[index].u4Val = value;
}

void CcuCtrlLtm::fillUpHlrDebugInfo(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_hlr_debug_info, ccu_hlr_algo_data *p_hlr_algo_data)
{
	//memset(ccu_hlr_debug_info, 0, sizeof(HLR_TAG_DEBUG_INFO_T));
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_ChipVersion, 6779);
    setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_MainVersion, 40);
    setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SubVersion, 1);
    setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SystemVersion, 1);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_VERSION, p_hlr_algo_data->hlr_version);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_VERSION, p_hlr_algo_data->hlr_version);
	//INPUT INFO
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_CAM_ID, p_hlr_algo_data->cam_id);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_MASTER_CAM_ID, p_hlr_algo_data->master_cam_id);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_ALGO_SCENE_LV, p_hlr_algo_data->lv_input);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_ALGO_SCENE_CCT, p_hlr_algo_data->cct_input);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_LTM_CLIP_TH, p_hlr_algo_data->ltm_clip_th);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_HDR_RATIO, p_hlr_algo_data->hdr_ratio);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_DGN_GR, p_hlr_algo_data->dgn_gr);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_DGN_GB, p_hlr_algo_data->dgn_gb);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_DGN_R, p_hlr_algo_data->dgn_r);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_DGN_B, p_hlr_algo_data->dgn_b);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_MODE, p_hlr_algo_data->hlr_mode);
	//NVRAM
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SAT_0, p_hlr_algo_data->hlr_sat_0);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SAT_1, p_hlr_algo_data->hlr_sat_1);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SAT_2, p_hlr_algo_data->hlr_sat_2);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SAT_3, p_hlr_algo_data->hlr_sat_3);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SAT_4, p_hlr_algo_data->hlr_sat_4);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SAT_5, p_hlr_algo_data->hlr_sat_5);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SAT_6, p_hlr_algo_data->hlr_sat_6);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SAT_7, p_hlr_algo_data->hlr_sat_7);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_NODE_0, p_hlr_algo_data->hlr_node_0);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_NODE_1, p_hlr_algo_data->hlr_node_1);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_NODE_2, p_hlr_algo_data->hlr_node_2);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_NODE_3, p_hlr_algo_data->hlr_node_3);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_NODE_4, p_hlr_algo_data->hlr_node_4);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_NODE_5, p_hlr_algo_data->hlr_node_5);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_NODE_6, p_hlr_algo_data->hlr_node_6);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_NODE_7, p_hlr_algo_data->hlr_node_7);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_BLD_HIGH_SWO, p_hlr_algo_data->hlr_bld_high_swo);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_BLD_LOW_SWO, p_hlr_algo_data->hlr_bld_low_swo);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_PRT_HIGH_SWO, p_hlr_algo_data->hlr_prt_high_swo);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_PRT_LOW_SWO, p_hlr_algo_data->hlr_prt_low_swo);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_PRT_EN_SWO, p_hlr_algo_data->hlr_prt_en_swo);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SLP_0, p_hlr_algo_data->hlr_slp_0);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SLP_1, p_hlr_algo_data->hlr_slp_1);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SLP_2, p_hlr_algo_data->hlr_slp_2);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SLP_3, p_hlr_algo_data->hlr_slp_3);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SLP_4, p_hlr_algo_data->hlr_slp_4);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SLP_5, p_hlr_algo_data->hlr_slp_5);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_MAX_RAT, p_hlr_algo_data->hlr_max_rat);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_BLUE_PRT_STR, p_hlr_algo_data->hlr_blue_prt_str);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_RED_PRT_STR, p_hlr_algo_data->hlr_red_prt_str);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_SLP_6, p_hlr_algo_data->hlr_slp_6);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_BLD_FG, p_hlr_algo_data->hlr_bld_fg);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_BLD_HIGH, p_hlr_algo_data->hlr_bld_high);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_BLD_SLP, p_hlr_algo_data->hlr_bld_slp);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_PRT_TH, p_hlr_algo_data->hlr_prt_th);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_PRT_SLP, p_hlr_algo_data->hlr_prt_slp);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_PRT_EN, p_hlr_algo_data->hlr_prt_en);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_CLIP_VAL, p_hlr_algo_data->hlr_clip_val);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_OFF_CLIP_VAL, p_hlr_algo_data->hlr_off_clip_val);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_EFCT_ON, p_hlr_algo_data->hlr_efct_on);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_TILE_EDGE, p_hlr_algo_data->hlr_tile_edge);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_TDR_WD, p_hlr_algo_data->hlr_tdr_wd);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_TDR_HT, p_hlr_algo_data->hlr_tdr_ht);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_R1_EN, p_hlr_algo_data->hlr_r1_en);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_D1_EN, p_hlr_algo_data->hlr_d1_en);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_R1_LKMSB, p_hlr_algo_data->hlr_r1_lkmsb);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_D1_LKMSB, p_hlr_algo_data->hlr_d1_lkmsb);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_R1_LKMODE, p_hlr_algo_data->hlr_r1_lkmode);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_D1_LKMODE, p_hlr_algo_data->hlr_d1_lkmode);
	setHlrDebugTag(ccu_hlr_debug_info, NSIspExifDebug::SW_HLR_RESERV_A, p_hlr_algo_data->hlr_resrv_a);
	
}
void CcuCtrlLtm::setHlrDebugTag(NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_hlr_debug_info, unsigned int index, unsigned int value)
{
	ccu_hlr_debug_info->tags[index].u4Val = value;
}

/*******************************************************************************
* Overridden Functions
********************************************************************************/
enum ccu_feature_type CcuCtrlLtm::_getFeatureType()
{
    return CCU_FEATURE_LTM;
}

bool CcuCtrlLtm::getLtmInitStatus()
{
    return m_initialized;
}

};  //namespace NSCcuIf