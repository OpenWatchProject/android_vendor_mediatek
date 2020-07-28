/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "NvramDrv"

#include <sys/types.h>
#include <sys/stat.h>
#include <utils/Errors.h>
#include <mtkcam/utils/std/Log.h>
#include <fcntl.h>
#include <aaa_types.h>
#include <nvram_drv.h>
#include "nvram_drv_imp.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <mutex>

#ifdef NVRAM_SUPPORT
#include "camera_custom_msdk.h"
#endif

/*******************************************************************************
*
********************************************************************************/

/*******************************************************************************
*
********************************************************************************/
#define logI(fmt, arg...)    CAM_LOGD("[%s:%d] " fmt, __FUNCTION__, __LINE__, ##arg)
#define logE(fmt, arg...)    CAM_LOGE("[%s:%d] MError: " fmt, __FUNCTION__, __LINE__, ##arg)

/*******************************************************************************
*
********************************************************************************/

static bool gIsNvInit=0;
static std::mutex gNvLock;

extern void getNvFileName(int dev, int id, char* outName);


static int writeNv(int dev, int id, void* buf)
{
    logI("dev(%d) id(%d) buf(%p)",dev, id, buf);
    std::lock_guard<std::mutex> lock(gNvLock);
    int mask = umask(0);
    logI("umask(%d)",mask);
    if(gIsNvInit==0)
    {
        int ret;
        ret = mkdir(gCreateDir, S_IRWXU | S_IRWXG | S_IXOTH);
        logI("mkdir ret(%d)",ret);
        gIsNvInit = 1;
    }
    char fname[MAX_FILE_PATH_LEN];
    getNvFileName(dev, id, fname);
    FILE* fp;
    fp = fopen(fname,"wb");
    if(fp==0)
    {
        logE("fopen fail");
        umask(mask);
        return -1;
    }

    fwrite(buf,1,g_u4NvramDataSize[id],fp);
    fclose(fp);
    umask(mask);
    return 0;
}

static int readNv(int dev, int id, void* buf)
{
    logI("dev(%d) id(%d) buf(%p)",dev, id, buf);
    std::lock_guard<std::mutex> lock(gNvLock);
    int mask = umask(0);
    logI("umask(%d)",mask);
    if(gIsNvInit==0)
    {
        int ret;
        ret = mkdir(gCreateDir, S_IRWXU | S_IRWXG | S_IXOTH);
        logI("mkdir ret(%d)",ret);
        gIsNvInit = 1;
    }
    char fname[MAX_FILE_PATH_LEN];
    getNvFileName(dev, id, fname);
    FILE* fp;
    fp = fopen(fname,"rb");
    if(fp==0)
    {
        logE("fopen fail");
        umask(mask);
        return -1;
    }
    int ret = 0;
    ret = fread(buf,1,g_u4NvramDataSize[id],fp);
    if ((unsigned long)ret != g_u4NvramDataSize[id])
    {
        fclose(fp);
        logE("read fail");
        umask(mask);
        return -1;
    }
    fclose(fp);
    umask(mask);
    return 0;
}

static int getMs()
{
    int t;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = (ts.tv_sec*1000+ts.tv_nsec/1000000);
    return t;
}

static int isNvExist(int dev, int id)
{
    logI("dev(%d) id(%d)",dev, id);
    std::lock_guard<std::mutex> lock(gNvLock);
    FILE* fp;
    char fname[MAX_FILE_PATH_LEN];
    getNvFileName(dev, id, fname);
    fp = fopen(fname,"rb");
    if(fp==0)
    {
        logI("return 0");
        return 0;
    }
    fclose(fp);
    logI("return 1");
    return 1;
}



bool bCustomInit = 0; //[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
/*******************************************************************************
*
********************************************************************************/
NvramDrvBase*
NvramDrvBase::createInstance()
{
    return NvramDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
NvramDrvBase*
NvramDrv::getInstance()
{
    static NvramDrv singleton;
    return &singleton;
}

int NvramDrv::isNvramExist(int dev, CAMERA_DATA_TYPE_ENUM id)
{
    return isNvExist(dev, id);
}

/*******************************************************************************
*
********************************************************************************/
void
NvramDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::NvramDrv()
    : NvramDrvBase()
{
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::~NvramDrv()
{
}

/*******************************************************************************
*
********************************************************************************/

extern int checkDataVersionNew(
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    int version
);

int NvramDrv::readNoDefault(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                          CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
                          void *a_pNvramData,
                          unsigned long a_u4NvramDataSize)
{
    int err = NVRAM_NO_ERROR;
    logI("id(%d) dev(%d) a_u4NvramDataSize(%lu)",a_eSensorType,a_eNvramDataType, a_u4NvramDataSize);
    err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);

    if (err != NVRAM_NO_ERROR)
        logE("err(%d)", err);
    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readNvramData(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{
    logI("a_eSensorType(%d), a_eNvramDataType(%d)", a_eSensorType, a_eNvramDataType);
//seanlin 121221 avoid camera has not inited>
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
    if (!bCustomInit) {
        #ifndef USING_MTK_LDVT
        cameraCustomInit();
        if((a_eSensorType==DUAL_CAMERA_MAIN_2_SENSOR) || (a_eSensorType==DUAL_CAMERA_MAIN_SECOND_SENSOR))
            LensCustomInit(8);
        else
            LensCustomInit((unsigned int)a_eSensorType);
        #endif
        bCustomInit = 1;
    }
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
//seanlin 121221 avoid camera has not inited<

#ifdef NVRAM_SUPPORT

    readNv(a_eSensorType, a_eNvramDataType, a_pNvramData);

#endif
    unsigned char* dd;
    dd = (unsigned char*)a_pNvramData;
    logI("%d %d %d %d %d %d %d %d %d %d",
        (int)dd[0], (int)dd[1], (int)dd[2], (int)dd[3], (int)dd[4],
        (int)dd[5], (int)dd[6], (int)dd[7], (int)dd[8], (int)dd[9]);

    return NVRAM_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::writeNvramData(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
    void *a_pNvramData
)
{
    logI("+ a_eSensorType(%d), a_eNvramDataType(%d)", a_eSensorType, a_eNvramDataType);
    unsigned char* dd;
    dd = (unsigned char*)a_pNvramData;
    logI("%d %d %d %d %d %d %d %d %d %d",
        (int)dd[0], (int)dd[1], (int)dd[2], (int)dd[3], (int)dd[4],
        (int)dd[5], (int)dd[6], (int)dd[7], (int)dd[8], (int)dd[9]);

#ifdef NVRAM_SUPPORT

    writeNv(a_eSensorType, a_eNvramDataType, a_pNvramData);

#endif
    logI("-");

    return NVRAM_NO_ERROR;
}

int NvramDrv::writeNvramReal(void* buf, int id, int dev)
{
    logI("not support, buf(%p), id(%d), dev(%d)", buf, id, dev);
    return 0;
}

int NvramDrv::readNvramReal(void* buf, int id, int dev)
{
    logI("not support, buf(%p), id(%d), dev(%d)", buf, id, dev);
    return 0;
}

int NvramDrv::readMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
{
    logI("not support, buf(%p), bufSz(%d), idList(%p), num(%d), dev(%d)", buf, bufSz, (void*)idList, num, dev);
    return 0;
}


int NvramDrv::writeMultiNvram(void* buf, int bufSz, int* idList, int num, int dev)
{
    logI("not support, buf(%p), bufSz(%d), idList(%p), num(%d), dev(%d)", buf, bufSz, (void*)idList, num, dev);
    return 0;
}
