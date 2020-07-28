#include "case_af.h"

#define LOG_TAG "CaseAF"
//
#include <stdio.h>
#include <stdlib.h>
//
#include <mtkcam/utils/std/Log.h>

#undef MY_LOG
#undef MY_ERR
#define MY_LOG(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_ERR(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

// PrepareMetaFunc
IMP_PRE_META_FUNC(CASE_AF_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_PRE_META_FUNC(CASE_AF_1)
{
    MBOOL ret = MTRUE;
    MY_LOG("PrepareMetaFunc CASE_AF_1");
    return ret;
}

// UpdateMetaFunc
IMP_UPD_META_FUNC(CASE_AF_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_UPD_META_FUNC(CASE_AF_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

// VerifyFunc
IMP_VER_FUNC(CASE_AF_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_VER_FUNC(CASE_AF_1)
{
    MBOOL ret = MTRUE;
    MY_LOG("VerifyFunc CASE_AF_1");
    return ret;
}

// PrepareMetaFunc_Isp
IMP_PRE_META_FUNC_ISP(CASE_AF_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_PRE_META_FUNC_ISP(CASE_AF_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

// UpdateMetaFunc_Isp
IMP_UPD_META_FUNC_ISP(CASE_AF_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_UPD_META_FUNC_ISP(CASE_AF_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

// VerifyFunc_Isp
IMP_VER_FUNC_ISP(CASE_AF_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

// VerifyFunc_Isp
IMP_VER_FUNC_ISP(CASE_AF_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

MVOID
CaseAF::
destroyInstance()
{
}

// PrepareMetaFunc
SET_FUNC
CaseAF::
getPrepareMetaFunc_Set(MUINT32 caseNum)
{
    CASE_AF_T case_cocde = (CASE_AF_T)caseNum;
    switch (case_cocde)
    {
        RET_PRE_META_FUNC(CASE_AF_PREVIEW);
        RET_PRE_META_FUNC(CASE_AF_1);
        default:
            break;
    }
    return NULL;
}

SET_FUNC_ISP
CaseAF::
getPrepareMetaFunc_SetIsp(MUINT32 caseNum)
{
    CASE_AF_T case_cocde = (CASE_AF_T)caseNum;
    switch (case_cocde)
    {
        RET_PRE_META_FUNC_ISP(CASE_AF_PREVIEW);
        RET_PRE_META_FUNC_ISP(CASE_AF_1);
        default:
            break;
    }
    return NULL;
}

// UpdateMetaFunc
SET_FUNC
CaseAF::
getUpdateMetaFunc_Set(MUINT32 caseNum)
{
    CASE_AF_T case_cocde = (CASE_AF_T)caseNum;
    switch (case_cocde)
    {
        RET_UPD_META_FUNC(CASE_AF_PREVIEW);
        RET_UPD_META_FUNC(CASE_AF_1);
        default:
            break;
    }
    return NULL;
}

SET_FUNC_ISP
CaseAF::
getUpdateMetaFunc_SetIsp(MUINT32 caseNum)
{
    CASE_AF_T case_cocde = (CASE_AF_T)caseNum;
    switch (case_cocde)
    {
        RET_UPD_META_FUNC_ISP(CASE_AF_PREVIEW);
        RET_UPD_META_FUNC_ISP(CASE_AF_1);
        default:
            break;
    }
    return NULL;
}

// VerifyFunc
VER_FUNC
CaseAF::
getVerifyFunc_Get(MUINT32 caseNum)
{
    CASE_AF_T case_cocde = (CASE_AF_T)caseNum;
    switch (case_cocde)
    {
        RET_VER_FUNC(CASE_AF_PREVIEW);
        RET_VER_FUNC(CASE_AF_1);
        default:
            break;
    }
    return NULL;
}

VER_FUNC_ISP
CaseAF::
getVerifyFunc_SetIsp(MUINT32 caseNum)
{
    CASE_AF_T case_cocde = (CASE_AF_T)caseNum;
    switch (case_cocde)
    {
        RET_VER_FUNC_ISP(CASE_AF_PREVIEW);
        RET_VER_FUNC_ISP(CASE_AF_1);
        default:
            break;
    }
    return NULL;
}

MBOOL
CaseAF::
sendCmd(MUINT32 caseNum, void* p1, void* p2, void* p3)
{
    MBOOL ret = MTRUE;
    return ret;
}