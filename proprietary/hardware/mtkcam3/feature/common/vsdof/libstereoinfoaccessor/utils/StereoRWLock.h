#ifndef STEREO_RWLOCK_H
#define STEREO_RWLOCK_H

#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>
#include <utils/Errors.h>
#include <utils/ThreadDefs.h>

namespace stereo {

class StereoRWLock {

public:
    StereoRWLock();
    ~StereoRWLock();

    int readLock();
    int writeLock();
    int unlock();

private:
    // A RWLock cannot be copied
    StereoRWLock(const StereoRWLock&);
    StereoRWLock& operator = (const StereoRWLock&);

    pthread_rwlock_t mRWLock;
    pthread_mutex_t mMutexLock;
    int count;
};

inline StereoRWLock::StereoRWLock() : count(0) {
    pthread_rwlock_init(&mRWLock, NULL);
    pthread_mutex_init(&mMutexLock, NULL);
}

inline StereoRWLock::~StereoRWLock() {
    pthread_rwlock_destroy(&mRWLock);
    pthread_mutex_destroy(&mMutexLock);
}

inline int StereoRWLock::readLock() {
    if (0 == pthread_rwlock_rdlock(&mRWLock)) {
        pthread_mutex_lock(&mMutexLock);
        count++;
        pthread_mutex_unlock(&mMutexLock);
    }
    return count;
}

inline int StereoRWLock::writeLock() {
    if (0 == pthread_rwlock_wrlock(&mRWLock)) {
        pthread_mutex_lock(&mMutexLock);
        count++;
        pthread_mutex_unlock(&mMutexLock);
    }
    return count;
}

inline int StereoRWLock::unlock() {
    if (0 == pthread_rwlock_unlock(&mRWLock)) {
        pthread_mutex_lock(&mMutexLock);
        count--;
        pthread_mutex_unlock(&mMutexLock);
    }
    return count;
}

}

#endif
