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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "StereoTuningProvider_DVS_CTRL"

#include <mtkcam/utils/std/ULog.h>
#define CAM_ULOG_MODULE_ID MOD_MULTICAM_PROVIDER
#include "hw_dpe_dvs_ctrl_tuning.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

HW_DPE_DVS_CTRL_Tuning::~HW_DPE_DVS_CTRL_Tuning()
{
    for(auto &t : __tuningMap) {
        if(t.second) {
            delete t.second;
        }
    }
}

bool
HW_DPE_DVS_CTRL_Tuning::retrieveTuningParams(TuningQuery_T &query)
{
    NSCam::NSIoPipe::DPEConfig *dst = (NSCam::NSIoPipe::DPEConfig *)query.results[STEREO_TUNING_NAME[E_TUNING_HW_DPE]];

    if(NULL == dst) {
        MY_LOGE("Cannot get target");
        return false;
    }

    NSCam::NSIoPipe::DPEConfig *src = NULL;
    if(StereoSettingProvider::isActiveStereo() &&
       __tuningMap.find(QUERY_KEY_ACTIVE) != __tuningMap.end())
    {
        src = __tuningMap[QUERY_KEY_ACTIVE];
    }
    else if(__tuningMap.find(QUERY_KEY_PREVIEW) != __tuningMap.end())
    {
        src = __tuningMap[QUERY_KEY_PREVIEW];
    }
    else
    {
        src = __tuningMap.begin()->second;
    }

    if(NULL == src) {
        MY_LOGE("Cannot get source");
        return false;
    }

    dst->Dpe_engineSelect                                 = src->Dpe_engineSelect;
    dst->Dpe_is16BitMode                                  = src->Dpe_is16BitMode;
    dst->Dpe_DVSSettings.mainEyeSel                       = src->Dpe_DVSSettings.mainEyeSel;
    dst->Dpe_DVSSettings.frmWidth                         = src->Dpe_DVSSettings.frmWidth;
    dst->Dpe_DVSSettings.frmHeight                        = src->Dpe_DVSSettings.frmHeight;
    dst->Dpe_DVSSettings.engWidth                         = src->Dpe_DVSSettings.engWidth;
    dst->Dpe_DVSSettings.engHeight                        = src->Dpe_DVSSettings.engHeight;
    dst->Dpe_DVSSettings.engStart_x                       = src->Dpe_DVSSettings.engStart_x;
    dst->Dpe_DVSSettings.engStart_y                       = src->Dpe_DVSSettings.engStart_y;
    dst->Dpe_DVSSettings.occWidth                         = src->Dpe_DVSSettings.occWidth;
    dst->Dpe_DVSSettings.occStart_x                       = src->Dpe_DVSSettings.occStart_x;
    dst->Dpe_DVSSettings.haveDVInput                      = src->Dpe_DVSSettings.haveDVInput;
    dst->Dpe_DVSSettings.SubModule_EN.dvme_en             = src->Dpe_DVSSettings.SubModule_EN.dvme_en;
    dst->Dpe_DVSSettings.SubModule_EN.conf_en             = src->Dpe_DVSSettings.SubModule_EN.conf_en;
    dst->Dpe_DVSSettings.SubModule_EN.cv_en               = src->Dpe_DVSSettings.SubModule_EN.cv_en;
    dst->Dpe_DVSSettings.SubModule_EN.occ_en              = src->Dpe_DVSSettings.SubModule_EN.occ_en;
    dst->Dpe_DVSSettings.Iteration.y_IterTimes            = src->Dpe_DVSSettings.Iteration.y_IterTimes;
    dst->Dpe_DVSSettings.Iteration.y_IterStartDirect      = src->Dpe_DVSSettings.Iteration.y_IterStartDirect;
    dst->Dpe_DVSSettings.Iteration.x_IterTimes_Down       = src->Dpe_DVSSettings.Iteration.x_IterTimes_Down;
    dst->Dpe_DVSSettings.Iteration.x_IterTimes_Up         = src->Dpe_DVSSettings.Iteration.x_IterTimes_Up;
    dst->Dpe_DVSSettings.Iteration.x_IterStartDirect_Down = src->Dpe_DVSSettings.Iteration.x_IterStartDirect_Down;
    dst->Dpe_DVSSettings.Iteration.x_IterStartDirect_Up   = src->Dpe_DVSSettings.Iteration.x_IterStartDirect_Up;

    return true;
}

void
HW_DPE_DVS_CTRL_Tuning::_initFrameSize(NSCam::NSIoPipe::DVS_Settings *setting, ENUM_STEREO_SCENARIO scenario)
{
    StereoArea area = StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y, scenario);
    area *= 8;
    MSize contentSize = area.contentSize();
    setting->frmWidth    = area.size.w;
    setting->frmHeight   = area.size.h;
    setting->engWidth    = area.size.w - area.startPt.x;
    setting->engHeight   = area.size.h - area.startPt.y;
    setting->engStart_x  = area.startPt.x;
    setting->engStart_y  = area.startPt.y;
    setting->occWidth    = contentSize.w;
    setting->occStart_x  = area.startPt.x;
}

void
HW_DPE_DVS_CTRL_Tuning::_initDefaultValues()
{
    NSCam::NSIoPipe::DPEConfig *tuningBuffer = new NSCam::NSIoPipe::DPEConfig();

    tuningBuffer->Dpe_engineSelect            = MODE_DVS_DVP_BOTH;
    tuningBuffer->Dpe_is16BitMode             = 0;
    if(0 == StereoSettingProvider::getSensorRelativePosition()) {
        tuningBuffer->Dpe_DVSSettings.mainEyeSel = LEFT;
    } else {
        tuningBuffer->Dpe_DVSSettings.mainEyeSel = RIGHT;
    }

    tuningBuffer->Dpe_DVSSettings.haveDVInput = 0;
    _initFrameSize(&(tuningBuffer->Dpe_DVSSettings), eSTEREO_SCENARIO_PREVIEW);

    tuningBuffer->Dpe_DVSSettings.SubModule_EN.dvme_en             = 1;
    tuningBuffer->Dpe_DVSSettings.SubModule_EN.conf_en             = 1;
    tuningBuffer->Dpe_DVSSettings.SubModule_EN.cv_en               = 1;
    tuningBuffer->Dpe_DVSSettings.SubModule_EN.occ_en              = 1;
    tuningBuffer->Dpe_DVSSettings.Iteration.y_IterTimes            = 2;
    tuningBuffer->Dpe_DVSSettings.Iteration.y_IterStartDirect      = 0;
    tuningBuffer->Dpe_DVSSettings.Iteration.x_IterTimes_Down       = 2;
    tuningBuffer->Dpe_DVSSettings.Iteration.x_IterTimes_Up         = 2;
    tuningBuffer->Dpe_DVSSettings.Iteration.x_IterStartDirect_Down = 0;
    tuningBuffer->Dpe_DVSSettings.Iteration.x_IterStartDirect_Up   = 0;

    __tuningMap[QUERY_KEY_PREVIEW] = tuningBuffer;
}

void
HW_DPE_DVS_CTRL_Tuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger.FastLogD("===== DVS CTRL Parameters =====");
    for(auto &tuning : __tuningMap) {
        NSCam::NSIoPipe::DPEConfig *tuningBuffer = tuning.second;
        logger
        .FastLogD("Scenario               %s", tuning.first.c_str())
        .FastLogD("Dpe_engineSelect       %d", tuningBuffer->Dpe_engineSelect)
        .FastLogD("Dpe_is16BitMode        %d", tuningBuffer->Dpe_is16BitMode)
        .FastLogD("mainEyeSel             %d", tuningBuffer->Dpe_DVSSettings.mainEyeSel)
        .FastLogD("frmWidth               %d", tuningBuffer->Dpe_DVSSettings.frmWidth)
        .FastLogD("frmHeight              %d", tuningBuffer->Dpe_DVSSettings.frmHeight)
        .FastLogD("engWidth               %d", tuningBuffer->Dpe_DVSSettings.engWidth)
        .FastLogD("engHeight              %d", tuningBuffer->Dpe_DVSSettings.engHeight)
        .FastLogD("engStart_x             %d", tuningBuffer->Dpe_DVSSettings.engStart_x)
        .FastLogD("engStart_y             %d", tuningBuffer->Dpe_DVSSettings.engStart_y)
        .FastLogD("occWidth               %d", tuningBuffer->Dpe_DVSSettings.occWidth)
        .FastLogD("occStart_x             %d", tuningBuffer->Dpe_DVSSettings.occStart_x)
        .FastLogD("haveDVInput            %d", tuningBuffer->Dpe_DVSSettings.haveDVInput)
        .FastLogD("dvme_en                %d", tuningBuffer->Dpe_DVSSettings.SubModule_EN.dvme_en)
        .FastLogD("conf_en                %d", tuningBuffer->Dpe_DVSSettings.SubModule_EN.conf_en)
        .FastLogD("cv_en                  %d", tuningBuffer->Dpe_DVSSettings.SubModule_EN.cv_en)
        .FastLogD("occ_en                 %d", tuningBuffer->Dpe_DVSSettings.SubModule_EN.occ_en)
        .FastLogD("y_IterTimes            %d", tuningBuffer->Dpe_DVSSettings.Iteration.y_IterTimes)
        .FastLogD("y_IterStartDirect      %d", tuningBuffer->Dpe_DVSSettings.Iteration.y_IterStartDirect)
        .FastLogD("x_IterTimes_Down       %d", tuningBuffer->Dpe_DVSSettings.Iteration.x_IterTimes_Down)
        .FastLogD("x_IterTimes_Up         %d", tuningBuffer->Dpe_DVSSettings.Iteration.x_IterTimes_Up)
        .FastLogD("x_IterStartDirect_Down %d", tuningBuffer->Dpe_DVSSettings.Iteration.x_IterStartDirect_Down)
        .FastLogD("x_IterStartDirect_Up   %d", tuningBuffer->Dpe_DVSSettings.Iteration.x_IterStartDirect_Up)
        .FastLogD("-------------------------------");
    }
    logger.print();
}

void
HW_DPE_DVS_CTRL_Tuning::_loadValuesFromDocument(const json& dpeValues)
{
    for(auto &tuning : dpeValues) {
        NSCam::NSIoPipe::DPEConfig *tuningBuffer = NULL;
        std::string scenarioKey = tuning[VALUE_KEY_PARAMETERS][QUERY_KEY_SCENARIO].get<std::string>();
        if(__tuningMap.find(scenarioKey) == __tuningMap.end()) {
            tuningBuffer = new NSCam::NSIoPipe::DPEConfig();
            __tuningMap[scenarioKey] = tuningBuffer;
        } else {
            tuningBuffer = __tuningMap[scenarioKey];
        }

        ENUM_STEREO_SCENARIO scenario = eSTEREO_SCENARIO_PREVIEW;
        // std::transform(scenarioKey.begin(), scenarioKey.end(), scenarioKey.begin(), ::tolower);
        // if(std::string::npos != scenarioKey.find("cap")) {
        //     scenario = eSTEREO_SCENARIO_CAPTURE;
        // }

        const json &tuningValues = tuning[VALUE_KEY_VALUES];
        if(LogicalCamJSONUtil::HasMember(tuningValues, "mainEyeSel")) {
            tuningBuffer->Dpe_DVSSettings.mainEyeSel = static_cast<DPE_MAINEYE_SEL>(_getInt(tuningValues, "mainEyeSel"));
        } else {
            if(0 == StereoSettingProvider::getSensorRelativePosition()) {
                tuningBuffer->Dpe_DVSSettings.mainEyeSel = LEFT;
            } else {
                tuningBuffer->Dpe_DVSSettings.mainEyeSel = RIGHT;
            }
        }

        tuningBuffer->Dpe_engineSelect = static_cast<DPEMODE>(_getInt(tuningValues, "Dpe_engineSelect"));
        tuningBuffer->Dpe_is16BitMode  = _getInt(tuningValues, "Dpe_is16BitMode");

        tuningBuffer->Dpe_DVSSettings.haveDVInput = _getInt(tuningValues, "haveDVInput");
        _initFrameSize(&(tuningBuffer->Dpe_DVSSettings), scenario);
        _getValue(tuningValues, "frmWidth",   tuningBuffer->Dpe_DVSSettings.frmWidth);
        _getValue(tuningValues, "frmHeight",  tuningBuffer->Dpe_DVSSettings.frmHeight);
        _getValue(tuningValues, "engStart_x", tuningBuffer->Dpe_DVSSettings.engStart_x);
        _getValue(tuningValues, "engStart_y", tuningBuffer->Dpe_DVSSettings.engStart_y);
        _getValue(tuningValues, "engWidth",   tuningBuffer->Dpe_DVSSettings.engWidth);
        _getValue(tuningValues, "engHeight",  tuningBuffer->Dpe_DVSSettings.engHeight);
        _getValue(tuningValues, "occWidth",   tuningBuffer->Dpe_DVSSettings.occWidth);
        _getValue(tuningValues, "occStart_x", tuningBuffer->Dpe_DVSSettings.occStart_x);

        tuningBuffer->Dpe_DVSSettings.SubModule_EN.dvme_en             = _getInt(tuningValues, "dvme_en");
        tuningBuffer->Dpe_DVSSettings.SubModule_EN.conf_en             = _getInt(tuningValues, "conf_en");
        tuningBuffer->Dpe_DVSSettings.SubModule_EN.cv_en               = _getInt(tuningValues, "cv_en");
        tuningBuffer->Dpe_DVSSettings.SubModule_EN.occ_en              = _getInt(tuningValues, "occ_en");
        tuningBuffer->Dpe_DVSSettings.Iteration.y_IterTimes            = _getInt(tuningValues, "y_IterTimes");
        tuningBuffer->Dpe_DVSSettings.Iteration.y_IterStartDirect      = _getInt(tuningValues, "y_IterStartDirect");
        tuningBuffer->Dpe_DVSSettings.Iteration.x_IterTimes_Down       = _getInt(tuningValues, "x_IterTimes_Down");
        tuningBuffer->Dpe_DVSSettings.Iteration.x_IterTimes_Up         = _getInt(tuningValues, "x_IterTimes_Up");
        tuningBuffer->Dpe_DVSSettings.Iteration.x_IterStartDirect_Down = _getInt(tuningValues, "x_IterStartDirect_Down");
        tuningBuffer->Dpe_DVSSettings.Iteration.x_IterStartDirect_Up   = _getInt(tuningValues, "x_IterStartDirect_Up");
    }
}
