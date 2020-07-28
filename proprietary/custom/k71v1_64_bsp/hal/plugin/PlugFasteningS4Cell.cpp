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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCamCustom/PlugFasteningS4Cell"
//
#include <cstdint>
#include <fstream>
#include <fcntl.h>
#include "PlugFasteningS4Cell.h"

#define GAINTBL_DATA_LEN        16*D_SONY_QD_LIB_GAIN_KNOT_H*D_SONY_QD_LIB_GAIN_KNOT_V
// TEST Parameter
#define USE_GAINTBL_DATA        0
#define USE_TEST_RAW_DATA       0
#define TEST_RAW_DATA_PATH       "sdcard/fcell_src.raw"
//
#define PLUG_DEBUG_DUMP_PATH    "/sdcard"

using namespace android;
using namespace NSCamCustom;
char gGainTbl[560] = {
#include "gaintbl.h"
};
/******************************************************************************
 *
 ******************************************************************************/
PLUGFASTENING_CLASSNAME::
PLUGFASTENING_CLASSNAME(OPEN_VENDOR_ID id)
{
    FUNC_BGN;
    mOpenID = id; // must set OpenID
    // add implementation here
    //
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
PLUGFASTENING_CLASSNAME::
~PLUGFASTENING_CLASSNAME()
{
    FUNC_BGN;
    //
    // add implementation here
    //
    FUNC_END;
};


/******************************************************************************
 * init function implementation
 ******************************************************************************/
MBOOL
PLUGFASTENING_CLASSNAME::
initImp(NSCam::PlugInitParam initParam)
{
    FUNC_BGN;
    //
    MBOOL res = true;
    unsigned int version = SonyQdLibGetVersion();
    MY_LOGD("Sony plug version: 0x%x", version);
    MY_LOGD_IF(mLogLevel, "wxh: %dx%d eeprom: %p size: %zu", initParam.img_w, initParam.img_h, initParam.eep_buf_addr, initParam.eep_buf_size);

    /*
     * Need to fill in otp dpc data
    */
    SonyQdLibDfctTbl_t dfcttbl;
    dfcttbl.sg_dfct_num   = initParam.dpc_num;
    dfcttbl.sg_dfct_addr  = (initParam.dpc_num > 0) ? (int*) initParam.dpc_addr : nullptr;
    dfcttbl.fd_dfct_num   = initParam.fd_dpc_num;
    dfcttbl.fd_dfct_addr  = (initParam.fd_dpc_num > 0) ? (int*) initParam.fd_dpc_addr : nullptr;

    SonyQdLibGainTbl_t gaintbl;
    ::memset(&gaintbl.gain, 0, sizeof(gaintbl.gain));
#if USE_GAINTBL_DATA
    long length = sizeof(gaintbl.gain);
    MY_LOGD_IF(mLogLevel, "DEBUG: gain table length(%d) equal to GAINTBL_DATA_LEN(%d)", length, GAINTBL_DATA_LEN);
    ::memcpy(&gaintbl.gain, &gGainTbl, sizeof(gaintbl.gain));
	if(mLogLevel>=2) {
        char filename[100];
        sprintf(filename, PLUG_DEBUG_DUMP_PATH "/fcell_gain.dat");
        std::ofstream out(filename);
        out.write(reinterpret_cast<const char*>(gaintbl.gain), length);
        //
        size_t idx_x;
        size_t idx_y;
        for(idx_x=0;idx_x<16;idx_x++)
            for(idx_y=0;idx_y<D_SONY_QD_LIB_GAIN_KNOT_H*D_SONY_QD_LIB_GAIN_KNOT_V;idx_y++)
                MY_LOGD("GAIN_Table[%zu][%zu] = %d;\n", idx_x, idx_y, gaintbl.gain[idx_x][idx_y]);
   }
#else
    if(initParam.eep_buf_size < sizeof(gaintbl.gain))
        MY_LOGE("eep_buf_size(%zu) < gaintbl.gain size(%zu)", initParam.eep_buf_size, sizeof(gaintbl.gain));
    ::memcpy(&gaintbl.gain, initParam.eep_buf_addr, sizeof(gaintbl.gain));

#endif

    nsecs_t startTime = ::systemTime();
    int r = SonyQdLibInitialize(&dfcttbl, &gaintbl);
    nsecs_t processTime = ::systemTime() - startTime;
    MY_LOGI("SonyQdLibInitialize - %" PRId64 "ns", processTime);
    if(r<0) {
        MY_LOGE("SonyQdLibInitialize fail! %d", r);
        res = false;
    }
    else MY_LOGD("SonyQdLibInitialize success!");
    //
    FUNC_END;
    return res;
};


/******************************************************************************
 * uninit function implementation
 ******************************************************************************/
MBOOL
PLUGFASTENING_CLASSNAME::
uninitImp(void)
{
    FUNC_BGN;
    MBOOL res = MTRUE;
    //
    SonyQdLibFinalize();
    FUNC_END;
    return res;
};


/******************************************************************************
 * send command function implementation
 ******************************************************************************/
MBOOL
PLUGFASTENING_CLASSNAME::
sendCommandImp(MINT32 const cmd,
    MINTPTR arg1, MINTPTR arg2, MINTPTR arg3, MINTPTR arg4)
{
    FUNC_BGN;
    MBOOL res = MTRUE;
    MY_LOGI_IF(mLogLevel > 0, "CMD[%d]"
        "(%" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ")",
        cmd, arg1, arg2, arg3, arg4);
    //
    switch (cmd)
    {
        case NSCam::NSCamPlug::GET_PARAM:
        {
            NSCam::PlugStaticInfo info;
            info.eep_buf_size = GAINTBL_DATA_LEN;
            *(NSCam::PlugStaticInfo*)arg1 = info;
            break;
        }
        case NSCam::NSCamPlug::PROCESS:
        {

            NSCam::PlugProcessingParam _param = *(NSCam::PlugProcessingParam*)arg1;
            SonyQdLibConfig_t lib_param;
            lib_param.width     = _param.img_w;
            lib_param.height    = _param.img_h;
            lib_param.offset_x  = (D_SONY_QD_LIB_FULL_IMAGE_WIDTH -_param.img_w )/2;
            lib_param.offset_y  = (D_SONY_QD_LIB_FULL_IMAGE_HEIGHT -_param.img_h)/2;
            lib_param.cid       = D_SONY_QD_LIB_FULL_IMAGE_CID;
            lib_param.wb_gain_r     = _param.gain_awb_r*2;
            lib_param.wb_gain_gr    = _param.gain_awb_gr*2;
            lib_param.wb_gain_gb    = _param.gain_awb_gb*2;
            lib_param.wb_gain_b     = _param.gain_awb_b*2;
            lib_param.analog_gain   = _param.gain_analog/32;
            lib_param.digital_gain  = _param.gain_digital;
            lib_param.mode      = 0;
			MY_LOGD_IF(mLogLevel >=0 ,"D_SONY_QD_LIB_FULL_IMAGE_WIDTH: %d", D_SONY_QD_LIB_FULL_IMAGE_WIDTH);
			MY_LOGD_IF(mLogLevel >=0 ,"D_SONY_QD_LIB_FULL_IMAGE_HEIGHT: %d", D_SONY_QD_LIB_FULL_IMAGE_HEIGHT);
            MY_LOGD_IF(mLogLevel >=0 ,"width: %d", lib_param.width);
            MY_LOGD_IF(mLogLevel >=0 ,"height: %d", lib_param.height);
            MY_LOGD_IF(mLogLevel >=0 ,"offset_x: %d", lib_param.offset_x);
            MY_LOGD_IF(mLogLevel >=0 ,"offset_y: %d", lib_param.offset_y);
            MY_LOGD_IF(mLogLevel >=0 ,"cid: %d", lib_param.cid);
            MY_LOGD_IF(mLogLevel >=0 ,"wb_gain_r: %d", lib_param.wb_gain_r);
            MY_LOGD_IF(mLogLevel >=0 ,"wb_gain_gr: %d", lib_param.wb_gain_gr);
            MY_LOGD_IF(mLogLevel >=0 ,"wb_gain_gb: %d", lib_param.wb_gain_gb);
            MY_LOGD_IF(mLogLevel >=0 ,"wb_gain_b: %d", lib_param.wb_gain_b);
            MY_LOGD_IF(mLogLevel >=0 ,"analog_gain: %d", lib_param.analog_gain);
            MY_LOGD_IF(mLogLevel >=0 ,"digital_gain: %d", lib_param.digital_gain);
            MY_LOGD_IF(mLogLevel >=0 ,"mode: %d", lib_param.mode);
            if((lib_param.width + lib_param.offset_x*2) != D_SONY_QD_LIB_FULL_IMAGE_WIDTH)
                MY_LOGW("Need to check. image width(%d)+2*offset(%d) != full.w(%d).", lib_param.width, lib_param.offset_x, D_SONY_QD_LIB_FULL_IMAGE_WIDTH);
            //
            SonyQdLibImageOut_t img_out;
            SonyQdLibImageIn_t  img_in;
            int src_buf_fd = _param.src_buf_fd;
            int dst_buf_fd = _param.dst_buf_fd;
            size_t src_buf_size = _param.src_buf_size;
            size_t dst_buf_size = _param.dst_buf_size;
            img_out.fd = _param.dst_buf_fd;
            img_out.p_addr = _param.dst_buf;
            img_in.fd = _param.src_buf_fd;
            img_in.p_addr = _param.src_buf;
			MY_LOGD("srcFd:%d dstFd:%d srcAddr:%p dstAddr:%p srcBufSize:%zu dstBufSize:%zu arg1:%p",
                     img_in.fd, img_out.fd, img_in.p_addr, img_out.p_addr, src_buf_size, dst_buf_size, (void*)arg1);
#if USE_TEST_RAW_DATA
			src_buf_size = D_SONY_QD_LIB_FULL_IMAGE_WIDTH*D_SONY_QD_LIB_FULL_IMAGE_HEIGHT*2;
            dst_buf_size = D_SONY_QD_LIB_FULL_IMAGE_WIDTH*D_SONY_QD_LIB_FULL_IMAGE_HEIGHT*2;
            lib_param.width     = D_SONY_QD_LIB_FULL_IMAGE_WIDTH;
            lib_param.height    = D_SONY_QD_LIB_FULL_IMAGE_HEIGHT;
            lib_param.offset_x  = 0;
            lib_param.offset_y  = 0;
            void* pFcellImage = ::malloc(src_buf_size);
			img_in.p_addr = (unsigned short *)pFcellImage;
			img_in.fd = -1;
            if(pFcellImage == NULL)
            {
                MY_LOGE("pFcellImage input allocate fail.");
            }
			//
            void *pOutImage = ::malloc(src_buf_size);
            img_out.p_addr = (unsigned short *)pOutImage;
			img_out.fd = -1;
            if(pOutImage == NULL)
            {
                MY_LOGE("pOutImage output allocate fail.");
            }
			//
			const char * fcellImgFile = TEST_RAW_DATA_PATH;
            int fp0 = open(fcellImgFile,O_RDONLY);
            if(fp0)
            {
                int imgsize = src_buf_size;
                read(fp0,pFcellImage, imgsize);
                close(fp0);
                {
                    char filename[100];
                    sprintf(filename, "sdcard/fcell_src_testdata.raw");
                    std::ofstream out(filename);
                    out.write(reinterpret_cast<char *>(pFcellImage), src_buf_size);
                }
            }
            else
            {
                MY_LOGE("Cannot open test fcell image file %s\n", fcellImgFile);
				MY_LOGE("Remind push image to %s\n", fcellImgFile);
                return -1;
            }
            MY_LOGD("replace working buffer for test => wxh:%dx%d srcFd:%d dstFd:%d srcAddr:%p dstAddr:%p srcBufSize:%zu dstBufSize:%zu arg1:%p",
                     lib_param.width, lib_param.height, img_in.fd, img_out.fd, img_in.p_addr, img_out.p_addr, src_buf_size, dst_buf_size, (void*)arg1);
#endif
            static int count = 0;
            if(mLogLevel >= 2)
           	{
       	        char filename[100];
                sprintf(filename, PLUG_DEBUG_DUMP_PATH "/fcell_src_%d_%d_%d.raw", lib_param.width, lib_param.height, count);
                std::ofstream out(filename);
                out.write(reinterpret_cast<char *>(img_in.p_addr), src_buf_size);
            }
			//Process Fcell.
            nsecs_t startTime = ::systemTime();
            int r = SonyQdLibExecute(&img_out, &img_in, &lib_param);
            nsecs_t processTime = ::systemTime() - startTime;
            MY_LOGI("SonyQdLibExecute -  %" PRId64 "ns", processTime);
            if(r!=0)
                res = false;
            //
            if(mLogLevel >= 2)
            {
                char filename[100];
                sprintf(filename, PLUG_DEBUG_DUMP_PATH "/fcell_dst_%d_%d_%d.raw", lib_param.width, lib_param.height, count);
                std::ofstream out(filename);
                out.write(reinterpret_cast<char *>(img_out.p_addr), src_buf_size);
                count ++;
            }
            break;

        }

    }
    //
    FUNC_END;
    return res;
};


