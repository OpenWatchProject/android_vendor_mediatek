#include "FakeIspClk.h"
#include "FakeSensor.h"
#include <kd_imgsensor_define.h>

//4176; 2088; 1600;
//3088; 1544; 1200;
#define SENSOR_WIDTH_PRV        (1600)
#define SENSOR_HEIGHT_PRV       (1200)
#define SENSOR_WIDTH_PRV_SUB    (1600)
#define SENSOR_HEIGHT_PRV_SUB   (1200)
#define SENSOR_WIDTH_CAP        (4176)//1600 3200 4176
#define SENSOR_HEIGHT_CAP       (3120)//1200 2400 3088
#define SENSOR_WIDTH_CAP_SUB    (4032)
#define SENSOR_HEIGHT_CAP_SUB   (3016)

#define MAKE_SENINF_REG(_a, _r)     (_a - 0 + _r)

SensorStaticInfo TS_FakeSensorList::mSInfo;
IMetadata TS_FakeSensorList::mDummyMetadata;

Mutex TS_FakeSensor::cfgGLock;


static MUINT32 mPowerOnTM[] = {
    //0x200 -> 0x100,0x500
    0x0200, 0x00001001, //SENINF1_CTRL
    //0xd00 -> 0x120,0x520
    0x0D00, 0x96DF1080, //SENINF1_MUX_CTRL
    0x0D04, 0x8000007F, //SENINF1_MUX_INTEN(RST)
    0x0D0c, 0x00000000, //SENINF1_MUX_SIZE(RST)
    0x0D2c, 0x000E2000, //SENINF1_MUX_SPARE(RST)
    0x0D38, 0x00000000, //SENINF1_MUX_CROP(RST)

    //0x600 -> 0x200
    0x0608, 0x002804C1, // TG1_TM_CTL
                            // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
    0x060c, 0x0FF014E0, // TG1_TM_SIZE
    0x0610, 0x00000008, // TG1_TM_CLK
    0x0614, 0x1        // always setting timestamp dividor to 1 for test only
};



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
    static TS_FakeSensor single;

    TS_LOGD("create : %d", pArrayOfIndex[0]);
    if (pArrayOfIndex[0] >= 2) {
        TS_LOGE("Wrong sensor idx: %d", pArrayOfIndex[0]);
        return NULL;
    }

    (void)szCallerName;
    (void)uCountOfIndex;


    return &single;
}

MVOID TS_FakeSensorList::querySensorStaticInfo (
        MUINT sensorDevIdx,
        SensorStaticInfo *pSensorStaticInfo) const
{
    FAKE_CLK fakeclk;
    MUINT32 fps;
    (void)sensorDevIdx;

    memset((void*)pSensorStaticInfo, 0, sizeof(SensorStaticInfo));

    fps = (mPowerOnTM[2*8 -1]&0xffff) * ((mPowerOnTM[2*8 -1]>>16)&0xffff);
    fps = (MUINT32)(((MUINT64)fakeclk.Get_Default_Clk() * 10 + (fps*10/2)) / fps);
    TS_LOGD("Test pattern fpsx10:%d default_clk:%d\n",fps, fakeclk.Get_Default_Clk());

    if (sensorDevIdx & 0x01) {
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

        pSensorStaticInfo[0].previewFrameRate = fps;
        pSensorStaticInfo[0].captureFrameRate = fps;
        pSensorStaticInfo[0].videoFrameRate = fps;
    }
    if (sensorDevIdx & 0x02) {
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

        pSensorStaticInfo[0].previewFrameRate = fps;
        pSensorStaticInfo[0].captureFrameRate = fps;
        pSensorStaticInfo[0].videoFrameRate = fps;
    }
}

MVOID TS_FakeSensor::setFakeAttribute(eFakeAttr eAttr, MUINTPTR value)
{
    switch (eAttr) {
    case eFake_PixelMode:
        this->mSensorPixMode = (MUINT32)value;
        break;
    case eFake_CamsvChannel:
        this->mCamsvTgEn = !!value;
        TS_LOGD("Virtual channel: %d\n", this->mCamsvTgEn);
    default:
        break;
    }
}

MBOOL TS_FakeSensor::powerOn(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    MINT32 nNum = 0, i = 0, ret = 0;
    MUINT32 regOffset = 0;
    Mutex::Autolock _lock(cfgGLock);

    (void)szCallerName; (void)uCountOfIndex; (void)pArrayOfIndex;

    TS_LOGD("PowerOn: %d, pixMode: %d", pArrayOfIndex[0], mSensorPixMode);

    /* alaska only seninf0 has TM */
    if (m_pIspDrv) {
        MUINT32 mux_ctl = 0;

        m_pIspDrv->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only

        mux_ctl = m_pIspDrv->readReg(MAKE_SENINF_REG(0x0010, 0));
        mux_ctl = mux_ctl & ~(0xF << (pArrayOfIndex[0] * 4));
        if (mCamsvTgEn) {
            mux_ctl = mux_ctl & ~(0xF << ((pArrayOfIndex[0] * 4) + 8/* cama:[3:0] camb:[7:4]*/));
        }

        TS_LOGD("Update mux_ctl: 0x%x => 0x%x", m_pIspDrv->readReg(MAKE_SENINF_REG(0x0010, 0)), mux_ctl);

        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0010, 0), mux_ctl); //SENINF_TOP_CAM_MUX_CTRL

        m_pIspDrv->setRWMode(ISP_DRV_RW_MMAP);

        mPowerOnCnt++;

        return MTRUE;
    }

    m_pIspDrv = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);

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

    m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0000, 0), 0x00000C00); //SENINF_TOP_CTRL

    TS_LOGD("Set TM enable");
    m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0204, regOffset), m_pIspDrv->readReg(MAKE_SENINF_REG(0x0204, regOffset)) | 0x02);//SENINF1_CTRL_EXT
    TS_LOGD("#################################################\n");
    TS_LOGD("load default setting ............................\n");
    TS_LOGD("#################################################\n");

    {
        MUINT32 mux_ctl = 0;

        mux_ctl = m_pIspDrv->readReg(MAKE_SENINF_REG(0x0010, 0));
        mux_ctl = mux_ctl & ~(0xF << (pArrayOfIndex[0] * 4));
        if (mCamsvTgEn) {
            mux_ctl = mux_ctl & ~(0xF << ((pArrayOfIndex[0] * 4) + 8/* cama:[3:0] camb:[7:4]*/));
        }

        TS_LOGD("UPDATE MUX_CTL: 0x%x => 0x%x", m_pIspDrv->readReg(MAKE_SENINF_REG(0x0010, 0)), mux_ctl);

        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0010, 0), mux_ctl); //SENINF_TOP_CAM_MUX_CTRL
    }

    nNum = (sizeof(mPowerOnTM) / sizeof(mPowerOnTM[0])) / 2;
    TS_LOGD("Total %d registers", nNum);

    for (i = 0; i < nNum; i++) {
        /*
        if (0x1A040208 == mPowerOnTM[i<<1]) {
            TS_LOGD("Seninf Test Mode : %d", (atoi(szCallerName) & 0xF));
            mPowerOnTM[(i<<1)+1] = (0x00040401 | ((atoi(szCallerName) & 0xF) << 4));
        }
        */
        m_pIspDrv->writeReg(MAKE_SENINF_REG(mPowerOnTM[i<<1], regOffset), mPowerOnTM[(i<<1)+1]);
    }

    {
        MUINT32 reg = 0;
        reg = m_pIspDrv->readReg(MAKE_SENINF_REG(0x0200, 0));
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0200, 0), (reg|0x1)); // seninf_en
        reg = m_pIspDrv->readReg(MAKE_SENINF_REG(0x0204, 0));
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0204, 0), (reg|0x40)); // csi2_en
        reg = m_pIspDrv->readReg(MAKE_SENINF_REG(0x0204, 0));
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0204, 0), (reg|0x2)); // testmdl_en
        reg = m_pIspDrv->readReg(MAKE_SENINF_REG(0x0608, 0));
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0608, 0), (reg|0x1)); // Prevent early start , TM_EN
    }

    if (mSensorPixMode == 2) //two pixel mode
    {
        TS_LOGD("Set two pixel mode");
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0D00, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(0x0D00, 0)) | 0x100);//SENINF1_MUX_CTRL
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0D3C, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(0x0D3C, 0)) & ~0x10);//SENINF1_MUX_CTRL_EXT
    }
    else if (mSensorPixMode == 4)//four pixel mode
    {
        TS_LOGD("Set four pixel mode");
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0D00, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(0x0D00, 0)) & ~0x100);
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0D3C, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(0x0D3C, 0)) | 0x10);
    }
    else
    {
        TS_LOGD("Set one pixel mode");
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0D00, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(0x0D00, 0)) & ~0x100);
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0D3C, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(0x0D3C, 0)) & ~0x10);
    }

    if (0) {//(1 == bYUV)
        m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0608, 0), m_pIspDrv->readReg(MAKE_SENINF_REG(0x0608, 0)) | 0x4); //SENINF_TG1_TM_CTL
    }

    for (i = 0; i < nNum; i++) {
        TS_LOGD(" Reg[x%08x] = x%08x/x%08x", mPowerOnTM[i<<1],
            m_pIspDrv->readReg(MAKE_SENINF_REG(mPowerOnTM[i<<1], regOffset)), mPowerOnTM[(i<<1)+1]);
    }

    m_pIspDrv->setRWMode(ISP_DRV_RW_MMAP);

    mPowerOnCnt++;

    return MTRUE;
}

MBOOL TS_FakeSensor::powerOff(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    Mutex::Autolock _lock(cfgGLock);
    (void)szCallerName; (void)uCountOfIndex; (void)pArrayOfIndex;

    if (mPowerOnCnt == 0) {
        TS_LOGE("Already powerOff");
        return MFALSE;
    }

    mPowerOnCnt--;

    if (mPowerOnCnt == 0) {
        if (m_pIspDrv) {
            TS_LOGD("Set TM disable");
            m_pIspDrv->writeReg(MAKE_SENINF_REG(0x0608, 0), (m_pIspDrv->readReg(MAKE_SENINF_REG(0x0608, 0)) & ~0x01));
            m_pIspDrv->uninit("FakeSensor");
            m_pIspDrv->destroyInstance();
            m_pIspDrv = NULL;
        }
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

    }
    if (sensorIdx == 0x02) {
        pSensorDynamicInfo->TgInfo = CAM_TG_2;
    }

    if (mSensorPixMode == 2) {
        pSensorDynamicInfo->pixelMode = TWO_PIXEL_MODE;
        pSensorDynamicInfo->HDRPixelMode = 1;
        pSensorDynamicInfo->PDAFPixelMode = 1;
    }
    else if(mSensorPixMode == 4) {
        pSensorDynamicInfo->pixelMode = FOUR_PIXEL_MODE;
        pSensorDynamicInfo->HDRPixelMode = 2;
        pSensorDynamicInfo->PDAFPixelMode = 2;
    }
    else {
        pSensorDynamicInfo->pixelMode = ONE_PIXEL_MODE;
        pSensorDynamicInfo->HDRPixelMode = 0;
        pSensorDynamicInfo->PDAFPixelMode = 0;
    }

    pSensorDynamicInfo->TgCLKInfo = 1000;

    if (this->mCamsvTgEn) {
        pSensorDynamicInfo->HDRInfo = CAM_SV_1 + pSensorDynamicInfo->TgInfo - CAM_TG_1;
        pSensorDynamicInfo->PDAFInfo = pSensorDynamicInfo->HDRInfo;
        TS_LOGD("Update virtual channel info: HDRInfo/PDAFInfo(%d)", pSensorDynamicInfo->HDRInfo);
    }

    return 1;
}

MINT TS_FakeSensor::sendCommand(MUINT sensorDevIdx,MUINTPTR cmd,MUINTPTR arg1,MUINTPTR arg2,MUINTPTR arg3)
{
    FAKE_CLK fakeclk;
    MUINT32 fps;
    MINT32 ret = 0;

    (void)sensorDevIdx; (void)cmd;  (void)arg3;

    switch (cmd) {
    case SENSOR_CMD_GET_SENSOR_VC_INFO:
        {
            SENSOR_VC_INFO_STRUCT *pVcInfo = (SENSOR_VC_INFO_STRUCT *)arg1;

            if (pVcInfo) {
                pVcInfo->VC1_SIZEH = SENSOR_WIDTH_PRV;
                pVcInfo->VC1_SIZEV = SENSOR_HEIGHT_PRV;
                pVcInfo->VC2_SIZEH = SENSOR_WIDTH_PRV;
                pVcInfo->VC2_SIZEV = SENSOR_HEIGHT_PRV;
            }
            else {
                TS_LOGE("error: wrong null arg1\n");
            }
        }
        break;
    case SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE:
        *(MUINT32 *)arg1 = 0xdead;
        break;
    case SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        {
            FAKE_CLK fakeclk;
            MUINT32 fps;

            fps = (mPowerOnTM[2*8 -1]&0xffff) * ((mPowerOnTM[2*8 -1]>>16)&0xffff);
            TS_LOGE("ethan:%d\n",fps);
            fps = (MUINT32)(((MUINT64)fakeclk.Get_Default_Clk() * 10 + (fps*10/2)) / fps);
            TS_LOGE("ethan:%d,%d\n",fps,fakeclk.Get_Default_Clk());
            *(MUINT32 *)arg2 = fps;
        }
        break;
    default:
        TS_LOGD("unsupported cmd:0x%p\n",(void*)cmd);
        //ret = -1;
        break;
    }

    return ret;
}




