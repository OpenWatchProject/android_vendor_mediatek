#ifndef __FLASH_HAL_H__
#define __FLASH_HAL_H__

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IHalFlash.h>
#include <isp_tuning.h>

using namespace NSCam;

class FlashHal : public IHalFlash
{
public:
    FlashHal(ESensorDev_T const eSensorDev);
    FlashHal();
    ~FlashHal(){};

public:
    static FlashHal* getInstance(MINT32 const i4SensorOpenIdx);
    void destroyInstance();

    /* functions */
    MINT32  getTorchStatus();
    MINT32  setTorchOnOff(MBOOL en);

    MINT32  egGetDutyRange(int* st, int* ed);
    MINT32  egGetStepRange(int* st, int* ed);
    MINT32  egSetMfDutyStep(int duty, int step);

private:
    static MINT32 getSensorDevIdx(MINT32 i4SensorOpenIndex);

private:
    MBOOL m_status;
    MINT32 m_eSensorDev;
    MINT32 m_prevSensorDev;

};

#endif
