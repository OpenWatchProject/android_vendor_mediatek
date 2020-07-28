/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "libPowerHal"

#include <utils/Log.h>
#include <utils/RefBase.h>
#include <dlfcn.h>
#include <hidl/HidlSupport.h>
#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <expat.h>

#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>
#include <android/hardware/power/1.3/IPower.h>

#include "common.h"
#include "perfservice.h"
#include "perfservice_dec.h"
#include "mtkpower_hint.h"
#include "mtkperf_resource.h"
#include "mtkpower_types.h"


#include <power_cmd_types.h>

#include "tinyxml2.h"
using namespace tinyxml2;

using std::string;
using android::hardware::hidl_string;
using namespace vendor::mediatek::hardware::power::V2_0;
using ::android::hardware::power::V1_3::PowerHint;


/* Definition */
#define STATE_ON 1
#define STATE_OFF 0

#define PACK_NAME_MAX   128
#define CLASS_NAME_MAX  128

#define APP_LIST_XMLPATH        "/vendor/etc/power_app_cfg.xml"
#define APP_LIST_XMLPATH_2      "/data/vendor/powerhal/power_app_cfg.xml"
#define PACK_LIST_XMLPATH       "/vendor/etc/power_whitelist_cfg.xml"
//#define PACK_LIST_XMLPATH_2     "/data/system/power_whitelist_cfg.xml"
#define PACK_LIST_XMLPATH_2     "/data/vendor/powerhal/power_whitelist_cfg.xml"

#define CUS_SCN_TABLE           "/vendor/etc/powerscntbl.xml"
#define CUS_CONFIG_TABLE        "/vendor/etc/powercontable.xml"
#define CUS_CONFIG_TABLE_T      "/vendor/etc/powercontable_t.xml"
#define COMM_NAME_SIZE  64
#define PATH_LENGTH     128

#define REG_SCN_MAX     256   // user scenario max number
#define DEFAULT_HTASK_THRSHOLD (1000)
#define HTASK_THRESHOLD_MAX    (1023)

#define XMLPARSE_GET_ACTIVITYNUM      1
#define XMLPARSE_GET_ACTIVITYLIST     2

#if defined (__LP64__) ||  defined (_LP64)
#define THM_LIB_FULL_NAME  "/vendor/lib64/libmtcloader.so"
#else
#define THM_LIB_FULL_NAME  "/vendor/lib/libmtcloader.so"
#endif

#define MAX_CONF_NAME   (18)
#define MAX_FILE_PATH   (MAX_CONF_NAME+17)
#define MAX_POLICY      (23) /* 3 for PowerHAL and 20 for specific scenarios. */


#ifdef max
#undef max
#endif
#define max(a,b) (((a) > (b)) ? (a) : (b))

#ifdef min
#undef min
#endif
#define min(a,b) (((a) < (b)) ? (a) : (b))

static xml_activity  *ptXmlActList = NULL;
static int        SCN_APP_RUN_BASE = (int)MtkPowerHint::MTK_POWER_HINT_NUM + REG_SCN_MAX;

static int        xmlPerService_flag = 0;
static int        nXmlPackNum = 0;
static int        nXmlActNum = 0;
static int        nXmlCmdNum = 0;

static int        nXmlCmdIndex = 0;
static int        nXmlCommonIndex = 0;
static char       cXmlPack[PACK_NAME_MAX];
static char       cXmlAct[CLASS_NAME_MAX];

const string LESS("less");

tScnConTable tConTable[FIELD_SIZE];

void checkConTable(void) {
    ALOGI("Cmd name, Cmd ID, Entry, default value, current value, compare, max value, min value, isValid, normal value, sport value");
    for(int idx = 0; idx < FIELD_SIZE; idx++) {
        if(tConTable[idx].cmdName.length() == 0)
            continue;
        ALOGI("%s, %d, %s, %d, %d, %s, %d, %d %d %d %d", tConTable[idx].cmdName.c_str(),  tConTable[idx].cmdID, tConTable[idx].entry.c_str(), tConTable[idx].defaultVal,
                tConTable[idx].curVal, tConTable[idx].comp.c_str(), tConTable[idx].maxVal, tConTable[idx].minVal, tConTable[idx].isValid, tConTable[idx].normalVal, tConTable[idx].sportVal);
    }
}

/* Function */
void xmlparser_start(void *userData, const char *name, const char *arg[])
{
    int i, len;
    int *xmlaction = (int *)userData;

    if(*xmlaction == XMLPARSE_GET_ACTIVITYNUM)
    {
        if (!strcmp(name, "PerfService")) {
            xmlPerService_flag = 1;
        }

        if((xmlPerService_flag == 1) && !strcmp(name, "Package")){
            nXmlPackNum++;
        }

        if((xmlPerService_flag == 1) && !strcmp(name, "Activity")){
            nXmlActNum++;
        }

        if((xmlPerService_flag == 1) && !strncmp(name, "PERF_RES", 8)){
            nXmlCmdNum++;
        }
    }
    else
    if(*xmlaction == XMLPARSE_GET_ACTIVITYLIST) {
        if (!strcmp(name, "PerfService")) {
            xmlPerService_flag = 1;
        }

        if((xmlPerService_flag == 1) && !strcmp(name, "Package")){
            memset(cXmlPack, 0, PACK_NAME_MAX);
            len = strlen(arg[1]);
            if(len >= PACK_NAME_MAX)
                len = PACK_NAME_MAX-1;
            strncpy(cXmlPack, arg[1], len);
        }

        if((xmlPerService_flag == 1) && !strcmp(name, "Activity")){
            if(!strcmp(arg[1], "Common")){
                sprintf(cXmlAct, "Common_%s", cXmlPack);
                nXmlCommonIndex++;
            }
            else{
                memset(cXmlAct, 0, CLASS_NAME_MAX);
                len = strlen(arg[1]);
                if(len >= CLASS_NAME_MAX)
                len = CLASS_NAME_MAX-1;
                strncpy(cXmlAct, arg[1], len);
            }
        }

        if((xmlPerService_flag == 1) && !strncmp(name, "PERF_RES", 8)){
            memset(ptXmlActList[nXmlCmdIndex].cmd, 0, 128);
            len = strlen(name);
            if(len>=128)
               len=127;
            strncpy(ptXmlActList[nXmlCmdIndex].cmd, name,len);

            memset(ptXmlActList[nXmlCmdIndex].packName, 0, 128);
            len = strlen(cXmlPack);
            if(len>=128)
                len=127;
            strncpy(ptXmlActList[nXmlCmdIndex].packName, cXmlPack, len);

            memset(ptXmlActList[nXmlCmdIndex].actName, 0, 128);
            len = strlen(cXmlAct);
            if(len>=128)
                len=127;
            strncpy(ptXmlActList[nXmlCmdIndex].actName, cXmlAct, len);
            for(i=0;arg[i]!=0;i+=2) {
                if(i==0)
                    ptXmlActList[nXmlCmdIndex].param1 = atoi(arg[1]);
                if(i==2)
                    ptXmlActList[nXmlCmdIndex].param2 = atoi(arg[3]);
                if(i==4)
                    ptXmlActList[nXmlCmdIndex].param3 = atoi(arg[5]);
                if(i==6)
                    ptXmlActList[nXmlCmdIndex].param4 = atoi(arg[7]);
            }
            ALOGD("[xmlparser_start] XMLPARSE_GET_ACTIVITYLIST CmdIndex:%d cmd:%s pack:%s, activity:%s p1:%d p2:%d p3:%d p4:%d",nXmlCmdIndex,
            ptXmlActList[nXmlCmdIndex].cmd,
            ptXmlActList[nXmlCmdIndex].packName,
            ptXmlActList[nXmlCmdIndex].actName,
            ptXmlActList[nXmlCmdIndex].param1,
            ptXmlActList[nXmlCmdIndex].param2,
            ptXmlActList[nXmlCmdIndex].param3,
            ptXmlActList[nXmlCmdIndex].param4
            );
            nXmlCmdIndex++;
        }
    }
}

void xmlparser_end(void *userData, const char *name)
{
    ALOGV("[xmlparser_end] userData:%p", userData);
    if (!strcmp(name, "PerfService")) {
        xmlPerService_flag = 0;
    }
}

void updateScnListfromXML(tScnNode *pPackList)
{
    char **act_name;
    int   i = 0, j = 0, num = 0, lines = 0;

    lines += nXmlCmdNum;

    if(lines > 0) {
        act_name = (char **) malloc(lines * sizeof(char*));
        if(act_name == NULL)
            return;
    }
    else
        return;

    for (i = 0; i < lines; i++)
        act_name[i] = (char *) malloc(CLASS_NAME_MAX * sizeof(char));

    //ALOGI("[updateScnListfromXML] new activity from perf_app, num:%d", num);

    for(j = 0; j < nXmlCmdNum; j++){
        if(strncmp(ptXmlActList[j].cmd, "PERF_RES", 8) == 0){
            for(i = 0; i < num && strcmp(ptXmlActList[j].actName, act_name[i]) != 0; i++);
            if(i == num) { // new pack
                set_str_cpy(act_name[i], ptXmlActList[j].actName, CLASS_NAME_MAX);
                pPackList[i].scn_type     = SCN_APP_RUN_BASE;
                pPackList[i].scn_state    = STATE_OFF;
                pPackList[i].pack_name[0] = '\0';
                pPackList[i].act_name[0]  = '\0';
                pPackList[i].screen_off_action = MTKPOWER_SCREEN_OFF_WAIT_RESTORE;
                //pPackList[i].scn_valid    = (mode == PERF_MODE_NORMAL) ? 1 : 0;  // only normal mode app are default valid
                pPackList[i].scn_valid    = 1;
                set_str_cpy(pPackList[i].pack_name, ptXmlActList[j].packName, PACK_NAME_MAX);
                set_str_cpy(pPackList[i].act_name, ptXmlActList[j].actName, CLASS_NAME_MAX);
            }

            // normal mode table
            cmdSetting(-1, ptXmlActList[j].cmd, &pPackList[i], ptXmlActList[j].param1);
            if (i == num)
                num++;
        }
        else{
            continue;
        }
    }

    ALOGI("[updateScnListfromXML] new activity from perf_app and whiteList, num:%d", num);

    for (i = 0; i < lines; i++) {
        free(act_name[i]);
        act_name[i] = NULL;
    }
    free(act_name);
    act_name = NULL;
}


void get_activity_num_XML(const char *path)
{
    int len = 0, xmlaction = XMLPARSE_GET_ACTIVITYNUM;
    bool done;
    char val[512];
    FILE *fh = NULL;

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, (void *)&xmlaction);
    XML_SetElementHandler(parser, xmlparser_start, xmlparser_end);
    //nXmlPackNum = nXmlActNum = nXmlCmdNum = 0;
    xmlPerService_flag = 0;

    fh = fopen(path, "r");

    if (fh){
        len = fread(val, 1, 512, fh);
        while(len)
        {
            done = feof(fh) ? true : false;
            if(0 == XML_Parse(parser, val, len, done))
            {
                int code = XML_GetErrorCode(parser);
                const char *msg = (const char *)XML_ErrorString((XML_Error)code);
                ALOGI("[get_activity_num_XML] Parsing error code %d message \"%s\"\n",code, msg);
                break;
            }
            if (done)
                break;
            len = fread(val, 1, 512, fh);
        }

        XML_ParserFree(parser);
        fclose(fh);
    }
    else{
        ALOGI("[get_activity_num_XML] whitelist_cfg.xml does not exist");
    }

}

void get_activity_list_XML(const char *path)
{
    int len = 0, xmlaction = XMLPARSE_GET_ACTIVITYLIST;
    bool done;
    char val[512];
    FILE *fh = NULL;

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, (void *)&xmlaction);
    XML_SetElementHandler(parser, xmlparser_start, xmlparser_end);
    xmlPerService_flag = 0;
    nXmlCommonIndex = 0;
    //nXmlCmdIndex = 0;

    fh = fopen(path, "r");
    if (fh)
    {
        len = fread(val, 1, 512, fh);
        while(len)
        {
            done = feof(fh) ? true : false;
            if(0 == XML_Parse(parser, val, len, done))
            {
                int code = XML_GetErrorCode(parser);
                const char *msg = (const char *)XML_ErrorString((XML_Error)code);
                ALOGI("[get_activity_list_XML] Parsing error code %d message \"%s\"\n",code, msg);
                break;
            }
            if (done)
                break;
            len = fread(val, 1, 512, fh);
        }

        XML_ParserFree(parser);
        fclose(fh);
    }
    else {
        ALOGI("[get_activity_list_XML] whitelist_cfg.xml does not exist");
    }
}

int get_activity_totalnum(void)
{
    char **act_name;
    int num = 0, lines = 0, i, j;

    lines += nXmlCmdNum;

    if(lines > 0) {
        act_name = (char **) malloc(lines * sizeof(char*));
        if(act_name == NULL)
            return 0;
    }
    else
        return 0;

    for (i = 0; i < lines; i++)
        act_name[i] = (char *) malloc(CLASS_NAME_MAX * sizeof(char));

    for(j = 0; j < nXmlCmdNum; j++) {
        if(strncmp(ptXmlActList[j].cmd, "CMD_SET", 7) == 0){
            for(i = 0; i < num && strcmp(ptXmlActList[j].actName, act_name[i]) != 0; i++);

            if(i == num) { // new pack
                num++;
                set_str_cpy(act_name[i], ptXmlActList[j].actName, CLASS_NAME_MAX);
            }
        }
        else{
            if(!strncmp(ptXmlActList[j].cmd, "[SPORTS]", 8))
                continue;

            for(i = 0; i < num && strcmp(ptXmlActList[j].actName, act_name[i]) != 0; i++);

            if (i == num){ //new pack
                num++;
                set_str_cpy(act_name[i], ptXmlActList[j].actName, CLASS_NAME_MAX);
            }
        }
    }
    ALOGI("[get_activity_totalnum]  total ture activity num :%d\n", num);

    for (i = 0; i < lines; i++) {
        free(act_name[i]);
        act_name[i] = NULL;
    }
    free(act_name);
    act_name = NULL;

    return num;
}

void perfxml_read_cmddata(XMLElement *elmtScenario, int scn)
{
    XMLElement *dataelmt = elmtScenario->FirstChildElement("data");
    int param_1 = 0;
    const char* str;
    char  cmd[64];

    while(dataelmt){
        str = dataelmt->Attribute("cmd");
        param_1 = dataelmt->IntAttribute("param1");
        ALOGD("[updateCusScnTable] cmd:%s, scn:%d, param_1:%d",
        str, scn, param_1);
        if(strlen(str) < 64)
            set_str_cpy(cmd, str, 64);
        Scn_cmdSetting(cmd, scn, param_1);
        dataelmt = dataelmt->NextSiblingElement();
    }
}

int updateCusScnTable(const char *path)
{
    XMLDocument docXml;
    XMLError errXml = docXml.LoadFile(path);
    int scn = MTKPOWER_HINT_NUM;

    ALOGI("[updateCusScnTable]");

    if (errXml != XML_SUCCESS) {
        ALOGE("%s: Unable to powerhal CusScnTable config file '%s'. Error: %s",
            __FUNCTION__, path, XMLDocument::ErrorIDToName(errXml));
        return 0;
    } else {
        ALOGI("%s: load powerhal CusScnTable config succeed!", __FUNCTION__);
    }

    ALOGI("[updateCusScnTable] errXml:%d" , errXml);

    XMLElement* elmtRoot = docXml.RootElement();
    XMLElement *elmtScenario = elmtRoot->FirstChildElement("scenario");

    while (elmtScenario != NULL) {
        if(!strcmp("MTKPOWER_HINT_PROCESS_CREATE", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_PROCESS_CREATE;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_PACK_SWITCH", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_PACK_SWITCH;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_ACT_SWITCH", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_ACT_SWITCH;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_GAME_LAUNCH", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_GAME_LAUNCH;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_APP_ROTATE", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_APP_ROTATE;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_APP_TOUCH", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_APP_TOUCH;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        //} else if(!strcmp("MTKPOWER_HINT_FRAME_UPDATE", (elmtScenario->Attribute("powerhint")))) {
        //    scn = MTKPOWER_HINT_FRAME_UPDATE;
        //    perfxml_read_cmddata(elmtScenario, scn);
        //    elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_GAMING", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_GAMING;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_GALLERY_BOOST", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_GALLERY_BOOST;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_GALLERY_STEREO_BOOST", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_GALLERY_STEREO_BOOST;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_SPORTS", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_SPORTS;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_TEST_MODE", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_TEST_MODE;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_WFD", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_WFD;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_PMS_INSTALL", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_PMS_INSTALL;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_EXT_LAUNCH", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_EXT_LAUNCH;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_WHITELIST_LAUNCH", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_WHITELIST_LAUNCH;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_WIPHY_SPEED_DL", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_WIPHY_SPEED_DL;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("MTKPOWER_HINT_SDN", (elmtScenario->Attribute("powerhint")))) {
            scn = MTKPOWER_HINT_SDN;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("LAUNCH", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::LAUNCH;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("VSYNC", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::VSYNC;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("INTERACTION", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::INTERACTION;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("VIDEO_ENCODE", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::VIDEO_ENCODE;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("VIDEO_DECODE", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::VIDEO_DECODE;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("LOW_POWER", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::LOW_POWER;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("SUSTAINED_PERFORMANCE", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::SUSTAINED_PERFORMANCE;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("VR_MODE", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::VR_MODE;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("AUDIO_STREAMING", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::AUDIO_STREAMING;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("AUDIO_LOW_LATENCY", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::AUDIO_LOW_LATENCY;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("CAMERA_LAUNCH", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::CAMERA_LAUNCH;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("CAMERA_STREAMING", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::CAMERA_STREAMING;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("CAMERA_SHOT", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::CAMERA_SHOT;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else if(!strcmp("EXPENSIVE_RENDERING", (elmtScenario->Attribute("powerhint")))) {
            scn = (int)PowerHint::EXPENSIVE_RENDERING;
            perfxml_read_cmddata(elmtScenario, scn);
            elmtScenario = elmtScenario->NextSiblingElement();
        } else {
            elmtScenario = elmtScenario->NextSiblingElement();
        }
    }
    return 0;
}

int loadConTable(const char *file_name)
{
    XMLDocument docXml;
    XMLError errXml = docXml.LoadFile(file_name);
    const char* str;
    const char* id;
    int cmdid;
    int idx = 0;

    ALOGI("[loadConTable]");

    if (errXml != XML_SUCCESS) {
        ALOGE("%s: Unable to powerhal ConTable config file '%s'. Error: %s",
            __FUNCTION__, file_name, XMLDocument::ErrorIDToName(errXml));
        return 0;
    } else {
        ALOGI("%s: load powerhal ConTable config succeed!", __FUNCTION__);
    }

    XMLElement* elmtRoot = docXml.RootElement();
    XMLElement *elmtCMD = elmtRoot->FirstChildElement("CMD");

    while (elmtCMD) {
        str = elmtCMD->Attribute("name");
        id = elmtCMD->Attribute("id");
        cmdid = strtol(id, NULL, 16);
        tConTable[idx].cmdName = str;
        tConTable[idx].cmdID = cmdid;
        ALOGD("[loadConTable][%d] str:%s id:%s cmdid:%x", idx, str, id, cmdid);

        XMLElement *elmtEntry = elmtCMD->FirstChildElement("Entry");
        if (elmtEntry) {
            const char* path = elmtEntry->Attribute("path");
            ALOGD("[loadConTable][%d] path:%s ", idx, path);

            tConTable[idx].entry = path;

            if(access(path, W_OK) != -1)
                tConTable[idx].isValid = 0;
            else {
                tConTable[idx].isValid = -1;
                ALOGE("%s cannot access!!!!", tConTable[idx].cmdName.c_str());
                ALOGE("write of %s failed: %s\n", tConTable[idx].entry.c_str(), strerror(errno));
            }
        }

        XMLElement *elmtValid = elmtCMD->FirstChildElement("Valid");

        if (elmtValid != NULL) {
            tConTable[idx].ignore = atoi(elmtValid->GetText());
            if (tConTable[idx].ignore == 1) {
                tConTable[idx].isValid = 0;
                ALOGI("[loadConTable][%d] ignore:%d isValid:%d ",
                    idx, tConTable[idx].ignore, tConTable[idx].isValid);
            }
        } else {
            ALOGD("Valid is empty");
            tConTable[idx].ignore = 0;
        }

        XMLElement *elmtLegacyCmdID = elmtCMD->FirstChildElement("LegacyCmdID");

        if (elmtLegacyCmdID != NULL) {
            tConTable[idx].legacyCmdID = atoi(elmtLegacyCmdID->GetText());
            ALOGD("[loadConTable][%d] LegacyCmdID:%d ", idx ,tConTable[idx].legacyCmdID);
        } else {
            ALOGD("legacyCmdID value is empty");
            tConTable[idx].legacyCmdID = -1;
        }

        XMLElement *elmtCompare = elmtCMD->FirstChildElement("Compare");

        if (elmtCompare != NULL) {
            tConTable[idx].comp = elmtCompare->GetText();
            ALOGD("[loadConTable][%d] Compare:%s ", idx ,tConTable[idx].comp.c_str());
        } else {
            ALOGD("compare value is empty");
            tConTable[idx].comp.assign("");
        }

        XMLElement *elmtMaxVal = elmtCMD->FirstChildElement("MaxValue");

        if (elmtMaxVal != NULL) {
            tConTable[idx].maxVal = atoi(elmtMaxVal->GetText());
            ALOGD("[loadConTable][%d] MaxValue:%d ", idx ,tConTable[idx].maxVal);
        } else {
            ALOGD("MaxValue value is empty");
            tConTable[idx].maxVal = 0;
        }

        XMLElement *elmtMinVal = elmtCMD->FirstChildElement("MinValue");

        if (elmtMinVal != NULL) {
            tConTable[idx].minVal = atoi(elmtMinVal->GetText());
            ALOGD("[loadConTable][%d] MinValue:%d ", idx, tConTable[idx].minVal);
        } else {
            ALOGD("MinValue is empty");
            tConTable[idx].minVal = 0;
        }

        XMLElement *elmtDefaultVal = elmtCMD->FirstChildElement("DefaultValue");

        if (elmtDefaultVal != NULL) {
            tConTable[idx].normalVal = atoi(elmtDefaultVal->GetText());
            ALOGD("[loadConTable][%d] DefaultValue:%d ", idx, tConTable[idx].normalVal);
        } else {
            ALOGD("DefaultValue is empty");
            tConTable[idx].normalVal = CFG_TBL_INVALID_VALUE;
        }

        XMLElement *elmtSportVal = elmtCMD->FirstChildElement("SportValue");

        if (elmtSportVal != NULL) {
            tConTable[idx].sportVal = atoi(elmtSportVal->GetText());
            ALOGD("[loadConTable][%d] sportVal:%d ", idx, tConTable[idx].sportVal);
        } else {
            ALOGD("SportVal is empty");
            tConTable[idx].sportVal = CFG_TBL_INVALID_VALUE;
        }

        XMLElement *elmtPrefix = elmtCMD->FirstChildElement("Prefix");

        if (elmtPrefix != NULL) {
            tConTable[idx].prefix = elmtPrefix->GetText();
            ALOGI("[loadConTable][%d] Prefix:%s ", idx, tConTable[idx].prefix.c_str());
        } else {
            ALOGD("prefix is empty");
            tConTable[idx].prefix.assign("");
        }

        if(tConTable[idx].prefix.length() != 0) {
            //ALOGD("[loadConTable] cmd:%s, path:%s, prefix 1:%s;", tConTable[idx].cmdName.c_str(),
            //    tConTable[idx].entry.c_str(), tConTable[idx].prefix.c_str());
            // Support one space. Use '^' to instead of ' ', i.e, "test^" => "test ".
            std::size_t found = tConTable[idx].prefix.find_first_of('^');
            if(found != std::string::npos)
                tConTable[idx].prefix.replace(found, 1, " ");
            ALOGD("[loadConTable] cmd:%s, path:%s, prefix 2:%s;", tConTable[idx].cmdName.c_str(),
            tConTable[idx].entry.c_str(), tConTable[idx].prefix.c_str());
        }

        if (tConTable[idx].normalVal != CFG_TBL_INVALID_VALUE) {
            tConTable[idx].defaultVal = tConTable[idx].normalVal;

            if(tConTable[idx].isValid == 0)
                set_value(tConTable[idx].entry.c_str(), tConTable[idx].normalVal);
        }
        else
            tConTable[idx].defaultVal = get_int_value(tConTable[idx].entry.c_str());

        ALOGI("[loadConTable] cmd:%s, path:%s, normal:%d, default:%d", tConTable[idx].cmdName.c_str(),
            tConTable[idx].entry.c_str(), tConTable[idx].normalVal, tConTable[idx].defaultVal);

        // initial setting should be an invalid value
        if(tConTable[idx].comp == LESS)
            tConTable[idx].resetVal = tConTable[idx].maxVal + 1;
        else
            tConTable[idx].resetVal = tConTable[idx].minVal - 1;
        tConTable[idx].curVal = tConTable[idx].resetVal;

        idx++;
        elmtCMD = elmtCMD->NextSiblingElement();
    }

    return 1;
}

void perfservice_xmlparse_freeList()
{
    if(ptXmlActList!=NULL) {
        free(ptXmlActList);
        ptXmlActList = NULL;
    }
}

const char* perfservice_xmlparse_get_applist_file()
{
    const char * file_path;

    if (access(APP_LIST_XMLPATH, F_OK) != -1) {
        file_path = APP_LIST_XMLPATH;
    } else if (access(PACK_LIST_XMLPATH, F_OK) != -1) {
        file_path = PACK_LIST_XMLPATH;
    } else {
        file_path = NULL;
        ALOGE("can't find app list file\n");
    }

    return file_path;
}

const char* perfservice_xmlparse_get_data_applist_file()
{
    const char * file_path;

    if (access(APP_LIST_XMLPATH_2, F_OK) != -1) {
        file_path = APP_LIST_XMLPATH_2;
    } else if (access(PACK_LIST_XMLPATH_2, F_OK) != -1) {
        file_path = PACK_LIST_XMLPATH_2;
    } else {
        file_path = NULL;
        ALOGE("can't find app list file\n");
    }

    return file_path;
}

int perfservice_xmlparse_reload_whitelist()
{
    int PackNum = 0;
    /* re-initialize */
    nXmlPackNum = nXmlActNum = nXmlCmdNum = 0;
    const char * app_file_path;
    const char * data_app_file_path;

    app_file_path = perfservice_xmlparse_get_applist_file();
    data_app_file_path = perfservice_xmlparse_get_data_applist_file();

    if (app_file_path == NULL)
        return -1;

    /* get file */
    get_activity_num_XML(app_file_path);
    if (access(data_app_file_path, F_OK) != -1) {
        get_activity_num_XML(data_app_file_path);
    } else {
        ALOGI("access of %s failed 1: %s\n", data_app_file_path, strerror(errno));
    }

    if(nXmlCmdNum > 0){
        ALOGI("[re-init] nXmlPackNum:%d nXmlActivityNum:%d nXmlCmdNum:%d",
            nXmlPackNum, nXmlActNum, nXmlCmdNum);

        if((ptXmlActList = (xml_activity*)malloc(sizeof(xml_activity)*(nXmlCmdNum))) == NULL) {
            ALOGE("Can't allocate memory");
            return -1;
        }

        memset(ptXmlActList, 0, sizeof(xml_activity)*nXmlCmdNum);

        nXmlCmdIndex = 0;
        get_activity_list_XML(app_file_path);
        if (access(data_app_file_path, F_OK) != -1) {
            get_activity_list_XML(data_app_file_path);
        } else {
            ALOGI("access of %s failed 2: %s\n", data_app_file_path, strerror(errno));
        }
    }
    else {
        ALOGI("[re-init] No activity data from white list!!");
    }

    PackNum = get_activity_totalnum();
    ALOGI("[re-init] PackNum:%d", PackNum);
    if(PackNum < 0) {
        ALOGE("invalid nPackNum");
        return 0;
    }

    return PackNum;

}

int perfservice_xmlparse_init()
{
    int PackNum = 0;
    const char * app_file_path;
    const char * data_app_file_path;

    app_file_path = perfservice_xmlparse_get_applist_file();
    data_app_file_path = perfservice_xmlparse_get_data_applist_file();

    if (app_file_path == NULL)
        return 0;

    /* get file */
    get_activity_num_XML(app_file_path);

    if (access(data_app_file_path, F_OK) != -1)
        get_activity_num_XML(data_app_file_path);
    else
        ALOGI("access of %s failed 1: %s\n", data_app_file_path, strerror(errno));

    if(nXmlCmdNum > 0){
        ALOGI("[init] nXmlPackNum:%d nXmlActivityNum:%d nXmlCmdNum:%d ", nXmlPackNum, nXmlActNum, nXmlCmdNum);
        if((ptXmlActList = (xml_activity*)malloc(sizeof(xml_activity)*(nXmlCmdNum))) == NULL) {
            ALOGE("Can't allocate memory");
            return 0;
        }

        memset(ptXmlActList, 0, sizeof(xml_activity)*nXmlCmdNum);

        get_activity_list_XML(app_file_path);

        if (access(data_app_file_path, F_OK) != -1)
            get_activity_list_XML(data_app_file_path);
        else
            ALOGI("access of %s failed 2: %s\n", data_app_file_path, strerror(errno));
    } else {
        ALOGI("[init] No activity data from white list!!");
    }

    PackNum = get_activity_totalnum();
    ALOGI("[init] nPackNum:%d", PackNum);
    if(PackNum < 0) {
        ALOGE("invalid nPackNum");
        return 0;
    }

    return PackNum;
}

