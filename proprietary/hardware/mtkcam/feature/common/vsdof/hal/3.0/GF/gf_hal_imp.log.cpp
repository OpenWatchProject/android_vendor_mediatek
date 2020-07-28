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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "gf_hal_imp.h"

void
GF_HAL_IMP::__logInitData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= GF Init Info =========");
    //m_initInfo
    FAST_LOGD("[GF Mode]      %d", m_initInfo.gfMode);
    FAST_LOGD("[InputWidth]   %d", m_initInfo.inputWidth);
    FAST_LOGD("[InputHeight]  %d", m_initInfo.inputHeight);
    //
    FAST_LOGD("[OutputWidth]  %d", m_initInfo.outputWidth);
    FAST_LOGD("[OutputHeight] %d", m_initInfo.outputHeight);
    //=== Init tuning info ===
    FAST_LOGD("[CoreNumber]   %d", m_initInfo.pTuningInfo->coreNumber);

    std::ostringstream oss;
    for(int i = 0; i < m_initInfo.pTuningInfo->clrTblSize; ++i) {
        oss << m_initInfo.pTuningInfo->clrTbl[i] << " ";
    }
    FAST_LOGD("[Tuning Table] %s", oss.str().c_str());

    oss.clear();
    oss.str("");
    for(int i = 0; i < m_initInfo.pTuningInfo->ctrlPointNum; ++i) {
        oss << m_initInfo.pTuningInfo->dispCtrlPoints[i] << " ";
    }
    FAST_LOGD("[dispCtrlPoints] %s", oss.str().c_str());

    oss.clear();
    oss.str("");
    for(int i = 0; i < m_initInfo.pTuningInfo->ctrlPointNum; ++i) {
        oss << m_initInfo.pTuningInfo->blurGainTable[i] << " ";
    }
    FAST_LOGD("[blurGainTable] %s", oss.str().c_str());

#ifdef GF_CUSTOM_PARAM
    FAST_LOGD("[TuningInfo.NumOfParam]    %d", m_initInfo.pTuningInfo->NumOfParam);
    for(MUINT32 j = 0; j < m_initInfo.pTuningInfo->NumOfParam; ++j) {
        FAST_LOGD("[TuningInfo.params][%d]     %s: %d", j,
                  m_initInfo.pTuningInfo->params[j].key,
                  m_initInfo.pTuningInfo->params[j].value);
    }
#endif

    FAST_LOG_PRINT;
}

void
GF_HAL_IMP::__logSetProcData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= GF Proc Data =========");
    //m_procInfo
    FAST_LOGD("[Magic]        %d", __magicNumber);
    FAST_LOGD("[Request]      %d", __requestNumber);
    FAST_LOGD("[TouchTrigger] %d", m_procInfo.touchTrigger);
    FAST_LOGD("[TouchPos]     (%d, %d)", m_procInfo.touchX, m_procInfo.touchY);
    FAST_LOGD("[DoF Level]    %d", m_procInfo.dof);
    FAST_LOGD("[DAC]          %d (%d-%d)", m_procInfo.dacInfo.cur, m_procInfo.dacInfo.min, m_procInfo.dacInfo.max);
    FAST_LOGD("[FD]           %d Ratio %.2f", m_procInfo.fdInfo.isFd, m_procInfo.fdInfo.ratio);
    FAST_LOGD("[Depth Value]  %d", m_procInfo.depthValue);
    FAST_LOGD("[ConvOffset]   %f", m_procInfo.cOffset);
    FAST_LOGD("[FB]           %f", m_procInfo.fb);
    string focusType;
    switch(m_procInfo.afInfo.afType) {
    case GF_AF_NONE:
        focusType = "None";
        break;
    case GF_AF_AP:
        focusType = "AP";
        break;
    case GF_AF_OT:
        focusType = "OT";
        break;
    case GF_AF_FD:
        focusType = "FD";
        break;
    case GF_AF_CENTER:
        focusType = "Center";
        break;
    case GF_AF_DEFAULT:
    default:
        focusType = "Default";
        break;
    }
    FAST_LOGD("[Focus Type]   %d(%s)", m_procInfo.afInfo.afType, focusType.c_str());
    FAST_LOGD("[Focus ROI]    (%d, %d) (%d, %d)", m_procInfo.afInfo.x1,
                                                  m_procInfo.afInfo.y1,
                                                  m_procInfo.afInfo.x2,
                                                  m_procInfo.afInfo.y2);

    for(unsigned int i = 0; i < m_procInfo.numOfBuffer; i++) {
        __logGFBufferInfo(m_procInfo.bufferInfo[i], (int)i);
    }

    FAST_LOG_PRINT;
}

void
GF_HAL_IMP::__logGFResult()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= GF Result =========");
    //m_resultInfo
    FAST_LOGD("[Return code] %d", m_resultInfo.RetCode);
    for(unsigned int i = 0; i < m_resultInfo.numOfBuffer; i++) {
        __logGFBufferInfo(m_resultInfo.bufferInfo[i], (int)i);
    }

    FAST_LOG_PRINT;
}

void
GF_HAL_IMP::__logGFBufferInfo(const GFBufferInfo &buf, int index)
{
    if(!LOG_ENABLED) {
        return;
    }

    if(index >= 0) {
        FAST_LOGD("[Buffer %d][Type]          %d", index, buf.type);
        FAST_LOGD("[Buffer %d][Format]        %d", index, buf.format);
        FAST_LOGD("[Buffer %d][Width]         %d", index, buf.width);
        FAST_LOGD("[Buffer %d][Height]        %d", index, buf.height);
        FAST_LOGD("[Buffer %d][PlaneAddr0]    %p", index, buf.planeAddr0);
        FAST_LOGD("[Buffer %d][PlaneAddr1]    %p", index, buf.planeAddr1);
        FAST_LOGD("[Buffer %d][PlaneAddr2]    %p", index, buf.planeAddr2);
        FAST_LOGD("[Buffer %d][PlaneAddr3]    %p", index, buf.planeAddr3);
    } else {
        FAST_LOGD("[Type]          %d", buf.type);
        FAST_LOGD("[Format]        %d", buf.format);
        FAST_LOGD("[Width]         %d", buf.width);
        FAST_LOGD("[Height]        %d", buf.height);
        FAST_LOGD("[PlaneAddr0]    %p", buf.planeAddr0);
        FAST_LOGD("[PlaneAddr1]    %p", buf.planeAddr1);
        FAST_LOGD("[PlaneAddr2]    %p", buf.planeAddr2);
        FAST_LOGD("[PlaneAddr3]    %p", buf.planeAddr3);
    }
}
