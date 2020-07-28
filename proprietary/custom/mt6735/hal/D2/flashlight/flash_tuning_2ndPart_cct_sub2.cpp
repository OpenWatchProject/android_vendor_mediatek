#define LOG_TAG "flash_custom_2ndpart_cct_sub2.cpp"
#include "string.h"
#include "camera_custom_nvram.h"
#include "camera_custom_types.h"
#include "camera_custom_AEPlinetable.h"
#include <cutils/log.h>
#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>

//==============================================================================
//
//==============================================================================
int cust_fillDefaultStrobeNVRam_sub2 (void* data)
{
	int i;
	NVRAM_CAMERA_STROBE_STRUCT* p;
	p = (NVRAM_CAMERA_STROBE_STRUCT*)data;

	static short engTab[]=
    { 892,1284,1653,2338,2966,3564,4103,4573,4972,5260};

	//version
	p->u4Version = NVRAM_CAMERA_STROBE_FILE_VERSION;
	//eng tab
	memcpy(p->engTab.yTab, engTab, sizeof(engTab));

	//tuningPara[8];
	for(i=0;i<8;i++)
    {
        p->tuningPara[i].yTarget = 188;
        p->tuningPara[i].fgWIncreaseLevelbySize = 10;
        p->tuningPara[i].fgWIncreaseLevelbyRef = 0;
        p->tuningPara[i].ambientRefAccuracyRatio = 5;
        p->tuningPara[i].flashRefAccuracyRatio = 1;

        p->tuningPara[i].backlightAccuracyRatio = 18;
        p->tuningPara[i].backlightUnderY = 40;
        p->tuningPara[i].backlightWeakRefRatio = 32;
        p->tuningPara[i].safetyExp = 33322;
        p->tuningPara[i].maxUsableISO = 680;
        p->tuningPara[i].yTargetWeight = 0;
        p->tuningPara[i].lowReflectanceThreshold = 13;

        p->tuningPara[i].flashReflectanceWeight = 0;
        p->tuningPara[i].bgSuppressMaxDecreaseEV = 20;
        p->tuningPara[i].bgSuppressMaxOverExpRatio = 6;
        p->tuningPara[i].fgEnhanceMaxIncreaseEV = 50;
        p->tuningPara[i].fgEnhanceMaxOverExpRatio = 6;

        p->tuningPara[i].isFollowCapPline = 1;
        p->tuningPara[i].histStretchMaxFgYTarget = 266;
        p->tuningPara[i].histStretchBrightestYTarget = 328;
        p->tuningPara[i].fgSizeShiftRatio = 0;

        p->tuningPara[i].backlitPreflashTriggerLV = 90;
        p->tuningPara[i].backlitMinYTarget = 100;
    }
	p->tuningPara[0].isFollowCapPline = 0;

    p->paraIdxForceOn[0] =1;    //default
    p->paraIdxForceOn[1] =0;    //LIB3A_AE_SCENE_OFF
    p->paraIdxForceOn[2] =0;    //LIB3A_AE_SCENE_AUTO
    p->paraIdxForceOn[3] =1;    //LIB3A_AE_SCENE_NIGHT
    p->paraIdxForceOn[4] =1;    //LIB3A_AE_SCENE_ACTION
    p->paraIdxForceOn[5] =1;    //LIB3A_AE_SCENE_BEACH
    p->paraIdxForceOn[6] =1;    //LIB3A_AE_SCENE_CANDLELIGHT
    p->paraIdxForceOn[7] =1;    //LIB3A_AE_SCENE_FIREWORKS
    p->paraIdxForceOn[8] =1;    //LIB3A_AE_SCENE_LANDSCAPE
    p->paraIdxForceOn[9] =1;    //LIB3A_AE_SCENE_PORTRAIT
    p->paraIdxForceOn[10] =1;   //LIB3A_AE_SCENE_NIGHT_PORTRAIT
    p->paraIdxForceOn[11] =1;   //LIB3A_AE_SCENE_PARTY
    p->paraIdxForceOn[12] =1;   //LIB3A_AE_SCENE_SNOW
    p->paraIdxForceOn[13] =1;   //LIB3A_AE_SCENE_SPORTS
    p->paraIdxForceOn[14] =1;   //LIB3A_AE_SCENE_STEADYPHOTO
    p->paraIdxForceOn[15] =1;   //LIB3A_AE_SCENE_SUNSET
    p->paraIdxForceOn[16] =1;   //LIB3A_AE_SCENE_THEATRE
    p->paraIdxForceOn[17] =1;   //LIB3A_AE_SCENE_ISO_ANTI_SHAKE
    p->paraIdxForceOn[18] =1;   //LIB3A_AE_SCENE_BACKLIGHT

    p->paraIdxAuto[0] =1;  //default
    p->paraIdxAuto[1] =0;  //LIB3A_AE_SCENE_OFF
    p->paraIdxAuto[2] =0;  //LIB3A_AE_SCENE_AUTO
    p->paraIdxAuto[3] =1;  //LIB3A_AE_SCENE_NIGHT
    p->paraIdxAuto[4] =1;  //LIB3A_AE_SCENE_ACTION
    p->paraIdxAuto[5] =1;  //LIB3A_AE_SCENE_BEACH
    p->paraIdxAuto[6] =1;  //LIB3A_AE_SCENE_CANDLELIGHT
    p->paraIdxAuto[7] =1;  //LIB3A_AE_SCENE_FIREWORKS
    p->paraIdxAuto[8] =1;  //LIB3A_AE_SCENE_LANDSCAPE
    p->paraIdxAuto[9] =1;  //LIB3A_AE_SCENE_PORTRAIT
    p->paraIdxAuto[10] =1; //LIB3A_AE_SCENE_NIGHT_PORTRAIT
    p->paraIdxAuto[11] =1; //LIB3A_AE_SCENE_PARTY
    p->paraIdxAuto[12] =1; //LIB3A_AE_SCENE_SNOW
    p->paraIdxAuto[13] =1; //LIB3A_AE_SCENE_SPORTS
    p->paraIdxAuto[14] =1; //LIB3A_AE_SCENE_STEADYPHOTO
    p->paraIdxAuto[15] =1; //LIB3A_AE_SCENE_SUNSET
    p->paraIdxAuto[16] =1; //LIB3A_AE_SCENE_THEATRE
    p->paraIdxAuto[17] =1; //LIB3A_AE_SCENE_ISO_ANTI_SHAKE
    p->paraIdxAuto[18] =1; //LIB3A_AE_SCENE_BACKLIGHT



	//--------------------
	//eng level
	//index mode
	//torch
	p->engLevel.torchDuty = 1;
	//af
	p->engLevel.afDuty = 1;
	//pf, mf, normal
	p->engLevel.pfDuty = 1;
	p->engLevel.mfDutyMax = 6;
	p->engLevel.mfDutyMin = 0;


	//low bat
	p->engLevel.IChangeByVBatEn=0;
	p->engLevel.vBatL = 3600;	//mv
	p->engLevel.pfDutyL = 1;
	p->engLevel.mfDutyMaxL = 2;
	p->engLevel.mfDutyMinL = 0;


	//burst setting
	p->engLevel.IChangeByBurstEn=1;
	p->engLevel.pfDutyB = 1;
	p->engLevel.mfDutyMaxB = 3;
	p->engLevel.mfDutyMinB = 0;
	//high current setting
	p->engLevel.decSysIAtHighEn = 1;
	p->engLevel.dutyH = 8;

	return 0;
}

