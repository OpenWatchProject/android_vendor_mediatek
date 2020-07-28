#define LOG_TAG "flash_hal"
#include <mtkcam/common.h>
#include <camera_custom_nvram.h>
#include <flash_hal.h>
#include <isp_mgr.h>
#include <flash_mgr.h>
#include <flash_param.h>
#include <flash_tuning_custom.h>
#include "strobe_drv.h"
#include <aaa_log.h>
using namespace NSCam;


IHalFlash*
IHalFlash::
getInstance(MINT32 const i4SensorOpenIdx)
{
    return FlashHal::getInstance(i4SensorOpenIdx);
}

template <ESensorDev_T const eSensorDev>
class FlashHalDev : public FlashHal
{
public:
    static
    FlashHal*
getInstance()
{
        static FlashHalDev<eSensorDev> singleton;
        return &singleton;
}

    FlashHalDev()
        : FlashHal(eSensorDev)
    {}

    virtual ~FlashHalDev() {}
};

FlashHal*
FlashHal::
getInstance(MINT32 const i4SensorOpenIdx)
{
    MINT32 i4SensorDev = getSensorDevIdx(i4SensorOpenIdx);
    switch (i4SensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  FlashHalDev<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  FlashHalDev<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  FlashHalDev<ESensorDev_Sub>::getInstance();
    default:
        return  FlashHalDev<ESensorDev_Main>::getInstance();
    }
}

FlashHal::
FlashHal(ESensorDev_T const eSensorDev)
    :m_status(0)
    ,m_eSensorDev(eSensorDev)
    ,m_prevSensorDev(0)
{

}


MINT32
FlashHal::
getTorchStatus(MINT32 i4SensorOpenIndex)
{
/*  StrobeDrv* pStrobe = StrobeDrv::createInstance();
    m_prevSensorDev = pStrobe->getSensorDev();
*/
    MY_LOG("SensorDev=%d, status=%d", (int)this->m_eSensorDev, this->m_status);
    return this->m_status;
}


MINT32
FlashHal::
setTorchOnOff(MINT32 i4SensorOpenIndex, MBOOL en)
{
    int duty=0;
    int dutyLt=0;

    cust_getFlashHalTorchDuty(m_eSensorDev, &duty, &dutyLt);
    MY_LOG("SensorDev=%d, en=%d", (int)this->m_eSensorDev, en);
    if(this->m_eSensorDev == 2 && cust_isSubFlashSupport() == 0)
          return 1;
    StrobeDrv* pStrobe = StrobeDrv::getInstance(this->m_eSensorDev);
    StrobeDrv* pStrobe2 = StrobeDrv::getInstance(this->m_eSensorDev, 2);
    if(en==1){
        pStrobe->init();
        pStrobe->setDuty(duty);
        pStrobe->setTimeOutTime(0);
        int isLow;
        pStrobe->isLowPower(&isLow);
        if(isLow!=1 && duty>=0){
            pStrobe->setOnOff(1);
        }
        if(cust_isDualFlashSupport(this->m_eSensorDev)==1){
            pStrobe2->init();
            pStrobe2->setDuty(dutyLt);
            pStrobe2->setTimeOutTime(0);
            pStrobe2->isLowPower(&isLow);
            if(isLow!=1 && dutyLt>=0){
                pStrobe2->setOnOff(1);
            }
        }
        this->m_status = 1;
    }else{
        pStrobe->setOnOff(0);
        pStrobe->uninit();
        if(cust_isDualFlashSupport(this->m_eSensorDev)==1){
            pStrobe2->setOnOff(0);
            pStrobe2->uninit();
        }
        this->m_status = 0;
    }
	return 0;
}

MINT32
FlashHal::
getSensorDevIdx(MINT32 i4SensorOpenIdx)
{
    IHalSensorList *const pHalSensorList = IHalSensorList::get();
    MINT32 i4SensorDev = (!pHalSensorList)? 0: pHalSensorList->querySensorDevIdx(i4SensorOpenIdx);
    return i4SensorDev;
}
