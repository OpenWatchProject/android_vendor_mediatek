#ifndef __FLASH_HAL_H__
#define __FLASH_HAL_H__

#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/hal/IHalFlash.h>
#include <isp_tuning.h>

using namespace NSCam;

class FlashHal : public IHalFlash
{
protected:
    FlashHal(ESensorDev_T const eSensorDev);
    ~FlashHal(){};
public:
    static FlashHal* getInstance(MINT32 const i4SensorOpenIdx);

    /* functions */
    MINT32 getTorchStatus(MINT32 i4SensorOpenIndex);
    MINT32 setTorchOnOff(MINT32 i4SensorOpenIndex, MBOOL en);
private:
    static MINT32 getSensorDevIdx(MINT32 i4SensorOpenIndex);

private:
    MBOOL m_status;
    MINT32 m_eSensorDev;
    MINT32 m_prevSensorDev;
};

#endif
