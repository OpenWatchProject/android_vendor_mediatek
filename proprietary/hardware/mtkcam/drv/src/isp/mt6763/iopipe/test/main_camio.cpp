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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG     "main_camio"

#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <queue>
#include <semaphore.h>
#include <pthread.h>

#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/threads.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/def/PriorityDefs.h>

using namespace NSCam;
using namespace NSCam::Utils;
using namespace android;
using namespace std;


#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/ICamsvStatisticPipe.h>

#include <ispio_pipe_ports.h>

/* For statistic dmao: eiso, lcso */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <imageio/Cam_Notify_datatype.h>
#include <tuning_mgr.h>
#include <mtkcam/drv/IHwSyncDrv.h>
#include "pic/bpci_tbl_s5k2x8.h"    //for PDO test
#include "cam_crop.h"              //for crop test

#include "npipeut.h"

using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

#define NPIPE_TM_KEY    "iopipeUseTM"

#define bitmap(bit)({\
        MUINT32 _fmt = 0;\
        switch(bit){\
        case 8: _fmt = eImgFmt_BAYER8; break;\
        case 10: _fmt = eImgFmt_BAYER10; break;\
        case 12: _fmt = eImgFmt_BAYER12; break;\
        default: _fmt = eImgFmt_BAYER10; break;\
        }\
        _fmt;})


#define __RRZ_FMT(fmt) ({                                           \
        MUINT32 __fmt;                                              \
        switch (fmt) {                                              \
        case eImgFmt_BAYER8: __fmt = eImgFmt_FG_BAYER8; break;      \
        case eImgFmt_BAYER10: __fmt = eImgFmt_FG_BAYER10; break;    \
        default: __fmt = eImgFmt_FG_BAYER10; break;                 \
        }                                                           \
        __fmt;                                                      \
})

enum {
    TG_INTERRUPT_TEST       = 0x00000001,
    EFUSE_TEST              = 0x00000002,
    PIPELINERAW10           = 0x00000004,
    PIPELINERAW12           = 0x00000008,
    PIPELINERAW14           = 0x00000010,
    PIPELINERAW16           = 0x00000020,
    UNI_DYNAMIC_SWITCH       = 0x00000100,
    ZVHDR_TUNING            = 0x01000000, //Need to enable one of sttport
    HLR_AND_HDR_TUNNG       = 0x02000000, //Need to enable one of sttport
    MAX_CMDS_FILED
};

typedef struct HwPortConfig {
    PortID          mPortID;
    EImageFormat    mFmt;
    MSize           mSize;  //dest size
    MRect           mCrop;
    MBOOL           mPureRaw;
    MUINT32         mStrideInByte[3];
} HwPortConfig_t;

#define BD_SEG_NUM (3+1) //13M, 16M, 20M, no limitration
struct efMappingTable
{
    int             TG_grabW;
    int             TG_grabH;
    const char*     resultStr;
};
efMappingTable efTable[BD_SEG_NUM]//TBD
{
    {5376, 4032,  "No limitation"},
    {5344, 3760,  "Bonding at 20M"},
    {5344, 3000,  "Bonding at 16M"},
    {4224, 3168,  "Bonding at 13M"},
};

static char *mThisName = (char*)LOG_TAG;

/******************************************************************************
 *
 ******************************************************************************/
static INormalPipeModule* getNormalPipeModule()
{
    static auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        MY_LOGE("INormalPipeModule::get() fail");
    }
    return pModule;
}

static MUINT32 selectNormalPipeVersion(MUINT32 sensorIdx)
{
    MUINT32 selectedVersion = 0;
    auto pModule = getNormalPipeModule();

    if  ( ! pModule ) {
        MY_LOGE("getNormalPipeModule() fail");
        return selectedVersion;
    }

    MUINT32 const* version = NULL;
    size_t count = 0;
    int err = pModule->get_sub_module_api_version(&version, &count, sensorIdx);
    if  ( err < 0 || ! count || ! version ) {
        MY_LOGE(
            "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
            sensorIdx, err, count, version
        );
        return selectedVersion;
    }

    selectedVersion = *(version + count - 1); //Select max. version
    MY_LOGD("[%d] count:%zu Selected CamIO Version:%0#x", sensorIdx, count, selectedVersion);
    return selectedVersion;
}


/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool saveBuf2File(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        printf(": failed to create file [%s]: %s \n", fname, ::strerror(errno));
        return false;
}

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            printf(": failed to write to file [%s]: %s\n", fname, ::strerror(errno));
            break;
    }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}

void onDumpBuffer(const char* usr, MUINT32 sensor_idx, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
//#define DUMP_PREFIX "/storage/sdcard0/cameradump_"
#define DUMP_PREFIX "/data/cameradump_"
//#define DUMP_PREFIX "/system/bin/cameradump_"
#define DUMP_IImageBuffer( type, pbuf, fileExt )                \
    do {                                                        \
        IImageBuffer* buffer = (IImageBuffer*)pbuf;             \
        char filename[256];                                     \
        sprintf(filename, "%s%d_%d_%dx%d_%d_%d.%s",             \
                dumppath,                                       \
                sensor_idx,                                     \
                type,                                           \
                buffer->getImgSize().w,buffer->getImgSize().h,  \
                (MINT32)buffer->getBufStridesInBytes(0),        \
                ext,                                            \
                fileExt                                         \
               );                                               \
        buffer->saveToFile(filename);                           \
        MY_LOGD("Dump x%x: %dx%d, stride %d to %s", (MUINT32)buffer->getBufPA(0), (MUINT32)buffer->getImgSize().w,\
                (MUINT32)buffer->getImgSize().h, (MUINT32)buffer->getBufStridesInBytes(0), filename);\
    } while(0)
    char dumppath[256];

    sprintf(dumppath, "%s%s/", DUMP_PREFIX, usr);

    if (!makePath(dumppath, 0660)) {
        MY_LOGD("makePath [%s] fail",dumppath);
        return;
    }

    DUMP_IImageBuffer(data, buf, "raw");
}

static bool saveBuf(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, %p, %d)", fname, buf, size);
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return true;
}

extern int test_camio_mgr(int argc, char** argv);
static void* open_another_cam(void* arg)
{
    NPipeUT*    _this = (NPipeUT*)arg;

    MUINT32     _sensorDev = 0;
    int         _argc = 0, i = 0;
    char**      _argv = NULL;

    _argc = _this->mArgc;

    MY_LOGD("switch cam: argc(%d)", _argc);

    _argv = (char **)malloc(sizeof(char *) * _argc);
    if (NULL == _argv) {
        MY_LOGD("switch cam: create another cam failed");
        goto _ANOTHER_FAIL;
    }
    memset(_argv, 0, sizeof(*_argv));

    MY_LOGD("switch cam: copy paramters: ");

    for (i = 0; i < _argc; i++) {
        #define CAM_ARG_LEN         (32)
        MY_LOGD("argv[%d]=(%s)", i, _this->mArgv[i]);

        if (CMD_IDX_SNRDEVID == i) { // switch sensorDev idx
            MUINT32     _sensorDev2 = 0;

            _sensorDev = atoi(_this->mArgv[i]);

            _argv[i] = (char *)malloc(CAM_ARG_LEN);
            if (NULL == _argv[i]) {
                goto _ANOTHER_FAIL;
            }

            if(0 == _sensorDev){
                _sensorDev = UT_MAIN_FKSNR;
            }

            if (_sensorDev & UT_MAIN_FKSNR) {
                _sensorDev2 = UT_SUB_FKSNR;
            }
            if (_sensorDev & UT_SUB_FKSNR) {
                _sensorDev2 = UT_MAIN_FKSNR;
            }
            if (_sensorDev & UT_MAIN_SNR) {
                _sensorDev2 = UT_SUB_SNR;
            }
            if (_sensorDev & UT_SUB_SNR) {
                _sensorDev2 = UT_MAIN_SNR;
            }
            MY_LOGD("Old sensorDev: 0x%x New sensorDev: 0x%x", _sensorDev, _sensorDev2);

            snprintf(_argv[i], CAM_ARG_LEN, "%d", _sensorDev2);

        } else if (CMD_IDX_SWITCHCAM == i) { // off switchCam after switched
            _argv[i] = (char *)malloc(CAM_ARG_LEN);
            if (NULL == _argv[i]) {
                goto _ANOTHER_FAIL;
            }

            snprintf(_argv[i], CAM_ARG_LEN, "%d", 0);

        } else {

            int _size = strlen(_this->mArgv[i]) * sizeof(char);

            _argv[i] = (char *)malloc(_size);
            if (NULL == _argv[i]) {
                goto _ANOTHER_FAIL;
            }

            strncpy(_argv[i], _this->mArgv[i], _size);
        }

        MY_LOGD("    : (%s)", _argv[i]);
    }

    //MY_LOGD("switch cam: post for switch ready");
    //::sem_post(&_this->m_semSwitchThd);

    MY_LOGD("switch cam: open another camera ...");
    test_camio_mgr(_argc, _argv);

    MY_LOGD("switch cam: post for switch ready");
    ::sem_post(&_this->m_semSwitchThd);

_ANOTHER_FAIL:

    if (_argv) {
        MY_LOGD("free argv[] ...");

        for (i = 0; i < _argc; i++) {
            if (_argv[i]) {
                free(_argv[i]);
            }
        }

        free(_argv);
    }

    return NULL;
}

int getSensorSize (MSize* pSize, SensorStaticInfo mSensorInfo, MUINT32 SenScenario)
{
    MBOOL ret = MTRUE;

#define scenario_case(scenario, KEY, pSize)       \
        case scenario:                            \
            (pSize)->w = mSensorInfo.KEY##Width;  \
            (pSize)->h = mSensorInfo.KEY##Height; \
            break;

    switch(SenScenario)
    {
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_VIDEO, video, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2, pSize );
        default:
            MY_LOGD("not support sensor scenario(0x%x)\n", SenScenario);
            ret = MFALSE;
            break;
    }
#undef scenario_case

    return ret;
}

MUINT32 getPortBitDepth(PortID _port, MUINT32 PortEnableMap)
{
    MUINT32 bitDepth = 0;

    if (_port.index == PORT_IMGO.index) {
        bitDepth = (PortEnableMap & ((__IMGO_ENABLE) << 8))? 12: 10;
    }
    else if (_port.index == PORT_RRZO.index) {
        bitDepth = (PortEnableMap & ((__RRZO_ENABLE) << 8))? 12: 10;
    }
    else if (_port.index == PORT_UFEO.index) {
        bitDepth = (PortEnableMap & ((__RRZO_ENABLE) << 8))? 12: 10;
    }
    else {
        MY_LOGE("Unsupported port depth query");
    }

    return bitDepth;
}

MBOOL getOutputFmt(PortID port, MUINT32 bitDepth, SensorStaticInfo& mSensorInfo, EImageFormat* pFmt)
{
    MBOOL ret = MFALSE;

#define case_Format( order_bit, mappedfmt, pFmt) \
        case order_bit:                          \
            (*(pFmt)) = mappedfmt;               \
        break;

    if ( mSensorInfo.sensorType == SENSOR_TYPE_YUV ) {
        switch( mSensorInfo.sensorFormatOrder ) {
        case_Format( SENSOR_FORMAT_ORDER_UYVY, eImgFmt_UYVY, pFmt);
        case_Format( SENSOR_FORMAT_ORDER_VYUY, eImgFmt_VYUY, pFmt);
        case_Format( SENSOR_FORMAT_ORDER_YUYV, eImgFmt_YUY2, pFmt);
        case_Format( SENSOR_FORMAT_ORDER_YVYU, eImgFmt_YVYU, pFmt);
        default:
            MY_LOGE("formatOrder not supported, 0x%x", mSensorInfo.sensorFormatOrder);
            goto lbExit;
        }
        //printf("sensortype:(0x%x), fmt(0x%x)\n", mSensorInfo.sensorType, *pFmt);
    }
    else if ( mSensorInfo.sensorType == SENSOR_TYPE_RAW ) {
        if (port.index == PORT_IMGO.index) {
            switch (bitDepth) {
            case_Format(  8, eImgFmt_BAYER8 , pFmt);
            case_Format( 10, eImgFmt_BAYER10, pFmt);
            case_Format( 12, eImgFmt_BAYER12, pFmt);
            case_Format( 14, eImgFmt_BAYER14, pFmt);
            default:
                MY_LOGE("IMGO bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if (port.index == PORT_RRZO.index) { //rrzo
            switch( bitDepth) {
            case_Format(  8, eImgFmt_FG_BAYER8 , pFmt);
            case_Format( 10, eImgFmt_FG_BAYER10, pFmt);
            case_Format( 12, eImgFmt_FG_BAYER12, pFmt);
            case_Format( 14, eImgFmt_FG_BAYER14, pFmt);
            default:
                MY_LOGE("RRZO bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if (port.index == PORT_UFEO.index) { //ufeo
            switch( bitDepth) {
            case_Format( 10, eImgFmt_UFEO_BAYER10, pFmt);
            case_Format( 12, eImgFmt_UFEO_BAYER12, pFmt);
            default:
                MY_LOGE("UFEO bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if ((port.index == PORT_EISO.index) ||
                (port.index == PORT_LCSO.index)) {
            MY_LOGE("port not supported, 0x%x", port.index);
            goto lbExit;
        }
    }
    else {
        MY_LOGE("sensorType not supported yet(0x%x)", mSensorInfo.sensorType);
        goto lbExit;
    }

    ret = MTRUE;

#undef case_Format

lbExit:
    return ret;
}

void Drop_Test(MUINT32 magic, void* cookie)
{
    cookie;
    MY_LOGD("Drop mag:%d\n",magic);
}

MBOOL CamPathCFG(_arg cam_arg, ENPipeCmd cmd, MUINT32 PathCFG)
{

    MBOOL ret = MTRUE;

    if(cam_arg.Ut_obj == NULL){
        MY_LOGE("obj not exist: obj_0x%p, cmd_0x%x, PathCFG_0x%x", cam_arg.Ut_obj, cmd, (E_CamHwPathCfg)PathCFG);
        return MFALSE;
    }

    switch(cmd){
        case ENPipeCmd_SET_HW_PATH_CFG:
            ret = cam_arg.Ut_obj->mpNPipe->sendCommand(cmd, (E_CamHwPathCfg)PathCFG, 0, 0);
            if(!ret){
                MY_LOGE("sendCommand fail: obj_0x%p, cmd_0x%x, PathCFG_0x%x", cam_arg.Ut_obj, cmd, (E_CamHwPathCfg)PathCFG);
                return ret;
            }
            break;
        case ENPipeCmd_GET_HW_PATH_CFG:
            {
                E_CamHwPathCfg PathStatus = eCamHwPathCfg_One_TG;
                while(1){
                    if (cam_arg.Ut_obj->mEnqCount > 1){ //check Normalpipe is finish configpipe phrase
                        ret = cam_arg.Ut_obj->mpNPipe->sendCommand(cmd, (MINTPTR)&PathStatus, 0, 0);
                        if (!ret){
                            MY_LOGE("sendCommand fail: obj_0x%p, cmd_0x%x, PathCFG_0x%x", cam_arg.Ut_obj, cmd, (E_CamHwPathCfg)PathCFG);
                            return ret;
                        }
                        if (PathStatus == (E_CamHwPathCfg)PathCFG){
                            MY_LOGD("#########CFG is switched_%d#########",PathStatus);
                            break;
                        }
                    }
                    usleep(30000);
                }
            }
            break;
        default:
            MY_LOGE("Not support this cmd: obj_0x%p, cmd_0x%x, PathCFG_0x%x", cam_arg.Ut_obj, cmd, (E_CamHwPathCfg)PathCFG);
            ret = MFALSE;
            break;
    }
    return ret;
}

/*******************************************************************************
*  Main Function
********************************************************************************/
typedef enum {
    eNormal = 0,
    eDualPD,
    eQuadCode
} eDataPattern;
static MVOID* test_camio_thread(void *ts_arg)
{
    char                filename[256];
    MUINT32             i = 0, j = 0, sidx = 0, dma = 0, bSetSensorTP = 0;
    MUINT32             sensorDev, scenario, sensorFps, enablePort,  sensorNum = 0, sttPorts = 0;
    CROP_TEST::ECROP    croptestsel = CROP_TEST::CROP_NONE;
    MUINT32             pixelMode[2] = {0}, stopFlow = 0, waitTgInt = 0;
    MUINT32             twinCtrl = 1, sensorPixMode = 1, forceBinOff = 0, cmdsfiled=0;
    eDataPattern        dataPat = eNormal;
    uint32_t            sensorArray[_MAXTG_];
    SensorStaticInfo    mSensorInfo[_MAXTG_];
    IHalSensor*         pSensorHalObj[_MAXTG_] = {NULL};
    vector<IHalSensor::ConfigParam> vSensorCfg[2];
    IHalSensorList*     pHalSensorList = NULL;
    IMEM_BUF_INFO       imgiBuf;
    NPipeUT*            pUt[_MAXTG_] = {NULL};
    MBOOL               stN3D = MFALSE, _efuse = MFALSE, m_bN3DTest = MFALSE, ret = MFALSE;
    MINT32              _efuse_idx = BD_SEG_NUM - 1;
    MSize               size;//specific size
#ifndef HWSYNC_TEST
    MUINT32             _frameNum = 8;
#else
    MUINT32             _frameNum = 64;
#endif
    MUINT32             _burstQNum = 1, _switchCam = 0;
    pthread_t           _switchThd;
    MUINT32             checksum[2] = {0}, crc_accum[2] = {0}, suspendFrm = 0, suspendFlow = 0, suspendTest = 0;
    MBOOL               crc_test_result = MTRUE;
    _arg*               arg;
    MUINT32             temp_loopCnt = 0;

    arg = (_arg*)ts_arg;

#if 1
    MY_LOGD("sizeof long : %d", (int)sizeof(long));
    if (arg->_argc < 5) { // iopipeTest 1 0 0 2 0, test pattern no crop
        MY_LOGI("Param: 1 <sensorDev> <scenario> <enablePort> <cropTestSel> "\
            "<sttPorts> <burstQNum> <FrameNum> <stopFlow> <switchCam> <twinCtrl> <combinedcmds> <suspend_test>");
        MY_LOGI("<sensorDev>    : 1 main, 2 sub, bitmap; 0 for seninf test model");
        MY_LOGI("<scenario>     : preview %d, capture %d, video %d, slim1 %d, slim2 %d, sensorTestPattern 255",\
                SENSOR_SCENARIO_ID_NORMAL_PREVIEW, SENSOR_SCENARIO_ID_NORMAL_CAPTURE,\
                SENSOR_SCENARIO_ID_NORMAL_VIDEO, SENSOR_SCENARIO_ID_SLIM_VIDEO1, SENSOR_SCENARIO_ID_SLIM_VIDEO2);
        MY_LOGI("<enablePort>   : 0x01 rrzo, 0x02 imgo, 0x04 eiso, 0x08 lcso, 0x10 rrzo+ufeo bitmap, 0x100");
        MY_LOGI("<cropTestSel>  : 0 no crop, 1 symmetric crop, 2 crop left only, 3 crop right only,4 specific size");
        MY_LOGI("<sttPorts>     : 1 aao, 2 afo, 4 flko, 8 pdo, 8192 camsv imgo, bitmap");
        MY_LOGI("<burstQNum>    : 1 for normal");
        MY_LOGI("<FrameNum>     : 1 for 1 frame");
        MY_LOGI("<stopFlow>     : 0 : blocking stop, thread 1~2 vsync, cam 1 vsync");
        MY_LOGI("               : 1 : non-blocking stop, thread 0 vsync, cam 1 vsync");
        MY_LOGI("               : 2 : abort, no wait");
        MY_LOGI("<switchCam>    : 0 : no, means on sensor");
        MY_LOGI("               : 1 : after uninit switch to another cam");
        MY_LOGI("               : 2 : PIP, two sensor");
        MY_LOGI("               : 3 : non-stop test dynamic twin (Twin -> Single)");
        MY_LOGI("               : 4 : non-stop test dynamic twin by suspend/resume (Single -> Twin)");
        MY_LOGI("<twinCtrl>     : 0x01 1-pix, 0x02 2-pix");
        MY_LOGI("               : 0x10 : bin force off");
        MY_LOGI("               : 0x20 : Dbin force on");
        MY_LOGI("               : 0x30 : quad code pattern");
        MY_LOGI("<combinedcmds> : (input 0x00000011) 0: no, 0x1:TG_INTERRUPT_TEST, 0x2:EFUSE_TEST, 0x4:PIPELINE_RAW10, 0x8:PIPELINE_RAW12, 0x10:PIPELINE_RAW14, 0x20:PIPELINE_RAW16");
        MY_LOGI("<suspend_test> : b'[0]     : 0).off 1).on");
        MY_LOGI("               : b'[3:1]   : 0).one time suspend->resume 1).non-stop suspend->resume(dynamic twin) 2).suspend->stop");
        MY_LOGI("               : b'[15:4]  : frame# for enter suspend for random time");
        return NULL;
    }

    sensorDev = atoi(arg->_argv[CMD_IDX_SNRDEVID]);
    scenario  = atoi(arg->_argv[CMD_IDX_SNRSCEN]);
    enablePort = atoi(arg->_argv[CMD_IDX_ENABLEPORTS]);

    switch(atoi(arg->_argv[CMD_IDX_CROPSEL])){
        case 0: croptestsel = CROP_TEST::CROP_NONE;
            break;
        case 1: croptestsel = CROP_TEST::CROP_SYSMATRIC;
            break;
        case 2: croptestsel = CROP_TEST::CROP_LEFT_ONLY;
            break;
        case 3: croptestsel = CROP_TEST::CROP_RIGHT_ONLY;
            break;
        case 4:
            {
                char str[32];
                char _str[2][16];
                char* ptr=0;
                int i=0;
                ptr = str;
                str[0] = _str[0][0] = _str[1][0] = '\0';

                MY_LOGI("Input specific out size,Param:<output width>,<output height>");
                fgets(str,sizeof(str),stdin);

                for(;*ptr!='\0';ptr++){
                    if(*ptr == ',')
                        i++;
                    else
                        strncat(_str[i],ptr,sizeof(char));
                }

                size.w = atoi(_str[0]);
                size.h = atoi(_str[1]);
            }
        default:
            break;
    }

    if (arg->_argc > CMD_IDX_STTPORTS) {
        sttPorts = atoi(arg->_argv[CMD_IDX_STTPORTS]);
    }
    if (arg->_argc > CMD_IDX_BURSTNUM) {
        _burstQNum = atoi(arg->_argv[CMD_IDX_BURSTNUM]);
    }
    else {
        _burstQNum = 1;
    }
    if (arg->_argc > CMD_IDX_FRAMENUM) {
        _frameNum = atoi(arg->_argv[CMD_IDX_FRAMENUM]);
    }
    if (arg->_argc > CMD_IDX_STOPFLOW) {
        /*
         * 0 : blocking stop, as-is 2 ~ 3 vsync
         * 1 : non-blocking stop, wait 1 vsync
         * 2 : abort, no wait
         */
        stopFlow = atoi(arg->_argv[CMD_IDX_STOPFLOW]);
        MY_LOGD("stopFlow : %d", stopFlow);

        if (stopFlow > 2) {
            stopFlow = 1;
            MY_LOGD("stopFlow set %d", stopFlow);
        }
    }
    if (arg->_argc > CMD_IDX_SWITCHCAM) {
        if(_switchCam < (MUINT32)SWITCH_CAM_ENUM_MAX){
            _switchCam = atoi(arg->_argv[CMD_IDX_SWITCHCAM]);
        }
        else{
            MY_LOGE("command not support\n");
            return NULL;
        }
    }
    if (arg->_argc > CMD_IDX_PIXELMODE) {
        twinCtrl = atoi(arg->_argv[CMD_IDX_PIXELMODE]);
        sensorPixMode = twinCtrl & 0xF;

        switch ((twinCtrl & 0xF0) >> 4) {
        case 1:
            dataPat = eNormal;
            forceBinOff = 1;
            break;
        case 2:
            dataPat = eDualPD;
            if(sensorDev != 0){
                MY_LOGE("dbn support only in fake sensor\n");
                return NULL;
            }
            if(scenario != SENSOR_SCENARIO_ID_NORMAL_PREVIEW ){
                MY_LOGE("dbn support only in preview, no capture\n");
                return NULL;
            }
            break;
        case 3:
            dataPat = eQuadCode;
            break;
        default:
            dataPat = eNormal;
            break;
        }

        MY_LOGD("twinCtrl : %d", twinCtrl);
    }

    if(arg->_argc > CMD_IDX_CMDS_FILED) {
        cmdsfiled = (MUINT32)strtol(arg->_argv[CMD_IDX_CMDS_FILED], NULL, 16);
        MY_LOGD("cmdsfiled : %x", cmdsfiled);

        if(cmdsfiled & EFUSE_TEST) {
            _efuse = MTRUE;
            scenario = 1;
            sensorDev = UT_MAIN_FKSNR;
            MY_LOGD("eFuse test: sce_%d, sdev_%d", \
                scenario, sensorDev);
        }

        if(cmdsfiled & TG_INTERRUPT_TEST) {
            waitTgInt = 1;
            MY_LOGD("waitTgInt : %d", waitTgInt);
        }
    }
    if (arg->_argc > CMD_IDX_SUSPEND) {
        suspendTest = (atoi(arg->_argv[CMD_IDX_SUSPEND]) & 0x0001);
        if(suspendTest){
            suspendFlow = (atoi(arg->_argv[CMD_IDX_SUSPEND]) & 0xF) >> 1;
            if(suspendFlow < (MUINT32)SUSPEND_FLOW_MAX){
                suspendFrm = (atoi(arg->_argv[CMD_IDX_SUSPEND]) >> 4) & 0xFFF;
                MY_LOGD("Test%d Frame#%d enable SW %d test flow", suspendTest, suspendFrm, suspendFlow);
            }
            else{
                MY_LOGE("case not support");
                return NULL;
            }
        }
    }

    if (arg->_argc > CMD_IDX_N3DEN) {
        stN3D = !!atoi(arg->_argv[CMD_IDX_SUSPEND]);
        MY_LOGD("N3D control: 0x%x", stN3D);
    }

    MY_LOGD("sttPorts 0x%08x", sttPorts);
    if (sttPorts) {
        if (sttPorts & __AAO_ENABLE) {
            MY_LOGD("enable aao");
        }
        if (sttPorts & __AFO_ENABLE) {
            MY_LOGD("enable afo");
        }
        if (sttPorts & __FLKO_ENABLE) {
            MY_LOGD("enable FLKO");
        }
        if (sttPorts & __CAMSV_IMGO_ENABLE) {
            MY_LOGD("output CAMSV IMGO");
        }
        if(sttPorts & __PDO_ENABLE){
            MY_LOGD("enable PDO\n");
        }
        if(sttPorts & __PSO_ENABLE){
            MY_LOGD("enable PSO\n");
        }
    }

    sensorNum = 0;
    if (0 == sensorDev) {
        sensorDev = UT_MAIN_FKSNR;
    }
    if (sensorDev & (UT_MAIN_SNR | UT_MAIN_FKSNR)) {
        sensorArray[sensorNum] = 0;
        sensorNum++;
    }
    if (sensorDev & (UT_SUB_SNR | UT_SUB_FKSNR)) {
        sensorArray[sensorNum] = 1;
        sensorNum++;
    }
    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_MAIN2_SNR | UT_MAIN2_FKSNR)) {
            sensorArray[sensorNum] = 2;
            sensorNum++;
        }
    }
    if (sensorDev & (UT_MAIN_FKSNR | UT_SUB_FKSNR | UT_MAIN2_FKSNR)) {
        mThisName = (char*)NPIPE_TM_KEY;
    }

#ifdef HWSYNC_TEST
    if(sensorDev & (UT_MAIN_SNR | UT_SUB_SNR))
        m_bN3DTest = MTRUE;
    else
        m_bN3DTest = MFALSE;
#endif

    if (255 == scenario) {
        MY_LOGD("using sensor test pattern");
        scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        bSetSensorTP = 1;
    }

    MY_LOGD("sensorDev_0x%x senario_0x%x enablePort_0x%x sensorNum_0x%x croptestsel_0x%x sttPort_0x%x",\
            sensorDev, scenario,  enablePort, sensorNum, croptestsel, sttPorts);
    MY_LOGD("supported_bit_%d/12 ", SEN_PIX_BITDEPTH);
    MY_LOGD("sensorArray: %d, %d", sensorArray[0], sensorArray[1]);
    MY_LOGD("burstNum: %d", _burstQNum);
    MY_LOGD("twinCtrl: %d , pix/bin_off/datPat(%d/%d/0x%x), ", twinCtrl, sensorPixMode, forceBinOff, (int)dataPat);
    MY_LOGD("efuse: %d", _efuse);
#endif
    #if 0 // force config smi larb mmu setting to PA, for ion carve-out
    {
        #include "isp_drv_cam.h"

        ISP_DRV_CAM *ptr;
        ISP_LARB_MMU_STRUCT larbInfo;
        int idx = 0;

        MY_LOGD("#################################################");
        MY_LOGD("#################################################");
        MY_LOGD("Force config SMI_LARB to PA temporarily");
        MY_LOGD("#################################################");
        MY_LOGD("#################################################");

        ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_A");
        if(ptr == NULL){
            MY_LOGE("CAM_A create fail\n");
        }
        MY_LOGD("Create ISP_DRV_CAM obj : %p", ptr);

        if(ptr->init("CONFIG_ISP_DMA_PORT") == MFALSE){
            ptr->destroyInstance();
            MY_LOGE("CAM_A init failure\n");
            ptr = NULL;
        }

        larbInfo.regVal = 0;

        larbInfo.LarbNum = 2;
        for (idx = 0; idx < 3; idx++) {
            larbInfo.regOffset = 0x380 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

            larbInfo.regOffset = 0xf80 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
        }

        larbInfo.LarbNum = 3;
        for (idx = 0; idx < 5; idx++) {
            larbInfo.regOffset = 0x380 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

            larbInfo.regOffset = 0xf80 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
        }

        larbInfo.LarbNum = 5;
        for (idx = 0; idx < 12; idx++) {
            larbInfo.regOffset = 0x380 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

            larbInfo.regOffset = 0xf80 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
        }

        larbInfo.LarbNum = 6;
        for (idx = 0; idx < 19; idx++) {
            larbInfo.regOffset = 0x380 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

            larbInfo.regOffset = 0xf80 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

        }

        ptr->uninit("CONFIG_ISP_DMA_PORT");
        ptr->destroyInstance();
        MY_LOGD("Config SMI LARB done");
    }
    #endif

    /*
     * sensor create/query
     */
    if (sensorDev & UT_SNR_MASK) {
        pHalSensorList = IHalSensorList::get();
    }
    else {
        pHalSensorList = TS_FakeSensorList::getTestModel();
    }

SEARCH_SENSOR:

    MY_LOGD("search sensor...");
    pHalSensorList->searchSensors();

    for (sidx = 0; sidx < sensorNum; sidx++) {
        MUINT32 sttDma = 0;

        pUt[sidx] = NPipeUT::create();
        arg->Ut_obj = pUt[sidx];

        if (sttPorts & __CAMSV_IMGO_ENABLE) {
            pUt[sidx]->mbCamsvEn = MTRUE;
            pUt[sidx]->mEnableSttPort = (sttPorts & ~__CAMSV_IMGO_ENABLE);
        }
        else {
        pUt[sidx]->mEnableSttPort = sttPorts; //modify for uni
        }
        pUt[sidx]->mStaticEnqCnt = 2; //should > 0
        pUt[sidx]->mFrameNum = _frameNum;
        pUt[sidx]->mBurstQNum = _burstQNum;
        pUt[sidx]->mPortBufDepth = _burstQNum * ENQUE_BURST_CNT;
        pUt[sidx]->mArgc = arg->_argc;
        pUt[sidx]->mArgv = arg->_argv;
        pUt[sidx]->mEnableTgInt = waitTgInt;
        pUt[sidx]->m_Dualpd.Bits.bDual = (dataPat == eDualPD);
        if (pUt[sidx]->m_Dualpd.Bits.bDual) {
            int str;
            MY_LOGI("Input PD density,Param:1 for High, 0 for Low");
            str = getchar();
            pUt[sidx]->m_Dualpd.Bits.Density = (str == '1')?(1):(0);
        }
        pUt[sidx]->mSuspendTest = suspendTest;
        pUt[sidx]->mSuspendFrm = suspendFrm;
        pUt[sidx]->mSuspendFlow = suspendFlow;
        pUt[sidx]->mSuspendState = 0;

        ::sem_init(&pUt[sidx]->m_semSwitchThd, 0, 0);

        if(enablePort & __UFEO_ENABLE){
            pUt[sidx]->m_ufeo_en = 1;
            enablePort ^= __UFEO_ENABLE;
            enablePort |= __RRZO_ENABLE;
        }
        else{
            pUt[sidx]->m_ufeo_en = 0;
        }
        //uni check
        if ((enablePort & __EISO_ENABLE) && (sidx > 0)) {
            if(cmdsfiled & UNI_DYNAMIC_SWITCH)
                pUt[sidx]->m_enablePort = enablePort; //modify for uni
            else {
                pUt[sidx]->m_enablePort = (enablePort & ~__EISO_ENABLE); //uni
                pUt[sidx]->m_enablePort &= (~__RSSO_ENABLE); // remove rsso from cam_b
            }
        }
        else {
            pUt[sidx]->m_enablePort = enablePort; //modify for uni
        }
        if ((sttPorts & __FLKO_ENABLE) && (sidx > 0)) {
            pUt[sidx]->mEnableSttPort = (sttPorts & ~__FLKO_ENABLE);
        }

        if (pUt[sidx]->mStaticEnqCnt > ENQUE_BURST_CNT) {
            pUt[sidx]->mStaticEnqCnt = ENQUE_BURST_CNT;
        }

        if(cmdsfiled & UNI_DYNAMIC_SWITCH) {
            pUt[sidx]->m_uniSwitch = MTRUE;
            MY_LOGD("UNI dynamic switch for dual-zoom feature is enabled");
        }

        MY_LOGD("create %d obj : [%d] %p", sensorNum, sidx, (MUINT8*)pUt[sidx]);
    }

    for (sidx = 0; sidx < sensorNum; sidx++) {
        pHalSensorList->querySensorStaticInfo(
                pHalSensorList->querySensorDevIdx(sensorArray[sidx]), &mSensorInfo[sidx]);
    }

    for (sidx = 0; sidx < sensorNum; sidx++) {
        pSensorHalObj[sidx] = pHalSensorList->createSensor(mThisName, sensorArray[sidx]);
        if (pSensorHalObj[sidx] == NULL) {
            MY_LOGE("mpSensorHalObj is NULL");
            return NULL;
        }

        if (!(sensorDev & UT_SNR_MASK)) {
            ((TS_FakeSensor*)pSensorHalObj[sidx])->setFakeAttribute(TS_FakeSensor::eFake_PixelMode, (MUINTPTR)sensorPixMode);

            if (pUt[sidx]->mbCamsvEn) {
                ((TS_FakeSensor*)pSensorHalObj[sidx])->setFakeAttribute(TS_FakeSensor::eFake_CamsvChannel, (MUINTPTR)MTRUE);
            }
        }

        pSensorHalObj[sidx]->powerOn(mThisName, 1, &sensorArray[sidx]);
    }

    MY_LOGD("create...");

    for (sidx = 0; sidx < sensorNum; sidx++) {
        MY_LOGD("SenIdx: %d", sensorArray[sidx]);

        pUt[sidx]->mSensorIdx = sensorArray[sidx];

        getNormalPipeModule()->createSubModule(
            sensorArray[sidx], mThisName, selectNormalPipeVersion(sensorArray[sidx]), (MVOID**)&(pUt[sidx]->mpNPipe));

        ret = pUt[sidx]->mpNPipe->init();
        if (MFALSE == ret) {
            MY_LOGE("normalPipe init error");
            return NULL;
        }
    }

    if(_efuse){
        MY_LOGD("BOUNDING_%d_%s [%dx%d] ", _efuse_idx, \
            efTable[_efuse_idx].resultStr, \
            efTable[_efuse_idx].TG_grabW, \
            efTable[_efuse_idx].TG_grabH);
    }
    MY_LOGD("prepare sensor config...");
    /*
     * prepare sensor config
     */
    for (sidx = 0; sidx < sensorNum; sidx++) {
        getSensorSize(&pUt[sidx]->mTgSize, mSensorInfo[sidx], scenario);
        if(_efuse) {
            pUt[sidx]->m_bEfuse = MTRUE;
            pUt[sidx]->mEfuseIdx = _efuse_idx;
            pUt[sidx]->mTgSize.w = efTable[_efuse_idx].TG_grabW;
            pUt[sidx]->mTgSize.h = efTable[_efuse_idx].TG_grabH;
        }

        //cfg sensor's fps info
        switch(scenario){
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                sensorFps = mSensorInfo[sidx].previewFrameRate;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                sensorFps = mSensorInfo[sidx].captureFrameRate;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                sensorFps = mSensorInfo[sidx].videoFrameRate;
                break;
        }

        IHalSensor::ConfigParam sensorCfg =
        {
            (MUINT)pUt[sidx]->mSensorIdx,   /* index            */
            pUt[sidx]->mTgSize,             /* crop             */
            scenario,                       /* scenarioId       */
            0,                              /* isBypassScenario */
            1,                              /* isContinuous     */
            MFALSE,                         /* iHDROn           */
            sensorFps/10,                   /* framerate        */
            0,                              /* two pixel on     */
            0,                              /* debugmode           */
            0,                              /* exposureTime       */
            0,                              /* gain                      */
            0,                              /* exposureTime_se  */
            0,                              /* gain_se                 */
        };
        sensorCfg.twopixelOn = (sensorPixMode & 0x2) >> 1; // JSS, pixel mode

        vSensorCfg[sidx].push_back(sensorCfg);



        MY_LOGD("senidx:0x%x, sensor %dx%d, sce %d, bypass %d, con %d, hdr %d, fps %d, twopxl %d",
                pUt[sidx]->mSensorIdx, sensorCfg.crop.w, sensorCfg.crop.h, sensorCfg.scenarioId, sensorCfg.isBypassScenario,
                sensorCfg.isContinuous, sensorCfg.HDRMode, sensorCfg.framerate, sensorCfg.twopixelOn);
    }

    MY_LOGD("dmao cfg...");

    EImageFormat fmt;
    MRect _cropsize;
    MSize _destsize;
    list<HwPortConfig_t> lHwPortCfg[2];

    for (sidx = 0; sidx < sensorNum; sidx++) {

        //crop mode
        pUt[sidx]->m_crop.InitCfg(pUt[sidx]->mTgSize,croptestsel,pUt[sidx]->mTgSize);


        if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
            NormalPipe_QueryInfo qry;

            MY_LOGD("0x%x: port IMGO", sidx);

            if (!getOutputFmt(PORT_IMGO, getPortBitDepth(PORT_IMGO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                MY_LOGE("get pix fmt error");
                return NULL;
            }

            getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                fmt, pUt[sidx]->mTgSize.w, qry);

            //set/get crop info
            if(croptestsel == CROP_TEST::CROP_SPECIFIC){
                pUt[sidx]->m_crop.SetInfo(PORT_IMGO,fmt,size);
            }
            {
                MSize imgo_size[2];
                pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_IMGO_INFO,(MINTPTR)imgo_size,0,0);
                if(imgo_size[0].w == 0)
                    pUt[sidx]->m_crop.GetCropInfo(PORT_IMGO,fmt,_cropsize,MTRUE);
                else
                    pUt[sidx]->m_crop.GetCropInfo(PORT_IMGO,fmt,_cropsize,MFALSE);
            }
            //pUt[sidx]->m_crop.GetScaleInfo(PORT_IMGO,fmt,_destsize);

            MY_LOGD("imgo query x_pix:(%d) x_strid_byte:(%d)", qry.x_pix, qry.stride_byte);
            MY_LOGD("sensor size width:(%d) x height:(%d)", pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h);
            MY_LOGD("crop size (%d_%d),(%d_%d)", _cropsize.p.x, _cropsize.p.y,_cropsize.s.w, _cropsize.s.h);
            MY_LOGD("dest size width:(%d) x height:(%d)", _cropsize.s.w, _cropsize.s.h);

            {
                HwPortConfig_t full = {
                    PORT_IMGO,
                    fmt,
                    _cropsize.s,
                    _cropsize,
                    (pUt[sidx]->m_enablePort & __IMGO_PURE_RAW)?1:0, //if raw type != 1 -> pure-raw
                    { qry.stride_byte, 0, 0 } //mStrideInByte[3]
                };

                lHwPortCfg[sidx].push_back(full);
            }
        }


        if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
            NormalPipe_QueryInfo qry;

            if(pUt[sidx]->m_ufeo_en == 1){
                 if (!getOutputFmt(PORT_UFEO, getPortBitDepth(PORT_UFEO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                    MY_LOGE("get pix fmt error");
                    return NULL;
                 }
            }
            else{
                if (!getOutputFmt(PORT_RRZO, getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                    MY_LOGE("get pix fmt error");
                    return NULL;
                }
            }
            //set/get crop info
            if(croptestsel == CROP_TEST::CROP_SPECIFIC){
                pUt[sidx]->m_crop.SetInfo(PORT_RRZO,fmt,size);
            }
            pUt[sidx]->m_crop.GetCropInfo(PORT_RRZO,fmt,_cropsize);
            pUt[sidx]->m_crop.GetScaleInfo(PORT_RRZO,fmt,_destsize);

            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_STRIDE_BYTE,
                                fmt, _cropsize.s.w, qry);

            {
                HwPortConfig_t resized = {
                    PORT_RRZO,
                    fmt,
                    _destsize,
                    _cropsize,
                    0, //if raw type != 1 -> pure-raw
                    { qry.stride_byte, 0, 0 }
                };

                lHwPortCfg[sidx].push_back(resized);
            }

            MY_LOGD("rrzo query x_pix:(%d) x_strid_byte:(%d)", qry.x_pix, qry.stride_byte);
            MY_LOGD("scale_size: %d_%d", _destsize.w, _destsize.h);
            MY_LOGD("crop size (%d_%d),(%d_%d)", _cropsize.p.x, _cropsize.p.y,_cropsize.s.w, _cropsize.s.h);
        }
        if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
            MSize _ssize(EISO_SIZE, 1);
            HwPortConfig_t eiso_cfg = {
                PORT_EISO,
                eImgFmt_BLOB,
                _ssize,
                MRect(MPoint(0, 0), _ssize),
                0,
                { (MUINT32)_ssize.w, 0, 0 }
            };

            MY_LOGD("0x%x: port EISO", sidx);

            lHwPortCfg[sidx].push_back(eiso_cfg);
        }
        if(pUt[sidx]->m_enablePort & __RSSO_ENABLE) {
            MSize _ssize(288, 511);
            HwPortConfig_t rsso_cfg = {
                PORT_RSSO,
                eImgFmt_BLOB,
                _ssize,
                MRect(MPoint(0, 0), _ssize),
                0,
                { (MUINT32)_ssize.w, 0, 0 }
            };

            MY_LOGD("0x%x: port RSSO", sidx);

            lHwPortCfg[sidx].push_back(rsso_cfg);
        }
        if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
            MSize _ssize = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize);
            HwPortConfig_t lcso_cfg = {
                PORT_LCSO,
                eImgFmt_BLOB,
                _ssize,
                MRect(MPoint(0, 0), _ssize),
                0,
                { (MUINT32)_ssize.w, 0, 0 }
            };

            MY_LOGD("0x%x: port LCSO, size_%d_%d", sidx, _ssize.w, _ssize.h);

            lHwPortCfg[sidx].push_back(lcso_cfg);
        }

        MY_LOGD("%d hwport size:%d", sidx, (MUINT32)lHwPortCfg[sidx].size());
    }

    /*
     * NormalPipe configPipe
     */
    MY_LOGD("configPipe...");

    vector<portInfo> vPortInfo[2];
    list<HwPortConfig_t>::const_iterator pPortCfg;

    for (sidx = 0; sidx < sensorNum; sidx++) {
        NormalPipe_QueryInfo    qry;

        for (pPortCfg = lHwPortCfg[sidx].begin(); pPortCfg != lHwPortCfg[sidx].end(); pPortCfg++) {
            portInfo OutPort(
                    pPortCfg->mPortID,
                    pPortCfg->mFmt,
                    pPortCfg->mSize, //dst size
                    pPortCfg->mCrop, //crop
                    pPortCfg->mStrideInByte[0],
                    pPortCfg->mStrideInByte[1],
                    pPortCfg->mStrideInByte[2],
                    pPortCfg->mPureRaw, // pureraw
                    MTRUE               //packed
                    );
            vPortInfo[sidx].push_back(OutPort);

            MY_LOGD("configPipe: 0x%x id:0x%x, crop:%d,%d,%dx%d, size:%dx%d, fmt:0x%x, stride:%d, pureraw:%d",\
                    sidx,\
                    pPortCfg->mPortID.index,\
                    pPortCfg->mCrop.p.x,pPortCfg->mCrop.p.y,pPortCfg->mCrop.s.w,pPortCfg->mCrop.s.h,\
                    pPortCfg->mSize.w,pPortCfg->mSize.h,\
                    pPortCfg->mFmt,\
                    pPortCfg->mStrideInByte[0],\
                    pPortCfg->mPureRaw);
        }

        if (!(sensorDev & UT_SNR_MASK)) {
            /* FakeSensor do not support HWSync */
            stN3D = MFALSE;
        }
        else if (sensorNum == 1) {
            stN3D = MFALSE;
        }

        QInitParam param(
        0, // 2: sensor uses pattern
        SEN_PIX_BITDEPTH,
        vSensorCfg[sidx],
        vPortInfo[sidx],
        MTRUE,
        stN3D); // N3D

        param.m_bOffBin = forceBinOff; // force bin off
        switch (dataPat) {
        case eDualPD:
            param.m_Func.Bits.DATA_PATTERN = EPipe_Dual_pix;
            break;
        case eQuadCode:
            param.m_Func.Bits.DATA_PATTERN = EPipe_QuadCode;
            break;
        case eNormal:
        default:
            param.m_Func.Bits.DATA_PATTERN = EPipe_Normal;
            break;
        }

        if(getNormalPipeModule()->query(0, ENPipeQueryCmd_D_Twin,
                            fmt, 0, qry)){
            if(qry.D_TWIN){
                param.m_DynamicTwin = MTRUE;
                MY_LOGD("enable Dynamic twin mode\n");
            }
            else{
                param.m_DynamicTwin = MFALSE;
                MY_LOGD("dynamic twin is not supported\n");
            }
        }
        else{
            MY_LOGE("unsupported cmd\n");
            return NULL;
        }

        switch((SWITCH_CAM_ENUM)_switchCam){
            case ONE_SENSOR:
            case SWITCH_AFTER_UNINIT:
                param.m_Func.Bits.SensorNum = E_1_SEN;
                break;
            case NONSTOP_DYNAMIC_TWIN:
                if(arg->TG_Num == _TG_1_){
                    param.m_Func.Bits.SensorNum = E_1_SEN;

                }
                if(arg->TG_Num == _TG_2_){
                    param.m_Func.Bits.SensorNum = E_2_SEN;
                }
                break;
            case TWO_SENSOR:
            case NONSTOP_DYNAMIC_TWIN_BY_SUSPEND:
                param.m_Func.Bits.SensorNum = E_2_SEN;
                break;
            default:
                MY_LOGE("not support case(%d)", _switchCam);
                return NULL;
                break;
        }

        param.m_DropCB = Drop_Test;
        MY_LOGD("m_bOffBin:%d, param.m_bOffBin:%d", param.m_bOffBin, forceBinOff);

        //fake clk is applied when using fake sensor
        if(!(sensorDev & UT_SNR_MASK)){
            FAKE_CLK fakeclk;
            FAKE_CLK::E_pixmode pix = FAKE_CLK::pix_1;
            switch(sensorPixMode){
                case 1:
                    pix = FAKE_CLK::pix_1;
                    break;
                case 2:
                    pix = FAKE_CLK::pix_2;
                    break;
                case 4:
                    pix = FAKE_CLK::pix_4;
                    break;
            }
            if(fakeclk.Init_Info(sensorFps,pUt[sidx]->mTgSize.w,pUt[sidx]->mTgSize.h,pix) == MFALSE){
                MY_LOGE("fake isp clk fail\n");
                return NULL;
            }
            if(fakeclk.Start_Cfg() == MFALSE){
                MY_LOGE("fake isp clk fail\n");
                return NULL;
            }
        }

        if(cmdsfiled & UNI_DYNAMIC_SWITCH) {
            param.m_UniLinkSel = E_UNI_LINK_ON;
        }

        if (!pUt[sidx]->mpNPipe->configPipe(param, pUt[sidx]->mBurstQNum)) {
            MY_LOGE("senidx:0x%x configPipe failed", sidx);
            return NULL;
        }


        if (bSetSensorTP) {
            MINT32 u32Enable = 1;

            MY_LOGD("Set sensor test pattern");
            pSensorHalObj[sidx]->sendCommand(
                    pHalSensorList->querySensorDevIdx(pUt[sidx]->mSensorIdx),
                    SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                    (MUINTPTR)&u32Enable,
                    0,
                    0);
        }

        #if (TEST_ZHDR_HW == 1)
        if (1)
        #else
        if (pUt[sidx]->mEnableSttPort)
        #endif
        {
            pUt[sidx]->tuningMgr = (TuningMgr*)TuningMgr::getInstance(pUt[sidx]->mSensorIdx);
            if (MFALSE == pUt[sidx]->tuningMgr->init(mThisName, pUt[sidx]->mBurstQNum)) {
                MY_LOGE("tuningMgr init error");
                return NULL;
            }
        }

        if (pUt[sidx]->mEnableSttPort) {
            MUINT32 sdma = 0;
            std::vector<statPortInfo> vp;
            QInitStatParam statParm(vp);

            pUt[sidx]->mpSttPipe = IStatisticPipe::createInstance(pUt[sidx]->mSensorIdx, mThisName);
            if (MFALSE == pUt[sidx]->mpSttPipe->init()) {
                MY_LOGE("statisticPipe init error");
                return NULL;
            }

            if (pUt[sidx]->mEnableSttPort & __AAO_ENABLE) {
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_AAO));
            }
            if (pUt[sidx]->mEnableSttPort & __AFO_ENABLE) {
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_AFO));
            }
            if (pUt[sidx]->mEnableSttPort & __FLKO_ENABLE) {
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_FLKO));
            }
            if (pUt[sidx]->mEnableSttPort & __PDO_ENABLE) {
                //use full TG size here is just easiest example,
                //theoratically, PDO size msut be smaller than TG size,
                //e.g.: PDO width = 1/8 TG_W,  PDO height = 1/4 TG_H
                if(pUt[sidx]->m_Dualpd.Bits.bDual)
                    statParm.mStatPortInfo.push_back(statPortInfo(PORT_PDO,pUt[sidx]->mTgSize.w,pUt[sidx]->mTgSize.h,EPipe_Dual_pix,pUt[sidx]->m_Dualpd.Bits.Density));
                else
                    statParm.mStatPortInfo.push_back(statPortInfo(PORT_PDO,pUt[sidx]->mTgSize.w,pUt[sidx]->mTgSize.h));
            }
            if (pUt[sidx]->mEnableSttPort & __PSO_ENABLE) {
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_PSO));
            }

            MY_LOGD("stt configPipe");
            pUt[sidx]->mpSttPipe->configPipe(statParm);
        }

        if (pUt[sidx]->mbCamsvEn) {
            MBOOL bRet = MFALSE;
            MUINT32 argu1 = 0;
            std::vector<statPortInfo> vp;
            QInitStatParam statParm(vp);

            int f;

            do {
                MY_LOGD("#############################################");
                MY_LOGD("# Please select 1 feature: PDAF(0)/mVHDR(1) #");
                MY_LOGD("#############################################");
                f = getchar();
                if (f == '1' || f =='0') {
                    f = f - '0';
                    break;
                }
            } while(1);

            pUt[sidx]->mpCamsvSttPipe = ICamsvStatisticPipe::createInstance(pUt[sidx]->mSensorIdx, mThisName , f); // alwasy use main
            if (!pUt[sidx]->mpCamsvSttPipe) {
                MY_LOGD("ICamsvStatisticPipe createInstance fail");
            }

            if (pUt[sidx]->mpCamsvSttPipe->init() == MFALSE) {
                MY_LOGD("ICamsvStatisticPipe init fail");
            }

            #if 0
            /* imx258mipiraw_Sensor.c, wxh=0x0_0 */
            argu1 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
            bRet = mpCamsvSttPipe->sendCommand(ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID, (MINTPTR)&argu1, NULL,NULL);
            #else
            /* imx258mipiraw_Sensor.c, wxh=0xa0_0x780 */
            argu1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            bRet = pUt[sidx]->mpCamsvSttPipe->sendCommand(ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID, (MINTPTR)&argu1, NULL,NULL);
            #endif

            pUt[sidx]->mpCamsvSttPipe->configPipe(statParm);
        }

        if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
            MY_LOGD("Reg SGG2 CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_SGG2_CBFP, (MINTPTR)&pUt[sidx]->mSgg2Notify, 0, 0);
            MY_LOGD("Reg EIS CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_EIS_CBFP, (MINTPTR)&pUt[sidx]->mEisNotify, 0, 0);
        }
        if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
            MY_LOGD("Reg LCS CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_LCS_CBFP, (MINTPTR)&pUt[sidx]->mLcsNotify, 0, 0);
        }

    }

    for (sidx = 0; sidx < sensorNum; sidx++) {
        pUt[sidx]->mpImemDrv = IMemDrv::createInstance();
        if (!pUt[sidx]->mpImemDrv) {
            MY_LOGE("IMem create fail");
            return NULL;
        }
        ret = pUt[sidx]->mpImemDrv->init();
        if (!ret) {
            MY_LOGE("IMem init fail");
            return NULL;
        }

        if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
            pUt[sidx]->mpImgBuffer[__RRZO].resize(pUt[sidx]->mPortBufDepth);
            pUt[sidx]->mImemBuf[__RRZO].resize(pUt[sidx]->mPortBufDepth);

            if(pUt[sidx]->m_ufeo_en == 1){
                pUt[sidx]->mpImgBuffer[__UFEO].resize(pUt[sidx]->mPortBufDepth);
                pUt[sidx]->mImemBuf[__UFEO].resize(pUt[sidx]->mPortBufDepth);
            }
        }
        if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
            pUt[sidx]->mpImgBuffer[__IMGO].resize(pUt[sidx]->mPortBufDepth);
            pUt[sidx]->mImemBuf[__IMGO].resize(pUt[sidx]->mPortBufDepth);
        }
        if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
            pUt[sidx]->mpImgBuffer[__EISO].resize(pUt[sidx]->mPortBufDepth);
            pUt[sidx]->mImemBuf[__EISO].resize(pUt[sidx]->mPortBufDepth);
        }
        if (pUt[sidx]->m_enablePort & __RSSO_ENABLE) {
            pUt[sidx]->mpImgBuffer[__RSSO].resize(pUt[sidx]->mPortBufDepth);
            pUt[sidx]->mImemBuf[__RSSO].resize(pUt[sidx]->mPortBufDepth);
        }
        if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
            pUt[sidx]->mpImgBuffer[__LCSO].resize(pUt[sidx]->mPortBufDepth);
            pUt[sidx]->mImemBuf[__LCSO].resize(pUt[sidx]->mPortBufDepth);
        }

        for (j = 0; j < pUt[sidx]->mPortBufDepth; j++) {
            IImageBuffer* pIBuf = NULL;
            IImageBuffer* pIBuf_UFEO = NULL;
            if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
                NormalPipe_QueryInfo    qry;
                MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                if(pUt[sidx]->m_ufeo_en == 1){
                     if (!getOutputFmt(PORT_UFEO, getPortBitDepth(PORT_UFEO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                        MY_LOGE("UFEO get pix fmt error");
                        return NULL;
                     }
                }
                else{
                    if (!getOutputFmt(PORT_RRZO, getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                        MY_LOGE("RRZO get pix fmt error\n");
                        return NULL;
                    }
                }


                getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                    fmt, pUt[sidx]->mTgSize.w, qry);

                imgiBuf.size = pUt[sidx]->mTgSize.h * qry.stride_byte;

                if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                    MY_LOGE("error: imem alloc fail");
                    return NULL;
                }
                pUt[sidx]->mImemBuf[__RRZO].at(j) = imgiBuf;
                MY_LOGD("rrzo imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);

                bufStridesInBytes[0] = qry.stride_byte;

                {
                    PortBufInfo_v1 portBufInfo(imgiBuf.memID, imgiBuf.virtAddr, 0, imgiBuf.bufSecu, imgiBuf.bufCohe);

                    IImageBufferAllocator::ImgParam imgParam(__RRZ_FMT(bitmap(getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort))), pUt[sidx]->mTgSize, bufStridesInBytes,
                                                        bufBoundaryInBytes, 1);
                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(mThisName, imgParam,portBufInfo, MTRUE);

                    pIBuf = pHeap->createImageBuffer();
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    pUt[sidx]->mpImgBuffer[__RRZO].at(j) = pIBuf;

                    MY_LOGD("rrzo img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                }
            }

            if(pUt[sidx]->m_ufeo_en == 1){
                if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
                            NormalPipe_QueryInfo    qry;
                            MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                            MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                            if (!getOutputFmt(PORT_UFEO, getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                                MY_LOGE("get pix fmt error\n");
                                return NULL;
                            }

                            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                            fmt, pUt[sidx]->mTgSize.w, qry);

                        imgiBuf.size = pUt[sidx]->mTgSize.h * qry.stride_byte;

                            if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                                MY_LOGE("error: imem alloc fail");
                                return NULL;
                            }

                            pUt[sidx]->mImemBuf[__UFEO].at(j) = imgiBuf;
                            MY_LOGD("ufeo imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);
                            bufStridesInBytes[0] = qry.stride_byte;
                            {
                            PortBufInfo_v1 portBufInfo(imgiBuf.memID, imgiBuf.virtAddr, 0, imgiBuf.bufSecu, imgiBuf.bufCohe);

                        IImageBufferAllocator::ImgParam imgParam(__RRZ_FMT(bitmap(getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort))), pUt[sidx]->mTgSize, bufStridesInBytes,
                                                                bufBoundaryInBytes, 1);
                            sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(mThisName, imgParam,portBufInfo, MTRUE);

                            pIBuf = pHeap->createImageBuffer();
                            pIBuf->incStrong(pIBuf);
                            pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                            pUt[sidx]->mpImgBuffer[__UFEO].at(j) = pIBuf;

                            MY_LOGD("ufeo img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                            }
                        }
            }
            if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
                NormalPipe_QueryInfo    qry;
                MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                if (!getOutputFmt(PORT_IMGO, getPortBitDepth(PORT_IMGO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                    MY_LOGE("get pix fmt error");
                    return NULL;
                }

                getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                    fmt, pUt[sidx]->mTgSize.w, qry);

                imgiBuf.size = pUt[sidx]->mTgSize.h * qry.stride_byte;

                if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                    MY_LOGE("error: imem alloc fail");
                    return NULL;
                }
                pUt[sidx]->mImemBuf[__IMGO].at(j) = imgiBuf;
                MY_LOGD("imgo imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);

                bufStridesInBytes[0] = qry.stride_byte;

                {
                    PortBufInfo_v1 portBufInfo(imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                    IImageBufferAllocator::ImgParam imgParam((bitmap(getPortBitDepth(PORT_IMGO, pUt[sidx]->m_enablePort))), pUt[sidx]->mTgSize, bufStridesInBytes,
                                                        bufBoundaryInBytes, 1);

                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( mThisName, imgParam,portBufInfo,MTRUE);

                    pIBuf = pHeap->createImageBuffer();
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    pUt[sidx]->mpImgBuffer[__IMGO].at(j) = pIBuf;

                    MY_LOGD("imgo img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                }
            }

            if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
                MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                imgiBuf.size = EISO_SIZE * 1; //TBD
                if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                    MY_LOGE("error: imem alloc fail");
                    return NULL;
                }

                pUt[sidx]->mImemBuf[__EISO].at(j) = imgiBuf;
                MY_LOGD("eiso imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);

                bufStridesInBytes[0] = EISO_SIZE;

                {
                    PortBufInfo_v1 portBufInfo(imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                    IImageBufferAllocator::ImgParam imgParam(eImgFmt_BAYER8, MSize(EISO_SIZE, 1), bufStridesInBytes,
                                                        bufBoundaryInBytes, 1);

                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(mThisName, imgParam, portBufInfo, MTRUE);

                    pIBuf = pHeap->createImageBuffer();
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    pUt[sidx]->mpImgBuffer[__EISO].at(j) = pIBuf;

                    MY_LOGD("eiso img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                }
            }
            if (pUt[sidx]->m_enablePort & __RSSO_ENABLE) {
                MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                imgiBuf.size = 288*511;
                if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                    MY_LOGE("error: imem alloc fail");
                    return NULL;
                }
                if (pUt[sidx]->mpImemDrv->mapPhyAddr(&imgiBuf) < 0) {
                    MY_LOGE("error: imem map fail");
                    return NULL;
                }

                pUt[sidx]->mImemBuf[__RSSO].at(j) = imgiBuf;
                MY_LOGD("rsso imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);

                bufStridesInBytes[0] = 288;

                {
                    PortBufInfo_v1 portBufInfo(imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                    IImageBufferAllocator::ImgParam imgParam(eImgFmt_BAYER8, MSize(288, 511), bufStridesInBytes,
                                                        bufBoundaryInBytes, 1);

                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(mThisName, imgParam, portBufInfo, MTRUE);

                    pIBuf = pHeap->createImageBuffer();
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    pUt[sidx]->mpImgBuffer[__RSSO].at(j) = pIBuf;

                    MY_LOGD("rsso img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                }
            }
            if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
                MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                imgiBuf.size = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize).w *
                                pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize).h; //TBD
                if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                    MY_LOGE("error: imem alloc fail");
                    return NULL;
                }

                pUt[sidx]->mImemBuf[__LCSO].at(j) = imgiBuf;
                MY_LOGD("lcso imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);

                bufStridesInBytes[0] = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize).w;

                {
                    PortBufInfo_v1 portBufInfo(imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                    IImageBufferAllocator::ImgParam imgParam(eImgFmt_BAYER8,
                            pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize), bufStridesInBytes,
                            bufBoundaryInBytes, 1);

                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(mThisName, imgParam, portBufInfo, MTRUE);

                    pIBuf = pHeap->createImageBuffer();
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    pUt[sidx]->mpImgBuffer[__LCSO].at(j) = pIBuf;

                    MY_LOGD("lcso img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                }
            }

        }
        if (pUt[sidx]->mEnableSttPort & __PDO_ENABLE) {
            //allocate bpci table
            //bpci
            pUt[sidx]->m_bpciBuf.bufCohe = MFALSE;
            pUt[sidx]->m_bpciBuf.size = sizeof(bpci_array_s5k2x8);
            pUt[sidx]->mpImemDrv->allocVirtBuf(&pUt[sidx]->m_bpciBuf);
            pUt[sidx]->mpImemDrv->mapPhyAddr(&pUt[sidx]->m_bpciBuf);

            memcpy((char*)pUt[sidx]->m_bpciBuf.virtAddr,(char*)&bpci_array_s5k2x8[0],sizeof(bpci_array_s5k2x8));
        }
    }


    MY_LOGD("enque...");

    for (sidx = 0; sidx < sensorNum; sidx++) {
        QBufInfo    qBuf;
        MUINT32     magicNum;
        MUINT32     *pMag = NULL;
        MUINT32 bin_w,bin_h;
        MSize       _size;

        pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_BIN_INFO, (MINTPTR)&bin_w, (MINTPTR)&bin_h, 0);
        _size.w = bin_w;
        _size.h = bin_h;
        //patch pipeline size after bin ctrl
        pUt[sidx]->m_crop.PatchPipeLineIn(_size);


        for (pUt[sidx]->mEnqCount = 0; pUt[sidx]->mEnqCount < pUt[sidx]->mBurstQNum*pUt[sidx]->mStaticEnqCnt;
                pUt[sidx]->mEnqCount += pUt[sidx]->mBurstQNum) {
            qBuf.mvOut.clear();
            //
            pMag = (MUINT32*)malloc(sizeof(MUINT32)*pUt[sidx]->mBurstQNum);
            for (j = pUt[sidx]->mEnqCount; j < (pUt[sidx]->mEnqCount + pUt[sidx]->mBurstQNum); j++) {
                magicNum = j;
                pMag[j-pUt[sidx]->mEnqCount] = j;

                if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
                    //note:crop size can't < outsize
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__RRZO].at(j);

                    MY_LOGD("RRZ max input size: %dx%d", bin_w, bin_h);


                    pUt[sidx]->m_crop.GetCropInfo(PORT_RRZO,(EImageFormat)pIBuf->getImgFormat(),_cropsize);
                    pUt[sidx]->m_crop.GetScaleInfo(PORT_RRZO,(EImageFormat)pIBuf->getImgFormat(),_destsize);

                    BufInfo _buf(PORT_RRZO, pIBuf, _destsize,_cropsize, magicNum, 0, 0xFFFF);
                    qBuf.mvOut.push_back(_buf);

                    if(pUt[sidx]->m_ufeo_en == 1){
                        IImageBuffer* pIBuf_UFEO = pUt[sidx]->mpImgBuffer[__UFEO].at(j);

                        BufInfo _buf_UFEO(PORT_UFEO, pIBuf_UFEO, _destsize,_cropsize, magicNum, 0, 0xFFFF);
                        qBuf.mvOut.push_back(_buf_UFEO);
                    }
                }
                if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__IMGO].at(j);
                    {
                        MSize imgo_size[2];
                        pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_IMGO_INFO,(MINTPTR)imgo_size,0,0);
                        if(imgo_size[0].w == 0)
                            pUt[sidx]->m_crop.GetCropInfo(PORT_IMGO,fmt,_cropsize,MTRUE);
                        else
                            pUt[sidx]->m_crop.GetCropInfo(PORT_IMGO,fmt,_cropsize,MFALSE);
                    }
                    pUt[sidx]->m_crop.GetScaleInfo(PORT_IMGO,(EImageFormat)pIBuf->getImgFormat(),_destsize);

                    BufInfo _buf(PORT_IMGO, pIBuf, _destsize,_cropsize, magicNum, 0, (pUt[sidx]->m_enablePort & __IMGO_PURE_RAW)?1:0, 0xFFFF);
                    qBuf.mvOut.push_back(_buf);
                }
                if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__EISO].at(j);

                    _destsize = MSize(EISO_SIZE, 1);

                    qBuf.mvOut.push_back(BufInfo(PORT_EISO, pIBuf, _destsize, MRect(MPoint(0,0), _destsize), magicNum, 0, 0xFFFF));
                }
                if (pUt[sidx]->m_enablePort & __RSSO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__RSSO].at(j);

                    _destsize = MSize(288, 511);

                    qBuf.mvOut.push_back(BufInfo(PORT_RSSO, pIBuf, _destsize, MRect(MPoint(0,0), _destsize), magicNum, 0, 0xFFFF));
                }
                if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__LCSO].at(j);

                    _destsize = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize);

                    qBuf.mvOut.push_back(BufInfo(PORT_LCSO, pIBuf, _destsize, MRect(MPoint(0,0), _destsize), magicNum, 0, 0xFFFF));
                }
            }
            //
            if (pUt[sidx]->mEnableSttPort) {
                pUt[sidx]->setTuning(pMag);
            }

            #if (TEST_ZHDR_HW == 1)
            pUt[sidx]->setZHdrTuning(pMag);
            #endif

            free(pMag);

            if (MFALSE == pUt[sidx]->mpNPipe->enque(qBuf)) {
                MY_LOGE("Enque fail...");
                return NULL;
            }
        }
    }

    MY_LOGD("ISP START");

    for (sidx = 0; sidx < sensorNum; sidx++) {

        if (pUt[sidx]->mbCamsvEn) {
            pUt[sidx]->mpCamsvSttPipe->start();
        }

        if (pUt[sidx]->mEnableSttPort) {

            if (MFALSE == pUt[sidx]->mpSttPipe->start()) {
                MY_LOGE("Start STT fail...");
                return NULL;
            }
        }

        if (MFALSE == pUt[sidx]->mpNPipe->start()) {
            MY_LOGE("Start fail...");
            return NULL;
        }
    }

    /*
     * note: need to start isp fist(can't use deque first,than use another to start isp, or deadlock will hanppen)
     */

    MY_LOGD("start deque/denque thread");

    for (sidx = 0; sidx < sensorNum; sidx++) {
        pUt[sidx]->startThread();
    }

    if(m_bN3DTest == true){
        pUt[0]->startHwsyncThread();
    }

    while (1) {
        MBOOL running = MFALSE;

        for (sidx = 0; sidx < sensorNum; sidx++) {
            if (!pUt[sidx]->m_bStop) {
                running = MTRUE;
            }
        }

        if (MFALSE == running) {
            MY_LOGD("all stopped");
            break;
        }
        else {
            MUINT32 wait_us = 0;

            if (sensorFps) {
                wait_us = (1000000/2)/sensorFps;
            }
            if (0 == wait_us) {
                wait_us = 500000;
            }

            for (sidx = 0; sidx < sensorNum; sidx++) {
                if(temp_loopCnt != pUt[sidx]->mLoopCount)
                    MY_LOGD("[%d] wait %d us: %d/%d", pUt[sidx]->mSensorIdx, wait_us, pUt[sidx]->mLoopCount, pUt[sidx]->mFrameNum);

                temp_loopCnt = pUt[sidx]->mLoopCount;
            }
            usleep(wait_us);
        }
    }

FAIL_EXIT:
    for (sidx = 0; sidx < sensorNum; sidx++) {
        pUt[sidx]->stopThread();

        if(m_bN3DTest == true){
            if(sidx == 0){
                pUt[0]->stopHwsyncThread();
            }
        }

        MY_LOGD("STT STOP");
        if (pUt[sidx]->mEnableSttPort) {
            pUt[sidx]->mpSttPipe->stop();

        }
        if (pUt[sidx]->mbCamsvEn) {
            pUt[sidx]->mpCamsvSttPipe->stop();
        }

        if (stopFlow == 2) {
            MY_LOGD("NPIPE ABORT");
            pUt[sidx]->mpNPipe->abort();
        }

        if (pUt[sidx]->tuningMgr) {
            pUt[sidx]->tuningMgr->uninit(mThisName);
        }

        switch (stopFlow) {
        case 1://non-blocking stop
            MY_LOGD("NPIPE NONBLOCKING STOP");

            pUt[sidx]->mpNPipe->stop(MTRUE);

            if (pUt[sidx]->mSuspendFrm) {
                MY_LOGD("Flush wait sof irq");
                pUt[sidx]->mpNPipe->signal(EPipeSignal_SOF, pUt[sidx]->mSofUserKey);
            }
            break;
        case 0:
            MY_LOGD("NPIPE BLOCKING STOP");

            pUt[sidx]->mpNPipe->stop(MFALSE);
            break;
        case 2:
        default:
            break;
        }

        if(pUt[sidx]->m_bEfuse == MTRUE) {
            MY_LOGD("BOUNDING Result: %s %s",\
                efTable[pUt[sidx]->mEfuseIdx].resultStr, \
                (pUt[sidx]->mFailCount > 0 ? "FAIL":"PASS"));
        }

        if (pUt[sidx]->mEnableSttPort) {
            pUt[sidx]->mpSttPipe->uninit();
            pUt[sidx]->mpSttPipe->destroyInstance(mThisName);
        }
        if (pUt[sidx]->mbCamsvEn) {
            if (pUt[sidx]->mCamsvBufVa && pUt[sidx]->mCamsvBufSize) {
                #define DUMP_PATH       "/data/cameradump_camio/"

                /* dump before buffer free in destroyInstance */
                MY_LOGD("Dump camsv buf va:%" PRIXPTR " size:%d", pUt[sidx]->mCamsvBufVa, pUt[sidx]->mCamsvBufSize);

                if (!makePath(DUMP_PATH, 0660)) {
                    MY_LOGE("error: makePath fail");
                    return NULL;
                }
                else {
                    sprintf(filename, DUMP_PATH "%d_camsv_%d.raw", sidx, j);
                    saveBuf(filename, (MUINT8*)pUt[sidx]->mCamsvBufVa, pUt[sidx]->mCamsvBufSize);
                }
            }

            pUt[sidx]->mpCamsvSttPipe->uninit();
            pUt[sidx]->mpCamsvSttPipe->destroyInstance(mThisName);
        }

        pUt[sidx]->mpNPipe->uninit();

        if ((SWITCH_CAM_ENUM)_switchCam == SWITCH_AFTER_UNINIT) {

            pthread_create(&_switchThd, NULL, open_another_cam, pUt[sidx]);

            MY_LOGD("Wait another cam create...");
            ::sem_wait(&pUt[sidx]->m_semSwitchThd);
            MY_LOGD("Wait another cam create done");
        }

        pUt[sidx]->mpNPipe->destroyInstance(mThisName);

        //power off sensor
        pSensorHalObj[sidx]->powerOff(mThisName,1, &pUt[sidx]->mSensorIdx);
    }

    for (sidx = 0; sidx < sensorNum; sidx++) { //Save File
        for (j = 0; j < 1/*mPortBufDepth*/; j++) {
            for (dma = 0; dma < __MAXDMAO; dma++) {
                if (pUt[sidx]->m_enablePort & (1 << dma)) {
                    onDumpBuffer("camio", sidx, dma, (MUINTPTR)pUt[sidx]->mpImgBuffer[dma].at(j), j);
#if 1
                    if((pUt[sidx]->m_ufeo_en == 1) & (dma == __RRZO)){
                        onDumpBuffer("camio", sidx, __UFEO, (MUINTPTR)pUt[sidx]->mpImgBuffer[__UFEO].at(j), j);
                    }
#endif
                }
            }
        }
        for (j = 0; j < 1; j++) {
            for (dma = 0; dma < __MAXDMAO_STT; dma++) {
                if (pUt[sidx]->mEnableSttPort & (1 << dma)) {
                    sprintf(filename, "/data/cameradump_camio/%d_stt_%d_%d.raw", sidx, dma, j);
                    saveBuf(filename, (MUINT8*)pUt[sidx]->mpSttBuf[dma], pUt[sidx]->mpSttBufSize[dma]);
                }
            }
        }
    }

    MY_LOGD("Clean up");
    for (sidx = 0; sidx < sensorNum; sidx++) {
        vSensorCfg[sidx].clear();
        lHwPortCfg[sidx].clear();
        for (dma = 0; dma < __MAXDMAO; dma++) {
            if (pUt[sidx]->m_enablePort & (1 << dma)) {
                for (j = 0; j < pUt[sidx]->mImemBuf[dma].size(); j++) {
                    MY_LOGD("Freeing VA: %p", (void*)pUt[sidx]->mImemBuf[dma].at(j).virtAddr);
                    pUt[sidx]->mpImemDrv->freeVirtBuf(&pUt[sidx]->mImemBuf[dma].at(j));
                }
            }
        }
        #if 0
        for (dma = 0; dma < __MAXDMAO_STT; dma++) {
            if (pUt[sidx]->mpSttBuf[dma]) {
                delete (unsigned char*)pUt[sidx]->mpSttBuf[dma];
            }

        }
        #endif

        if(pUt[sidx]->mEnableSttPort & __PDO_ENABLE)
            pUt[sidx]->mpImemDrv->freeVirtBuf(&pUt[sidx]->m_bpciBuf);

        pUt[sidx]->mpImemDrv->uninit();
        pUt[sidx]->mpImemDrv->destroyInstance();

        MY_LOGD("destroy...");
        pUt[sidx]->destroy();

        if ((SWITCH_CAM_ENUM)_switchCam == SWITCH_AFTER_UNINIT ) {
            pthread_join(_switchThd, NULL);
        }
    }

    if(_efuse && _efuse_idx > 0) {
        _efuse_idx --;
        goto SEARCH_SENSOR;
    }

    MY_LOGD("EOT");
    return NULL;
}

void dupArgv(char ***src, char ***dst, int argcnt)
{
    int i;

    if ((src == NULL) || (dst == NULL) || (argcnt == 0)) {
        MY_LOGE("Cannot be null/zero %p, %p, %d", src, dst, argcnt);
        return;
    }

    *dst = (char **)malloc(sizeof(char*) * argcnt);

    for(i = 0; i < argcnt; i++) {
        int nstr = strlen((*src)[i]);

        (*dst)[i] = (char *)malloc(sizeof(char) * (nstr+1));

        strcpy((*dst)[i], (*src)[i]);
    }
}

void freeDupArgv(int the_argc, char **the_argv)
{
    int i;

    if ((the_argv == NULL) || (the_argc == 0)) {
        MY_LOGE("Cannot be null/zero %p, %d", the_argv, the_argc);
        return;
    }

    for (i = 0; i < the_argc; i++) {
        if (the_argv[i]) {
            free(the_argv[i]);
        }
    }
    free(the_argv);
}

int test_camio_mgr(int argc, char** argv)
{
    MUINT32             sensorDev, DevCmd, sensorNum, FrameNum, _switchCam, EnqCount, i, ran = 0;
    MUINT32             sensorArray[_MAXTG_] =  {0, 0};
    pthread_t           CAM_A = (pthread_t)NULL, CAM_B = (pthread_t)NULL;
    _arg                ts_arg[_MAXTG_] = {{0,NULL}, {0,NULL}};
    MBOOL               avalible = MTRUE,ret = MTRUE, ReleaseDone = MFALSE;
    char*               s = (char*)malloc(sizeof(char)*64);
    NPipeUT*            pUt[_MAXTG_] = {NULL, NULL};
    MUINT32             PathCFG = 0;

    sensorDev = atoi(argv[CMD_IDX_SNRDEVID]);
    FrameNum = atoi(argv[CMD_IDX_FRAMENUM]);
    _switchCam = atoi(argv[CMD_IDX_SWITCHCAM]);

    memset((void*)ts_arg, 0, sizeof(ts_arg));

    sensorNum = 0;
    if (0 == sensorDev) {
        sensorDev = UT_MAIN_FKSNR;
    }
    if (sensorDev & (UT_MAIN_SNR | UT_MAIN_FKSNR)) {
        if(sensorDev & UT_MAIN_SNR){
            sensorArray[sensorNum] = UT_MAIN_SNR;
        }
        if(sensorDev & UT_MAIN_FKSNR){
            sensorArray[sensorNum] = UT_MAIN_FKSNR;
        }
        sensorNum++;
    }
    if (sensorDev & (UT_SUB_SNR | UT_SUB_FKSNR)) {
        if(sensorDev & UT_SUB_SNR){
            sensorArray[sensorNum] = UT_SUB_SNR;
        }
        if(sensorDev & UT_SUB_FKSNR){
            sensorArray[sensorNum] = UT_SUB_FKSNR;
        }
        sensorNum++;
    }

    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_MAIN2_SNR | UT_MAIN2_FKSNR)) {
            if(sensorDev & UT_MAIN2_SNR){
                sensorArray[sensorNum] = UT_MAIN2_SNR;
            }
            if(sensorDev & UT_MAIN2_FKSNR){
                sensorArray[sensorNum] = UT_MAIN2_FKSNR;
            }
            sensorNum++;
        }
    }

    if(sensorNum > 2){
        MY_LOGE("sensor number(%d) is not supported ",sensorNum);
        free(s);
        return 0;
    }

    switch(_switchCam){
        case ONE_SENSOR:
        case SWITCH_AFTER_UNINIT: {
            if (sensorNum != 1) {
                MY_LOGD("switch cam need config only one cam a time");
                break;
            }
            if (sensorArray[_TG_1_]){
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(argv[i]));
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_]._argv = argv;
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }
            MUINT32 cnt = 0;
            while(cnt++ < 100){
                if(ts_arg[_TG_1_].Ut_obj)
                    break;
                usleep(3000);
            }

            //suspend and resume flow
            if(ts_arg[_TG_1_].Ut_obj->mSuspendTest){
                if(ts_arg[_TG_1_].Ut_obj->mSuspendFlow == NONSTOP_SUSPEND){
                    MY_LOGE("Non-stop Suspend now olny for dynamic twin test");
                    break;
                }
                else{
                    while(1){
                        if(ts_arg[_TG_1_].Ut_obj->mSuspendFrm && (ts_arg[_TG_1_].Ut_obj->mLoopCount == ts_arg[_TG_1_].Ut_obj->mSuspendFrm)){
                            MY_LOGD("#######suspend start#######");
                            ts_arg[_TG_1_].Ut_obj->mSuspendStart = MTRUE;
                            break;
                        }
                        usleep(3000);
                    }
                }
                while(1){
                    if(!ts_arg[_TG_1_].Ut_obj->mSuspendStart)
                        break;
                    usleep(3000);
                }

                MY_LOGD("#######Wait %d us#######", ran);
                ran = ((rand() % 1)*1000000) +((rand() % 9)*100000);
                usleep(ran);

                if (ts_arg[_TG_1_].Ut_obj->mSuspendFlow != SUSPEND_STOP) {
                    MY_LOGD("#######resume start#######");
                    ts_arg[_TG_1_].Ut_obj->mResumeStart = MTRUE;

                    while(1){
                        if(!ts_arg[_TG_1_].Ut_obj->mResumeStart)
                            break;
                        usleep(3000);
                    }
                }

                MY_LOGD("#######Suspend/Resume flow is completed#######");
            }
            //
            pthread_join(CAM_A, NULL);
            }
            break;
        case TWO_SENSOR: {
            char **tg1_argv = NULL, **tg2_argv = NULL;

            if (sensorNum != 2) {
                MY_LOGD("switch cam need config two cam a time");
                break;
            }
            if (sensorArray[_TG_1_]){
                MY_LOGD("########CAM A###########");
                dupArgv(&argv, &tg1_argv, argc);
                if (tg1_argv == NULL) {
                    LOG_ERR("Error: dup argv fail\n");
                    break;
                }
                sprintf(s,"%d",sensorArray[_TG_1_]);
                if(argc > CMD_IDX_SNRDEVID)
                    tg1_argv[CMD_IDX_SNRDEVID] = (char*)s;
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg1_argv[i]));
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_]._argv = tg1_argv;
                ts_arg[_TG_1_].TG_Num = _TG_1_;
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }
            if (sensorArray[_TG_2_]){
                MY_LOGD("#######CAM B###########");
                dupArgv(&argv, &tg2_argv, argc);
                if (tg2_argv == NULL) {
                    LOG_ERR("Error: dup argv fail\n");
                    break;
                }
                sprintf(s,"%d",sensorArray[_TG_2_]);
                tg2_argv[CMD_IDX_SNRDEVID] = (char*)s;
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg2_argv[i]));
                ts_arg[_TG_2_]._argc = argc;
                ts_arg[_TG_2_]._argv = tg2_argv;
                ts_arg[_TG_2_].TG_Num = _TG_2_;
                pthread_create(&CAM_B, NULL, test_camio_thread, &ts_arg[_TG_2_]);
            }
            MUINT32 cnt = 0;
            while(cnt++ < 100){
                if(ts_arg[_TG_2_].Ut_obj)
                    break;
                usleep(3000);
            }
            //suspend and resume flow
            if(ts_arg[_TG_2_].Ut_obj->mSuspendTest){
                if(ts_arg[_TG_2_].Ut_obj->mSuspendFlow == NONSTOP_SUSPEND){
                    MY_LOGE("Non-stop Suspend now olny for dynamic twin test");
                    break;
                }
                else{
                    while(1){
                        if(ts_arg[_TG_2_].Ut_obj->mSuspendFrm && (ts_arg[_TG_2_].Ut_obj->mLoopCount == ts_arg[_TG_2_].Ut_obj->mSuspendFrm)){
                            MY_LOGD("#######suspend start#######");
                            ts_arg[_TG_2_].Ut_obj->mSuspendStart = MTRUE;
                            break;
                        }
                        usleep(3000);
                    }
                }

                while(1){
                    if(!ts_arg[_TG_2_].Ut_obj->mSuspendStart)
                        break;
                    usleep(3000);
                }
                ran = ((rand() % 2)*1000000) +((rand() % 9)*100000);
                usleep(ran);
                MY_LOGD("Wait %d us", ran);
                MY_LOGD("Resume start");
                ts_arg[_TG_2_].Ut_obj->mResumeStart = MTRUE;

                while(1){
                    if(!ts_arg[_TG_2_].Ut_obj->mResumeStart)
                        break;
                    usleep(3000);
                }
                MY_LOGD("Suspend/Resume flow is completed");
            }
            pthread_join(CAM_A, NULL);
            pthread_join(CAM_B, NULL);
            if (tg1_argv) {
                freeDupArgv(argc, tg1_argv);
            }
            if (tg2_argv) {
                freeDupArgv(argc, tg2_argv);
            }
            }
            break;
        case NONSTOP_DYNAMIC_TWIN:
            if (sensorNum != 2) {
                MY_LOGD("switch cam need config two cam a time");
                break;
            }
            if (sensorArray[_TG_1_]){
                MY_LOGD("########CAM A###########");
                sprintf(s,"%d",sensorArray[_TG_1_]);
                if(argc > CMD_IDX_SNRDEVID)
                    argv[CMD_IDX_SNRDEVID] = (char*)s;
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(argv[i]));
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_]._argv = argv;
                ts_arg[_TG_1_].TG_Num = _TG_1_;
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }
            while(1){
                sleep(1);

                MY_LOGD("########Switch to Two###########");

                if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
                    MY_LOGE("CamPathCFG fail");
                    break;
                }
                if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
                    MY_LOGE("CamPathCFG fail");
                    break;
                }

                if (sensorArray[_TG_2_]){
                    MY_LOGD("#######CAM B###########");
                    sprintf(s,"%d",sensorArray[_TG_2_]);
                    argv[CMD_IDX_SNRDEVID] = (char*)s;
                    for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                        MY_LOGD("cmd_%d",atoi(argv[i]));
                    ts_arg[_TG_2_]._argc = argc;
                    ts_arg[_TG_2_]._argv = argv;
                    ts_arg[_TG_2_].TG_Num = _TG_2_;
                    pthread_create(&CAM_B, NULL, test_camio_thread, &ts_arg[_TG_2_]);
                }
                while(1){
                    usleep(20000);
                    if(ts_arg[_TG_2_].Ut_obj){
                        if (ts_arg[_TG_2_].Ut_obj->mEnqCount > 3)
                        break;
                    }
                }

                ts_arg[_TG_2_].Ut_obj->m_bStop = MTRUE;
                pthread_join(CAM_B, NULL);
                ts_arg[_TG_2_].Ut_obj = NULL;

                MY_LOGD("########Switch to One###########");

                if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
                    MY_LOGE("CamPathCFG fail");
                    break;
                }
                if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
                    MY_LOGE("CamPathCFG fail");
                    break;
                }

            }

            break;
        case NONSTOP_DYNAMIC_TWIN_BY_SUSPEND: {
            char **tg1_argv = NULL, **tg2_argv = NULL;

            if (sensorNum != 2) {
                MY_LOGD("switch cam need config two cam a time");
                break;
            }
            if (sensorArray[_TG_1_]){
                MY_LOGD("########CAM A###########");
                dupArgv(&argv, &tg1_argv, argc);
                if (tg1_argv == NULL) {
                    LOG_ERR("Error: dup argv fail\n");
                    break;
                }
                sprintf(tg1_argv[CMD_IDX_SNRDEVID],"%d",sensorArray[_TG_1_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg1_argv[i]));
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_]._argv = tg1_argv;
                ts_arg[_TG_1_].TG_Num = _TG_1_;
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }
                if (sensorArray[_TG_2_]){
                    MY_LOGD("#######CAM B###########");
                    dupArgv(&argv, &tg2_argv, argc);
                    if (tg2_argv == NULL) {
                        LOG_ERR("Error: dup argv fail\n");
                        break;
                    }
                    sprintf(tg2_argv[CMD_IDX_SNRDEVID],"%d",sensorArray[_TG_2_]);
                    for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                        MY_LOGD("cmd_%d",atoi(tg2_argv[i]));
                    ts_arg[_TG_2_]._argc = argc;
                    ts_arg[_TG_2_]._argv = tg2_argv;
                    ts_arg[_TG_2_].TG_Num = _TG_2_;
                    pthread_create(&CAM_B, NULL, test_camio_thread, &ts_arg[_TG_2_]);
                }
                MUINT32 cnt = 0;
                while(cnt++ < 100){
                    if(ts_arg[_TG_2_].Ut_obj)
                        break;
                    usleep(3000);
                }

                while(ts_arg[_TG_2_].Ut_obj->mSuspendTest){
                    if(ts_arg[_TG_2_].Ut_obj->mSuspendFlow != NONSTOP_SUSPEND){
                        MY_LOGE("dynamic twin suspend test need use non-stop suspend");
                        break;
                    }
                    while(1){
                        if(!ts_arg[_TG_2_].Ut_obj->mResumeStart)
                            break;
                        usleep(3000);
                    }

                    /*Give it a random time to run 1 sensor(twin)*/
                    ran = ((rand() % 1)*1000000) +((rand() % 9)*100000);
                    usleep(ran);

                    /*Suspend start*/
                    ts_arg[_TG_2_].Ut_obj->mSuspendStart = MTRUE;

                    while(1){
                        if(!ts_arg[_TG_2_].Ut_obj->mSuspendStart)
                            break;
                        usleep(3000);
                    }

                    MY_LOGD("########[wait %d]Switch to One###########", ran);
                    if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
                        MY_LOGE("CamPathCFG fail");
                        break;
                    }
                    if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
                        MY_LOGE("CamPathCFG fail");
                        break;
                    }

                    /*Give it a random time to run 1 sensor(twin)*/
                    ran = ((rand() % 1)*1000000) +((rand() % 9)*100000);
                    usleep(ran);

                    MY_LOGD("########[wait %d]Switch to Two###########", ran);
                    if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
                        MY_LOGE("CamPathCFG fail");
                        break;
                    }
                    if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
                        MY_LOGE("CamPathCFG fail");
                        break;
                    }

                    /*Resume start*/
                    ts_arg[_TG_2_].Ut_obj->mResumeStart = MTRUE;
                }

                if (tg1_argv) {
                    freeDupArgv(argc, tg1_argv);
                }
                if (tg2_argv) {
                    freeDupArgv(argc, tg2_argv);
                }
            break;
            }
        default:
            MY_LOGE("sensor number(%d) is not supported ",sensorNum);
            break;
    }

    free(s);

    return 0;
}

