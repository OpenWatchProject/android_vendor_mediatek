/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   
 *
 * Project:
 * --------
 *   
 *
 * Description:
 * ------------
 *   
 *
 * Author:
 * -------
 *   
 *
 ****************************************************************************/

#define LOG_TAG "batterywarning"

#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <utils/String16.h>
#include <binder/BinderService.h>
#include <binder/Parcel.h>
#include <cutils/uevent.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAX_CHAR 100
//M: Currently disabling battery path.. in later stage we will update in code to check path by order. @{
//#ifdef MTK_GM_30
//#define FILE_NAME "/sys/devices/platform/charger/BatteryNotify"
//#else
//#define FILE_NAME "/sys/devices/platform/mt-battery/BatteryNotify"
//#endif
//M: @}
#define ACTION "mediatek.intent.action.BATTERY_WARNING"
#define NORMAL_ACTION "mediatek.intent.action.BATTERY_NORMAL"
#define TYPE "type"
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
using namespace android;

#define MAX_EPOLL_EVENTS 40
static int uevent_fd;
static int epollfd;
static int eventct;

bool sendBroadcastMessage(String16 action, int value)
{
    ALOGD("sendBroadcastMessage(): Action: %s, Value: %d ", (char *)(action.string()), value);
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> am = sm->getService(String16("activity"));
    if (am != NULL) {
        Parcel data, reply;
        data.writeInterfaceToken(String16("android.app.IActivityManager"));
        data.writeStrongBinder(NULL);
        // Add for match AMS change on O
        data.writeInt32(1);
        // intent begin
        data.writeString16(action); // action
        data.writeInt32(0); // URI data type
        data.writeString16(NULL, 0); // type
        data.writeString16(NULL, 0); // mIdentifier
        data.writeInt32(0x04000000); // flags: FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT
        data.writeString16(NULL, 0); // package name
        data.writeString16(NULL, 0); // component name
        data.writeInt32(0); // source bound - size
        data.writeInt32(0); // categories - size
        data.writeInt32(0); // selector - size
        data.writeInt32(0); // clipData - size
        data.writeInt32(-2); // contentUserHint: -2 -> UserHandle.USER_CURRENT
        data.writeInt32(-1); // bundle extras length
        data.writeInt32(0x4C444E42); // 'B' 'N' 'D' 'L'
        int oldPos = data.dataPosition();
        data.writeInt32(1);  // size
        // data.writeInt32(0); // VAL_STRING, need to remove because of analyze common intent
        data.writeString16(String16(TYPE));
        data.writeInt32(1); // VAL_INTEGER
        data.writeInt32(value);
        int newPos = data.dataPosition();
        data.setDataPosition(oldPos - 8);
        data.writeInt32(newPos - oldPos); // refill bundle extras length
        data.setDataPosition(newPos);
        // intent end
        data.writeString16(NULL, 0); // resolvedType
        data.writeStrongBinder(NULL); // resultTo
        data.writeInt32(0); // resultCode
        data.writeString16(NULL, 0); // resultData
        data.writeInt32(-1); // resultExtras
        data.writeString16(NULL, 0); // permission
        data.writeInt32(0); // appOp
        data.writeInt32(-1); // option
        data.writeInt32(1); // serialized: != 0 -> ordered
        data.writeInt32(0); // sticky
        data.writeInt32(-2); // userId: -2 -> UserHandle.USER_CURRENT

        status_t ret = am->transact(IBinder::FIRST_CALL_TRANSACTION + 11, data, &reply); // BROADCAST_INTENT_TRANSACTION
        if (ret == NO_ERROR) {
            int exceptionCode = reply.readExceptionCode();
            if (exceptionCode) {
                ALOGE("sendBroadcastMessage(%s) caught exception %d\n",
                        (char *)(action.string()), exceptionCode);
                return false;
            }
        } else {
            return false;
        }
    } else {
        ALOGE("getService() couldn't find activity service!\n");
        return false;
    }
    return true;
}


static const char *charger_file_path[] = {
    "/sys/devices/platform/charger/BatteryNotify",
    "/sys/devices/platform/mt-battery/BatteryNotify",
};

static int read_from_file(const char* path) {
  if(!path) {
     return 0;

  }
  int fd =open(path,O_RDONLY);

  if(fd<0) {
      close(fd);
      return 0;

  }
  else {
      close(fd);
      return 1;
  }
}

int get_charger_file_path() {
  int i = 0;
  for(i=0;i<ARRAY_SIZE(charger_file_path);i++) {
      if(read_from_file(charger_file_path[i])) {
         return i;
      }
  }
  return 0;
}

void readType(char* buffer) {
    FILE * pFile;
    int file_index;
    file_index=get_charger_file_path();
    ALOGD("Inside file_index value : %d\n", file_index);
    pFile = fopen(charger_file_path[file_index], "r");
    if (pFile == NULL) {
        ALOGE("error opening file");
        return;
    } else {
        if (fgets(buffer, MAX_CHAR, pFile) == NULL) {
            fclose(pFile);
            ALOGE("can not get the string from the file");
            return;
        }
    }
    fclose(pFile);
    int type = atoi(buffer);
    if (type==0)
    {
        ALOGD("start activity by send intent to BatteryWarningReceiver to remove notification, type = %d\n", type);
        sendBroadcastMessage(String16(NORMAL_ACTION), type);
    }
    if (type > 0)
    {
        ALOGD("start activity by send intent to BatteryWarningReceiver, type = %d\n", type);
        sendBroadcastMessage(String16(ACTION), type);
    }
}

#define UEVENT_MSG_LEN 2048
static void uevent_event(uint32_t /*epevents*/) {
    char msg[UEVENT_MSG_LEN + 2];
    char *cp;
    char *status;
    int n;
    char *buffer = (char*) malloc(MAX_CHAR * sizeof(char));
    if (buffer == NULL) {
        ALOGD("malloc memory failed");
        return ;
    }
    n = uevent_kernel_multicast_recv(uevent_fd, msg, UEVENT_MSG_LEN);
    if (n <= 0) return;
    if (n >= UEVENT_MSG_LEN) /* overflow -- discard */
        return;

    msg[n] = '\0';
    msg[n + 1] = '\0';
    cp = msg;

    while (*cp) {

        if (!strncmp(cp, "CHGSTAT=", strlen("CHGSTAT="))) { // This CHGSTAT value will be provided by kernel driver
            readType(buffer);
            break;
        }

        /* advance to after the next \0 */
        while (*cp++)
            ;
    }
    free(buffer);
}

int batterywarn_register_event(int fd, void (*handler)(uint32_t)) {
    struct epoll_event ev;
    ev.events = EPOLLIN;

    //if (wakeup == EVENT_WAKEUP_FD) ev.events |= EPOLLWAKEUP;

    ev.data.ptr = (void*)handler;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        ALOGD("epoll_ctl failed; errno=%d\n", errno);
        return -1;
    }
    return 0;
}


static void uevent_init(void) {
    uevent_fd = uevent_open_socket(64 * 1024, true);
    if (uevent_fd < 0) {
        ALOGD("uevent_init: uevent_open_socket failed\n");
        return;
    }

    fcntl(uevent_fd, F_SETFL, O_NONBLOCK);
    if (batterywarn_register_event(uevent_fd, uevent_event))
        ALOGD("register for uevent events failed\n");
}
static void batterywarn_mainloop(void) {
    int nevents = 0;
    while (1) {
        struct epoll_event events[1];
        nevents = epoll_wait(epollfd, events, 1, -1);
        if (nevents == -1) {
            if (errno == EINTR) continue;
            ALOGD("batterywarn_mainloop: epoll_wait failed\n");
            break;
        }

        for (int n = 0; n < nevents; ++n) {
            if (events[n].data.ptr) (*(void (*)(int))events[n].data.ptr)(events[n].events);
        }
    }

    return;
}
static int batterywarn_init() {
    epollfd = epoll_create(MAX_EPOLL_EVENTS);
    if (epollfd == -1) {
        ALOGD("epoll_create failed; errno=%d\n", errno);
        return -1;
    }
    uevent_init();
    return 0;

}
int main()
{
    char *buffer = (char*) malloc(MAX_CHAR * sizeof(char));
    if (buffer == NULL) {
        ALOGD("malloc memory failed");
        return 0;
    }
  int ret;

  /* Read the status to catch the event when batterywarning is not started */
  readType(buffer);

  ret= batterywarn_init();
  if (ret) {
      ALOGD("Initialization failed, exiting\n");
      exit(1);
  }
  batterywarn_mainloop();
  free(buffer);
  return 0;
}

