#pragma once

#include <deque>
#include <vector>
#include "camera_custom_nvram.h"

/***********************************************************
 * Define macros
 **********************************************************/
/* CCT file path */
#define CCT_FLASH_PATH_ENG               "/data/vendor/flash/eng_code.txt"
#define CCT_FLASH_PATH_FWB               "/data/vendor/flash/fwb_code.txt"
#define CCT_FLASH_PATH_CALIBRAION_AE     "/data/vendor/flash/cali_ae.txt"
#define CCT_FLASH_PATH_ENG_ALL           "/data/vendor/flash/eng_all.txt"
#define CCT_FLASH_PATH_CALIBRAION_RESULT "/data/vendor/flash/flash_cali_result"
#define CCT_FLASH_PATH_ENG_XML           "/data/vendor/flash/engTab.xml"
#define CCT_FLASH_PATH_FWB_XML           "/data/vendor/flash/fwbGain.xml"
#define CCT_FLASH_PATH_CALIBRAION_AE_XML "/data/vendor/flash/caliAe.xml"
#define CCT_FLASH_PATH_ENG_ALL_XML       "/data/vendor/flash/engAll.xml"

#define CCT_FLASH_PATH_CALI        "/data/vendor/flash/flash_cali.bin"
#define CCT_FLASH_PATH_CALI_QUICK2 "/data/vendor/flash/flash_cali_quick2.bin"

/* CCT property name */
#define CCT_FLASH_AE_EXP "vendor.flash_cal_exp"
#define CCT_FLASH_AE_ISP "vendor.flash_cal_isp"
#define CCT_FLASH_AE_AFE "vendor.flash_cal_afe"

#define CCT_FLASH_CALIBRATION_NUM    "vendor.flash_calibration_num"
#define CCT_FLASH_CALIBRATION_DUTY   "vendor.flash_calibration_duty"
#define CCT_FLASH_CALIBRATION_DUTYLT "vendor.flash_calibration_dutylt"
#define CCT_FLASH_CALIBRATION_EXP    "vendor.flash_calibration_exp"
#define CCT_FLASH_CALIBRATION_AFE    "vendor.flash_calibration_afe"
#define CCT_FLASH_CALIBRATION_ISP    "vendor.flash_calibration_isp"

/* CCT AE cycle */
#define CCT_FLASH_DO_AE_MAX_CYCLE 20
#define CCT_FLASH_AE_CYCLE_FRAME_COUNT 40
#define CCT_FLASH_AE_POST_WAIT_FRAME_COUNT 35
#define CCT_FLASH_RECONVERGE_AE_MAX_CYCLE 5

/* CCT calculate frame */
#define CCT_FLASH_CALCULATE_FRAME 2

/* CCT state */
#define CCT_FLASH_STATE_INIT 0
#define CCT_FLASH_STATE_AE 1
#define CCT_FLASH_STATE_AE_POST 2
#define CCT_FLASH_STATE_RATIO 3
#define CCT_FLASH_STATE_RATIO2 4
#define CCT_FLASH_STATE_END 5
#define CCT_FLASH_STATE_END2 6

#define AFE_GAIN_MAX_RATIO 8
#define ISP_GAIN_MAX_RATIO 8

enum {
    e_AeTooDark = -100,
    e_SensorSetFail = -99,
    e_AeIsTooLong = -98,
    e_ObjIsFar = -97,
    e_LightSourceNotConstant = -96,
    e_NvramError = -95,
};

class CaliData
{
    public:
        int duty;
        int dutyLT;
        float ref;
        int exp;
        int afe;
        int isp;
        float y[CCT_FLASH_CALCULATE_FRAME];
        float r[CCT_FLASH_CALCULATE_FRAME];
        float g[CCT_FLASH_CALCULATE_FRAME];
        float b[CCT_FLASH_CALCULATE_FRAME];
        int t[CCT_FLASH_CALCULATE_FRAME];
};

typedef std::deque<CaliData> dqCaliData;
typedef std::vector<int> vectorInt;

/***********************************************************
 * AE
 **********************************************************/
struct AeOut {
    int exp;
    int afe;
    int isp;
    int isEnd;
};

struct AeIn {
    int sensorDev;
    double y;
    double tar;
    double tarMax;
    double tarMin;
};

int adjExp(int &exp, int &afe, int &isp, double m = 1);
int setExp(int sensorDev, int exp, int afe, int isp);
void doAeInit();
int doAe(AeIn *in, AeOut *out);

int dumpExp(const char *fname, int exp, int afe, int isp);
int dumpFlashAe(const char *fname, short *y, int len, int div = FLASH_CUSTOM_MAX_DUTY_NUM_HT);
int dumpFlashAwb(const char *fname, AWB_GAIN_T *gain, int len, int isDual, int dutyNum, int dutyNumLt);
int dumpDequeCaliData(const char *fname,
        dqCaliData &caliData1, dqCaliData &caliData2, dqCaliData &caliData3, int driverFault);

int cmpCaliData(const void *a, const void *b);

