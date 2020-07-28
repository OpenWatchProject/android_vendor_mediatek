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

#ifndef __MTKCAM_UTILS_STD_ULOG_FILE_H__
#define __MTKCAM_UTILS_STD_ULOG_FILE_H__

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <deque>
#include <condition_variable>
#include <chrono>
#include <string>
#include <signal.h>
#include <time.h>
#include <mtkcam/utils/std/ULog.h>


namespace NSCam {
namespace Utils {
namespace ULog {
namespace File {

enum LogType
{
    INVALID = 0,
    ENTER,
    EXIT,
    DISCARD,
    SUBREQS,
    PATH_DIV,
    PATH_JOIN,
    FUNC_LIFE,
    FUNC_LIFE_EXT,
    DETAILS
};


struct LogHeader
{
    LogType type : 8;
    size_t size : 24;
    int tid;
    timespec timeStamp;
};


struct LogEnterExit : public LogHeader
{
    unsigned int logSerial;
    ModuleId moduleId;
    RequestSerial requestSerial;
    RequestTypeId requestTypeId : 8;
    char tag[0]; // the space is allocated dynamically
};

using LogEnter = LogEnterExit;
using LogExit = LogEnterExit;


struct LogDiscard : public LogHeader
{
    unsigned int logSerial;
    ModuleId moduleId;
    RequestTypeId requestTypeId;
    char *tag; // right after the requestSerialList[]
    int numOfRequestSerials;
    RequestSerial requestSerialList[0];
};


struct LogSubreqs : public LogHeader
{
    unsigned int logSerial;
    ModuleId moduleId;
    RequestTypeId requestTypeId;
    RequestSerial requestSerial;
    char *tag; // right after the subrequestSerialList[]
    RequestTypeId subrequestTypeId;
    int numOfSubreqSerials;
    RequestSerial subrequestSerialList[0];
};


struct LogPathTopo : public LogHeader
{
    ModuleId moduleId;
    RequestTypeId requestTypeId;
    RequestSerial requestSerial;
    char *tag; // right after the requestSerialList[]
    int numOfCoModules;
    ModuleId coModuleList[0];
};

using LogPathDiv = LogPathTopo;
using LogPathJoin = LogPathTopo;


struct LogFunc : public LogHeader
{
    unsigned int logSerial;
    ModuleId moduleId;
    char *funcName;
    FuncLifeTag lifeTag: 8;
    char tag[0];
};


struct LogFuncExt : public LogHeader
{
    unsigned int logSerial;
    ModuleId moduleId;
    char *funcName;
    std::intptr_t values[3];
    FuncLifeTag lifeTag : 8;
    char tag[0];
};


struct LogDetails : public LogHeader
{
    ModuleId moduleId;
    char *content;
    DetailsType detailsType : 8;
    char tag[0];
};


class FileULogWriter
{
public:
    FileULogWriter();

    ~FileULogWriter() {
        if (mFd >= 0)
            close();
    }

    void setFolder(const char *folderPath) {
        mFolderPath = folderPath;
    }

    bool isOpened() {
        return (mFd >= 0);
    }

    const char *getCurrentFilePath() const {
        return mFilePath.c_str();
    }

    void reserveSpace(bool inSignal);
    bool open(bool newSession = true, const char *prefix = "", bool inSignal = false);
    void close();
    void flush(bool deepFlush);
    void onLogEnter(const LogEnter *log);
    void onLogExit(const LogExit *log);
    void onLogDiscard(const LogDiscard *log);
    void onLogSubreqs(const LogSubreqs *log);
    void onLogPathDiv(const LogPathDiv *log);
    void onLogPathJoin(const LogPathJoin *log);
    void onLogFunc(const LogFunc *log);
    void onLogFuncExt(const LogFuncExt *log);
    void onLogDetails(const LogDetails *log);
    void writeString(const char *format, ...);

private:
    static constexpr size_t MAX_FILE_SIZE = 180 * 1024 * 1024; // 180 MB
    static constexpr size_t BUFFER_SIZE = 6 * 4096;

    char *mBufferEmpty;
    size_t mBufferFilled;
    size_t mBytesWritten;
    int mPid;
    int mFd;
    char mBuffer[BUFFER_SIZE];
    std::string mFolderPath;
    std::string mFilePath;
    char mSessionTimestamp[40];
    char mFileTag[30];
    bool mInSignal;
    int mFileSerial;

    inline char *getBufferBegin() {
        return mBuffer;
    }

    inline char *getBuffer() {
        return mBufferEmpty;
    }

    inline size_t getBufferSize() const {
        return BUFFER_SIZE - mBufferFilled;
    }

    inline int getPid() const {
        return mPid;
    }

    void writeModuleList(const ModuleId *coModuleList, int nModules);
    void write(size_t size, bool isCompleted = false);
};


class FileULoggerBase : public ULogger
{
protected:
    FileULoggerBase();
    virtual ~FileULoggerBase() { }

protected:
    static constexpr size_t BUFFER_SIZE = 256 * 1024;
    static constexpr size_t MAX_STRING_LEN = 128;

    struct Buffer {
        std::atomic_bool mIsTerminated;
        std::atomic_int mNumOfWriting;
        intptr_t mNextEmpty;
        size_t mEmptySize;
        void *mData[BUFFER_SIZE / sizeof(void*)];

        Buffer() : mIsTerminated(false), mNumOfWriting(0) {
            mNextEmpty = reinterpret_cast<intptr_t>(getData());
            mEmptySize = BUFFER_SIZE;
        }

        void *getData() {
            return &(mData[0]);
        }

        const void *getData() const {
            return &(mData[0]);
        }

        void clear() {
            mIsTerminated.store(false, std::memory_order_relaxed);
            mNextEmpty = reinterpret_cast<intptr_t>(getData());
            mEmptySize = BUFFER_SIZE;
        }

        bool hasData() const {
            return (mNextEmpty != reinterpret_cast<intptr_t>(getData()));
        }
    };

    clockid_t mClockId;
    std::mutex mBufferMutex;
    std::deque<std::unique_ptr<Buffer>> mToBeFlushed;
    std::deque<std::unique_ptr<Buffer>> mEmptyBuffers;
    std::unique_ptr<Buffer> mActiveBuffer;
    std::atomic_bool mContinueWorking;
    std::condition_variable mBufferCond;

    inline unsigned int getLogSerial() {
        return ++mLogSerial;
    }

    void writeLogDone(Buffer *activeBuffer, LogHeader *header, LogType type);
    static void writeTo(FileULogWriter &writer, const LogHeader *header);
    std::unique_ptr<Buffer> allocEmptyBuffer();
    void getFolderPath(char *folderPath, size_t bufferSize);
    static char* getTimeStamp(clockid_t clockId, char *buffer, size_t bufferSize);

    void initHook();
    void uninitHook();

    int getNeHookLevel() const {
        return mNeHookLevel;
    }

private:
    unsigned int mLogSerial;  // We are always in mutex to get serial
    int mNeHookLevel;

    bool allocLogSpace(size_t size, LogHeader **logSpace, Buffer **activeBuffer, unsigned int *logSerial);
    static size_t safeStrlen(const char *str, size_t maxLen = MAX_STRING_LEN);
    static size_t safeStrncpy(char *dest, const char *src, size_t size);

    static FileULoggerBase *sActiveLogger;

    struct sigaction mMySigAbrtAction, mMySigSegvAction, mMySigBusAction;
    struct sigaction mOldSigAbrtAction, mOldSigSegvAction, mOldSigBusAction;
    std::unique_ptr<FileULogWriter> mSigWriter;

    void registerSigAction(
        int sigNum, struct sigaction &mySigAction, struct sigaction &oldSigAction);
    static void sigWriteBuffer(FileULogWriter &writer, Buffer *writingBuffer);
    static void sigHandler(int sig, siginfo_t *info, void *ucontext);

public:
    virtual void onLogEnter(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) override;
    virtual void onLogExit(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) override;
    virtual void onLogDiscard(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, const RequestSerial *requestSerialList, size_t n);
    virtual void onLogSubreqs(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n) override;
    virtual void onLogPathDiv(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ModuleId *toModuleIdList, size_t n) override;
    virtual void onLogPathJoin(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ModuleId *fromModuleIdList, size_t n) override;
    virtual void onLogFuncLife(ModuleId , const char *tag, const char *funcName, FuncLifeTag lifeTag) override;
    virtual void onLogFuncLifeExt(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag,
        std::intptr_t v1, std::intptr_t v2, std::intptr_t v3) override;
    virtual void onLogDetails(ModuleId moduleId, const char *tag, DetailsType type,
        const char *details, size_t contentLen) override;
};


class FileULogger final : public FileULoggerBase
{
public:
    FileULogger();

    virtual ~FileULogger() {
        FileULogger::onUninit();
    }

    virtual void onInit() override;
    virtual void onUninit() override;
    virtual void onFlush(int waitDoneSec) override;

private:
    std::thread mFlushThread;
    FileULogWriter mWriter;
    bool mIsFlushing;
    std::condition_variable mFlushDone;

    void flushThreadEntry();
};


class PassiveULogger final : public FileULoggerBase
{
public:
    PassiveULogger();

    virtual ~PassiveULogger() {
        PassiveULogger::onUninit();
    }

    virtual void onInit() override;
    virtual void onUninit() override;
    virtual void onFlush(int waitDoneSec) override;

private:
    std::thread mFlushThread;
    FileULogWriter mWriter;
    bool mIsFlushing;
    std::condition_variable mFlushDone;

    void flushThreadEntry();
};


}
}
}
}


#endif

