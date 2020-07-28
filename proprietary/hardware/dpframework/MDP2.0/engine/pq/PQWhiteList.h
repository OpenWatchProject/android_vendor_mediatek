#ifndef __PQ_WHITE_LIST_H__
#define __PQ_WHITE_LIST_H__

#include <utils/threads.h>


class PQWhiteList {
private:
    static PQWhiteList sInstance;

    mutable android::Mutex mLock;
    void *mWhiteListDL;
    void *mWLHandle;
    void *mPerfServiceDL;

    bool (*pWLGetAppValidName)(void *, char *);
    char* (*pPerfGetPackName)(void);

    // Should we enable PQ feature on active package?
    bool mIsPQEnabledOnActive;

    // No copy constructor
    PQWhiteList(PQWhiteList &) :
            mWhiteListDL(NULL), mWLHandle(NULL), mPerfServiceDL(NULL),
            pWLGetAppValidName(NULL), pPerfGetPackName(NULL),
            mIsPQEnabledOnActive(true)
    {
    }

    bool isEnabledOnPackage(const char *packName) const;

public:

    PQWhiteList(const char *feature);
    ~PQWhiteList();

    bool isAvailable() const {
        return (mWhiteListDL != NULL && mWLHandle != NULL && mPerfServiceDL != NULL &&
                pWLGetAppValidName != NULL && pPerfGetPackName != NULL);
    }

    // Whether PQ should be enabled on active package
    bool isPQEnabledOnActive() const {
        android::Mutex::Autolock _l(mLock);
        // We should avoid accessing services in onConfigFrame() due to the overhead.
        // So we cache the query result only on scenario switch.
        return mIsPQEnabledOnActive; // Updated in queryWhiteListConfig()
    }

    static PQWhiteList& getInstance() {
        return sInstance;
    }

    void queryWhiteListConfig();
};

#endif // __PQ_WHITE_LIST_H__
