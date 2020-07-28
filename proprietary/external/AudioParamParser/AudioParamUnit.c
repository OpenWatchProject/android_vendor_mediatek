/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

/*
 * Description:
 *   Implement ParamUnit related APIs
 */

#include "AudioParamParserPriv.h"

EXPORT ParamUnit *paramUnitCreate(AudioType *audioType, int id, Param *param) {
    size_t numOfParam, i;
    ParamUnit *paramUnit = (ParamUnit *)malloc(sizeof(ParamUnit));
    if (!paramUnit) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    paramUnit->paramId = id;
    paramUnit->refCount = 0;
    paramUnit->audioType = audioType;
    paramUnit->paramHash = param;

    /* Update param's param unit info */
    numOfParam = paramUnitGetNumOfParam(paramUnit);
    for (i = 0; i < numOfParam; i++) {
        Param *param = paramUnitGetParamByIndex(paramUnit, i);
        param->paramUnit = paramUnit;
    }

    return paramUnit;
}

EXPORT ParamUnit *paramUnitClone(ParamUnit *oldParamUnit) {
    Param *item;
    ParamUnit *paramUnit;

    if (!oldParamUnit) {
        ERR_LOG("Original ParamUnit is NULL\n");
        return NULL;
    }

    paramUnit = (ParamUnit *)malloc(sizeof(ParamUnit));
    if (!paramUnit) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    paramUnit->paramId = oldParamUnit->paramId;
    paramUnit->refCount = oldParamUnit->refCount;
    paramUnit->audioType = oldParamUnit->audioType;
    paramUnit->paramHash = paramHashClone(oldParamUnit->paramHash);

    /* Update param's param unit info */
    if (paramUnit->paramHash) {
        for (item = paramUnit->paramHash; item != NULL; item = item->hh.next) {
            item->paramUnit = paramUnit;
        }
    }

    return paramUnit;
}

EXPORT void paramUnitRelease(ParamUnit *paramUnit) {
    if (paramUnit) {
        /* Free ParamUnit's param hash */
        if (paramUnit->paramHash) {
            Param *tmp, *item;
            HASH_ITER(hh, paramUnit->paramHash, item, tmp) {
                if (paramUnit->paramHash) {
                    HASH_DEL(paramUnit->paramHash, item);
                    paramRelease(item);
                }
            }
            free(paramUnit->paramHash);
        }
        free(paramUnit);
    }
}

EXPORT size_t paramUnitGetNumOfParam(ParamUnit *paramUnit) {
    if (!paramUnit) {
        ERR_LOG("paramUnit is NULL!\n");
        return 0;
    }

    return HASH_COUNT(paramUnit->paramHash);
}

EXPORT Param *paramUnitGetParamByIndex(ParamUnit *paramUnit, size_t index) {
    Param *param = NULL;
    size_t i = 0;

    if (!paramUnit) {
        ERR_LOG("paramUnit is NULL!\n");
        return NULL;
    }

    for (param = paramUnit->paramHash; param ; param = param->hh.next) {
        if (index == i++) {
            return param;
        }
    }

    return NULL;
}

EXPORT Param *paramUnitGetParamByName(ParamUnit *paramUnit, const char *name) {
    Param *param = NULL;

    if (!paramUnit) {
        ERR_LOG("paramUnit is NULL!\n");
        return NULL;
    }

    INFO_LOG("AudioType = %s, name = %s\n", paramUnit->audioType ? paramUnit->audioType->name : "NULL", name);

    HASH_FIND_STR(paramUnit->paramHash, name, param);

    if (param && appDebugLevel <= DEBUG_LEVEL) {
        utilShowParamValue(param);
    }

    DEBUG_LOG("name = %s, param data = 0x%p, size = "APP_SIZE_T_FT"\n", name, param ? param->data : NULL, param ? param->arraySize : 0);
    return param;
}


EXPORT APP_STATUS paramUnitGetFieldVal(ParamUnit *paramUnit, const char *paramName, const char *fieldName, unsigned int *val) {
    ParamInfo *paramInfo;
    FieldInfo *fieldInfo;
    Param *param;

    if (!paramUnit) {
        ERR_LOG("paramUnit is NULL!\n");
        return APP_ERROR;
    }

    /* Query field Info */
    paramInfo = audioTypeGetParamInfoByName(paramUnit->audioType, paramName);
    if (!paramInfo) {
        WARN_LOG("Cannot find paramInfo. (param = %s\n)", paramName);
        return APP_ERROR;
    }

    fieldInfo = paramInfoGetFieldInfoByName(paramInfo, fieldName);
    if (!fieldInfo) {
        WARN_LOG("Cannot find fieldInfo. (fieldName = %s\n)", fieldName);
        return APP_ERROR;
    }

    /* Query param */
    param = paramUnitGetParamByName(paramUnit, paramName);
    if (!param) {
        WARN_LOG("Cannot get param. (name = %s)\n", paramName);
        return APP_ERROR;
    }

    /* Query field val */
    return paramGetFieldVal(param, fieldInfo, val);
}

EXPORT ParamInfo *paramUnitGetParamInfo(ParamUnit *paramUnit, const char *paramInfoName) {
    if (!paramUnit) {
        ERR_LOG("paramUnit is NULL!\n");
        return NULL;
    }

    return audioTypeGetParamInfoByName(paramUnit->audioType, paramInfoName);
}

EXPORT FieldInfo *paramUnitGetFieldInfo(ParamUnit *paramUnit, const char *paramName, const char *fieldName) {
    ParamInfo *paramInfo;
    if (!paramUnit || !paramName || !fieldName) {
        WARN_LOG("Cannot get field info. (paramUnit id=%d, paramInfoName=%s, fieldInfoName=%s\n)", paramUnit ? paramUnit->paramId : -1, paramName, fieldName);
        return NULL;
    }

    paramInfo = audioTypeGetParamInfoByName(paramUnit->audioType, paramName);
    if (paramInfo) {
        return paramInfoGetFieldInfoByName(paramInfo, fieldName);
    } else {
        return NULL;
    }
}
