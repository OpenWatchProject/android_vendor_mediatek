#define LOG_TAG "FlashCct"

/***********************************************************
 * Headers
 **********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cutils/properties.h>

/* aaa common headers */
#include "property_utils.h"
#include "time_utils.h"
#include "log_utils.h"

/* aaa headers */
#include "nvbuf_util.h"
#include "ae_mgr.h"
#include "awb_mgr_if.h"
#include "aaa_sensor_mgr.h"

/* custom headers */
#include "cct_feature.h"
#include "camera_custom_nvram.h"
#include "aaa_error_code.h"
#include "ae_param.h"
#include "flash_awb_param.h"
#include "flash_tuning_custom.h"
#include "flash_feature.h"

/* flash headers */
#include "flash_mgr_m.h"
#include "flash_pline.h"
#include "flash_utils.h"
#include "flash_duty.h"
#include "flash_nvram.h"
#include "flash_cct.h"
#include "flash_hal.h"
#include "tools/flash_cali_utils.h"
#include "tools/flash_cali_xml.h"
#include "flash_custom_adapter.h"
#include "flash_platform_adapter.h"

using namespace NS3Av3;

/***********************************************************
 * Global variables
 **********************************************************/
int FlashMgrM::mIsManualFlashEnCct = 0;
int FlashMgrM::mManualDutyCct = 0;
int FlashMgrM::mManualDutyLtCct = 0;
int FlashMgrM::mSpModeLockCct = 0;
int FlashMgrM::mSpModeCct = e_SpModeNormal;

static CaliData g_CaliDataDark[2];
static dqCaliData g_CaliData;
static dqCaliData g_CaliData1;
static dqCaliData g_CaliData2;
static dqCaliData g_CaliData3;
static vectorInt g_dutyArr;
static vectorInt g_dutyArrLt;


/***********************************************************
 * AE
 **********************************************************/
static int aeCycleCount = 0;

int adjExp(int &exp, int &afe, int &isp, double m)
{
    int err = 0;
    int ispGainBase = FlashPlatformAdapter::getIspGainBase();
    double expLev = (double)exp * afe * isp / 1000 / 1024 / ispGainBase * m;

    if (expLev >= 30 * AFE_GAIN_MAX_RATIO * ISP_GAIN_MAX_RATIO) {
        exp = 30000;
        afe = 1024 * AFE_GAIN_MAX_RATIO;
        isp = ispGainBase * ISP_GAIN_MAX_RATIO;
        err = e_AeTooDark;
    } else if (expLev > 30 * AFE_GAIN_MAX_RATIO) {
        exp = 30000;
        afe = 1024 * AFE_GAIN_MAX_RATIO;
        isp = (expLev / 30 / AFE_GAIN_MAX_RATIO) * ispGainBase;
    } else if (expLev > 30) {
        exp = 30000;
        afe = (expLev / 30) * 1024;
        isp = ispGainBase;
    } else {
        exp = expLev * 1000;
        afe = 1024;
        isp = ispGainBase;
    }

    return err;
}

int setExp(int sensorDev, int exp, int afe, int isp)
{
    logI("setExp(): sensorDev(%d), exp(%d), afe(%d), isp(%d).",
            sensorDev, exp, afe, isp);

    /* set to sensor */
    int err;
    err = AAASensorMgr::getInstance().setSensorExpTime(sensorDev, exp);
    if (FAILED(err))
        return e_SensorSetFail;
    err = AAASensorMgr::getInstance().setSensorGain(sensorDev, afe);
    if (FAILED(err))
        return e_SensorSetFail;

    FlashPlatformAdapter::getInstance(sensorDev)->setAeExpSetting(exp, afe, isp);
    return 0;
}

void doAeInit()
{
    aeCycleCount = 0;
}

int doAe(AeIn *in, AeOut *out)
{
    static int exp;
    static int afe;
    static int isp;

    out->isEnd = 0;

    if (!aeCycleCount) {
        /* the first AE cycle */
        logI("doAe(): init exposure.");
        exp = 30000;
        afe = 1024;
        isp = FlashPlatformAdapter::getIspGainBase();
    } else {
        /* each AE cycle */
        if (in->y > in->tarMin && in->y < in->tarMax) {
            logI("doAe(): target exposure.");
            out->isEnd = 1;
        } else if (in->y > 250) {
            logI("doAe(): over exposure.");
            adjExp(exp, afe, isp, 1 / 3.0);
        } else if (in->y < 8) {
            logI("doAe(): under exposure.");
            adjExp(exp, afe, isp, 20.0);
        } else {
            logI("doAe(): normal exposure.");
            adjExp(exp, afe, isp, in->tar / in->y);
        }
    }
    logI("doAe(): exp(%d), afe(%d), isp(%d), y(%5.3lf).", exp, afe, isp, in->y);

    if (!out->isEnd)
        /* set exposure parameters to image sensor and ISP */
        setExp(in->sensorDev, exp, afe, isp);
    else
        logI("doAe(): done at cycle(%d).", aeCycleCount);

    /* update exposure parameters */
    out->exp = exp;
    out->afe = afe;
    out->isp = isp;

    /* increase cycle count */
    aeCycleCount++;
    if (aeCycleCount > CCT_FLASH_DO_AE_MAX_CYCLE)
        return e_AeIsTooLong;

    return 0;
}


/***********************************************************
 * Misc
 **********************************************************/
int dumpExp(const char *fname, int exp, int afe, int isp)
{
    /* verify arguments */
    if (!fname)
        return -1;

    FILE *fp;
    fp = fopen(fname, "wt");
    if (!fp)
        return -1;

    fprintf(fp, "exp=%d\n",exp);
    fprintf(fp, "afe=%d\n",afe);
    fprintf(fp, "isp=%d\n",isp);
    fclose(fp);

    return 0;
}
int dumpFlashAe(const char *fname, short *y, int len, int div)
{
    /* verify arguments */
    if (!fname || !y)
        return -1;

    FILE *fp = fopen(fname, "wt");
    if (!fp)
        return -1;

    int i;
    fprintf(fp,
            "    static short engTab[] =\n"
            "    {\n");
    for (i = 0; i < len; i++) {
        if (i % div == 0)
            fprintf(fp, "        ");
        fprintf(fp, "%4d,", y[i]);
        if (i % div == (div - 1))
            fprintf(fp, "\n");
    }
    fprintf(fp, "    };\n");
    fclose(fp);

    return 0;
}

int dumpFlashAwb(const char *fname, AWB_GAIN_T *gain, int len, int isDual, int dutyNum, int dutyNumLt)
{
    /* verify arguments */
    if (!fname || !gain)
        return -1;

    FILE *fp = fopen(fname, "wt");
    if (!fp)
        return -1;

    fprintf(fp, "{{\n");

    int i;
    for (i = 0; i < len; i++) {
        if (isDual) {
            int duty = 0;
            int dutyLt = 0;
            FlashDuty::ind2duty(i, duty, dutyLt, dutyNum, dutyNumLt, isDual);
            fprintf(fp, "    {%4d,%4d,%4d},  //duty=%d, dutyLt=%d\n", gain[i].i4R, gain[i].i4G, gain[i].i4B, duty, dutyLt);
        } else
            fprintf(fp, "    {%4d,%4d,%4d},\n", gain[i].i4R, gain[i].i4G, gain[i].i4B);
    }
    fprintf(fp, "}}\n");
    fclose(fp);

    return 0;
}

int dumpDequeCaliData(const char *fname,
        dqCaliData &caliData1, dqCaliData &caliData2, dqCaliData &caliData3, int driverFault)
{
    /* verify arguments */
    if (!fname)
        return -1;

    FILE *fp = fopen(fname, "wt");
    if (!fp)
        return -1;

    int i, j;
    fprintf(fp,"t\texp\tafe\tisp\tduty\tdutyLt\ty\tr\tg\tb\n");
    for (j = 0; j < (int)caliData1.size(); j++)
        for (i = 0; i < CCT_FLASH_CALCULATE_FRAME; i++)
            fprintf(fp, "%d\t%d\t%d\t%d\t%d\t%d\t%5.3f\t%5.3f\t%5.3f\t%5.3f\n",
                    caliData1[j].t[i], caliData1[j].exp,
                    caliData1[j].afe, caliData1[j].isp,
                    caliData1[j].duty, caliData1[j].dutyLT,
                    caliData1[j].y[i], caliData1[j].r[i],
                    caliData1[j].g[i], caliData1[j].b[i]);
    fprintf(fp,"============\n");
    for (j = 0; j < (int)caliData2.size(); j++)
        for (i = 0; i < CCT_FLASH_CALCULATE_FRAME; i++)
            fprintf(fp, "%d\t%d\t%d\t%d\t%d\t%d\t%5.3f\t%5.3f\t%5.3f\t%5.3f\n",
                    caliData2[j].t[i], caliData2[j].exp,
                    caliData2[j].afe, caliData2[j].isp,
                    caliData2[j].duty, caliData2[j].dutyLT,
                    caliData2[j].y[i], caliData2[j].r[i],
                    caliData2[j].g[i], caliData2[j].b[i]);
    fprintf(fp,"============\n");
    for (j = 0; j < (int)caliData3.size(); j++)
        for (i = 0; i < CCT_FLASH_CALCULATE_FRAME; i++)
            fprintf(fp, "%d\t%d\t%d\t%d\t%d\t%d\t%5.3f\t%5.3f\t%5.3f\t%5.3f\n",
                    caliData3[j].t[i], caliData3[j].exp,
                    caliData3[j].afe, caliData3[j].isp,
                    caliData3[j].duty, caliData3[j].dutyLT,
                    caliData3[j].y[i], caliData3[j].r[i],
                    caliData3[j].g[i], caliData3[j].b[i]);
    fprintf(fp,"============\n");
    fprintf(fp, "Flash Driver Fault:%d\n", driverFault);
    fprintf(fp,"none zero is error, need to check driver data sheet.\n");
    fclose(fp);

    return 0;
}

static int verifyDuty(int &duty, const int dutyNum)
{
    if (duty < -1) {
        duty = -1;
        return -1;
    }

    if (duty >= dutyNum) {
        duty = dutyNum - 1;
        return -1;
    }

    return 0;
}

/***********************************************************
 * Calibration
 **********************************************************/
int cmpCaliData(const void *a, const void *b)
{
    const CaliData *pa = (CaliData *)a;
    const CaliData *pb = (CaliData *)b;
    if (pa->ref > pb->ref)
        return -1;
    else if (pa->ref < pb->ref)
        return 1;
    else
        return 0;
}

int FlashMgrM::cctCalibration(FlashExePara *para, FlashExeRep *rep)
{
    static int state = CCT_FLASH_STATE_INIT;
    static int dutyNum;
    static int dutyNumLt;
    static int preStateEndCnt = -1;
    static int exp;
    static int afe;
    static int isp;
    static int expS1;
    static int afeS1;
    static int ispS1;
    static int dutyIndex = 0;
    static int frameTar = 0;
    static int frameBase = 0;
    static int ratioRound = 0;

    static AWB_GAIN_T awbPreGain1Prev;
    static AWB_GAIN_T awbPreGain1Curr;
    static int inCharge = FlashHal::getInstance(mSensorDev)->getInCharge();
    static short ITab1[FLASH_CUSTOM_MAX_DUTY_NUM];
    static short ITab2[FLASH_CUSTOM_MAX_DUTY_NUM];

    int i, j;
    int err = 0;
    rep->isEnd = 0;

    if (!mPfFrameCount) {
        /* backup AWB pregain1 */
        awbPreGain1Prev.i4R = 512;
        awbPreGain1Prev.i4G = 512;
        awbPreGain1Prev.i4B = 512;

        /* set AWB pregain1 to default */
        awbPreGain1Curr.i4R = 512;
        awbPreGain1Curr.i4G = 512;
        awbPreGain1Curr.i4B = 512;

        FlashPlatformAdapter::getInstance(mSensorDev)->getAwbModuleVariationGain(&awbPreGain1Prev, &awbPreGain1Curr);
        mLinearOutputEn = FlashPlatformAdapter::getInstance(mSensorDev)->getAwbLinearOutputEn();
        logI("cctCalibration(): backup AWB pregain1 (%d,%d,%d).",
                awbPreGain1Prev.i4R, awbPreGain1Prev.i4G, awbPreGain1Prev.i4B);

        /* clear deque */
        g_CaliData.clear();
        g_CaliData1.clear();
        g_CaliData2.clear();
        g_CaliData3.clear();

        /* clear vector */
        g_dutyArr.clear();
        g_dutyArrLt.clear();

        dutyNum = 0;
        dutyNumLt = 0;

        preStateEndCnt = 0;

        exp = 0;
        afe = 0;
        isp = 0;
        expS1 = 0;
        afeS1 = 0;
        ispS1 = 0;

        dutyIndex = 0;
        frameTar = 0;
        frameBase = 0;
        ratioRound = 1;
        state = CCT_FLASH_STATE_INIT;
    }


    FlashPlatformAdapter::getInstance(mSensorDev)->clearAwbModuleVariationGain(&awbPreGain1Curr);
    if (state == CCT_FLASH_STATE_INIT) {
        logI("cctCalibration(): state(INIT).");

        FlashPlatformAdapter::getInstance(mSensorDev)->setAeCCUOnOff(MFALSE);

        dutyNum = mDutyNum;
        dutyNumLt = 1;
        FlashHal::getInstance(mSensorDev)->getCurrentTab(ITab1, ITab2);

        if (!cust_isDualFlashSupport(mFacingSensor)) {
            /* create duty array */
            g_dutyArr.push_back(-1);
            g_dutyArrLt.push_back(-1);
            for (i = 0; i < mDutyNum; i++) {
                g_dutyArr.push_back(i);
                g_dutyArrLt.push_back(-1);
            }
            g_dutyArr.push_back(-1);
            g_dutyArrLt.push_back(-1);
        } else {
            /* create duty array */
            int calibrationNum = 0;
            int calibrationDuty = 0;
            int calibrationDutyLt = 0;
            getPropInt(CCT_FLASH_CALIBRATION_NUM, &calibrationNum, -1);
            getPropInt(CCT_FLASH_CALIBRATION_DUTY, &calibrationDuty, -1);
            getPropInt(CCT_FLASH_CALIBRATION_DUTYLT, &calibrationDutyLt, -1);
            if(calibrationNum != -1)
            {
                g_dutyArr.push_back(-1);
                g_dutyArrLt.push_back(-1);
                for(i = 0; i < calibrationNum; i++)
                {
                    g_dutyArr.push_back(calibrationDuty);
                    g_dutyArrLt.push_back(calibrationDutyLt);
                }
                g_dutyArr.push_back(-1);
                g_dutyArrLt.push_back(-1);
            }
            else
            {
                g_dutyArr.push_back(-1);
                g_dutyArrLt.push_back(-1);
                dutyNumLt = mDutyNumLT;
                for (j = -1; j < mDutyNumLT; j++)
                    for (i = -1; i < mDutyNum; i++) {
                        g_dutyArr.push_back(i);
                        g_dutyArrLt.push_back(j);
                    }
                g_dutyArr.push_back(-1);
                g_dutyArrLt.push_back(-1);
            }
        }

        /* init strobe */
        FlashHal::getInstance(mSensorDev)->init();
        FlashHal::getInstance(mSensorDev)->setInCharge(1);

        /* init AE */
        doAeInit();

        /* update status */
        state = CCT_FLASH_STATE_AE;
        preStateEndCnt = mPfFrameCount;
    }

    if (state == CCT_FLASH_STATE_AE) {
        logI("cctCalibration(): state(AE).");
        if (mPfFrameCount % CCT_FLASH_AE_CYCLE_FRAME_COUNT == 0) {
            int ret;
            /* set mas duty */
            int duty = 0;
            int dutyLt = 0;
            if (cust_isDualFlashSupport(mFacingSensor)) {
                /* get max duty and verify */
                FlashCustomAdapter::getInstance(mSensorDev)->getFlashMaxIDuty(&duty, &dutyLt);

                verifyDuty(duty, dutyNum);
                verifyDuty(dutyLt, dutyNumLt);
                logI("cctCalibration(): set max duty(%d/%d,%d/%d)",
                        duty, dutyNum, dutyLt, dutyNumLt);
            } else {
                /* set max duty and verify */
                duty = dutyNum - 1;
                verifyDuty(duty, dutyNum);
                logI("cctCalibration(): set max duty(%d/%d)", duty, dutyNum);
            }
            /* enable strobe */
            FlashHal::getInstance(mSensorDev)->setPreOn();
            ret = usleep(15000);
            if (ret != 0)
                logI("usleep failed!");
            FlashHalInfo flashHalInfo = {};
            flashHalInfo.duty = duty;
            flashHalInfo.dutyLt = dutyLt;
            flashHalInfo.timeout = 300;
            flashHalInfo.timeoutLt = 300;
            FlashHal::getInstance(mSensorDev)->setOnOff(1, flashHalInfo);

        } else if (mPfFrameCount % CCT_FLASH_AE_CYCLE_FRAME_COUNT == 3) {
            /*
             * At each AE cycle plus 3 frames
             * - get mean of yrgb from AAO
             * - turn off flashlight
             * - adjust AE exposure level to target y
             */

            /* get mean of yrgb */
            double yrgb[4] = {0};
            cal_1_4_yrgb_mean(para->staBuf, para->staX, para->staY, yrgb, mLinearOutputEn);
            logI("cctCalibration(): yrgb(%5.3lf,%5.3lf,%5.3lf,%5.3lf).",
                    yrgb[0], yrgb[1], yrgb[2], yrgb[3]);

            /* disable strobe */
            FlashHalInfo flashHalInfo = {};
            FlashHal::getInstance(mSensorDev)->setOnOff(0, flashHalInfo);

            /* do AE */
            AeIn in;
            AeOut out;
            in.sensorDev = mSensorDev;
            in.y = yrgb[2];
            in.tar = 200;
            in.tarMax = 220;
            in.tarMin = 180;
            err = doAe(&in, &out);

            if (out.isEnd) {
                /* get calibration exposure parameters */
                exp = out.exp;
                afe = out.afe;
                isp = out.isp;
                expS1 = out.exp;
                afeS1 = out.afe;
                ispS1 = out.isp;

                int calibrationExp = 0;
                int calibrationAfe = 0;
                int calibrationIsp = 0;
                getPropInt(CCT_FLASH_CALIBRATION_EXP, &calibrationExp, -1);
                getPropInt(CCT_FLASH_CALIBRATION_AFE, &calibrationAfe, -1);
                getPropInt(CCT_FLASH_CALIBRATION_ISP, &calibrationIsp, -1);
                if(calibrationExp != -1 && calibrationAfe != -1 && calibrationIsp != -1)
                {
                    exp = calibrationExp;
                    afe = calibrationAfe;
                    isp = calibrationIsp;
                    expS1 = calibrationExp;
                    afeS1 = calibrationAfe;
                    ispS1 = calibrationIsp;
                    setExp(in.sensorDev, exp, afe, isp);
                }

                /* dump calibration exposure parameters */
                logI("cctCalibration(): calibration AE exp(%d), afe(%d), isp(%d)",
                        exp, afe, isp);

                logI("cctCalibration(): dump calibration AE exposure level.");
                dumpExp(CCT_FLASH_PATH_CALIBRAION_AE, exp, afe, isp);
                writeCaliAeXml(CCT_FLASH_PATH_CALIBRAION_AE_XML,
                        mSensorDev, FlashHal::getInstance(mSensorDev)->getPartId(),
                        exp, afe, isp);

                /* update status */
                state = CCT_FLASH_STATE_AE_POST;
                preStateEndCnt = mPfFrameCount;
            }
        }

    } else if (state == CCT_FLASH_STATE_AE_POST) {
        logI("cctCalibration(): state(AE POST).");

        if (mPfFrameCount > preStateEndCnt + CCT_FLASH_AE_POST_WAIT_FRAME_COUNT) {
            /* update status */
            state = CCT_FLASH_STATE_RATIO;
            preStateEndCnt = mPfFrameCount;
        }

    } else if (state == CCT_FLASH_STATE_RATIO) {
        logI("cctCalibration(): state(RATIO).");

        if (mPfFrameCount == preStateEndCnt + 1) {
            /* init duty index */
            dutyIndex = 0;

            /* init frame base and target */
            frameBase = mPfFrameCount;
            frameTar = mPfFrameCount + 40;
        }

NextId:
        if (mPfFrameCount == frameTar) {
            if (ratioRound == 2) {
                /* get max y */
                float maxY = 0;
                for (j = 0; j < CCT_FLASH_CALCULATE_FRAME; j++)
                    maxY = std::max(maxY, g_CaliData[dutyIndex].g[j]);

                if (maxY < 50 && !adjExp(exp, afe, isp)) {
                    /* adjust exposure level and do it again
                     *
                     * Note, the duty is already sort with decrease.
                     * This will not lead to over exposure.
                     */
                    adjExp(exp, afe, isp, 4);
                    g_CaliData.pop_back();
                } else
                    dutyIndex++;
            } else
                dutyIndex++;

            /* update frame base and target */
            frameBase = mPfFrameCount;
            frameTar = mPfFrameCount + 40;
        }

        int count;
        count = mPfFrameCount - frameBase;
        logI("cctCalibration(): id(%d), count(%d)", dutyIndex, count);

        /* end or next round */
        if (dutyIndex >= (int)g_dutyArr.size()) {
            if (ratioRound == 1) {
                ratioRound = 2;

                /* clear duty array */
                g_dutyArr.clear();
                g_dutyArrLt.clear();

                /* get calibration data dark */
                int sz;
                sz = g_CaliData.size();
                g_CaliDataDark[0] = g_CaliData[0];
                g_CaliDataDark[1] = g_CaliData[sz - 1];

                /* remove front and back */
                g_CaliData.pop_back();
                g_CaliData.pop_front();

                /* get g_CaliData2 from which max y is under 50 in g_CaliData */
                for (i = 0; i < (int)g_CaliData.size(); i++) {
                    int j;
                    float maxY = 0;
                    for (j = 0; j < CCT_FLASH_CALCULATE_FRAME; j++)
                        maxY = std::max(maxY, g_CaliData[i].g[j]);

                    if (maxY < 50) {
                        CaliData tmp;
                        tmp = g_CaliData[i];
                        tmp.ref = maxY;
                        g_CaliData2.push_back(tmp);
                    }
                }

                /* get g_CaliData1 */
                g_CaliData1 = g_CaliData;

                sz = g_CaliData2.size();
                if (!sz) {
                    /* update status */
                    preStateEndCnt = mPfFrameCount;
                    state = CCT_FLASH_STATE_RATIO2;
                    goto FUNC_NEXT;

                } else {
                    /* sort g_CaliData2 */
                    CaliData *tmp;
                    tmp = new CaliData[sz];
                    for (i = 0; i < sz; i++)
                        tmp[i] = g_CaliData2[i];
                    qsort(tmp, sz, sizeof(CaliData), cmpCaliData);
                    for (i = 0; i < sz; i++)
                        g_CaliData2[i] = tmp[i];
                    delete [] tmp;

                    /* set duty array */
                    for (i = 0; i < sz; i++) {
                        g_dutyArr.push_back(g_CaliData2[i].duty);
                        g_dutyArrLt.push_back(g_CaliData2[i].dutyLT);
                    }
                    g_CaliData2.clear();
                    g_CaliData.clear();
                    adjExp(exp, afe, isp, 4);

                    /* reset duty index */
                    dutyIndex = 0;

                    /* reset frame base and target */
                    frameBase = mPfFrameCount + 1;
                    frameTar = mPfFrameCount + 41;
                    goto FUNC_NEXT;
                }
            } else {
                g_CaliData2 = g_CaliData;

                /* update status */
                state = CCT_FLASH_STATE_RATIO2;
                preStateEndCnt = mPfFrameCount;

                goto FUNC_NEXT;
            }
        }

        if (count == 0) {
            int isValidDutyIndex = 0;
            isValidDutyIndex = FlashCustomAdapter::getInstance(mSensorDev)->isValidDuty(
                    g_dutyArr[dutyIndex], g_dutyArrLt[dutyIndex]);

            if (!isValidDutyIndex) {
                frameTar = mPfFrameCount;
                goto NextId;
            }

            /* generate calibration data */
            CaliData tmp;
            tmp.duty = g_dutyArr[dutyIndex];
            tmp.dutyLT = g_dutyArrLt[dutyIndex];
            tmp.exp = exp;
            tmp.afe = afe;
            tmp.isp = isp;
            g_CaliData.push_back(tmp);

            if (ratioRound == 2)
                setExp(mSensorDev, exp, afe, isp);

            /* disable and pre-on strobe */
            FlashHalInfo flashHalInfo = {};
            FlashHal::getInstance(mSensorDev)->setOnOff(0, flashHalInfo);
            FlashHal::getInstance(mSensorDev)->setPreOn();
        }

        if (count == 3) {
            /* enable strobe */
            FlashHalInfo flashHalInfo = {};
            flashHalInfo.duty = g_dutyArr[dutyIndex];
            flashHalInfo.dutyLt = g_dutyArrLt[dutyIndex];
            flashHalInfo.timeout = 300;
            flashHalInfo.timeoutLt = 300;
            FlashHal::getInstance(mSensorDev)->setOnOff(1, flashHalInfo);
        }

        if (count == 6) {
            /* disable strobe */
            FlashHalInfo flashHalInfo = {};
            FlashHal::getInstance(mSensorDev)->setOnOff(0, flashHalInfo);
        }

        if (count == 3 || count == 6) {
            int dataIndex = g_CaliData.size() - 1;
            int index = count / 3 - 1;

            /* get mean of yrgb */
            double yrgb[4] = {0};
            cal_1_4_yrgb_mean(para->staBuf, para->staX, para->staY, yrgb, mLinearOutputEn);

            /* set yrgbt */
            //g_CaliData[dataIndex].y[index] = yrgb[0];
            g_CaliData[dataIndex].y[index] = (5 * yrgb[1] + 9 * yrgb[2] + 2 * yrgb[3]) / 16.0;
            g_CaliData[dataIndex].r[index] = yrgb[1];
            g_CaliData[dataIndex].g[index] = yrgb[2];
            g_CaliData[dataIndex].b[index] = yrgb[3];
            g_CaliData[dataIndex].t[index] = getMs();
            logI("cctCalibration(): id(%d), index(%d), yrgb(%5.3lf,%5.3lf,%5.3lf,%5.3lf).",
                    dutyIndex, index,
                    g_CaliData[dataIndex].y[index], g_CaliData[dataIndex].r[index],
                    g_CaliData[dataIndex].g[index], g_CaliData[dataIndex].b[index]);
        }

    } else if (state == CCT_FLASH_STATE_RATIO2) {
        logI("cctCalibration(): state(RATIO2).");
        static int testNum = 0;
        static int duty[3] = {-1, -1, -1};
        static int dutyLt[3] = {-1, -1, -1};

        if (mPfFrameCount == preStateEndCnt + 1) {
            frameBase = mPfFrameCount;

            /* set the original exposure level */
            setExp(mSensorDev, expS1, afeS1, ispS1);

            int duty1 = -1;
            int dutyLt2 = -1;
            int duty3 = -1;
            int dutyLt3 = -1;
            float maxYRef1 = -1;
            float maxYRef2 = -1;
            float maxYRef3 = -1;

            for (i = 0; i < (int)g_CaliData1.size(); i++) {
                /* get max y */
                int j;
                float maxY = 0;
                for (j = 0; j < CCT_FLASH_CALCULATE_FRAME; j++)
                    maxY = std::max(maxY, g_CaliData1[i].g[j]);

                /* get max reference y and related duty */
                if (maxYRef1 < maxY && g_CaliData1[i].dutyLT == -1) {
                    maxYRef1 = maxY;
                    duty1 = g_CaliData1[i].duty;
                }
                if (maxYRef2 < maxY && g_CaliData1[i].duty == -1) {
                    maxYRef2 = maxY;
                    dutyLt2 = g_CaliData1[i].dutyLT;
                }
                if (maxYRef3 < maxY) {
                    maxYRef3 = maxY;
                    duty3 = g_CaliData1[i].duty;
                    dutyLt3 = g_CaliData1[i].dutyLT;
                }
            }

            /* get duty arrary */
            testNum = 0;
            if (maxYRef1 != -1) {
                duty[testNum] = duty1;
                dutyLt[testNum] = -1;
                testNum++;
            }
            if (maxYRef2 != -1) {
                duty[testNum] = -1;
                dutyLt[testNum] = dutyLt2;
                testNum++;
            }
            if ((duty3 != -1 && dutyLt3 != -1) || !testNum) {
                duty[testNum] = duty3;
                dutyLt[testNum] = dutyLt3;
                testNum++;
            }

            /* clear calibration */
            g_CaliData3.clear();
        }

        int id = (mPfFrameCount - frameBase) / 40;
        int count = (mPfFrameCount - frameBase) % 40;
        logI("cctCalibration(): id(%d), count(%d)", id, count);

        if (id >= testNum) {
            /* update status */
            state = CCT_FLASH_STATE_END;
            preStateEndCnt = mPfFrameCount;
            goto FUNC_NEXT;
        }

        if (count == 0) {
            CaliData tmp;
            tmp.exp = expS1;
            tmp.afe = afeS1;
            tmp.isp = ispS1;
            tmp.duty = duty[id];
            tmp.dutyLT = dutyLt[id];
            g_CaliData3.push_back(tmp);

            /* disable and pre-on strobe */
            FlashHalInfo flashHalInfo = {};
            FlashHal::getInstance(mSensorDev)->setOnOff(0, flashHalInfo);
            FlashHal::getInstance(mSensorDev)->setPreOn();
        } else if (count == 3) {
            /* enable strobe */
            FlashHalInfo flashHalInfo = {};
            flashHalInfo.duty = duty[id];
            flashHalInfo.dutyLt = dutyLt[id];
            flashHalInfo.timeout = 300;
            flashHalInfo.timeoutLt = 300;
            FlashHal::getInstance(mSensorDev)->setOnOff(1, flashHalInfo);

        } else if (count == 6) {
            /* disable strobe */
            FlashHalInfo flashHalInfo = {};
            FlashHal::getInstance(mSensorDev)->setOnOff(0, flashHalInfo);
        }

        if (count == 3 || count == 6) {
            int index = count / 3 - 1;

            /* get mean of yrgb */
            double yrgb[4] = {0};
            cal_1_4_yrgb_mean(para->staBuf, para->staX, para->staY, yrgb, mLinearOutputEn);

            /* set yrgbt */
            //g_CaliData3[id].y[count] = yrgb[0];
            g_CaliData3[id].y[index] = (5 * yrgb[1] + 9 * yrgb[2] + 2 * yrgb[3]) / 16.0;
            g_CaliData3[id].r[index] = yrgb[1];
            g_CaliData3[id].g[index] = yrgb[2];
            g_CaliData3[id].b[index] = yrgb[3];
            g_CaliData3[id].t[index] = getMs();
            logI("cctCalibration(): id(%d), index(%d), yrgb(%5.3lf,%5.3lf,%5.3lf,%5.3lf).",
                    id, index,
                    g_CaliData3[id].y[index], g_CaliData3[id].r[index],
                    g_CaliData3[id].g[index], g_CaliData3[id].b[index]);
        }

    } else if (state == CCT_FLASH_STATE_END) {
        logI("cctCalibration(): state(END).");

        /* dump calibration data */
        int driverFault =FlashHal::getInstance(mSensorDev)->getDriverFault();
        dumpDequeCaliData(CCT_FLASH_PATH_ENG_ALL, g_CaliData1, g_CaliData2, g_CaliData3, driverFault);
        logI("cctCalibration(): dump xml(%ld) +.", getMs());
        writeEngAllXml(CCT_FLASH_PATH_ENG_ALL_XML, g_CaliData1, g_CaliData2, g_CaliData3,
                mSensorDev, FlashHal::getInstance(mSensorDev)->getPartId());
        logI("cctCalibration(): dump xml(%ld) -.", getMs());

        /* get diff of dark y */
        double ymDark1 = 0;
        double ymDark2 = 0;
        double difY;
        for (i = 0; i < CCT_FLASH_CALCULATE_FRAME; i++) {
            ymDark1 += g_CaliDataDark[0].y[i];
            ymDark2 += g_CaliDataDark[1].y[i];
        }
        ymDark1 /= CCT_FLASH_CALCULATE_FRAME;
        ymDark2 /= CCT_FLASH_CALCULATE_FRAME;
        difY = ymDark1 - ymDark2;
        if (difY < 0)
            difY = -difY;

        /* check light source constancy */
        if (difY > 8) {
            err = e_LightSourceNotConstant;
            goto FUNC_END;
        }

        /* get ob of y */
        float yob = (ymDark1 + ymDark2) / 2;

        /* get max of y */
        float ymax = 0;
        for (i = 0; i < (int)g_CaliData1.size(); i++)
            for (j = 0; j < CCT_FLASH_CALCULATE_FRAME; j++)
                ymax = std::max(ymax, g_CaliData1[i].y[j]);

        /* check object distance */
        if (ymax < 2 * yob) {
            err = e_ObjIsFar;
            goto FUNC_END;
        }

        /* OB and normalize y to [0-9999] */
        for (i = 0; i < (int)g_CaliData1.size(); i++)
            for (j = 0; j < CCT_FLASH_CALCULATE_FRAME; j++)
                g_CaliData1[i].y[j] = (g_CaliData1[i].y[j] - yob) / (ymax - yob) * 9999;

        /* get nvram buf for dump xml */
        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *nvCali = NULL;
        FlashNvram::nvReadFlashCali(nvCali, mSensorDev);
        if (nvCali == NULL) {
            logE("cctCalibration(): NULL pointer from nvReadFlashCali.");
            err = e_NvramError;
            goto FUNC_END;
        }

        /* clear energy table */
        FlashCustomAdapter::getInstance(mSensorDev)->clearYTab();

        //memset(&nvFlash, 0, sizeof(NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT));

        /* get max of y and related rgb */
        int ind;
        int duty;
        int dutyLt;
        int isDual = cust_isDualFlashSupport(mFacingSensor);
        double Vr;
        double Vg;
        double Vb;
        for (i = 0; i < FLASH_CUSTOM_MAX_DUTY_NUM; i++) {
            ymax = -1;
            Vr = -1;
            Vg = -1;
            Vb = -1;

            if (!FlashDuty::ind2duty(i, duty, dutyLt, dutyNum, dutyNumLt, isDual)) {
                /* search from calibration data 1 */
                ind = -1;
                for (j = 0; j < (int)g_CaliData1.size(); j++) {
                    if (g_CaliData1[j].duty == duty &&
                            (g_CaliData1[j].dutyLT == dutyLt || !isDual)) {
                        ind = j;
                        break;
                    }
                }

                if (ind != -1)
                    for (j = 0; j < CCT_FLASH_CALCULATE_FRAME; j++)
                        if (ymax < g_CaliData1[ind].y[j]) {
                            ymax = g_CaliData1[ind].y[j];
                            Vr = g_CaliData1[ind].r[j];
                            Vg = g_CaliData1[ind].g[j];
                            Vb = g_CaliData1[ind].b[j];
                        }

                /* search from calibration data 2 */
                if (ind != -1 && Vg <= 50) {
                    ind = -1;
                    for (j = 0; j < (int)g_CaliData2.size(); j++) {
                        if (g_CaliData2[j].duty == duty &&
                                (g_CaliData2[j].dutyLT == dutyLt || !isDual)) {
                            ind = j;
                            break;
                        }
                    }
                    if (ind != -1) {
                        double ymax2 = 0;
                        for (j = 0; j < CCT_FLASH_CALCULATE_FRAME; j++) {
                            if (ymax2 < g_CaliData2[ind].y[j]) {
                                ymax2 = g_CaliData2[ind].y[j];
                                Vr = g_CaliData2[ind].r[j];
                                Vg = g_CaliData2[ind].g[j];
                                Vb = g_CaliData2[ind].b[j];
                            }
                        }
                    }
                }
            }
            FlashCustomAdapter::getInstance(mSensorDev)->setYTabByDutyIndex(i, ymax);
            FlashCustomAdapter::getInstance(mSensorDev)->setWBGainByDutyIndex(i,
                    Vg * 512 / Vr, 512, Vg * 512 / Vb);
        }

        /* dump to file */
        FlashCustomAdapter::getInstance(mSensorDev)->dumpNvFlashAe(CCT_FLASH_PATH_ENG);
        FlashCustomAdapter::getInstance(mSensorDev)->dumpNvFlashAwb(CCT_FLASH_PATH_FWB, FLASH_CUSTOM_MAX_DUTY_NUM);

        logI("cctCalibration(): dump xml(%ld) +.", getMs());
        writeFlashCali(CCT_FLASH_PATH_CALI, nvCali);

        writeEngTabXml(CCT_FLASH_PATH_ENG_XML, nvCali,
                mSensorDev, FlashHal::getInstance(mSensorDev)->getPartId(),
                dutyNum, dutyNumLt);

        writeFwbGainXml(CCT_FLASH_PATH_FWB_XML, nvCali,
                mSensorDev, FlashHal::getInstance(mSensorDev)->getPartId(),
                dutyNum, dutyNumLt);
        logI("cctCalibration(): dump xml(%ld) -.", getMs());

        FlashPlatformAdapter::getInstance(mSensorDev)->restoreAwbModuleVariationGain(&awbPreGain1Prev);
        mLinearOutputEn = FlashPlatformAdapter::getInstance(mSensorDev)->getAwbLinearOutputEn();
        FlashPlatformAdapter::getInstance(mSensorDev)->setAeCCUOnOff(MTRUE);

        /* clear data deque */
        g_CaliData.clear();
        g_CaliData1.clear();
        g_CaliData2.clear();
        g_CaliData3.clear();

        /* clear duty array */
        g_dutyArr.clear();
        g_dutyArrLt.clear();

        /* update flash info */
        rep->isEnd = 1;
        mIsCalibration = 0;

        /* uninit strobe */
        FlashHal::getInstance(mSensorDev)->setInCharge(inCharge);
        FlashHal::getInstance(mSensorDev)->uninit();

        /* update status */
        state = CCT_FLASH_STATE_END2;
    }

FUNC_NEXT:
FUNC_END:
    if (err) {
        logI("cctCalibration(): calibration failed(%d).", err);

        FlashPlatformAdapter::getInstance(mSensorDev)->restoreAwbModuleVariationGain(&awbPreGain1Prev);
        FlashPlatformAdapter::getInstance(mSensorDev)->setAeCCUOnOff(MTRUE);

        /* update flash info */
        rep->isEnd = 1;
        mIsCalibration = 0;
        mCapRep.isMainFlash = 0;

        /* uninit strobe */
        FlashHal::getInstance(mSensorDev)->setInCharge(inCharge);
        FlashHal::getInstance(mSensorDev)->uninit();

        /* update AE capture info */
        AE_MODE_CFG_T capInfo;
        AeMgr::getInstance(mSensorDev).getCaptureParams(capInfo);
        capInfo.u4Eposuretime = 1000;
        capInfo.u4AfeGain = 1024;
        capInfo.u4IspGain = FlashPlatformAdapter::getIspGainBase();
        AeMgr::getInstance(mSensorDev).updateCaptureParams(capInfo);
    }
    return err;
}


/***********************************************************
 * CCT feature interface
 **********************************************************/
int FlashMgrM::setManualFlash(int duty, int dutyLt)
{
    /* set flash mode */
    setFlashMode(LIB3A_FLASH_MODE_FORCE_ON);

    /* clear and set manual duty */
    mManualDutyCct = -1;
    mManualDutyLtCct = -1;
    if (!duty && !dutyLt)
        mIsManualFlashEnCct = 0;
    else {
        mIsManualFlashEnCct = 1;
        /* only care on or off */
        if (duty)
            mManualDutyCct = FlashCustomAdapter::getInstance(mSensorDev)->getMfDutyMax();
        if (dutyLt)
            mManualDutyLtCct = FlashCustomAdapter::getInstance(mSensorDev)->getMfDutyMaxLt();
        if (duty && dutyLt && cust_isDualFlashSupport(mFacingSensor))
            FlashCustomAdapter::getInstance(mSensorDev)->getFlashMaxIDuty(&mManualDutyCct,
                    &mManualDutyLtCct);
    }

    logD("setManualFlash(): enable manual(%d), manual duty(%d), manual dutyLt(%d).",
            mIsManualFlashEnCct, mManualDutyCct, mManualDutyLtCct);

    return 0;
}

int FlashMgrM::clearManualFlash()
{
    logD("clearManualFlash().");
    setFlashMode(LIB3A_FLASH_MODE_FORCE_OFF);
    mIsManualFlashEnCct = 0;
    return 0;
}

int FlashMgrM::cctSetSpModeNormal()
{
    if (!mSpModeLockCct)
        mSpModeCct = e_SpModeNormal;
    return 0;
}

int FlashMgrM::cctSetSpModeQuickCalibration2()
{
    if (!mSpModeLockCct)
        mSpModeCct = e_SpModeQuickCalibration2;
    return 0;
}

int FlashMgrM::cctSetSpModeCalibration()
{
    if (!mSpModeLockCct)
        mSpModeCct = e_SpModeCalibration;
    return 0;
}

MBOOL FlashMgrM::cctIsSpModeCalibration()
{
    if(mSpModeCct == e_SpModeCalibration)
    {
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}

int FlashMgrM::cctSetSpModeLock(int lock)
{
    mSpModeLockCct = lock;
    return 0;
}

int FlashMgrM::cctGetFlashInfo(int *isOn)
{
    logD("cctGetFlashInfo().");

    if (getFlashMode() == LIB3A_FLASH_MODE_FORCE_ON)
        *isOn = 1;
    else
        *isOn = 0;

    return 0;
}

int FlashMgrM::cctFlashLightTest(void *pIn)
{
    (void)pIn;
    int ret;

    logD("cctFlashLightTest().");

    int inCharge = FlashHal::getInstance(mSensorDev)->getInCharge();

    FlashHal::getInstance(mSensorDev)->init();
    FlashHal::getInstance(mSensorDev)->setInCharge(1);
    FlashHal::getInstance(mSensorDev)->setOnOff(1, FLASH_HAL_SCENARIO_VIDEO_TORCH);
    ret = usleep(300000);
    if (ret != 0)
        logI("usleep failed!");
    FlashHal::getInstance(mSensorDev)->setOnOff(0, FLASH_HAL_SCENARIO_VIDEO_TORCH);
    FlashHal::getInstance(mSensorDev)->setInCharge(inCharge);
    FlashHal::getInstance(mSensorDev)->uninit();

    return 0;
}

int FlashMgrM::cctReadNvramToPcMeta(void *out, MUINT32 *realOutSize)
{
    logI("cctReadNvramToPcMeta().");

    /* verify arguments */
    if (!out || !realOutSize) {
        logE("cctReadNvramToPcMeta(): invalid arguments.");
        return -1;
    }

    NVRAM_CAMERA_STROBE_STRUCT *pNv = NULL;
    int error = FlashNvram::nvReadStrobe(pNv, mSensorDev, 1);
    if (error)
    {
        logE("cctReadNvramToPcMeta(): nvReadStrobe failed(%d).", error);
        return error;
    }
    if (pNv == NULL) {
        logE("cctReadNvramToPcMeta(): NULL pointer from nvReadStrobe.");
        return -1;
    }
    *realOutSize = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
    memcpy((void *)out, pNv, *realOutSize);

    return 0;
}

int FlashMgrM::cctSetNvdataMeta(void *in, int inSize)
{
    logI("cctSetNvdataMeta().");

    /* verify arguments */
    if (!in || inSize != sizeof(ACDK_STROBE_STRUCT)) {
        logE("cctSetNvdataMeta(): invalid arguments.");
        return FL_ERR_CCT_INPUT_SIZE_WRONG;
    }

    NVRAM_CAMERA_STROBE_STRUCT *pNv = NULL;
    int error = FlashNvram::nvReadStrobe(pNv, mSensorDev);
    if (error)
    {
        logE("cctSetNvdataMeta(): nvReadStrobe failed(%d).", error);
        return error;
    }
    if (pNv == NULL) {
        logE("cctSetNvdataMeta(): NULL pointer from nvReadStrobe.");
        return -1;
    }
    memcpy(pNv, (void *)in, sizeof(NVRAM_CAMERA_STROBE_STRUCT));

    return 0;
}

int FlashMgrM::cctWriteNvram()
{
    logI("cctWriteNvram().");
    return FlashNvram::nvWriteStrobe(mSensorDev);
}

int FlashMgrM::cctFlashEnable(int enable)
{
    logI("cctFlashEnable(): enable(%d).", enable);

    if (enable)
        setAeFlashMode(MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH, MTK_FLASH_MODE_SINGLE);
    else
        setAeFlashMode(MTK_CONTROL_AE_MODE_ON, MTK_FLASH_MODE_OFF);

    return 0;
}

