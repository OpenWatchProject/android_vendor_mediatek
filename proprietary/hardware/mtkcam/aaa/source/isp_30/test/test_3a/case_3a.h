#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_CASE_3A_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_CASE_3A_H_

#include "ICaseControl.h"

enum CASE_3A_T
{
    PREVIEW = 0,
    CASE_3A_1,
    CASE_3A_2,
    CASE_3A_3,
    CASE_3A_NUM
};

class Case3A : public ICaseControl
{
public:
    Case3A(){};

    ~Case3A(){};
public:

    virtual MVOID              destroyInstance();
    virtual SET_FUNC           getPrepareMetaFunc_Set(MUINT32 caseNum);
    virtual SET_FUNC_ISP       getPrepareMetaFunc_SetIsp(MUINT32 caseNum);
    virtual SET_FUNC           getUpdateMetaFunc_Set(MUINT32 caseNum);
    virtual SET_FUNC_ISP       getUpdateMetaFunc_SetIsp(MUINT32 caseNum);
    virtual VER_FUNC           getVerifyFunc_Get(MUINT32 caseNum);
    virtual VER_FUNC_ISP       getVerifyFunc_SetIsp(MUINT32 caseNum);
    virtual MBOOL              sendCmd(MUINT32 caseNum, void* p1, void* p2, void* p3);

protected:

//  PrepareMetaFunc_Set
    template <MUINT32 case_cocde>
    static MBOOL prepareMetaFunc(MUINT32 magicNum, MetaSet_T& inMeta);

#define IMP_PRE_META_FUNC(case_cocde) \
    template <> \
    MBOOL Case3A::prepareMetaFunc<case_cocde>(MUINT32 magicNum, MetaSet_T& inMeta)

#define RET_PRE_META_FUNC(case_cocde)  \
    case case_cocde: \
        return &(Case3A::prepareMetaFunc<case_cocde>); \
        break

//  UpdateMetaFunc_Set
    template <MUINT32 case_cocde>
    static MBOOL updateMetaFunc(MUINT32 magicNum, MetaSet_T& inMeta);

#define IMP_UPD_META_FUNC(case_cocde) \
    template <> \
    MBOOL Case3A::updateMetaFunc<case_cocde>(MUINT32 magicNum, MetaSet_T& inMeta)

#define RET_UPD_META_FUNC(case_cocde)  \
    case case_cocde: \
        return &(Case3A::updateMetaFunc<case_cocde>); \
        break

//  VerifyFunc_Get
    template <MUINT32 case_cocde>
    static MBOOL verifyFunc(MUINT32 magicNum, const MetaSet_T& inMeta, char* failMsg);

#define IMP_VER_FUNC(case_cocde) \
    template <> \
    MBOOL Case3A::verifyFunc<case_cocde>(MUINT32 magicNum, const MetaSet_T& inMeta, char* failMsg)

#define RET_VER_FUNC(case_cocde)  \
    case case_cocde: \
        return &(Case3A::verifyFunc<case_cocde>); \
        break

//  VerifyFunc_SetIsp
    template <MUINT32 case_cocde>
    static MBOOL verifyFunc_Isp(MUINT32 magicNum, const MetaSet_T& inMeta, const TuningParam&, char* failMsg);

#define IMP_VER_FUNC_ISP(case_cocde) \
    template <> \
    MBOOL Case3A::verifyFunc_Isp<case_cocde>(MUINT32 magicNum, const MetaSet_T& inMeta, const TuningParam&, char* failMsg)

#define RET_VER_FUNC_ISP(case_cocde)  \
    case case_cocde: \
        return &(Case3A::verifyFunc_Isp<case_cocde>); \
        break
};

static Case3A case3A;

#endif