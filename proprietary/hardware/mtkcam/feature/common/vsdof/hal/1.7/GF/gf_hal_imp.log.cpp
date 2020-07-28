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
    //__initInfo
    FAST_LOGD("[GF Mode]      %d", __initInfo.gfMode);
    FAST_LOGD("[InputWidth]   %d", __initInfo.inputWidth);
    FAST_LOGD("[InputHeight]  %d", __initInfo.inputHeight);
    //
    FAST_LOGD("[OutputWidth]  %d", __initInfo.outputWidth);
    FAST_LOGD("[OutputHeight] %d", __initInfo.outputHeight);
    //=== Init tuning info ===
    FAST_LOGD("[CoreNumber]   %d", __initInfo.pTuningInfo->coreNumber);

    std::ostringstream oss;
    for(int i = 0; i < __initInfo.pTuningInfo->clrTblSize; ++i) {
        oss << __initInfo.pTuningInfo->clrTbl[i] << " ";
    }
    FAST_LOGD("[Tuning Table] %s", oss.str().c_str());

    oss.clear();
    oss.str("");
    for(int i = 0; i < __initInfo.pTuningInfo->ctrlPointNum; ++i) {
        oss << __initInfo.pTuningInfo->dispCtrlPoints[i] << " ";
    }
    FAST_LOGD("[dispCtrlPoints] %s", oss.str().c_str());

    oss.clear();
    oss.str("");
    for(int i = 0; i < __initInfo.pTuningInfo->ctrlPointNum; ++i) {
        oss << __initInfo.pTuningInfo->blurGainTable[i] << " ";
    }
    FAST_LOGD("[blurGainTable] %s", oss.str().c_str());

#ifdef GF_CUSTOM_PARAM
    FAST_LOGD("[TuningInfo.NumOfParam]    %d", __initInfo.pTuningInfo->NumOfParam);
    for(MUINT32 j = 0; j < __initInfo.pTuningInfo->NumOfParam; ++j) {
        FAST_LOGD("[TuningInfo.params][%d]     %s: %d", j,
                  __initInfo.pTuningInfo->params[j].key,
                  __initInfo.pTuningInfo->params[j].value);
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
    //__procInfo
    FAST_LOGD("[Magic]        %d", __magicNumber);
    FAST_LOGD("[Request]      %d", __requestNumber);
    FAST_LOGD("[TouchTrigger] %d", __procInfo.touchTrigger);
    FAST_LOGD("[TouchPos]     (%d, %d)", __procInfo.touchX, __procInfo.touchY);
    FAST_LOGD("[DoF Level]    %d", __procInfo.dof);
    FAST_LOGD("[DAC]          %d (%d-%d)", __procInfo.dacInfo.cur, __procInfo.dacInfo.min, __procInfo.dacInfo.max);
    FAST_LOGD("[FD]           %d Ratio %.2f", __procInfo.fdInfo.isFd, __procInfo.fdInfo.ratio);
    FAST_LOGD("[Depth Value]  %d", __procInfo.depthValue);
    FAST_LOGD("[ConvOffset]   %f", __procInfo.cOffset);
    FAST_LOGD("[FB]           %f", __procInfo.fb);
    string focusType;
    switch(__procInfo.afInfo.afType) {
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
    FAST_LOGD("[Focus Type]   %d(%s)", __procInfo.afInfo.afType, focusType.c_str());
    FAST_LOGD("[Focus ROI]    (%d, %d) (%d, %d)", __procInfo.afInfo.x1,
                                                  __procInfo.afInfo.y1,
                                                  __procInfo.afInfo.x2,
                                                  __procInfo.afInfo.y2);

    for(unsigned int i = 0; i < __procInfo.numOfBuffer; i++) {
        __logGFBufferInfo(__procInfo.bufferInfo[i], (int)i);
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
    //__resultInfo
    FAST_LOGD("[Return code] %d", __resultInfo.RetCode);
    for(unsigned int i = 0; i < __resultInfo.numOfBuffer; i++) {
        __logGFBufferInfo(__resultInfo.bufferInfo[i], (int)i);
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
