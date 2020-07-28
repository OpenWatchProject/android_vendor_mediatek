/**
* @file FakeSensor.h
*
* FakeSensor Header File
*/

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/drv/isp_drv.h>

#define TS_LOGD(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define TS_LOGE(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)

using namespace NSCam;

class FakeSensor : public IHalSensor {
public:
    FakeSensor():
        m_pIspDrv(NULL),
        m_pIspReg(NULL)
        {}

    IspDrv      *m_pIspDrv;
    isp_reg_t   *m_pIspReg;

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
                                    ) { return 1; }
    MBOOL                   querySensorDynamicInfo(
                                      MUINT32 sensorIdx,
                                      SensorDynamicInfo *pSensorDynamicInfo
                                   );
    MINT32                  setDebugInfo(IBaseCamExif *pIBaseCamExif) { (void)pIBaseCamExif; return 0; }
    MINT32                  releaseI2CTrigLock() { return 0; }

};

class FakeSensorList : public IHalSensorList {
public:
    static IHalSensorList*      getTestModel();

    virtual MUINT               queryNumberOfSensors() const { return 3; }
    virtual IMetadata const&    queryStaticInfo(MUINT const index) const { (void)index; return mDummyMetadata; }
    virtual char const*         queryDriverName(MUINT const index) const { return "Fakegarnett"; }
    virtual MUINT               queryType(MUINT const index)const { (void)index; return 0; }
    virtual MUINT               queryFacingDirection(MUINT const index) const { (void)index; return 0; }
    virtual MUINT               querySensorDevIdx(MUINT const index) const { return (1 << (index&0x1)); };//main or sub
    virtual SensorStaticInfo const* querySensorStaticInfo(MUINT const index) const
                                    {
                                        (void)index;
                                        querySensorStaticInfo(index, &mSInfo);
                                        return &mSInfo;
                                    }
    virtual MVOID               querySensorStaticInfo(
                                        MUINT sensorDevIdx,
                                        SensorStaticInfo *pSensorStaticInfo
                                    ) const;
    virtual MUINT               searchSensors() { return 0; }
    virtual IHalSensor*         createSensor(
                                        char const* szCallerName,
                                        MUINT const index
                                    )
                                    {
                                        MUINT arrayOfIndex = index;
                                        return createSensor(szCallerName, 1, &arrayOfIndex);
                                    }
    virtual IHalSensor*         createSensor(
                                    char const* szCallerName,
                                    MUINT const uCountOfIndex,
                                    MUINT const*pArrayOfIndex
                                );
public:
    static SensorStaticInfo         mSInfo;
    static IMetadata                mDummyMetadata;
};


