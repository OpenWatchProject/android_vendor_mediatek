 /**
* @file FakeSensor.h
*
* FakeSensor Header File
*/
#include "memory.h"

#ifndef MFALSE
#define MFALSE 0
#endif
#ifndef MTRUE
#define MTRUE 1
#endif
#ifndef MUINT8
typedef unsigned char MUINT8;
#endif

#ifndef MUINT32
typedef unsigned int MUINT32;
#endif
#ifndef MINT32
typedef int MINT32;
#endif
#ifndef MBOOL
typedef int MBOOL;
#endif

#ifndef MVOID
typedef void MVOID;
#endif

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
	typedef enum{
        eSeninf_1 = 0,
        eSeninf_2,
        eSeninf_3,
        eSeninf_4,
        eSeninf_5,
        eSeninf_max,
    }E_SENINF_ID;

    TS_FakeDrvSensor()
        : mfd(0)
        , mpSeninfHwRegBA(NULL)
        {
            memset((void*)mCaller, 0, sizeof(mCaller));
            memset((void*)mpSeninfDrv, 0, sizeof(mpSeninfDrv));
        }
    MBOOL                   init(char const* szCallerName);
    MBOOL                   uninit(char const* szCallerName);
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
    MVOID                   setPixelMode(MUINT32 pixMode = 1);
    MVOID                   adjustPatternSize(MUINT32 Height);
    MUINT32                 readReg(
                                E_SENINF_ID hw_module,
                                MUINT32 RegOffset
                            );
    MVOID                   writeReg(
                                E_SENINF_ID hw_module,
                                MUINT32 RegOffset,
                                MUINT32 Value
                            );
private:
    int              mfd;
    char             mCaller[32];
    unsigned char*   mpSeninfDrv[eSeninf_max];
    unsigned char*   mpSeninfHwRegBA;
    static int       mInitCount;
};


