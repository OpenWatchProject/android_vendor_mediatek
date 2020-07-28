 /**
* @file FakeSensor.h
*
* FakeSensor Header File
*/

#include <isp_drv_cam.h> //for cfg the test pattern of seninf only
#include "Emulation/project.h"  //this is for fakesensor's register address


/*******************************************************************************
*
********************************************************************************/
#define TS_LOGD(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define TS_LOGE(fmt, arg...)    printf("TS' [%s:%d][%s] " fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##arg)

/*******************************************************************************
*
********************************************************************************/

using namespace std;


class TS_FakeDrvSensor {
public:
    TS_FakeDrvSensor(): mSensorPixMode(1)
        {}

    static IspDrvImp       *m_pIspDrv;

    MBOOL                   powerOn(
                                           char const* szCallerName,
                                           MUINT32 const uCountOfIndex,
                                           MUINT32 const*pArrayOfIndex,
                                           MUINT32 const*pSetingTbl = NULL
                                    );
    MBOOL                   powerOff(
                                          char const* szCallerName,
                                          MUINT32 const uCountOfIndex,
                                          MUINT32 const*pArrayOfIndex
                                    );
    void                    setPixelMode(MUINT32 pixMode = 1);
    void                    adjustPatternSize(MUINT32 Height);
public:
    MINT32                  mSensorPixMode;

};


