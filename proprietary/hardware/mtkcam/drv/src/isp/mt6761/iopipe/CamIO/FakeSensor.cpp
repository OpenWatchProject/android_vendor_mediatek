#include "FakeSensor.h"
#include <mtkcam/drv/IHalSensor.h>
#include <kd_imgsensor_define.h>


//4176x3088, 2088x1544, 1600x1200, 1280x720
#define SENSOR_PREVIEW_MODE     (0)
#define SENSOR_CAPTURE_MODE     (1)
#define SENSOR_MODE             (1)
#if (SENSOR_MODE == SENSOR_PREVIEW_MODE)
#define SENSOR_WIDTH        (1600)
#define SENSOR_HEIGHT       (1200)
#elif (SENSOR_MODE == SENSOR_CAPTURE_MODE)
#define SENSOR_WIDTH        (2088) //(4176) //
#define SENSOR_HEIGHT       (1544) //(3088) //
#else
#define SENSOR_WIDTH        (1600)
#define SENSOR_HEIGHT       (1200)
#endif
SensorStaticInfo FakeSensorList::mSInfo;
IMetadata FakeSensorList::mDummyMetadata;
IHalSensorList *FakeSensorList::getTestModel()
{
    static FakeSensorList single;
    return &single;
}
IHalSensor *FakeSensorList::createSensor (
                char const* szCallerName,
                MUINT const uCountOfIndex,
                MUINT const*pArrayOfIndex)
{
    static FakeSensor single;
    UNUSED(szCallerName);
    UNUSED(uCountOfIndex);
    UNUSED(pArrayOfIndex);
    return &single;
}
MVOID FakeSensorList::querySensorStaticInfo(
        MUINT sensorDevIdx,
        SensorStaticInfo *pSensorStaticInfo) const
{
    UNUSED(sensorDevIdx);
    pSensorStaticInfo[0].previewWidth = SENSOR_WIDTH;
    pSensorStaticInfo[0].previewHeight = SENSOR_HEIGHT;
    // get sensor full range
    pSensorStaticInfo[0].captureWidth = SENSOR_WIDTH;
    pSensorStaticInfo[0].captureHeight = SENSOR_HEIGHT;
    // get sensor video range
    pSensorStaticInfo[0].videoWidth = SENSOR_WIDTH;
    pSensorStaticInfo[0].videoHeight = SENSOR_HEIGHT;
    // get sensor video1 range
    pSensorStaticInfo[0].video1Width = SENSOR_WIDTH;
    pSensorStaticInfo[0].video1Height = SENSOR_HEIGHT;
    // get sensor video2 range
    pSensorStaticInfo[0].video2Width = SENSOR_WIDTH;
    pSensorStaticInfo[0].video2Height = SENSOR_HEIGHT;
    // get sensor video3 range
    pSensorStaticInfo[0].SensorCustom1Width = SENSOR_WIDTH;
    pSensorStaticInfo[0].SensorCustom1Height = SENSOR_HEIGHT;
    // get sensor video4 range
    pSensorStaticInfo[0].SensorCustom2Width = SENSOR_WIDTH;
    pSensorStaticInfo[0].SensorCustom2Height = SENSOR_HEIGHT;
    // get sensor video5 range
    pSensorStaticInfo[0].SensorCustom3Width = SENSOR_WIDTH;
    pSensorStaticInfo[0].SensorCustom3Height = SENSOR_HEIGHT;
    // get sensor video6 range
    pSensorStaticInfo[0].SensorCustom4Width = SENSOR_WIDTH;
    pSensorStaticInfo[0].SensorCustom4Height = SENSOR_HEIGHT;
    // get sensor video7 range
    pSensorStaticInfo[0].SensorCustom5Width = SENSOR_WIDTH;
    pSensorStaticInfo[0].SensorCustom5Height = SENSOR_HEIGHT;
    // get RAW or YUV
    pSensorStaticInfo[0].sensorType = SENSOR_TYPE_RAW; //1:RAW 2:YUV
    //get sernsor orientation angle
    pSensorStaticInfo[0].orientationAngle = 90;
    //get sensor format info
    pSensorStaticInfo[0].rawSensorBit = 1; //RAW10
    pSensorStaticInfo[0].sensorFormatOrder = 1;//3;
    pSensorStaticInfo[0].SensorModeNum = 1;
}
static MUINT32 mPowerOnTG[] = {
    0x00004410, 0x00000005,
    0x00004418, ((((SENSOR_WIDTH+0x0002)&0x7FFF)<<16) | 0x0002),    // TG_SEN_GRAB_PXL
    0x0000441C, ((((SENSOR_HEIGHT+0x0002)&0x1FFF)<<16) | 0x0002),   // TG_SEN_GRAB_LIN
};
static MUINT32 mPowerOnTM[] = {
    0x00000000, 0x00000C00, //SENINF_TOP_CTRL
    0x00000200, 0x00001001,
    0x00000204, 0x00000002,
    0x00000208, 0x00000000,
    0x0000020C, 0x00000000,
    0x00000210, 0x00000000,
    0x00000214, 0x00000000,
    0x00000218, 0x00000000,
    0x0000021C, 0x00000000,
    0x00000220, 0x96DF1080,
    0x00000224, 0x8000007F,
    0x00000D00, 0x86DF1080,
    0x00000D0C, 0x00000000,
    0x00000D38, 0x00000000,
    0x00000608, 0x00FF01C1, // TG1_TM_CTL
                            // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
    0x0000060C, 0x0FA01388, // TG1_TM_SIZE
    0x00000610, 0x00000008  // TG1_TM_CLK
};
MBOOL FakeSensor::powerOn(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    MUINT32 nNum = 0, i = 0, ret = 0;
    UNUSED(szCallerName);
    UNUSED(uCountOfIndex);
    UNUSED(pArrayOfIndex);
    m_pIspDrv = IspDrv::createInstance();
    if (NULL == m_pIspDrv) {
        TS_LOGD("Error: IspDrv CreateInstace fail");
        return 0;
    }
    ret = m_pIspDrv->init("FakeSeninf");
    if (ret < 0) {
        TS_LOGD("Error: IspDrv init fail");
        return 0;
    }
    nNum = (sizeof(mPowerOnTM) / sizeof(mPowerOnTM[0])) / 2;
    TS_LOGD(" Total %d registers", nNum);
    for (i = 0; i < nNum; i++) {
        if (0x00008208 == mPowerOnTM[i<<1])
        {
            //TS_LOGD("Seninf Test Mode : %d", (atoi(szCallerName) & 0xF));
            //mPowerOnTM[(i<<1)+1] = (0x00040401 | ((atoi(szCallerName) & 0xF) << 4));
            MUINT m = 13;
            TS_LOGD("Seninf Test Mode : %d", m);
            mPowerOnTM[(i<<1)+1] = (0x00040401 | ((m & 0xF) << 4));
        }
        m_pIspDrv->writeReg(mPowerOnTM[i<<1], mPowerOnTM[(i<<1)+1],ISP_DRV_USER_SENF);
    }
    for (i = 0; i < nNum; i++) {
        TS_LOGD(" Reg[x%08x] = x%08x/x%08x", mPowerOnTM[i<<1],
            m_pIspDrv->readReg(mPowerOnTM[i<<1]), mPowerOnTM[(i<<1)+1]);
    }


    if (m_pIspDrv) {
        m_pIspDrv->uninit("FakeSeninf");
    }

    ret = m_pIspDrv->init("FakeTG");
    if (ret < 0) {
        TS_LOGD("Error: IspDrv init fail");
        return 0;
    }
    nNum = (sizeof(mPowerOnTG) / sizeof(mPowerOnTG[0])) / 2;
    TS_LOGD(" Total %d registers", nNum);
    for (i = 0; i < nNum; i++) {
        if (0x00008208 == mPowerOnTG[i<<1])
        {
            //TS_LOGD("Seninf Test Mode : %d", (atoi(szCallerName) & 0xF));
            //mPowerOnTG[(i<<1)+1] = (0x00040401 | ((atoi(szCallerName) & 0xF) << 4));
            MUINT m = 13;
            TS_LOGD("Seninf Test Mode : %d", m);
            mPowerOnTG[(i<<1)+1] = (0x00040401 | ((m & 0xF) << 4));
        }
        m_pIspDrv->writeReg(mPowerOnTG[i<<1], mPowerOnTG[(i<<1)+1],ISP_DRV_USER_SENF);
    }
    for (i = 0; i < nNum; i++) {
        TS_LOGD(" Reg[x%08x] = x%08x/x%08x", mPowerOnTG[i<<1],
            m_pIspDrv->readReg(mPowerOnTG[i<<1]), mPowerOnTG[(i<<1)+1]);
    }

    if (m_pIspDrv) {
        m_pIspDrv->uninit("FakeTG");
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }


    return 1;
}
MBOOL FakeSensor::powerOff(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    UNUSED(szCallerName);
    UNUSED(uCountOfIndex);
    UNUSED(pArrayOfIndex);
    if (m_pIspDrv) {
        m_pIspDrv->uninit();
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }
    return 1;
}
MBOOL FakeSensor::configure(
        MUINT const         uCountOfParam,
        IHalSensor::ConfigParam const*  pArrayOfParam)
{
    UNUSED(uCountOfParam);
    UNUSED(pArrayOfParam);
    return 1;
}
MBOOL FakeSensor::querySensorDynamicInfo(
          MUINT32 sensorIdx,
          SensorDynamicInfo *pSensorDynamicInfo)
{
    UNUSED(sensorIdx);
    pSensorDynamicInfo->TgInfo = 1;
    pSensorDynamicInfo->pixelMode = 0; //ONE_PIXEL_MODE, TWO_PIXEL_MODE
    return 1;
}
