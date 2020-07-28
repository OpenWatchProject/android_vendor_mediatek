#include "FakeSensor.h"
#include <mtkcam/drv/IHalSensor.h>
#include <kd_imgsensor_define.h>


//4176x3088, 2088x1544, 1600x1200, 1280x720
#define SENSOR_PREVIEW_MODE     (0)
#define SENSOR_CAPTURE_MODE     (1)
#define SENSOR_MODE             (1)
#define SENSOR_RAW_TYPE         (0)
#define SENSOR_YUV_TYPE         (1)
#define SENSOR_TYPE             (1)

#if (SENSOR_MODE == SENSOR_PREVIEW_MODE)
#define SENSOR_WIDTH        (1600)
#define SENSOR_HEIGHT       (1200)
#elif (SENSOR_MODE == SENSOR_CAPTURE_MODE)
#define SENSOR_WIDTH        (2096)//(2088) //(4176) //
#define SENSOR_HEIGHT       (1550)//(1544) //(3088) //
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
    return &single;
}

MVOID FakeSensorList::querySensorStaticInfo(
        MUINT sensorDevIdx,
        SensorStaticInfo *pSensorStaticInfo) const
{
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
#if (SENSOR_TYPE != SENSOR_YUV_TYPE)
    pSensorStaticInfo[0].sensorType = SENSOR_TYPE_RAW; //1:RAW 2:YUV
#else
    pSensorStaticInfo[0].sensorType = SENSOR_TYPE_YUV;
#endif
    //get sernsor orientation angle
    pSensorStaticInfo[0].orientationAngle = 90;
    //get sensor format info
    pSensorStaticInfo[0].rawSensorBit = 1; //RAW10
#if (SENSOR_TYPE != SENSOR_YUV_TYPE)
    pSensorStaticInfo[0].sensorFormatOrder = 1;//3;
#else
    pSensorStaticInfo[0].sensorFormatOrder = 4;//for yuv fmt
#endif
    pSensorStaticInfo[0].SensorModeNum = 1;

}


static MUINT32 mPowerOnTM[] = {

#if (SENSOR_TYPE == SENSOR_RAW_TYPE)
    0x00004410, 0x00000005,

    //0x00004414, 0x00001001, //CAM_TG_VF_CON, VFDATA_EN

    0x00004418, ((((SENSOR_WIDTH+0x0002)&0x7FFF)<<16) | 0x0002),    // TG_SEN_GRAB_PXL
    0x0000441C, ((((SENSOR_HEIGHT+0x0002)&0x1FFF)<<16) | 0x0002),   // TG_SEN_GRAB_LIN
    // (2,2), 2088x1544
    //0x00004418, 0x082A0002, // TG_SEN_GRAB_PXL
    //0x0000441C, 0x060A0002, // TG_SEN_GRAB_LIN

    #if 0 //no need
    0x00004420, 0x00000000,
    0x00004424, 0x00001430,
    0x00004428, 0x00000000,
    0x0000442C, 0x00000000,
    0x00004430, 0x00000000,
    0x00004434, 0x00000010,
    0x00004438, 0x0FFFFFFF,
    0x0000443C, 0x00000000,
    #endif

    #if 0//RO reg
    0x00004440, 0x0031313F,
    0x00004444, 0x00000000,
    0x00004448, 0x13890FA1, //CAM_TG_FRMSIZE_ST
    0x0000444C, 0xA5A50403,
    #endif

    #if 0 //no need
    0x00004460, 0x00000000,
    0x00004464, 0x00000000,
    0x00004468, 0x00000000,
    0x0000446C, 0x00000000,
    0x00004470, 0x00000000,
    0x00004474, 0x00000000,
    0x00004478, 0x00000000,
    0x0000447C, 0x00000000,
    0x00004480, 0x00000000,
    0x00004484, 0x00000000,
    0x00004488, 0x00000000,
    0x0000448C, 0x00000000,
    0x00004490, 0x00000000,
    0x00004494, 0x00000000,
    #endif

    0x00008000, 0x00000C00, //SENINF_TOP_CTRL
    #if 0 //no need
    //0x00008004, 0x00000000, //SENINF_TOP_CMODEL_PAR
    //0x00008008, 0x00003210, //SENINF_TOP_MUX_CTRL
    #endif
    0x00008100, 0x00001001,
    0x00008104, 0x00000000,
    0x00008108, 0x00000000,
    0x0000810C, 0x00000000,
    0x00008110, 0x00000000,
    0x00008114, 0x00000000,
    0x00008118, 0x00000000,
    0x0000811C, 0x00000000,
    0x00008120, 0x96DF1080,
    0x00008124, 0x8000007F,
    #if 0 //RO
    0x00008128, 0x00000078,
    #endif
    0x0000812C, 0x00000000,
    #if 0 //RO
    0x00008130, 0xAE00C200,
    0x00008134, 0x13890FA0,
    0x00008138, 0x13890FA0,
    0x0000813C, 0x13890FA0,
    0x00008140, 0x13890FA0,
    0x00008144, 0xFFFFEE90,
    0x00008148, 0xFFFFEE90,
    #endif
    0x0000814C, 0x000E2000,
    #if 0 //RO
    0x00008150, 0xC0007858,
    0x00008154, 0x01315029,
    #endif
    0x00008158, 0x00000000,

    #if 0 //no need
    //0x00008200, 0xA0000001, // TG1_PH_CNT
    //0x00008204, 0x00010001, // TG1_SEN_CK
    #endif
    0x00008208, 0x000404D1, // TG1_TM_CTL
                            // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
    0x0000820C, 0x0FA01388, // TG1_TM_SIZE
    0x00008210, 0x0000000F  // TG1_TM_CLK

#else

    0x00004410, 0x00000005,
    //0x00004414, 0x00001001, //CAM_TG_VF_CON, VFDATA_EN
    0x00004418, ((((SENSOR_WIDTH+0x0002)&0x7FFF)<<16) | 0x0002),    // TG_SEN_GRAB_PXL
    0x0000441C, ((((SENSOR_HEIGHT+0x0002)&0x1FFF)<<16) | 0x0002),   // TG_SEN_GRAB_LIN

    0x00009410, 0x00000005, // CAMSV_TG_SEN_MODE
    //0x00009414, 0x00001001, //CAMSV_TG_VF_CON, VFDATA_EN
    0x00009418, ((((SENSOR_WIDTH+0x0002)&0x7FFF)<<16) | 0x0002),    // CAMSV_TG_SEN_GRAB_PXL
    0x0000941C, ((((SENSOR_HEIGHT+0x0002)&0x1FFF)<<16) | 0x0002),   // CAMSV_TG_SEN_GRAB_LIN

    0x00008000, 0x00000C00, //SENINF_TOP_CTRL
    0x00008008, 0x00003210, //SENINF_TOP_MUX_CTRL

    0x00008100, 0x00001001,
    0x00008104, 0x00000000,
    0x00008108, 0x00000000,
    0x0000810C, 0x00000000,
    0x00008110, 0x00000000,
    0x00008114, 0x00000000,
    0x00008118, 0x00000000,
    0x0000811C, 0x00000000,
    0x00008900, 0x00001001,
    0x00008904, 0x00000000,
    0x00008908, 0x00000000,
    0x0000890C, 0x00000000,
    0x00008910, 0x00000000,
    0x00008914, 0x00000000,
    0x00008918, 0x00000000,
    0x0000891C, 0x00000000,
    0x00008120, 0x96DF1080,
    0x00008124, 0x8000007F,
    0x0000812C, 0x00000000,
    0x0000814C, 0x000E2000,
    0x00008158, 0x00000000,
    0x00008920, 0x96DF1080,
    0x00008924, 0x8000007F,
    0x0000892C, 0x00000000,
    0x0000894C, 0x000E2000,
    0x00008958, 0x00000000,

    0x00008208, 0x000404D1, // TG1_TM_CTL
                            // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
    0x0000820C, 0x0FA01388, // TG1_TM_SIZE
    0x00008210, 0x0000000F,  // TG1_TM_CLK
    0x00008A08, 0x000404D1, // TG3_TM_CTL
                            // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
    0x00008A0C, 0x0FA01388, // TG1_TM_SIZE
    0x00008A10, 0x0000000F  // TG1_TM_CLK
#endif
};
MBOOL FakeSensor::powerOn(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    MUINT32 nNum = 0, i = 0, ret = 0;

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
    if (m_pIspReg != NULL){
		m_pIspReg = NULL;
	}
    m_pIspReg = (isp_reg_t *)m_pIspDrv->getRegAddr();
	TS_LOGD("m_pIspReg:0x%08x ", m_pIspReg);
	if (m_pIspReg == NULL) {
		TS_LOGD("getVirRegAddr fail ");
		return -1;
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

    return 1;
}
MBOOL FakeSensor::powerOff(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    int ret = 0;
    if (m_pIspDrv) {
        ret = m_pIspDrv->uninit("FakeSeninf");
        if (ret < 0) {
            TS_LOGE("m_pIspDrv->uninit() fail");
            return ret;
        }
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }

    return 1;
}
MBOOL FakeSensor::configure(
        MUINT const         uCountOfParam,
        IHalSensor::ConfigParam const*  pArrayOfParam)
{
    return 1;
}
MBOOL FakeSensor::querySensorDynamicInfo(
          MUINT32 sensorIdx,
          SensorDynamicInfo *pSensorDynamicInfo)
{
#if (SENSOR_TYPE == SENSOR_YUV_TYPE)
    pSensorDynamicInfo->TgInfo = CAM_SV_1;
#else
    pSensorDynamicInfo->TgInfo = CAM_TG_1;//CAM_TG_1 = 0x1, CAM_TG_2 = 0x2, CAM_SV_1 = 0x10, CAM_SV_2 = 0x11
#endif
    pSensorDynamicInfo->pixelMode = 0; //ONE_PIXEL_MODE, TWO_PIXEL_MODE

    return 1;
}



