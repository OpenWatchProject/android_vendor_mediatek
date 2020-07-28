#define LOG_TAG "BufferQueueDump"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#define MTK_LOG_ENABLE 1
#include <sys/stat.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include <gui_debug/BufferQueueDump.h>

// ----------------------------------------------------------------------------
#define PROP_DUMP_NAME      "vendor.debug.bq.dump"
#define DEFAULT_DUMP_NAME   "[none]"

#define DUMP_FILE_PATH      "/data/SF_dump/"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define BQD_LOGV(x, ...) ALOGV("[%s] " x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGD(x, ...) ALOGD("[%s] " x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGI(x, ...) ALOGI("[%s] " x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGW(x, ...) ALOGW("[%s] " x, mName.string(), ##__VA_ARGS__)
#define BQD_LOGE(x, ...) ALOGE("[%s] " x, mName.string(), ##__VA_ARGS__)


namespace android {

String8 reduceFileName(const String8& prefix);

// ----------------------------------------------------------------------------
BufferQueueDump::BufferQueueDump() :
        mName("unnamed BufferQueueDump"),
        mBackupBufPusher(NULL),
        mBackupBufDumper(NULL),
        mIsBackupBufInited(false),
        mObtainedBufs(NULL),
        mLastObtainedBuf(NULL),
        mPid(getpid())
{
    mProcName = "\?\?\?";
    FILE *fp = fopen(String8::format("/proc/%d/cmdline", mPid), "r");
    if (NULL != fp) {
        const size_t size = 64;
        char proc_name[size];
        fgets(proc_name, size, fp);
        fclose(fp);
        setDownSample();
        mProcName = proc_name;
    }
}

void BufferQueueDump::setName(const String8& name) {
    mName = name;

    // update dumper's name
    if (mBackupBufDumper != NULL) {
        mBackupBufDumper->setName(name);
    }

    // check and reset current dump setting

    char value[PROPERTY_VALUE_MAX];
    property_get(PROP_DUMP_NAME, value, DEFAULT_DUMP_NAME);

    uint32_t backupCnt = 0;
    bool isMatched = parseDumpConfig(value, &backupCnt);
    if (isMatched && strstr(value, "#") != NULL) {
        setBackupCount(backupCnt);
    }
}

void BufferQueueDump::setBackupCount(uint32_t count) {
    BQD_LOGV("setBackupCount: %d", count);
    if (count > 0) {
        // create backup buffer if needed
        if (!mIsBackupBufInited) {
            mBackupBufPusher = new BackupBufPusher(mBackupBuf);
            mBackupBufDumper = new BackupBufDumper(mBackupBuf);
            if ((mBackupBufPusher != NULL) && (mBackupBufDumper != NULL)) {
                mBackupBufDumper->setName(mName);
                sp< RingBuffer< sp<BackupBuffer> >::Pusher > proxyPusher = mBackupBufPusher;
                sp< RingBuffer< sp<BackupBuffer> >::Dumper > proxyDumper = mBackupBufDumper;
                mBackupBuf.setPusher(proxyPusher);
                mBackupBuf.setDumper(proxyDumper);
                mIsBackupBufInited = true;
                mBackupBufPusher->setDownSampleConfig(mConfig);
            } else {
                mBackupBufPusher.clear();
                mBackupBufDumper.clear();
                count = 0;
                BQD_LOGE("[%s] create Backup pusher or dumper failed", __func__);
            }
        }

        // resize backup buffer
        mBackupBuf.resize(count);
    } else {
        mBackupBuf.resize(0);
    }
}

void BufferQueueDump::parseDownSampleConfig(const char* str) {

    BQD_LOGD("checkBackupCnt: str:%s", str);

    const char *numberSignX = strstr(str, "*X");
    const char *numberSignY = strstr(str, "*Y");
    const char *numberSignR = strstr(str, "*R");
    const char *numberSignT = strstr(str, "*T");
    const char *numberSignL = strstr(str, "*L");
    const char *numberSignB = strstr(str, "*B");

    int downX = (numberSignX) ? atoi(numberSignX + 2) : 1;
    int downY = (numberSignY) ? atoi(numberSignY + 2) : 1;

    Rect crop = {((numberSignL) ? atoi(numberSignL + 2) : 0),
                 ((numberSignT) ? atoi(numberSignT + 2) : 0),
                 ((numberSignR) ? atoi(numberSignR + 2) : 0),
                 ((numberSignB) ? atoi(numberSignB + 2) : 0)};

    BQD_LOGI("Set downcopy (x=%d, y=%d) crop((l, t, r, b)=(%d, %d, %d, %d))",
              downX, downY, crop.left, crop.top, crop.right, crop.bottom);

    setDownSample(downY, downX, crop);
}

void BufferQueueDump::setDownSample(int downY, int downX, Rect crop) {

    DownSampleConfig config = {downX, downY, crop};
    mConfig = config;
    if(mBackupBufPusher.get()){
        mBackupBufPusher->setDownSampleConfig(mConfig);
    }
}

void BufferQueueDump::dumpObtainedBufs() {
    String8 name;
    const char* bufName = "Acquired";
    getDumpFileName(name, mName);
    // dump acquired buffers
    if (!mObtainedBufs.size()) {
        // if no acquired buf, try to dump the last one kept
        if (mLastObtainedBuf != NULL) {
            String8 name_prefix = String8::format("[%s](LAST_ts%" PRIu64 ")",
                    name.string(), ns2ms(mLastObtainedBuf->mTimeStamp));
            mLastObtainedBuf->dump(name_prefix);

            BQD_LOGD("[dump] LAYER, handle(%p)", mLastObtainedBuf->mGraphicBuffer->handle);
        }
    } else {
        // dump acquired buf old to new
        for (uint32_t i = 0; i < mObtainedBufs.size(); i++) {
            const sp<DumpBuffer>& buffer = mObtainedBufs[i];
            if (buffer->mGraphicBuffer != NULL) {
                String8 name_prefix = String8::format("[%s](%s%02u_ts%" PRIu64 ")",
                        name.string(), bufName, i, ns2ms(buffer->mTimeStamp));
                buffer->dump(name_prefix);

                BQD_LOGD("[dump] %s:%02u, handle(%p)", bufName, i, buffer->mGraphicBuffer->handle);
            }
        }
    }
}

unsigned int BufferQueueDump::checkBackupCnt(char* str) {
    BQD_LOGV("checkBackupCnt: str:%s", str);
    unsigned int cnt = 0;
    char *numberSign = strchr(str, '#');

    if (!numberSign)
        return cnt;
    *numberSign = '\0';
    int tmp = atoi(numberSign + 1);
    if (tmp < 0) {
        BQD_LOGE("Backup count can not be negative");
    }
    cnt = static_cast<unsigned int>(tmp);
    return cnt;
}

bool BufferQueueDump::matchProc(char* str) {
    BQD_LOGV("matchProc: str:%s mConsumerProcName:%s(%d)", str, mProcName.string(), mPid);
    char *pProc = strstr(str, "@@");

    if (pProc) {
        pProc += strlen("@@");
        // matching process's name

        // if pProc is equal to "", it means all process
        if (strlen(pProc) == 0) {
            return true;
        }
        int pid = atoi(pProc);
        BQD_LOGV("pid:%d pProc:%s", pid, pProc);
        return pid != 0 && pid == mPid ? true : false;
    } else if ((pProc = strstr(str, "@")) != NULL) {
        pProc += strlen("@");
        // matching process's pid
        return (!strlen(pProc) || strstr(mProcName, pProc)) ? true : false;
    }
    return false;
}

bool BufferQueueDump::matchCName(char* str) {
    BQD_LOGV("matchName: str:%s mName:%s \n", str, mName.string());

    return strstr(mName, str) != NULL ? true : false;
}

enum ACTION {
    ACTION_NONE,
    ACTION_INCLUDE,
    ACTION_EXCLUDE
};

int BufferQueueDump::match(char* substr) {
    // '^' means matching BufferQueue is excluded and others are included
    // the semantic is ambiguous in '@@;^Frame' case
    char* inversion = strchr(substr, '^');
    //char* inversion = strlen(substr) > 0 && substr[0] == '^';
    if (inversion) {
        substr = inversion + 1;
    }
    bool isMatchProc = true;
    char *atSign = strchr(substr, '@');
    if (atSign) {
        isMatchProc = matchProc(atSign);
        BQD_LOGV("matchProc: %d", isMatchProc);
        *atSign = '\0';
    }
    bool isMatchName = true;
    if (strlen(substr)) {
        isMatchName = matchCName(substr);
        BQD_LOGV("matchCName: %d", isMatchName);
    }
    if (inversion) {
        BQD_LOGV("match result:%d", !(isMatchProc && isMatchName) ? ACTION_INCLUDE : ACTION_EXCLUDE);
        return !(isMatchProc && isMatchName) ? ACTION_INCLUDE : ACTION_EXCLUDE;
    } else {
        BQD_LOGV("match result:%d", isMatchProc && isMatchName ? ACTION_INCLUDE : ACTION_NONE);
        return isMatchProc && isMatchName ? ACTION_INCLUDE : ACTION_NONE;
    }
}

bool BufferQueueDump::parseDumpConfig(const char* value, uint32_t* pBackupCnt) {
    if (!value || !pBackupCnt) {
        BQD_LOGE("invalid value:%p pBackupCnt:%p", value, pBackupCnt);
        return false;
    }

    bool isMatched = false;
    bool isSetting = (strchr(value, '#') != NULL);
    *pBackupCnt = 0;

    // should not modify value, so backup value
    char str[PROPERTY_VALUE_MAX] = "";
    const size_t strSize = PROPERTY_VALUE_MAX - 1 < strlen(value) ? PROPERTY_VALUE_MAX - 1 : strlen(value);
    memmove(str, value, strSize);
    str[strSize] = '\0';

    // split str into substrs
    Vector<char*> substrs;
    substrs.push(str);
    const char* delimiter = ";";
    char *lastptr = NULL;
    char *substr = strtok_r(str, delimiter, &lastptr);
    while (substr) {
        substrs.push(substr);
        substr = strtok_r(NULL, delimiter, &lastptr);
    }

    // start matching from tail
    // because the latter rule will override the former
    BQD_LOGV("parse str:%s", value);
    for (size_t i = substrs.size() - 1, j = 0; j < substrs.size(); --i, ++j) {
        BQD_LOGV("parse substr:%s", substrs[i]);
        // check invalid rules
        char *numSign = strchr(substrs[i], '#');
        if ((numSign && !(numSign - substrs[i])) ||
                !strlen(substrs[i]) ||
                strchr(substrs[i], ' ')) {
            BQD_LOGW("invalid matching rules");
            continue;
        }
        uint32_t tmpBackupCnt = checkBackupCnt(substrs[i]);

        int matchResult = match(substrs[i]);
        if (isSetting) {
            if (matchResult == ACTION_INCLUDE) {
                isMatched = true;
                *pBackupCnt = tmpBackupCnt;
                break;
            }
        } else {
            if (matchResult != ACTION_NONE) {
                isMatched = (matchResult == ACTION_INCLUDE) ? true : false;
                break;
            }
        }
    }

    if (strstr(value, "*") != NULL) {
        parseDownSampleConfig(value);
    }

    BQD_LOGV("parse * isMatched:%d backupCnt:%d", isMatched, *pBackupCnt);

    return isMatched;
}


void BufferQueueDump::dump(String8& result, const char* prefix) {
    // dump status to log buffer first
    const char* bufName = "Acquired";

    result.appendFormat("%s*BufferQueueDump mIsBackupBufInited=%d, m%sBufs(size=%zu)\n",
            prefix, mIsBackupBufInited, bufName, mObtainedBufs.size());

    if ((mLastObtainedBuf != NULL) && (mLastObtainedBuf->mGraphicBuffer != NULL)) {
        result.appendFormat("%s [-1] mLast%sBuf->mGraphicBuffer->handle=%p\n",
                prefix, bufName, mLastObtainedBuf->mGraphicBuffer->handle);
    }

    for (size_t i = 0; i < mObtainedBufs.size(); i++) {
        const sp<DumpBuffer>& buffer = mObtainedBufs[i];
        result.appendFormat("%s [%02zu] handle=%p, fence=%p, time=%#" PRIx64 ", xform=0x%02x\n",
            prefix, i, buffer->mGraphicBuffer->handle, buffer->mFence.get(), buffer->mTimeStamp,
            buffer->mTransform);
    }

    // dump fps information
    mQueueFps.dump(&result, prefix);

    // start buffer dump check and process
    char value[PROPERTY_VALUE_MAX];
    property_get(PROP_DUMP_NAME, value, DEFAULT_DUMP_NAME);
    if (strcmp(value, DEFAULT_DUMP_NAME) == 0 || strlen(value) == 0) {
        // debug feature (bqdump) is not enabled
        return;
    }

    // For aee manual dump, we must create a directory to save files.
    // The step should not be completed by a script.
    struct stat sb;
    if (stat(DUMP_FILE_PATH, &sb) != 0) {
        // ths permission of /data/SF_dump must be 777,
        // or some processes cannot save files to /data/SF_dump
        mode_t mode = umask(0);
        if (mkdir(DUMP_FILE_PATH, 0777) != 0) {
            BQD_LOGE("mkdir(%s) failed", DUMP_FILE_PATH);
        }
        umask(mode);
    }
    if (access(DUMP_FILE_PATH, R_OK | W_OK | X_OK) != 0) {
        BQD_LOGE("The permission of %s cannot be access by this process", DUMP_FILE_PATH);
    }

    uint32_t backupCnt = 0;
    bool isMatched = parseDumpConfig(value, &backupCnt);

    // if value contains '#', it means setting continues dump
    // otherwise, dump buffers
    if (strchr(value, '#') != NULL) {
        if (isMatched) {
            setBackupCount(backupCnt);
        }
    } else {
        if (isMatched) {
            if (mBackupBuf.getSize() > 0) {
                mBackupBuf.dump(result, prefix);
                setBackupCount(0);
            }
            dumpObtainedBufs();
        }
    }
}

void BufferQueueDump::getDumpFileName(String8& fileName, const String8& name) {
    fileName = name;

    // check file name, filter out invalid chars
    const char invalidChar[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};
    size_t size = fileName.size();
    char *buf = fileName.lockBuffer(size);
    for (unsigned int i = 0; i < ARRAY_SIZE(invalidChar); i++) {
        for (size_t c = 0; c < size; c++) {
            if (buf[c] == invalidChar[i]) {
                // find invalid char, replace it with '_'
                buf[c] = '_';
            }
        }
    }
    fileName.unlockBuffer(size);
}

void BufferQueueDump::addBuffer(const int& slot,
                                const sp<GraphicBuffer>& buffer,
                                const sp<Fence>& fence,
                                const int64_t& timestamp,
                                const uint32_t& transform,
                                const uint64_t bufferNum) {
    if (buffer == NULL) {
        return;
    }

    sp<DumpBuffer> v = mObtainedBufs.valueFor(static_cast<const uint32_t>(slot));
    if (v == NULL) {
        sp<DumpBuffer> b = new DumpBuffer(buffer, fence, timestamp, transform, bufferNum);
        mObtainedBufs.add(static_cast<const unsigned int>(slot), b);
        mLastObtainedBuf = NULL;
    } else {
        BQD_LOGW("[%s] slot(%d) acquired, seems to be abnormal, just update ...", __func__, slot);
        v->mGraphicBuffer = buffer;
        v->mFence = fence;
        v->mTimeStamp = timestamp;
        v->mTransform = transform;
        v->mBufferNum = bufferNum;
    }
}

void BufferQueueDump::updateBuffer(const int& slot, const int64_t& timestamp) {
    if (mBackupBuf.getSize() > 0) {
        const sp<DumpBuffer>& v = mObtainedBufs.valueFor(static_cast<const unsigned int>(slot));
        if (v != NULL) {
            // push GraphicBuffer into backup buffer if buffer ever Acquired
            sp<BackupBuffer> buffer = NULL;
            if (timestamp != -1)
                buffer = new BackupBuffer(v->mGraphicBuffer, timestamp, v->mBufferNum);
            else
                buffer = new BackupBuffer(v->mGraphicBuffer, v->mTimeStamp, v->mBufferNum);
            mBackupBuf.push(buffer);
        }
    }

    // keep for the last one before removed
    if (1 == mObtainedBufs.size()) {
        if (timestamp != -1)
            mObtainedBufs[0]->mTimeStamp = timestamp;

        mLastObtainedBuf = mObtainedBufs[0];
    }
    mObtainedBufs.removeItem(static_cast<const unsigned int>(slot));
}

void BufferQueueDump::onAcquireBuffer(const int& slot,
                                      const sp<GraphicBuffer>& buffer,
                                      const sp<Fence>& fence,
                                      const int64_t& timestamp,
                                      const uint32_t& transform,
                                      const uint64_t bufferNum) {
    addBuffer(slot, buffer, fence, timestamp, transform, bufferNum);
}

void BufferQueueDump::onReleaseBuffer(const int& slot) {
        updateBuffer(slot);
}

void BufferQueueDump::onFreeBuffer(const int& slot) {
        updateBuffer(slot);
}

void BufferQueueDump::onConsumerDisconnect() {
    mName += "(consumer disconnected)";

    if (mBackupBuf.getSize() > 0) {
        String8 result;
        mBackupBuf.dump(result, "");
    }

    mBackupBuf.resize(0);
    mBackupBufPusher = NULL;
    mBackupBufDumper = NULL;
    mIsBackupBufInited = false;

    mObtainedBufs.clear();
    mLastObtainedBuf = NULL;
}

status_t BufferQueueDump::drawDebugLineToGraphicBuffer(
        const sp<GraphicBuffer>& gb, uint32_t cnt, uint8_t val) {
    const uint32_t DEFAULT_LINE_W = 4;
    const uint32_t DEFAULT_LINE_H = 4;
    if (gb == NULL) {
        return INVALID_OPERATION;
    }

    int line_number_w = DEFAULT_LINE_W;
    int line_number_h = DEFAULT_LINE_H;
    int line_w = DEFAULT_LINE_W;
    int line_h = DEFAULT_LINE_H;

    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.debug.bq.line_p", value, "-1");
    int line_pos = atoi(value);
    if (line_pos >= 0)
        cnt = static_cast<uint32_t>(line_pos);

    property_get("vendor.debug.bq.line_g", value, "-1");
    sscanf(value, "%d:%d", &line_w, &line_h);
    if (line_w > 0)
        line_number_w = line_w;
    if (line_h > 0)
        line_number_h = line_h;

    property_get("vendor.debug.bq.line_c", value, "-1");
    int8_t line_c = static_cast<int8_t>(atoi(value));
    if (line_c >= 0)
        val = static_cast<uint8_t>(line_c);

    getGraphicBufferUtil().drawLine(gb, val, line_number_w, line_number_h, static_cast<int>(cnt));
    return NO_ERROR;
}

void BufferQueueDump::onSetIonInfo(const sp<GraphicBuffer>& gb, const int p_pid,
                                   const int& id, const int& length, const String8 cname) {
#ifndef MTK_DO_NOT_USE_GPU_EXT
    if (gb->handle != NULL) {
        gralloc_extra_ion_debug_t info;

        snprintf(info.name, length, "%s", cname.string());
        info.data[0] = p_pid;
        info.data[1] = id;
        gralloc_extra_perform(gb->handle, GRALLOC_EXTRA_SET_IOCTL_ION_DEBUG, &info);
    } else {
        BQD_LOGE("handle of graphic buffer is NULL when producer set ION info");
    }
#else
    (void)gb;
    (void)p_pid;
    (void)id;
    (void)length;
    (void)cname;
#endif
}

bool BufferQueueDump::onCheckFps(String8* result) {
    if (true == mQueueFps.update()) {
        *result = String8::format("queueBuffer: fps=%.2f dur=%.2f max=%.2f min=%.2f",
                static_cast<double>(mQueueFps.getFps()),
                mQueueFps.getLastLogDuration() / 1e6,
                mQueueFps.getMaxDuration() / 1e6,
                mQueueFps.getMinDuration() / 1e6);
        return true;
    } else {
        return false;
    }
}

// ----------------------------------------------------------------------------
bool BackupBufPusher::push(const sp<BackupBuffer>& in) {
    if ((in == NULL) || (in->mGraphicBuffer == NULL)) {
        return false;
    }

    const int format = in->mGraphicBuffer->format;
    const uint32_t bits = getGraphicBufferUtil().getBitsPerPixel(format);
    if (bits == 0) {
        // do not copy unknown format to avoid memory corruption
        return false;
    }

    sp<BackupBuffer>& buffer = editHead();

    // check property of GraphicBuffer, realloc if needed
    bool needCreate = false;
    if (buffer == NULL) {
        needCreate = true;
    }

    if (needCreate) {
        sp<GraphicBuffer> newGraphicBuffer = new GraphicBuffer(
                                             static_cast<uint32_t>(in->mGraphicBuffer->width), static_cast<uint32_t>(in->mGraphicBuffer->height),
                                             in->mGraphicBuffer->format, static_cast<uint32_t>(in->mGraphicBuffer->usage));
        if (newGraphicBuffer == NULL) {
            ALOGE("[%s] alloc GraphicBuffer failed", __func__);
            return false;
        }

        if (buffer == NULL) {
            buffer = new BackupBuffer();
            if (buffer == NULL) {
                ALOGE("[%s] alloc BackupBuffer failed", __func__);
                return false;
            }
        }

        buffer->mGraphicBuffer = newGraphicBuffer;
    }
    buffer->mConfig = getGraphicBufferUtil().downSampleCopy(mConfig,
                                                      in->mGraphicBuffer,
                                                      buffer->mGraphicBuffer);
    // update timestamp
    buffer->mTimeStamp = in->mTimeStamp;
    buffer->mSourceHandle = in->mGraphicBuffer->handle;
    buffer->mSrcWidth = in->mGraphicBuffer->width;
    buffer->mSrcHeight = in->mGraphicBuffer->height;
    buffer->mBufferNum = in->mBufferNum;

    return true;
}

// ----------------------------------------------------------------------------
void BackupBufDumper::dump(String8 &result, const char* prefix) {
    // dump status to log buffer first
    result.appendFormat("%s*BackupBufDumper mRingBuffer(size=%u, count=%u)\n",
        prefix, mRingBuffer.getSize(), mRingBuffer.getCount());

    for (uint32_t i = 0; i < mRingBuffer.getValidSize(); i++) {
        const sp<BackupBuffer>& buffer = getItem(i);

        result.appendFormat("%s [%02u] handle(source=%p, backup=%p)"
            " down(x=%d, y=%d) crop( (l, t, r, b)=(%d, %d, %d, %d))"
            " src(w0=%d, h0=%d)_N(%" PRIu64 ")\n",
            prefix, i, buffer->mSourceHandle, buffer->mGraphicBuffer->handle,
            buffer->mConfig.mDownSampleX, buffer->mConfig.mDownSampleX,
            buffer->mConfig.mCrop.left, buffer->mConfig.mCrop.top,
            buffer->mConfig.mCrop.right, buffer->mConfig.mCrop.bottom,
            buffer->mSrcWidth, buffer->mSrcHeight, buffer->mBufferNum);
    }

    // start buffer dump check and process
    String8 name;
    String8 name_prefix;

    BufferQueueDump::getDumpFileName(name, mName);

    for (uint32_t i = 0; i < mRingBuffer.getValidSize(); i++) {
        const sp<BackupBuffer>& buffer = getItem(i);

        name_prefix = String8::format("[%s](B%02u_H%p_N%" PRId64 ")"
            "(dX%d_dY%d)crop(l%d_t%d_r%d_b%d)src(w%d_h%d)",
            reduceFileName(name).string(), i, buffer->mSourceHandle,buffer->mBufferNum,
            buffer->mConfig.mDownSampleX,buffer->mConfig.mDownSampleY,
            buffer->mConfig.mCrop.left, buffer->mConfig.mCrop.top,
            buffer->mConfig.mCrop.right,buffer->mConfig.mCrop.bottom,
            buffer->mSrcWidth, buffer->mSrcHeight);
        getGraphicBufferUtil().dump(buffer->mGraphicBuffer, name_prefix.string()
                                    ,DUMP_FILE_PATH);

        BQD_LOGI("[dump] Backup:%02u, handle(source=%p, backup=%p)",
            i, buffer->mSourceHandle, buffer->mGraphicBuffer->handle);
    }
}

// ----------------------------------------------------------------------------
void DumpBuffer::dump(const String8& prefix) {
    if (mFence != NULL) {
        mFence->waitForever(__func__);
    }
    char temp[256];
    memset(temp, '\0', 256);
    sprintf(temp, "%s_N(%" PRIu64 ")", prefix.c_str(), mBufferNum);
    getGraphicBufferUtil().dump(mGraphicBuffer, temp, DUMP_FILE_PATH);
}

// ----------------------------------------------------------------------------
BufferQueueDumpAPI *createInstance() {
    return new BufferQueueDump;
}
// ----------------------------------------------------------------------------


/*
Try to reduce file name
Input : SurfaceView - com.android.gallery3d_com.android.gallery3d.app.MovieActivity#0
Output : SurfaceView - com.android.gallery3d...MovieActivity#0

Input : com.android.gallery3d_com.android.gallery3d.app.MovieActivity#0
Output : com.android.gallery3d...MovieActivity#0

Input : com.android.systemui.ImageWallpaper#0
Output : com.android.systemui.ImageWallpaper#0
*/
String8 reduceFileName(const String8& prefix)
{
    String8 result;
    const char* p = prefix.c_str();
    int leng = 0;
    size_t i=0;

    for(i=0; i<prefix.size(); i++)
    {
        if (*(p+i) != '_')
            continue;
        else {
            leng = i;
            break;
        }
    }

    if(i==prefix.size())
        return prefix;

    result.append(prefix, leng);
    result.append("...");

    leng = 0;
    p = prefix.c_str() + prefix.size() - 1;
    for(i=0; i<prefix.size(); i++)
    {
        if (*(p-i) != '.')
            continue;
        else {
            leng = i;
            break;
        }
    }
    result.append(prefix + prefix.size() - leng, leng);
    return result;
}

}
