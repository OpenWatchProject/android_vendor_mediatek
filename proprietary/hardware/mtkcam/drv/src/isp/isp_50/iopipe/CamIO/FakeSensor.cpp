#include "FakeIspClk.h"
#include "FakeSensor.h"
#include <kd_imgsensor_define.h>
//#include "seninf_type.h"
#include "iseninf_drv.h"

#define SENSOR_WIDTH_PRV        (1600)
#define SENSOR_HEIGHT_PRV       (1200)
#define SENSOR_WIDTH_PRV_SUB    (1600)
#define SENSOR_HEIGHT_PRV_SUB   (1200)
#define SENSOR_WIDTH_PRV_MAIN2  (1600)
#define SENSOR_HEIGHT_PRV_MAIN2 (1200)
#define SENSOR_WIDTH_PRV_SUB2   (1600)
#define SENSOR_HEIGHT_PRV_SUB2  (1200)
#define SENSOR_WIDTH_CAP        (5344)
#define SENSOR_HEIGHT_CAP       (4016)
#define SENSOR_WIDTH_CAP_SUB    (5344)
#define SENSOR_HEIGHT_CAP_SUB   (4016)
#define SENSOR_WIDTH_CAP_MAIN2  (5344)
#define SENSOR_HEIGHT_CAP_MAIN2 (4016)
#define SENSOR_WIDTH_CAP_SUB2   (5344)
#define SENSOR_HEIGHT_CAP_SUB2  (4016)


#define MAKE_SENINF_REG(_a, _r)     (_a - 0 + _r)

SensorStaticInfo TS_FakeSensorList::mSInfo;
IMetadata TS_FakeSensorList::mDummyMetadata;

#define TG1_TM_SIZE (0x10001F00)

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
    if (pArrayOfIndex[0] > 3) { // 0:main1 1:sub1 2:main2 3:sub2
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
    (void)sensorDevIdx;

    memset((void*)pSensorStaticInfo, 0, sizeof(SensorStaticInfo));

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

        pSensorStaticInfo[0].previewFrameRate = 30 * 10;
        pSensorStaticInfo[0].captureFrameRate = 30 * 10;
        pSensorStaticInfo[0].videoFrameRate = 30 * 10;
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

        pSensorStaticInfo[0].previewFrameRate = 30 * 10;
        pSensorStaticInfo[0].captureFrameRate = 30 * 10;
        pSensorStaticInfo[0].videoFrameRate = 30 * 10;
    }
    if (sensorDevIdx & 0x04) {
        pSensorStaticInfo[0].sensorDevID = 0x5566;
        pSensorStaticInfo[0].sensorType = SENSOR_TYPE_RAW;
        pSensorStaticInfo[0].sensorFormatOrder = 1;
        pSensorStaticInfo[0].rawSensorBit = RAW_SENSOR_10BIT;

        pSensorStaticInfo[0].previewWidth = SENSOR_WIDTH_PRV_MAIN2;
        pSensorStaticInfo[0].previewHeight = SENSOR_HEIGHT_PRV_MAIN2;
        pSensorStaticInfo[0].captureWidth = SENSOR_WIDTH_CAP_MAIN2;
        pSensorStaticInfo[0].captureHeight = SENSOR_HEIGHT_CAP_MAIN2;
        pSensorStaticInfo[0].videoWidth = SENSOR_WIDTH_PRV_MAIN2;
        pSensorStaticInfo[0].videoHeight = SENSOR_HEIGHT_PRV_MAIN2;

        pSensorStaticInfo[0].orientationAngle = 90;

        pSensorStaticInfo[0].previewDelayFrame = 0;
        pSensorStaticInfo[0].captureDelayFrame = 0;
        pSensorStaticInfo[0].videoDelayFrame = 0;

        pSensorStaticInfo[0].previewFrameRate = 30 * 10;
        pSensorStaticInfo[0].captureFrameRate = 30 * 10;
        pSensorStaticInfo[0].videoFrameRate = 30 * 10;
    }
    if (sensorDevIdx & 0x08) {
        pSensorStaticInfo[0].sensorDevID = 0x5566;
        pSensorStaticInfo[0].sensorType = SENSOR_TYPE_RAW;
        pSensorStaticInfo[0].sensorFormatOrder = 1;
        pSensorStaticInfo[0].rawSensorBit = RAW_SENSOR_10BIT;

        pSensorStaticInfo[0].previewWidth = SENSOR_WIDTH_PRV_SUB2;
        pSensorStaticInfo[0].previewHeight = SENSOR_HEIGHT_PRV_SUB2;
        pSensorStaticInfo[0].captureWidth = SENSOR_WIDTH_CAP_SUB2;
        pSensorStaticInfo[0].captureHeight = SENSOR_HEIGHT_CAP_SUB2;
        pSensorStaticInfo[0].videoWidth = SENSOR_WIDTH_PRV_SUB2;
        pSensorStaticInfo[0].videoHeight = SENSOR_HEIGHT_PRV_SUB2;

        pSensorStaticInfo[0].orientationAngle = 90;

        pSensorStaticInfo[0].previewDelayFrame = 0;
        pSensorStaticInfo[0].captureDelayFrame = 0;
        pSensorStaticInfo[0].videoDelayFrame = 0;

        pSensorStaticInfo[0].previewFrameRate = 30 * 10;
        pSensorStaticInfo[0].captureFrameRate = 30 * 10;
        pSensorStaticInfo[0].videoFrameRate = 30 * 10;
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
        break;
    case eFake_FPS:
        this->m_Fps = (MUINT32)value;
        break;
    default:
        break;
    }
}

MBOOL TS_FakeSensor::powerOn(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    (void)szCallerName; (void)uCountOfIndex; (void)pArrayOfIndex;

    return MTRUE;
}

MBOOL TS_FakeSensor::powerOff(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    (void)szCallerName; (void)uCountOfIndex; (void)pArrayOfIndex;

    return MTRUE;
}

MBOOL TS_FakeSensor::configure(
        MUINT const uCountOfParam,
        IHalSensor::ConfigParam const*  pArrayOfParam)
{
    (void)uCountOfParam; (void)pArrayOfParam;

    return MTRUE;
}

MBOOL TS_FakeSensor::querySensorDynamicInfo(
          MUINT32 sensorIdx,
          SensorDynamicInfo *pSensorDynamicInfo)
{
    TS_LOGD("sensorIdx: 0x%x", sensorIdx);

    if (mSensorPixMode == 2) {
        pSensorDynamicInfo->pixelMode = TWO_PIXEL_MODE;
        pSensorDynamicInfo->HDRPixelMode = 1;
        pSensorDynamicInfo->PDAFPixelMode = 1;
        for(MUINT32 i = 0 ; i < HDR_DATA_MAX_NUM ; i++){
            pSensorDynamicInfo->PixelMode[i] = 1;
        }
    }
    else if(mSensorPixMode == 4) {
        pSensorDynamicInfo->pixelMode = FOUR_PIXEL_MODE;
        pSensorDynamicInfo->HDRPixelMode = 2;
        pSensorDynamicInfo->PDAFPixelMode = 2;
        for(MUINT32 i = 0 ; i < HDR_DATA_MAX_NUM ; i++){
            pSensorDynamicInfo->PixelMode[i] = 2;
        }
    }
    else {
        pSensorDynamicInfo->pixelMode = ONE_PIXEL_MODE;
        pSensorDynamicInfo->HDRPixelMode = 0;
        pSensorDynamicInfo->PDAFPixelMode = 0;
        for(MUINT32 i = 0 ; i < HDR_DATA_MAX_NUM ; i++){
            pSensorDynamicInfo->PixelMode[i] = 0;
        }
    }

    pSensorDynamicInfo->TgCLKInfo = 1000;

    if (this->mCamsvTgEn) {
        pSensorDynamicInfo->HDRInfo = CAM_SV_1;
        pSensorDynamicInfo->PDAFInfo = CAM_SV_2;
        pSensorDynamicInfo->CamInfo[EmbInfo] = CAM_SV_1;
        pSensorDynamicInfo->CamInfo[YInfo] = CAM_SV_2;
        pSensorDynamicInfo->CamInfo[AEInfo] = CAM_SV_3;
        pSensorDynamicInfo->CamInfo[FlickerInfo] = CAM_SV_4;
    }

    return 1;
}

MINT TS_FakeSensor::sendCommand(MUINT sensorDevIdx,MUINTPTR cmd,MUINTPTR arg1,MUINTPTR arg2,MUINTPTR arg3)
{
    FAKE_CLK fakeclk;
    MUINT32 fps;
    MINT32 ret = 0;

    (void)sensorDevIdx;

    SeninfDrv *const pSeninfDrv = SeninfDrv::getInstance();

    switch (cmd) {
    case SENSOR_CMD_GET_SENSOR_VC_INFO:
        {
            SENSOR_VC_INFO_STRUCT *pVcInfo = (SENSOR_VC_INFO_STRUCT *)arg1;

            if (pVcInfo) {
                pVcInfo->VC1_SIZEH = SENSOR_WIDTH_PRV;
                pVcInfo->VC1_SIZEV = SENSOR_HEIGHT_PRV;
                pVcInfo->VC2_SIZEH = SENSOR_WIDTH_PRV;
                pVcInfo->VC2_SIZEV = SENSOR_HEIGHT_PRV;
                pVcInfo->VC3_SIZEH = SENSOR_WIDTH_PRV;
                pVcInfo->VC3_SIZEV = SENSOR_HEIGHT_PRV;
                pVcInfo->VC4_SIZEH = SENSOR_WIDTH_PRV;
                pVcInfo->VC4_SIZEV = SENSOR_HEIGHT_PRV;
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
            *(MUINT32 *)arg2 = m_Fps;
        }
        break;
    case SENSOR_CMD_SET_TEST_MODEL:
        {
#define HALSENSOR_TEST_MODEL_ISP_CLOCK   6000000
#define HALSENSOR_TEST_MODEL_DUMMY_PIXEL 16
            /* FPS = ISP clock / ((TM_DUMMYPXL + TM_PXL) * (TM_VSYNC + TM_LINE)) */

            MUINT32 TM_LINE, TM_PXL, ispClock, vsync;
            MINT dummyPixel;

            TM_LINE     = ((TG1_TM_SIZE >> 16) & 0xffff);
            TM_PXL      = (TG1_TM_SIZE & 0xffff);
            fps         = TM_LINE * TM_PXL;
            fps         = (MUINT32)(((MUINT64)fakeclk.Get_Fake_Clk(FCLK_LEVEL_L) + (fps*10/2)) / fps);
            ispClock    = HALSENSOR_TEST_MODEL_ISP_CLOCK;
            vsync       = TM_LINE >> 4;
            dummyPixel  = (ispClock / fps) / (TM_LINE + vsync) - TM_PXL;

            ret = pSeninfDrv->init();
            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_MUX1, SENINF_1);
            ret = pSeninfDrv->setSeninfMuxCtrl(SENINF_MUX1, 0, 0, TEST_MODEL, RAW_10BIT_FMT, 1);
            ret = pSeninfDrv->enableMUX(SENINF_MUX1);

            // Stream On
            if(*(MUINT32 *)arg1 == MTRUE){
                if(dummyPixel < HALSENSOR_TEST_MODEL_DUMMY_PIXEL) {
                    TS_LOGE("[sendCommand] SENSOR_CMD_SET_TEST_MODEL fail, set to higher framerate! %d\n",
                        ispClock / ((HALSENSOR_TEST_MODEL_DUMMY_PIXEL + TM_PXL) * (vsync + TM_LINE)));
                    ret = pSeninfDrv->setTestModel(true, HALSENSOR_TEST_MODEL_DUMMY_PIXEL, vsync, TM_LINE, TM_PXL);
                } else {
                    ret = pSeninfDrv->setTestModel(true, dummyPixel, vsync, TM_LINE, TM_PXL);
                }
            }
            // Stream Off
            else{
                if(dummyPixel < HALSENSOR_TEST_MODEL_DUMMY_PIXEL) {
                    TS_LOGE("[sendCommand] SENSOR_CMD_SET_TEST_MODEL fail, set to higher framerate! %d\n",
                        ispClock / ((HALSENSOR_TEST_MODEL_DUMMY_PIXEL + TM_PXL) * (vsync + TM_LINE)));
                    ret = pSeninfDrv->setTestModel(false, HALSENSOR_TEST_MODEL_DUMMY_PIXEL, vsync, TM_LINE, TM_PXL);
                } else {
                    ret = pSeninfDrv->setTestModel(false, dummyPixel, vsync, TM_LINE, TM_PXL);
                }
            }
        }
        break;
    case SENSOR_CMD_SET_SENINF_CAM_TG_MUX:
        {
            ret = pSeninfDrv->init();
            ret = pSeninfDrv->setSeninfCamTGMuxCtrl(*(MUINT32 *)arg2 - CAM_TG_1, (SENINF_MUX_ENUM)pSeninfDrv->getSeninfCamTGMuxCtrl(*(MUINT32 *)arg1 - CAM_TG_1));
        }
        break;
    default:
        TS_LOGD("unsupported cmd:0x%x\n",cmd);
        //ret = -1;
        break;
    }

    return ret;
}




