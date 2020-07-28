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

#define LOG_TAG "CAMTOOL"

#include <utils/Log.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>          // for errno in OpenFifo()
#include <dirent.h>
#include <pthread.h>

#include "camtool.h"
//#include "aaa_types.h"
///vendor/mediatek/proprietary/hardware/mtkcam/include/mtkcam/def/BuiltinTypes.h
//#include "cct_feature.h"

#define PIPE_OUT_FNAME      "/mnt/vendor/cct/myfifoin"
#define PIPE_IN_FNAME       "/mnt/vendor/cct/myfifoout"
#define CCT_SHELL_CMD_MAX_LENGTH            (1024)

#if 1
#define CAMTOOL_LOGD(fmt,arg...)      ({ printf(fmt "\n", ##arg);   ALOGD("[%s] " fmt, __FUNCTION__,##arg); })
#define CAMTOOL_WARN(fmt,arg...)      ({ printf(fmt "\n", ##arg);   ALOGW("[%s] " fmt, __FUNCTION__,##arg); })
#define CAMTOOL_LOGE(fmt,arg...)      ({ printf(fmt "\n", ##arg);   ALOGE("[%s](%s:%d) " fmt, __FUNCTION__,__FILE__,__LINE__,##arg); })
#define system_call(cmd)              ({ int ret;   printf("[SystemCall] %s\n", cmd);   ALOGD("[SystemCall] %s", cmd);   ret = system(cmd);   ALOGD("[SystemCall] ret=%d", ret);   ret; })
#else
#define CAMTOOL_LOGD(fmt,arg...)      ALOGD("[%s] " fmt, __FUNCTION__,##arg)
#define CAMTOOL_WARN(fmt,arg...)      ALOGW("[%s] " fmt, __FUNCTION__,##arg)
#define CAMTOOL_LOGE(fmt,arg...)      ALOGE("[%s](%s:%d) " fmt, __FUNCTION__,__FILE__,__LINE__,##arg)
#define system_call(cmd)              ({ int ret;   ALOGD("[SystemCall] %s", cmd);   ret = system(cmd);   ret; })
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


typedef enum
{
     LNX_FIFO_IN_MODE = 0
    ,LNX_FIFO_OUT_MODE
} LNX_FIFO_MODE_T;

typedef enum {
     CT_SENSOR_MAIN = 1
    ,CT_SENSOR_SUB
    ,CT_SENSOR_MAIN2
    ,CT_SENSOR_SUB2
    ,CT_SENSOR_MAIN3
    ,CT_SENSOR_SUB3
    ,CT_SENSOR_MAIN4
    ,CT_SENSOR_SUB4
    ,CT_SENSOR_MAIN5
    ,CT_SENSOR_SUB5
    ,CT_SENSOR_MAIN6
    ,CT_SENSOR_SUB6
    ,CT_SENSOR_MAIN7
    ,CT_SENSOR_SUB7
    ,CT_SENSOR_MAIN8
    ,CT_SENSOR_SUB8
    ,CT_SENSOR_MAIN9
    ,CT_SENSOR_SUB9
    ,CT_SENSOR_MAIN10
    ,CT_SENSOR_SUB10
} CT_SENSOR_T;

typedef enum {
     CT_PROFILE_MODE_PREVIEW = 1
    ,CT_PROFILE_MODE_CAPTURE
    ,CT_PROFILE_MODE_VIDEO
} __attribute__((aligned(4))) CT_PROFILE_MODE_T;

typedef enum {
    CT_SENSOR_MODE_PREVIEW = 0
    ,CT_SENSOR_MODE_CAPTURE
    ,CT_SENSOR_MODE_VIDEO
    ,CT_SENSOR_MODE_SLIM_VIDEO1
    ,CT_SENSOR_MODE_SLIM_VIDEO2
    ,CT_SENSOR_MODE_CUSTOM1
    ,CT_SENSOR_MODE_CUSTOM2
    ,CT_SENSOR_MODE_CUSTOM3
    ,CT_SENSOR_MODE_CUSTOM4
    ,CT_SENSOR_MODE_CUSTOM5
} __attribute__((aligned(4))) CT_SENSOR_MODE_T;

typedef enum {
     CT_FEATURE_MODE_NONE = 1
    ,CT_FEATURE_MODE_MFLL
    ,CT_FEATURE_MODE_MNR_HW
    ,CT_FEATURE_MODE_MNR_SW
    ,CT_FEATURE_MODE_MFLL_MNR_HW
    ,CT_FEATURE_MODE_MFLL_MNR_SW
} __attribute__((aligned(4))) CT_FEATURE_MODE_T;

typedef enum {
     CT_FORMAT_PURE = 1
    ,CT_FORMAT_PROC
    ,CT_FORMAT_JPEG
    //,CT_FORMAT_YV12
} __attribute__((aligned(4))) CT_OUT_FORMAT_T;

struct CCT_CAPTURE_IN
{
    CT_OUT_FORMAT_T out_format         :32;
    CT_PROFILE_MODE_T profile_mode    :32;
    CT_FEATURE_MODE_T feature_mode    :32;
};

struct CCT_CAPTURE_CALLBACK
{
    int out_format;
    int profile_mode;
    int dump_time;
};

struct CCT_CAPTURE_OUT
{
    MUINT32 width;
    MUINT32 height;
    MUINT32 size;
    MUINT32 bayerPattern;
    MUINT32 bitsNum;
    CT_OUT_FORMAT_T out_format        :32;
};


class CamTool
{
public:
    //    Ctor/Dtor.
                            CamTool();
    virtual                 ~CamTool();

                            CamTool(const CamTool&);
                            CamTool& operator=(const CamTool&);
    CamTool *               createInstance();
    virtual void            destroyInstance();
    MINT32                  parseCommandFile(char *infname, char *outfname);


private:
    /************************
     * function
     ************************/
    MINT32 getCctCommand(FILE *pInFPtr, MUINT32 *pCmdOp, MUINT32 *pBufSize, void **pCmdBuf);
    MINT32 putCctOutputResult(FILE *outFPtr, MUINT32 cmdOp, MUINT32 repStatus, MUINT32 repSize, MUINT8 *outBuf);
    MINT32 doCctOperation(MUINT32 cmdOp, MUINT32 inBufSize, MUINT8 *pInBuf, MUINT32 *pRepStatus, MUINT32 *pRepSize, MUINT8 **pOutBuf);
    MINT32 ReOpenFifo();

    MINT32 changeSensor(CT_SENSOR_T sensor_type);
    MINT32 changeProfileMode(CT_PROFILE_MODE_T profile_mode);
    MINT32 changeSensorMode(CT_SENSOR_MODE_T sensor_mode);
    MINT32 changeFeatureMode(CT_FEATURE_MODE_T feature_mode);

private:
    /************************
     * member
     ************************/
    MINT32 m_iFdIn;
    MINT32 m_iFdOut;
};

CamTool::
CamTool()
    : m_iFdIn(-1)
    , m_iFdOut(-1)
{
}

CamTool::
~CamTool()
{
}

CamTool*
CamTool::
createInstance()
{
    return new CamTool();
}

void
CamTool::
destroyInstance()
{
    delete this;
}


MINT32 launchSafeMode()
{
    char valstr[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.camera.cct.launch.safe.mode", valstr, "0");
    MINT32 value = atoi(valstr);

    return value;
}

MINT32 getDebugDumpTime()
{
    char valstr[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.camera.cct.dump_time", valstr, "0");
    MINT32 value = atoi(valstr);
    return value;
}

MINT32 checkDebugDump()
{
    char valstr[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.camera.cct.dump", valstr, "0");
    MINT32 value = atoi(valstr);

    if(0 != value){
        system_call("mkdir /data/vendor/cct_debug_dump");
    }
    return value;
}
void set_vendor_property(const char *name, const char *value)
{
    char tcmd[300];
    snprintf(tcmd, 300, "setprop %s %s", name, value);
    system_call(tcmd);
}
/****************************************************************************/
// mode = 0 for FIFO IN
//      = 1 for FIFO OUT
MINT32 OpenFifo( MINT32 *fd, char *FifoFilename, LNX_FIFO_MODE_T mode )
{
    MINT32 flag;
    MINT32 state;

    umask(0);

#if 1
    // the FIFO needs to be created by the server side. If it is not existed, the client side
    // should wait for the server to create it.
    while ( (state = access(FifoFilename, F_OK)) != 0 );    // Need to add a timeout here

    CAMTOOL_LOGD("FIFO file %s exist",FifoFilename);
#else
    state = access(FifoFilename, F_OK);

    if (state == 0)
        CAMTOOL_LOGD("FIFO IN file %s exist",FifoFilename);
    else if(state == -1)
    {
        CAMTOOL_LOGD("Creat FIFO file %s",FifoFilename);
        if(mkfifo(FifoFilename, S_IFIFO|0777)<0)  { // create pipe
            CAMTOOL_LOGE("mkfifo() failed");
            return -1;
        }
        else
            CAMTOOL_LOGD("Creat FIFO file success");
    }
#endif

    if (mode == LNX_FIFO_IN_MODE)  // FIFO in
        flag = O_RDONLY;
    else  // FIFO out
        flag = O_WRONLY;

    *fd = open(FifoFilename, flag, 0); // open pipe
    if(*fd == -1)
    {
        CAMTOOL_LOGE("Open FIFO failed");
        return -1;
    }
    CAMTOOL_LOGD("Open FIFO file %s success",FifoFilename);
    return 0;
}


MINT32 CloseFifo( MINT32 fd, char *FifoFilename )
{
    int state;

    state = close(fd);
    //unlink(FifoFilename);
    if (state == 0)
        CAMTOOL_LOGD("Close FIFO file %s success", FifoFilename);
    else
        CAMTOOL_LOGD("Close FIFO file failed");
    return state;
}

#define FLEN 256
MINT32 _get_value(char *&ptr, int &vlu)
{
    char *pstart = ptr;
    vlu = 0;
    while(ptr && (*ptr >= '0') && (*ptr <= '9')){

        int v = (int)(*ptr - '0');
        vlu = 10 * vlu + v;
        ptr++;
    }

    return (ptr != pstart)? 0:-1;
}
MINT32 _get_w_h_b_o_string(char *filename, char *foundStr)
{
    char tstr[FLEN], *p, *pstr;
    int ret = -1;

    strncpy(tstr, filename, FLEN-1);
    tstr[FLEN-1] = 0;
    p = tstr;
    while((pstr = p = strstr(p, "__")))
    {
        int w = 0, h = 0, b = 0, o = 0;

        p+=2;
        if(0 != _get_value(p, w))
            continue;

        if('x' != *p++)
            continue;

        if(0 != _get_value(p, h))
            continue;

        if('_' != *p++)
            continue;

        if(0 != _get_value(p, b))
            continue;

        if('_' != *p++)
            continue;

        if(0 != _get_value(p, o))
            continue;

        *p = 0;
        strncpy(foundStr, pstr, FLEN);
        ret = 0;
        break;
    }
    return ret;
}
MINT32 _get_w_h_string(char *filename, char *foundStr)
{
    char tstr[FLEN], *p, *pstr;
    int ret = -1;

    strncpy(tstr, filename, FLEN-1);
    tstr[FLEN-1] = 0;
    p = tstr;
    while((pstr = p = strstr(p, "__")))
    {
        int w = 0, h = 0;

        p+=2;
        if(0 != _get_value(p, w))
            continue;

        if('x' != *p++)
            continue;

        if(0 != _get_value(p, h))
            continue;

        *p = 0;
        strncpy(foundStr, pstr, FLEN);
        ret = 0;
        break;
    }
    return ret;
}
MINT32 _find_file_match(char *findfile, const char *folder, char *mat)
{
    CAMTOOL_LOGD("folder=%s, mat=%s", folder, mat);
    findfile[0] = 0;

    DIR *dir;
    if ((dir=opendir(folder)) == NULL)
    {
        CAMTOOL_LOGE("Open dir failed: %s", folder);
        return -1;
    }

    struct dirent *ptr;
    while ((ptr=readdir(dir)) != NULL)
    {
        char filename[FLEN];
        strncpy(filename, ptr->d_name, FLEN-1);
        filename[FLEN-1] = 0;

        if( 0 == strncmp(filename, mat, FLEN) ) // match string: mat
        {
            CAMTOOL_LOGD("find %s", filename);
            strncpy(findfile, filename, FLEN);
            break;
        }
    }
    closedir(dir);
    return (0 == findfile[0])? (-2):0;
}
MINT32 _find_file_latest(char *findfile, const char *folder, const char *pre , const char *mid, const char *post)
{
    CAMTOOL_LOGD("folder=%s, pre=%s, mid=%s, post=%s", folder, pre ? pre : "[NULL]", mid ? mid : "[NULL]", post ? post : "[NULL]");
    int latest_count = 10;
    findfile[0] = 0;

    DIR *dir;
    if ((dir=opendir(folder)) == NULL)
    {
        CAMTOOL_LOGE("Open dir failed: %s", folder);
        return -1;
    }

    struct dirent *ptr;
    while ((ptr=readdir(dir)) != NULL)
    {
        char filename[FLEN];
        strncpy(filename, ptr->d_name, FLEN-1);
        filename[FLEN-1] = 0;

        if( ((NULL == pre)  || strstr(filename, pre) == filename) &&     // start with string: pre
            ((NULL == mid)  || strstr(filename, mid) != 0) &&            // contains string: mid
            ((NULL == post) ||
             ( (strlen(filename)>=strlen(post)) && (0 == strcmp(filename+strlen(filename)-strlen(post), post)) )) )
        {
            CAMTOOL_LOGD("find %s", filename);

            if( (0 == findfile[0]) || (0 > strncmp(findfile, filename, FLEN)) )
            {
                strncpy(findfile, filename, FLEN);
            }

            if(0 == latest_count--)
            {
                break;
            }
        }
    }
    closedir(dir);
    return (0 == findfile[0])? (-2):0;
}
MINT32 _print_folder(const char *folder)
{
    CAMTOOL_LOGD("folder=%s", folder);

    DIR *dir;
    if ((dir=opendir(folder)) == NULL)
    {
        CAMTOOL_LOGE("Open dir failed: %s", folder);
        return -1;
    }

    struct dirent *ptr;
    while ((ptr=readdir(dir)) != NULL)
    {
        char filename[FLEN];
        strncpy(filename, ptr->d_name, FLEN-1);
        filename[FLEN-1] = 0;

        CAMTOOL_LOGD("file=%s", filename);
    }
    closedir(dir);
    return 0;
}

MINT32 _cmd_capture(int &get_unpacked_raw, CT_OUT_FORMAT_T out_format, int wait_time)
{
    int ret = 0;
    MINT32 debgu_dump = checkDebugDump();

    // 1. INIT
    ret |= system_call("rm -rf /data/vendor/camera_dump");
    ret |= system_call("rm /mnt/vendor/cct/pure__*");
    ret |= system_call("rm /mnt/vendor/cct/proc__*");
    ret |= system_call("rm /mnt/vendor/cct/*.jpg");
    ret |= system_call("rm /mnt/vendor/cct/*.yv12");
    ret |= system_call("sync");
    ret |= system_call("mkdir /data/vendor/camera_dump");
    CAMTOOL_LOGD("init: ret=%d", ret);

#ifdef MTKCAM_CCT_TUNING_FOR_LOWDRAM
    set_vendor_property("vendor.debug.camera.noskip.frame", "1");
#endif

    // 2. START
    ret = 0;
    switch(out_format){
    case CT_FORMAT_PURE:
        set_vendor_property("vendor.debug.camera.raw.type", "1");
        break;
    case CT_FORMAT_JPEG:
        set_vendor_property("vendor.debug.camera.raw.type", "0");
        break;
    }
    //set_vendor_property("vendor.debug.camera.dump.JpegNode", "1");
    //set_vendor_property("vendor.debug.camera.p2.dump", "1");
    // for mt6765 HAL1
    set_vendor_property("vendor.debug.camera.dump.p2", "1");
    //ret |= system_call("sync");

    ret |= system_call("input keyevent KEYCODE_CAMERA");
    CAMTOOL_LOGD("dump: ret=%d", ret);


    // 3. CHECK
    char checkFile[FLEN];
    char dumpFile[FLEN];
    usleep(3000*1000);
    for(int i=0; i<wait_time; i++) {
        ret = _find_file_latest(checkFile, "/data/vendor/camera_dump", NULL, NULL, ".jpg"); //system_call("test -e /data/vendor/camera_dump/*.jpg");
        if(!ret) {break;}
        CAMTOOL_LOGD("(%d) ret=%d, wait a moment..", i, ret);
        usleep(4000*1000);
    }
    if(ret){
        CAMTOOL_LOGE("dump jpeg failed");
        goto FAILED;
    }

    for(int i=0; i<wait_time; i++) {
        ret = _find_file_latest(dumpFile, "/data/vendor/camera_dump", NULL, "-imgo", ".packed_word"); //system_call("test -e /data/vendor/camera_dump/*-imgo*.packed_word");
        if(!ret) {break;}
        ret = _find_file_latest(dumpFile, "/data/vendor/camera_dump", NULL, "-imgo", ".raw"); //system_call("test -e /data/vendor/camera_dump/*-imgo*.raw");
        if(!ret)
        {
            get_unpacked_raw = 1;
            CAMTOOL_LOGD("found unpacked .raw instead of .packed_word");
            break;
        }
        CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
        usleep(500*1000);
    }
    if(ret){
        CAMTOOL_LOGE("dump packed_word/raw failed");
        goto FAILED;
    }
    char whboStr[FLEN];
    ret = _get_w_h_b_o_string(dumpFile, whboStr);
    if(ret){
        CAMTOOL_LOGE("get w/h/b/o failed");
        goto FAILED;
    }

    // 4. MOVE
    for(int i=0; i<wait_time; i++) {
        ret = system_call("mv /data/vendor/camera_dump/*.jpg /mnt/vendor/cct/acdkCap.jpg");
        if(!ret) {break;}
        CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
        usleep(500*1000);
    }
    if(ret){
        CAMTOOL_LOGE("move jpeg failed");
        goto FAILED;
    }

    char srcPath[FLEN];
    char dstPath[FLEN];
    char dstDebugPath[FLEN];
    for(int i=0; i<wait_time; i++) {
        //    tar=$(dirname $src)/$(echo $src | sed -r 's/.+(__[0-9]+x[0-9]+_[0-9]+_[0-9]+).+/pure\\1/');
        //if( get_unpacked_raw )
        //{
        //    ret = system_call(
        //        "src=$(find /data/vendor/camera_dump -name \"*-imgo*.raw\"); \
        //        tar=/mnt/vendor/cct/$(echo $src | sed -r 's/.+(__[0-9]+x[0-9]+_[0-9]+_[0-9]+).+/pure\\1/'); \
        //        mv $src $tar"
        //    );
        //}
        //else
        //{
        //    ret = system_call(
        //        "src=$(find /data/vendor/camera_dump -name \"*-imgo*.packed_word\"); \
        //        tar=/mnt/vendor/cct/$(echo $src | sed -r 's/.+(__[0-9]+x[0-9]+_[0-9]+_[0-9]+).+/pure\\1/'); \
        //        mv $src $tar"
        //    );
        //}
        snprintf(srcPath, FLEN, "/data/vendor/camera_dump/%s", dumpFile);
        snprintf(dstPath, FLEN, "/mnt/vendor/cct/pure%s", whboStr);
        snprintf(dstDebugPath, FLEN, "/data/vendor/cct_debug_dump/%s", dumpFile);

        //CAMTOOL_LOGD("move file: src=%s, dst=%s", srcPath, dstPath);
        //ret = rename(srcPath, dstPath);
        char tcmd[600];

        if(debgu_dump){
            snprintf(tcmd, 600, "cp %s %s", srcPath, dstDebugPath);
            ret = system_call(tcmd);
        }

        snprintf(tcmd, 600, "mv %s %s", srcPath, dstPath);
        ret = system_call(tcmd);
        if(!ret) {break;}

        CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
        usleep(500*1000);
    }
    if(ret){
        CAMTOOL_LOGE("move packed_word/raw failed");
        goto FAILED;
    }

    for(int i=0; i<wait_time; i++) {
        usleep(1000*1000);
        ret = _find_file_latest(checkFile, "/mnt/vendor/cct", NULL, NULL, ".jpg"); //system_call("test -e /mnt/vendor/cct/*.jpg");
        if(!ret) {break;}
        CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
    }
    if(ret){
        CAMTOOL_LOGE("move jpeg failed");
        goto FAILED;
    }

    for(int i=0; i<wait_time; i++) {
        ret = _find_file_latest(checkFile, "/mnt/vendor/cct", "pure__", NULL, NULL); //system_call("test -e /mnt/vendor/cct/pure__*");
        if(!ret) {break;}
        CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
        usleep(1000*1000);
    }
    if(ret){
        CAMTOOL_LOGE("move packed_word/raw failed");
        goto FAILED;
    }

    // 4. STOP
    //set_vendor_property("vendor.debug.camera.dump.JpegNode", "0");
    //set_vendor_property("vendor.debug.camera.p2.dump", "0");

#ifdef MTKCAM_CCT_TUNING_FOR_LOWDRAM
    set_vendor_property("vendor.debug.camera.noskip.frame", "\"\"");
#endif
    return ret;
FAILED:
    CAMTOOL_LOGE("FAILED");

#ifdef MTKCAM_CCT_TUNING_FOR_LOWDRAM
    set_vendor_property("vendor.debug.camera.noskip.frame", "\"\"");
#endif
    _print_folder("/data/vendor/camera_dump");
    return ret;
}

MINT32 _cmd_capture_recap(int &get_unpacked_raw, CT_OUT_FORMAT_T out_format)
{
    int ret = 0;

    ret = _cmd_capture(get_unpacked_raw, out_format, 300);
    if(ret)
    {
        CAMTOOL_LOGE("re-cap");
        usleep(2000*1000);
        ret = _cmd_capture(get_unpacked_raw, out_format, 30);
    }
    return ret;
}

MINT32 _cmd_preview(CT_PROFILE_MODE_T profile_mode, CT_OUT_FORMAT_T out_format, int dump_time, int wait_time)
{
    (void)profile_mode;
    int ret = 0;
    MINT32 debgu_dump = checkDebugDump();
    CAMTOOL_LOGD("dump_time=%d, wait_time=%d", dump_time, wait_time);

    int debug_dump_time = getDebugDumpTime();
    if(0 != debug_dump_time){
        dump_time = debug_dump_time;
        CAMTOOL_LOGD("change dump_time=%d", dump_time);
    }
    // 1. INIT
    ret |= system_call("rm -rf /data/vendor/camera_dump");
    ret |= system_call("rm /mnt/vendor/cct/pure__*");
    ret |= system_call("rm /mnt/vendor/cct/proc__*");
    ret |= system_call("rm /mnt/vendor/cct/*.jpg");
    ret |= system_call("rm /mnt/vendor/cct/*.yv12");
    ret |= system_call("sync");
    ret |= system_call("mkdir /data/vendor/camera_dump");
    CAMTOOL_LOGD("init: ret=%d", ret);

    // 2. START
    ret = 0;
    switch(out_format){
    case CT_FORMAT_PURE:
        set_vendor_property("vendor.debug.camera.raw.type", "1");
        set_vendor_property("vendor.debug.camera.dump.p1.imgo", "1");
        break;
    case CT_FORMAT_PROC:
        set_vendor_property("vendor.debug.camera.raw.type", "0");
        set_vendor_property("vendor.debug.camera.dump.p1.imgo", "1");
        break;
    case CT_FORMAT_JPEG:
        set_vendor_property("vendor.debug.camera.raw.type", "0");
        set_vendor_property("vendor.debug.camera.preview.dump", "1"); //*-img3o*.yv12, *-wdmao*.yv12
        break;
    }
    ret |= system_call("sync");

    usleep(dump_time);
    //ret |= system_call("input keyevent KEYCODE_CAMERA");

    CAMTOOL_LOGD("dump: ret=%d", ret);


    // 3. STOP
    switch(out_format){
    case CT_FORMAT_PURE:
    case CT_FORMAT_PROC:
        set_vendor_property("vendor.debug.camera.dump.p1.imgo", "0");
        break;
    case CT_FORMAT_JPEG:
        set_vendor_property("vendor.debug.camera.preview.dump", "0"); //*-img3o*.yv12, *-wdmao*.yv12
        break;
    }

    // 4. CHECK
    char dumpFile[FLEN];
    usleep(2000*1000);
    for(int i=0; i<wait_time; i++) {
        switch(out_format){
        case CT_FORMAT_PURE:
        case CT_FORMAT_PROC:
            ret = _find_file_latest(dumpFile, "/data/vendor/camera_dump", NULL, "-imgo", ".packed_word"); //system_call("test -e /data/vendor/camera_dump/*-imgo*.packed_word");
            break;
        case CT_FORMAT_JPEG:
            ret = _find_file_latest(dumpFile, "/data/vendor/camera_dump", NULL, "-img3o", ".yv12");  //system_call("test -e /data/vendor/camera_dump/*.yv12");
            if(0 != ret) {
                CAMTOOL_LOGD("cannot find *-img3o*.yv12, try *-wdmao*.yv12 ,ret=%d", ret);
                ret = _find_file_latest(dumpFile, "/data/vendor/camera_dump", NULL, "-wdmao", ".yv12");  //system_call("test -e /data/vendor/camera_dump/*.yv12");
            }
            break;
        }

        if(!ret) {break;}
        CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
        usleep(2000*1000);
    }
    if(ret){
        CAMTOOL_LOGE("dump file failed");
        goto FAILED;
    }

    char whboStr[FLEN];
    switch(out_format){
    case CT_FORMAT_PURE:
    case CT_FORMAT_PROC:
        ret = _get_w_h_b_o_string(dumpFile, whboStr);
        break;
    case CT_FORMAT_JPEG:
        ret = _get_w_h_string(dumpFile, whboStr);
        break;
    }
    if(ret){
        CAMTOOL_LOGE("get w/h/b/o failed");
        goto FAILED;
    }

    // 5. MOVE
    char srcPath[FLEN];
    char dstPath[FLEN];
    char dstDebugPath[FLEN];
    for(int i=0; i<wait_time; i++) {
        switch(out_format){
        case CT_FORMAT_PURE:
            snprintf(srcPath, FLEN, "/data/vendor/camera_dump/%s", dumpFile);
            snprintf(dstPath, FLEN, "/mnt/vendor/cct/pure%s", whboStr);
            snprintf(dstDebugPath, FLEN, "/data/vendor/cct_debug_dump/%s", dumpFile);
            usleep(500*1000);
            //ret = system_call(
            //    "src=$(find /data/vendor/camera_dump -name \"*-imgo*.packed_word\" | head -1); \
            //    tar=/mnt/vendor/cct/$(echo $src | sed -r 's/.+(__[0-9]+x[0-9]+_[0-9]+_[0-9]+).+/pure\\1/'); \
            //    mv $src $tar"
            //);
            break;
        case CT_FORMAT_PROC:
            snprintf(srcPath, FLEN, "/data/vendor/camera_dump/%s", dumpFile);
            snprintf(dstPath, FLEN, "/mnt/vendor/cct/proc%s", whboStr);
            snprintf(dstDebugPath, FLEN, "/data/vendor/cct_debug_dump/%s", dumpFile);
            usleep(500*1000);
            //ret = system_call(
            //    "src=$(find /data/vendor/camera_dump -name \"*-rrzo*.packed_word\" | head -1); \
            //    tar=/mnt/vendor/cct/$(echo $src | sed -r 's/.+(__[0-9]+x[0-9]+_[0-9]+_[0-9]+).+/proc\\1/'); \
            //    mv $src $tar"
            //);
            break;
        case CT_FORMAT_JPEG:
            snprintf(srcPath, FLEN, "/data/vendor/camera_dump/%s", dumpFile);
            snprintf(dstPath, FLEN, "/mnt/vendor/cct/jpegSrc%s.yv12", whboStr);
            snprintf(dstDebugPath, FLEN, "/data/vendor/cct_debug_dump/%s", dumpFile);
            //ret = system_call(
            //    /*"src=$(find /data/vendor/camera_dump -name \"*-img3o*.yv12\" | head -1); \ */
            //    "src=$(find /data/vendor/camera_dump -name \"*.yv12\" | head -1); \
            //    tar=/mnt/vendor/cct/$(echo $src | sed -r 's/.+(__[0-9]+x[0-9]+_[0-9]+_[0-9]+).+/yv12\\1/'); \
            //    mv $src $tar"
            //);
            break;
        }
        usleep(500*1000);
        //CAMTOOL_LOGD("move file: src=%s, dst=%s", srcPath, dstPath);
        //ret = rename(srcPath, dstPath);
        char tcmd[600];

        if(debgu_dump){
            snprintf(tcmd, 600, "cp %s %s", srcPath, dstDebugPath);
            ret = system_call(tcmd);
        }

        snprintf(tcmd, 600, "mv %s %s", srcPath, dstPath);
        ret = system_call(tcmd);
        if(!ret) {break;}

        CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
        usleep(500*1000);
    }
    if(ret){
        CAMTOOL_LOGE("move file failed");
        goto FAILED;
    }

    // 6. encode JPEG
    if(out_format == CT_FORMAT_JPEG)
    {
        char tcmd[600];
        snprintf(tcmd, 600, "jpegtool %s %s", dstPath, "/mnt/vendor/cct/acdkCap.jpg");
        ret = system_call(tcmd);
        if(ret){
            CAMTOOL_LOGE("encode JPEG failed");
            goto FAILED;
        }
    }

    return ret;
FAILED:
    CAMTOOL_LOGE("FAILED");
    _print_folder("/data/vendor/camera_dump");
    return ret;
}

MINT32 _cmd_preview_recap(CT_PROFILE_MODE_T profile_mode, CT_OUT_FORMAT_T out_format, int dump_time = 500*1000)
{
    const int recap_max = 1;
    const int dump_time_max = MAX(dump_time, 10*1000*1000);
    int ret = 0;

    ret = _cmd_preview(profile_mode, out_format, dump_time, 5);
    for(int i=1; (0!=ret) && (i<=recap_max); i++)
    {
        CAMTOOL_LOGE("re-cap count=%d", i);
        usleep(2000*1000);
        ret = _cmd_preview(profile_mode, out_format, MIN(dump_time*3, dump_time_max), 15);
    }
    return ret;
}

MINT32 captureCommand(CCT_CAPTURE_IN cap_in, int &unpacked_raw, int dump_time/* = -1*/)
{
    CAMTOOL_LOGD("+" );
    CAMTOOL_LOGD("out_format=%d, profile_mode=%d, feature_mode=%d", cap_in.out_format, cap_in.profile_mode, cap_in.feature_mode );

    int ret = 0;
    switch(cap_in.profile_mode){
    case CT_PROFILE_MODE_CAPTURE:
        switch(cap_in.out_format){
        case CT_FORMAT_PURE:
        case CT_FORMAT_JPEG:
            ret = _cmd_capture_recap(unpacked_raw, cap_in.out_format);
            break;
        case CT_FORMAT_PROC:
            ret = _cmd_preview_recap(cap_in.profile_mode, cap_in.out_format);
            break;
        }
        break;
    case CT_PROFILE_MODE_PREVIEW:
    case CT_PROFILE_MODE_VIDEO:
        switch(cap_in.out_format){
        case CT_FORMAT_PURE:
            if(dump_time > 0)
                ret = _cmd_preview_recap(cap_in.profile_mode, cap_in.out_format, dump_time);
            else
                ret = _cmd_preview_recap(cap_in.profile_mode, cap_in.out_format);
            break;
        case CT_FORMAT_PROC:
            ret = _cmd_preview_recap(cap_in.profile_mode, cap_in.out_format);
            break;
        case CT_FORMAT_JPEG:
            ret = _cmd_preview_recap(cap_in.profile_mode, cap_in.out_format);
            break;
        }
        break;
    }

    system_call("sync");
    usleep(1000*1000);

    return ret;
}

MINT32 _unPackRawImg(MUINT8 *a_pPackedImgBuf, MUINT8 *a_pUnPackedImgBuf,  MUINT32 a_u4ImgSize, MUINT32 a_u4Width, MUINT32 a_u4Height, MUINT8 a_bitdepth, MINT32 a_u4Stride = -1)
{
    int width = a_u4Width, bitdepth = a_bitdepth, height = a_u4Height;
    int stride = 0;

    if ((a_pPackedImgBuf== NULL) || (a_pUnPackedImgBuf== NULL)) {
        return 1;
    }

    if(a_u4Stride > 0) {
        stride = a_u4Stride;
    }
    else if(bitdepth == 10) {
        if((width * bitdepth) & 0x7) {
            stride = (width * bitdepth + 7) >> 3;
            stride = ((7 + stride) >> 3) << 3;  //for 8 alignment
        }
        else {
            stride = (width * bitdepth) >> 3;
        }
    }
    else if(bitdepth == 12) {
        stride = (width * bitdepth + 7) >> 3;
        stride = ((5 + stride)/6) * 6;
    }
    CAMTOOL_LOGD("stride=%d", stride );


    int w_div4 = width >> 2;
    int w_res4 = width & 3;
    unsigned char byte0, byte1, byte2, byte3, byte4;
    unsigned char *psrc = (unsigned char *)a_pPackedImgBuf;
    unsigned short *pdst = (unsigned short *)a_pUnPackedImgBuf;

    if(bitdepth == 8) {
        MUINT16 pix;

        for(int i=0;i<(int)a_u4ImgSize; i++) {
            pix = *(psrc++);
            *(pdst++) = (pix << 2);
        }
    }
    else if(bitdepth == 10) {
        for(int h=0;h<height; h++) {

            MUINT8 *lineBuf = psrc + h * stride;

            for(int w=0;w<w_div4; w++) {
                byte0 = (MUINT8)(*(lineBuf++));
                byte1 = (MUINT8)(*(lineBuf++));
                byte2 = (MUINT8)(*(lineBuf++));
                byte3 = (MUINT8)(*(lineBuf++));
                byte4 = (MUINT8)(*(lineBuf++));

                *(pdst++) = (unsigned short) (byte0 + ((byte1 & 0x3) << 8));
                *(pdst++) = (unsigned short) (((byte1 & 0xFC) >> 2) + ((byte2 & 0xf) << 6));
                *(pdst++) = (unsigned short) (((byte2 & 0xf0) >> 4) + ((byte3 & 0x3f) << 4));
                *(pdst++) = (unsigned short) (((byte3 & 0xc0) >> 6) + (byte4 << 2));

            }


            // process last pixel in the width
            if(w_res4 != 0) {
                byte0 = *(lineBuf++);
            byte1 = *(lineBuf++);
            byte2 = *(lineBuf++);
            byte3 = *(lineBuf++);
          byte4 = *(lineBuf++);

                for(int w=0;w<w_res4; w++) {
                    switch(w) {
                        case 0:
                            *(pdst++) = (unsigned short) (byte0 + ((byte1 & 0x3) << 8));
                            break;
                        case 1:
                            *(pdst++) = (unsigned short) (((byte1 & 0x3F) >> 2) + ((byte2 & 0xf) << 6));
                            break;
                        case 2:
                            *(pdst++) = (unsigned short) (((byte2 & 0xf0) >> 4) + ((byte3 & 0x3f) << 6));
                            break;
                        case 3:
                            *(pdst++) = (unsigned short) (((byte3 & 0xc0) >> 6) + (byte4 << 2));
                            break;
                    }
                }
            }

        }
    }
    else if(bitdepth == 12) {
        //N.A.
        return 1;
    }

    return 0;

}
MINT32 _chk_pure_raw(CCT_CAPTURE_IN cap_in, MUINT32 *pRepSize, MUINT8 **pOutBuf, int unpacked_raw)
{
    int ret = 0;

    *pRepSize = 0;
    if(*pOutBuf){
        free(*pOutBuf);
        *pOutBuf = NULL;
    }

    while(1) {
        char checkFile[FLEN];
        ret = _find_file_latest(checkFile, "/mnt/vendor/cct", "pure__", NULL, NULL); //system_call("test -e /mnt/vendor/cct/pure__*");
        if(!ret)
        {
            FILE *pipeOut;

            // 1.
            CAMTOOL_LOGD("Get Raw Width/Height/BitDepth/ColorOrder" );
            int rawWidth = 0;
            int rawHeight = 0;
            int rawBitDepth = 0;
            int rawColorOrder = 0;
            if(( pipeOut = popen("echo /mnt/vendor/cct/pure__* | sed -r 's/^.+\\///' | sed 's/_/ /g;s/x/ /g;s/[^0-9 ]*//g;'", "r") ))
            {
                auto getValue = [] (int &value, FILE *fp) -> void
                {
                    int c;
                    int digFlag = 0;
                    do
                    {
                        c = fgetc(fp);
                        if( c >= '0' && c <= '9' )
                        {
                            value = 10*value + (c - '0');
                            digFlag = 1;
                        }
                        else if( digFlag )
                        {
                            break;
                        }
                    } while (c != EOF);
                };

                getValue(rawWidth, pipeOut);
                getValue(rawHeight, pipeOut);
                getValue(rawBitDepth, pipeOut);
                getValue(rawColorOrder, pipeOut);

                fclose(pipeOut);

                CAMTOOL_LOGD("Width(%d), Height(%d), BitDepth(%d), ColorOrder(%d)", rawWidth, rawHeight, rawBitDepth, rawColorOrder );

                if(rawWidth==0 || rawHeight==0 || rawBitDepth==0)
                {
                    CAMTOOL_LOGE("Width/Height/BitDepth should not be ZERO" );
                    ret = -1;
                    break;
                }
            }
            else
            {
                CAMTOOL_LOGE("Failed to Get Raw Width/Height/BitDepth/ColorOrder" );
                ret = -1;
                break;
            }

            //2.
            CAMTOOL_LOGD("Get Packed Raw data" );
            int packedSize = 0;
            MUINT8 *packedBuf = NULL;
            if(( pipeOut = popen("echo /mnt/vendor/cct/pure__*", "r") ))
            {
                // get file path
                char path[1024];
                fgets(path, sizeof(path)-1, pipeOut);
                for(int i = 0; i< 1024; i++)
                {
                    if( path[i] < 32 || path[i] > 126 )
                    {
                        path[i] = 0;
                        break;
                    }
                }

                FILE *packedFile = fopen(path, "rb");
                if(packedFile)
                {
                    // get raw data size
                    fseek(packedFile, 0, SEEK_END);
                    packedSize = ftell(packedFile);
                    CAMTOOL_LOGD("Raw Size: Packed=(%d, %d)", (rawWidth * rawHeight * rawBitDepth)/8, packedSize );

                    // get raw data
                    fseek(packedFile, 0, SEEK_SET);
                    packedBuf = (MUINT8*)malloc(packedSize);
                    if(packedBuf)
                    {
                        int readSize = fread( (void*)packedBuf, 1, packedSize, packedFile);

                        if( packedSize != readSize ){
                            CAMTOOL_LOGE("read file size wrong=%d, %d", packedSize, readSize );
                            ret = -1;
                        }

                        fclose(packedFile);

                    }
                    else
                    {
                        CAMTOOL_LOGE("packed raw data allocate failed");
                        fclose(packedFile);
                        ret = -1;
                        break;
                    }
                }
                else
                {
                    CAMTOOL_LOGE("File open failed: %s", path );
                    ret = -1;
                    break;
                }
            }
            else
            {
                CAMTOOL_LOGE("Failed to Get Packed Raw File" );
                ret = -1;
                break;
            }

            //2.1
            int packedStride = -1;
            if( (rawWidth * rawHeight * rawBitDepth)/8 != packedSize )
            {
                packedStride = packedSize / rawHeight;
                CAMTOOL_LOGD("Get Packed Raw Stride=%d", packedStride );
            }

            //3.
            CAMTOOL_LOGD("Unpack Raw data" );
            int rawFileSize = rawWidth * rawHeight * 2;
            *pRepSize = sizeof(CCT_CAPTURE_OUT) + rawFileSize;
            *pOutBuf = (MUINT8*)malloc(*pRepSize);
            MUINT8 *unpackedBuf = (MUINT8*)(*pOutBuf + sizeof(CCT_CAPTURE_OUT));
            if( *pOutBuf == NULL )
            {
                CAMTOOL_LOGE("Failed to allocate pOutBuf" );
                *pRepSize = 0;
                ret = -1;
                break;
            }
            else if(unpacked_raw)
            {
                CAMTOOL_LOGD("Copy for unpacked raw: =size(%d, %d)", packedSize, rawFileSize );
                memcpy(unpackedBuf, packedBuf, rawFileSize);
            }
            else if( _unPackRawImg(packedBuf, unpackedBuf, packedSize, rawWidth, rawHeight, rawBitDepth, packedStride) )
            {
                CAMTOOL_LOGE("Failed to unpack raw" );
                *pRepSize = 0;
                free(*pOutBuf);
                *pOutBuf = NULL;
                ret = -1;
                break;
            }

            //4.
            CAMTOOL_LOGD("Set CCT_CAPTURE_OUT");
            if( ret == 0 )
            {
                CCT_CAPTURE_OUT *t_cap_para = (CCT_CAPTURE_OUT*)*pOutBuf;
                t_cap_para->width = rawWidth;
                t_cap_para->height = rawHeight;
                t_cap_para->size = rawFileSize;
                t_cap_para->bayerPattern = rawColorOrder;
                t_cap_para->bitsNum = rawBitDepth;
                t_cap_para->out_format = cap_in.out_format;
                CAMTOOL_LOGD("Set CCT_CAPTURE_OUT=(%d, %d, %d, %d, %d, %d)",
                                t_cap_para->width,
                                t_cap_para->height,
                                t_cap_para->size,
                                t_cap_para->bayerPattern,
                                t_cap_para->bitsNum,
                                t_cap_para->out_format );
            }
            else
            {
                CAMTOOL_LOGE("Failed to set CCT_CAPTURE_OUT");
                ret = -1;
            }

            break;
        }

        CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
        usleep(500*1000);
    }
    return ret;
}
MINT32 _chk_process_raw(CCT_CAPTURE_IN cap_in, MUINT32 *pRepSize, MUINT8 **pOutBuf)
{
    int ret = 0;

    *pRepSize = 0;
    if(*pOutBuf){
        free(*pOutBuf);
        *pOutBuf = NULL;
    }

    while(1) {
        char checkFile[FLEN];
        ret = _find_file_latest(checkFile, "/mnt/vendor/cct", "proc__", NULL, NULL); //system_call("test -e /mnt/vendor/cct/proc__*");
        if(!ret)
        {
            FILE *pipeOut;

            // 1.
            CAMTOOL_LOGD("Get Raw Width/Height/BitDepth/ColorOrder" );
            int rawWidth = 0;
            int rawHeight = 0;
            int rawBitDepth = 0;
            int rawColorOrder = 0;
            if(( pipeOut = popen("echo /mnt/vendor/cct/proc_* | sed -r 's/^.+\\///' | sed 's/_/ /g;s/x/ /g;s/[^0-9 ]*//g;'", "r") ))
            {
                auto getValue = [] (int &value, FILE *fp) -> void
                {
                    int c;
                    int digFlag = 0;
                    do
                    {
                        c = fgetc(fp);
                        if( c >= '0' && c <= '9' )
                        {
                            value = 10*value + (c - '0');
                            digFlag = 1;
                        }
                        else if( digFlag )
                        {
                            break;
                        }
                    } while (c != EOF);
                };

                getValue(rawWidth, pipeOut);
                getValue(rawHeight, pipeOut);
                getValue(rawBitDepth, pipeOut);
                getValue(rawColorOrder, pipeOut);

                fclose(pipeOut);

                CAMTOOL_LOGD("Width(%d), Height(%d), BitDepth(%d), ColorOrder(%d)", rawWidth, rawHeight, rawBitDepth, rawColorOrder );

                if(rawWidth==0 || rawHeight==0 || rawBitDepth==0)
                {
                    CAMTOOL_LOGE("Width/Height/BitDepth should not be ZERO" );
                    ret = -1;
                    break;
                }
            }
            else
            {
                CAMTOOL_LOGE("Failed to Get Raw Width/Height/BitDepth/ColorOrder" );
                ret = -1;
                break;
            }

            //2.
            CAMTOOL_LOGD("Get Packed Raw data" );
            int packedSize = 0;
            MUINT8 *packedBuf = NULL;
            if(( pipeOut = popen("echo /mnt/vendor/cct/proc_*", "r") ))
            {
                // get file path
                char path[1024];
                fgets(path, sizeof(path)-1, pipeOut);
                for(int i = 0; i< 1024; i++)
                {
                    if( path[i] < 32 || path[i] > 126 )
                    {
                        path[i] = 0;
                        break;
                    }
                }

                FILE *packedFile = fopen(path, "rb");
                if(packedFile)
                {
                    // get raw data size
                    fseek(packedFile, 0, SEEK_END);
                    packedSize = ftell(packedFile);
                    CAMTOOL_LOGD("Raw Size: Packed=(%d, %d)", (rawWidth * rawHeight * rawBitDepth)/8, packedSize );


                    // get raw data
                    fseek(packedFile, 0, SEEK_SET);
                    packedBuf = (MUINT8*)malloc(packedSize);
                    if(packedBuf)
                    {
                        int readSize = fread( (void*)packedBuf, 1, packedSize, packedFile);

                        if( packedSize != readSize ){
                            CAMTOOL_LOGE("read file size wrong=%d, %d", packedSize, readSize );
                            ret = -1;
                        }

                        fclose(packedFile);

                    }
                    else
                    {
                        CAMTOOL_LOGE("packed raw data allocate failed");
                        fclose(packedFile);
                        ret = -1;
                        break;
                    }
                }
                else
                {
                    CAMTOOL_LOGE("File open failed: %s", path );
                    ret = -1;
                    break;
                }
            }
            else
            {
                CAMTOOL_LOGE("Failed to Get Packed Raw File" );
                ret = -1;
                break;
            }

            //2.1
            int packedStride = -1;
            if( (rawWidth * rawHeight * rawBitDepth)/8 != packedSize )
            {
                packedStride = packedSize / rawHeight;
                CAMTOOL_LOGD("Get Packed Raw Stride=%d", packedStride );
            }

            //3.
            CAMTOOL_LOGD("Unpack Raw data" );
            int rawFileSize = rawWidth * rawHeight * 2;
            *pRepSize = sizeof(CCT_CAPTURE_OUT) + rawFileSize;
            *pOutBuf = (MUINT8*)malloc(*pRepSize);
            MUINT8 *unpackedBuf = (MUINT8*)(*pOutBuf + sizeof(CCT_CAPTURE_OUT));
            if( *pOutBuf == NULL )
            {
                CAMTOOL_LOGE("Failed to allocate pOutBuf" );
                *pRepSize = 0;
                ret = -1;
                break;
            }
            else if( _unPackRawImg(packedBuf, unpackedBuf, packedSize, rawWidth, rawHeight, rawBitDepth, packedStride) )
            {
                CAMTOOL_LOGE("Failed to unpack raw" );
                *pRepSize = 0;
                free(*pOutBuf);
                *pOutBuf = NULL;
                ret = -1;
                break;
            }

            //4.
            CAMTOOL_LOGD("Set CCT_CAPTURE_OUT");
            if( ret == 0 )
            {
                CCT_CAPTURE_OUT *t_cap_para = (CCT_CAPTURE_OUT*)*pOutBuf;
                t_cap_para->width = rawWidth;
                t_cap_para->height = rawHeight;
                t_cap_para->size = rawFileSize;
                t_cap_para->bayerPattern = rawColorOrder;
                t_cap_para->bitsNum = rawBitDepth;
                t_cap_para->out_format = cap_in.out_format;
                CAMTOOL_LOGD("Set CCT_CAPTURE_OUT=(%d, %d, %d, %d, %d, %d)",
                                t_cap_para->width,
                                t_cap_para->height,
                                t_cap_para->size,
                                t_cap_para->bayerPattern,
                                t_cap_para->bitsNum,
                                t_cap_para->out_format );
            }
            else
            {
                CAMTOOL_LOGE("Failed to set CCT_CAPTURE_OUT");
                ret = -1;
            }

            break;
        }

        CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
        usleep(500*1000);
    }
    return ret;
}

int _get_short_int(MUINT8* p)
{
    int v = *p;
    v <<= 8;
    v += *(p+1);
    return v;
}
int _parse_jpeg_w_h(MUINT8 *buf, int buf_size, int &w, int &h)
{
    int i=0;
    if(buf[i] == 0xff && buf[i+1] == 0xd8 && buf[i+2] == 0xff && buf[i+3] == 0xe1)
    {
        i += 4;
        int size = _get_short_int(buf+i);
        i += size;

        CAMTOOL_LOGD("[EXIF PARSE] 0xFFD8");
        CAMTOOL_LOGD("[EXIF PARSE] 0xFFE1, size=%d", size);
    }
    for(; i<buf_size; )
    {
        if(buf[i] == 0xff && buf[i+1] == 0xc0)
        {
            w = _get_short_int(buf+i+5);
            h = _get_short_int(buf+i+7);

            CAMTOOL_LOGD("[EXIF PARSE] width=%d, height=%d", w, h);
            break;
        }
        else if(buf[i] == 0xff) // && ( buf[i+1] == 0xe5 || buf[i+1] == 0xe6 || buf[i+1] == 0xe7 || buf[i+1] == 0xe8)
        {
            i += 2;
            int size = _get_short_int(buf+i);
            i += size;

            CAMTOOL_LOGD("[EXIF PARSE] 0xFF%X, size=%d", buf[i+1], size);
            continue;
        }
        else
        {
#if 1
            //warning
            static int cnt = 0;
            if(cnt%1000==0){
                CAMTOOL_LOGD("[EXIF PARSE] stepping");
                cnt++;
            }
#endif
            i++;
        }
    }
    return 1;
}
MINT32 _chk_jpeg(CCT_CAPTURE_IN cap_in, MUINT32 *pRepSize, MUINT8 **pOutBuf)
{
    int ret = 0;

    *pRepSize = 0;
    if(*pOutBuf){
        free(*pOutBuf);
        *pOutBuf = NULL;
    }

    while(1) {
        char checkFile[FLEN];
        ret = _find_file_match(checkFile, "/mnt/vendor/cct", "acdkCap.jpg"); //system_call("test -e /mnt/vendor/cct/acdkCap.jpg");
        if(ret)
        {
            CAMTOOL_LOGD("ret=%d, wait a moment..", ret);
            usleep(500*1000);
        }
        else
        {
            //1.
            CAMTOOL_LOGD("Get JPEG File" );
            int jpegWidth = 0;
            int jpegHeight = 0;
            int jpegFileSize = 0;
            FILE *jpegFile = fopen("/mnt/vendor/cct/acdkCap.jpg", "rb");
            if(jpegFile)
            {
                // get jpeg size
                fseek(jpegFile, 0, SEEK_END);
                jpegFileSize = ftell(jpegFile);
                CAMTOOL_LOGD("Jpeg Size: %d", jpegFileSize );

                // allocate output buffer
                *pRepSize = sizeof(CCT_CAPTURE_OUT) + jpegFileSize;
                *pOutBuf = (MUINT8*)malloc(*pRepSize);
                if( *pOutBuf == NULL )
                {
                    CAMTOOL_LOGE("Failed to allocate pOutBuf" );
                    *pRepSize = 0;
                    ret = -1;
                    break;
                }

                // get jpeg data
                fseek(jpegFile, 0, SEEK_SET);
                MUINT8 *jpegBuf = (MUINT8*)(*pOutBuf + sizeof(CCT_CAPTURE_OUT));
                int readSize = fread( (void*)jpegBuf, 1, jpegFileSize, jpegFile);
                fclose(jpegFile);

                if( jpegFileSize != readSize ){
                    CAMTOOL_LOGE("read file size wrong=%d, %d", jpegFileSize, readSize );
                    ret = -1;
                    break;
                }

                // get width/ height
                _parse_jpeg_w_h(jpegBuf, jpegFileSize, jpegWidth, jpegHeight);
            }
            else
            {
                CAMTOOL_LOGE("File open failed" );
                ret = -1;
                break;
            }


            //2.
            CAMTOOL_LOGD("Set CCT_CAPTURE_OUT");
            if( ret == 0 )
            {
                CCT_CAPTURE_OUT *t_cap_para = (CCT_CAPTURE_OUT*)*pOutBuf;
                t_cap_para->width = jpegWidth;
                t_cap_para->height = jpegHeight;
                t_cap_para->size = jpegFileSize;
                t_cap_para->bayerPattern = 0;
                t_cap_para->bitsNum = 0;
                t_cap_para->out_format = cap_in.out_format;
                CAMTOOL_LOGD("Set CCT_CAPTURE_OUT=(%d, %d, %d, %d, %d, %d)",
                                t_cap_para->width,
                                t_cap_para->height,
                                t_cap_para->size,
                                t_cap_para->bayerPattern,
                                t_cap_para->bitsNum,
                                t_cap_para->out_format );
            }
            else
            {
                CAMTOOL_LOGE("Failed to set CCT_CAPTURE_OUT");
                ret = -1;
            }

            break;
        }
    }

    return ret;
}

MINT32 checkCapture(CCT_CAPTURE_IN cap_in, MUINT32 *pRepSize, MUINT8 **pOutBuf, int unpacked_raw)
{
    CAMTOOL_LOGD("+" );
    int ret = 0;

    *pRepSize = 0;
    if(*pOutBuf)
        free(*pOutBuf);
    *pOutBuf = 0;

    switch(cap_in.out_format){
    case CT_FORMAT_PURE:
        ret = _chk_pure_raw(cap_in, pRepSize, pOutBuf, unpacked_raw);
        break;
    case CT_FORMAT_PROC:
        ret = _chk_process_raw(cap_in, pRepSize, pOutBuf);
        break;
    case CT_FORMAT_JPEG:
        switch(cap_in.profile_mode){
        case CT_PROFILE_MODE_CAPTURE:
            ret = _chk_jpeg(cap_in, pRepSize, pOutBuf);
            break;
        case CT_PROFILE_MODE_PREVIEW:
        case CT_PROFILE_MODE_VIDEO:
            //ret = _chk_yv12(...);
            ret = _chk_jpeg(cap_in, pRepSize, pOutBuf);
            break;
        }
        break;
    }

    return ret;
}

MINT32 WriteCctOp( MUINT32 Op, MUINT32 Size, MINT32 fd)
{
    MUINT32 temp[2];
    temp[0] = Op;
    temp[1] = Size;

    if ( write(fd, temp, sizeof(temp)) != sizeof(temp) )
        return -1;

    return 0;
}

// In or Out definition is based on the point of view of CCT server
MINT32 WriteCctInData( MUINT32 Size, MUINT8 *pInBuf, MUINT32 *pRetSize, MINT32 fd)
{
    MINT32 wrSize, accWrSize;

    accWrSize = 0;
    wrSize = 0;
    while (accWrSize < (int)Size && wrSize >= 0) {
        wrSize = write(fd, (pInBuf+accWrSize), (Size-accWrSize));
        if (wrSize > 0)
            accWrSize += wrSize;
    }

    *pRetSize = accWrSize;

    if (accWrSize != (int)Size || wrSize < 0)
        return -1;

    return 0;   //Size;
}

MINT32 ReadCctOutStatus( MUINT32 *pOp, MUINT32 *pStatus, MUINT32 *pSize, MINT32 fd)
{
    MUINT32 temp[3];
    MINT32 len;

    len = read(fd, temp, sizeof(temp));
    if (len < 0)
        return -1;

    if (len == sizeof(temp)) {
        *pOp = temp[0];
        *pStatus = temp[1];
        *pSize = temp[2];
    }

    return len;
}

MINT32 ReadCctOutData( MUINT32 Size, MUINT8 *pOutBuf, MINT32 fd)
{
    MINT32 rdSize, accRdSize;
    MINT32 blockSize, err, retryCount;

    accRdSize = 0;
    rdSize = 0;
    blockSize = 32768;
    retryCount = 0;
    while (accRdSize < (int)Size && rdSize >= 0) {
        if (blockSize > ((int)Size-accRdSize))
            blockSize = ((int)Size-accRdSize);

        rdSize = read(fd, (pOutBuf+accRdSize), blockSize);
        CAMTOOL_LOGD("Read CCT reply data rdSize %d  accSize%x",rdSize,accRdSize);

        if (rdSize > 0) {
            accRdSize += rdSize;
            retryCount = 0;
        } else if (rdSize < 0) {
            err = errno;
            CAMTOOL_LOGD("Read CCT reply data error %d  accSize%x",err,accRdSize);
            retryCount++;
            if (err == 11 && retryCount<300)
                rdSize = 0;
        } else {
            usleep(1000);
            retryCount++;
            if (retryCount > 1000)
                rdSize = -1;
        }
    }

    if (rdSize < 0 || accRdSize != (int)Size)
        return -1;

    return accRdSize;   //Size;
}

MINT32 ReadCctOutStatusOrCapture( MUINT32 *pOp, MUINT32 *pStatus, MUINT32 *pSize, MINT32 fdin, MINT32 fdout)
{
    MINT32 len;

    while ((len = ReadCctOutStatus(pOp, pStatus, pSize, fdin)) > 0 )
    {
        if( *pOp == 0xF1F1 )
        {
            int state;
            CCT_CAPTURE_CALLBACK cb_param;

            if( (int)(*pSize) != sizeof(CCT_CAPTURE_CALLBACK) )
            {
                CAMTOOL_LOGE("wrong size of CCT_CAPTURE_CALLBACK");
                return -5;
            }

            state = ReadCctOutData( *pSize, (MUINT8*)&cb_param, fdin);

            if ( state < 0 ) {
                CAMTOOL_LOGE("Read FIFO reply data failure");
                return -6;
            } else if ( state != (int)(*pSize) ) {
                CAMTOOL_LOGE("Read FIFO reply data incomplete");
                return -7;
            }

            CAMTOOL_LOGD("callback capture parameter: out_format=%d, profile_mode=%d, dump_time=%d", cb_param.out_format, cb_param.profile_mode, cb_param.dump_time);

            CCT_CAPTURE_IN cap_in;
            cap_in.out_format   = (CT_OUT_FORMAT_T) cb_param.out_format;
            cap_in.profile_mode = (CT_PROFILE_MODE_T) cb_param.profile_mode;
            cap_in.feature_mode = CT_FEATURE_MODE_NONE;

            int unpacked_raw = 0;
            captureCommand(cap_in, unpacked_raw, cb_param.dump_time);

            if ( WriteCctOp( 0xE2E2, 0, fdout ) == -1 ) {
                CAMTOOL_LOGE("Write FIFO op failure");
                return -2;
            }
        }
        else
        {
            break;
        }
    }

    return len;
}

// mode = 0 ... input
//      = 1 ... output
MINT32 WaitForCctDataReady( MINT32 mode, MINT32 fd )
{
    fd_set fset;
    struct timeval tout;

    tout.tv_sec = 5;  // sec
    tout.tv_usec = 0;   // usec

    FD_ZERO( &fset );
    FD_SET( fd, &fset );

    if ( mode == 0 ) { // for input
        if( select( 1, &fset, NULL, NULL, &tout ) < 0 )
            return -1;
    } else { // for output
        if( select( 1, NULL, &fset, NULL, &tout ) < 0 )
            return -1;
    }
    if ( FD_ISSET( fd, &fset ) )
        return 0;
    else
        return -1;
}

/*******************************************************************************/

enum
{
    FT_CCT_OP_CAPTURE_START = 0x0000,
    FT_CCT_OP_SENSOR_START  = 0x1000,
    FT_CCT_OP_3A_START      = 0x2000,
    FT_CCT_OP_ISP_START     = 0x3000,
    FT_CCT_OP_NVRAM_START   = 0x4000,
    FT_CCT_OP_CAL_START     = 0x5000,
    FT_CCT_OP_SHELL_START   = 0x6000,
    FT_CCT_OP_INTERNAL_START= 0x7000
};

typedef enum
{
    CT_CMD_OP_STOP_EXEC = 0xFF01


    ,FT_CCT_OP_CAMERA_CAPTURE = FT_CCT_OP_CAPTURE_START
    ,FT_CCT_OP_SET_PROFILE_MODE
    ,FT_CCT_OP_GET_PROFILE_MODE
    ,FT_CCT_OP_SET_FEATURE_MODE
    ,FT_CCT_OP_GET_FEATURE_MODE
    ,FT_CCT_OP_SET_SENSOR_MODE

    ,FT_CCT_OP_GET_SENSOR = FT_CCT_OP_SENSOR_START
    ,FT_CCT_OP_SWITCH_SENSOR
    ,FT_CCT_OP_SET_SENSOR_REG
    ,FT_CCT_OP_GET_SENSOR_REG
    ,FT_CCT_OP_LSC_GET_SENSOR_RESOLUTION
    ,FT_CCT_OP_GET_SENSOR_MODE_NUM
    ,FT_CCT_OP_SET_PROP
    ,FT_CCT_OP_GET_PROP
    ,FT_CCT_OP_SENSOR_TYPE_MAX

    ,FT_CCT_OP_AE_GET_ON_OFF = FT_CCT_OP_3A_START
    ,FT_CCT_OP_AE_SET_ON_OFF
    ,FT_CCT_OP_AE_GET_BAND
    ,FT_CCT_OP_AE_SET_BAND
    ,FT_CCT_OP_AE_GET_METERING_MODE
    ,FT_CCT_OP_AE_SET_METERING_MODE
    ,FT_CCT_OP_AE_GET_SCENE_MODE
    ,FT_CCT_OP_AE_SET_SCENE_MODE
    ,FT_CCT_OP_AE_GET_AUTO_PARA
    ,FT_CCT_OP_AE_SET_AUTO_PARA
    ,FT_CCT_OP_AE_GET_CAPTURE_PARA
    ,FT_CCT_OP_AE_SET_CAPTURE_PARA
    ,FT_CCT_OP_AF_GET_RANGE
    ,FT_CCT_OP_AF_GET_POS
    ,FT_CCT_OP_AF_SET_POS
    ,FT_CCT_OP_AWB_GET_ON_OFF
    ,FT_CCT_OP_AWB_SET_ON_OFF
    ,FT_CCT_OP_AWB_GET_LIGHT_PROB
    ,FT_CCT_OP_AWB_GET_MODE
    ,FT_CCT_OP_AWB_SET_MODE
    ,FT_CCT_OP_AWB_GET_GAIN
    ,FT_CCT_OP_AWB_SET_GAIN
    ,FT_CCT_OP_FLASH_GET_MODE
    ,FT_CCT_OP_FLASH_SET_MODE
    ,FT_CCT_OP_3A_TYPE_MAX

    ,FT_CCT_OP_GET_ID    = FT_CCT_OP_ISP_START
    ,FT_CCT_OP_ISP_GET_ON_OFF
    ,FT_CCT_OP_ISP_SET_ON_OFF
    ,FT_CCT_OP_ISP_GET_CCM_FIXED_ON_OFF
    ,FT_CCT_OP_ISP_SET_CCM_FIXED_ON_OFF
    ,FT_CCT_OP_ISP_GET_CCM_MATRIX
    ,FT_CCT_OP_ISP_SET_CCM_MATRIX
    ,FT_CCT_OP_ISP_GET_INDEX
    ,FT_CCT_OP_ISP_GET_SMOOTH_CCM
    ,FT_CCT_OP_ISP_SET_SMOOTH_CCM
    ,FT_CCT_OP_GET_SHADING_ON_OFF
    ,FT_CCT_OP_SET_SHADING_ON_OFF
    ,FT_CCT_OP_GET_SHADING_INDEX
    ,FT_CCT_OP_SET_SHADING_INDEX
    ,FT_CCT_OP_GET_SHADING_TSF_ON_OFF
    ,FT_CCT_OP_SET_SHADING_TSF_ON_OFF
    ,FT_CCT_OP_ISP_TYPE_MAX

    ,FT_CCT_OP_ISP_GET_NVRAM_DATA = FT_CCT_OP_NVRAM_START
    ,FT_CCT_OP_ISP_SET_NVRAM_DATA
    ,FT_CCT_OP_ISP_SET_PARTIAL_NVRAM_DATA
    ,FT_CCT_OP_ISP_SAVE_NVRAM_DATA
    ,FT_CCT_OP_NVRAM_TYPE_MAX

    ,FT_CCT_OP_CAL_CDVT_CALIBRATION = FT_CCT_OP_CAL_START
    ,FT_CCT_OP_CAL_CDVT_TEST
    ,FT_CCT_OP_CAL_SHADING
    ,FT_CCT_OP_CAL_AE_GET_FALRE
    ,FT_CCT_OP_CAL_AE_GET_EV
    ,FT_CCT_OP_CAL_FLASH
    ,FT_CCT_OP_CAL_TYPE_MAX

    ,FT_CCT_OP_ISP_EXEC_SHELL_CMD = FT_CCT_OP_SHELL_START

    //,FT_CCT_OP_AE_SET_AUTO_EXPOSURE_ON_OFF = FT_CCT_OP_INTERNAL_START
    //,FT_CCT_OP_AF_SET_ON_OFF
    //,FT_CCT_OP_GET_SHADING_PARA
    //,FT_CCT_OP_SET_SHADING_TABLE_POLYCOEF
    //,FT_CCT_OP_INTERNAL_TYPE_MAX

    ,FT_CCT_OP_END
} CCT_OP_ID;


MINT32
CamTool::
ReOpenFifo()
{
    CloseFifo(m_iFdIn, (char*)PIPE_IN_FNAME);
    CloseFifo(m_iFdOut, (char*)PIPE_OUT_FNAME);
    usleep(100*1000);
    CAMTOOL_LOGD("re-open FIFO.");

    if ( OpenFifo(&m_iFdOut, (char*)PIPE_OUT_FNAME, LNX_FIFO_OUT_MODE) == -1 ) {
        CAMTOOL_LOGE("Open Output FIFO failure ");
        return -1;
    }
    CAMTOOL_LOGD("Open Output FIFO success");
    if ( OpenFifo(&m_iFdIn, (char*)PIPE_IN_FNAME, LNX_FIFO_IN_MODE) == -1 ) {
        CAMTOOL_LOGE("Open Input FIFO failure ");
        CloseFifo(m_iFdOut, (char*)PIPE_OUT_FNAME);
        return -1;
    }
    CAMTOOL_LOGD("Open Input FIFO success");
    return 0;
}

MINT32
CamTool::
changeSensor(CT_SENSOR_T sensor_type)
{
    int ret = 0;

    char cct_sensor_index[10];
    snprintf(cct_sensor_index, 10, "%d", 1 << (sensor_type - 1) );
    set_vendor_property("vendor.3a.cct.sensor.index", cct_sensor_index);

    if(sensor_type == 1)
    {
    CAMTOOL_LOGD("camera re-start");
    system_call("am force-stop com.mediatek.camera");
    usleep(1000*1000);
    system_call("am start com.mediatek.camera");
    usleep(2*1000*1000);
    }
    //set_vendor_property("mtk.camera.switch.camera.debug", "1");
    switch(sensor_type){
        default:
        case CT_SENSOR_MAIN:
            set_vendor_property("mtk.camera.switch.id.debug", "back-0");
            break;
        case CT_SENSOR_SUB:
            set_vendor_property("mtk.camera.switch.id.debug", "front-0");
            break;
        case CT_SENSOR_MAIN2:
            set_vendor_property("mtk.camera.switch.id.debug", "back-1");
            break;
        case CT_SENSOR_SUB2:
            set_vendor_property("mtk.camera.switch.id.debug", "front-1");
            break;
        case CT_SENSOR_MAIN3:
            set_vendor_property("mtk.camera.switch.id.debug", "back-2");
            break;
        case CT_SENSOR_SUB3:
            set_vendor_property("mtk.camera.switch.id.debug", "front-2");
            break;
        case CT_SENSOR_MAIN4:
            set_vendor_property("mtk.camera.switch.id.debug", "back-3");
            set_vendor_property("vendor.3a.cct.sensor.index", "32");
            break;
        case CT_SENSOR_SUB4:
            set_vendor_property("mtk.camera.switch.id.debug", "front-3");
            break;
        case CT_SENSOR_MAIN5:
            set_vendor_property("mtk.camera.switch.id.debug", "back-4");
            break;
        case CT_SENSOR_SUB5:
            set_vendor_property("mtk.camera.switch.id.debug", "front-4");
            break;
        case CT_SENSOR_MAIN6:
            set_vendor_property("mtk.camera.switch.id.debug", "back-5");
            break;
        case CT_SENSOR_SUB6:
            set_vendor_property("mtk.camera.switch.id.debug", "front-5");
            break;
        case CT_SENSOR_MAIN7:
            set_vendor_property("mtk.camera.switch.id.debug", "back-6");
            break;
        case CT_SENSOR_SUB7:
            set_vendor_property("mtk.camera.switch.id.debug", "front-6");
            break;
        case CT_SENSOR_MAIN8:
            set_vendor_property("mtk.camera.switch.id.debug", "back-7");
            break;
        case CT_SENSOR_SUB8:
            set_vendor_property("mtk.camera.switch.id.debug", "front-7");
            break;
        case CT_SENSOR_MAIN9:
            set_vendor_property("mtk.camera.switch.id.debug", "back-8");
            break;
        case CT_SENSOR_SUB9:
            set_vendor_property("mtk.camera.switch.id.debug", "front-8");
            break;
        case CT_SENSOR_MAIN10:
            set_vendor_property("mtk.camera.switch.id.debug", "back-9");
            break;
        case CT_SENSOR_SUB10:
            set_vendor_property("mtk.camera.switch.id.debug", "front-9");
            break;
    }
    if(sensor_type != 1)
    {
    system_call("input keyevent KEYCODE_C"); //switch sensor
    usleep(2000*1000);
    }
    CAMTOOL_LOGD("disable ZSD");
    system_call("input keyevent KEYCODE_E"); //ZSD off
    usleep(1000*1000);

#if 1
    if ( 0 != ReOpenFifo() ) {
        return -2;
    }
#endif

    return ret;
}
MINT32
CamTool::
changeProfileMode(CT_PROFILE_MODE_T profile_mode)
{
    int ret = 0;

    switch(profile_mode){
        case CT_PROFILE_MODE_PREVIEW:
        case CT_PROFILE_MODE_CAPTURE:
            system_call("input keyevent KEYCODE_A"); //switch to photo
            break;
        case CT_PROFILE_MODE_VIDEO:
            system_call("input keyevent KEYCODE_B"); //switch to video
            break;
        default:
            break;
    }
    usleep(2000*1000);

#if 1
    if ( 0 != ReOpenFifo() ) {
        return -2;
    }
#endif
    return ret;
}

MINT32
CamTool::
changeSensorMode(CT_SENSOR_MODE_T sensor_mode)
{
    switch(sensor_mode){
        case CT_SENSOR_MODE_PREVIEW:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "0");
            break;
        case CT_SENSOR_MODE_CAPTURE:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "1");
            break;
        case CT_SENSOR_MODE_VIDEO:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "2");
            break;
        case CT_SENSOR_MODE_SLIM_VIDEO1:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "3");
            break;
        case CT_SENSOR_MODE_SLIM_VIDEO2:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "4");
            break;
        case CT_SENSOR_MODE_CUSTOM1:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "5");
            break;
        case CT_SENSOR_MODE_CUSTOM2:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "6");
            break;
        case CT_SENSOR_MODE_CUSTOM3:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "7");
            break;
        case CT_SENSOR_MODE_CUSTOM4:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "8");
            break;
        case CT_SENSOR_MODE_CUSTOM5:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "9");
            break;
        default:
            set_vendor_property("vendor.debug.cameng.force_sensormode", "");
            break;
    }
    usleep(2000*1000);
    return 0;
}


MINT32
CamTool::
changeFeatureMode(CT_FEATURE_MODE_T feature_mode)
{
    int ret = 0;

    switch(feature_mode){
        case CT_FEATURE_MODE_NONE:
            break;
        case CT_FEATURE_MODE_MFLL:
            break;
        case CT_FEATURE_MODE_MNR_HW:
            break;
        case CT_FEATURE_MODE_MNR_SW:
            break;
        case CT_FEATURE_MODE_MFLL_MNR_HW:
            break;
        case CT_FEATURE_MODE_MFLL_MNR_SW:
            break;
        default:
            break;
    }

    return ret;
}


MINT32
CamTool::
getCctCommand(FILE *pInFPtr, MUINT32 *pCmdOp, MUINT32 *pBufSize, void **pCmdBuf)
{
    MINT32 size;
    void *tempBuf = NULL;

    //if (feof(inFPtr))
    //    return 1; //end of file

    if ((size=fread(pCmdOp, 1, 4, pInFPtr))<4)
        return 1; //end of file

    if ((size=fread(pBufSize, 1, 4, pInFPtr))<4)
        return 2; //end of file

    if (*pBufSize > 0) {
        tempBuf = malloc(*pBufSize);

        if (tempBuf) {
            if ((size=fread(tempBuf, 1, *pBufSize, pInFPtr)) != (MINT32) *pBufSize) {
                free(tempBuf);
                return 3; //end of file
            }
        }
    }
    *pCmdBuf = tempBuf;

    return 0;
}


MINT32
CamTool::
doCctOperation(MUINT32 cmdOp, MUINT32 inBufSize, MUINT8 *pInBuf, MUINT32 *pRepStatus, MUINT32 *pRepSize, MUINT8 **pOutBuf)
{
    MINT32 state;
    MUINT32 WrSize;
    MUINT32 RepOp = 0;

    if (m_iFdIn == -1 || m_iFdOut == -1 ) {
        return -1;
    }
    if ( (state = WriteCctOp( cmdOp, inBufSize, m_iFdOut )) == -1 ) {
        CAMTOOL_LOGD("Write FIFO op failure");
        return -2;
    }
    if (inBufSize > 0)
        if ( (state = WriteCctInData( inBufSize, pInBuf, &WrSize, m_iFdOut )) == -1 ) {
            CAMTOOL_LOGD("Write FIFO data failure");
            return -2;
        }
    while ((state = ReadCctOutStatusOrCapture( &RepOp, pRepStatus, pRepSize, m_iFdIn, m_iFdOut )) == 0 );

    CAMTOOL_LOGD("Read Cct reply status %d size %d ", state, *pRepSize);

    if ( state < 0 ) {
        CAMTOOL_LOGD("Read FIFO reply status failure");
        return -3;
    } else if ( state != 12 ) {
        CAMTOOL_LOGD("Read FIFO reply status incomplete");
        return -4;
    }

    if ( RepOp != cmdOp ) {
        CAMTOOL_LOGD("reply not match ! Op:%x Status:%x Size:%x", RepOp, *pRepStatus, *pRepSize);
        return -5;
    }

    if (*pRepStatus != 0 ) {
        CAMTOOL_LOGD("CCT Command Status Failed!");
        return -9;
    }

    if (*pRepSize != 0 ) {
        *pOutBuf = (MUINT8 *) malloc( *pRepSize );
        if ( *pOutBuf == NULL ) {
            CAMTOOL_LOGD("Mem alloc for output data failure");
            return -8;
        }

        state = ReadCctOutData( *pRepSize, *pOutBuf, m_iFdIn );   // Need to add a timeout
        if ( state < 0 ) {
            CAMTOOL_LOGD("Read FIFO reply data failure");
            return -6;
        } else if ( state != (int)(*pRepSize) ) {
            CAMTOOL_LOGD("Read FIFO reply data incomplete");
            return -7;
        } else {
            CAMTOOL_LOGD("Read FIFO reply data complete");
        }
    }

    return 0;
}


MINT32
CamTool::
putCctOutputResult(FILE *outFPtr, MUINT32 cmdOp, MUINT32 repStatus, MUINT32 repSize, MUINT8 *outBuf)
{
    fwrite(&cmdOp, 1, sizeof(cmdOp), outFPtr);
    fwrite(&repStatus, 1, sizeof(repStatus), outFPtr);
    fwrite(&repSize, 1, sizeof(repSize), outFPtr);
    if (repSize != 0 && outBuf != NULL)
        fwrite(outBuf, repSize, sizeof(MUINT8), outFPtr);
    return 0;
}

MINT32
CamTool::
parseCommandFile(char *infname, char *outfname)
{
    FILE *inFPtr, *outFPtr;
    MUINT32 cmdOp, bufSize;
    MUINT8 *cmdBuf;
    MINT32 err;
    MUINT32 cmdStatus, cmdRepSize;
    MUINT8 *cmdOutBuf = NULL;
    MBOOL stopFlag = MFALSE;
    MBOOL reStartApkFlag = MFALSE;
    MBOOL bPIPE = MFALSE;

    if ((inFPtr = fopen(infname, "rb")) == NULL) {
        CAMTOOL_LOGE("Open CCT in file error! ");
        return -1;
    }
    if ((outFPtr = fopen(outfname, "wb")) == NULL) {
        CAMTOOL_LOGE("Open CCT out file error! ");
        fclose(inFPtr);
        return -1;
    }

    CAMTOOL_LOGD("Open CCT in/out file success ");

    cmdBuf = NULL;
    while (feof(inFPtr) == 0 && stopFlag == MFALSE) {
        if ((err = getCctCommand(inFPtr, &cmdOp, &bufSize, (void **)&cmdBuf)) == 0) {
            CAMTOOL_LOGD("[CCT In File]Cmd:%x  Size:%x",cmdOp, bufSize);
            if(!bPIPE && cmdOp != FT_CCT_OP_SWITCH_SENSOR)
            {
                if ( OpenFifo(&m_iFdOut, (char*)PIPE_OUT_FNAME, LNX_FIFO_OUT_MODE) == -1 ) {
                    CAMTOOL_LOGE("Open Output FIFO faulure ");
                    fclose(inFPtr);
                    fclose(outFPtr);
                    return -2;
                }
                CAMTOOL_LOGD("Open Output FIFO success ");

                if ( OpenFifo(&m_iFdIn, (char*)PIPE_IN_FNAME, LNX_FIFO_IN_MODE) == -1 ) {
                    CAMTOOL_LOGE("Open Input FIFO faulure ");
                    fclose(inFPtr);
                    fclose(outFPtr);
                    CloseFifo(m_iFdOut, (char*)PIPE_OUT_FNAME);
                    return -2;
                }
                CAMTOOL_LOGD("Open Input FIFO success ");
                bPIPE = MTRUE;
            }
            if ( cmdOp == FT_CCT_OP_CAL_FLASH ) {
                CAMTOOL_LOGD("Open Flash");
                system_call("input keyevent KEYCODE_F"); //Open Flash
                usleep(1000*1000);
            }
            //
            if ( cmdOp == CT_CMD_OP_STOP_EXEC ) {
                CAMTOOL_LOGD("Cmd to Stop Exec");
                stopFlag = MTRUE;
            } else if ( cmdOp == FT_CCT_OP_ISP_EXEC_SHELL_CMD ) {
                char cmdstr[CCT_SHELL_CMD_MAX_LENGTH];
                MINT32 cmdlen;

                cmdlen = strlen((char *)cmdBuf);
                if (cmdlen >= CCT_SHELL_CMD_MAX_LENGTH) {
                    cmdlen = CCT_SHELL_CMD_MAX_LENGTH - 1;
                }
                strncpy(cmdstr, (char *)cmdBuf, cmdlen );
                cmdstr[cmdlen] = 0;

                system_call(cmdstr);
            } else if ( cmdOp == FT_CCT_OP_SWITCH_SENSOR) {

                if(bufSize != 4) {
                    CAMTOOL_LOGE("wrong input size");
                    err = -1;
                }
                else {
                    CT_SENSOR_T sensor_type = *((CT_SENSOR_T*)cmdBuf);
                    err = changeSensor( sensor_type );
                    reStartApkFlag = MTRUE;
                }
                if (err != 0) {
                    CAMTOOL_LOGE("err=%d",err);
                }

                cmdStatus = err;
                cmdRepSize = 0;
                cmdOutBuf = NULL;
                putCctOutputResult(outFPtr, cmdOp, cmdStatus, cmdRepSize, cmdOutBuf);

            } else if ( cmdOp == FT_CCT_OP_SET_PROFILE_MODE) {

                if(bufSize != 4) {
                    CAMTOOL_LOGE("wrong input size");
                    err = -1;
                }
                else {
                    CT_PROFILE_MODE_T profile_mode = *((CT_PROFILE_MODE_T*)cmdBuf);
                    err = changeProfileMode( profile_mode );
                }
                if (err != 0) {
                    CAMTOOL_LOGE("err=%d",err);
                }

                cmdStatus = err;
                cmdRepSize = 0;
                cmdOutBuf = NULL;
                putCctOutputResult(outFPtr, cmdOp, cmdStatus, cmdRepSize, cmdOutBuf);

            } else if ( cmdOp == FT_CCT_OP_SET_SENSOR_MODE) {

                if(bufSize != 4) {
                    CAMTOOL_LOGE("wrong input size");
                    err = -1;
                }
                else {
                    CT_SENSOR_MODE_T sensor_mode = *((CT_SENSOR_MODE_T*)cmdBuf);
                    err = changeSensorMode( sensor_mode );
                }
                if (err != 0) {
                    CAMTOOL_LOGE("err=%d",err);
                }

                cmdStatus = err;
                cmdRepSize = 0;
                cmdOutBuf = NULL;
                putCctOutputResult(outFPtr, cmdOp, cmdStatus, cmdRepSize, cmdOutBuf);

            } else if ( cmdOp == FT_CCT_OP_SET_FEATURE_MODE) {

                if(bufSize != 4) {
                    CAMTOOL_LOGE("wrong input size");
                    err = -1;
                }
                else {
                    CT_FEATURE_MODE_T feature_mode = *((CT_FEATURE_MODE_T*)cmdBuf);
                    err = changeFeatureMode( feature_mode );
                }
                if (err != 0) {
                    CAMTOOL_LOGE("err=%d",err);
                }

                cmdStatus = err;
                cmdRepSize = 0;
                cmdOutBuf = NULL;
                putCctOutputResult(outFPtr, cmdOp, cmdStatus, cmdRepSize, cmdOutBuf);

            } else if ( cmdOp == FT_CCT_OP_CAMERA_CAPTURE ) {

                cmdRepSize = 0;
                cmdOutBuf = NULL;
                if(bufSize != sizeof(CCT_CAPTURE_IN)) {
                    CAMTOOL_LOGE("wrong input size");
                    err = -1;
                }
                else {
                    CCT_CAPTURE_IN cap_in;
                    memcpy((void*)&cap_in, (void*)cmdBuf, sizeof(CCT_CAPTURE_IN));

                    int unpacked_raw = 0;
                    err = captureCommand(cap_in, unpacked_raw, -1);

                    if(err == 0)
                    {
                        err = checkCapture(cap_in, &cmdRepSize, &cmdOutBuf, unpacked_raw);
                    }
                }
                if (err != 0) {
                    CAMTOOL_LOGE("err=%d",err);
                }

                cmdStatus = err;
                putCctOutputResult(outFPtr, cmdOp, cmdStatus, cmdRepSize, cmdOutBuf);
            } else {
                // CCT command handling
                CAMTOOL_LOGD("Cmd to doCctOperation");

                cmdOutBuf = NULL;
                cmdRepSize = 0;
                err = doCctOperation(cmdOp, bufSize, cmdBuf, &cmdStatus, &cmdRepSize, &cmdOutBuf);
                if (err != 0) {
                    CAMTOOL_LOGD("CCT command handling error! %d",err);
                    if ( cmdOutBuf != NULL ) { free(cmdOutBuf); }
                    stopFlag = MTRUE;
                } else {

                    putCctOutputResult(outFPtr, cmdOp, cmdStatus, cmdRepSize, cmdOutBuf);

                    CAMTOOL_LOGD("Op:%x  Status:%x  RSize:%x", cmdOp, cmdStatus, cmdRepSize );
                    if ( cmdRepSize != 0 && cmdOutBuf != NULL ) {
#if 0 //debug
                        MUINT32 i;
                        CAMTOOL_LOGD("Out data dump:");
                        if ( cmdRepSize > 64 )
                            cmdRepSize=64;
                        for (i=0; i< cmdRepSize; i++) {
                            CAMTOOL_LOGD("%02x ",cmdOutBuf[i]);
                            if ((i & 0x0f) == 0x0F)
                                CAMTOOL_LOGD("");
                        }
                        CAMTOOL_LOGD("");
#endif
                        free(cmdOutBuf);
                    }
                }
            }
            //
            if ( cmdOp == FT_CCT_OP_CAL_FLASH ) {
                CAMTOOL_LOGD("Close Flash");
                system_call("input keyevent KEYCODE_G"); //Close Flash
            }
            //
            if (cmdBuf != NULL)
                free(cmdBuf);
        } else {
            // file reading error or eof
            stopFlag = MTRUE;
            if ( err != 1 )
                CAMTOOL_LOGD("Get CCT command from file error %d",err);
        }
    }


    CloseFifo(m_iFdIn, (char*)PIPE_IN_FNAME);
    CloseFifo(m_iFdOut, (char*)PIPE_OUT_FNAME);
    fclose(inFPtr);
    fclose(outFPtr);

    return 0;
}

int mainFunc(int argc, char *argv[])
{
    char    InFilename[256];
    char    OutFilename[256];
    CamTool camtool;

#if 0//debug
    CAMTOOL_LOGD("camtool argc=%d", argc);
    for(int i=0; i<argc; i++)
    {
        CAMTOOL_LOGD("argv[%d]=%s", i, argv[i]);
    }
    CAMTOOL_LOGD("");
#endif

    if (argc < 2) {
        CAMTOOL_LOGE("Command Error!");
        CAMTOOL_LOGE("Command Format > camtool cmd [ arg, ... ]");
        return 1;
    }
    if ((argc > 2 && strlen(argv[2]) > 256) ||
        (argc > 3 && strlen(argv[3]) > 256))
    {
        CAMTOOL_LOGE("filename is too long");
        return 1;
    }

    if (strncmp(argv[1], "camera-on", 9) == 0) {

        MINT32 safe_mode = launchSafeMode();
        if( 0 != safe_mode )
        {
            CAMTOOL_LOGD("Launch CCT Safe Mode");
            set_vendor_property("vendor.3a.cct.enable", "1");
            system_call("am start com.mediatek.camera");
        }
        else
        {
            CAMTOOL_LOGD("enable NDD properties");
            set_vendor_property("vendor.debug.camera.ufo_off"           , "1");
            set_vendor_property("vendor.debug.camera.imgBuf.enFC"       , "1");
            set_vendor_property("vendor.debug.camera.p2.dump"           , "1");
            set_vendor_property("vendor.debug.camera.dump.JpegNode"     , "1");
            set_vendor_property("vendor.debug.camera.dump.en"           , "1");
            set_vendor_property("vendor.debug.feature.forceEnableIMGO"  , "1");
            set_vendor_property("vendor.debug.fpipe.force.img3o"        , "1");
            set_vendor_property("vendor.debug.p2f.dump.enable"          , "1"); //*-wdmao*.yv12
            set_vendor_property("vendor.debug.p2f.dump.mode"            , "1"); //*-wdmao*.yv12
            //set_vendor_property("vendor.debug.camera.cfg.ProcRaw", "1");

            CAMTOOL_LOGD("enable APK Keycode");
            set_vendor_property("mtk.camera.app.keycode.enable"         , "1");
            set_vendor_property("mtk.camera.switch.camera.debug"        , "1");

            CAMTOOL_LOGD("disable mfll");
            set_vendor_property("vendor.mfll.force"                     , "0");

            CAMTOOL_LOGD("camera start");
            set_vendor_property("vendor.3a.cct.enable"                  , "1");

            usleep(500);
            system_call("am start com.mediatek.camera");
            usleep(6*1000*1000);

            CAMTOOL_LOGD("disable ZSD");
            // disable ZSD:
            // 1. At CDVT + capture mode, AE updateCaptureParams(.) only works on non-ZSD
            // 2. ZSD capture mdoe setting from preview mode
            //system_call("input keyevent KEYCODE_D"); //ZSD on
            system_call("input keyevent KEYCODE_E"); //ZSD off

            usleep(3*1000*1000);
            CAMTOOL_LOGD("camera re-start for ProcRaw setting");
            system_call("am force-stop com.mediatek.camera");
            usleep(500*1000);

            // camera might crush if you open this property at ZSD
            set_vendor_property("vendor.debug.camera.cfg.ProcRaw",       "1");
            usleep(500);
            system_call("am start com.mediatek.camera");
            usleep(3*1000*1000);
        }

    }
    else if (strncmp(argv[1], "camera-off", 10) == 0) {

        CAMTOOL_LOGD("disable NDD properties");
        set_vendor_property("vendor.debug.camera.ufo_off", "0");
        set_vendor_property("vendor.debug.camera.imgBuf.enFC", "0");
        set_vendor_property("vendor.debug.camera.p2.dump", "0");
        set_vendor_property("vendor.debug.camera.dump.JpegNode", "0");
        set_vendor_property("vendor.debug.camera.dump.en", "0");
        set_vendor_property("vendor.debug.feature.forceEnableIMGO", "0");
        set_vendor_property("vendor.debug.fpipe.force.img3o", "0");
        set_vendor_property("vendor.debug.camera.cfg.ProcRaw", "0");
        set_vendor_property("vendor.debug.p2f.dump.enable", "0"); //*-wdmao*.yv12
        set_vendor_property("vendor.debug.p2f.dump.mode", "0"); //*-wdmao*.yv12

        CAMTOOL_LOGD("disable APK Keycode");
        set_vendor_property("mtk.camera.app.keycode.enable", "\"\"");
        set_vendor_property("mtk.camera.switch.camera.debug", "\"\"");
        set_vendor_property("mtk.camera.switch.id.debug", "\"\"");

        CAMTOOL_LOGD("reset mfll");
        set_vendor_property("vendor.mfll.force", "\"\"");

        CAMTOOL_LOGD("camera end");
        set_vendor_property("vendor.3a.cct.enable", "\"\"");
        set_vendor_property("vendor.3a.cct.sensor.index", "\"\"");

        system_call("am force-stop com.mediatek.camera");
    }
    //else if (strncmp(argv[1], "capture-dump", 12) == 0) {
    //    captureCommand();
    //}
    else if (strncmp(argv[1], "capture", 7) == 0) {
        system_call("input keyevent KEYCODE_CAMERA");
    }
    else if (strncmp(argv[1], "cctv", 4) == 0) {
        strncpy(InFilename, argv[2], 255);
        strncpy(OutFilename, argv[3], 255);
        CAMTOOL_LOGD("execute CCT op");
        camtool.parseCommandFile(InFilename, OutFilename);
    }

    CAMTOOL_LOGD("camtool done!");
    return 0;
}

int main(int argc, char *argv[])
{
    return mainFunc(argc, argv);
}
