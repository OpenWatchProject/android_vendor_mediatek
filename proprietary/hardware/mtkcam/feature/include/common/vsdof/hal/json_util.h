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
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "extradata_def.h"

using namespace rapidjson;

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
    JSON_Util()
        : _faceResult(NULL)
        , _dofLevelResult(NULL)
        , _main2MaskResult(NULL)
        , _mergeResult(NULL)
    {}

    /**
     * \brief Default destructor
     * \details Default destructor.
     *          It will delete the returned string,
     *          so callers should NOT delete returned string.
     */
    ~JSON_Util() {
        if(_faceResult) {
            delete [] _faceResult;
            _faceResult = NULL;
        }

        if(_dofLevelResult) {
            delete [] _dofLevelResult;
            _dofLevelResult = NULL;
        }

        if(_main2MaskResult) {
            delete [] _main2MaskResult;
            _main2MaskResult = NULL;
        }

        if(_mergeResult) {
            delete [] _mergeResult;
            _mergeResult = NULL;
        }
    }

    /**
     * \brief Transfer faces into JSON format
     * \details Transfer faces into JSON format:
     *          face_detections: {
     *              [{
     *                  left: xxx,
     *                  top: xxx,
     *                  right: xxx,
     *                  bottom: xxx,
     *                  rotation-in-plane: xxx
     *              }]
     *          }
     *
     * \param faces An array consists of face detection data
     * \return JSON format string
     *         Caller should NOT delete the string.
     */
    char *facesToJSON(std::vector<FD_DATA_STEREO_T> faces)
    {
        Document document(kObjectType);
        Document::AllocatorType& allocator = document.GetAllocator();

        Value faceArray(kArrayType);
        for(std::vector<FD_DATA_STEREO_T>::iterator it = faces.begin(); it != faces.end(); ++it) {
            Value face(kObjectType);
            face.AddMember(EXTRA_DATA_LEFT,      Value(it->left).Move(), allocator);
            face.AddMember(EXTRA_DATA_TOP,       Value(it->top).Move(), allocator);
            face.AddMember(EXTRA_DATA_RIGHT,     Value(it->right).Move(), allocator);
            face.AddMember(EXTRA_DATA_BOTTOM,    Value(it->bottom).Move(), allocator);
            face.AddMember(EXTRA_DATA_ROTATION_IN_PLANE, Value(it->rotation).Move(), allocator);  //0, 1, ...11

            faceArray.PushBack(face.Move(), allocator);
        }
        document.AddMember(EXTRA_DATA_FACE_DETECTIONS, faceArray, allocator);

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

        const char *jsonString = sb.GetString();

        if(_faceResult) {
            delete [] _faceResult;
            _faceResult = NULL;
        }

        if(jsonString) {
            const int STR_LEN = strlen(jsonString);
            if(STR_LEN > 0) {
                _faceResult = new char[STR_LEN+1];
                strcpy(_faceResult, jsonString);
            }
        }

        return _faceResult;
    }

    /**
     * \brief Transfer DoF level to JSON format
     *
     * \param dofLevel DoF level to transfer
     * \return JSON format string like this {"dof_level":8}
     */
    char *dofLevelToJSON(int dofLevel)
    {
        Document document(kObjectType);
        Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember(EXTRA_DATA_DOF_LEVEL, Value(dofLevel).Move(), allocator);

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

        const char *jsonString = sb.GetString();

        if(_dofLevelResult) {
            delete [] _dofLevelResult;
            _dofLevelResult = NULL;
        }

        if(jsonString) {
            const int STR_LEN = strlen(jsonString);
            if(STR_LEN > 0) {
                _dofLevelResult = new char[STR_LEN+1];
                strcpy(_dofLevelResult, jsonString);
            }
        }

        return _dofLevelResult;
    }

    /**
     * \brief Transfer mask to JSON
     *
     * \param mask Mask buffer
     * \param maskSize Mask size
     *
     * \return JSON string consists of:
     *         "mask_info" : {
     *             "width":2176,
     *             "height":1152,
     *             "mask description": "Data(0xFF), format: [offset,length]",
     *             "mask": [[28,1296],[1372,1296],[2716,1296],...]
     *         }
     */
    char *maskToJSON(MUINT8 *mask, NSCam::MSize maskSize)
    {
        //1. Compress mask
        std::vector<RUN_LENGTH_DATA> runLengthMaskData;
        MaskCompressor::compressMask(mask, maskSize, runLengthMaskData);

        //2. Transfer to JSON
        Document document(kObjectType);
        Document::AllocatorType& allocator = document.GetAllocator();

        Value mask_info(kObjectType);
        mask_info.AddMember("width",  maskSize.w, allocator);
        mask_info.AddMember("height", maskSize.h, allocator);
        mask_info.AddMember("mask description", "Data(0xFF), format: [offset,length]", allocator);

        Value maskValue(kArrayType);
        for(std::vector<RUN_LENGTH_DATA>::iterator it = runLengthMaskData.begin(); it != runLengthMaskData.end(); ++it) {
            Value maskData(kArrayType);
            maskData.PushBack(Value(it->offset).Move(), allocator);
            maskData.PushBack(Value(it->len).Move(), allocator);
            maskValue.PushBack(maskData.Move(), allocator);
        }
        mask_info.AddMember("mask", maskValue, allocator);
        document.AddMember("mask_info", mask_info, allocator);

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

        const char *jsonString = sb.GetString();

        if(_main2MaskResult) {
            delete [] _main2MaskResult;
            _main2MaskResult = NULL;
        }

        if(jsonString) {
            const int STR_LEN = strlen(jsonString);
            if(STR_LEN > 0) {
                _main2MaskResult = new char[STR_LEN+1];
                strcpy(_main2MaskResult, jsonString);
            }
        }

        return _main2MaskResult;
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
        Document document1;
        document1.Parse(jsonStr1);
        if(document1.HasParseError())
        {
            return NULL;
        }
        Document::AllocatorType& allocator = document1.GetAllocator();

        Document document2;
        document2.Parse(jsonStr2);
        if(document2.HasParseError())
        {
            return NULL;
        }

        for(Value::MemberIterator it = document2.MemberBegin(); it != document2.MemberEnd(); ++it) {
            document1.AddMember(it->name, it->value, allocator);
        }

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        document1.Accept(writer);    // Accept() traverses the DOM and generates Handler events.

        const char *jsonString = sb.GetString();
        if(_mergeResult) {
            delete [] _mergeResult;
            _mergeResult = NULL;
        }
        char *result = NULL;
        if(jsonString) {
            const int STR_LEN = strlen(jsonString);
            if(STR_LEN > 0) {
                _mergeResult = new char[STR_LEN+1];
                strcpy(_mergeResult, jsonString);
            }
        }

        return _mergeResult;
    }
private:
    char *_faceResult;
    char *_dofLevelResult;
    char *_main2MaskResult;
    char *_mergeResult;
};
#endif