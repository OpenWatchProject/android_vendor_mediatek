#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_CASE_AF_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_CASE_AF_H_

#include "ICaseControl.h"

enum CASE_AF_T
{
    CASE_AF_PREVIEW = 0,
    CASE_AF_1,
    CASE_AF_2,
    CASE_AF_3,
    CASE_AF_NUM
};

class CaseAF : public ICaseControl
{
public:
    CaseAF(){};

    ~CaseAF(){};
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
    MBOOL CaseAF::prepareMetaFunc<case_cocde>(MUINT32 magicNum, MetaSet_T& inMeta)

#define RET_PRE_META_FUNC(case_cocde)  \
    case case_cocde: \
        return &(CaseAF::prepareMetaFunc<case_cocde>); \
        break

//  PrepareMetaFunc_SetIsp
    template <MUINT32 case_cocde>
    static MBOOL prepareMetaFunc_Isp(MUINT32 magicNum, MetaSet_T& inMeta, TuningParam& inParam);

#define IMP_PRE_META_FUNC_ISP(case_cocde) \
    template <> \
    MBOOL CaseAF::prepareMetaFunc_Isp<case_cocde>(MUINT32 magicNum, MetaSet_T& inMeta, TuningParam& inParam)

#define RET_PRE_META_FUNC_ISP(case_cocde)  \
    case case_cocde: \
        return &(CaseAF::prepareMetaFunc_Isp<case_cocde>); \
        break

//  UpdateMetaFunc_Set
    template <MUINT32 case_cocde>
    static MBOOL updateMetaFunc(MUINT32 magicNum, MetaSet_T& inMeta);

#define IMP_UPD_META_FUNC(case_cocde) \
    template <> \
    MBOOL CaseAF::updateMetaFunc<case_cocde>(MUINT32 magicNum, MetaSet_T& inMeta)

#define RET_UPD_META_FUNC(case_cocde)  \
    case case_cocde: \
        return &(CaseAF::updateMetaFunc<case_cocde>); \
        break

//  UpdateMetaFunc_SetIsp
    template <MUINT32 case_cocde>
    static MBOOL updateMetaFunc_Isp(MUINT32 magicNum, MetaSet_T& inMeta, TuningParam& inParam);

#define IMP_UPD_META_FUNC_ISP(case_cocde) \
    template <> \
    MBOOL CaseAF::updateMetaFunc_Isp<case_cocde>(MUINT32 magicNum, MetaSet_T& inMeta, TuningParam& inParam)

#define RET_UPD_META_FUNC_ISP(case_cocde)  \
    case case_cocde: \
        return &(CaseAF::updateMetaFunc_Isp<case_cocde>); \
        break

//  VerifyFunc_Get
    template <MUINT32 case_cocde>
    static MBOOL verifyFunc(MUINT32 magicNum, const MetaSet_T& inMeta, char* failMsg);

#define IMP_VER_FUNC(case_cocde) \
    template <> \
    MBOOL CaseAF::verifyFunc<case_cocde>(MUINT32 magicNum, const MetaSet_T& inMeta, char* failMsg)

#define RET_VER_FUNC(case_cocde)  \
    case case_cocde: \
        return &(CaseAF::verifyFunc<case_cocde>); \
        break

//  VerifyFunc_SetIsp
    template <MUINT32 case_cocde>
    static MBOOL verifyFunc_Isp(MUINT32 magicNum, const MetaSet_T& inMeta, const TuningParam&, char* failMsg);

#define IMP_VER_FUNC_ISP(case_cocde) \
    template <> \
    MBOOL CaseAF::verifyFunc_Isp<case_cocde>(MUINT32 magicNum, const MetaSet_T& inMeta, const TuningParam&, char* failMsg)

#define RET_VER_FUNC_ISP(case_cocde)  \
    case case_cocde: \
        return &(CaseAF::verifyFunc_Isp<case_cocde>); \
        break
};

static CaseAF caseAF;

#endif