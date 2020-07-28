#define LOG_TAG "CamCalDrv"
#define MTK_LOG_ENABLE 1

#include <utils/Errors.h>
#include <cutils/properties.h>
#include <log/log.h>
#include <mtkcam/utils/std/ULog.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include "cam_cal_drv_imp.h"

#ifdef CAM_CAL_SUPPORT
#include "camera_custom_msdk.h"
#endif

#include "camera_custom_cam_cal.h"

#include <mtkcam/drv/IHalSensor.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_CAMCAL);
using namespace NSCam;

/*******************************************************************************
*
********************************************************************************/

#define CAM_CAL_SHOW_LOG

#ifdef CAM_CAL_SHOW_LOG
#define CAM_CAL_DRV_LOG(fmt, arg...)    CAM_ULOGMD(LOG_TAG " " fmt, ##arg)
#define CAM_CAL_DRV_ERR(fmt, arg...)    CAM_ULOGME(LOG_TAG "Err: %5d: " fmt, __LINE__, ##arg)
#else
#define CAM_CAL_DRV_LOG(fmt, arg...)    void(0)
#define CAM_CAL_DRV_ERR(fmt, arg...)    void(0)
#endif

#define CAM_CAL_DRV_LOG_IF(cond, ...)      do { if ( (cond) ) { CAM_CAL_DRV_LOG(__VA_ARGS__); } }while(0)

/*******************************************************************************
* Mutex
********************************************************************************/
static pthread_mutex_t mCam_Cal_Mutex = PTHREAD_MUTEX_INITIALIZER;

/*******************************************************************************
*
********************************************************************************/
CAM_CAL_DATA_STRUCT CamCalDrv::StCamCalCaldata;

/*******************************************************************************
*
********************************************************************************/
CamCalDrvBase*
CamCalDrvBase::createInstance()
{
    return CamCalDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
CamCalDrvBase*
CamCalDrv::getInstance()
{
    static CamCalDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
CamCalDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
CamCalDrv::CamCalDrv()
    : CamCalDrvBase()
{
}

/*******************************************************************************
*
********************************************************************************/
CamCalDrv::~CamCalDrv()
{

}

int CamCalDrv::getDumpEnable()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("vendor.camcaldrv.log", value, "0");
    return atoi(value);
}

int CamCalDrv::GetCamCalCalDataBase(unsigned long i4SensorDevId,
        CAMERA_CAM_CAL_TYPE_ENUM a_eCamCalDataType,
        PCAM_CAL_DATA_STRUCT pCamCalData)
{
    unsigned int result;
    MINT32 i4CurrSensorId = 0xFF;
    MINT32 dumpEnable = getDumpEnable();

    CAM_CAL_DRV_LOG_IF(dumpEnable, "CamCalDrv::GetCamCalCalDataBase().");
    CAM_CAL_DRV_LOG_IF(dumpEnable, "CAMERA_CAM_CAL_TYPE_ENUM: %d", a_eCamCalDataType);

    // Check parameter
    if ((a_eCamCalDataType >= CAMERA_CAM_CAL_DATA_LIST) ||
        (pCamCalData == NULL) ) {
        CAM_CAL_DRV_ERR("[CAM_CAL_ERR_NO_CMD] i4SensorDevId(%lu),a_eCamCalDataType(%u),pCamCalData(%p)",
                i4SensorDevId, a_eCamCalDataType, pCamCalData);

        return CAM_CAL_ERR_NO_CMD;
    }

    // Get sensor id
    CAM_CAL_DRV_LOG_IF(dumpEnable, "Get the sensor id of sensor dev: %lu", i4SensorDevId);

    IHalSensorList* const pSensorList = MAKE_HalSensorList();
    SensorStaticInfo senStaticInfo;

    if (pSensorList == NULL)
        return CAM_CAL_ERR_NO_DEVICE;

    switch (i4SensorDevId)
    {
        case SENSOR_DEV_MAIN_3D:
            pSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN, &senStaticInfo);
            i4CurrSensorId = senStaticInfo.sensorDevID;
            CAM_CAL_DRV_LOG_IF(dumpEnable, "i4CurrSensorId 0x%x...", i4CurrSensorId);
            break;
        default:
            if ((IMGSENSOR_SENSOR_IDX_MAP(i4SensorDevId)) != IMGSENSOR_SENSOR_IDX_NONE) {
                pSensorList->querySensorStaticInfo(i4SensorDevId, &senStaticInfo);
                i4CurrSensorId = senStaticInfo.sensorDevID;
                CAM_CAL_DRV_LOG_IF(dumpEnable, "i4CurrSensorId 0x%x...", i4CurrSensorId);
            } else {
                CAM_CAL_DRV_ERR("[CAM_CAL_ERR_NO_DEVICE] i4SensorDevId = %lu", i4SensorDevId);
                return CAM_CAL_ERR_NO_DEVICE;
            }
            break;
    }

    // Get cal data
    CAM_CAL_DRV_LOG_IF(dumpEnable, "Start to get CamCal data!! CamCalDrv::GetCamCalCalDataBase().....");

    pCamCalData->deviceID = i4SensorDevId;
    pCamCalData->Command = a_eCamCalDataType;
    #ifdef CAM_CAL_SUPPORT  //20170616
    pCamCalData->sensorID = i4CurrSensorId;
    result = GetCameraCalData(i4CurrSensorId, (MUINT32*)pCamCalData);
    #else
    result = CamCalReturnErr[a_eCamCalDataType];
    #endif

    return result;
}

int CamCalDrv::GetCamCalCalData(unsigned long i4SensorDevId,
                          CAMERA_CAM_CAL_TYPE_ENUM a_eCamCalDataType,
	                      void *a_pCamCalData)
{
    unsigned int result;
    PCAM_CAL_DATA_STRUCT la_pCamCalData = (PCAM_CAL_DATA_STRUCT)a_pCamCalData;
    PCAM_CAL_DATA_STRUCT pCamcalData = &StCamCalCaldata;
    MINT32 dumpEnable = getDumpEnable();

    // Check parameter
    if (a_pCamCalData == NULL) {
        CAM_CAL_DRV_ERR("[CAM_CAL_ERR_NO_CMD] i4SensorDevId(%lu),a_eCamCalDataType(%u),a_pCamCalData(%p)",
                i4SensorDevId, a_eCamCalDataType, a_pCamCalData);

        return CAM_CAL_ERR_NO_CMD;
    }

    pthread_mutex_lock(&mCam_Cal_Mutex);

    result = GetCamCalCalDataBase(i4SensorDevId, a_eCamCalDataType, pCamcalData);

    if (result == CAM_CAL_ERR_NO_ERR)
        memcpy((unsigned char*)la_pCamCalData, (unsigned char*)pCamcalData, sizeof(CAM_CAL_DATA_STRUCT));

    pthread_mutex_unlock(&mCam_Cal_Mutex);

    // Dump error messages
    CAM_CAL_DRV_LOG_IF(dumpEnable, "All CAM_CAL ERROR if any.");
    for(MUINT32 mulIdx = 0; mulIdx < CAMERA_CAM_CAL_DATA_LIST; mulIdx++) {
        if(result & CamCalReturnErr[mulIdx])
            CAM_CAL_DRV_LOG_IF(dumpEnable,"Return ERROR %s",CamCalErrString[mulIdx]);
    }

    CAM_CAL_DRV_LOG_IF(dumpEnable, "Done get CamCal data!! CamCalDrv::GetCamCalCalData().....");

    return result;
}

int CamCalDrv::GetCamCalCalDataV2(unsigned long i4SensorDevId,
        CAMERA_CAM_CAL_TYPE_ENUM a_eCamCalDataType,
        void *a_pCamCalData, unsigned int calDataSize)
{
    unsigned int result;
    PCAM_CAL_DATA_STRUCT pCamcalData = &StCamCalCaldata;
    MINT32 dumpEnable = getDumpEnable();
    unsigned int expectSize = 0;

    switch(a_eCamCalDataType) {
        case CAMERA_CAM_CAL_DATA_MODULE_VERSION:
            expectSize = sizeof(CAM_CAL_MODULE_VERSION_STRUCT);
            break;
        case CAMERA_CAM_CAL_DATA_PART_NUMBER:
            expectSize = sizeof(CAM_CAL_PART_NUM_STRUCT);
            break;
        case CAMERA_CAM_CAL_DATA_SHADING_TABLE:
            expectSize = sizeof(CAM_CAL_LSC_DATA_STRUCT);
            break;
        case CAMERA_CAM_CAL_DATA_3A_GAIN:
            expectSize = sizeof(CAM_CAL_2A_DATA_STRUCT);
            break;
        case CAMERA_CAM_CAL_DATA_STEREO_DATA:
            expectSize = sizeof(CAM_CAL_STEREO_DATA_STRUCT);
            break;
        case CAMERA_CAM_CAL_DATA_PDAF:
            expectSize = sizeof(CAM_CAL_PDAF_DATA_STRUCT);
            break;
        case CAMERA_CAM_CAL_DATA_DUMP:
            expectSize = sizeof(CAM_CAL_DATA_STRUCT);
            break;
        case CAMERA_CAM_CAL_DATA_LENS_ID:
            expectSize = sizeof(CAM_CAL_LENS_ID_STRUCT);
            break;
        default:
            CAM_CAL_DRV_ERR("[CAM_CAL_ERR_NO_CMD] a_eCamCalDataType(0x%8x)", a_eCamCalDataType);
            return CAM_CAL_ERR_NO_CMD;
    }

    // Check parameter
    if (a_pCamCalData == NULL || expectSize != calDataSize) {
        CAM_CAL_DRV_ERR("[CAM_CAL_ERR_NO_CMD] i4SensorDevId(%lu),a_eCamCalDataType(%u),a_pCamCalData(%p), calDataSize(%u)",
                i4SensorDevId, a_eCamCalDataType, a_pCamCalData, calDataSize);

        return CAM_CAL_ERR_NO_CMD;
    }

    pthread_mutex_lock(&mCam_Cal_Mutex);

    result = GetCamCalCalDataBase(i4SensorDevId, a_eCamCalDataType, pCamcalData);

    // Copy result
    if (result == CAM_CAL_ERR_NO_ERR) {
        switch(a_eCamCalDataType) {
            case CAMERA_CAM_CAL_DATA_MODULE_VERSION:
                {
                    PCAM_CAL_MODULE_VERSION_STRUCT pRetData = (PCAM_CAL_MODULE_VERSION_STRUCT) a_pCamCalData;
                    memcpy(&(pRetData->DataVer),
                            &(pCamcalData->DataVer), sizeof(CAM_CAL_DATA_VER_ENUM));
                }
                break;
            case CAMERA_CAM_CAL_DATA_PART_NUMBER:
                {
                    PCAM_CAL_PART_NUM_STRUCT pRetData = (PCAM_CAL_PART_NUM_STRUCT) a_pCamCalData;
                    memcpy(&(pRetData->PartNumber),
                            &(pCamcalData->PartNumber), sizeof(pCamcalData->PartNumber));
                }
                break;
            case CAMERA_CAM_CAL_DATA_SHADING_TABLE:
                {
                    PCAM_CAL_LSC_DATA_STRUCT pRetData = (PCAM_CAL_LSC_DATA_STRUCT) a_pCamCalData;
                    memcpy(&(pRetData->SingleLsc),
                            &(pCamcalData->SingleLsc), sizeof(CAM_CAL_SINGLE_LSC_STRUCT));
                    memcpy(&(pRetData->N3DLsc),
                            &(pCamcalData->N3DLsc), sizeof(CAM_CAL_N3D_LSC_STRUCT));
                }
                break;
            case CAMERA_CAM_CAL_DATA_3A_GAIN:
                {
                    PCAM_CAL_2A_DATA_STRUCT pRetData = (PCAM_CAL_2A_DATA_STRUCT) a_pCamCalData;
                    memcpy(&(pRetData->Single2A),
                            &(pCamcalData->Single2A), sizeof(CAM_CAL_SINGLE_2A_STRUCT));
                }
                break;
            case CAMERA_CAM_CAL_DATA_STEREO_DATA:
                {
                    PCAM_CAL_STEREO_DATA_STRUCT pRetData = (PCAM_CAL_STEREO_DATA_STRUCT) a_pCamCalData;
                    memcpy(&(pRetData->Stereo_Data),
                            &(pCamcalData->Stereo_Data), sizeof(CAM_CAL_Stereo_Data_STRUCT));
                }
                break;
            case CAMERA_CAM_CAL_DATA_PDAF:
                {
                    PCAM_CAL_PDAF_DATA_STRUCT pRetData = (PCAM_CAL_PDAF_DATA_STRUCT) a_pCamCalData;
                    memcpy(&(pRetData->PDAF),
                            &(pCamcalData->PDAF), sizeof(CAM_CAL_PDAF_STRUCT));
                }
                break;
            case CAMERA_CAM_CAL_DATA_DUMP:
                {
                    PCAM_CAL_DATA_STRUCT pRetData = (PCAM_CAL_DATA_STRUCT) a_pCamCalData;
                    memcpy(pRetData, pCamcalData, sizeof(CAM_CAL_DATA_STRUCT));
                }
                break;
            case CAMERA_CAM_CAL_DATA_LENS_ID:
                {
                    PCAM_CAL_LENS_ID_STRUCT pRetData = (PCAM_CAL_LENS_ID_STRUCT) a_pCamCalData;
                    memcpy(&(pRetData->LensDrvId),
                            &(pCamcalData->LensDrvId), sizeof(CAM_CAL_LENS_ID_STRUCT));
                }
                break;
            default:
                result = CAM_CAL_ERR_NO_CMD;
                break;
        }
    }

    pthread_mutex_unlock(&mCam_Cal_Mutex);

    // Dump error messages
    CAM_CAL_DRV_LOG_IF(dumpEnable, "All CAM_CAL ERROR if any.");
    for(MUINT32 mulIdx = 0; mulIdx < CAMERA_CAM_CAL_DATA_LIST; mulIdx++) {
        if(result & CamCalReturnErr[mulIdx])
            CAM_CAL_DRV_LOG_IF(dumpEnable,"Return ERROR %s",CamCalErrString[mulIdx]);
    }

    CAM_CAL_DRV_LOG_IF(dumpEnable, "Done get CamCal data!! CamCalDrv::GetCamCalCalDataV2().....");

    return result;
}

/*******************************************************************************
*
********************************************************************************/

int
CamCalDrv::Init(unsigned long u4SensorID,void *a_pCamCalData)
{
	u4SensorID = 0;
	a_pCamCalData = NULL;

    return CAM_CAL_ERR_NO_ERR;
}

