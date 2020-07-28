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
#ifndef _CCU_MGR_H_
#define _CCU_MGR_H_

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

#include "ccu_udrv.h"
#include "ccuif_compat.h"
#include "ccu_cmd_param_info.h"
#include "iccu_mgr.h"
#include "ccu_buffer.h"
#include "ccu_platform_def.h"
#include "iccu_ctrl_system.h"
#include "kd_camera_feature.h"

namespace NSCcuIf {

class ICcuMgrPriv
{
public:

    static ICcuMgrPriv* createInstance();
    static void destroyInstance();
    static bool isCcuBooted();

    virtual ~ICcuMgrPriv(){}
    virtual struct CcuBufferList *getCcuBufferList(enum ccu_feature_type ccuFeature, uint32_t sensor_idx) = 0;
};

class CcuMgr: ICcuMgrExt, ICcuMgrPriv
{
public:
    CcuMgr();
    ~CcuMgr();
    int ccuInit();
    int ccuUninit();
    int ccuBoot();
    int ccuShutdown();
    int ccuSetSensorIdx(ESensorDev_T sensorDev, MUINT32 sensorIdx);
    int ccuInvalidateSensorIdx(ESensorDev_T sensorDev, MUINT32 sensorIdx);
    struct CcuBufferList *getCcuBufferList(enum ccu_feature_type ccuFeature, uint32_t sensor_idx);
    int ccuSuspend(ESensorDev_T sensorDev, MUINT32 sensorIdx);
    int ccuResume(ESensorDev_T sensorDev, MUINT32 sensorIdx);
    static bool IsCcuMgrEnabled;

private:
    //memory/buffer
    void _checkCcuEnable();
    bool _initBufferLists();
    bool _importBuffersToKernel();
    bool _importBufferListToKernel(struct CcuBufferList *list, import_mem_t *import_buf, uint32_t *importIdx);

    MINT8 m_UserName[32];
    struct CcuAeBufferList m_AeBufferList[IMGSENSOR_SENSOR_IDX_MAX_NUM];
    struct CcuAfBufferList m_AfBufferList[IMGSENSOR_SENSOR_IDX_MAX_NUM];
    struct CcuLtmBufferList  m_LtmBufferList[IMGSENSOR_SENSOR_IDX_MAX_NUM];
    struct Ccu3ASyncBufferList m_3asyncBufferList;
    struct CcuSystemBufferList m_sysctrlBufferList;
    struct CcuBuffer m_logBuffers[CCU_DRAM_LOG_BUF_CNT];
    struct CcuBuffer m_ddrBuffer;
    struct CcuBuffer m_backupDataBuffer;

    ICcuCtrlSystem *m_pCcuCtrlSys;
    CcuDrvImp *m_pDrvCcu;
};

};  //namespace NSCcuIf

#endif
