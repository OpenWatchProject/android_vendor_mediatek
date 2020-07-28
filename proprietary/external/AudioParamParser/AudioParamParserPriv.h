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
 *   Explor all private AudioParamParser APIs
 */

#include "AudioParamParser.h"
#include <math.h>

#ifdef WIN32
#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT __declspec(dllexport)
#endif
#include <process.h>
#include <Windows.h>
#include <direct.h>
#else   /* WIN32*/
#define EXPORT
#ifdef __cplusplus
extern "C" {
#endif
#include <errno.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <utils/Timers.h>
#include <cutils/properties.h>
#endif

#ifdef __linux__
#define FOLDER "/"
#else
#define FOLDER "\\"
#endif

#define MAX_PROP_VALUE_LEN (512)

#define PROPERTY_KEY_APP_LOG_LEVEL "vendor.audio.applog.level"
#define PROPERTY_KEY_XML_DEF_PATH  "persist.vendor.audio.tuning.def_path"

extern FILE *appLogFp;
extern int outputLogToStdout;

#ifdef WIN32
#define ERR_LOG(format, ...) \
    if(appDebugLevel <= ERR_LEVEL) \
        utilLog("ERROR[%d,%d](): %s(), " format"\n^^^^\n", _getpid(), GetCurrentThreadId(), __FUNCTION__, __VA_ARGS__)

#define WARN_LOG(format, ...) \
    if(appDebugLevel <= WARN_LEVEL) \
        utilLog("WARNING[%d,%d]: %s(), " format, _getpid(), GetCurrentThreadId(), __FUNCTION__, __VA_ARGS__)

#define INFO_LOG(format, ...) \
    if(appDebugLevel <= INFO_LEVEL) \
        utilLog("INFO[%d,%d]: %s(), " format, _getpid(), GetCurrentThreadId(), __FUNCTION__, __VA_ARGS__)

#define DEBUG_LOG(format, ...) \
    if(appDebugLevel <= DEBUG_LEVEL) \
        utilLog("DEBUG[%d,%d]: %s(), " format, _getpid(), GetCurrentThreadId(), __FUNCTION__, __VA_ARGS__)

#define MUST_LOG(format, ...) \
    utilLog("INFO[%d,%d]: %s(), " format, _getpid(), GetCurrentThreadId(), __FUNCTION__, __VA_ARGS__)

#define APP_SIZE_T_FT "%lu"
#define snprintf _snprintf
#else   /* WIN32 */
#undef LOG_TAG
#if defined(SYS_IMPL)
#define LOG_TAG "AudioParamParser-sys"
#else
#define LOG_TAG "AudioParamParser-vnd"
#endif

#include <utils/Log.h>

#define ERR_LOG(format, args...) \
    if(appDebugLevel <= ERR_LEVEL) \
        ALOGE("%s(), " format, __FUNCTION__, ##args)

#define WARN_LOG(format, args...) \
    if(appDebugLevel <= WARN_LEVEL) \
        ALOGW("%s(), " format, __FUNCTION__, ##args)

#define INFO_LOG(format, args...) \
    if(appDebugLevel <= INFO_LEVEL) \
        ALOGI("%s(), " format, __FUNCTION__, ##args)

#define DEBUG_LOG(format, args...) \
    if(appDebugLevel <= DEBUG_LEVEL) \
        ALOGD("%s(), " format, __FUNCTION__, ##args)

#define MUST_LOG(format, args...) \
    ALOGD("%s(), " format, __FUNCTION__, ##args)

#define APP_SIZE_T_FT "%zu"
#endif

struct _CategoryPathRetrievedParam {
AudioType *audioType;
xmlNodePtr paramTreeNode;
xmlNodePtr paramUnitPoolNode;
size_t paramIdCounter;
};
typedef struct _CategoryPathRetrievedParam CategoryPathRetrievedParam;

typedef void(*CATEGORY_PATH_RETRIEVED_CB_FUN)(const char *categoryPath, const char *fullCategoryPath, CategoryPathRetrievedParam *categoryPathRetrievedParam);

/* Force adding following category group info and bypass categoryGroup path checking */
static const char *HARD_CATEGORY_GROUP[][3] = {
/* {AudioTypeName, CategoryTypeName, CategoryGroupName} */
{"Speech", "Band", "NB"},
{"Speech", "Band", "WB"},
{NULL, NULL, NULL}
};

/***********************
 * Public API
 **********************/
EXPORT int              appSetAudioTypeLoadingList(const char *audioTypeLoadingList[]);
EXPORT const char      *appGetAudioTypeLoadingList(void);

EXPORT void             appSetDebugLevel(MSG_LEVEL level);
EXPORT MSG_LEVEL        appGetDebugLevel(void);

EXPORT char           **appGetXmlDirFromProperty(void);

/* appHandle API */
EXPORT APP_STATUS       appHandleInit(AppHandle *appHandle);
EXPORT APP_STATUS       appHandleUninit(AppHandle *appHandle);
EXPORT void             appHandleRedirectIOToConsole(void);
EXPORT AppHandle       *appHandleGetInstance(void);     /* Never uninit global instance */
EXPORT size_t           appHandleGetNumOfAudioType(AppHandle *appHandle);
EXPORT AudioType       *appHandleGetAudioTypeByIndex(AppHandle *appHandle, size_t index);
EXPORT AudioType       *appHandleGetAudioTypeByName(AppHandle *appHandle, const char *name);
EXPORT const char      *appHandleGetFeatureOptionValue(AppHandle *appHandle, const char *featureOptionName);
EXPORT int              appHandleIsFeatureOptionEnabled(AppHandle *appHandle, const char *featureOptionName);
EXPORT size_t           appHandleGetNumOfFeatureOption(AppHandle *appHandle);
EXPORT FeatureOption   *appHandleGetFeatureOptionByIndex(AppHandle *appHandle, size_t index);
EXPORT const char      *appHandleGetBuildTimeStamp();
EXPORT APP_STATUS       appHandleCompressFiles(const char *srcDir, const char *destFile);
EXPORT APP_STATUS       appHandleUncompressFile(const char *srcFile, const char *destDir);
EXPORT void             appHandleCustXmlEnableChanged(AppHandle *appHandle, int enable);

/* Following 4 APIs will acquire app handle write lock automatically */
EXPORT APP_STATUS       appHandleParseXml(AppHandle *appHandle, const char *dir[], const char *cusDir);
EXPORT APP_STATUS       appHandleReloadAudioType(AppHandle *appHandle, const char *audioTypeName);
EXPORT void             appHandleRegXmlChangedCb(AppHandle *appHandle, NOTIFY_CB_FUN nofiyCallback);
EXPORT void             appHandleUnregXmlChangedCb(AppHandle *appHandle, NOTIFY_CB_FUN nofiyCallback);

/* AudioType API */
EXPORT APP_STATUS       audioTypeIsTuningToolSupportedXmlVer(AudioType *audioType);
EXPORT APP_STATUS       audioTypeIsDeviceSupportedXmlVer(AudioType *audioType);
EXPORT size_t           audioTypeGetNumOfCategoryType(AudioType *audioType);
EXPORT CategoryType    *audioTypeGetCategoryTypeByIndex(AudioType *audioType, size_t idnex);
EXPORT CategoryType    *audioTypeGetCategoryTypeByName(AudioType *audioType, const char *categoryTypeName);
EXPORT CategoryType    *audioTypeGetCategoryTypeByWording(AudioType *audioType, const char *categoryTypeWording);
EXPORT xmlNode         *audioTypeGetCategoryTypeListNode(AudioType *audioType);
EXPORT ParamUnit       *audioTypeGetParamUnit(AudioType *audioType, const char *categoryPath);
EXPORT size_t           audioTypeGetNumOfParamInfo(AudioType *audioType);
EXPORT ParamInfo       *audioTypeGetParamInfoByIndex(AudioType *audioType, size_t index);
EXPORT ParamInfo       *audioTypeGetParamInfoByName(AudioType *audioType, const char *paramName);
EXPORT APP_STATUS       audioTypeSaveAudioParamXml(AudioType *audioType, const char *saveDir, int clearDirtyBit);
EXPORT int              audioTypeReadLock(AudioType *audioType, const char *callerFun);
EXPORT int              audioTypeWriteLock(AudioType *audioType, const char *callerFun);
EXPORT int              audioTypeUnlock(AudioType *audioType);
EXPORT TreeRoot        *audioTypeGetTreeRoot(AudioType *audioType, const char *treeRootName);

/* Following 3 write APIs will acquire write lock automatically */
EXPORT APP_STATUS       audioTypeSetParamData(AudioType *audioType, const char *categoryPath, ParamInfo *paramName, void *dataPtr, int arraySize);
EXPORT APP_STATUS       audioTypeSetFieldData(AudioType *audioType, const char *categoryPath, FieldInfo *fieldInfo, unsigned int val);
EXPORT APP_STATUS       audioTypeParamUnitCopy(AudioType *audioType, const char *srcCategoryPath, const char *dstCategoryPath);

/* CategoryType API */
EXPORT size_t           categoryTypeGetNumOfCategoryGroup(CategoryType *categoryType);
EXPORT CategoryGroup   *categoryTypeGetCategoryGroupByIndex(CategoryType *categoryType, size_t index);
EXPORT CategoryGroup   *categoryTypeGetCategoryGroupByWording(CategoryType *categoryType, const char *wording);
EXPORT size_t           categoryTypeGetNumOfCategory(CategoryType *categoryType);
EXPORT Category        *categoryTypeGetCategoryByIndex(CategoryType *categoryType, size_t index);
EXPORT Category        *categoryTypeGetCategoryByWording(CategoryType *categoryType, const char *wording);

/* CategoryGroup API */
EXPORT size_t           categoryGroupGetNumOfCategory(CategoryGroup *categoryGroup);
EXPORT Category        *categoryGroupGetCategoryByIndex(CategoryGroup *categoryGroup, size_t index);
EXPORT Category        *categoryGroupGetCategoryByWording(CategoryGroup *categoryGroup, const char *index);

/* CategoryAlias API */
EXPORT CategoryAlias   *categoryAliasCreate(const char *alias, Category *category);
EXPORT void             categoryAliasRelease(CategoryAlias *categoryAlias);

/* ParamInfo API */
EXPORT size_t           paramInfoGetNumOfFieldInfo(ParamInfo *paramInfo);
EXPORT FieldInfo       *paramInfoGetFieldInfoByIndex(ParamInfo *paramInfo, size_t index);
EXPORT FieldInfo       *paramInfoGetFieldInfoByName(ParamInfo *paramInfo, const char *fieldName);
EXPORT char            *paramNewDataStr(Param *param);
EXPORT char            *paramNewDataStrWithMode(Param *param, int uArrayHexMode);

/* ParamUnit API */
EXPORT size_t           paramUnitGetNumOfParam(ParamUnit *paramUnit);
EXPORT Param           *paramUnitGetParamByIndex(ParamUnit *paramUnit, size_t index);
EXPORT Param           *paramUnitGetParamByName(ParamUnit *paramUnit, const char *paramName);
EXPORT ParamInfo       *paramUnitGetParamInfo(ParamUnit *paramUnit, const char *paramInfoName);
EXPORT FieldInfo       *paramUnitGetFieldInfo(ParamUnit *paramUnit, const char *paramName, const char *fieldName);
EXPORT APP_STATUS       paramUnitGetFieldVal(ParamUnit *paramUnit, const char *paramName, const char *fieldName, unsigned int *val);

/* Param API */
EXPORT size_t           paramGetArraySizeFromString(const char *str);
EXPORT size_t           paramGetNumOfBytes(Param* param);
EXPORT APP_STATUS       paramGetFieldVal(Param *param, FieldInfo *fieldInfo, unsigned int *val);
EXPORT APP_STATUS       paramSetFieldVal(Param *param, FieldInfo *fieldInfo, unsigned int val);
EXPORT DATA_TYPE        paramDataTypeToEnum(const char *dataType);
EXPORT const char      *paramDataTypeToStr(DATA_TYPE dataType);

/* Field API */
EXPORT APP_STATUS       fieldInfoGetCheckListValue(FieldInfo *fieldInfo, const char *checkName, unsigned int *checkVal);

/* TreeRoot API */
EXPORT Feature         *treeRootGetFeatureByName(TreeRoot *treeRoot, const char *featureName);
EXPORT int              featureIsCategoryPathSupport(Feature *feature, const char *categoryPath);

/* Xml Node related APIs */
EXPORT xmlNode         *findXmlNodeByElemName(xmlNode *node, const char *elemName);
EXPORT xmlChar         *xmlNodeGetProp(xmlNode *node, const char *prop);
EXPORT xmlChar         *xmlNodeGetWording(xmlNode *node);

/* Utils APIs */
EXPORT APP_STATUS       utilConvDataStringToNative(DATA_TYPE dataType, const char *paramDataStr, void **paramData, size_t *arraySize);

/* Unit test */
EXPORT APP_STATUS       unitTest(AppHandle *appHandle);
EXPORT char            *utilGetStdin(char *buf, int bufSize);

/* Following APIs is designed for EM tool integration */
EXPORT APP_STATUS       utilNativeSetField(const char *audioTypeName, const char *categoryPath, const char *paramName, const char *fieldName, const char *fieldValueStr);
EXPORT APP_STATUS       utilNativeSetParam(const char *audioTypeName, const char *categoryPath, const char *paramName, const char *paramDataStr);
EXPORT char            *utilNativeGetCategory(const char *audioTypeName, const char *categoryTypeName);
EXPORT char            *utilNativeGetParam(const char *audioTypeName, const char *categoryPath, const char *paramName);
EXPORT unsigned int     utilNativeGetField(const char *audioTypeName, const char *categoryPath, const char *paramName, const char *fieldName);
EXPORT APP_STATUS       utilNativeSaveXml(const char *audioTypeName);
EXPORT const char      *utilNativeGetChecklist(const char *audioTypeName, const char *paramName, const char *fieldName);

/***********************
 * Private APIs
 **********************/
/* appHandle API */
EXPORT APP_STATUS       appHandleLoadDirAudioTypeInfo(AppHandle *appHandle);
EXPORT APP_STATUS       appHandleLoadAllAudioTypeXml(AppHandle *appHandle);
EXPORT APP_STATUS       appHandleLoadAudioTypeXml(AppHandle *appHandle, AudioType *audioType);
EXPORT APP_STATUS       appHandleReleaseAllAudioTypeXml(AppHandle *appHandle);
EXPORT int              appHandleIsValidAudioType(AppHandle *appHandle, const char *audioType);
EXPORT AudioType       *appHandleAddAudioType(AppHandle *appHandle, const char *audioType);
EXPORT AudioType       *appHandleGetAudioType(AppHandle *appHandle, size_t index);
EXPORT void             appHandleReleaseAudioTypeHash(AppHandle *appHandle);
EXPORT APP_STATUS       appHandleLoadDirFeatureOptionsInfo(AppHandle *appHandle);
EXPORT void             appHandleReleaseFeatureOptionsHash(AppHandle *appHandle);
EXPORT void             appHandleDumpAudioTypeList(AppHandle *appHandle);
EXPORT char            *appHandleGetAudioTypeFilePath(AppHandle *appHandle, const char *audioType, const char *posfix);
EXPORT char            *appHandleGetPreloadAudioTypeFilePath(AppHandle *appHandle, const char *audioType, const char *posfix);
EXPORT APP_STATUS       appHandleLoadAllAudioTypeHash(AppHandle *appHandle);
EXPORT int              appHandleWriteLock(AppHandle *appHandle, const char *callerFun);
EXPORT int              appHandleReadLock(AppHandle *appHandle, const char *callerFun);
EXPORT int              appHandleUnlock(AppHandle *appHandle);
EXPORT int              appHandleInstWriteLock(const char *callerFun);
EXPORT int              appHandleInstUnlock(void);
EXPORT int              appHandleIsNodeFeatureOptionEnabled(AppHandle *appHandle, xmlNode *node, int defaultValue);
EXPORT void             appHandleRemoveAudioTypeByFeatureOptions(AppHandle *appHandle);
EXPORT void             appHandleReviseXmlDocByFeatureOptions(AppHandle *appHandle);
EXPORT APP_STATUS       appHandleGetAudioTypeSupportedVerInfo(const char *audioTypeName, int *paramUnitDescVerMaj, int *paramUnitDescVerMin, int *audioParamVerMaj, int *audioParamVerMin);
EXPORT void             appHandleShowAudioTypeSupportedVerInfo(AppHandle *appHandle);
EXPORT void             appHandleReloadCustXml(AppHandle *appHandle);
EXPORT void             appHandleNotifyAllCallbacks(AppHandle *appHandle, const char *audioTypeName);
EXPORT void            *appHandleThreadLoop(void *arg);
EXPORT void            *reloadCustXmlThreadLoop(void *arg);

/* AudioType API */
EXPORT AudioType       *audioTypeCreate(AppHandle *appHandle, const char *audioTypeName);
EXPORT void             audioTypeRelease(AudioType *audioType);
EXPORT void             audioTypeReleaseAudioParam(AudioType *audioType);
EXPORT void             audioTypeDump(AudioType *audioType);
EXPORT APP_STATUS       audioTypeParseTabName(AudioType *audioType);
EXPORT APP_STATUS       audioTypeLoadStage1Hash(AudioType *audioType);
EXPORT APP_STATUS       audioTypeLoadStage2Hash(AudioType *audioType);
EXPORT APP_STATUS       audioTypeLoadParamTreeHash(AudioType *audioType);
EXPORT APP_STATUS       audioTypeLoadParamTreeView(AudioType *audioType);
EXPORT APP_STATUS       audioTypeLoadParamUnitHash(AudioType *audioType);
EXPORT Param           *audioTypeGetParamHash(AudioType *audioType, xmlNode *paramUnitNode);
EXPORT xmlNode         *audioTypeGetParamUnitDescNode(AudioType *audioType);
EXPORT xmlNode         *audioTypeGetParamUnitNode(AudioType *audioType);
EXPORT APP_STATUS       audioTypeParseXmlVer(AudioType *audioType);
EXPORT APP_STATUS       audioTypeLoadParamFieldInfoHash(AudioType *audioType);
EXPORT APP_STATUS       audioTypeLoadCategoryTypeHash(AudioType *audioType);
EXPORT size_t           audioTypeGetNumOfParamTree(AudioType *audioType);
EXPORT APP_STATUS       audioTypeValidCategoryGroupName(AudioType *audioType, const char *name);
EXPORT int              audioTypeIsHardCategoryGroup(AudioType *audioType, const char *categoryName);
EXPORT APP_STATUS       audioTypeSetupAudioParamNode(AudioType *audioType, xmlNodePtr audioParamNode);

/* CategoryType API */
EXPORT CategoryType    *categoryTypeCreate(const char *name, const char *wording, AudioType *audioType, int visible);
EXPORT void             categoryTypeRelease(CategoryType *categoryType);
EXPORT size_t           categoryTypeGetNumOfAllCategory(CategoryType *categoryType);
EXPORT Category        *categoryTypeGetAllCategoryByIndex(CategoryType *categoryType, size_t index);
EXPORT CategoryAlias   *categoryTypeGetCategoryByAlias(CategoryType *categoryType, const char *alias);
EXPORT Category        *categoryTypeGetCategoryByName(CategoryType *categoryType, const char *name);

/* CategoryGroup API */
EXPORT CategoryGroup   *categoryGroupCreate(const char *categoryGroupName, const char *categoryGroupWording, CategoryType *categoryType, int visible);
EXPORT void             categoryGroupRelease(CategoryGroup *categoryGroup);

/* Category API */
EXPORT Category        *categoryCreate(const char *name, const char *wording, CATEGORY_PARENT_TYPE parentTypeIsCategoryType, void *parent, int visible);
EXPORT void             categoryRelease(Category *category);

/* ParamTree API */
EXPORT ParamTree       *paramTreeCreate(int paramId, const char *categoryPath);
EXPORT void             paramTreeRelease(ParamTree *paramTree);
EXPORT size_t           paramTreeGetNumOfParam(ParamTree *paramTree);

/* ParamUnit API */
EXPORT ParamUnit       *paramUnitCreate(AudioType *audioType, int id, Param *param);
EXPORT ParamUnit       *paramUnitClone(ParamUnit *paramUnit);
EXPORT void             paramUnitRelease(ParamUnit *paramUnit);

/* ParamInfo API */
EXPORT ParamInfo       *paramInfoCreate(const char *name, DATA_TYPE dataType, AudioType *audioType);
EXPORT void             paramInfoRelease(ParamInfo *paramInfo);

/* FieldInfo API */
EXPORT FieldInfo       *fieldInfoCreate(const char *fieldName, unsigned int arrayIndex, int startBit, int endBit, const char *checkList, ParamInfo *paramInfo);
EXPORT void             fieldInfoRelease(FieldInfo *paramInfo);

/* Param API */
EXPORT Param           *paramCreate(const char *paramName, ParamInfo *paramInfo, const char *paramValue);
EXPORT void             paramRelease(Param *param);
EXPORT APP_STATUS       paramSetupDataInfoByStr(Param *param, const char *str);
EXPORT APP_STATUS       paramSetupDataInfoByVal(Param *param, void *data, int arraySize);
EXPORT Param           *paramHashClone(Param *paramHash);

/* ParamTreeView API */
EXPORT ParamTreeView   *paramTreeViewCreate(AudioType *audioType, int verMaj, int verMin);
EXPORT void             paramTreeViewRelease(ParamTreeView *paramTreeView);
EXPORT TreeRoot        *treeRootCreate(const char *name, xmlNode *treeRootNode, ParamTreeView *paramTreeView);
EXPORT void             treeRootRelease(TreeRoot *treeRoot);
EXPORT Feature         *featureCreate(const char *name, AudioType *audioType, FieldInfo *switchFieldInfo, const char *featureOption);
EXPORT void             featureRelease(Feature *feature);
EXPORT CategoryPath    *categoryPathCreate(Feature *feature, const char *path);
EXPORT void             categoryPathRelease(CategoryPath *categoryPath);
EXPORT APP_STATUS       categoryPathValidation(CategoryPath *categoryPath);
EXPORT FeatureField    *featureFieldCreate(FieldInfo *fieldInfo);
EXPORT void             featureFieldRelease(FeatureField *featureField);

/* Feature Options API */
EXPORT FeatureOption   *featureOptionCreate(const char *name, const char *value);
EXPORT void             featureOptionRelease(FeatureOption *featureOption);

/* Utils API */
EXPORT char            *utilConvDataToString(DATA_TYPE dataType, void *data, int arraySize, int uArrayHexMode);
EXPORT UT_string       *utilNormalizeCategoryPathForAudioType(const char *categoryPath, AudioType *audioType);
EXPORT UT_string       *utilNormalizeCategoryGroupPathForAudioType(const char *categoryPath, AudioType *audioType);
EXPORT int              utilFindUnusedParamId(AudioType *audioType);
EXPORT void             utilUsleep(unsigned int usec);
EXPORT void             utilLog(char *str, ...);
EXPORT void             utilLogClose(void);
EXPORT FieldInfo       *utilXmlNodeGetFieldInfo(AppHandle *appHandle, xmlNode *node, const char *audioTypeAttrName, const char *paramAttrName, const char *fieldAttrName);
EXPORT void             appDumpXmlDoc(xmlDoc *doc);
EXPORT void             redirectIOToConsole(void);
EXPORT void             utilMkdir(const char *dir);
EXPORT void             utilShowParamValue(Param *param);
EXPORT char            *utilStrtok(char *str, const char *delim, char **saveptr);
EXPORT char            *utilGenCheckList(int bits);
EXPORT int              utilCompNormalizeCategoryPath(AudioType *audioType, const char *srcCategoryPath, const char *dstCategoryPath);
EXPORT int              isCustXmlEnable(void);
EXPORT void             utilShellExecute(const char *prog, const char *params);
EXPORT int              utilIsAudioTypeInLoadingList(const char *audioType);
EXPORT int              utilIsUIAudioType(const char *audioType);
EXPORT void             utilDumpAudioTypeLoadingList(const char *audioTypeLoadingList[]);

#if defined(SYS_IMPL)
EXPORT APP_STATUS       registerAudioParameterChangedCallback(AppHandle *appHandle);
EXPORT APP_STATUS       unregisterAudioParameterChangedCallback(AppHandle *appHandle);
#endif

#ifndef WIN32
EXPORT void             signalHandler(int sig, siginfo_t *info, void *ucontext);
EXPORT char            *audioSystemGetParameters(const char *str);
EXPORT void             audioSystemSetParameters(const char *str);
#endif

/* Unit Test */
EXPORT void             testDebugLevel(void);
EXPORT void             testHashParamTree(void);
EXPORT void             testHashParamUnit(void);
EXPORT void             testHashParam(void);
EXPORT APP_STATUS       testReadWriteParam(AppHandle *appHandle);
EXPORT APP_STATUS       testMemoryLeak(AppHandle *appHandle);
EXPORT APP_STATUS       testAudioTypeLock(AppHandle *appHandle);
EXPORT APP_STATUS       testAppHandleInitUninit(void);
EXPORT void             inotifyTest(const char *path);
EXPORT void             notifyCbTest(AppHandle *appHandle);

#ifndef WIN32
#ifdef __cplusplus
}
#endif
#endif
