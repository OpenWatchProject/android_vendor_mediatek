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
#define LOG_TAG "PDTblGen"

#include <functional>
#include <algorithm>
#include <map>
#include <iostream>
#include <sys/stat.h>

//
#include "private/PDTblGen.h"

// sensor information
#include <kd_imgsensor_define.h>
#include <mtkcam/drv/IHalSensor.h>

// log
#include <mtkcam/utils/std/Log.h>

// hw buffer
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>

// isp information
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

//
#include <pd_buf_common.h>

// property
#include <cutils/properties.h>

using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;


class PDTblGenImpl : public IPDTblGen
{
    enum PD_TYPE
    {
        PD_TYPE_L,
        PD_TYPE_R,
        PD_TYPE_NUM
    };

    struct point
    {
        unsigned int x;
        unsigned int y;
        PD_TYPE type;

        point() :
            x(0),
            y(0),
            type(PD_TYPE_NUM)
        {}

        point(unsigned int _x, unsigned int _y, PD_TYPE _type) :
            x(_x),
            y(_y),
            type(_type)
        {}

        point& operator =(point &_in)
        {
            x=_in.x;
            y=_in.y;
            type=_in.type;
            return (*this);
        }
        bool   operator <=(point &_in)
        {
            return ((y<_in.y)||(y==_in.y && x<_in.x));
        }
    };

    struct TblBuf
    {
        TblInfo tblInfo;
        IImageBuffer *tblmem;

        TblBuf() :
            tblInfo(),
            tblmem(nullptr)
        {}

        TblBuf& operator =(TblBuf &_in)
        {
            tblInfo  = _in.tblInfo;
            tblmem   = _in.tblmem;
            return (*this);
        }

        TblBuf& operator =(TblBuf _in)
        {
            tblInfo  = _in.tblInfo;
            tblmem   = _in.tblmem;
            return (*this);
        }

        TblBuf( TblInfo &_tblInfo, IImageBuffer *_tblmem)
        {
            tblInfo = _tblInfo;
            tblmem  = _tblmem;
        }

    };

    struct TblData
    {
        TblBuf full;
        TblBuf bin;

        TblData() :
            full(),
            bin()
        {}

    };


private:
    int mDgbLogLv;
    map<int, map<int, TblData>> mTblBufMap;
    IHalSensor     *mIHalSensor;
    IHalSensorList *mIHalSensorList;
    mutable Mutex mLock;

    /**
     * @Brief :
     *          Arrary of PD pixels' coordinate is sorted by ascendant order.
     * @Param :
     *          [ in] src   : source arrary of pd pixels' coordinate.
     *          [ in] begin : define the begin index of sorting range in source arrary.
     *          [ in] end   : define the end index of sorting range in source arrary.
     *          [ in] dst   : destination arrary.
     * @Return:
     *          [out] NA.
     */
    void mergeSort( point *src,
                    unsigned char begin,
                    unsigned char end,
                    point *dst);

    /**
     * @Brief :
     *          Table generator.
     * @Param :
     *          [ in] imgSz      : describe the image size that is corresponding to following pd block information.
     *          [ in] numblkX    : describe how many repeated PD blocks in X axis.
     *          [ in] numblkY    : describe how many repeated PD blocks in Y axis.
     *          [ in] pitchX     : describe x size for one PD block.
     *          [ in] pitchY     : describe y size for one PD block.
     *          [ in] numPixs    : describe how many PD pixels in one PD block.
     *          [ in] sortedPixs : PD pixles' coordinate which is sorted by ascendant order.
     *          [out] outTblInfo : table information.
     * @Return:
     *          [out] true  : pd table is successfully generated.
     *                false : pd table can not be generated because of HW constraint.
     */
    bool generateTbl( MSize imgSz,
                      unsigned short numblkX,
                      unsigned short numblkY,
                      unsigned short pitchX,
                      unsigned short pitchY,
                      unsigned short numPixs,
                      point *sortedPixs,
                      PDTblGenImpl::TblBuf &outTblInfo);
    /**
     * @Brief :
     *          Transform PD pixels' coordinate from 4:3 coordinate to current coordinate which is corresponding to sensor mode.
     *          The information, pd_info, is quired from sensor driver directly.
     * @Param :
     *          [ in] boundarySz : describe the size that is corresponding to following pd block information.
     *          [ in] pd_info    : PD block information.
     *          [ in] scenario   : Current sensor scenario, and it is used to query crop information.
     *          [ in] numPixs    : Describe how many PD pixels in one PD block.
     *          [out] outAllPixs : Output PD pixles' coordinate which is applied crop, mirror and flip information.
     * @Return:
     *          [out] true  : pd_info information is correct
     *                false : pd_info information is not correct. Please check sensor dirver setting.
     */
    bool transformCoordinate( MSize boundarySz,
                              SET_PD_BLOCK_INFO_T &pd_info,
                              int const scenario,
                              unsigned short numPixs,
                              point *outAllPixs);

    /**
     * @Brief :
     *          parsing pd table to getting pdo HW related setting and verifying pd table is correct.
     * @Param :
     *          [ in] img_w        : the image width that is used to generate the table(3rd input parameter).
     *          [ in] img_h        : the image height that is used to generate the table(3rd input parameter).
     *          [ in] tbl_va       : pd table
     *          [ in] tbl_sz       : the pd table size in BYTE.
     *          [ in] pTblFilePath : file path for dumping data.
     * @Return:
     *          [out] true  : table is correct
     *                false : table is not correct.
     */
    bool pd_tbl_reader( unsigned short img_w,
                        unsigned short img_h,
                        unsigned char *tbl_va,
                        unsigned short tbl_sz,
                        char *pTblFilePath);

public:
    /**
     * @Brief :
     *          Default constructor
     * @Param :
     *          [ in] NA.
     * @Return:
     *          [out] NA.
     */
    PDTblGenImpl();

    /**
     * @Brief :
     *          Default destructor
     * @Param :
     *          [ in] NA.
     * @Return:
     *          [out] NA.
     */
    virtual ~PDTblGenImpl();

    /**
     * @Brief :
     *          Interface : Start pd table generate utility
     * @Param :
     *          [ in] sensorDev : It is quired from sensor driver by using IHalSensorList API.
     *          [ in] sensorIdx : provide by MW
     * @Return:
     *          [out] status.
     */
    virtual bool start( int const sensorDev, int const sensorIdx);

    /**
     * @Brief :
     *          Interface : Stop pd table generate utility
     * @Param :
     *          [ in] sensorDev.
     * @Return:
     *          [out] status.
     */
    virtual bool stop( int const sensorDev);

    /**
     * @Brief :
     *          Interface : get BPCI tables for frontal binning module is disable and enable.
     * @Param :
     *          [ in] sensorDev
     *          [ in] scenario
     *          [out] outTbl : tables for disabling and enabling frontal binning module.
     * @Return:
     *          [out] status.
     */
    virtual bool getTbl( int const sensorDev, int const scenario, Tbl &outTbl);
};

/******************************************************************************
 *
 ******************************************************************************/
static INormalPipeModule* getNormalPipeModule()
{
    static auto pModule = INormalPipeModule::get();
    CAM_LOGE_IF(!pModule, "INormalPipeModule::get() fail");
    return pModule;
}

static MVOID* createDefaultNormalPipe(MUINT32 sensorIndex, char const* szCallerName)
{
    auto pModule = getNormalPipeModule();
    if  ( ! pModule )
    {
        CAM_LOGE("getNormalPipeModule() fail");
        return NULL;
    }

    //  Select CamIO version
    size_t count = 0;
    MUINT32 const* version = NULL;
    int err = pModule->get_sub_module_api_version(&version, &count, sensorIndex);
    if  ( err < 0 || ! count || ! version )
    {
        CAM_LOGE(
            "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
            sensorIndex, err, count, version
        );
        return NULL;
    }

    MUINT32 const selected_version = *(version + count - 1); //Select max. version
    CAM_LOGD("[%d] count:%zu Selected CamIO Version:%0#x", sensorIndex, count, selected_version);

    MVOID* pPipe = NULL;
    pModule->createSubModule(sensorIndex, szCallerName, selected_version, (MVOID**)&pPipe);
    return pPipe;
}

/******************************************************************************
 *                                  IPDTblGen
 ******************************************************************************/
IPDTblGen* IPDTblGen::getInstance()
{
    static PDTblGenImpl singleton;
    return &singleton;
}

/******************************************************************************
 *                                 PDTblGenImpl
 ******************************************************************************/
PDTblGenImpl::PDTblGenImpl() :
    mDgbLogLv(0),
    mIHalSensor(nullptr),
    mIHalSensorList(nullptr)
{}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PDTblGenImpl::~PDTblGenImpl()
{
    //buffer are distroyed when cameraserver is destroyed.
    for( map<int, map<int, TblData>>::iterator itrBufMap=mTblBufMap.begin(); itrBufMap!=mTblBufMap.end(); itrBufMap++)
    {
        for( map<int, TblData>::iterator itrTblData=itrBufMap->second.begin(); itrTblData!=itrBufMap->second.end(); itrTblData++)
        {
            // clean all stored tables when camera service is destroyed.
            std::string strName = LOG_TAG;
            if(itrTblData->second.full.tblmem)
            {
                // unlock buffer until free buffer
                itrTblData->second.full.tblmem->unlockBuf(strName.c_str());
                sp<IImageBuffer> spfulltbl = itrTblData->second.full.tblmem;

                itrTblData->second.full.tblmem = nullptr;
                memset(&(itrTblData->second.full.tblInfo), 0, sizeof(TblInfo));
            }

            if(itrTblData->second.bin.tblmem)
            {
                itrTblData->second.bin.tblmem->unlockBuf(strName.c_str());
                sp<IImageBuffer> spbintbl  = itrTblData->second.bin.tblmem;

                itrTblData->second.bin.tblmem = nullptr;
                memset(&(itrTblData->second.bin.tblInfo), 0, sizeof(TblInfo));
            }
        }
        // clean all tables when camera service is destroyed.
        itrBufMap->second.clear();
    }
    mTblBufMap.clear();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool PDTblGenImpl::start( int const sensorDev, int const sensorIdx)
{
    Mutex::Autolock lock( mLock);

    bool ret=false;

    mDgbLogLv = property_get_int32("vendor.debug.pdinfo.enable", 0);

    // get IHalSensor instance
    mIHalSensorList = MAKE_HalSensorList();

    //
    if( mIHalSensorList)
    {
        MUINT _sensorDev = mIHalSensorList->querySensorDevIdx(sensorIdx);

        if(_sensorDev==(int)(sensorDev))
        {
            // query pd sensor type
            SensorStaticInfo sensorStaticInfo;
            mIHalSensorList->querySensorStaticInfo( sensorDev, &sensorStaticInfo);
            IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM pdaf_support = (IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM)sensorStaticInfo.PDAF_Support;

            if( /* checked pd sensor type */
                pdaf_support==PDAF_SUPPORT_RAW ||
                pdaf_support==PDAF_SUPPORT_CAMSV ||
                pdaf_support==PDAF_SUPPORT_CAMSV_LEGACY ||
                pdaf_support==PDAF_SUPPORT_RAW_LEGACY)
            {
                //get pd buffer type from custom setting.
                MUINT32 pdBufType = GetPDBuf_Type( sensorDev, sensorStaticInfo.sensorDevID);

                CAM_LOGD("dev[%d] sensor ID(0x%x) pd buf type(0x%x) pd sensor type(%d)",
                         sensorDev,
                         sensorStaticInfo.sensorDevID,
                         pdBufType,
                         pdaf_support);

                if( /* checking supporting pd buffer type */
                    ((pdBufType&MSK_CATEGORY_OPEN  )==0) &&
                    ((pdBufType&MSK_CATEGORY_DUALPD)==0))
                {
                    if(mIHalSensor==nullptr)
                    {
                        mIHalSensor = mIHalSensorList->createSensor( LOG_TAG, sensorDev);
                    }

                    // HalSensor must be successuflly created, because pd block information must be queried from sensor driver.
                    if( mIHalSensor)
                    {
                        // find device. If devices is not found, add a new device
                        if(mTblBufMap.find(sensorDev)!= mTblBufMap.end())
                        {
                            CAM_LOGD("dev[%d] pdtbl is already started\n", sensorDev);
                        }
                        else
                        {
                            mTblBufMap.insert( pair<int, map<int, TblData>>(sensorDev, map<int, TblData>()));
                            CAM_LOGD("dev[%d] pdtbl is starting\n", sensorDev);
                        }

                        ret = true;
                    }
                    else
                    {
                        ret = false;
                        CAM_LOGE( "dev[%d] Fail to create instance IHalSensor(%p)", sensorDev, mIHalSensor);
                    }
                }
                else
                {
                    ret = false;
                    CAM_LOGD( "dev[%d] Not supported pd buffer type(0x%x)", sensorDev, pdBufType);
                }
            }
            else
            {
                ret = false;
                CAM_LOGD( "dev[%d] Not supported pd sensor type(%d)", sensorDev, pdaf_support);
            }
        }
        else
        {
            ret = false;
            CAM_LOGD( "input sensor dev/idx error. dev:%d/%d, idx:%d", sensorDev, _sensorDev, sensorIdx);

        }
    }
    else
    {
        ret = false;
        CAM_LOGE( "dev[%d] Get HalSensorList fail %p", sensorDev, mIHalSensorList);
    }



    if( ret==false)
    {
        CAM_LOGD( "dev[%d] module is not started!!", sensorDev);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool PDTblGenImpl::stop( int const sensorDev)
{
    Mutex::Autolock lock( mLock);

    if( mIHalSensor)
    {
        mIHalSensor->destroyInstance(LOG_TAG);
        mIHalSensor     = nullptr;
        mIHalSensorList = nullptr;
    }

    CAM_LOGD( "dev[%d] stop module", sensorDev);
    return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool PDTblGenImpl::getTbl( int const sensorDev, int const scenario, Tbl &outTbl)
{
    Mutex::Autolock lock( mLock);

    bool ret = false;
    memset( &outTbl, 0, sizeof(Tbl));

    /*******************************************
     * Try to find tables' information from data struct.
     * If table is not found, generating tables for enabling and disabling frontal binning mode.
     * After tables are generated, talbes are stored by camera scenario.
     *******************************************/
    map<int, map<int, TblData>>::iterator itrDev = mTblBufMap.find(sensorDev);
    if( itrDev==mTblBufMap.end())
    {
        CAM_LOGW("[%s] dev(%d) scenario(%d) device is not started, no tables can be output. Please module should be started before getting table\n", __FUNCTION__, sensorDev, scenario);
    }
    else
    {
        map<int, TblData>::iterator itrTbl = itrDev->second.find(scenario);

        if( itrTbl==itrDev->second.end())
        {
            itrDev->second.insert( pair<int, TblData>(scenario, TblData()));

            if(mIHalSensor)
            {
                // Get pd block information from sensor driver.
                SET_PD_BLOCK_INFO_T pd_info;
                memset( &pd_info, 0, sizeof(SET_PD_BLOCK_INFO_T));
                mIHalSensor->sendCommand( sensorDev, SENSOR_CMD_GET_SENSOR_PDAF_INFO, (MINTPTR)&scenario, (MINTPTR)&pd_info, 0);

                /*******************************************
                 * mapping sensor index
                 *******************************************/
                unsigned int numberOfSensors = mIHalSensorList->queryNumberOfSensors();
                unsigned int curSensorIdx    = numberOfSensors;
                for(unsigned int i=0; i<numberOfSensors; i++)
                {
                    if( sensorDev == (int)(mIHalSensorList->querySensorDevIdx(i)))
                    {
                        curSensorIdx = i;
                    }
                }

                if( /* checking result of mapping sensor index */
                    (0<=curSensorIdx) &&
                    (curSensorIdx<=(numberOfSensors-1)))
                {
                    // get normalPipe instance and query current tg size
                    INormalPipe* const normalPipe = (INormalPipe*)createDefaultNormalPipe( curSensorIdx, LOG_TAG);
                    if(normalPipe)
                    {
                        bool res;

                        //
                        MSize tg_sz, bin_sz;
                        res = normalPipe->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)(&tg_sz.w), (MINTPTR)(&tg_sz.h), 0);
                        res = normalPipe->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO, (MINTPTR)(&bin_sz.w), (MINTPTR)(&bin_sz.h), 0);
                        normalPipe->destroyInstance( LOG_TAG);

                        //
                        SensorCropWinInfo sensorCropInfo;
                        MINT32 err = mIHalSensor->sendCommand( sensorDev, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO, (MUINTPTR)&(scenario)/*input*/, (MUINTPTR)&(sensorCropInfo)/*Output*/, 0);

                        //
                        if( /* */
                            (sensorCropInfo.w2_tg_size == tg_sz.w) &&
                            (sensorCropInfo.h2_tg_size == tg_sz.h) &&
                            (0<tg_sz.w) &&
                            (0<tg_sz.h) &&
                            (0<bin_sz.w) &&
                            (0<bin_sz.h) &&
                            (err==0 /* 0=SENSOR_NO_ERROR */))
                        {
                            /*******************************************
                             * Table is not calculated in this scenario.
                             * Both full size and binning size tables are calculated.
                             *******************************************/
                            CAM_LOGD("[%s] dev(%d) scenario(%d) generating table. tg_sz(%d,%d) cur_bin_sz(%d,%d)\n", __FUNCTION__, sensorDev, scenario, tg_sz.w, tg_sz.h, bin_sz.w, bin_sz.h);

                            //
                            unsigned short numPair    = pd_info.i4PairNum;
                            unsigned short numAllPixs = 2*numPair;

                            // allocate resource
                            point *allPixs = new point[numAllPixs];
                            point *allPixsSorted = new point[numAllPixs];

                            // 1. coordinate pre-processing and checking PD block information
                            ret = transformCoordinate( tg_sz, pd_info, scenario, numAllPixs, allPixs);
                            if( ret)
                            {
                                //2. sorting pd pixels' coordinate
                                memcpy( allPixsSorted, allPixs, sizeof(point)*numAllPixs); // initial working buffer
                                mergeSort(allPixs, 0, numAllPixs, allPixsSorted);

                                if( mDgbLogLv)
                                {
                                    for(unsigned int i=0; i<numAllPixs; i++)
                                    {
                                        CAM_LOGD("[%s] %2d, x:%3d, y:%3d, type:%d\n", __FUNCTION__, i, allPixsSorted[i].x,  allPixsSorted[i].y,  allPixsSorted[i].type);
                                    }
                                }

                                // 3. generating full table
                                ret = generateTbl( tg_sz,
                                                   pd_info.i4BlockNumX,
                                                   pd_info.i4BlockNumY,
                                                   pd_info.i4PitchX,
                                                   pd_info.i4PitchY,
                                                   numAllPixs,
                                                   allPixsSorted,
                                                   itrDev->second[scenario].full);

                                if(ret)
                                {
                                    // 4. calculating pd coordinator when frontal binning is enabled
                                    point *allPixsBin = allPixs;
                                    point *allPixsSortedBin = allPixsSorted;
                                    for( unsigned short i=0; i<numAllPixs; i++)
                                    {
                                        unsigned int  xdis = allPixsSorted[i].x+1;
                                        unsigned int  xgup = xdis>>2;
                                        unsigned int  xoff = xdis-(xgup<<2);
                                        unsigned int _xdis = (xgup<<1) + ( xoff ? (xoff-1)%2+1 : 0);

                                        unsigned int  ydis = allPixsSorted[i].y+1;
                                        unsigned int  ygup = ydis>>2;
                                        unsigned int  yoff = ydis-(ygup<<2);
                                        unsigned int _ydis = (ygup<<1) + ( yoff ? (yoff-1)%2+1 : 0);

                                        point binCoordinate( _xdis-1, _ydis-1, allPixsSorted[i].type);
                                        allPixsBin[i] = binCoordinate;
                                    }

                                    memcpy( allPixsSortedBin, allPixsBin, sizeof(point)*numAllPixs); // initial working buffer
                                    mergeSort(allPixsBin, 0, numAllPixs, allPixsSortedBin);

                                    if( mDgbLogLv)
                                    {
                                        for(unsigned int i=0; i<numAllPixs; i++)
                                        {
                                            CAM_LOGD("[%s] %2d, x:%3d, y:%3d, type:%d\n", __FUNCTION__, i, allPixsSortedBin[i].x,  allPixsSortedBin[i].y,  allPixsSortedBin[i].type);
                                        }
                                    }

                                    // 5. generating binning table
                                    ret = generateTbl( MSize(tg_sz.w>>1, tg_sz.h>>1),
                                                       pd_info.i4BlockNumX,
                                                       pd_info.i4BlockNumY,
                                                       pd_info.i4PitchX>>1,
                                                       pd_info.i4PitchY>>1,
                                                       numAllPixs,
                                                       allPixsSortedBin,
                                                       itrDev->second[scenario].bin);

                                    if(ret)
                                    {
                                        // output
                                        outTbl.tbl      = itrDev->second[scenario].full.tblInfo;
                                        outTbl.tbl_bin  = itrDev->second[scenario].bin.tblInfo;
                                        outTbl.pbuf     = itrDev->second[scenario].full.tblmem;
                                        outTbl.pbuf_bin = itrDev->second[scenario].bin.tblmem;
                                    }
                                    else
                                    {
                                        std::string strName = LOG_TAG;
                                        if(itrDev->second[scenario].full.tblmem)
                                        {
                                            itrDev->second[scenario].full.tblmem->unlockBuf(strName.c_str());
                                            sp<IImageBuffer> spfulltbl = itrDev->second[scenario].full.tblmem;

                                            itrDev->second[scenario].full.tblmem = nullptr;
                                            memset(&(itrDev->second[scenario].full.tblInfo), 0, sizeof(TblInfo));
                                        }

                                        if(itrDev->second[scenario].bin.tblmem)
                                        {
                                            itrDev->second[scenario].bin.tblmem->unlockBuf(strName.c_str());
                                            sp<IImageBuffer> spbintbl  = itrDev->second[scenario].bin.tblmem;

                                            itrDev->second[scenario].bin.tblmem  = nullptr;
                                            memset(&(itrDev->second[scenario].bin.tblInfo), 0, sizeof(TblInfo));
                                        }
                                    }
                                }
                            }
                            else
                            {
                                CAM_LOGE("[%s] pre-processing PD coordinate is fail. Please checking previous error log.\n", __FUNCTION__);
                            }

                            // delete resource
                            delete []allPixs;
                            delete []allPixsSorted;

                        }
                        else
                        {
                            ret = false;
                            CAM_LOGE( "[%s] dev[%d] tg size is not match:p1_tg_info(%d/%d),sensor_crop_info(%d/%d). Please check sensor driver setting and timing of getting getTbl and module starting. ",
                                      __FUNCTION__,
                                      sensorDev,
                                      tg_sz.w,
                                      tg_sz.h,
                                      sensorCropInfo.w2_tg_size,
                                      sensorCropInfo.h2_tg_size);
                        }

                    }
                    else
                    {
                        ret = false;
                        CAM_LOGE( "[%s] dev[%d] Fail to create instance normalPipe(%p)", __FUNCTION__, sensorDev, normalPipe);
                    }
                }
                else
                {
                    ret = false;
                    CAM_LOGE( "[%s] dev[%d] Fail to mapping sensor index", __FUNCTION__, sensorDev);
                }
            }
            else
            {
                CAM_LOGW("[%s] dev(%d) scenario(%d) pdtbl is not started can not get IHalSensor to get pd block info. Please check initial flow in 3A framework.\n", __FUNCTION__, sensorDev, scenario);
            }
        }
        else
        {
            if( itrDev->second[scenario].full.tblmem && itrDev->second[scenario].bin.tblmem)
            {
                outTbl.tbl     = itrDev->second[scenario].full.tblInfo;
                outTbl.tbl_bin = itrDev->second[scenario].bin.tblInfo;
                outTbl.pbuf     = itrDev->second[scenario].full.tblmem;
                outTbl.pbuf_bin = itrDev->second[scenario].bin.tblmem;

                //
                ret = true;
                CAM_LOGD_IF(mDgbLogLv, "[%s] dev(%d) scenario(%d) Read from data base directly\n", __FUNCTION__, sensorDev, scenario);
            }
            else
            {
                ret = false;
                CAM_LOGE("[%s] dev(%d) scenario(%d) : Table is not generated. Please check error log during generating table (1st time lunch sensor mode(%d))\n", __FUNCTION__, sensorDev, scenario, scenario);
            }
        }
    }


    // debug
    if(ret)
    {
        if(mDgbLogLv)
        {
            FILE *fp = fopen("/sdcard/pdtbl/pdtbl_info", "w");
            if( NULL == fp)
            {
                MINT32 err = mkdir( "/sdcard/pdtbl", S_IRWXU | S_IRWXG | S_IRWXO);

                if( 0!=err && EEXIST!=errno )
                {
                    CAM_LOGW("[DUMP] fail to create folder /sdcard/pdtbl : %d[%s]", errno, ::strerror(errno));
                }
                else
                {
                    CAM_LOGD("[DUMP] create folder /sdcard/pdtbl (status:%d)", err);
                }
            }
            else
            {
                fprintf( fp, "folder /sdcard/pdtbl is exist");
                fclose( fp);
            }

            //
#define DBG_DUMP_FILE_NAME_SIZE 1023
            char fileName[DBG_DUMP_FILE_NAME_SIZE];

            //
            {
                int cnt = 0;
                cnt = snprintf(fileName,
                               DBG_DUMP_FILE_NAME_SIZE,
                               "/sdcard/pdtbl/pd_tbl_dev_%d_scenario_%d_imgSz_%d_%d_tblSz_%d_full",
                               sensorDev,
                               scenario,
                               outTbl.tbl.img_xsz,
                               outTbl.tbl.img_ysz,
                               (outTbl.tbl.tbl_xsz+1));
                fp = fopen(fileName, "w");
                if( fp)
                {
                    CAM_LOGD( "[DUMP] dump pd table for non-bin size raw as file:%s", fileName);
                    fwrite( reinterpret_cast<void *>(outTbl.tbl.tbl_va), 1, (outTbl.tbl.tbl_xsz+1), fp);
                    fclose( fp);
                }

                pd_tbl_reader( outTbl.tbl.img_xsz, outTbl.tbl.img_ysz, reinterpret_cast<unsigned char*>(outTbl.tbl.tbl_va), (unsigned short)(outTbl.tbl.tbl_xsz+1), fileName);
            }

            {
                int cnt = 0;
                cnt = snprintf(fileName,
                               DBG_DUMP_FILE_NAME_SIZE,
                               "/sdcard/pdtbl/pd_tbl_dev_%d_scenario_%d_imgSz_%d_%d_tblSz_%d_bin",
                               sensorDev,
                               scenario,
                               outTbl.tbl_bin.img_xsz,
                               outTbl.tbl_bin.img_ysz,
                               (outTbl.tbl_bin.tbl_xsz+1));
                fp = fopen(fileName, "w");
                if( fp)
                {
                    CAM_LOGD( "[DUMP] dump pd table for bin size raw as file:%s", fileName);
                    fwrite( reinterpret_cast<void *>(outTbl.tbl_bin.tbl_va), 1, (outTbl.tbl_bin.tbl_xsz+1), fp);
                    fclose( fp);
                }

                pd_tbl_reader( outTbl.tbl_bin.img_xsz, outTbl.tbl_bin.img_ysz, reinterpret_cast<unsigned char*>(outTbl.tbl_bin.tbl_va), (unsigned short)(outTbl.tbl_bin.tbl_xsz+1), fileName);
            }
        }
    }

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
PDTblGenImpl::mergeSort( point *src,
                         unsigned char begin,
                         unsigned char end,
                         point *dst)
{
    if(end - begin < 2)
        return;

    unsigned char middle = (begin + end) / 2;

    // recursively sort
    mergeSort(dst, begin,  middle, src);
    mergeSort(dst, middle,    end, src);

    // merge the resulting
    unsigned char i = begin, j = middle;
    for(unsigned char k = begin; k < end; k++)
    {
        if(i<middle && (j>=end || src[i]<=src[j]))
        {
            dst[k] = src[i];
            i = i + 1;
        }
        else
        {
            dst[k] = src[j];
            j = j + 1;
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
PDTblGenImpl::generateTbl( MSize imgSz,
                           unsigned short numblkX,
                           unsigned short numblkY,
                           unsigned short pitchX,
                           unsigned short pitchY,
                           unsigned short numPixs,
                           point *sortedPixs,
                           PDTblGenImpl::TblBuf &outTblInfo)
{
    bool ret = false;

    /*********************
     * calculate table size
     *********************/
    unsigned int tbl_xsz = 0;
    for(unsigned int curIdx=0, nxtIdx=0; curIdx<numPixs; )
    {
        // Define the index range of sorted pixels' array, and y coordinate is the same in this range
        for(nxtIdx=curIdx; nxtIdx<numPixs; nxtIdx++)
        {
            if(sortedPixs[nxtIdx].y==sortedPixs[curIdx].y)
                continue;
            else
                break;
        }

        CAM_LOGD("[%s] curIdx(%3d) nxtIdx(%3d) y_coord(%4d)\n", __FUNCTION__, curIdx, nxtIdx, sortedPixs[curIdx].y);

        // for each row
        tbl_xsz+=8;

        // for each PD in the row
        for(unsigned int i=curIdx; i<nxtIdx; i++)
            tbl_xsz+=2;

        // next pitch
        curIdx = nxtIdx;
    }
    tbl_xsz *= numblkY;

    CAM_LOGD("[%s] tbl_xsz(0x%x) numblk(%d,%d) pitch_sz(%d,%d)\n", __FUNCTION__, tbl_xsz, numblkX, numblkY, pitchX, pitchY);


    /*********************
     * create buffer
     *********************/
    TblInfo retInfo;
    IImageBuffer *retBuf = nullptr;

    unsigned int buf_Stride_InBytes[3]   = {tbl_xsz, 0, 0};
    unsigned int buf_Boundary_InBytes[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam bufParam((EImageFormat)eImgFmt_STA_BYTE, MSize(tbl_xsz, 1), buf_Stride_InBytes, buf_Boundary_InBytes, 1);

    std::string strName = LOG_TAG;
    sp<IIonImageBufferHeap> pHeap = IIonImageBufferHeap::create(strName.c_str(), bufParam);

    if( pHeap!=nullptr)
    {
        retBuf = pHeap->createImageBuffer();

        if( retBuf!=nullptr)
        {
            // lock buffer : Before access buffer, buffer should be locked.
            if( retBuf->lockBuf(strName.c_str(), (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_HW_CAMERA_WRITE)))
            {
                //
                unsigned short min_pd_area_x = 0xFFFF;
                unsigned short min_pd_area_y = 0xFFFF;
                unsigned short max_pd_area_x = 0x0;
                unsigned short max_pd_area_y = 0x0;

                /*********************
                 * generate table information
                 *********************/
                char *bpc_table = (char *) retBuf->getBufVA(0);

                unsigned int pdo_ysz=0;
                unsigned int max_x_num_per_pitch=0;
                unsigned int table_index = 0;
                unsigned short *ptrTbl = reinterpret_cast<unsigned short*>(bpc_table);

                bool err = false;
                for(unsigned int it=0; it<numblkY; it++)  // numbers of repeating PD_BLOCK in vertical direction
                {

                    for(unsigned int curIdx=0, nxtIdx=0; curIdx<numPixs; )
                    {
                        // Define the index range of sorted pixels' array, and y coordinate is the same in this range
                        for(nxtIdx=curIdx; nxtIdx<numPixs; nxtIdx++)
                        {
                            if(sortedPixs[nxtIdx].y==sortedPixs[curIdx].y)
                                continue;
                            else
                                break;
                        }


                        //C:  PD table(8: BPC table) XXX:  y coordinate = current_y  XX CX
                        unsigned short current_y = sortedPixs[curIdx].y + it*pitchY;
                        ptrTbl[table_index++] = 0xC000|(0x3FFF&current_y);
                        pdo_ysz++;

                        //0:  PD table(8: BPC table) XXX:  x coordinate = current_x  XX 0X
                        unsigned short current_x = sortedPixs[curIdx].x;
                        ptrTbl[table_index++] = 0x0000|(0x3FFF&current_x);

                        //1069 defines the total pixel length between first PD and the last PD. In this case, 1069(HEX) = 4201 pixel width
                        unsigned short x_total_distance = (numblkX-1)*pitchX + (sortedPixs[nxtIdx-1].x-sortedPixs[curIdx].x) + 1;
                        ptrTbl[table_index++] = 0x0000|(0x3FFF&x_total_distance);

                        //0001 number of PD types in current line = PNUM(0001) + 1 = 2 PD types (L and R, list in the following lines)
                        // from current_idx to index-1 ((index-1)-current_idx+1)  +1 means for the next pitch's first pd; -1: represent 1 for 2 kind of PDs
                        unsigned short x_num_per_pitch = nxtIdx-curIdx;
                        ptrTbl[table_index++] = 0x0000|(0x3FFF&(x_num_per_pitch-1));

                        //for each PD
                        for(unsigned int i=curIdx; i<nxtIdx; i++)
                        {
                            //4008  P1: distance between the first PD and the next PD pixel   (R:4 L:0)
                            int dis = (i==nxtIdx-1)                                   ?
                                      (pitchX+sortedPixs[curIdx].x) - sortedPixs[i].x :
                                      (sortedPixs[i+1].x-sortedPixs[i].x);

                            ptrTbl[table_index++] = sortedPixs[i].type==PD_TYPE_R ?
                                                    0x4000|(0x3FFF&dis) :
                                                    0x0000|(0x3FFF&dis);

                            if( dis < 0 || dis > x_total_distance)
                            {
                                unsigned int _cur_x = sortedPixs[i].x;
                                unsigned int _cur_y = sortedPixs[i].y;
                                unsigned int _nxt_x = (i==nxtIdx-1) ? pitchX+sortedPixs[curIdx].x : sortedPixs[i+1].x;
                                unsigned int _nxt_y = (i==nxtIdx-1) ? pitchY+sortedPixs[curIdx].y : sortedPixs[i+1].y;

                                CAM_LOGE("[%s] distance between the current PD [%d,%d] and the next PD pixel [%d,%d] is %d\n",
                                         __FUNCTION__,
                                         _cur_x,
                                         _cur_y,
                                         _nxt_x,
                                         _nxt_y,
                                         dis);

                                //
                                err = true;
                                break;
                            }

                        }

                        if(!err)
                        {
                            //
                            max_x_num_per_pitch = x_num_per_pitch>max_x_num_per_pitch ? x_num_per_pitch : max_x_num_per_pitch;

                            //Debug information
                            min_pd_area_x = (current_x<min_pd_area_x) ? current_x : min_pd_area_x;
                            max_pd_area_x = (max_pd_area_x<(x_total_distance+sortedPixs[curIdx].x)) ? x_total_distance+sortedPixs[curIdx].x : max_pd_area_x;
                            min_pd_area_y = (current_y<min_pd_area_y) ? current_y : min_pd_area_y;
                            max_pd_area_y = (max_pd_area_y<current_y) ? current_y : max_pd_area_y;

                            //
                            curIdx = nxtIdx;
                        }
                        else
                        {
                            //
                            max_x_num_per_pitch=0;

                            //
                            min_pd_area_x = 0xFFFF;
                            min_pd_area_y = 0xFFFF;
                            max_pd_area_x = 0x0;
                            max_pd_area_y = 0x0;

                            CAM_LOGE("[%s] generate pd pixel x interval is fail. Please contact image kernel driver porting owner for checking pd position setting in pd block information.\n", __FUNCTION__);
                            break;
                        }

                    }

                    //
                    if(err)
                    {
                        CAM_LOGE("[%s] generate pd table error.\n", __FUNCTION__);
                        break;
                    }
                }


                CAM_LOGD("[%s] maximum x number per pitch(%d) pd_roi:minX(%d), minY(%d), maxX(%d), maxY(%d)\n", __FUNCTION__, max_x_num_per_pitch, min_pd_area_x, min_pd_area_y, max_pd_area_x, max_pd_area_y);

                if( /* checking HW constraint : (1) pd pixel can not appear at the first line. (2) pd area should less than image size*/
                    (!err) &&
                    (0<=min_pd_area_x) && (min_pd_area_x<imgSz.w) &&
                    (0<=min_pd_area_y) && (min_pd_area_y<imgSz.h))
                {
                    if (min_pd_area_y == 0)
                    {
                        CAM_LOGW("[%s] PD extraction HW constraint: pd pixel can not appear at the first line in full size. Please request sensor driver porting owner to modify sensor driver setting.\n", __FUNCTION__);
                    }

                    // maximum pd pixel in a line (numbers of pixel-1)* 2byte/pixel
                    unsigned int x_num_per_row = max_x_num_per_pitch*numblkX;
                    unsigned int pdo_xsz       = x_num_per_row*2;//((((bpci_info.PDO_XSIZE)>>4)<<4)+15)*2;

                    retInfo.memID   = (unsigned int) retBuf->getFD(0);
                    retInfo.tbl_xsz = (unsigned int) tbl_xsz-1; //BYTE
                    retInfo.tbl_ysz = (unsigned int) 0;
                    retInfo.tbl_pa  = (void*) retBuf->getBufPA(0);
                    retInfo.tbl_va  = (void*) retBuf->getBufVA(0);
                    retInfo.pdo_xsz = (unsigned int) pdo_xsz-1;
                    retInfo.pdo_ysz = (unsigned int) pdo_ysz-1;
                    retInfo.img_xsz = imgSz.w;
                    retInfo.img_ysz = imgSz.h;

                    CAM_LOGD("[%s] imgSz(%d,%d), pd_tbl:[memID(%d) tbl_sz(0x%x,0x%x) PA(%p) VA(%p)], pdo_sz(0x%x,0x%x), tbl_buf is locked by %s with property(0x%x)",
                             __FUNCTION__,
                             retInfo.img_xsz,
                             retInfo.img_ysz,
                             retInfo.memID,
                             retInfo.tbl_xsz,
                             retInfo.tbl_ysz,
                             retInfo.tbl_pa,
                             retInfo.tbl_va,
                             retInfo.pdo_xsz,
                             retInfo.pdo_ysz,
                             strName.c_str(),
                             (eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_HW_CAMERA_WRITE));

                    //
                    ret = true;
                }
                else
                {
                    CAM_LOGE("[%s] Table can not created because of HW constraint: pd area should less than image size. Please request sensor driver porting owner to modify sensor driver setting.\n", __FUNCTION__);
                }
            }
            else
            {
                CAM_LOGE("[%s] lock ImageBuffer fail by %s", __FUNCTION__,  strName.c_str());
            }
        }
        else
        {
            CAM_LOGE("[%s] create image buffer fail by %s with size %d", __FUNCTION__, strName.c_str(), tbl_xsz);
        }
    }
    else
    {
        CAM_LOGE("[%s] IIonImageBufferHeap::create fail by %s with size %d", __FUNCTION__, strName.c_str(), tbl_xsz);
    }


    //
    outTblInfo = TblBuf( retInfo, retBuf);

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
PDTblGenImpl::transformCoordinate( MSize boundarySz,
                                   SET_PD_BLOCK_INFO_T &pd_info,
                                   int const scenario,
                                   unsigned short numPixs,
                                   point *outAllPixs)
{
    memset(outAllPixs, 0, sizeof(point)*numPixs);

    unsigned int pitch_x    = pd_info.i4PitchX;
    unsigned int pitch_y    = pd_info.i4PitchY;
    unsigned int nblk_x     = pd_info.i4BlockNumX;
    unsigned int nblk_y     = pd_info.i4BlockNumY;
    unsigned int pairNum    = pd_info.i4PairNum;
    unsigned int full_lOffx = pd_info.i4OffsetX;
    unsigned int full_lOffy = pd_info.i4OffsetY;

    CAM_LOGD( "SensorMode(%d), pd block info(Offset:%d,%d Pitch:%d,%d PairNum:%d SubBlk:%d,%d), block number for current sensor mode(%d,%d), LeFirst(%d)\n",
              scenario,
              full_lOffx,
              full_lOffy,
              pitch_x,
              pitch_y,
              pairNum,
              pd_info.i4SubBlkW,
              pd_info.i4SubBlkH,
              nblk_x,
              nblk_y,
              pd_info.i4LeFirst);

    /*******************************************************************************
     * basic check pd block information.
     *******************************************************************************/
    if( /* check block number and pitch information */
        ( nblk_x  == 0) ||
        ( nblk_y  == 0) ||
        ( pitch_x == 0) ||
        ( pitch_y == 0) ||
        ( (unsigned int)(boundarySz.w) < (nblk_x*pitch_x)) ||
        ( (unsigned int)(boundarySz.h) < (nblk_y*pitch_y)))
    {
        CAM_LOGE("block number is not correct (%d)<(%d*%d) or (%d)<(%d*%d), checking sensor driver\n",
                 boundarySz.w,
                 nblk_x,
                 pitch_x,
                 boundarySz.h,
                 nblk_y,
                 pitch_y);

        return false;
    }
    else if( /* check pairNum information */
        ((pairNum*pd_info.i4SubBlkH*pd_info.i4SubBlkW) != (pitch_x*pitch_y)) ||
        (pairNum != (numPixs/2)))
    {
        CAM_LOGE("pairNum is not correct (%d)!=(%d/%d)*(%d/%d) or (%d)!=(%d/2), checking sensor driver\n",
                 pd_info.i4PairNum,
                 pd_info.i4PitchX,
                 pd_info.i4SubBlkH,
                 pd_info.i4PitchY,
                 pd_info.i4SubBlkW,
                 pd_info.i4PairNum,
                 numPixs);

        return false;
    }

    /*******************************************************************************
     * Reference crop region and mirrorflip inction to modify PD block information.
     *******************************************************************************/
    // Current image related full size coordinate
    unsigned int crop_x = pd_info.i4Crop[scenario][0];
    unsigned int crop_y = pd_info.i4Crop[scenario][1];

    // Current pd block offset related to full size coordinate
    int shift_x = crop_x-full_lOffx;
    if( shift_x<=0)
    {
        shift_x = -crop_x;
    }
    else if( shift_x%pitch_x)
    {
        shift_x = ( (shift_x+(pitch_x-1)) / pitch_x) * pitch_x - crop_x;
    }
    else
    {
        shift_x = ( shift_x / pitch_x) * pitch_x - crop_x;
    }


    int shift_y = crop_y-full_lOffy;
    if( shift_y<=0)
    {
        shift_y = -crop_y;
    }
    else if( shift_y%pitch_y)
    {
        shift_y = ( (shift_y+(pitch_y-1)) / pitch_y) * pitch_y - crop_y;
    }
    else
    {
        shift_y = ( shift_y / pitch_y) * pitch_y - crop_y;
    }

    CAM_LOGD( "Crop info (%d, %d) that is described as the offset between current sensor mode image size and calibration image size. PD coordinate shift in current sensor mode:(%d, %d)\n", crop_x, crop_y, shift_x, shift_y);

    /*******************************************************************************
     * calculate pd pixels' position by orientation and crop information for general separate function
     *******************************************************************************/
    unsigned int cur_lOffx = full_lOffx + shift_x;
    unsigned int cur_lOffy = full_lOffy + shift_y;
    unsigned int cur_rOffx = boundarySz.w - cur_lOffx - pitch_x * nblk_x;
    unsigned int cur_rOffy = boundarySz.h - cur_lOffy - pitch_y * nblk_y;

    CAM_LOGD("mirror_flip(%x). block offset for current sensor mode : left side(%d, %d) right sied(%d, %d)\n", pd_info.iMirrorFlip, cur_lOffx, cur_lOffy, cur_rOffx, cur_rOffy);

    for(unsigned int Pidx=0, k=0; Pidx<pairNum; Pidx++, k+=2)
    {
        unsigned int PosL_X = pd_info.i4PosL[Pidx][0];
        unsigned int PosL_Y = pd_info.i4PosL[Pidx][1];
        unsigned int PosR_X = pd_info.i4PosR[Pidx][0];
        unsigned int PosR_Y = pd_info.i4PosR[Pidx][1];

        if( /* boundary check */
            ((PosL_X-full_lOffx)<pitch_x) && ((PosL_Y-full_lOffy)<pitch_y) &&
            ((PosR_X-full_lOffx)<pitch_x) && ((PosR_Y-full_lOffy)<pitch_y))
        {
            /* shift coordinate by crop information */
            PosL_X += shift_x;
            PosL_Y += shift_y;
            PosR_X += shift_x;
            PosR_Y += shift_y;

            /* mirror*/
            if(pd_info.iMirrorFlip & 0x1)
            {
                PosL_X = pitch_x - (PosL_X - cur_lOffx) - 1 + cur_rOffx;
                PosR_X = pitch_x - (PosR_X - cur_lOffx) - 1 + cur_rOffx;
            }

            /* flip*/
            if(pd_info.iMirrorFlip & 0x2)
            {
                PosL_Y = pitch_y - (PosL_Y - cur_lOffy) - 1 + cur_rOffy;
                PosR_Y = pitch_y - (PosR_Y - cur_lOffy) - 1 + cur_rOffy;
            }

            /* Final*/
            point LCoordinate( PosL_X, PosL_Y, PD_TYPE_L);
            point RCoordinate( PosR_X, PosR_Y, PD_TYPE_R);

            /* Output*/
            outAllPixs[k  ] = LCoordinate;
            outAllPixs[k+1] = RCoordinate;

            CAM_LOGD("PDPos [L][%3d %3d]->[%3d %3d], [R][%3d %3d]->[%3d %3d]\n",
                     pd_info.i4PosL[Pidx][0],
                     pd_info.i4PosL[Pidx][1],
                     outAllPixs[k].x,
                     outAllPixs[k].y,
                     pd_info.i4PosR[Pidx][0],
                     pd_info.i4PosR[Pidx][1],
                     outAllPixs[k+1].x,
                     outAllPixs[k+1].y);


        }
        else
        {
            CAM_LOGE("PD coordinate is not sutible at pair index(%d), PDPos [L](%3d %3d) [R](%3d %3d)\n",
                     Pidx,
                     PosL_X,
                     PosL_Y,
                     PosR_X,
                     PosR_Y);

            return false;
        }
    }
    return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool
PDTblGenImpl::pd_tbl_reader( unsigned short img_w,
                             unsigned short img_h,
                             unsigned char *tbl_va,
                             unsigned short tbl_sz,
                             char *pTblFilePath)
{
    bool ret = false;

    auto _PBM_P4_STRIDE_ = [=](unsigned int W) -> unsigned int
    {
        unsigned int _align = 0;
        if (((W) % 8) != 0)
        {
            _align = ((W + 7) / 8 * 8);
        }
        else
        {
            _align = W;
        }
        return _align;
    };

    // allocat buffer with 1 byte align strid for pbm P4 image output
    unsigned int pbm_stride = _PBM_P4_STRIDE_(img_w)/8;
    unsigned char *pd_layout = new unsigned char[pbm_stride*img_h];
    memset(pd_layout, 0, sizeof(unsigned char)*pbm_stride*img_h);

    //
    unsigned int idx_tbl_rd = 0;
    unsigned short *pTbl = (unsigned short *)tbl_va;

    // output information
    unsigned int pd_pix_x_sz = 0, pd_pix_y_sz = 0;

    // parsing table
    while(1)
    {
        bool pd_is_normal_for_cur_line = false;

        if((pTbl[idx_tbl_rd] & 0xC000) == 0xC000)
        {
            pd_pix_y_sz++;

            // parsing pd line information
            unsigned short y_offset        = pTbl[idx_tbl_rd++] & 0x3FFF;
            unsigned short x_offset        = pTbl[idx_tbl_rd++] & 0x3FFF;
            unsigned short x_total_dis     = pTbl[idx_tbl_rd++] & 0x3FFF;
            unsigned short x_num_per_pitch = pTbl[idx_tbl_rd++] & 0x3FFF;

            // parsing pd interval information for current line
            std::vector<unsigned short> pd_interval;
            pd_interval.clear();
            while(1)
            {
                unsigned short interval = pTbl[idx_tbl_rd++] & 0x3FFF;
                pd_interval.push_back(interval);

                if ( idx_tbl_rd * sizeof(unsigned short) >= tbl_sz)
                    break;
                else if((pTbl[idx_tbl_rd] & 0xC000) == 0xC000)
                    break;
            }

            //
            if(x_num_per_pitch + 1 != pd_interval.size())
            {
                CAM_LOGE("[%s] pd tble error : pd interval configuration is not match at y=%d. interval number=%d(%d)", __FUNCTION__, y_offset, (x_num_per_pitch + 1), (int)(pd_interval.size()));
            }
            else
            {
                // label pd pixel for current line by using pd interval information
                unsigned int _x = x_offset;

                pd_layout[y_offset*pbm_stride + _x/8] |= 1<<(7-_x%8); /* label pd pixel */

                unsigned int _pd_pix_x_sz = 1;

                while(1)
                {
                    bool dn = false;

                    for(unsigned int i = 0; i < pd_interval.size(); i++)
                    {
                        unsigned int __x = _x + pd_interval.at(i);
                        if(__x < img_w && (__x - x_offset + 1) <= x_total_dis)
                        {
                            _pd_pix_x_sz++;

                            _x = __x;

                            pd_layout[y_offset*pbm_stride + _x / 8] |= 1 << (7-_x % 8);  /* label pd pixel */

                        }
                        else
                        {
                            if((_x - x_offset + 1) == x_total_dis)
                            {
                                pd_is_normal_for_cur_line = true;

                                pd_pix_x_sz = pd_pix_x_sz < _pd_pix_x_sz ? _pd_pix_x_sz : pd_pix_x_sz;
                            }
                            else
                            {

#define DBG_MSG_BUF_SIZE 1023
                                char dbg_msg[DBG_MSG_BUF_SIZE];
                                int cnt = 0;

                                cnt = snprintf( dbg_msg, DBG_MSG_BUF_SIZE, "[%s] pd tble error : last pd position is not correct at y=%d. x_offset=%d x_total_dis=%d interval_num=%d:", __FUNCTION__, y_offset, x_offset, x_total_dis, (int)(pd_interval.size()));
                                for(unsigned int i = 0; i < pd_interval.size(); i++)
                                {
                                    cnt += snprintf( dbg_msg+cnt, DBG_MSG_BUF_SIZE-cnt, "%d ", (pd_interval.at(i) & 0x3FFF));
                                }
                                cnt += snprintf( dbg_msg+cnt, DBG_MSG_BUF_SIZE-cnt, "\n");
                                CAM_LOGE( "%s", dbg_msg);
                            }
                            dn = true;
                            break;
                        }
                    }

                    if(dn)
                    {
                        break;
                    }

                }
            }
        }
        else
        {
            CAM_LOGE("[%s] pd tble error : can not get pd line information at %d BYTE", __FUNCTION__, idx_tbl_rd);
        }


        if(pd_is_normal_for_cur_line==false)
        {
            CAM_LOGE("[%s] pd tble error : please checking previours error message for more detail.", __FUNCTION__);
            break;
        }
        else if( idx_tbl_rd * sizeof(unsigned short) >= tbl_sz)
        {
            // parsing table is done without error message.
            ret = true;
            break;
        }
    };


    if(ret)
    {
#define EOL_DATA_SIZE 1
        unsigned short hw_cfg_pdo_x_sz = ((pd_pix_x_sz + EOL_DATA_SIZE) * sizeof(unsigned short)) - 1; /* HW is counting from 0 */
        unsigned short hw_cfg_pdo_y_sz = pd_pix_y_sz - 1; /* HW is counting from 0 */
        unsigned short hw_cfg_pdo_stride = _PDO_STRIDE_ALIGN_(pd_pix_x_sz + EOL_DATA_SIZE) * sizeof(unsigned short);


#define DUMP_FILE_NAME_SIZE (strlen(pTblFilePath)+100)
        char *pResFilePath = new char[DUMP_FILE_NAME_SIZE];
        memset(pResFilePath, '\0', sizeof(char)*DUMP_FILE_NAME_SIZE);
        int cnt = snprintf(pResFilePath, DUMP_FILE_NAME_SIZE, "%s_layout_pdoSz_0x%x_0x%x_stride_0x%x.pbm", pTblFilePath, hw_cfg_pdo_x_sz, hw_cfg_pdo_y_sz, hw_cfg_pdo_stride);

        //
        FILE *fp = nullptr;
        fp = fopen(pResFilePath, "w");
        if(fp)
        {
            fprintf(fp, "P4 \n%d %d ", img_w, img_h);
            fwrite(reinterpret_cast<void *>(pd_layout), sizeof(unsigned char), pbm_stride*img_h, fp);
            fclose(fp);

            CAM_LOGD("[DUMP] dump file path(string size=%d):%s", cnt, pResFilePath);
        }
        else
        {
            CAM_LOGE("[DUMP] can not dump file : %s", pResFilePath);
        }
        delete []pResFilePath;

        //
        CAM_LOGD("[DUMP] hw configuration for pdo : pdo_sz(0x%x,0x%x) pdo_stride(0x%x)", hw_cfg_pdo_x_sz, hw_cfg_pdo_y_sz, hw_cfg_pdo_stride);

    }


    //
    delete[]pd_layout;

    //
    return ret;
}


