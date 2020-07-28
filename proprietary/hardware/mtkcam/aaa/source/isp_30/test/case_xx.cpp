#include "case_xx.h"

#define LOG_TAG "CaseXX"
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
IMP_PRE_META_FUNC(CASE_XX_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_PRE_META_FUNC(CASE_XX_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

// UpdateMetaFunc
IMP_UPD_META_FUNC(CASE_XX_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_UPD_META_FUNC(CASE_XX_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

// VerifyFunc
IMP_VER_FUNC(CASE_XX_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_VER_FUNC(CASE_XX_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

// PrepareMetaFunc_Isp
IMP_PRE_META_FUNC_ISP(CASE_XX_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_PRE_META_FUNC_ISP(CASE_XX_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

// UpdateMetaFunc_Isp
IMP_UPD_META_FUNC_ISP(CASE_XX_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

IMP_UPD_META_FUNC_ISP(CASE_XX_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

// VerifyFunc_Isp
IMP_VER_FUNC_ISP(CASE_XX_PREVIEW)
{
    MBOOL ret = MTRUE;
    return ret;
}

// VerifyFunc_Isp
IMP_VER_FUNC_ISP(CASE_XX_1)
{
    MBOOL ret = MTRUE;
    return ret;
}

MVOID
CaseXX::
destroyInstance()
{
}

// PrepareMetaFunc
SET_FUNC
CaseXX::
getPrepareMetaFunc_Set(MUINT32 caseNum)
{
    CASE_XX_T case_cocde = (CASE_XX_T)caseNum;
    switch (case_cocde)
    {
        RET_PRE_META_FUNC(CASE_XX_PREVIEW);
        RET_PRE_META_FUNC(CASE_XX_1);
        default:
            break;
    }
    return NULL;
}

SET_FUNC_ISP
CaseXX::
getPrepareMetaFunc_SetIsp(MUINT32 caseNum)
{
    CASE_XX_T case_cocde = (CASE_XX_T)caseNum;
    switch (case_cocde)
    {
        RET_PRE_META_FUNC_ISP(CASE_XX_PREVIEW);
        RET_PRE_META_FUNC_ISP(CASE_XX_1);
        default:
            break;
    }
    return NULL;
}

// UpdateMetaFunc
SET_FUNC
CaseXX::
getUpdateMetaFunc_Set(MUINT32 caseNum)
{
    CASE_XX_T case_cocde = (CASE_XX_T)caseNum;
    switch (case_cocde)
    {
        RET_UPD_META_FUNC(CASE_XX_PREVIEW);
        RET_UPD_META_FUNC(CASE_XX_1);
        default:
            break;
    }
    return NULL;
}

SET_FUNC_ISP
CaseXX::
getUpdateMetaFunc_SetIsp(MUINT32 caseNum)
{
    CASE_XX_T case_cocde = (CASE_XX_T)caseNum;
    switch (case_cocde)
    {
        RET_UPD_META_FUNC_ISP(CASE_XX_PREVIEW);
        RET_UPD_META_FUNC_ISP(CASE_XX_1);
        default:
            break;
    }
    return NULL;
}

// VerifyFunc
VER_FUNC
CaseXX::
getVerifyFunc_Get(MUINT32 caseNum)
{
    CASE_XX_T case_cocde = (CASE_XX_T)caseNum;
    switch (case_cocde)
    {
        RET_VER_FUNC(CASE_XX_PREVIEW);
        RET_VER_FUNC(CASE_XX_1);
        default:
            break;
    }
    return NULL;
}

VER_FUNC_ISP
CaseXX::
getVerifyFunc_SetIsp(MUINT32 caseNum)
{
    CASE_XX_T case_cocde = (CASE_XX_T)caseNum;
    switch (case_cocde)
    {
        RET_VER_FUNC_ISP(CASE_XX_PREVIEW);
        RET_VER_FUNC_ISP(CASE_XX_1);
        default:
            break;
    }
    return NULL;
}

MBOOL
CaseXX::
sendCmd(MUINT32 caseNum, void* p1, void* p2, void* p3)
{
    MBOOL ret = MTRUE;
    return ret;
}