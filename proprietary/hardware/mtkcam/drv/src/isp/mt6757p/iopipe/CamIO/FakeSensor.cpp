
#include "FakeSensor.h"


//4176; 2088; 1600;
//3088; 1544; 1200;
#define SENSOR_WIDTH_PRV        (1600)
#define SENSOR_HEIGHT_PRV       (1200)
#define SENSOR_WIDTH_PRV_SUB    (1600)
#define SENSOR_HEIGHT_PRV_SUB   (1200)
#define SENSOR_WIDTH_CAP        (5376)//1600 3200 4176 5400
#define SENSOR_HEIGHT_CAP       (4032)//1200 2400 3088 4060
#define SENSOR_WIDTH_CAP_SUB    (5400)
#define SENSOR_HEIGHT_CAP_SUB   (4060)


SensorStaticInfo TS_FakeSensorList::mSInfo;
IMetadata TS_FakeSensorList::mDummyMetadata;

extern "C" IHalSensorList* getInstance_FakeSensorList()
{
    return TS_FakeSensorList::getTestModel();
}

IHalSensorList *TS_FakeSensorList::getTestModel()
{
    static TS_FakeSensorList single;

    return &single;
}

IHalSensor *TS_FakeSensorList::createSensor (
                char const* szCallerName,
                MUINT const uCountOfIndex,
                MUINT const*pArrayOfIndex)
{
    static TS_FakeSensor single[2];

    TS_LOGD("create : %d", pArrayOfIndex[0]);
    if (pArrayOfIndex[0] >= 2) {
        TS_LOGE("Wrong sensor idx: %d", pArrayOfIndex[0]);
        return NULL;
    }

    (void)szCallerName;
    (void)uCountOfIndex;


    return &single[pArrayOfIndex[0]];
}

MVOID TS_FakeSensorList::querySensorStaticInfo (
        MUINT sensorDevIdx,
        SensorStaticInfo *pSensorStaticInfo) const
{
    (void)sensorDevIdx;

    TS_LOGD("+");

    memset((void*)pSensorStaticInfo, 0, sizeof(SensorStaticInfo));

    if (sensorDevIdx & 0x01) {
        TS_LOGD("dev 0x1");
        pSensorStaticInfo[0].sensorDevID = 0x5566;
        pSensorStaticInfo[0].sensorType = SENSOR_TYPE_RAW;
        pSensorStaticInfo[0].sensorFormatOrder = 1;
        pSensorStaticInfo[0].rawSensorBit = RAW_SENSOR_10BIT;

        pSensorStaticInfo[0].previewWidth = SENSOR_WIDTH_PRV;
        pSensorStaticInfo[0].previewHeight = SENSOR_HEIGHT_PRV;
        pSensorStaticInfo[0].captureWidth = SENSOR_WIDTH_CAP;
        pSensorStaticInfo[0].captureHeight = SENSOR_HEIGHT_CAP;
        pSensorStaticInfo[0].videoWidth = SENSOR_WIDTH_PRV;
        pSensorStaticInfo[0].videoHeight = SENSOR_HEIGHT_PRV;

        pSensorStaticInfo[0].orientationAngle = 90;

        pSensorStaticInfo[0].previewDelayFrame = 0;
        pSensorStaticInfo[0].captureDelayFrame = 0;
        pSensorStaticInfo[0].videoDelayFrame = 0;

        pSensorStaticInfo[0].previewFrameRate = 300;
        pSensorStaticInfo[0].captureFrameRate = 150;
        pSensorStaticInfo[0].videoFrameRate = 300;
    }
    if (sensorDevIdx & 0x02) {
        TS_LOGD("dev 0x2");
        pSensorStaticInfo[0].sensorDevID = 0x5566;
        pSensorStaticInfo[0].sensorType = SENSOR_TYPE_RAW;
        pSensorStaticInfo[0].sensorFormatOrder = 1;
        pSensorStaticInfo[0].rawSensorBit = RAW_SENSOR_10BIT;

        pSensorStaticInfo[0].previewWidth = SENSOR_WIDTH_PRV_SUB;
        pSensorStaticInfo[0].previewHeight = SENSOR_HEIGHT_PRV_SUB;
        pSensorStaticInfo[0].captureWidth = SENSOR_WIDTH_CAP_SUB;
        pSensorStaticInfo[0].captureHeight = SENSOR_HEIGHT_CAP_SUB;
        pSensorStaticInfo[0].videoWidth = SENSOR_WIDTH_PRV_SUB;
        pSensorStaticInfo[0].videoHeight = SENSOR_HEIGHT_PRV_SUB;

        pSensorStaticInfo[0].orientationAngle = 90;

        pSensorStaticInfo[0].previewDelayFrame = 0;
        pSensorStaticInfo[0].captureDelayFrame = 0;
        pSensorStaticInfo[0].videoDelayFrame = 0;

        pSensorStaticInfo[0].previewFrameRate = 300;
        pSensorStaticInfo[0].captureFrameRate = 150;
        pSensorStaticInfo[0].videoFrameRate = 300;
    }
}


static MUINT32 mPowerOnTM[] = {
        //0x200 -> 0x100,0x500
        0x1A040100, 0x00001001, //SENINF1_CTRL
        //0xd00 -> 0x120,0x520
        0x1A040120, 0x96DF1080, //SENINF1_MUX_CTRL
        0x1A040124, 0x8000007F, //SENINF1_MUX_INTEN(RST)
        0x1A04012c, 0x00000000, //SENINF1_MUX_SIZE(RST)
        0x1A04014c, 0x000E2000, //SENINF1_MUX_SPARE(RST)
        0x1A040158, 0x00000000, //SENINF1_MUX_CROP(RST)

        //0x600 -> 0x200
        0x1A040208, 0x000404C1, // TG1_TM_CTL
                                // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
        0x1A04020c, 0x10681F00, // TG1_TM_SIZE
        0x1A040210, 0x00000000, // TG1_TM_CLK
        0x1A040214, 0x1        // always setting timestamp dividor to 1 for test only
};

MVOID TS_FakeSensor::setPixelMode(MUINT pixMode)
{
    mSensorPixMode = pixMode;
}

#define MAKE_SENINF_REG(_a, _r)     (_a - 0x1A040000 + _r)
MBOOL TS_FakeSensor::powerOn(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    MINT32 nNum = 0, i = 0, ret = 0;
    MUINT32 regOffset = (pArrayOfIndex[0] == 0)? 0x0: 0x400;

    (void)szCallerName; (void)uCountOfIndex; (void)pArrayOfIndex;

    TS_LOGD("PowerOn: %d, pixMode: %d", pArrayOfIndex[0], mSensorPixMode);

#if 1 //OLY new setting
    m_pIspDrv = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);
#else
    m_pIspDrv = (IspDrvImp*)IspDrvImp::createInstance((pArrayOfIndex[0] == 0)? CAM_A: CAM_B);
#endif

    if (NULL == m_pIspDrv) {
        TS_LOGD("Error: IspDrv CreateInstace fail");
        return 0;
    }

    ret = m_pIspDrv->init("FakeSensor");
    if (ret < 0) {
        TS_LOGD("Error: IspDrv init fail");
        return 0;
    }
    m_pIspDrv->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only

    m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A040000, 0), 0x00000C00); //SENINF_TOP_CTRL
    #if 1 //EP dbg only
    TS_LOGD("Temp not set TM enable for EP, TBDDDDDDDDD");
    #else
    TS_LOGD("Set TM enable");
    m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A040104, regOffset), m_pIspDrv->readReg(MAKE_SENINF_REG(0x1A040104, regOffset)) | 0x02);
    #endif

    nNum = (sizeof(mPowerOnTM) / sizeof(mPowerOnTM[0])) / 2;
    TS_LOGD("Total %d registers", nNum);

    for (i = 0; i < nNum; i++) {
        #if 0 //change test model pattern
        if (0x1A040208 == mPowerOnTM[i<<1]) {
            TS_LOGD("Seninf Test Mode : %d", (atoi(szCallerName) & 0xF));
            mPowerOnTM[(i<<1)+1] = (0x00040401 | ((atoi(szCallerName) & 0xF) << 4));
        }
        #endif
        m_pIspDrv->writeReg(MAKE_SENINF_REG(mPowerOnTM[i<<1], regOffset), mPowerOnTM[(i<<1)+1]);
    }
    #if 0//EP dbg only
    TS_LOGD("Temp not set pixel mode, TBDDDDDDDDD");
    #else
    if(mSensorPixMode == 2) //two pixel mode
    {
    TS_LOGD("Set two pixel mode");
    m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A040120, regOffset), m_pIspDrv->readReg(MAKE_SENINF_REG(0x1A040120, regOffset)) | 0x100);
    //m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A04015C, regOffset), m_pIspDrv->readReg(MAKE_SENINF_REG(0x1A04015C, regOffset)) & 0x10);//TBDDDDDD
    }
#if 0 // Note: no 4 pixel mode in this platform!!!
    else if (mSensorPixMode == 4)//four pixel mode
    {
    TS_LOGD("Set four pixel mode");
    m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A040120, regOffset), m_pIspDrv->readReg(MAKE_SENINF_REG(0x1A040120, regOffset)) & ~0x100);
    m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A04015C, regOffset), m_pIspDrv->readReg(MAKE_SENINF_REG(0x1A04015C, regOffset)) | 0x10);
    }
#endif
    else
    {
    TS_LOGD("Set one pixel mode");
    m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A040120, regOffset), m_pIspDrv->readReg(MAKE_SENINF_REG(0x1A040120, regOffset)) & ~0x100);
    //m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A04015C, regOffset), m_pIspDrv->readReg(MAKE_SENINF_REG(0x1A04015C, regOffset)) & ~0x10);
    }
    #endif

    if (0) {//(1 == bYUV)
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A040208, regOffset), m_pIspDrv->readReg(MAKE_SENINF_REG(0x1A040208, regOffset)) | 0x4);
    }

    for (i = 0; i < nNum; i++) {
        TS_LOGD(" Reg[x%08x] = x%08x/x%08x", mPowerOnTM[i<<1],
            m_pIspDrv->readReg(MAKE_SENINF_REG(mPowerOnTM[i<<1], regOffset)), mPowerOnTM[(i<<1)+1]);
    }

    m_pIspDrv->setRWMode(ISP_DRV_RW_MMAP);

    return 1;
}

MBOOL TS_FakeSensor::powerOff(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    MUINT32 regOffset = (pArrayOfIndex[0] == 0)? 0x0: 0x400;

    (void)szCallerName; (void)uCountOfIndex; (void)pArrayOfIndex;

    if (m_pIspDrv) {
        TS_LOGD("Set TM disable");
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x1A040208, regOffset), (m_pIspDrv->readReg(MAKE_SENINF_REG(0x1A040208, regOffset)) & ~0x01));
        //m_pIspDrv->writeReg(0x15040204 - 0x15040000 , m_pIspDrv->readReg(0x15040204 - 0x15040000 ) & ~0x02);
        m_pIspDrv->uninit("FakeSensor");
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }

    return 1;
}

MBOOL TS_FakeSensor::configure(
        MUINT const uCountOfParam,
        IHalSensor::ConfigParam const*  pArrayOfParam)
{
    (void)uCountOfParam; (void)pArrayOfParam;
    return 1;
}

MBOOL TS_FakeSensor::querySensorDynamicInfo(
          MUINT32 sensorIdx,
          SensorDynamicInfo *pSensorDynamicInfo)
{
    TS_LOGD("sensorIdx: x%x", sensorIdx);

    if (sensorIdx == 0x01) {
        pSensorDynamicInfo->TgInfo = CAM_TG_1;
        if(mSensorPixMode == 2)
        {    pSensorDynamicInfo->pixelMode = TWO_PIXEL_MODE;}
        else if(mSensorPixMode == 4)
        {    pSensorDynamicInfo->pixelMode = FOUR_PIXEL_MODE;}
        else
        {    pSensorDynamicInfo->pixelMode = ONE_PIXEL_MODE;}
    }
    if (sensorIdx == 0x02) {
        pSensorDynamicInfo->TgInfo = CAM_TG_2;
        if(mSensorPixMode == 2)
        {    pSensorDynamicInfo->pixelMode = TWO_PIXEL_MODE;}
        else if(mSensorPixMode == 4)
        {    pSensorDynamicInfo->pixelMode = FOUR_PIXEL_MODE;}
        else
        {    pSensorDynamicInfo->pixelMode = ONE_PIXEL_MODE;}
    }
    pSensorDynamicInfo->TgCLKInfo = 1000;

    return 1;
}
