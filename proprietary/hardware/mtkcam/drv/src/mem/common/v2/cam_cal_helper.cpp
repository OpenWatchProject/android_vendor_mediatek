#define LOG_TAG "CamCalHelper"
#include <mtkcam/drv/mem/icam_cal_custom.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <cam_cal_helper.h>
#include <log/log.h>
#include <mtkcam/utils/std/ULog.h>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

#include <cam_cal.h>
#include <cam_cal_define.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_CAMCAL);

#define MY_CALLER_NAME "cam_cal"

#define CAM_CAL_SHOW_LOG

#ifdef CAM_CAL_SHOW_LOG
#define CAM_CAL_DRV_LOG(fmt, arg...)    CAM_ULOGMD(LOG_TAG " " fmt, ##arg)
#define CAM_CAL_DRV_ERR(fmt, arg...)    CAM_ULOGME(LOG_TAG "Err: %5d: " fmt, __LINE__, ##arg)
#else
#define CAM_CAL_DRV_LOG(fmt, arg...)    void(0)
#define CAM_CAL_DRV_ERR(fmt, arg...)    void(0)
#endif

#define CAM_CAL_DRV_LOG_IF(cond, ...)      do { if ( (cond) ) { CAM_CAL_DRV_LOG(__VA_ARGS__); } }while(0)

CamCalHelper::CamCalHelper() : mpCamCalCustom(NULL)
{
    CAM_CAL_DRV_LOG("Construct");

    for (int i = 0; i < IDX_MAX_CAM_NUMBER; ++i) {
        mpHalSensor[i] = NULL;
        mpEepromPreload[i] = NULL;
    }
}

CamCalHelper::~CamCalHelper()
{
    CAM_CAL_DRV_LOG("Destruct");

    if (mpCamCalCustom != NULL) {
        delete mpCamCalCustom;
        mpCamCalCustom = NULL;
    }
    for (int i = 0; i < IDX_MAX_CAM_NUMBER; ++i) {
        if (mpHalSensor[i] != NULL) {
            mpHalSensor[i]->powerOff(MY_CALLER_NAME, 1, (MUINT *)&i);
            mpHalSensor[i]->destroyInstance(MY_CALLER_NAME);
            mpHalSensor[i] = NULL;
        }
        if (mpEepromPreload[i] != NULL) {
            delete mpEepromPreload[i];
            mpEepromPreload[i] = NULL;
        }
    }
}

CamCalHelper*
CamCalHelper::getInstance()
{
    static CamCalHelper singleton;

    return &singleton;
}

unsigned int
CamCalHelper::getCamCalData(unsigned int* pGetSensorCalData)
{
    unsigned int ret;

    CAM_CAL_DRV_LOG("Get cal data.");

    init();
    if (mpCamCalCustom == NULL) {
        return CAM_CAL_ERR_NO_DEVICE;
    }

    ret = mpCamCalCustom->getCalData(pGetSensorCalData);

    if (ICamCalCustom::sDrvConfig.enableAutoPowerOn) {
        sensorPowerOff(((PCAM_CAL_DATA_STRUCT)pGetSensorCalData)->deviceID,
                ((PCAM_CAL_DATA_STRUCT)pGetSensorCalData)->sensorID);
    }

    return ret;
}

void
CamCalHelper::init()
{
    if (mpCamCalCustom == NULL)
        mpCamCalCustom = ICamCalCustom::createInstance(this);
}

int
CamCalHelper::getSensorIndexByDev(unsigned int devId)
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();

    if (pHalSensorList == NULL)
        return -1;

    unsigned int const sensorNum = pHalSensorList->queryNumberOfSensors();

    for (unsigned int i = 0; i < sensorNum; ++i) {
        if (pHalSensorList->querySensorDevIdx(i) == devId)
            return i;
    }

    // not found
    return -1;
}

bool
CamCalHelper::sensorPowerOn(unsigned int sensorDev, unsigned int sensorId)
{
    unsigned int devId = mpCamCalCustom->getEepromSensorDev(sensorDev, sensorId);
    CAM_CAL_DRV_LOG("Sensor dev 0x%x, EEPROM dev 0x%x", sensorDev, devId);

    int idx = getSensorIndexByDev(devId);

    if (idx < 0) {
        CAM_CAL_DRV_ERR("Bad dev id 0x%x and index %d", devId, idx);
        return false;
    }
    if (mpHalSensor[idx] == NULL) {
        CAM_CAL_DRV_LOG("Power on sensor index %u", idx);
        IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
        if (pHalSensorList == NULL)
            return false;
        mpHalSensor[idx] = pHalSensorList->createSensor(MY_CALLER_NAME, idx);
        if (!mpHalSensor[idx]->powerOn(MY_CALLER_NAME, 1, (MUINT *)&idx)) {
            mpHalSensor[idx]->destroyInstance(MY_CALLER_NAME);
            mpHalSensor[idx] = NULL;
            return false;
        }
    }
    return true;
}

bool
CamCalHelper::sensorPowerOff(unsigned int sensorDev, unsigned int sensorId)
{
    unsigned int devId = mpCamCalCustom->getEepromSensorDev(sensorDev, sensorId);
    CAM_CAL_DRV_LOG("Sensor dev 0x%x, EEPROM dev 0x%x", sensorDev, devId);

    int idx = getSensorIndexByDev(devId);

    if (idx < 0) {
        CAM_CAL_DRV_ERR("Bad dev id 0x%x and index %d", devId, idx);
        return false;
    }
    if (mpHalSensor[idx] != NULL) {
        CAM_CAL_DRV_LOG("Power off sensor index %u", idx);
        mpHalSensor[idx]->powerOff(MY_CALLER_NAME, 1, (MUINT *)&idx);
        mpHalSensor[idx]->destroyInstance(MY_CALLER_NAME);
        mpHalSensor[idx] = NULL;
    }
    return true;
}

int
CamCalHelper::readData(unsigned int sensorId, unsigned int deviceId,
        unsigned int offset, unsigned int length, unsigned char *data)
{
    int preloadIndex = IMGSENSOR_SENSOR_DUAL2IDX(deviceId);
    unsigned int bufSize = ICamCalCustom::sDrvConfig.preloadSizeLimit;

    if (ICamCalCustom::sDrvConfig.enablePreload) {
        // Preloading to memory and read from memory
        if (mpEepromPreload[preloadIndex] == NULL) {
            CAM_CAL_DRV_LOG("Preloading data %u bytes", bufSize);
            mpEepromPreload[preloadIndex] = new unsigned char[bufSize];
            if (readDataFromEeprom(sensorId, deviceId, 0, bufSize,
                        mpEepromPreload[preloadIndex]) != bufSize) {
                CAM_CAL_DRV_LOG("Preload data failed");
                delete mpEepromPreload[preloadIndex];
                mpEepromPreload[preloadIndex] = NULL;
            }
        }
        if ((mpEepromPreload[preloadIndex] != NULL) &&
                (offset + length <= bufSize)) {
            CAM_CAL_DRV_LOG("Read data from memory[%d]", preloadIndex);
            memcpy(data, mpEepromPreload[preloadIndex] + offset, length);
            return length;
        }
    }
    // Read data from EEPROM
    CAM_CAL_DRV_LOG("Read data from EEPROM");
    return readDataFromEeprom(sensorId, deviceId, offset, length, data);
}

int
CamCalHelper::readDataFromEeprom(unsigned int sensorId, unsigned int deviceId,
        unsigned int offset, unsigned int length, unsigned char *data)
{
    stCAM_CAL_INFO_STRUCT cam_calCfg;
    int ioctlerr;
    int CamcamFID = open("/dev/CAM_CAL_DRV", O_RDWR);

    if (CamcamFID == -1) {
        CAM_CAL_DRV_ERR("fd open fail!");
        return 0;
    }

    if (ICamCalCustom::sDrvConfig.enableAutoPowerOn) {
        // power on eeprom
        if (!sensorPowerOn(deviceId, sensorId)) {
            CAM_CAL_DRV_ERR("Power on sensor dev 0x%x failed", deviceId);
            close(CamcamFID);
            return 0;
        }
    }

    cam_calCfg.u4Offset = offset;
    cam_calCfg.u4Length = length;
    cam_calCfg.pu1Params = (u8 *) data;
    cam_calCfg.sensorID = sensorId;
    cam_calCfg.deviceID = deviceId;

    ioctlerr = ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);
    close(CamcamFID);

    CAM_CAL_DRV_LOG("ioctl return %d", ioctlerr);
    if (ioctlerr > 0)
        return ioctlerr;
    return 0;
}
