/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "ULogFile"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <utils/AndroidThreads.h>
#include <cutils/properties.h>
#include <log/log.h>
#include <mtkcam/utils/std/Log.h>
#include "ULogInt.h"
#include "ULogTable.h"
#include "ULogFile.h"
#include "ULogRTDiag.h"


static const char * const ULOG_FOLDERPATH_PROP_NAME = "vendor.debug.camera.ulog.folder";
static const char * const ULOG_FOLDERPATH_DEFAULT = "/sdcard/debuglogger/cam_ulog";
static const char * const ULOG_PASSIVE_MAX_BUFFER_KB_PROP_NAME = "vendor.debug.camera.ulog.maxkb";
static const char * const ULOG_FILE_HOOK_NE_PROP_NAME = "vendor.debug.camera.ulog.hookne";
static const char * const ULOG_FILE_MONOCLOCK_PROP_NAME = "vendor.debug.camera.ulog.monoclock";
static const char * const ULOG_FILE_FLUSHMS_PROP_NAME = "vendor.debug.camera.ulog.flushms";


// Flush every second by default. However, this is only an active wake-up period.
// When a buffer is full, the flush thread will be also informed to wake up.
static const int ULOG_FILE_DEFAULT_FLUSHMS = 1000;

// If the buffer is under writing when flush, ULog file will peek the buffer status frequently
static const int ULOG_WAIT_WRITING_INTERVAL_MS = 40;


#define FULOG_DBG(fmt, arg...) do { } while(0)
// #define FULOG_DBG(fmt, arg...) ULOG_IMP_LOGD("[%s]" fmt, __func__, ##arg)


namespace NSCam {
namespace Utils {
namespace ULog {
namespace File {

FileULogWriter::FileULogWriter() :
    mBytesWritten(0), mFd(-1), mInSignal(false), mFileSerial(1)
{
    mPid = static_cast<int>(getpid());
    mFileTag[0] = '\0';
    mSessionTimestamp[0] = '\0';
    mBufferFilled = 0;
    mBufferEmpty = getBufferBegin();
}


void FileULogWriter::reserveSpace(bool inSignal)
{
    char filePath[512];
    snprintf(filePath, sizeof(filePath), "%s/", mFolderPath.c_str());

    struct statvfs statOfStorage;
    if (statvfs(filePath, &statOfStorage) != 0)
        return;

    const std::int64_t TO_RESERVE = MAX_FILE_SIZE + 100 * 1024 * 1024;
    std::int64_t freeSpace =
        static_cast<std::int64_t>(statOfStorage.f_bsize) * static_cast<std::int64_t>(statOfStorage.f_bfree);
    if (freeSpace > TO_RESERVE)
        return;

    DIR *dirp = opendir(filePath); // Not guarantee signal-safe but we try
    if (dirp == NULL) {
        if (!inSignal) {
            ULOG_IMP_LOGE("Open folder %s failed", filePath);
        }
        return;
    }

    struct FileInfo {
        off_t st_size;
        time_t mtime_tv_sec;
        std::string filePath;

        inline FileInfo(off_t size, time_t mtime, const char *pathStr) :
            st_size(size), mtime_tv_sec(mtime), filePath(pathStr)
        {
        }
    };
    std::vector<std::unique_ptr<FileInfo>> fileList;

    struct stat fileStat;
    for (;;) {
        struct dirent *direntp = readdir(dirp);
        if (direntp == NULL)
            break;

        snprintf(filePath, sizeof(filePath), "%s/%s", mFolderPath.c_str(), direntp->d_name);
        if (stat(filePath, &fileStat) == 0) {
            if (S_ISREG(fileStat.st_mode)) {
                std::unique_ptr<FileInfo> fileInfo =
                    std::make_unique<FileInfo>(fileStat.st_size, fileStat.st_mtim.tv_sec, filePath);
                fileList.emplace_back(std::move(fileInfo));
            }
        }
    }

    closedir(dirp);

    std::sort(fileList.begin(), fileList.end(), [] (std::unique_ptr<FileInfo> &a, std::unique_ptr<FileInfo> &b) {
        return (a->mtime_tv_sec < b->mtime_tv_sec);
    });

    // Unlink oldest until sufficient space
    for (std::unique_ptr<FileInfo> &fileInfo : fileList) {
        if (unlink(fileInfo->filePath.c_str()) == 0) {
            if (!inSignal) {
                ULOG_IMP_LOGW("Storage full. Unlinked: %s", fileInfo->filePath.c_str());
            }
            freeSpace += static_cast<decltype(freeSpace)>(fileInfo->st_size);
            if (freeSpace > TO_RESERVE)
                break;
        }
    }
}


bool FileULogWriter::open(bool newSession, const char *fileTag, bool inSignal)
{
    if (newSession) {
        timespec timeStamp;
        clock_gettime(ANDROID_CLOCK_ID, &timeStamp);
        struct tm logTm;
        strftime(mSessionTimestamp, sizeof(mSessionTimestamp), "%Y_%m%d_%H%M%S", ulocaltime_r(&timeStamp.tv_sec, &logTm));
        mFileSerial = 1;
        if (fileTag != NULL) {
            strncpy(mFileTag, fileTag, sizeof(mFileTag));
            mFileTag[sizeof(mFileTag) - 1] = '\0';
        }
        mInSignal = inSignal;
    }

    reserveSpace(mInSignal);

    char filePath[512];
    filePath[0] = '\0'; // To avoid Coverity warning
    auto openNewFile = [&] () {
        ::snprintf(filePath, sizeof(filePath), "%s/cam_ulog_%d_%s%s_%02d.txt",
            mFolderPath.c_str(), getPid(), mFileTag, mSessionTimestamp, mFileSerial);
        int fd = ::open(filePath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd >= 0) {
            mFilePath = filePath;
        }
        return fd;
    };

    mFd = openNewFile();
    if (mFd < 0) {
        if (!mInSignal) {
            ULOG_IMP_LOGE("FileULogWriter: file open failed %s", filePath);
        }

        mFolderPath = "/data/vendor";
        mFd = openNewFile();
        if (mFd < 0) {
            if (!mInSignal) {
                ULOG_IMP_LOGE("Unable to open file. Please disable SELinux and restart camerahalserver. !!!");
            }
            return false;
        }
    }

    if (!mInSignal) {
        ULOG_IMP_LOGI("Write ulog to %s", filePath);
    }

    mFileSerial++;
    mBufferFilled = 0;
    mBufferEmpty = getBufferBegin();
    mBytesWritten = 0;

    return true;
}


void FileULogWriter::flush(bool deepFlush)
{
    if (mBufferFilled > 0 && mFd >= 0) {
        ::write(mFd, getBufferBegin(), mBufferFilled);
        if (deepFlush)
            ::fsync(mFd);
    }
    mBufferFilled = 0;
    mBufferEmpty = getBufferBegin();
}


void FileULogWriter::close()
{
    if (mFd >= 0) {
        flush(false);
        ::close(mFd);
        mFd = -1;
    }
}


inline void FileULogWriter::write(size_t size, bool isCompleted)
{
    mBufferFilled += size;
    mBufferEmpty += size;
    if (__unlikely(mBufferFilled + __CAM_ULOG_DETAILS_BUFFER_SIZE__ + 100 >= BUFFER_SIZE)) {
        flush(false);
    }

    mBytesWritten += size;
    if (__unlikely(mBytesWritten > MAX_FILE_SIZE && isCompleted)) {
        close();
        open(false);

        if (mFd >= 0) {
            static const char * const BEGIN_TAG = "-- CONTINUED --\n";
            ::write(mFd, const_cast<char*>(BEGIN_TAG), strlen(BEGIN_TAG));
        }
    }
}


#define HEADER_FORMAT_D ".%06ld %5d %5d D ULog    : "
#define HEADER_FORMAT_I ".%06ld %5d %5d I ULog    : "
#define HEADER_ARGUMENTS(logHeader, pid) (logHeader->timeStamp.tv_nsec / 1000), (pid), logHeader->tid
#define HEADER_FORMAT_TAG ".%06ld %5d %5d %c %-8s: "


void FileULogWriter::onLogEnter(const LogEnter *log)
{
    struct tm logTm;
    write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

    size_t nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_D "R %s:%u M[%s:%x] +  :%s #%u\n",
        HEADER_ARGUMENTS(log, getPid()), getRequestTypeName(log->requestTypeId), log->requestSerial,
        getModuleName(log->moduleId), log->moduleId, log->tag, log->logSerial);

    write(nPrinted, true);
}


void FileULogWriter::onLogExit(const LogExit *log)
{
    struct tm logTm;
    write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

    size_t nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_D "R %s:%u M[%s:%x] -  :%s #%u\n",
        HEADER_ARGUMENTS(log, getPid()), getRequestTypeName(log->requestTypeId), log->requestSerial,
        getModuleName(log->moduleId), log->moduleId, log->tag, log->logSerial);

    write(nPrinted, true);
}


void FileULogWriter::onLogDiscard(const LogDiscard *log)
{
    int nRequests = log->numOfRequestSerials;

    if (nRequests == 1) {
        struct tm logTm;
        write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

        size_t nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_D "R %s:%u M[%s:%x] - (discard)  :%s #%u\n",
            HEADER_ARGUMENTS(log, getPid()),
            getRequestTypeName(log->requestTypeId), log->requestSerialList[0],
            getModuleName(log->moduleId), log->moduleId, log->tag, log->logSerial);
        write(nPrinted, true);
    } else {
        char reqBuffer[256]; // can be optimized

        const RequestSerial *requestSerialList = log->requestSerialList;
        while (nRequests > 0) {
            struct tm logTm;
            write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

            size_t nReqPrinted = printIntArray(reqBuffer, sizeof(reqBuffer), requestSerialList, nRequests);
            nRequests -= nReqPrinted;
            requestSerialList += nReqPrinted;
            size_t nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_D "R %s:%s M[%s:%x] - (discard) cont:%d  :%s #%u\n",
                HEADER_ARGUMENTS(log, getPid()),
                getRequestTypeName(log->requestTypeId), reqBuffer,
                getModuleName(log->moduleId), log->moduleId, nRequests, log->tag, log->logSerial);
            write(nPrinted, true);
        }
    }
}


void FileULogWriter::onLogSubreqs(const LogSubreqs *log)
{
    int nSubreqs = log->numOfSubreqSerials;

    if (nSubreqs == 1) {
        struct tm logTm;
        write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));
        size_t nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_D "R %s:%u -> R %s:%u genOn M[%s:%x]  :%s #%u\n",
            HEADER_ARGUMENTS(log, getPid()),
            getRequestTypeName(log->requestTypeId), log->requestSerial,
            getRequestTypeName(log->subrequestTypeId), log->subrequestSerialList[0],
            getModuleName(log->moduleId), log->moduleId, log->tag, log->logSerial);
        write(nPrinted, true);
    } else {
        char reqBuffer[256];
        struct tm logTm;

        const RequestSerial *subrequestSerialList = log->subrequestSerialList;
        while (nSubreqs > 0) {
            write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

            size_t nReqPrinted = printIntArray(reqBuffer, sizeof(reqBuffer), log->subrequestSerialList, nSubreqs);
            nSubreqs -= nReqPrinted;
            subrequestSerialList += nReqPrinted;
            size_t nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_D "R %s:%u -> R %s:%s genOn M[%s:%x] cont:%d  :%s #%u\n",
                HEADER_ARGUMENTS(log, getPid()),
                getRequestTypeName(log->requestTypeId), log->requestSerial,
                getRequestTypeName(log->subrequestTypeId), reqBuffer + 1,
                getModuleName(log->moduleId), log->moduleId, nSubreqs, log->tag, log->logSerial);
            write(nPrinted, true);
        }
    }
}


void FileULogWriter::writeModuleList(const ModuleId *coModuleList, int nModules)
{
    while (nModules > 0) {
        if (nModules >= 3) {
            size_t nPrinted = snprintf(getBuffer(), getBufferSize(), " M[%s] M[%s] M[%s]",
                getModuleName(coModuleList[0]), getModuleName(coModuleList[1]), getModuleName(coModuleList[2]));
            write(nPrinted);
            nModules -= 3;
            coModuleList += 3;
        } else if (nModules >= 2) {
            size_t nPrinted = snprintf(getBuffer(), getBufferSize(), " M[%s] M[%s]",
                getModuleName(coModuleList[0]), getModuleName(coModuleList[1]));
            write(nPrinted);
            nModules -= 2;
            coModuleList += 2;
        } else if (nModules >= 1) {
            size_t nPrinted = snprintf(getBuffer(), getBufferSize(), " M[%s]", getModuleName(coModuleList[0]));
            write(nPrinted);
            nModules -= 1;
            coModuleList += 1;
        }
    }
}


void FileULogWriter::onLogPathDiv(const LogPathDiv *log)
{
    struct tm logTm;
    write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

    size_t nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_D "R %s:%u  M[%s:%x] divTo",
        HEADER_ARGUMENTS(log, getPid()),
        getRequestTypeName(log->requestTypeId), log->requestSerial,
        getModuleName(log->moduleId), log->moduleId);
    write(nPrinted);

    writeModuleList(log->coModuleList, log->numOfCoModules);

    nPrinted = snprintf(getBuffer(), getBufferSize(), "  :%s\n", log->tag);
    write(nPrinted, true);
}


void FileULogWriter::onLogPathJoin(const LogPathJoin *log)
{
    struct tm logTm;
    write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

    size_t nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_D "R %s:%u  M[%s:%x] joinFrom",
        HEADER_ARGUMENTS(log, getPid()),
        getRequestTypeName(log->requestTypeId), log->requestSerial,
        getModuleName(log->moduleId), log->moduleId);
    write(nPrinted);

    writeModuleList(log->coModuleList, log->numOfCoModules);

    nPrinted = snprintf(getBuffer(), getBufferSize(), "  :%s\n", log->tag);
    write(nPrinted, true);
}


void FileULogWriter::onLogFunc(const LogFunc *log)
{
    struct tm logTm;
    write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

    size_t nPrinted = 0;
    if ((log->lifeTag & ~EXIT_BIT) == API_ENTER) {
        nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_TAG "[%s/%s] %c  ULog#%u\n",
            HEADER_ARGUMENTS(log, getPid()), 'D', log->tag, getModuleName(log->moduleId), log->funcName,
            ((log->lifeTag & EXIT_BIT) ? '-' : '+'), log->logSerial);
    } else {
        nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_TAG "[%s] %c  ULog#%u\n",
            HEADER_ARGUMENTS(log, getPid()), 'D', log->tag, log->funcName,
            ((log->lifeTag & EXIT_BIT) ? '-' : '+'), log->logSerial);
    }

    write(nPrinted, true);
}


void FileULogWriter::onLogFuncExt(const LogFuncExt *log)
{
    struct tm logTm;
    write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

    size_t nPrinted = 0;
    if ((log->lifeTag & ~EXIT_BIT) == API_ENTER) {
        nPrinted = snprintf(getBuffer(), getBufferSize(),
            HEADER_FORMAT_TAG "[%s/%s] %c (0x%" PRIxPTR ",0x%" PRIxPTR ",0x%" PRIxPTR ") ULog#%u\n",
            HEADER_ARGUMENTS(log, getPid()), 'D', log->tag, getModuleName(log->moduleId), log->funcName,
            ((log->lifeTag & EXIT_BIT) ? '-' : '+'),
            log->values[0], log->values[1], log->values[2],
            log->logSerial);
    } else {
        nPrinted = snprintf(getBuffer(), getBufferSize(),
            HEADER_FORMAT_TAG "[%s] %c (0x%" PRIxPTR ",0x%" PRIxPTR ",0x%" PRIxPTR ") ULog#%u\n",
            HEADER_ARGUMENTS(log, getPid()), 'D', log->tag, log->funcName,
            ((log->lifeTag & EXIT_BIT) ? '-' : '+'),
            log->values[0], log->values[1], log->values[2],
            log->logSerial);
    }

    write(nPrinted, true);
}


void FileULogWriter::onLogDetails(const LogDetails *log)
{
    struct tm logTm;
    write(strftime(getBuffer(), getBufferSize(), ULOG_TIME_FORMAT, ulocaltime_r(&log->timeStamp.tv_sec, &logTm)));

    char levelChar = 'D';
    switch (log->detailsType) {
    case DETAILS_ERROR:   levelChar = 'E'; break;
    case DETAILS_WARNING: levelChar = 'W'; break;
    case DETAILS_INFO:    levelChar = 'I'; break;
    case DETAILS_DECISION_KEY:
    case DETAILS_DEBUG:   levelChar = 'D'; break;
    default:              levelChar = 'V'; break;
    }

    size_t nPrinted = snprintf(getBuffer(), getBufferSize(), HEADER_FORMAT_TAG "%s\n",
            HEADER_ARGUMENTS(log, getPid()), levelChar, log->tag, log->content);

    write(nPrinted, true);
}


void FileULogWriter::writeString(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    size_t nPrinted = vsnprintf(getBuffer(), getBufferSize(), format, args);
    va_end(args);

    write(nPrinted, true);
}


FileULoggerBase::FileULoggerBase() :
    mClockId(ANDROID_CLOCK_ID), mContinueWorking(false), mLogSerial(0), mNeHookLevel(0), mSigWriter(nullptr)
{
    mActiveBuffer = std::make_unique<Buffer>();

    if (property_get_int32(ULOG_FILE_MONOCLOCK_PROP_NAME, 0) == 1) {
        mClockId = CLOCK_MONOTONIC;
    }

    memset(&mMySigAbrtAction, 0, sizeof(struct sigaction));
    memset(&mMySigSegvAction, 0, sizeof(struct sigaction));
    memset(&mMySigBusAction, 0, sizeof(struct sigaction));
    memset(&mOldSigAbrtAction, 0, sizeof(struct sigaction));
    memset(&mOldSigSegvAction, 0, sizeof(struct sigaction));
    memset(&mOldSigBusAction, 0, sizeof(struct sigaction));
}


void FileULoggerBase::getFolderPath(char *folderPath, size_t bufferSize)
{
    property_get(ULOG_FOLDERPATH_PROP_NAME, folderPath, ULOG_FOLDERPATH_DEFAULT);

    mode_t mode = S_IRWXU | S_IRWXG | S_IXOTH;
    int ret = mkdir(folderPath, mode);
    if (ret == 0 || errno == EEXIST)
        return;

    ULOG_IMP_LOGE("mkdir(%s) failed, errno = %d", folderPath, errno);

    auto tryMkDir = [folderPath, bufferSize] (const char *candidatePath) {
        strncpy(folderPath, candidatePath, bufferSize);
        int ret = mkdir(folderPath, S_IRWXU | S_IRWXG | S_IXOTH);
        if (ret == 0 || errno == EEXIST) {
            ULOG_IMP_LOGD("mkdir(%s) OK", folderPath);
            return true;
        }
        return false;
    };

    if (tryMkDir("/sdcard/cam_ulog"))
        return;

    if (tryMkDir("/data/vendor/cam_ulog"))
        return;

    if (tryMkDir("/data/cam_ulog"))
        return;

    strncpy(folderPath, "/data", bufferSize);
}


char* FileULoggerBase::getTimeStamp(clockid_t clockId, char *buffer, size_t bufferSize)
{
    char *bufferTail = buffer;

    timespec timeStamp;
    clock_gettime(clockId, &timeStamp);
    struct tm logTm;
    size_t secLen = strftime(bufferTail, bufferSize, ULOG_TIME_FORMAT, ulocaltime_r(&timeStamp.tv_sec, &logTm));
    bufferTail += secLen;
    bufferSize -= secLen;
    snprintf(bufferTail, bufferSize, ".%06ld", (timeStamp.tv_nsec / 1000));

    return buffer;
}


inline std::unique_ptr<FileULoggerBase::Buffer> FileULoggerBase::allocEmptyBuffer()
{
    std::unique_ptr<Buffer> empty;

    if (mEmptyBuffers.empty()) {
        empty = std::make_unique<Buffer>();
        std::atomic_thread_fence(std::memory_order_release);
    } else {
        // LIFO for page locality
        empty = std::move(mEmptyBuffers.back());
        mEmptyBuffers.pop_back();
    }

    return empty;
}


bool FileULoggerBase::allocLogSpace(size_t size, LogHeader **logSpace, Buffer **activeBuffer, unsigned int *logSerial)
{
    if (sizeof(void*) == 4) // compile-time optimized
        size = (size + 3) & ~(0x3);
    else
        size = (size + 7) & ~(0x7);

    if (__unlikely(size > BUFFER_SIZE)) {
        ULOG_IMP_LOGE("Log size too large: %zu", size);
        return false;
    }

    LogHeader *header = nullptr;

    {
        // mBufferMutex is only to protect space allocation
        // Writing can be done simutaneously, but syncronized with mNumOfWriting
        std::lock_guard<std::mutex> lock(mBufferMutex);

        if (__unlikely(mActiveBuffer->mEmptySize < size)) {
            mActiveBuffer->mIsTerminated.store(true, std::memory_order_relaxed);
            mToBeFlushed.emplace_back(std::move(mActiveBuffer));
            mActiveBuffer = allocEmptyBuffer();
            mBufferCond.notify_one();
        }

        header = reinterpret_cast<LogHeader*>(mActiveBuffer->mNextEmpty);
        header->type = INVALID;
        header->size = size;
        // For NE hook, I hope the INVALID can be effective first
        // But it is not economical to add release fence here
        *activeBuffer = mActiveBuffer.get();
        // coverity[side_effect_free : FALSE]
        mActiveBuffer->mNumOfWriting.fetch_add(1, std::memory_order_relaxed);
        mActiveBuffer->mNextEmpty += size;
        mActiveBuffer->mEmptySize -= size;

        if (logSerial != nullptr)
            *logSerial = getLogSerial();
    }

    header->tid = getTid();
    clock_gettime(mClockId, &(header->timeStamp));

    *logSpace = header;

    FULOG_DBG("size = %zu, %p of buffer %p", size, *logSpace, *activeBuffer);

    return true;
}


inline void FileULoggerBase::writeLogDone(Buffer *activeBuffer, LogHeader *header, LogType type)
{
    if (__unlikely(mNeHookLevel > 0)) {
        std::atomic_thread_fence(std::memory_order_release);
    }

    header->type = type; // was INVALID

    if (__unlikely(
        activeBuffer->mNumOfWriting.fetch_sub(1, std::memory_order_release) == 1 &&
        activeBuffer->mIsTerminated.load(std::memory_order_relaxed)))
    {
        // The last log to be written in the buffer, wake up flush thread
        mBufferCond.notify_one();
    }
}


inline size_t FileULoggerBase::safeStrlen(const char *str, size_t maxLen)
{
    size_t n = 0;
    const char *c = str;
    while (*c != '\0' && n < maxLen) { // Manually unroll
        n++; c++;
        if (*c == '\0') break;
        n++; c++;
        if (*c == '\0') break;
        n++; c++;
        if (*c == '\0') break;
        n++; c++;
    }

    return (n < maxLen) ? n : maxLen;
}


inline size_t FileULoggerBase::safeStrncpy(char *dest, const char *src, size_t size)
{
    // We have calculated the string length in size, use memcpy() for efficiency
    memcpy(dest, src, size);
    dest[size - 1] = '\0';
    // coverity[overflow_sink : FALSE]
    return size - 1;
}


void FileULoggerBase::onLogEnter(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial)
{
    LogHeader *dataBuffer = nullptr;
    Buffer *activeBuffer;
    size_t tagSize = safeStrlen(tag) + 1;
    unsigned int logSerial = 0;

    if (__unlikely( !allocLogSpace(sizeof(LogEnter) + tagSize, &dataBuffer, &activeBuffer, &logSerial) ))
        return;

    LogEnter *log = reinterpret_cast<LogEnter*>(dataBuffer);
    log->logSerial = logSerial;
    log->moduleId = moduleId;
    log->requestSerial = requestSerial;
    log->requestTypeId = requestTypeId;
    // We have allocated sufficient space for tag
    safeStrncpy(log->tag, tag, tagSize);

    writeLogDone(activeBuffer, dataBuffer, ENTER);
}


void FileULoggerBase::onLogExit(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial)
{
    LogHeader *dataBuffer = nullptr;
    Buffer *activeBuffer;
    size_t tagSize = safeStrlen(tag) + 1;
    unsigned int logSerial = 0;

    if (__unlikely( !allocLogSpace(sizeof(LogExit) + tagSize, &dataBuffer, &activeBuffer, &logSerial) ))
        return;

    LogExit *log = reinterpret_cast<LogExit*>(dataBuffer);
    log->logSerial = logSerial;
    log->moduleId = moduleId;
    log->requestSerial = requestSerial;
    log->requestTypeId = requestTypeId;
    // We have allocated sufficient space for tag
    safeStrncpy(log->tag, tag, tagSize);

    writeLogDone(activeBuffer, dataBuffer, EXIT);
}


void FileULoggerBase::onLogDiscard(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, const RequestSerial *requestSerialList, size_t n)
{
    LogHeader *dataBuffer = nullptr;
    Buffer *activeBuffer;
    size_t tagSize = safeStrlen(tag) + 1;
    size_t reqListSize = sizeof(RequestSerial) * n;
    unsigned int logSerial = 0;

    if (__unlikely( !allocLogSpace(sizeof(LogDiscard) + reqListSize + tagSize, &dataBuffer, &activeBuffer, &logSerial) ))
        return;

    LogDiscard *log = reinterpret_cast<LogDiscard*>(dataBuffer);
    log->logSerial = logSerial;
    log->moduleId = moduleId;
    log->requestTypeId = requestTypeId;
    log->numOfRequestSerials = n;
    static_assert(sizeof(log->requestSerialList[0]) == sizeof(requestSerialList[0]),
        "Type of requestSerialList should be the same for memcpy");
    memcpy(log->requestSerialList, requestSerialList, sizeof(log->requestSerialList[0]) * n);
    log->tag = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(log) + sizeof(LogDiscard) + reqListSize);
    safeStrncpy(log->tag, tag, tagSize);

    writeLogDone(activeBuffer, dataBuffer, DISCARD);
}


void FileULoggerBase::onLogSubreqs(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
    RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n)
{
    LogHeader *dataBuffer = nullptr;
    Buffer *activeBuffer;
    size_t tagSize = safeStrlen(tag) + 1;
    size_t subreqListSize = sizeof(RequestSerial) * n;
    unsigned int logSerial = 0;

    if (__unlikely( !allocLogSpace(sizeof(LogSubreqs) + subreqListSize + tagSize, &dataBuffer, &activeBuffer, &logSerial) ))
        return;

    LogSubreqs *log = reinterpret_cast<LogSubreqs*>(dataBuffer);
    log->logSerial = logSerial;
    log->moduleId = moduleId;
    log->requestTypeId = requestTypeId;
    log->requestSerial = requestSerial;
    log->subrequestTypeId = subrequestTypeId;
    log->numOfSubreqSerials = n;
    static_assert(sizeof(log->subrequestSerialList[0]) == sizeof(subrequestSerialList[0]),
        "Type of subrequestSerialList should be the same for memcpy");
    memcpy(log->subrequestSerialList, subrequestSerialList, sizeof(log->subrequestSerialList[0]) * n);
    log->tag = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(log) + sizeof(LogSubreqs) + subreqListSize);
    safeStrncpy(log->tag, tag, tagSize);

    writeLogDone(activeBuffer, dataBuffer, SUBREQS);
}


void FileULoggerBase::onLogPathDiv(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
    const ModuleId *toModuleIdList, size_t n)
{
    LogHeader *dataBuffer = nullptr;
    Buffer *activeBuffer;
    size_t tagSize = safeStrlen(tag) + 1;
    size_t coModuleListSize = sizeof(ModuleId) * n;

    if (__unlikely( !allocLogSpace(sizeof(LogPathDiv) + coModuleListSize + tagSize, &dataBuffer, &activeBuffer, nullptr) ))
        return;

    LogPathDiv *log = reinterpret_cast<LogPathDiv*>(dataBuffer);
    log->moduleId = moduleId;
    log->requestTypeId = requestTypeId;
    log->requestSerial = requestSerial;
    log->numOfCoModules = n;
    static_assert(sizeof(log->coModuleList[0]) == sizeof(toModuleIdList[0]),
        "Type of coModuleList should be the same for memcpy");
    memcpy(log->coModuleList, toModuleIdList, sizeof(log->coModuleList[0]) * n);
    log->tag = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(log) + sizeof(LogPathDiv) + coModuleListSize);
    safeStrncpy(log->tag, tag, tagSize);

    writeLogDone(activeBuffer, dataBuffer, PATH_DIV);
}


void FileULoggerBase::onLogPathJoin(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
    const ModuleId *fromModuleIdList, size_t n)
{
    LogHeader *dataBuffer = nullptr;
    Buffer *activeBuffer;
    size_t tagSize = safeStrlen(tag) + 1;
    size_t coModuleListSize = sizeof(ModuleId) * n;

    if (__unlikely( !allocLogSpace(sizeof(LogPathJoin) + coModuleListSize + tagSize, &dataBuffer, &activeBuffer, nullptr) ))
        return;

    LogPathJoin *log = reinterpret_cast<LogPathJoin*>(dataBuffer);
    log->moduleId = moduleId;
    log->requestTypeId = requestTypeId;
    log->requestSerial = requestSerial;
    log->numOfCoModules = n;
    static_assert(sizeof(log->coModuleList[0]) == sizeof(fromModuleIdList[0]),
        "Type of coModuleList should be the same for memcpy");
    memcpy(log->coModuleList, fromModuleIdList, sizeof(log->coModuleList[0]) * n);
    log->tag = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(log) + sizeof(LogPathJoin) + coModuleListSize);
    safeStrncpy(log->tag, tag, tagSize);

    writeLogDone(activeBuffer, dataBuffer, PATH_JOIN);
}


void FileULoggerBase::onLogFuncLife(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag)
{
    LogHeader *dataBuffer = nullptr;
    Buffer *activeBuffer;
    size_t tagSize = safeStrlen(tag) + 1;
    // Can we hold the pointer of funcName instead of content copy?
    // Yes, but can not improve significantly and will be unsafe some day.
    size_t funcNameSize = safeStrlen(funcName) + 1;
    unsigned int logSerial = 0;

    if (__unlikely( !allocLogSpace(sizeof(LogFunc) + tagSize + funcNameSize, &dataBuffer, &activeBuffer, &logSerial) ))
        return;

    LogFunc *log = reinterpret_cast<LogFunc*>(dataBuffer);
    log->logSerial = logSerial;
    log->moduleId = moduleId;
    log->lifeTag = lifeTag;
    safeStrncpy(log->tag, tag, tagSize);
    log->funcName = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(log) + sizeof(LogFunc) + tagSize);
    safeStrncpy(log->funcName, funcName, funcNameSize);

    writeLogDone(activeBuffer, dataBuffer, FUNC_LIFE);
}


void FileULoggerBase::onLogFuncLifeExt(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag,
    std::intptr_t v1, std::intptr_t v2, std::intptr_t v3)
{
    LogHeader *dataBuffer = nullptr;
    Buffer *activeBuffer;
    size_t tagSize = safeStrlen(tag) + 1;
    size_t funcNameSize = safeStrlen(funcName) + 1;
    unsigned int logSerial = 0;

    if (__unlikely( !allocLogSpace(sizeof(LogFuncExt) + tagSize + funcNameSize, &dataBuffer, &activeBuffer, &logSerial) ))
        return;

    LogFuncExt *log = reinterpret_cast<LogFuncExt*>(dataBuffer);
    log->logSerial = logSerial;
    log->moduleId = moduleId;
    log->values[0] = v1;
    log->values[1] = v2;
    log->values[2] = v3;
    log->lifeTag = lifeTag;
    safeStrncpy(log->tag, tag, tagSize);
    log->funcName = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(log) + sizeof(LogFuncExt) + tagSize);
    safeStrncpy(log->funcName, funcName, funcNameSize);

    writeLogDone(activeBuffer, dataBuffer, FUNC_LIFE_EXT);
}


void FileULoggerBase::onLogDetails(ModuleId moduleId, const char *tag, DetailsType type,
    const char *content, size_t contentLen)
{
    LogHeader *dataBuffer = nullptr;
    Buffer *activeBuffer;
    size_t tagSize = safeStrlen(tag) + 1;
    size_t contentSize = contentLen + 1;
    if (__unlikely(contentSize > __CAM_ULOG_DETAILS_BUFFER_SIZE__))
        contentSize = __CAM_ULOG_DETAILS_BUFFER_SIZE__;

    if (__unlikely( !allocLogSpace(sizeof(LogDetails) + tagSize + contentSize, &dataBuffer, &activeBuffer, nullptr) ))
        return;

    LogDetails *log = reinterpret_cast<LogDetails*>(dataBuffer);
    log->moduleId = moduleId;
    log->detailsType = type;
    safeStrncpy(log->tag, tag, tagSize);
    log->content = reinterpret_cast<char*>(reinterpret_cast<intptr_t>(log) + sizeof(LogDetails) + tagSize);
    safeStrncpy(log->content, content, contentSize);

    writeLogDone(activeBuffer, dataBuffer, DETAILS);
}


inline void FileULoggerBase::writeTo(FileULogWriter &writer, const LogHeader *header)
{
    switch (header->type) {
    case ENTER: {
        const LogEnter *logEnter = static_cast<const LogEnter*>(header);
        writer.onLogEnter(logEnter);
        break;
        }
    case EXIT: {
        const LogExit *logExit = static_cast<const LogExit*>(header);
        writer.onLogExit(logExit);
        break;
        }
    case DISCARD: {
        const LogDiscard *logDiscard = static_cast<const LogDiscard*>(header);
        writer.onLogDiscard(logDiscard);
        break;
        }
    case SUBREQS: {
        const LogSubreqs *logSubreqs = static_cast<const LogSubreqs*>(header);
        writer.onLogSubreqs(logSubreqs);
        break;
        }
    case PATH_DIV: {
        const LogPathDiv *logPathDiv = static_cast<const LogPathDiv*>(header);
        writer.onLogPathDiv(logPathDiv);
        break;
        }
    case PATH_JOIN: {
        const LogPathJoin *logPathJoin = static_cast<const LogPathJoin*>(header);
        writer.onLogPathJoin(logPathJoin);
        break;
        }
    case FUNC_LIFE: {
        const LogFunc *logFunc = static_cast<const LogFunc*>(header);
        writer.onLogFunc(logFunc);
        break;
        }
    case FUNC_LIFE_EXT: {
        const LogFuncExt *logFuncExt = static_cast<const LogFuncExt*>(header);
        writer.onLogFuncExt(logFuncExt);
        break;
        }
    case DETAILS: {
        const LogDetails *logDetails = static_cast<const LogDetails*>(header);
        writer.onLogDetails(logDetails);
        break;
        }
    default:
        break;
    }

}


FileULogger::FileULogger() : mWriter(), mIsFlushing(false)
{
}


void FileULogger::onInit()
{
    char folderPath[128];
    getFolderPath(folderPath, sizeof(folderPath));
    mWriter.setFolder(folderPath);

    mWriter.open(true);
    mContinueWorking.store(true, std::memory_order_release);
    mFlushThread = std::thread(&FileULogger::flushThreadEntry, this);

    initHook();
}


void FileULogger::onUninit()
{
    uninitHook();

    if (mContinueWorking.load(std::memory_order_relaxed)) {
        {
            std::lock_guard<std::mutex> lock(mBufferMutex);
            mContinueWorking.store(false, std::memory_order_release);
            mBufferCond.notify_all();
        }

        mFlushThread.join();

        std::lock_guard<std::mutex> lock(mBufferMutex);
        if (mIsFlushing) {
            mIsFlushing = false;
            mFlushDone.notify_all();
        }

        mWriter.close();
    }
}


void FileULogger::onFlush(int waitDoneSec)
{
    std::unique_lock<std::mutex> lock(mBufferMutex);

    mIsFlushing = true;
    if (mActiveBuffer->hasData()) {
        mActiveBuffer->mIsTerminated.store(true, std::memory_order_relaxed);
        mToBeFlushed.emplace_back(std::move(mActiveBuffer));
        mActiveBuffer = allocEmptyBuffer();
    }

    mBufferCond.notify_all();

    if (waitDoneSec > 0 && mFlushThread.joinable()) {
        for (int n = 0; mIsFlushing && n < waitDoneSec; n++) {
            mFlushDone.wait_for(lock, std::chrono::milliseconds(1000));
        }
    }
}


void FileULogger::flushThreadEntry()
{
    androidSetThreadName("FileULogger");

    static constexpr int PRIORITY_LEVEL[] = {
        android::PRIORITY_LOWEST,
        android::PRIORITY_BACKGROUND,
        android::PRIORITY_NORMAL,
        android::PRIORITY_FOREGROUND
    };
    static constexpr int PRIORITY_LEVEL_HIGH_LOADING = 2;
    static constexpr int PRIORITY_LEVEL_MAX = sizeof(PRIORITY_LEVEL) / sizeof(PRIORITY_LEVEL[0]) - 1;
    int currentPriority = 0;
    androidSetThreadPriority(getTid(), PRIORITY_LEVEL[currentPriority]);

    const int flushIntervalMs = property_get_int32(ULOG_FILE_FLUSHMS_PROP_NAME, ULOG_FILE_DEFAULT_FLUSHMS);

    ULOG_IMP_LOGI("File ULogger is running; flushInterval = %d ms, neHook = %d", flushIntervalMs, getNeHookLevel());

    auto continueWorking = [this] {
        return (mContinueWorking.load(std::memory_order_relaxed) ||
                !mToBeFlushed.empty() ||
                mActiveBuffer->hasData());
    };

    unsigned int nLogWritten = 0;
    unsigned int tick = 0;
    while (true) {
        Buffer *writingBuffer = nullptr;
        size_t toBeFlushedRemain = 0;
        tick++;

        {
            std::unique_lock<std::mutex> lock(mBufferMutex);

            while (continueWorking()) {
                if (mToBeFlushed.empty()) {
                    if (currentPriority > 0) {
                        currentPriority--; // Slow down
                        androidSetThreadPriority(getTid(), PRIORITY_LEVEL[currentPriority]);
                    }

                    if (mIsFlushing) {
                        mIsFlushing = false;
                        mFlushDone.notify_all();
                    }

                    if (mContinueWorking.load(std::memory_order_relaxed)) {
                        if (flushIntervalMs > 0)
                            mBufferCond.wait_for(lock, std::chrono::milliseconds(flushIntervalMs));
                        else
                            std::this_thread::yield();

                        if (!continueWorking())
                            break;
                    } else {
                        // Stopped but there may be something to be flushed
                        // We don't wait
                    }

                    if (mToBeFlushed.empty() && mActiveBuffer->hasData()) {
                        // Terminate mActiveBuffer and move it into mToBeFlushed
                        mActiveBuffer->mIsTerminated.store(true, std::memory_order_release);
                        if (mActiveBuffer->mNumOfWriting.load(std::memory_order_acquire) == 0) {
                            mToBeFlushed.emplace_back(std::move(mActiveBuffer));
                            mActiveBuffer = allocEmptyBuffer();
                            writingBuffer = mToBeFlushed.front().get();
                            toBeFlushedRemain = mToBeFlushed.size() - 1;
                            break;
                        } else {
                            mToBeFlushed.emplace_back(std::move(mActiveBuffer));
                            mActiveBuffer = allocEmptyBuffer();
                            mBufferCond.wait_for(lock, std::chrono::milliseconds(ULOG_WAIT_WRITING_INTERVAL_MS));
                        }
                    }
                } else {
                    if (currentPriority < PRIORITY_LEVEL_MAX) {
                        if (mIsFlushing || mToBeFlushed.size() > 10) {
                            currentPriority = PRIORITY_LEVEL_MAX;
                            androidSetThreadPriority(getTid(), PRIORITY_LEVEL[currentPriority]);
                        } else if (mToBeFlushed.size() > 3 && currentPriority < PRIORITY_LEVEL_HIGH_LOADING) {
                            currentPriority = PRIORITY_LEVEL_HIGH_LOADING;
                            androidSetThreadPriority(getTid(), PRIORITY_LEVEL[currentPriority]);
                        }
                    }

                    while (mToBeFlushed.front()->mNumOfWriting.load(std::memory_order_acquire) > 0) {
                        mBufferCond.wait_for(lock, std::chrono::milliseconds(ULOG_WAIT_WRITING_INTERVAL_MS));
                    }

                    // Reserve the buffer in mToBeFlushed for NE flush
                    writingBuffer = mToBeFlushed.front().get();
                    toBeFlushedRemain = mToBeFlushed.size() - 1;
                    break;
                }
            }
        }

        // writingBuffer shall not be nullptr when exit above loop
        // Unless stopped
        if (writingBuffer == nullptr)
            break;

        if (mWriter.isOpened()) {
            const LogHeader *header = reinterpret_cast<const LogHeader*>(writingBuffer->getData());
            intptr_t filled = writingBuffer->mNextEmpty - reinterpret_cast<intptr_t>(writingBuffer->getData());
            intptr_t printed = 0;
            while (printed < filled) {
                writeTo(mWriter, header);
                printed += static_cast<intptr_t>(header->size);
                header = reinterpret_cast<const LogHeader*>(
                    reinterpret_cast<intptr_t>(header) + static_cast<intptr_t>(header->size));
                nLogWritten++;
            }

            if (toBeFlushedRemain == 0)
                mWriter.flush(true);

            if (((tick & 0x3) == 0) || // fewer log
                ((flushIntervalMs >= 500 || mIsFlushing) && (toBeFlushedRemain == 0)))
            {
                if (mClockId != ANDROID_CLOCK_ID) {
                    char monoTimeStamp[40], realTimeStamp[40];
                    getTimeStamp(CLOCK_MONOTONIC, monoTimeStamp, sizeof(monoTimeStamp));
                    getTimeStamp(CLOCK_REALTIME, realTimeStamp, sizeof(realTimeStamp));
                    ULOG_IMP_LOGD("File ULogger: %u logs written, %zu buffers queueing; mono = %s, real = %s (%s)",
                        nLogWritten, toBeFlushedRemain, monoTimeStamp, realTimeStamp, mWriter.getCurrentFilePath());
                } else {
                    ULOG_IMP_LOGD("File ULogger: %u logs written, %zu buffers queueing (%s)",
                        nLogWritten, toBeFlushedRemain, mWriter.getCurrentFilePath());
                }
            }
        } else {
            ULOG_IMP_LOGE("Unable to open file. Please disable SELinux and restart camerahalserver. !!!");
        }

        {
            std::lock_guard<std::mutex> lock(mBufferMutex);

            writingBuffer->clear();
            if (mEmptyBuffers.size() >= 3)
                mEmptyBuffers.pop_front();
            mEmptyBuffers.emplace_back(std::move(mToBeFlushed.front()));

            mToBeFlushed.pop_front();
        }
    }

    {
        std::lock_guard<std::mutex> lock(mBufferMutex);
        if (mIsFlushing) {
            mIsFlushing = false;
            mFlushDone.notify_all();
        }
    }

    ULOG_IMP_LOGI("File ULogger stopped");
}


PassiveULogger::PassiveULogger() : mIsFlushing(false)
{
}


void PassiveULogger::onInit()
{
    mContinueWorking.store(true, std::memory_order_release);
    mFlushThread = std::thread(&PassiveULogger::flushThreadEntry, this);

    initHook();
}


void PassiveULogger::onUninit()
{
    uninitHook();

    if (mContinueWorking.load(std::memory_order_relaxed)) {
        {
            std::lock_guard<std::mutex> lock(mBufferMutex);
            mContinueWorking.store(false, std::memory_order_release);
            mBufferCond.notify_all();
        }

        mFlushThread.join();

        if (mIsFlushing) {
            mIsFlushing = false;
            mFlushDone.notify_all();
        }
    }
}


void PassiveULogger::onFlush(int waitDoneSec)
{
    std::unique_lock<std::mutex> lock(mBufferMutex);

    mIsFlushing = true;
    if (mActiveBuffer->hasData()) {
        mActiveBuffer->mIsTerminated.store(true, std::memory_order_relaxed);
        mToBeFlushed.emplace_back(std::move(mActiveBuffer));
        mActiveBuffer = allocEmptyBuffer();
    }

    mBufferCond.notify_all();

    if (waitDoneSec > 0 && mFlushThread.joinable()) {
        for (int n = 0; mIsFlushing && n < waitDoneSec; n++) {
            mFlushDone.wait_for(lock, std::chrono::milliseconds(1000));
        }
    }
}


void PassiveULogger::flushThreadEntry()
{
    androidSetThreadName("PassiveULogger");

    static constexpr int PRIORITY_LEVEL[] = {
        android::PRIORITY_LOWEST,
        android::PRIORITY_FOREGROUND // higher priority for flush
    };
    static constexpr int PRIORITY_LEVEL_MAX = sizeof(PRIORITY_LEVEL) / sizeof(PRIORITY_LEVEL[0]) - 1;
    int currentPriority = 0;
    androidSetThreadPriority(getTid(), PRIORITY_LEVEL[currentPriority]);

    int maxKb = property_get_int32(ULOG_PASSIVE_MAX_BUFFER_KB_PROP_NAME, 0);
    if (maxKb == 0) {
        struct sysinfo si;
        if (sysinfo(&si) == 0) {
            maxKb = static_cast<int>(static_cast<std::int64_t>(si.totalram) * static_cast<std::int64_t>(si.mem_unit) /
                    std::int64_t{16 * 1024});
        }
    }
    maxKb = std::max<int>(maxKb, BUFFER_SIZE * 8 / 1024);

    int nBuffersInQueue = static_cast<int>(static_cast<std::int64_t>(maxKb) * 1024 / BUFFER_SIZE) - 2;
    maxKb = (nBuffersInQueue + 2) * BUFFER_SIZE / 1024;
    int nDiscarded = 0;

    ULOG_IMP_LOGI("Passive ULogger is running, maxKb = %d KB, buffers = %d+2; neHook = %d",
        maxKb, nBuffersInQueue, getNeHookLevel());

    auto continueWorking = [this] {
        return (mContinueWorking.load(std::memory_order_relaxed));
    };

    // The logic is complicated.
    // We make the code similar to FileULogger intentionally to reduce review effort
    unsigned int nLogWritten = 0;
    while (true) {
        Buffer *writingBuffer = nullptr;
        size_t toBeFlushedRemain = 0;

        {
            std::unique_lock<std::mutex> lock(mBufferMutex);

            while (continueWorking()) {
                if (mToBeFlushed.empty()) {
                    if (currentPriority > 0) {
                        currentPriority--;
                        androidSetThreadPriority(getTid(), PRIORITY_LEVEL[currentPriority]);
                    }

                    if (mIsFlushing) {
                        mWriter.close();
                        mIsFlushing = false;
                        nLogWritten = 0;
                        mFlushDone.notify_all();
                        ULOG_IMP_LOGI("Passive log flushed.");
                    }
                } else {
                    if (currentPriority < PRIORITY_LEVEL_MAX && mIsFlushing) {
                        // Raise priority for flush
                        currentPriority = PRIORITY_LEVEL_MAX;
                        androidSetThreadPriority(getTid(), PRIORITY_LEVEL[currentPriority]);
                    }

                    if (mIsFlushing ||
                        mToBeFlushed.size() > static_cast<size_t>(nBuffersInQueue))
                    {
                        while (mToBeFlushed.front()->mNumOfWriting.load(std::memory_order_acquire) > 0) {
                            mBufferCond.wait_for(lock, std::chrono::milliseconds(ULOG_WAIT_WRITING_INTERVAL_MS));
                        }

                        // Reserve the buffer in mToBeFlushed for NE flush
                        writingBuffer = mToBeFlushed.front().get();
                        toBeFlushedRemain = mToBeFlushed.size() - 1;
                        break;
                    }
                }

                mBufferCond.wait_for(lock, std::chrono::milliseconds(ULOG_FILE_DEFAULT_FLUSHMS));
            }
        }

        // writingBuffer shall not be nullptr when exit above loop
        // Unless stopped
        if (writingBuffer == nullptr)
            break;

        if (mIsFlushing) {
            if (!mWriter.isOpened()) {
                char folderPath[128];
                getFolderPath(folderPath, sizeof(folderPath));
                mWriter.setFolder(folderPath);
                mWriter.open();
                nLogWritten = 0;
            }

            if (mWriter.isOpened()) {
                const LogHeader *header = reinterpret_cast<const LogHeader*>(writingBuffer->getData());
                intptr_t filled = writingBuffer->mNextEmpty - reinterpret_cast<intptr_t>(writingBuffer->getData());
                intptr_t printed = 0;
                while (printed < filled) {
                    writeTo(mWriter, header);
                    printed += static_cast<intptr_t>(header->size);
                    header = reinterpret_cast<const LogHeader*>(
                        reinterpret_cast<intptr_t>(header) + static_cast<intptr_t>(header->size));
                    nLogWritten++;
                }

                if (toBeFlushedRemain == 0)
                    mWriter.flush(true);

                if ((toBeFlushedRemain & 0x7) == 0) {
                    ULOG_IMP_LOGD("Passive ULogger: %u logs written, %zu buffers queueing (%s)",
                        nLogWritten, toBeFlushedRemain, mWriter.getCurrentFilePath());
                }
            } else {
                mIsFlushing = false;
                mFlushDone.notify_all();
                continue;
            }
        } else {
            nDiscarded++;
            if ((nDiscarded & 0x7) == 0) {
                ULOG_IMP_LOGI("Passive ULogger is running, %d log buffers were discarded; maxKb = %d KB", nDiscarded, maxKb);
            } else {
                ULOG_IMP_LOGD("Passive ULogger is running, %d log buffers were discarded; maxKb = %d KB", nDiscarded, maxKb);
            }
        }

        writingBuffer->clear();

        {
            std::lock_guard<std::mutex> lock(mBufferMutex);

            if (mEmptyBuffers.size() > 0)
                mEmptyBuffers.pop_front();
            mEmptyBuffers.emplace_back(std::move(mToBeFlushed.front()));

            mToBeFlushed.pop_front();
        }
    }

    {
        std::lock_guard<std::mutex> lock(mBufferMutex);
        if (mIsFlushing) {
            mIsFlushing = false;
            mFlushDone.notify_all();
        }
    }

    ULOG_IMP_LOGW("Passive ULogger stopped");
}


FileULoggerBase *FileULoggerBase::sActiveLogger = nullptr;


void FileULoggerBase::registerSigAction(
    int sigNum, struct sigaction &mySigAction, struct sigaction &oldSigAction)
{
    sigaction(sigNum, nullptr, &oldSigAction);
    mySigAction.sa_handler = nullptr;
    mySigAction.sa_sigaction = &FileULoggerBase::sigHandler;
    mySigAction.sa_mask = oldSigAction.sa_mask;
    mySigAction.sa_flags = SA_SIGINFO;
    mySigAction.sa_restorer = oldSigAction.sa_restorer;
    sigaction(sigNum, &mySigAction, nullptr);
}


void FileULoggerBase::initHook()
{
    sActiveLogger = this;
    int hookNe = property_get_int32(ULOG_FILE_HOOK_NE_PROP_NAME, 2);

    if (hookNe > 0 && mNeHookLevel == 0) {
        mSigWriter.reset(new FileULogWriter);
        char folderPath[128];
        getFolderPath(folderPath, sizeof(folderPath));
        mSigWriter->setFolder(folderPath);

        mNeHookLevel = hookNe;

        std::atomic_thread_fence(std::memory_order_release);

        registerSigAction(SIGABRT, mMySigAbrtAction, mOldSigAbrtAction);
        registerSigAction(SIGSEGV, mMySigSegvAction, mOldSigSegvAction);
        registerSigAction(SIGBUS, mMySigBusAction, mOldSigBusAction);
    }
}


void FileULoggerBase::uninitHook()
{
    if (mNeHookLevel > 0) {
        mNeHookLevel = 0;
        sigaction(SIGABRT, &mOldSigAbrtAction, nullptr);
        sigaction(SIGSEGV, &mOldSigSegvAction, nullptr);
        sigaction(SIGBUS, &mOldSigBusAction, nullptr);
        std::atomic_thread_fence(std::memory_order_release);
    }
}


inline void FileULoggerBase::sigWriteBuffer(FileULogWriter &writer, Buffer *writingBuffer)
{
    if (writingBuffer == NULL) // possible if the buffer is moving
        return;

    const LogHeader *header = reinterpret_cast<const LogHeader*>(writingBuffer->getData());
    // Make sure buffer will always be cleared before free
    intptr_t filled = writingBuffer->mNextEmpty - reinterpret_cast<intptr_t>(writingBuffer->getData());
    intptr_t printed = 0;
    while (printed < filled) {
        if (header->type != INVALID)
            writeTo(writer, header);
        printed += static_cast<intptr_t>(header->size);
        header = reinterpret_cast<const LogHeader*>(
            reinterpret_cast<intptr_t>(header) + static_cast<intptr_t>(header->size));
    }
}


void FileULoggerBase::sigHandler(int sig, siginfo_t *info, void *ucontext)
{
    const char *sigType = "NE";
    struct sigaction *oldAction = nullptr;

    if (sig == SIGABRT) {
        sigType = "SIGABRT";
        oldAction = &(sActiveLogger->mOldSigAbrtAction);
        sigaction(SIGABRT, oldAction, nullptr);
    } else if (sig == SIGSEGV) {
        sigType = "SIGSEGV";
        oldAction = &(sActiveLogger->mOldSigSegvAction);
        sigaction(SIGSEGV, oldAction, nullptr);
    } else if (sig == SIGBUS) {
        sigType = "SIGBUS";
        oldAction = &(sActiveLogger->mOldSigBusAction);
        sigaction(SIGBUS, oldAction, nullptr);
    }

    if (sActiveLogger != nullptr) {
        FileULogWriter &writer = *(sActiveLogger->mSigWriter.get());
        if (writer.open(true, "NE_", true)) {
            writer.writeString("%s : addr = %p\n", sigType, info->si_addr);

            intptr_t filledInActive = sActiveLogger->mActiveBuffer->mNextEmpty -
                    reinterpret_cast<intptr_t>(sActiveLogger->mActiveBuffer->getData());
            if (sActiveLogger->mNeHookLevel >= 2 || filledInActive < 64 * 200) {
                // The data structure in mToBeFlushed may be not valid, but we just try
                for (std::unique_ptr<Buffer> &buffer : sActiveLogger->mToBeFlushed) {
                    sigWriteBuffer(writer, buffer.get());
                }
            }

            sigWriteBuffer(writer, sActiveLogger->mActiveBuffer.get());
            writer.close();
        }
    }

    if (oldAction != nullptr) {
        if ((oldAction->sa_flags & SA_SIGINFO) &&
            (oldAction->sa_sigaction != nullptr))
        {
            (*oldAction->sa_sigaction)(sig, info, ucontext);
        } else if (oldAction->sa_handler != nullptr) {
            (*oldAction->sa_handler)(sig);
        } else if (oldAction->sa_sigaction != nullptr) {
            (*oldAction->sa_sigaction)(sig, info, ucontext);
        } else {
            raise(sig);
        }
    } else {
        raise(sig);
    }
}


}
}
}
}

