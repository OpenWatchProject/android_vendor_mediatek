#include <android/log.h>
#include <unistd.h>    // usleep
#include <sys/epoll.h>
#include <sys/prctl.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <cutils/properties.h>
#include <hidl/LegacySupport.h>

#include "fm_hidl_service.h"
#include "fmr.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using ::android::sp;
using ::android::status_t;
using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::IBase;
using ::android::wp;

extern struct fmr_ds fmr_data;

namespace vendor {
namespace mediatek {
namespace hardware {
namespace fm {
namespace V1_0 {
namespace implementation {

//#define PR_SET_NAME 15

FmHidlService::FmHidlService()
    : mFmFd(-1),
      mFmCallback(NULL),
      mScanChlCount(0),
      mTuneFreq(0),
      mScanDwell(1),
      mLastRdsStatus(0),
      mEnabled(false),
      mFmTunned(false),
      mInPwrMode(false),
      mSearching(false),
      mRdsReady(false),
      mSearchCancelled(false),
      mRdsOn(false),
      mSearchMode(SearchMode::SEEK),
      mRegion(Region::UE),
      mEmphasis(Deemphasis::D50),
      mSpacing(Spacing::SPACE100),
      mRds(Rds::US),
      mCommandThread(NULL),
      mRdsThread(NULL),
      mFmHidlDeathRecipient(new FmHidlDeathRecipient(this))
{
    LOGI("Construct FmHidlService.\n");

    mFmFd = FMR_init();
    memset(mAfList, 0, 50 * sizeof(int));
    pthread_mutex_init(&mCommandLock, (const pthread_mutexattr_t *) NULL);
    pthread_cond_init(&mCommandCond, (const pthread_condattr_t *) NULL);
    pthread_mutex_lock(&mCommandLock);
    list_init(&mFmCommandList);
    pthread_mutex_unlock(&mCommandLock);

    pthread_create(&mCommandThread, NULL, fm_hidl_thread, this);

}

FmHidlService::~FmHidlService()
{
    LOGI("Distruct FmHidlService.\n");
    sendCommand(CMD_EXIT, 0, NULL);
    LOGI("Wait command thread to be terminated.\n");
    pthread_join(mCommandThread, (void **) NULL);
}

void FmHidlService::handleHidlDeath()
{
    mFmCallback = NULL;
    mEnabled = false;
}

Return<Result> FmHidlService::enable(const sp<IFmRadioCallback>& callback)
{
    int deemphasis = 0, ret = 0;

    LOGI("%s\n", __func__);

    if (mFmFd < 0) {
        mFmFd = FMR_init();
    }

    if (mFmFd < 0)
        return Result::NOT_INITIALIZED;

    if (mEnabled) {
        LOGW("Already enabled.\n");
        return Result::OK;
    }

    ret = FMR_open_dev(mFmFd);
    if (ret != 0)
        return Result::FAILED;

    if (mEmphasis == Deemphasis::D75)
    	deemphasis = 1;

    ret = FMR_pwr_up(mFmFd, 8755, deemphasis);
    if (ret != 0) {
        FMR_close_dev(mFmFd);
        return Result::FAILED;
    }

    mEnabled = true;
    // Send pre scan event
    //sendCommand(CMD_PRE_SCAN, 0, NULL);
    mFmCallback = callback;
    mFmCallback->linkToDeath(mFmHidlDeathRecipient, 0);

    pthread_mutex_init(&mRdsLock, (const pthread_mutexattr_t *) NULL);
    pthread_cond_init(&mRdsCond, (const pthread_condattr_t *) NULL);
    pthread_create(&mRdsThread, NULL, fm_rds_thread, this);

    return Result::OK;
}

Return<Result> FmHidlService::disable()
{
    int ret = 0;

    if (!mEnabled) {
        LOGW("Not enabled.\n");
        return Result::FAILED;
    }

    LOGI("%s\n", __func__);
    ret = FMR_pwr_down(mFmFd, 0);
    ret = FMR_close_dev(mFmFd);

    mEnabled = false;
    mFmTunned = false;
    mInPwrMode = false;
    mSearching = false;
    mRdsReady = false;
    mSearchCancelled = false;
    mLastRdsStatus = 0;
    mFmCallback = NULL;

    if (!mRdsOn) {
        LOGI("Send signal to RDS thread.\n");
        pthread_cond_signal(&mRdsCond);
    }

    LOGI("Wait RDS thread to be terminated.\n");
    pthread_join(mRdsThread, (void **) NULL);
    mRdsOn = false;

    return ret > 0 ? Result::FAILED : Result::OK;
}

Return<Result> FmHidlService::startSearch(Direction dir)
{
    FmCommandType type;

    LOGI("Start search, direction: %d.\n", dir);

    if (mFmCallback == NULL || !mEnabled)
        return Result::NOT_INITIALIZED;

    if (mSearchMode == SearchMode::SEEK) {
        type = (dir == Direction::UP) ? CMD_SEEK_UP : CMD_SEEK_DOWN;
    } else {
        type = (dir == Direction::UP) ? CMD_SCAN_UP : CMD_SCAN_DOWN;
    }
    pthread_mutex_lock(&mCommandLock);
    sendCommand(type, 0, NULL);
    pthread_mutex_unlock(&mCommandLock);
    return Result::OK;
}

Return<Result> FmHidlService::cancelSearch()
{
    LOGI("Cancel search.\n");
    if (!mSearching)
        LOGW("Not in search state.\n");
    else {
        FMR_stop_scan(mFmFd);
        mSearchCancelled = true;
    }
    return Result::OK;
}

Return<Result> FmHidlService::setControl(Control id, uint32_t value)
{
    int ret = 0;
    Result res = Result::OK;

    LOGI("SetControl id: %d, value: %d", id, value);
    switch(id) {
    case Control::SEARCH_MODE:
    {
        SearchMode temp = static_cast<SearchMode>(value);
        mSearchMode = temp;
    }
        break;
    case Control::SCAN_DEWELL:
        if (value >= 0 && value <= 7)
            mScanDwell = value;
        else
            res = Result::INVALID_ARGUMENTS;
        break;
    case Control::REGION:
    {
        Region temp = static_cast<Region>(value);
        if (temp == Region::UE)
            FMR_set_region(FM_BAND_UE);
        else if (temp == Region::JAPAN)
            FMR_set_region(FM_BAND_JAPAN);
        else if (temp == Region::JAPANW)
            FMR_set_region(FM_BAND_JAPANW);
        else
            FMR_set_region(FM_BAND_SPECIAL);
        mRegion = temp;
    }
        break;
    case Control::EMPHASIS:
    {
        Deemphasis temp = static_cast<Deemphasis>(value);
        mEmphasis = temp;
    }
        break;

    case Control::SPACING:
    {
        Spacing temp = static_cast<Spacing>(value);
        if (temp == Spacing::SPACE50)
            FMR_set_customized_space(5);
        else if (temp == Spacing::SPACE100)
            FMR_set_customized_space(10);
        else if (temp == Spacing::SPACE200)
            FMR_set_customized_space(20);
        mSpacing = temp;
    }
        break;
    case Control::RDS_STD:
    {
        Rds temp = static_cast<Rds>(value);
        mRds = temp;
    }
        break;
    case Control::LP_MODE:
    {
        mInPwrMode = value ? true : false;
        if (mRdsOn) {
            if (mInPwrMode)
                ret = FMR_turn_on_off_rds(mFmFd, FMR_RDS_OFF);
            else
                ret = FMR_turn_on_off_rds(mFmFd, FMR_RDS_ON);
            LOGI("Set LP mode %d, result: %d.\n", mInPwrMode, ret);
        }
    }
        break;
    default:
        break;
    }
    return Result::OK;
}

Return<Result> FmHidlService::getControl(Control id)
{
    UNUSED(id);
    return Result::OK;
}

Return<void> FmHidlService::getFreq(getFreq_cb _hidl_cb)
{
    _hidl_cb(Result::OK, mTuneFreq);
    return Void();
}

Return<void> FmHidlService::getRssi(getRssi_cb _hidl_cb)
{
    int ret = 0, rssi = -1;
    Result res = Result::OK;

    ret = FMR_get_rssi(mFmFd, &rssi);
    if(ret)
        res = Result::FAILED;
    _hidl_cb(res, rssi);
    return Void();
}

Return<Result> FmHidlService::setFreq(uint32_t freq)
{
    LOGI("FM Tune freq: %d", freq);
    mTuneFreq = freq;
    pthread_mutex_lock(&mCommandLock);
    sendCommand(CMD_TUNE, 0, NULL);
    pthread_mutex_unlock(&mCommandLock);
    return Result::OK;
}

Return<Result> FmHidlService::setBand(uint32_t low, uint32_t high)
{
    LOGI("Update band boundary, low: %d, high: %d",
        low, high);
    FMR_set_customized_band_boundary((int) low, (int) high);
    return Result::OK;
}

Return<Result> FmHidlService::setRdsOnOff(uint32_t onOff)
{
    int ret = 0;
    LOGI("Set RDS onoff: %d", onOff);

    if (!mEnabled) {
        LOGI("FM not enabled.");
        return Result::NOT_INITIALIZED;
    }

    if (onOff == 0) {
        if (mRdsOn) {
            ret = FMR_turn_on_off_rds(mFmFd, FMR_RDS_OFF);
            if (ret == 0)
                mRdsOn = false;
        }
    } else if (onOff == 1) {
        if (!mRdsOn) {
            ret = FMR_turn_on_off_rds(mFmFd, FMR_RDS_ON);
            if (ret == 0) {
                pthread_mutex_lock(&mRdsLock);
                mRdsOn = true;
                pthread_cond_signal(&mRdsCond);
                pthread_mutex_unlock(&mRdsLock);
            }
        }
    } else {
        LOGE("Wrong onOff value: %d", onOff);
        return Result::INVALID_ARGUMENTS;
    }
    return Result::OK;
}

Return<void> FmHidlService::getRdsStatus(getRdsStatus_cb _hidl_cb)
{
    int ret = 0;

    _hidl_cb(Result::OK, mLastRdsStatus);

    return Void();
}

Return<void> FmHidlService::getRadioText(getRadioText_cb _hidl_cb)
{
    int ret = 0, rtLen = 0;
    unsigned char *rt = NULL;
    char *rtTemp = NULL;

    ret = FMR_get_rt(mFmFd, &rt, &rtLen);

    if (ret != 0 || rtLen == 0)
        _hidl_cb(Result::FAILED, "");
    else {
        rtTemp = (char*) malloc(rtLen + 1);
        if (rtTemp == NULL) {
            LOGW("Can't allocate radio text buffer.");
            _hidl_cb(Result::FAILED, "");
        } else {
	          for (int i = 0; i < rtLen; i++) {
	              memcpy(rtTemp, rt, rtLen);
	          }
	          *(rtTemp + rtLen) = '\0';
	          _hidl_cb(Result::OK, rtTemp);
	      }
    }
    return Void();
}

Return<void> FmHidlService::getPrgmServices(getPrgmServices_cb _hidl_cb)
{
    int ret = 0, psLen = 0;
    unsigned char *ps = NULL;
    char *psTemp = NULL;

    ret = FMR_get_ps(mFmFd, &ps, &psLen);

    if (ret != 0 || psLen == 0)
        _hidl_cb(Result::FAILED, "");
    else {
        psTemp = (char*) malloc(psLen + 1);
        if (psTemp == NULL) {
            LOGW("Can't allocate radio text buffer.");
            _hidl_cb(Result::FAILED, "");
        } else {
            for (int i = 0; i < psLen; i++) {
                memcpy(psTemp, ps, psLen);
            }
            *(psTemp + psLen) = '\0';
            _hidl_cb(Result::OK, psTemp);
        }
    }
    return Void();
}

Return<void> FmHidlService::getPrgmId(getPrgmId_cb _hidl_cb)
{
    int ret = 0;
    unsigned short pi = 0;
    Result res = Result::OK;

    ret = FMR_get_pi(mFmFd, &pi);
    if (ret) {
        pi = -1;
        LOGW("Failed to get pi, ret = %d", ret);
        res = Result::FAILED;
    }
    _hidl_cb(res, pi);
    return Void();
}

Return<void> FmHidlService::getPrgmType(getPrgmType_cb _hidl_cb)
{
    int ret = 0;
    unsigned char pty = 0;
    Result res = Result::OK;

    ret = FMR_get_pty(mFmFd, &pty);
    if (ret) {
        pty = -1;
        LOGW("Failed to get pty, ret = %d", ret);
        res = Result::FAILED;
    }
    _hidl_cb(res, pty);
    return Void();
}

Return<void> FmHidlService::getAfInfo(getAfInfo_cb _hidl_cb)
{
    int i = 0, len = 0, ret = 0;
    int16_t *list = NULL;

    ret = FMR_get_af_list(mFmFd, &list, &len);

    if (ret != 0 || len == 0) {
        LOGI("Failed to get AF list.");
        mAfList[0] = 0;
        _hidl_cb(Result::FAILED, mAfList);
    }
    else {
        len = len > 49 ? 49 : len;
        LOGI("AF list lengh: %d.", len);
        mAfList[0] = len;
        for (i = 0; i < len; i++) {
            LOGI("Add AF freq (%d) to list.", list[i]);
            mAfList[1 + i] = list[i];
        }
        _hidl_cb(Result::OK, mAfList);
    }
    return Void();
}

int FmHidlService::sendCommand(FmCommandType type, unsigned int delay_ms, void *param)
{
    UNUSED(param);
    struct FmCommand *cmd = (struct FmCommand *)calloc(1, sizeof(struct FmCommand));
    struct timespec ts;

    LOGE("Send command.\n");
    if (cmd == NULL)
        return -ENOMEM;

    cmd->type = type;

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += delay_ms/1000;
    ts.tv_nsec += (delay_ms%1000) * 1000000;

    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec += 1;
    }
    cmd->ts = ts;
    list_add_tail(&mFmCommandList, &cmd->node);
    pthread_cond_signal(&mCommandCond);
    LOGE("Add command.\n");
    return 0;
}

void* FmHidlService::fm_rds_thread(void *context)
{
    FmHidlService *service = (class FmHidlService *) context;
    int ret;

    prctl(PR_SET_NAME, (unsigned long)"fm_hidl_service RDS thread", 0, 0, 0);
    pthread_mutex_lock(&service->mRdsLock);

    while (1) {
        LOGI("In fm_rds_thread.");
        if (!service->mRdsOn) {
            LOGI("RDS disabled, wait...");
            pthread_cond_wait(&service->mRdsCond, &service->mRdsLock);
        }

        if (!service->mEnabled) {
            LOGI("Exit RDS thread.");
            goto exit;
        }

        ret = FMR_read_rds_data(service->mFmFd, &service->mLastRdsStatus);
        if (ret == 0) {
            LOGI("RDS ready.");
            service->mRdsReady = true;
            if (service->mFmCallback) {
                auto _ret = service->mFmCallback->eventNotifyCb(Result::OK, Event::RDS_AVAL);
                if (!_ret.isOk()) {
                    LOGI("Failed to notify event.");
                }
            }
        } else {
            LOGI("RDS not ready.");
            service->mRdsReady = false;
        }

        /* check RDS event every 1s */
        sleep(1);
    }

exit:
    pthread_mutex_unlock(&service->mRdsLock);
    LOGI("Exit fm_hidl_service RDS thread.");
    return NULL;
}
void* FmHidlService::fm_hidl_thread(void *context)
{
    //pthread_mutex_lock(&mCommandLock);
    int ret = 0;
    FmHidlService *service = (class FmHidlService *) context;
    while(1) {
        struct FmCommand *cmd = NULL;
        struct listnode *item;
        struct listnode *tmp;
        struct timespec cur_ts;

        if (list_empty(&service->mFmCommandList)) {
            LOGI("List empty.\n");
            pthread_cond_wait(&service->mCommandCond, &service->mCommandLock);
        }
        clock_gettime(CLOCK_REALTIME, &cur_ts);

        list_for_each_safe(item, tmp, &service->mFmCommandList) {
            cmd = node_to_item(item, struct FmCommand, node);

            if (cmd->type == CMD_EXIT) {
                LOGI("Exit command thread, remove all commands.\n");
                list_remove(item);
                free(cmd);
                list_for_each_safe(item, tmp, &service->mFmCommandList) {
                    cmd = node_to_item(item, struct FmCommand, node);
                    list_remove(item);
                    free(cmd);
                }
                goto exit;
            }

            if (!service->mEnabled) {
                LOGI("Disabled, clear all pending commands.");
                list_remove(item);
                free(cmd);
                continue;
            }
            list_remove(item);
            switch (cmd->type) {
            case CMD_PRE_SCAN:
                LOGI("Start pre-search.");
                FMR_Pre_Search(service->mFmFd);
                ret = FMR_scan(service->mFmFd, service->mScanTable, &service->mScanChlCount);
                FMR_Restore_Search(service->mFmFd);
                LOGI("Finish pre-search, scanned channel count: %d.", service->mScanChlCount);
                break;
            case CMD_TUNE:
                if (service->mRdsOn && service->mFmTunned) {
                    ret = FMR_turn_on_off_rds(service->mFmFd, FMR_RDS_OFF);
                    if (ret)
                        LOGE("%s turn off rds fail.", __func__);
                }
                ret = FMR_tune(service->mFmFd, service->mTuneFreq);
                if (ret)
                    service->mFmTunned = false;
                else
                    service->mFmTunned = true;

                if (service->mRdsOn) {
                    ret = FMR_turn_on_off_rds(service->mFmFd, FMR_RDS_ON);
                }
                if (service->mFmCallback) {
                    if (service->mFmTunned) {
                        LOGI("Tunned to %d.", service->mTuneFreq);
                        auto _ret = service->mFmCallback->eventNotifyCb(Result::OK, Event::TUNE);
                        if (!_ret.isOk()) {
                            LOGI("Failed to notify event.");
                        }
                    } else {
                        LOGI("Tunned to %d failed.", service->mTuneFreq);
                        auto _ret = service->mFmCallback->eventNotifyCb(Result::FAILED, Event::TUNE);
                        if (!_ret.isOk()) {
                            LOGI("Failed to notify event.");
                        }
                    }
                }
                break;
            case CMD_SEEK_UP:
            case CMD_SEEK_DOWN:
            case CMD_SCAN_UP:
            case CMD_SCAN_DOWN:
            {
                bool isScan = false, scanDone = false;
                int round = 0;
                int isUp = 0;
                int maxFreq, minFreq, scanFreq;
                int startFreq = service->mTuneFreq;
                int origFreq = service->mTuneFreq;
#if 0
                int channelCount = 0;
                unsigned short scanTBL[2];
#endif
                Result res;

                service->mSearching = true;

                if (cmd->type == CMD_SCAN_UP || cmd->type == CMD_SCAN_DOWN)
                    isScan = true;
                if (cmd->type == CMD_SCAN_UP || cmd->type == CMD_SEEK_UP)
                    isUp = 1;
                FMR_get_band_boundary(&minFreq, &maxFreq);

                if (startFreq < minFreq || startFreq > maxFreq) {
                    LOGI("%s Invalid start freq (%d), adjust start freq to min freq.", __func__, startFreq);
                    startFreq = minFreq;
                }

                LOGI("%s Start search station, scan: %d, dir: %d, start: %d, min: %d, max: %d.",
                    __func__, isScan, isUp, startFreq, minFreq, maxFreq);

                do {

                    if (service->mRdsOn && service->mFmTunned) {
                        ret = FMR_turn_on_off_rds(service->mFmFd, FMR_RDS_OFF);
                        if (ret)
                            LOGE("%s turn off rds fail.", __func__);
                    }
                    ret = FMR_set_mute(service->mFmFd, 1);
#if 1
                    //ret = FMR_seek_cust(service->mFmFd, startFreq, minFreq, maxFreq, isUp, &scanFreq);
                    ret = FMR_seek(service->mFmFd, startFreq, isUp, &scanFreq);
#else
                    FMR_Pre_Search(service->mFmFd);
//                    ret = FMR_scan_cust(service->mFmFd,
//                        startFreq, minFreq, maxFreq, scanTBL, &channelCount);
                    if (ret == 0)
                        scanFreq = scanTBL[0];

                    FMR_Restore_Search(service->mFmFd);
#endif
                    if (ret == 0)
                        ret = FMR_tune(service->mFmFd, scanFreq);

                    if (ret) {
                        LOGE("Failed to find station (%d), ret: %d.", scanFreq, ret);
                        ret = FMR_tune(service->mFmFd, service->mTuneFreq);
                        LOGW("Tune to last tuned frequency: %d, ret: %d",
                            service->mTuneFreq, ret);
                        if (ret)
                            service->mFmTunned = false;
                        else
                            ret = FMR_set_mute(service->mFmFd, 0);
                    }
                    else {
                        LOGI("Found station, freq: %d.", scanFreq);
                        service->mFmTunned = true;
                        service->mTuneFreq = scanFreq;
                        ret = FMR_set_mute(service->mFmFd, 0);
                    }
                    res = service->mFmTunned ? Result::OK : Result::FAILED;
                    if (service->mFmCallback) {
                        auto _ret = service->mFmCallback->eventNotifyCb(res, Event::TUNE);
                        if (!_ret.isOk()) {
                             LOGI("Failed to notify event.");
                        }
                    }
                    if (service->mRdsOn && service->mFmTunned) {
                        ret = FMR_turn_on_off_rds(service->mFmFd, FMR_RDS_ON);
                    }

                    if (isScan) {
                        if (scanFreq == 0 || scanFreq == startFreq) {
                            LOGI("%s Scan failed.", __func__);
                            scanDone = true;
                        }
                        // Wait mScanDwell seconds
                        sleep(service->mScanDwell);
                        if (!service->mSearchCancelled) {
                            if ((isUp == 1 && scanFreq < startFreq)
                                || (isUp == 0 && scanFreq > startFreq))
                                round++;
                            if ((round == 1 && ((isUp == 1 && scanFreq > origFreq)
                                || (isUp == 0 && scanFreq < origFreq)))
                                || (scanFreq == origFreq) || (round == 2)) {
                                scanDone = true;
                                LOGI("%s Search complete.", __func__);
                            }

                            if (service->mFmCallback && !scanDone) {
                                auto _ret = service->mFmCallback->eventNotifyCb(Result::OK, Event::SEARCH_IN_PROGRESS);
                                if (!_ret.isOk()) {
                                    LOGI("Failed to notify event.");
                                }
                                LOGI("%s Search in progress", __func__);
                            }
                            startFreq = scanFreq;
                        }
                    }
                } while (isScan && !service->mSearchCancelled && !scanDone && service->mEnabled);

                if (service->mFmCallback) {
                    auto _ret = service->mFmCallback->eventNotifyCb(Result::OK, Event::SEARCH_COMPLETE);
                    if (!_ret.isOk()) {
                        LOGI("Failed to notify event.");
                    }
                }
                if (service->mSearchCancelled) {
                    if (service->mFmCallback) {
                        auto _ret = service->mFmCallback->eventNotifyCb(Result::OK, Event::SEARCH_CANCELLED);
                        if (!_ret.isOk()) {
                            LOGI("Failed to notify event.");
                        }
                    }
                    service->mSearchCancelled = false;
                    LOGI("%s Search cancelled.", __func__);
                }
                service->mSearching = false;
            }
            default:
                break;
            }
            free(cmd);
        }
    }
exit:
    return NULL;
}

}  // implementation
}  // namespace V1_0
}  // namespace fm
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

int main(int /*argc*/, char** argv)
{
    UNUSED(argv);
    LOG(INFO) << "FM Hal is starting...";

    configureRpcThreadpool(1, true /* callerWillJoin */);

    android::sp<vendor::mediatek::hardware::fm::V1_0::IFmRadio> service =
        new vendor::mediatek::hardware::fm::V1_0::implementation::FmHidlService;
    CHECK_EQ(service->registerAsService("fm_hidl_service"), android::NO_ERROR)
        << "Failed to register fm HAL";

    joinRpcThreadpool();
    LOG(INFO) << "FM Hal is terminating...";
    return 0;
}