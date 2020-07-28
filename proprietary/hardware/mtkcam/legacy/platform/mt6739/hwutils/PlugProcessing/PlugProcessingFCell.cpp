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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCam/HwUtils/PlugProcessingFCell"
//
#include "PlugProcessingFCell.h"
#include <mtkcam/hwutils/IPlugProcessing.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/hal/IHalSensor.h>
#include <fstream>

using namespace android;
using namespace NSCam;

#define PLUG_DEBUG_DUMP_PATH  "/sdcard/camera_dump"
#define BUFFER_DEFAULT_SIZE 3072+2
// Debug
#define XTALK_DATA  "sdcard/xtalk_otp.dat"
#define OTPDPC_DATA "sdcard/dpc_otp.dat"
#define OTPFDDPC_DATA "sdcard/fd_dpc_otp.dat"
/******************************************************************************
 *
 ******************************************************************************/
PLUGPROCESSING_CLASSNAME::
PLUGPROCESSING_CLASSNAME(MUINT32 id):
     mpInitFun(NULL),
     mpSendCommandFuc(NULL),
     mpDeinitFun(NULL),
     mDeviceId(-1),
     muEEPROMSize(0),
     mDebugQuerySensorData(0)
{
    FUNC_BGN;
    mDebugQuerySensorData = ::property_get_int32("debug.camera.4cell.querydata", 0);
    mOpenID = id; // must set OpenID
    mspDlFunc = new DlFunc("libcameracustom.plugin.so", mOpenID, mLogLevel); // set DlFunc if need
    //
    if (mspDlFunc != NULL && mspDlFunc->isValid()) {
#define getFuc(_funcName_, _funcType_, _funcPointer_) \
            do{                                                         \
                MVOID* getFn = NULL;\
                mspDlFunc->getFunction(_funcName_, &getFn);     \
                if(getFn != NULL)\
                    _funcPointer_ = reinterpret_cast<_funcType_>(getFn);\
                else                \
                    MY_LOGE("can't get func(%s)", _funcName_);   \
            }while(0)

        getFuc("plugfastening_init",          pfnInit_T,          mpInitFun);
        getFuc("plugfastening_sendCommand",   pfnSendCommand_T,   mpSendCommandFuc);
        getFuc("plugfastening_uninit",        pfnDeinit_T,        mpDeinitFun);
    }
    else MY_LOGW("dlopen fail");
    //
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
PLUGPROCESSING_CLASSNAME::
~PLUGPROCESSING_CLASSNAME()
{
    FUNC_BGN;
    //
    // add implementation here
    //
    FUNC_END;
};


/******************************************************************************
 * init function implementation
 ******************************************************************************/
MERROR
PLUGPROCESSING_CLASSNAME::
initImp(void)
{
    if(!mspDlFunc->isValid() || !mpInitFun )
    {
        MY_LOGE("init fail. check valid: Dpfunc(%d) InitFun(%p) ", mspDlFunc->isValid(), mpInitFun);
        return BAD_VALUE;
    }

    FUNC_BGN;
    MERROR res = OK;
    nsecs_t startTime = ::systemTime();
    //
    if(mDeviceId < 0) MY_LOGE("Need sendCommand IPlugProcessing::SET_PARAM to set OpenId!");
    if(muEEPROMSize == 0) {
        PlugStaticInfo info;
        mpSendCommandFuc(mDeviceId, NSCam::NSCamPlug::GET_PARAM, (MINTPTR)&info, 0, 0, 0);
        MY_LOGD("get eeprom size: %zu", info.eep_buf_size);
        muEEPROMSize = info.eep_buf_size;
    }

 #define addBuffer2Vector(v, type, size)                \
    do {                                                \
        bufferInfo info;                                \
        info.bufSize = size;                            \
        info.bufAddr = (char *)::malloc(size);          \
        if(info.bufAddr == NULL) {                      \
            MY_LOGE("alloc eeprom buffer fail");        \
            return NO_MEMORY;                           \
        }                                               \
        ::memset((char *) (info.bufAddr), 0, size);     \
        v.add(type, info);                              \
        MY_LOGD_IF(mLogLevel>=0, "add type(%s) buffer addr(%p) size(%zu) to vector.", #type, info.bufAddr, info.bufSize); \
    } while(0)

    addBuffer2Vector(mvWorkingBuffer, FOUR_CELL_CAL_TYPE_GAIN_TBL, BUFFER_DEFAULT_SIZE);
    addBuffer2Vector(mvWorkingBuffer, FOUR_CELL_CAL_TYPE_DPC     , BUFFER_DEFAULT_SIZE);
    addBuffer2Vector(mvWorkingBuffer, FOUR_CELL_CAL_TYPE_FD_DPC  , BUFFER_DEFAULT_SIZE);

    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    if( !pIHalSensorList ) { MY_LOGE("pHalSensorList == NULL"); return DEAD_OBJECT; }
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor("LOG_TAG", mDeviceId);
    if( pIHalSensor == NULL ) { MY_LOGE("pIHalSensor is NULL"); return DEAD_OBJECT; }
    MY_LOGD_IF(mLogLevel>0, "DEBUG openid %d  queryId %d",mDeviceId, pIHalSensorList->querySensorDevIdx(mDeviceId));
    MINT sensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    SensorCropWinInfo cropInfo;
    pIHalSensor->sendCommand(
                    pIHalSensorList->querySensorDevIdx(mDeviceId),
                    SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                    (MUINTPTR)&sensorMode,
                    (MUINTPTR)&cropInfo,
                    0);
    mInitParam.full_w = cropInfo.full_w;
    mInitParam.full_h = cropInfo.full_h;
    mInitParam.img_w  = cropInfo.w0_size;
    mInitParam.img_h  = cropInfo.h0_size;
    mInitParam.offset_x  = cropInfo.x0_offset;
    mInitParam.offset_y  = cropInfo.y0_offset;
    MY_LOGD("winInfo full(%dx%d) img(%dx%d) offset(%d,%d)", mInitParam.full_w, mInitParam.full_h, mInitParam.img_w, mInitParam.img_h, mInitParam.offset_x, mInitParam.offset_y);
    if(mDebugQuerySensorData == 0) {
        unsigned char *pbuffer = nullptr;
        pbuffer = (unsigned char *)::malloc(BUFFER_DEFAULT_SIZE);
        if(pbuffer == nullptr)
        {
            MY_LOGE("malloc buffer fail");
        }
        else
        {
            for(int i=0; i< mvWorkingBuffer.size(); i++)
            {
                MINT32 bufferType = mvWorkingBuffer.keyAt(i);
                size_t bufferSize = mvWorkingBuffer.valueAt(i).bufSize;

                struct FourCellSensorInfo sSensor4CellInfo;
                ::memset((unsigned char *) (pbuffer), 0, BUFFER_DEFAULT_SIZE);
                MY_LOGD("alloc temp buffer bufferType(%d) addr(%p) size(%zu)",bufferType, pbuffer, bufferSize);
                pIHalSensor->sendCommand( pIHalSensorList->querySensorDevIdx(mDeviceId), SENSOR_CMD_GET_4CELL_SENSOR, (MUINTPTR)&bufferType, (MUINTPTR)&pbuffer, (MUINTPTR)&bufferSize);
                MY_LOGD("query sensor type(%d) addr(%p) size(%zu).", bufferType, pbuffer, bufferSize);
                if(mvWorkingBuffer.valueAt(i).bufSize < (bufferSize)){
                     MY_LOGE("buffer size not match: return size(%zu+2) need smaller than alloc size(%zu) ", bufferSize, mvWorkingBuffer.valueAt(i).bufSize);
                     continue;
                }
                sSensor4CellInfo.iNum = (pbuffer[0] | (pbuffer[1] <<8));
                sSensor4CellInfo.iBuffer= (unsigned char *)mvWorkingBuffer.valueAt(i).bufAddr;
                ::memcpy(sSensor4CellInfo.iBuffer, pbuffer+2, bufferSize-2);
                MY_LOGD("memcpy dst addr(%p) src addr(%p) size(%zu) num(%d).",sSensor4CellInfo.iBuffer, pbuffer+2, bufferSize, sSensor4CellInfo.iNum);
                //
                switch (bufferType)
                {
                    case FOUR_CELL_CAL_TYPE_GAIN_TBL:
                    {
                       mInitParam.eep_buf_size = sSensor4CellInfo.iNum;
                       mInitParam.eep_buf_addr = (void*)sSensor4CellInfo.iBuffer;
                       break;
                    }
                    case FOUR_CELL_CAL_TYPE_DPC:
                    {
                        mInitParam.dpc_num = sSensor4CellInfo.iNum;
                        mInitParam.dpc_addr = (void*)sSensor4CellInfo.iBuffer;
                        break;
                    }
                    case FOUR_CELL_CAL_TYPE_FD_DPC:
                    {
                        mInitParam.fd_dpc_num = sSensor4CellInfo.iNum;
                        mInitParam.fd_dpc_addr = (void*)sSensor4CellInfo.iBuffer;
                        break;
                    }
                    default:
                    {
                        MY_LOGE("not defined type.");
                        break;
                    }
                }
            }
            ::free(pbuffer);
        }
    } else {
        if(mDebugQuerySensorData & 0x01) {
            MY_LOGI("DEBUG : read gain table from sdcard");
            bufferInfo& info = mvWorkingBuffer.editValueFor(FOUR_CELL_CAL_TYPE_GAIN_TBL);
            char *pbuffer= info.bufAddr;
            int file_fd = open(XTALK_DATA, O_RDONLY);
            if(file_fd < 0){
                MY_LOGD("EEPROM data file open failed %s with rc = %d \n",strerror(errno), file_fd);
            }else{
                MY_LOGD("EEPROM data file open succeed, shared fd=%d", file_fd);
            }
            read(file_fd, pbuffer, info.bufSize);
            close(file_fd);
            info.bufSize = muEEPROMSize;
            mInitParam.eep_buf_size =  muEEPROMSize;
            mInitParam.eep_buf_addr = (void*)pbuffer;
        } else {
            mInitParam.eep_buf_size =  0;
            mInitParam.eep_buf_addr =  nullptr;
        }

        if(mDebugQuerySensorData & 0x02) {
            MY_LOGI("DEBUG: read dpc[2] from sdcard");
            FILE* odf = fopen(OTPDPC_DATA, "rb");
            fseek(odf, 0, SEEK_END);
            bufferInfo& info = mvWorkingBuffer.editValueFor(FOUR_CELL_CAL_TYPE_DPC);
            long file_size = ftell(odf);
            MY_LOGI("DEBUG: dpc ftell num: %d", file_size );
            fseek(odf, 0, SEEK_SET);
            char *pbuffer= info.bufAddr;
            fread(pbuffer, file_size , 1, odf );
            fclose(odf);
            info.bufSize = file_size;
            mInitParam.dpc_num =  file_size;
            mInitParam.dpc_addr = (void*)pbuffer;
        }  else {
            mInitParam.dpc_num =  0;
            mInitParam.dpc_addr =  nullptr;
        }

        if(mDebugQuerySensorData & 0x04) {
            MY_LOGI("DEBUG: read fd dpc[4] from sdcard");
            FILE* odf = fopen(OTPFDDPC_DATA, "rb");
            fseek(odf, 0, SEEK_END);
            bufferInfo& info = mvWorkingBuffer.editValueFor(FOUR_CELL_CAL_TYPE_FD_DPC);
            long file_size = ftell(odf);
            MY_LOGI("DEBUG: fd dpc ftell num: %d", file_size );
            fseek(odf, 0, SEEK_SET);
            char *pbuffer= info.bufAddr;
            fread(pbuffer, file_size , 1, odf );
            fclose(odf);
            info.bufSize = file_size;
            mInitParam.fd_dpc_num =  file_size;
            mInitParam.fd_dpc_addr = (void*)pbuffer;
        } else {
            mInitParam.fd_dpc_num =  0;
            mInitParam.fd_dpc_addr =  nullptr;
        }
    }
    pIHalSensor->destroyInstance(LOG_TAG);
    /*if(mLogLevel>=2) {
        if (!NSCam::Utils::makePath(PLUG_DEBUG_DUMP_PATH, 0660))
            MY_LOGW("makePath[%s] fails", PLUG_DEBUG_DUMP_PATH);
        char filename[100];
        for(int i=0; i< mvWorkingBuffer.size(); i++)
        {
            MINT32 bufferType = mvWorkingBuffer.keyAt(i);
            size_t bufferSize = mvWorkingBuffer.valueAt(i).bufSize;
            if(bufferSize == 0) continue;
            const char *pbuf= mvWorkingBuffer.valueAt(i).bufAddr;
            if(bufferType == FOUR_CELL_CAL_TYPE_GAIN_TBL)
                sprintf(filename, PLUG_DEBUG_DUMP_PATH "/gainTbl_%d.dat", bufferSize);
            else if(bufferType == FOUR_CELL_CAL_TYPE_DPC)
                sprintf(filename, PLUG_DEBUG_DUMP_PATH "/dpc_%d.dat", bufferSize);
            else if(bufferType == FOUR_CELL_CAL_TYPE_FD_DPC)
                sprintf(filename, PLUG_DEBUG_DUMP_PATH "/fd_dpc_%d.dat", bufferSize);

            std::ofstream out(filename);
            out.write(pbuf, bufferSize);
        }
   }*/
    CAM_TRACE_END();
    CAM_TRACE_BEGIN("REMOSAIC - GenData");

    //if(mLogLevel > 0) mInitParam.dump();
    if(!mpInitFun(mDeviceId, mInitParam))
        MY_LOGE("init fail.");
    CAM_TRACE_END();
    nsecs_t processTime = ::systemTime() - startTime;
    MY_LOGI("PlugRemosaic - init - %" PRId64 " ns", processTime);
    FUNC_END;
    return res;
};


/******************************************************************************
 * uninit function implementation
 ******************************************************************************/
MERROR
PLUGPROCESSING_CLASSNAME::
uninitImp(void)
{
    if(!mspDlFunc->isValid() || !mpDeinitFun) return BAD_VALUE;
    FUNC_BGN;
    MERROR res = OK;
    nsecs_t startTime = ::systemTime();
    //
    for(int i=0; i< mvWorkingBuffer.size(); i++)
    {
        char *pbuffer= mvWorkingBuffer.valueAt(i).bufAddr;
        ::free(pbuffer);
        pbuffer = NULL;
    }
    mvWorkingBuffer.clear();
    // add implementation here

    mpDeinitFun(mDeviceId);
    nsecs_t processTime = ::systemTime() - startTime;
    MY_LOGI("PlugRemosaic - uninit - %" PRId64 " ns", processTime);
    FUNC_END;
    return res;
};


/******************************************************************************
 * send command function implementation
 ******************************************************************************/
MERROR
PLUGPROCESSING_CLASSNAME::
sendCommandImp(MINT32 const cmd,
    MINTPTR arg1, MINTPTR arg2, MINTPTR arg3, MINTPTR arg4)
{
    FUNC_BGN;
    MY_LOGI_IF(mLogLevel > 0, "CMD[%d]"
            "(%" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ")",
            cmd, arg1, arg2, arg3, arg4);

    if(cmd == NSCam::NSCamPlug::ACK)
    {
        MBOOL *pbRet = (MBOOL *)arg1;
        *pbRet = mspDlFunc->isValid();
        return OK;
    }
    //
    if(!mspDlFunc->isValid()  || !mpSendCommandFuc) return BAD_VALUE;

    MERROR res = OK;
    //
    switch (cmd)
    {
        case NSCam::NSCamPlug::GET_PARAM:
        {
            break;
        }
        case NSCam::NSCamPlug::SET_PARAM:
        {
            PARAM_TYPE _type = (PARAM_TYPE)arg1;

            if( _type == PARAM_INIT ) {
                PlugInitParam param = *(PlugInitParam*)arg2;
                mDeviceId = param.openId;
                mInitParam.openId = param.openId;
                mInitParam.img_w  = param.img_w;
                mInitParam.img_h  = param.img_h;
                MY_LOGD("openId(%d) img(%dx%d)", mDeviceId, mInitParam.img_w, mInitParam.img_h);
            } else if ( _type == PARAM_PROCESSING ) {
                PlugProcessingParam param = *(PlugProcessingParam*)arg2;
                mProcessingParam = param;
            }
            break;
        }
        case NSCam::NSCamPlug::PROCESS: //process
        {
            CAM_TRACE_BEGIN("REMOSAIC - processing");
            nsecs_t startTime = ::systemTime();
            NSCam::PlugProcessingParam _param = *(NSCam::PlugProcessingParam*)arg1;
            //if(mLogLevel > 0) _param.dump();
            if(mpSendCommandFuc(mDeviceId, NSCam::NSCamPlug::PROCESS,
                                arg1, arg2, arg3, arg4) == false)
                res = BAD_VALUE;
            nsecs_t processTime = ::systemTime() - startTime;
            MY_LOGI("Plugremosaic - processing -  %" PRId64 "ns", processTime);
            CAM_TRACE_END();
            break;
        }
        default:
        {
            MY_LOGE("not defined cmd.");
            break;
        }
    }
    FUNC_END;
    return res;
};


