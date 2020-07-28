/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MY_RWLOCK_H
#define _MY_RWLOCK_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#if !defined(_WIN32)
# include <pthread.h>
#endif

#include <utils/Errors.h>
#include <utils/ThreadDefs.h>
#include <string>

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

#if !defined(_WIN32)

/*
 * Simple mutex class.  The implementation is system-dependent.
 *
 * The mutex must be unlocked by the thread that locked it.  They are not
 * recursive, i.e. the same thread can't lock it multiple times.
 */
class MyRWLock {
public:
    enum {
        PRIVATE = 0,
        SHARED = 1
    };

                MyRWLock();
                MyRWLock(const char* name);
                MyRWLock(int type, const char* name = NULL);
                ~MyRWLock();

    status_t    readLock();
    status_t    tryReadLock();
    status_t    writeLock();
    status_t    tryWriteLock();
    status_t    unlock();

    class AutoRLock {
    public:
        inline AutoRLock(MyRWLock& rwlock, const char *TAG=NULL) : mLock(rwlock), mTAG((TAG)?TAG:"")
        {
            if(mWriteTID == gettid() &&
               mWritePID == getpid())
            {
                if(mTAG.size() > 0) {
                    ALOGD("[%s] Ignore read lock", mTAG.c_str());
                } else {
                    ALOGD("Ignore read lock");
                }

                return;
            }

            status_t s = mLock.readLock();
            if(0 == s) {
                isLocked = true;
            } else {
                if(mTAG.size() > 0) {
                    ALOGE("[%s] Readlock fail, err %d: %s", mTAG.c_str(), s, strerror(s));
                } else {
                    ALOGE("Readlock fail, err %d: %s", s, strerror(s));
                }
            }
        }

        inline ~AutoRLock()
        {
            if(!isLocked) {
                return;
            }

            status_t s = mLock.unlock();
            if(0 != s) {
                if(mTAG.size() > 0) {
                    ALOGE("[%s] Unlock read lock fail, err %d: %s", mTAG.c_str(), s, strerror(s));
                } else {
                    ALOGE("Unlock read lock fail, err %d: %s", s, strerror(s));
                }
            }
        }
    private:
        MyRWLock& mLock;
        std::string mTAG;
        bool isLocked = false;
    };

    class AutoWLock {
    public:
        inline AutoWLock(MyRWLock& rwlock, const char *TAG=NULL) : mLock(rwlock), mTAG((TAG)?TAG:"")
        {
            status_t s = mLock.writeLock();
            if(0 == s) {
                mWriteTID = gettid();
                mWritePID = getpid();
            } else {
                if(mTAG.size() > 0) {
                    ALOGE("[%s] Write lock fail, err %d: %s", mTAG.c_str(), s, strerror(s));
                } else {
                    ALOGE("Write lock fail, err %d: %s", s, strerror(s));
                }
            }
        }

        inline ~AutoWLock()
        {
            status_t s = mLock.unlock();
            if(0 != s) {
                if(mTAG.size() > 0) {
                    ALOGE("[%s] Unlock write lock fail, err %d: %s", mTAG.c_str(), s, strerror(s));
                } else {
                    ALOGE("Unlock write lock fail, err %d: %s", s, strerror(s));
                }
            }

            mWriteTID = -1;
            mWritePID = -1;
        }
    private:
        MyRWLock& mLock;
        std::string mTAG;
    };

private:
    // A MyRWLock cannot be copied
                MyRWLock(const MyRWLock&);
   MyRWLock&      operator = (const MyRWLock&);

   pthread_rwlock_t mRWLock;

   static pid_t mWriteTID;
   static pid_t mWritePID;
};

inline MyRWLock::MyRWLock() {
    pthread_rwlock_init(&mRWLock, NULL);
}
inline MyRWLock::MyRWLock(__attribute__((unused)) const char* name) {
    pthread_rwlock_init(&mRWLock, NULL);
}
inline MyRWLock::MyRWLock(int type, __attribute__((unused)) const char* name) {
    if (type == SHARED) {
        pthread_rwlockattr_t attr;
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_rwlock_init(&mRWLock, &attr);
        pthread_rwlockattr_destroy(&attr);
    } else {
        pthread_rwlock_init(&mRWLock, NULL);
    }
}
inline MyRWLock::~MyRWLock() {
    pthread_rwlock_destroy(&mRWLock);
}
inline status_t MyRWLock::readLock() {
    return pthread_rwlock_rdlock(&mRWLock);
}
inline status_t MyRWLock::tryReadLock() {
    return pthread_rwlock_tryrdlock(&mRWLock);
}
inline status_t MyRWLock::writeLock() {
    return pthread_rwlock_wrlock(&mRWLock);
}
inline status_t MyRWLock::tryWriteLock() {
    return pthread_rwlock_trywrlock(&mRWLock);
}
inline status_t MyRWLock::unlock() {
    return pthread_rwlock_unlock(&mRWLock);
}

#endif // !defined(_WIN32)

// ---------------------------------------------------------------------------
}; // namespace android
// ---------------------------------------------------------------------------

#endif // _LIBS_UTILS_RWLOCK_H
