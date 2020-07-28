#ifndef __PQCOMMON_H__
#define __PQCOMMON_H__

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <math.h>

#define PQ_PROT_LOGD(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, "PQCommon", "[PQ_PROT] " fmt, ##__VA_ARGS__)
#define PQ_PROT_LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, "PQCommon", "[PQ_PROT] " fmt, ##__VA_ARGS__)

class CustParameters {
private:
    static CustParameters mPQCust; // Singleton

    void *mLib;
    void *mLib2; // prior to mLib

    template <typename T>
    void logLoadVar(const char *name, const T* var) {
        PQ_PROT_LOGD("Load %s[%d]", name, (int)*var);
    }

public:
    CustParameters(const char *libPath, const char *libPath2 = NULL);

    ~CustParameters();

    bool isGood() {
        return (mLib != NULL || mLib2 != NULL);
    }

    void *getSymbol(const char *name);

    template <typename T>
    bool loadVar(const char *name, T* var, int index = 0) {
        if (index < 0)
            return false;

        void *symbolPtr = getSymbol(name);
        if (symbolPtr != NULL) {
            *var = static_cast<T*>(symbolPtr)[index];
            logLoadVar(name, var);
            return true;
        }

        return false;
    }

    template <typename T>
    bool loadArray(const char *name, T* var, unsigned int len, int index = 0) {
        if (index < 0)
            return false;

        void *symbolPtr = getSymbol(name);
        if (symbolPtr != NULL) {
            T *arrPtr = static_cast<T*>(symbolPtr) + len * index;
            memcpy(var, arrPtr, sizeof(T) * len);
            PQ_PROT_LOGD("Load %s[%d][]", name, index);
            return true;
        }

        return false;
    }

    void* getFunction(const char *name) {
        return getSymbol(name);
    }

    static CustParameters& getPQCust() {
        return mPQCust;
    }
};


template <>
inline void CustParameters::logLoadVar<int>(const char *name, const int* var) {
    PQ_PROT_LOGD("Load %s = %d", name, *var);
}

template <>
inline void CustParameters::logLoadVar<float>(const char *name, const float* var) {
    PQ_PROT_LOGD("Load %s = %f", name, *var);
}

#endif // __PQCOMMON_H__
