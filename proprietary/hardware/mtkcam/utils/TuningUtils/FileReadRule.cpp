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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "mtkcam-aaa-util"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <fstream>
#include <cutils/properties.h>
#include <string>
#include <cstring>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/TuningUtils/CommonRule.h>
#if MTK_DP_VERSION
#include "debug_exif/aaa/dbg_isp_param.h"
#endif
#include <mtkcam/def/common.h>
#include <mtkcam/utils/sys/IFileCache.h>
#include <utils/RefBase.h>
/*
 *
 ******************************************************************************/
//
namespace NSCam {
namespace TuningUtils {

static
inline void
removeRedundancy(std::string* pStr){
    std::string::size_type i = 0;
    i = pStr->find_first_of(";", i);
    *pStr = pStr->substr(i+1, pStr->length());
    i = 0;
    while (i < pStr->length()) {
        i = pStr->find_first_of("\n\r ", i);
        if (i == std::string::npos) {
            break;
        }
        pStr->erase(i);
    }
}
static
inline void
Extract_Tuning_CFG(std::string* pStr, TUNING_CFG_T* pTuning_cfg){
    std::string::size_type i = 0;
    i = pStr->find_first_of(",");
    pTuning_cfg->str_format = pStr->substr(0,i);

    *pStr = pStr->substr(i+1,pStr->length());
    i = pStr->find_first_of(",");
    pTuning_cfg->str_reqNum = pStr->substr(0,i);

    *pStr = pStr->substr(i+1,pStr->length());
    i = pStr->find_first_of(";");
    pTuning_cfg->str_ispProfile = pStr->substr(0,i);

    *pStr = pStr->substr(i+1,pStr->length());
    i = pStr->find_first_of("\n\r");
    pTuning_cfg->str_answer = pStr->substr(0,i);
}

static
inline void
Extract_Tuning_CFG_AINR(std::string* pStr, TUNING_CFG_T* pTuning_cfg){
    std::string::size_type i = 0;
    i = pStr->find_first_of(",");
    pTuning_cfg->str_format = pStr->substr(0,i);

    *pStr = pStr->substr(i+1,pStr->length());
    i = pStr->find_first_of(",");
    pTuning_cfg->str_reqNum = pStr->substr(0,i);

    *pStr = pStr->substr(i+1,pStr->length());
    i = pStr->find_first_of(",");
    pTuning_cfg->str_usage = pStr->substr(0,i);

    *pStr = pStr->substr(i+1,pStr->length());
    i = pStr->find_first_of(";");
    pTuning_cfg->str_ispProfile = pStr->substr(0,i);

    *pStr = pStr->substr(i+1,pStr->length());
    i = pStr->find_first_of("\n\r");
    pTuning_cfg->str_answer = pStr->substr(0,i);
}

FileReadRule::FileReadRule()
{
}
bool FileReadRule::getFile_RAW(int reqNum, const char* ispProfile, IImageBuffer *pbuf, const char *pUserString, int SensorID)
{
    MBOOL ret = MTRUE;
    if (!isREADEnable(pUserString)){
       return MFALSE;
    }

    ALOGD("[%s] reqNum(%d) ispProfile(%s) pbuf(%x) pUserString(%s)",__FUNCTION__, reqNum, ispProfile, pbuf, pUserString);

    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("raw");
    std::string str_ispProfile(ispProfile);

    char str_reqNum[50];
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        std::string::size_type i = 0;
        i = str.find_first_of(",", i);
        if (str.substr(0,i).compare(str_format) == 0){ //format
            str = str.substr(i+1,str.length());
            i = str.find_first_of(",", i);
            if ((std::string::npos != str_ispProfile.find("MFNR"))){
                // mfnr needs check req
                if (str.substr(0,i).compare(str_reqNum) == 0){
                    removeRedundancy(&str);
                    break;
                }
            } else {
                removeRedundancy(&str);
                break;
            }
        }
    }

    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] Fail find imgo path in config", __FUNCTION__);
        ret = MFALSE;
    }
    else {
        ALOGD("[%s] Load imgo from (%s)",__FUNCTION__, str.c_str());
        pbuf->unlockBuf(pUserString);
        ret = pbuf->loadFromFile(str.c_str());
        if (!ret)
            ALOGD("[%s] Fail to load imgo from (%s)", __FUNCTION__, str.c_str());
        const auto __usage = (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);
        pbuf->lockBuf(pUserString, __usage);
        pbuf->syncCache();
    }
    return ret;
}

bool FileReadRule::getFile_WarpingRaw(int reqNum, const char* ispProfile, IImageBuffer *pbuf, const char *pUserString
                                                , const char *pBufString, int SensorID)
{
    MBOOL ret = MTRUE;
    if (!isREADEnable(pUserString))
       return MFALSE;

    ALOGD("[%s] reqNum(%d) ispProfile(%s) pbuf(%x) pUserString(%s), pBufString(%s)",__FUNCTION__, reqNum, ispProfile, pbuf, pUserString, pBufString);

    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("packed_word");
    std::string str_ispProfile(ispProfile);
    std::string str_usage(pUserString);

    char str_reqNum[50];
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        TUNING_CFG_T cfg;
        Extract_Tuning_CFG_AINR(&str, &cfg);

        //ALOGD("cfg str_format(%s), str_req(%s) str_profile(%s) str_usage(%s)",
        //    cfg.str_format.c_str(), cfg.str_reqNum.c_str(), cfg.str_ispProfile.c_str(), cfg.str_usage.c_str());

        if (cfg.str_format.compare(str_format) == 0){ //format
            if (cfg.str_usage.compare(str_usage) == 0){
                str = cfg.str_answer;
                break;
            }
        }
    }

    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] Fail find WarpingRaw path in config", __FUNCTION__);
        ret = MFALSE;
    }
    else {
        ALOGD("[%s] Load WarpingRaw from (%s)",__FUNCTION__, str.c_str());
        pbuf->unlockBuf(pBufString);
        ret = pbuf->loadFromFile(str.c_str());
        if (!ret)
            ALOGD("[%s] Fail to load WarpingRaw from (%s)", __FUNCTION__, str.c_str());
        const auto __usage = (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);
        pbuf->lockBuf(pBufString, __usage);
        pbuf->syncCache();
    }

    return ret;
}

bool FileReadRule::getFile_LCSO(int reqNum, const char* ispProfile, IImageBuffer *pbuf, const char *pUserString, int SensorID)
{
    MBOOL ret = MTRUE;
    if (!isREADEnable(pUserString))
       return MFALSE;

    ALOGD("[%s] reqNum(%d) ispProfile(%s) pbuf(%x) pUserString(%s)",__FUNCTION__, reqNum, ispProfile, pbuf, pUserString);

    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("lcso");
    std::string str_ispProfile(ispProfile);

    char str_reqNum[50] = {'\0'};
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        std::string::size_type i = 0;
        i = str.find_first_of(",", i);
        if (str.substr(0,i).compare(str_format) == 0){ //format
            str = str.substr(i+1,str.length());
            i = str.find_first_of(",", i);
            if ((std::string::npos != str_ispProfile.find("MFNR"))){
                // mfnr needs check req
                if (str.substr(0,i).compare(str_reqNum) == 0){
                    removeRedundancy(&str);
                    break;
                }
            } else {
                removeRedundancy(&str);
                break;
            }
        }
    }
    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] Fail find LCSO path in config", __FUNCTION__);
        ret = MFALSE;
    }
    else {
        ALOGD("[%s] Load LCSO from (%s)",__FUNCTION__, str.c_str());
        pbuf->unlockBuf(pUserString);
        ret = pbuf->loadFromFile(str.c_str());
        if (!ret)
            ALOGD("[%s] Fail to load LCSO from (%s)", __FUNCTION__, str.c_str());
        const auto __usage = (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);
        pbuf->lockBuf(pUserString, __usage);
        pbuf->syncCache();
    }
    return ret;
}
bool FileReadRule::getFile_LSC(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString)
{
    ALOGD("[%s] reqNum(%d) ispProfile(%s) pUserString(%s)",__FUNCTION__, reqNum, ispProfile, pUserString);

    MBOOL ret = MTRUE;
    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("lsc");
    std::string str_ispProfile(ispProfile);
    char *ptr = filename;

    char str_reqNum[50];
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        std::string::size_type i = 0;
        i = str.find_first_of(",", i);
        if (str.substr(0,i).compare(str_format) == 0){ //format
            str = str.substr(i+1,str.length());
            i = str.find_first_of(",", i);
            if ((std::string::npos != str_ispProfile.find("MFNR"))){
                // mfnr needs check req
                if (str.substr(0,i).compare(str_reqNum) == 0){
                    str = str.substr(i+1,str.length());
                    i = str.find_first_of(";", i);
                    //tuning & shading table needs check isp profile, raw & lcso just need req num
                    if (str.substr(0,i).find(str_ispProfile) != std::string::npos){
                        removeRedundancy(&str);
                        break;
                    }
                }
            } else {
                removeRedundancy(&str);
                break;
            }
        }
    }
    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] file path not found in config", __FUNCTION__);
        ret = MFALSE;
    } else {
        std::ifstream file(str.c_str());
        snprintf(ptr, filesize, "%s", str.c_str());
        if (file.good())
        {
            ALOGD("[%s] file exist(%s)",__FUNCTION__, str.c_str());
            ret = MTRUE;
        }
        else
        {
            ALOGD("[%s] file not found(%s)", __FUNCTION__, str.c_str());
            ret = MFALSE;
        }
    }
    return ret;
}

bool FileReadRule::getFile_RRZO(int reqNum, const char* ispProfile, IImageBuffer *pbuf, const char *pUserString, int SensorID)
{
    MBOOL ret = MTRUE;
    if (!isREADEnable(pUserString))
       return MFALSE;

    ALOGD("[%s] reqNum(%d) ispProfile(%s) pbuf(%x) pUserString(%s)",__FUNCTION__, reqNum, ispProfile, pbuf, pUserString);

    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("rrzo");
    std::string str_ispProfile(ispProfile);

    char str_reqNum[50] = {'\0'};
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        std::string::size_type i = 0;
        i = str.find_first_of(",", i);
        if (str.substr(0,i).compare(str_format) == 0){ //format
            str = str.substr(i+1,str.length());
            i = str.find_first_of(",", i);
            if ((std::string::npos != str_ispProfile.find("MFNR"))){
                // mfnr needs check req
                if (str.substr(0,i).compare(str_reqNum) == 0){
                    removeRedundancy(&str);
                    break;
                }
            } else {
                removeRedundancy(&str);
                break;
            }
        }
    }

    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] Fail to find RRZO path in config", __FUNCTION__);
        ret = MFALSE;
    }
    else {
        ALOGD("[%s] Load RRZO from (%s)",__FUNCTION__, str.c_str());
        pbuf->unlockBuf(pUserString);
        ret = pbuf->loadFromFile(str.c_str());
        if (!ret)
            ALOGD("[%s] Fail to load RRZO from (%s)", __FUNCTION__, str.c_str());
        const auto __usage = (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);
        pbuf->lockBuf(pUserString, __usage);
        pbuf->syncCache();
    }
    return ret;

}


bool FileReadRule::getFile_LPCNR_TUNING(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString)
{
    ALOGD("[%s] reqNum(%d) ispProfile(%s) pUserString(%s)",__FUNCTION__, reqNum, ispProfile, pUserString);

    MBOOL ret = MTRUE;
    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_ispProfile(ispProfile);
    std::string str_usage;
    char *ptr = filename;
    char str_reqNum[50];
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        if (pUserString != NULL){
            TUNING_CFG_T cfg;
            Extract_Tuning_CFG(&str, &cfg);
            str_usage = pUserString;
            if (cfg.str_format.compare(str_usage) == 0){ //format
                if ((std::string::npos != cfg.str_ispProfile.find("MFNR"))){
                    if (cfg.str_ispProfile.compare(str_ispProfile) == 0){
                        if (cfg.str_reqNum.compare(str_reqNum) == 0){
                            str = cfg.str_answer;
                            break;
                        }
                    }
                } else {
                    if (cfg.str_ispProfile.compare(str_ispProfile) == 0){
                        str = cfg.str_answer;
                        break;
                    }
                }
            }
        }
    }

    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] file path not found in config", __FUNCTION__);
        ret = MFALSE;
    } else {
        std::ifstream file(str.c_str());
        snprintf(ptr, filesize, "%s", str.c_str());
        if (file.good())
        {
            ALOGD("[%s] file exist(%s)",__FUNCTION__, str.c_str());
            ret = MTRUE;
        }
        else
        {
            ALOGD("[%s] file not found(%s)", __FUNCTION__, str.c_str());
            ret = MFALSE;
        }
    }
    return ret;


}

bool FileReadRule::getFile_P2TUNING(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString)
{
    ALOGD("[%s] reqNum(%d) ispProfile(%s) pUserString(%s)",__FUNCTION__, reqNum, ispProfile, pUserString);

    MBOOL ret = MTRUE;
    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("tuning");
    std::string str_ispProfile(ispProfile);
    char *ptr = filename;
    char str_reqNum[50];
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        if (pUserString != NULL){
            TUNING_CFG_T cfg;
            Extract_Tuning_CFG(&str, &cfg);
            int cfg_str_profile_length = cfg.str_ispProfile.length();
            int str_profile_length = str_ispProfile.length();

            if (cfg.str_format.compare(str_format) == 0){ //format
                if ( (std::string::npos != cfg.str_ispProfile.find("MFNR"))
                    || (std::string::npos != cfg.str_ispProfile.find("AINR_MainYUV")) ){
                    if (cfg.str_ispProfile.compare(str_ispProfile) == 0){
                        if (cfg.str_reqNum.compare(str_reqNum) == 0){
                            str = cfg.str_answer;
                            break;
                        }
                    }
                } else {
                    if (cfg.str_ispProfile.compare(str_ispProfile) == 0){
                        str = cfg.str_answer;
                        break;
                    }
                }
            }
        } else {
            std::string::size_type i = 0;
            i = str.find_first_of(",", i);
            if (str.substr(0,i).compare(str_format) == 0){ //format
                str = str.substr(i+1,str.length());
                if ((std::string::npos != str_ispProfile.find("MFNR"))){
                    // mfnr needs check req
                    i = 0;
                    i = str.find_first_of(",", i);
                    if (str.substr(0,i).compare(str_reqNum) == 0){
                        str = str.substr(i+1,str.length());
                        i = str.find_first_of(";", i);
                        //tuning & shading table needs check isp profile, raw & lcso just need req num
                        if (str.substr(0,i).find(str_ispProfile) != std::string::npos){
                            removeRedundancy(&str);
                            break;
                        }
                    }
                } else {
                    removeRedundancy(&str);
                    break;
                }
            }
        }
    }

    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] file path not found in config", __FUNCTION__);
        ret = MFALSE;
    } else {
        std::ifstream file(str.c_str());
        snprintf(ptr, filesize, "%s", str.c_str());
        if (file.good())
        {
            ALOGD("[%s] file exist(%s)",__FUNCTION__, str.c_str());
            ret = MTRUE;
        }
        else
        {
            ALOGE("[%s] file not found(%s)", __FUNCTION__, str.c_str());
            ret = MFALSE;
        }
    }
    return ret;

}

bool FileReadRule::getFile_LTMCURVE(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString)
{
    ALOGD("[%s] reqNum(%d) ispProfile(%s) pUserString(%s)",__FUNCTION__, reqNum, ispProfile, pUserString);

    MBOOL ret = MTRUE;
    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("ltm");
    std::string str_ispProfile(ispProfile);
    char *ptr = filename;
    char str_reqNum[50];
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        TUNING_CFG_T cfg;
        Extract_Tuning_CFG(&str, &cfg);
        /*ALOGD("cfg str_format(%s), str_req(%s) str_profile(%s) str_path(%s)",
            cfg.str_format.c_str(), cfg.str_reqNum.c_str(), cfg.str_ispProfile.c_str(), cfg.str_answer.c_str());*/
        if (cfg.str_format.compare(str_format) == 0){ //format
            if ((std::string::npos != cfg.str_ispProfile.find("MFNR"))){
                if (cfg.str_reqNum.compare(str_reqNum) == 0){
                    str = cfg.str_answer;
                    break;
                }
            } else {
                if (cfg.str_format.compare(str_format) == 0){
                    str = cfg.str_answer;
                    break;
                }
            }
        }
    }

    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] file path not found in config", __FUNCTION__);
        ret = MFALSE;
    } else {
        std::ifstream file(str.c_str());
        snprintf(ptr, filesize, "%s", str.c_str());
        if (file.good())
        {
            ALOGD("[%s] file exist(%s)",__FUNCTION__, str.c_str());
            ret = MTRUE;
        }
        else
        {
            ALOGD("[%s] file not found(%s)", __FUNCTION__, str.c_str());
            ret = MFALSE;
        }
    }
    return ret;


}

bool FileReadRule::getFile_MFB(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString)
{
    ALOGD("[%s] reqNum(%d) ispProfile(%s) pUserString(%s)",__FUNCTION__, reqNum, ispProfile, pUserString);

    MBOOL ret =  MTRUE;
    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("mfb");
    std::string str_ispProfile(ispProfile);
    char *ptr = filename;
    char str_reqNum[50];
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        std::string::size_type i = 0;
        i = str.find_first_of(",", i);
        if (str.substr(0,i).compare(str_format) == 0){ //format
            str = str.substr(i+1,str.length());
            removeRedundancy(&str);
            break;
        }
    }

    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] file path not found in config", __FUNCTION__);
        ret = MFALSE;
    } else {
        std::ifstream file(str.c_str());
        snprintf(ptr, filesize, "%s", str.c_str());
        if (file.good())
        {
            ALOGD("[%s] file exist(%s)",__FUNCTION__, str.c_str());
            ret = MTRUE;
        }
        else
        {
            ALOGD("[%s] file not found(%s)", __FUNCTION__, str.c_str());
            ret = MFALSE;
        }
    }
    return ret;

}

bool FileReadRule::getFile_YUV(int reqNum, const char *ispProfile, IImageBuffer *pbuf, const char *pUserString, YUV_PORT type, int SensorID)
{
    if (!isREADEnable(pUserString))
       return MFALSE;
    else if (type != YUV_PORT_IMG3O)
       return MFALSE;

    ALOGD("[%s] reqNum(%d) ispProfile(%s) pUserString(%s) yuv_port(%d)",__FUNCTION__, reqNum, ispProfile, pUserString, type);

    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("img3o");
    std::string str_ispProfile(ispProfile);

    char str_reqNum[50];
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str)) {
        std::string::size_type i = 0;
        i = str.find_first_of(",", i);
        if (str.substr(0,i).compare(str_format) == 0){ //format
            removeRedundancy(&str);
            ALOGD("DEBUG load yuv from %s", str.c_str());
            pbuf->saveToFile(str.c_str());
            break;
        }
    }
    return MTRUE;
}
int FileReadRule::getFile_uniqueKey(const char *pUserString, int SensorID){
    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str;
    std::string str_format("uniqueKey");


    while (std::getline(infile, str))
    {
        if (std::string::npos != str.find(str_format)){ //format
            removeRedundancy(&str);
            ALOGD("[%s] DEBUG uniqueKey from %d", __FUNCTION__, atoi(str.c_str()));
            return atoi(str.c_str());
        }
    }
    return 0;
}
int  get_raw_stride(const char *pUserString, int SensorID = 0){
        char _config[1024] = {'\0'};
        ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
        std::ifstream infile(_config);
        std::string str;
        std::string str_format("raw_stride");

        while (std::getline(infile, str))
        {
            if (std::string::npos != str.find(str_format)){ //format
                removeRedundancy(&str);
                ALOGD("get_raw_stride from %d", atoi(str.c_str()));
                return atoi(str.c_str());
            }
        }
        return 0;
}


int FileReadRule::getFile_requestNo(const char *pUserString, int SensorID){
    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str;
    std::string str_format("requestNo");


    while (std::getline(infile, str))
    {
        if (std::string::npos != str.find(str_format)){ //format
            removeRedundancy(&str);
            ALOGD("DEBUG uniqueKey from %d", atoi(str.c_str()));
            return atoi(str.c_str());
        }
    }
    return 0;
}

bool FileReadRule::isREADEnable(const char *pUserString)
{
    int _dump = property_get_int32("vendor.debug.camera.dumpin.en", 0);
    if ((_dump&0x2) == 2)
       return true;
    else
       return false;
}
bool FileReadRule::isDumpEnable(const char *pUserString)
{
    int _dump = property_get_int32("vendor.debug.camera.dumpin.en", 0);
    if ((_dump&0x1) == 1)
       return true;
    else
       return false;
}

bool FileReadRule::DumpP2ForDP(const char* fname, const char* pP2Reg, int fsize)
{
#if MTK_DP_VERSION

#if MTK_DP_VERSION>= 9
    if (fsize > (P2_REG_SIZE*4))
    {
        ALOGD("P2 input size larger than DP structure size(%d)", fsize);
    }

    char strDump[512] = {'\0'};
    sprintf(strDump, "%s_p2.reg", fname);

    NSIspExifDebug::P2RegInfo_Dump_T rDebugInfo;
    rDebugInfo.u4TableSize = P2_REG_SIZE+1;
    rDebugInfo.u4HwVersion = MTK_DP_VERSION;
    memcpy(&rDebugInfo.regData, (MUINT32*)pP2Reg, P2_REG_SIZE*4);

    android::sp<IFileCache> fidDump;
    fidDump = IFileCache::open(strDump);

    if (fidDump->write(&rDebugInfo, sizeof(NSIspExifDebug::P2RegInfo_Dump_T)) != sizeof(NSIspExifDebug::P2RegInfo_Dump_T)){
        CAM_LOGW("[%s] write config error to %s", __FUNCTION__, strDump);
        return false;
    } else {
        ALOGD("[%s] %s pRegBuf size(%d)",__FUNCTION__, strDump, fsize);
        return true;
    }
#else
    if (fsize > (P2_BUFFER_SIZE*4))
    {
        ALOGD("P2 input size larger than DP structure size(%d)", fsize);
    }

    char strDump[512] = {'\0'};
    sprintf(strDump, "%s_p2.reg", fname);

    NSIspExifDebug::IspExifDebugInfo_T rDebugInfo;
    rDebugInfo.P2RegInfo.u4TableSize = P2_BUFFER_SIZE+1;
    rDebugInfo.P2RegInfo.u4HwVersion = MTK_DP_VERSION;
    memcpy(&rDebugInfo.P2RegInfo.regDataP2, pP2Reg, P2_BUFFER_SIZE*4);

    android::sp<IFileCache> fidDump;
    fidDump = IFileCache::open(strDump);

    if (fidDump->write(&rDebugInfo.P2RegInfo, sizeof(rDebugInfo.P2RegInfo)) != sizeof(rDebugInfo.P2RegInfo)){
        CAM_LOGW("[%s] write config error to %s", __FUNCTION__, strDump);
        return false;
    } else {
        ALOGD("[%s] %s pRegBuf size(%d)",__FUNCTION__, strDump, fsize);
        return true;
    }

#endif

#else
    return false;
#endif
}

bool FileReadRule::DumpP1ForDP(const char* fname, const char* pP1Reg, int fsize)
{
#if MTK_DP_VERSION

#if MTK_DP_VERSION>= 9
    if (fsize > (P1_REG_SIZE*4))
    {
        ALOGD("P1 input size larger than DP structure size(%d)", fsize);
    }

    char strDump[512] = {'\0'};
    sprintf(strDump, "%s_p1.reg", fname);

    NSIspExifDebug::P1RegInfo_Dump_T rDebugInfo;
    rDebugInfo.u4TableSize = P1_REG_SIZE+1;
    rDebugInfo.u4HwVersion = MTK_DP_VERSION;
    memcpy(&(rDebugInfo.regData), (MUINT32*)pP1Reg, P1_REG_SIZE*4);

    android::sp<IFileCache> fidDump;
    fidDump = IFileCache::open(strDump);

    if (fidDump->write(&rDebugInfo, sizeof(NSIspExifDebug::P1RegInfo_Dump_T)) != sizeof(NSIspExifDebug::P1RegInfo_Dump_T)){
        CAM_LOGW("[%s] write config error to %s", __FUNCTION__, strDump);
        return false;
    } else {
        ALOGD("[%s] %s pRegBuf size(%d)",__FUNCTION__, strDump, fsize);
        return true;
    }

#else

    if (fsize > (P1_BUFFER_SIZE*4))
    {
        ALOGD("P1 input size larger than DP structure size(%d)", fsize);
    }

    char strDump[512] = {'\0'};
    sprintf(strDump, "%s_p1.reg", fname);

    NSIspExifDebug::IspExifDebugInfo_T rDebugInfo;
    rDebugInfo.P1RegInfo.u4TableSize = P1_BUFFER_SIZE+1;
    rDebugInfo.P1RegInfo.u4HwVersion = MTK_DP_VERSION;
    memcpy(&(rDebugInfo.P1RegInfo.regData), pP1Reg, P1_BUFFER_SIZE*4);

    android::sp<IFileCache> fidDump;
    fidDump = IFileCache::open(strDump);

    if (fidDump->write(&rDebugInfo.P1RegInfo, sizeof(rDebugInfo.P1RegInfo)) != sizeof(rDebugInfo.P1RegInfo)){
        CAM_LOGW("[%s] write config error to %s", __FUNCTION__, strDump);
        return false;
    } else {
        ALOGD("[%s] %s pRegBuf size(%d)",__FUNCTION__, strDump, fsize);
        return true;
    }

#endif

#else
    return false;
#endif
}

bool FileReadRule::DumpMfbForDP(const char* fname, const char* pMfbReg, int fsize)
{
#if MTK_DP_VERSION
    if (fsize > (MFB_BUFFER_SIZE*4))
    {
        ALOGD("MFB input size larger than DP structure size(%d)", fsize);
    }

    char strDump[512] = {'\0'};
    sprintf(strDump, "/sdcard/camera_dump/%s_mfb.reg", fname);

    NSIspExifDebug::IspExifDebugInfo_T rDebugInfo;
    rDebugInfo.MFBRegInfo.u4TableSize = MFB_BUFFER_SIZE+1;
    rDebugInfo.MFBRegInfo.u4HwVersion = MTK_DP_VERSION;
    memcpy(&(rDebugInfo.MFBRegInfo.regDataMFB), pMfbReg, MFB_BUFFER_SIZE*4);

    android::sp<IFileCache> fidDump;
    fidDump = IFileCache::open(strDump);

    if (fidDump->write(&rDebugInfo.MFBRegInfo, sizeof(rDebugInfo.MFBRegInfo)) != sizeof(rDebugInfo.MFBRegInfo)){
        CAM_LOGW("[%s] write config error to %s", __FUNCTION__, strDump);
        return false;
    } else {
        ALOGD("[%s] %s pRegBuf size(%d)",__FUNCTION__, strDump, fsize);
        return true;
    }
#else
    return false;
#endif
}

bool FileReadRule::getFile_YNR_MAP(int reqNum, const char* ispProfile, IImageBuffer *pbuf, const char *pUserString, int SensorID)
{
    MBOOL ret = MTRUE;
    if (!isREADEnable(pUserString))
       return MFALSE;

    ALOGD("[%s] reqNum(%d) ispProfile(%s) pUserString(%s) pbuf(%x)",__FUNCTION__, reqNum, ispProfile, pUserString, pbuf);

    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str("default");
    std::string str_format("ynr_map");
    std::string str_ispProfile(ispProfile);

    char str_reqNum[50] = {'\0'};
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        TUNING_CFG_T cfg;
        Extract_Tuning_CFG(&str, &cfg);
        /*ALOGD("cfg str_format(%s), str_req(%s) str_profile(%s) str_path(%s)",
            cfg.str_format.c_str(), cfg.str_reqNum.c_str(), cfg.str_ispProfile.c_str(), cfg.str_answer.c_str());*/
        if (cfg.str_format.compare(str_format) == 0){ //format
            if ((std::string::npos != cfg.str_ispProfile.find("MFNR"))){
                if (cfg.str_reqNum.compare(str_reqNum) == 0){
                    str = cfg.str_answer;
                    break;
                }
            } else {
                if (cfg.str_format.compare(str_format) == 0){
                    str = cfg.str_answer;
                    break;
                }
            }
        }
    }
    if ((std::string::npos != str.find("default"))) //should not be default value
    {
        ALOGD("[%s] Fail find YNR_MAP path in config", __FUNCTION__);
        ret = MFALSE;
    }
    else {
        ALOGD("[%s] Load YNR_MAP from (%s)",__FUNCTION__, str.c_str());
        pbuf->unlockBuf(pUserString);
        ret = pbuf->loadFromFile(str.c_str());
        if (!ret)
            ALOGD("[%s] Fail to load YNR_MAP from (%s)", __FUNCTION__, str.c_str());
        const auto __usage = (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);
        pbuf->lockBuf(pUserString, __usage);
        pbuf->syncCache();
    }
    return ret;

}

bool FileReadRule::on_device_dump_file_rename(int reqNum, const char* ispProfile, FILE_DUMP_NAMING_HINT *pHint, const char *pUserString)
{
    if (!isREADEnable(pUserString))
       return MFALSE;
    char _config[1024] = {'\0'};
    ::property_get("vendor.debug.camera.dumpin.path.config", _config, "");
    std::ifstream infile(_config);
    std::string str;
    std::string str_format("frameNo");
    std::string str_ispProfile(ispProfile);

    char str_reqNum[50] = {'\0'};
    snprintf(str_reqNum, 50, "%04d", reqNum);

    while (std::getline(infile, str))
    {
        TUNING_CFG_T cfg;
        Extract_Tuning_CFG(&str, &cfg);

        if (cfg.str_format.compare(str_format) == 0){ //format
            if ((std::string::npos != cfg.str_ispProfile.find("MFNR"))){
                if (cfg.str_reqNum.compare(str_reqNum) == 0){
                    str = cfg.str_answer;
                    break;
                }
            } else {
                if (cfg.str_format.compare(str_format) == 0){
                    str = cfg.str_answer;
                    break;
                }
            }
        }
    }
    pHint->FrameNo = atoi(str.c_str());
    pHint->UniqueKey = getFile_uniqueKey("on_device_dump_file_rename", 0);
    pHint->RequestNo = getFile_requestNo("on_device_dump_file_rename", 0);
    return MTRUE;
}

} //namespace FileDump
} //namespace NSCam

