#include "stdlib.h"
#include "stdio.h"
#include "malloc.h"
#include <libexpat/expat.h>
#include <dlfcn.h>
#include <cutils/log.h>
#include <sys/types.h>

#include <utils/KeyedVector.h>
#include <utils/Vector.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>

#include <IMtkWLService.h>
#include <binder/IServiceManager.h>

#include "MtkWLParser.h"
#include "PerfServiceNative.h"

#define PERF_LIB_FULL_NAME "libperfservicenative.so"

#undef LOG_TAG
#define LOG_TAG "WLParser"

typedef char* (*getPackName)(void);

using namespace android;

//#define ALOGV ALOGE

extern "C" void *WLInitWLParser(char *feature)
{
    P_WLPARSER_T pWLParser = NULL;

    pWLParser = (P_WLPARSER_T) malloc(sizeof(WLPARSER_T));
    if (pWLParser != NULL) {
        memset(pWLParser, 0, sizeof(WLPARSER_T));
        pWLParser->mMagicCode           = WLPARSER_MAGIC;
        strcpy(pWLParser->mFeature, feature);
        ALOGV("%s %s (0x%08x)\n", feature, pWLParser->mFeature, pWLParser);
        pWLParser->mApplicationExist    = false;
        pWLParser->mOptionCount         = 0;
    }
    else {
        ALOGE("allocate memory fail\n");
    }



    return (void *)pWLParser;
}

extern "C" bool WLGetAppValidName(void *hWLHandle, char *application)
{
    bool bSuccess = false;
    char *callerAPP;
    P_WLPARSER_T pWLParser = (P_WLPARSER_T) hWLHandle;

    if (pWLParser == NULL || pWLParser->mMagicCode != WLPARSER_MAGIC) {
        ALOGE("invalid white list parser structure");
        bSuccess = false;
        return bSuccess;
    }

    // check application call name
    void *handle;
    handle = dlopen(PERF_LIB_FULL_NAME, RTLD_NOW);
    if (handle != NULL) {
        char* (*func)(void);
        char* (*perfGetPackName)(void) = NULL;

        func = (char* (*)(void))dlsym(handle, "PerfServiceNative_getPackName");
        perfGetPackName = reinterpret_cast<getPackName>(func);
        if (perfGetPackName != NULL) {
            callerAPP = perfGetPackName();
            ALOGV("get app name done. %s\n", callerAPP);
            strcpy(pWLParser->mCallerApp, callerAPP);
        }
        else {
            pWLParser->mCallerApp[0] = 0;
        }
        dlclose(handle);
    }

    if (application != NULL) {
        strcpy(pWLParser->mApplication, application);
    }
    else {
        strcpy(pWLParser->mApplication, pWLParser->mCallerApp);
    }
    ALOGV("%s %s (0x%08x) \n", application, pWLParser->mApplication, pWLParser);
    ALOGV("%s (0x%08x) \n", pWLParser->mCallerApp, pWLParser);

    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->checkService(String16("WhiteListService"));;
    if (binder == 0) {
        ALOGV("Get white list service failed!\n");
        return bSuccess;
    }
    else {
        ALOGV("Get white list service OK!\n");
    }

    sp<IWhiteListService> WhiteListService = interface_cast<IWhiteListService>(binder);

    pWLParser->mApplicationExist = WhiteListService->CheckOptionOnorOff(String8(pWLParser->mFeature), String8(pWLParser->mApplication), String8(pWLParser->mCallerApp));

    bSuccess = pWLParser->mApplicationExist;
    if (bSuccess == true) {
        ALOGV("application %s is found\n", application);
        P_Option    pOptionTemp = NULL;
        String8 __name, __value_;
        for (int i = 0; ; i++) {
            if (WhiteListService->GetOptionForApp(i, &__name, &__value_) == true) {
                ALOGV("WLGetAppValidName : %d name %s, value %s", i, __name.string(), __value_.string());
                pOptionTemp = (P_Option) malloc(sizeof(Option));
                if (pOptionTemp != NULL) {
                    pWLParser->mOptionList[i] = pOptionTemp;
                    strcpy(pWLParser->mOptionList[i]->_name, __name.string());
                    strcpy(pWLParser->mOptionList[i]->_value, __value_.string());
                }
                pWLParser->mOptionCount = i + 1;
            }
            else {
                break;
            }
        }
    }
    else {
        pWLParser->mOptionCount = 0;
        ALOGV("application %s is not found\n", application);
    }
    return bSuccess;
}

extern "C" bool WLDeinitWLParser(void *hWLHandle)
{
    bool bSuccess = true;
    P_WLPARSER_T pWLParser = (P_WLPARSER_T) hWLHandle;

    for (int i = 0; i < pWLParser->mOptionCount; i++) {
        if (pWLParser->mOptionList[i] != NULL) {
            delete(pWLParser->mOptionList[i]);
            pWLParser->mOptionList[i] = NULL;
        }
    }
    pWLParser->mOptionCount = 0;
    
    if (pWLParser != NULL) {
        free(pWLParser);
    }

    return bSuccess;
}

#if 0
void WLPARSER_T::startElementHandler(
        const char *name, const char **attrs) {

    size_t i;

    switch (mCurrentSection) {
        case SECTION_TOPLEVEL:
        {
            ALOGV("SECTION_TOPLEVEL name : %s\n", name);
            mCurrentSection = SECTION_WHITELIST;
            break;
        }

        case SECTION_WHITELIST:
        {
            ALOGV("SECTION_WHITELIST name : %s\n", name);
#if 1
            if (!strcmp(name, mFeature)) {
                mCurrentSection = SECTION_FEATURE;
            }
            else {
                ALOGV("different feature request %s %s\n", name, mFeature);
            }
#else
            if (!strcmp(name, "MJC")) {
                mCurrentSection = SECTION_FEATURE;
            } else if (!strcmp(name, "PQ")) {
                mCurrentSection = SECTION_FEATURE;
            }
            else {
                ALOGE("Unknown feature request %s %s\n", name, mFeature);
            }
#endif
            break;
        }

        case SECTION_FEATURE:
        {
            ALOGV("SECTION_FEATURE name : %s\n", name);
            if (!strcmp(name, "Application")) {
                i = 0;
                while (attrs[i] != NULL) {
                    ALOGV("%s : %s\n",attrs[i], attrs[i + 1]);
                    if (!strcmp(attrs[i], "name")) {
                        if (attrs[i + 1] == NULL) {
                            break;
                        }
                        // check application name here
                        if (!strcmp(attrs[i+1], mApplication)) {
                            mCurrentSection = SECTION_APPLICATION;
                            mApplicationExist = true;
                        }
                        if (!strcmp(attrs[i+1], ALLENABLE) || !strcmp(mApplication, ALLENABLE)) {
                            mApplicationExist = true;
                        }
                        if (!strcmp(attrs[i+1], ALLDISABLE) || !strcmp(mApplication, ALLDISABLE)) {
                            mApplicationExist = false;
                        }
                        if (!strcmp(attrs[i+1], mCallerApp)) {
                            mCurrentSection = SECTION_APPLICATION;
                        }
                        ++i;
                    } else {
                        ALOGV("return\n");
                        break;
                    }

                    ++i;
                }
            }
            break;
        }

        case SECTION_APPLICATION:
        {
            ALOGV("SECTION_APPLICATION name : %s\n", name);
            if (!strcmp(name, "option")) {
                mCurrentSection = SECTION_OPTION;
                i = 0;
                while (attrs[i] != NULL) {
                    ALOGV("%s : %s\n",attrs[i], attrs[i + 1]);
                    if (!strcmp(attrs[i], "name")) {
                        if (attrs[i + 1] == NULL) {
                            break;
                        }
                        // create option handle here
                        CreateOptionForApplication(attrs[i+1]);
                        ++i;
                    }
                    else if (!strcmp(attrs[i], "value")) {
                        if (attrs[i + 1] == NULL) {
                            break;
                        }
                        // add option value here
                        AddOptionValue(attrs[i+1]);
                        ++i;
                    } else {
                        ALOGV("return\n");
                        break;
                    }

                    ++i;
                }
            }
            break;
        }
        case SECTION_OPTION:
        {
            ALOGV("SECTION_OPTION name : %s\n", name);
            i = 0;
            while (attrs[i] != NULL) {
                ALOGV("%s : %s\n",attrs[i], attrs[i + 1]);
                if (!strcmp(attrs[i], "name")) {
                    if (attrs[i + 1] == NULL) {
                        break;
                    }
                    // nothing to do now
                    ++i;
                } else {
                    ALOGV("return\n");
                    break;
                }

                ++i;
            }
            break;
        }

        default:
            break;
    }

    ++mDepth;
}

void WLPARSER_T::endElementHandler(const char *name) {

    switch (mCurrentSection) {
        case SECTION_TOPLEVEL:
        {
            ALOGV("endElementHandler SECTION_TOPLEVEL in %s\n", name);
        }

        case SECTION_WHITELIST:
        {
            ALOGV("endElementHandler SECTION_WHITELIST in %s\n", name);
            //mCurrentSection = SECTION_TOPLEVEL;
            if (!strcmp(name, "WHITELIST")) {
                mCurrentSection = SECTION_TOPLEVEL;
            }
            else {
                ALOGV("No upper level %s\n", name);
            }
            break;
        }
        case SECTION_FEATURE:
        {
            ALOGV("endElementHandler SECTION_FEATURE in %s\n", name);
            if (!strcmp(name, mFeature)) {
                mCurrentSection = SECTION_WHITELIST;
            }
            else {
                ALOGV("different feature request %s\n", name);
            }
            break;
        }

        case SECTION_APPLICATION:
        {
            ALOGV("endElementHandler SECTION_APPLICATION in %s\n", name);
            mCurrentSection = SECTION_FEATURE;
            break;
        }

        case SECTION_OPTION:
        {
            ALOGV("endElementHandler SECTION_OPTION in %s\n", name);
            mCurrentSection = SECTION_APPLICATION;
            break;
        }

        default:
            break;
    }

    --mDepth;
}
#endif
