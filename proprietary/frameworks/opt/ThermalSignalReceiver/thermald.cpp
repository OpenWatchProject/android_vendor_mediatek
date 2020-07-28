#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <utils/Log.h>
#include <cutils/log.h>
#include <sys/types.h>
#include <utils/String16.h>
#include <binder/BinderService.h>
#include <binder/Parcel.h>
#include <cmdtc.h>
#include <logwrap/logwrap.h>

#define FEATURE_MD_DIAG

#define ACTION "mediatek.intent.action.THERMAL_WARNING"
#ifdef FEATURE_MD_DIAG
#define MDWarninig "mediatek.intent.action.THERMAL_DIAG"
#endif
#define TYPE "type"

#ifdef FEATURE_MD_DIAG
/*
 * use "si_code" for Action identify
 */
enum {
/* TMD_Alert_ShutDown = 1, */
   TMD_Alert_ULdataBack = 2,
   TMD_Alert_NOULdata = 3
};
#endif

static int TMD_WMT_SET_THROS = 4;

#define TM_LOG_TAG "thermald"

#define TM_INFO_LOG(_fmt_, args...) \
    do { LOG_PRI(ANDROID_LOG_INFO, TM_LOG_TAG, _fmt_, ##args); } while(0)

#define PROCFS_MTK_CL_SD_PID "/proc/driver/thermal/clsd_pid"
#ifdef FEATURE_MD_DIAG
#define PROCFS_MTK_CLMUTT_TMD_PID "/proc/driver/thermal/clmutt_tmd_pid"
#endif

#define PROCFS_MTK_CL_WMT_PID "/proc/driver/thermal/clwmtx_pid"

#define WLAN_IFC_PATH "/sys/class/net/wlan0/operstate"
#define AP_IFC_PATH "/sys/class/net/ap0/operstate"
#define P2P_IFC_PATH "/sys/class/net/p2p0/operstate"

enum {
	WLAN_IFC = 0,
	AP_IFC = 1,
	P2P_IFC = 2,

	IFC_NUM /*Last one*/
};
static char IFC_NAME[IFC_NUM][17] = {
      "wlan0",
      "ap0",
      "p2p0"
};

static char IFC_PATH[IFC_NUM][50] = {
        WLAN_IFC_PATH,
        AP_IFC_PATH,
        P2P_IFC_PATH,
    };
using namespace android;
using namespace android::thermalcmdwrapper;

#define UNUSED(x)    if(x){}

bool sendBroadcastMessage(String16 action, int value)
{
    TM_INFO_LOG("sendBroadcastMessage(): Action: %s, Value: %d ", (char *) String8(action).string(), value);
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
        data.writeString16(NULL, 0);  // identifier
        data.writeInt32(0); // flags
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
                TM_INFO_LOG("sendBroadcastMessage(%s) caught exception %d\n",
                            (char *) String8(action).string(), exceptionCode);
                return false;
            }
        } else {
            return false;
        }
    } else {
        TM_INFO_LOG("getService() couldn't find activity service!\n");
        return false;
    }
    return true;
}

void throttle_reset(const char *iface) {
    char cmd[128];

    TM_INFO_LOG("reset %s qdisc", iface);

    sprintf(cmd, "qdisc del dev %s root", iface);
    execTcCmd(cmd);

    sprintf(cmd, "qdisc del dev %s ingress", iface);
    execTcCmd(cmd);

    execTcCmd("qdisc del dev ifb0 root");
}

int throttle_set_if(const char *iface, int txKbps) {
    char cmd[512];
    char ifn[65];

    memset(ifn, 0, sizeof(ifn));
    strncpy(ifn, iface, sizeof(ifn)-1);

    if (txKbps == -1) {
        throttle_reset(ifn);
        return 0;
    }

    /*
     * by mtk80842, reset configuration before setting
     */
    throttle_reset(ifn);

    /*
     * Target interface configuration
     */

    /*
     * Add root qdisc for the interface
     */
    sprintf(cmd, "qdisc add dev %s root handle 1: htb default 1 r2q 1000", ifn);
    if (execTcCmd(cmd)) {
        TM_INFO_LOG("Failed to add root qdisc (%s)", strerror(errno));
        goto fail;
    }

    /*
     * Add our egress throttling class
     */
    sprintf(cmd, "class add dev %s parent 1: classid 1:1 htb rate %dkbit", ifn, txKbps);
    if (execTcCmd(cmd)) {
        TM_INFO_LOG("Failed to add egress throttling class (%s)", strerror(errno));
        goto fail;
    }

    /*
     * Add ingress qdisc for pkt redirection
     */
    sprintf(cmd, "qdisc add dev %s ingress", ifn);
    if (execTcCmd(cmd)) {
        ALOGE("Failed to add ingress qdisc (%s)", strerror(errno));
        goto fail;
    }

    TM_INFO_LOG("throttle_set_if success, ifn = %s", ifn);

    return 0;
fail:
    throttle_reset(ifn);
    return -1;
}



static void set_wifi_throttle(int level)
{
	int i = 0;
        UNUSED(level);
	for ( i=0; i<IFC_NUM; i++) {
		TM_INFO_LOG("checking %s", IFC_PATH[i]);
		if (0 == access(IFC_PATH[i], R_OK)) {
			TM_INFO_LOG("hh[%d]step1", i);
			char buf[80];
			int fd = open(IFC_PATH[i], O_RDONLY);
			if (fd < 0) {
				TM_INFO_LOG("Can't open %s: %s", IFC_PATH[i], strerror(errno));
				continue;
			}
			TM_INFO_LOG("hh[%d]step2", i);
			int len = read(fd, buf, sizeof(buf) - 1);
			if (len < 0) {
				TM_INFO_LOG("Can't read %s: %s", IFC_PATH[i], strerror(errno));
                close(fd);
				continue;
			}
			close(fd);
			TM_INFO_LOG("hh[%d]step3", i);
			if(!strncmp (buf, "up", 2)) {
                                throttle_set_if(IFC_NAME[i], level);
			} else
				TM_INFO_LOG("%s is down!", IFC_NAME[i]);
		}
	}
}

static void signal_handler(int signo, siginfo_t *si, void *uc)
{
    static int cur_thro = 0;
    int set_thro = si->si_errno;

    UNUSED(signo);
    UNUSED(uc);
    switch(si->si_signo) {
    // Add more signo or code to expand thermald
      case SIGIO:
        if (1 == si->si_code) {
              //system("am start com.mediatek.thermalmanager/.ShutDownAlertDialogActivity");
              sendBroadcastMessage(String16(ACTION), 1);
              TM_INFO_LOG("thermal shutdown signal received, si_signo=%d, si_code=%d\n", si->si_signo, si->si_code);
        }
#ifdef FEATURE_MD_DIAG
        else if (TMD_Alert_ULdataBack == si->si_code) {
               sendBroadcastMessage(String16(MDWarninig), 0);
               TM_INFO_LOG("thermald signal received (TMD_Alert_ULdataBack), \
                           si_signo=%d, si_code=%d\n", si->si_signo, si->si_code);
              }
              else if (TMD_Alert_NOULdata == si->si_code) {
                 sendBroadcastMessage(String16(MDWarninig), 1);
                 TM_INFO_LOG("thermald signal received (TMD_Alert_NOULdata), \
                                            si_signo=%d, si_code=%d\n", si->si_signo, si->si_code);
              }
#endif
        else if (TMD_WMT_SET_THROS == si->si_code) {
            if (cur_thro != set_thro) {
                TM_INFO_LOG("set_wifi_throttle cur=%d set=%d\n", cur_thro, set_thro);
                set_thro = ((set_thro>=0)&&(set_thro<1000))?1000:set_thro; //1Kbps
                set_wifi_throttle(set_thro);
                cur_thro = set_thro;
           }
        }
         break;
       default:
         TM_INFO_LOG("what!!!\n");
         break;
       }
}

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    int fd = open(PROCFS_MTK_CL_SD_PID, O_RDWR);
#ifdef FEATURE_MD_DIAG
    int Mfd = open(PROCFS_MTK_CLMUTT_TMD_PID, O_RDWR);
#endif

    int Wfd = open(PROCFS_MTK_CL_WMT_PID, O_RDWR);
    int pid = getpid();
    int ret = 0;
    char pid_string[32] = {0};

    struct sigaction act;

    TM_INFO_LOG("START+++++++++ %d", getpid());

    /* Create signal handler */
    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_SIGINFO;
    //act.sa_handler = signal_handler;
    act.sa_sigaction = signal_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGIO, &act, NULL);

    /* Write pid to procfs */
    sprintf(pid_string, "%d", pid);

    if(fd >=0 ){
      ret = write(fd, pid_string, sizeof(char) * strlen(pid_string));
       if (ret <= 0){
            TM_INFO_LOG("Fail to write %d to %s %x\n", pid, PROCFS_MTK_CL_SD_PID, ret);
          }
          else {
                 TM_INFO_LOG("Success to write %d to %s\n", pid, PROCFS_MTK_CL_SD_PID);
          }
          close(fd);
       }
#ifdef FEATURE_MD_DIAG
    if(Mfd >=0 ){
      ret = write(Mfd, pid_string, sizeof(char) * strlen(pid_string));
        if (ret <= 0) {
          TM_INFO_LOG("Fail to write %d to %s %x\n", pid, PROCFS_MTK_CLMUTT_TMD_PID, ret);
        }
        else {
          TM_INFO_LOG("Success to write %d to %s\n", pid, PROCFS_MTK_CLMUTT_TMD_PID);
        }
        close(Mfd);
    }
#endif

    if(Wfd >=0 ){
      ret = write(Wfd, pid_string, sizeof(char) * strlen(pid_string));
        if (ret <= 0) {
          TM_INFO_LOG("Fail to write %d to %s %x\n", pid, PROCFS_MTK_CL_WMT_PID, ret);
        }
        else {
          TM_INFO_LOG("Success to write %d to %s\n", pid, PROCFS_MTK_CL_WMT_PID);
        }
        close(Wfd);
    }

    TM_INFO_LOG("Enter infinite loop");

    while(1) {
     sleep(100);
    }

    TM_INFO_LOG("END-----------");

    return 0;
}
