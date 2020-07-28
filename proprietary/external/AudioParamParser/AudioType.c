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
 *   Implement AudioType related APIs
 */

#include "AudioParamParserPriv.h"

EXPORT AudioType *audioTypeCreate(AppHandle *appHandle, const char *audioTypeName) {
    AudioType *audioType = malloc(sizeof(AudioType));
    if (!audioType) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    audioType->name = strdup(audioTypeName);
    audioType->tabName = NULL;
    audioType->audioParamDoc = NULL;
    audioType->paramUnitDescDoc = NULL;
    audioType->paramTreeViewDoc = NULL;
    audioType->paramTreeHash = NULL;
    audioType->paramTreeView = NULL;
    audioType->paramUnitHash = NULL;
    audioType->paramInfoHash = NULL;
    audioType->categoryTypeHash = NULL;
    audioType->unusedParamId = 0;
    audioType->dirty = 0;
    audioType->allowReload = 0;
    audioType->appHandle = appHandle;
    audioType->paramUnitDescVerMaj = 1;
    audioType->paramUnitDescVerMin = 0;
    audioType->audioParamVerMaj = 1;
    audioType->audioParamVerMin = 0;
#ifndef WIN32
    pthread_rwlock_init(&audioType->lock, NULL);
#endif

    return audioType;
}

EXPORT void audioTypeReleaseAudioParam(AudioType *audioType) {
    if (audioType->paramTreeHash) {
        ParamTree *tmp, *item;
        HASH_ITER(hh, audioType->paramTreeHash, item, tmp) {
            if (audioType->paramTreeHash) {
                HASH_DEL(audioType->paramTreeHash, item);
                paramTreeRelease(item);
            }
        }
        free(audioType->paramTreeHash);
    }
    audioType->paramTreeHash = NULL;

    if (audioType->paramUnitHash) {
        ParamUnit *tmp, *item;
        HASH_ITER(hh, audioType->paramUnitHash, item, tmp) {
            if (audioType->paramUnitHash) {
                HASH_DEL(audioType->paramUnitHash, item);
                paramUnitRelease(item);
            }
        }
        free(audioType->paramUnitHash);
    }
    audioType->paramUnitHash = NULL;
}

EXPORT void audioTypeRelease(AudioType *audioType) {
    // Release AudioType resources
    INFO_LOG("Free %s AudioType\n", audioType->name);

    /* release XML */
    if (audioType->audioParamDoc) {
        xmlFreeDoc(audioType->audioParamDoc);
    }

    if (audioType->paramUnitDescDoc) {
        xmlFreeDoc(audioType->paramUnitDescDoc);
    }

    if (audioType->paramTreeViewDoc) {
        xmlFreeDoc(audioType->paramTreeViewDoc);
    }

    /* Release hash & it's content */
    if (audioType->categoryTypeHash) {
        CategoryType *tmp, *item;
        HASH_ITER(hh, audioType->categoryTypeHash, item, tmp) {
            if (audioType->categoryTypeHash) {
                HASH_DEL(audioType->categoryTypeHash, item);
                categoryTypeRelease(item);
            }
        }
    }

    if (audioType->paramInfoHash) {
        ParamInfo *tmp, *item;
        HASH_ITER(hh, audioType->paramInfoHash, item, tmp) {
            if (audioType->paramInfoHash) {
                HASH_DEL(audioType->paramInfoHash, item);
                paramInfoRelease(item);
            }
        }
    }

    /* Release audio param info */
    audioTypeReleaseAudioParam(audioType);

    /* Release param treee view info */
    paramTreeViewRelease(audioType->paramTreeView);

#ifndef WIN32
    pthread_rwlock_destroy(&audioType->lock);
#endif

    audioType->appHandle = NULL;
    free(audioType->tabName);
    free(audioType->name);
    free(audioType);
}

EXPORT int audioTypeReadLock(AudioType *audioType, const char *callerFun) {
    int ret = 0;

    if (!audioType) {
        WARN_LOG("audioType is NULL\n");
        return ret;
    }

    /* Lock appHandle */
    appHandleReadLock(audioType->appHandle, callerFun);

#ifndef WIN32
    while (1) {
        if (pthread_rwlock_tryrdlock(&audioType->lock) == 0) {
            audioType->lockCallerFun = callerFun;
            DEBUG_LOG("%s audioType lock is locked by %s()\n", audioType->name, audioType->lockCallerFun);
            break;
        } else {
            DEBUG_LOG("Cannot lock the %s audioType lock, delay some time. (the locker is %s())\n", audioType->name, audioType->lockCallerFun);
            appHandleUnlock(audioType->appHandle);
            utilUsleep(1);
            appHandleReadLock(audioType->appHandle, callerFun);
        }
    }
#else
    //DEBUG_LOG("Not support this function yet!\n");
#endif

    return ret;
}

EXPORT int audioTypeWriteLock(AudioType *audioType, const char *callerFun) {
    int ret = 0;

    if (!audioType) {
        WARN_LOG("audioType is NULL\n");
        return ret;
    }

    /* Lock appHandle */
    appHandleWriteLock(audioType->appHandle, callerFun);

#ifndef WIN32
    while (1) {
        if (pthread_rwlock_trywrlock(&audioType->lock) == 0) {
            audioType->lockCallerFun = callerFun;
            DEBUG_LOG("%s audioType lock is locked by %s()\n", audioType->name, audioType->lockCallerFun);
            break;
        } else {
            DEBUG_LOG("Cannot lock the %s audioType lock, delay some time. (the locker is %s())\n", audioType->name, audioType->lockCallerFun);
            appHandleUnlock(audioType->appHandle);
            utilUsleep(1);
            appHandleWriteLock(audioType->appHandle, callerFun);
        }
    }
#else
    //DEBUG_LOG("Not support this function yet!\n");
#endif
    return ret;
}

EXPORT int audioTypeUnlock(AudioType *audioType) {
    int ret = 0;

    if (!audioType) {
        WARN_LOG("audioType is NULL\n");
        return ret;
    }

#ifndef WIN32
    DEBUG_LOG("Unlock %s audioType lock\n", audioType->name);
    ret = pthread_rwlock_unlock(&audioType->lock);
#endif

    /* Unlock appHandle */
    appHandleUnlock(audioType->appHandle);
    return ret;
}

EXPORT void audioTypeDump(AudioType *audioType) {
    CategoryType *categoryType;
    ParamUnit *paramUnit;
    ParamTree *paramTree;
    ParamInfo *paramInfo;

    INFO_LOG("====================================");
    INFO_LOG("name = %s\n", audioType->name);
    INFO_LOG("tabName = %s\n", audioType->tabName);
    INFO_LOG("paramUnitDescVerMaj = %d\n", audioType->paramUnitDescVerMaj);
    INFO_LOG("paramUnitDescVerMin = %d\n", audioType->paramUnitDescVerMin);
    INFO_LOG("audioParamVerMaj = %d\n", audioType->audioParamVerMaj);
    INFO_LOG("audioParamVerMin = %d\n", audioType->audioParamVerMin);
    INFO_LOG("unusedParamId = %d\n", audioType->unusedParamId);
    INFO_LOG("dirty = %d\n", audioType->dirty);
    INFO_LOG("allowReload = %d\n", audioType->allowReload);

    /* Dump CategoryType */
    for (categoryType = audioType->categoryTypeHash; categoryType; categoryType = categoryType->hh.next) {
        // TODO: categoryTypeDump(categoryType);
    }

    /* Dump ParamInfo */
    for (paramInfo = audioType->paramInfoHash; paramInfo; paramInfo = paramInfo->hh.next) {
        // TODO: paramInfoDump(paramInfo);
    }

    /* Dump ParamUnit */
    for (paramTree = audioType->paramTreeHash; paramTree; paramTree = paramTree->hh.next) {
        // TODO: paramTreeDump(paramTree);
    }

    /* Dump ParamUnit */
    for (paramUnit = audioType->paramUnitHash; paramUnit; paramUnit = paramUnit->hh.next) {
        // TODO: paramUnitDump(paramUnit);
    }
}

EXPORT APP_STATUS audioTypeIsTuningToolSupportedXmlVer(AudioType *audioType) {
    int paramUnitDescVerMaj;
    int paramUnitDescVerMin;
    int audioParamVerMaj;
    int audioParamVerMin;

    INFO_LOG("AudioType = %s ParamUnitDesc ver = (%d.%d), AudioParam ver = (%d.%d)\n", audioType ? audioType->name : "NULL", audioType ? audioType->paramUnitDescVerMaj : -1, audioType ? audioType->paramUnitDescVerMin : -1, audioType ? audioType->audioParamVerMaj : -1, audioType ? audioType->audioParamVerMin : -1);

    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return APP_ERROR;
    }

    if (appHandleGetAudioTypeSupportedVerInfo(audioType->name, &paramUnitDescVerMaj, &paramUnitDescVerMin, &audioParamVerMaj, &audioParamVerMin) == APP_ERROR) {
        WARN_LOG("Cannot find the %s AudioType support info, don't check it's version\n", audioType->name);
        return APP_NO_ERROR;
    }

    if (audioType->paramUnitDescVerMaj > paramUnitDescVerMaj) {
        ERR_LOG("%s AudioType XML ParamUnitDesc's maj version is not support! XML ver (%d.%d) > AppLib support ver (%d.%d)\n", audioType->name, audioType->paramUnitDescVerMaj, audioType->paramUnitDescVerMin, paramUnitDescVerMaj, paramUnitDescVerMin);
        return APP_ERROR;
    }

    if (audioType->audioParamVerMaj > audioParamVerMaj) {
        ERR_LOG("%s AudioType XML AudioParam's maj version is not support! XML ver (%d.%d) > AppLib support ver (%d.%d)\n", audioType->name, audioType->audioParamVerMaj, audioType->audioParamVerMin, audioParamVerMaj, audioParamVerMin);
        return APP_ERROR;
    }

    /* If XML ver is lower then tuning tool support, we can upgrade the XML content here automatically */

    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeIsDeviceSupportedXmlVer(AudioType *audioType) {
    int paramUnitDescVerMaj;
    int paramUnitDescVerMin;
    int audioParamVerMaj;
    int audioParamVerMin;

    INFO_LOG("AudioType = %s paramUnitDesc ver = (%d.%d), audioParam ver = (%d.%d)\n", audioType ? audioType->name : "NULL", audioType ? audioType->paramUnitDescVerMaj : -1, audioType ? audioType->paramUnitDescVerMin : -1, audioType ? audioType->audioParamVerMaj : -1, audioType ? audioType->audioParamVerMin : -1);

    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return APP_ERROR;
    }

    if (appHandleGetAudioTypeSupportedVerInfo(audioType->name, &paramUnitDescVerMaj, &paramUnitDescVerMin, &audioParamVerMaj, &audioParamVerMin) == APP_ERROR) {
        ERR_LOG("Cannot find the %s AudioType support info\n", audioType->name);
        return APP_ERROR;
    }

    if (audioType->paramUnitDescVerMaj > paramUnitDescVerMaj) {
        ERR_LOG("%s AudioType XML ParamUnitDesc's maj version is not support! XML ver (%d.%d) != AppLib support ver (%d.%d)\n", audioType->name, audioType->paramUnitDescVerMaj, audioType->paramUnitDescVerMin, paramUnitDescVerMaj, paramUnitDescVerMin);
        return APP_ERROR;
    }

    if (audioType->audioParamVerMaj > audioParamVerMaj) {
        ERR_LOG("%s AudioType XML AudioParam maj's version is not support! XML ver (%d.%d) != AppLib support ver (%d.%d)\n", audioType->name, audioType->audioParamVerMaj, audioType->audioParamVerMin, audioParamVerMaj, audioParamVerMin);
        return APP_ERROR;
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeParseTabName(AudioType *audioType) {
    xmlChar *tabName = NULL;
    xmlNode *node = NULL;
    xmlNode *root_element = xmlDocGetRootElement(audioType->paramUnitDescDoc);
    node = findXmlNodeByElemName(root_element, ELEM_PARAM_UNIT_DESC);

    if (!node) {
        ERR_LOG("Cannnot find the %s element\n", ELEM_PARAM_UNIT_DESC);
        return APP_ERROR;
    }

    tabName = xmlGetProp(node, (const xmlChar *)ATTRI_TAB_NAME);
    if (tabName) {
        audioType->tabName = strdup((char *)tabName);
        xmlFree(tabName);
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeParseXmlVer(AudioType *audioType) {
    xmlChar *ver = NULL;
    xmlNode *node = NULL;

    /* Parse paramUnitDescVer */
    xmlNode *root_element = xmlDocGetRootElement(audioType->paramUnitDescDoc);

    if (root_element) {
        node = findXmlNodeByElemName(root_element, ELEM_PARAM_UNIT_DESC);
    }

    if (node) {
        ver = xmlGetProp(node, (const xmlChar *)ATTRI_VERSION);
    }

    if (ver) {
        sscanf((const char *)ver, "%d.%d", &audioType->paramUnitDescVerMaj, &audioType->paramUnitDescVerMin);
        xmlFree(ver);
    } else {
        audioType->paramUnitDescVerMaj = 1;
        audioType->paramUnitDescVerMin = 0;
        INFO_LOG("Cannot parse paramUnitDesc xml version (set as default ver = 1.0)\n");
    }

    /* Parse audioParamVer */
    root_element = xmlDocGetRootElement(audioType->audioParamDoc);

    if (root_element) {
        node = findXmlNodeByElemName(root_element, ELEM_AUDIO_PARAM);
    }

    if (node) {
        ver = xmlGetProp(node, (const xmlChar *)ATTRI_VERSION);
    }

    if (ver) {
        sscanf((const char *)ver, "%d.%d", &audioType->audioParamVerMaj, &audioType->audioParamVerMin);
        xmlFree(ver);
    } else {
        INFO_LOG("Cannot parse audioParam xml version (set as default ver = 1.0)\n");
        audioType->audioParamVerMaj = 1;
        audioType->audioParamVerMin = 0;
    }

    return APP_NO_ERROR;
}

ParamTree *findParamTree(char **arr, int *Switch, int n, AudioType *audioType) {
    ParamTree *paramTree = NULL;
    UT_string *path = NULL;
    int i;

    /* Generate the search string */
    utstring_new(path);
    for (i = 0; i < n; ++i) {
        if (i == n - 1) {
            utstring_printf(path, "%s", arr[Switch[i]]);
        } else {
            utstring_printf(path, "%s,", arr[Switch[i]]);
        }
    }

    /* Find the paramTree */
    HASH_FIND_STR(audioType->paramTreeHash, utstring_body(path), paramTree);
    INFO_LOG("Search path = %s, paramTree = 0x%p\n", utstring_body(path), paramTree);

    utstring_free(path);
    return paramTree;
}

ParamTree *fuzzySearchParamTree(char **arr, int totalSize, int pickSize, AudioType *audioType) {
    ParamTree *paramTree = NULL;
    int i, j, pos = pickSize - 1;
    int *swpArray;

    if (pickSize > totalSize) {
        return paramTree;
    }

    swpArray = (int *)malloc(sizeof(int) * totalSize);
    if (!swpArray) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    for (i = 0; i < totalSize; ++i) {
        swpArray[i] = i;
    }

    paramTree = findParamTree(arr, swpArray, pickSize, audioType);
    if (paramTree) {
        free(swpArray);
        return paramTree;
    }

    do {
        if (swpArray[pickSize - 1] == totalSize - 1) {
            --pos;
        } else {
            pos = pickSize - 1;
        }

        ++swpArray[pos];

        for (j = pos + 1; j < pickSize; ++j) {
            swpArray[j] = swpArray[j - 1] + 1;
        }

        paramTree = findParamTree(arr, swpArray, pickSize, audioType);
        if (paramTree) {
            free(swpArray);
            return paramTree;
        }

    } while (swpArray[0] < totalSize - pickSize);

    free(swpArray);
    return paramTree;
}

ParamTree *searchParamTree(AudioType *audioType, const char *categoryPath) {
    ParamTree *paramTree = NULL;
    char **categoryArray = NULL;
    char *category = NULL;
    char *tmpStr = NULL;
    char *restOfStr = NULL;
    size_t numOfCategoryType = 0;
    size_t numOfCategory = 0;
    size_t i = 0;

    if (audioType) {
        DEBUG_LOG("+AudioType = %s, categoryPath = %s\n", audioType->name, categoryPath);
    } else {
        ERR_LOG("audioType is NULL\n");
        return NULL;
    }

    /* Full path search first */
    HASH_FIND_STR(audioType->paramTreeHash, categoryPath, paramTree);
    if (paramTree) {
        DEBUG_LOG("fuzzySearch paramTree found. (path = %s, id = %d)\n", categoryPath, paramTree->paramId);
        return paramTree;
    }

    /* Setup array for fuzzy search path enum */
    numOfCategoryType = audioTypeGetNumOfCategoryType(audioType);
    if (numOfCategoryType > 0) {
        categoryArray = malloc(sizeof(char *) * numOfCategoryType);
        if (NULL == categoryArray) {
            ERR_LOG("allocate working buf fail");
            return NULL;
        }
    }

    if (categoryArray) {
        tmpStr = strdup(categoryPath ? categoryPath : "");
        category = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);
        if (!category) {
            INFO_LOG("Cannot parse category\n");
            free(categoryArray);
            free(tmpStr);
            return NULL;
        }

        categoryArray[i++] = category;

        while ((category = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr)) != NULL) {
            categoryArray[i++] = category;
        }
        numOfCategory = i;

        /* Fuzzy search */
        for (i = 1; i < numOfCategory; i++) {
            paramTree = fuzzySearchParamTree(categoryArray, numOfCategory, numOfCategory - i, audioType);
            if (paramTree) {
                break;
            }
        }

        if (!paramTree) {
            /* If no paramTree found, try to get the root paramTree */
            UT_string *path = NULL;
            utstring_new(path);
            utstring_printf(path, "%s", "");
            HASH_FIND_STR(audioType->paramTreeHash, utstring_body(path), paramTree);
            utstring_free(path);
        }

        free(categoryArray);
        free(tmpStr);
    }

    if (paramTree) {
        DEBUG_LOG("-fuzzySearch paramTree %s found. (path = %s, id = %d)\n", paramTree ? "" : "not ", paramTree->categoryPath, paramTree->paramId);
    }

    return paramTree;
}

EXPORT ParamUnit *audioTypeGetParamUnit(AudioType *audioType, const char *categoryPath) {
    /* Get the category path */
    ParamTree *paramTree = NULL;
    ParamUnit *paramUnit = NULL;
    UT_string *searchPath;

    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return NULL;
    }

    if (!categoryPath) {
        ERR_LOG("categoryPath is NULL (%s)\n", audioType->name);
        return NULL;
    }

    searchPath = utilNormalizeCategoryPathForAudioType(categoryPath, audioType);
    if (!searchPath) {
        ERR_LOG("Cannot normalize categoryPath for %s AudioType. (path = %s)\n", audioType->name, categoryPath);
        return NULL;
    }

    /* CategoryPath -> Param Id */
    paramTree = searchParamTree(audioType, utstring_body(searchPath));

    /* Param Id -> Param Unit */
    if (paramTree) {
        HASH_FIND_INT(audioType->paramUnitHash, &paramTree->paramId, paramUnit);
        INFO_LOG("AudioType = %s, Search category path = \"%s\" -> \"%s\" -> ref_id = %d -> ParamUnit = 0x%p\n", audioType->name, categoryPath, utstring_body(searchPath), paramTree ? paramTree->paramId : -1, paramUnit);
    } else {
        INFO_LOG("No match ref id!!  (AudioType = %s, Search category path = \"%s\" -> \"%s\" -> ref id not found)\n", audioType->name, categoryPath, utstring_body(searchPath));
    }

    utstring_free(searchPath);

    return paramUnit;
}

EXPORT APP_STATUS audioTypeLoadStage1Hash(AudioType *audioType) {
    if (audioTypeLoadCategoryTypeHash(audioType) == APP_ERROR) {
        return APP_ERROR;
    }

    if (audioTypeLoadParamFieldInfoHash(audioType) == APP_ERROR) {
        return APP_ERROR;
    }

    if (audioTypeLoadParamUnitHash(audioType) == APP_ERROR) {
        return APP_ERROR;
    }

    if (audioTypeLoadParamTreeHash(audioType) == APP_ERROR) {
        return APP_ERROR;
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeLoadStage2Hash(AudioType *audioType) {
    if (audioTypeLoadParamTreeView(audioType) == APP_ERROR) {
        return APP_ERROR;
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeLoadParamTreeView(AudioType *audioType) {
    xmlNode *root = NULL, *paramTreeViewNode = NULL, *treeRootNode = NULL;
    if (!audioType) {
        DEBUG_LOG("AudioType is NULL\n");
        return APP_NO_ERROR;
    }

    if (!audioType->paramTreeViewDoc) {
        DEBUG_LOG("No %s%s file exist\n", audioType->name, PARAM_TREE_VIEW_XML_POSFIX);
        return APP_NO_ERROR;
    }

    if (audioType->paramTreeView) {
        ERR_LOG("The audio type's paramTreeView already exist!\n");
        return APP_ERROR;
    }

    root = xmlDocGetRootElement(audioType->paramTreeViewDoc);

    if (root) {
        paramTreeViewNode = findXmlNodeByElemName(root, ELEM_PARAM_TREE_VIEW);
    }

    if (paramTreeViewNode && paramTreeViewNode->children) {
        /* Parse version info */
        int maj, min;
        xmlChar *ver = xmlGetProp(paramTreeViewNode, (const xmlChar *)ATTRI_VERSION);

        if (ver) {
            sscanf((const char *)ver, "%d.%d", &maj, &min);
            xmlFree(ver);
        } else {
            INFO_LOG("Cannot parse xml version (ver = %s)\n", ver);
            return APP_ERROR;
        }

        audioType->paramTreeView = paramTreeViewCreate(audioType, maj, min);
        treeRootNode = paramTreeViewNode->children;
        while ((treeRootNode = findXmlNodeByElemName(treeRootNode->next, ELEM_TREE_ROOT))) {
            /* Process <TreeRoot> */
            xmlChar *treeRootName = xmlNodeGetProp(treeRootNode, ATTRI_NAME);
            TreeRoot *treeRoot = treeRootCreate((const char *)treeRootName, treeRootNode, audioType->paramTreeView);
            HASH_ADD_KEYPTR(hh, audioType->paramTreeView->treeRootHash, treeRoot->name, strlen(treeRoot->name), treeRoot);
            INFO_LOG("Add treeRoot name = %s, audioType = %s\n", treeRoot->name, treeRoot->paramTreeView->audioType->name);
            xmlFree(treeRootName);

            if (treeRootNode->children) {
                /* Process <Sheet> */
                xmlNode *featureNode = NULL;
                xmlNode *sheetNode = findXmlNodeByElemName(treeRootNode->children, ELEM_SHEET);
                if (sheetNode) {
                    treeRoot->switchFieldInfo = utilXmlNodeGetFieldInfo(audioType->appHandle, sheetNode, ATTRI_SWITCH_AUDIO_TYPE, ATTRI_SWITCH_PARAM, ATTRI_SWITCH_FIELD);
                    if (treeRoot->switchFieldInfo) {
                        INFO_LOG("Add Sheet %s=%s, %s=%s, %s=%s\n", ATTRI_SWITCH_AUDIO_TYPE, treeRoot->switchFieldInfo->paramInfo->audioType->name, ATTRI_SWITCH_PARAM, treeRoot->switchFieldInfo->paramInfo->name, ATTRI_SWITCH_FIELD, treeRoot->switchFieldInfo->name);
                    }
                }

                /* Process <Feature> */
                featureNode = treeRootNode->children;
                while ((featureNode = findXmlNodeByElemName(featureNode->next, ELEM_FEATURE))) {
                    xmlNode *fieldListNode, *fieldNode, *categoryPathListNode, *categoryNode = NULL;
                    FieldInfo *switchFieldInfo = NULL;
                    Feature *feature = NULL;

                    /* Check feature option first */
                    /* TODO: check feature options */
                    xmlChar *featureName = xmlGetProp(featureNode, (const xmlChar *)ATTRI_NAME);
                    xmlChar *featureOptionName = xmlGetProp(featureNode, (const xmlChar *)ATTRI_FEATURE_OPTION);
                    /*if (!appHandleIsFeatureOptionEnabled(audioType->appHandle, featureOptionName)) {
                        INFO_LOG("Feature %s not enabled! (depends on feature_option %s\n", featureName, featureOptionName);
                        continue;
                    }*/

                    /* Check switch info */
                    switchFieldInfo = utilXmlNodeGetFieldInfo(audioType->appHandle, featureNode, ATTRI_SWITCH_AUDIO_TYPE, ATTRI_SWITCH_PARAM, ATTRI_SWITCH_FIELD);

                    /* Create Feature */
                    feature = featureCreate((const char *)featureName, audioType, switchFieldInfo, (const char *)featureOptionName);
                    HASH_ADD_KEYPTR(hh, treeRoot->featureHash, feature->name, strlen(feature->name), feature);

                    if (feature->switchFieldInfo) {
                        INFO_LOG("Add Feature name = %s, feature_option = %s, %s=%s, %s=%s, %s=%s\n", featureName, featureOptionName, ATTRI_AUDIO_TYPE, feature->switchFieldInfo->paramInfo->audioType->name, ATTRI_PARAM, feature->switchFieldInfo->paramInfo->name, ATTRI_NAME, feature->switchFieldInfo->name);
                    }

                    xmlFree(featureName);
                    xmlFree(featureOptionName);

                    /* Process <FieldList> */
                    fieldListNode = findXmlNodeByElemName(featureNode->children, ELEM_FIELD_LIST);
                    if (fieldListNode && fieldListNode->children) {
                        fieldNode = fieldListNode->children;
                        while ((fieldNode = findXmlNodeByElemName(fieldNode->next, ELEM_FIELD))) {
                            /* Process <Field> */
                            FieldInfo *fieldInfo = utilXmlNodeGetFieldInfo(audioType->appHandle, fieldNode, ATTRI_AUDIO_TYPE, ATTRI_PARAM, ATTRI_NAME);
                            if (fieldInfo) {
                                FeatureField *featureField = featureFieldCreate(fieldInfo);
                                HASH_ADD_KEYPTR(hh, feature->featureFieldHash, featureField->fieldInfo->name, strlen(featureField->fieldInfo->name), featureField);
                                INFO_LOG("Add FeatureField %s=%s, %s=%s, %s=%s\n", ATTRI_AUDIO_TYPE, featureField->fieldInfo->paramInfo->audioType->name, ATTRI_PARAM, featureField->fieldInfo->paramInfo->name, ATTRI_NAME, featureField->fieldInfo->name);
                            }
                        }
                    }

                    /* Process <CategoryPathList> */
                    categoryPathListNode = findXmlNodeByElemName(featureNode->children, ELEM_CATEGORY_PATH_LIST);
                    if (categoryPathListNode && categoryPathListNode->children) {
                        categoryNode = categoryPathListNode->children;
                        while ((categoryNode = findXmlNodeByElemName(categoryNode->next, ELEM_CATEGORY))) {
                            /* Process <Category> */
                            xmlChar *path = xmlGetProp(categoryNode, (const xmlChar *)ATTRI_PATH);
                            if (path) {
                                CategoryPath *categoryPath = categoryPathCreate(feature, (const char *)path);
                                if (categoryPath) {
                                    HASH_ADD_KEYPTR(hh, feature->categoryPathHash, categoryPath->path, strlen(categoryPath->path), categoryPath);
                                    INFO_LOG("Add CategoryPath path=%s\n", categoryPath->path);
                                } else {
                                    ERR_LOG("CategoryPath creation fail! (%s)\n", path);
                                }
                                xmlFree(path);
                            }
                        }
                    }
                }
            }
        }
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeLoadParamTreeHash(AudioType *audioType) {
    xmlNode *root = NULL, *audioParamNode = NULL, *paramTreeNode = NULL, *paramNode = NULL;
    root = xmlDocGetRootElement(audioType->audioParamDoc);

    if (root) {
        audioParamNode = findXmlNodeByElemName(root, ELEM_AUDIO_PARAM);
    }

    if (audioParamNode) {
        paramTreeNode = findXmlNodeByElemName(audioParamNode->children, ELEM_PARAM_TREE);
    }

    if (paramTreeNode && paramTreeNode->children) {
        paramNode = paramTreeNode->children;
    } else {
        ERR_LOG("No param element found!\n");
        return APP_ERROR;
    }

    while ((paramNode = findXmlNodeByElemName(paramNode->next, ELEM_PARAM))) {
        xmlChar *paramId = xmlNodeGetProp(paramNode, ATTRI_PARAM_ID);
        xmlChar *path = xmlNodeGetProp(paramNode, ATTRI_PATH);

        if (paramId && path) {
            ParamUnit *paramUnit = NULL;
            ParamTree *item = paramTreeCreate((int)strtoul((char *)paramId, NULL, 0), (char *)path);
            HASH_ADD_KEYPTR(hh, audioType->paramTreeHash, item->categoryPath, strlen(item->categoryPath), item);
            DEBUG_LOG("Add param to ParamTree hash, path=\"%s\", paramId = %d\n", item->categoryPath, item->paramId);

            /* Update the paramUnit's refCount */
            HASH_FIND_INT(audioType->paramUnitHash, &item->paramId, paramUnit);
            if (paramUnit) {
                paramUnit->refCount++;
            }
        } else {
            WARN_LOG("Invalid ParamTree item! (path=%s, param_id=%s)\n", (char *)path, (char *)paramId);
        }

        xmlFree(paramId);
        xmlFree(path);
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeLoadParamUnitHash(AudioType *audioType) {
    xmlNode *root = NULL, *audioParamNode = NULL, *paramTreeNode = NULL, *paramUnitNode = NULL;
    root = xmlDocGetRootElement(audioType->audioParamDoc);

    if (root) {
        audioParamNode = findXmlNodeByElemName(root, ELEM_AUDIO_PARAM);
    }

    if (audioParamNode) {
        paramTreeNode = findXmlNodeByElemName(audioParamNode->children, ELEM_PARAM_UNIT_POOL);
    }

    if (paramTreeNode && paramTreeNode->children) {
        paramUnitNode = paramTreeNode->children;
    } else {
        ERR_LOG("No paramUnit element found!\n");
        return APP_ERROR;
    }

    while ((paramUnitNode = findXmlNodeByElemName(paramUnitNode->next, ELEM_PARAM_UNIT))) {
        xmlChar *paramIdXmlStr = xmlNodeGetProp(paramUnitNode, ATTRI_PARAM_ID);
        int paramId = (int)strtoul((char *)paramIdXmlStr, NULL, 0);
        Param *paramHash = audioTypeGetParamHash(audioType, paramUnitNode);
        ParamUnit *paramUnit = paramUnitCreate(audioType, paramId, paramHash);

        /* Insert to hash */
        HASH_ADD_INT(audioType->paramUnitHash, paramId, paramUnit);
        DEBUG_LOG("Add ParamHash to ParamUnit hash. (id=%d, param = %p)\n", paramUnit->paramId, paramUnit->paramHash);
        xmlFree(paramIdXmlStr);
    }

    return APP_NO_ERROR;
}
EXPORT Param *audioTypeGetParamHash(AudioType *audioType, xmlNode *paramUnitNode) {
    Param *paramHash = NULL;
    Param *paramItem = NULL;
    ParamInfo *paramInfo = NULL;
    xmlNode *paramNode = paramUnitNode->children;

    if (!paramNode) {
        return NULL;
    }

    while ((paramNode = findXmlNodeByElemName(paramNode->next, ELEM_PARAM))) {
        /* Get param info */
        xmlChar *paramName = xmlNodeGetProp(paramNode, ATTRI_NAME);
        xmlChar *paramValue = xmlNodeGetProp(paramNode, ATTRI_VALUE);

        if (paramName && paramValue) {
            HASH_FIND_STR(audioType->paramInfoHash, (char *)paramName, paramInfo);

            /* Add param to hash */
            if (paramInfo) {
                paramItem = paramCreate((char *)paramName, paramInfo, (char *)paramValue);
                HASH_ADD_KEYPTR(hh, paramHash, paramItem->name, strlen(paramItem->name), paramItem);
                DEBUG_LOG("Add param (name = %s, value = %s) to paramHash(0x%p)\n", paramName, paramValue, paramHash);
            } else {
                WARN_LOG("Invalid paramName = %s", paramName);
            }
        } else {
            WARN_LOG("Invalid paramName = %s, paramValue = %s", paramName, paramValue);
        }

        if (paramName) {
            xmlFree(paramName);
        }

        if (paramValue) {
            xmlFree(paramValue);
        }
    }

    return paramHash;
}

EXPORT APP_STATUS audioTypeLoadCategoryTypeHash(AudioType *audioType) {
    xmlNode *node, *categoryTypeListNode, *categoryTypeNode, *categoryNode, *subCategoryNode;
    xmlNode *root_element = xmlDocGetRootElement(audioType->paramUnitDescDoc);
    if (!root_element) {
        return APP_ERROR;
    }

    node = findXmlNodeByElemName(root_element, ELEM_PARAM_UNIT_DESC);
    if (!node) {
        return APP_ERROR;
    }

    categoryTypeListNode = findXmlNodeByElemName(node->children, ELEM_CATEGORY_TYPE_LIST);
    if (!categoryTypeListNode) {
        return APP_ERROR;
    }

    categoryTypeNode = categoryTypeListNode->children;
    if (!categoryTypeNode) {
        return APP_ERROR;
    }

    while ((categoryTypeNode = findXmlNodeByElemName(categoryTypeNode->next, ELEM_CATEGORY_TYPE))) {
        xmlChar *categoryTypeName = xmlNodeGetProp(categoryTypeNode, ATTRI_NAME);
        xmlChar *categoryTypeWording = xmlNodeGetWording(categoryTypeNode);
        xmlChar *categoryTypeVisibleStr = xmlNodeGetProp(categoryTypeNode, ATTRI_VISIBLE);
        int categoryTypeVisible = categoryTypeVisibleStr ? strncmp((const char *)categoryTypeVisibleStr, "false", strlen("false") + 1) : 1;

        CategoryType *categoryType = categoryTypeCreate((char *)categoryTypeName, (char *)categoryTypeWording, audioType, categoryTypeVisible);
        HASH_ADD_KEYPTR(hh, audioType->categoryTypeHash, categoryType->wording, strlen(categoryType->wording), categoryType);

        categoryNode = categoryTypeNode->children;
        for (categoryNode = categoryTypeNode->children; categoryNode; categoryNode = categoryNode->next) {
            if (!strncmp((char *)categoryNode->name, ELEM_CATEGORY, strlen(ELEM_CATEGORY) + 1)) {
                int size;
                xmlChar *categoryName = xmlNodeGetProp(categoryNode, ATTRI_NAME);
                xmlChar *categoryWording = xmlNodeGetWording(categoryNode);
                xmlChar *aliasStr = xmlNodeGetProp(categoryNode, ATTRI_ALIAS);
                xmlChar *categoryVisibleStr = xmlNodeGetProp(categoryNode, ATTRI_VISIBLE);
                int categoryVisible = categoryVisibleStr ? strncmp((const char *)categoryVisibleStr, "false", strlen("false") + 1) : 1;

                /* Process wording info */
                Category *category = categoryCreate((char *)categoryName, (char *)categoryWording, PARENT_IS_CATEGORY_TYPE, categoryType, categoryVisible);
                HASH_ADD_KEYPTR(hh, categoryType->categoryHash, category->wording, strlen(category->wording), category);
                HASH_ADD_KEYPTR(hh2, categoryType->allCategoryHash, category->name, strlen(category->name), category);

                /* Process alias info */
                if (aliasStr) {
                    size = paramGetArraySizeFromString((char *)aliasStr);
                    if (size == 1) {
                        CategoryAlias *categoryAlias = categoryAliasCreate((char *)aliasStr, category);
                        HASH_ADD_KEYPTR(hh, categoryType->categoryAliasHash, categoryAlias->alias, strlen(categoryAlias->alias), categoryAlias);
                    } else {
                        char *tmpStr = strdup((char *)aliasStr);
                        char *restOfStr = NULL;
                        char *alias = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);

                        if (alias != NULL) {
                            CategoryAlias *categoryAlias = categoryAliasCreate(alias, category);
                            HASH_ADD_KEYPTR(hh, categoryType->categoryAliasHash, categoryAlias->alias, strlen(categoryAlias->alias), categoryAlias);

                            while ((alias = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
                                categoryAlias = categoryAliasCreate(alias, category);
                                HASH_ADD_KEYPTR(hh, categoryType->categoryAliasHash, categoryAlias->alias, strlen(categoryAlias->alias), categoryAlias);
                            }
                        }
                        free(tmpStr);
                    }
                }

                xmlFree(categoryName);
                xmlFree(categoryWording);
                xmlFree(aliasStr);
                xmlFree(categoryVisibleStr);
            } else if (!strncmp((char *)categoryNode->name, ELEM_CATEGORY_GROUP, strlen(ELEM_CATEGORY_GROUP) + 1)) {
                xmlChar *categoryGroupName = xmlNodeGetProp(categoryNode, ATTRI_NAME);
                xmlChar *categoryGroupWording = xmlNodeGetWording(categoryNode);
                xmlChar *categoryGroupVisibleStr = xmlNodeGetProp(categoryNode, ATTRI_VISIBLE);
                int categoryGroupVisible = categoryGroupVisibleStr ? strncmp((const char *)categoryGroupVisibleStr, "false", strlen("false") + 1) : 1;

                CategoryGroup *categoryGroup = categoryGroupCreate((char *)categoryGroupName, (char *)categoryGroupWording, categoryType, categoryGroupVisible);
                HASH_ADD_KEYPTR(hh, categoryType->categoryGroupHash, categoryGroup->wording, strlen(categoryGroup->wording), categoryGroup);

                xmlFree(categoryGroupName);
                xmlFree(categoryGroupWording);
                xmlFree(categoryGroupVisibleStr);

                //Category* subCategory = categoryCreate(subCategoryNode->name, xmlNodeGetWording(subCategoryNode), IS_CATEGORY_GROUP, categoryType, NULL);
                for (subCategoryNode = categoryNode->children; subCategoryNode; subCategoryNode = subCategoryNode->next) {
                    if (!strncmp((char *)subCategoryNode->name, ELEM_CATEGORY, strlen(ELEM_CATEGORY) + 1)) {
                        xmlChar *categoryName = xmlNodeGetProp(subCategoryNode, ATTRI_NAME);
                        xmlChar *categoryWording = xmlNodeGetWording(subCategoryNode);
                        xmlChar *categoryVisibleStr = xmlNodeGetProp(subCategoryNode, ATTRI_VISIBLE);
                        int visible = categoryVisibleStr ? strncmp((const char *)categoryVisibleStr, "false", strlen("false") + 1) : 1;

                        Category *category = categoryCreate((char *)categoryName, (char *)categoryWording, PARENT_IS_CATEGORY_GROUP, categoryGroup, visible);
                        HASH_ADD_KEYPTR(hh, categoryGroup->categoryHash, category->wording, strlen(category->wording), category);
                        HASH_ADD_KEYPTR(hh2, categoryType->allCategoryHash, category->name, strlen(category->name), category);
                        //printf("\t\t%s wording = %s (name = %s)\n", subCategoryNode->name, xmlNodeGetWording(subCategoryNode), xmlNodeGetProp(subCategoryNode, ATTRI_NAME));

                        xmlFree(categoryName);
                        xmlFree(categoryWording);
                        xmlFree(categoryVisibleStr);
                    }
                }

            }
        }
        xmlFree(categoryTypeName);
        xmlFree(categoryTypeWording);
        xmlFree(categoryTypeVisibleStr);
    }

    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeLoadParamFieldInfoHash(AudioType *audioType) {
    xmlNode *root = NULL, *audioParamUnitDescNode = NULL, *paramUnitNode = NULL, *paramNode = NULL, *fieldNode = NULL;
    root = xmlDocGetRootElement(audioType->paramUnitDescDoc);

    if (root) {
        audioParamUnitDescNode = findXmlNodeByElemName(root, ELEM_PARAM_UNIT_DESC);
    }

    if (audioParamUnitDescNode && audioParamUnitDescNode->children) {
        paramUnitNode = findXmlNodeByElemName(audioParamUnitDescNode->children, ELEM_PARAM_UNIT);
    }

    if (paramUnitNode && paramUnitNode->children) {
        paramNode = paramUnitNode->children;
    } else {
        ERR_LOG("No paramUnit element found!\n");
        return APP_ERROR;
    }

    while ((paramNode = findXmlNodeByElemName(paramNode->next, ELEM_PARAM))) {
        ParamInfo *paramInfo = NULL;
        /* Get all param */
        xmlChar *paramName = xmlNodeGetProp(paramNode, ATTRI_NAME);
        xmlChar *paramTypeStr = xmlNodeGetProp(paramNode, ATTRI_TYPE);
        DATA_TYPE paramType = paramDataTypeToEnum((char *)paramTypeStr);

        if (!paramName) {
            ERR_LOG("Cannot find the paramName\n");
            xmlFree(paramTypeStr);
            continue;
        }

        if (!paramTypeStr) {
            ERR_LOG("Cannot find the paramType\n");
            xmlFree(paramName);
            continue;
        }

        if (paramType == TYPE_UNKNOWN) {
            /* The default data type of param element is string*/
            paramType = TYPE_STR;
        }

        /* Insert to hash */
        paramInfo = paramInfoCreate((char *)paramName, paramType, audioType);
        HASH_ADD_KEYPTR(hh, audioType->paramInfoHash, paramInfo->name, strlen(paramInfo->name), paramInfo);
        DEBUG_LOG("Add ParamInfo to paramInfoHash hash. (name=%s, type = %x)\n", paramInfo->name, (paramInfo->dataType));

        fieldNode = paramNode->children;
        if (!fieldNode) {
            xmlFree(paramName);
            xmlFree(paramTypeStr);
            continue;
        }

        while ((fieldNode = findXmlNodeByElemName(fieldNode->next, ELEM_FIELD))) {
            xmlChar *fieldName;
            xmlChar *fieldArrayIndex;
            xmlChar *fieldBit;
            xmlChar *fieldCheckList;
            char *tmpStr;
            char *restOfStr = NULL;
            char *startBitStr = NULL;
            char *endBitStr = NULL;
            size_t arrayIndex;
            int startBit;
            int endBit;
            errno = 0;

            /* Get name */
            fieldName = xmlNodeGetProp(fieldNode, ATTRI_NAME);
            if (!fieldName) {
                ERR_LOG("Cannot find the fieldName\n");
                continue;
            }

            /* Get array_index */
            fieldArrayIndex = xmlNodeGetProp(fieldNode, ATTRI_ARRAY_INDEX);
            if (!fieldArrayIndex) {
                ERR_LOG("Cannot find the array_index\n");
                xmlFree(fieldName);
                continue;
            }

            arrayIndex = strtoul((const char *)fieldArrayIndex, NULL, 0);
            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to array_index!\n", fieldArrayIndex);
                xmlFree(fieldName);
                xmlFree(fieldArrayIndex);
                continue;
            }

            /* Get bit */
            fieldBit = xmlNodeGetProp(fieldNode, ATTRI_BIT);
            if (!fieldBit) {
                ERR_LOG("Cannot find the fieldBit\n");
                xmlFree(fieldName);
                xmlFree(fieldArrayIndex);
                continue;
            }

            tmpStr = strdup(fieldBit != NULL ? ((char *)fieldBit) : "");
            startBitStr = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);
            if (startBitStr == NULL) {
                ERR_LOG("No start bit string");
                xmlFree(fieldName);
                xmlFree(fieldArrayIndex);
                xmlFree(fieldBit);
                free(tmpStr);
                continue;
            }

            startBit = strtoul(startBitStr, NULL, 0);
            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to startBit!\n", tmpStr);
                xmlFree(fieldName);
                xmlFree(fieldArrayIndex);
                xmlFree(fieldBit);
                free(tmpStr);
                continue;
            }

            endBitStr = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr);
            if (endBitStr == NULL) {
                ERR_LOG("No end bit string");
                xmlFree(fieldName);
                xmlFree(fieldArrayIndex);
                xmlFree(fieldBit);
                free(tmpStr);
                continue;
            }

            endBit = strtoul(endBitStr, NULL, 0);
            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to endBit!\n", tmpStr);
                xmlFree(fieldName);
                xmlFree(fieldArrayIndex);
                xmlFree(fieldBit);
                free(tmpStr);
                continue;
            }
            free(tmpStr);

            /* Get check list */
            fieldCheckList = xmlNodeGetProp(fieldNode, ATTRI_CHECK_LIST);
            if (startBit >= 0 && endBit >= 0) {
                FieldInfo *fieldInfo = fieldInfoCreate((char *)fieldName, arrayIndex, startBit, endBit, (char *)fieldCheckList, paramInfo);
                HASH_ADD_KEYPTR(hh, paramInfo->fieldInfoHash, fieldInfo->name, strlen(fieldInfo->name), fieldInfo);
                DEBUG_LOG("Add FieldInfo to fieldInfoHash hash. (name=%s, arrayIndex="APP_SIZE_T_FT", bit[%d,%d], check_list = %s, hash size="APP_SIZE_T_FT")\n", fieldInfo->name, arrayIndex, fieldInfo->startBit, fieldInfo->endBit, fieldCheckList, (size_t)paramInfoGetNumOfFieldInfo(paramInfo));
            } else {
                ERR_LOG("Cannot create field information. (name=%s, arrayIndex="APP_SIZE_T_FT", startBit=%d, endBit=%d, fieldCheckList=%s)\n", fieldName, arrayIndex, startBit, endBit, fieldCheckList);
            }

            xmlFree(fieldName);
            xmlFree(fieldArrayIndex);
            xmlFree(fieldBit);
            xmlFree(fieldCheckList);
        }

        xmlFree(paramName);
        xmlFree(paramTypeStr);
    }

    return APP_NO_ERROR;
}

EXPORT ParamTree *paramTreeCreate(int paramId, const char *categoryPath) {
    ParamTree *paramTree = (ParamTree *)malloc(sizeof(ParamTree));
    if (!paramTree) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    paramTree->paramId = paramId;
    paramTree->categoryPath = strdup(categoryPath ? categoryPath : "");
    return paramTree;
}

EXPORT void paramTreeRelease(ParamTree *paramTree) {
    if (paramTree) {
        if (paramTree->categoryPath) {
            free(paramTree->categoryPath);
        }
        free(paramTree);
    }
}

EXPORT size_t audioTypeGetNumOfParamTree(AudioType *audioType) {
    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return 0;
    }

    return HASH_COUNT(audioType->paramTreeHash);
}

EXPORT size_t audioTypeGetNumOfParamInfo(AudioType *audioType) {
    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return 0;
    }

    return HASH_COUNT(audioType->paramInfoHash);
}

EXPORT ParamInfo *audioTypeGetParamInfoByIndex(AudioType *audioType, size_t index) {
    ParamInfo *paramInfo = NULL;
    size_t i = 0;

    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return NULL;
    }

    for (paramInfo = audioType->paramInfoHash; paramInfo ; paramInfo = paramInfo->hh.next) {
        if (index == i++) {
            return paramInfo;
        }
    }

    return NULL;
}

EXPORT ParamInfo *audioTypeGetParamInfoByName(AudioType *audioType, const char *paramName) {
    ParamInfo *paramInfo;

    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return NULL;
    }

    if (!paramName) {
        ERR_LOG("AudioType = %s, paramName is NULL\n", audioType->name);
        return NULL;
    }

    /* Query Param name */
    HASH_FIND_STR(audioType->paramInfoHash, paramName, paramInfo);

    return paramInfo;
}

EXPORT size_t audioTypeGetNumOfCategoryType(AudioType *audioType) {
    return HASH_COUNT(audioType->categoryTypeHash);
}

EXPORT CategoryType *audioTypeGetCategoryTypeByIndex(AudioType *audioType, size_t index) {
    CategoryType *categoryType = NULL;
    size_t i = 0;

    //INFO_LOG("audioType = %s, index = %d\n", audioType ? audioType->name : "NULL", index);

    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return NULL;
    }

    for (categoryType = audioType->categoryTypeHash; categoryType ; categoryType = categoryType->hh.next) {
        if (index == i++) {
            return categoryType;
        }
    }

    return NULL;
}

EXPORT CategoryType *audioTypeGetCategoryTypeByName(AudioType *audioType, const char *categoryTypeName) {
    CategoryType *categoryType = NULL;

    //INFO_LOG("audioType = %s, name = %s\n", audioType ? audioType->name : "NULL", categoryTypeName);

    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return NULL;
    }

    for (categoryType = audioType->categoryTypeHash; categoryType ; categoryType = categoryType->hh.next) {
        if (!strncmp(categoryType->name, categoryTypeName, strlen(categoryTypeName) + 1)) {
            return categoryType;
        }
    }

    return NULL;
}

EXPORT CategoryType *audioTypeGetCategoryTypeByWording(AudioType *audioType, const char *categoryTypeWording) {
    CategoryType *categoryType;

    //INFO_LOG("audioType = %s, categoryTypeWording = %s\n", audioType ? audioType->name : "NULL", categoryTypeWording);

    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return NULL;
    }

    /* Query Param name */
    HASH_FIND_STR(audioType->categoryTypeHash, categoryTypeWording, categoryType);

    return categoryType;
}

EXPORT APP_STATUS audioTypeSetParamData(AudioType *audioType, const char *fullCategoryPath, ParamInfo *paramInfo, void *paramData, int arraySize) {
    ParamTree *paramTree = NULL;
    UT_string *categoryPath = NULL;
    char *paramDataStr;

    INFO_LOG("+++\n");
    if (!audioType) {
        ERR_LOG("---audioType is NULL\n");
        return APP_ERROR;
    }

    if (!fullCategoryPath) {
        ERR_LOG("---categoryPath is NULL\n");
        return APP_ERROR;
    }

    if (!paramInfo) {
        ERR_LOG("---paramInfo is NULL\n");
        return APP_ERROR;
    }

    categoryPath = utilNormalizeCategoryPathForAudioType(fullCategoryPath, audioType);
    if (!categoryPath) {
        ERR_LOG("---Cannot normalize categoryPath for %s AudioType. (path = %s)\n", audioType->name, fullCategoryPath);
        return APP_ERROR;
    }

    paramDataStr = utilConvDataToString(paramInfo->dataType, paramData, arraySize, 1);
    INFO_LOG("audioType = %s, fullCategoryPath = %s, param = %s, paramData = 0x%p, arraySize = %d\n", audioType->name, fullCategoryPath, paramInfo->name, paramData, arraySize);
    INFO_LOG("paramData = %s\n", paramDataStr);
    free(paramDataStr);

    /* Write lock */
    audioTypeWriteLock(audioType, __FUNCTION__);

    /* Check the target paramUnit status (category path exactly match? ref count?) */
    HASH_FIND_STR(audioType->paramTreeHash, utstring_body(categoryPath), paramTree);
    if (paramTree) {
        /* Category path exactly match */
        ParamUnit *paramUnit = NULL;
        HASH_FIND_INT(audioType->paramUnitHash, &paramTree->paramId, paramUnit);

        if (!paramUnit) {
            ERR_LOG("---Cannot find target paramUnit for paramId %d for %s AudioType. (path = %s)\n", paramTree->paramId, audioType->name, utstring_body(categoryPath));
            /* Unlock */
            audioTypeUnlock(audioType);
            utstring_free(categoryPath);
            return APP_ERROR;
        }

        if (paramUnit->refCount == 1) {
            /* ParamUnit only used by itself, update param data directly */
            Param *param = paramUnitGetParamByName(paramUnit, paramInfo->name);
            if (param) {
                void *oldData = param->data;    // Keep old data ptr & free it after new data settled.
                paramSetupDataInfoByVal(param, paramData, arraySize);
                free(oldData);
                DEBUG_LOG("Find the ParamUnit by full category path. (paramId = %d, refCount = %d, categoryPath = %s)\n", paramTree->paramId, paramUnit->refCount, paramTree->categoryPath);
            }
        } else {
            /* ParamUnit not only used by itself, new ParamUnit */
            Param *param;

            /* New paramUnit */
            ParamUnit *newParamUnit = paramUnitClone(paramUnit);
            newParamUnit->refCount = 1;
            newParamUnit->paramId = utilFindUnusedParamId(audioType);
            HASH_ADD_INT(audioType->paramUnitHash, paramId, newParamUnit);

            /* Update the paramId of ParamTree */
            paramTree->paramId = newParamUnit->paramId;

            /* Old ParamUnit ref count -1 */
            paramUnit->refCount--;

            /* Update new ParamUnit param */
            param = paramUnitGetParamByName(newParamUnit, paramInfo->name);
            if (param) {
                free(param->data);
            }
            paramSetupDataInfoByVal(param, paramData, arraySize);
            DEBUG_LOG("Find the ParamUnit by full category path.(ref count = %d, new ParamUnit id = %d, categoryPath = %s)\n", paramUnit->refCount, newParamUnit->paramId, paramTree->categoryPath);
        }
    } else {
        /* No exactly match ParamUnit, create new ParamUnit & ParamTree */
        /* New paramUnit */
        Param *param;
        ParamUnit *paramUnit = audioTypeGetParamUnit(audioType, fullCategoryPath);
        ParamUnit *newParamUnit = paramUnitClone(paramUnit);

        if (!paramUnit) {
            ERR_LOG("---paramUnit is NULL\n");
            audioTypeUnlock(audioType);
            utstring_free(categoryPath);
            return APP_NO_ERROR;
        }

        newParamUnit->refCount = 1;
        newParamUnit->paramId = utilFindUnusedParamId(audioType);
        HASH_ADD_INT(audioType->paramUnitHash, paramId, newParamUnit);

        /* New ParamTree */
        paramTree = paramTreeCreate(newParamUnit->paramId, utstring_body(categoryPath));
        HASH_ADD_KEYPTR(hh, audioType->paramTreeHash, paramTree->categoryPath, strlen(paramTree->categoryPath), paramTree);

        /* Update new ParamUnit param */
        param = paramUnitGetParamByName(newParamUnit, paramInfo->name);
        if (param) {
            free(param->data);
        }
        paramSetupDataInfoByVal(param, paramData, arraySize);

        DEBUG_LOG("Not found the match paramTree, new paramUnit & paramTree (paramId = %d, categoryPath = %s)\n", newParamUnit->paramId, utstring_body(categoryPath));
    }

    /* Param data modified, change the dirty info */
    audioType->dirty = 1;

    /* Unlock */
    audioTypeUnlock(audioType);

    utstring_free(categoryPath);
    INFO_LOG("---\n");
    return APP_NO_ERROR;
}

EXPORT void categoryPathRetrievedHandler(const char *categoryPath, const char *fullCategoryPath, CategoryPathRetrievedParam *categoryPathRetrievedParam) {
    /* Write category path's param unit to XML */
    ParamUnit *paramUnit = audioTypeGetParamUnit(categoryPathRetrievedParam->audioType, fullCategoryPath);
    UT_string *paramId;
    Param *param;
    xmlNodePtr paramNode, paramUnitNode;

    utstring_new(paramId);
    utstring_printf(paramId, APP_SIZE_T_FT, categoryPathRetrievedParam->paramIdCounter);

    paramNode = xmlNewChild(categoryPathRetrievedParam->paramTreeNode, NULL, BAD_CAST ELEM_PARAM, NULL);
    xmlNewProp(paramNode, BAD_CAST ATTRI_PATH, BAD_CAST categoryPath);
    xmlNewProp(paramNode, BAD_CAST ATTRI_PARAM_ID, BAD_CAST utstring_body(paramId));

    if (paramUnit) {
        /* add ParamUnitPool */
        paramUnitNode = xmlNewChild(categoryPathRetrievedParam->paramUnitPoolNode, NULL, BAD_CAST ELEM_PARAM_UNIT, NULL);
        xmlNewProp(paramUnitNode, BAD_CAST ATTRI_PARAM_ID, BAD_CAST utstring_body(paramId));
        /* Add category path information */
        xmlNewProp(paramUnitNode, BAD_CAST ATTRI_PATH_DESC, BAD_CAST categoryPath);

        /* Add Param */
        for (param = paramUnit->paramHash; param != NULL; param = param->hh.next) {
            char *data = paramNewDataStrWithMode(param, categoryPathRetrievedParam->audioType->appHandle->saveXmlWithHexMode);
            paramNode = xmlNewChild(paramUnitNode, NULL, BAD_CAST ELEM_PARAM, NULL);
            xmlNewProp(paramNode, BAD_CAST ATTRI_NAME, BAD_CAST param->name);
            xmlNewProp(paramNode, BAD_CAST ATTRI_VALUE, BAD_CAST data);
            free(data);
        }
    }

    utstring_free(paramId);
    categoryPathRetrievedParam->paramIdCounter++;
}

EXPORT APP_STATUS retrieveAllCategoryPath(AudioType *audioType, size_t curCategoryTypeIndex, size_t numOfCategoryType, size_t curCategoryIndex, const char *preCategoryPath, const char *preFullCategoryPath, CATEGORY_PATH_RETRIEVED_CB_FUN categoryPathRetrievedHandler, CategoryPathRetrievedParam *categoryPathRetrievedParam) {
    CategoryType *categoryType;
    size_t numOfAllCategory;
    Category *category;
    UT_string *categoryPath = NULL;
    UT_string *fullCategoryPath = NULL;

    if (curCategoryTypeIndex >= numOfCategoryType) {
        //printf("%s\n", prePathStr);
        categoryPathRetrievedHandler(preCategoryPath, preFullCategoryPath, categoryPathRetrievedParam);
        return APP_NO_ERROR;
    }

    // Check category is valid
    categoryType = audioTypeGetCategoryTypeByIndex(audioType, curCategoryTypeIndex);
    numOfAllCategory = categoryTypeGetNumOfAllCategory(categoryType);
    if (curCategoryIndex >= numOfAllCategory) {
        return APP_NO_ERROR;
    }

    // category type & category are valid
    utstring_new(categoryPath);
    utstring_new(fullCategoryPath);
    category = categoryTypeGetAllCategoryByIndex(categoryType, curCategoryIndex);
    if (!strncmp(preCategoryPath, "", strlen("") + 1)) {
        utstring_printf(categoryPath, "%s", category->name);
        utstring_printf(fullCategoryPath, "%s,%s", categoryType->name, category->name);
    } else {
        utstring_printf(categoryPath, "%s,%s", preCategoryPath, category->name);
        utstring_printf(fullCategoryPath, "%s,%s,%s", preFullCategoryPath, categoryType->name, category->name);
    }

    // Retrieve next CategoryType
    retrieveAllCategoryPath(audioType, curCategoryTypeIndex + 1, numOfCategoryType, 0, utstring_body(categoryPath), utstring_body(fullCategoryPath), categoryPathRetrievedHandler, categoryPathRetrievedParam);

    // Retrieve next Category
    retrieveAllCategoryPath(audioType, curCategoryTypeIndex, numOfCategoryType, curCategoryIndex + 1, preCategoryPath, preFullCategoryPath, categoryPathRetrievedHandler, categoryPathRetrievedParam);

    utstring_free(fullCategoryPath);
    utstring_free(categoryPath);
    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeSetupAudioParamNode(AudioType *audioType, xmlNodePtr audioParamNode) {
    xmlNodePtr paramTreeNode = NULL, paramNode = NULL, paramUnitPoolNode = NULL, paramUnitNode = NULL;
    AppHandle *appHandle = audioType->appHandle;
    ParamTree *paramTreeItem;
    ParamUnit *paramUnitItem;
    Param *paramItem;
    UT_string *str;

    utstring_new(str);

    if (appHandle->normalizeXmlContent == 1) {
        size_t numOfCategoryType = audioTypeGetNumOfCategoryType(audioType);
        CategoryPathRetrievedParam categoryPathRetrievedParam;

        /* Create paramTree & paramUnit node */
        paramTreeNode = xmlNewChild(audioParamNode, NULL, BAD_CAST ELEM_PARAM_TREE, NULL);
        paramUnitPoolNode = xmlNewChild(audioParamNode, NULL, BAD_CAST ELEM_PARAM_UNIT_POOL, NULL);

        /* Setup callback parameters */
        categoryPathRetrievedParam.paramTreeNode = paramTreeNode;
        categoryPathRetrievedParam.paramUnitPoolNode = paramUnitPoolNode;
        categoryPathRetrievedParam.paramIdCounter = 0;

        /* Setup callback parameters */
        categoryPathRetrievedParam.audioType = audioType;

        /* Add default category path("") */
        categoryPathRetrievedHandler("", "", &categoryPathRetrievedParam);

        /* Enum all category path */
        retrieveAllCategoryPath(audioType, 0, numOfCategoryType, 0, "", "", categoryPathRetrievedHandler, &categoryPathRetrievedParam);
    } else {
        /* add ParamTree & Param */
        paramTreeNode = xmlNewChild(audioParamNode, NULL, BAD_CAST ELEM_PARAM_TREE, NULL);
        for (paramTreeItem = audioType->paramTreeHash; paramTreeItem != NULL; paramTreeItem = paramTreeItem->hh.next) {
            paramNode = xmlNewChild(paramTreeNode, NULL, BAD_CAST ELEM_PARAM, NULL);
            utstring_printf(str, "%d", paramTreeItem->paramId);
            xmlNewProp(paramNode, BAD_CAST ATTRI_PATH, BAD_CAST paramTreeItem->categoryPath);
            xmlNewProp(paramNode, BAD_CAST ATTRI_PARAM_ID, BAD_CAST utstring_body(str));
            utstring_clear(str);
        }

        /* add ParamUnitPool */
        paramUnitPoolNode = xmlNewChild(audioParamNode, NULL, BAD_CAST ELEM_PARAM_UNIT_POOL, NULL);
        for (paramUnitItem = audioType->paramUnitHash; paramUnitItem != NULL; paramUnitItem = paramUnitItem->hh.next) {
            paramUnitNode = xmlNewChild(paramUnitPoolNode, NULL, BAD_CAST ELEM_PARAM_UNIT, NULL);
            utstring_printf(str, "%d", paramUnitItem->paramId);
            xmlNewProp(paramUnitNode, BAD_CAST ATTRI_PARAM_ID, BAD_CAST utstring_body(str));
            utstring_clear(str);

            /* Add Param */
            for (paramItem = paramUnitItem->paramHash; paramItem != NULL; paramItem = paramItem->hh.next) {
                char *data = paramNewDataStrWithMode(paramItem, audioType->appHandle->saveXmlWithHexMode);
                paramNode = xmlNewChild(paramUnitNode, NULL, BAD_CAST ELEM_PARAM, NULL);
                xmlNewProp(paramNode, BAD_CAST ATTRI_NAME, BAD_CAST paramItem->name);
                xmlNewProp(paramNode, BAD_CAST ATTRI_VALUE, BAD_CAST data);
                free(data);
            }
        }
    }

    utstring_free(str);
    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeSaveAudioParamXml(AudioType *audioType, const char *saveDir, int clearDirtyBit) {
    xmlDocPtr doc = NULL;
    xmlNodePtr audioParamNode = NULL;

    UT_string *filePath, *str;
    APP_STATUS result = APP_NO_ERROR;

    INFO_LOG("audioType = %s, saveDir = %s\n", audioType ? audioType->name : "NULL", saveDir);

    if (!audioType) {
        ERR_LOG("audioType is NULL\n");
        return APP_ERROR;
    }

    if (!saveDir) {
        ERR_LOG("saveDir is NULL\n");
        return APP_ERROR;
    }

    utstring_new(filePath);
    utstring_new(str);
    doc = xmlNewDoc(BAD_CAST "1.0");

    /* Read lock */
    audioTypeReadLock(audioType, __FUNCTION__);

    audioParamNode = xmlNewNode(NULL, BAD_CAST ELEM_AUDIO_PARAM);
    utstring_printf(str, "%d.%d", audioType->audioParamVerMaj, audioType->audioParamVerMin);
    xmlNewProp(audioParamNode, BAD_CAST ATTRI_VERSION, BAD_CAST utstring_body(str));
    utstring_clear(str);
    xmlDocSetRootElement(doc, audioParamNode);

    audioTypeSetupAudioParamNode(audioType, audioParamNode);

    utilMkdir(saveDir);

    utstring_printf(filePath, "%s"FOLDER"%s"AUDIO_PARAM_XML_POSFIX, saveDir, audioType->name);
    if (xmlSaveFormatFileEnc(utstring_body(filePath), doc, "UTF-8", 1) == -1) {
        result = APP_ERROR;
    } else {
        if (clearDirtyBit) {
            audioType->dirty = 0;
        }
    }

    /* Unlock */
    audioTypeUnlock(audioType);
    xmlFreeDoc(doc);
    utstring_free(str);
    utstring_free(filePath);

    return result;
}

EXPORT APP_STATUS audioTypeSetFieldData(AudioType *audioType, const char *fullCategoryPath, FieldInfo *fieldInfo, unsigned int val) {
    ParamTree *paramTree = NULL;
    UT_string *categoryPath = NULL;

    INFO_LOG("+++\n");
    if (!audioType) {
        ERR_LOG("---audioType is NULL\n");
        return APP_ERROR;
    }

    if (!fullCategoryPath) {
        ERR_LOG("---categoryPath is NULL\n");
        return APP_ERROR;
    }

    if (!fieldInfo) {
        ERR_LOG("---fieldInfo is NULL\n");
        return APP_ERROR;
    }

    INFO_LOG("audioType = %s, categoryPath = %s, field = %s, value = 0x%x\n", audioType->name, fullCategoryPath, fieldInfo->name, val);

    /* Write lock */
    audioTypeWriteLock(audioType, __FUNCTION__);

    categoryPath = utilNormalizeCategoryPathForAudioType(fullCategoryPath, audioType);
    if (!categoryPath) {
        ERR_LOG("---Cannot normalize categoryPath for %s AudioType. (path = %s)\n", audioType->name, fullCategoryPath);
        /* Unlock */
        audioTypeUnlock(audioType);
        return APP_ERROR;
    }

    /* Check the target paramUnit status (category path exactly match? ref count?) */
    HASH_FIND_STR(audioType->paramTreeHash, utstring_body(categoryPath), paramTree);
    if (paramTree) {
        /* Category path exactly match */
        ParamUnit *paramUnit = NULL;
        HASH_FIND_INT(audioType->paramUnitHash, &paramTree->paramId, paramUnit);

        if (!paramUnit) {
            ERR_LOG("---Cannot find target paramUnit for paramId %d for %s AudioType. (path = %s)\n", paramTree->paramId, audioType->name, fullCategoryPath);
            utstring_free(categoryPath);
            /* Unlock */
            audioTypeUnlock(audioType);
            return APP_ERROR;
        }

        if (paramUnit->refCount == 1) {
            /* ParamUnit only used by itself, update field data directly */
            Param *param = paramUnitGetParamByName(paramUnit, fieldInfo->paramInfo->name);
            paramSetFieldVal(param, fieldInfo, val);
            DEBUG_LOG("Find the ParamUnit by full category path. (paramId = %d, refCount = %d, categoryPath = %s)\n", paramTree->paramId, paramUnit->refCount, paramTree->categoryPath);

            if (param && appDebugLevel <= DEBUG_LEVEL) {
                utilShowParamValue(param);
            }
        } else {
            /* ParamUnit not only used by itself, new ParamUnit */
            Param *param;

            /* New paramUnit */
            ParamUnit *newParamUnit = paramUnitClone(paramUnit);
            newParamUnit->refCount = 1;
            newParamUnit->paramId = utilFindUnusedParamId(audioType);
            HASH_ADD_INT(audioType->paramUnitHash, paramId, newParamUnit);

            /* Update the paramId of ParamTree */
            paramTree->paramId = newParamUnit->paramId;

            /* Old ParamUnit ref count -1 */
            paramUnit->refCount--;

            /* Update new ParamUnit field */
            param = paramUnitGetParamByName(newParamUnit, fieldInfo->paramInfo->name);
            paramSetFieldVal(param, fieldInfo, val);
            DEBUG_LOG("Find the ParamUnit by full category path.(ref count = %d, new ParamUnit id = %d, categoryPath = %s)\n", paramUnit->refCount, newParamUnit->paramId, paramTree->categoryPath);

            if (param && appDebugLevel <= DEBUG_LEVEL) {
                utilShowParamValue(param);
            }
        }
    } else {
        /* No exactly match ParamUnit, create new ParamUnit & ParamTree */
        /* New paramUnit */
        Param *param;
        ParamUnit *paramUnit = audioTypeGetParamUnit(audioType, fullCategoryPath);
        ParamUnit *newParamUnit;
        if (!paramUnit) {
            ERR_LOG("---Cannot find the param unit (category path = %s)\n", fullCategoryPath);
            utstring_free(categoryPath);
            /* Unlock */
            audioTypeUnlock(audioType);
            return APP_ERROR;
        }
        newParamUnit = paramUnitClone(paramUnit);
        newParamUnit->refCount = 1;
        newParamUnit->paramId = utilFindUnusedParamId(audioType);
        HASH_ADD_INT(audioType->paramUnitHash, paramId, newParamUnit);

        /* New ParamTree */
        paramTree = paramTreeCreate(newParamUnit->paramId, utstring_body(categoryPath));
        HASH_ADD_KEYPTR(hh, audioType->paramTreeHash, paramTree->categoryPath, strlen(paramTree->categoryPath), paramTree);

        /* Update new ParamUnit field */
        param = paramUnitGetParamByName(newParamUnit, fieldInfo->paramInfo->name);
        paramSetFieldVal(param, fieldInfo, val);
        DEBUG_LOG("Not found the match paramTree, new paramUnit & paramTree (paramId = %d, categoryPath = %s)\n", newParamUnit->paramId, utstring_body(categoryPath));

        if (param && appDebugLevel <= DEBUG_LEVEL) {
            utilShowParamValue(param);
        }
    }

    /* Param data modified, change the dirty info */
    audioType->dirty = 1;

    /* Unlock */
    audioTypeUnlock(audioType);

    utstring_free(categoryPath);
    INFO_LOG("---\n");
    return APP_NO_ERROR;
}

EXPORT APP_STATUS audioTypeParamUnitCopy(AudioType *audioType, const char *srcCategoryPath, const char *dstCategoryPath) {
    ParamTree *paramTree;
    ParamUnit *dstParamUnit = NULL;
    UT_string *searchPath;
    ParamUnit *srcParamUnit;

    INFO_LOG("+++audioType = %s, srcCategoryPath = %s, dstCategoryPath = %s\n", audioType ? audioType->name : "NULL", srcCategoryPath, dstCategoryPath);

    if (utilCompNormalizeCategoryPath(audioType, srcCategoryPath, dstCategoryPath) == 1) {
        WARN_LOG("---srcCategoryPath == dstCategoryPath, ignore it (%s)\n", srcCategoryPath);
        return APP_NO_ERROR;
    }

    /* Write lock */
    audioTypeWriteLock(audioType, __FUNCTION__);

    srcParamUnit = audioTypeGetParamUnit(audioType, srcCategoryPath);
    if (!srcParamUnit) {
        ERR_LOG("---Cannot find src param unit DONOT copy ParamUnit! (audioType=%s, category=%s)\n", audioType->name, srcCategoryPath);
        /* Unlock */
        audioTypeUnlock(audioType);
        return APP_ERROR;
    }

    searchPath = utilNormalizeCategoryPathForAudioType(dstCategoryPath, audioType);
    if (!searchPath) {
        ERR_LOG("---Cannot normalize categoryPath for %s AudioType. DONOT copy ParamUnit. (path = %s)\n", audioType->name, dstCategoryPath);
        /* Unlock */
        audioTypeUnlock(audioType);
        utstring_free(searchPath);
        return APP_ERROR;
    }

    HASH_FIND_STR(audioType->paramTreeHash, utstring_body(searchPath), paramTree);
    if (paramTree) {
        /* Exactly match ParamUnit found */
        HASH_FIND_INT(audioType->paramUnitHash, &paramTree->paramId, dstParamUnit);
        if (!dstParamUnit) {
            ERR_LOG("---Cannot find the dstParamUnit! DONOT copy ParamUnit. (%s -> %d -> NULL)\n", utstring_body(searchPath), paramTree->paramId);
            /* Unlock */
            audioTypeUnlock(audioType);
            utstring_free(searchPath);
            return APP_ERROR;
        }

        if (dstParamUnit->refCount == 1) {
            /* Remove this param unit */
            HASH_DEL(audioType->paramUnitHash, dstParamUnit);
            paramUnitRelease(dstParamUnit);
            dstParamUnit = NULL;
        } else {
            /* Update original ParamUnit ref count */
            dstParamUnit->refCount--;
            //INFO_LOG("dstParamUnit refCount = %d\n", dstParamUnit->refCount);
        }

        /* Update the paramTree's paramId info */
        paramTree->paramId = srcParamUnit->paramId;
    } else {
        /* Add new ParamTree and refer to the srcParamUnit id */
        paramTree = paramTreeCreate(srcParamUnit->paramId, utstring_body(searchPath));
        HASH_ADD_KEYPTR(hh, audioType->paramTreeHash, paramTree->categoryPath, strlen(paramTree->categoryPath), paramTree);
    }

    /* Update src param unit ref count */
    srcParamUnit->refCount++;
    //INFO_LOG("srcParamUnit refCount = %d\n", srcParamUnit->refCount);

    /* Param data modified, change the dirty info */
    audioType->dirty = 1;
    INFO_LOG("---  srcParamUnit = dstParamUnit(id = %d, ref count = %d), old dstParamUnit(ref count = %d)\n", srcParamUnit->paramId, srcParamUnit->refCount, dstParamUnit ? dstParamUnit->refCount : 0);

    /* Unlock */
    audioTypeUnlock(audioType);

    utstring_free(searchPath);
    return APP_NO_ERROR;
}

EXPORT xmlNode *audioTypeGetCategoryTypeListNode(AudioType *audioType) {
    xmlNode *root_element, *node;

    root_element = xmlDocGetRootElement(audioType->paramUnitDescDoc);
    if (!root_element) {
        WARN_LOG("No root element!\n");
        return NULL;
    }

    node = findXmlNodeByElemName(root_element, ELEM_PARAM_UNIT_DESC);
    if (!node) {
        WARN_LOG("No param unit desc node!\n");
        return NULL;
    }

    node = findXmlNodeByElemName(node->children, ELEM_CATEGORY_TYPE_LIST);
    if (!node) {
        WARN_LOG("No category type list node!\n");
        return NULL;
    }

    return node;
}

EXPORT xmlNode *audioTypeGetParamUnitNode(AudioType *audioType) {
    xmlNode *root_element, *node;

    root_element = xmlDocGetRootElement(audioType->paramUnitDescDoc);
    if (!root_element) {
        WARN_LOG("No root element!\n");
        return NULL;
    }

    node = findXmlNodeByElemName(root_element, ELEM_PARAM_UNIT_DESC);
    if (!node) {
        WARN_LOG("No param unit desc node!\n");
        return NULL;
    }

    node = findXmlNodeByElemName(node->children, ELEM_PARAM_UNIT);
    if (!node) {
        WARN_LOG("No param unit node!\n");
        return NULL;
    }

    return node;
}

EXPORT xmlNode *audioTypeGetParamUnitDescNode(AudioType *audioType) {
    xmlNode *root_element, *node;

    root_element = xmlDocGetRootElement(audioType->paramUnitDescDoc);
    if (!root_element) {
        WARN_LOG("No root element!\n");
        return NULL;
    }

    node = findXmlNodeByElemName(root_element, ELEM_PARAM_UNIT_DESC);
    if (!node) {
        WARN_LOG("No param unit desc node!\n");
        return NULL;
    }

    return node;
}

EXPORT TreeRoot *audioTypeGetTreeRoot(AudioType *audioType, const char *treeRootName) {
    TreeRoot *treeRoot = NULL;

    INFO_LOG("audioType = %s, treeType = %s\n", audioType ? audioType->name : "NULL", treeRootName);

    if (!audioType) {
        ERR_LOG("AudioType is NULL!\n");
        return NULL;
    }

    if (!treeRootName) {
        ERR_LOG("treeRootName is NULL!\n");
        return NULL;
    }

    if (audioType->paramTreeView) {
        HASH_FIND_STR(audioType->paramTreeView->treeRootHash, treeRootName, treeRoot);
    } else {
        ERR_LOG("paramTreeView is NULL\n");
    }

    return treeRoot;
}

EXPORT APP_STATUS audioTypeValidCategoryGroupName(AudioType *audioType, const char *name) {
    CategoryType *categoryType;
    CategoryGroup *categoryGroup;

    if (audioTypeIsHardCategoryGroup(audioType, name)) {
        return APP_NO_ERROR;
    }

    for (categoryType = audioType->categoryTypeHash; categoryType; categoryType = categoryType->hh.next) {
        HASH_FIND_STR(categoryType->categoryGroupHash, name, categoryGroup);
        if (categoryGroup) {
            return APP_NO_ERROR;
        }
    }

    return APP_ERROR;
}

EXPORT int audioTypeIsHardCategoryGroup(AudioType *audioType, const char *categoryName) {
    int arrayIndex;

    if (audioType == NULL || categoryName == NULL) {
        return 0;
    }

    for (arrayIndex = 0; HARD_CATEGORY_GROUP[arrayIndex][0]; arrayIndex++) {
        if (!strncmp(HARD_CATEGORY_GROUP[arrayIndex][0], audioType->name, strlen(audioType->name) + 1)
            && !strncmp(HARD_CATEGORY_GROUP[arrayIndex][2], categoryName, strlen(categoryName) + 1)) {
            return 1;
        }
    }

    return 0;
}
