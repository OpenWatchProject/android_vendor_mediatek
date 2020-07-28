#define LOG_TAG "FlashCaliUtils"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "flash_cali_xml.h"

#ifdef LINUX
#include "flash_cali_tools.h"
#else
#include "camera_custom_nvram.h"
#include "kd_camera_feature.h"
#include "flash_utils.h"
#endif


#define SENSOR_STR_MAIN "main"
#define SENSOR_STR_MAIN2 "main2"
#define SENSOR_STR_MAIN3 "main3"
#define SENSOR_STR_SUB "sub"
#define SENSOR_STR_SUB2 "sub2"
const char *getSensorStr(int sensorDev)
{
	if (sensorDev == DUAL_CAMERA_MAIN_SENSOR)
		return SENSOR_STR_MAIN;
	else if (sensorDev == DUAL_CAMERA_SUB_SENSOR)
		return SENSOR_STR_SUB;
	else if (sensorDev == DUAL_CAMERA_MAIN_2_SENSOR)
		return SENSOR_STR_MAIN2;
	else if (sensorDev == DUAL_CAMERA_MAIN_3_SENSOR)
		return SENSOR_STR_MAIN3;
	else // if (sensorDev == DUAL_CAMERA_SUB_2_SENSOR)
		return SENSOR_STR_SUB2;
}

int writeXml(const char *fname, char *xml, int size)
{
	/* verify arguments */
	if (!fname || !xml) {
		logE("%s(): invalid arguments.", __FUNCTION__);
		return -1;
	}

	/* set creation mask */
    int mask = umask(0);

	/* write data */
    FILE *fp = fopen(fname, "wb");
    if (!fp) {
        logE("%s(): failed to open.", __FUNCTION__);
        umask(mask);
        return -1;
    }
    fwrite(xml, 1, size, fp);
    fclose(fp);

	/* restore mask */
    umask(mask);

	return 0;
}

int writeCaliAeXml(const char *fname, int sensorDev, int part, int exp, int afe, int isp)
{
	/* verify arguments */
	if (!fname) {
		logE("%s(): invalid arguments.", __FUNCTION__);
		return -1;
	}

    logI("%s().", __FUNCTION__);

	char xml[256];
	snprintf(xml, sizeof(xml), XML_HEADER CALI_AE_XML,
			getSensorStr(sensorDev), part, exp, afe, isp);
	//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
	writeXml(fname, xml, strlen(xml));

    return 0;
}

int writeEngTabXml(const char *fname,
		PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash,
		int sensorDev, int part, int dutyNum, int dutyNumLt)
{
	/* verify arguments */
	if (!fname || !nvFlash) {
		logE("%s(): invalid arguments.", __FUNCTION__);
		return -1;
	}

    logI("%s().", __FUNCTION__);

	/* duty index include off status if dual */
	int dutyIndex = dutyNum;
	int dutyLtIndex = dutyNumLt;
	if (dutyNumLt) {
		dutyIndex++;
		dutyLtIndex++;
	}

	/* set creation mask */
    int mask = umask(0);

	/* write data */
    FILE *fp = fopen(fname, "wb");
    if (!fp) {
        logE("%s(): failed to open.", __FUNCTION__);
        umask(mask);
        return -1;
    }

	/* XML header and table */
	int i, j;
	char xml[128];
	snprintf(xml, sizeof(xml),
			XML_HEADER ENG_TAB_XML_engTabTable);
	//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
	fwrite(xml, 1, strlen(xml), fp);
	for (j = 0; j < dutyLtIndex; j++) {
		/* XML item */
		snprintf(xml, sizeof(xml), ENG_TAB_XML_engTab,
				getSensorStr(sensorDev), part, j + 1);
		//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
		fwrite(xml, 1, strlen(xml), fp);
		for (i = 0; i < dutyIndex; i++) {
			/* XML each dutyHt */
#if (!CAM3_3A_ISP_30_EN && !CAM3_3A_ISP_40_EN)
			snprintf(xml, sizeof(xml), ENG_TAB_XML_dutyHt,
					i + 1, nvFlash->Flash_Calibration[0].engTab.yTab[i + j * dutyLtIndex], i + 1);
#else
			snprintf(xml, sizeof(xml), ENG_TAB_XML_dutyHt,
					i + 1, nvFlash->yTab[i + j * dutyLtIndex], i + 1);
#endif
			//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
			fwrite(xml, 1, strlen(xml), fp);
		}
		snprintf(xml, sizeof(xml), ENG_TAB_XML_engTab_);
		//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
		fwrite(xml, 1, strlen(xml), fp);
	}
	snprintf(xml, sizeof(xml), ENG_TAB_XML_engTabTable_);
	//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
	fwrite(xml, 1, strlen(xml), fp);
	fclose(fp);

	/* restore mask */
    umask(mask);
    return 0;
}

int writeFwbGainXml(const char *fname,
		PNVRAM_CAMERA_FLASH_CALIBRATION_STRUCT nvFlash,
		int sensorDev, int part, int dutyNum, int dutyNumLt)
{
	/* verify arguments */
	if (!fname || !nvFlash) {
		logE("%s(): invalid arguments.", __FUNCTION__);
		return -1;
	}

    logI("%s().", __FUNCTION__);

	/* duty index include off status if dual */
	int dutyIndex = dutyNum;
	int dutyLtIndex = dutyNumLt;
	if (dutyNumLt) {
		dutyIndex++;
		dutyLtIndex++;
	}

	/* set creation mask */
    int mask = umask(0);

	/* write data */
    FILE *fp = fopen(fname, "wb");
    if (!fp) {
        logE("%s(): failed to open.", __FUNCTION__);
        umask(mask);
        return -1;
    }

	/* XML header and table */
	int i, j;
	char xml[256];
	snprintf(xml, sizeof(xml),
			XML_HEADER FWB_GAIN_XML_fwbGainTable);
	//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
	fwrite(xml, 1, strlen(xml), fp);
	for (j = 0; j < dutyLtIndex; j++) {
		for (i = 0; i < dutyIndex; i++) {
			/* XML item */
#if (!CAM3_3A_ISP_30_EN && !CAM3_3A_ISP_40_EN && !CAM3_3A_ISP_50_EN)
			snprintf(xml, sizeof(xml), FWB_GAIN_XML_fwbGain,
					getSensorStr(sensorDev), part, i + 1, j + 1,
					nvFlash->Flash_Calibration[0].flashWBGain[i + j * dutyLtIndex].i2R,
					512,
					nvFlash->Flash_Calibration[0].flashWBGain[i + j * dutyLtIndex].i2B);
#elif (!CAM3_3A_ISP_30_EN && !CAM3_3A_ISP_40_EN)
			snprintf(xml, sizeof(xml), FWB_GAIN_XML_fwbGain,
					getSensorStr(sensorDev), part, i + 1, j + 1,
					nvFlash->Flash_Calibration[0].flashWBGain[i + j * dutyLtIndex].i4R,
					nvFlash->Flash_Calibration[0].flashWBGain[i + j * dutyLtIndex].i4G,
					nvFlash->Flash_Calibration[0].flashWBGain[i + j * dutyLtIndex].i4B);
#else
			snprintf(xml, sizeof(xml), FWB_GAIN_XML_fwbGain,
					getSensorStr(sensorDev), part, i + 1, j + 1,
					nvFlash->flashWBGain[i + j * dutyLtIndex].i4R,
					nvFlash->flashWBGain[i + j * dutyLtIndex].i4G,
					nvFlash->flashWBGain[i + j * dutyLtIndex].i4B);
#endif
			//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
			fwrite(xml, 1, strlen(xml), fp);
		}
	}
	snprintf(xml, sizeof(xml), FWB_GAIN_XML_fwbGainTable_);
	//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
	fwrite(xml, 1, strlen(xml), fp);
	fclose(fp);

	/* restore mask */
    umask(mask);
    return 0;
}

int writeEngAllXml(const char *fname,
		dqCaliData &caliData1, dqCaliData &caliData2, dqCaliData &caliData3,
		int sensorDev, int part)
{
	/* verify arguments */
	if (!fname) {
		logE("%s(): invalid arguments.", __FUNCTION__);
		return -1;
	}

    logI("%s().", __FUNCTION__);

	/* set creation mask */
    int mask = umask(0);

	/* write data */
    FILE *fp = fopen(fname, "wb");
    if (!fp) {
        logE("%s(): failed to open.", __FUNCTION__);
        umask(mask);
        return -1;
    }

	/* XML header and table */
	int i, j;
	char xml[320];
	snprintf(xml, sizeof(xml),
			XML_HEADER ENG_ALL_XML_engAllTable);
	//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
	fwrite(xml, 1, strlen(xml), fp);

	/* XML deque 1 */
	for (j = 0; j < (int)caliData1.size(); j++)
		for (i = 0; i < CCT_FLASH_CALCULATE_FRAME; i++) {
			snprintf(xml, sizeof(xml), ENG_ALL_XML_engAll,
					getSensorStr(sensorDev), part, 1,
					caliData1[j].t[i], caliData1[j].exp,
					caliData1[j].afe, caliData1[j].isp,
					caliData1[j].duty, caliData1[j].dutyLT,
					caliData1[j].y[i], caliData1[j].r[i],
					caliData1[j].g[i], caliData1[j].b[i]);
			//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
			fwrite(xml, 1, strlen(xml), fp);
		}

	/* XML deque 2 */
	for (j = 0; j < (int)caliData2.size(); j++)
		for (i = 0; i < CCT_FLASH_CALCULATE_FRAME; i++) {
			snprintf(xml, sizeof(xml), ENG_ALL_XML_engAll,
					getSensorStr(sensorDev), part, 2,
					caliData2[j].t[i], caliData2[j].exp,
					caliData2[j].afe, caliData2[j].isp,
					caliData2[j].duty, caliData2[j].dutyLT,
					caliData2[j].y[i], caliData2[j].r[i],
					caliData2[j].g[i], caliData2[j].b[i]);
			//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
			fwrite(xml, 1, strlen(xml), fp);
		}

	/* XML deque 3 */
	for (j = 0; j < (int)caliData3.size(); j++)
		for (i = 0; i < CCT_FLASH_CALCULATE_FRAME; i++) {
			snprintf(xml, sizeof(xml), ENG_ALL_XML_engAll,
					getSensorStr(sensorDev), part, 3,
					caliData3[j].t[i], caliData3[j].exp,
					caliData3[j].afe, caliData3[j].isp,
					caliData3[j].duty, caliData3[j].dutyLT,
					caliData3[j].y[i], caliData3[j].r[i],
					caliData3[j].g[i], caliData3[j].b[i]);
			//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
			fwrite(xml, 1, strlen(xml), fp);
		}
	snprintf(xml, sizeof(xml), ENG_ALL_XML_engAllTable_);
	//logI("length(%d): ==========\n%s", (int)strlen(xml), xml);
	fwrite(xml, 1, strlen(xml), fp);
	fclose(fp);

	/* restore mask */
	umask(mask);

    return 0;
}
