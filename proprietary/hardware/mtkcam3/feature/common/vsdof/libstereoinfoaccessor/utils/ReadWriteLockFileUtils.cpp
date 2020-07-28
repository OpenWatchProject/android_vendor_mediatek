//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/ReadWriteLockFileUtils"

#include "ReadWriteLockFileUtils.h"
#include "StereoLog.h"
#include <utils/Trace.h>

using namespace stereo;
using namespace android;

#define ATRACE_TAG ATRACE_TAG_APP

StereoMap<StereoString, StereoRWLock*> ReadWriteLockFileUtils::sLockCollections;
static pthread_mutex_t sAccessLock = PTHREAD_MUTEX_INITIALIZER;

void ReadWriteLockFileUtils::readLock(const StereoString &filePath) {
    StereoLogD("<readLock>, filePath: %s", filePath.c_str());
    ATRACE_NAME(">>>>ReadWriteLockFileUtils-readLock");
    StereoRWLock *rwlock;
    pthread_mutex_lock(&sAccessLock);
    if (!sLockCollections.empty() && sLockCollections.find(filePath) != sLockCollections.end()) {
        rwlock = sLockCollections[filePath];
    } else {
        rwlock = new StereoRWLock();
        sLockCollections.insert(
            StereoMap<StereoString, StereoRWLock*> :: value_type(filePath, rwlock));
    }
    pthread_mutex_unlock(&sAccessLock);
    rwlock->readLock();
}

void ReadWriteLockFileUtils::writeLock(const StereoString &filePath) {
    StereoLogD("<writeLock>, filePath: %s", filePath.c_str());
    ATRACE_NAME(">>>>ReadWriteLockFileUtils-writeLock");
    StereoRWLock *rwlock;
    pthread_mutex_lock(&sAccessLock);
    if (!sLockCollections.empty() && sLockCollections.find(filePath) != sLockCollections.end()) {
        rwlock = sLockCollections[filePath];
    } else {
        rwlock = new StereoRWLock();
        sLockCollections.insert(
            StereoMap<StereoString, StereoRWLock*> :: value_type(filePath, rwlock));
    }
    pthread_mutex_unlock(&sAccessLock);
    rwlock->writeLock();
}

void ReadWriteLockFileUtils::unlock(const StereoString &filePath) {
    StereoLogD("<unlock>, filePath: %s", filePath.c_str());
    ATRACE_NAME(">>>>ReadWriteLockFileUtils-unlock");
    StereoRWLock *rwlock;
    pthread_mutex_lock(&sAccessLock);
    if (!sLockCollections.empty() && sLockCollections.find(filePath) != sLockCollections.end()) {
        rwlock = sLockCollections[filePath];
    } else {
        StereoLogD("not find rwlock, so cannot unlock");
        pthread_mutex_unlock(&sAccessLock);
        return;
    }
    pthread_mutex_unlock(&sAccessLock);
    if (rwlock->unlock() <= 0) {
        StereoLogD("delete rwlock");
        delete rwlock;
        sLockCollections.erase(filePath);
    }
}