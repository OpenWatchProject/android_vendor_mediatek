
#ifndef __PERFSERV_H__
#define __PERFSERV_H__

#ifdef __cplusplus
extern "C" {
#endif


/*** STANDARD INCLUDES *******************************************************/


/*** PROJECT INCLUDES ********************************************************/


/*** MACROS ******************************************************************/
#define PS_CLUSTER_MAX  8
#define MAX_NAME_LEN    256
#define MAX_SYSINFO_LEN  521
#define MAX_ARGS_PER_REQUEST 40 // 20 * 2

/*** GLOBAL TYPES DEFINITIONS ************************************************/
enum tPowerMsg {
    POWER_MSG_AOSP_HINT,
    POWER_MSG_MTK_HINT,
    POWER_MSG_MTK_CUS_HINT,
    POWER_MSG_NOTIFY_STATE,
    POWER_MSG_QUERY_INFO,
    POWER_MSG_SCN_REG,
    POWER_MSG_SCN_CONFIG,
    POWER_MSG_SCN_UNREG,
    POWER_MSG_SCN_ENABLE,
    POWER_MSG_SCN_DISABLE,
    POWER_MSG_SCN_DISABLE_ALL,
    POWER_MSG_SCN_RESTORE_ALL,
    POWER_MSG_SCN_ULTRA_CFG,
    POWER_MSG_SET_SYSINFO,
    POWER_MSG_PERF_LOCK_ACQ,
    POWER_MSG_PERF_LOCK_REL,
};

struct tPowerData {
    enum tPowerMsg msg;
    void          *pBuf;
};

struct tHintData {
    int hint;
    int data;
};

struct tAppStateData {
    char pack[MAX_NAME_LEN];
    char activity[MAX_NAME_LEN];
    int  pid;
    int  state;
    int  uid;
};

struct tQueryInfoData {
    int cmd;
    int param;
    int value;
};

struct tScnData {
    int handle;
    int command;
    int timeout;
    int param1;
    int param2;
    int param3;
    int param4;
};

struct tSysInfoData {
    int type;
    int ret;
    char data[MAX_SYSINFO_LEN];
};

struct tCusConfig {
    int hint;
    int size;
    int rscList[MAX_ARGS_PER_REQUEST];
};

struct tPerfLockData {
    int hdl;
	int pid;
	int uid;
    int duration;
    int *rscList;
    int size;
    int reserved;
};

/*** PRIVATE TYPES DEFINITIONS ***********************************************/


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PUBLIC FUNCTION PROTOTYPES **********************************************/
long power_msg(void * pMsg, void **ppRspMsg);
int powerd_cus_init(int *pCusHintTbl, void *fnptr);

#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __PERFSERV_H__ */

