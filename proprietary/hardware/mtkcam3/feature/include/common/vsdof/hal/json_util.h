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
#ifndef _JSON_UTIL_H_
#define _JSON_UTIL_H_

#include <vector>
#include <mtkcam/def/common.h>
#include "mask_compressor.h"
#include "extradata_def.h"
#include <mtkcam/utils/json/json.hpp>

using json = nlohmann::json;

struct FD_DATA_STEREO_T
{
    int left;
    int top;
    int right;
    int bottom;
    int rotation;   //0~11

    FD_DATA_STEREO_T()
        : left(-9999)
        , top(-9999)
        , right(-9999)
        , bottom(-9999)
        , rotation(-1) {}

    FD_DATA_STEREO_T(int l, int t, int r, int b, int rot)
        : left(l)
        , top(t)
        , right(r)
        , bottom(b)
        , rotation(rot) {}
};

class JSON_Util
{
public:
    /**
     * \brief Default constructor
     */
    JSON_Util() {}

    /**
     * \brief Default destructor
     * \details Default destructor.
     *          It will delete the returned string,
     *          so callers should NOT delete returned string.
     */
    ~JSON_Util() {}

    /**
     * \brief Transfer faces into JSON format
     * \details Transfer faces into JSON format:
     *          face_detections: [
     *              {
     *                  left: xxx,
     *                  top: xxx,
     *                  right: xxx,
     *                  bottom: xxx,
     *                  rotation-in-plane: xxx
     *              }
     *          ]
     *
     * \param faces An array consists of face detection data
     * \return JSON format string
     *         Caller should NOT delete the string.
     */
    char *facesToJSON(std::vector<FD_DATA_STEREO_T> faces)
    {
        json resultJson;

        json faceListJson;
        for(std::vector<FD_DATA_STEREO_T>::iterator it = faces.begin(); it != faces.end(); ++it) {
            json faceJson;
            faceJson[EXTRA_DATA_LEFT]              = it->left;
            faceJson[EXTRA_DATA_TOP]               = it->top;
            faceJson[EXTRA_DATA_RIGHT]             = it->right;
            faceJson[EXTRA_DATA_BOTTOM]            = it->bottom;
            faceJson[EXTRA_DATA_ROTATION_IN_PLANE] = it->rotation;  //0, 1, ...11

            faceListJson.push_back(faceJson);
        }
        resultJson[EXTRA_DATA_FACE_DETECTIONS] = faceListJson;

        _faceResult = resultJson.dump();
        return (char *)_faceResult.c_str();
    }

    /**
     * \brief Transfer DoF level to JSON format
     *
     * \param dofLevel DoF level to transfer
     * \return JSON format string like this {"dof_level":8}
     */
    char *dofLevelToJSON(int dofLevel)
    {
        json resultJson;
        resultJson[EXTRA_DATA_DOF_LEVEL] = dofLevel;
        _dofLevelResult = resultJson.dump();

        return (char *)_dofLevelResult.c_str();
    }

    /**
     * \brief Merge two JSON format string
     * \details Merge two JSON format string
     *
     * \param jsonStr1 The first JSON string to merge
     * \param jsonStr2 The second JSON string to merge
     *
     * \return Merged JSON string.
     *         If any of the input string has parse error, returns NULL.
     *         Caller should NOT delete the string.
     */
    char *mergeJSON(const char *jsonStr1, const char *jsonStr2)
    {
        json json1 = json::parse(jsonStr1);
        json json2 = json::parse(jsonStr2);

        if(json1.is_discarded() ||
           json2.is_discarded())
        {
            return NULL;
        }

        json1.merge_patch(json2);
        _mergeResult = json1.dump();

        return (char *)_mergeResult.c_str();
    }
private:
    std::string _faceResult;
    std::string _dofLevelResult;
    std::string _mergeResult;
};
#endif