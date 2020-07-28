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
 *   Explor all public AudioParamParser APIs
 */

#ifndef AUDIO_PARAM_PARSER_H
#define AUDIO_PARAM_PARSER_H

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>

#ifdef WIN32
#pragma warning( disable : 4996 )
#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT __declspec(dllexport)
#endif
#else   /* WIN32*/
#define EXPORT
#ifdef __cplusplus
extern "C" {
#endif
#endif

#include "utstring.h"
#include "uthash.h"
#include "utlist.h"

#ifndef WIN32
#include <dlfcn.h>
#include <pthread.h>
#include <utils/Log.h>
#endif

/* Enable cus xml support */
#define APP_FORCE_ENABLE_CUS_XML

/* Debugging Macro Definition */
//#define FORCE_DEBUG_LEVEL

static const char *XML_FOLDER_LIST_ON_TUNING_TOOL[] = {
    ".\\preload_xml\\",
    NULL
};
#define XML_CUS_FOLDER_ON_TUNING_TOOL   ".\\cus_xml\\"

static const char *XML_FOLDER_LIST_ON_DEVICE[] = {
    "/odm/etc/audio_param/",
    "/vendor/etc/audio_param/",
    "/system/etc/audio_param/",
    NULL
};

/* For AudioParamParser on EM, No valid audio type list */
static const char *EM_AUDIO_TYPE_LOADING_LIST[] = {
    NULL
};

static const char *ATCMDHANDLER_AUDIO_TYPE_LOADING_LIST[] = {
    "PlaybackACF",
    "PlaybackDRC",
    "PlaybackHCF",
    "PlaybackVolDigi",
    "SpeechVol",
    "VoIPVol",
    "Volume",
    "VolumeGainMap",
    NULL
};

#if defined(SYS_IMPL)
#define APP_LIB_NAME "libaudio_param_parser-sys.so"
#else
#define APP_LIB_NAME "libaudio_param_parser-vnd.so"
#endif

#ifndef XML_CUS_FOLDER_ON_DEVICE
#define XML_CUS_FOLDER_ON_DEVICE        "/data/vendor/audiohal/audio_param/"
#endif

#define MAX_AUDIO_TYPE_LEN 50
#define INOTIFY_BUF_SIZE 512

#define AUDIO_PARAM_XML_POSFIX          "_AudioParam.xml"
#define PARAM_UNIT_DESC_XML_POSFIX      "_ParamUnitDesc.xml"
#define PARAM_TREE_VIEW_XML_POSFIX      "_ParamTreeView.xml"
#define FEATURE_OPTIONS_XML             "AudioParamOptions.xml"

/* XML element definition */
#define ELEM_AUDIO_FEATURE_OPTIONS      "AudioParamOptions"
#define ELEM_PARAM                      "Param"
#define ELEM_PARAM_UNIT_DESC            "ParamUnitDesc"
#define ELEM_CATEGORY_TYPE_LIST         "CategoryTypeList"
#define ELEM_CATEGORY_TYPE              "CategoryType"
#define ELEM_CATEGORY_GROUP             "CategoryGroup"
#define ELEM_CATEGORY                   "Category"

#define ELEM_AUDIO_PARAM                "AudioParam"
#define ELEM_PARAM_TREE                 "ParamTree"
#define ELEM_PARAM_UNIT_POOL            "ParamUnitPool"
#define ELEM_PARAM_UNIT                 "ParamUnit"
#define ELEM_PARAM                      "Param"
#define ELEM_FIELD                      "Field"

#define ELEM_PARAM_TREE_VIEW            "ParamTreeView"
#define ELEM_TREE_ROOT                  "TreeRoot"
#define ELEM_SHEET                      "Sheet"
#define ELEM_FEATURE                    "Feature"
#define ELEM_FIELD_LIST                 "FieldList"
#define ELEM_CATEGORY_PATH_LIST         "CategoryPathList"

/* XML attribute definition */
#define ATTRI_NAME                      "name"
#define ATTRI_TAB_NAME                  "tab_name"
#define ATTRI_VERSION                   "version"
#define ATTRI_WORDING                   "wording"
#define ATTRI_PARAM_ID                  "param_id"
#define ATTRI_PATH                      "path"
#define ATTRI_VALUE                     "value"
#define ATTRI_TYPE                      "type"
#define ATTRI_ARRAY_INDEX               "array_index"
#define ATTRI_BIT                       "bit"
#define ATTRI_CHECK_LIST                "check_list"
#define ATTRI_ALIAS                     "alias"
#define ATTRI_FEATURE_OPTION            "feature_option"
#define ATTRI_SWITCH_AUDIO_TYPE         "switch_audio_type"
#define ATTRI_SWITCH_PARAM              "switch_param"
#define ATTRI_SWITCH_FIELD              "switch_field"
#define ATTRI_AUDIO_TYPE                "audio_type"
#define ATTRI_PARAM                     "param"
#define ATTRI_VISIBLE                   "visible"
#define ATTRI_PATH_DESC                 "path_desc"

/* DATA_TYPE string */
#define DATA_TYPE_UNKNOWN_STRING        "unknown"
#define DATA_TYPE_STR_STRING            "string"
#define DATA_TYPE_INT_STRING            "int"
#define DATA_TYPE_UINT_STRING           "uint"
#define DATA_TYPE_FLOAT_STRING          "float"
#define DATA_TYPE_BYTE_ARRAY_STRING     "byte_array"
#define DATA_TYPE_UBYTE_ARRAY_STRING    "ubyte_array"
#define DATA_TYPE_SHORT_ARRAY_STRING    "short_array"
#define DATA_TYPE_USHORT_ARRAY_STRING   "ushort_array"
#define DATA_TYPE_INT_ARRAY_STRING      "int_array"
#define DATA_TYPE_UINT_ARRAY_STRING     "uint_array"
#define DATA_TYPE_DOUBLE_ARRAY_STRING   "double_array"
#define DATA_TYPE_FIELD_STRING          "Field"

#define ARRAY_SEPERATOR                 ","
#define ARRAY_SEPERATOR_CH              ','
#define PARAM_FIELD_NAME_SEPERATOR      "/"

#define AUDIO_TYPE_FMT_STR(STR_LEN) AUDIO_TYPE_FMT(STR_LEN)
#define AUDIO_TYPE_FMT(STR_LEN) "%"#STR_LEN"[^_]"

#define APP_GET_FIELD_KEY               "APP_GET_FIELD"
#define APP_GET_FO_KEY                  "APP_GET_FO"
#define APP_GET_PARAM_KEY               "APP_GET_PARAM"
#define APP_GET_CATEGORY_KEY            "APP_GET_CATEGORY"
#define APP_SET_PARAM_KEY               "APP_SET_PARAM"
#define APP_SET_FIELD_KEY               "APP_SET_FIELD"
#define APP_SAVE_XML_KEY                "APP_SAVE_XML"
#define APP_GET_CHECKLIST_KEY           "APP_GET_CHECKLIST"

typedef struct _AppHandle               AppHandle;
typedef struct _AudioType               AudioType;
typedef struct _FieldInfo               FieldInfo;
typedef struct _Category                Category;
typedef struct _CategoryAlias           CategoryAlias;
typedef struct _CategoryGroup           CategoryGroup;
typedef struct _CategoryNameAlias       CategoryNameAlias;
typedef struct _CategoryPath            CategoryPath;
typedef struct _CategoryType            CategoryType;
typedef struct _Feature                 Feature;
typedef struct _FeatureField            FeatureField;
typedef struct _FeatureOption           FeatureOption;
typedef struct _Param                   Param;
typedef struct _ParamInfo               ParamInfo;
typedef struct _ParamTreeView           ParamTreeView;
typedef struct _ParamUnit               ParamUnit;
typedef struct _TreeRoot                TreeRoot;
typedef struct _NotifyCb                NotifyCb;

typedef void(*NOTIFY_CB_FUN)(AppHandle *appHandle, const char *audioType);

typedef enum {
    DEBUG_LEVEL = 0,
    INFO_LEVEL,
    WARN_LEVEL,
    ERR_LEVEL,
} MSG_LEVEL;

typedef enum {
    APP_ERROR = 0,
    APP_NO_ERROR = 1,
} APP_STATUS;

typedef enum {
    PARENT_IS_CATEGORY_GROUP = 0,
    PARENT_IS_CATEGORY_TYPE = 1,
} CATEGORY_PARENT_TYPE;

/*
   Due to the system/media/camera/include/system/camera_metadata.h declare the same TYPE_FLOAT enum name,
   If module include the camera_metadata.h and AudioParamParser.h, AudioParamParser change the DATA_TYPE
   enum decleration to avoid conflict.
   User could using the APP_TYPE_FLOAT enum instead the TYPE_FLOAT.
*/
#ifndef SYSTEM_MEDIA_INCLUDE_ANDROID_CAMERA_METADATA_H
typedef enum {
    TYPE_UNKNOWN = -1,
    TYPE_STR,
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_BYTE_ARRAY,
    TYPE_UBYTE_ARRAY,
    TYPE_SHORT_ARRAY,
    TYPE_USHORT_ARRAY,
    TYPE_INT_ARRAY,
    TYPE_UINT_ARRAY,
    TYPE_DOUBLE_ARRAY,
    TYPE_FIELD,
} DATA_TYPE;
#else
typedef enum {
    APP_TYPE_UNKNOWN = -1,
    APP_TYPE_STR,
    APP_TYPE_INT,
    APP_TYPE_UINT,
    APP_TYPE_FLOAT,
    APP_TYPE_BYTE_ARRAY,
    APP_TYPE_UBYTE_ARRAY,
    APP_TYPE_SHORT_ARRAY,
    APP_TYPE_USHORT_ARRAY,
    APP_TYPE_INT_ARRAY,
    APP_TYPE_UINT_ARRAY,
    APP_TYPE_DOUBLE_ARRAY,
    APP_TYPE_FIELD,
} DATA_TYPE;
#endif

typedef union CategoryParent {
    Category *category;                 /* Link to parent Category if it's not CategoryGroup */
    CategoryType *categoryType;         /* Link to parent CategoryType if it's CategoryGroup */
} CategoryParent;

/* UHash the parameter tree info from ParamTreeView.xml */
struct _CategoryPath {
    char *path;
    Feature *feature;
    UT_hash_handle hh;
};

struct _FeatureField {
    FieldInfo *fieldInfo;
    UT_hash_handle hh;
};

struct _Feature {
    char *name;
    char *featureOption;
    FieldInfo *switchFieldInfo;
    CategoryPath *categoryPathHash;
    FeatureField *featureFieldHash;
    AudioType *audioType;
    UT_hash_handle hh;
};

struct _TreeRoot {
    char *name;                     /* Key */
    FieldInfo *switchFieldInfo;
    xmlNode *treeRootNode;          /* Used to traversal tree */
    Feature *featureHash;           /* Used to opt feature information */
    ParamTreeView *paramTreeView;   /* Belong to which paramTreeView */
    UT_hash_handle hh;
};

struct _ParamTreeView {
    int verMaj;
    int verMin;
    AudioType *audioType;
    TreeRoot *treeRootHash;
};

/* Hash the Param & Field info from ParamUnitDesc.xml */
struct _FieldInfo {
    char *name;                         /* key */
    size_t arrayIndex;
    int startBit;
    int endBit;
    char *checkListStr;                 /* check list string array */
    struct _ParamInfo *paramInfo;       /* Link to parent ParamInfo */
    UT_hash_handle hh;                  /* hash handle */
};

struct _ParamInfo {
    char *name;                         /* key */
    DATA_TYPE dataType;
    struct _FieldInfo *fieldInfoHash;
    AudioType *audioType;               /* Link to parent AudioType */
    UT_hash_handle hh;                  /* hash handle */
};

/* Hash the param name with value from AudioParam.xml */
struct _Param {
    char *name;                         /* key */
    void *data;                         /* raw data */
    size_t arraySize;                   /* Array size if the data is the array pointer */
    ParamInfo *paramInfo;
    struct _ParamUnit *paramUnit;       /* Link to it's ParamUnit */
    UT_hash_handle hh;                  /* hash handle */
};

/* Hash the id with ParamUnit from AudioParam.xml */
struct _ParamUnit {
    int paramId;                        /* key */
    int refCount;
    AudioType *audioType;               /* Link to it's AudioType */
    struct _Param *paramHash;           /* ParamUnit's params */
    UT_hash_handle hh;
};

/* Hash ParamTree info from AudioParam.xml */
typedef struct {
    char *categoryPath;                 /* key */
    int paramId;                        /* Param id */
    UT_hash_handle hh;
} ParamTree;

struct _Category {
    char *wording;                      /* key */
    char *name;
    int visible;
    CategoryParent parent;
    CATEGORY_PARENT_TYPE parentType;
    UT_hash_handle hh;                 /* Used to handle CategoryType->categoryHash */
    UT_hash_handle hh2;                 /* Used to handle CategoryType->allCategoryHash */
};

struct _CategoryAlias {
    char *alias;                        /* key */
    Category *category;
    UT_hash_handle hh;
};

struct _CategoryGroup {
    char *wording;                      /* key */
    char *name;
    int visible;
    Category *categoryHash;             /* Link to children */
    CategoryType *categoryType;         /* Link to parent */
    UT_hash_handle hh;
};

struct _CategoryType {
    char *wording;                      /* key */
    char *name;
    int visible;
    CategoryGroup *categoryGroupHash;   /* Link to children */
    Category *categoryHash;             /* Link to children (not include these category under CategoryGroup) */
    Category *allCategoryHash;          /* Link to children (include these category under CategoryGroup) */
    CategoryAlias *categoryAliasHash;   /* Save category alias information */
    AudioType *audioType;               /* Link to parent */
    UT_hash_handle hh;
};

struct _AudioType {
    char *name;
    char *tabName;
    int paramUnitDescVerMaj;            /* ParamUniDesc version */
    int paramUnitDescVerMin;
    int audioParamVerMaj;               /* AudioParam version */
    int audioParamVerMin;
    xmlDocPtr audioParamDoc;
    xmlDocPtr paramUnitDescDoc;
    xmlDocPtr paramTreeViewDoc;
    ParamTree *paramTreeHash;
    ParamUnit *paramUnitHash;
    ParamInfo *paramInfoHash;
    ParamTreeView *paramTreeView;
    int unusedParamId;
    int dirty;                          /* Indicate if the audio type modified without saveing*/
    int allowReload;                    /* Indicate the audio type can be reload since xml updated */
    CategoryType *categoryTypeHash;
#ifndef WIN32
    pthread_rwlock_t lock;
    const char *lockCallerFun;          /* Used to cache the lock holder */
#endif
    AppHandle *appHandle;               /* Link to it's appHandle parent */
    UT_hash_handle hh;
};

struct _FeatureOption {
    char *name;
    char *value;
    UT_hash_handle hh;
};

struct _NotifyCb {
    NOTIFY_CB_FUN cb;
    struct _NotifyCb *next, *pre;
};

struct _AppHandle {
    const char **xmlDir;
    char *xmlCusDir;
    AudioType *audioTypeHash;
    FeatureOption *featureOptionsHash;
    xmlDocPtr featureOptionsDoc;
    int xmlCusDirReady;
    int xmlChangedNotifyEnabled;        /* Used to identify notify enabled */
#ifndef WIN32
    pthread_t appThread;
    int appThreadExit;                  /* Used to identify thread exit */
    int inotifyFd;
    pthread_rwlock_t lock;
    pthread_rwlock_t notifyLock;
    const char *lockCallerFun;          /* Used to cache the lock holder */
#endif
    NotifyCb *noficyCbList;
    int saveXmlWithHexMode;
    int normalizeXmlContent;            /* Breakdown all parameter tree element & add category path */
};

typedef struct AudioTypeVerInfo {
    const char *audioTypeName;
    int paramUnitDescVerMaj;
    int paramUnitDescVerMin;
    int audioParamVerMaj;
    int audioParamVerMin;
} AudioTypeVerInfo;

/*
   AudioParamParser will built-in ParamUnitDesc/AudioParam's maj number checking,
   The ParamUnitDesc/AudioParam's min number is checking by client.
*/
static const AudioTypeVerInfo audioTypeSupportVerInfo [] = {
    /* AudioType name, ParamUnitDescVer (maj, min), AudioParamVer (maj, min) */
    {"AudioCommonSetting",  1, 0, 1, 0},
    {"PlaybackACF",         1, 0, 1, 0},
    {"Playback",            1, 0, 1, 0},
    {"PlaybackDRC",         1, 0, 1, 0},
    {"PlaybackHCF",         1, 0, 1, 0},
    {"PlaybackVolAna",      1, 0, 1, 0},
    {"PlaybackVolDigi",     1, 0, 1, 0},
    {"PlaybackVolUI",       1, 0, 1, 0},
    {"Record",              1, 0, 1, 0},
    {"RecordDMNR",          1, 0, 1, 0},
    {"RecordFIR",           1, 0, 1, 0},
    {"RecordUI",            1, 0, 1, 0},
    {"RecordVol",           1, 0, 1, 0},
    {"RecordVolUI",         1, 0, 1, 0},
    {"Speech",              1, 0, 1, 0},
    {"SpeechDMNR",          1, 0, 1, 0},
    {"SpeechGeneral",       1, 0, 1, 0},
    {"SpeechMagiClarity",   1, 0, 1, 0},
    {"SpeechUI",            1, 0, 1, 0},
    {"SpeechVol",           1, 0, 1, 0},
    {"SpeechVolUI",         1, 0, 1, 0},
    {"VoIP",                1, 0, 1, 0},
    {"VoIPDMNR",            1, 0, 1, 0},
    {"VoIPGeneral",         1, 0, 1, 0},
    {"VoIPUI",              1, 0, 1, 0},
    {"VoIPVol",             1, 0, 1, 0},
    {"VoIPVolUI",           1, 0, 1, 0},
    {"Volume",              1, 0, 1, 0},
    {"VolumeGainMap",       1, 0, 1, 0},
    {NULL,                  0, 0, 0, 0}
};

typedef struct AppOps {
    void            *handle;
    AppHandle       *(*appHandleGetInstance)(void);

    int              (*appSetAudioTypeLoadingList)(const char *audioTypeLoadingList[]);
    const char      *(*appGetAudioTypeLoadingList)(void);

    void             (*appSetDebugLevel)(MSG_LEVEL level);
    MSG_LEVEL        (*appGetDebugLevel)(void);

    APP_STATUS       (*appHandleInit)(AppHandle *appHandle);
    APP_STATUS       (*appHandleUninit)(AppHandle *appHandle);
    void             (*appHandleRedirectIOToConsole)(void);
    size_t           (*appHandleGetNumOfAudioType)(AppHandle *appHandle);
    AudioType       *(*appHandleGetAudioTypeByIndex)(AppHandle *appHandle, size_t index);
    AudioType       *(*appHandleGetAudioTypeByName)(AppHandle *appHandle, const char *name);
    const char      *(*appHandleGetFeatureOptionValue)(AppHandle *appHandle, const char *featureOptionName);
    int              (*appHandleIsFeatureOptionEnabled)(AppHandle *appHandle, const char *featureOptionName);
    size_t           (*appHandleGetNumOfFeatureOption)(AppHandle *appHandle);
    FeatureOption   *(*appHandleGetFeatureOptionByIndex)(AppHandle *appHandle, size_t index);
    const char      *(*appHandleGetBuildTimeStamp)(void);
    APP_STATUS       (*appHandleCompressFiles)(const char *srcDir, const char *destFile);
    APP_STATUS       (*appHandleUncompressFile)(const char *srcFile, const char *destDir);

    /* Following 2 APIs will acquire app handle write lock automatically */
    APP_STATUS       (*appHandleParseXml)(AppHandle *appHandle, const char *dir[], const char *cusDir);
    APP_STATUS       (*appHandleReloadAudioType)(AppHandle *appHandle, const char *audioTypeName);

    /* AudioType API */
    APP_STATUS       (*audioTypeIsTuningToolSupportedXmlVer)(AudioType *audioType);
    APP_STATUS       (*audioTypeIsDeviceSupportedXmlVer)(AudioType *audioType);
    size_t           (*audioTypeGetNumOfCategoryType)(AudioType *audioType);
    CategoryType    *(*audioTypeGetCategoryTypeByIndex)(AudioType *audioType, size_t idnex);
    CategoryType    *(*audioTypeGetCategoryTypeByName)(AudioType *audioType, const char *categoryTypeName);
    CategoryType    *(*audioTypeGetCategoryTypeByWording)(AudioType *audioType, const char *categoryTypeWording);
    xmlNode         *(*audioTypeGetCategoryTypeListNode)(AudioType *audioType);
    xmlNode         *(*audioTypeGetParamUnitNode)(AudioType *audioType);
    ParamUnit       *(*audioTypeGetParamUnit)(AudioType *audioType, const char *categoryPath);
    size_t           (*audioTypeGetNumOfParamInfo)(AudioType *audioType);
    ParamInfo       *(*audioTypeGetParamInfoByIndex)(AudioType *audioType, size_t index);
    ParamInfo       *(*audioTypeGetParamInfoByName)(AudioType *audioType, const char *paramName);
    APP_STATUS       (*audioTypeSaveAudioParamXml)(AudioType *audioType, const char *saveDir, int clearDirtyBit);
    int              (*audioTypeReadLock)(AudioType *audioType, const char *callerFun);
    int              (*audioTypeWriteLock)(AudioType *audioType, const char *callerFun);
    int              (*audioTypeUnlock)(AudioType *audioType);
    TreeRoot        *(*audioTypeGetTreeRoot)(AudioType *audioType, const char *treeRootName);

    /* Following 3 write APIs will acquire write lock automatically */
    APP_STATUS       (*audioTypeSetParamData)(AudioType *audioType, const char *categoryPath, ParamInfo *paramName, void *dataPtr, int arraySize);
    APP_STATUS       (*audioTypeSetFieldData)(AudioType *audioType, const char *categoryPath, FieldInfo *fieldInfo, unsigned int val);
    APP_STATUS       (*audioTypeParamUnitCopy)(AudioType *audioType, const char *srcCategoryPath, const char *dstCategoryPath);

    /* CategoryType API */
    size_t           (*categoryTypeGetNumOfCategoryGroup)(CategoryType *categoryType);
    CategoryGroup   *(*categoryTypeGetCategoryGroupByIndex)(CategoryType *categoryType, size_t index);
    CategoryGroup   *(*categoryTypeGetCategoryGroupByWording)(CategoryType *categoryType, const char *wording);
    size_t           (*categoryTypeGetNumOfCategory)(CategoryType *categoryType);
    Category        *(*categoryTypeGetCategoryByIndex)(CategoryType *categoryType, size_t index);
    Category        *(*categoryTypeGetCategoryByWording)(CategoryType *categoryType, const char *wording);
    Category        *(*categoryTypeGetCategoryByName)(CategoryType *categoryType, const char *name);

    /* CategoryGroup API */
    size_t           (*categoryGroupGetNumOfCategory)(CategoryGroup *categoryGroup);
    Category        *(*categoryGroupGetCategoryByIndex)(CategoryGroup *categoryGroup, size_t index);
    Category        *(*categoryGroupGetCategoryByWording)(CategoryGroup *categoryGroup, const char *index);

    /* CategoryAlias API */
    CategoryAlias   *(*categoryAliasCreate)(const char *alias, Category *category);
    void             (*categoryAliasRelease)(CategoryAlias *categoryAlias);

    /* ParamInfo API */
    size_t           (*paramInfoGetNumOfFieldInfo)(ParamInfo *paramInfo);
    FieldInfo       *(*paramInfoGetFieldInfoByIndex)(ParamInfo *paramInfo, size_t index);
    FieldInfo       *(*paramInfoGetFieldInfoByName)(ParamInfo *paramInfo, const char *fieldName);
    char            *(*paramNewDataStr)(Param *param);
    char            *(*paramNewDataStrWithMode)(Param *param, int uArrayHexMode);

    /* ParamUnit API */
    size_t           (*paramUnitGetNumOfParam)(ParamUnit *paramUnit);
    Param           *(*paramUnitGetParamByIndex)(ParamUnit *paramUnit, size_t index);
    Param           *(*paramUnitGetParamByName)(ParamUnit *paramUnit, const char *paramName);
    ParamInfo       *(*paramUnitGetParamInfo)(ParamUnit *paramUnit, const char *paramInfoName);
    FieldInfo       *(*paramUnitGetFieldInfo)(ParamUnit *paramUnit, const char *paramName, const char *fieldName);
    APP_STATUS       (*paramUnitGetFieldVal)(ParamUnit *paramUnit, const char *paramName, const char *fieldName, unsigned int *val);

    /* Param API */
    size_t           (*paramGetArraySizeFromString)(const char *str);
    size_t           (*paramGetNumOfBytes)(Param *param);
    APP_STATUS       (*paramGetFieldVal)(Param *param, FieldInfo *fieldInfo, unsigned int *val);
    APP_STATUS       (*paramSetFieldVal)(Param *param, FieldInfo *fieldInfo, unsigned int val);
    DATA_TYPE        (*paramDataTypeToEnum)(const char *dataType);
    const char      *(*paramDataTypeToStr)(DATA_TYPE dataType);

    /* Field API */
    APP_STATUS       (*fieldInfoGetCheckListValue)(FieldInfo *fieldInfo, const char *checkName, unsigned int *checkVal);

    /* TreeRoot API */
    Feature         *(*treeRootGetFeatureByName)(TreeRoot *treeRoot, const char *featureName);
    int              (*featureIsCategoryPathSupport)(Feature *feature, const char *categoryPath);

    /* Xml Node related APIs */
    xmlNode         *(*findXmlNodeByElemName)(xmlNode *node, const char *elemName);
    xmlChar         *(*xmlNodeGetProp)(xmlNode *node, const char *prop);
    xmlChar         *(*xmlNodeGetWording)(xmlNode *node);

    /* Control cust XML enabl API */
    void             (*appHandleCustXmlEnableChanged)(AppHandle* appHandle, int enable);

    /* XML changed callback APIs */
    void             (*appHandleRegXmlChangedCb)(AppHandle *appHandle, NOTIFY_CB_FUN nofiyCallback);
    void             (*appHandleUnregXmlChangedCb)(AppHandle *appHandle, NOTIFY_CB_FUN nofiyCallback);

    /* Utils APIs */
    APP_STATUS       (*utilConvDataStringToNative)(DATA_TYPE dataType, const char *paramDataStr, void **paramData, size_t *arraySize);

    /* Unit test */
    APP_STATUS       (*unitTest)(AppHandle *appHandle);
    char            *(*utilGetStdin)(char *buf, int bufSize);

    /* Following APIs is designed for EM tool integration */
    APP_STATUS       (*utilNativeSetField)(const char *audioTypeName, const char *categoryPath, const char *paramName, const char *fieldName, const char *fieldValueStr);
    APP_STATUS       (*utilNativeSetParam)(const char *audioTypeName, const char *categoryPath, const char *paramName, const char *paramDataStr);
    char            *(*utilNativeGetCategory)(const char *audioTypeName, const char *categoryTypeName);
    char            *(*utilNativeGetParam)(const char *audioTypeName, const char *categoryPath, const char *paramName);
    unsigned int     (*utilNativeGetField)(const char *audioTypeName, const char *categoryPath, const char *paramName, const char *fieldName);
    APP_STATUS       (*utilNativeSaveXml)(const char *audioTypeName);
    const char      *(*utilNativeGetChecklist)(const char *audioTypeName, const char *paramName, const char *fieldName);
} AppOps;

extern const char **appAudioTypeLoadingList;
extern int          appDebugLevel;
static AppOps       appOps;
static short        appOpsInited = 0;

#ifndef WIN32
EXPORT static AppOps *appOpsGetInstance(void) {
const char *error;
const char *funName = NULL;

if (appOpsInited == 0) {
    ALOGD("%s(), init AppOps struct, lib is %s", __FUNCTION__, APP_LIB_NAME);

    /* dlopen */
    appOps.handle = dlopen(APP_LIB_NAME, RTLD_LAZY);
    if (!appOps.handle) {
        ALOGE("%s(), dlopen fail! (%s)\n", __FUNCTION__, dlerror());
        return NULL;
    }
    dlerror();    /* Clear any existing error */

    funName = "appHandleGetInstance";
    appOps.appHandleGetInstance = (AppHandle * ( *)(void)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* dlsym */
    funName = "appSetAudioTypeLoadingList";
    appOps.appSetAudioTypeLoadingList = (int ( *)(const char *audioTypeLoadingList[])) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appGetAudioTypeLoadingList";
    appOps.appGetAudioTypeLoadingList = (const char*( *)(void)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appSetDebugLevel";
    appOps.appSetDebugLevel = (void ( *)(MSG_LEVEL level)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appGetDebugLevel";
    appOps.appGetDebugLevel = (MSG_LEVEL( *)(void)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* appHandle API */
    funName = "appHandleInit";
    appOps.appHandleInit = (APP_STATUS( *)(AppHandle * appHandle)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleUninit";
    appOps.appHandleUninit = (APP_STATUS( *)(AppHandle * appHandle)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleRedirectIOToConsole";
    appOps.appHandleRedirectIOToConsole = (void ( *)(void)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleGetNumOfAudioType";
    appOps.appHandleGetNumOfAudioType = (size_t ( *)(AppHandle * appHandle)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleGetAudioTypeByIndex";
    appOps.appHandleGetAudioTypeByIndex = (AudioType * ( *)(AppHandle * appHandle, size_t index)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleGetAudioTypeByName";
    appOps.appHandleGetAudioTypeByName = (AudioType * ( *)(AppHandle * appHandle, const char * name)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleGetFeatureOptionValue";
    appOps.appHandleGetFeatureOptionValue = (const char * ( *)(AppHandle * appHandle, const char * featureOptionName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleIsFeatureOptionEnabled";
    appOps.appHandleIsFeatureOptionEnabled = (int ( *)(AppHandle * appHandle, const char * featureOptionName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleGetNumOfFeatureOption";
    appOps.appHandleGetNumOfFeatureOption = (size_t ( *)(AppHandle * appHandle)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleGetFeatureOptionByIndex";
    appOps.appHandleGetFeatureOptionByIndex = (FeatureOption * ( *)(AppHandle * appHandle, size_t index)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleGetBuildTimeStamp";
    appOps.appHandleGetBuildTimeStamp = (const char * ( *)(void)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleCompressFiles";
    appOps.appHandleCompressFiles = (APP_STATUS( *)(const char * srcDir, const char * destFile)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleUncompressFile";
    appOps.appHandleUncompressFile = (APP_STATUS( *)(const char * srcFile, const char * destDir)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* Following 2 APIs will acquire app handle write lock automatically */
    funName = "appHandleParseXml";
    appOps.appHandleParseXml = (APP_STATUS( *)(AppHandle * appHandle, const char * dir[], const char * cusDir)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleReloadAudioType";
    appOps.appHandleReloadAudioType = (APP_STATUS( *)(AppHandle * appHandle, const char * audioTypeName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* AudioType API */
    funName = "audioTypeIsTuningToolSupportedXmlVer";
    appOps.audioTypeIsTuningToolSupportedXmlVer = (APP_STATUS( *)(AudioType * audioType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeIsDeviceSupportedXmlVer";
    appOps.audioTypeIsDeviceSupportedXmlVer = (APP_STATUS( *)(AudioType * audioType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetNumOfCategoryType";
    appOps.audioTypeGetNumOfCategoryType = (size_t ( *)(AudioType * audioType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetCategoryTypeByIndex";
    appOps.audioTypeGetCategoryTypeByIndex = (CategoryType * ( *)(AudioType * audioType, size_t idnex)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetCategoryTypeByName";
    appOps.audioTypeGetCategoryTypeByName = (CategoryType * ( *)(AudioType * audioType, const char * categoryTypeName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetCategoryTypeByWording";
    appOps.audioTypeGetCategoryTypeByWording = (CategoryType * ( *)(AudioType * audioType, const char * categoryTypeWording)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetCategoryTypeListNode";
    appOps.audioTypeGetCategoryTypeListNode = (xmlNode * ( *)(AudioType * audioType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetParamUnitNode";
    appOps.audioTypeGetParamUnitNode = (xmlNode * ( *)(AudioType * audioType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetParamUnit";
    appOps.audioTypeGetParamUnit = (ParamUnit * ( *)(AudioType * audioType, const char * categoryPath)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetNumOfParamInfo";
    appOps.audioTypeGetNumOfParamInfo = (size_t ( *)(AudioType * audioType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetParamInfoByIndex";
    appOps.audioTypeGetParamInfoByIndex = (ParamInfo * ( *)(AudioType * audioType, size_t index)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetParamInfoByName";
    appOps.audioTypeGetParamInfoByName = (ParamInfo * ( *)(AudioType * audioType, const char * paramName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeSaveAudioParamXml";
    appOps.audioTypeSaveAudioParamXml = (APP_STATUS( *)(AudioType * audioType, const char * saveDir, int clearDirtyBit)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeReadLock";
    appOps.audioTypeReadLock = (int ( *)(AudioType * audioType, const char * callerFun)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeWriteLock";
    appOps.audioTypeWriteLock = (int ( *)(AudioType * audioType, const char * callerFun)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeUnlock";
    appOps.audioTypeUnlock = (int ( *)(AudioType * audioType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeGetTreeRoot";
    appOps.audioTypeGetTreeRoot = (TreeRoot * ( *)(AudioType * audioType, const char * treeRootName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* Following 3 write APIs will acquire write lock automatically */
    funName = "audioTypeSetParamData";
    appOps.audioTypeSetParamData = (APP_STATUS( *)(AudioType * audioType, const char * categoryPath, ParamInfo * paramName, void * dataPtr, int arraySize)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeSetFieldData";
    appOps.audioTypeSetFieldData = (APP_STATUS( *)(AudioType * audioType, const char * categoryPath, FieldInfo * fieldInfo, unsigned int val)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "audioTypeParamUnitCopy";
    appOps.audioTypeParamUnitCopy = (APP_STATUS( *)(AudioType * audioType, const char * srcCategoryPath, const char * dstCategoryPath)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* CategoryType API */
    funName = "categoryTypeGetNumOfCategoryGroup";
    appOps.categoryTypeGetNumOfCategoryGroup = (size_t ( *)(CategoryType * categoryType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "categoryTypeGetCategoryGroupByIndex";
    appOps.categoryTypeGetCategoryGroupByIndex = (CategoryGroup * ( *)(CategoryType * categoryType, size_t index)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "categoryTypeGetCategoryGroupByWording";
    appOps.categoryTypeGetCategoryGroupByWording = (CategoryGroup * ( *)(CategoryType * categoryType, const char * wording)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "categoryTypeGetNumOfCategory";
    appOps.categoryTypeGetNumOfCategory = (size_t ( *)(CategoryType * categoryType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "categoryTypeGetCategoryByIndex";
    appOps.categoryTypeGetCategoryByIndex = (Category * ( *)(CategoryType * categoryType, size_t index)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "categoryTypeGetCategoryByWording";
    appOps.categoryTypeGetCategoryByWording = (Category * ( *)(CategoryType * categoryType, const char * wording)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "categoryTypeGetCategoryByName";
    appOps.categoryTypeGetCategoryByName = (Category * ( *)(CategoryType * categoryType, const char * name)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* CategoryGroup API */
    funName = "categoryGroupGetNumOfCategory";
    appOps.categoryGroupGetNumOfCategory = (size_t ( *)(CategoryGroup * categoryGroup)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "categoryGroupGetCategoryByIndex";
    appOps.categoryGroupGetCategoryByIndex = (Category * ( *)(CategoryGroup * categoryGroup, size_t index)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "categoryGroupGetCategoryByWording";
    appOps.categoryGroupGetCategoryByWording = (Category * ( *)(CategoryGroup * categoryGroup, const char * index)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* CategoryAlias API */
    funName = "categoryAliasCreate";
    appOps.categoryAliasCreate = (CategoryAlias * ( *)(const char * alias, Category * category)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "categoryAliasRelease";
    appOps.categoryAliasRelease = (void ( *)(CategoryAlias * categoryAlias)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* ParamInfo API */
    funName = "paramInfoGetNumOfFieldInfo";
    appOps.paramInfoGetNumOfFieldInfo = (size_t ( *)(ParamInfo * paramInfo)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramInfoGetFieldInfoByIndex";
    appOps.paramInfoGetFieldInfoByIndex = (FieldInfo * ( *)(ParamInfo * paramInfo, size_t index)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramInfoGetFieldInfoByName";
    appOps.paramInfoGetFieldInfoByName = (FieldInfo * ( *)(ParamInfo * paramInfo, const char * fieldName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramNewDataStr";
    appOps.paramNewDataStr = (char * ( *)(Param * param)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramNewDataStrWithMode";
    appOps.paramNewDataStrWithMode = (char * ( *)(Param * param, int uArrayHexMode)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* ParamUnit API */
    funName = "paramUnitGetNumOfParam";
    appOps.paramUnitGetNumOfParam = (size_t ( *)(ParamUnit * paramUnit)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramUnitGetParamByIndex";
    appOps.paramUnitGetParamByIndex = (Param * ( *)(ParamUnit * paramUnit, size_t index)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramUnitGetParamByName";
    appOps.paramUnitGetParamByName = (Param * ( *)(ParamUnit * paramUnit, const char * paramName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramUnitGetParamInfo";
    appOps.paramUnitGetParamInfo = (ParamInfo * ( *)(ParamUnit * paramUnit, const char * paramInfoName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramUnitGetFieldInfo";
    appOps.paramUnitGetFieldInfo = (FieldInfo * ( *)(ParamUnit * paramUnit, const char * paramName, const char * fieldName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramUnitGetFieldVal";
    appOps.paramUnitGetFieldVal = (APP_STATUS( *)(ParamUnit * paramUnit, const char * paramName, const char * fieldName, unsigned int * val)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* Param API */
    funName = "paramGetArraySizeFromString";
    appOps.paramGetArraySizeFromString = (size_t ( *)(const char * str)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramGetNumOfBytes";
    appOps.paramGetNumOfBytes = (size_t ( *)(Param * param)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramGetFieldVal";
    appOps.paramGetFieldVal = (APP_STATUS( *)(Param * param, FieldInfo * fieldInfo, unsigned int * val)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramSetFieldVal";
    appOps.paramSetFieldVal = (APP_STATUS( *)(Param * param, FieldInfo * fieldInfo, unsigned int val)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramDataTypeToEnum";
    appOps.paramDataTypeToEnum = (DATA_TYPE( *)(const char * dataType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "paramDataTypeToStr";
    appOps.paramDataTypeToStr = (const char * ( *)(DATA_TYPE dataType)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* Field API */
    funName = "fieldInfoGetCheckListValue";
    appOps.fieldInfoGetCheckListValue = (APP_STATUS( *)(FieldInfo * fieldInfo, const char * checkName, unsigned int * checkVal)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* TreeRoot API */
    funName = "treeRootGetFeatureByName";
    appOps.treeRootGetFeatureByName = (Feature * ( *)(TreeRoot * treeRoot, const char * featureName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "featureIsCategoryPathSupport";
    appOps.featureIsCategoryPathSupport = (int ( *)(Feature * feature, const char * categoryPath)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* Xml Node related APIs */
    funName = "findXmlNodeByElemName";
    appOps.findXmlNodeByElemName = (xmlNode * ( *)(xmlNode * node, const char * elemName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "xmlNodeGetProp";
    appOps.xmlNodeGetProp = (xmlChar * ( *)(xmlNode * node, const char * prop)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "xmlNodeGetWording";
    appOps.xmlNodeGetWording = (xmlChar * ( *)(xmlNode * node)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleCustXmlEnableChanged";
    appOps.appHandleCustXmlEnableChanged = (void ( *)(AppHandle * appHandle, int enable)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* XML changed callback APIs */
    funName = "appHandleRegXmlChangedCb";
    appOps.appHandleRegXmlChangedCb = (void ( *)(AppHandle * appHandle, NOTIFY_CB_FUN nofiyCallback)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "appHandleUnregXmlChangedCb";
    appOps.appHandleUnregXmlChangedCb = (void ( *)(AppHandle * appHandle, NOTIFY_CB_FUN nofiyCallback)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* Utils APIs */
    funName = "utilConvDataStringToNative";
    appOps.utilConvDataStringToNative = (APP_STATUS( *)(DATA_TYPE dataType, const char * paramDataStr, void **paramData, size_t * arraySize)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* Unit test */
    funName = "unitTest";
    appOps.unitTest = (APP_STATUS( *)(AppHandle * appHandle)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "utilGetStdin";
    appOps.utilGetStdin = (char * ( *)(char * buf, int bufSize)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    /* Following APIs is designed for EM tool integration */
    funName = "utilNativeSetField";
    appOps.utilNativeSetField = (APP_STATUS( *)(const char * audioTypeName, const char * categoryPath, const char * paramName, const char * fieldName, const char * fieldValueStr)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "utilNativeSetParam";
    appOps.utilNativeSetParam = (APP_STATUS( *)(const char * audioTypeName, const char * categoryPath, const char * paramName, const char * paramDataStr)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "utilNativeGetCategory";
    appOps.utilNativeGetCategory = (char * ( *)(const char * audioTypeName, const char * categoryTypeName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "utilNativeGetParam";
    appOps.utilNativeGetParam = (char * ( *)(const char * audioTypeName, const char * categoryPath, const char * paramName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "utilNativeGetField";
    appOps.utilNativeGetField = (unsigned int ( *)(const char * audioTypeName, const char * categoryPath, const char * paramName, const char * fieldName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "utilNativeSaveXml";
    appOps.utilNativeSaveXml = (APP_STATUS( *)(const char * audioTypeName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    funName = "utilNativeGetChecklist";
    appOps.utilNativeGetChecklist = (const char * ( *)(const char * audioTypeName, const char * paramName, const char * fieldName)) dlsym(appOps.handle, funName);
    error = dlerror();
    if (error != NULL) {
        ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
        return NULL;
    }

    appOpsInited = 1;
}

return &appOps;
}

EXPORT static __attribute__((unused)) int appIsFeatureOptionEnabled(const char* fo) {
    AppOps* appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), AppOps is NULL!\n", __FUNCTION__);
        return -1;
    }

    AppHandle *appHandle = appOps->appHandleGetInstance();
    if (appHandle == NULL) {
        ALOGE("%s(), AppHandle is NULL!\n", __FUNCTION__);
        return -1;
    }

    return appOps->appHandleIsFeatureOptionEnabled(appHandle, fo);
}

EXPORT static __attribute__((unused)) const char* appGetFeatureOptionValue(const char* fo) {
    AppOps* appOps = appOpsGetInstance();
    if (appOps == NULL) {
        ALOGE("%s(), AppOps is NULL!\n", __FUNCTION__);
        return NULL;
    }

    AppHandle *appHandle = appOps->appHandleGetInstance();
    if (appHandle == NULL) {
        ALOGE("%s(), AppHandle is NULL!\n", __FUNCTION__);
        return NULL;
    }

    return appOps->appHandleGetFeatureOptionValue(appHandle, fo);
}

static __attribute__((unused)) void appOpsDelInstance() {
if (appOpsInited == 1) {
    dlclose(appOps.handle);
    appOps.handle = NULL;
    appOpsInited = 0;
}
}
#endif

#ifndef WIN32
#ifdef __cplusplus
}
#endif
#endif

#endif
