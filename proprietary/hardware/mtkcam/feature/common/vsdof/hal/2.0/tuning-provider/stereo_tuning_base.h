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

//========= Using rapidjson ==========
#include <vsdof/hal/rapidjson/writer.h>
#include <vsdof/hal/rapidjson/stringbuffer.h>
#include <vsdof/hal/rapidjson/document.h>     // rapidjson's DOM-style API
#include <vsdof/hal/rapidjson/prettywriter.h> // for stringify JSON
#include <vsdof/hal/extradata_def.h>
using namespace rapidjson;
//====================================
#include <map>
#include <mtkcam/utils/std/Log.h>
#include <vsdof/hal/ProfileUtil.h>
#include <vsdof/hal/AffinityUtil.h>
#include <mtkcam/feature/stereo/hal/FastLogger.h>

#define PROPERTY_ENABLE_TUNING  "vendor.STEREO.tuning"           //read/write stereo_tuning.json
#define PROPERTY_EXPORT_TUNING  "vendor.STEREO.tuning.export"    //Generate camera_custom_stereo_tuning.h
#define PROPERTY_REGEN_TUNING   "vendor.STEREO.tuning.regen"     //Force regen json and h with default values

#define VALUE_KEY_PARAMETERS    "Parameters"
#define VALUE_KEY_VALUES        "Values"

using namespace android;
using namespace StereoHAL;
using namespace std;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using android::Mutex;

extern const char *STEREO_TUNING_NAME[]; //defined in stereo_tuning_provider_kernel.cpp

class StereoTuningBase
{
public:
    StereoTuningBase() {}

    virtual ~StereoTuningBase() {}

    //Return the key of tuning in JSON, such as "HW_DPE", "SW_GF", etc
    virtual const char *getTuningKey() = 0;

    //Assign values to input object
    virtual bool retrieveTuningParams(TuningQuery_T &query) = 0;

    //Get tuning object in JSON, for merge
    virtual Document &getTuningObject()
    {
        return _document;
    }

    //print tuning params
    virtual void log(FastLogger &logger, bool inJSON=false)
    {
        if(LOG_ENABLED && TUNING_ENABLED && inJSON) {
            StringBuffer sb;
            PrettyWriter<StringBuffer> writer(sb);
            writer.SetFormatOptions(kFormatSingleLineArray);
            _document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

            const char *src = sb.GetString();
            if(src) {
                const size_t STR_LEN = strlen(src);
                if(STR_LEN > 0) {
                    if(STR_LEN < FastLogger::MAX_LOG_SIZE) {
                        logger.FastLogD("%s", src);
                    } else {
                        int restLen = STR_LEN;
                        const int LINE_SIZE = FastLogger::MAX_LOG_SIZE-1;
                        char lineBuffer[FastLogger::MAX_LOG_SIZE];
                        while(restLen > 0) {
                            if(restLen < LINE_SIZE) {
                                strncpy(lineBuffer, src, restLen);
                                lineBuffer[restLen] = 0;
                            } else {
                                strncpy(lineBuffer, src, LINE_SIZE);
                                lineBuffer[LINE_SIZE] = 0;
                            }

                            logger.FastLogD("%s", lineBuffer);
                            restLen -= LINE_SIZE;
                            src += LINE_SIZE;
                        }
                    }

                    logger.print();
                }
            }
        }
    }

protected:
    virtual void _initDefaultValues()      = 0;
    virtual void _initDocument()           = 0;
    virtual void _loadValuesFromDocument() = 0;

    virtual void _init(Value &document)
    {
        Value key;
        key.SetString(StringRef(getTuningKey(), strlen(getTuningKey())));
        if(document.HasMember(key))
        {
            _document.AddMember(key, document[key].Move(), _allocator);
            _loadValuesFromDocument();
        } else {
            _initDefaultValues();
            _initDocument();
        }
    }

protected:
    Document _document = Document(kObjectType);
    Document::AllocatorType& _allocator = _document.GetAllocator();
    const bool LOG_ENABLED = (1 == checkStereoProperty(PERPERTY_TUNING_PROVIDER_LOG));
    const bool TUNING_ENABLED = (1 == checkStereoProperty(PROPERTY_ENABLE_TUNING));

    //Align order of ENUM_STEREO_SCENARIO
    const char *SCENARIO_NAMES[TOTAL_STEREO_SCENARIO] = { "Unknown", "Preview", "Record", "Capture" };
};

#endif