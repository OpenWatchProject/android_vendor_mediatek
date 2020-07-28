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
 *   Implement ParamTreeView related APIs
 */

#include "AudioParamParserPriv.h"

EXPORT ParamTreeView *paramTreeViewCreate(AudioType *audioType, int verMaj, int verMin) {
    ParamTreeView *paramTreeView = malloc(sizeof(ParamTreeView));
    if (!paramTreeView) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    paramTreeView->audioType = audioType;
    paramTreeView->verMaj = verMaj;
    paramTreeView->verMin = verMin;
    paramTreeView->treeRootHash = NULL;
    return paramTreeView;
}

EXPORT void paramTreeViewRelease(ParamTreeView *paramTreeView) {
    if (paramTreeView) {
        if (paramTreeView->treeRootHash) {
            TreeRoot *tmp, *item;
            HASH_ITER(hh, paramTreeView->treeRootHash, item, tmp) {
                if (paramTreeView->treeRootHash) {
                    HASH_DEL(paramTreeView->treeRootHash, item);
                    treeRootRelease(item);
                }
            }
        }
        free(paramTreeView);
    }
}

EXPORT TreeRoot *treeRootCreate(const char *name, xmlNode *treeRootNode, ParamTreeView *paramTreeView) {
    TreeRoot *treeRoot = malloc(sizeof(TreeRoot));
    if (!treeRoot) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    treeRoot->name = strdup(name);
    treeRoot->treeRootNode = treeRootNode;
    treeRoot->paramTreeView = paramTreeView;
    treeRoot->featureHash = NULL;
    treeRoot->switchFieldInfo = NULL;
    return treeRoot;
}

EXPORT void treeRootRelease(TreeRoot *treeRoot) {
    if (treeRoot) {
        if (treeRoot->featureHash) {
            Feature *tmp, *item;
            HASH_ITER(hh, treeRoot->featureHash, item, tmp) {
                if (treeRoot->featureHash) {
                    HASH_DEL(treeRoot->featureHash, item);
                    featureRelease(item);
                }
            }
        }

        free(treeRoot->name);
        free(treeRoot);
    }
}

EXPORT Feature *featureCreate(const char *name, AudioType *audioType, FieldInfo *switchFieldInfo, const char *featureOption) {
    Feature *feature = malloc(sizeof(Feature));
    if (!feature) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    feature->name = strdup(name);
    feature->audioType = audioType;

    if (featureOption) {
        feature->featureOption = strdup(featureOption);
    } else {
        feature->featureOption = NULL;
    }

    feature->categoryPathHash = NULL;
    feature->featureFieldHash = NULL;
    feature->switchFieldInfo = switchFieldInfo;
    return feature;
}

EXPORT void featureRelease(Feature *feature) {
    if (feature) {
        if (feature->categoryPathHash) {
            CategoryPath *tmp, *item;
            HASH_ITER(hh, feature->categoryPathHash, item, tmp) {
                if (feature->categoryPathHash) {
                    HASH_DEL(feature->categoryPathHash, item);
                    categoryPathRelease(item);
                }
            }
        }

        if (feature->featureFieldHash) {
            FeatureField *tmp, *item;
            HASH_ITER(hh, feature->featureFieldHash, item, tmp) {
                if (feature->featureFieldHash) {
                    HASH_DEL(feature->featureFieldHash, item);
                    featureFieldRelease(item);
                }
            }
        }

        if (feature->name) {
            free(feature->name);
        }

        if (feature->featureOption) {
            free(feature->featureOption);
        }

        free(feature);
    }
}

EXPORT APP_STATUS categoryPathValidation(CategoryPath *categoryPath) {
    char *path;
    char *categoryGroup;
    char *restOfStr = NULL;

    if (!strncmp(categoryPath->path, "", strlen("") + 1)) {
        return APP_NO_ERROR;
    }

    path = strdup(categoryPath->path);
    categoryGroup = utilStrtok(path, ARRAY_SEPERATOR, &restOfStr);
    if (categoryGroup == NULL || audioTypeValidCategoryGroupName(categoryPath->feature->audioType, categoryGroup) == APP_ERROR) {
        free(path);
        return APP_ERROR;
    }

    while ((categoryGroup = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr)) != NULL) {
        if (audioTypeValidCategoryGroupName(categoryPath->feature->audioType, categoryGroup) == APP_ERROR) {
            free(path);
            return APP_ERROR;
        }
    }

    free(path);
    return APP_NO_ERROR;
}

EXPORT CategoryPath *categoryPathCreate(Feature *feature, const char *path) {
    CategoryPath *categoryPath = malloc(sizeof(CategoryPath));
    if (!categoryPath) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    categoryPath->path = strdup(path);
    categoryPath->feature = feature;

#ifdef WIN32
    /* The category path validation only run on win32 */
    if (categoryPathValidation(categoryPath) == APP_ERROR) {
        ERR_LOG("The %s feature's category path is not belong to categoryGroup! (%s)\n", feature->name, categoryPath->path);
        categoryPathRelease(categoryPath);
        return NULL;
    }
#endif

    return categoryPath;
}

EXPORT void categoryPathRelease(CategoryPath *categoryPath) {
    if (categoryPath) {
        free(categoryPath->path);
        free(categoryPath);
    }
}

EXPORT FeatureField *featureFieldCreate(FieldInfo *fieldInfo) {
    FeatureField *featureField = malloc(sizeof(FeatureField));
    if (!featureField) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    featureField->fieldInfo = fieldInfo;

    return featureField;
}

EXPORT void featureFieldRelease(FeatureField *featureField) {
    if (featureField) {
        free(featureField);
    }
}

EXPORT Feature *treeRootGetFeatureByName(TreeRoot *treeRoot, const char *featureName) {
    Feature *feature;
    HASH_FIND_STR(treeRoot->featureHash, featureName, feature);
    return feature;
}

CategoryPath *findFeatureCategoryPath(char **arr, int *Switch, int n, Feature *feature) {
    CategoryPath *categoryPath = NULL;
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

    /* Find the categoryPath */
    HASH_FIND_STR(feature->categoryPathHash, utstring_body(path), categoryPath);
    DEBUG_LOG("Search path = %s, paramTree = 0x%p\n", utstring_body(path), categoryPath);

    utstring_free(path);
    return categoryPath;
}

CategoryPath *fuzzySearchFeatureCategoryPath(char **arr, int totalSize, int pickSize, Feature *feature) {
    CategoryPath *categoryPath = NULL;
    int i, j, pos = pickSize - 1;
    int *swpArray;

    if (pickSize > totalSize) {
        return categoryPath;
    }

    swpArray = (int *)malloc(sizeof(int) * totalSize);
    if (!swpArray) {
        ERR_LOG("malloc fail!\n");
        return NULL;
    }

    for (i = 0; i < totalSize; ++i) {
        swpArray[i] = i;
    }

    categoryPath = findFeatureCategoryPath(arr, swpArray, pickSize, feature);
    if (categoryPath) {
        free(swpArray);
        return categoryPath;
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

        categoryPath = findFeatureCategoryPath(arr, swpArray, pickSize, feature);
        if (categoryPath) {
            free(swpArray);
            return categoryPath;
        }

    } while (swpArray[0] < totalSize - pickSize);

    free(swpArray);
    return categoryPath;
}

CategoryPath *searchFeatureCategoryPath(Feature *feature, const char *categoryPath) {
    CategoryPath *featureCategoryPath = NULL;
    char **categoryArray = NULL;
    char *category = NULL;
    char *tmpStr = NULL;
    char *restOfStr = NULL;
    size_t numOfCategoryType = 0;
    size_t numOfCategory = 0;
    size_t i = 0;

    DEBUG_LOG("+Feature = %s, categoryPath = %s\n", feature->name, categoryPath);

    /* Full path search first */
    HASH_FIND_STR(feature->categoryPathHash, categoryPath, featureCategoryPath);
    if (featureCategoryPath) {
        DEBUG_LOG("fuzzySearch paramTree found. (path = %s)\n", featureCategoryPath->path);
        return featureCategoryPath;
    } else if (!strncmp(categoryPath, "", strlen("") + 1)) {
        return NULL;
    }

    /* Setup array for fuzzy search path enum */
    numOfCategoryType = audioTypeGetNumOfCategoryType(feature->audioType);
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
            ERR_LOG("Cannot parse category\n");
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
            featureCategoryPath = fuzzySearchFeatureCategoryPath(categoryArray, numOfCategory, numOfCategory - i, feature);
            if (featureCategoryPath) {
                break;
            }
        }

        if (!featureCategoryPath) {
            /* If no paramTree found, try to get the root paramTree */
            UT_string *path = NULL;
            utstring_new(path);
            utstring_printf(path, "%s", "");
            HASH_FIND_STR(feature->categoryPathHash, utstring_body(path), featureCategoryPath);
            utstring_free(path);
        }

        free(categoryArray);
        free(tmpStr);
    }

    DEBUG_LOG("-fuzzySearch featureCategoryPath %s found. \n", featureCategoryPath ? "" : "not ");
    return featureCategoryPath;
}

EXPORT int featureIsCategoryPathSupport(Feature *feature, const char *categoryPath) {
    /* Get the category path */
    CategoryPath *featureCategoryPath = NULL;
    UT_string *searchPath;

    if (!feature) {
        ERR_LOG("feature is NULL\n");
        return 0;
    }

    if (!categoryPath) {
        ERR_LOG("categoryPath is NULL\n");
        return 0;
    }

    /* Check if feature support all categoryPath first */
    featureCategoryPath = searchFeatureCategoryPath(feature, "");
    if (featureCategoryPath) {
        return 1;
    }

    searchPath = utilNormalizeCategoryGroupPathForAudioType(categoryPath, feature->audioType);
    if (!searchPath) {
        ERR_LOG("Cannot normalize categoryPath for %s AudioType. (path = %s)\n", feature->audioType->name, categoryPath);
        return 0;
    }

    /* Search the feature's category path */
    featureCategoryPath = searchFeatureCategoryPath(feature, utstring_body(searchPath));
    utstring_free(searchPath);

    if (featureCategoryPath) {
        return 1;
    } else {
        return 0;
    }
}
