#include "AudioParamParserPriv.h"
#include <time.h>

typedef struct StrPair {
    char *key;                          /* key */
    char *value;
    UT_hash_handle hh;                          /* hash handle */
} StrPair;

StrPair *strPairCreate(const char *key, const char *value) {
    StrPair *pair = malloc(sizeof(StrPair));
    if (pair) {
        pair->key = strdup(key ? key : "");
        if (!pair->key) {
            ERR_LOG("malloc fail!\n");
        }

        pair->value = strdup(value ? value : "");
        if (!pair->value) {
            ERR_LOG("malloc fail!\n");
        }
    } else {
        ERR_LOG("malloc fail!\n");
    }

    return pair;
}

void strPairRelease(StrPair *pair) {
    if (pair) {
        free(pair->key);
        free(pair->value);
        free(pair);
    }
}

EXPORT void utilDumpAudioTypeLoadingList(const char *audioTypeLoadingList[]) {
    int index = 0;
    INFO_LOG("===AudioTypeLoadingList===\n");
    if (audioTypeLoadingList != NULL) {
        for (index = 0; audioTypeLoadingList[index] != NULL; index++) {
            INFO_LOG("[%d] %s\n", index, audioTypeLoadingList[index]);
        }
    } else {
        INFO_LOG("All audio type\n");
    }
}

EXPORT char **appGetXmlDirFromProperty() {
#if !defined(WIN32) && !defined(SYS_IMPL)
    char **xmlDirList = NULL;
    char *xmlDir = malloc(MAX_PROP_VALUE_LEN);

    property_get(PROPERTY_KEY_XML_DEF_PATH, xmlDir, "");
    if (strcmp(xmlDir, "")) {
        MUST_LOG("%s = %s", PROPERTY_KEY_XML_DEF_PATH, xmlDir);
        xmlDirList = malloc(sizeof(char*) * 2);
        xmlDirList[0] = xmlDir;
        xmlDirList[1] = NULL;
    } else {
        free(xmlDir);
    }

    return xmlDirList;
#else
    return NULL;
#endif
}

EXPORT int appSetAudioTypeLoadingList(const char *audioTypeLoadingList[]) {
    if (appAudioTypeLoadingList == NULL) {
        appAudioTypeLoadingList = audioTypeLoadingList;
        MUST_LOG("Set audioTypeLoadingList with %p\n", appAudioTypeLoadingList);
        utilDumpAudioTypeLoadingList(appAudioTypeLoadingList);
        return 1;
    } else {
        WARN_LOG("audioTypeLoadingList already been setup. (cur = %s, new = %s)\n", appAudioTypeLoadingList[0], audioTypeLoadingList[0]);
        return 0;
    }
}

EXPORT const char *appGetAudioTypeLoadingList(void) {
    return (const char*)appAudioTypeLoadingList;
}

EXPORT void appSetDebugLevel(MSG_LEVEL level) {
    INFO_LOG("appSetDebugLevel(), level = %d\n", level);
#ifndef FORCE_DEBUG_LEVEL
    appDebugLevel = level;
#endif
}

EXPORT MSG_LEVEL appGetDebugLevel() {
    INFO_LOG("appGetDebugLevel(), level = %d\n", appDebugLevel);
    return appDebugLevel;
}

EXPORT xmlNode *findXmlNodeByElemName(xmlNode *node, const char *elemName) {
    xmlNode *cur_node;

    if (!node) {
        DEBUG_LOG("xmlNode is NULL\n");
        return NULL;
    }

    if (!elemName) {
        DEBUG_LOG("elemName is NULL\n");
        return NULL;
    }

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE && !strncmp((const char *)cur_node->name, elemName, strlen(elemName) + 1)) {
            return cur_node;
        }
    }
    return NULL;
}

EXPORT int utilIsUIAudioType(const char *audioType) {
    char* uiStr = strstr(audioType, "UI");
    if (uiStr == NULL || strlen(uiStr) != 2) {
        return 0;
    }

    return 1;
}

EXPORT int utilIsAudioTypeInLoadingList(const char *audioType) {

    if (appAudioTypeLoadingList == NULL) {
        /* If appAudioTypeLoadingList not specified, all audioType are allowed */
        DEBUG_LOG("%s audio type is allow by default!\n", audioType);
        return 1;
    } else {
        int index = 0;
        for (index = 0; appAudioTypeLoadingList[index] != NULL;  index++) {
            if (!strcmp(audioType, appAudioTypeLoadingList[index])) {
                DEBUG_LOG("%s audio type is in allow list!\n", audioType);
                return 1;
            }
        }
        DEBUG_LOG("%s audio type is not in allow list!\n", audioType);

        return 0;
    }
}

EXPORT xmlChar *xmlNodeGetProp(xmlNode *node, const char *prop) {
    if (!node) {
        ERR_LOG("xmlNode is NULL\n");
        return NULL;
    }

    if (!prop) {
        ERR_LOG("prop is NULL\n");
        return NULL;
    }

    return xmlGetProp(node, (const xmlChar *)prop);
}

EXPORT xmlChar *xmlNodeGetWording(xmlNode *node) {
    xmlChar *wording = xmlNodeGetProp(node, ATTRI_WORDING);
    if (wording == NULL) {
        wording = xmlNodeGetProp(node, ATTRI_NAME);
    }
    return wording;
}

void print_element_names(xmlNode *a_node) {
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur_node->name);
        }

        print_element_names(cur_node->children);
    }
}

void appDumpXmlDoc(xmlDoc *doc) {
    /*Get the root element node */
    xmlNode *root_element = xmlDocGetRootElement(doc);

    print_element_names(root_element);
}

#ifndef WIN32
EXPORT void signalHandler(int sig, siginfo_t *info, void *ucontext) {
    INFO_LOG("Got thread notify signal. sig = %d, info = %p, ucontext = %p\n", sig, info, ucontext);
}
#endif

EXPORT APP_STATUS utilConvDataStringToNative(DATA_TYPE dataType, const char *str, void **data, size_t *arraySize) {
    errno = 0;
    switch (dataType) {
    case TYPE_STR:
        *data = strdup(str);
        *arraySize = 0;
        break;
    case TYPE_INT: {
        int *val = malloc(sizeof(int));
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        *val = strtol(str, NULL, 0);

        if (errno == ERANGE) {
            ERR_LOG("Cannot convert \"%s\" to int!\n", str);
            free(val);
            return APP_ERROR;
        }

        *data = val;
        *arraySize = 0;
        break;
    }
    case TYPE_UINT: {
        unsigned int *val = malloc(sizeof(unsigned int));
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        *val = strtoul(str, NULL, 0);
        if (errno == ERANGE) {
            ERR_LOG("Cannot convert \"%s\" to uint!\n", str);
            free(val);
            return APP_ERROR;
        }

        *data = val;
        *arraySize = 0;
        break;
    }
    case TYPE_FLOAT: {
        float *val = malloc(sizeof(float));
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        *val = (float)strtod(str, NULL);
        if (errno == ERANGE) {
            ERR_LOG("Cannot convert \"%s\" to float!\n", str);
            free(val);
            return APP_ERROR;
        }

        *data = val;
        *arraySize = 0;
        break;
    }
    case TYPE_BYTE_ARRAY: {
        char *elemData;
        unsigned int convVal;
        int index = 0;
        APP_STATUS result = APP_NO_ERROR;

        int size = paramGetArraySizeFromString(str);
        char *val = malloc(sizeof(char) * size);
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        if (size == 1) {
            /* Convert str */
            convVal = strtol(str, NULL, 0);
            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to byte array!\n", str);
                result = APP_ERROR;
            } else {
                val[0] = (char)convVal & 0xFF;
            }
        } else {
            char *tmpStr = strdup(str);
            char *restOfStr = NULL;
            elemData = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);

            /* Convert str */
            if (elemData != NULL) {
                convVal = strtol(elemData, NULL, 0);
            } else {
                errno = ERANGE;
            }

            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to byte array!\n", str);
                result = APP_ERROR;
            } else {
                val[index++] = (char)convVal & 0xFF;

                while ((elemData = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
                    convVal = strtol(elemData, NULL, 0);
                    if (errno == ERANGE) {
                        ERR_LOG("Cannot convert \"%s\" to byte array!\n", str);
                        result = APP_ERROR;
                        break;
                    }
                    val[index++] = (char)convVal & 0xFF;
                }
            }

            free(tmpStr);
        }

        if (result == APP_NO_ERROR) {
            *data = val;
            *arraySize = size;
        } else {
            free(val);
            *data = NULL;
            *arraySize = 0;
        }
        return result;
    }
    case TYPE_UBYTE_ARRAY: {
        char *elemData;
        unsigned int convVal;
        int index = 0;
        APP_STATUS result = APP_NO_ERROR;

        int size = paramGetArraySizeFromString(str);
        unsigned char *val = malloc(sizeof(unsigned char) * size);
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        if (size == 1) {
            /* Convert str */
            convVal = strtoul(str, NULL, 0);
            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to ubyte array!\n", str);
                result = APP_ERROR;
            } else {
                val[0] = (unsigned char)convVal & 0xFF;
            }
        } else {
            char *tmpStr = strdup(str);
            char *restOfStr = NULL;
            elemData = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);

            /* Convert str */
            if (elemData != NULL) {
                convVal = strtoul(elemData, NULL, 0);
            } else {
                errno = ERANGE;
            }

            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to ubyte array!\n", str);
                result = APP_ERROR;
            } else {
                val[index++] = (unsigned char)convVal & 0xFF;

                while ((elemData = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
                    convVal = strtoul(elemData, NULL, 0);
                    if (errno == ERANGE) {
                        ERR_LOG("Cannot convert \"%s\" to ubyte array!\n", str);
                        result = APP_ERROR;
                        break;
                    }
                    val[index++] = (unsigned char)convVal & 0xFF;
                }
            }

            free(tmpStr);
        }

        if (result == APP_NO_ERROR) {
            *data = val;
            *arraySize = size;
        } else {
            free(val);
            *data = NULL;
            *arraySize = 0;
        }
        return result;
    }
    case TYPE_SHORT_ARRAY: {
        char *elemData;
        unsigned int convVal;
        int index = 0;
        APP_STATUS result = APP_NO_ERROR;

        int size = paramGetArraySizeFromString(str);
        short *val = malloc(sizeof(short) * size);
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        if (size == 1) {
            /* Convert str */
            convVal = strtol(str, NULL, 0);
            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to short array!\n", str);
                result = APP_ERROR;
            } else {
                val[0] = (short)convVal & 0xFFFF;
            }
        } else {
            char *tmpStr = strdup(str);
            char *restOfStr = NULL;
            elemData = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);

            /* Convert str */
            if (elemData != NULL) {
                convVal = strtol(elemData, NULL, 0);
            } else {
                errno = ERANGE;
            }

            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to short array!\n", str);
                result = APP_ERROR;
            } else {
                val[index++] = (short)convVal & 0xFFFF;

                while ((elemData = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
                    convVal = strtol(elemData, NULL, 0);
                    if (errno == ERANGE) {
                        ERR_LOG("Cannot convert \"%s\" to short array!\n", str);
                        result = APP_ERROR;
                        break;
                    }
                    val[index++] = (short)convVal & 0xFFFF;
                }
            }

            free(tmpStr);
        }

        if (result == APP_NO_ERROR) {
            *data = val;
            *arraySize = size;
        } else {
            free(val);
            *data = NULL;
            *arraySize = 0;
        }
        return result;
    }
    case TYPE_USHORT_ARRAY: {
        char *elemData;
        unsigned int convVal;
        int index = 0;
        APP_STATUS result = APP_NO_ERROR;

        int size = paramGetArraySizeFromString(str);
        unsigned short *val = malloc(sizeof(unsigned short) * size);
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        if (size == 1) {
            /* Convert str */
            convVal = strtoul(str, NULL, 0);
            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to ushort array!\n", str);
                result = APP_ERROR;
            } else {
                val[0] = (unsigned short)convVal & 0xFFFF;
            }
        } else {
            char *tmpStr = strdup(str);
            char *restOfStr = NULL;
            elemData = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);

            /* Convert str */
            if (elemData != NULL) {
                convVal = strtoul(elemData, NULL, 0);
            } else {
                errno = ERANGE;
            }

            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to ushort array!\n", str);
                result = APP_ERROR;
            } else {
                val[index++] = (unsigned short)convVal & 0xFFFF;

                while ((elemData = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
                    convVal = strtoul(elemData, NULL, 0);
                    if (errno == ERANGE) {
                        ERR_LOG("Cannot convert \"%s\" to ushort array!\n", str);
                        result = APP_ERROR;
                        break;
                    }
                    val[index++] = (unsigned short)convVal & 0xFFFF;
                }
            }

            free(tmpStr);
        }

        if (result == APP_NO_ERROR) {
            *data = val;
            *arraySize = size;
        } else {
            free(val);
            *data = NULL;
            *arraySize = 0;
        }
        return result;
    }
    case TYPE_INT_ARRAY: {
        char *elemData;
        unsigned int convVal;
        int index = 0;
        APP_STATUS result = APP_NO_ERROR;

        int size = paramGetArraySizeFromString(str);
        int *val = malloc(sizeof(int) * size);
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        if (size == 1) {
            /* Convert str */
            convVal = strtol(str, NULL, 0);
            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to int array!\n", str);
                result = APP_ERROR;
            } else {
                val[0] = (int)convVal & 0xFFFFFFFF;
            }
        } else {
            char *tmpStr = strdup(str);
            char *restOfStr = NULL;
            elemData = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);

            /* Convert str */
            if (elemData != NULL) {
                convVal = strtol(elemData, NULL, 0);
            } else {
                errno = ERANGE;
            }

            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to int array!\n", str);
                result = APP_ERROR;
            } else {
                val[index++] = (int)convVal & 0xFFFFFFFF;

                while ((elemData = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
                    convVal = strtoul(elemData, NULL, 0);
                    if (errno == ERANGE) {
                        ERR_LOG("Cannot convert \"%s\" to int array!\n", str);
                        result = APP_ERROR;
                        break;
                    }
                    val[index++] = (int)convVal & 0xFFFFFFFF;
                }
            }

            free(tmpStr);
        }

        if (result == APP_NO_ERROR) {
            *data = val;
            *arraySize = size;
        } else {
            free(val);
            *data = NULL;
            *arraySize = 0;
        }
        return result;
    }
    case TYPE_UINT_ARRAY: {
        char *elemData;
        unsigned int convVal;
        int index = 0;
        APP_STATUS result = APP_NO_ERROR;

        int size = paramGetArraySizeFromString(str);
        unsigned int *val = malloc(sizeof(unsigned int) * size);
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        if (size == 1) {
            /* Convert str */
            convVal = strtoul(str, NULL, 0);
            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to uint array!\n", str);
                result = APP_ERROR;
            } else {
                val[0] = (unsigned int)convVal & 0xFFFFFFFF;
            }
        } else {
            char *tmpStr = strdup(str);
            char *restOfStr = NULL;
            elemData = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);

            /* Convert str */
            if (elemData != NULL) {
                convVal = strtoul(elemData, NULL, 0);
            } else {
                errno = ERANGE;
            }

            if (errno == ERANGE) {
                ERR_LOG("Cannot convert \"%s\" to uint array!\n", str);
                result = APP_ERROR;
            } else {
                val[index++] = (unsigned int)convVal & 0xFFFFFFFF;

                while ((elemData = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
                    convVal = strtoul(elemData, NULL, 0);
                    if (errno == ERANGE) {
                        ERR_LOG("Cannot convert \"%s\" to uint array!\n", str);
                        result = APP_ERROR;
                        break;
                    }
                    val[index++] = (unsigned int)convVal & 0xFFFFFFFF;
                }
            }

            free(tmpStr);
        }

        if (result == APP_NO_ERROR) {
            *data = val;
            *arraySize = size;
        } else {
            free(val);
            *data = NULL;
            *arraySize = 0;
        }
        return result;
    }
    case TYPE_DOUBLE_ARRAY: {
        char *elemData, *p;
        double convVal;
        int index = 0;
        APP_STATUS result = APP_NO_ERROR;

        int size = paramGetArraySizeFromString(str);
        double *val = malloc(sizeof(double) * size);
        if (!val) {
            ERR_LOG("malloc fail!\n");
            return APP_ERROR;
        }

        if (size == 1) {
            /* Convert str */
            convVal = strtod(str, &p);
            if (p != (str + strlen(str))) {
                ERR_LOG("Cannot convert \"%s\" to double array!\n", str);
                result = APP_ERROR;
            } else {
                val[0] = (double)convVal;
            }
        } else {
            char *tmpStr = strdup(str);
            char *restOfStr = NULL;
            elemData = utilStrtok(tmpStr, ARRAY_SEPERATOR, &restOfStr);

            if (elemData) {
                /* Convert str */
                convVal = strtod(elemData, &p);
                if (p != (elemData + strlen(elemData))) {
                    ERR_LOG("Cannot convert \"%s\" to double array!\n", elemData);
                    result = APP_ERROR;
                } else {
                    val[index++] = (double)convVal;

                    while ((elemData = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
                        convVal = strtod(elemData, &p);
                        if (p != (elemData + strlen(elemData))) {
                            ERR_LOG("Cannot convert \"%s\" to double array!\n", elemData);
                            result = APP_ERROR;
                            break;
                        }
                        val[index++] = (double)convVal;
                    }
                }
            } else {
                ERR_LOG("elemData is NULL");
                result = APP_ERROR;
            }

            free(tmpStr);
        }

        if (result == APP_NO_ERROR) {
            *data = val;
            *arraySize = size;
        } else {
            free(val);
            *data = NULL;
            *arraySize = 0;
        }
        return result;
    }
    case TYPE_UNKNOWN:
        *data = strdup(str);
        *arraySize = 0;
        break;
    default:
        *data = NULL;
        *arraySize = 0;
        break;
    }

    return APP_NO_ERROR;
}

EXPORT char *utilConvDataToString(DATA_TYPE dataType, void *data, int arraySize, int uArrayHexMode) {
    char *str = NULL;
    UT_string *dataStr = NULL;

    switch (dataType) {
    case TYPE_STR: {
        str = strdup((char *)data);
        return str;
    }
    case TYPE_INT: {
        int value = *(int *) data;
        utstring_new(dataStr);
        utstring_printf(dataStr, "%d", value);
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_UINT: {
        unsigned int value = *(unsigned int *) data;
        utstring_new(dataStr);
        utstring_printf(dataStr, "%u", value);
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_FLOAT: {
        float value = *(float *) data;
        utstring_new(dataStr);
        utstring_printf(dataStr, "%f", value);
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_BYTE_ARRAY: {
        char *byteArray = (char *)data;
        int i = 0;
        utstring_new(dataStr);
        for (i = 0; i < arraySize; i++) {
            if (i == arraySize - 1) {
                utstring_printf(dataStr, "%d", byteArray[i]);
            } else {
                utstring_printf(dataStr, "%d,", byteArray[i]);
            }
        }
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_UBYTE_ARRAY: {
        unsigned char *ubyteArray = (unsigned char *)data;
        int i = 0;
        utstring_new(dataStr);
        for (i = 0; i < arraySize; i++) {
            if (i == arraySize - 1) {
                utstring_printf(dataStr, uArrayHexMode ? "0x%X" : "%d", ubyteArray[i]);
            } else {
                utstring_printf(dataStr, uArrayHexMode ? "0x%X," : "%d,", ubyteArray[i]);
            }
        }
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_SHORT_ARRAY: {
        short *shortArray = (short *)data;
        int i = 0;
        utstring_new(dataStr);
        for (i = 0; i < arraySize; i++) {
            if (i == arraySize - 1) {
                utstring_printf(dataStr, "%d", shortArray[i]);
            } else {
                utstring_printf(dataStr, "%d,", shortArray[i]);
            }
        }
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_USHORT_ARRAY: {
        unsigned short *ushortArray = (unsigned short *)data;
        int i = 0;
        utstring_new(dataStr);
        for (i = 0; i < arraySize; i++) {
            if (i == arraySize - 1) {
                utstring_printf(dataStr, uArrayHexMode ? "0x%X" : "%d", ushortArray[i]);
            } else {
                utstring_printf(dataStr, uArrayHexMode ? "0x%X," : "%d,", ushortArray[i]);
            }
        }
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_INT_ARRAY: {
        int *intArray = (int *)data;
        int i = 0;
        utstring_new(dataStr);
        for (i = 0; i < arraySize; i++) {
            if (i == arraySize - 1) {
                utstring_printf(dataStr, "%d", intArray[i]);
            } else {
                utstring_printf(dataStr, "%d,", intArray[i]);
            }
        }
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_UINT_ARRAY: {
        unsigned int *uintArray = (unsigned int *)data;
        int i = 0;
        utstring_new(dataStr);
        for (i = 0; i < arraySize; i++) {
            if (i == arraySize - 1) {
                utstring_printf(dataStr, uArrayHexMode ? "0x%X" : "%d", uintArray[i]);
            } else {
                utstring_printf(dataStr, uArrayHexMode ? "0x%X," : "%d,", uintArray[i]);
            }
        }
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_DOUBLE_ARRAY: {
        double *doubleArray = (double *)data;
        int i = 0;
        utstring_new(dataStr);
        for (i = 0; i < arraySize; i++) {
            if (i == arraySize - 1) {
                utstring_printf(dataStr, "%f", doubleArray[i]);
            } else {
                utstring_printf(dataStr, "%f,", doubleArray[i]);
            }
        }
        str = strdup(utstring_body(dataStr));
        utstring_free(dataStr);
        return str;
    }
    case TYPE_UNKNOWN:
    case TYPE_FIELD:
        break;
    }

    return str;
}

/* Convert category path to category group path. eg: HAC -> Handset */
EXPORT UT_string *utilNormalizeCategoryGroupPathForAudioType(const char *categoryPath, AudioType *audioType) {
    UT_string *searchPath = NULL;
    char *categoryType, *category, *tmpCategoryPath;
    char *restOfStr = NULL;
    StrPair *strPairHash = NULL, *pair = NULL;
    size_t numOfCategoryType, i;
    utstring_new(searchPath);

    /* Split string with token to parse category path info */
    tmpCategoryPath = strdup(categoryPath);
    if ((categoryType = utilStrtok(tmpCategoryPath, ARRAY_SEPERATOR, &restOfStr)) == NULL) {
        free(tmpCategoryPath);
        return searchPath;
    }

    if ((category = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr)) == NULL) {
        free(tmpCategoryPath);
        return searchPath;
    }

    pair = strPairCreate(categoryType, category);
    HASH_ADD_KEYPTR(hh, strPairHash, pair->key, strlen(pair->key), pair);
    while ((categoryType = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
        if ((category = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
            pair = strPairCreate(categoryType, category);
            HASH_ADD_KEYPTR(hh, strPairHash, pair->key, strlen(pair->key), pair);
        }
    }
    free(tmpCategoryPath);

    /* Finding the audioType related Cateory*/
    numOfCategoryType = audioTypeGetNumOfCategoryType(audioType);
    for (i = 0; i < numOfCategoryType; i++) {
        CategoryType *categoryType = audioTypeGetCategoryTypeByIndex(audioType, i);
        HASH_FIND_STR(strPairHash, categoryType->name, pair);
        if (pair) {
            /* Checking if there is alias for the category name */
            CategoryAlias *categoryAlias = categoryTypeGetCategoryByAlias(categoryType, pair->value);
            if (categoryAlias) {
                if (categoryAlias->category->parentType == PARENT_IS_CATEGORY_GROUP) {
                    utstring_printf(searchPath, "%s"ARRAY_SEPERATOR, ((CategoryGroup *)categoryAlias->category->parent.category)->name);
                } else {
                    WARN_LOG("Cannot get the categroup name of %s category!\n", categoryAlias->category->name);
                }
            } else {
                Category *category = categoryTypeGetCategoryByName(categoryType, pair->value);
                if (category && category->parentType == PARENT_IS_CATEGORY_GROUP) {
                    utstring_printf(searchPath, "%s"ARRAY_SEPERATOR, ((CategoryGroup *)category->parent.category)->name);
                } else {
                    /* If the category is not category group, checking the bypass list */
                    int arrayIndex = 0;
                    int bypassCategoryName = 0;

                    for (arrayIndex = 0; HARD_CATEGORY_GROUP[arrayIndex][0]; arrayIndex++) {
                        if (!strncmp(audioType->name, HARD_CATEGORY_GROUP[arrayIndex][0], strlen(audioType->name) + 1)
                            && !strncmp(pair->key, HARD_CATEGORY_GROUP[arrayIndex][1], strlen(pair->key) + 1)
                            && !strncmp(pair->value, HARD_CATEGORY_GROUP[arrayIndex][2], strlen(pair->value) + 1)) {
                            bypassCategoryName = 1;
                            break;
                        }
                    }

                    if (bypassCategoryName) {
                        utstring_printf(searchPath, "%s"ARRAY_SEPERATOR, HARD_CATEGORY_GROUP[arrayIndex][2]);
                    } else {
                        WARN_LOG("Cannot get the categroup name of %s category!\n", pair->value);
                    }
                }
            }
        }
    }

    /* Remove the end of seperator char */
    {
        char *ch = strrchr(utstring_body(searchPath), ARRAY_SEPERATOR_CH);
        if (ch) {
            *ch = '\0';
        }
    }

    /* Release strPair */
    if (strPairHash) {
        StrPair *tmp, *item;
        HASH_ITER(hh, strPairHash, item, tmp) {
            if (strPairHash) {
                HASH_DEL(strPairHash, item);
                strPairRelease(item);
            }
        }
    }

    return searchPath;
}

EXPORT UT_string *utilNormalizeCategoryPathForAudioType(const char *categoryPath, AudioType *audioType) {
    UT_string *searchPath = NULL;
    char *categoryType, *category, *tmpCategoryPath;
    char *restOfStr = NULL;
    StrPair *strPairHash = NULL, *pair = NULL;
    size_t numOfCategoryType, i;
    utstring_new(searchPath);

    /* Split string with token to parse category path info */
    tmpCategoryPath = strdup(categoryPath);
    if ((categoryType = utilStrtok(tmpCategoryPath, ARRAY_SEPERATOR, &restOfStr)) == NULL) {
        free(tmpCategoryPath);
        return searchPath;
    }

    if ((category = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr)) == NULL) {
        free(tmpCategoryPath);
        return searchPath;
    }

    pair = strPairCreate(categoryType, category);
    HASH_ADD_KEYPTR(hh, strPairHash, pair->key, strlen(pair->key), pair);
    while ((categoryType = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
        if ((category = utilStrtok(NULL, ARRAY_SEPERATOR, &restOfStr))) {
            pair = strPairCreate(categoryType, category);
            HASH_ADD_KEYPTR(hh, strPairHash, pair->key, strlen(pair->key), pair);
        }
    }
    free(tmpCategoryPath);

    /* Finding the audioType related Cateory*/
    numOfCategoryType = audioTypeGetNumOfCategoryType(audioType);
    for (i = 0; i < numOfCategoryType; i++) {
        CategoryType *categoryType = audioTypeGetCategoryTypeByIndex(audioType, i);
        HASH_FIND_STR(strPairHash, categoryType->name, pair);
        if (pair) {
            /* Checking if there is alias for the category name */
            CategoryAlias *categoryAlias = categoryTypeGetCategoryByAlias(categoryType, pair->value);
            if (categoryAlias) {
                utstring_printf(searchPath, "%s"ARRAY_SEPERATOR, categoryAlias->category->name);
            } else {
                utstring_printf(searchPath, "%s"ARRAY_SEPERATOR, pair->value);
            }
        }
    }

    /* Remove the end of seperator char */
    {
        char *ch = strrchr(utstring_body(searchPath), ARRAY_SEPERATOR_CH);
        if (ch) {
            *ch = '\0';
        }
    }

    /* Release strPair */
    if (strPairHash) {
        StrPair *tmp, *item;
        HASH_ITER(hh, strPairHash, item, tmp) {
            if (strPairHash) {
                HASH_DEL(strPairHash, item);
                strPairRelease(item);
            }
        }
    }

    return searchPath;
}

EXPORT int utilFindUnusedParamId(AudioType *audioType) {
    ParamUnit *paramUnit;
    while (1) {
        HASH_FIND_INT(audioType->paramUnitHash, &audioType->unusedParamId, paramUnit);
        if (paramUnit) {
            audioType->unusedParamId++;
        } else {
            INFO_LOG("Unsed param ID found (id = %d)\n", audioType->unusedParamId);
            return audioType->unusedParamId++;
        }
    }

    return 0;
}


EXPORT void utilUsleep(unsigned int usec) {
#ifndef WIN32
    usleep(usec);
#else
    Sleep(usec);
#endif
}

EXPORT char *utilGetStdin(char *buf, int bufSize) {
    char *input;
    input = fgets(buf, bufSize, stdin);
    if ((input = strchr(input, '\n')) != NULL) {
        *input = '\0';
    }
    return buf;
}

EXPORT void utilLog(char *format, ...) {
    time_t timep;
    struct tm *p;
    va_list arglist;

    /* Get time struct */
    time(&timep);
    p = localtime(&timep);
    if (!p) {
        ERR_LOG("localtime fail!\n");
        return;
    }

    if (appLogFp == NULL) {
        /* Create file handle */
        UT_string *fileName = NULL;
        utstring_new(fileName);
        utstring_printf(fileName, "%04d%02d%02d_%02d%02d%02d_AppLogFile.txt", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
        appLogFp = fopen(utstring_body(fileName), "a");
        utstring_free(fileName);

        if (!appLogFp) {
            printf("Log file open failed!\n");
            exit(1);
        }
    }

    /* Write to file */
    fprintf(appLogFp, "%02d-%02d %02d:%02d:%02d ", 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    va_start(arglist, format);
    vfprintf(appLogFp, format, arglist);
    va_end(arglist);

    /* Show log to console */
    if (outputLogToStdout) {
        va_start(arglist, format);
        vfprintf(stdout, format, arglist);
        va_end(arglist);
    }
}

EXPORT void utilLogClose() {
    if (appLogFp) {
        fflush(appLogFp);
        fclose(appLogFp);
        appLogFp = NULL;
    }
}

EXPORT void utilShowParamValue(Param *param) {
    if (!param) {
        ERR_LOG("param is NULL\n");
        return;
    }

    switch (param->paramInfo->dataType) {
    case TYPE_STR: {
        char *value = (char *)param->data;
        printf("param name = %s, value = %s (type = %s, bytes = "APP_SIZE_T_FT")\n", param->name, value, paramDataTypeToStr(param->paramInfo->dataType), paramGetNumOfBytes(param));
        break;
    }
    case TYPE_INT: {
        int value = *(int *) param->data;
        printf("param name = %s, value = %d (type = %s, bytes = "APP_SIZE_T_FT")\n", param->name, value, paramDataTypeToStr(param->paramInfo->dataType), paramGetNumOfBytes(param));
        break;
    }
    case TYPE_UINT: {
        unsigned int value = *(unsigned int *) param->data;
        printf("param name = %s, value = %d (type = %s, bytes = "APP_SIZE_T_FT")\n", param->name, value, paramDataTypeToStr(param->paramInfo->dataType), paramGetNumOfBytes(param));
        break;
    }
    case TYPE_FLOAT: {
        float value = *(float *) param->data;
        /* JH's platform cannot show the float type, wei chiu could show the value by %f*/
        printf("param name = %s, value = %d (type = %s, bytes = "APP_SIZE_T_FT")\n", param->name, (int)value, paramDataTypeToStr(param->paramInfo->dataType), paramGetNumOfBytes(param));
        break;
    }
    case TYPE_BYTE_ARRAY: {
        char *byteArray = (char *)param->data;
        int arraySize = param->arraySize;
        int i = 0;
        printf("param name = %s (type = %s, size = %d, bytes = "APP_SIZE_T_FT")\n", param->name, paramDataTypeToStr(param->paramInfo->dataType), arraySize, paramGetNumOfBytes(param));
        for (i = 0; i < arraySize; i++) {
            printf("\tarray[%d] 0x%x\n", i, byteArray[i]);
        }
        break;
    }
    case TYPE_UBYTE_ARRAY: {
        unsigned char *ubyteArray = (unsigned char *)param->data;
        int arraySize = param->arraySize;
        int i = 0;
        printf("param name = %s (type = %s, size = %d, bytes = "APP_SIZE_T_FT")\n", param->name, paramDataTypeToStr(param->paramInfo->dataType), arraySize, paramGetNumOfBytes(param));
        for (i = 0; i < arraySize; i++) {
            printf("\tarray[%d] 0x%x\n", i, ubyteArray[i]);
        }
        break;
    }
    case TYPE_SHORT_ARRAY: {
        short *shortArray = (short *)param->data;
        int arraySize = param->arraySize;
        int i = 0;
        printf("param name = %s (type = %s, size = %d, bytes = "APP_SIZE_T_FT")\n", param->name, paramDataTypeToStr(param->paramInfo->dataType), arraySize, paramGetNumOfBytes(param));
        for (i = 0; i < arraySize; i++) {
            printf("\tarray[%d] %d\n", i, shortArray[i]);
        }
        break;
    }
    case TYPE_USHORT_ARRAY: {
        unsigned short *ushortArray = (unsigned short *)param->data;
        int arraySize = param->arraySize;
        int i = 0;
        printf("param name = %s (type = %s, size = %d, bytes = "APP_SIZE_T_FT")\n", param->name, paramDataTypeToStr(param->paramInfo->dataType), arraySize, paramGetNumOfBytes(param));
        for (i = 0; i < arraySize; i++) {
            printf("\tarray[%d] 0x%x\n", i, ushortArray[i]);
        }
        break;
    }
    case TYPE_INT_ARRAY: {
        int *intArray = (int *)param->data;
        int arraySize = param->arraySize;
        int i = 0;
        printf("param name = %s (type = %s, size = %d, bytes = "APP_SIZE_T_FT")\n", param->name, paramDataTypeToStr(param->paramInfo->dataType), arraySize, paramGetNumOfBytes(param));
        for (i = 0; i < arraySize; i++) {
            printf("\tarray[%d] %d\n", i, intArray[i]);
        }
        break;
    }
    case TYPE_UINT_ARRAY: {
        unsigned int *uintArray = (unsigned int *)param->data;
        int arraySize = param->arraySize;
        int i = 0;
        printf("param name = %s (type = %s, size = %d, bytes = "APP_SIZE_T_FT")\n", param->name, paramDataTypeToStr(param->paramInfo->dataType), arraySize, paramGetNumOfBytes(param));
        for (i = 0; i < arraySize; i++) {
            printf("\tarray[%d] 0x%x\n", i, uintArray[i]);
        }
        break;
    }
    case TYPE_DOUBLE_ARRAY: {
        double *doubleArray = (double *)param->data;
        int arraySize = param->arraySize;
        int i = 0;
        printf("param name = %s (type = %s, size = %d, bytes = "APP_SIZE_T_FT")\n", param->name, paramDataTypeToStr(param->paramInfo->dataType), arraySize, paramGetNumOfBytes(param));
        for (i = 0; i < arraySize; i++) {
            printf("\tarray[%d] %f\n", i, doubleArray[i]);
        }
        break;
    }
    default:
        printf("param name = %s, value = 0x%p (type = %s, bytes = "APP_SIZE_T_FT")\n", param->name, param->data, paramDataTypeToStr(param->paramInfo->dataType), paramGetNumOfBytes(param));
    }
}

EXPORT FieldInfo *utilXmlNodeGetFieldInfo(AppHandle *appHandle, xmlNode *node, const char *audioTypeAttrName, const char *paramAttrName, const char *fieldAttrName) {
    AudioType *audioType = NULL;
    ParamInfo *paramInfo = NULL;
    FieldInfo *fieldInfo = NULL;

    xmlChar *audioTypeName = NULL;
    xmlChar *paramName = NULL;
    xmlChar *fieldName = NULL;

    audioTypeName = xmlNodeGetProp(node, audioTypeAttrName);
    if (audioTypeName) {
        audioType = appHandleGetAudioTypeByName(appHandle, (const char *)audioTypeName);
    } else {
        xmlFree(audioTypeName);
        return NULL;
    }

    paramName = xmlNodeGetProp(node, paramAttrName);
    if (audioType && paramName) {
        paramInfo = audioTypeGetParamInfoByName(audioType, (const char *)paramName);
    } else {
        WARN_LOG("Cannot find FieldInfo (AudioType = %s, Param = %s, Field = %s)\n", audioTypeName, paramName, fieldName);
        xmlFree(audioTypeName);
        xmlFree(paramName);
        return NULL;
    }

    fieldName = xmlNodeGetProp(node, fieldAttrName);
    if (paramInfo && fieldName) {
        fieldInfo = paramInfoGetFieldInfoByName(paramInfo, (const char *)fieldName);
    }

    if (!fieldInfo) {
        WARN_LOG("Cannot find FieldInfo (AudioType = %s, Param = %s, Field = %s)\n", audioTypeName, paramName, fieldName);
    }

    xmlFree(audioTypeName);
    xmlFree(paramName);
    xmlFree(fieldName);
    return fieldInfo;
}

EXPORT char *utilGenCheckList(int bits) {
    UT_string *dataStr = NULL;
    double num = pow(2, bits);
    int i = 0;
    char *retStr = NULL;

    utstring_new(dataStr);
    for (i = 0; i < num; i++) {
        if (i != num - 1) {
            utstring_printf(dataStr, "%d,%d,", i, i);
        } else {
            utstring_printf(dataStr, "%d,%d", i, i);
        }
    }

    retStr = strdup(utstring_body(dataStr));
    utstring_free(dataStr);
    return retStr;
}

EXPORT void showTreeRoot(xmlNode *treeRootNode, const char *categoryPath) {
}

EXPORT void utilMkdir(const char *dir) {
#ifdef WIN32
    _mkdir(dir);
#else
    mkdir(dir, 0770);
#endif
}

unsigned int utilNativeGetField(const char *audioTypeName, const char *categoryPath, const char *paramName, const char *fieldName) {
    INFO_LOG("audioTypeName = %s, categoryPath = %s, paramName = %s, fieldName = %s", audioTypeName, categoryPath, paramName, fieldName);

    if (audioTypeName && categoryPath && paramName && fieldName) {
#if defined(SYS_IMPL)
        char *resultStr = NULL;
        unsigned int result = 0;
        UT_string *str = NULL;

        utstring_new(str);
        utstring_printf(str, APP_GET_FIELD_KEY "#%s#%s#%s#%s", audioTypeName, categoryPath, paramName, fieldName);
        resultStr = audioSystemGetParameters(utstring_body(str));
        result = atoi(resultStr);
        free(resultStr);
        utstring_free(str);

        return result;
#else
        AppHandle *appHandle = NULL;
        AudioType *audioType = NULL;
        ParamUnit *paramUnit = NULL;
        unsigned int fieldValue = 0;

        appHandle = appHandleGetInstance();
        if (appHandle) {
            audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
        } else {
            ERR_LOG("appHandle is NULL\n");
        }

        audioTypeReadLock(audioType, __FUNCTION__);

        if (audioType) {
            paramUnit = audioTypeGetParamUnit(audioType, categoryPath);
        } else {
            ERR_LOG("categoryType is NULL\n");
        }

        if (paramUnitGetFieldVal(paramUnit, paramName, fieldName, &fieldValue) == APP_ERROR) {
            ERR_LOG("Query field value fail!\n");
        } else {
            audioTypeUnlock(audioType);
            return fieldValue;
        }
        audioTypeUnlock(audioType);
#endif
    } else {
        ERR_LOG("Invalid parameter: audioType = %s, category path = %s, param = %s, field = %s\n", audioTypeName, categoryPath, paramName, fieldName);
    }

    return 0;
}

char *utilNativeGetParam(const char *audioTypeName, const char *categoryPath, const char *paramName) {
    char *paramDataStr = NULL;
    if (audioTypeName && categoryPath && paramName) {
#if defined(SYS_IMPL)
        INFO_LOG("audioTypeName = %s, categoryPath = %s, paramName = %s", audioTypeName, categoryPath, paramName);
        UT_string *str = NULL;

        utstring_new(str);
        utstring_printf(str, APP_GET_PARAM_KEY "#%s#%s#%s", audioTypeName, categoryPath, paramName);
        paramDataStr = audioSystemGetParameters(utstring_body(str));
        utstring_free(str);
#else
        AppHandle *appHandle = NULL;
        AudioType *audioType = NULL;
        ParamUnit *paramUnit = NULL;
        Param *param = NULL;

        appHandle = appHandleGetInstance();
        if (appHandle) {
            audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
        } else {
            ERR_LOG("appHandle is NULL\n");
        }

        audioTypeReadLock(audioType, __FUNCTION__);

        if (audioType) {
            paramUnit = audioTypeGetParamUnit(audioType, categoryPath);
        } else {
            ERR_LOG("categoryType is NULL\n");
        }

        if (paramUnit) {
            param = paramUnitGetParamByName(paramUnit, paramName);
        } else {
            ERR_LOG("paramUnit is NULL\n");
        }

        if (param) {
            paramDataStr = paramNewDataStr(param);
        }

        audioTypeUnlock(audioType);
#endif
    } else {
        ERR_LOG("Invalid parameter: audioType = %s, category path = %s, param = %s\n", audioTypeName, categoryPath, paramName);
    }

    return paramDataStr;
}

EXPORT char *utilNativeGetCategory(const char *audioTypeName, const char *categoryTypeName) {
    INFO_LOG("audioTypeName = %s, categoryTypeName = %s", audioTypeName, categoryTypeName);

    if (audioTypeName && categoryTypeName) {
#if defined(SYS_IMPL)
        char *result = NULL;
        UT_string *str = NULL;

        utstring_new(str);
        utstring_printf(str, APP_GET_CATEGORY_KEY "#%s#%s", audioTypeName, categoryTypeName);
        result = audioSystemGetParameters(utstring_body(str));
        utstring_free(str);

        return result;
#else
        UT_string *utString = NULL;
        char *result;
        int firstCategory = 1;
        int numOfCategory, numOfCategoryGroup = 0;
        int i, j, k;
        AppHandle *appHandle = NULL;
        AudioType *audioType = NULL;
        CategoryType *categoryType = NULL;

        utstring_new(utString);
        appHandle = appHandleGetInstance();
        if (appHandle) {
            audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
        } else {
            ERR_LOG("appHandle is NULL\n");
        }

        if (audioType) {
            categoryType = audioTypeGetCategoryTypeByName(audioType, categoryTypeName);
        } else {
            ERR_LOG("audioType is NULL\n");
        }

        if (!categoryType) {
            ERR_LOG("categoryType is NULL\n");
        }

        numOfCategoryGroup = categoryTypeGetNumOfCategoryGroup(categoryType);
        for (i = 0; i < numOfCategoryGroup; i++) {
            CategoryGroup *categoryGroup = categoryTypeGetCategoryGroupByIndex(categoryType, i);
            numOfCategory = categoryGroupGetNumOfCategory(categoryGroup);
            for (j = 0; j < numOfCategory; j++) {
                Category *category = categoryGroupGetCategoryByIndex(categoryGroup, j);
                if (firstCategory) {
                    utstring_printf(utString, "%s,%s", category->name, category->wording);
                    firstCategory = 0;
                } else {
                    utstring_printf(utString, ",%s,%s", category->name, category->wording);
                }
            }
        }

        numOfCategory = categoryTypeGetNumOfCategory(categoryType);
        for (k = 0; k < numOfCategory; k++) {
            Category *category = categoryTypeGetCategoryByIndex(categoryType, k);
            if (firstCategory) {
                utstring_printf(utString, "%s,%s", category->name, category->wording);
                firstCategory = 0;
            } else {
                utstring_printf(utString, ",%s,%s", category->name, category->wording);
            }
        }

        result = strdup(utstring_body(utString));
        utstring_free(utString);
        return result;
#endif
    }

    return NULL;
}

EXPORT APP_STATUS utilNativeSetParam(const char *audioTypeName, const char *categoryPath, const char *paramName, const char *paramDataStr) {
    INFO_LOG("audioTypeName = %s, categoryPath = %s, paramName = %s", audioTypeName, categoryPath, paramName);

    if (audioTypeName && categoryPath && paramName && paramDataStr) {
#if defined(SYS_IMPL)
        UT_string *str = NULL;

        utstring_new(str);
        utstring_printf(str, APP_SET_PARAM_KEY "=%s#%s#%s#%s", audioTypeName, categoryPath, paramName, paramDataStr);
        audioSystemSetParameters(utstring_body(str));
        utstring_free(str);

        return APP_NO_ERROR;
#else
        AppHandle *appHandle = NULL;
        AudioType *audioType = NULL;
        ParamInfo *paramInfo = NULL;

        appHandle = appHandleGetInstance();
        if (appHandle) {
            audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
        } else {
            ERR_LOG("appHandle is NULL\n");
        }

        if (audioType) {
            paramInfo = audioTypeGetParamInfoByName(audioType, paramName);
        } else {
            ERR_LOG("audioType is NULL\n");
        }

        if (paramInfo) {
            void *paramData = NULL;
            size_t arraySize = 0;

            if (utilConvDataStringToNative(paramInfo->dataType, paramDataStr, &paramData, &arraySize) == APP_NO_ERROR) {
                if (audioTypeSetParamData(audioType, categoryPath, paramInfo, paramData, arraySize) == APP_ERROR) {
                    ERR_LOG("audioTypeSetParamData fail! audioType = %s, categoryPath = %s, paramInfo = %s\n", audioType->name, categoryPath, paramInfo->name);
                } else {
                    if (paramData) {
                        free(paramData);
                    }
                    return APP_NO_ERROR;
                }
            } else {
                ERR_LOG("Cannot convert param string to native type (param str = %s)\n", paramDataStr);
            }

            if (paramData) {
                free(paramData);
            }
        } else {
            ERR_LOG("paramInfo is NULL\n");
        }
#endif
    } else {
        ERR_LOG("Invalid parameter\n");
    }

    return APP_ERROR;
}

EXPORT APP_STATUS utilNativeSetField(const char *audioTypeName, const char *categoryPath, const char *paramName, const char *fieldName, const char *fieldValueStr) {
    INFO_LOG("audioTypeName = %s, categoryPath = %s, paramName = %s, fieldName = %s", audioTypeName, categoryPath, paramName, fieldName);

    if (audioTypeName && categoryPath && paramName && fieldName && fieldValueStr) {
#if defined(SYS_IMPL)
        UT_string *str = NULL;

        utstring_new(str);
        utstring_printf(str, APP_SET_FIELD_KEY "=%s#%s#%s#%s#%s", audioTypeName, categoryPath, paramName, fieldName, fieldValueStr);
        audioSystemSetParameters(utstring_body(str));
        utstring_free(str);

        return APP_NO_ERROR;
#else
        AppHandle *appHandle = NULL;
        AudioType *audioType = NULL;
        ParamInfo *paramInfo = NULL;
        FieldInfo *fieldInfo = NULL;

        appHandle = appHandleGetInstance();
        if (appHandle) {
            audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
        } else {
            ERR_LOG("appHandle is NULL\n");
        }

        if (audioType) {
            paramInfo = audioTypeGetParamInfoByName(audioType, paramName);
        } else {
            ERR_LOG("audioType is NULL\n");
        }

        if (paramInfo) {
            fieldInfo = paramInfoGetFieldInfoByName(paramInfo, fieldName);
        } else {
            ERR_LOG("paramInfo is NULL\n");
        }

        if (fieldInfo) {
            if (audioTypeSetFieldData(audioType, categoryPath, fieldInfo, strtoul(fieldValueStr, NULL, 0)) == APP_ERROR) {
                ERR_LOG("audioTypeSetFieldData fail!. audioType = %s, categoryPath = %s, paramInfo = %s, fieldInfo = %s, value = %s\n", audioType->name, categoryPath, paramInfo->name, fieldInfo->name, fieldValueStr);
            } else {
                return APP_NO_ERROR;
            }
        } else {
            ERR_LOG("fieldInfo is NULL\n");
        }
#endif
    } else {
        ERR_LOG("Invalid parameter\n");
    }

    return APP_ERROR;
}

EXPORT APP_STATUS utilNativeSaveXml(const char *audioTypeName) {

#if defined(SYS_IMPL)
    INFO_LOG("audioTypeName = %s", audioTypeName);
    UT_string *str = NULL;

    utstring_new(str);
    utstring_printf(str, APP_SAVE_XML_KEY "=%s", audioTypeName);
    audioSystemSetParameters(utstring_body(str));
    utstring_free(str);
#else
    AppHandle *appHandle = appHandleGetInstance();
    AudioType *audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
    if (audioType) {
        if (audioType->dirty) {
#ifdef WIN32
            return audioTypeSaveAudioParamXml(audioType, XML_CUS_FOLDER_ON_TUNING_TOOL, 1) ;
#else
            return audioTypeSaveAudioParamXml(audioType, XML_CUS_FOLDER_ON_DEVICE, 1);
#endif
        } else {
            INFO_LOG("%s AudioType's parameter not changed, don't save XML\n", audioType->name);
        }
    } else {
        ERR_LOG("audioType is NULL\n");
        return APP_ERROR;
    }
#endif

    return APP_NO_ERROR;
}

EXPORT const char *utilNativeGetChecklist(const char *audioTypeName, const char *paramName, const char *fieldName) {
#if defined(SYS_IMPL)
    INFO_LOG("audioTypeName = %s, paramName = %s, fieldName = %s", audioTypeName, paramName, fieldName);

    char *result = NULL;
    UT_string *str = NULL;

    utstring_new(str);
    utstring_printf(str, APP_GET_CHECKLIST_KEY "#%s#%s#%s", audioTypeName, paramName, fieldName);
    result = audioSystemGetParameters(utstring_body(str));
    utstring_free(str);

    return result;
#else
    AppHandle *appHandle = appHandleGetInstance();
    AudioType *audioType = appHandleGetAudioTypeByName(appHandle, audioTypeName);
    ParamInfo *paramInfo = NULL;
    FieldInfo *fieldInfo = NULL;

    if (!audioType) {
        ERR_LOG("Cannot find %s AudioType\n", audioTypeName);
        return NULL;
    }

    paramInfo = audioTypeGetParamInfoByName(audioType, paramName);
    if (!paramInfo) {
        ERR_LOG("Cannot find %s paramInfo of %s AudioType\n", paramName, audioTypeName);
        return NULL;
    }

    fieldInfo = paramInfoGetFieldInfoByName(paramInfo, fieldName);
    if (!fieldInfo) {
        ERR_LOG("Cannot find %s fieldInfo of %s paramInfo of %s AudioType\n", fieldName, paramName, audioTypeName);
        return NULL;
    }

    return fieldInfo->checkListStr;
#endif
}

EXPORT int utilCompNormalizeCategoryPath(AudioType *audioType, const char *srcCategoryPath, const char *dstCategoryPath) {
    UT_string *srcNormalizedPath;
    UT_string *dstNormalizedPath;
    int equal = 0;

    if (!strncmp(srcCategoryPath, dstCategoryPath, strlen(dstCategoryPath) + 1)) {
        return 1;
    }

    srcNormalizedPath = utilNormalizeCategoryPathForAudioType(srcCategoryPath, audioType);
    dstNormalizedPath = utilNormalizeCategoryPathForAudioType(dstCategoryPath, audioType);

    if (!strncmp(utstring_body(srcNormalizedPath), utstring_body(dstNormalizedPath), strlen(utstring_body(dstNormalizedPath)) + 1)) {
        equal = 1;
        INFO_LOG("Src path %s and dst path %s are equal to %s\n", srcCategoryPath, dstCategoryPath, utstring_body(srcNormalizedPath));
    } else {
        equal = 0;
    }

    utstring_free(srcNormalizedPath);
    utstring_free(dstNormalizedPath);

    return equal;
}

EXPORT char *utilStrtok(char *str, const char *delim, char **saveptr) {
#ifdef WIN32
    return strtok(str, delim);
#else
    return strtok_r(str, delim, saveptr);
#endif
}

EXPORT void utilShellExecute(const char *prog, const char *params) {
#ifdef WIN32
    SHELLEXECUTEINFO ShExecInfo = {0};
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = prog;
    ShExecInfo.lpParameters = params;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
#endif
}
