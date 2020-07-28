#ifndef READ_WRITE_LOCK_FILE_UTILS_H
#define READ_WRITE_LOCK_FILE_UTILS_H

#include "types.h"
#include "StereoRWLock.h"

namespace stereo {

class ReadWriteLockFileUtils {

public:
    static void readLock(const StereoString &filePath);
    static void writeLock(const StereoString &filePath);
    static void unlock(const StereoString &filePath);
private:
    static StereoMap<StereoString, StereoRWLock*> sLockCollections;
};

}

#endif
