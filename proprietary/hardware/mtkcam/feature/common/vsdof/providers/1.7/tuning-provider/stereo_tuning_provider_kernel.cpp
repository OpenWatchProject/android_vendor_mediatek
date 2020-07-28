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
#define LOG_TAG "StereoTuningProviderKernel"

#include "stereo_tuning_provider_kernel.h"
#include <mtkcam/utils/std/Log.h>
#include <string.h>

#include <camera_custom_stereo.h>   //For DEFAULT_STEREO_TUNING

#include "tunings/hw_mdp_pq_tuning.h"

#if (1==VSDOF_SUPPORTED)
#include "tunings/hw_dpe_tuning.h"
#include "tunings/hw_wmf_tuning.h"
#include "tunings/hw_fe_tuning.h"
#include "tunings/hw_fm_tuning.h"
#include "tunings/hw_bokeh_tuning.h"
#include "tunings/hw_mdp_pq_tuning.h"

#include "tunings/sw_n3d_tuning.h"
#include "tunings/sw_gf_tuning.h"
#include "tunings/sw_occ_tuning.h"
#include "tunings/sw_bokeh_tuning.h"
#endif

#include "../setting-provider/stereo_setting_provider_kernel.h"

Mutex StereoTuningProviderKernel::__instanceLock;
StereoTuningProviderKernel * StereoTuningProviderKernel::__instance = NULL;

#define TUNING_HEADER_PATH      "/sdcard/camera_custom_stereo_tuning.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

StereoTuningProviderKernel *
StereoTuningProviderKernel::getInstance()
{
    Mutex::Autolock lock(__instanceLock);

    if(NULL == __instance) {
        __instance = new StereoTuningProviderKernel();
    }

    return __instance;
}

void
StereoTuningProviderKernel::destroyInstance()
{
    Mutex::Autolock lock(__instanceLock);

    if(__instance) {
        delete __instance;
        __instance = NULL;
    }
}

StereoTuningProviderKernel::StereoTuningProviderKernel()
    : __logger(LOG_TAG, PERPERTY_TUNING_PROVIDER_LOG)
{
    __logger.setSingleLineMode(0);
}

StereoTuningProviderKernel::~StereoTuningProviderKernel()
{
    //Release tuning instances
    for(auto const &element : __tuningHolderMap)
    {
        if(element.second) {
            delete element.second;
        } else {
            MY_LOGE("Cannot delete tuning of %s", STEREO_TUNING_NAME[element.first]);
        }
    }

    __tuningHolderMap.clear();
}

void
StereoTuningProviderKernel::init()
{
    const bool TUNING_ENABLED = (1 == checkStereoProperty(PROPERTY_ENABLE_TUNING));

    //Init __tuningHolderMap
    json loadedJson;
    int status = 0;
    {
        AutoProfileUtil profile(LOG_TAG, "parseStereoCustomTuning");
        status = LogicalCamJSONUtil::parseStereoCustomTuning(loadedJson);
        if(!status) {
            return;
        }

        if((1 == checkStereoProperty(PROPERTY_ENABLE_TUNING)) &&
           IS_JSON_FROM_DEFAULT(status))
        {
            __saveTuningToFile(loadedJson);
        }
    }

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    StereoSensorSetting_T *sensorSettings[2] =
        { StereoSettingProviderKernel::getInstance()->getSensorSetting(main1Idx),
          StereoSettingProviderKernel::getInstance()->getSensorSetting(main2Idx) };

    json tuningJson;
    if(NULL == sensorSettings[0] ||
       NULL == sensorSettings[1])
    {
        MY_LOGD("Unknown sensors, init tuning parameters with whole document");
        tuningJson = std::move(loadedJson);
    }
    else
    {
        std::string combinationKey = sensorSettings[0]->name + std::string("+") + sensorSettings[1]->name;
        // Check if tuning parameters exist for current profile
        if(LogicalCamJSONUtil::HasMember(loadedJson, combinationKey)) {
            MY_LOGD("Init tuning parameters of %s", combinationKey.c_str());
            tuningJson = std::move(loadedJson[combinationKey]);
        } else {
            MY_LOGD("Init tuning parameters with whole document");
            tuningJson = std::move(loadedJson);
        }
    }

    //Create map
    {
        AutoProfileUtil profile(LOG_TAG, "Init individual tunings");

        __tuningHolderMap[E_TUNING_HW_MDPPQ] = new HW_MDPPQTuning(tuningJson);
#if (1==VSDOF_SUPPORTED)
        __tuningHolderMap[E_TUNING_HW_DPE]   = new HW_DPETuning(tuningJson);
        __tuningHolderMap[E_TUNING_HW_WMF]   = new HW_WMFTuning(tuningJson);
        __tuningHolderMap[E_TUNING_HW_FE]    = new HW_FETuning(tuningJson);
        __tuningHolderMap[E_TUNING_HW_FM]    = new HW_FMTuning(tuningJson);
        __tuningHolderMap[E_TUNING_HW_BOKEH] = new HW_BOKEHTuning(tuningJson);

        __tuningHolderMap[E_TUNING_SW_N3D]   = new SW_N3DTuning(tuningJson);
        __tuningHolderMap[E_TUNING_SW_GF]    = new SW_GFTuning(tuningJson);
        __tuningHolderMap[E_TUNING_SW_OCC]   = new SW_OCCTuning(tuningJson);
        __tuningHolderMap[E_TUNING_SW_BOKEH] = new SW_BOKEHTuning(tuningJson);
#endif
    }

    //Log tuning parameters
    for(auto const &element : __tuningHolderMap)
    {
        if(element.second) {
            element.second->log(__logger);
        }
    }
}

bool
StereoTuningProviderKernel::getTuningParams(ENUM_STEREO_TUNING tuningID, TuningQuery_T &param)
{
    StereoTuningBase *holder = __tuningHolderMap[tuningID];
    if(NULL == holder) {
        MY_LOGE("Cannot get holder of %s", STEREO_TUNING_NAME[tuningID]);
        return false;
    }

    return holder->retrieveTuningParams(param);
}

void
StereoTuningProviderKernel::__saveTuningToFile(const json &loadedJson)
{
    AutoProfileUtil profile(LOG_TAG, "saveTuningToFile");

    //Save to json
    std::string s = loadedJson.dump(4);
    char *data = (char *)s.c_str();

    FILE *fp = fopen(STEREO_TUNING_FILE_PATH, "w");
    if(fp) {
        fwrite(data, 1, strlen(data), fp);

        fflush(fp);
        fclose(fp);
    }

    if(1 == checkStereoProperty(PROPERTY_EXPORT_TUNING)) {
        FILE *fp = fopen(TUNING_HEADER_PATH, "w");
        if(fp) {
            //Write copy right
            const char *COPY_RIGHT =
            "/* Copyright Statement:\n"
            " *\n"
            " * This software/firmware and related documentation (\"MediaTek Software\") are\n"
            " * protected under relevant copyright laws. The information contained herein\n"
            " * is confidential and proprietary to MediaTek Inc. and/or its licensors.\n"
            " * Without the prior written permission of MediaTek inc. and/or its licensors,\n"
            " * any reproduction, modification, use or disclosure of MediaTek Software,\n"
            " * and information contained herein, in whole or in part, shall be strictly prohibited.\n"
            " */\n"
            "/* MediaTek Inc. (C) 2018. All rights reserved.\n"
            " *\n"
            " * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES\n"
            " * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS (\"MEDIATEK SOFTWARE\")\n"
            " * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON\n"
            " * AN \"AS-IS\" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,\n"
            " * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF\n"
            " * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.\n"
            " * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE\n"
            " * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR\n"
            " * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH\n"
            " * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES\n"
            " * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES\n"
            " * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK\n"
            " * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR\n"
            " * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND\n"
            " * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,\n"
            " * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,\n"
            " * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO\n"
            " * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.\n"
            " *\n"
            " * The following software/firmware and/or related documentation (\"MediaTek Software\")\n"
            " * have been modified by MediaTek Inc. All revisions are subject to any receiver's\n"
            " * applicable license agreements with MediaTek Inc.\n"
            " */\n\n"
            "/********************************************************************************************\n"
            " *     LEGAL DISCLAIMER\n"
            " *\n"
            " *     (Header of MediaTek Software/Firmware Release or Documentation)\n"
            " *\n"
            " *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES\n"
            " *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS (\"MEDIATEK SOFTWARE\") RECEIVED\n"
            " *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN \"AS-IS\" BASIS\n"
            " *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,\n"
            " *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR\n"
            " *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY\n"
            " *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,\n"
            " *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK\n"
            " *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO\n"
            " *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION\n"
            " *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.\n"
            " *\n"
            " *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH\n"
            " *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,\n"
            " *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE\n"
            " *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.\n"
            " *\n"
            " *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS\n"
            " *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.\n"
            " ************************************************************************************************/\n"
            "#ifndef CAMERA_CUSTOM_STEREO_TUNING_H_\n"
            "#define CAMERA_CUSTOM_STEREO_TUNING_H_\n"
            "const char *DEFAULT_STEREO_TUNING =\n";
            fwrite(COPY_RIGHT, 1, strlen(COPY_RIGHT), fp);

            //Write JSON line-by-line
            char lineBuffer[1024];
            char *line = strtok(data, "\n");
            int lineSize;
            int i, j;
            bool isPreSpace = true;
            while(line) {
                lineSize = strlen(line);
                isPreSpace = true;
                // lineBuffer[0] = '\"';
                for(i = 0, j = 0; i < lineSize; ++i, ++j) {
                    if(isPreSpace &&
                       line[i] != ' ')
                    {
                        isPreSpace = false;
                        lineBuffer[j++] = '"';
                    }

                    if(line[i] == '\"') {
                        lineBuffer[j++] = '\\';
                    }

                    lineBuffer[j] = line[i];
                }
                line  = strtok(NULL, "\n");
                lineBuffer[j++] = '\"';

                if(NULL == line) {
                    lineBuffer[j++] = ';';
                }
                lineBuffer[j++] = '\n';

                fwrite(lineBuffer, 1, j, fp);
            }

            const char *LAST_LINE = "#endif\n";
            size_t LAST_LINE_SIZE = strlen(LAST_LINE);
            ::memcpy(lineBuffer, LAST_LINE, LAST_LINE_SIZE);
            fwrite(lineBuffer, 1, LAST_LINE_SIZE, fp);

            fflush(fp);
            fclose(fp);
        }
    }
}
