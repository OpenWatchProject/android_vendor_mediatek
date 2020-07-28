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
 *   Implement AppHandle related APIs
 */

#include "AudioParamParserPriv.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __linux__
#include <dirent.h>
#include <unistd.h>
#else
#include <windows.h>
#include <io.h>
#pragma warning( disable : 4996 )
#endif

static AppHandle appHandleInst;
static int appHandleInited = 0;

#ifdef FORCE_DEBUG_LEVEL
int appDebugLevel = DEBUG_LEVEL;        /* Global debug level setting */
#else
int appDebugLevel = WARN_LEVEL;         /* Global debug level setting */
#endif

const char **appAudioTypeLoadingList = NULL;

FILE *appLogFp = NULL;
int outputLogToStdout = 0;

#ifndef WIN32
static pthread_rwlock_t appHandleInstLock = PTHREAD_RWLOCK_INITIALIZER;
static const char *appHandleInstLockCallerFun = NULL;  /* Used to cache the lock holder */
#else
int __stdcall DllMain(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved) {
    return TRUE;
}
#endif

int getDebugLevel() {
#if !defined(WIN32) && !defined(SYS_IMPL)
    int debugLevel;;
    char appDebugLevelStr[MAX_PROP_VALUE_LEN];

    property_get(PROPERTY_KEY_APP_LOG_LEVEL, appDebugLevelStr, "-1");
    debugLevel = atoi(appDebugLevelStr);
    MUST_LOG("debug level = %d", debugLevel);

    if (debugLevel != -1) {
        return debugLevel;
    }
#endif

    return appDebugLevel;
}

EXPORT APP_STATUS appHandleInit(AppHandle *appHandle) {
#ifdef _DEBUG
    /* Alwasy show the output console for debug build */
    appHandleRedirectIOToConsole();
#endif

    INFO_LOG("appHandle = 0x%p\n", appHandle);

    if (appHandle) {
        appHandle->xmlDir = NULL;
        appHandle->xmlCusDir = NULL;
        appHandle->audioTypeHash = NULL;
        appHandle->featureOptionsHash = NULL;
        appHandle->featureOptionsDoc = NULL;
        appHandle->noficyCbList = NULL;
        appHandle->xmlCusDirReady = 0;
        appHandle->saveXmlWithHexMode = 1;
#ifndef WIN32
        appHandle->lockCallerFun = NULL;
        appHandle->appThreadExit = 0;
        appHandle->inotifyFd = -1;
        appHandle->xmlChangedNotifyEnabled = isCustXmlEnable();

        pthread_rwlock_init(&appHandle->lock, NULL);
        pthread_rwlock_init(&appHandle->notifyLock, NULL);
#else
        appHandle->xmlChangedNotifyEnabled = 1;
#endif
        appDebugLevel = getDebugLevel();
        appHandleShowAudioTypeSupportedVerInfo(appHandle);
        utilDumpAudioTypeLoadingList(appAudioTypeLoadingList);
        return APP_NO_ERROR;
    } else {
        WARN_LOG("AppHandle is NULL!\n");
        return APP_ERROR;
    }
}

EXPORT APP_STATUS appHandleUninit(AppHandle *appHandle) {
    INFO_LOG("appHandle = 0x%p\n", appHandle);

    if (!appHandle) {
        WARN_LOG("AppHandle is NULL!\n");
        return APP_ERROR;
    } else {
        NotifyCb *notifyCb, *tmp;

#if defined(SYS_IMPL)
        /* sys: Unload hw module & unregister hidl callback function */
        unregisterAudioParameterChangedCallback(appHandle);
#endif

        /* Lock */
        appHandleWriteLock(appHandle, __FUNCTION__);

        if (appHandle->xmlDir != XML_FOLDER_LIST_ON_DEVICE && appHandle->xmlDir != XML_FOLDER_LIST_ON_TUNING_TOOL) {
            /* Free the xmlDir which get from property */
            free((void *)appHandle->xmlDir[0]);
            free((void *)appHandle->xmlDir);
        }
        appHandle->xmlDir = NULL;

        if (appHandle->xmlCusDir) {
            free(appHandle->xmlCusDir);
        }
        appHandle->xmlCusDir = NULL;

        if (appHandle->featureOptionsDoc) {
            xmlFreeDoc(appHandle->featureOptionsDoc);
        }
        appHandle->featureOptionsDoc = NULL;

#ifndef WIN32
        if (appHandle->appThreadExit == 0) {
            void *status;
            appHandle->appThreadExit = 1;
            INFO_LOG("Send signal to appThread\n");
            pthread_kill(appHandle->appThread, SIGUSR1);

            /* TODO: Don't join the inotify thread, since the read function is block waiting */
            INFO_LOG("Waiting inotify thread join...\n");
            pthread_join(appHandle->appThread, &status);
            INFO_LOG("inotify thread joined\n");
            appHandle->inotifyFd = 0;
        }
#endif

        // release notify callback list
        LL_FOREACH_SAFE(appHandle->noficyCbList, notifyCb, tmp) {
            LL_DELETE(appHandle->noficyCbList, notifyCb);
            free(notifyCb);
        }
        appHandle->noficyCbList = NULL;

        /* If appHandle is singleton instance, reset the init info */
        if (appHandle == &appHandleInst) {
            appHandleInited = 0;
        }

        appHandleReleaseAudioTypeHash(appHandle);

        appHandleReleaseFeatureOptionsHash(appHandle);

        xmlCleanupParser();

        /* Flush app log */
        if (appLogFp) {
            fflush(appLogFp);
        }

        appAudioTypeLoadingList = NULL;

        /* Unlock */
        appHandleUnlock(appHandle);

        return APP_NO_ERROR;
    }
}

EXPORT const char *appHandleGetBuildTimeStamp() {
#ifdef WIN32
    return __DATE__ " " __TIME__;
#else
    return "";
#endif
}

EXPORT int appHandleWriteLock(AppHandle *appHandle, const char *callerFun) {
    int res = 0;

    if (!appHandle) {
        WARN_LOG("appHandle is NULL\n");
        return res;
    }

#ifndef WIN32
    while (1) {
        if (pthread_rwlock_trywrlock(&appHandle->lock) == 0) {
            appHandle->lockCallerFun = callerFun;
            DEBUG_LOG("AppHandle lock is locked by %s()\n", appHandle->lockCallerFun);
            break;
        } else {
            DEBUG_LOG("Cannot lock the AppHandle lock, delay some time. (the locker is %s())\n", appHandle->lockCallerFun);
            utilUsleep(1);
        }
    }
#else
    //DEBUG_LOG("Not support this function yet!\n");
#endif
    return res;
}

EXPORT int appHandleReadLock(AppHandle *appHandle, const char *callerFun) {
    int res = 0;

    if (!appHandle) {
        WARN_LOG("appHandle is NULL\n");
        return res;
    }

#ifndef WIN32
    while (1) {
        if (pthread_rwlock_tryrdlock(&appHandle->lock) == 0) {
            appHandle->lockCallerFun = callerFun;
            DEBUG_LOG("AppHandle lock is locked by %s()\n", appHandle->lockCallerFun);
            break;
        } else {
            DEBUG_LOG("Cannot lock the AppHandle lock, delay some time. (the locker is %s())\n", appHandle->lockCallerFun);
            utilUsleep(1);
        }
    }
#else
    //DEBUG_LOG("Not support this function yet!\n");
#endif
    return res;
}

EXPORT int appHandleUnlock(AppHandle *appHandle) {
    int res = 0;

    if (!appHandle) {
        WARN_LOG("appHandle is NULL\n");
        return res;
    }

#ifndef WIN32
    DEBUG_LOG("Unlock appHandle lock\n");
    res = pthread_rwlock_unlock(&appHandle->lock);
#endif
    return res;
}

EXPORT int appHandleInstWriteLock(const char *callerFun) {
    int res = 0;

#ifndef WIN32
    while (1) {
        if (pthread_rwlock_trywrlock(&appHandleInstLock) == 0) {
            appHandleInstLockCallerFun = callerFun;
            DEBUG_LOG("%s acquired the appHandleInstLock\n", callerFun);
            break;
        } else {
            DEBUG_LOG("Cannot lock the appHandleInstLock, delay some time. (the locker is %s)\n", callerFun);
            utilUsleep(1);
        }
    }
#else
    //DEBUG_LOG("Not support this function yet!\n");
#endif
    return res;
}

EXPORT int appHandleInstUnlock() {
    int res = 0;
#ifndef WIN32
    DEBUG_LOG("Unlock appHandleInst lock\n");
    res = pthread_rwlock_unlock(&appHandleInstLock);
#endif
    return res;
}

EXPORT FeatureOption *featureOptionCreate(const char *name, const char *value) {
    FeatureOption *featureOption = malloc(sizeof(FeatureOption));
    if (!featureOption) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    featureOption->name = strdup(name);
    featureOption->value = strdup(value);
    return featureOption;
}

EXPORT void featureOptionRelease(FeatureOption *featureOption) {
    free(featureOption->name);
    free(featureOption->value);
    free(featureOption);
}

EXPORT void appHandleReleaseFeatureOptionsHash(AppHandle *appHandle) {
    if (appHandle->featureOptionsHash) {
        FeatureOption *tmp, *item;
        HASH_ITER(hh, appHandle->featureOptionsHash, item, tmp) {
            if (appHandle->featureOptionsHash) {
                HASH_DEL(appHandle->featureOptionsHash, item);
                featureOptionRelease(item);
            }
        }
    }
    appHandle->featureOptionsHash = NULL;
}

EXPORT AppHandle *appHandleGetInstance() {
    appHandleInstWriteLock(__FUNCTION__);

    if (!appHandleInited) {
        appHandleInit(&appHandleInst);
#ifdef WIN32
        appHandleParseXml(&appHandleInst, XML_FOLDER_LIST_ON_TUNING_TOOL, XML_CUS_FOLDER_ON_TUNING_TOOL);
#else
        char **xmlDirFromProperty = appGetXmlDirFromProperty();

        if (xmlDirFromProperty) {
            appHandleParseXml(&appHandleInst, (const char**)xmlDirFromProperty, XML_CUS_FOLDER_ON_DEVICE);
        } else {
            appHandleParseXml(&appHandleInst, XML_FOLDER_LIST_ON_DEVICE, XML_CUS_FOLDER_ON_DEVICE);
        }
#endif
        appHandleInited = 1;
    }

    appHandleInstUnlock();

    return &appHandleInst;
}

EXPORT APP_STATUS appHandleParseXml(AppHandle *appHandle, const char *dir[], const char *cusDir) {
    int i;
    INFO_LOG("appHandle = 0x%p, dir = %s, cusDir = %s\n", appHandle, dir[0], cusDir);

    if (!appHandle) {
        ERR_LOG("appHandle is NULL!\n");
        return APP_ERROR;
    }

    if (!dir) {
        ERR_LOG("dir is NULL\n");
        return APP_ERROR;
    }

    if (appHandle->xmlDir || appHandle->xmlCusDir) {
        ERR_LOG("XML already parsed, don't call the appHandleParseXml twice!\n");
        return APP_ERROR;
    }

    appHandleWriteLock(appHandle, __FUNCTION__);

    appHandle->xmlDir = dir;
    appHandle->xmlCusDir = strdup(cusDir);
    for (i = 0; appHandle->xmlDir[i]; i++) {
        INFO_LOG("XmlDir[%d] = %s\n", i, appHandle->xmlDir[i]);
    }
    INFO_LOG("XmlCusDir = %s\n", appHandle->xmlCusDir);

#if !defined(SYS_IMPL)
    /* Load feature options information */
    appHandleLoadDirFeatureOptionsInfo(appHandle);

    /* Load audio type information */
    appHandleLoadDirAudioTypeInfo(appHandle);
#endif

    appHandleUnlock(appHandle);

#ifndef WIN32
#if defined(SYS_IMPL)
    /* sys: For system AudioParamParser, it's need to register AudioParamParserChanged HIDL callback with AudioHAL */
    registerAudioParameterChangedCallback(appHandle);
#else
    /* For vendor AudioParamParser, it's need to monitor file change */
    /* Setup file system monitor thread */
    if (pthread_create(&appHandle->appThread, NULL, appHandleThreadLoop, (void *)appHandle)) {
        ERR_LOG("Create app thread fail!\n");
        return APP_ERROR;
    } else {
        INFO_LOG("Create app thread successfully\n");
    }
#endif
#endif

    return APP_NO_ERROR;
}

EXPORT void appHandleCustXmlEnableChanged(AppHandle *appHandle, int enable) {
#ifndef WIN32
#if defined(SYS_IMPL)
    /* SYS: Notify vendor parser to change the cust XML monitor status */
    if (enable) {
        audioSystemSetParameters("SET_CUST_XML_ENABLE=1");
    } else {
        audioSystemSetParameters("SET_CUST_XML_ENABLE=0");
    }
#else
    /* VND: Reload XML or disable the XML changed notification */
    if (!appHandle) {
        ERR_LOG("AppHandle is NULL\n");
        return;
    }

    if (enable == 1) {
        pthread_t reloadCustXmlThread;

        appHandle->xmlChangedNotifyEnabled = 1;
        INFO_LOG("xmlChangedNotifyEnabled = %d\n", appHandle->xmlChangedNotifyEnabled);

        /* Create a thread to reload all cust XML */
        if (pthread_create(&reloadCustXmlThread, NULL, reloadCustXmlThreadLoop, (void *)appHandle)) {
            ERR_LOG("Create reload cust xml thread fail!\n");
        }
    } else {
        appHandle->xmlChangedNotifyEnabled = 0;
        INFO_LOG("xmlChangedNotifyEnabled = %d\n", appHandle->xmlChangedNotifyEnabled);
    }
#endif
#endif
}

EXPORT APP_STATUS appHandleLoadDirFeatureOptionsInfo(AppHandle *appHandle) {
    struct stat fileStat;
    int strLen;
    char *featureOptionsFile = NULL;
    xmlNode *node = NULL;
    xmlNode *root = NULL;
    xmlChar *name = NULL;
    xmlChar *value = NULL;
    int i = 0;

    if (!appHandle) {
        ERR_LOG("appHandle is NULL!\n");
        return APP_ERROR;
    }

    if (!appHandle->xmlDir) {
        ERR_LOG("xmlDir is NULL!\n");
        return APP_ERROR;
    }

    if (appHandle->featureOptionsHash) {
        WARN_LOG("Feature options already loaded, don't reload it!\n");
        return APP_NO_ERROR;
    }

    /* Get feature option file path */
    for (i = 0; appHandle->xmlDir[i]; i++) {
        strLen = strlen(appHandle->xmlDir[i]) + strlen(FEATURE_OPTIONS_XML) + 2;
        featureOptionsFile = (char *)malloc(strLen);
        snprintf(featureOptionsFile, strLen, "%s%s%s", appHandle->xmlDir[i], FOLDER, FEATURE_OPTIONS_XML);

        if (stat(featureOptionsFile, &fileStat) == -1) {
            INFO_LOG("No %s file\n", featureOptionsFile);
            free(featureOptionsFile);
            featureOptionsFile = NULL;
        } else {
            break;
        }
    }

    if (featureOptionsFile == NULL) {
        ERR_LOG("Feature option file not found!");
        return APP_ERROR;
    }

    appHandle->featureOptionsDoc = xmlParseFile(featureOptionsFile);
    if (appHandle->featureOptionsDoc == NULL) {
        ERR_LOG("Failed to parse %s\n", featureOptionsFile);
        free(featureOptionsFile);
        return APP_ERROR;
    } else {
        INFO_LOG("Load xml file successfully. (%s)\n", featureOptionsFile);
    }
    free(featureOptionsFile);

    /* Parse informatino to feature options hash */
    root = xmlDocGetRootElement(appHandle->featureOptionsDoc);
    if (!root) {
        ERR_LOG("Root element is NULL\n");
        return APP_ERROR;
    }

    node = findXmlNodeByElemName(root, ELEM_AUDIO_FEATURE_OPTIONS);
    if (node && node->children) {
        node = node->children;
    } else {
        ERR_LOG("No feature options found!\n");
        return APP_ERROR;
    }

    while ((node = findXmlNodeByElemName(node->next, ELEM_PARAM))) {
        FeatureOption *featureOption;
        name = xmlGetProp(node, (const xmlChar *)ATTRI_NAME);
        value = xmlGetProp(node, (const xmlChar *)ATTRI_VALUE);

        featureOption = featureOptionCreate((const char *)name, (const char *)value);
        HASH_ADD_KEYPTR(hh, appHandle->featureOptionsHash, featureOption->name, strlen(featureOption->name), featureOption);

        if (name) {
            xmlFree(name);
        }

        if (value) {
            xmlFree(value);
        }
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS appHandleLoadDirAudioTypeInfo(AppHandle *appHandle) {
    char audioType[MAX_AUDIO_TYPE_LEN + 1];

#ifdef __linux__
    struct dirent **namelist;
    int n;
    int dirIndex;

    if (!appHandle) {
        ERR_LOG("appHandle is NULL!\n");
        return APP_ERROR;
    }

    /* Release old audio type first */
    appHandleReleaseAudioTypeHash(appHandle);
    for (dirIndex = 0; appHandle->xmlDir[dirIndex];  dirIndex++) {
        n = scandir(appHandle->xmlDir[dirIndex], &namelist, 0, alphasort);
        if (n < 0) {
            INFO_LOG("Scandir error (%s)\n", appHandle->xmlDir[dirIndex]);
        } else {
            while (n--) {
                if (strstr(namelist[n]->d_name, AUDIO_PARAM_XML_POSFIX) == NULL) {
                    DEBUG_LOG("File name's posfix is not AudioParam.xml (%s)\n", namelist[n]->d_name);
                    free(namelist[n]);
                    continue;
                }
                sscanf(namelist[n]->d_name, AUDIO_TYPE_FMT_STR(MAX_AUDIO_TYPE_LEN), audioType);
                if (appHandleIsValidAudioType(appHandle, audioType)) {
                    appHandleAddAudioType(appHandle, audioType);
                } else {
                    MUST_LOG("Don't load audio param xml = %s\n", namelist[n]->d_name);
                }
                free(namelist[n]);
            }
            free(namelist);
        }
    }

    INFO_LOG("xmlCusDirReady = %d\n", appHandle->xmlCusDirReady);
#else
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    UT_string *path = NULL;
    int i;

    if (!appHandle) {
        ERR_LOG("appHandle is NULL!\n");
        return APP_ERROR;
    }

    /* Release old audio type first */
    appHandleReleaseAudioTypeHash(appHandle);

    /* Check preload xml folder */
    for (i = 0; appHandle->xmlDir[i]; i++) {
        utstring_new(path);
        utstring_printf(path, "%s"FOLDER"*"AUDIO_PARAM_XML_POSFIX, appHandle->xmlDir[i]);
        hFind = FindFirstFile(utstring_body(path), &FindFileData);
        utstring_free(path);

        if (hFind == INVALID_HANDLE_VALUE) {
            WARN_LOG("No xml found! (%s)\n", appHandle->xmlDir[i]);
            continue;
        }

        do {
            sscanf(FindFileData.cFileName, AUDIO_TYPE_FMT_STR(MAX_AUDIO_TYPE_LEN), audioType);

            if (appHandleIsValidAudioType(appHandle, audioType)) {
                appHandleAddAudioType(appHandle, audioType);
            } else {
                INFO_LOG("Invalid audio param xml = %s\n", FindFileData.cFileName);
            }
        } while (FindNextFile(hFind, &FindFileData));
    }

    /* Assume xml cust dir always ready */
    appHandle->xmlCusDirReady = 1;
#endif

    /* Load all XMLs */
    appHandleLoadAllAudioTypeXml(appHandle);
    INFO_LOG("Load all audio type XML - ok\n");

    /* Remove audio type if it's feature options disabled */
    appHandleRemoveAudioTypeByFeatureOptions(appHandle);

    /* Modify data depends on feature options */
    appHandleReviseXmlDocByFeatureOptions(appHandle);

    /* Load hash info from XML */
    appHandleLoadAllAudioTypeHash(appHandle);
    INFO_LOG("Load all audio Hash - ok\n");

    if (appDebugLevel == DEBUG_LEVEL) {
        appHandleDumpAudioTypeList(appHandle);
    }

    /* Unload all audio type xml */
    appHandleReleaseAllAudioTypeXml(appHandle);

    return APP_NO_ERROR;
}

EXPORT size_t appHandleGetNumOfAudioType(AppHandle *appHandle) {
    if (!appHandle) {
        ERR_LOG("appHandle is NULL!\n");
        return APP_ERROR;
    }

    return HASH_COUNT(appHandle->audioTypeHash);
}

EXPORT APP_STATUS appHandleLoadAllAudioTypeXml(AppHandle *appHandle) {
    size_t i;
    size_t count = appHandleGetNumOfAudioType(appHandle);

    for (i = 0; i < count; i++) {
        AudioType *audioType = appHandleGetAudioTypeByIndex(appHandle, i);

        /* Load xml struct */
        if (appHandleLoadAudioTypeXml(appHandle, audioType) == APP_ERROR) {
            WARN_LOG("Load audio type XML failed. (%s)\n", audioType->name);
        }
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS appHandleLoadAudioTypeXml(AppHandle *appHandle, AudioType *audioType) {
    char *audioTypeFile;

    INFO_LOG("audioType = %s\n", audioType->name);

    // Load AudioParamXml
    audioTypeFile = appHandleGetAudioTypeFilePath(appHandle, audioType->name, AUDIO_PARAM_XML_POSFIX);
    if (audioTypeFile == NULL) {
        WARN_LOG("The AudioTypeFile(%s%s) doesn't exist.\n", audioType->name, AUDIO_PARAM_XML_POSFIX);
        return APP_ERROR;
    }

    audioType->audioParamDoc = xmlParseFile(audioTypeFile);

    if (audioType->audioParamDoc == NULL) {
        ERR_LOG("Failed to parse %s\n", audioTypeFile);
        free(audioTypeFile);

        // Audio param file broken, load preload xml file instead
        audioTypeFile = appHandleGetPreloadAudioTypeFilePath(appHandle, audioType->name, AUDIO_PARAM_XML_POSFIX);
        if (audioTypeFile == NULL) {
            WARN_LOG("The AudioTypeFile(%s%s) doesn't exist.\n", audioType->name, AUDIO_PARAM_XML_POSFIX);
            return APP_ERROR;
        }

        WARN_LOG("Trying to load preload %s file instead of broken XML file!\n", audioTypeFile);
        audioType->audioParamDoc = xmlParseFile(audioTypeFile);
        if (audioType->audioParamDoc == NULL) {
            ERR_LOG("Failed to parse %s\n", audioTypeFile);
            free(audioTypeFile);
            return APP_ERROR;
        } else {
            INFO_LOG("Load xml file successfully. (%s)\n", audioTypeFile);
        }
    } else {
        INFO_LOG("Load xml file successfully. (%s)\n", audioTypeFile);
    }

    free(audioTypeFile);

    // Load ParamUnitDescXml
    audioTypeFile = appHandleGetAudioTypeFilePath(appHandle, audioType->name, PARAM_UNIT_DESC_XML_POSFIX);
    if (audioTypeFile == NULL) {
        WARN_LOG("The AudioTypeFile(%s%s) doesn't exist.\n", audioType->name, PARAM_UNIT_DESC_XML_POSFIX);
        return APP_ERROR;
    }

    audioType->paramUnitDescDoc = xmlParseFile(audioTypeFile);
    if (audioType->paramUnitDescDoc == NULL) {
        ERR_LOG("Failed to parse %s%s\n", audioTypeFile, PARAM_UNIT_DESC_XML_POSFIX);
        free(audioTypeFile);
        return APP_ERROR;
    } else {
        INFO_LOG("Load xml file successfully. (%s)\n", audioTypeFile);
    }
    free(audioTypeFile);

#ifdef WIN32
    // Load ParamTreeViewXml only for tuning tool
    audioTypeFile = appHandleGetAudioTypeFilePath(appHandle, audioType->name, PARAM_TREE_VIEW_XML_POSFIX);
    if (audioTypeFile == NULL) {
        INFO_LOG("The AudioTypeFile(%s%s) doesn't exist.\n", audioType->name, PARAM_TREE_VIEW_XML_POSFIX);
        free(audioTypeFile);
    } else {
        audioType->paramTreeViewDoc = xmlParseFile(audioTypeFile);
        if (audioType->paramTreeViewDoc == NULL) {
            DEBUG_LOG("Failed to parse %s%s\n", audioTypeFile, PARAM_TREE_VIEW_XML_POSFIX);
        } else {
            INFO_LOG("Load xml file successfully. (%s)\n", audioTypeFile);
        }
        free(audioTypeFile);
    }
#endif

    /* Get tab name info */
    audioTypeParseTabName(audioType);

    /* Get version info */
    if (audioTypeParseXmlVer(audioType) == APP_ERROR) {
        ERR_LOG("Cannot parse xml version info. (%s)\n", audioType->name);
        return APP_ERROR;
    }

#ifndef WIN32
    /* XML Version check for device driver or HAL */
    if (!audioTypeIsDeviceSupportedXmlVer(audioType)) {
        abort();
    }
#endif

    return APP_NO_ERROR;
}

EXPORT char *appHandleGetAudioTypeFilePath(AppHandle *appHandle, const char *audioType, const char *posfix) {
    /* Check cus folder xml first */
    struct stat fileStat;
    int strLen;
    char *path;
    int i;

    if (appHandle->xmlChangedNotifyEnabled && appHandle->xmlCusDir && !strncmp(posfix, AUDIO_PARAM_XML_POSFIX, strlen(AUDIO_PARAM_XML_POSFIX) + 1)) {
        strLen = strlen(appHandle->xmlCusDir) + strlen(audioType) + strlen(posfix) + 2;
        path = (char *)malloc(strLen);
        if (!path) {
            ERR_LOG("malloc fail!\n");
            return NULL;
        }

        snprintf(path, strLen, "%s%s%s%s", appHandle->xmlCusDir, FOLDER, audioType, posfix);

        if (stat(path, &fileStat) != -1) {
            return path;
        } else {
            free(path);
        }
    }

    /* Check default folder */
    for (i = 0; appHandle->xmlDir[i]; i++) {
        strLen = strlen(appHandle->xmlDir[i]) + strlen(audioType) + strlen(posfix) + 2;
        path = (char *)malloc(strLen);
        if (!path) {
            ERR_LOG("malloc fail!\n");
            continue;
        }

        snprintf(path, strLen, "%s%s%s%s", appHandle->xmlDir[i], FOLDER, audioType, posfix);

        if (stat(path, &fileStat) != -1) {
            return path;
        }

        free(path);
    }

    return NULL;
}

EXPORT char *appHandleGetPreloadAudioTypeFilePath(AppHandle *appHandle, const char *audioType, const char *posfix) {
    /* Check cus folder xml first */
    struct stat fileStat;
    int strLen;
    char *path;
    int i;

    /* Check default folder */
    for (i = 0; appHandle->xmlDir[i]; i++) {
        strLen = strlen(appHandle->xmlDir[i]) + strlen(audioType) + strlen(posfix) + 2;
        path = (char *)malloc(strLen);
        if (!path) {
            ERR_LOG("malloc fail!\n");
            continue;
        }

        snprintf(path, strLen, "%s%s%s%s", appHandle->xmlDir[i], FOLDER, audioType, posfix);

        if (stat(path, &fileStat) != -1) {
            return path;
        }

        free(path);
    }
    return NULL;
}

EXPORT int appHandleIsValidAudioType(AppHandle *appHandle, const char *audioType) {
    char *filePath;

    assert(appHandle != NULL);

#ifndef WIN32
    /* UI parameter is valid for win32 */
    if (utilIsUIAudioType(audioType)) {
        return 0;
    }
#endif

    if (utilIsAudioTypeInLoadingList(audioType) == 0) {
        return 0;
    }

    filePath = appHandleGetAudioTypeFilePath(appHandle, audioType, PARAM_UNIT_DESC_XML_POSFIX);
    if (filePath == NULL) {
        ERR_LOG("%s audio type is not valid! (%s is not exist)\n", audioType, filePath);
        free(filePath);
        return 0;
    }

    free(filePath);
    return 1;
}

EXPORT AudioType *appHandleAddAudioType(AppHandle *appHandle, const char *audioTypeName) {
    AudioType *audioType = NULL;

    if (!appHandle) {
        ERR_LOG("The appHandle is NULL\n");
        return NULL;
    }

    if (!audioTypeName) {
        ERR_LOG("The audioTypeName is NULL\n");
        return NULL;
    }

    if (appHandleGetAudioTypeByName(appHandle, audioTypeName) == NULL) {
        audioType = audioTypeCreate(appHandle, audioTypeName);

        /* Add audio type to hash */
        HASH_ADD_KEYPTR(hh, appHandle->audioTypeHash, audioType->name, strlen(audioType->name), audioType);
    } else {
        INFO_LOG("%s(), %s Audio type alread added\n", __FUNCTION__, audioTypeName);
    }
    return audioType;
}

EXPORT AudioType *appHandleGetAudioTypeByIndex(AppHandle *appHandle, size_t index) {
    AudioType *audioType = NULL;
    size_t i = 0;

    DEBUG_LOG("appHandle = 0x%p, index = "APP_SIZE_T_FT"\n", appHandle, index);

    if (!appHandle) {
        ERR_LOG("appHandle is NULL!\n");
        return NULL;
    }

    for (audioType = appHandle->audioTypeHash; audioType ; audioType = audioType->hh.next) {
        if (index == i++) {
            return audioType;
        }
    }

    return NULL;
}

EXPORT AudioType *appHandleGetAudioTypeByName(AppHandle *appHandle, const char *name) {
    AudioType *audioType = NULL;

    INFO_LOG("appHandle = 0x%p, name = %s\n", appHandle, name);

    if (!appHandle) {
        ERR_LOG("appHandle is NULL!\n");
        return NULL;
    }

    HASH_FIND_STR(appHandle->audioTypeHash, name, audioType);

    return audioType;
}

EXPORT void appHandleReleaseAudioTypeHash(AppHandle *appHandle) {
    if (appHandle->audioTypeHash) {
        AudioType *tmp, *item;
        HASH_ITER(hh, appHandle->audioTypeHash, item, tmp) {
            if (appHandle->audioTypeHash) {
                HASH_DEL(appHandle->audioTypeHash, item);
                audioTypeRelease(item);
            }
        }
    }
    appHandle->audioTypeHash = NULL;
}

EXPORT void appHandleDumpAudioTypeList(AppHandle *appHandle) {
    size_t index = 0;
    size_t numOfAudioType = appHandleGetNumOfAudioType(appHandle);
    INFO_LOG("=================================\n");
    INFO_LOG("Totoal num of Audio Type List = "APP_SIZE_T_FT"\n", numOfAudioType);
    for (index = 0; index < numOfAudioType; index++) {
        AudioType *audioType = appHandleGetAudioTypeByIndex(appHandle, index);
        INFO_LOG("AudioType["APP_SIZE_T_FT"] = %s\n", index, audioType->name);
        audioTypeDump(audioType);
    }
}

EXPORT APP_STATUS appHandleLoadAllAudioTypeHash(AppHandle *appHandle) {
    size_t index = 0;
    size_t numOfAudioType = appHandleGetNumOfAudioType(appHandle);
    /* Load stage1 information */
    for (index = 0; index < numOfAudioType; index++) {
        AudioType *audioType = appHandleGetAudioTypeByIndex(appHandle, index);
        audioTypeLoadStage1Hash(audioType);
    }

    /* Load stage2 information (ex: ParamTreeView's switch object)*/
    for (index = 0; index < numOfAudioType; index++) {
        AudioType *audioType = appHandleGetAudioTypeByIndex(appHandle, index);
        audioTypeLoadStage2Hash(audioType);
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS appHandleReleaseAllAudioTypeXml(AppHandle *appHandle) {
    size_t index = 0;
    size_t numOfAudioType = appHandleGetNumOfAudioType(appHandle);

    for (index = 0; index < numOfAudioType; index++) {
        AudioType *audioType = appHandleGetAudioTypeByIndex(appHandle, index);
        if (audioType->paramUnitDescDoc) {
            xmlFreeDoc(audioType->paramUnitDescDoc);
            audioType->paramUnitDescDoc = NULL;
        }

        if (audioType->audioParamDoc) {
            xmlFreeDoc(audioType->audioParamDoc);
            audioType->audioParamDoc = NULL;
        }
    }

    if (appHandle->featureOptionsDoc) {
        xmlFreeDoc(appHandle->featureOptionsDoc);
        appHandle->featureOptionsDoc = NULL;
    }

    return APP_NO_ERROR;
}

EXPORT void appHandleRegXmlChangedCb(AppHandle *appHandle, NOTIFY_CB_FUN callbackFun) {
    INFO_LOG("appHandle = 0x%p, callbackFun = 0x%p\n", appHandle, callbackFun);

    appHandleWriteLock(appHandle, __FUNCTION__);

    if (appHandle && callbackFun) {
        /* Checking the duplicated callback function registration */
        NotifyCb *notifyCb;
        LL_FOREACH(appHandle->noficyCbList, notifyCb) {
            if (notifyCb->cb == callbackFun) {
                INFO_LOG("Same callback function found. ignore it\n");
                appHandleUnlock(appHandle);
                return;
            }
        }

        notifyCb = malloc(sizeof(NotifyCb));
        if (notifyCb) {
            notifyCb->cb = callbackFun;
            LL_APPEND(appHandle->noficyCbList, notifyCb);
        } else {
            ERR_LOG("malloc fail!\n");
        }
    } else {
        WARN_LOG("Cannot register xml callback! (AppHandle = 0x%p, callbackFun = 0x%p)\n", appHandle, callbackFun);
    }

    appHandleUnlock(appHandle);
}

EXPORT void appHandleUnregXmlChangedCb(AppHandle *appHandle, NOTIFY_CB_FUN callbackFun) {
    INFO_LOG("appHandle = 0x%p, callbackFun = 0x%p\n", appHandle, callbackFun);

    appHandleWriteLock(appHandle, __FUNCTION__);

    if (appHandle && callbackFun) {
        NotifyCb *notifyCb, *tmp;
        LL_FOREACH_SAFE(appHandle->noficyCbList, notifyCb, tmp) {
            if (notifyCb->cb == callbackFun) {
                LL_DELETE(appHandle->noficyCbList, notifyCb);
                free(notifyCb);
                INFO_LOG("Callback function unregistered. (0x%p, 0x%p)\n", callbackFun, callbackFun);
                break;
            }
        }
    } else {
        WARN_LOG("Cannot unregister xml callback! (AppHandle = 0x%p, callbackFun = %p)\n", appHandle, callbackFun);
    }

    appHandleUnlock(appHandle);
}

EXPORT void appHandleReloadCustXml(AppHandle *appHandle) {
#ifndef WIN32
    char audioTypeName[MAX_AUDIO_TYPE_LEN + 1];
    AudioType *audioType;
    struct dirent **namelist;
    int n;

    if (!appHandle) {
        ERR_LOG("AppHandle is NULL\n");
        return;
    }

    INFO_LOG("Scan the folder for applying cust audio parameter\n");

    n = scandir(appHandle->xmlCusDir, &namelist, 0, alphasort);
    if (n < 0) {
        ERR_LOG("Scandir error (%s)\n", appHandle->xmlCusDir);
    } else {
        while (n--) {
            /* File name checking */
            if (strstr(namelist[n]->d_name, AUDIO_PARAM_XML_POSFIX) == NULL) {
                DEBUG_LOG("File name's posfix is not AudioParam.xml (%s)\n", namelist[n]->d_name);
                free(namelist[n]);
                continue;
            }

            /* Find related audio type handle */
            sscanf(namelist[n]->d_name, AUDIO_TYPE_FMT_STR(MAX_AUDIO_TYPE_LEN), audioTypeName);
            audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
            if (!audioType) {
                ERR_LOG("Cannot find the audioType handle (%s)\n", audioTypeName);
                free(namelist[n]);
                continue;
            }
            audioType->allowReload = 1;
            MUST_LOG("Notify xml file changed. (%s)\n", audioType->name);

            appHandleNotifyAllCallbacks(appHandle, audioTypeName);

            /* Reload AudioType automatically */
            appHandleReloadAudioType(appHandle, audioTypeName);
            MUST_LOG("Reload audio type done. (%s)\n", audioType->name);

            free(namelist[n]);
        }
        free(namelist);
    }
#endif
}

#ifndef WIN32
EXPORT void *reloadCustXmlThreadLoop(void *arg) {
    AppHandle *appHandle = (AppHandle *)arg;
    if (!appHandle) {
        ERR_LOG("AppHandle is NULL\n");
        return NULL;
    }

    /* Reload cust XML */
    while (appHandle->inotifyFd < 0) {
        WARN_LOG("inotify not ready, waiting 1 sec...");
        utilUsleep(1000000);
    }
    appHandleReloadCustXml(appHandle);

    return NULL;
}

EXPORT void *appHandleThreadLoop(void *arg) {
    /* This thread only work on linux platform */
    /* Only eng load could monitor custom folder */
    AppHandle *appHandle = (AppHandle *)arg;
    ssize_t len;
    char buf[INOTIFY_BUF_SIZE];
    char *ptr;
    const struct inotify_event *event;
    int remonitor = 0;
    uint32_t iNotifyReMonitorMask = IN_DELETE_SELF | IN_UNMOUNT | IN_IGNORED | IN_Q_OVERFLOW;
    uint32_t iNotifyReloadMask = IN_CLOSE_WRITE;
    uint32_t iNotifyWatchEvent = IN_ALL_EVENTS | iNotifyReMonitorMask;

    if (!appHandle->xmlCusDir) {
        WARN_LOG("xmlCusDir is NULL, don't run the appHandleThreadLoop !!!");
        exit(1);
    }

    do {    /* Re-monitor loop */
        /* Create folder first to make inotify work */
        utilMkdir(appHandle->xmlCusDir);

        /* Register signal handler */
        struct sigaction sa;
        sa.sa_handler = NULL;
        sa.sa_sigaction = &signalHandler;
        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);

        if (sigaction(SIGUSR1, &sa, NULL) < 0) {
            ERR_LOG("sigaction fail");
            exit(1);
        }

        /* inotify registration */
        appHandle->inotifyFd = inotify_init();
        if (appHandle->inotifyFd < 0) {
            ERR_LOG("inotify_init failed !!!");
            exit(1);
        }

        INFO_LOG("Add inotify monitor path = %s, fd = %d, remonitor = %d\n", appHandle->xmlCusDir, appHandle->inotifyFd, remonitor);

        while (1) { /* Add watch loop */
            if (inotify_add_watch(appHandle->inotifyFd, appHandle->xmlCusDir, iNotifyWatchEvent) < 0) {
                INFO_LOG("inotify_add_watch failed !!! try again...");
                utilMkdir(appHandle->xmlCusDir);
                utilUsleep(1000000);
            } else {
                break;
            }
        }

        /* If the cust xml storage not ready before, reload XMLs here */
        if (appHandle->xmlCusDirReady == 0) {
            appHandleReloadCustXml(appHandle);
            appHandle->xmlCusDirReady = 1;
        }

        while (!appHandle->appThreadExit) { /* Read event loop */
            remonitor = 0;
            INFO_LOG("inotify read waiting... (fd = %d)\n", appHandle->inotifyFd);
            len = read(appHandle->inotifyFd, buf, sizeof(buf));

            if (len < 0) {
                if (appHandle->appThreadExit) {
                    break;
                }

                ERR_LOG("inotify read error!\n");
                pthread_exit(NULL);
            }

            /* Loop over all events in the buffer */
            for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
                event = (const struct inotify_event *) ptr;

                /* Checking event type that if we have to re-monitor */
                if (event->mask & iNotifyReMonitorMask) {
                    WARN_LOG("Got IN_DELETE_SELF|IN_UNMOUNT|IN_IGNORED event! set xmlCusDirReady with 0. (0x%x)", event->mask);
                    remonitor = 1;
                    appHandle->xmlCusDirReady = 0;
                    break;
                }

                /* Check if it's reload event */
                if (!(event->mask & iNotifyReloadMask)) {
                    INFO_LOG("Not reload event! (0x%x)", event->mask);
                    continue;
                }

                if (event->len) {
                    char audioTypeName[MAX_AUDIO_TYPE_LEN + 1];
                    AudioType *audioType;

                    if (strstr(event->name, AUDIO_PARAM_XML_POSFIX) == NULL) {
                        INFO_LOG("File name's posfix is not AudioParam.xml (%s)\n", event->name);
                        continue;
                    }

                    sscanf(event->name, AUDIO_TYPE_FMT_STR(MAX_AUDIO_TYPE_LEN), audioTypeName);
                    INFO_LOG("XML File chanegd (%s)\n", event->name);

                    audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
                    if (audioType) {
                        audioType->allowReload = 1;
                    }

                    appHandleNotifyAllCallbacks(appHandle, audioTypeName);

                    /* Reload AudioType automatically */
                    appHandleReloadAudioType(appHandle, audioTypeName);
                }
            }

            /* Once inode changed, try to remonitor it! */
            if (remonitor) {
                INFO_LOG("remonitor = %d, remount inotify change!", remonitor);
                break;
            }
        }

        inotify_rm_watch(appHandle->inotifyFd, IN_ALL_EVENTS);

        if (appHandle->inotifyFd) {
            INFO_LOG("close inotify handle %d, remonitor = %d\n", appHandle->inotifyFd, remonitor);
            close(appHandle->inotifyFd);
        }
    } while (remonitor);

    INFO_LOG("appHandleThreadLoop exit\n");
    return NULL;
}

EXPORT void appHandleNotifyAllCallbacks(AppHandle *appHandle, const char *audioTypeName) {
    if (!appHandle) {
        ERR_LOG("AppHandle is NULL\n");
        return;
    }

    if (appHandle->xmlChangedNotifyEnabled == 1) {
        /* notify users */
        NotifyCb *notifyCb;
        pthread_rwlock_wrlock(&appHandle->notifyLock);
        INFO_LOG("Notify all callback function.\n");
        LL_FOREACH(appHandle->noficyCbList, notifyCb) {
            INFO_LOG("Notify callback function. (0x%p, %pf)\n", notifyCb->cb, notifyCb->cb);
            (*notifyCb->cb)(appHandle, audioTypeName);
        }
        pthread_rwlock_unlock(&appHandle->notifyLock);
    } else {
        INFO_LOG("appHandle->xmlChangedNotifyEnabled = %d, don't notify callback!\n", appHandle->xmlChangedNotifyEnabled);
    }
}
#endif

EXPORT APP_STATUS appHandleReloadAudioType(AppHandle *appHandle, const char *audioTypeName) {
    /* Release old audioType */
    INFO_LOG("appHandle = 0x%p, audioTypeName = %s\n", appHandle, audioTypeName);

#if !defined(SYS_IMPL)
    AudioType *audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
    if (!audioType) {
        ERR_LOG("Invalid AudioType name = %s\n", audioTypeName);
        return APP_ERROR;
    }

    /* Write lock */
    audioTypeWriteLock(audioType, __FUNCTION__);

    /* Checking if the audioType reloaded */
    if (!audioType->allowReload) {
        INFO_LOG("AudioType is already reloaded!\n");
        audioTypeUnlock(audioType);
        return APP_NO_ERROR;
    }

    /* Load AudioParam XML */
    char *audioTypeFile = appHandleGetAudioTypeFilePath(appHandle, audioType->name, AUDIO_PARAM_XML_POSFIX);
    if (audioTypeFile == NULL) {
        WARN_LOG("The AudioTypeFile(%s%s) doesn't exist.\n", audioType->name, AUDIO_PARAM_XML_POSFIX);
        audioTypeUnlock(audioType);
        return APP_ERROR;
    }

    xmlDocPtr newAudioParamDoc = xmlParseFile(audioTypeFile);
    if (newAudioParamDoc == NULL) {
        ERR_LOG("Failed to parse %s, ignore the audio type reload\n", audioTypeFile);
        free(audioTypeFile);
        audioTypeUnlock(audioType);
        return APP_ERROR;
    }
    MUST_LOG("Load xml file successfully. (%s)\n", audioTypeFile);
    free(audioTypeFile);

    /* Release audio param XML & data */
    audioTypeReleaseAudioParam(audioType);
    if (audioType->audioParamDoc) {
        xmlFreeDoc(audioType->audioParamDoc);
        audioType->audioParamDoc = NULL;
    }

    /* Update audioParamDoc */
    audioType->audioParamDoc = newAudioParamDoc;

    /* Load AudioParam hash */
    if (audioTypeLoadParamUnitHash(audioType) == APP_ERROR) {
        audioTypeUnlock(audioType);
        return APP_ERROR;
    }

    if (audioTypeLoadParamTreeHash(audioType) == APP_ERROR) {
        audioTypeUnlock(audioType);
        return APP_ERROR;
    }

    if (audioType->audioParamDoc) {
        xmlFreeDoc(audioType->audioParamDoc);
        audioType->audioParamDoc = NULL;
    }

    /* AudioType reloaded */
    audioType->allowReload = 0;

    audioTypeUnlock(audioType);
#else
    INFO_LOG("No need to reload XML for system parser\n");
#endif
    return APP_NO_ERROR;
}

EXPORT const char *appHandleGetFeatureOptionValue(AppHandle *appHandle, const char *featureOptionName) {
    FeatureOption *featureOption = NULL;

    if (!appHandle) {
        ERR_LOG("appHandle is NULL\n");
        return NULL;
    }

    if (!featureOptionName) {
        DEBUG_LOG("featureOptionName is NULL\n");
        return NULL;
    }

    HASH_FIND_STR(appHandle->featureOptionsHash, featureOptionName, featureOption);
    if (featureOption) {
        INFO_LOG("Cache found: %s = %s", featureOptionName, featureOption->value);
        return featureOption->value;
    } else {
        INFO_LOG("Cache not found: %s", featureOptionName);
    }

#if defined(SYS_IMPL)
    char *featureOptionStr = NULL;
    UT_string *str = NULL;
    utstring_new(str);
    utstring_printf(str, APP_GET_FO_KEY "#%s", featureOptionName);
    featureOptionStr = audioSystemGetParameters(utstring_body(str));
    utstring_free(str);

    // Cache FO value
    featureOption = featureOptionCreate(featureOptionName, featureOptionStr);
    HASH_ADD_KEYPTR(hh, appHandle->featureOptionsHash, featureOption->name, strlen(featureOption->name), featureOption);
    free(featureOptionStr);

    return featureOption->value;
#else
    return NULL;
#endif
}

EXPORT int appHandleIsFeatureOptionEnabled(AppHandle *appHandle, const char *featureOptionName) {
    const char *featureOptionValueStr;
    if (!appHandle) {
        WARN_LOG("appHandle is NULL\n");
        return 0;
    }

    if (!featureOptionName) {
        WARN_LOG("featureOptionName is NULL\n");
        return 0;
    }

    featureOptionValueStr = appHandleGetFeatureOptionValue(appHandle, featureOptionName);
    if (featureOptionValueStr) {
        return !strncmp(featureOptionValueStr, "yes", strlen("yes") + 1);
    } else {
        DEBUG_LOG("No %s such feature option\n", featureOptionName);
        return 0;
    }
}

EXPORT size_t appHandleGetNumOfFeatureOption(AppHandle *appHandle) {
    if (!appHandle) {
        ERR_LOG("appHandle is NULL!\n");
        return APP_ERROR;
    }

    return HASH_COUNT(appHandle->featureOptionsHash);
}

EXPORT FeatureOption *appHandleGetFeatureOptionByIndex(AppHandle *appHandle, size_t index) {
    FeatureOption *featureOption = NULL;
    size_t i = 0;

    if (!appHandle) {
        ERR_LOG("appHandle is NULL\n");
        return NULL;
    }

    for (featureOption = appHandle->featureOptionsHash; featureOption ; featureOption = featureOption->hh.next) {
        if (index == i++) {
            return featureOption;
        }
    }

    return NULL;
}

/* This function is only work for windows */
EXPORT void appHandleRedirectIOToConsole() {
    INFO_LOG("");
#ifdef WIN32
    if (outputLogToStdout == 0) {
        outputLogToStdout = 1;
        redirectIOToConsole();
    }
#endif
}

int removeNodeByFeatureOption(AppHandle *appHandle, xmlNode *node) {
    /* Process Category of CategoryTpe Node */
    xmlChar *featureOption = xmlNodeGetProp(node, ATTRI_FEATURE_OPTION);
    xmlChar *featureOptionFullStr = featureOption;
    if (featureOption) {
        int not = 0;
        if (featureOption[0] == '!') {
            not = 1;
            featureOption++;
        }

        if (!(not ^ appHandleIsFeatureOptionEnabled(appHandle, (char *)featureOption))) {
            xmlChar *deleteNodeName;
            xmlNode *deleteNode = node;
            node = node->next;

            deleteNodeName = xmlNodeGetProp(deleteNode, ATTRI_NAME);
            INFO_LOG("Remove %s category (%s feature option is disabled)\n", deleteNodeName, featureOption);
            xmlFree(deleteNodeName);
            xmlUnlinkNode(deleteNode);
            xmlFreeNode(deleteNode);
            xmlFree(featureOptionFullStr);
            return 1;
        }
        xmlFree(featureOptionFullStr);
    }

    return 0;
}

EXPORT int appHandleIsNodeFeatureOptionEnabled(AppHandle *appHandle, xmlNode *node, int defaultValue) {
    xmlChar *featureOption = xmlNodeGetProp(node, ATTRI_FEATURE_OPTION);
    xmlChar *featureOptionFullStr = featureOption;
    int ret = defaultValue;
    if (featureOption) {
        int not = 0;
        if (featureOption[0] == '!') {
            not = 1;
            featureOption++;
        }

        if (!(not ^ appHandleIsFeatureOptionEnabled(appHandle, (char *)featureOption))) {
            ret = 0;
        } else {
            ret = 1;
        }

        xmlFree(featureOptionFullStr);
    }

    return ret;
}

EXPORT void appHandleRemoveAudioTypeByFeatureOptions(AppHandle *appHandle) {
    int i;
    size_t numOfAudioType = appHandleGetNumOfAudioType(appHandle);
    for (i = numOfAudioType - 1; i >= 0; i--) {
        AudioType *audioType = appHandleGetAudioTypeByIndex(appHandle, i);
        xmlNode *paramUnitDescNode = audioTypeGetParamUnitDescNode(audioType);

        if (paramUnitDescNode) {
            /* Checking if the ParamUnitDesc node's feature option is disabled */
            if (appHandleIsNodeFeatureOptionEnabled(appHandle, paramUnitDescNode, 1) == 0) {
                INFO_LOG("%s AudioType's feature option is disabled, remove it!", audioType->name);
                HASH_DEL(appHandle->audioTypeHash, audioType);
                audioTypeRelease(audioType);
            }
        }
    }
}

EXPORT void appHandleReviseXmlDocByFeatureOptions(AppHandle *appHandle) {
    // Travel all audioType's category & category group node
    size_t i;
    size_t numOfAppHandle = appHandleGetNumOfAudioType(appHandle);
    for (i = 0; i < numOfAppHandle; i++) {
        xmlNode *categoryTypeListNode, *categoryTypeNode, *categoryGroupNode, *categoryNode, *prevCategoryGroupNode, *prevCategoryNode, *prevCategoryTypeNode;
        xmlNode *paramUnitNode, *paramNode, *fieldNode, *preFieldNode;
        AudioType *audioType = appHandleGetAudioTypeByIndex(appHandle, i);

        /* Revise param unit */
        paramUnitNode = audioTypeGetParamUnitNode(audioType);
        if (paramUnitNode) {
            paramNode = paramUnitNode->children;
            while ((paramNode = findXmlNodeByElemName(paramNode->next, ELEM_PARAM))) {
                fieldNode = paramNode->children;
                while (fieldNode && (fieldNode = findXmlNodeByElemName(fieldNode->next, ELEM_FIELD))) {
                    preFieldNode = fieldNode->prev;
                    if (removeNodeByFeatureOption(appHandle, fieldNode)) {
                        fieldNode = preFieldNode;
                        continue;
                    }
                }
            }
        }


        /* Revise category */
        categoryTypeListNode = audioTypeGetCategoryTypeListNode(audioType);
        if (!categoryTypeListNode) {
            continue;
        }

        categoryTypeNode = categoryTypeListNode->children;
        while ((categoryTypeNode = findXmlNodeByElemName(categoryTypeNode->next, ELEM_CATEGORY_TYPE))) {
            prevCategoryTypeNode = categoryTypeNode->prev;
            if (removeNodeByFeatureOption(appHandle, categoryTypeNode)) {
                categoryTypeNode = prevCategoryTypeNode;
                continue;
            }

            /* Process CategoryType node */
            categoryGroupNode = categoryTypeNode->children;
            while ((categoryGroupNode = findXmlNodeByElemName(categoryGroupNode->next, ELEM_CATEGORY_GROUP))) {
                /* Process CategoryGroup of CategoryType Node */
                prevCategoryGroupNode = categoryGroupNode->prev;
                if (removeNodeByFeatureOption(appHandle, categoryGroupNode)) {
                    categoryGroupNode = prevCategoryGroupNode;
                    continue;
                }

                categoryNode = categoryGroupNode->children;
                while ((categoryNode = findXmlNodeByElemName(categoryNode->next, ELEM_CATEGORY))) {
                    /* Process Category of CategoryGroup Node */
                    prevCategoryNode = categoryNode->prev;
                    if (removeNodeByFeatureOption(appHandle, categoryNode)) {
                        categoryNode = prevCategoryNode;
                    }
                }
            }

            categoryNode = categoryTypeNode->children;
            while ((categoryNode = findXmlNodeByElemName(categoryNode->next, ELEM_CATEGORY))) {
                prevCategoryNode = categoryNode->prev;
                if (removeNodeByFeatureOption(appHandle, categoryNode)) {
                    categoryNode = prevCategoryNode;
                }
            }
        }
    }
}

EXPORT APP_STATUS appHandleCompressFiles(const char *srcDir, const char *destFile) {
#ifdef WIN32
    INFO_LOG("%s(), src = %s, dest = %s\n", __FUNCTION__, srcDir, destFile);
    if (!srcDir || !destFile) {
        ERR_LOG("%s(), srcDir or destFile is NULL\n", __FUNCTION__);
        return APP_ERROR;
    } else {
        UT_string *path = NULL;
        utstring_new(path);
        utstring_printf(path, "a -tzip %s %s\\*", destFile, srcDir);
        utilShellExecute("7za.exe", utstring_body(path));
        utstring_free(path);
    }
#else
    ERR_LOG("Not support on linux (src:%s, dst:%s)\n", srcDir, destFile);
#endif
    return APP_NO_ERROR;
}

EXPORT APP_STATUS appHandleUncompressFile(const char *srcFile, const char *destDir) {
#ifdef WIN32
    INFO_LOG("%s(), src = %s, dest = %s\n", __FUNCTION__, srcFile, destDir);
    if (!srcFile || !destDir) {
        ERR_LOG("%s(), srcFile or destDir is NULL\n", __FUNCTION__);
        return APP_ERROR;
    } else {
        UT_string *path = NULL;
        utstring_new(path);
        utstring_printf(path, "x %s -y -o%s\\", srcFile, destDir);
        utilShellExecute("7za.exe", utstring_body(path));
        utstring_free(path);
    }
#else
    ERR_LOG("Not support on linux(src:%s, dest:%s)\n", srcFile, destDir);
#endif
    return APP_NO_ERROR;
}

EXPORT APP_STATUS appHandleGetAudioTypeSupportedVerInfo(const char *audioTypeName, int *paramUnitDescVerMaj, int *paramUnitDescVerMin, int *audioParamVerMaj, int *audioParamVerMin) {
    int i = 0;
    while (audioTypeSupportVerInfo[i].audioTypeName != NULL) {
        if (!strncmp(audioTypeName, audioTypeSupportVerInfo[i].audioTypeName, strlen(audioTypeName) + 1)) {
            *paramUnitDescVerMaj = audioTypeSupportVerInfo[i].paramUnitDescVerMaj;
            *paramUnitDescVerMin = audioTypeSupportVerInfo[i].paramUnitDescVerMin;
            *audioParamVerMaj = audioTypeSupportVerInfo[i].audioParamVerMaj;
            *audioParamVerMin = audioTypeSupportVerInfo[i].audioParamVerMin;
            return APP_NO_ERROR;
        }
        i++;
    }

    /* No audio type info found, using default version */
    *paramUnitDescVerMaj = 1;
    *paramUnitDescVerMin = 0;
    *audioParamVerMaj = 1;
    *audioParamVerMin = 0;

    INFO_LOG("%s AudioType version support info not found! Set the version with default 1.0\n", audioTypeName);
    return APP_NO_ERROR;
}

EXPORT void appHandleShowAudioTypeSupportedVerInfo(AppHandle *appHandle) {
    int i = 0;
    INFO_LOG("===========(AppHandle = 0x%p)============\n", appHandle);
    while (audioTypeSupportVerInfo[i].audioTypeName != NULL) {
        INFO_LOG("[%d] %s, ParamUnitDesc ver(%d.%d), AudioParam ver(%d.%d)\n",
                 i,
                 audioTypeSupportVerInfo[i].audioTypeName, audioTypeSupportVerInfo[i].paramUnitDescVerMaj,
                 audioTypeSupportVerInfo[i].paramUnitDescVerMin, audioTypeSupportVerInfo[i].audioParamVerMaj, audioTypeSupportVerInfo[i].audioParamVerMin);
        i++;
    }
}
