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

#include "tunings/sw_gf_tuning.h"
#include "tunings/sw_occ_tuning.h"
#include "tunings/sw_bokeh_tuning.h"
#include "tunings/sw_refocus_tuning.h"
#endif

Mutex StereoTuningProviderKernel::__instanceLock;
StereoTuningProviderKernel * StereoTuningProviderKernel::__instance = NULL;

#define TUNING_FILE_PATH        "/sdcard/stereo_tuning.json"
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

const char *STEREO_TUNING_NAME[] =
{
    "HW_MDP_PQ",
#if (1==VSDOF_SUPPORTED)
    "HW_FE",
    "HW_FM",
    "HW_DPE",
    "HW_WMF",
    "HW_BOKEH",

    "SW_GF",
    "SW_OCC",
    "SW_BOKEH",
    "SW_REFOCUS",
#endif
};

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
    struct stat st;
    Document loadDocument(kObjectType);
    bool useCustomTuning = true;
    if(1 != checkStereoProperty(PROPERTY_REGEN_TUNING))
    {
        if(TUNING_ENABLED) {
            if(0 == stat(TUNING_FILE_PATH, &st))
            {
                if(__loadTuningFromFile(loadDocument)) {
                    useCustomTuning = false;
                } else {
                    MY_LOGW("Cannot load tuning from file, use default tuning instead");
                }
            }
        }

        if(useCustomTuning) {
            MY_LOGD("Parse tuning from custom header");
            loadDocument.Parse(DEFAULT_STEREO_TUNING);
            if(loadDocument.HasParseError()) {
                MY_LOGE("Fail to parse build-in tunings, error: %s (offset: %zu)", GetParseError_En(loadDocument.GetParseError()), loadDocument.GetErrorOffset());
            }
        }
    }


    const char *PROFILE_NAME = PROFILE_NAMES[StereoSettingProvider::stereoProfile()];
    const char *DEFAULT_PROFILE_NAME = PROFILE_NAMES[STEREO_SENSOR_PROFILE_REAR_REAR];
    Value profileKey, defaultKey;
    profileKey.SetString(StringRef(PROFILE_NAME, strlen(PROFILE_NAME)));
    defaultKey.SetString(StringRef(DEFAULT_PROFILE_NAME, strlen(DEFAULT_PROFILE_NAME)));

    Value document(kObjectType);
    // Check if tuning parameters exist for current profile
    if (loadDocument.HasMember(profileKey)) {
        MY_LOGD("Init tuning parameters with profile %s", PROFILE_NAME);
        document.CopyFrom(loadDocument[profileKey], loadDocument.GetAllocator());
    // If not, try to use default profile instead
    } else if (loadDocument.HasMember(defaultKey)) {
        MY_LOGW("Profile %s does not exist, init tuning parameters with default profile %s", PROFILE_NAME, DEFAULT_PROFILE_NAME);
        document.CopyFrom(loadDocument[defaultKey], loadDocument.GetAllocator());
    } else {
        MY_LOGW("Profile %s does not exist, init tuning parameters using whole document", PROFILE_NAME);
        document.CopyFrom(loadDocument, loadDocument.GetAllocator());
    }

    //Create map
    __tuningHolderMap[E_TUNING_HW_MDPPQ]   = new HW_MDPPQTuning(document);
#if (1==VSDOF_SUPPORTED)
    __tuningHolderMap[E_TUNING_HW_DPE]     = new HW_DPETuning(document);
    __tuningHolderMap[E_TUNING_HW_WMF]     = new HW_WMFTuning(document);
    __tuningHolderMap[E_TUNING_HW_FE]      = new HW_FETuning(document);
    __tuningHolderMap[E_TUNING_HW_FM]      = new HW_FMTuning(document);
    __tuningHolderMap[E_TUNING_HW_BOKEH]   = new HW_BOKEHTuning(document);

    __tuningHolderMap[E_TUNING_SW_GF]      = new SW_GFTuning(document);
    __tuningHolderMap[E_TUNING_SW_OCC]     = new SW_OCCTuning(document);

    Value refocusDocument(kObjectType);
    refocusDocument.CopyFrom(loadDocument, loadDocument.GetAllocator());
    SW_REFOCUSTuning *refocusTuning = new SW_REFOCUSTuning(refocusDocument);
    __tuningHolderMap[E_TUNING_SW_REFOCUS] = refocusTuning;
    __tuningHolderMap[E_TUNING_SW_BOKEH]   = new SW_BOKEHTuning(document, refocusTuning);
#endif

    //Log tuning parameters
    for(auto const &element : __tuningHolderMap)
    {
        if(element.second) {
            element.second->log(__logger);
        }
    }

    if(TUNING_ENABLED &&
       (1 == checkStereoProperty(PROPERTY_EXPORT_TUNING) ||
        useCustomTuning))
    {
        __saveTuningToFile(loadDocument);
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
StereoTuningProviderKernel::__saveTuningToFile(Document &document)
{
    AutoProfileUtil profile(LOG_TAG, "saveTuningToFile");

    Document::AllocatorType& allocator = document.GetAllocator();

    std::string tuningInJSON;

    const bool IS_REGEN = (1 == checkStereoProperty(PROPERTY_REGEN_TUNING));
    if(IS_REGEN) {
        Value(kObjectType).Swap(document);  //Clear original document

        Value parameters(kObjectType);
        for(auto const &element : __tuningHolderMap)
        {
            if(element.second) {
                Document &doc = element.second->getTuningObject();
                auto root = doc.MemberBegin();
                parameters.AddMember(root->name, root->value, allocator);
            }
        }

        if(MAKE_HalSensorList()->queryNumberOfSensors() < 4)
        {
            document.CopyFrom(parameters, allocator);
        }
        else
        {
            ENUM_STEREO_SENSOR_PROFILE currentProfile = StereoSettingProvider::stereoProfile();
            const char *PROFILE_NAME;
            Value profileKey;
            Value anotherProfileKey;

            ENUM_STEREO_SENSOR_PROFILE anotherProfile =
                                       (STEREO_SENSOR_PROFILE_FRONT_FRONT == currentProfile)
                                       ? STEREO_SENSOR_PROFILE_REAR_REAR
                                       : STEREO_SENSOR_PROFILE_FRONT_FRONT;
            PROFILE_NAME = PROFILE_NAMES[anotherProfile];
            anotherProfileKey.SetString(StringRef(PROFILE_NAME, strlen(PROFILE_NAME)));
            Value anotherParameters(parameters, allocator);

            PROFILE_NAME = PROFILE_NAMES[currentProfile];
            profileKey.SetString(StringRef(PROFILE_NAME, strlen(PROFILE_NAME)));
            document.AddMember(profileKey, parameters.Move(), allocator);
            document.AddMember(anotherProfileKey, anotherParameters.Move(), allocator);
        }
    }

    char fileName[PATH_MAX+1];
    //Save to json
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    writer.SetFormatOptions(kFormatSingleLineArray);
    document.Accept(writer);
    char *data = (char *)sb.GetString();

    struct stat st;
    if(IS_REGEN ||
       stat(TUNING_FILE_PATH, &st))
    {
        FILE *fp = fopen(TUNING_FILE_PATH, "w");
        if(fp) {
            fwrite(data, 1, strlen(data), fp);

            fflush(fp);
            fclose(fp);
        }
    }

    if(1 == checkStereoProperty(PROPERTY_EXPORT_TUNING)) {
        FILE *fp = fopen(TUNING_HEADER_PATH, "w");
        if(fp) {
            //Write copy right
            const char *COPY_RIGHT =
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
            strcpy(lineBuffer, "#endif\n");
            fwrite(lineBuffer, 1, strlen(lineBuffer), fp);

            fflush(fp);
            fclose(fp);
        }
    }
}

bool
StereoTuningProviderKernel::__loadTuningFromFile(Document &document)
{
    FILE *fp = fopen(TUNING_FILE_PATH, "r");
    if(fp) {
        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        document.ParseStream(is);
        fclose(fp);
    } else {
        MY_LOGE("Cannot open %s, err: %s", TUNING_FILE_PATH, strerror(errno));
    }

    return true;
}