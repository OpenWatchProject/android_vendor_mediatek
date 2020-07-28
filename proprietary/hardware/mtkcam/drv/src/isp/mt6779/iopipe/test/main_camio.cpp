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
#include <cutils/properties.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/IDummyImageBufferHeap.h>

#include <mtkcam/def/PriorityDefs.h>

using namespace NSCam;
using namespace NSCam::Utils;
using namespace android;
using namespace std;


#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/ICamsvStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/Cam_QueryDef.h>


#include <ispio_pipe_ports.h>

/* For statistic dmao: eiso, lcso */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <imageio/Cam_Notify_datatype.h>
#include <tuning_mgr.h>
#include <mtkcam/drv/IHwSyncDrv.h>
#include "pic/bpci_tbl_s5k2x8.h"    //for PDO test
#include "cam_crop.h"              //for crop test

#include "npipeut.h"

//secure cam
#include <cam/sec_mgr.h>

using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

#define ePlane_max NSImageio::NSIspio::ePlane_max
#define ePlane_1st NSImageio::NSIspio::ePlane_1st
#define ePlane_2nd NSImageio::NSIspio::ePlane_2nd
#define ePlane_3rd NSImageio::NSIspio::ePlane_3rd

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
    MUINT32         mStrideInByte[ePlane_max];
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
        MY_LOGD("Dump x%x: %dx%d, stride %d to %s", (MUINT32)buffer->getBufPA(ePlane_1st), (MUINT32)buffer->getImgSize().w,\
                (MUINT32)buffer->getImgSize().h, (MUINT32)buffer->getBufStridesInBytes(ePlane_1st), filename);\
    } while(0)
    char dumppath[256];

    sprintf(dumppath, "%s%s/", DUMP_PREFIX, usr);

    if (!makePath(dumppath, 0660)) {
        MY_LOGD("makePath [%s] fail",dumppath);
        return;
    }
    switch(data){
    case __YUVO:
    case __CRZO:
    case __CRZBO:
    case __CRZBO_R2:
        DUMP_IImageBuffer(data, buf, "yuv");
        break;
    default:
        DUMP_IImageBuffer(data, buf, "raw");
        break;
    }
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

extern int test_camio(int argc, char** argv);
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
    test_camio(_argc, _argv);

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
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    (PortEnableMap);

    if (_port.index == PORT_IMGO.index ||
        _port.index == PORT_RRZO.index ||
        _port.index == PORT_CAMSV_IMGO.index) {
        property_get("vendor.camiotest.bitdepth", value, "0");
        if (atoi(value) == 8 || atoi(value) == 10 ||
            atoi(value) == 12 || atoi(value) == 14) {
            bitDepth = atoi(value);
        }
        else {
            bitDepth = 10;
        }
    }
    else if (_port.index == PORT_RAWI.index){
        bitDepth = 8; //8bit is more eaiser to gen image!!
    }
    else {
        MY_LOGE("Unsupported port depth query");
    }
    MY_LOGD("bitDepth(%d)\n", bitDepth); // Cathy debug
    return bitDepth;
}

static EImageFormat getExtentFmt(PortID _port,MUINT32 &plane)
{
    #define case_Format( order_bit, mappedfmt, pFmt,p_num) \
        case order_bit:                          \
            (*(pFmt)) = mappedfmt;               \
            plane = p_num;                       \
        break;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MBOOL bmatch = MFALSE;
    EImageFormat fmt = NSCam::eImgFmt_IMPLEMENTATION_DEFINED;
    sCAM_QUERY_QUERY_FMT q_fmt;
    q_fmt.QueryInput.portId = _port.index;
    q_fmt.QueryOutput.clear();
    plane = 0;

    property_get("vendor.camiotest.extFmt", value, "0");

    if(_port.index == PORT_YUVO.index){
        getNormalPipeModule()->query(ENPipeQueryCmd_QUERY_FMT,(MUINTPTR)&q_fmt);
        switch(atoi(value)){
            case_Format( 1, NSCam::eImgFmt_YUY2 , &fmt,1);
            case_Format( 2, NSCam::eImgFmt_YVYU , &fmt,1);
            case_Format( 3, NSCam::eImgFmt_UYVY , &fmt,1);
            case_Format( 4, NSCam::eImgFmt_VYUY , &fmt,1);
            case_Format( 5, NSCam::eImgFmt_NV16 , &fmt,2);
            case_Format( 6, NSCam::eImgFmt_NV61 , &fmt,2);
            case_Format( 7, NSCam::eImgFmt_NV21 , &fmt,2);
            case_Format( 8, NSCam::eImgFmt_NV12 , &fmt,2);
            case_Format( 9, NSCam::eImgFmt_I422 , &fmt,3);
            case_Format(10, NSCam::eImgFmt_I420 , &fmt,3);
            case_Format(11, NSCam::eImgFmt_MTK_YUYV_Y210 , &fmt,1);
            case_Format(12, NSCam::eImgFmt_MTK_YVYU_Y210 , &fmt,1);
            case_Format(13, NSCam::eImgFmt_MTK_UYVY_Y210 , &fmt,1);
            case_Format(14, NSCam::eImgFmt_MTK_VYUY_Y210 , &fmt,1);
            case_Format(15, NSCam::eImgFmt_MTK_YUV_P210 , &fmt,2);
            case_Format(16, NSCam::eImgFmt_MTK_YVU_P210 , &fmt,2);
            case_Format(17, NSCam::eImgFmt_MTK_YUV_P010 , &fmt,2);
            case_Format(18, NSCam::eImgFmt_MTK_YVU_P010 , &fmt,2);
            case_Format(19, NSCam::eImgFmt_MTK_YUV_P210_3PLANE , &fmt,3);
            case_Format(20, NSCam::eImgFmt_MTK_YUV_P010_3PLANE , &fmt,3);
            case_Format(21, NSCam::eImgFmt_YUYV_Y210 , &fmt,1);
            case_Format(22, NSCam::eImgFmt_YVYU_Y210 , &fmt,1);
            case_Format(23, NSCam::eImgFmt_UYVY_Y210 , &fmt,1);
            case_Format(24, NSCam::eImgFmt_VYUY_Y210 , &fmt,1);
            case_Format(25, NSCam::eImgFmt_YUV_P210 , &fmt,2);
            case_Format(26, NSCam::eImgFmt_YVU_P210 , &fmt,2);
            case_Format(27, NSCam::eImgFmt_YUV_P010 , &fmt,2);
            case_Format(28, NSCam::eImgFmt_YVU_P010 , &fmt,2);
            case_Format(29, NSCam::eImgFmt_YUV_P210_3PLANE , &fmt,3);
            case_Format(30, NSCam::eImgFmt_YUV_P010_3PLANE , &fmt,3);
            default:
                fmt = NSCam::eImgFmt_YUY2;
                plane = 1;
                break;
        }
    }
    else if(_port.index == PORT_CRZO_R1.index){
        getNormalPipeModule()->query(ENPipeQueryCmd_QUERY_FMT,(MUINTPTR)&q_fmt);
        switch(atoi(value)){
            case_Format( 1, NSCam::eImgFmt_YUY2 , &fmt,1);
            case_Format( 2, NSCam::eImgFmt_YVYU , &fmt,1);
            case_Format( 3, NSCam::eImgFmt_UYVY , &fmt,1);
            case_Format( 4, NSCam::eImgFmt_VYUY , &fmt,1);
            case_Format( 5, NSCam::eImgFmt_NV16 , &fmt,2);
            case_Format( 6, NSCam::eImgFmt_NV61 , &fmt,2);
            case_Format( 7, NSCam::eImgFmt_NV21 , &fmt,2);
            case_Format( 8, NSCam::eImgFmt_NV12 , &fmt,2);
            default:
                fmt = NSCam::eImgFmt_YUY2;
                plane = 1;
                break;
        }
    }
    else if(_port.index == PORT_CRZO_R2.index){
        getNormalPipeModule()->query(ENPipeQueryCmd_QUERY_FMT,(MUINTPTR)&q_fmt);
        switch(atoi(value)){
            case_Format( 1, NSCam::eImgFmt_YUY2 , &fmt,1);
            case_Format( 2, NSCam::eImgFmt_YVYU , &fmt,1);
            case_Format( 3, NSCam::eImgFmt_UYVY , &fmt,1);
            case_Format( 4, NSCam::eImgFmt_VYUY , &fmt,1);
            case_Format( 5, NSCam::eImgFmt_NV16 , &fmt,2);
            case_Format( 6, NSCam::eImgFmt_NV61 , &fmt,2);
            default:
                fmt = NSCam::eImgFmt_YUY2;
                plane = 1;
                break;
        }
    }
    else{
        MY_LOGE("unsupported port:%d. fmt error, crash!\n",_port.index);
    }

    for(MUINT i=0;i<q_fmt.QueryOutput.size();i++){
        if(q_fmt.QueryOutput.at(i) == fmt){
            bmatch = MTRUE;
            break;
        }
    }

    if(bmatch == MFALSE){
        MY_LOGE("current fmt is not supported by hw(0x%x),crash!\n",fmt);
    }

    return fmt;
}


MBOOL getOutputFmt(PortID port, MUINT32 bitDepth, SensorStaticInfo& mSensorInfo, EImageFormat* pFmt, MBOOL bUFO)
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
        if ((port.index == PORT_IMGO.index || port.index == PORT_CAMSV_IMGO.index) && bUFO == MFALSE) {
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
        else if (port.index == PORT_RRZO.index && bUFO == MFALSE) { //rrzo
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
        else if ((port.index == PORT_IMGO.index || port.index == PORT_CAMSV_IMGO.index) && bUFO == MTRUE) { //ufeo
            switch( bitDepth) {
            case_Format(  8, eImgFmt_UFO_BAYER8, pFmt);
            case_Format( 10, eImgFmt_UFO_BAYER10, pFmt);
            case_Format( 12, eImgFmt_UFO_BAYER12, pFmt);
            case_Format( 14, eImgFmt_UFO_BAYER14, pFmt);
            default:
                MY_LOGE("UFEO bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if (port.index == PORT_RRZO.index && bUFO == MTRUE) { //ufgo
            switch( bitDepth) {
            case_Format(  8, eImgFmt_UFO_FG_BAYER8, pFmt);
            case_Format( 10, eImgFmt_UFO_FG_BAYER10, pFmt);
            case_Format( 12, eImgFmt_UFO_FG_BAYER12, pFmt);
            case_Format( 14, eImgFmt_UFO_FG_BAYER14, pFmt);
            default:
                MY_LOGE("UFGO bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if (port.index == PORT_RAWI.index && bUFO == MFALSE) {
            switch (bitDepth) {
            case_Format(  8, eImgFmt_BAYER8 , pFmt);
            case_Format( 10, eImgFmt_BAYER10, pFmt);
            case_Format( 12, eImgFmt_BAYER12, pFmt);
            default:
                MY_LOGE("RAWI bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if (port.index == PORT_RAWI.index && bUFO == MTRUE) { //ufdi
            switch( bitDepth) {
            case_Format(  8, eImgFmt_UFO_BAYER8, pFmt);
            case_Format( 10, eImgFmt_UFO_BAYER10, pFmt);
            case_Format( 12, eImgFmt_UFO_BAYER12, pFmt);
            default:
                MY_LOGE("RAWI bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if ((port.index == PORT_EISO.index) ||
                 (port.index == PORT_LCSO.index) ||
                 (port.index == PORT_RSSO.index)) {
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
    MUINT32 cnt=0;

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
                    cnt++;
                    while(cnt >10){
                        MY_LOGE("#########CFG can't be switched to %d: %d#########",PathCFG, PathStatus);
                        return 0;
                    }

                    if (cam_arg.Ut_obj->mbConfig){ //check Normalpipe configpipe phrase is finished or not
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

MBOOL SetImageQuality(_arg cam_arg_1, _arg cam_arg_2, E_CamIQLevel level_1, E_CamIQLevel level_2)
{
    MBOOL ret = MTRUE;
    CAM_QUALITY level_arg;
    CAM_QUALITY level_res;

    if(cam_arg_1.Ut_obj == NULL || cam_arg_2.Ut_obj == NULL){
        MY_LOGE("obj1 or obj2 null pointer");
        ret = MFALSE;
        goto Exit;
    }

    if(cam_arg_1.Ut_obj->mbIQNotify == MFALSE ||
        cam_arg_2.Ut_obj->mbIQNotify == MFALSE){
        MY_LOGE("un-receive the image set notification for previous time, cam1:%d cam2:%d",
            cam_arg_1.Ut_obj->mbIQNotify, cam_arg_2.Ut_obj->mbIQNotify);
        ret = MFALSE;
        goto Exit;
    }

    /* un-necessary to sendcmd for both cam, just choose one */
    /* update notify status first */
    cam_arg_1.Ut_obj->mbIQNotify = MFALSE;

    /* set quality */
    level_arg.Bits.Qlvl_0 = level_1;
    level_arg.Bits.SensorIdx_0 = cam_arg_1.Ut_obj->mSensorIdx;
    level_arg.Bits.Qlvl_1 = level_2;
    level_arg.Bits.SensorIdx_1 = cam_arg_2.Ut_obj->mSensorIdx;
    ret = cam_arg_1.Ut_obj->mpNPipe->sendCommand(ENPipeCmd_SET_QUALITY, (MINTPTR)&cam_arg_1.Ut_obj->mIQNotify, (MINTPTR)level_arg.Raw, 0);
    cam_arg_1.Ut_obj->mpNPipe->sendCommand(ENPipeCmd_GET_QUALITY, NULL, (MINTPTR)&level_res, 0);
    MY_LOGI("SET_QUALITY:%d,GET_QUALITY:%d",level_arg.Raw, level_res.Raw);

    /* check return call and re-update notify status */
    if(ret == MFALSE){
        cam_arg_1.Ut_obj->mbIQNotify = MTRUE;
        MY_LOGE("set quality fail");
    }

Exit:
    return ret;
}

inline MSize GetBufMSize(NPipeUT* pNpipeUt, PortID const PortId)
{
    MSize _msize;

    switch(PortId.index) {
    case NSImageio::NSIspio::EPortIndex_EISO:
        _msize = pNpipeUt->mEisNotify.queryEisOutSize();
        break;
    case NSImageio::NSIspio::EPortIndex_RSSO:
        _msize = pNpipeUt->mRssNotify.queryRssOutSize();
        break;
    case NSImageio::NSIspio::EPortIndex_LCSO:
        _msize = pNpipeUt->mTgSize;
        break;
    default:
        MY_LOGE("unsupport port(0x%x)", PortId.index);
        break;
    }
    return _msize;
}

inline MUINT32 _GetPort(PortID const PortId, MUINT32 const PlaneID = ePlane_1st)
{
    MUINT32 BufPort = __GARBAGE;

    switch(PortId.index) {
    case NSImageio::NSIspio::EPortIndex_RAWI:
        switch(PlaneID) {
        case ePlane_2nd:
            BufPort = __UFDI;
            break;
        case ePlane_3rd:
            BufPort = __UFDI_META;
            break;
        default:
            BufPort = __RAWI;
            break;
        }
        break;
    case NSImageio::NSIspio::EPortIndex_IMGO:
        switch(PlaneID) {
        case ePlane_2nd:
            BufPort = __UFEO;
            break;
        case ePlane_3rd:
            BufPort = __UFEO_META;
            break;
        default:
            BufPort = __IMGO;
            break;
        }
        break;
    case NSImageio::NSIspio::EPortIndex_CAMSV_IMGO:
        switch(PlaneID) {
        case ePlane_2nd:
            BufPort = __SV_UFEO;
            break;
        case ePlane_3rd:
            BufPort = __SV_UFEO_META;
            break;
        default:
            BufPort = __SV_IMGO;
            break;
        }
        break;
    case NSImageio::NSIspio::EPortIndex_RRZO:
        switch(PlaneID) {
            case ePlane_2nd:
                BufPort = __UFGO;
                break;
            case ePlane_3rd:
                BufPort = __UFGO_META;
                break;
            default:
                BufPort = __RRZO;
                break;
        }
        break;
    case NSImageio::NSIspio::EPortIndex_YUVO:
        switch(PlaneID) {
        case ePlane_2nd:
            BufPort = __YUVBO;
            break;
        case ePlane_3rd:
            BufPort = __YUVCO;
            break;
        default:
            BufPort = __YUVO;
            break;
        }
        break;
    case NSImageio::NSIspio::EPortIndex_CRZO:
        switch(PlaneID) {
        case ePlane_2nd:
            BufPort = __CRZBO;
            break;
        case ePlane_3rd:
            BufPort = __GARBAGE;
            break;
        default:
            BufPort = __CRZO;
            break;
        }
        break;
    case NSImageio::NSIspio::EPortIndex_CRZO_R2:
        switch(PlaneID) {
        case ePlane_2nd:
            BufPort = __CRZBO_R2;
            break;
        case ePlane_3rd:
            BufPort = __GARBAGE;
            break;
        default:
            BufPort = __CRZO_R2;
            break;
        }
        break;
    case NSImageio::NSIspio::EPortIndex_EISO:
        BufPort = __EISO;
        break;
    case NSImageio::NSIspio::EPortIndex_RSSO:
        BufPort = __RSSO;
        break;
    case NSImageio::NSIspio::EPortIndex_LCSO:
        BufPort = __LCSO;
        break;
    default:
        MY_LOGE("unsupport port(0x%x)", PortId.index);
        break;
    }

    return BufPort;
}

inline MBOOL RAWI_GEN_IMG(NPipeUT* ptr,IMEM_BUF_INFO* pbuf,SensorStaticInfo& mSensorInfo){
    EImageFormat fmt;
    MUINTPTR addr;
    getOutputFmt(PORT_RAWI, getPortBitDepth(PORT_RAWI, ptr->m_enablePort), mSensorInfo, &fmt, MFALSE);
    if(fmt != eImgFmt_BAYER8){
        MY_LOGE("support only 8bit normal bayer only\n");
        return MFALSE;
    }

    if((ptr->mTgSize.w %8) != 0){
        MY_LOGE("support only 8-alignment\n");
        return MFALSE;
    }
    if(ptr->mTgSize.h&0x1){
        MY_LOGE("support only 2-alignment\n");
        return MFALSE;
    }

    //1st line + 2nd line,   stride = width
    for(MUINT32 i=0; i<ptr->mTgSize.w; i+= 8){
        addr = pbuf->virtAddr+i;
        *(MUINTPTR*)addr = 0x40804080;
        *(MUINTPTR*)(addr+4) = 0x10201020;
    }
    //2nd line
    for(MUINT32 i=ptr->mTgSize.w; i<2*ptr->mTgSize.w; i+= 8){
        addr = pbuf->virtAddr+i;
        *(MUINTPTR*)addr = 0x80408040;
        *(MUINTPTR*)(addr+4) = 0x20102010;
    }
    MY_LOGI("######################");
    MY_LOGI("######################");
    MY_LOGI("SET RAWI!!!!!!!");
    MY_LOGI("######################");
    MY_LOGI("######################");
    for(MUINT32 i=2;i<ptr->mTgSize.h;i+=2){
        addr = pbuf->virtAddr+i*ptr->mTgSize.w;
        memcpy((char*)addr,(char*)pbuf->virtAddr,2*ptr->mTgSize.w);
    }

    return MTRUE;
}


inline MBOOL MutiPlaneAllocator(NPipeUT* ptr, PortID PortId, SensorStaticInfo mSensorInfo)
{
    IImageBuffer* pIBuf = NULL;
    MUINT32 portDepth;
    MUINT32 tmp;
    if(PortId.index == NSImageio::NSIspio::EPortIndex_RAWI)
        portDepth = 1;
    else
        portDepth = (PortId.index == NSImageio::NSIspio::EPortIndex_CAMSV_IMGO)?ptr->mDCIFPortBufDepth:ptr->mPortBufDepth;
    ptr->mpImgBuffer[_GetPort(PortId)].resize(portDepth);
    ptr->mpHeap[_GetPort(PortId)].resize(portDepth);

    for (MUINT32 PlaneID = ePlane_1st; PlaneID < ePlane_max; PlaneID++) {
        ptr->mImemBuf[_GetPort(PortId,PlaneID)].resize(portDepth);
    }
    for (MUINT32 j = 0; j < portDepth; j++) {
        EImageFormat fmt;
        NormalPipe_QueryInfo qry;
        MUINT32 bufStridesInBytes[ePlane_max] = {0,0,0};
        MINT32  bufBoundaryInBytes[ePlane_max] = {0,0,0};
        MUINTPTR bufVA[ePlane_max] = {0,0,0}, bufPA[ePlane_max] = {0,0,0};
        IMEM_BUF_INFO imgiBuf[ePlane_max];

        switch(PortId.index){
            case NSImageio::NSIspio::EPortIndex_RAWI:
            case NSImageio::NSIspio::EPortIndex_IMGO:
            case NSImageio::NSIspio::EPortIndex_RRZO:
            case NSImageio::NSIspio::EPortIndex_CAMSV_IMGO:
                if (!getOutputFmt(PortId, getPortBitDepth(PortId, ptr->m_enablePort), mSensorInfo, &fmt, MTRUE)) {
                    MY_LOGE("Port(%d) get pix fmt error", PortId.index);
                    return MFALSE;
                }
                break;
            case NSImageio::NSIspio::EPortIndex_YUVO:
            case NSImageio::NSIspio::EPortIndex_CRZO:
            case NSImageio::NSIspio::EPortIndex_CRZO_R2:
                if( (fmt = getExtentFmt(PortId,tmp)) == NSCam::eImgFmt_IMPLEMENTATION_DEFINED){
                    MY_LOGE("Port(%d) get pix fmt error", PortId.index);
                    return NULL;
                }
                break;
            default:
                MY_LOGE("muti-plane is not supported at Port(%d)", PortId.index);
                return MFALSE;
                break;
        }

        getNormalPipeModule()->query(PortId.index, ENPipeQueryCmd_STRIDE_BYTE, fmt, ptr->mTgSize.w, qry);

        for (MUINT32 PlaneID = ePlane_1st; PlaneID < ePlane_max; PlaneID++) {
            imgiBuf[PlaneID].size = ptr->mTgSize.h * qry.stride_B[PlaneID];

            if (ptr->mpImemDrv->allocVirtBuf(&imgiBuf[PlaneID]) < 0) {
                MY_LOGE("error: Port(%d) imem[%d] alloc fail", PortId.index, PlaneID);
                return MFALSE;
            }
            if (ptr->mpImemDrv->mapPhyAddr(&imgiBuf[PlaneID], MFALSE) < 0) {
                MY_LOGE("error: Port(%d) imem[%d] map fail", PortId.index, PlaneID);
                return MFALSE;
            }

            ptr->mImemBuf[_GetPort(PortId,PlaneID)].at(j) = imgiBuf[PlaneID];

            bufStridesInBytes[PlaneID] = qry.stride_B[PlaneID];
            bufVA[PlaneID]             = imgiBuf[PlaneID].virtAddr;
            bufPA[PlaneID]             = imgiBuf[PlaneID].phyAddr;
            MY_LOGD("Port(%d) imem[%d] id:%d va:%p pa:%p", PortId.index, PlaneID,
                imgiBuf[PlaneID].memID, (void*)imgiBuf[PlaneID].virtAddr, (void*)imgiBuf[PlaneID].phyAddr);
        }

        {
            //always use 1st plane to create BufferHeap
            PortBufInfo_dummy portBufInfo(imgiBuf[ePlane_1st].memID, bufVA, bufPA, ePlane_max);
            IImageBufferAllocator::ImgParam imgParam(fmt, ptr->mTgSize/*imgo/rrzo use Tg size*/,
                                                        bufStridesInBytes, bufBoundaryInBytes, ePlane_max);
            ptr->mpHeap[_GetPort(PortId)].at(j) = IDummyImageBufferHeap::create(mThisName, imgParam, portBufInfo, false);

            ImgBufCreator creator(fmt);
            pIBuf = ptr->mpHeap[_GetPort(PortId)].at(j)->createImageBuffer(&creator);
            pIBuf->incStrong(pIBuf);
            pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE |
                                      eBUFFER_USAGE_SW_READ_OFTEN |
                                      eBUFFER_USAGE_SW_WRITE_OFTEN);

            ptr->mpImgBuffer[_GetPort(PortId)].at(j) = pIBuf;

            for (MUINT32 PlaneID = ePlane_1st; PlaneID < ePlane_max; PlaneID++) {
                MY_LOGD("[%d]Port(%d) img[%d] pa(0x%lx)", j, PortId.index, PlaneID, pIBuf->getBufPA(PlaneID));
            }
        }

        if(PortId.index==NSImageio::NSIspio::EPortIndex_RAWI) {
            if(RAWI_GEN_IMG(ptr,&imgiBuf[0],mSensorInfo) == MFALSE)
                return MFALSE;
        }
    }
    return MTRUE;
}

inline MBOOL SinglePlaneAllocator(NPipeUT* ptr, PortID PortId, SensorStaticInfo mSensorInfo, MUINT32 SecEnable)
{
    IImageBuffer* pIBuf = NULL;
    MUINT32 portDepth;
    MUINT32 tmp;
    if(PortId.index == NSImageio::NSIspio::EPortIndex_RAWI)
        portDepth = 1;
    else
        portDepth = (PortId.index == NSImageio::NSIspio::EPortIndex_CAMSV_IMGO)?ptr->mDCIFPortBufDepth:ptr->mPortBufDepth;
    ptr->mpImgBuffer[_GetPort(PortId)].resize(portDepth);
    ptr->mImemBuf[_GetPort(PortId)].resize(portDepth);
    ptr->mpHeap[_GetPort(PortId)].resize(portDepth);
    if(SecEnable){
        ptr->mImemSecBuf[_GetPort(PortId)].resize(portDepth);
        ptr->mImemSecChkBuf[_GetPort(PortId)].resize(portDepth);
    }


    for (MUINT32 j = 0; j < portDepth; j++) {
        MUINT32 bufStridesInBytes[3] = {0,0,0};
        MINT32  bufBoundaryInBytes[3] = {0,0,0};
        MUINTPTR bufVA = 0, bufPA = 0;
        EImageFormat fmt;
        MSize _msize;
        IMEM_BUF_INFO imgiBuf,imgiSecBuf,imgiSecChkBuf;
        NormalPipe_QueryInfo qry;

        switch(PortId.index){
            case NSImageio::NSIspio::EPortIndex_RAWI:
            case NSImageio::NSIspio::EPortIndex_IMGO:
            case NSImageio::NSIspio::EPortIndex_RRZO:
            case NSImageio::NSIspio::EPortIndex_CAMSV_IMGO:
                if (PortId.index == NSImageio::NSIspio::EPortIndex_IMGO && ptr->mbRanImgoFmt == MTRUE) {
                    MY_LOGD("[%d]getOutputFmt IMGO Depth 12");
                    if(!getOutputFmt(PortId, 12, mSensorInfo, &fmt, MFALSE)){
                        MY_LOGE("[%d]Port(%d) get pix fmt error\n", j, PortId.index);
                        return MFALSE;
                    }
                } else {
                    if(!getOutputFmt(PortId, getPortBitDepth(PortId, ptr->m_enablePort), mSensorInfo, &fmt, MFALSE)){
                        MY_LOGE("[%d]Port(%d) get pix fmt error\n", j, PortId.index);
                        return MFALSE;
                    }
                }
                MY_LOGD("[%d]Port(%d) img fmt(0x%x)", j, PortId.index, fmt);
                getNormalPipeModule()->query(PortId.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                           fmt, ptr->mTgSize.w, qry);

                _msize = ptr->mTgSize;
                imgiBuf.size = ptr->mTgSize.h * qry.stride_byte;
                MY_LOGD("[%d]Port(%d) qry.stride_byte(%d)", j, PortId.index, qry.stride_byte);
                bufStridesInBytes[ePlane_1st] = qry.stride_byte;
                if(SecEnable){
                    imgiSecBuf.size = ptr->mTgSize.h * qry.stride_byte;
                    imgiSecChkBuf.size = 0x10;
                }
                break;
            case NSImageio::NSIspio::EPortIndex_YUVO:
            case NSImageio::NSIspio::EPortIndex_CRZO:
            case NSImageio::NSIspio::EPortIndex_CRZO_R2:
                if( (fmt = getExtentFmt(PortId,tmp)) == NSCam::eImgFmt_IMPLEMENTATION_DEFINED){
                    MY_LOGE("Port(%d) get pix fmt error", PortId.index);
                    return NULL;
                }

                getNormalPipeModule()->query(PortId.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                       fmt, ptr->mTgSize.w, qry);

                _msize = ptr->mTgSize;
                _msize.w = qry.x_pix;

                imgiBuf.size = ptr->mTgSize.h * qry.stride_byte;
                bufStridesInBytes[ePlane_1st] = qry.stride_byte;
                if(SecEnable)
                    MY_LOGE("Port(%d) does not support secure", PortId.index);
                break;
            default:
                fmt = eImgFmt_BLOB;
                _msize = GetBufMSize(ptr, PortId);
                imgiBuf.size = _msize.h * _msize.w;
                bufStridesInBytes[ePlane_1st] = _msize.w;
                break;
        }

        if(SecEnable){
            //return imgiBuf.memID, imgiBuf.virtAddr, imgiBuf.phyAddr
            if(ptr->mpImemDrv->allocSecBuf(&imgiSecBuf) < 0){
                MY_LOGE("error:[%d]Port(%d) secure imem alloc fail", j, PortId.index);
                return MFALSE;
            }
            if(ptr->mpImemDrv->allocSecBuf(&imgiSecChkBuf) < 0){
                MY_LOGE("error:[%d]Port(%d) secure imem alloc fail", j, PortId.index);
                return MFALSE;
            }
            //For dump secure buffer
            if(ptr->mpImemDrv->allocVirtBuf(&imgiBuf) < 0){
                MY_LOGE("error:[%d]Port(%d) imem alloc fail", j, PortId.index);
                return MFALSE;
            }
        }else{
            if(ptr->mpImemDrv->allocVirtBuf(&imgiBuf) < 0){
                MY_LOGE("error:[%d]Port(%d) imem alloc fail", j, PortId.index);
                return MFALSE;
            }
        }

        if(SecEnable){
            ptr->mImemBuf[_GetPort(PortId)].at(j) = imgiSecBuf;
            ptr->mImemSecChkBuf[_GetPort(PortId)].at(j) = imgiSecChkBuf;
            ptr->mImemSecBuf[_GetPort(PortId)].at(j) = imgiBuf; //For secure mem dump
        }else{
            ptr->mImemBuf[_GetPort(PortId)].at(j) = imgiBuf;
        }

        MY_LOGD("[%d]Sec(%d) Port(%d) imem id:%d va:%p", j, SecEnable, PortId.index, imgiBuf.memID, (void*)imgiBuf.virtAddr);
        {
         if(SecEnable){
             PortBufInfo_v1 portBufInfo(imgiSecBuf.memID, imgiSecBuf.virtAddr, 0, SecEnable, imgiSecBuf.bufCohe);

             IImageBufferAllocator::ImgParam imgParam(fmt, _msize, bufStridesInBytes, bufBoundaryInBytes, 1);
             ptr->mpHeap[_GetPort(PortId)].at(j) = ImageBufferHeap::create(mThisName, imgParam,portBufInfo, MTRUE);

         }else{
             PortBufInfo_v1 portBufInfo(imgiBuf.memID, imgiBuf.virtAddr, 0, imgiBuf.bufSecu, imgiBuf.bufCohe);

             IImageBufferAllocator::ImgParam imgParam(fmt, _msize, bufStridesInBytes, bufBoundaryInBytes, 1);
             ptr->mpHeap[_GetPort(PortId)].at(j) = ImageBufferHeap::create(mThisName, imgParam, portBufInfo, MTRUE);
         }

         pIBuf = ptr->mpHeap[_GetPort(PortId)].at(j)->createImageBuffer();
         pIBuf->incStrong(pIBuf);
         pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
         ptr->mpImgBuffer[_GetPort(PortId)].at(j) = pIBuf;
         if(SecEnable)
             MY_LOGD("[%d]Port(%d) secure img pa:0x%llx chk pa:0x%llx", j, PortId.index, imgiSecBuf.phyAddr, imgiSecChkBuf.phyAddr);
         MY_LOGD("[%d]Port(%d) img pa:0x%llx", j, PortId.index, pIBuf->getBufPA(ePlane_1st));
        }

        if(PortId.index==NSImageio::NSIspio::EPortIndex_RAWI) {
            if(RAWI_GEN_IMG(ptr,&imgiBuf,mSensorInfo) == MFALSE)
                return MFALSE;
        }

    }
    return MTRUE;
}

MBOOL portMap(MUINT32 enableport, MUINT32 portID){
    switch(portID){
        case __RRZO:
            if(enableport & __RRZO_ENABLE)
                return MTRUE;
            break;
        case __IMGO:
            if(enableport & __IMGO_ENABLE)
                return MTRUE;
            break;
        case __EISO:
            if(enableport & __EISO_ENABLE)
                return MTRUE;
            break;
        case __LCSO:
            if(enableport & __LCSO_ENABLE)
                return MTRUE;
            break;
        case __RSSO:
            if(enableport & __RSSO_ENABLE)
                return MTRUE;
            break;
        case __SV_IMGO:
            if(enableport & __SV_DCIF_IMGO_ENABLE)
                return MTRUE;
            break;
        case __YUVO:
            if(enableport & __YUVO_ENABLE)
                return MTRUE;
            break;
        case __CRZO:
            if(enableport & __CRZO_ENABLE)
                return MTRUE;
            break;
        case __CRZO_R2:
            if(enableport & __CRZO_R2_ENABLE)
                return MTRUE;
            break;
        case __RAWI:
            if(enableport & __RAWI_ENABLE)
                return MTRUE;
            break;
        case __SV_UFEO:
        case __UFDI:
        case __UFGO:
        case __UFEO:
        default:
            MY_LOGW("not a phy-port:%d\n",portID);
            break;
    }

    return MFALSE;
}

/*******************************************************************************
*  Main Function
********************************************************************************/
static MVOID* test_camio_thread(void *ts_arg)
{
    char                filename[256];
    MUINT32             i = 0, j = 0, k = 0, sidx = 0, dma = 0, bSetSensorTP = 0, fidx;
    MUINT32             sensorDev, scenario, sensorFps, enablePort,  sensorNum = 0, sttPorts = 0;
    CROP_TEST::ECROP    croptestsel = CROP_TEST::CROP_NONE;
    MUINT32             pixelMode[2] = {0}, stopFlow = 0, waitTgInt = 0;
    MUINT32             twinCtrl = 1, sensorPixMode = 2, forceBinOff = 0,Dual_PD = 0, cmdsfiled = 0, fpsCtrl = 0, scaleCtrl = 0;
    uint32_t            sensorArray[_MAXTG_];
    SensorStaticInfo    mSensorInfo[_MAXTG_];
    IHalSensor*         pSensorHalObj[_MAXTG_] = {NULL};
    vector<IHalSensor::ConfigParam> vSensorCfg[2];
    IHalSensorList*     pHalSensorList = NULL;
    IMEM_BUF_INFO       imgiBuf, imgiBuf_2nd, imgiBuf_3rd;
    NPipeUT*            pUt[_MAXTG_] = {NULL};
    MBOOL               stN3D = MFALSE, _efuse = MFALSE, m_bN3DTest = MFALSE, ret = MFALSE;
    MBOOL               bRanIQ = MFALSE, bRanUF = MFALSE, bRanImgoFmt = MFALSE, bEnDTwin = MTRUE;
    MINT32              _efuse_idx = BD_SEG_NUM - 1;
#ifndef HWSYNC_TEST
    MUINT32             _frameNum = 8;
#else
    MUINT32             _frameNum = 64;
#endif
    MUINT32             _burstQNum = 1, _switchCam = 0, _dcif_burstQNum = 0, tuningBurstNum = 1;
    pthread_t           _switchThd;
    MUINT32             checksum[2] = {0}, crc_accum[2] = {0}, suspendFrm = 0, suspendFlow = 0, suspendTest = 0;
    MBOOL               crc_test_result = MTRUE;
    _arg*               arg;
    MUINT32             temp_loopCnt = 0;
    E_CamPattern        data_pattern = eCAM_NORMAL;
    MUINT32             scaleSel = SCALE_NONE, scaleFactor = 100;
    MUINT32             SecEnable = 0;
    MUINT32             needConfigSensorByMW = 0;

    arg = (_arg*)ts_arg;


#if 1
    MY_LOGD("sizeof long : %d", (int)sizeof(long));
    if (arg->_argc < 5) { // iopipeTest 1 0 0 2 0, test pattern no crop
        MY_LOGI("Param: 1 <sensorDev> <scenario> <enablePort> <cropTestSel> "\
            "<sttPorts> <burstQNum> <FrameNum> <stopFlow> <switchCam> <twinCtrl> <combinedcmds> <suspend_test> "\
            "<N3D_enable> <fpsCtrl> <ranIQ_enable> <data_pattern> <dtwin_enable> <scaleTestSel>");
        MY_LOGI("<sensorDev>    : 1 main, 2 sub, bitmap; 0 for seninf test model");
        MY_LOGI("<scenario>     : preview %d, capture %d, video %d, slim1 %d, slim2 %d, sensorTestPattern 255",\
                SENSOR_SCENARIO_ID_NORMAL_PREVIEW, SENSOR_SCENARIO_ID_NORMAL_CAPTURE,\
                SENSOR_SCENARIO_ID_NORMAL_VIDEO, SENSOR_SCENARIO_ID_SLIM_VIDEO1, SENSOR_SCENARIO_ID_SLIM_VIDEO2);
        MY_LOGI("<enablePort>   : 0x01 rrzo, 0x02 imgo, 0x04 eiso, 0x08 lcso,0x20 rsso, 0x10 imgo+ufeo, 0x40 rrzo+ufgo bitmap, 0x100 DCIF, 0x1000 RAWI,0x2000 UFDI+RAWI");
        MY_LOGI("<enablePort>   : 0x200 YUVO, 0x400 CRZO, 0x800 CRZO_R2");
        MY_LOGI("<cropTestSel>  : 0 no crop, 1 symmetric crop, 2 crop left only, 3 crop right only,4 specific size");
        MY_LOGI("<sttPorts>     : 1 aao, 2 afo, 4 flko, 8 pdo, 0x10 tsfso, 8192 camsv imgo, bitmap");
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
        MY_LOGI("<combinedcmds> : (input 0x00000011) 0: no, 0x1:TG_INTERRUPT_TEST, 0x2:EFUSE_TEST, 0x4:PIPELINE_RAW10, 0x8:PIPELINE_RAW12, 0x10:PIPELINE_RAW14, 0x20:PIPELINE_RAW16");
        MY_LOGI("<suspend_test> : b'[0]     : 0).off 1).on");
        MY_LOGI("               : b'[3:1]   : 0).one time suspend->resume 1).non-stop suspend->resume(dynamic twin) 2).suspend->stop");
        MY_LOGI("               : b'[15:4]  : frame# for enter suspend for random time");
        MY_LOGI("<N3D_enable>   : N3D Control");
        MY_LOGI("<fpsCtrl>      : b'[1:0]   : 0).default fps 1).specific fps 2).random fps");
        MY_LOGI("               : b'[31:2]  : specific fps");
        MY_LOGI("<random_test>  : 0).default 1).randomly switch 2).random imgoFmt");
        MY_LOGI("               : b'[0]     : IQ");
        MY_LOGI("               : b'[1]     : Bayer Encoder");
        MY_LOGI("               : b'[2]     : ImgoFmt Bayer10/Bayer12 Switch");
        MY_LOGI("<data_pattern> : refer to enum E_CamPattern");
        MY_LOGI("<dtwin_enable> : 0 disable dtwin, 1 enable dtwin");
        MY_LOGI("<scaleTestSel> : b'[1:0]   : 0).none scaling 1).default scaling factor 2).specific scaling factor");
        MY_LOGI("               : b'[31:2]  : specific scaling factor");
        MY_LOGI("<DCIFburstQNum>: 1 for normal");
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
        forceBinOff = (twinCtrl & 0x10) >> 4;

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
        stN3D = atoi(arg->_argv[CMD_IDX_N3DEN]);
        MY_LOGD("N3D control: 0x%x", stN3D);
    }

    if (arg->_argc > CMD_IDX_FPS_CTRL) {
        fpsCtrl = atoi(arg->_argv[CMD_IDX_FPS_CTRL]);
        MY_LOGD("FPS Ctrl: %d", fpsCtrl);
    }

    if (arg->_argc > CMD_IDX_RANDOM_TEST) {
        bRanIQ = (atoi(arg->_argv[CMD_IDX_RANDOM_TEST]) & 0x1) ? MTRUE : MFALSE;
        bRanUF = (atoi(arg->_argv[CMD_IDX_RANDOM_TEST]) & 0x2) ? MTRUE : MFALSE;
        bRanImgoFmt = (atoi(arg->_argv[CMD_IDX_RANDOM_TEST]) & 0x4) ? MTRUE : MFALSE;
        MY_LOGD("Random IQ(%d) UF(%d) Imgo(%d)", bRanIQ, bRanUF, bRanImgoFmt);
    }

    if (arg->_argc > CMD_IDX_DATA_PATTERN) {
        Dual_PD = MFALSE;
        switch(atoi(arg->_argv[CMD_IDX_DATA_PATTERN])){
            case eCAM_NORMAL:
            case eCAM_QuadCode:
            case eCAM_4CELL:
            case eCAM_MONO:
            case eCAM_IVHDR:
            case eCAM_ZVHDR:
            case eCAM_4CELL_IVHDR:
            case eCAM_4CELL_ZVHDR:
                data_pattern = (E_CamPattern)atoi(arg->_argv[CMD_IDX_DATA_PATTERN]);
                break;
            case eCAM_DUAL_PIX_IVHDR:
            case eCAM_DUAL_PIX_ZVHDR:
            case eCAM_DUAL_PIX:
                Dual_PD = MTRUE;
                data_pattern = (E_CamPattern)atoi(arg->_argv[CMD_IDX_DATA_PATTERN]);
                break;
            default:
                MY_LOGE("pattern r not support_%d\n",atoi(arg->_argv[CMD_IDX_DATA_PATTERN]));
                return NULL;
                break;
        }

        MY_LOGD("Sensor Data Pattern: %d", data_pattern);
    }

    if (arg->_argc > CMD_IDX_DTWINEN) {
        bEnDTwin = atoi(arg->_argv[CMD_IDX_DTWINEN]);
        MY_LOGD("DTwin Enable: %d", bEnDTwin);
    }

    if (arg->_argc > CMD_IDX_SCALESEL) {
        scaleCtrl = atoi(arg->_argv[CMD_IDX_SCALESEL]);
        MY_LOGD("Scale Ctrl: %d", scaleCtrl);

        // scale selection
        switch(scaleCtrl & 0x3){
            case 0:  scaleSel = SCALE_NONE;     break;
            case 1:  scaleSel = SCALE_DEFAULT;  break;
            case 2:  scaleSel = SCALE_SPECIFIC; break;
            default: scaleSel = SCALE_NONE;     break;
        }

        // scale factor
        if(scaleSel == SCALE_SPECIFIC){
            if((scaleCtrl >> 2) > 0 && (scaleCtrl >> 2) <= 100){
                scaleFactor = (scaleCtrl >> 2);
            }
            else{
                MY_LOGE("Illegal scaling factor:%d", scaleCtrl >> 2);
                scaleSel = SCALE_NONE;
                scaleFactor = 100;
            }
        }
    }

    if (arg->bDCIF) {
        if (arg->_argc > CMD_IDX_DCIF_BURSTNUM) {
            _dcif_burstQNum = atoi(arg->_argv[CMD_IDX_DCIF_BURSTNUM]);
            if (_dcif_burstQNum <= 0) {
                 MY_LOGE("dcif burst number must be larger than zero\n");
                 return NULL;
            }
        }
        else {
            _dcif_burstQNum = 4;
        }
    }

    if (arg->_argc > CMD_IDX_SECURECAM) {
        SecEnable = atoi(arg->_argv[CMD_IDX_SECURECAM]);
        MY_LOGD("Secure Cam Enable: %d", SecEnable);
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
        if (sttPorts & __SV_STT_IMGO_ENABLE) {
            MY_LOGD("output CAMSV IMGO");
        }
        if (sttPorts & __PDO_ENABLE){
            MY_LOGD("enable PDO\n");
        }
        if (sttPorts & __TSFSO_ENABLE){
            MY_LOGD("enable TSFSO\n");
        }
    }

    sensorNum = 0;
    if (0 == sensorDev) {
        sensorDev = UT_MAIN_FKSNR;
    }
    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_MAIN_SNR | UT_MAIN_FKSNR)) {
            sensorArray[sensorNum] = 0;
            sensorNum++;
        }
    }
    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_SUB_SNR | UT_SUB_FKSNR)) {
            sensorArray[sensorNum] = 1;
            sensorNum++;
        }
    }
    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_MAIN2_SNR | UT_MAIN2_FKSNR)) {
            sensorArray[sensorNum] = 2;
            sensorNum++;
        }
    }
    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_SUB2_SNR | UT_SUB2_FKSNR)) {
            sensorArray[sensorNum] = 3;
            sensorNum++;
        }
    }
    if (sensorDev & UT_FKSNR_MASK) {
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

    #if 0
    MY_LOGD("wait patch for query command...\n");
    #else
    {
        sCAM_QUERY_TUNING_FREQ qryTun;
        getNormalPipeModule()->query(ENPipeQueryCmd_TUNING_FREQ,(MUINTPTR)&qryTun);
        if (qryTun.tuningFreq == TUNING_FREQ_ALIGN_ENQ_REQ)
            tuningBurstNum = 1;
        else
            tuningBurstNum = _burstQNum;
    }
    #endif

    MY_LOGD("sensorDev_0x%x senario_0x%x enablePort_0x%x sensorNum_0x%x croptestsel_0x%x sttPort_0x%x",\
            sensorDev, scenario,  enablePort, sensorNum, croptestsel, sttPorts);
    MY_LOGD("supported_bit_%d/12 ", SEN_PIX_BITDEPTH);
    MY_LOGD("sensorArray: %d, %d", sensorArray[0], sensorArray[1]);
    MY_LOGD("burstNum: %d, dcif_burstNum:%d tuningBurstNum: %d", _burstQNum, _dcif_burstQNum, tuningBurstNum);
    MY_LOGD("twinCtrl: %d , pix/bin_off/DBN(%d/%d/%d), ", twinCtrl, sensorPixMode, forceBinOff, Dual_PD);
    MY_LOGD("efuse: %d", _efuse);
#endif
    #if 0 // force config smi larb mmu setting to PA, for ion carve-out
    {
        #include "isp_drv_cam.h"

        static MBOOL bConfigSmiLarb = MFALSE;
        static Mutex cfgSmiLabMutex;
        ISP_DRV_CAM *ptr;
        ISP_LARB_MMU_STRUCT larbInfo;
        int idx = 0;

        cfgSmiLabMutex.lock();

        if(bConfigSmiLarb)
            goto BYPASS_SMI_LARB_CFG;

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
            cfgSmiLabMutex.unlock();
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

        bConfigSmiLarb = MTRUE;
BYPASS_SMI_LARB_CFG:
        cfgSmiLabMutex.unlock();
        MY_LOGD("Config SMI LARB done");
    }
    #endif

    pHalSensorList = (IHalSensorList*)arg->mIHalSensorList;

SEARCH_SENSOR:

    for (sidx = 0; sidx < sensorNum; sidx++) {
        MUINT32 sttDma = 0;

        pUt[sidx] = NPipeUT::create();
        arg->Ut_obj = pUt[sidx];

        if (sttPorts & __SV_STT_IMGO_ENABLE) {
            pUt[sidx]->mbCamsvEn = MTRUE;
            pUt[sidx]->mEnableSttPort = (sttPorts & ~__SV_STT_IMGO_ENABLE);
        }
        else {
            pUt[sidx]->mEnableSttPort = sttPorts; //modify for uni
        }
        pUt[sidx]->mStaticEnqCnt = 1; //should > 0
        pUt[sidx]->mFrameNum = _frameNum;
        pUt[sidx]->mBurstQNum = _burstQNum;
        pUt[sidx]->mBurstTuningQNum = tuningBurstNum;
        pUt[sidx]->mPortBufDepth = _burstQNum * ENQUE_BURST_CNT;
        pUt[sidx]->mDCIFBurstQNum = _dcif_burstQNum;
        pUt[sidx]->mDCIFPortBufDepth = _burstQNum * _dcif_burstQNum * ENQUE_BURST_CNT;
        pUt[sidx]->mArgc = arg->_argc;
        pUt[sidx]->mArgv = arg->_argv;
        pUt[sidx]->m_enablePort = enablePort;
        pUt[sidx]->mEnableTgInt = waitTgInt;
        pUt[sidx]->m_Dualpd.Bits.bDual = Dual_PD;
        pUt[sidx]->mScaleSel = scaleSel;
        pUt[sidx]->mScaleFactor = scaleFactor;
        pUt[sidx]->m_bDCIF = arg->bDCIF;
        pUt[sidx]->mSecEnable = SecEnable;
        pUt[sidx]->mbDynSwitch = arg->bDynSwitch;
        if(Dual_PD){
            // nowadays only support low density
            #if 0
            int str;
            MY_LOGI("Input PD density,Param:1 for High, 0 for Low");
            str = getchar();
            pUt[sidx]->m_Dualpd.Bits.Density = (str == '1')?(1):(0);
            #endif

            // low density
            pUt[sidx]->m_Dualpd.Bits.Density = 0;
        }
        pUt[sidx]->mSuspendTest = suspendTest;
        pUt[sidx]->mSuspendFrm = suspendFrm;
        pUt[sidx]->mSuspendFlow = suspendFlow;
        pUt[sidx]->mSuspendState = 0;
        pUt[sidx]->mbRanImgoFmt = bRanImgoFmt;

        ::sem_init(&pUt[sidx]->m_semSwitchThd, 0, 0);

        if(pUt[sidx]->m_enablePort & __UFDI_ENABLE){
            pUt[sidx]->m_ufdi_en = 1;
            pUt[sidx]->m_enablePort ^= __UFDI_ENABLE;
            pUt[sidx]->m_enablePort |= __RAWI_ENABLE;
        }
        else{
            pUt[sidx]->m_ufdi_en = 0;
        }

        if(pUt[sidx]->m_enablePort & __UFEO_ENABLE){
            pUt[sidx]->m_ufeo_en = 1;
            pUt[sidx]->m_enablePort ^= __UFEO_ENABLE;
            pUt[sidx]->m_enablePort |= __IMGO_ENABLE;
            pUt[sidx]->mbRanUF       = bRanUF;
        }
        else{
            pUt[sidx]->m_ufeo_en = 0;
        }

        if(pUt[sidx]->m_enablePort & __UFGO_ENABLE){
            pUt[sidx]->m_ufgo_en = 1;
            pUt[sidx]->m_enablePort ^= __UFGO_ENABLE;
            pUt[sidx]->m_enablePort |= __RRZO_ENABLE;
            pUt[sidx]->mbRanUF       = bRanUF;
        }
        else{
            pUt[sidx]->m_ufgo_en = 0;
        }

        if(pUt[sidx]->m_bDCIF){
            if(pUt[sidx]->m_enablePort & __SV_DCIF_UFEO_ENABLE){
                pUt[sidx]->m_sv_ufeo_en = MTRUE;
                pUt[sidx]->m_enablePort ^= __SV_DCIF_UFEO_ENABLE;
                pUt[sidx]->m_enablePort |= __SV_DCIF_IMGO_ENABLE;
            }
            else{
                pUt[sidx]->m_sv_ufeo_en = MFALSE;
                pUt[sidx]->m_enablePort |= __SV_DCIF_IMGO_ENABLE;
            }
        }
        else{
            pUt[sidx]->m_sv_ufeo_en = MFALSE;
            pUt[sidx]->m_enablePort &= ~__SV_DCIF_UFEO_ENABLE;
            pUt[sidx]->m_enablePort &= ~__SV_DCIF_IMGO_ENABLE;
        }

        if (pUt[sidx]->mStaticEnqCnt > ENQUE_BURST_CNT) {
            pUt[sidx]->mStaticEnqCnt = ENQUE_BURST_CNT;
        }

        MY_LOGD("create %d obj : [SenIdx:%d] %p", sidx + 1, arg->TG_Num, (MUINT8*)pUt[sidx]);
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
            MUINT32 _pix;

            switch(sensorPixMode){
            case 2:
                _pix = TWO_PIXEL_MODE;
                break;
            case 4:
                _pix = FOUR_PIXEL_MODE;
                break;
            case 1:
            default:
                _pix = ONE_PIXEL_MODE;
                break;
            }
            ((TS_FakeSensor*)pSensorHalObj[sidx])->setFakeAttribute(TS_FakeSensor::eFake_PixelMode, (MUINTPTR)_pix);

            if (pUt[sidx]->mbCamsvEn) {
                ((TS_FakeSensor*)pSensorHalObj[sidx])->setFakeAttribute(TS_FakeSensor::eFake_CamsvChannel, (MUINTPTR)MTRUE);
            }
        }

        pSensorHalObj[sidx]->powerOn(mThisName, 1, &sensorArray[sidx]);
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

        if (SecEnable) {
            IMEM_BUF_INFO initSecBuf;
            initSecBuf.size = 0x100;
            MY_LOGD("Allocate First Secure buffer ...");
            if(pUt[sidx]->mpImemDrv->allocSecBuf(&initSecBuf) < 0){
                MY_LOGD("error:init secure imem alloc fail (sidx:%d)",sidx);
            }
        }
    }

    MY_LOGD("create...");

    for (sidx = 0; sidx < sensorNum; sidx++) {
        MY_LOGD("SenIdx: %d", sensorArray[sidx]);

        pUt[sidx]->mSensorIdx = sensorArray[sidx];

        getNormalPipeModule()->createSubModule(
            sensorArray[sidx], mThisName, selectNormalPipeVersion(sensorArray[sidx]), (MVOID**)&(pUt[sidx]->mpNPipe));

        ret = pUt[sidx]->mpNPipe->init(SecEnable);
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
        pUt[sidx]->mCurTgOut = pUt[sidx]->mTgSize;

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
        if(fpsCtrl){
            if((fpsCtrl & 0x3) == 0x1){
                sensorFps = fpsCtrl >> 2;
                ((TS_FakeSensor*)pSensorHalObj[sidx])->setFakeAttribute(TS_FakeSensor::eFake_FPS, (MUINTPTR)sensorFps);
                MY_LOGD("#####Specific FPS: %d#####", sensorFps);
            }
            else if((fpsCtrl & 0x3) == 0x2){
                /* Choose Random FPS */
                /* Maximum: defaultFps * sensorPixMode * factor */
                MUINT32 lowerBound = sensorFps;
                MUINT32 higherBound = sensorFps * 30;
                sensorFps = sensorFps + (rand() % (higherBound - lowerBound + 1));
                ((TS_FakeSensor*)pSensorHalObj[sidx])->setFakeAttribute(TS_FakeSensor::eFake_FPS, (MUINTPTR)sensorFps);
                MY_LOGD("#####Enable Random FPS: %d#####", sensorFps);
            }
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

    EImageFormat            fmt;
    MRect                   _cropsize;
    MSize                   _destsize;
    list<HwPortConfig_t>    lHwPortCfg[2];
    MUINT32 yuv_plane=0,crz_plane=0,crz_r2_plane=0;

    for (sidx = 0; sidx < sensorNum; sidx++) {

        //crop mode
        pUt[sidx]->m_crop.InitCfg(pUt[sidx]->mTgSize,croptestsel,pUt[sidx]->mTgSize);

        if (pUt[sidx]->m_enablePort & __RAWI_ENABLE) {
            NormalPipe_QueryInfo qry;


            if (!getOutputFmt(PORT_RAWI, getPortBitDepth(PORT_RAWI, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt, pUt[sidx]->m_ufdi_en)) {
                MY_LOGE("get pix fmt error");
                return NULL;
            }

            MY_LOGD("0x%x: port RAWI", arg->TG_Num);


            //set/get crop info
            if(croptestsel != CROP_TEST::CROP_NONE){
                MY_LOGE("rawi support no crop");
                return NULL;
            }
            else{
                _cropsize = MRect(MPoint(0,0),MSize(pUt[sidx]->mTgSize.w,pUt[sidx]->mTgSize.h));
            }

            getNormalPipeModule()->query(PORT_RAWI.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                fmt, _cropsize.s.w, qry);


            MY_LOGD("RAWI query x_pix:(%d) x_strid_byte:(%d/%d/%d) sensor size:(%d_%d) crop:(%d_%d),(%d_%d) dest:(%d_%d)",
                qry.x_pix, qry.stride_B[ePlane_1st], qry.stride_B[ePlane_2nd], qry.stride_B[ePlane_3rd],
                pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h, _cropsize.p.x, _cropsize.p.y,_cropsize.s.w,
                _cropsize.s.h, _cropsize.s.w, _cropsize.s.h);
            {
                HwPortConfig_t full = {
                    .mPortID  = PORT_RAWI,
                    .mFmt     = fmt,
                    .mSize    = _cropsize.s,
                    .mCrop    = _cropsize,
                    .mPureRaw = 0, //don't care
                    .mStrideInByte[ePlane_1st] = qry.stride_B[ePlane_1st],
                    .mStrideInByte[ePlane_2nd] = qry.stride_B[ePlane_2nd],
                    .mStrideInByte[ePlane_3rd] = qry.stride_B[ePlane_3rd]
                };

                lHwPortCfg[sidx].push_back(full);
            }
        }

        if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
            NormalPipe_QueryInfo qry;

            if(pUt[sidx]->m_ufeo_en == 1){
                if (!getOutputFmt(PORT_IMGO, getPortBitDepth(PORT_IMGO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt, MTRUE)) {
                    MY_LOGE("get pix fmt error");
                    return NULL;
                }
            }
            else{
                if (!getOutputFmt(PORT_IMGO, getPortBitDepth(PORT_IMGO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt, MFALSE)) {
                    MY_LOGE("get pix fmt error");
                    return NULL;
                }
            }

            MY_LOGD("0x%x: port IMGO", arg->TG_Num);

            getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                fmt, pUt[sidx]->mTgSize.w, qry);

            //set/get crop info
            if(croptestsel == CROP_TEST::CROP_SPECIFIC){
                pUt[sidx]->m_crop.SetInfo(PORT_IMGO,fmt,arg->custCropSize);
            }
            pUt[sidx]->m_crop.GetCropInfo(PORT_IMGO,fmt,_cropsize,MTRUE);

            MY_LOGD("imgo query x_pix:(%d) x_strid_byte:(%d/%d/%d) sensor size:(%d_%d) crop:(%d_%d),(%d_%d) dest:(%d_%d)",
                qry.x_pix, qry.stride_B[ePlane_1st], qry.stride_B[ePlane_2nd], qry.stride_B[ePlane_3rd],
                pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h, _cropsize.p.x, _cropsize.p.y,_cropsize.s.w,
                _cropsize.s.h, _cropsize.s.w, _cropsize.s.h);
            {
                HwPortConfig_t full = {
                    .mPortID  = PORT_IMGO,
                    .mFmt     = fmt,
                    .mSize    = _cropsize.s,
                    .mCrop    = _cropsize,
                    .mPureRaw = (pUt[sidx]->m_enablePort & __IMGO_PURE_RAW) ? 1 : 0, //if raw type != 1 -> pure-raw
                    .mStrideInByte[ePlane_1st] = qry.stride_B[ePlane_1st],
                    .mStrideInByte[ePlane_2nd] = qry.stride_B[ePlane_2nd],
                    .mStrideInByte[ePlane_3rd] = qry.stride_B[ePlane_3rd]
                };

                lHwPortCfg[sidx].push_back(full);
            }
        }

        if (pUt[sidx]->m_enablePort & __YUVO_ENABLE) {
            NormalPipe_QueryInfo qry;

            if(arg->qryFunc.QueryInput.vOutData.at(sidx).function.Bits.isFull_dir_YUV || pUt[sidx]->mbDynSwitch){
                MY_LOGD("0x%x: port YUVO", arg->TG_Num);

                if( (fmt = getExtentFmt(PORT_YUVO,yuv_plane)) == NSCam::eImgFmt_IMPLEMENTATION_DEFINED){
                    MY_LOGE("get yuv fmt error");
                    return NULL;
                }
                MY_LOGD("YUVO fmt:0x%x",fmt);
                getNormalPipeModule()->query(PORT_YUVO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                    fmt, pUt[sidx]->mTgSize.w, qry);

                //for easy coding, use full frame here, random crop when enque
                _cropsize.s = pUt[sidx]->mTgSize;
                _cropsize.p.x = _cropsize.p.y = 0;


                MY_LOGD("yuvo query x_pix:(%d) x_strid_byte:(%d/%d/%d) sensor size:(%d_%d) crop:(%d_%d),(%d_%d) dest:(%d_%d)",
                    qry.x_pix, qry.stride_B[ePlane_1st], qry.stride_B[ePlane_2nd], qry.stride_B[ePlane_3rd],
                    pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h, _cropsize.p.x, _cropsize.p.y,_cropsize.s.w,
                    _cropsize.s.h, _cropsize.s.w, _cropsize.s.h);
                {
                    HwPortConfig_t full = {
                        .mPortID  = PORT_YUVO,
                        .mFmt     = fmt,
                        .mSize    = _cropsize.s,
                        .mCrop    = _cropsize,
                        .mPureRaw = 0, //dont care
                        .mStrideInByte[ePlane_1st] = qry.stride_B[ePlane_1st],
                        .mStrideInByte[ePlane_2nd] = qry.stride_B[ePlane_2nd],
                        .mStrideInByte[ePlane_3rd] = qry.stride_B[ePlane_3rd]
                    };

                    lHwPortCfg[sidx].push_back(full);
                }
            }
            else{
                MY_LOGE("YUVO is not supported at sensor_id:%d\n",sidx);
                pUt[sidx]->m_enablePort  &= ~(__YUVO_ENABLE);
            }
        }
        if (pUt[sidx]->m_enablePort & __CRZO_ENABLE) {
            NormalPipe_QueryInfo qry;

            if(arg->qryFunc.QueryInput.vOutData.at(sidx).function.Bits.isG_Scaled_YUV || pUt[sidx]->mbDynSwitch){
                MY_LOGD("0x%x: port CRZO_R1", arg->TG_Num);

                if( (fmt = getExtentFmt(PORT_CRZO_R1,crz_plane)) == NSCam::eImgFmt_IMPLEMENTATION_DEFINED){
                    MY_LOGE("get crz fmt error");
                    return NULL;
                }

                getNormalPipeModule()->query(PORT_CRZO_R1.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                    fmt, pUt[sidx]->mTgSize.w, qry);


                _cropsize.s = pUt[sidx]->mTgSize;
                _cropsize.p.x = _cropsize.p.y = 0;
                _destsize.w = qry.x_pix;
                _destsize.h = _cropsize.s.h;
                if(qry.x_pix != pUt[sidx]->mTgSize.w){//crz have max output size
                    pUt[sidx]->mCrzMaxSize.w = qry.x_pix;
                }
                else
                    pUt[sidx]->mCrzMaxSize.w = pUt[sidx]->mTgSize.w;
                pUt[sidx]->mCrzMaxSize.h = _cropsize.s.h;

                MY_LOGD("crzo query x_pix:(%d) x_strid_byte:(%d/%d/%d) sensor size:(%d_%d) crop:(%d_%d),(%d_%d) dest:(%d_%d)",
                    qry.x_pix, qry.stride_B[ePlane_1st], qry.stride_B[ePlane_2nd], qry.stride_B[ePlane_3rd],
                    pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h, _cropsize.p.x, _cropsize.p.y,_cropsize.s.w,
                    _cropsize.s.h, _cropsize.s.w, _cropsize.s.h);
                {
                    HwPortConfig_t resized = {
                        .mPortID  = PORT_CRZO_R1,
                        .mFmt     = fmt,
                        .mSize    = _destsize,
                        .mCrop    = _cropsize,
                        .mPureRaw = 0, //dont care
                        .mStrideInByte[ePlane_1st] = qry.stride_B[ePlane_1st],
                        .mStrideInByte[ePlane_2nd] = qry.stride_B[ePlane_2nd],
                        .mStrideInByte[ePlane_3rd] = qry.stride_B[ePlane_3rd]
                    };

                    lHwPortCfg[sidx].push_back(resized);
                }
            }
            else{
                MY_LOGE("crzO is not supported at sensor_id:%d\n",sidx);
                pUt[sidx]->m_enablePort  &= ~(__CRZO_ENABLE);
            }
        }
        if (pUt[sidx]->m_enablePort & __CRZO_R2_ENABLE) {
            NormalPipe_QueryInfo qry;

            if(arg->qryFunc.QueryInput.vOutData.at(sidx).function.Bits.isScaled_YUV || pUt[sidx]->mbDynSwitch){
                MY_LOGD("0x%x: port CRZO_R2", arg->TG_Num);

                if( (fmt = getExtentFmt(PORT_CRZO_R2,crz_r2_plane)) == NSCam::eImgFmt_IMPLEMENTATION_DEFINED){
                    MY_LOGE("get crz_R2 fmt error");
                    return NULL;
                }

                getNormalPipeModule()->query(PORT_CRZO_R2.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                    fmt, pUt[sidx]->mTgSize.w, qry);


                _cropsize.s = pUt[sidx]->mTgSize;
                _cropsize.p.x = _cropsize.p.y = 0;
                _destsize.w = qry.x_pix;
                _destsize.h = _cropsize.s.h;
                if(qry.x_pix != pUt[sidx]->mTgSize.w){//crz have max output size
                    pUt[sidx]->mCrzR2MaxSize.w = qry.x_pix;
                }
                else
                    pUt[sidx]->mCrzR2MaxSize.w = pUt[sidx]->mTgSize.w;
                pUt[sidx]->mCrzR2MaxSize.h = _cropsize.s.h;

                MY_LOGD("CRZO_R2 query x_pix:(%d) x_strid_byte:(%d/%d/%d) sensor size:(%d_%d) crop:(%d_%d),(%d_%d) dest:(%d_%d)",
                    qry.x_pix, qry.stride_B[ePlane_1st], qry.stride_B[ePlane_2nd], qry.stride_B[ePlane_3rd],
                    pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h, _cropsize.p.x, _cropsize.p.y,_cropsize.s.w,
                    _cropsize.s.h, _cropsize.s.w, _cropsize.s.h);
                {
                    HwPortConfig_t resized = {
                        .mPortID  = PORT_CRZO_R2,
                        .mFmt     = fmt,
                        .mSize    = _destsize,
                        .mCrop    = _cropsize,
                        .mPureRaw = 0, //dont care
                        .mStrideInByte[ePlane_1st] = qry.stride_B[ePlane_1st],
                        .mStrideInByte[ePlane_2nd] = qry.stride_B[ePlane_2nd],
                        .mStrideInByte[ePlane_3rd] = qry.stride_B[ePlane_3rd]
                    };

                    lHwPortCfg[sidx].push_back(resized);
                }
            }
            else{
                MY_LOGE("CRZO_R2 is not supported at sensor_id:%d\n",sidx);
                pUt[sidx]->m_enablePort  &= ~(__CRZO_R2_ENABLE);
            }
        }

        if (pUt[sidx]->m_enablePort & __SV_DCIF_IMGO_ENABLE) {
            NormalPipe_QueryInfo qry;

            if(pUt[sidx]->m_sv_ufeo_en == 1){
                if (!getOutputFmt(PORT_CAMSV_IMGO, getPortBitDepth(PORT_CAMSV_IMGO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt, MTRUE)) {
                    MY_LOGE("get pix fmt error");
                    return NULL;
                }
            }
            else{
                if (!getOutputFmt(PORT_CAMSV_IMGO, getPortBitDepth(PORT_CAMSV_IMGO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt, MFALSE)) {
                    MY_LOGE("get pix fmt error");
                    return NULL;
                }
            }

            MY_LOGD("0x%x: port SV_IMGO", arg->TG_Num);

            getNormalPipeModule()->query(PORT_CAMSV_IMGO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                fmt, pUt[sidx]->mTgSize.w, qry);

            //set/get crop info
            if(croptestsel == CROP_TEST::CROP_SPECIFIC){
                pUt[sidx]->m_crop.SetInfo(PORT_CAMSV_IMGO,fmt,arg->custCropSize);
            }
            pUt[sidx]->m_crop.GetCropInfo(PORT_CAMSV_IMGO,fmt,_cropsize,MTRUE);


            MY_LOGD("sv_imgo query x_pix:(%d) x_strid_byte:(%d/%d/%d) sensor size:(%d_%d) crop:(%d_%d),(%d_%d) dest:(%d_%d)",
                qry.x_pix, qry.stride_B[ePlane_1st], qry.stride_B[ePlane_2nd], qry.stride_B[ePlane_3rd],
                pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h, _cropsize.p.x, _cropsize.p.y,_cropsize.s.w,
                _cropsize.s.h, _cropsize.s.w, _cropsize.s.h);
            {
                HwPortConfig_t full = {
                    .mPortID  = PORT_CAMSV_IMGO,
                    .mFmt     = fmt,
                    .mSize    = _cropsize.s,
                    .mCrop    = _cropsize,
                    .mPureRaw = 1, // only pure raw supported at camsv channel
                    .mStrideInByte[ePlane_1st] = qry.stride_B[ePlane_1st],
                    .mStrideInByte[ePlane_2nd] = qry.stride_B[ePlane_2nd],
                    .mStrideInByte[ePlane_3rd] = qry.stride_B[ePlane_3rd]
                };

                lHwPortCfg[sidx].push_back(full);
            }
        }

        if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
            NormalPipe_QueryInfo qry;

            if(pUt[sidx]->m_ufgo_en == 1){
                if (!getOutputFmt(PORT_RRZO, getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt, MTRUE)) {
                    MY_LOGE("get pix fmt error");
                    return NULL;
                }
            }
            else{
                if (!getOutputFmt(PORT_RRZO, getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt, MFALSE)) {
                    MY_LOGE("get pix fmt error");
                    return NULL;
                }
            }

            //set/get crop info
            if(croptestsel == CROP_TEST::CROP_SPECIFIC){
                pUt[sidx]->m_crop.SetInfo(PORT_RRZO,fmt,arg->custCropSize);
            }
            pUt[sidx]->m_crop.GetCropInfo(PORT_RRZO,fmt,_cropsize);
            pUt[sidx]->m_crop.GetScaleInfo(PORT_RRZO,fmt,_destsize,pUt[sidx]->mScaleSel,pUt[sidx]->mScaleFactor);

            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_STRIDE_BYTE, fmt, _cropsize.s.w, qry);
            {
                HwPortConfig_t resized = {
                    .mPortID  = PORT_RRZO,
                    .mFmt     = fmt,
                    .mSize    = _destsize,
                    .mCrop    = _cropsize,
                    .mPureRaw = 0, //if raw type != 1 -> pure-raw
                    .mStrideInByte[ePlane_1st] = qry.stride_B[ePlane_1st],
                    .mStrideInByte[ePlane_2nd] = qry.stride_B[ePlane_2nd],
                    .mStrideInByte[ePlane_3rd] = qry.stride_B[ePlane_3rd]
                };

                lHwPortCfg[sidx].push_back(resized);
            }

            MY_LOGD("rrzo query x_pix:(%d) x_strid_byte:(%d/%d/%d) scale_size:(%d_%d) crop size:(%d_%d),(%d_%d)",
                qry.x_pix, qry.stride_B[ePlane_1st], qry.stride_B[ePlane_2nd], qry.stride_B[ePlane_3rd],
                _destsize.w, _destsize.h, _cropsize.p.x, _cropsize.p.y,_cropsize.s.w, _cropsize.s.h);

        }

        if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
            MSize _ssize = pUt[sidx]->mEisNotify.queryEisOutSize();
            HwPortConfig_t eiso_cfg = {
                .mPortID  = PORT_EISO,
                .mFmt     = eImgFmt_BLOB,
                .mSize    = _ssize,
                .mCrop    = MRect(MPoint(0, 0), _ssize),
                .mPureRaw = 0,
                .mStrideInByte[ePlane_1st] = (MUINT32)_ssize.w,
                .mStrideInByte[ePlane_2nd] = 0,
                .mStrideInByte[ePlane_3rd] = 0
            };

            if(pUt[sidx]->mbDynSwitch){
                lHwPortCfg[sidx].push_back(eiso_cfg);
            }
            else{
                if(arg->qryFunc.QueryOutput){
                    if(arg->qryFunc.QueryInput.vOutData.at(sidx).function.Bits.isLMV){
                        MY_LOGD("0x%x: port EISO", arg->TG_Num);
                        lHwPortCfg[sidx].push_back(eiso_cfg);
                    }
                    else{
                        MY_LOGE("EIS is not supported at sensor_id:%d\n",sidx);
                        pUt[sidx]->m_enablePort  &= ~(__EISO_ENABLE);
                    }
                }
            }
        }

        if(pUt[sidx]->m_enablePort & __RSSO_ENABLE) {
            MSize _ssize = pUt[sidx]->mRssNotify.queryRssOutSize();
            HwPortConfig_t rsso_cfg = {
                .mPortID  = PORT_RSSO,
                .mFmt     = eImgFmt_BLOB,
                .mSize    = _ssize,
                .mCrop    = MRect(MPoint(0, 0), _ssize),
                .mPureRaw = 0,
                .mStrideInByte[ePlane_1st] = (MUINT32)_ssize.w,
                .mStrideInByte[ePlane_2nd] = 0,
                .mStrideInByte[ePlane_3rd] = 0
            };

            if(pUt[sidx]->mbDynSwitch){
                lHwPortCfg[sidx].push_back(rsso_cfg);
            }
            else{
                if(arg->qryFunc.QueryOutput){
                    if(arg->qryFunc.QueryInput.vOutData.at(sidx).function.Bits.isRSS){
                        MY_LOGD("0x%x: port RSSO", arg->TG_Num);
                        lHwPortCfg[sidx].push_back(rsso_cfg);
                    }
                    else{
                        MY_LOGE("RSS is not supported at sensor_id:%d\n",sidx);
                        pUt[sidx]->m_enablePort  &= ~(__RSSO_ENABLE);
                    }
                }
            }
        }

        if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
            MSize _ssize = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize);
            HwPortConfig_t lcso_cfg = {
                .mPortID  = PORT_LCSO,
                .mFmt     = eImgFmt_BLOB,
                .mSize    = _ssize,
                .mCrop    = MRect(MPoint(0, 0), _ssize),
                .mPureRaw = 0,
                .mStrideInByte[ePlane_1st] = (MUINT32)_ssize.w,
                .mStrideInByte[ePlane_2nd] = 0,
                .mStrideInByte[ePlane_3rd] = 0
            };

            MY_LOGD("0x%x: port LCSO, size_%d_%d", sidx, _ssize.w, _ssize.h);

            lHwPortCfg[sidx].push_back(lcso_cfg);
        }

        MY_LOGD("%d hwport size:%d", arg->TG_Num, (MUINT32)lHwPortCfg[sidx].size());
    }

    /*
     * NormalPipe configPipe
     */
    MY_LOGD("configPipe...");

    vector<portInfo> vPortInfo[2];
    list<HwPortConfig_t>::const_iterator pPortCfg;

    for (sidx = 0; sidx < sensorNum; sidx++) {
        NormalPipe_QueryInfo qry;

        for (pPortCfg = lHwPortCfg[sidx].begin(); pPortCfg != lHwPortCfg[sidx].end(); pPortCfg++) {
            portInfo OutPort(
                    pPortCfg->mPortID,
                    pPortCfg->mFmt,
                    pPortCfg->mSize,
                    pPortCfg->mCrop,
                    pPortCfg->mStrideInByte[ePlane_1st],
                    pPortCfg->mStrideInByte[ePlane_2nd],
                    pPortCfg->mStrideInByte[ePlane_3rd],
                    pPortCfg->mPureRaw,
                    MTRUE
                    );
            if ((pPortCfg->mPortID == PORT_IMGO) || (pPortCfg->mPortID == PORT_RRZO)) {
                if(SecEnable)
                    OutPort.mSecon = 1;
            }
            vPortInfo[sidx].push_back(OutPort);

            MY_LOGD("configPipe: 0x%x id:0x%x, crop:%d,%d,%dx%d, size:%dx%d, fmt:0x%x, stride:%d/%d/%d, pureraw:%d",\
                    arg->TG_Num,\
                    pPortCfg->mPortID.index,\
                    pPortCfg->mCrop.p.x,pPortCfg->mCrop.p.y,pPortCfg->mCrop.s.w,pPortCfg->mCrop.s.h,\
                    pPortCfg->mSize.w,pPortCfg->mSize.h,\
                    pPortCfg->mFmt,\
                    pPortCfg->mStrideInByte[ePlane_1st],\
                    pPortCfg->mStrideInByte[ePlane_2nd],\
                    pPortCfg->mStrideInByte[ePlane_3rd],\
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
        (sensorDev & UT_SNR_MASK)?0:1, // 0: real sensor 1: test model
        SEN_PIX_BITDEPTH,
        vSensorCfg[sidx],
        vPortInfo[sidx],
        MTRUE,
        stN3D); // N3D

        param.m_bOffBin = forceBinOff; // force bin off
        param.m_Func.Bits.DATA_PATTERN = data_pattern;
        param.m_Func.Bits.SVRawBurstNum = (pUt[sidx]->m_bDCIF)?_dcif_burstQNum:0;
        param.m_CamResConfig.Bits.targetTG = arg->targetTG;
        param.m_IQlv = arg->iqLevel;

        if(bEnDTwin){
            if(getNormalPipeModule()->query(0, ENPipeQueryCmd_D_Twin, fmt, 0, qry)){
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
        }
        else{
            param.m_DynamicTwin = MFALSE;
        }

        if(getNormalPipeModule()->query(0, ENPipeQueryCmd_NEED_CFG_SENSOR_BY_MW, fmt, 0, qry)){
            needConfigSensorByMW = qry.function.Bits.needCfgSensorByMW;
            MY_LOGI("Query needConfigSensorByMW(%d)\n", needConfigSensorByMW);
        } else {
            needConfigSensorByMW = 0;
            MY_LOGE("unsupported cmd\n");
        }
        if (needConfigSensorByMW) {
            if(pSensorHalObj[sidx]->configure(1, &vSensorCfg[sidx].at(0)) == MFALSE) {
                MY_LOGE("fail when configure sensor driver\n");
                return NULL;
            }
        }

        switch((SWITCH_CAM_ENUM)_switchCam){
            case ONE_SENSOR:
            case ONESENSOR_NORMALSV:
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
            case NONSTOP_RANDOM_SWITCH_BY_SUSPEND:
            case IQ_LEVEL_TEST:
            case THREE_SENSOR:
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

        if (SecEnable) {
            IMEM_BUF_INFO SecChkBuf;
            SecChkBuf.size = 0x10;
            if(pUt[sidx]->mpImemDrv->allocSecBuf(&SecChkBuf) < 0){
                MY_LOGE("error:secure chk buf alloc fail");
            }

            param.m_SecureCam.buf_type = E_SEC_LEVEL_PROTECTED; //secure mem type:MTEE
            param.m_SecureCam.chk_handle = SecChkBuf.phyAddr;
        }

        if (!(pUt[sidx]->mbConfig = pUt[sidx]->mpNPipe->configPipe(param, pUt[sidx]->mBurstQNum))) {
            MY_LOGE("senidx:0x%x configPipe failed", arg->TG_Num);
            return NULL;
        }

        #if (TEST_ZHDR_HW == 1)
        if (1)
        #else
        if (pUt[sidx]->mEnableSttPort)
        #endif
        {
            pUt[sidx]->tuningMgr = (TuningMgr*)TuningMgr::getInstance(pUt[sidx]->mSensorIdx);
            if (MFALSE == pUt[sidx]->tuningMgr->init(mThisName, pUt[sidx]->mBurstTuningQNum)) {
                MY_LOGE("tuningMgr init error");
                return NULL;
            }
        }

        if (pUt[sidx]->mEnableSttPort) {
            MUINT32 sdma = 0;
            std::vector<statPortInfo> vp;
            QInitStatParam statParm(vp);

            if(pUt[sidx]->mbDynSwitch){
                statParm.m_StatFunc.Bits.D_CHG_SRC = 1;
            }
            else{
                statParm.m_StatFunc.Bits.D_CHG_SRC = 0;
            }

            pUt[sidx]->mpSttPipe = IStatisticPipe::createInstance(pUt[sidx]->mSensorIdx, mThisName);
            if (MFALSE == pUt[sidx]->mpSttPipe->init()) {
                MY_LOGE("statisticPipe init error");
                return NULL;
            }

            if (pUt[sidx]->mEnableSttPort & __AAO_ENABLE) {
                pUt[sidx]->mSttSofUserKey[__AAO] = pUt[sidx]->mpNPipe->attach("SttDeqLoopAAO");
                if (pUt[sidx]->mSttSofUserKey[__AAO] < 0) {
                    MY_LOGE("Wrong userkey : %d", pUt[sidx]->mSttSofUserKey[__AAO]);
                    return NULL;
                }
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_AAO));
            }
            if (pUt[sidx]->mEnableSttPort & __AFO_ENABLE) {
                pUt[sidx]->mSttSofUserKey[__AFO] = pUt[sidx]->mpNPipe->attach("SttDeqLoopAFO");
                if (pUt[sidx]->mSttSofUserKey[__AFO] < 0) {
                    MY_LOGE("Wrong userkey : %d", pUt[sidx]->mSttSofUserKey[__AFO]);
                    return NULL;
                }
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_AFO));
            }
            if (pUt[sidx]->mEnableSttPort & __FLKO_ENABLE) {
                pUt[sidx]->mSttSofUserKey[__FLKO] = pUt[sidx]->mpNPipe->attach("SttDeqLoopFLKO");
                if (pUt[sidx]->mSttSofUserKey[__FLKO] < 0) {
                    MY_LOGE("Wrong userkey : %d", pUt[sidx]->mSttSofUserKey[__FLKO]);
                    return NULL;
                }
                if(pUt[sidx]->mbDynSwitch){
                    statParm.mStatPortInfo.push_back(statPortInfo(PORT_FLKO));
                }
                else{
                    if(arg->qryFunc.QueryOutput){
                        if(arg->qryFunc.QueryInput.vOutData.at(sidx).function.Bits.isFLK){
                            statParm.mStatPortInfo.push_back(statPortInfo(PORT_FLKO));
                        }
                        else{
                            MY_LOGE("FLK is not supported at sensor_id:%d\n",sidx);
                            pUt[sidx]->mEnableSttPort  &= ~(__FLKO_ENABLE);
                        }
                    }
                }
            }
            if (pUt[sidx]->mEnableSttPort & __PDO_ENABLE) {
                //use full TG size here is just easiest example,
                //theoratically, PDO size must be smaller than TG size,
                //e.g.: PDO width = 1/8 TG_W,  PDO height = 1/4 TG_H
                pUt[sidx]->mSttSofUserKey[__PDO] = pUt[sidx]->mpNPipe->attach("SttDeqLoopPDO");
                if (pUt[sidx]->mSttSofUserKey[__PDO] < 0) {
                    MY_LOGE("Wrong userkey : %d", pUt[sidx]->mSttSofUserKey[__PDO]);
                    return NULL;
                }
                if(pUt[sidx]->m_Dualpd.Bits.bDual){
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM cur_cam;
                    pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_CUR_RAW,(MINTPTR)&cur_cam,NULL,NULL);
                    if(cur_cam == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C){
                        pUt[sidx]->mEnableSttPort &= ~(__PDO_ENABLE);
                        MY_LOGE("dualPD with PDO is not supported at CAM_C(senidx:%d)\n",sidx);
                    }
                    else
                        statParm.mStatPortInfo.push_back(statPortInfo(PORT_PDO, pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h, eCAM_DUAL_PIX, pUt[sidx]->m_Dualpd.Bits.Density));
                }
                else{
#if 1 // for w+t switch test, allow user config only one native pdo
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM cur_cam;
                    pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_CUR_RAW,(MINTPTR)&cur_cam,NULL,NULL);
                    if(cur_cam == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C){
                        pUt[sidx]->mEnableSttPort &= ~(__PDO_ENABLE);
                    }
                    else
                        statParm.mStatPortInfo.push_back(statPortInfo(PORT_PDO, pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h));
#else
                    statParm.mStatPortInfo.push_back(statPortInfo(PORT_PDO, pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h));
#endif
                }
            }
            if (pUt[sidx]->mEnableSttPort & __TSFSO_ENABLE) {
                pUt[sidx]->mSttSofUserKey[__TSFSO] = pUt[sidx]->mpNPipe->attach("SttDeqLoopTSFSO");
                if (pUt[sidx]->mSttSofUserKey[__TSFSO] < 0) {
                    MY_LOGE("Wrong userkey : %d", pUt[sidx]->mSttSofUserKey[__TSFSO]);
                    return NULL;
                }
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_TSFSO));
            }
            MY_LOGD("stt configPipe");
            pUt[sidx]->mpSttPipe->configPipe(statParm);
        }

        if (pUt[sidx]->mbCamsvEn) {
            MBOOL bRet = MFALSE;
            MUINT32 argu1 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
            std::vector<statPortInfo> vp;
            QInitStatParam statParm(vp);
            struct SensorVCInfo2 VcInfo;

            int f;

            do {
                MY_LOGD("#############################################################");
                MY_LOGD("# Please select 1 feature: PDAF(0)/mVHDR(1)/4-Cell mVHDR(2) #");
                MY_LOGD("#############################################################");
                f = getchar();
                if (f == '0' || f =='1' || f == '2') {
                    f = f - '0';
                    break;
                }
            } while(1);

            if(f == 0){
                pUt[sidx]->m_vCamsvFeatureIdx.clear();
                pUt[sidx]->m_vCamsvFeatureIdx.push_back(VC_PDAF_STATS);
            }else if(f == 1){
                pUt[sidx]->m_vCamsvFeatureIdx.clear();
                pUt[sidx]->m_vCamsvFeatureIdx.push_back(VC_HDR_MVHDR);
            }
            else if(f == 2){ //4-cell mVHDR needs to use multiple camsv channel
                pUt[sidx]->m_vCamsvFeatureIdx.clear();
                pSensorHalObj[sidx]->sendCommand(
                    pHalSensorList->querySensorDevIdx(pUt[sidx]->mSensorIdx), SENSOR_CMD_GET_SENSOR_VC_INFO2, (MUINTPTR)&VcInfo, (MINTPTR)&argu1, 0);
                for(i = VC_3HDR_MIN_NUM ; i < VC_3HDR_MAX_NUM ; i++){
                    for(j = 0 ; j < VcInfo.vcInfo2s.size() ; j++){
                        if(i == VcInfo.vcInfo2s[j].VC_FEATURE){
                            pUt[sidx]->m_vCamsvFeatureIdx.push_back(i);
                        }
                    }
            }
            }

            MY_LOGD("used camsv channel size:%d", pUt[sidx]->m_vCamsvFeatureIdx.size());

            for(i = 0 ; i < pUt[sidx]->m_vCamsvFeatureIdx.size() ; i++){
                //
                fidx = pUt[sidx]->m_vCamsvFeatureIdx[i];
                //
                pUt[sidx]->mpCamsvSttPipe[fidx] = ICamsvStatisticPipe::createInstance(pUt[sidx]->mSensorIdx, mThisName , fidx);
                if (!pUt[sidx]->mpCamsvSttPipe[fidx]) {
                    MY_LOGD("ICamsvStatisticPipe createInstance fail %d", fidx);
                }

                //
                if (pUt[sidx]->mpCamsvSttPipe[fidx]->init() == MFALSE) {
                    MY_LOGD("ICamsvStatisticPipe init fail %d", fidx);
                }

                // set camsv scenario
                bRet = pUt[sidx]->mpCamsvSttPipe[fidx]->sendCommand(ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID, (MINTPTR)&argu1, NULL,NULL);

                //
                pUt[sidx]->mpCamsvSttPipe[fidx]->configPipe(statParm);
            }
        }

        if (pUt[sidx]->mEnableSttPort) {
            MY_LOGD("Reg Tuning CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_3A_HAL_CBFP, (MINTPTR)&pUt[sidx]->m3ACBNotify, 0, 0);
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_WT_TUNING_CBFP, (MINTPTR)&pUt[sidx]->mTuningCBNotify, 0, 0);
        }
        if (pUt[sidx]->mEnableSttPort & __AAO_ENABLE) {
            MY_LOGD("Reg AWB CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_AWB_CBFP, (MINTPTR)&pUt[sidx]->mAwbCBNotify, 0, 0);
        }
        if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
            MY_LOGD("Reg RRZO CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_RRZ_CBFP, (MINTPTR)&pUt[sidx]->mRRZCBNotify, 0, 0);
        }

        if (pUt[sidx]->m_enablePort & __CRZO_ENABLE) {
            MY_LOGD("Reg CRZO CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_CRZO_R1_CBFP, (MINTPTR)&pUt[sidx]->mCRZCBNotify, 0, 0);
        }
        if (pUt[sidx]->m_enablePort & __CRZO_R2_ENABLE) {
            MY_LOGD("Reg CRZO_R2 CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_CRZO_R2_CBFP, (MINTPTR)&pUt[sidx]->mCRZR2CBNotify, 0, 0);
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
        if (pUt[sidx]->m_enablePort & __RSSO_ENABLE) {
            MY_LOGD("Reg SGG2 CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_SGG2_CBFP, (MINTPTR)&pUt[sidx]->mSgg2Notify, 0, 0);
            MY_LOGD("Reg RSS CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_RSS_CBFP, (MINTPTR)&pUt[sidx]->mRssNotify, 0, 0);
        }
        MY_LOGD("Reg DUMP CBFP");
        pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_REGDUMP_CBFP, (MINTPTR)&pUt[sidx]->mDumpNotify, 0, 0);
        MY_LOGD("CQ Start CBFP");
        pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_CQSTART_CBFP, (MINTPTR)&pUt[sidx]->mCQStartNotify, 0, 0);
    }

    for (sidx = 0; sidx < sensorNum; sidx++) {
        if (pUt[sidx]->m_enablePort & __YUVO_ENABLE) {
            if(yuv_plane> 1){
                if (!MutiPlaneAllocator(pUt[sidx], PORT_YUVO, mSensorInfo[sidx])) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_YUVO.index);
                    return NULL;
                }
            }
            else if(yuv_plane == 1)
            {
                if (!SinglePlaneAllocator(pUt[sidx], PORT_YUVO, mSensorInfo[sidx],SecEnable)) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_YUVO.index);
                    return NULL;
                }
            }
            else{
                MY_LOGE("Port(%d) fail", PORT_YUVO.index);
                return NULL;
            }
        }
        if (pUt[sidx]->m_enablePort & __CRZO_ENABLE) {
            if(crz_plane > 1){
                if (!MutiPlaneAllocator(pUt[sidx], PORT_CRZO_R1, mSensorInfo[sidx])) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_CRZO_R1.index);
                    return NULL;
                }
            }
            else if(crz_plane == 1){
                if (!SinglePlaneAllocator(pUt[sidx], PORT_CRZO_R1, mSensorInfo[sidx],SecEnable)) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_CRZO_R1.index);
                    return NULL;
                }
            }
            else{
                MY_LOGE("Port(%d) fail", PORT_CRZO_R1.index);
                return NULL;
            }
        }
        if (pUt[sidx]->m_enablePort & __CRZO_R2_ENABLE) {
            if(crz_r2_plane > 1){
                if (!MutiPlaneAllocator(pUt[sidx], PORT_CRZO_R2, mSensorInfo[sidx])) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_CRZO_R2.index);
                    return NULL;
                }
            }
            else if(crz_r2_plane == 1){
                if (!SinglePlaneAllocator(pUt[sidx], PORT_CRZO_R2, mSensorInfo[sidx],SecEnable)) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_CRZO_R2.index);
                    return NULL;
                }
            }
            else{
                MY_LOGE("Port(%d) fail", PORT_CRZO_R2.index);
                return NULL;
            }
        }

        if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
            if(pUt[sidx]->m_ufgo_en == 1){
                if (!MutiPlaneAllocator(pUt[sidx], PORT_RRZO, mSensorInfo[sidx])) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_RRZO.index);
                    return NULL;
                }
            }
            else{
                if (!SinglePlaneAllocator(pUt[sidx], PORT_RRZO, mSensorInfo[sidx],SecEnable)) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_RRZO.index);
                    return NULL;
                }
            }
        }
        if (pUt[sidx]->m_enablePort & __RAWI_ENABLE) {
            if(pUt[sidx]->m_ufdi_en == 1){
                if (!MutiPlaneAllocator(pUt[sidx], PORT_RAWI, mSensorInfo[sidx])) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_RAWI.index);
                    return NULL;
                }
            }
            else{
                if (!SinglePlaneAllocator(pUt[sidx], PORT_RAWI, mSensorInfo[sidx],SecEnable)) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_RAWI.index);
                    return NULL;
                }
            }
        }
        if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
            if(pUt[sidx]->m_ufeo_en == 1){
                if (!MutiPlaneAllocator(pUt[sidx], PORT_IMGO, mSensorInfo[sidx])) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_IMGO.index);
                    return NULL;
                }
            }
            else{
                if (!SinglePlaneAllocator(pUt[sidx], PORT_IMGO, mSensorInfo[sidx],SecEnable)) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_IMGO.index);
                    return NULL;
                }
            }
        }
        if (pUt[sidx]->m_enablePort & __SV_DCIF_IMGO_ENABLE) {
            if(pUt[sidx]->m_sv_ufeo_en == 1){
                if (!MutiPlaneAllocator(pUt[sidx], PORT_CAMSV_IMGO, mSensorInfo[sidx])) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_CAMSV_IMGO.index);
                    return NULL;
                }
            }
            else{
                if (!SinglePlaneAllocator(pUt[sidx], PORT_CAMSV_IMGO, mSensorInfo[sidx],SecEnable)) {
                    MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_CAMSV_IMGO.index);
                    return NULL;
                }
            }
        }
        if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
            if (!SinglePlaneAllocator(pUt[sidx], PORT_EISO, mSensorInfo[sidx],SecEnable)) {
                MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_EISO.index);
                return NULL;
            }
        }
        if (pUt[sidx]->m_enablePort & __RSSO_ENABLE) {
            if (!SinglePlaneAllocator(pUt[sidx], PORT_RSSO, mSensorInfo[sidx],SecEnable)) {
                MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_RSSO.index);
                return NULL;
            }
        }
        if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
            if (!SinglePlaneAllocator(pUt[sidx], PORT_LCSO, mSensorInfo[sidx],SecEnable)) {
                MY_LOGE("Port(%d) IImageBufferAllocat fail", PORT_LCSO.index);
                return NULL;
            }
        }
        if (pUt[sidx]->mEnableSttPort & __PDO_ENABLE) {
            //allocate bpci table
            //bpci
            pUt[sidx]->m_bpciBuf.bufCohe = MFALSE;
            pUt[sidx]->m_bpciBuf.size = sizeof(bpci_array_s5k2x8);
            pUt[sidx]->mpImemDrv->allocVirtBuf(&pUt[sidx]->m_bpciBuf);
            pUt[sidx]->mpImemDrv->mapPhyAddr(&pUt[sidx]->m_bpciBuf, MFALSE);

            memcpy((char*)pUt[sidx]->m_bpciBuf.virtAddr,(char*)&bpci_array_s5k2x8[0],sizeof(bpci_array_s5k2x8));
        }
    }

    MY_LOGD("enque...");

    for (sidx = 0; sidx < sensorNum; sidx++) {
        QBufInfo    qBuf;
        MUINT32     *pMag = NULL;
        MUINT32     bin_w,bin_h,hbin_w,hbin_h;
        MSize       _size;

        pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_BIN_INFO, (MINTPTR)&bin_w, (MINTPTR)&bin_h, 0);
        _size.w = bin_w;
        _size.h = bin_h;
        //patch pipeline size after bin ctrl
        pUt[sidx]->m_crop.PatchPipeLineIn(_size);
        pUt[sidx]->mTarBINOut = _size;
        pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO, (MINTPTR)&hbin_w, (MINTPTR)&hbin_h, 0);
        _size.w = hbin_w;
        _size.h = hbin_h;
        pUt[sidx]->mTarQBNOut = _size;

        pUt[sidx]->mMagicNum = 0;
        for (pUt[sidx]->mEnqCount = 0; pUt[sidx]->mEnqCount < pUt[sidx]->mBurstQNum * pUt[sidx]->mStaticEnqCnt;
                pUt[sidx]->mEnqCount += pUt[sidx]->mBurstQNum) {
            qBuf.mvOut.clear();
            qBuf.mShutterTimeNs = 10 * 1000 * 1000 * 1000; // for test pattern use

            pMag = (MUINT32*)malloc(sizeof(MUINT32)*pUt[sidx]->mBurstQNum);

            for (j = pUt[sidx]->mEnqCount; j < (pUt[sidx]->mEnqCount + pUt[sidx]->mBurstQNum); j++) {
                if (pUt[sidx]->m_enablePort & __RAWI_ENABLE) {
                    IImageBuffer* pIBuf;

                    if(pUt[sidx]->m_ufdi_en == MTRUE){//only 1 buffer for RAWI
                        pIBuf = pUt[sidx]->mpImgBuffer[__UFDI].at(0);
                    }
                    else{
                        pIBuf = pUt[sidx]->mpImgBuffer[__RAWI].at(0);
                    }

                    _cropsize.p.x = 0;
                    _cropsize.p.y = 0;
                    _cropsize.s.w = pUt[sidx]->mTgSize.w;
                    _cropsize.s.h = pUt[sidx]->mTgSize.h;
                    _destsize.w = pUt[sidx]->mTgSize.w;
                    _destsize.h = pUt[sidx]->mTgSize.h;


                    BufInfo _buf(PORT_RAWI, pIBuf, _destsize, _cropsize, pUt[sidx]->mMagicNum, 0, 0xFFFF );
                    qBuf.mvOut.push_back(_buf);
                    MY_LOGD("RAWI max input size: %dx%d", _cropsize.s.w, _cropsize.s.h);
                }
                if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
                    //note:crop size can't < outsize
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__RRZO].at(j);

                    MY_LOGD("RRZ max input size: %dx%d", bin_w, bin_h);

                    pUt[sidx]->m_crop.GetCropInfo(PORT_RRZO,(EImageFormat)pIBuf->getImgFormat(),_cropsize);
                    pUt[sidx]->m_crop.GetScaleInfo(PORT_RRZO,(EImageFormat)pIBuf->getImgFormat(),_destsize,pUt[sidx]->mScaleSel,pUt[sidx]->mScaleFactor);

                    BufInfo _buf(PORT_RRZO, pIBuf, _destsize,_cropsize, pUt[sidx]->mMagicNum, 0, 0xFFFF);
                    if(SecEnable){
                        _buf.mSecon = 1;
                        _buf.mChk_handle = pUt[sidx]->mImemSecChkBuf[__RRZO].at(j).phyAddr;
                        MY_LOGD("RRZO size:0x%x va:0x%x pa:0x%x memID:0x%x chk:0x%llx",pUt[sidx]->mImemSecBuf[__RRZO].at(j).size,
                                pUt[sidx]->mImemSecBuf[__RRZO].at(j).virtAddr,pUt[sidx]->mImemSecBuf[__RRZO].at(j).phyAddr,
                                pUt[sidx]->mImemSecBuf[__RRZO].at(j).memID,pUt[sidx]->mImemSecChkBuf[__RRZO].at(j).phyAddr);
                    }
                    qBuf.mvOut.push_back(_buf);
                }
                if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__IMGO].at(j);
                    {
                        MSize imgo_size[2];
                        pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_IMGO_INFO,(MINTPTR)imgo_size,0,0);
                        if(imgo_size[0].w == 0){ // pure raw -> use imgo_size[1]
                            _cropsize.p.x = 0;
                            _cropsize.p.y = 0;
                            _cropsize.s.w = imgo_size[1].w;
                            _cropsize.s.h = imgo_size[1].h;
                            _destsize.w = imgo_size[1].w;
                            _destsize.h = imgo_size[1].h;
                        }
                        else { // process raw -> use imgo_size[0]
                            _cropsize.p.x = 0;
                            _cropsize.p.y = 0;
                            _cropsize.s.w = imgo_size[0].w;
                            _cropsize.s.h = imgo_size[0].h;
                            _destsize.w = imgo_size[0].w;
                            _destsize.h = imgo_size[0].h;
                        }
                    }

                    BufInfo _buf(PORT_IMGO, pIBuf, _destsize, _cropsize, pUt[sidx]->mMagicNum, 0, (pUt[sidx]->m_enablePort & __IMGO_PURE_RAW)?1:0, 0xFFFF);
                    if(SecEnable){
                        _buf.mSecon = 1;
                        _buf.mChk_handle = pUt[sidx]->mImemSecChkBuf[__IMGO].at(j).phyAddr;
                        MY_LOGD("IMGO size:0x%x va:0x%x pa:0x%x memID:0x%x chk:0x%llx",pUt[sidx]->mImemSecBuf[__IMGO].at(j).size,
                                 pUt[sidx]->mImemSecBuf[__IMGO].at(j).virtAddr,pUt[sidx]->mImemSecBuf[__IMGO].at(j).phyAddr,
                                 pUt[sidx]->mImemSecBuf[__IMGO].at(j).memID,pUt[sidx]->mImemSecChkBuf[__IMGO].at(j).phyAddr);
                    }
                    qBuf.mvOut.push_back(_buf);
                }
                if (pUt[sidx]->m_enablePort & __CRZO_ENABLE) {
                    //note:crop size can't < outsize
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__CRZO].at(j);
                    MUINT32 ran ;
                    if(pUt[sidx]->m_enablePort & __YUVO_ENABLE){//due to dm_sel is different when YUV is opened
                        _cropsize.s.w = bin_w;
                        _cropsize.s.h = bin_h;
                        _cropsize.p.x = _cropsize.p.y = 0;
                    }
                    else{
                        pUt[sidx]->m_crop.GetScaleInfo(PORT_RRZO,(EImageFormat)pUt[sidx]->mpImgBuffer[__RRZO].at(j)->getImgFormat(),_destsize,pUt[sidx]->mScaleSel,pUt[sidx]->mScaleFactor);
                        _cropsize.s.w = _destsize.w;
                        _cropsize.s.h = _destsize.h;
                        _cropsize.p.x = _cropsize.p.y = 0;
                    }
                    MY_LOGD("CRZ max input size: %dx%d", _cropsize.s.w, _cropsize.s.h);
                    ran = rand() % 10;
                    ran = (ran == 0)? (ran++):(ran);
                    _destsize.w = _cropsize.s.w / ran;
                    _destsize.w = (_destsize.w & 0x1) ? (_destsize.w - 1): (_destsize.w);
                    if(_destsize.w > pUt[sidx]->mCrzMaxSize.w)
                        _destsize.w = pUt[sidx]->mCrzMaxSize.w;

                    _destsize.h = _cropsize.s.h / ran;
                    _destsize.h = (_destsize.h & 0x1) ? (_destsize.h - 1): (_destsize.h);
                    if(_destsize.h > pUt[sidx]->mCrzMaxSize.h)
                        _destsize.h = pUt[sidx]->mCrzMaxSize.h;

                    MY_LOGD("CRZ output size: %dx%d", _destsize.w, _destsize.h);

                    BufInfo _buf(PORT_CRZO_R1, pIBuf, _destsize,_cropsize, pUt[sidx]->mMagicNum, 0, 0xFFFF);
                    qBuf.mvOut.push_back(_buf);
                }
                if (pUt[sidx]->m_enablePort & __CRZO_R2_ENABLE) {
                    //note:crop size can't < outsize
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__CRZO_R2].at(j);
                    MUINT32 ran ;
                    if(pUt[sidx]->m_enablePort & __YUVO_ENABLE){//due to dm_sel is different when YUV is opened
                        _cropsize.s.w = bin_w;
                        _cropsize.s.h = bin_h;
                        _cropsize.p.x = _cropsize.p.y = 0;
                    }
                    else{
                        pUt[sidx]->m_crop.GetScaleInfo(PORT_RRZO,(EImageFormat)pUt[sidx]->mpImgBuffer[__RRZO].at(j)->getImgFormat(),_destsize,pUt[sidx]->mScaleSel,pUt[sidx]->mScaleFactor);
                        _cropsize.s.w = _destsize.w;
                        _cropsize.s.h = _destsize.h;
                        _cropsize.p.x = _cropsize.p.y = 0;
                    }
                    MY_LOGD("CRZ_R2 max input size: %dx%d", _cropsize.s.w, _cropsize.s.h);
                    ran = rand() % 10;
                    ran = (ran == 0)? (ran++):(ran);
                    _destsize.w = _cropsize.s.w / ran;
                    _destsize.w = (_destsize.w & 0x1) ? (_destsize.w - 1): (_destsize.w);
                    if(_destsize.w > pUt[sidx]->mCrzR2MaxSize.w)
                        _destsize.w = pUt[sidx]->mCrzR2MaxSize.w;

                    _destsize.h = _cropsize.s.h / ran;
                    _destsize.h = (_destsize.h & 0x1) ? (_destsize.h - 1): (_destsize.h);
                    if(_destsize.h > pUt[sidx]->mCrzR2MaxSize.h)
                        _destsize.h = pUt[sidx]->mCrzR2MaxSize.h;

                    MY_LOGD("CRZ_R2 output size: %dx%d", _destsize.w, _destsize.h);

                    BufInfo _buf(PORT_CRZO_R2, pIBuf, _destsize,_cropsize, pUt[sidx]->mMagicNum, 0, 0xFFFF);
                    qBuf.mvOut.push_back(_buf);
                }
                if (pUt[sidx]->m_enablePort & __YUVO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__YUVO].at(j);
                    _destsize.w = _cropsize.s.w = bin_w;
                    _destsize.h = _cropsize.s.h = bin_h;
                    _cropsize.p.x = _cropsize.p.y = 0;
                    MY_LOGD("YUV max input size: %dx%d", _cropsize.s.w, _cropsize.s.h);
                    BufInfo _buf(PORT_YUVO, pIBuf, _destsize,_cropsize, pUt[sidx]->mMagicNum, 0, 0xFFFF);
                    qBuf.mvOut.push_back(_buf);
                }
                if (pUt[sidx]->m_enablePort & __SV_DCIF_IMGO_ENABLE) {
                    for(k = 0 ; k < pUt[sidx]->mDCIFBurstQNum ; k++){
                        IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__SV_IMGO].at(k + (j * pUt[sidx]->mDCIFBurstQNum));
                        {
                            _cropsize.p.x = 0;
                            _cropsize.p.y = 0;
                            _cropsize.s.w = pUt[sidx]->mTgSize.w;
                            _cropsize.s.h = pUt[sidx]->mTgSize.h;
                            _destsize.w = pUt[sidx]->mTgSize.w;
                            _destsize.h = pUt[sidx]->mTgSize.h;
                        }
                        // magic number must be identical for dcif case
                        BufInfo _buf(PORT_CAMSV_IMGO, pIBuf, _destsize, _cropsize, pUt[sidx]->mMagicNum, 0, 1, 0xFFFF);
                        qBuf.mvOut.push_back(_buf);
                    }
                }
                if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__EISO].at(j);

                    _destsize = pUt[sidx]->mEisNotify.queryEisOutSize();

                    qBuf.mvOut.push_back(BufInfo(PORT_EISO, pIBuf, _destsize, MRect(MPoint(0,0), _destsize), pUt[sidx]->mMagicNum, 0, 0xFFFF));
                }
                if (pUt[sidx]->m_enablePort & __RSSO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__RSSO].at(j);

                    _destsize = pUt[sidx]->mRssNotify.queryRssOutSize();

                    qBuf.mvOut.push_back(BufInfo(PORT_RSSO, pIBuf, _destsize, MRect(MPoint(0,0), _destsize), pUt[sidx]->mMagicNum, 0, 0xFFFF));
                }
                if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__LCSO].at(j);

                    _destsize = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize);

                    qBuf.mvOut.push_back(BufInfo(PORT_LCSO, pIBuf, _destsize, MRect(MPoint(0,0), _destsize), pUt[sidx]->mMagicNum, 0, 0xFFFF));
                }

                pMag[j-pUt[sidx]->mEnqCount] = pUt[sidx]->mMagicNum + j - pUt[sidx]->mEnqCount;
            }

            pUt[sidx]->mMagicNum += pUt[sidx]->mBurstTuningQNum;

            if (pUt[sidx]->mEnableSttPort) {
                pUt[sidx]->setTuning(pMag, TYPE_SET_TUNING, NULL, NULL);
            }

            #if (TEST_ZHDR_HW == 1)
            pUt[sidx]->setZHdrTuning(pMag);
            #endif

            free(pMag);

            if (!(pUt[sidx]->mbEnque = pUt[sidx]->mpNPipe->enque(qBuf))) {
                MY_LOGE("Enque fail...");
                return NULL;
            }
        }
    }

    MY_LOGD("ISP START");

    for (sidx = 0; sidx < sensorNum; sidx++) {

        if (pUt[sidx]->mbCamsvEn) {
            for (i = 0 ; i < pUt[sidx]->m_vCamsvFeatureIdx.size() ; i++){
                fidx = pUt[sidx]->m_vCamsvFeatureIdx[i];
                pUt[sidx]->mpCamsvSttPipe[fidx]->start();
            }
        }

        if (pUt[sidx]->mEnableSttPort) {

            if (MFALSE == pUt[sidx]->mpSttPipe->start()) {
                MY_LOGE("Start STT fail...");
                return NULL;
            }
        }

        if (MFALSE == (pUt[sidx]->mbStart = pUt[sidx]->mpNPipe->start())) {
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

    if(m_bN3DTest == MTRUE){
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

        if(m_bN3DTest == MTRUE){
            if(sidx == 0){
                pUt[0]->stopHwsyncThread();
            }
        }

        MY_LOGD("STT STOP");
        if (pUt[sidx]->mEnableSttPort) {
            pUt[sidx]->mpSttPipe->stop();

        }
        if (pUt[sidx]->mbCamsvEn) {
            for (i = 0 ; i < pUt[sidx]->m_vCamsvFeatureIdx.size() ; i++){
                fidx = pUt[sidx]->m_vCamsvFeatureIdx[i];
                pUt[sidx]->mpCamsvSttPipe[fidx]->stop();
            }
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
            for(i = 0 ; i < pUt[sidx]->m_vCamsvFeatureIdx.size() ; i++){
                fidx = pUt[sidx]->m_vCamsvFeatureIdx[i];
                if (pUt[sidx]->mCamsvBufVa[fidx] && pUt[sidx]->mCamsvBufSize[fidx]) {
#define DUMP_PATH "/data/cameradump_camio/"

                    /* dump before buffer free in destroyInstance */
                    MY_LOGD("Dump camsv[%d] buf va:0x%" PRIXPTR " size:%d", fidx, pUt[sidx]->mCamsvBufVa[fidx], pUt[sidx]->mCamsvBufSize[fidx]);

                    if (!makePath(DUMP_PATH, 0660)) {
                        MY_LOGE("error: makePath fail");
                        return NULL;
                    }
                    else {
                        sprintf(filename, DUMP_PATH "%d_camsv_%d.raw", sidx, fidx);
                        saveBuf(filename, (MUINT8*)pUt[sidx]->mCamsvBufVa[fidx], pUt[sidx]->mCamsvBufSize[fidx]);
                    }
                }

                pUt[sidx]->mpCamsvSttPipe[fidx]->uninit();
                pUt[sidx]->mpCamsvSttPipe[fidx]->destroyInstance(mThisName);
            }
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

    //only for normal camera
    if (!SecEnable) {
        for (sidx = 0; sidx < sensorNum; sidx++) { //Save File
            for (j = 0; j < 1/*mPortBufDepth*/; j++) {
                for (dma = 0; dma < __MEM_MAX; dma++) {
                    if (portMap(pUt[sidx]->m_enablePort,dma) == MTRUE) {
                        MY_LOGD("enablePort:0x%x dma:%d",pUt[sidx]->m_enablePort,dma);
                        onDumpBuffer("camio", sidx, dma, (MUINTPTR)pUt[sidx]->mpImgBuffer[dma].at(j), j);
#if 0
                        if((pUt[sidx]->m_ufgo_en == 1) & (dma == __RRZO)){
                            onDumpBuffer("camio", sidx, __UFGO, (MUINTPTR)pUt[sidx]->mpImgBuffer[__UFGO].at(j), j);
                        }
                        if((pUt[sidx]->m_ufeo_en == 1) & (dma == __IMGO)){
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
    }

    MY_LOGD("Clean up");
    for (sidx = 0; sidx < sensorNum; sidx++) {
        vSensorCfg[sidx].clear();
        lHwPortCfg[sidx].clear();
        for (dma = 0; dma < __MEM_MAX; dma++) {
            if (pUt[sidx]->m_enablePort & (1 << dma)) {
                for (j = 0; j < pUt[sidx]->mImemBuf[dma].size(); j++) {
                    MY_LOGD("Freeing VA: %p", (void*)pUt[sidx]->mImemBuf[dma].at(j).virtAddr);
                    pUt[sidx]->mpImemDrv->freeVirtBuf(&pUt[sidx]->mImemBuf[dma].at(j), MFALSE);
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
            pUt[sidx]->mpImemDrv->freeVirtBuf(&pUt[sidx]->m_bpciBuf, MFALSE);

        pUt[sidx]->mpImemDrv->uninit();
        pUt[sidx]->mpImemDrv->destroyInstance();

        MY_LOGD("[%d] destroy...", pUt[sidx]->mSensorIdx);
        pUt[sidx]->destroy();

        if ((SWITCH_CAM_ENUM)_switchCam == SWITCH_AFTER_UNINIT ) {
            pthread_join(_switchThd, NULL);
        }
    }

    if(_efuse && _efuse_idx > 0) {
        _efuse_idx --;

        MY_LOGD("search sensor... +");
        // sensor number less or equal to zero means that searchSensors() not called yet
        if(pHalSensorList->queryNumberOfSensors() <= 0){
            pHalSensorList->searchSensors();
        }
        MY_LOGD("search sensor... -");

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

        strlcpy((*dst)[i], (*src)[i], sizeof(char) * (nstr+1));
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
            the_argv[i] = NULL;
        }
    }
    free(the_argv);
    the_argv = NULL;
}

void WaitSttEnqBack(_arg *pCam)
{
    MUINT32 cnt = 0;
    MUINT32 enqCnt = 0, deqCnt = 0;

    while(cnt++ < 3000){
        pCam->Ut_obj->mSttCntMutex.lock();
        enqCnt = pCam->Ut_obj->mSttEnqCnt;
        deqCnt = pCam->Ut_obj->mSttDeqCnt;
        pCam->Ut_obj->mSttCntMutex.unlock();

        if(deqCnt == enqCnt){
            MY_LOGD("Can wait for sttproc to enque back after deque");
            return;
        }

        usleep(3000);
    }

    MY_LOGE("Can't wait for sttproc to enque back after deque");
    return;
}

MBOOL SwitchImageQuality(MUINT32 caseNo, _arg *pCam1, _arg *pCam2)
{
    MBOOL ret = MTRUE;
    MUINT32 cnt = 0;

    switch(caseNo % 4){
        case 0: SetImageQuality(*pCam1, *pCam2, eCamIQ_H, eCamIQ_L); break;
        case 1: SetImageQuality(*pCam1, *pCam2, eCamIQ_L, eCamIQ_H); break;
        case 2: SetImageQuality(*pCam1, *pCam2, eCamIQ_L, eCamIQ_L); break;
        case 3: SetImageQuality(*pCam1, *pCam2, eCamIQ_H, eCamIQ_H); break;
        default: break;
    }

    cnt = 0;
    while(cnt++ < 1000){
        if(pCam1->Ut_obj->mbIQNotify == MTRUE &&
           pCam2->Ut_obj->mbIQNotify == MTRUE){
            break;
        }

        //check enq/deq stop
        if((pCam1->Ut_obj->mLoopCount > pCam1->Ut_obj->mFrameNum) &&
           (pCam2->Ut_obj->mLoopCount > pCam2->Ut_obj->mFrameNum)){
            MY_LOGD("[%s] enq/deq stop, break wait IQ Callback", __func__);
            break;
        }

        usleep(3000);
    }
    if(cnt >= 1000){
        MY_LOGE("[%s] Can't wait IQ Callback", __func__);
        ret = MFALSE;
        goto Exit;
    }

Exit:
    return ret;
}

MBOOL SwitchCamState(enum SWITCH_CAM_STATE_ENUM caseNo, _arg *pCam1, _arg *pCam2, MBOOL bRanIQ)
{
    MBOOL ret = MTRUE;
    MUINT32 ran = 0, cnt = 0;

    /* set quality */
    if(bRanIQ){
        ret = SwitchImageQuality(rand() % 4, pCam1, pCam2);
        if(ret == MFALSE){
            MY_LOGE("[%s] Can't wait IQ Callback", __func__);
            goto Exit;
        }
    }

    switch(caseNo){
    case SWITCH_CASE_1:
        /* pCam1 start suspend */
        pCam1->Ut_obj->mSuspendStart = MTRUE;

        /* wait pCam1 suspend ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam1->Ut_obj->mSuspendStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);

        // pCam2 start twin
        if(CamPathCFG(*pCam2, ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }
        if(CamPathCFG(*pCam2, ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);
        break;
    case SWITCH_CASE_2:
        /* pCam1 & pCam2 start suspend */
        pCam1->Ut_obj->mSuspendStart = MTRUE;
        pCam2->Ut_obj->mSuspendStart = MTRUE;

        /* wait pCam1 suspend ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam1->Ut_obj->mSuspendStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* wait pCam2 suspend ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam2->Ut_obj->mSuspendStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);
        break;
    case SWITCH_CASE_3:
        /* pCam1 stop twin */
        if(CamPathCFG(*pCam1, ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }
        if(CamPathCFG(*pCam1, ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);

        /* pCam2 start resume */
        WaitSttEnqBack(pCam2);
        pCam2->Ut_obj->mResumeStart= MTRUE;

        /* wait pCam2 resume ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam2->Ut_obj->mResumeStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);
        break;
    case SWITCH_CASE_4:
        /* pCam1 start suspend */
        pCam1->Ut_obj->mSuspendStart = MTRUE;

        /* wait pCam1 suspend ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam1->Ut_obj->mSuspendStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);

        /* pCam2 start twin */
#if 0 // wait to confirm the using timing for this cmd
        if(CamPathCFG(*pCam2, ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }
        if(CamPathCFG(*pCam2, ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }
#endif

        /* pCam2 start resume */
        WaitSttEnqBack(pCam2);
        pCam2->Ut_obj->mResumeStart= MTRUE;

        /* wait pCam2 resume ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam2->Ut_obj->mResumeStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);
        break;
    case SWITCH_CASE_5:
        /* pCam1 start suspend */
        pCam1->Ut_obj->mSuspendStart = MTRUE;

        /* wait pCam1 suspend ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam1->Ut_obj->mSuspendStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);
        break;
    case SWITCH_CASE_6:
        /* pCam1 stop twin */
#if 0 // wait to confirm the using timing for this cmd
        if(CamPathCFG(*pCam1, ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }
        if(CamPathCFG(*pCam1, ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }
#endif

        /* pCam1 start resume */
        WaitSttEnqBack(pCam1);
        pCam1->Ut_obj->mResumeStart= MTRUE;

        /* wait pCam1 resume ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam1->Ut_obj->mResumeStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);

        /* pCam2 stop twin */
        if(CamPathCFG(*pCam2, ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }
        if(CamPathCFG(*pCam2, ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }

        /* pCam2 start resume */
        WaitSttEnqBack(pCam2);
        pCam2->Ut_obj->mResumeStart= MTRUE;

        /* wait pCam2 resume ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam2->Ut_obj->mResumeStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);
        break;
    case SWITCH_CASE_7:
        /* pCam1 start twin */
#if 0 // wait to confirm the using timing for this cmd

        if(CamPathCFG(*pCam1, ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }
        if(CamPathCFG(*pCam1, ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_One_TG) == MFALSE){
            MY_LOGE("CamPathCFG fail");
            ret = MFALSE;
            break;
        }
#endif

        /* pCam1 start resume */
        WaitSttEnqBack(pCam1);
        pCam1->Ut_obj->mResumeStart= MTRUE;

        /* wait pCam1 resume ready */
        cnt = 0;
        while(cnt++ < 1000){
            if(!pCam1->Ut_obj->mResumeStart)
                break;
            usleep(3000);
        }
        if(cnt >= 1000){
            ret = MFALSE;
            break;
        }

        /* give it a random time interval */
        ran = ((rand() % 1)*1000000) + ((rand() % 9)*100000);
        usleep(ran);
        break;
    default:
        break;
    }

Exit:
    return ret;
}

void test_iq_query()
{
    sCAM_QUERY_IQ_LEVEL sIQ;
    QueryInData_t queryInfo1, queryInfo2;

    // sensor 1
    queryInfo1.sensorIdx = 0;
    queryInfo1.scenarioId = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    queryInfo1.rrz_out_w = 1920;
    queryInfo1.minCamProcessedFps = 30;
    queryInfo1.pattern = eCAM_NORMAL;

    // sensor 2
    queryInfo2.sensorIdx = 1;
    queryInfo2.scenarioId = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    queryInfo2.rrz_out_w = 1920;
    queryInfo2.minCamProcessedFps = 30;
    queryInfo2.pattern = eCAM_NORMAL;

    // input: all sensor information
    sIQ.QueryInput.vInData.clear();
    sIQ.QueryInput.vInData.push_back(queryInfo1);
    sIQ.QueryInput.vInData.push_back(queryInfo2);

    // query
    getNormalPipeModule()->query((MUINT32)ENPipeQueryCmd_IQ_LEVEL, (MUINTPTR)&sIQ);

    // support iq query or not
    if(sIQ.QueryOutput == MTRUE){
        MY_LOGI("support iq query");
    }
    else{
        MY_LOGI("not support iq query");
    }

    // query result
    for(MUINT32 i = 0 ; i < sIQ.QueryInput.vOutData.size() ; i++){
        MY_LOGI("[IQ Table] ===== Set %d =====", i);
        for(MUINT32 j = 0 ; j < sIQ.QueryInput.vOutData[i].size() ; j++){
            MY_LOGI("[IQ Table] sensorIdx:%d isTwin:%d lvIQ:%d lvClk:%d result:%d", sIQ.QueryInput.vOutData[i][j].sensorIdx \
                ,sIQ.QueryInput.vOutData[i][j].isTwin \
                ,sIQ.QueryInput.vOutData[i][j].IQlv \
                ,sIQ.QueryInput.vOutData[i][j].clklv \
                ,sIQ.QueryInput.vOutData[i][j].result);
        }
    }
}

#if 1
void update_arg(char** argv, int argc, MUINT32 argIdx, MUINT32 content)
{
    if(argIdx >= 0 && argIdx < argc){
        char* s = (char*)malloc(sizeof(char) * 64);
        sprintf(s, "%d", content);
        free(argv[argIdx]);
        argv[argIdx] = s;
    }
}

void wait_npipe_config(_arg* pArg, MUINT32 tgIdx)
{
    MUINT32 cnt = 0;

    while(1){
        cnt++;
        if(cnt % 200 == 0)
            MY_LOGD("[%d's] wait for Ut_obj to be created:tg_%d", cnt / 200, tgIdx);
        if(pArg[tgIdx].Ut_obj){
            if(pArg[tgIdx].Ut_obj->mbConfig)
                break;
        }
        usleep(5000);
   }
}

#else
void update_SenIdx(char** argv, int argc, MUINT32 senIdx)
{
    if(argIdx >= 0 && argIdx < argc){
        char* s = (char*)malloc(sizeof(char) * 64);
        sprintf(s, "%d", content);
        free(argv[argIdx]);
        argv[argIdx] = s;
    }
}
#endif

MBOOL query_hw_func(MUINT32 sensorArrayLocal[_MAXTG_], SensorStaticInfo mSensorInfo[_MAXTG_], MUINT32 scenarioId, E_CamPattern dataPattern,
        MUINT32 sensorNum, MUINT32 enablePort, MUINT32 minCamFps, CROP_TEST::ECROP croptestsel, MSize customCropSize[_MAXTG_],
        MUINT32 scaleSel, MUINT32 scaleFactor, sCAM_QUERY_FUNC &qryFunc)
{
    MBOOL           ret = MTRUE;
    CROP_TEST       crop[_MAXTG_];
    MSize           TgSize;
    EImageFormat    fmt;

    for (MUINT32 sidx = 0; sidx < sensorNum; sidx++){
        QueryInData_t   in;
        MRect           _cropsize;
        MSize           _dstsize;

        getSensorSize(&TgSize, mSensorInfo[sidx], scenarioId);

        crop[sidx].InitCfg(TgSize, croptestsel, TgSize);


        if(enablePort & (__RAWI_ENABLE|__UFDI_ENABLE)){
            if(enablePort & (__SV_DCIF_IMGO_ENABLE|__SV_DCIF_UFEO_ENABLE)){
                MY_LOGE("##########");
                MY_LOGE("can't enable BIT-TRUE & DCIF at the same time");
                MY_LOGE("##########");
                return MFALSE;
            }
        }

        if (enablePort & (__SV_DCIF_IMGO_ENABLE|__SV_DCIF_UFEO_ENABLE))
            in.IQlv = eCamIQ_H;
        else
            in.IQlv = eCamIQ_L;
        in.pattern = dataPattern;
        in.scenarioId = scenarioId;

        if (enablePort & (__RRZO_ENABLE|__UFGO_ENABLE)) {
            in.minCamProcessedFps = minCamFps;
            if (!getOutputFmt(PORT_RRZO, getPortBitDepth(PORT_RRZO, enablePort), mSensorInfo[sidx], &fmt, !!(enablePort & __UFGO_ENABLE))) {
                MY_LOGE("get pix fmt error");
                return 0;
            }

            //set/get crop info
            if (croptestsel == CROP_TEST::CROP_SPECIFIC) {
                crop[sidx].SetInfo(PORT_RRZO,fmt,customCropSize[sidx]);
            }
            crop[sidx].GetCropInfo(PORT_RRZO,fmt,_cropsize);
            crop[sidx].GetScaleInfo(PORT_RRZO,fmt,_dstsize,scaleSel,scaleFactor);

            in.rrz_out_w = _dstsize.w;
        }
        else
            in.rrz_out_w = 0;


        in.sensorIdx = sensorArrayLocal[sidx];
        qryFunc.QueryInput.vInData.push_back(in);
    }

    MY_LOGE("################ query_func start\n");
    getNormalPipeModule()->query(ENPipeQueryCmd_FUNC,(MUINTPTR)&qryFunc);
    MY_LOGE("################ query_func end result: %d in/out depth: %d_%d\n",
        qryFunc.QueryOutput, qryFunc.QueryInput.vInData.size(), qryFunc.QueryInput.vOutData.size());

    return ret;
}


MBOOL query_hw_res_alloc(MUINT32 sensorArrayLocal[_MAXTG_], SensorStaticInfo mSensorInfo[_MAXTG_], MUINT32 scenarioId, E_CamPattern dataPattern,
        MUINT32 sensorNum, MUINT32 enablePort, MUINT32 minCamFps, CROP_TEST::ECROP croptestsel, MSize customCropSize[_MAXTG_],
        MUINT32 scaleSel, MUINT32 scaleFactor, MBOOL* dcifArray)

{
    MBOOL       ret = MTRUE;
    MUINT32     scaleCtrl, cropSel = 0;
    CROP_TEST   crop[_MAXTG_];
    MSize       TgSize;
    EImageFormat fmt;
    sCAM_QUERY_HW_RES_ALLOC qryHwRes;

    // clear input data vector
    qryHwRes.QueryParam.vInData.clear();

    for(MUINT32 i = 0 ; i < sensorNum ; i++){
        QueryInResAlloc_t   qryInData;
        MRect           _cropsize;
        MSize           _dstsize;

        // set input data
        // qryInData.sensorIdx = sensorArray[i];
        qryInData.sensorIdx = i;
        qryInData.scenarioId = scenarioId;
        qryInData.minCamProcessedFps = minCamFps;
        qryInData.pattern = (E_CamPattern)dataPattern;
        if(enablePort & (__RAWI_ENABLE|__UFDI_ENABLE)){
            if(enablePort & (__SV_DCIF_IMGO_ENABLE|__SV_DCIF_UFEO_ENABLE)){
                MY_LOGE("##########");
                MY_LOGE("can't enable BIT-TRUE & DCIF at the same time");
                MY_LOGE("##########");
                return MFALSE;
            }
        }

        if (enablePort & (__SV_DCIF_IMGO_ENABLE|__SV_DCIF_UFEO_ENABLE))
            qryInData.IQlv = eCamIQ_H;
        else
            qryInData.IQlv = eCamIQ_L;


        // get sensor size
        getSensorSize(&TgSize, mSensorInfo[i], scenarioId);

        // crop configuration
        crop[i].InitCfg(TgSize, croptestsel, TgSize);

        if (enablePort & (__RRZO_ENABLE|__UFGO_ENABLE)) {
            // get max rrz output size
            if (!getOutputFmt(PORT_RRZO, getPortBitDepth(PORT_RRZO, enablePort), mSensorInfo[i], &fmt, !!(enablePort & __UFGO_ENABLE))) {
                MY_LOGE("get pix fmt error");
                ret = MFALSE;
                goto Exit;
            }

            if (croptestsel == CROP_TEST::CROP_SPECIFIC) {
                crop[i].SetInfo(PORT_RRZO, fmt, customCropSize[i]);
            }
            crop[i].GetCropInfo(PORT_RRZO, fmt, _cropsize);
            crop[i].GetScaleInfo(PORT_RRZO, fmt, _dstsize, scaleSel, scaleFactor);

            // rrz ouput width
            MY_LOGD("maxCamProcessedOutW: %d", _dstsize.w);
            qryInData.maxCamProcessedOutW = _dstsize.w;
        }
        else {
            MY_LOGE("not yet support rrzo not enable");
            ret = MFALSE;
            goto Exit;
        }

        // push input data
        qryHwRes.QueryParam.vInData.push_back(qryInData);
    }

    // query
    getNormalPipeModule()->query((MUINT32)ENPipeQueryCmd_HW_RES_ALLOC, (MUINTPTR)&qryHwRes);
    if(qryHwRes.result == MFALSE){
        MY_LOGE("query hw res alloc fail");
        ret = MFALSE;
        goto Exit;
    }

    // query result
    for(MUINT32 i = 0 ; i < qryHwRes.QueryParam.vOutData.size() ; i++){
        for(MUINT32 j = 0 ; j < sensorNum ; j++){
            if(qryHwRes.QueryParam.vOutData.at(i).sensorIdx == j){
                dcifArray[j] = qryHwRes.QueryParam.vOutData.at(i).needFullFpsSVRawDump;
            }
        }
    }

Exit:
    return ret;
}

void updateFunc(sCAM_QUERY_FUNC& dst, MUINT32 sensorIdx, sCAM_QUERY_FUNC src)
{
    // update query result
    dst.QueryOutput = src.QueryOutput;

    // clear vector
    dst.QueryInput.vOutData.clear();

    if(src.QueryOutput){
        // push corresponding info
        for(MUINT32 i = 0 ; i < src.QueryInput.vOutData.size() ; i++){
            if(sensorIdx == src.QueryInput.vOutData.at(i).sensorIdx){
                dst.QueryInput.vOutData.push_back(src.QueryInput.vOutData.at(i));
            }
        }

        // check vector size
        if(dst.QueryInput.vOutData.size() != 1){
            MY_LOGE("unexpected vector size");
            dst.QueryOutput = MFALSE;
            dst.QueryInput.vOutData.clear();
        }
    }
}

int test_camio(int argc, char** argv)
{
    MUINT32             sensorDev, DevCmd, sensorNum, _switchCam = ONE_SENSOR, EnqCount, i, ran = 0, minCamFps = 30;
    MUINT32             sensorArray[_MAXTG_] = {0, 0, 0}, sensorArrayLocal[_MAXTG_] =  {0, 0, 0}, scenario = 0, enablePort;
    MBOOL               dcifArray[_MAXTG_] = {0, 0, 0};
    pthread_t           CAM_A = (pthread_t)NULL, CAM_B = (pthread_t)NULL, CAM_C = (pthread_t)NULL;
    _arg                ts_arg[_MAXTG_];
    MBOOL               avalible = MTRUE, ret = MTRUE, ReleaseDone = MFALSE, bRanIQ = MFALSE;
    NPipeUT*            pUt[_MAXTG_] = {NULL, NULL, NULL};
    MUINT32             PathCFG = 0, cnt = 0;

    IHalSensorList*     pHalSensorList = NULL;
    SensorStaticInfo    mSensorInfo[_MAXTG_];
    CROP_TEST::ECROP    croptestsel = CROP_TEST::CROP_NONE;
    MSize               customCropSize[_MAXTG_];
    E_CamPattern        data_pattern = eCAM_NORMAL;
    MUINT32             scaleSel = SCALE_NONE, scaleFactor = 100, scaleCtrl = 0;
    sCAM_QUERY_FUNC     q_func;

    #if 0 // iq table query test
    test_iq_query();
    return 0;
    #endif

    sensorDev = atoi(argv[CMD_IDX_SNRDEVID]);
    scenario  = atoi(argv[CMD_IDX_SNRSCEN]);
    enablePort = atoi(argv[CMD_IDX_ENABLEPORTS]);

    if (argc > CMD_IDX_SWITCHCAM) {
        _switchCam = atoi(argv[CMD_IDX_SWITCHCAM]);
    }

    if (argc > CMD_IDX_DATA_PATTERN) {
        data_pattern = (E_CamPattern)atoi(argv[CMD_IDX_DATA_PATTERN]);
        MY_LOGD("Sensor Data Pattern: %d", data_pattern);
    }

    if (argc > CMD_IDX_SCALESEL) {
        scaleCtrl = atoi(argv[CMD_IDX_SCALESEL]);
        MY_LOGD("Scale Ctrl: %d", scaleCtrl);

        // scale selection
        switch(scaleCtrl & 0x3){
        case 0:  scaleSel = SCALE_NONE;     break;
        case 1:  scaleSel = SCALE_DEFAULT;  break;
        case 2:  scaleSel = SCALE_SPECIFIC; break;
        default: scaleSel = SCALE_NONE;     break;
        }

        // scale factor
        if(scaleSel == SCALE_SPECIFIC){
            if((scaleCtrl >> 2) > 0 && (scaleCtrl >> 2) <= 100){
                scaleFactor = (scaleCtrl >> 2);
            }
            else{
                MY_LOGE("Illegal scaling factor:%d", scaleCtrl >> 2);
                scaleSel = SCALE_NONE;
                scaleFactor = 100;
            }
        }
    }

    if(argc > CMD_IDX_FPS_CTRL){
        minCamFps = atoi(argv[CMD_IDX_FPS_CTRL])>>2;
        MY_LOGD("minCamFps: %d", minCamFps);
    }

    memset((void*)ts_arg, 0, sizeof(ts_arg));

    sensorNum = 0;
    if (0 == sensorDev) {
        sensorDev = UT_MAIN_FKSNR;
    }

    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_MAIN_SNR | UT_MAIN_FKSNR)) {
            if(sensorDev & UT_MAIN_SNR){
                sensorArray[sensorNum] = UT_MAIN_SNR;
            }
            if(sensorDev & UT_MAIN_FKSNR){
                sensorArray[sensorNum] = UT_MAIN_FKSNR;
            }
            sensorArrayLocal[sensorNum] = 0;
            sensorNum++;
        }
    }

    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_SUB_SNR | UT_SUB_FKSNR)) {
            if(sensorDev & UT_SUB_SNR){
                sensorArray[sensorNum] = UT_SUB_SNR;
            }
            if(sensorDev & UT_SUB_FKSNR){
                sensorArray[sensorNum] = UT_SUB_FKSNR;
            }
            sensorArrayLocal[sensorNum] = 1;
            sensorNum++;
        }
    }

    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_MAIN2_SNR | UT_MAIN2_FKSNR)) {
            if(sensorDev & UT_MAIN2_SNR){
                sensorArray[sensorNum] = UT_MAIN2_SNR;
            }
            if(sensorDev & UT_MAIN2_FKSNR){
                sensorArray[sensorNum] = UT_MAIN2_FKSNR;
            }
            sensorArrayLocal[sensorNum] = 2;
            sensorNum++;
        }
    }

    if (sensorNum < _MAXTG_) {
        if (sensorDev & (UT_SUB2_SNR | UT_SUB2_FKSNR)) {
            if(sensorDev & UT_SUB2_SNR){
                sensorArray[sensorNum] = UT_SUB2_SNR;
            }
            if(sensorDev & UT_SUB2_FKSNR){
                sensorArray[sensorNum] = UT_SUB2_FKSNR;
            }
            sensorArrayLocal[sensorNum] = 3;
            sensorNum++;
        }
    }

    switch (atoi(argv[CMD_IDX_CROPSEL])) {
    case 0: croptestsel = CROP_TEST::CROP_NONE; break;
    case 1: croptestsel = CROP_TEST::CROP_SYSMATRIC; break;
    case 2: croptestsel = CROP_TEST::CROP_LEFT_ONLY; break;
    case 3: croptestsel = CROP_TEST::CROP_RIGHT_ONLY; break;
    case 4:
    {
        for (MUINT32 j = 0; j < sensorNum; j++) {
            char str[32];
            char _str[2][16];
            char* ptr=0;
            int i=0;

            ptr = str;
            str[0] = _str[0][0] = _str[1][0] = '\0';

            MY_LOGI("Input specific out size for sensor_%d, Param:<output width>,<output height>", i);
            fgets(str,sizeof(str),stdin);

            for(;*ptr!='\0';ptr++){
                if(*ptr == ',')
                    i++;
                else
                    strncat(_str[i],ptr,sizeof(char));
            }

            customCropSize[i].w = atoi(_str[0]);
            customCropSize[i].h = atoi(_str[1]);
        }
        break;
    }
    default:
        break;
    }

    if(sensorNum > _MAXTG_){
        MY_LOGE("sensor number(%d) is not supported ",sensorNum);
        return 0;
    }

    if (argc > CMD_IDX_RANDOM_TEST) {
       bRanIQ = (atoi(argv[CMD_IDX_RANDOM_TEST]) & 0x1) ? MTRUE : MFALSE;
    }

    if (sensorDev & UT_SNR_MASK) {
        TS_FakeSensorList::setFakeMode(MFALSE);
        pHalSensorList = IHalSensorList::get();
    }
    else {
        TS_FakeSensorList::setFakeMode(MTRUE);
        pHalSensorList = TS_FakeSensorList::getTestModel();
    }

    MY_LOGD("search sensor .... ++\n");
    // sensor number less or equal to zero means that searchSensors() not called yet
    if(pHalSensorList->queryNumberOfSensors() <= 0){
        pHalSensorList->searchSensors();
    }
    MY_LOGD("search sensor .... --\n");

    for (MUINT32 sidx = 0; sidx < sensorNum; sidx++) {
        MY_LOGD("[%d] sid:%d qSdevIdx: %d\n", sidx, sensorArrayLocal[sidx], pHalSensorList->querySensorDevIdx(sensorArrayLocal[sidx]));
        pHalSensorList->querySensorStaticInfo(
                pHalSensorList->querySensorDevIdx(sensorArrayLocal[sidx]), &mSensorInfo[sidx]);

        ts_arg[sidx].qryFunc.QueryOutput = MFALSE;
    }


    if (query_hw_func(sensorArrayLocal, mSensorInfo, scenario, data_pattern, sensorNum, enablePort, minCamFps,
                        croptestsel, customCropSize, scaleSel, scaleFactor, q_func) == MFALSE) {
        MY_LOGE("hw func query fail");
        return 0;
    }
    if((q_func.QueryOutput == MFALSE) && (sensorNum > 1)){
        MY_LOGE("query supported function fail,multi-sensor will be failed too\n");
        return NULL;
    }
    else if(q_func.QueryInput.vOutData.size() != sensorNum){
        MY_LOGE("resoult of supported function r not match sensorNum %d_%d\n", q_func.QueryInput.vOutData.size(), sensorNum);
        return NULL;
    }
    if (q_func.QueryOutput) {
        for (MUINT32 s = 0; s < q_func.QueryInput.vOutData.size(); s++) {
            MY_LOGI("[result] sensorIdx : %d", q_func.QueryInput.vOutData.at(s).sensorIdx);
            MY_LOGI("[result] isTwin    : %d", q_func.QueryInput.vOutData.at(s).isTwin);
            MY_LOGI("[result] IQlv      : %d", q_func.QueryInput.vOutData.at(s).IQlv);
            MY_LOGI("[result] clklv     : %d", q_func.QueryInput.vOutData.at(s).clklv);
            MY_LOGI("[result] result    : %d", q_func.QueryInput.vOutData.at(s).result);
            MY_LOGI("[result] D_UFO     : %d", q_func.QueryInput.vOutData.at(s).function.Bits.D_BayerENC);
            MY_LOGI("[result] isFLK     : %d", q_func.QueryInput.vOutData.at(s).function.Bits.isFLK);
            MY_LOGI("[result] isRSS     : %d", q_func.QueryInput.vOutData.at(s).function.Bits.isRSS);
            MY_LOGI("[result] isLMV     : %d", q_func.QueryInput.vOutData.at(s).function.Bits.isLMV);
            MY_LOGI("[result] isFullYUV : %d", q_func.QueryInput.vOutData.at(s).function.Bits.isFull_dir_YUV);
            MY_LOGI("[result] isSclY    : %d", q_func.QueryInput.vOutData.at(s).function.Bits.isScaled_Y);
            MY_LOGI("[result] isSclYUV  : %d", q_func.QueryInput.vOutData.at(s).function.Bits.isScaled_YUV);
            MY_LOGI("[result] isGSclYUV : %d", q_func.QueryInput.vOutData.at(s).function.Bits.isG_Scaled_YUV);
        }
    }
    else {
        MY_LOGI("qurey func result fail");
    }

    // hw res query, dcif not supported for normalsv case & RAWI case
    if(_switchCam != ONESENSOR_NORMALSV){
        if(query_hw_res_alloc(sensorArray, mSensorInfo, scenario, data_pattern, sensorNum, enablePort, minCamFps,
                                croptestsel, customCropSize, scaleSel, scaleFactor, dcifArray) == MFALSE){
            MY_LOGE("hw res query fail");
        }
        else
            MY_LOGI("[DCIF result] (%d/%d/%d)", dcifArray[0], dcifArray[1], dcifArray[2]);
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
                dupArgv(&argv, &ts_arg[_TG_1_]._argv, argc);
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_1_].bDCIF = dcifArray[_TG_1_];
                ts_arg[_TG_1_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_1_].custCropSize = customCropSize[_TG_1_];
                ts_arg[_TG_1_].targetTG = TG_CAM_MAX;
                updateFunc(ts_arg[_TG_1_].qryFunc, sensorArrayLocal[_TG_1_], q_func);
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_1_);

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
                    WaitSttEnqBack(&ts_arg[_TG_1_]);
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
            freeDupArgv(ts_arg[_TG_1_]._argc, ts_arg[_TG_1_]._argv);
            }
            break;
        case TWO_SENSOR: {
            char **tg1_argv = NULL, **tg2_argv = NULL;

            if (sensorNum != 2) {
                MY_LOGD("switch cam need config two cam a time");
                break;
            }

            if (sensorArray[_TG_1_]) {
                MY_LOGD("########CAM A###########");
                dupArgv(&argv, &tg1_argv, argc);
                if (tg1_argv == NULL) {
                    LOG_ERR("Error: dup argv fail\n");
                    break;
                }
                update_arg(tg1_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_1_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg1_argv[i]));
                dupArgv(&tg1_argv, &ts_arg[_TG_1_]._argv, argc);
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_].TG_Num = _TG_1_;
                ts_arg[_TG_1_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_1_].bDCIF = dcifArray[_TG_1_];
                ts_arg[_TG_1_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_1_].custCropSize = customCropSize[_TG_1_];
                ts_arg[_TG_1_].targetTG = TG_CAM_MAX;
                updateFunc(ts_arg[_TG_1_].qryFunc, sensorArrayLocal[_TG_1_], q_func);
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_1_);

            if (sensorArray[_TG_2_]){
                MY_LOGD("#######CAM B###########");
                dupArgv(&argv, &tg2_argv, argc);
                if (tg2_argv == NULL) {
                    LOG_ERR("Error: dup argv fail\n");
                    break;
                }
                update_arg(tg2_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_2_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg2_argv[i]));
                dupArgv(&tg2_argv, &ts_arg[_TG_2_]._argv, argc);
                ts_arg[_TG_2_]._argc = argc;
                ts_arg[_TG_2_].TG_Num = _TG_2_;
                ts_arg[_TG_2_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_2_].bDCIF = dcifArray[_TG_2_];
                ts_arg[_TG_2_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_2_].custCropSize = customCropSize[_TG_2_];
                ts_arg[_TG_2_].targetTG = TG_CAM_MAX;
                updateFunc(ts_arg[_TG_2_].qryFunc, sensorArrayLocal[_TG_2_], q_func);
                pthread_create(&CAM_B, NULL, test_camio_thread, &ts_arg[_TG_2_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_2_);

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
                WaitSttEnqBack(&ts_arg[_TG_2_]);
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
            freeDupArgv(argc, tg1_argv);
            freeDupArgv(argc, tg2_argv);
            freeDupArgv(ts_arg[_TG_1_]._argc, ts_arg[_TG_1_]._argv);
            freeDupArgv(ts_arg[_TG_2_]._argc, ts_arg[_TG_2_]._argv);
            }
            break;
        case NONSTOP_DYNAMIC_TWIN:{
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
                update_arg(tg1_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_1_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(argv[i]));
                dupArgv(&tg1_argv, &ts_arg[_TG_1_]._argv, argc);
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_].TG_Num = _TG_1_;
                ts_arg[_TG_1_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_1_].bDCIF = dcifArray[_TG_1_];
                ts_arg[_TG_1_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_1_].custCropSize = customCropSize[_TG_1_];
                ts_arg[_TG_1_].targetTG = TG_CAM_MAX;
                updateFunc(ts_arg[_TG_1_].qryFunc, sensorArrayLocal[_TG_1_], q_func);
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_1_);

            while(1){
                sleep(1);

                MY_LOGD("########Switch to Two###########");

                if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_SET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
                    MY_LOGE("CamPathCFG fail");
                    sleep(10);
                    continue;
                }
                if(CamPathCFG(ts_arg[_TG_1_], ENPipeCmd_GET_HW_PATH_CFG, eCamHwPathCfg_Two_TG) == MFALSE){
                    MY_LOGE("CamPathCFG fail");
                    break;
                }

                if (sensorArray[_TG_2_]){
                    MY_LOGD("#######CAM B###########");
                    dupArgv(&argv, &tg2_argv, argc);
                    if (tg2_argv == NULL) {
                        LOG_ERR("Error: dup argv fail\n");
                        break;
                    }
                    update_arg(tg2_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_2_]);
                    for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                        MY_LOGD("cmd_%d",atoi(argv[i]));
                    dupArgv(&tg2_argv, &ts_arg[_TG_2_]._argv, argc);
                    ts_arg[_TG_2_]._argc = argc;
                    ts_arg[_TG_2_].TG_Num = _TG_2_;
                    ts_arg[_TG_2_].pipeSel = EPipeSelect_Normal;
                    ts_arg[_TG_2_].bDCIF = dcifArray[_TG_2_];
                    ts_arg[_TG_2_].mIHalSensorList = pHalSensorList;
                    ts_arg[_TG_2_].custCropSize = customCropSize[_TG_2_];
                    ts_arg[_TG_2_].targetTG = TG_CAM_MAX;
                    updateFunc(ts_arg[_TG_2_].qryFunc, sensorArrayLocal[_TG_2_], q_func);
                    pthread_create(&CAM_B, NULL, test_camio_thread, &ts_arg[_TG_2_]);
                }

                // wait npipe finish config
                wait_npipe_config(ts_arg, _TG_2_);

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

                if(ts_arg[_TG_1_].Ut_obj->mLoopCount > ts_arg[_TG_1_].Ut_obj->mFrameNum){
                    ts_arg[_TG_1_].Ut_obj->m_bStop = MTRUE;
                    pthread_join(CAM_A, NULL);
                    ts_arg[_TG_1_].Ut_obj = NULL;
                    break;
                }
            }

            if(ts_arg[_TG_1_].Ut_obj){
                ts_arg[_TG_1_].Ut_obj->m_bStop = MTRUE;
                pthread_join(CAM_A, NULL);
                ts_arg[_TG_1_].Ut_obj = NULL;
            }
            if(ts_arg[_TG_2_].Ut_obj){
                ts_arg[_TG_2_].Ut_obj->m_bStop = MTRUE;
                pthread_join(CAM_B, NULL);
                ts_arg[_TG_2_].Ut_obj = NULL;
            }
            freeDupArgv(argc, tg1_argv);
            freeDupArgv(argc, tg2_argv);
            freeDupArgv(ts_arg[_TG_1_]._argc, ts_arg[_TG_1_]._argv);
            freeDupArgv(ts_arg[_TG_2_]._argc, ts_arg[_TG_2_]._argv);
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
                update_arg(tg1_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_1_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg1_argv[i]));
                dupArgv(&tg1_argv, &ts_arg[_TG_1_]._argv, argc);
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_].TG_Num = _TG_1_;
                ts_arg[_TG_1_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_1_].bDCIF = dcifArray[_TG_1_];
                ts_arg[_TG_1_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_1_].custCropSize = customCropSize[_TG_1_];
                ts_arg[_TG_1_].targetTG = TG_CAM_MAX;
                updateFunc(ts_arg[_TG_1_].qryFunc, sensorArrayLocal[_TG_1_], q_func);
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_1_);

            if (sensorArray[_TG_2_]){
                MY_LOGD("#######CAM B###########");
                dupArgv(&argv, &tg2_argv, argc);
                if (tg2_argv == NULL) {
                    LOG_ERR("Error: dup argv fail\n");
                    break;
                }
                update_arg(tg2_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_2_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg2_argv[i]));
                dupArgv(&tg2_argv, &ts_arg[_TG_2_]._argv, argc);
                ts_arg[_TG_2_]._argc = argc;
                ts_arg[_TG_2_].TG_Num = _TG_2_;
                ts_arg[_TG_2_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_2_].bDCIF = dcifArray[_TG_2_];
                ts_arg[_TG_2_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_2_].custCropSize = customCropSize[_TG_2_];
                ts_arg[_TG_2_].targetTG = TG_CAM_MAX;
                updateFunc(ts_arg[_TG_2_].qryFunc, sensorArrayLocal[_TG_2_], q_func);
                pthread_create(&CAM_B, NULL, test_camio_thread, &ts_arg[_TG_2_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_2_);

            while(ts_arg[_TG_2_].Ut_obj->mSuspendTest){
                if(ts_arg[_TG_2_].Ut_obj->mSuspendFlow != NONSTOP_SUSPEND){
                    MY_LOGE("dynamic twin suspend test need use non-stop suspend");
                    break;
                }

                cnt = 0;
                while(cnt++ < 100){
                    if(!ts_arg[_TG_2_].Ut_obj->mResumeStart)
                        break;
                    usleep(3000);
                }
                if(cnt >= 100){
                     MY_LOGD("[TIME OUT] Wait Resume Ready");
                     break;
                }

                /*Give it a random time to run 1 sensor(twin)*/
                ran = ((rand() % 1)*1000000) +((rand() % 9)*100000);
                usleep(ran);

                /*Suspend start*/
                ts_arg[_TG_2_].Ut_obj->mSuspendStart = MTRUE;

                cnt = 0;
                while(cnt++ < 100){
                    if(!ts_arg[_TG_2_].Ut_obj->mSuspendStart)
                        break;
                    usleep(3000);
                }
                if(cnt >= 100){
                     MY_LOGD("[TIME OUT] Wait Suspend Ready");
                     break;
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
                WaitSttEnqBack(&ts_arg[_TG_2_]);
                ts_arg[_TG_2_].Ut_obj->mResumeStart = MTRUE;
            }

            if(ts_arg[_TG_1_].Ut_obj){
                ts_arg[_TG_1_].Ut_obj->m_bStop = MTRUE;
                pthread_join(CAM_A, NULL);
                ts_arg[_TG_1_].Ut_obj = NULL;
            }
            if(ts_arg[_TG_2_].Ut_obj){
                ts_arg[_TG_2_].Ut_obj->m_bStop = MTRUE;
                pthread_join(CAM_B, NULL);
                ts_arg[_TG_2_].Ut_obj = NULL;
            }

            freeDupArgv(argc, tg1_argv);
            freeDupArgv(argc, tg2_argv);
            freeDupArgv(ts_arg[_TG_1_]._argc, ts_arg[_TG_1_]._argv);
            freeDupArgv(ts_arg[_TG_2_]._argc, ts_arg[_TG_2_]._argv);
            }
            break;
        case NONSTOP_RANDOM_SWITCH_BY_SUSPEND: {
            char **tg1_argv = NULL, **tg2_argv = NULL;
            MUINT32 prevCase = 0, nextCase = 0;

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
                update_arg(tg1_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_1_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg1_argv[i]));
                dupArgv(&tg1_argv, &ts_arg[_TG_1_]._argv, argc);
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_].TG_Num = _TG_1_;
                ts_arg[_TG_1_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_1_].bDCIF = dcifArray[_TG_1_];
                ts_arg[_TG_1_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_1_].custCropSize = customCropSize[_TG_1_];
                ts_arg[_TG_1_].targetTG = TG_CAM_MAX;
                updateFunc(ts_arg[_TG_1_].qryFunc, sensorArrayLocal[_TG_1_], q_func);
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_1_);

            if (sensorArray[_TG_2_]){
                MY_LOGD("#######CAM B###########");
                dupArgv(&argv, &tg2_argv, argc);
                if (tg2_argv == NULL) {
                    LOG_ERR("Error: dup argv fail\n");
                    break;
                }
                update_arg(tg2_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_2_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg2_argv[i]));
                dupArgv(&tg2_argv, &ts_arg[_TG_2_]._argv, argc);
                ts_arg[_TG_2_]._argc = argc;
                ts_arg[_TG_2_].TG_Num = _TG_2_;
                ts_arg[_TG_2_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_2_].bDCIF = dcifArray[_TG_2_];
                ts_arg[_TG_2_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_2_].custCropSize = customCropSize[_TG_2_];
                ts_arg[_TG_2_].targetTG = TG_CAM_MAX;
                updateFunc(ts_arg[_TG_2_].qryFunc, sensorArrayLocal[_TG_2_], q_func);
                pthread_create(&CAM_B, NULL, test_camio_thread, &ts_arg[_TG_2_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_2_);

            while(ts_arg[_TG_1_].Ut_obj->mSuspendTest &&
                  ts_arg[_TG_2_].Ut_obj->mSuspendTest){

                if(ts_arg[_TG_1_].Ut_obj->mSuspendFlow != NONSTOP_SUSPEND ||
                   ts_arg[_TG_2_].Ut_obj->mSuspendFlow != NONSTOP_SUSPEND){
                    MY_LOGE("dynamic twin suspend test need use non-stop suspend");
                    break;
                }

                // no switch during the last five frames, to avoid sendcmd error if no enque
                if((ts_arg[_TG_1_].Ut_obj->mLoopCount < ts_arg[_TG_1_].Ut_obj->mFrameNum - 5) &&
                   (ts_arg[_TG_2_].Ut_obj->mLoopCount < ts_arg[_TG_2_].Ut_obj->mFrameNum - 5)){
                    /* choose next case, in total 4 cases for dual cam   */
                    /* case'0: both camA and camB keep streaming on      */
                    /* case'1: camA suspends and camB keeps streaming on */
                    /* case'2: camA keep streaming on and camB suspends  */
                    /* case'3: both camA and camB suspend                */
                    nextCase = rand() % 3; // temporarily un-support case'3, so mod 3
                }

                MY_LOGD("prevCase:%d nextCase:%d", prevCase, nextCase);
                switch(prevCase){
                case 0:
                    if(nextCase == 0)      { /* do nothing */ }
                    else if(nextCase == 1) {ret = SwitchCamState(SWITCH_CASE_1, (ts_arg + _TG_2_), (ts_arg + _TG_1_), bRanIQ);}
                    else if(nextCase == 2) {ret = SwitchCamState(SWITCH_CASE_1, (ts_arg + _TG_1_), (ts_arg + _TG_2_), bRanIQ);}
                    else if(nextCase == 3) {ret = SwitchCamState(SWITCH_CASE_2, (ts_arg + _TG_1_), (ts_arg + _TG_2_), bRanIQ);}
                break;
                case 1:
                    if(nextCase == 0)      {ret = SwitchCamState(SWITCH_CASE_3, (ts_arg + _TG_1_), (ts_arg + _TG_2_), bRanIQ);}
                    else if(nextCase == 1) { /* do nothing */ }
                    else if(nextCase == 2) {ret = SwitchCamState(SWITCH_CASE_4, (ts_arg + _TG_1_), (ts_arg + _TG_2_), bRanIQ);}
                    else if(nextCase == 3) {ret = SwitchCamState(SWITCH_CASE_5, (ts_arg + _TG_1_), (ts_arg + _TG_2_), bRanIQ);}
                break;
                case 2:
                    if(nextCase == 0)      {ret = SwitchCamState(SWITCH_CASE_3, (ts_arg + _TG_2_), (ts_arg + _TG_1_), bRanIQ);}
                    else if(nextCase == 1) {ret = SwitchCamState(SWITCH_CASE_4, (ts_arg + _TG_2_), (ts_arg + _TG_1_), bRanIQ);}
                    else if(nextCase == 2) { /* do nothing */ }
                    else if(nextCase == 3) {ret = SwitchCamState(SWITCH_CASE_5, (ts_arg + _TG_1_), (ts_arg + _TG_2_), bRanIQ);}
                break;
                case 3:
                    if(nextCase == 0)      {ret = SwitchCamState(SWITCH_CASE_6, (ts_arg + _TG_1_), (ts_arg + _TG_2_), bRanIQ);}
                    else if(nextCase == 1) {ret = SwitchCamState(SWITCH_CASE_7, (ts_arg + _TG_1_), (ts_arg + _TG_2_), bRanIQ);}
                    else if(nextCase == 2) {ret = SwitchCamState(SWITCH_CASE_7, (ts_arg + _TG_2_), (ts_arg + _TG_1_), bRanIQ);}
                    else if(nextCase == 3) { /* do nothing */ }
                break;
                default:
                break;
                }

                /* switch fail */
                if(ret == MFALSE){
                    MY_LOGE("########## Switch Fail ##########");
                    ts_arg[_TG_1_].Ut_obj->m_bStop = MTRUE;
                    ts_arg[_TG_2_].Ut_obj->m_bStop = MTRUE;
                    pthread_join(CAM_A, NULL);
                    pthread_join(CAM_B, NULL);
                    ts_arg[_TG_1_].Ut_obj = NULL;
                    ts_arg[_TG_2_].Ut_obj = NULL;
                    break;
                }

                /* check loop cnt over frame num */
                if(ts_arg[_TG_1_].Ut_obj->mLoopCount > ts_arg[_TG_1_].Ut_obj->mFrameNum &&
                   ts_arg[_TG_2_].Ut_obj->mLoopCount > ts_arg[_TG_2_].Ut_obj->mFrameNum){
                    MY_LOGE("########## Finish enq/deq ##########");
                    ts_arg[_TG_1_].Ut_obj->m_bStop = MTRUE;
                    ts_arg[_TG_2_].Ut_obj->m_bStop = MTRUE;
                    pthread_join(CAM_A, NULL);
                    pthread_join(CAM_B, NULL);
                    ts_arg[_TG_1_].Ut_obj = NULL;
                    ts_arg[_TG_2_].Ut_obj = NULL;
                    break;
                }

                /* give it a random time interval before starting next switch case*/
                ran = ((rand() % 1)*1000000) +((rand() % 9)*100000);
                MY_LOGD("########[Wait %d(us)]Start Next Case###########", ran);
                usleep(ran);
                prevCase = nextCase;
            }

            if(ts_arg[_TG_1_].Ut_obj){
                ts_arg[_TG_1_].Ut_obj->m_bStop = MTRUE;
                pthread_join(CAM_A, NULL);
                ts_arg[_TG_1_].Ut_obj = NULL;
            }
            if(ts_arg[_TG_2_].Ut_obj){
                ts_arg[_TG_2_].Ut_obj->m_bStop = MTRUE;
                pthread_join(CAM_B, NULL);
                ts_arg[_TG_2_].Ut_obj = NULL;
            }

            freeDupArgv(argc, tg1_argv);
            freeDupArgv(argc, tg2_argv);
            freeDupArgv(ts_arg[_TG_1_]._argc, ts_arg[_TG_1_]._argv);
            freeDupArgv(ts_arg[_TG_2_]._argc, ts_arg[_TG_2_]._argv);
            }
            break;
        case IQ_LEVEL_TEST: {
            char **tg1_argv = NULL, **tg2_argv = NULL;
            MUINT32 prevCase = 0, nextCase = 0;

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
                update_arg(tg1_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_1_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg1_argv[i]));
                dupArgv(&tg1_argv, &ts_arg[_TG_1_]._argv, argc);
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_].TG_Num = _TG_1_;
                ts_arg[_TG_1_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_1_].bDCIF = dcifArray[_TG_1_];
                ts_arg[_TG_1_].bDynSwitch = MTRUE;
                ts_arg[_TG_1_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_1_].custCropSize = customCropSize[_TG_1_];
                ts_arg[_TG_1_].targetTG = TG_A;
                ts_arg[_TG_1_].iqLevel = eCamIQ_H;
                updateFunc(ts_arg[_TG_1_].qryFunc, sensorArrayLocal[_TG_1_], q_func);
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_1_);

            if (sensorArray[_TG_2_]){
                MY_LOGD("#######CAM B###########");
                dupArgv(&argv, &tg2_argv, argc);
                if (tg2_argv == NULL) {
                    LOG_ERR("Error: dup argv fail\n");
                    break;
                }
                update_arg(tg2_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_2_]);
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(tg2_argv[i]));
                dupArgv(&tg2_argv, &ts_arg[_TG_2_]._argv, argc);
                ts_arg[_TG_2_]._argc = argc;
                ts_arg[_TG_2_].TG_Num = _TG_2_;
                ts_arg[_TG_2_].pipeSel = EPipeSelect_Normal;
                ts_arg[_TG_2_].bDCIF = dcifArray[_TG_2_];
                ts_arg[_TG_2_].bDynSwitch = MTRUE;
                ts_arg[_TG_2_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_2_].custCropSize = customCropSize[_TG_2_];
                ts_arg[_TG_2_].targetTG = TG_C;
                ts_arg[_TG_2_].iqLevel = eCamIQ_L;
                updateFunc(ts_arg[_TG_2_].qryFunc, sensorArrayLocal[_TG_2_], q_func);
                pthread_create(&CAM_B, NULL, test_camio_thread, &ts_arg[_TG_2_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_2_);

            while(1){

                usleep(100 * 1000);

                if(ts_arg[_TG_1_].Ut_obj->mbStart == MFALSE || ts_arg[_TG_2_].Ut_obj->mbStart == MFALSE){
                    continue;
                }

                // no switch during the last five frames, to avoid sendcmd error if no enque
                if((ts_arg[_TG_1_].Ut_obj->mLoopCount < ts_arg[_TG_1_].Ut_obj->mFrameNum - 5) &&
                   (ts_arg[_TG_2_].Ut_obj->mLoopCount < ts_arg[_TG_2_].Ut_obj->mFrameNum - 5)){
                    /* choose next case */
                    nextCase = (prevCase + 1) % 2;
                    switch(prevCase){
                        case 0:
                            ret = SwitchImageQuality(1, (ts_arg + _TG_1_), (ts_arg + _TG_2_)); // switch to low/High
                            break;
                        case 1:
                            ret = SwitchImageQuality(0, (ts_arg + _TG_1_), (ts_arg + _TG_2_)); // switch to High/Low
                            break;
                        default:
                            break;
                    }
                }

                /* switch fail */
                if(ret == MFALSE){
                    MY_LOGE("########## Switch Fail ##########");
                    break;
                }

                MY_LOGD("IQ case: TG1(%d/%d)TG2(%d/%d)",
                ts_arg[_TG_1_].Ut_obj->mLoopCount,ts_arg[_TG_1_].Ut_obj->mFrameNum,
                   ts_arg[_TG_2_].Ut_obj->mLoopCount,ts_arg[_TG_2_].Ut_obj->mFrameNum);

                /* check loop cnt over frame num */
                if((ts_arg[_TG_1_].Ut_obj->mLoopCount > ts_arg[_TG_1_].Ut_obj->mFrameNum) &&
                   (ts_arg[_TG_2_].Ut_obj->mLoopCount > ts_arg[_TG_2_].Ut_obj->mFrameNum)){
                    ts_arg[_TG_1_].Ut_obj->m_bStop = MTRUE;
                    ts_arg[_TG_2_].Ut_obj->m_bStop = MTRUE;
                    pthread_join(CAM_A, NULL);
                    pthread_join(CAM_B, NULL);
                    ts_arg[_TG_1_].Ut_obj = NULL;
                    ts_arg[_TG_2_].Ut_obj = NULL;
                    break;
                }

                /* change to next case */
                prevCase = nextCase;
            }

            if(ts_arg[_TG_1_].Ut_obj){
                pthread_join(CAM_A, NULL);
                ts_arg[_TG_1_].Ut_obj = NULL;
            }
            if(ts_arg[_TG_2_].Ut_obj){
                pthread_join(CAM_B, NULL);
                ts_arg[_TG_2_].Ut_obj = NULL;
            }

            freeDupArgv(argc, tg1_argv);
            freeDupArgv(argc, tg2_argv);
            freeDupArgv(ts_arg[_TG_1_]._argc, ts_arg[_TG_1_]._argv);
            freeDupArgv(ts_arg[_TG_2_]._argc, ts_arg[_TG_2_]._argv);
            }
            break;
        case THREE_SENSOR:
            {
                char **tg1_argv = NULL, **tg2_argv = NULL, **tg3_argv = NULL;
                MUINT32 npPort = atoi(argv[CMD_IDX_ENABLEPORTS]);
                MUINT32 sttPort = atoi(argv[CMD_IDX_STTPORTS]);

                if (sensorNum != 3) {
                    MY_LOGD("switch cam need config three cam a time");
                    break;
                }
                if (sensorArray[_TG_1_]){
                    MY_LOGD("########CAM A###########");
                    dupArgv(&argv, &tg1_argv, argc);
                    if (tg1_argv == NULL) {
                        LOG_ERR("Error: dup argv fail\n");
                        break;
                    }
                    update_arg(tg1_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_1_]);
                    for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                        MY_LOGD("cmd_%d",atoi(tg1_argv[i]));
                    dupArgv(&tg1_argv, &ts_arg[_TG_1_]._argv, argc);
                    ts_arg[_TG_1_]._argc = argc;
                    ts_arg[_TG_1_].TG_Num = _TG_1_;
                    ts_arg[_TG_1_].pipeSel = EPipeSelect_Normal;
                    ts_arg[_TG_1_].bDCIF = dcifArray[_TG_1_];
                    ts_arg[_TG_1_].mIHalSensorList = pHalSensorList;
                    ts_arg[_TG_1_].custCropSize = customCropSize[_TG_1_];
                    ts_arg[_TG_1_].targetTG = TG_CAM_MAX;
                    updateFunc(ts_arg[_TG_1_].qryFunc, sensorArrayLocal[_TG_1_], q_func);
                    pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
                }

                // wait npipe finish config
                wait_npipe_config(ts_arg, _TG_1_);

                if (sensorArray[_TG_2_]){
                    MY_LOGD("#######CAM B###########");
                    dupArgv(&argv, &tg2_argv, argc);
                    if (tg2_argv == NULL) {
                        LOG_ERR("Error: dup argv fail\n");
                        break;
                    }
                    update_arg(tg2_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_2_]);
                    for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                        MY_LOGD("cmd_%d",atoi(tg2_argv[i]));
                    dupArgv(&tg2_argv, &ts_arg[_TG_2_]._argv, argc);
                    ts_arg[_TG_2_]._argc = argc;
                    ts_arg[_TG_2_].TG_Num = _TG_2_;
                    ts_arg[_TG_2_].pipeSel = EPipeSelect_Normal;
                    ts_arg[_TG_2_].bDCIF = dcifArray[_TG_2_];
                    ts_arg[_TG_2_].mIHalSensorList = pHalSensorList;
                    ts_arg[_TG_2_].custCropSize = customCropSize[_TG_2_];
                    ts_arg[_TG_2_].targetTG = TG_CAM_MAX;
                    updateFunc(ts_arg[_TG_2_].qryFunc, sensorArrayLocal[_TG_2_], q_func);
                    pthread_create(&CAM_B, NULL, test_camio_thread, &ts_arg[_TG_2_]);
                }

                // wait npipe finish config
                wait_npipe_config(ts_arg, _TG_2_);

                if (sensorArray[_TG_3_]){
                    MY_LOGD("#######CAM C###########");
                    dupArgv(&argv, &tg3_argv, argc);
                    if (tg3_argv == NULL) {
                        LOG_ERR("Error: dup argv fail\n");
                        break;
                    }

                    // simple raw c: rsso/lmvo/flko not supported
                    npPort &= ~(__RSSO_ENABLE|__EISO_ENABLE);
                    sttPort &= ~(__FLKO_ENABLE);

                    update_arg(tg3_argv, argc, CMD_IDX_SNRDEVID, sensorArray[_TG_3_]);
                    update_arg(tg3_argv, argc, CMD_IDX_ENABLEPORTS, npPort);
                    update_arg(tg3_argv, argc, CMD_IDX_STTPORTS, sttPort);
                    for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                        MY_LOGD("cmd_%d",atoi(tg3_argv[i]));
                    dupArgv(&tg3_argv, &ts_arg[_TG_3_]._argv, argc);
                    ts_arg[_TG_3_]._argc = argc;
                    ts_arg[_TG_3_].TG_Num = _TG_3_;
                    ts_arg[_TG_3_].pipeSel = EPipeSelect_Normal;
                    ts_arg[_TG_3_].bDCIF = dcifArray[_TG_3_];
                    ts_arg[_TG_3_].mIHalSensorList = pHalSensorList;
                    ts_arg[_TG_3_].custCropSize = customCropSize[_TG_3_];
                    ts_arg[_TG_3_].targetTG = TG_CAM_MAX;
                    updateFunc(ts_arg[_TG_3_].qryFunc, sensorArrayLocal[_TG_3_], q_func);
                    pthread_create(&CAM_C, NULL, test_camio_thread, &ts_arg[_TG_3_]);
                }

                // wait npipe finish config
                wait_npipe_config(ts_arg, _TG_3_);

                pthread_join(CAM_A, NULL);
                pthread_join(CAM_B, NULL);
                pthread_join(CAM_C, NULL);
                freeDupArgv(argc, tg1_argv);
                freeDupArgv(argc, tg2_argv);
                freeDupArgv(argc, tg3_argv);
                freeDupArgv(ts_arg[_TG_1_]._argc, ts_arg[_TG_1_]._argv);
                freeDupArgv(ts_arg[_TG_2_]._argc, ts_arg[_TG_2_]._argv);
                freeDupArgv(ts_arg[_TG_3_]._argc, ts_arg[_TG_3_]._argv);
            }
            break;

        case ONESENSOR_NORMALSV:{
            if (sensorNum != 1) {
                MY_LOGD("switch cam need config only one cam a time");
                break;
            }
            if (sensorArray[_TG_1_]){
                for(i = (MUINT32)CMD_IDX_SNRDEVID; i < (MUINT32)argc; i++)
                    MY_LOGD("cmd_%d",atoi(argv[i]));
                dupArgv(&argv, &ts_arg[_TG_1_]._argv, argc);
                ts_arg[_TG_1_]._argc = argc;
                ts_arg[_TG_1_].pipeSel = EPipeSelect_NormalSv;
                ts_arg[_TG_1_].bDCIF = MFALSE;
                ts_arg[_TG_1_].mIHalSensorList = pHalSensorList;
                ts_arg[_TG_1_].custCropSize = customCropSize[_TG_1_];
                ts_arg[_TG_1_].targetTG = TG_CAM_MAX;
                updateFunc(ts_arg[_TG_1_].qryFunc, sensorArrayLocal[_TG_1_], q_func);
                pthread_create(&CAM_A, NULL, test_camio_thread, &ts_arg[_TG_1_]);
            }

            // wait npipe finish config
            wait_npipe_config(ts_arg, _TG_1_);

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
                    WaitSttEnqBack(&ts_arg[_TG_1_]);
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
            freeDupArgv(ts_arg[_TG_1_]._argc, ts_arg[_TG_1_]._argv);
            }
            break;
        default:
            MY_LOGE("sensor number(%d) is not supported ",sensorNum);
            break;
    }

    return 0;
}
