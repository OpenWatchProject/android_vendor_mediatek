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
#ifndef __FILE_READ_RULES_H__
#define __FILE_READ_RULES_H__

#include <string>
//tuning utils
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>

namespace NSCam {
namespace TuningUtils {

typedef enum
{
    E_Raw_intput,
    E_Lcso_Input,
    E_Lsc_Output,
    E_Tuning_Output,
    E_YUV_Output,
    E_MFB_Output,
    E_LTM_CURVE_Output,
    E_YNR_MAP_Output,
    E_LPCNR_Output
} ETuningFormat_T;

typedef struct {
    std::string str_format;
    std::string str_reqNum;
    std::string str_ispProfile;
    std::string str_usage;
    std::string str_answer;
} TUNING_CFG_T;

class FileReadRule
{

public:
    FileReadRule();
    bool getFile_RAW(int reqNum, const char* ispProfile, IImageBuffer *pbuf, const char *pUserString, int SensorID = 0);
    bool getFile_WarpingRaw(int reqNum, const char* ispProfile, IImageBuffer *pbuf, const char *pUserString, const char *pBufString, int SensorID = 0);
    bool getFile_LCSO(int reqNum, const char *ispProfile, IImageBuffer *pbuf, const char *pUserString, int SensorID = 0);
    bool getFile_RRZO(int reqNum, const char* ispProfile, IImageBuffer *pbuf, const char *pUserString, int SensorID = 0);
    bool getFile_LSC(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString = NULL);
    bool getFile_P2TUNING(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString = NULL);
    bool getFile_LPCNR_TUNING(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString = NULL);
    bool getFile_LTMCURVE(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString = NULL);
    //void getFile_YUV(int reqNum, const char *ispProfile, hint *phint, const char *pUserString, YUV_PORT type, int SensorID = 0);
    bool getFile_MFB(int reqNum, const char* ispProfile, char *filename, int filesize, const char *pUserString = NULL);
    bool getFile_YUV(int reqNum, const char *ispProfile, IImageBuffer *pbuf, const char *pUserString, YUV_PORT type, int SensorID = 0);
    bool isDumpEnable(const char *pUserString = NULL);
    bool isREADEnable(const char *pUserString = NULL);
    int  getFile_uniqueKey(const char *pUserString, int SensorID = 0);
    int  getFile_requestNo(const char *pUserString, int SensorID = 0);
    int  get_raw_stride(const char *pUserString, int SensorID = 0);
    bool DumpP2ForDP(const char* fname, const char* pP2Reg, int fsize);
    bool DumpP1ForDP(const char* fname, const char* pP1Reg, int fsize);
    bool DumpMfbForDP(const char* fname, const char* pMfbReg, int fsize);
    bool getFile_YNR_MAP(int reqNum, const char *ispProfile, IImageBuffer *pbuf, const char *pUserString, int SensorID = 0);
    bool on_device_dump_file_rename(int reqNum, const char* ispProfile, FILE_DUMP_NAMING_HINT *pHint, const char *pUserString = NULL);
};
}
}
#endif //__FILE_READ_RULES_H__
