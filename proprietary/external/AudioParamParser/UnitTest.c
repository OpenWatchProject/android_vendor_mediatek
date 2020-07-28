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
 *   Implement unit test cases
 */

#include "AudioParamParserPriv.h"

typedef APP_STATUS(*TEST_FUN)(AppHandle *appHandle);

typedef struct {
    AppHandle *appHandle;
    int times;
    TEST_FUN fun;
} ThreadParam;

EXPORT APP_STATUS unitTest(AppHandle *appHandle) {
    APP_STATUS res = APP_NO_ERROR;
    APP_STATUS finalRes = APP_NO_ERROR;

    printf("===APP internal unit test===\n");
#if 0
    res = testAppHandleInitUninit();
    if (res == APP_ERROR) {
        finalRes = APP_ERROR;
    }
    printf("testAppHandleInitUninit: %s\n", res ? "pass" : "fail");
#endif
    res = testReadWriteParam(appHandle);
    if (res == APP_ERROR) {
        finalRes = APP_ERROR;
    }
    printf("testReadWriteParam: %s\n", res ? "pass" : "fail");

    res = testMemoryLeak(appHandle);
    if (res == APP_ERROR) {
        finalRes = APP_ERROR;
    }
    printf("testMemoryLeak: %s\n", res ? "pass" : "fail");

    res = testAudioTypeLock(appHandle);
    if (res == APP_ERROR) {
        finalRes = APP_ERROR;
    }
    printf("testAudioTypeLock: %s\n", res ? "pass" : "fail");
    printf("=============================\n");

    return finalRes;
}


EXPORT void *commonThreadLoop(void *arg) {
    ThreadParam threadParam = *(ThreadParam *)arg;
    int i = 0 ;
    for (i = 0; i < threadParam.times; i++) {
        (*threadParam.fun)(threadParam.appHandle);
        INFO_LOG("2nd thread round = %d\n", i);
    }
    return NULL;
}

EXPORT void testDebugLevel() {
    appSetDebugLevel(ERR_LEVEL);
    ERR_LOG("error - pass\n");
    WARN_LOG("warn - ok\n");
    INFO_LOG("info - ok\n");
    DEBUG_LOG("debug - ok\n");

    appSetDebugLevel(WARN_LEVEL);
    ERR_LOG("error - fail\n");
    WARN_LOG("warn - pass\n");
    INFO_LOG("info - ok\n");
    DEBUG_LOG("debug - ok\n");

    appSetDebugLevel(INFO_LEVEL);
    ERR_LOG("error - fail\n");
    WARN_LOG("warn - fail\n");
    INFO_LOG("info - pass\n");
    DEBUG_LOG("debug - ok\n");

    appSetDebugLevel(DEBUG_LEVEL);
    ERR_LOG("error - fail\n");
    WARN_LOG("warn - fail\n");
    INFO_LOG("info - fail\n");
    DEBUG_LOG("debug - pass\n");
}

EXPORT void testHashParamTree() {
    ParamTree *item;
    ParamTree *ParamTreeHash = NULL, *tmp = NULL; /* Used for hash */
    const char *key;

    item = (ParamTree *)malloc(sizeof(ParamTree));
    if (item) {
        item->categoryPath = "NB,Normal,,";
        item->paramId = 1;
        HASH_ADD_KEYPTR(hh, ParamTreeHash, item->categoryPath, strlen(item->categoryPath), item);
    } else {
        ERR_LOG("malloc fail!\n");
    }

    item = (ParamTree *)malloc(sizeof(ParamTree));
    if (item) {
        item->categoryPath = "WB,Normal,,";
        item->paramId = 7;
        HASH_ADD_KEYPTR(hh, ParamTreeHash, item->categoryPath, strlen(item->categoryPath), item);
    } else {
        ERR_LOG("malloc fail!\n");
    }

    item = (ParamTree *)malloc(sizeof(ParamTree));
    if (item) {
        item->categoryPath = "NB,Normal,0,GSM,";
        item->paramId = 1;
        HASH_ADD_KEYPTR(hh, ParamTreeHash, item->categoryPath, strlen(item->categoryPath), item);
    } else {
        ERR_LOG("malloc fail!\n");
    }

    item = (ParamTree *)malloc(sizeof(ParamTree));
    if (item) {
        item->categoryPath = "NB,HAC,0,GSM";
        item->paramId = 0;
        HASH_ADD_KEYPTR(hh, ParamTreeHash, item->categoryPath, strlen(item->categoryPath), item);
    } else {
        ERR_LOG("malloc fail!\n");
    }

    /* Find string */
    key = "WB,,,";
    HASH_FIND_STR(ParamTreeHash, key, item);
    if (item) { printf("[%s] id is %d\n", key, item->paramId); }

    /* Free hash table content */
    HASH_ITER(hh, ParamTreeHash, item, tmp) {
        HASH_DEL(ParamTreeHash, item);
        free(item);
    }
}

EXPORT void testHashParamUnit() {
    ParamUnit *item;
    ParamUnit *ParamUnitHash = NULL, *tmp = NULL; /* Used for hash */
    int key;

    item = (ParamUnit *)malloc(sizeof(ParamUnit));
    if (item) {
        item->paramId = 0;
        item->paramHash = (Param *) 0x1;
        HASH_ADD_INT(ParamUnitHash, paramId, item);
    } else {
        ERR_LOG("malloc fail!\n");
    }

    item = (ParamUnit *)malloc(sizeof(ParamUnit));
    if (item) {
        item->paramId = 1;
        item->paramHash = (Param *)0x2;
        HASH_ADD_INT(ParamUnitHash, paramId, item);
    } else {
        ERR_LOG("malloc fail!\n");
    }

    item = (ParamUnit *)malloc(sizeof(ParamUnit));
    if (item) {
        item->paramId = 7;
        item->paramHash = (Param *)0x3;
        HASH_ADD_INT(ParamUnitHash, paramId, item);
    } else {
        ERR_LOG("malloc fail!\n");
    }

    /* Find string */
    key = 0;
    HASH_FIND_INT(ParamUnitHash, &key, item);
    if (item) {
        INFO_LOG("[%d] Param is %p\n", key, item->paramHash);
    } else {
        ERR_LOG("Cannot find parameter!");
    }

    key = 1;
    HASH_FIND_INT(ParamUnitHash, &key, item);
    if (item) {
        INFO_LOG("[%d] Param is %p\n", key, item->paramHash);
    } else {
        ERR_LOG("Cannot find parameter!");
    }

    key = 7;
    HASH_FIND_INT(ParamUnitHash, &key, item);
    if (item) {
        INFO_LOG("[%d] Param is %p\n", key, item->paramHash);
    } else {
        ERR_LOG("Cannot find parameter!");
    }


    /* Free hash table content */
    HASH_ITER(hh, ParamUnitHash, item, tmp) {
        HASH_DEL(ParamUnitHash, item);
        free(item);
    }
}

EXPORT void testHashParam() {
    Param *item;
    Param *paramHash = NULL, *tmp = NULL; /* Used for hash */
    const char *key;

    item = (Param *)malloc(sizeof(Param));
    if (item) {
        memset(item, 0, sizeof(Param));
        item->name = "speech_mode_para";
        item->data = "0x0011,0x2233,0x4455";
        HASH_ADD_KEYPTR(hh, paramHash, item->name, strlen(item->name), item);
    }

    item = (Param *)malloc(sizeof(Param));
    if (item) {
        memset(item, 0, sizeof(Param));
        item->name = "uint_param";
        item->data = "4294967295";
        HASH_ADD_KEYPTR(hh, paramHash, item->name, strlen(item->name), item);
    }

    item = (Param *)malloc(sizeof(Param));
    if (item) {
        memset(item, 0, sizeof(Param));
        item->name = "float_param";
        item->data = "0.1234567";
        HASH_ADD_KEYPTR(hh, paramHash, item->name, strlen(item->name), item);
    }

    /* Find string */
    key = "speech_mode_para";
    HASH_FIND_STR(paramHash, key, item);
    if (item) { INFO_LOG("[%s] value is %s\n", key, (char *)item->data); }

    key = "uint_param";
    HASH_FIND_STR(paramHash, key, item);
    if (item) { INFO_LOG("[%s] value is %s\n", key, (char *)item->data); }

    key = "float_param";
    HASH_FIND_STR(paramHash, key, item);
    if (item) { INFO_LOG("[%s] value is %s\n", key, (char *)item->data); }

    /* Free hash table content */
    HASH_ITER(hh, paramHash, item, tmp) {
        HASH_DEL(paramHash, item);
        free(item);
    }
}

void testCb(AppHandle *appHandle, const char *audioTypeName) {
    printf("XML file changed. (cus folder = %s, audioType = %s)\n", appHandle->xmlCusDir, audioTypeName);
}

EXPORT void notifyCbTest(AppHandle *appHandle) {
    NotifyCb *cb;

    appHandleRegXmlChangedCb(appHandle, testCb);

    LL_FOREACH(appHandle->noficyCbList, cb) {
        (*cb->cb)(appHandle, "OK");
    }

    appHandleUnregXmlChangedCb(appHandle, testCb);

    LL_FOREACH(appHandle->noficyCbList, cb) {
        (*cb->cb)(appHandle, "FAIL");
    }
}

EXPORT void inotifyTest(const char *path) {

#ifndef WIN32
#define INOTIFY_BUF_SIZE 512
    /* inotify test */
    int wd;
    ssize_t len;
    char buf[INOTIFY_BUF_SIZE];
    char *ptr;
    const struct inotify_event *event;

    int fd = inotify_init();
    if (fd < 0) {
        printf("inotify_init failed !!!");
        exit(1);
    }

    printf("inotify path = %s\n", path);
    wd = inotify_add_watch(fd, path, IN_CLOSE_WRITE);
    if (wd < 0) {
        printf("inotify_add_watch failed !!!");
        exit(1);
    }

    while (1) {
        len = read(fd, buf, sizeof(buf));
        if (len < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* Loop over all events in the buffer */
        for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event *) ptr;

            /* Print event type */
            if (event->mask & IN_OPEN) {
                printf("IN_OPEN: ");
            }
            if (event->mask & IN_CLOSE_NOWRITE) {
                printf("IN_CLOSE_NOWRITE: ");
            }
            if (event->mask & IN_CLOSE_WRITE) {
                printf("IN_CLOSE_WRITE: ");
            }
            if (event->mask & IN_ACCESS) {
                printf("IN_ACCESS: ");
            }

            /* Print the name of the file */

            if (event->len) {
                printf("%s", event->name);
            }

            /* Print type of filesystem object */

            if (event->mask & IN_ISDIR) {
                printf(" [directory]\n");
            } else {
                printf(" [file]\n");
            }
        }
    }
    inotify_rm_watch(fd, IN_CLOSE_NOWRITE);
#endif
}

/***********************************
 * Test Steps:
 * 1. Create thread to read/write param
 * 2. Check the lock is work well
 * Crash/deadlock checking
 **********************************/
EXPORT APP_STATUS testAudioTypeLockFun(AppHandle *appHandle) {
    /* Read param */
    char *audioTypeName = "Speech";
    char *categoryPath = "Band,NB,Profile,4_pole_Headset,VolIndex,3";
    char *paramName = "speech_mode_para";
    unsigned short shortArray1[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
    unsigned short shortArray2[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    int arraySize = 7;
    AudioType *audioType;
    ParamUnit *paramUnit;
    ParamInfo *paramInfo;
    Param *param;

    appHandle = appHandleGetInstance();

    /* Query AudioType */
    audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return APP_ERROR;
    }

    /* Read Lock */
    audioTypeReadLock(audioType, __FUNCTION__);

    /* Query the ParamUnit */
    paramUnit = audioTypeGetParamUnit(audioType, categoryPath);
    if (!paramUnit) {
        ERR_LOG("paramUnit is NULL\n");
        return APP_ERROR;
    }

    /* Query the param value */
    param = paramUnitGetParamByName(paramUnit, paramName);
    if (!param) {
        ERR_LOG("Error: Cannot query param value!\n");
        return APP_ERROR;
    }

    /* Read unlock */
    audioTypeUnlock(audioType);

    utilUsleep(1);  // delay time make cpu scheduling to other thread

    /* Write param */
    paramInfo = audioTypeGetParamInfoByName(audioType, paramName);
    if (audioTypeSetParamData(audioType, categoryPath, paramInfo, (void *)shortArray1, arraySize) == APP_ERROR) {
        ERR_LOG("Cannot update the param data!!\n");
        return APP_ERROR;
    }

    utilUsleep(1);  // delay time make cpu scheduling to other thread

    if (audioTypeSetParamData(audioType, categoryPath, paramInfo, (void *)shortArray2, arraySize) == APP_ERROR) {
        ERR_LOG("Cannot update the param data!!\n");
        return APP_ERROR;
    }

    utilUsleep(1);  // delay time make cpu scheduling to other thread

    /* Save XML */
    audioTypeSaveAudioParamXml(audioType, XML_CUS_FOLDER_ON_DEVICE, 1);

    utilUsleep(1);  // delay time make cpu scheduling to other thread

    /* Reload XML */
    if (appHandleReloadAudioType(appHandle, audioType->name) == APP_ERROR) {
        ERR_LOG("Cannot reload AudioType!\n (%s)", audioType->name);
        return APP_ERROR;
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS testAudioTypeLock(AppHandle *appHandle) {
#ifndef WIN32
    int i;
    pthread_t appThread;
    void *status;
    ThreadParam threadParam;

    threadParam.times = 50;
    threadParam.appHandle = appHandle;
    threadParam.fun = testAudioTypeLockFun;

    if (pthread_create(&appThread, NULL, commonThreadLoop, &threadParam)) {
        ERR_LOG("Create app thread fail!\n");
        return APP_ERROR;
    }

    for (i = 0; i < threadParam.times; i++) {
        (*threadParam.fun)(appHandle);
        INFO_LOG("Main thread test round = %d\n", i);
    }

    /* Waiting 2nd thread join */
    pthread_join(appThread, &status);
#else
    INFO_LOG("Not test this UT on windows\n");
#endif
    return APP_NO_ERROR;
}

EXPORT APP_STATUS testAppHandleInitUninit() {
    int times = 10;
    int i;
    for (i = 0; i < times; i++) {
        AppHandle testAppHandle;
        appHandleInit(&testAppHandle);
#ifdef WIN32
        appHandleParseXml(&testAppHandle, XML_FOLDER_LIST_ON_TUNING_TOOL, XML_CUS_FOLDER_ON_TUNING_TOOL);
#else
        char **xmlDirFromProperty = appGetXmlDirFromProperty();
        if (xmlDirFromProperty) {
            appHandleParseXml(&testAppHandle, (const char**)xmlDirFromProperty, XML_CUS_FOLDER_ON_DEVICE);
        } else {
            appHandleParseXml(&testAppHandle, XML_FOLDER_LIST_ON_DEVICE, XML_CUS_FOLDER_ON_DEVICE);
        }
#endif
        appHandleUninit(&testAppHandle);
    }
    return APP_NO_ERROR;
}

/***********************************
 * Test Steps:
 * 1. Reload audio type xml 100 times
 * Memory leak / crash checking
 **********************************/
EXPORT APP_STATUS testMemoryLeak(AppHandle *appHandle) {
    int i = 0;
    for (i = 0; i < 100; i++) {
        /* stress test query / release / create */
        AudioType *audioType = appHandleGetAudioTypeByName(appHandle, "Speech");
        audioType->allowReload = 1;
        if (appHandleReloadAudioType(appHandle, "Speech") == APP_ERROR) {
            return APP_ERROR;
        }
    }

    printf("Checking memory status and press enter key to continue\n");
    getchar();
    return APP_NO_ERROR;
}

/***********************************
 * Test Steps:
 * 1. Read param array
 * 2. Update param array one item with 32767
 * 3. Repeat array size times
 * 4. Check the result
 **********************************/
APP_STATUS testReadWriteParam(AppHandle *appHandle) {
    size_t i, j;
    ParamUnit *paramUnit;
    ParamInfo *paramInfo;
    Param *param;
    unsigned short *shortArray;
    size_t arraySize = 1;  // The size will update by real array size latter

    const char *audioTypeName = "Speech";
    const char *paraName = "speech_mode_para";
    const char *categoryPath = "Band,NB,Profile,Normal,VolIndex,2,Network,GSM";

    AudioType *audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);

    /* Test steps */
    for (j = 0; j < arraySize; j++) {
        paramUnit = audioTypeGetParamUnit(audioType, categoryPath);
        if (!paramUnit) {
            ERR_LOG("Cannot find paramUnit\n");
            return APP_ERROR;
        }

        param = paramUnitGetParamByName(paramUnit, paraName);
        if (!param) {
            ERR_LOG("Cannot query param value!\n");
            return APP_ERROR;
        }

        shortArray = (unsigned short *)param->data;
        arraySize = param->arraySize;
        /*for(i = 0; i < param->arraySize; i++)
        {
            printf("[%d]0x%x\n", i, ((unsigned short*)param->data)[i])
        }*/

        shortArray[j] = 32767;

        /* You should cache follow object in somewhere without query again */
        paramInfo = audioTypeGetParamInfoByName(audioType, paraName);

        /* The sph_in_fir param is short array type */
        if (audioTypeSetParamData(audioType, categoryPath, paramInfo, (void *)shortArray, param->arraySize) == APP_ERROR) {
            return APP_ERROR;
        }
    }

    /* Result check */
    paramUnit = audioTypeGetParamUnit(audioType, categoryPath);
    if (!paramUnit) {
        ERR_LOG("Cannot find paramUnit\n");
        return APP_ERROR;
    }

    param = paramUnitGetParamByName(paramUnit, paraName);
    if (!param) {
        ERR_LOG("Cannot query param value!\n");
        return APP_ERROR;
    }

    shortArray = (unsigned short *)param->data;
    for (i = 0; i < param->arraySize; i++) {
        if (shortArray[i] != 32767) {
            ERR_LOG("Verify short array["APP_SIZE_T_FT"] = %d != 32767\n", i, shortArray[i]);
            return APP_ERROR;
        }
    }

    return APP_NO_ERROR;
}
