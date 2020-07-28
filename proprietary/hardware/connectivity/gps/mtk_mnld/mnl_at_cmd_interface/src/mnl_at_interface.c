#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h>
#include <unistd.h> //for alarm function
#include "mnl_at_interface.h"
#include "nmea_parser.h"
#include "mtk_lbs_utility.h"
#include "mtk_gps_type.h"
#include "mtk_gps.h"

#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#if 0
#define LOGD(...) tag_log(1, "[at2mnl]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[at2mnl] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[at2mnl] ERR: ", __VA_ARGS__);
#else
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#endif
/*AT command test state*/
#define GPS_TEST_PRN 1
int MNL_AT_TEST_FLAG = 0;
int MNL_AT_SIGNAL_MODE = 0;
int MNL_AT_CW_MODE = 0;
static int MNL_AT_SIGNAL_TEST_BEGIN = 0;

int MNL_AT_SET_ALARM       = 0;
int MNL_AT_CANCEL_ALARM    = 0;

enum {
    MNL_AT_TEST_UNKNOWN = -1,
    MNL_AT_TEST_START       = 0x00,
    MNL_AT_TEST_STOP        = 0x01,
    MNL_AT_TEST_INPROGRESS   = 0x02,
    MNL_AT_TEST_DONE       = 0x03,
    MNL_AT_TEST_RESULT_DONE = 0x04,
};
int MNL_AT_TEST_STATE = MNL_AT_TEST_UNKNOWN;
typedef struct {
    int test_num;
    int prn_num;
    int time_delay;
}HAL_AT_TEST_T;

static HAL_AT_TEST_T hal_test_data = {
    .test_num = 0,
    .prn_num = 0,
    .time_delay = 0,
};

int* Dev_CNr = NULL;
int prn[32] = {0};
int snr[32] = {0};
int cn = 0;
/*
* Test result array: error_code, theta(0), phi(0), Success_Num,
* Complete_Num, Avg_CNo, Dev_CNo, Avg_Speed(0)
*/
static int result[8] = {1, 0, 0, 0, 0, 0, 0, 0};
static int test_num = 0;
static int CNo, DCNo;
static int Avg_CNo = 0;
static int Dev_CNo = 0;
static int Completed_Num = 0;
static int Success_Num = 0;
static int Failure_Num = 0;
static int Wait_Num = 0;
static int sig_suc_num = 0;
static int Err_Code = 1;
#define MAX_VALID_STATUS_WAIT_COUNT 20   // 12
int test_mode_flag = 1;    // 0: USB mode, 1: SMS mode

static int fd_at_test = 0;
static struct sockaddr_un remote;
static socklen_t remotelen;
static time_t start_time;

static int CNR = -1;
static float ClkDrift = 0.0f;

#define  GPS_OP   "AT%GPS"
#define  GNSS_OP  "AT%GNSS"
#define  GPS_CW_TEST  "AT%CWGPS"
#define  GPS_AT_ACK_SIZE        60


/*for GPS AT command test*/
int gpsinf_mtk_gps_test_stop() {
   int err;
   test_mode_flag = 1;

   if ((err = at_test2mnl_gps_stop())) {
       LOGE("at_test2mnl_gps_stop err = %d", err);
       return -1;
   }

//   gps_state_test_stop(s);

   hal_test_data.test_num = 0;
   hal_test_data.prn_num = 0;
   hal_test_data.time_delay = 0;
   MNL_AT_TEST_FLAG = 0;
   MNL_AT_SIGNAL_MODE = 0;
   MNL_AT_CW_MODE = 0;
   MNL_AT_TEST_STATE = MNL_AT_TEST_UNKNOWN;

   MNL_AT_SET_ALARM       = 0;
   MNL_AT_CANCEL_ALARM    = 0;
   MNL_AT_SIGNAL_TEST_BEGIN = 0;

   // release the variable
   Success_Num = 0;
   Completed_Num = 0;
   Wait_Num = 0;
   CNo = 0;
   DCNo = 0;
   Dev_CNo = 0;
   Err_Code = 1;
   test_num = 0;

   if (NULL != Dev_CNr) {
       LOGD("Free Dev_CNr");
       free(Dev_CNr);
       Dev_CNr = NULL;
   }
   return 0;
}

int gpsinf_mtk_gps_test_start(int test_num, int prn_num, int time_delay, int test_mode) {
   int err;

   if ((MNL_AT_TEST_STATE != MNL_AT_TEST_UNKNOWN) && test_mode_flag) {
       LOGD("[SMS test mode] Timeout, test_stop() before test_start()");
       gpsinf_mtk_gps_test_stop();
   }

   hal_test_data.test_num = test_num;
   hal_test_data.prn_num = prn_num;
   hal_test_data.time_delay = time_delay;
   time(&start_time);

   //  ithis code is moved from stop function to here to keep avg value for AT%GPS(GNSS) or AT%GPS=?(GNSS=?)
   Avg_CNo = 0;

   if ((0 == hal_test_data.test_num) && (0 == test_mode)) {
       LOGE("%s: test number is 0!!", __FUNCTION__);
       return -1;
   }

   if (hal_test_data.test_num > 0) {  //Only malloc and memset Dev_CNr when test num > 0
       Dev_CNr = (int*)malloc(sizeof(int)*hal_test_data.test_num);
       memset(Dev_CNr, 0, test_num*sizeof(int));
   }

   if (1 == test_mode) {
       LOGD("Signal test mode");
       MNL_AT_SIGNAL_MODE = 1;
   } else if (2 == test_mode) {
       LOGD("CW test mode");
       MNL_AT_CW_MODE = 1;
   } else {
       LOGD("Normal test mode");
       MNL_AT_TEST_FLAG = 1;
   }

   MNL_AT_TEST_STATE = MNL_AT_TEST_INPROGRESS;

   if ((err = at_test2mnl_gps_start())) {
       LOGE("at_test2mnl_gps_start err = %d", err);
       MNL_AT_TEST_STATE = MNL_AT_TEST_UNKNOWN;
       MNL_AT_TEST_FLAG = 0;
       MNL_AT_SIGNAL_MODE = 0;
       MNL_AT_CW_MODE = 0;
       return -1;
   }

   // gps_state_test_start(s);
   return 0;
}

int gpsinf_mtk_gps_test_inprogress() {
   int ret = -1;

   if ((MNL_AT_TEST_STATE == MNL_AT_TEST_DONE) || (MNL_AT_TEST_STATE == MNL_AT_TEST_RESULT_DONE)) {
       LOGD("**AT Command test done!!");
       ret = MNL_AT_TEST_DONE;
   } else if (MNL_AT_TEST_STATE == MNL_AT_TEST_INPROGRESS) {
       LOGD("**AT Command test is in progress!!");
       ret = MNL_AT_TEST_INPROGRESS;
   } else {
       LOGD("**AT Command test status unknown!!");
       ret = MNL_AT_TEST_UNKNOWN;
   }
   return ret;
}

void at_gps_command_parser(char* cmdline) {
    char* command = cmdline;
    test_mode_flag = 0;
    Wait_Num = 0;

    if (MNL_AT_CW_MODE == 1){
        LOGD("** GPS CW test is ongoing! **");
        char buff[] = "GPS CW test is ongoing, please wait";
        at_command_send_ack(buff, sizeof(buff));
        return;
    }

    if (!memcmp(command+6, "=", 1)) {
        if ((!memcmp(command+7, "?", 1)) && (!memcmp(command+8, "\0", 1))) {
               // AT%GPS=?
            LOGD("** AT Command Parse: AT GPS=?**");
            at_command_send_cno(command);
        } else if (((!memcmp(command+7, "E", 1)) ||(!memcmp(command+7, "e", 1))) && (!memcmp(command+8, "\0", 1))) {
               // AT%GPS=E
            LOGD("Open AGPS raw data");
        } else if (((!memcmp(command+7, "D", 1)) ||(!memcmp(command+7, "d", 1))) && (!memcmp(command+8, "\0", 1))) {
               // AT%GNS=D
        LOGD("Close AGPS raw data");
        } else {
               // AT%GPS=n
            LOGD("** AT Command Parse: AT GPS=num**");
            int test_num = at_command_parse_test_num(command);
            if (test_num >= 0) {
                int ret = gpsinf_mtk_gps_test_start(test_num, GPS_TEST_PRN, 2, 0);
                if (0 == ret) {
                    LOGD("** AT Command gps test start success **");
                    char buff[] = "GPS TEST START OK";
                    at_command_send_ack(buff, sizeof(buff));
                } else {
                    LOGD("** AT Command gps test start fail **");
                    char buff[] = "GPS ERROR";
                    at_command_send_ack(buff, sizeof(buff));
                }
            } else {
                char buff[] = "Invalid Test Num =0";
                at_command_send_ack(buff, sizeof(buff));
            }
        }
    } else if (!memcmp(command+6, "?", 1) && (!memcmp(command+7, "\0", 1))) {
           // AT%GPS?
        LOGD("** AT Command Parse: AT GPS? **");
        int ret = gpsinf_mtk_gps_test_inprogress();

        if (MNL_AT_TEST_INPROGRESS == ret) {
            LOGD("** AT Command test is inprogress **");
            char buff[] = "GPS Test In Progress";
            at_command_send_ack(buff, sizeof(buff));
        } else if (MNL_AT_TEST_DONE == ret) {
            LOGD("** AT Command test done");
            char buff[GPS_AT_ACK_SIZE];
            sprintf(buff, "<%d, %d, %d, %d, %d, %d, %d, %d>",
                result[0], result[1],  result[2], result[3], result[4], result[5], result[6], result[7]);
            at_command_send_ack(buff, sizeof(buff));
        } else {
            LOGD("** AT Command test status unknown");
            char buff[] = "ERROR";
            at_command_send_ack(buff, sizeof(buff));
        }
    } else if (!memcmp(command+6, "\0", 1)) {
           // AT%GPS
        LOGD("** AT Command Parse: AT GPS **");
        at_command_send_cno(command);
    } else {
        LOGD("** AT Command Parse: illegal command **");
        char buff[] = "GPS ERROR";
        at_command_send_ack(buff, sizeof(buff));
    }
}

void at_gnss_command_parser(char* cmdline) {
    char* command = cmdline;
    test_mode_flag = 0;

    if (MNL_AT_CW_MODE == 1){
        LOGD("** GPS CW test is ongoing! **");
        char buff[] = "GPS CW test is ongoing, please wait";
        at_command_send_ack(buff, sizeof(buff));
        return;
    }

    if (!memcmp(command+7, "=", 1)) {
        if ((!memcmp(command+8, "?", 1)) && (!memcmp(command+9, "\0", 1))) {
               // AT%GNSS=?
            LOGD("** AT Command Parse: AT GNSS=?**");
               // at_command_send_cno(command);
            at_command_send_test_result();
        } else if (((!memcmp(command+8, "S", 1)) || (!memcmp(command+8, "s", 1))) && (!memcmp(command+9, "\0", 1))) {
            LOGD("AT GNSS=S is set!!");
            int ret = 0;
            if (0 == MNL_AT_TEST_FLAG) {
                LOGD("Open GPS and set signal test mode");
                ret = gpsinf_mtk_gps_test_start(0, 1, 2, 1);
            } else {
                LOGD("GPS driver is opened, change mode");
                MNL_AT_SIGNAL_MODE = 1;
                MNL_AT_TEST_FLAG = 0;
                   // Cancel alarm if needed
                if (0 == MNL_AT_CANCEL_ALARM) {
                    LOGD("Cancel alarm!!");
                    alarm(0);
                    MNL_AT_CANCEL_ALARM = 0;
                    MNL_AT_SET_ALARM = 0;
                }
            }

            if (0 == ret) {
                LOGD("** AT GNSS=S set success ! **");
                char buff[] = "GNSS START START OK";
                at_command_send_ack(buff, sizeof(buff));
            } else {
                LOGD("** AT GNSS=S set fail **");
                char buff[] = "GNSS START START FAIL";
                at_command_send_ack(buff, sizeof(buff));
            }
        } else if (((!memcmp(command+8, "E", 1)) || (!memcmp(command+8, "e", 1))) && (!memcmp(command+9, "\0", 1))) {
            LOGD("AT GNSS=E is set, stop test!!");
            if (MNL_AT_TEST_STATE == MNL_AT_TEST_UNKNOWN) {
                LOGD("** MNL_AT_TEST_UNKNOWN **");
            char buff[] = "GNSS Test is Not In Progress";
            at_command_send_ack(buff, sizeof(buff));
            } else {
                   // if (MNL_AT_TEST_STATE != MNL_AT_TEST_UNKNOWN) {   // To avoid close gps driver many times
                int ret = gpsinf_mtk_gps_test_stop();
                if (0 == ret) {
                    LOGD("** AT GNSS=E set success ! **");
                    char buff[] = "GNSS TEST END OK";
                    at_command_send_ack(buff, sizeof(buff));
                } else {
                    LOGD("** AT GNSS=E set fail **");
                    char buff[] = "GNSS TEST END FAIL";
                    at_command_send_ack(buff, sizeof(buff));
                }
                   // }
            }
        } else {
            // AT%GNSS = n
            LOGD("** AT Command Parse: AT GNSS=n**");
            int test_num = at_command_parse_test_num(command);
            // initialzation of result whenever starting test again.
            memset(result, 0, sizeof(int)*8);
            result[0] = 1;
            Avg_CNo = 0;
            Success_Num = 0;
            Completed_Num = 0;
            CNo = 0;
            DCNo = 0;
            Dev_CNo = 0;
            cn = 0;
            // initialzation of result whenever starting test again.

            if (MNL_AT_SIGNAL_MODE == 1) {   // && (test_num != 0)) {
                if (test_num <= 0) {
                    char buff[] = "Invalid Test Num = 0";
                    at_command_send_ack(buff, sizeof(buff));
                    if (MNL_AT_TEST_STATE != MNL_AT_TEST_UNKNOWN)
                        gpsinf_mtk_gps_test_stop();
                } else {
                    LOGD("MNL_AT_SIGNAL_MODE_BEGIN");
                    MNL_AT_SIGNAL_TEST_BEGIN = 1;
                    MNL_AT_TEST_STATE = MNL_AT_TEST_INPROGRESS;   //  Brian test
                    char buff[] = "GNSS TEST START OK";
                    at_command_send_ack(buff, sizeof(buff));
                    time(&start_time);
                }
            } else {
                if (test_num >= 0) {
                    int ret = gpsinf_mtk_gps_test_start(test_num, GPS_TEST_PRN, 2, 0);
                    if (0 == ret) {
                        LOGD("** AT Command gps test start success **");
                        char buff[] = "GNSS TEST START OK";
                        at_command_send_ack(buff, sizeof(buff));
                    } else {
                        LOGD("** AT Command gps test start fail **");
                        char buff[] = "GNSS ERROR";
                        at_command_send_ack(buff, sizeof(buff));
                    }
                } else {
                    char buff[] = "Invalid Test Num =0";
                    at_command_send_ack(buff, sizeof(buff));
                }
            }
        }
    } else if (!memcmp(command+7, "?", 1) && (!memcmp(command+8, "\0", 1))) {
        // AT%GNSS?
        LOGD("** AT Command Parse: AT GNSS? **");
        at_command_send_test_result();

    } else if (!memcmp(command+7, "\0", 1)) {
        // AT%GNSS
        LOGD("** AT Command Parse: AT GNSS **");
        at_command_send_cno(command);
    } else {
        LOGD("** AT Command Parse: illegal command **");
        char buff[] = "GNSS ERROR";
        at_command_send_ack(buff, sizeof(buff));
    }
}

void at_cw_test_command_parser(char* cmdline) {
    int ret = 0;

    LOGD("** AT Command: %s **\n", cmdline);
    test_mode_flag = 0;

    if (MNL_AT_TEST_FLAG == 1 || MNL_AT_SIGNAL_MODE == 1){
        LOGD("** AT GPS or AT GNSS is ongoing! **");
        char buff[] = "AT GPS or AT GNSS is ongoing, please wait";
        at_command_send_ack(buff, sizeof(buff));
        return;
    }

    if (!memcmp(cmdline + 8, "=", 1))
    {
        //AT%CWGPS=E, close GPS for error handle.
        if (((!memcmp(cmdline + 9, "E", 1)) || (!memcmp(cmdline + 9, "e", 1))) && (!memcmp(cmdline + 10, "\0", 1)))
        {
            LOGD("AT CWGPS=E is set, stop test!!");
            if (MNL_AT_TEST_STATE == MNL_AT_TEST_UNKNOWN)
            {
                LOGD("** MNL_AT_TEST_UNKNOWN **");
                char buff[] = "CWGPS Test is not in progress";
                at_command_send_ack(buff, sizeof(buff));
                return;
            }
            else
            {
                int ret = gpsinf_mtk_gps_test_stop();
                if (0 == ret) {
                    LOGD("** AT CWGPS=E set success ! **");
                    char buff[] = "CWGPS test end success";
                    at_command_send_ack(buff, sizeof(buff));
                    return;
                } else {
                    LOGD("** AT CWGPS=E set fail **");
                    char buff[] = "CWGPS test end fail";
                    at_command_send_ack(buff, sizeof(buff));
                    return;
                }
            }
        }
        //AT%CWGPS=T, start GPS CW test.
        else if ((!memcmp(cmdline + 9, "?", 1)) && (!memcmp(cmdline + 10, "\0", 1)))
        {
            //Avoid customer input more than one AT%CWGPS=T.
            if (MNL_AT_CW_MODE == 1){
                LOGD("** AT CWGPS is ongoing! **");
                char buff[] = "AT CWGPS is ongoing, please wait";
                at_command_send_ack(buff, sizeof(buff));
                return;
            }

            ret = gpsinf_mtk_gps_test_start(0, GPS_TEST_PRN, 1, 2);
            CNR = -1;
            ClkDrift = 0.0f;

            if (0 == ret) {
                LOGD("** AT CWGPS set success ! **");
                char buff[] = "GPS Open Success";
                at_command_send_ack(buff, sizeof(buff));
                return;
            } else {
                LOGD("** AT CWGPS set fail **");
                char buff[] = "GPS Open fail";
                at_command_send_ack(buff, sizeof(buff));
                return;
            }
        }
        else
        {
            LOGD("** AT Command Parse: illegal command **");
            char buff[] = "Invalid CWGPS Command";
            at_command_send_ack(buff, sizeof(buff));
        }
    }
    else if (!memcmp(cmdline + 8, "?", 1) && (!memcmp(cmdline + 9, "\0", 1)))
    {
        // AT%CWGPS?
        LOGD("** AT Command Parse: AT CWGPS? **");
        at_cw_test_command_send_test_result();
    }
    else
    {
        LOGD("** AT Command Parse: illegal command **");
        char buff[] = "Invalid CWGPS Command";
        at_command_send_ack(buff, sizeof(buff));
    }
}

void at_cw_test_command_test_result(int snr, float clk_drift) {
    int ret;

    ret = gpsinf_mtk_gps_test_inprogress();
    if (MNL_AT_TEST_UNKNOWN == ret) {
        LOGD("** AT Command test is not inprogress **");
        return;
    }

    //sprintf(buff, "SNR: %d Clock drift: %.2f", snr, clk_drift);
    //at_command_send_ack(buff, sizeof(buff));
    LOGD("** AT cw test result = %d %.2f**", snr, clk_drift);
    CNR = snr;
    ClkDrift = clk_drift;

    MNL_AT_TEST_STATE = MNL_AT_TEST_RESULT_DONE;

    if ((MNL_AT_TEST_STATE == MNL_AT_TEST_RESULT_DONE) && (1 == MNL_AT_CW_MODE)) {
        gpsinf_mtk_gps_test_stop();
    }
}

void at_cw_test_command_send_test_result() {
    char buff[64];

    //AT test is inprogress
    if (MNL_AT_TEST_INPROGRESS == MNL_AT_TEST_STATE || MNL_AT_TEST_DONE == MNL_AT_TEST_STATE) {
        LOGD("** AT Command test is inprogress **");
        char buff[] = "AT Command test In Progress";
        at_command_send_ack(buff, sizeof(buff));
        return;
    }

    //There is no valid CW test result.
    if (CNR == -1){
        LOGD("** There is no valid result **");
        char buff[] = "CW Test no valid result";
        at_command_send_ack(buff, sizeof(buff));
        return;
    }

    sprintf(buff, "SNR: %d Clock drift: %.2f", CNR, ClkDrift);
    at_command_send_ack(buff, sizeof(buff));
}

static void at_command_parser(char* cmdline) {
    char* command = cmdline;
    LOGD("** AT Command, receive command %s**", command);
    /* begin to parse the command */
    if (!memcmp(command, GPS_OP, 6)) {
        at_gps_command_parser(command);
    } else if (!memcmp(command, GNSS_OP, 7)) {
        at_gnss_command_parser(command);
    } else if (!memcmp(command, GPS_CW_TEST, 8)) {
        at_cw_test_command_parser(command);
    }else {
        LOGD("** AT Command Parse: Not GPS/GNSS AT Command **");
        char buff[] = "GPS ERROR";
        at_command_send_ack(buff, sizeof(buff));
    }
}

void at_command_send_ack(char* ack, int len) {
    remotelen = sizeof(remote);
    if (fd_at_test != 0) {
        if (sendto(fd_at_test, ack, len, 0, (struct sockaddr*)&remote, remotelen) < 0) {
            LOGD("** AT Command send ack to USB failed: %s**", strerror(errno));
        } else {
            LOGD("** AT Command send ack to USB sucess **");
        }
    }
}

void at_command_send_cno(char* cmdline) {
    char* command = cmdline;

    int ret = gpsinf_mtk_gps_test_inprogress();
    if (MNL_AT_TEST_INPROGRESS == ret) {
        LOGD("** AT Command test is inprogress **");
        char buff[] = "GNSS Test In Progress";
        at_command_send_ack(buff, sizeof(buff));
        return;
    }


    if (MNL_AT_SIGNAL_MODE == 1) {
        if (0 == cn) {
        LOGD("** AT Command, cn is invalid **");
        char buff[] = "0, NA";
        at_command_send_ack(buff, sizeof(buff));
        } else {
        LOGD("** AT Command, CN is valid **");
        char buff[32];
           // sprintf(buff, "%d", Avg_CNo/10);    //  unit of AT%GNSS, AT%GNSS=? is 1dB
        sprintf(buff, "%d", cn/10);
        if (!memcmp(command, GNSS_OP, 7)) {
            LOGD("** GNSS test, report CN and NA**");
            int size = strlen(buff);
            MNLD_STRNCPY(buff+size, ", NA",sizeof(buff)-size);
            LOGD("** result = %s**", buff);
        }
        at_command_send_ack(buff, sizeof(buff));
        }
    } else {
        if (0 == Avg_CNo) {
        LOGD("** AT Command, cn is invalid **");
        char buff[] = "0, NA";
        at_command_send_ack(buff, sizeof(buff));
        } else {
        LOGD("** AT Command, CN is valid **");
        char buff[32];
        sprintf(buff, "%d", Avg_CNo/10);    //  unit of AT%GNSS, AT%GNSS=? is 1dB
           // sprintf(buff, "%d", cn/10);
                                if (!memcmp(command, GNSS_OP, 7)) {
            LOGD("** GNSS test, report CN and NA**");
            int size = strlen(buff);
            MNLD_STRNCPY(buff+size, ", NA",sizeof(buff)-size);
            LOGD("** result = %s**", buff);
        }
        at_command_send_ack(buff, sizeof(buff));
        }
    }
}

void at_command_send_test_result() {
    int ret = gpsinf_mtk_gps_test_inprogress();
    if (MNL_AT_TEST_INPROGRESS == ret) {
        LOGD("** AT Command test is inprogress **");
        char buff[] = "GNSS Test In Progress";
        at_command_send_ack(buff, sizeof(buff));
    } else if (MNL_AT_TEST_DONE == ret || MNL_AT_TEST_RESULT_DONE == ret) {
        LOGD("** AT Command test done");
        char buff[GPS_AT_ACK_SIZE];
        sprintf(buff, "[%d, %d, %d, %d, %d, %d, %d, %d][0, 0, 0, 0, 0, 0, 0, 0]",
        result[0], result[1],  result[2], result[3], result[4], result[5], result[6], result[7]);
        at_command_send_ack(buff, sizeof(buff));
    } else {
        LOGD("** AT Command test status unknown");
        if (result[0] !=0) {   // Brian test
            char buff[GPS_AT_ACK_SIZE]={0};
            LOGD("Normal: Return the result");
            sprintf(buff, "[%d, %d, %d, %d, %d, %d, %d, %d][0, 0, 0, 0, 0, 0, 0, 0]",
            result[0], result[1],  result[2], result[3], result[4], result[5], result[6], result[7]);
            LOGD("** result =[%s] **\n", buff);
            at_command_send_ack(buff, sizeof(buff));
        } else if ((result[5] != 0) && (!(MNL_AT_SIGNAL_MODE || MNL_AT_SIGNAL_TEST_BEGIN))) {
            LOGD("Normal: Return the result");
            char buff[GPS_AT_ACK_SIZE]={0};
            sprintf(buff, "[%d, %d, %d, %d, %d, %d, %d, %d][0, 0, 0, 0, 0, 0, 0, 0]",
            result[0], result[1],  result[2], result[3], result[4], result[5], result[6], result[7]);
            LOGD("** result =[%s] **\n", buff);
            at_command_send_ack(buff, sizeof(buff));
        } else {
            char buff[] = "[0, 0, 0, 0, 0, 0, 0, 0][0, 0, 0, 0, 0, 0, 0, 0]";
            at_command_send_ack(buff, sizeof(buff));
        }
    }
}

int at_command_parse_test_num(char* cmdline) {
    int res;
    char* command = cmdline;
    char* pos;

    if (!memcmp(command, GNSS_OP, 7)) {
           // AT%GNSS=n
        res = (int)strtol(command+8, &pos, 10);
    } else {
           // AT%GPS=n
        res = (int)strtol(command+7, &pos, 10);
    }

    if ((res != 0) && ((*pos) == '\0')) {
        LOGD("** AT Command Parse: get test_num = %ld success!**", res);
    } else {
        LOGD("** AT Command Parse: the test num may incorrect**");
        res = -1;
    }

    return res;
}

void nmea_parser_at_cmd_pre(void) {
    int time_diff;
    time_t current_time;
    static int prev_success_num = 0;

    if ((1 == MNL_AT_TEST_FLAG) ||(1 == MNL_AT_SIGNAL_TEST_BEGIN)) {
        // (1 == MNL_AT_SIGNAL_MODE)) {
        LOGD("MNL_AT_TEST_FLAG = %d, MNL_AT_SIGNAL_TEST_BEGIN = %d", MNL_AT_TEST_FLAG, MNL_AT_SIGNAL_TEST_BEGIN);
        time(&current_time);
        time_diff = current_time - start_time;
        if (time_diff >= hal_test_data.time_delay) {
                    LOGD("MNL_AT_SET_ALARM == 1, gps_nmea_end_tag(%d)", get_gps_nmea_parser_end_status());
            if (get_gps_nmea_parser_end_status()) {
                int test_stopped = 0;
                LOGD("Success_Num = %d, Prev_Success_Num = %d, Wait_Num = %d, \
                     Failure_Num = %d, Completed_Num = %d, Avg_CNo = %d, \
                     Dev_CNo = %d, Err_Code = %d, MNL_AT_TEST_STATE = %d",
                     Success_Num, prev_success_num, Wait_Num, Failure_Num, Completed_Num,
                     Avg_CNo, Dev_CNo, Err_Code, MNL_AT_TEST_STATE);
                if (Success_Num > 0) {
                    if (prev_success_num == Success_Num) {
                        Failure_Num++;
                }
                Completed_Num = Success_Num + Failure_Num;
                if (Completed_Num == hal_test_data.test_num) {
                    // 1. Call reportTestResult callback with detected SNR info
                   // sms_airtest_no_signal_report(Err_Code, Success_Num, Completed_Num, Avg_CNo, Dev_CNo);
                    // 2. Test Stop
                    test_stopped = 1;
                }
             }
                else {
                    Wait_Num++;
                    if (Wait_Num == MAX_VALID_STATUS_WAIT_COUNT) {
                           // 1. Call reportTestResult callback with <32, 0, 0, 0, 0, 0>
                        LOGD("TimeOut!! Wait_Num = %d", Wait_Num);
                        Completed_Num = hal_test_data.test_num;
                        Err_Code = (1 << 5);
                        Completed_Num = 1;
                       // sms_airtest_no_signal_report(Err_Code, Success_Num, Completed_Num, 0, 0);
                           // 2. Test Stop
                        test_stopped = 1;
                    }
                }
                prev_success_num = Success_Num;

                if (test_stopped == 1) {
                    result[0] = Err_Code;
                    result[3] = Success_Num;
                    result[4] = Completed_Num;
                    result[5] = Avg_CNo;
                    result[6] = Dev_CNo;
                    Wait_Num = 0;
                    MNL_AT_TEST_STATE = MNL_AT_TEST_DONE;

                    LOGD("**AT Command test_start done, Success_Num = %d, Completed_Num = %d, \
                       Avg_CNo = %d, Dev_CNo = %d, Err_Code = %d, test_num = %d, MNL_AT_TEST_STATE = %d",
                       Success_Num, Completed_Num, Avg_CNo, Dev_CNo, Err_Code, test_num, MNL_AT_TEST_STATE);
                    Err_Code = 1;
                    if ((MNL_AT_TEST_STATE == MNL_AT_TEST_DONE) && (1 == MNL_AT_TEST_FLAG)) {
                        LOGD("** AT Command test done, stop GPS driver **");
                        gpsinf_mtk_gps_test_stop();
                        Failure_Num = 0;
                        prev_success_num = 0;
                    }
                }
            }
        } else {    //  2sec waiting
            LOGD("static time, return...");
            return;
        }
    } else if (MNL_AT_CW_MODE == 1) {
        if (MNL_AT_TEST_STATE != MNL_AT_TEST_INPROGRESS) {
            return;
        }

        time(&current_time);
        time_diff = current_time - start_time;
        if (time_diff >= hal_test_data.time_delay) {
            int ret = 0;
            MTK_GPS_PARAM_TESTMODE param_test_config;
            memset(&param_test_config, 0x00, sizeof(param_test_config));
            param_test_config.svid = 1;
            param_test_config.test_mode = MTK_TESTMODE_CW_MODE;
            param_test_config.threshold = 0;  // 0:GPS Band(1575.42MHz)

            if ((ret = mtk_gps_set_param(MTK_PARAM_CMD_TESTMODE, &param_test_config))) {
                LOGE("set MTK_PARAM_CMD_TESTMODE to mnl fail, %d", ret);
            } else {
                LOGD("set MTK_PARAM_CMD_TESTMODE to mnl success, %d", ret);
            }
            MNL_AT_TEST_STATE = MNL_AT_TEST_DONE;
        } else {
            LOGD("static time, return...");
        }
    }
}
/*****************************************
AT test -> MNL Testing
*****************************************/
// No use now
static void
gps_at_command_search_satellite(NmeaReader*  r) {
    int i = 0, j = 0;
    for (i = 0; i < r->sv_count; i++) {
        if (prn[i] == hal_test_data.prn_num) {
            LOGD("**AT Command test SvID: %d", prn[i]);
            if (snr[i] != 0) {
                if (MNL_AT_SIGNAL_MODE && (!MNL_AT_SIGNAL_TEST_BEGIN)) {
                    LOGD("Set state to MNL_AT_TEST_INPROGRESS for read result");
                    MNL_AT_TEST_STATE = MNL_AT_TEST_INPROGRESS;
                }

                if (MNL_AT_SIGNAL_MODE && MNL_AT_SIGNAL_TEST_BEGIN) {
                    LOGD("Set state to MNL_AT_TEST_RESULT_DONE for read result");
                    MNL_AT_TEST_STATE = MNL_AT_TEST_RESULT_DONE;
                    sig_suc_num = 1;
                       // sig_com_num = 1;
                }

                if (MNL_AT_TEST_FLAG) {
                    Err_Code = 0;
                    LOGD("cn = %d", cn);
                    CNo += snr[i]*10;
                    Dev_CNr[Success_Num] = snr[i]*10;
                    Success_Num++;
                    Avg_CNo = CNo / Success_Num;
                       // test_num++;
                    LOGD("CNo = %d, Avg_CNo /= %d, Success_Num = %d", CNo, Avg_CNo, Success_Num);
                }
                cn = snr[i]*10;
                LOGD("cn = %d", cn);
            } else {
                LOGD("**SNR is 0, ignore!!!");
                if (MNL_AT_CANCEL_ALARM == 1) {    // It's not timeout, just no signal after 12s
                    if (MNL_AT_TEST_FLAG == 1)
                        test_num++;
                }
                sig_suc_num = 0;
            }

            if (Success_Num != 0) {
                for (j = 0; j < Success_Num; j++) {
                    DCNo += (Dev_CNr[j]-Avg_CNo) * (Dev_CNr[j]-Avg_CNo);
                    LOGD("Dev_CNr[%d] = %d, Dev_CNo2 += %d", j, Dev_CNr[j], DCNo);
                }
                Dev_CNo = DCNo / Success_Num;
                DCNo = 0;
                Dev_CNo = sqrt(Dev_CNo);
            }
            LOGD("**AT Command find SvID: %d, exit", prn[i]);
            break;
        } else {
            LOGD("**AT Command ignore SvID: %d", prn[i]);
        }
    }
}

void
gps_at_command_test_proc(NmeaReader* const r) {
       // For AT command test

    if (MNL_AT_TEST_STATE != MNL_AT_TEST_DONE) {
        LOGD("**AT Command test mode!!");

        if (MNL_AT_SIGNAL_MODE == 1) {
            if (MNL_AT_SIGNAL_TEST_BEGIN) {
                gps_at_command_search_satellite(r);
                LOGD("Update test result per second");
                result[0] = 0;
                result[3] = sig_suc_num;
                result[4] = 1;
                result[5]= cn;
                LOGD("result[5] = %d", result[5]);
                result[6] = 0;
            } else {
                LOGD("Wait AT GNSS=1...");
                return;
            }
        } else {
            LOGD("Not in MNL_AT_SIGNAL_MODE");
                LOGD("**AT Command Continue, search satellites...");
                   // Search satellites
                gps_at_command_search_satellite(r);
        }

    } else {
        LOGD("**AT Command test, test mode is MNL_AT_TEST_DONE");
    }
}

void at_cmd2mnl_hdlr(int fd) {
    char cmd[20];

    fd_at_test = fd;
    LOGD("** AT Command received **");
    /* receive and parse ATCM here */
    for (;;) {
        int ret;

        remotelen = sizeof(remote);
        ret = recvfrom(fd, cmd, sizeof(cmd) - 1, 0, (struct sockaddr *)&remote, &remotelen);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            if (errno != EWOULDBLOCK)
                LOGE("error while reading AT Command socket: %s: %p", strerror(errno), cmd);
            break;
        }
        LOGD("received %d bytes: %d.%s", ret, ret, cmd);
        cmd[ret] = 0x00;
        at_command_parser(cmd);                // need redefine
    }
    LOGD("** AT Command event done **");
}

int create_at2mnl_fd() {
    int fd = socket_bind_udp(GPS_AT_COMMAND_SOCK);
    socket_set_blocking(fd, 0);
    return fd;
}
