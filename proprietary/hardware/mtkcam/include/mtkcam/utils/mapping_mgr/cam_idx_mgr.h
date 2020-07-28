/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _CAM_IDX_MGR_H_
#define _CAM_IDX_MGR_H_

#include <stdarg.h>
#include <vector>
#include <mtkcam/def/common.h>
#include "tuning_mapping/cam_idx_struct_ext.h"
#include "tuning_mapping/cam_idx_struct_int.h"
#include "isp_tuning/ver1/isp_tuning_cam_info.h"
/*
extra include path needed to use isp_tuning_cam_info.h
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
*/
#include "isp_tuning_sensor.h"
#include "camera_custom_nvram.h"

#include <utils/Mutex.h>
#include <list>
#include <isp_tuning_custom.h>

#include <atomic>

#define MAPPING_INFO_LIST_SIZE (60)
#define IDXMGRIMP_SIZE  (5)

#define INVALID_SCENARIO    0xFFFF

// for mt6785 workaround: custom/mt6785/hal/inc/CFG_Camera_File_Max_Size.h  not updated
#ifndef SENSOR_IDX_MAX
#define SENSOR_IDX_MAX 20
#endif

using namespace android;
using namespace std;

typedef struct {
    MUINT16 idx;
    MUINT16 scenario;
    MUINT16 group;
} IDX_QUERY_RESULT;

typedef struct {
    MUINT16 idx;
    MUINT16 scenario;
    char* pScenario;
    MUINT16 group;
    MUINT16 base;
} IDXMGR_QUERY_RESULT;

typedef struct {
    CAM_IDX_QRY_COMB qry;
    MUINT32 u4FrmId;
} ISP_MAPPING_INFO_T;

class IdxBase
{
public:
    virtual ~IdxBase() {}
    virtual MVOID init(EModule_T, IDX_BASE_T*, MUINT16*)         = 0;
    virtual IDXMGR_QUERY_RESULT query(const CAM_IDX_QRY_COMB& qry)  = 0;

protected:
    void mismatchHandling(EModule_T mod,  const CAM_IDX_QRY_COMB& qry);
};

class IdxMask : public IdxBase
{
public:
    IdxMask();
    virtual ~IdxMask();
    virtual MVOID init(EModule_T, IDX_BASE_T*, MUINT16*);
    virtual IDXMGR_QUERY_RESULT query(const CAM_IDX_QRY_COMB& qry);

private:
    MVOID genKey(std::vector<MUINT16>& factor);
    MINT32 compareKey(MUINT32* key_in, MUINT32* key_golden);

    EModule_T m_mod;
    MUINT16 dim_ns;
    MUINT16* dims;
    MUINT16* expand;
    MUINT32 entry_ns;
    MUINT32 key_sz;
    IDX_MASK_ENTRY* entry;
    MUINT16* acc_factor_ns;
    MUINT32* key;
};

class IdxDM : public IdxBase
{
public:
    IdxDM();
    virtual ~IdxDM();
    virtual MVOID init(EModule_T, IDX_BASE_T*, MUINT16*);
    virtual IDXMGR_QUERY_RESULT query(const CAM_IDX_QRY_COMB& qry);

private:

    EModule_T m_mod;
    MUINT16 dim_ns;
    MUINT16* dims;
    MUINT16* expand;
    MUINT16* acc_factor_ns;
    MUINT16* idx_array;
    MUINT16* scenarios;
};

typedef MUINT32 (IspTuningCustom::*mappingfn)(MUINT32, MVOID*);
#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

class IdxMgrImplBase
{
public:
    static IdxMgrImplBase* getInstance(MUINT32 eSensorDev);

    IdxMgrImplBase();
    virtual ~IdxMgrImplBase();

    virtual MVOID init(MVOID* const pNvram_Idx) = 0;
    virtual IDXMGR_QUERY_RESULT query(EModule_T mod, const CAM_IDX_QRY_COMB& qry) = 0;
    virtual MBOOL updateMapping_List(MUINT32 FrmId, const CAM_IDX_QRY_COMB& qry) = 0;
    virtual MBOOL updateMapping_List_By_Dim(MUINT32 FrmId, MUINT32 dim, MVOID* input) = 0;
    virtual MUINT32 addMappingFunction(MUINT32 dim, MUINT32 (IspTuningCustom::*function)(MUINT32, MVOID*), IspTuningCustom& tuning) = 0;
    virtual MBOOL getMapping_List(MUINT32 FrmId, CAM_IDX_QRY_COMB& qry) = 0;
    virtual MBOOL getMapping_List_Last(CAM_IDX_QRY_COMB& qry) = 0;

private:
    virtual MVOID clearMapping_List() = 0;
};

class IdxMgrImpl: public IdxMgrImplBase
{
public:
    static IdxMgrImplBase* getInstance(MUINT32 eSensorDev, MVOID* const pNvram_Idx);

    IdxMgrImpl();
    ~IdxMgrImpl();

    virtual MVOID init(MVOID* const pNvram_Idx);
    virtual IDXMGR_QUERY_RESULT query(EModule_T mod, const CAM_IDX_QRY_COMB& qry);
    virtual MBOOL updateMapping_List(MUINT32 FrmId, const CAM_IDX_QRY_COMB& qry);
    virtual MBOOL updateMapping_List_By_Dim(MUINT32 FrmId, MUINT32 dim, MVOID* input);
    virtual MUINT32 addMappingFunction(MUINT32 dim, MUINT32 (IspTuningCustom::*function)(MUINT32, MVOID*), IspTuningCustom& tuning);
    virtual MBOOL getMapping_List(MUINT32 FrmId, CAM_IDX_QRY_COMB& qry);
    virtual MBOOL getMapping_List_Last(CAM_IDX_QRY_COMB& qry);
    char* getScenarioPointer();
    MUINT16 getScenarioNs();

private:
    IdxBase* classFactory(EModule_T, IDX_BASE_T*, MUINT16*);
    virtual MVOID clearMapping_List();

    list<ISP_MAPPING_INFO_T> MappingInfoQ;
    list<ISP_MAPPING_INFO_T> guard;
    Mutex m_lock;
    IdxBase* m_pMobObj[EModule_NUM];
    MUINT16* m_pFactorNs;
    MUINT16 m_scenarioNs;
    char (*m_pScenarios)[][64];
    mappingfn m_funcs[EDim_NUM];
    IspTuningCustom* m_pIspTuningCustom;
};

class IdxMgr
{
public:
    static IdxMgr* createInstance(MUINT32 eSensorDev);

    IdxMgr();
    virtual ~IdxMgr();

    virtual MUINT16 query(MUINT32 eSensorDev, EModule_T mod, const CAM_IDX_QRY_COMB& qry, IDXMGR_QUERY_RESULT& result, const char* caller);
    virtual MUINT16 query(MUINT32 eSensorDev, EModule_T mod, const CAM_IDX_QRY_COMB& qry, IDXMGR_QUERY_RESULT& result);
    virtual MUINT16 query(MUINT32 eSensorDev, EModule_T mod, const CAM_IDX_QRY_COMB& qry, MUINT16& u2ScenarioIdx);
    virtual MUINT16 query(MUINT32 eSensorDev, EModule_T mod, const CAM_IDX_QRY_COMB& qry, const char* caller);
    virtual MUINT16 query(MUINT32 eSensorDev, EModule_T mod, MUINT32 FrmId);

    MVOID setMappingInfo(MUINT32 eSensorDev, const CAM_IDX_QRY_COMB& output , MUINT32 FrmId);
    MVOID setMappingInfoByDim(MUINT32 eSensorDev, MUINT32 FrmId, MUINT32 dim, MVOID* value);
    MVOID getMappingInfo(MUINT32 eSensorDev, CAM_IDX_QRY_COMB& output , MUINT32 FrmId);
    MVOID patchMappingInfo(MUINT32 eSensorDev, CAM_IDX_QRY_COMB& output, EDim_T dim, MUINT32* value);
    MUINT32 addMappingFunction(MUINT32 eSensorDev, MUINT32 dim, MUINT32 (IspTuningCustom::*function)(MUINT32, MVOID*), IspTuningCustom& tuning);
    MVOID getScenarioInfo(MUINT32 eSensorDev, char **p_scenarios, MUINT16 &scenarioNs);
    MVOID setValidFlag(NSIspTuning::ESensorDev_T eSensorDev);
private:
    IdxMgrImplBase* m_pIdxMgrImpBase[SENSOR_IDX_MAX];
    std::atomic<MBOOL> m_flag[SENSOR_IDX_MAX];
};

#endif
