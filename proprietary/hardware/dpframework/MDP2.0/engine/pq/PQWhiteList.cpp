#define LOG_TAG "PQ"
#define MTK_LOG_ENABLE 1

#include <dlfcn.h>
#include <cutils/log.h>
#include <utils/Trace.h>
#include <MtkWLParser.h>
#include <PQWhiteList.h>
#include <PQConfig.h>

using namespace android;


PQWhiteList PQWhiteList::sInstance("PQ");


PQWhiteList::PQWhiteList(const char *feature) :
        mWLHandle(NULL),
        pWLGetAppValidName(NULL),
        pPerfGetPackName(NULL),
        mIsPQEnabledOnActive(true) // PQ feature is enabled by default
{
    mWhiteListDL = dlopen("libwlparser.so", RTLD_NOW);

    if (mWhiteListDL != NULL) {
        pWLGetAppValidName = (bool (*)(void *, char *))dlsym(mWhiteListDL, "WLGetAppValidName");

        if (pWLGetAppValidName != NULL) {
            void *(*WLInitWLParser_ptr)(char *) = (void *(*)(char *))dlsym(mWhiteListDL, "WLInitWLParser");
            mWLHandle = WLInitWLParser_ptr(const_cast<char*>(feature));
        }

        if (mWLHandle == NULL) {
            PQ_LOGD("[PQWhiteList] No feature entry is found: %s", feature);
        }
    } else {
        PQ_LOGI("[PQWhiteList] libwlparser.so is absent");
    }

    mPerfServiceDL = dlopen("libperfservicenative.so", RTLD_NOW);

    if (mPerfServiceDL != NULL) {
        pPerfGetPackName = (char* (*)(void))dlsym(mPerfServiceDL, "PerfServiceNative_getPackName");
    } else {
        PQ_LOGI("[PQWhiteList] libperfservicenative.so is absent");
    }
}


PQWhiteList::~PQWhiteList()
{
    if (mWhiteListDL != NULL) {
        if (mWLHandle != NULL) {
            bool (*WLInitWLParser_ptr)(void *) = (bool (*)(void *))(dlsym(mWhiteListDL, "WLDeinitWLParser"));
            WLInitWLParser_ptr(mWLHandle);
        }
        dlclose(mWhiteListDL);
    }

    if (mPerfServiceDL != NULL)
        dlclose(mPerfServiceDL);
}


bool PQWhiteList::isEnabledOnPackage(const char *packName) const
{
    // pWLGetAppValidName() returns
    //   true ->  feature on
    //   false -> feature off
    // Note the function name is NOT associated to the meaning.
    if (pWLGetAppValidName(mWLHandle, const_cast<char *>(packName))) {
        int32_t magicCode = static_cast<P_WLPARSER_T>(mWLHandle)->mMagicCode;

        if (magicCode == WLPARSER_MAGIC) {
            return true; // Feature on
        } else {
            PQ_LOGE("[PQWhiteList] Magic code does not match.");
        }
    }

    return false;
}


void PQWhiteList::queryWhiteListConfig()
{
    if (!isAvailable()) {
        PQ_LOGI("[PQWhiteList] White list function is not available. (WDL:%d, WH:%d, Perf:%d)",
                int(mWhiteListDL != NULL), int(mWLHandle != NULL), int(mPerfServiceDL != NULL));
        return;
    }

    ATRACE_CALL();
    Mutex::Autolock _l(mLock); // To protect packName & mIsPQEnabledOnActive

    const char *packName = pPerfGetPackName();
    if (packName == NULL) {
        mIsPQEnabledOnActive = true;
        PQ_LOGI("[PQWhiteList] Invalid package name. Default enabled.");
    } else if (isEnabledOnPackage(packName)) {
        mIsPQEnabledOnActive = true;
        PQ_LOGI("[PQWhiteList] Package %s enabled.", packName);
    } else {
        mIsPQEnabledOnActive = false;
        PQ_LOGI("[PQWhiteList] Package %s disabled.", packName);
    }
}


