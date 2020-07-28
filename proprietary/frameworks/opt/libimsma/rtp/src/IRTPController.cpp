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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
/*****************************************************************************
*
* Filename:
* ---------
* IRTPController.h
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*   RTP module controller interface for ViLTE
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/
//#define LOG_NDEBUG 0
#define LOG_TAG "[VT][RTP]IRTPController"
#include <utils/Log.h>

#include "IRTPController.h"
#include "RTPController.h"

#include <utils/List.h>
#include <utils/Vector.h>
#include <inttypes.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ALooper.h>
#include <cutils/properties.h>


#include <utils/Errors.h>


namespace android
{

uint32_t IRTPController::opID = 0;
uint8_t IRTPController::sVideoCapParamsNum = 0;
rtp_rtcp_capability_t* IRTPController::sVideoCapParamsFirstItem = NULL;

rtp_rtcp_capability_t** IRTPController::sVideoCapParams = NULL;


List<rtp_rtcp_capability_t*> IRTPController::sVideoCapParamsList;
List<rtp_rtcp_capability_t*> IRTPController::sAudioCapParamsList;

uint8_t IRTPController::getRtpProfile(uint32_t operatorID){

    char value[PROPERTY_VALUE_MAX];
    int enable_avpf = 2;
    uint8_t rtp_profile = IMSMA_RTP_AVP|IMSMA_RTP_AVPF;

    if(property_get("persist.vendor.vt.avpf", value, NULL)) {
        enable_avpf = atoi(value);
    }

    if(enable_avpf == 2) {
        // AIS & DTAC can't support AVPF
        if(operatorID == 122 || operatorID == 125){
            rtp_profile = IMSMA_RTP_AVP;
        }else{
            rtp_profile = IMSMA_RTP_AVP|IMSMA_RTP_AVPF;
        }
    }else if(enable_avpf == 1) {
        rtp_profile = IMSMA_RTP_AVP|IMSMA_RTP_AVPF;
    } else {
        rtp_profile = IMSMA_RTP_AVP;
    }

    ALOGD("enable_avpf %d operatorID=%d rtp_profile=%d", enable_avpf, operatorID, rtp_profile);

    return rtp_profile;
}

status_t IRTPController::getCapability(rtp_rtcp_capability_t** pRTPCapArray,uint8_t *pNumOfCap,uint8_t uiMediaType, uint32_t operatorID)
{
    ALOGD("%s,capArray %p,pNumOfCap %p,media type 0x%x,", __FUNCTION__,pRTPCapArray,pNumOfCap,uiMediaType);
    //ToDo: if only assign value to pRTPCapArray, but not use,will build error
    //so we print the value of it to pass build

    if(uiMediaType == IMSMA_RTP_VIDEO) {
        if(!sVideoCapParamsFirstItem) {
            ALOGD("%s,create video capability",__FUNCTION__);

            uint8_t numOfCap = 2;
            rtp_rtcp_capability_t* rtp_cap = new rtp_rtcp_capability_t[numOfCap];
            sVideoCapParamsFirstItem = rtp_cap;
            sVideoCapParamsNum = numOfCap;

            rtp_cap[0].media_type = IMSMA_RTP_VIDEO;
            rtp_cap[0].mime_Type = IMSMA_RTP_VIDEO_H264;
            //rtp_cap[0].rtp_payload_type = 98;

            rtp_cap[0].rtp_profile = getRtpProfile(operatorID);
            rtp_cap[0].sample_rate = 90000;

            //rtp_cap[0].rtp_packet_bandwidth = 2*1024;// b=AS in kbps
            rtp_cap[0].packetize_mode = IMSMA_SINGLE_NAL_MODE|IMSMA_NON_INTERLEAVED_MODE;

            //TODO
            //a=extmap info
            rtp_cap[0].rtp_header_extension_num = 1;
            rtp_cap[0].rtp_ext_map[0].extension_id = 7;
            rtp_cap->rtp_ext_map[0].direction = ViLTE_RTP_DIRECTION_SENDRECV;
            //ToCheck,whether can assign the string in this way
            //rtp_cap.rtp_ext_map[0].extension_uri = "urn:3gpp:video-orientation";
            const char* extension_uri = "urn:3gpp:video-orientation";
            memset(rtp_cap[0].rtp_ext_map[0].extension_uri,0,sizeof(rtp_cap[0].rtp_ext_map[0].extension_uri));
            //ToDo: sizof char* will return sizeof string? or 4
            //why can't sizeof(extension_uri)
            memcpy(rtp_cap[0].rtp_ext_map[0].extension_uri,extension_uri,26);    //sizeof(extension_uri));

            //rtp_cap[0].rtcp_sender_bandwidth = 25.6*1024; //b=RS in bps,1.25%*(b=AS)
            //rtp_cap[0].rtcp_receiver_bandwidth = 76.8 * 1024; //b=RR in bps, 3.75%*(b=AS)
            rtp_cap[0].rtcp_reduce_size = 0;


            ALOGD("\t media(0x%x),mime(0x%x),profile(0x%x),sampleRate(%d)",\
                  rtp_cap[0].media_type,rtp_cap[0].mime_Type,\
                  rtp_cap[0].rtp_profile,rtp_cap[0].sample_rate);


            ALOGD("\t ext_header_num:%d",rtp_cap[0].rtp_header_extension_num);

            for(uint8_t i = 0; i < rtp_cap[0].rtp_header_extension_num; i++) {
                ALOGD("\t ext_header[%d]:id(%d),uri(%s)",i,rtp_cap[0].rtp_ext_map[i].extension_id,rtp_cap[0].rtp_ext_map[i].extension_uri);
            }

            ALOGD("\t packetize_mode(%d),rtcp_rsize(%d)",rtp_cap[0].packetize_mode,rtp_cap[0].rtcp_reduce_size);


            rtp_cap[0].rtcp_fb_param_num = 6;

            //memset all rtcp_fb_type item
            memset(rtp_cap[0].rtcp_fb_type,0,sizeof(rtp_cap[0].rtcp_fb_type));
            ALOGI("%s,sizeof(%zu)",__FUNCTION__,sizeof(rtp_cap[0].rtcp_fb_type));
            //ToDo: Can not assign value this way? why?
            /*
            rtp_cap.rtcp_fb_type[0] = {NACK,GENERIC_NACK,""};
            rtp_cap.rtcp_fb_type[1] = {NACK,PLI,""};
            //rtp_cap.rtcp_fb_type[2] = {CCM,TMMBR,"smaxpr=120"};
            rtp_cap.rtcp_fb_type[2] = {NACK,SLI,""};
            rtp_cap.rtcp_fb_type[3] = {CCM,FIR,""};
            rtp_cap.rtcp_fb_type[4] = {TRR_INT,5000,""}; //5s what is the unit
            */

            rtp_cap[0].rtcp_fb_type[0].rtcp_fb_id = IMSMA_NACK;
            rtp_cap[0].rtcp_fb_type[0].rtcp_fb_param = IMSMA_GENERIC_NACK;
            memset(rtp_cap[0].rtcp_fb_type[0].rtcp_fb_sub_param,0,sizeof(rtp_cap[0].rtcp_fb_type[0].rtcp_fb_sub_param));

            rtp_cap[0].rtcp_fb_type[1].rtcp_fb_id = IMSMA_NACK;
            rtp_cap[0].rtcp_fb_type[1].rtcp_fb_param = IMSMA_PLI;
            uint8_t temp_size = sizeof(rtp_cap[0].rtcp_fb_type[1].rtcp_fb_sub_param);
            ALOGI("%s,temp_size(%d)",__FUNCTION__,temp_size);
            memset(rtp_cap[0].rtcp_fb_type[1].rtcp_fb_sub_param,0,sizeof(rtp_cap[0].rtcp_fb_type[1].rtcp_fb_sub_param));

            /*
            rtp_cap.rtcp_fb_type[2].rtcp_fb_id = IMSMA_CCM;
            rtp_cap.rtcp_fb_type[2].rtcp_fb_param = IMSMA_TMMBR;
            memset(rtp_cap.rtcp_fb_type[2].rtcp_fb_sub_param,0,sizeof(rtp_cap.rtcp_fb_type[2].rtcp_fb_sub_param));
            const char* tmmbr_sub_param = "smaxpr=120";
            //ToDo: check the sizeof (char*)
            //why can't sizeof(tmmbr_sub_param))
            memcpy(rtp_cap.rtcp_fb_type[2].rtcp_fb_sub_param,tmmbr_sub_param,10);//sizeof(tmmbr_sub_param));
            ALOGI("%s,2-id:%d,2-param:%d,sub:%s",\
                __FUNCTION__,rtp_cap.rtcp_fb_type[2].rtcp_fb_id,rtp_cap.rtcp_fb_type[2].rtcp_fb_param,rtp_cap.rtcp_fb_type[2].rtcp_fb_sub_param);
            */
            rtp_cap[0].rtcp_fb_type[2].rtcp_fb_id = IMSMA_NACK;
            rtp_cap[0].rtcp_fb_type[2].rtcp_fb_param = IMSMA_SLI;
            memset(rtp_cap[0].rtcp_fb_type[2].rtcp_fb_sub_param,0,sizeof(rtp_cap[0].rtcp_fb_type[2].rtcp_fb_sub_param));


            rtp_cap[0].rtcp_fb_type[3].rtcp_fb_id = IMSMA_CCM;
            rtp_cap[0].rtcp_fb_type[3].rtcp_fb_param = IMSMA_FIR;
            memset(rtp_cap[0].rtcp_fb_type[3].rtcp_fb_sub_param,0,sizeof(rtp_cap[0].rtcp_fb_type[3].rtcp_fb_sub_param));
            ALOGI("%s,3-id:%d,3-param:%d,sub:%s",\
                  __FUNCTION__,rtp_cap[0].rtcp_fb_type[3].rtcp_fb_id,rtp_cap[0].rtcp_fb_type[3].rtcp_fb_param,rtp_cap[0].rtcp_fb_type[3].rtcp_fb_sub_param);

            ///*not set trr-int
            rtp_cap[0].rtcp_fb_type[4].rtcp_fb_id = IMSMA_TRR_INT;
            rtp_cap[0].rtcp_fb_type[4].rtcp_fb_param = 1000;
            memset(rtp_cap[0].rtcp_fb_type[4].rtcp_fb_sub_param,0,sizeof(rtp_cap[0].rtcp_fb_type[4].rtcp_fb_sub_param));
            //*/

            rtp_cap[0].rtcp_fb_type[5].rtcp_fb_id = IMSMA_CCM;
            rtp_cap[0].rtcp_fb_type[5].rtcp_fb_param = IMSMA_TMMBR;
            memset(rtp_cap[0].rtcp_fb_type[5].rtcp_fb_sub_param,0,sizeof(rtp_cap[0].rtcp_fb_type[5].rtcp_fb_sub_param));

            ALOGD("\t fb_num(%d)",rtp_cap[0].rtcp_fb_param_num);

            for(uint8_t j = 0; j< rtp_cap[0].rtcp_fb_param_num; j++) {
                ALOGD("\t Feedback[%d]:fb_id(%d),fb_param(%d),fb_sub_param(%s)",\
                      j, (rtp_cap[0].rtcp_fb_type[j]).rtcp_fb_id, (rtp_cap[0].rtcp_fb_type[j]).rtcp_fb_param,\
                      (rtp_cap[0].rtcp_fb_type[j]).rtcp_fb_sub_param);
            }

            //add hevc here
            ALOGD("%s 1 support HEVC", __FUNCTION__);
            rtp_cap[1] = rtp_cap[0];
            rtp_cap[0].mime_Type = IMSMA_RTP_VIDEO_HEVC;
        }else if(opID != operatorID){
            sVideoCapParamsFirstItem[0].rtp_profile = getRtpProfile(operatorID);
            sVideoCapParamsFirstItem[1].rtp_profile = sVideoCapParamsFirstItem[0].rtp_profile;
            ALOGD("%s, op id change %d-->%d reset rtp_profile %d",__FUNCTION__, opID, operatorID, sVideoCapParamsFirstItem[0].rtp_profile);
        }

        *pRTPCapArray = sVideoCapParamsFirstItem;
        *pNumOfCap  = sVideoCapParamsNum;
        opID = operatorID;
        ALOGI("%s,cap array first item address:%p, cap num:%d rtp_profile(%d)",
             __FUNCTION__,sVideoCapParamsFirstItem,sVideoCapParamsNum, sVideoCapParamsFirstItem[0].rtp_profile);
        return OK;
    } else if(uiMediaType == IMSMA_RTP_AUDIO) {
        ALOGE("%s,should not be here,not support audio now",__FUNCTION__);
        return UNKNOWN_ERROR;
    } else {
        ALOGE("%s,should not be here,unknown media type 0x%x",__FUNCTION__,uiMediaType);
        return UNKNOWN_ERROR;
    }
}


//sVideoCapParams will occupy the memory always until process reboot
//Note: resolution is no related to RTPController; but new codec typw is relate to RTPController
uint8_t IRTPController::getCapability(rtp_rtcp_capability_t** pRTPCapArray,uint8_t uiMaxNumOfCap,uint8_t uiMediaType, uint32_t operatorID)
{
    ALOGD("%s,capArray %p,uiMaxNumOfCap %d,media type 0x%x,",__FUNCTION__,pRTPCapArray,uiMaxNumOfCap,uiMediaType);
    //ToDo: if only assign value to pRTPCapArray, but not use,will build error
    //so we print the value of it to pass build

    if(uiMediaType == IMSMA_RTP_VIDEO) {
        if(!sVideoCapParams) {
            ALOGD("%s,create video capability",__FUNCTION__);

            uint8_t numOfCap = 2;

            if(uiMaxNumOfCap < numOfCap) {
                numOfCap = uiMaxNumOfCap;
            }

            sVideoCapParamsNum = numOfCap;
            sVideoCapParams = new rtp_rtcp_capability_t*[numOfCap];

            rtp_rtcp_capability_t* rtp_cap = new rtp_rtcp_capability_t;
            rtp_cap->media_type = IMSMA_RTP_VIDEO;
            rtp_cap->mime_Type = IMSMA_RTP_VIDEO_H264;
            //rtp_cap->rtp_payload_type = 98;

            rtp_cap[0].rtp_profile = getRtpProfile(operatorID);
            rtp_cap->sample_rate = 90000;

            //rtp_cap->rtp_packet_bandwidth = 2*1024;// b=AS in kbps
            rtp_cap->packetize_mode = IMSMA_SINGLE_NAL_MODE|IMSMA_NON_INTERLEAVED_MODE;

            //TODO
            //a=extmap info
            rtp_cap->rtp_header_extension_num = 1;
            rtp_cap->rtp_ext_map[0].extension_id = 7;
            rtp_cap->rtp_ext_map[0].direction = ViLTE_RTP_DIRECTION_SENDRECV;
            //ToCheck,whether can assign the string in this way
            //rtp_cap->rtp_ext_map[0].extension_uri = "urn:3gpp:video-orientation";
            const char* extension_uri = "urn:3gpp:video-orientation";
            memset(rtp_cap->rtp_ext_map[0].extension_uri,0,sizeof(rtp_cap->rtp_ext_map[0].extension_uri));
            //ToDo: sizof char* will return sizeof string? or 4
            //why can't sizeof(extension_uri)
            memcpy(rtp_cap->rtp_ext_map[0].extension_uri,extension_uri,26);    //sizeof(extension_uri));

            //rtp_cap->rtcp_sender_bandwidth = 25.6*1024; //b=RS in bps,1.25%*(b=AS)
            //rtp_cap->rtcp_receiver_bandwidth = 76.8 * 1024; //b=RR in bps, 3.75%*(b=AS)
            rtp_cap->rtcp_reduce_size = 0;


            ALOGD("\t media(0x%x),mime(0x%x),profile(0x%x),sampleRate(%d)",\
                  rtp_cap->media_type,rtp_cap->mime_Type,\
                  rtp_cap->rtp_profile,rtp_cap->sample_rate);


            ALOGD("\t ext_header_num:%d",rtp_cap->rtp_header_extension_num);

            for(uint8_t i = 0; i < rtp_cap->rtp_header_extension_num; i++) {
                ALOGD("\t ext_header[%d]:id(%d),uri(%s)",i,rtp_cap->rtp_ext_map[i].extension_id,rtp_cap->rtp_ext_map[i].extension_uri);
            }

            ALOGD("\t packetize_mode(%d),rtcp_rsize(%d)",rtp_cap->packetize_mode,rtp_cap->rtcp_reduce_size);


            rtp_cap->rtcp_fb_param_num = 6;

            //memset all rtcp_fb_type item
            memset(rtp_cap->rtcp_fb_type,0,sizeof(rtp_cap->rtcp_fb_type));
            ALOGI("%s,sizeof(%zu)",__FUNCTION__,sizeof(rtp_cap->rtcp_fb_type));
            //ToDo: Can not assign value this way? why?
            /*
            rtp_cap->rtcp_fb_type[0] = {NACK,GENERIC_NACK,""};
            rtp_cap->rtcp_fb_type[1] = {NACK,PLI,""};
            //rtp_cap->rtcp_fb_type[2] = {CCM,TMMBR,"smaxpr=120"};
            rtp_cap->rtcp_fb_type[2] = {NACK,SLI,""};
            rtp_cap->rtcp_fb_type[3] = {CCM,FIR,""};
            rtp_cap->rtcp_fb_type[4] = {TRR_INT,5000,""}; //5s what is the unit
            */

            rtp_cap->rtcp_fb_type[0].rtcp_fb_id = IMSMA_NACK;
            rtp_cap->rtcp_fb_type[0].rtcp_fb_param = IMSMA_GENERIC_NACK;
            memset(rtp_cap->rtcp_fb_type[0].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[0].rtcp_fb_sub_param));

            rtp_cap->rtcp_fb_type[1].rtcp_fb_id = IMSMA_NACK;
            rtp_cap->rtcp_fb_type[1].rtcp_fb_param = IMSMA_PLI;
            size_t temp_size = sizeof(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param);
            ALOGI("%s,temp_size(%zu)",__FUNCTION__,temp_size);
            memset(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param));

            /*
            rtp_cap->rtcp_fb_type[2].rtcp_fb_id = IMSMA_CCM;
            rtp_cap->rtcp_fb_type[2].rtcp_fb_param = IMSMA_TMMBR;
            memset(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param));
            const char* tmmbr_sub_param = "smaxpr=120";
            //ToDo: check the sizeof (char*)
            //why can't sizeof(tmmbr_sub_param))
            memcpy(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param,tmmbr_sub_param,10);//sizeof(tmmbr_sub_param));
            ALOGI("%s,2-id:%d,2-param:%d,sub:%s",\
                __FUNCTION__,rtp_cap->rtcp_fb_type[2].rtcp_fb_id,rtp_cap->rtcp_fb_type[2].rtcp_fb_param,rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param);
            */
            rtp_cap->rtcp_fb_type[2].rtcp_fb_id = IMSMA_NACK;
            rtp_cap->rtcp_fb_type[2].rtcp_fb_param = IMSMA_SLI;
            memset(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param));


            rtp_cap->rtcp_fb_type[3].rtcp_fb_id = IMSMA_CCM;
            rtp_cap->rtcp_fb_type[3].rtcp_fb_param = IMSMA_FIR;
            memset(rtp_cap->rtcp_fb_type[3].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[3].rtcp_fb_sub_param));
            ALOGI("%s,3-id:%d,3-param:%d,sub:%s",\
                  __FUNCTION__,rtp_cap->rtcp_fb_type[3].rtcp_fb_id,rtp_cap->rtcp_fb_type[3].rtcp_fb_param,rtp_cap->rtcp_fb_type[3].rtcp_fb_sub_param);

            ///*not set trr-int
            rtp_cap->rtcp_fb_type[4].rtcp_fb_id = IMSMA_TRR_INT;
            rtp_cap->rtcp_fb_type[4].rtcp_fb_param = 1000;
            memset(rtp_cap->rtcp_fb_type[4].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[4].rtcp_fb_sub_param));
            //*/

            rtp_cap->rtcp_fb_type[5].rtcp_fb_id = IMSMA_CCM;
            rtp_cap->rtcp_fb_type[5].rtcp_fb_param = IMSMA_TMMBR;
            memset(rtp_cap->rtcp_fb_type[5].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[5].rtcp_fb_sub_param));

            ALOGD("\t fb_num(%d)",rtp_cap->rtcp_fb_param_num);

            for(uint8_t j = 0; j< rtp_cap->rtcp_fb_param_num; j++) {
                ALOGD("\t Feedback[%d]:fb_id(%d),fb_param(%d),fb_sub_param(%s)",\
                      j, (rtp_cap->rtcp_fb_type[j]).rtcp_fb_id, (rtp_cap->rtcp_fb_type[j]).rtcp_fb_param,\
                      (rtp_cap->rtcp_fb_type[j]).rtcp_fb_sub_param);
            }

            sVideoCapParams[0] = rtp_cap;


            //add hevc here
            ALOGD("%s 2 support HEVC", __FUNCTION__);
            rtp_rtcp_capability_t* rtp_cap2 = new rtp_rtcp_capability_t;
            *rtp_cap2 = *rtp_cap;
            rtp_cap2->mime_Type = IMSMA_RTP_VIDEO_HEVC;
            sVideoCapParams[1] = rtp_cap2;
        }

        for(uint8_t i = 0; i < sVideoCapParamsNum; i++) {
            pRTPCapArray[i] = sVideoCapParams[i];
        }

        return sVideoCapParamsNum;
    } else if(uiMediaType == IMSMA_RTP_AUDIO) {
        ALOGE("%s,should not be here,not support audio now",__FUNCTION__);
        return 0;
    } else {
        ALOGE("%s,should not be here,unknown media type 0x%x",__FUNCTION__,uiMediaType);
        return 0;
    }

}

List<rtp_rtcp_capability_t*> IRTPController::getCapability(uint8_t uiMediaType, uint32_t operatorID)
{
    //Vector<rtp_rtcp_capability_t*> rtp_caps;
    ALOGI("%s,uiMediaType=%d",__FUNCTION__,uiMediaType);

    if(uiMediaType == IMSMA_RTP_VIDEO) {
        if(sVideoCapParamsList.empty()) {
            ALOGD("%s,crate video capability",__FUNCTION__);

            //create AVC cap
            //ToDo: when to delete this buffer
            rtp_rtcp_capability_t* rtp_cap = new rtp_rtcp_capability_t;
            rtp_cap->media_type = IMSMA_RTP_VIDEO;
            rtp_cap->mime_Type = IMSMA_RTP_VIDEO_H264;
            //rtp_cap->rtp_payload_type = 98;

            rtp_cap->rtp_profile = getRtpProfile(operatorID);
            rtp_cap->sample_rate = 90000;

            //rtp_cap->rtp_packet_bandwidth = 2*1024;// b=AS in kbps
            rtp_cap->packetize_mode = IMSMA_SINGLE_NAL_MODE|IMSMA_NON_INTERLEAVED_MODE;

            //TODO
            //a=extmap info
            rtp_cap->rtp_header_extension_num = 1;
            rtp_cap->rtp_ext_map[0].extension_id = 7;
            rtp_cap->rtp_ext_map[0].direction = ViLTE_RTP_DIRECTION_SENDRECV;
            //ToCheck,whether can assign the string in this way
            //rtp_cap->rtp_ext_map[0].extension_uri = "urn:3gpp:video-orientation";
            const char* extension_uri = "urn:3gpp:video-orientation";
            memset(rtp_cap->rtp_ext_map[0].extension_uri,0,sizeof(rtp_cap->rtp_ext_map[0].extension_uri));
            //ToDo: sizof char* will return sizeof string? or 4
            //why can't sizeof(extension_uri)
            memcpy(rtp_cap->rtp_ext_map[0].extension_uri,extension_uri,26);    //sizeof(extension_uri));

            //rtp_cap->rtcp_sender_bandwidth = 25.6*1024; //b=RS in bps,1.25%*(b=AS)
            //rtp_cap->rtcp_receiver_bandwidth = 76.8 * 1024; //b=RR in bps, 3.75%*(b=AS)
            rtp_cap->rtcp_reduce_size = 0;

            ALOGD("\t media(0x%x),mime(0x%x),profile(0x%x),sampleRate(%d)",\
                  rtp_cap->media_type,rtp_cap->mime_Type,\
                  rtp_cap->rtp_profile,rtp_cap->sample_rate);


            ALOGD("\t ext_header_num:%d",rtp_cap->rtp_header_extension_num);

            for(uint8_t i = 0; i < rtp_cap->rtp_header_extension_num; i++) {
                ALOGD("\t ext_header[%d]:id(%d),uri(%s)",i,rtp_cap->rtp_ext_map[i].extension_id,rtp_cap->rtp_ext_map[i].extension_uri);
            }

            ALOGD("\t packetize_mode(%d),rtcp_rsize(%d)",rtp_cap->packetize_mode,rtp_cap->rtcp_reduce_size);


            rtp_cap->rtcp_fb_param_num = 6;

            //memset all rtcp_fb_type item
            memset(rtp_cap->rtcp_fb_type,0,sizeof(rtp_cap->rtcp_fb_type));
            //ToDo: Can not assign value this way? why?
            /*
            rtp_cap->rtcp_fb_type[0] = {NACK,GENERIC_NACK,""};
            rtp_cap->rtcp_fb_type[1] = {NACK,PLI,""};
            //rtp_cap->rtcp_fb_type[2] = {CCM,TMMBR,"smaxpr=120"};
            rtp_cap->rtcp_fb_type[2] = {NACK,SLI,""};
            rtp_cap->rtcp_fb_type[3] = {CCM,FIR,""};
            rtp_cap->rtcp_fb_type[4] = {TRR_INT,5000,""}; //5s what is the unit
            */
            rtp_cap->rtcp_fb_type[0].rtcp_fb_id = IMSMA_NACK;
            rtp_cap->rtcp_fb_type[0].rtcp_fb_param = IMSMA_GENERIC_NACK;
            memset(rtp_cap->rtcp_fb_type[0].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[0].rtcp_fb_sub_param));

            rtp_cap->rtcp_fb_type[1].rtcp_fb_id = IMSMA_NACK;
            rtp_cap->rtcp_fb_type[1].rtcp_fb_param = IMSMA_PLI;
            memset(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param));


            rtp_cap->rtcp_fb_type[2].rtcp_fb_id = IMSMA_NACK;
            rtp_cap->rtcp_fb_type[2].rtcp_fb_param = IMSMA_SLI;
            memset(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param));

            rtp_cap->rtcp_fb_type[3].rtcp_fb_id = IMSMA_CCM;
            rtp_cap->rtcp_fb_type[3].rtcp_fb_param = IMSMA_FIR;
            memset(rtp_cap->rtcp_fb_type[3].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[3].rtcp_fb_sub_param));

            ///* not set trr-int
            rtp_cap->rtcp_fb_type[4].rtcp_fb_id = IMSMA_TRR_INT;
            rtp_cap->rtcp_fb_type[4].rtcp_fb_param = 1000;
            memset(rtp_cap->rtcp_fb_type[4].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[4].rtcp_fb_sub_param));
            //*/

            rtp_cap->rtcp_fb_type[5].rtcp_fb_id = IMSMA_CCM;
            rtp_cap->rtcp_fb_type[5].rtcp_fb_param = IMSMA_TMMBR;
            memset(rtp_cap->rtcp_fb_type[5].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[5].rtcp_fb_sub_param));
            //const char* tmmbr_sub_param = "smaxpr=120";
            //ToDo: check the sizeof (char*)
            //why can't sizeof(tmmbr_sub_param))
            //memcpy(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param,tmmbr_sub_param,10);//sizeof(tmmbr_sub_param));

            ALOGD("\t fb_num(%d)",rtp_cap->rtcp_fb_param_num);

            for(uint8_t j = 0; j< rtp_cap->rtcp_fb_param_num; j++) {

                ALOGD("\t Feedback[%d]:fb_id(%d),fb_param(%d),fb_sub_param(%s)",\
                      j, (rtp_cap->rtcp_fb_type[j]).rtcp_fb_id, (rtp_cap->rtcp_fb_type[j]).rtcp_fb_param,\
                      (rtp_cap->rtcp_fb_type[j]).rtcp_fb_sub_param);
            }

            sVideoCapParamsList.push_back(rtp_cap);

            ALOGD("%s 3 support HEVC", __FUNCTION__);
            rtp_rtcp_capability_t* rtp_cap2 = new rtp_rtcp_capability_t;
            *rtp_cap2 = *rtp_cap;
            rtp_cap2->mime_Type = IMSMA_RTP_VIDEO_HEVC;
            sVideoCapParamsList.push_back(rtp_cap2);

            /***create HEVC cap***
            rtp_cap = new rtp_rtcp_capability_t;
            rtp_cap->media_type = IMSMA_RTP_VIDEO;
            rtp_cap->mime_Type = IMSMA_RTP_VIDEO_HEVC;
            rtp_cap->rtp_payload_type = 99;
            rtp_cap->rtp_profile = IMSMA_RTP_AVPF;
            rtp_cap->sample_rate = 90000;

            rtp_cap->rtp_packet_bandwidth = 2*1024;// b=AS in kbps
            rtp_cap->packetize_mode = 1;

            //TODO
            //a=extmap info
            rtp_cap->rtp_header_extension_num = 1;
            rtp_cap->rtp_ext_map[0].extension_id = 1;
            //ToCheck,whether can assign the string in this way
            //rtp_cap->rtp_ext_map[0].extension_uri = "urn:3gpp:video-orientation";
            extension_uri = "urn:3gpp:video-orientation";
            memset(rtp_cap->rtp_ext_map[0].extension_uri,0,sizeof(rtp_cap->rtp_ext_map[0].extension_uri));
            //ToDo: sizof char* will return sizeof string? or 4
            //why can't sizeof(extension_uri)
            memcpy(rtp_cap->rtp_ext_map[0].extension_uri,extension_uri,26);//sizeof(extension_uri));

            rtp_cap->rtcp_sender_bandwidth = 25.6*1024; //b=RS in bps,1.25%*(b=AS)
            rtp_cap->rtcp_receiver_bandwidth = 76.8 * 1024; //b=RR in bps, 3.75%*(b=AS)
            rtp_cap->rtcp_reduce_size = 0;

            ALOGD("\t media(0x%x),mime(0x%x),PT(%d),profile(0x%x),sampleRate(%d)",\
                rtp_cap->media_type,rtp_cap->mime_Type,\
                rtp_cap->rtp_payload_type,rtp_cap->rtp_profile,rtp_cap->sample_rate);


            ALOGD("\t ext_header_num:%d",rtp_cap->rtp_header_extension_num);

            for(uint8_t i = 0; i < rtp_cap->rtp_header_extension_num; i++){
                ALOGD("\t ext_header[%d]:id(%d),uri(%s)",i,rtp_cap->rtp_ext_map[i].extension_id,rtp_cap->rtp_ext_map[i].extension_uri);
            }

            ALOGD("\t b=AS(%d),packetize_mode(%d),b=RS(%" PRId64 "),b=RR(%" PRId64 "),rtcp_rsize(%d)",\
                rtp_cap->rtp_packet_bandwidth,rtp_cap->packetize_mode,\
                rtp_cap->rtcp_sender_bandwidth,rtp_cap->rtcp_receiver_bandwidth,rtp_cap->rtcp_reduce_size);

            rtp_cap->rtcp_fb_param_num = 5;

            //memset all rtcp_fb_type item
            memset(rtp_cap->rtcp_fb_type,0,sizeof(rtp_cap->rtcp_fb_type));
            //ToDo: Can not assign value this way? why?

            //rtp_cap->rtcp_fb_type[0] = {NACK,GENERIC_NACK,""};
            //rtp_cap->rtcp_fb_type[1] = {NACK,PLI,""};
            //rtp_cap->rtcp_fb_type[2] = {CCM,TMMBR,"smaxpr=120"};
            //rtp_cap->rtcp_fb_type[2] = {NACK,SLI,""};
            //rtp_cap->rtcp_fb_type[3] = {CCM,FIR,""};
            //rtp_cap->rtcp_fb_type[4] = {TRR_INT,5000,""}; //5s what is the unit

            rtp_cap->rtcp_fb_type[0].rtcp_fb_id = IMSMA_NACK;
            rtp_cap->rtcp_fb_type[0].rtcp_fb_param = IMSMA_GENERIC_NACK;
            memset(rtp_cap->rtcp_fb_type[0].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[0].rtcp_fb_sub_param));

            rtp_cap->rtcp_fb_type[1].rtcp_fb_id = IMSMA_NACK;
            rtp_cap->rtcp_fb_type[1].rtcp_fb_param = IMSMA_PLI;
            memset(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param));


            //rtp_cap->rtcp_fb_type[2].rtcp_fb_id = IMSMA_CCM;
            //rtp_cap->rtcp_fb_type[2].rtcp_fb_param = IMSMA_TMMBR;
            //memset(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param));
            //const char* tmmbr_sub_param = "smaxpr=120";
            //ToDo: check the sizeof (char*)
            //why can't sizeof(tmmbr_sub_param))
            //memcpy(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param,tmmbr_sub_param,10);//sizeof(tmmbr_sub_param));


            rtp_cap->rtcp_fb_type[2].rtcp_fb_id = IMSMA_NACK;
            rtp_cap->rtcp_fb_type[2].rtcp_fb_param = IMSMA_SLI;
            memset(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[2].rtcp_fb_sub_param));

            rtp_cap->rtcp_fb_type[3].rtcp_fb_id = IMSMA_CCM;
            rtp_cap->rtcp_fb_type[3].rtcp_fb_param = IMSMA_FIR;
            memset(rtp_cap->rtcp_fb_type[3].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param));

            rtp_cap->rtcp_fb_type[4].rtcp_fb_id = IMSMA_TRR_INT;
            rtp_cap->rtcp_fb_type[4].rtcp_fb_param = 5000;
            memset(rtp_cap->rtcp_fb_type[4].rtcp_fb_sub_param,0,sizeof(rtp_cap->rtcp_fb_type[1].rtcp_fb_sub_param));

            ALOGD("\t fb_num(%d)",rtp_cap->rtcp_fb_param_num);

            for(uint8_t j = 0; j< rtp_cap->rtcp_fb_param_num; j++ ){

                ALOGD("\t Feedback[%d]:fb_id(%d),fb_param(%d),fb_sub_param(%s)",\
                    j,(rtp_cap->rtcp_fb_type[j]).rtcp_fb_id,(rtp_cap->rtcp_fb_type[j]).rtcp_fb_param,\
                    (rtp_cap->rtcp_fb_type[j]).rtcp_fb_sub_param);
            }

            sVideoCapParamsList.push_back(rtp_cap);

            ***create HEVC cap ***/
        }

        return sVideoCapParamsList;
    } else if(uiMediaType == IMSMA_RTP_AUDIO) {
        ALOGE("%s,should not be here,not support audio now",__FUNCTION__);
        return sAudioCapParamsList;
    } else {
        ALOGE("%s,should not be here,unknown media type 0x%x",__FUNCTION__,uiMediaType);
        //ToDo: could not return NULL?
        return sVideoCapParamsList;
    }
}

sp<IRTPController> IRTPController::createRTPController(uint32_t simID,uint32_t operatorID)
{
    sp<IRTPController> rtpCon = dynamic_cast<IRTPController*>(new RTPController(simID,operatorID));
    return rtpCon;
}

IRTPController::IRTPController()
    :RefBase()
{
}

IRTPController::~IRTPController()
{

}

}


