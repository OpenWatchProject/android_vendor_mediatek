#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <poll.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cutils/properties.h>
#include <cutils/sockets.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#define ATRACE_TAG  ATRACE_TAG_VIDEO
#include <utils/Trace.h>
#include <utils/Timers.h>

#include "VTAVSync.h"

// Adjust for ms
#define AVSYNC_CAL_TIME (-60)

// For MD state query
#include "hardware/ccci_intf.h"

typedef enum {
    MD_BOOT_STAGE_0 = 0,
    MD_BOOT_STAGE_1 = 1,
    MD_BOOT_STAGE_2 = 2, /* boot up ready */
    MD_BOOT_STAGE_EXCEPTION = 3,
    MD_ACK_SCP_INIT = 4, /* for SCP sync convinince, not legal for md->boot_stage */
} AVSYNC_MD_BOOT_STAGE; /* for other module */


namespace VTService {

pthread_t g_pthreadDownlink;
bool g_bAVSyncDownlinkThreadRunning = false;
Mutex g_lockAVsync;

int g_fd_avsync=0;
struct pollfd g_poll_fds[2];
int g_poll_cnt = 0;

VTAVSync::VTAVSync() {
    char prop_val[100]={0};

    VTAVSYNC_LOGW("VTAVSync enter!!!");

    /*Clear parameters*/
    memset(&mAV, 0 , sizeof(av_param_t));

    /*open FD*/
    VTAVSYNC_ASSERT(openFD(), "openFD(para.video_ebi) fail");

    /*check FD*/
    checkFD();

    /*AVSync Init*/     
    avInit();

    /*Create DL Thread*/
    g_bAVSyncDownlinkThreadRunning = true;
    pthread_create(&g_pthreadDownlink, NULL, VTAVSync::downlinkThread, this);
}

VTAVSync::~VTAVSync() {
    VTAVSYNC_LOGW("~VTAVSync enter");

    g_bAVSyncDownlinkThreadRunning = false;

    pthread_join(g_pthreadDownlink, NULL);

    VTAVSYNC_LOGW("join exit");
    /*Close FD*/
    VTAVSYNC_ASSERT(closeFD(), "closeFD(mParam.video_ebi) fail");
}

/* for static member, it needs to define it before use */
av_param_t VTAVSync::mAV;

void VTAVSync::getDLAudioTimeStamp(int64_t *pAudioTime) {
    //Handle TZ AVSync issue
    int64_t cur_st=0;

    /* protect from possible mAV modification of HandleDLAVSync */
    Mutex::Autolock lock(g_lockAVsync);

    if(mAV.nsNTP ==0){ //NO Audio RTCP DL update
        *pAudioTime=0;
        VTAVSYNC_LOGD("getDLAudioTimeStamp, DLAudioTime=%llx", *pAudioTime);        
        return;
    }

    cur_st = systemTime();

    *pAudioTime = mAV.nsNTP + ((cur_st - mAV.st)/1000) + ((AVSYNC_CAL_TIME) * 1000);
    VTAVSYNC_LOGD("getDLAudioTimeStamp, NTPus=%llx, pre=%llx, cur=%llx, DLAudioTime=%llx", mAV.nsNTP, mAV.st, cur_st, *pAudioTime);
    return;
}

void VTAVSync::HandleDLAVSync(unsigned char* buffer) {
    av_msg_t *av_msg = (av_msg_t *)buffer;
    VTAVSYNC_LOGW("HandleDLAVSync enter, msg type=%d", av_msg->type);
    switch(av_msg->type){
        case AV_DL_AUDIO_NTP:
        {            
            mAV.st = systemTime();
            /*Transfer to ns*/
            VTAVSYNC_LOGW("av_msg->integer=%x, av_msg->fraction=%x", av_msg->integer, av_msg->fraction);
            mAV.nsNTP =  av_msg->integer;
            mAV.nsNTP *= 1000000; //sepereate this line in-order to escape av_msg->integer 32bits
            mAV.nsNTP += (av_msg->fraction/US_TO_NTP);
            avInit();
            break;
        }
	case AV_DL_AUDIO_RESET:
        {
            memset(&mAV, 0 , sizeof(av_param_t));
            avInit();
        }
        default:            
            break;
    }
    return;
}

int VTAVSync::openFD(void) {
    g_fd_avsync = open(VOLTE_AV_DEV_NAME, O_RDWR);

    int retry = 0;
    while (g_fd_avsync < 0) {
        VTAVSYNC_LOGE("open dev failed: retry=%d, g_fd_avsync=%d, errno=%d, %s", retry , g_fd_avsync, errno, strerror(errno));
        retry++;
        usleep(500 * 1000);
        g_fd_avsync = open(VOLTE_AV_DEV_NAME, O_RDWR);
    }
    
    VTAVSYNC_LOGW("open %s done, fd=%d", VOLTE_AV_DEV_NAME, g_fd_avsync);
    return 1;
}

int VTAVSync::closeFD(void) {
    int result = -1;
    VTAVSYNC_ASSERT(g_fd_avsync !=0, "fd_avsync is zero");
    result = close(g_fd_avsync);
    VTAVSYNC_ASSERT(result == 0, "g_fd_avsync close failed, errno=%d(%s)", errno, strerror(errno));
    g_fd_avsync = 0;
    return 1;
}

int VTAVSync::writeFD(int fd, void * buffer, int size) {
    int len = write(fd, buffer, size);
    if (len < 0) {
        VTAVSYNC_LOGE("writeFD fail: fd=%d, %d, %s", fd, errno, strerror(errno));
    } else if (len != size) {
        VTAVSYNC_LOGE("writeFD fail: len=%d, size=%d", len, size);
    } 
    return len;
}

void VTAVSync::checkFD(void) {
    int ret = 0;

    while (1) {
        ioctl(g_fd_avsync, CCCI_IOC_GET_MD_STATE, &ret);
        if (ret < 0) {
            VTAVSYNC_LOGE("ccci query md status error: fd=%d, %d, %s", g_fd_avsync, errno, strerror(errno));
            break;
        } else if (ret == MD_BOOT_STAGE_2) {
            VTAVSYNC_LOGI("CCCI status:MD ready");
            break;
        } else {
            sleep(1);
        }
    };
}

void VTAVSync::avInit(void) {
    
    int ret;
    av_msg_t av_msg;

    mAV.st = systemTime();

    av_msg.type = AV_UL_NTP;
    av_msg.integer  = (mAV.st/1000000000);
    av_msg.fraction = (mAV.st%1000000000) * NS_TO_NTP; //1ns == 4.3 NTP

    ret = writeFD(g_fd_avsync, (void*)&(av_msg), sizeof(av_msg_t));
 
    int retry = 0;
    while (ret < 0) {
        VTAVSYNC_LOGE("write avInit failed: retry=%d, ret=%d, errno=%d, %s", retry , ret, errno, strerror(errno));
        retry++;
        sleep(1);
        mAV.st = systemTime();

        av_msg.type = AV_UL_NTP;
        av_msg.integer  = (mAV.st/1000000000);
        av_msg.fraction = (mAV.st%1000000000) * NS_TO_NTP; //1ns == 4.3 NTP

        ret = writeFD(g_fd_avsync, (void*)&(av_msg), sizeof(av_msg_t));
    }
    VTAVSYNC_LOGI("avInit, st %llx, int=%x, frac=%x", 
         mAV.st, av_msg.integer, av_msg.fraction);
}

void VTAVSync::resetParam(void) {
    /*Clear parameters*/
    memset(&mAV, 0 , sizeof(av_param_t));
}

void* VTAVSync::downlinkThread(void *arg) {
    VTAVSync *me = reinterpret_cast<VTAVSync *> (arg);

    VTAVSYNC_LOGW("[AVSYNC] downlink_thread() entry");

    int ret = 0;

    nice(-16);

    prctl(PR_SET_NAME, (unsigned long)"downlink_thread", 0, 0, 0);

    VTAVSYNC_ASSERT(g_fd_avsync !=0, "g_fd_avsync is zero");

    VTAVSYNC_LOGE("g_fd_avsync =%d", g_fd_avsync);


    g_poll_fds[0].fd = g_fd_avsync;   // file descriptor (int)
    g_poll_fds[0].events = POLLIN;   // requested events (short)
    g_poll_fds[0].revents = 0;        // returned events (short)
    g_poll_cnt++;

    g_bAVSyncDownlinkThreadRunning = true;

    while (g_bAVSyncDownlinkThreadRunning) {
        if ((ret = poll(g_poll_fds, g_poll_cnt, 10000)) <= 0) {
            if (ret == 0) {
                //VTAVSYNC_LOGE("poll result TIMEOUT");
            } else {
                VTAVSYNC_LOGE("poll result fail %d", ret);
                continue;
            }
        }

        for (int i = 0; i < g_poll_cnt; i++) {
            if (g_poll_fds[i].revents & POLLIN) {
                if (g_poll_fds[i].fd == g_fd_avsync) {
                    /*Handle AVSync Message*/
                    unsigned char buffer[4096];
                    int len = read(g_fd_avsync, buffer, sizeof(buffer));
                    VTAVSYNC_LOGW("[AVSYNC] read avsync data: len=%d, dump %x %x %x %x. %x %x %x %x.  %x %x %x %x. %x %x %x %x.",
                                    len, 
                                    buffer[0], buffer[1], buffer[2], buffer[3],
                                    buffer[4], buffer[5], buffer[6], buffer[7],
                                    buffer[8], buffer[9], buffer[10], buffer[11],
                                    buffer[12], buffer[13], buffer[14], buffer[15]);

                    Mutex::Autolock lock(g_lockAVsync);
                    me->HandleDLAVSync(buffer);
                }
            }
        }
    }

    g_poll_fds[0].fd = 0;
    g_poll_fds[0].events = 0;
    g_poll_fds[0].revents = 0;
    g_poll_cnt = 0;

    VTAVSYNC_LOGW("[AVSYNC] downlink_thread() exit");
    return 0;
}

}
