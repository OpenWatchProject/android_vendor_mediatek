#define LOG_TAG "flashCctQuick2"

/***********************************************************
 * Headers
 **********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>

/* aaa common headers */
#include "time_utils.h"
#include "log_utils.h"
#include "property_utils.h"

/* aaa headers */
#include "nvbuf_util.h"
#include "ae_mgr.h"
#include "awb_mgr_if.h"
#include "aaa_sensor_mgr.h"

/* custom headers */
#include "cct_feature.h"
#include "camera_custom_nvram.h"
#include "aaa_error_code.h"
#include "flash_error_code.h"
#include "ae_param.h"
#include "flash_awb_param.h"
#include "flash_tuning_custom.h"

/* flash algo header */
#include "FlashAlg.h"

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
#include "flash_algo_adapter.h"

using namespace NS3Av3;

/***********************************************************
 * Define macros
 **********************************************************/
/* max unit gain variance property name */
#define PROP_FLASH_MAX_R_UNIT_GAIN_VARIANCE "vendor.flash_max_r_gain_variance"
#define PROP_FLASH_MAX_B_UNIT_GAIN_VARIANCE "vendor.flash_max_b_gain_variance"

#define PROP_FLASH_GOLDEN_DATA_DUMP_EN      "vendor.flash_golden_data_dump_en"

#define CCT_FLASH_CALIBRATION_NUM_MAX 7

/* max unit gain variance  */
#define CHECK_GAIN (0)
#define MAX_R_UNIT_GAIN_VARIANCE 100
#define MAX_B_UNIT_GAIN_VARIANCE 100

static dqCaliData g_CaliData;
static dqCaliData g_CaliData1;
static dqCaliData g_CaliData2;
static dqCaliData g_CaliData3;
static vectorInt g_dutyArr;
static vectorInt g_dutyArrLt;
static vectorInt g_expArr;
static vectorInt g_afeArr;
static vectorInt g_ispArr;
static CalData g_algCalData[CCT_FLASH_CALIBRATION_NUM_MAX];
static int g_quickCalNum = CCT_FLASH_CALIBRATION_NUM_MAX;


/***********************************************************
 * Misc
 **********************************************************/
int dumpCalibrationResult(const char *fname, int calResult)
{
    /* verify arguments */
    if (!fname)
        return -1;

    FILE *fp = fopen(fname, "wt");
    if (!fp)
        return -1;

    if (calResult == S_FLASH_OK)
        fprintf(fp, "@result pass\n");
    else
        fprintf(fp, "@result fail\n");

    if (calResult == E_FLASH_CALI_ENVIRONMENT_NOT_DARK_ERR) {
        fprintf(fp, "@err_detail E_FLASH_CALI_ENVIRONMENT_NOT_DARK_ERR\n");
        fprintf(fp, "Environment is not dark. Calibration environment should be dark.\n");
    } else if(calResult == E_FLASH_CALI_ENVIRONMENT_WRONG_Y_IS_LOW_ERR) {
        fprintf(fp, "@err_detail E_FLASH_CALI_ENVIRONMENT_WRONG_Y_IS_LOW_ERR\n");
        fprintf(fp, "1. Environment may not correct.\n");
        fprintf(fp, "2. init exp may not correct. set correct value to cust_getFlashQuick2CalibrationExp_main()\n");
    } else if(calResult == E_FLASH_CALI_OVEREXPOSURE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_CALI_OVEREXPOSURE_ERR\n");
        fprintf(fp, "during calibration, y mean is overexposure. Please modify exposure setting or environment.\n");
    } else if(calResult == E_FLASH_DUAL_CALI_PARAM_ERR) {
        fprintf(fp, "@err_detail E_FLASH_DUAL_CALI_PARAM_ERR\n");
        fprintf(fp, "There is error in calibration flow. Please provide log for further analyzing\n");
    } else if(calResult == E_FLASH_DUAL_CALI_IN_Y_RANGE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_DUAL_CALI_IN_Y_RANGE_ERR\n");
        fprintf(fp, "There is error in calibration flow. Please provide log for further analyzing\n");
    } else if(calResult == E_FLASH_DUAL_CALI_IN_Y_VALUE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_DUAL_CALI_IN_Y_VALUE_ERR\n");
        fprintf(fp, "Perhaps light leaks during calibration, please check the calibration envrironment.\n");
    } else if(calResult == E_FLASH_DUAL_CALI_IN_WB_RANGE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_DUAL_CALI_IN_WB_RANGE_ERR\n");
        fprintf(fp, "There is error in calibration flow. Please provide log for further analyzing\n");
    } else if(calResult == E_FLASH_DUAL_CALI_IN_WB_VALUE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_DUAL_CALI_IN_WB_VALUE_ERR\n");
        fprintf(fp, "The color or energy of flash leds is wrong. please check leds and driver\n");
    } else if(calResult == E_FLASH_DUAL_CALI_GOLD_Y_RANGE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_DUAL_CALI_GOLD_Y_RANGE_ERR\n");
        fprintf(fp, "There is error in calibration flow. Please provide log for further analyzing\n");
    } else if(calResult == E_FLASH_DUAL_CALI_GOLD_Y_VALUE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_DUAL_CALI_GOLD_Y_VALUE_ERR\n");
        fprintf(fp, "The color or energy of flash leds is wrong. please check leds and driver\n");
    } else if(calResult == E_FLASH_DUAL_CALI_GOLD_WB_RANGE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_DUAL_CALI_GOLD_WB_RANGE_ERR\n");
        fprintf(fp, "There is error in calibration flow. Please provide log for further analyzing\n");
    } else if(calResult == E_FLASH_DUAL_CALI_GOLD_WB_VALUE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_DUAL_CALI_GOLD_WB_VALUE_ERR\n");
        fprintf(fp, "The color or energy of flash leds is wrong. please check leds and driver\n");
    }
    else if(calResult == E_FLASH_CALI_UNIT_GAIN_VARIANCE_ERR) {
        fprintf(fp, "@err_detail E_FLASH_CALI_UNIT_GAIN_VARIANCE_ERR\n");
        fprintf(fp, "1. Environment may not correct.\n");
        fprintf(fp, "2. LED module color variance may be too large. Please adjust MAX_R_UNIT_GAIN_VARIANCE & MAX_B_UNIT_GAIN_VARIANCE\n");
    }
    fclose(fp);

    return 0;
}


/***********************************************************
 * Calibration
 **********************************************************/
int FlashMgrM::cctGetQuickCalibrationResult()
{
    return mQuickCalibrationResult;
}

int FlashMgrM::cctCalibrationQuick2(FlashExePara *para, FlashExeRep *rep)
{
    static int state = CCT_FLASH_STATE_INIT;
    static int dutyNum;
    static int dutyNumLt;
    static int preStateEndCnt = -1;
    static int dutyIndex = 0;
    static int frameTar = 0;
    static int frameBase = 0;

    static int aeReconvergeCnt = 0;

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
        logI("cctCalibrationQuick2(): backup AWB pregain1 (%d,%d,%d).",
                awbPreGain1Prev.i4R, awbPreGain1Prev.i4G, awbPreGain1Prev.i4B);

        /* clear deque */
        g_CaliData.clear();
        g_CaliData1.clear();
        g_CaliData2.clear();

        /* clear vector */
        g_dutyArr.clear();
        g_dutyArrLt.clear();
        g_expArr.clear();
        g_afeArr.clear();
        g_ispArr.clear();

        dutyNum = 0;
        dutyNumLt = 0;

        preStateEndCnt = 0;

        dutyIndex = 0;
        frameTar = 0;
        frameBase = 0;
        state = CCT_FLASH_STATE_INIT;
    }

    FlashPlatformAdapter::getInstance(mSensorDev)->clearAwbModuleVariationGain(&awbPreGain1Curr);
    if (state == CCT_FLASH_STATE_INIT) {
        logI("cctCalibrationQuick2(): state(INIT).");

        FlashPlatformAdapter::getInstance(mSensorDev)->setAeCCUOnOff(MFALSE);

        /* set base exposure level */
        int expBase;
        int afeBase;
        int ispBase;
        getPropInt(CCT_FLASH_AE_EXP, &expBase, 0);
        getPropInt(CCT_FLASH_AE_AFE, &afeBase, 0);
        getPropInt(CCT_FLASH_AE_ISP, &ispBase, 0);

        /* read nvram */
        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *pNvCali = NULL;
        FlashNvram::nvReadFlashCaliDefault(pNvCali, mSensorDev);
        if (pNvCali == NULL) {
            logE("cctCalibrationQuick2(): NULL pointer from nvReadFlashCaliDefault.");
            err = e_NvramError;
            goto FUNC_END;
        }

        int iGoldenDataDumpEn = 0;
        getPropInt(PROP_FLASH_GOLDEN_DATA_DUMP_EN, &iGoldenDataDumpEn, 0);
        if (iGoldenDataDumpEn)
            showFlashCali(pNvCali);
        if (!expBase || !afeBase || !ispBase){
            FlashCustomAdapter::getInstance(mSensorDev)->getFastKExp(&expBase, &afeBase, &ispBase);
        }
        logI("cctCalibrationQuick2(): expBase(%d), afeBase(%d), ispBase(%d).",
                expBase, afeBase, ispBase);

        /* get project parameters */
        FLASH_PROJECT_PARA prjPara;
        prjPara = FlashCustomAdapter::getInstance(mSensorDev)->getFlashProjectPara(LIB3A_AE_SCENE_AUTO, 0);
        dutyNum = mDutyNum;
        dutyNumLt = 1;
        if (cust_isDualFlashSupport(mFacingSensor))
            dutyNumLt = mDutyNumLT;

        FlashHal::getInstance(mSensorDev)->getCurrentTab(ITab1, ITab2);

        /* set fast calibration number */
        if (cust_isDualFlashSupport(mFacingSensor))
            g_quickCalNum = CCT_FLASH_CALIBRATION_NUM_MAX;
        else
            g_quickCalNum = 3;
        if (prjPara.quickCalibrationNum)
            g_quickCalNum = prjPara.quickCalibrationNum;
        /* get energy base */
        int dutyIndex = 0;
        int isDual = cust_isDualFlashSupport(mFacingSensor);
        FlashDuty::duty2ind(dutyIndex, dutyNum - 1, -1, dutyNum, dutyNumLt, isDual);

        double engBase = FlashCustomAdapter::getInstance(mSensorDev)->getYTabByDutyIndex(dutyIndex);

        /* clear calibration result */
        mQuickCalibrationResult = S_FLASH_OK;

        /* clear choose duty */
        ChooseResult choose[CCT_FLASH_CALIBRATION_NUM_MAX];
        for (i = 0; i < CCT_FLASH_CALIBRATION_NUM_MAX; i++)
            choose[i].dutyLT = -1;

        /* decide choose duty */
        logI("cctCalibrationQuick2(): calibration number(%d), dutyNum(%d), dutyNumLt(%d), isDual(%d).",
                g_quickCalNum, dutyNum, dutyNumLt, isDual);

        int flashCaliNvIdx = FlashCustomAdapter::getInstance(mSensorDev)->getFlashCaliNvIdx();
        FlashAlgoAdapter::getInstance(mSensorDev)->DecideCalFlashComb(g_quickCalNum, pNvCali,
                dutyNum, mDutyNumLT, choose, isDual, flashCaliNvIdx);
        if (err != S_FLASH_OK) {
            logE("cctCalibrationQuick2(): failed to decide choose duty.");
            goto FUNC_END;
        }

        for (i = 0; i < g_quickCalNum; i++) {
            /* create duty array */
            logI("cctCalibrationQuick2(): choose(%d), duty(%d), dutyLt(%d).", i, choose[i].duty, choose[i].dutyLT);
            g_dutyArr.push_back(choose[i].duty);
            g_dutyArrLt.push_back(choose[i].dutyLT);
            g_algCalData[i].duty = choose[i].duty;
            g_algCalData[i].dutyLT = choose[i].dutyLT;

            /* create exposure level array */
            FlashDuty::duty2ind(dutyIndex, choose[i].duty, choose[i].dutyLT, dutyNum, dutyNumLt, isDual);
            int eng = FlashCustomAdapter::getInstance(mSensorDev)->getYTabByDutyIndex(dutyIndex);
            if (eng == -1)
                eng = engBase;
            int exp2 = expBase;
            int afe2 = afeBase;
            int isp2 = ispBase;
            double m = engBase / eng;
            adjExp(exp2, afe2, isp2, m);

            int calibrationExp = 0;
            int calibrationAfe = 0;
            int calibrationIsp = 0;
            getPropInt(CCT_FLASH_CALIBRATION_EXP, &calibrationExp, -1);
            getPropInt(CCT_FLASH_CALIBRATION_AFE, &calibrationAfe, -1);
            getPropInt(CCT_FLASH_CALIBRATION_ISP, &calibrationIsp, -1);
            if(calibrationExp != -1 && calibrationAfe != -1 && calibrationIsp != -1)
            {
                exp2 = calibrationExp;
                afe2 = calibrationAfe;
                isp2 = calibrationIsp;
            }

            g_expArr.push_back(exp2);
            g_afeArr.push_back(afe2);
            g_ispArr.push_back(isp2);
        }
        /* init strobe */
        FlashHal::getInstance(mSensorDev)->init();
        m_bCctFlashHalInit = true;
        FlashHal::getInstance(mSensorDev)->setInCharge(1);

        /* update status */
        state = CCT_FLASH_STATE_RATIO;
        preStateEndCnt = mPfFrameCount;
    }

    if (state == CCT_FLASH_STATE_RATIO) {
        logI("cctCalibrationQuick2(): state(RATIO).");
        if (mPfFrameCount == preStateEndCnt) {
            /* init duty index */
            dutyIndex = 0;

            /* init frame base and target */
            frameBase = mPfFrameCount;
            frameTar = mPfFrameCount + 7;
        }

NextId:
        if (mPfFrameCount == frameTar) {
            int dataIndex = g_CaliData.size() - 1;
            if(dataIndex != 0 && (g_CaliData[dataIndex].g[1] < 40 || g_CaliData[dataIndex].g[1] > 230) && aeReconvergeCnt < CCT_FLASH_RECONVERGE_AE_MAX_CYCLE)
            {
                double m = 60.0 / double(g_CaliData[dataIndex].g[1]);
                for(i = dataIndex; i < g_quickCalNum; i++)
                {
                    int expBefore = g_expArr[i];
                    int afeBefore = g_afeArr[i];
                    int ispBefore = g_ispArr[i];
                    adjExp(g_expArr[i], g_afeArr[i], g_ispArr[i], m);
                    logI("cctCalibrationQuick2(): re-converge exposure setting, aeReconvergeCnt(%d) id(%d), exp(%d => %d), afe(%d => %d), isp(%d => %d).",
                        aeReconvergeCnt, i, expBefore, g_expArr[i], afeBefore, g_afeArr[i], ispBefore, g_ispArr[i]);
                }
                g_CaliData.pop_back();
                aeReconvergeCnt++;
            }
            else
            {
                aeReconvergeCnt = 0;
                dutyIndex++;
            }

            if(dutyIndex < (int)g_dutyArr.size())
            {
                FLASH_PROJECT_PARA prjPara = cust_getFlashProjectPara_V3(mFacingSensor, LIB3A_AE_SCENE_AUTO, 0, NULL);
                int coolingTM = 0;
                for(int i = 0;i < prjPara.coolTimeOutPara.tabNum; i++)
                {
                    if(g_dutyArr[dutyIndex] <= prjPara.coolTimeOutPara.tabId[i])
                    {
                        coolingTM = prjPara.coolTimeOutPara.coolingTM[i];
                        break;
                    }
                }
                /* update frame base and target */
                frameBase = mPfFrameCount;
                if(coolingTM > 0)
                {
                    frameTar = mPfFrameCount + 4 + 4*coolingTM;
                }
                else
                {
                    frameTar = mPfFrameCount + 7;
                }
            }
        }

        int count;
        count = mPfFrameCount - frameBase;
        logI("cctCalibrationQuick2(): id(%d), count(%d)", dutyIndex, count);

        /* end */
        if (dutyIndex >= (int)g_dutyArr.size()) {
            /* clear duty array and exposure level array */
            g_dutyArr.clear();
            g_dutyArrLt.clear();
            g_expArr.clear();
            g_afeArr.clear();
            g_ispArr.clear();

            /* get g_CaliData1 */
            g_CaliData1 = g_CaliData;

            /* update status */
            preStateEndCnt = mPfFrameCount;
            state = CCT_FLASH_STATE_END;
            goto FUNC_NEXT;
        }

        if (count == 0) {
            int isValidDutyIndex = 0;
            isValidDutyIndex = FlashCustomAdapter::getInstance(mSensorDev)->isValidDuty(
                    g_dutyArr[dutyIndex], g_dutyArrLt[dutyIndex]);

            if (!isValidDutyIndex) {
                frameTar = mPfFrameCount;
                goto NextId;
            }

            CaliData tmp;
            tmp.duty = g_dutyArr[dutyIndex];
            tmp.dutyLT = g_dutyArrLt[dutyIndex];
            tmp.exp = g_expArr[dutyIndex];
            tmp.afe = g_afeArr[dutyIndex];
            tmp.isp = g_ispArr[dutyIndex];
            g_CaliData.push_back(tmp);

            setExp(mSensorDev, g_expArr[dutyIndex], g_afeArr[dutyIndex], g_ispArr[dutyIndex]);

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
            double yrgb[4];
            cal_1_4_yrgb_mean(para->staBuf, para->staX, para->staY, yrgb, mLinearOutputEn);

            /* set yrgbt */
            //g_CaliData[dataIndex].y[index] = yrgb[0];
            g_CaliData[dataIndex].y[index] = (5 * yrgb[1] + 9 * yrgb[2] + 2 * yrgb[3]) / 16.0;
            g_CaliData[dataIndex].r[index] = yrgb[1];
            g_CaliData[dataIndex].g[index] = yrgb[2];
            g_CaliData[dataIndex].b[index] = yrgb[3];
            g_CaliData[dataIndex].t[index] = getMs();
            logI("cctCalibrationQuick2(): id(%d), index(%d), yrgb(%5.3lf,%5.3lf,%5.3lf,%5.3lf).",
                    dutyIndex, index,
                    g_CaliData[dataIndex].y[index], g_CaliData[dataIndex].r[index],
                    g_CaliData[dataIndex].g[index], g_CaliData[dataIndex].b[index]);
        }

    } else if (state == CCT_FLASH_STATE_END) {
FUNC_NEXT:
        logI("cctCalibrationQuick2(): state(END).");

        /* dump calibration data */
        int driverFault =FlashHal::getInstance(mSensorDev)->getDriverFault();
        dumpDequeCaliData(CCT_FLASH_PATH_ENG_ALL, g_CaliData1, g_CaliData2, g_CaliData3, driverFault);
        logI("cctCalibration(): dump xml(%ld) +.", getMs());
        writeEngAllXml(CCT_FLASH_PATH_ENG_ALL_XML, g_CaliData1, g_CaliData2, g_CaliData3,
                mSensorDev, FlashHal::getInstance(mSensorDev)->getPartId());
        logI("cctCalibration(): dump xml(%ld) -.", getMs());

        /* check calibration environment */
        float gmax = -1;
        float maxYTest = 0;
        for (j = 0; j < (int)g_CaliData1.size(); j++) {
            gmax = -1;

            /* get max g and max y */
            for (i = 0; i < CCT_FLASH_CALCULATE_FRAME; i++)
                gmax = std::max(gmax, g_CaliData1[j].g[i]);
            maxYTest = std::max(maxYTest, gmax);

            if (g_CaliData1[j].duty == -1 && g_CaliData1[j].dutyLT == -1) {
                if (gmax > 10) {
                    err = E_FLASH_CALI_ENVIRONMENT_NOT_DARK_ERR;
                    break;
                }
            }
            if (gmax > 230) {
                err = E_FLASH_CALI_OVEREXPOSURE_ERR;
                break;
            }
        }
        if (maxYTest < 40)
            err = E_FLASH_CALI_ENVIRONMENT_WRONG_Y_IS_LOW_ERR;
        if (err != S_FLASH_OK)
            goto FUNC_END;

        /* get nvram buf of strobe and 3a */
        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *nvCali = NULL;
        FlashNvram::nvReadFlashCaliDefault(nvCali, mSensorDev);
        if (nvCali == NULL) {
            logE("cctCalibrationQuick2(): NULL pointer from nvReadFlashCaliDefault.");
            err = e_NvramError;
            goto FUNC_END;
        }

        int iGoldenDataDumpEn = 0;
        getPropInt(PROP_FLASH_GOLDEN_DATA_DUMP_EN, &iGoldenDataDumpEn, 0);
        if (iGoldenDataDumpEn)
            showFlashCali(nvCali);

        /* convert algo calibration data */
        for (i = 0; i < g_quickCalNum; i++) {
            for (j = 0; j < (int)g_CaliData1.size(); j++) {
                if (g_algCalData[i].duty == g_CaliData1[j].duty &&
                        g_algCalData[i].dutyLT == g_CaliData1[j].dutyLT) {
                    double ymax = -1;
                    double Vr = -1;
                    double Vg = -1;
                    double Vb = -1;

                    int k;
                    for (k = 0; k < CCT_FLASH_CALCULATE_FRAME; k++) {
                        if (ymax < g_CaliData1[j].y[k]) {
                            ymax = g_CaliData1[j].y[k];
                            Vr = g_CaliData1[j].r[k];
                            Vg = g_CaliData1[j].g[k];
                            Vb = g_CaliData1[j].b[k];
                        }
                    }

                    double fac = (double)g_CaliData1[0].exp * g_CaliData1[0].afe * g_CaliData1[0].isp /
                        g_CaliData1[j].exp / g_CaliData1[j].afe / g_CaliData1[j].isp;
                    logI("cctCalibrationQuick2(): fac(%lf) from (%d,%d,%d)/(%d,%d,%d).", fac,
                            g_CaliData1[0].exp, g_CaliData1[0].afe, g_CaliData1[0].isp,
                            g_CaliData1[j].exp , g_CaliData1[j].afe , g_CaliData1[j].isp);
                    g_algCalData[i].LumaY = ymax * fac;
                    g_algCalData[i].Rgain = Vg * 512 / Vr;
                    g_algCalData[i].Ggain = 512;
                    g_algCalData[i].Bgain = Vg * 512 / Vb;
                    g_algCalData[i].R = Vr * fac;
                    g_algCalData[i].G = Vg * fac;
                    g_algCalData[i].B = Vb * fac;
                }
            }
        }

#if CHECK_GAIN
        /* check unit gain variance */
        int iMaxRUintGainVariance;
        int iMaxBUintGainVariance;
        getPropInt(PROP_FLASH_MAX_R_UNIT_GAIN_VARIANCE, &iMaxRUintGainVariance, MAX_R_UNIT_GAIN_VARIANCE);
        getPropInt(PROP_FLASH_MAX_B_UNIT_GAIN_VARIANCE, &iMaxBUintGainVariance, MAX_B_UNIT_GAIN_VARIANCE);
        if (iMaxRUintGainVariance < 0) iMaxRUintGainVariance = 0;
        if (iMaxRUintGainVariance > 100) iMaxRUintGainVariance = 100;
        if (iMaxBUintGainVariance < 0) iMaxBUintGainVariance = 0;
        if (iMaxBUintGainVariance > 100) iMaxBUintGainVariance = 100;

        int isDual = cust_isDualFlashSupport(mFacingSensor);

        for (int i = 1; i < g_quickCalNum; i++)
        {
            FlashDuty::duty2ind(dutyIndex, g_algCalData[i].duty, g_algCalData[i].dutyLT, dutyNum, dutyNumLt, isDual);
            int iRUnitGain = g_algCalData[i].Rgain;
            int iBUnitGain = g_algCalData[i].Bgain;
            int iRGoldenGain = FlashCustomAdapter::getInstance(mSensorDev)->getWBRGainByDutyIndex(dutyIndex);
            int iBGoldenGain = FlashCustomAdapter::getInstance(mSensorDev)->getWBBGainByDutyIndex(dutyIndex);

            logI("cctCalibrationQuick2(): duty(%d), dutyLT(%d), R unit gain(%d), B unit gain(%d), R golden gain(%d), B golden gain(%d).",
                 g_algCalData[i].duty, g_algCalData[i].dutyLT,
                 iRUnitGain, iBUnitGain, iRGoldenGain, iBGoldenGain);
            if (iRUnitGain*100 > iRGoldenGain*(100+iMaxRUintGainVariance) || iRUnitGain*100 < iRGoldenGain*(100-iMaxRUintGainVariance))
            {
                logI("cctCalibrationQuick2(): R unit gain variance too large!");
                err = E_FLASH_CALI_UNIT_GAIN_VARIANCE_ERR;
            }
            if (iBUnitGain*100 > iBGoldenGain*(100+iMaxBUintGainVariance) || iBUnitGain*100 < iBGoldenGain*(100-iMaxBUintGainVariance))
            {
                logI("cctCalibrationQuick2(): B gain variance too large!");
                err = E_FLASH_CALI_UNIT_GAIN_VARIANCE_ERR;
            }
            if (err != S_FLASH_OK)
                goto FUNC_END;
        }
#endif
        /* get current table */
        short iTabHt[FLASH_CUSTOM_MAX_DUTY_NUM_HT];
        short iTabLt[FLASH_CUSTOM_MAX_DUTY_NUM_LT];
        memset(iTabHt, 0, sizeof(iTabHt));
        memset(iTabLt, 0, sizeof(iTabLt));
        FlashHal::getInstance(mSensorDev)->getCurrentTab(iTabHt, iTabLt);

        /* shift current table */
        short iTabHtShift[FLASH_CUSTOM_MAX_DUTY_NUM_HT];
        short iTabLtShift[FLASH_CUSTOM_MAX_DUTY_NUM_LT];
        memset(iTabHtShift, 0, sizeof(iTabHtShift));
        memset(iTabLtShift, 0, sizeof(iTabLtShift));
        iTabHtShift[0] = 0;
        for (i = 0; i < dutyNum; i++)
            iTabHtShift[i + 1] = iTabHt[i];
        if (cust_isDualFlashSupport(mFacingSensor)) {
            iTabLtShift[0] = 0;
            for (i = 0; i < dutyNumLt; i++) {
                iTabLtShift[i + 1] = iTabLt[i];
            }
        }

        /* clear energy table and AWB gain table */
        short *yTab = new short[FLASH_CUSTOM_MAX_DUTY_NUM];
        AWB_GAIN_T *awbTab = new AWB_GAIN_T[FLASH_CUSTOM_MAX_DUTY_NUM];
        for (i = 0; i < FLASH_CUSTOM_MAX_DUTY_NUM; i++) {
            awbTab[i].i4R = 512;
            awbTab[i].i4G = 512;
            awbTab[i].i4B = 512;
        }

        /* show interpolation input data */
        logI("cctCalibrationQuick2(): InterpolateCalData input:");
        logI("isDual = %d", (int)cust_isDualFlashSupport(mFacingSensor));
        logI("CalNum = %d", g_quickCalNum);
        logI("dutyNum = %d", dutyNum);
        logI("dutyNumLt = %d", dutyNumLt);
        logI("iTabHt:");
        for (i = 0; i < dutyNum + 1; i++)
            logI("[%d]: %d", i, iTabHtShift[i]);
        if (cust_isDualFlashSupport(mFacingSensor)) {
            logI("iTabLt:");
            for (i = 0; i < dutyNumLt + 1; i++)
                logI("[%d]: %d", i, iTabLtShift[i]);
        }
        logI("       duty, dutyLT,     LumaY,          R,          G,          B, Rgain, Ggain, Bgain");
        for (i = 0; i < g_quickCalNum; i++) // range check
            logI("[%4d] %4d, %6d, %7.2lf, %7.3lf, %7.3lf, %7.3lf, %5d, %5d, %5d",
                    i, g_algCalData[i].duty,
                    g_algCalData[i].dutyLT,
                    g_algCalData[i].LumaY,
                    g_algCalData[i].R, g_algCalData[i].G, g_algCalData[i].B,
                    g_algCalData[i].Rgain, g_algCalData[i].Ggain, g_algCalData[i].Bgain);
        logI("        eng,r gain,g gain,b gain");
        for (i = 0; i < dutyNum; i++)
            logI("[%4d] %4d %5d %5d %5d", i,
                    FlashCustomAdapter::getInstance(mSensorDev)->getYTabByDutyIndex(i),
                    FlashCustomAdapter::getInstance(mSensorDev)->getWBRGainByDutyIndex(i),
                    FlashCustomAdapter::getInstance(mSensorDev)->getWBGGainByDutyIndex(i),
                    FlashCustomAdapter::getInstance(mSensorDev)->getWBBGainByDutyIndex(i));

        int flashCaliNvIdx = FlashCustomAdapter::getInstance(mSensorDev)->getFlashCaliNvIdx();

        /* interpolate energy and AWB gain table */
        if (cust_isDualFlashSupport(mFacingSensor)) {
            err = FlashAlgoAdapter::getInstance(mSensorDev)->InterpolateCalData(
                    g_quickCalNum, iTabHtShift, iTabLtShift, g_algCalData, nvCali,
                    dutyNum, dutyNumLt, awbTab, yTab, 1, flashCaliNvIdx);

        } else {
            if (dutyNum != 2) {
                err = FlashAlgoAdapter::getInstance(mSensorDev)->InterpolateCalData(
                        g_quickCalNum, iTabHtShift, iTabLtShift, g_algCalData, nvCali,
                        dutyNum, dutyNumLt, awbTab, yTab, 0, flashCaliNvIdx);

            } else {
                logI("cctCalibrationQuick2(): Single flash with only 2 duty:");
                logI("              r,          g,          b");
                logI("[-1] %7.3lf, %7.3lf, %7.3lf", g_algCalData[0].R, g_algCalData[0].G, g_algCalData[0].B);
                logI("[0]  %7.3lf, %7.3lf, %7.3lf", g_algCalData[1].R, g_algCalData[1].G, g_algCalData[1].B);
                logI("[1]  %7.3lf, %7.3lf, %7.3lf", g_algCalData[2].R, g_algCalData[2].G, g_algCalData[2].B);

                yTab[0] = 9999 / (g_algCalData[2].G - g_algCalData[0].G) * (g_algCalData[1].G - g_algCalData[0].G);
                yTab[1] = 9999 / (g_algCalData[2].G - g_algCalData[0].G) * (g_algCalData[2].G - g_algCalData[0].G);

                double r1 = g_algCalData[1].R - g_algCalData[0].R;
                double g1 = g_algCalData[1].G - g_algCalData[0].G;
                double b1 = g_algCalData[1].B - g_algCalData[0].B;
                double r2 = g_algCalData[2].R - g_algCalData[0].R;
                double g2 = g_algCalData[2].G - g_algCalData[0].G;
                double b2 = g_algCalData[2].B - g_algCalData[0].B;
                awbTab[0].i4R = 512 * g1 / r1;
                awbTab[0].i4G = 512;
                awbTab[0].i4B = 512 * g1 / b1;
                awbTab[1].i4R = 512 * g2 / r2;
                awbTab[1].i4G = 512;
                awbTab[1].i4B = 512 * g2 / b2;
                err = S_FLASH_OK;
            }
        }

        if (err != S_FLASH_OK) {
            logI("cctCalibrationQuick2(): failed interpolate energy table and AWB gain.");
            delete [] yTab;
            delete [] awbTab;
            goto FUNC_END;
        }

        /* write to NVRAM */
        for (j = 0; j < FLASH_CUSTOM_MAX_DUTY_NUM; j++) {
            FlashCustomAdapter::getInstance(mSensorDev)->setYTabByDutyIndex(j, yTab[j]);
            FlashCustomAdapter::getInstance(mSensorDev)->setWBGainByDutyIndex(j,
                    awbTab[j].i4R, awbTab[j].i4G, awbTab[j].i4B);
        }
        FlashNvram::nvWriteFlashCali(mSensorDev);
        FlashCustomAdapter::getInstance(mSensorDev)->transformFlashCaliData();
        delete [] yTab;
        delete [] awbTab;

        /* dump to file */
        FlashCustomAdapter::getInstance(mSensorDev)->dumpNvFlashAe(CCT_FLASH_PATH_ENG);
        FlashCustomAdapter::getInstance(mSensorDev)->dumpNvFlashAwb(CCT_FLASH_PATH_FWB, FLASH_CUSTOM_MAX_DUTY_NUM);

        logI("cctCalibrationQuick2(): dump xml(%ld) +.", getMs());
        writeFlashCali(CCT_FLASH_PATH_CALI_QUICK2, nvCali);
        writeEngTabXml(CCT_FLASH_PATH_ENG_XML, nvCali,
                mSensorDev, FlashHal::getInstance(mSensorDev)->getPartId(),
                dutyNum, dutyNumLt);
        writeFwbGainXml(CCT_FLASH_PATH_FWB_XML, nvCali,
                mSensorDev, FlashHal::getInstance(mSensorDev)->getPartId(),
                dutyNum, dutyNumLt);
        logI("cctCalibrationQuick2(): dump xml(%ld) -.", getMs());

        /* dump calibration result */
        dumpCalibrationResult(CCT_FLASH_PATH_CALIBRAION_RESULT, mQuickCalibrationResult);

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

        /* clear exposure level array */
        g_expArr.clear();
        g_afeArr.clear();
        g_ispArr.clear();

        /* update flash info */
        rep->isEnd = 1;
        mIsCalibration = 0;

        /* uninit strobe */
        FlashHal::getInstance(mSensorDev)->setInCharge(inCharge);
        if(m_bCctFlashHalInit)
        {
            FlashHal::getInstance(mSensorDev)->uninit();
            m_bCctFlashHalInit = false;
        }

        /* update status */
        state = CCT_FLASH_STATE_END2;
    }
FUNC_END:
    if (err) {
        logI("cctCalibrationQuick2(): calibration failed(%d).", err);

        FlashPlatformAdapter::getInstance(mSensorDev)->restoreAwbModuleVariationGain(&awbPreGain1Prev);
        mLinearOutputEn = FlashPlatformAdapter::getInstance(mSensorDev)->getAwbLinearOutputEn();
        FlashPlatformAdapter::getInstance(mSensorDev)->setAeCCUOnOff(MTRUE);

        /* update flash info */
        rep->isEnd = 1;
        mIsCalibration = 0;
        mQuickCalibrationResult = err;
        mCapRep.isMainFlash = 0;

        /* uninit strobe */
        FlashHal::getInstance(mSensorDev)->setInCharge(inCharge);
        if(m_bCctFlashHalInit)
        {
            FlashHal::getInstance(mSensorDev)->uninit();
            m_bCctFlashHalInit = false;
        }

        /* dump calibration result */
        dumpCalibrationResult(CCT_FLASH_PATH_CALIBRAION_RESULT, mQuickCalibrationResult);

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

