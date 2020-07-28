#include "case_3a.h"

#define LOG_TAG "Case3A"
//
#include <stdio.h>
#include <stdlib.h>
//
#include <mtkcam/utils/std/Log.h>

// #include "MediaTypes.h"

#undef MY_LOG
#undef MY_ERR
#define MY_LOG(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_ERR(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

// getPrepareMetaFunc
IMP_PRE_META_FUNC(PREVIEW)
{
    UPDATE_ENTRY_SINGLE(inMeta.halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, (MUINT8)0);
    return MTRUE;
}

IMP_PRE_META_FUNC(CASE_3A_1)
{
    UPDATE_ENTRY_SINGLE(inMeta.halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, (MUINT8)1);
    return MTRUE;
}

// getUpdateMetaFunc
IMP_UPD_META_FUNC(PREVIEW)
{
    MY_LOG("updateMetaFunc PREVIEW");
    return MTRUE;
}

IMP_UPD_META_FUNC(CASE_3A_1)
{
    MY_LOG("updateMetaFunc CASE_3A_1");
    return MTRUE;
}

// getVerifyFunc
IMP_VER_FUNC(PREVIEW)
{
    return MTRUE;
}

IMP_VER_FUNC(CASE_3A_1)
{
    MBOOL ret = MTRUE;
    IMetadata metaExif;
    MBOOL fgOK = QUERY_ENTRY_SINGLE(inMeta.halMeta, MTK_3A_EXIF_METADATA, metaExif);
    MY_LOG("MTK_3A_EXIF_METADATA fgOK(%d)", fgOK);
    if(!fgOK) {
        strcpy(failMsg, "muse! Case CASE_3A_1 Fail");
        ret = MFALSE;
    }

    return ret;
}
//

IMP_VER_FUNC_ISP(PREVIEW)
{
    MY_LOG("verifyFunc_Isp PREVIEW");
    return MTRUE;
}

MVOID
Case3A::
destroyInstance()
{
}

SET_FUNC
Case3A::
getPrepareMetaFunc_Set(MUINT32 caseNum)
{
    CASE_3A_T case_cocde = (CASE_3A_T)caseNum;
    switch (case_cocde)
    {
        RET_PRE_META_FUNC(PREVIEW);
        RET_PRE_META_FUNC(CASE_3A_1);
        default:
            break;
    }
    return NULL;
}

SET_FUNC
Case3A::
getUpdateMetaFunc_Set(MUINT32 caseNum)
{
    CASE_3A_T case_cocde = (CASE_3A_T)caseNum;
    switch (case_cocde)
    {
        RET_UPD_META_FUNC(PREVIEW);
        RET_UPD_META_FUNC(CASE_3A_1);
        default:
            break;
    }
    return NULL;
}

VER_FUNC
Case3A::
getVerifyFunc_Get(MUINT32 caseNum)
{
    CASE_3A_T case_cocde = (CASE_3A_T)caseNum;
    switch (case_cocde)
    {
        RET_VER_FUNC(PREVIEW);
        RET_VER_FUNC(CASE_3A_1);
        default:
            break;
    }
    return NULL;
}

SET_FUNC_ISP
Case3A::
getPrepareMetaFunc_SetIsp(MUINT32 caseNum)
{
    return NULL;
}

SET_FUNC_ISP
Case3A::
getUpdateMetaFunc_SetIsp(MUINT32 caseNum)
{
    return NULL;
}

VER_FUNC_ISP
Case3A::
getVerifyFunc_SetIsp(MUINT32 caseNum)
{
    CASE_3A_T case_cocde = (CASE_3A_T)caseNum;
    switch (case_cocde)
    {
        RET_VER_FUNC_ISP(PREVIEW);
        default:
            break;
    }
    return NULL;
}

MBOOL
Case3A::
sendCmd(MUINT32 caseNum, void* p1, void* p2, void* p3)
{
    return MTRUE;
}