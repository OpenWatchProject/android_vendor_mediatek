#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_ICASECONTROL_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_ICASECONTROL_H_

#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include "aaa_hal_if.h"
#include "aaa_utils.h"
// #include "MediaTypes.h"
//
using namespace std;
using namespace NSCam;
using namespace NS3Av3;

// typedef MBOOL (*PRE_SET_FUNC)(MUINT32, MetaSet_T&);
// typedef MBOOL (*PRE_SET_FUNC_ISP)(MUINT32, MetaSet_T&, TuningParam&);
typedef MBOOL (*SET_FUNC)(MUINT32, MetaSet_T&);
typedef MBOOL (*SET_FUNC_ISP)(MUINT32, MetaSet_T&, TuningParam&);
typedef MBOOL (*VER_FUNC)(MUINT32, const MetaSet_T&, char* msg);
typedef MBOOL (*VER_FUNC_ISP)(MUINT32, const MetaSet_T&, const TuningParam&, char* msg);

class ICaseControl{

public:
    static  ICaseControl    *createInstance();
    virtual                     ~ICaseControl(){};

public:
    virtual MVOID              destroyInstance() = 0;
    virtual SET_FUNC           getPrepareMetaFunc_Set(MUINT32 caseNum) = 0;
    virtual SET_FUNC_ISP       getPrepareMetaFunc_SetIsp(MUINT32 caseNum) = 0;
    virtual SET_FUNC           getUpdateMetaFunc_Set(MUINT32 caseNum) = 0;
    virtual SET_FUNC_ISP       getUpdateMetaFunc_SetIsp(MUINT32 caseNum) = 0;
    virtual VER_FUNC           getVerifyFunc_Get(MUINT32 caseNum) = 0;
    virtual VER_FUNC_ISP       getVerifyFunc_SetIsp(MUINT32 caseNum) = 0;
    virtual MBOOL              sendCmd(MUINT32 caseNum, void* p1, void* p2, void* p3) = 0;

    // inline MINT64   getTimeStamp_ns(MUINT32 i4TimeStamp_sec, MUINT32 i4TimeStamp_us) const
    // {
    //     return  i4TimeStamp_sec * 1000000000LL + i4TimeStamp_us * 1000LL;
    // }

};

#endif