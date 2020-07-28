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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#define LOG_TAG "{MfllShot}"

#include "../inc/MfllDebugExif.h"

#include <debug_exif/dbg_id_param.h>
#include <debug_exif/aaa/dbg_isp_param.h>
#include <debug_exif/cam/dbg_cam_param.h> // debug exif for CAM

#define MY_LOGI(fmt, arg...)    do {ALOGI(LOG_TAG fmt, ##arg);} while (0)
#define MY_LOGD(fmt, arg...)    do {ALOGD(LOG_TAG fmt, ##arg);} while (0)
#define MY_LOGE(fmt, arg...)    do {ALOGE("error" LOG_TAG fmt, ##arg);} while (0)

using namespace NSIspExifDebug;

// ----------------------------------------------------------------------------
// helpers
// ----------------------------------------------------------------------------

// bind debug key ID to type
template<typename T> uint32_t get_key_id()          { MY_LOGE("Un-bind type, cannot get ID"); return 0; }
template<> uint32_t get_key_id<DEBUG_MF_INFO_T>()   { return DEBUG_CAM_MF_MID; }
template<> uint32_t get_key_id<IspExifDebugInfo_T>(){ return ISP_DEBUG_KEYID; }

template <typename T>
T* convert_dbgcontainer_to_tag(IDbgInfoContainer* dbg)
{
    size_t blockSize = 0;
    auto blockAddr = dbg->queryMemoryIfExist(get_key_id<T>(), blockSize);

    if (blockAddr == nullptr) {
        MY_LOGE("No debug info block");
        return nullptr;
    }

    if (blockSize != sizeof(T)) {
        MY_LOGD("Size of blockSize doesn't match");
        return nullptr;
    }

    return reinterpret_cast<T*>(blockAddr);
}


inline void set_debug_tag(DEBUG_MF_INFO_T* pMfDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    pMfDebugInfo->Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_CMN_MID, a_i4ID, 0);
    pMfDebugInfo->Tag[a_i4ID].u4FieldValue = a_i4Value;
}

// ----------------------------------------------------------------------------

MfllDebugExif::MfllDebugExif()
    : m_dbgContainer(nullptr)
    , m_dbgInfo(nullptr)
{
    m_dbgContainer = IDbgInfoContainer::createInstance();
    // create memory block of DEBUG_MF_INFO_T
    if (m_dbgContainer) {
        m_dbgInfo = m_dbgContainer->queryMemory(
                get_key_id<DEBUG_MF_INFO_T>(), sizeof(DEBUG_MF_INFO_T), MTRUE);
    }
}


MfllDebugExif::~MfllDebugExif()
{
    m_dbgInfo = nullptr;
    if (m_dbgContainer) m_dbgContainer->destroyInstance();
}


bool MfllDebugExif::setData(MINT32 tag, MINT32 data)
{
    if (m_dbgInfo == nullptr) {
        MY_LOGE("setData failed due to no DEBUG_MF_INFO_T memory chunk");
        return false;
    }

    set_debug_tag(reinterpret_cast<DEBUG_MF_INFO_T*>(m_dbgInfo), tag, data);
    return true;
}


bool MfllDebugExif::setData(const IDbgInfoContainer* dbg, STAGE s)
{
    if (dbg == nullptr) {
        MY_LOGE("argument is NULL");
        return false;
    }

    if (m_dbgInfo == nullptr) {
        MY_LOGE("m_dbgInfo is NULL");
        return false;
    }

    // only valid stage for BFBLD, SF, MFB, AFBLD
    if ((s == STAGE_BFBLD) || (s == STAGE_SF) || (s == STAGE_MFB) || (s == STAGE_AFBLD))
        goto LABEL_OK;

    return false;


LABEL_OK:
    // retrieve IspExifDebugInfo_T from the container
    auto pIspExifDbgInfo = convert_dbgcontainer_to_tag<IspExifDebugInfo_T>(
            const_cast<IDbgInfoContainer*>(dbg));

    if (pIspExifDbgInfo == nullptr) {
        MY_LOGE("No debug info(IspExifDebugInfo_T) in container");
        return false;
    }

    // get registers debug tables
    auto pIspRegsInfo = &pIspExifDbgInfo->debugInfo;
    auto pMfDebugInfo = reinterpret_cast<DEBUG_MF_INFO_T*>(m_dbgInfo);



#define __VAR(T)                (pIspRegsInfo->tags[T].u4Val)
#define __MAKE_TAG(prefix, tag) prefix##_##tag
#define __ASSIGN(P, V)          do{set_debug_tag(pMfDebugInfo, __MAKE_TAG(P,V) ,__VAR(V));}while(0)
    // e.g.: __ASSIGN(MF_TAG_BASE, VAL0) represents:
    //
    // set_debug_tag(pMfDebugInfo, MF_TAG_BASE_VAL0, pIspRegsInfo->tags[VAL0].u4Val
    //
    switch (s) {
    case STAGE_SF:
        #define __MF_TAG  MF_TAG_BASE
        // {{{
        __ASSIGN(__MF_TAG, CAM_ANR_CON1               );
        __ASSIGN(__MF_TAG, CAM_ANR_CON2               );
        __ASSIGN(__MF_TAG, CAM_ANR_YAD1               );
        __ASSIGN(__MF_TAG, CAM_ANR_YAD2               );
        __ASSIGN(__MF_TAG, CAM_ANR_Y4LUT1             );
        __ASSIGN(__MF_TAG, CAM_ANR_Y4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_Y4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_C4LUT1             );
        __ASSIGN(__MF_TAG, CAM_ANR_C4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_C4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_A4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_A4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_L4LUT1             );
        __ASSIGN(__MF_TAG, CAM_ANR_L4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_L4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_PTY                );
        __ASSIGN(__MF_TAG, CAM_ANR_CAD                );
        __ASSIGN(__MF_TAG, CAM_ANR_PTC                );
        __ASSIGN(__MF_TAG, CAM_ANR_LCE                );
        __ASSIGN(__MF_TAG, CAM_ANR_MED1               );
        __ASSIGN(__MF_TAG, CAM_ANR_MED2               );
        __ASSIGN(__MF_TAG, CAM_ANR_MED3               );
        __ASSIGN(__MF_TAG, CAM_ANR_MED4               );
        __ASSIGN(__MF_TAG, CAM_ANR_HP1                );
        __ASSIGN(__MF_TAG, CAM_ANR_HP2                );
        __ASSIGN(__MF_TAG, CAM_ANR_HP3                );
        __ASSIGN(__MF_TAG, CAM_ANR_ACT1               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACT2               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACT3               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACTY               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACTC               );
        // ANR2
        __ASSIGN(__MF_TAG, CAM_ANR2_CON1              );
        __ASSIGN(__MF_TAG, CAM_ANR2_CON2              );
        __ASSIGN(__MF_TAG, CAM_ANR2_YAD1              );
        __ASSIGN(__MF_TAG, CAM_ANR2_Y4LUT1            );
        __ASSIGN(__MF_TAG, CAM_ANR2_Y4LUT2            );
        __ASSIGN(__MF_TAG, CAM_ANR2_Y4LUT3            );
        __ASSIGN(__MF_TAG, CAM_ANR2_L4LUT1            );
        __ASSIGN(__MF_TAG, CAM_ANR2_L4LUT2            );
        __ASSIGN(__MF_TAG, CAM_ANR2_L4LUT3            );
        __ASSIGN(__MF_TAG, CAM_ANR2_CAD               );
        __ASSIGN(__MF_TAG, CAM_ANR2_PTC               );
        __ASSIGN(__MF_TAG, CAM_ANR2_LCE               );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED1              );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED2              );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED3              );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED4              );
        __ASSIGN(__MF_TAG, CAM_ANR2_ACTY              );
        __ASSIGN(__MF_TAG, CAM_ANR2_ACTC              );
        // SEEE
        __ASSIGN(__MF_TAG, CAM_SEEE_SRK_CTRL          );
        __ASSIGN(__MF_TAG, CAM_SEEE_CLIP_CTRL         );
        __ASSIGN(__MF_TAG, CAM_SEEE_FLT_CTRL_1        );
        __ASSIGN(__MF_TAG, CAM_SEEE_FLT_CTRL_2        );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_01      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_02      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_03      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_04      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_05      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_06      );
        __ASSIGN(__MF_TAG, CAM_SEEE_EDTR_CTRL         );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_07      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_08      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_09      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_10      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_11      );
        __ASSIGN(__MF_TAG, CAM_SEEE_OUT_EDGE_CTRL     );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_Y_CTRL         );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_EDGE_CTRL_1    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_EDGE_CTRL_2    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_EDGE_CTRL_3    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_SPECL_CTRL     );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_CORE_CTRL_1    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_CORE_CTRL_2    );
        //}}}
        #undef __MF_TAG
        break;

    case STAGE_BFBLD:
        #define __MF_TAG  MF_TAG_BEFORE_MFB
        // {{{
        __ASSIGN(__MF_TAG, CAM_ANR_CON1               );
        __ASSIGN(__MF_TAG, CAM_ANR_CON2               );
        __ASSIGN(__MF_TAG, CAM_ANR_YAD1               );
        __ASSIGN(__MF_TAG, CAM_ANR_YAD2               );
        __ASSIGN(__MF_TAG, CAM_ANR_Y4LUT1             );
        __ASSIGN(__MF_TAG, CAM_ANR_Y4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_Y4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_C4LUT1             );
        __ASSIGN(__MF_TAG, CAM_ANR_C4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_C4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_A4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_A4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_L4LUT1             );
        __ASSIGN(__MF_TAG, CAM_ANR_L4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_L4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_PTY                );
        __ASSIGN(__MF_TAG, CAM_ANR_CAD                );
        __ASSIGN(__MF_TAG, CAM_ANR_PTC                );
        __ASSIGN(__MF_TAG, CAM_ANR_LCE                );
        __ASSIGN(__MF_TAG, CAM_ANR_MED1               );
        __ASSIGN(__MF_TAG, CAM_ANR_MED2               );
        __ASSIGN(__MF_TAG, CAM_ANR_MED3               );
        __ASSIGN(__MF_TAG, CAM_ANR_MED4               );
        __ASSIGN(__MF_TAG, CAM_ANR_HP1                );
        __ASSIGN(__MF_TAG, CAM_ANR_HP2                );
        __ASSIGN(__MF_TAG, CAM_ANR_HP3                );
        __ASSIGN(__MF_TAG, CAM_ANR_ACT1               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACT2               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACT3               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACTY               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACTC               );
        // ANR2
        __ASSIGN(__MF_TAG, CAM_ANR2_CON1              );
        __ASSIGN(__MF_TAG, CAM_ANR2_CON2              );
        __ASSIGN(__MF_TAG, CAM_ANR2_YAD1              );
        __ASSIGN(__MF_TAG, CAM_ANR2_Y4LUT1            );
        __ASSIGN(__MF_TAG, CAM_ANR2_Y4LUT2            );
        __ASSIGN(__MF_TAG, CAM_ANR2_Y4LUT3            );
        __ASSIGN(__MF_TAG, CAM_ANR2_L4LUT1            );
        __ASSIGN(__MF_TAG, CAM_ANR2_L4LUT2            );
        __ASSIGN(__MF_TAG, CAM_ANR2_L4LUT3            );
        __ASSIGN(__MF_TAG, CAM_ANR2_CAD               );
        __ASSIGN(__MF_TAG, CAM_ANR2_PTC               );
        __ASSIGN(__MF_TAG, CAM_ANR2_LCE               );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED1              );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED2              );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED3              );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED4              );
        __ASSIGN(__MF_TAG, CAM_ANR2_ACTY              );
        __ASSIGN(__MF_TAG, CAM_ANR2_ACTC              );
        // SEEE
        __ASSIGN(__MF_TAG, CAM_SEEE_SRK_CTRL          );
        __ASSIGN(__MF_TAG, CAM_SEEE_CLIP_CTRL         );
        __ASSIGN(__MF_TAG, CAM_SEEE_FLT_CTRL_1        );
        __ASSIGN(__MF_TAG, CAM_SEEE_FLT_CTRL_2        );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_01      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_02      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_03      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_04      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_05      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_06      );
        __ASSIGN(__MF_TAG, CAM_SEEE_EDTR_CTRL         );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_07      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_08      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_09      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_10      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_11      );
        __ASSIGN(__MF_TAG, CAM_SEEE_OUT_EDGE_CTRL     );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_Y_CTRL         );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_EDGE_CTRL_1    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_EDGE_CTRL_2    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_EDGE_CTRL_3    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_SPECL_CTRL     );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_CORE_CTRL_1    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_CORE_CTRL_2    );
        //}}}
        #undef __MF_TAG
        break;

    case STAGE_AFBLD:
        #define __MF_TAG  MF_TAG_AFTER_MFB
        // {{{
        __ASSIGN(__MF_TAG, CAM_ANR_CON1               );
        __ASSIGN(__MF_TAG, CAM_ANR_CON2               );
        __ASSIGN(__MF_TAG, CAM_ANR_YAD1               );
        __ASSIGN(__MF_TAG, CAM_ANR_YAD2               );
        __ASSIGN(__MF_TAG, CAM_ANR_Y4LUT1             );
        __ASSIGN(__MF_TAG, CAM_ANR_Y4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_Y4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_C4LUT1             );
        __ASSIGN(__MF_TAG, CAM_ANR_C4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_C4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_A4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_A4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_L4LUT1             );
        __ASSIGN(__MF_TAG, CAM_ANR_L4LUT2             );
        __ASSIGN(__MF_TAG, CAM_ANR_L4LUT3             );
        __ASSIGN(__MF_TAG, CAM_ANR_PTY                );
        __ASSIGN(__MF_TAG, CAM_ANR_CAD                );
        __ASSIGN(__MF_TAG, CAM_ANR_PTC                );
        __ASSIGN(__MF_TAG, CAM_ANR_LCE                );
        __ASSIGN(__MF_TAG, CAM_ANR_MED1               );
        __ASSIGN(__MF_TAG, CAM_ANR_MED2               );
        __ASSIGN(__MF_TAG, CAM_ANR_MED3               );
        __ASSIGN(__MF_TAG, CAM_ANR_MED4               );
        __ASSIGN(__MF_TAG, CAM_ANR_HP1                );
        __ASSIGN(__MF_TAG, CAM_ANR_HP2                );
        __ASSIGN(__MF_TAG, CAM_ANR_HP3                );
        __ASSIGN(__MF_TAG, CAM_ANR_ACT1               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACT2               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACT3               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACTY               );
        __ASSIGN(__MF_TAG, CAM_ANR_ACTC               );
        // ANR2
        __ASSIGN(__MF_TAG, CAM_ANR2_CON1              );
        __ASSIGN(__MF_TAG, CAM_ANR2_CON2              );
        __ASSIGN(__MF_TAG, CAM_ANR2_YAD1              );
        __ASSIGN(__MF_TAG, CAM_ANR2_Y4LUT1            );
        __ASSIGN(__MF_TAG, CAM_ANR2_Y4LUT2            );
        __ASSIGN(__MF_TAG, CAM_ANR2_Y4LUT3            );
        __ASSIGN(__MF_TAG, CAM_ANR2_L4LUT1            );
        __ASSIGN(__MF_TAG, CAM_ANR2_L4LUT2            );
        __ASSIGN(__MF_TAG, CAM_ANR2_L4LUT3            );
        __ASSIGN(__MF_TAG, CAM_ANR2_CAD               );
        __ASSIGN(__MF_TAG, CAM_ANR2_PTC               );
        __ASSIGN(__MF_TAG, CAM_ANR2_LCE               );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED1              );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED2              );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED3              );
        __ASSIGN(__MF_TAG, CAM_ANR2_MED4              );
        __ASSIGN(__MF_TAG, CAM_ANR2_ACTY              );
        __ASSIGN(__MF_TAG, CAM_ANR2_ACTC              );
        // SEEE
        __ASSIGN(__MF_TAG, CAM_SEEE_SRK_CTRL          );
        __ASSIGN(__MF_TAG, CAM_SEEE_CLIP_CTRL         );
        __ASSIGN(__MF_TAG, CAM_SEEE_FLT_CTRL_1        );
        __ASSIGN(__MF_TAG, CAM_SEEE_FLT_CTRL_2        );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_01      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_02      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_03      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_04      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_05      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_06      );
        __ASSIGN(__MF_TAG, CAM_SEEE_EDTR_CTRL         );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_07      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_08      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_09      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_10      );
        __ASSIGN(__MF_TAG, CAM_SEEE_GLUT_CTRL_11      );
        __ASSIGN(__MF_TAG, CAM_SEEE_OUT_EDGE_CTRL     );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_Y_CTRL         );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_EDGE_CTRL_1    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_EDGE_CTRL_2    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_EDGE_CTRL_3    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_SPECL_CTRL     );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_CORE_CTRL_1    );
        __ASSIGN(__MF_TAG, CAM_SEEE_SE_CORE_CTRL_2    );

        //}}}
        // MIXER3
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON2            );
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON3            );
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON4            );
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON5            );
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON6            );
        __ASSIGN(__MF_TAG, CAM_MIX3_CTRL_0            );
        __ASSIGN(__MF_TAG, CAM_MIX3_CTRL_1            );
        __ASSIGN(__MF_TAG, CAM_MIX3_SPARE             );
        #undef __MF_TAG
        break;

    case STAGE_MFB:
        #define __MF_TAG  MF_TAG_IN_MFB
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON2            );
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON3            );
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON4            );
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON5            );
        __ASSIGN(__MF_TAG, CAM_MFB_LL_CON6            );
        #undef __MF_TAG
        break;

    default:; // do nothing
    }
#undef __ASSIGN
#undef __MAKE_TAG
#undef __VAR

    return true;
}
