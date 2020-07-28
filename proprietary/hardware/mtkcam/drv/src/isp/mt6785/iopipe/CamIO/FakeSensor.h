/**
* @file FakeSensor.h
*
* FakeSensor Header File
*/

#include <mtkcam/drv/IHalSensor.h>
#include <imem_drv.h>

#include "../../inc/isp_drv_cam.h"//for cfg the test pattern of seninf only



/*******************************************************************************
*
********************************************************************************/
#define TS_LOGD(fmt, arg...)    printf("TS' [%d][%s] " fmt"\n", (MUINT32)gettid(), __FUNCTION__, ##arg)
#define TS_LOGE(fmt, arg...)    printf("TS' [%d][%s:%d][%s] " fmt"\n", (MUINT32)gettid(), __FILE__, __LINE__, __FUNCTION__, ##arg)

/*******************************************************************************
*
********************************************************************************/

using namespace std;
using namespace NSCam;

class TS_FakeSensor : public IHalSensor {
public:
    TS_FakeSensor()
       : m_pIspDrv(NULL)
       , mSensorPixMode(0x1)
       , mPowerOnCnt(0)
       , mCamsvTgEn(MFALSE)
        {
            m_Fps = 30 * 10;
        }

    IspDrvImp       *m_pIspDrv;
    MUINT32          m_Fps;           // For Random FPS Use

    MVOID                   destroyInstance(
                                        char const* szCallerName = ""
                                    ) {(void)szCallerName;}

    MBOOL                   powerOn(
                                           char const* szCallerName,
                                           MUINT const uCountOfIndex,
                                           MUINT const*pArrayOfIndex
                                    );
    MBOOL                   powerOff(
                                          char const* szCallerName,
                                          MUINT const uCountOfIndex,
                                          MUINT const*pArrayOfIndex
                                    );
    MBOOL                   configure(
                                            MUINT const         uCountOfParam,
                                            IHalSensor::ConfigParam const*  pArrayOfParam
                                        );
    MINT                    sendCommand(
                                            MUINT sensorDevIdx,
                                            MUINTPTR cmd,
                                            MUINTPTR arg1,
                                            MUINTPTR arg2,
                                            MUINTPTR arg3
                                    );
    MBOOL                   querySensorDynamicInfo(
                                      MUINT32 sensorIdx,
                                      SensorDynamicInfo *pSensorDynamicInfo
                                   );
    MINT32                  setDebugInfo(IBaseCamExif *pIBaseCamExif) { (void)pIBaseCamExif; return 0; }
    MINT32                  releaseI2CTrigLock() { return 0; }
    enum eFakeAttr {
        eFake_PixelMode,
        eFake_CamsvChannel,
        eFake_FPS
    };
    MVOID                   setFakeAttribute(eFakeAttr eAttr, MUINTPTR value);

public:
    MINT32                  mSensorPixMode;
    MINT32                  mPowerOnCnt;
    MBOOL                   mCamsvTgEn;
};

class TS_FakeSensorList : public IHalSensorList {
public:
    static IHalSensorList*      getTestModel();

    virtual MUINT                   queryNumberOfSensors() const { return 4; }
    virtual IMetadata const&        queryStaticInfo(MUINT const index) const { (void)index; return mDummyMetadata; }
    virtual char const*             queryDriverName(MUINT const index) const { (void)index; return "Fakegarnett"; }
    virtual MUINT                   queryType(MUINT const index) const { (void)index; return 0; }
    virtual MUINT                   queryFacingDirection(MUINT const index) const { (void)index; return 0; }
    virtual MUINT                   querySensorDevIdx(MUINT const index) const { return (1 << index); }
    virtual SensorStaticInfo const* querySensorStaticInfo(MUINT const index) const
                                    {
                                        (void)index;
                                        querySensorStaticInfo(index, &mSInfo);
                                        return &mSInfo;
                                    }
    virtual MVOID                   querySensorStaticInfo (
                                        MUINT sensorDevIdx,
                                        SensorStaticInfo *pSensorStaticInfo
                                    ) const;
    virtual MUINT                   searchSensors() { return 0; }
    virtual IHalSensor*             createSensor(
                                        char const* szCallerName,
                                        MUINT const index
                                    )
                                    {
                                        MUINT arrayOfIndex = index;
                                        return createSensor(szCallerName, 1, &arrayOfIndex);
                                    }
    virtual IHalSensor*             createSensor(
                                        char const* szCallerName,
                                        MUINT const uCountOfIndex,
                                        MUINT const*pArrayOfIndex
                                    );

public:
    static SensorStaticInfo         mSInfo;
    static IMetadata                mDummyMetadata;
};


