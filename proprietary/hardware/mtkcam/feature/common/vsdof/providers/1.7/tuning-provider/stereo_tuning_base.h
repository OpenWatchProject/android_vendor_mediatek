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
#ifndef STEREO_TUNING_BASE_H_
#define STEREO_TUNING_BASE_H_

#include <stereo_tuning_def.h>

#include <map>
#include <mtkcam/utils/std/Log.h>
#include <vsdof/hal/ProfileUtil.h>
#include <vsdof/hal/AffinityUtil.h>
#include <mtkcam/feature/stereo/hal/FastLogger.h>
#include <mtkcam/utils/LogicalCam/LogicalCamJSONUtil.h>

#define PROPERTY_ENABLE_TUNING  "vendor.STEREO.tuning"           //read/write stereo_tuning.json
#define PROPERTY_EXPORT_TUNING  "vendor.STEREO.tuning.export"    //Generate camera_custom_stereo_tuning.h

#define VALUE_KEY_PARAMETERS    "Parameters"
#define VALUE_KEY_VALUES        "Values"

using namespace android;
using namespace StereoHAL;
using namespace std;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using android::Mutex;

class StereoTuningBase
{
public:
    StereoTuningBase() {}

    virtual ~StereoTuningBase() {}

    //Return the key of tuning in JSON, such as "HW_DPE", "SW_GF", etc
    virtual const char *getTuningKey() = 0;

    //Assign values to input object
    virtual bool retrieveTuningParams(TuningQuery_T &query) = 0;

    //print tuning params
    virtual void log(FastLogger &logger, bool inJSON=false)
    {
        if(LOG_ENABLED && inJSON) {
            std::string jsonStr = _tuningInJson.dump(4);
            const char *src = jsonStr.c_str();
            if(src) {
                const size_t STR_LEN = strlen(src);
                if(STR_LEN > 0) {
                    if(STR_LEN < FastLogger::MAX_LOG_SIZE) {
                        logger.FastLogD("%s", src);
                    } else {
                        int restLen = STR_LEN;
                        const int LINE_SIZE = FastLogger::MAX_LOG_SIZE-1;
                        char lineBuffer[LINE_SIZE+1];
                        size_t copySize = 0;
                        while(restLen > 0) {
                            if(restLen < LINE_SIZE) {
                                strncpy(lineBuffer, src, restLen);
                                lineBuffer[restLen] = 0;
                                copySize = restLen;
                            } else {
                                copySize = LINE_SIZE;
                                for(const char *dst = src + LINE_SIZE-1; *dst != '\n' && copySize > 0; --dst, --copySize);

                                //if no newline in buffer, just copy it...
                                if(0 == copySize) {
                                    copySize = LINE_SIZE;
                                }

                                ::memcpy(lineBuffer, src, copySize);
                                lineBuffer[copySize] = 0;
                            }

                            logger.FastLogD("%s", lineBuffer);
                            restLen -= copySize;
                            src += copySize;
                        }
                    }

                    logger.print();
                }
            }
        }
    }

protected:
    virtual void _initDefaultValues()      = 0;
    virtual void _loadValuesFromDocument(const json &jsonObj) = 0;

    virtual void _init(const json &tuningJson)
    {
        if(LogicalCamJSONUtil::HasMember(tuningJson, getTuningKey())) {
            _loadValuesFromDocument(tuningJson.at(getTuningKey()));
        } else {
            _initDefaultValues();
        }
    }

    //Return 0 if the key is not found
    inline int _getInt(const json &tuningJson, const char *key) {
        int value = 0;
        if(!_getValue(tuningJson, key, value)) {
            if(LOG_ENABLED) {
                __android_log_print(ANDROID_LOG_DEBUG, "StereoTuningBase", "%s::%s not found in tuning, set to 0", getTuningKey(), key);
            }
        }

        return value;
    }

    //Not to change value if not found and return false is the key is not found
    template<typename T>
    inline bool _getValue(const json &tuningJson, const char *key, T &value) {
        if(LogicalCamJSONUtil::HasMember(tuningJson, key)) {
            try {
                value = tuningJson.at(key).get<T>();
                return true;
            } catch (json::exception& e) {
                __android_log_print(ANDROID_LOG_ERROR, "StereoTuningBase", "Get %s::%s failed, error: %s", getTuningKey(), key, e.what());
            }
        }

        return false;
    }

    //Not to change value if not found and return false is the key is not found
    inline bool _getValue(const json &tuningJson, const char *key, int &value) {
        if(LogicalCamJSONUtil::HasMember(tuningJson, key)) {
            try {
                value = tuningJson.at(key).get<int>();
                return true;
            } catch (json::exception& e) {
                __android_log_print(ANDROID_LOG_ERROR, "StereoTuningBase", "Get %s::%s failed, error: %s", getTuningKey(), key, e.what());
            }
        }

        return false;
    }

    //Not to change value if not found and return false is the key is not found
    inline bool _getValue(const json &tuningJson, const char *key, bool &value) {
        if(LogicalCamJSONUtil::HasMember(tuningJson, key)) {
            try {
                value = tuningJson.at(key).get<bool>();
                return true;
            } catch (json::exception& e) {
                __android_log_print(ANDROID_LOG_ERROR, "StereoTuningBase", "Get %s::%s failed, error: %s", getTuningKey(), key, e.what());
            }
        }

        return false;
    }

    //Not to change value if not found and return false is the key is not found
    inline bool _getValue(const json &tuningJson, const char *key, unsigned int &value) {
        if(LogicalCamJSONUtil::HasMember(tuningJson, key)) {
            try {
                value = tuningJson.at(key).get<unsigned int>();
                return true;
            } catch (json::exception& e) {
                __android_log_print(ANDROID_LOG_ERROR, "StereoTuningBase", "Get %s::%s failed, error: %s", getTuningKey(), key, e.what());
            }
        }

        return false;
    }

    //Not to change value if not found and return false is the key is not found
    inline bool _getValue(const json &tuningJson, const char *key, float &value) {
        if(LogicalCamJSONUtil::HasMember(tuningJson, key)) {
            try {
                value = tuningJson.at(key).get<float>();
                return true;
            } catch (json::exception& e) {
                __android_log_print(ANDROID_LOG_ERROR, "StereoTuningBase", "Get %s::%s failed, error: %s", getTuningKey(), key, e.what());
            }
        }

        return false;
    }

    inline bool _isArray(const json &tuningJson, const char *key)
    {
        if(LogicalCamJSONUtil::HasMember(tuningJson, key) &&
           tuningJson[key].is_array())
        {
            return true;
        }

        return false;
    }

    inline bool _checkArrayAndSize(const json &tuningJson, const char *key, const size_t size)
    {
        if(_isArray(tuningJson, key) &&
           tuningJson[key].size() == size)
        {
            return true;
        }

        return false;
    }

protected:
    json _tuningInJson;
    const bool LOG_ENABLED = (1 == checkStereoProperty(PERPERTY_TUNING_PROVIDER_LOG));

    //Align order of ENUM_STEREO_SCENARIO
    const char *SCENARIO_NAMES[TOTAL_STEREO_SCENARIO] = { "Unknown", "Preview", "Record", "Capture" };
};

#endif