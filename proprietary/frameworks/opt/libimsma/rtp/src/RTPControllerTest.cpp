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
 * RTPControllerTest.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   RTPController Test sample code
 *
 * Author:
 * -------
 *   Haizhen.Wang(mtk80691)
 *
 ****************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "[VT][RTP]RTPControllerTest"
#include <utils/Log.h>

#include "RTPBase.h"
#include "IRTPController.h"
#include "RTPController.h"

#include <SocketWrapper.h>

#include <inttypes.h>
#include <utils/List.h>
#include <utils/Vector.h>

#include <media/stagefright/foundation/ABase.h>
#include <utils/List.h>
#include <utils/RefBase.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ALooper.h>
#include <binder/ProcessState.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <linux/stat.h>

#include "media/stagefright/foundation/avc_utils.h"

//using namespace imsma;
using android::status_t;


using namespace android;

enum {
    kWhatAccu = 'accu',
    kWhatRtpPacket = 'rtpk' ,
    kWhatNotify  = 'ntfy',
};

static uint8_t payload_type_value = 97;
class RTPControllerListener : public AHandler
{
public:
    RTPControllerListener() {
        ALOGI("RTPControllerListener +");
        //ToDo: why can not register this in contructor
        /*mLooper = new ALooper;
        mLooper->registerHandler(this);
        ALOGE("start looper");
        mLooper->start();
        ALOGI("RTPControllerListener -");
        */
        accu_count = 0;
    }
protected:
    virtual ~RTPControllerListener() {
        ALOGI("~RTPControllerListener +");
        /*if(mLooper.get()){
            mLooper->stop();
            ALOGI("stop looper");
            mLooper->unregisterHandler(id());
        }*/
        ALOGI("~RTPControllerListener -");
    }
public:
    void setRTPController(sp<IRTPController>& rtpCon) {
        mRtpCon = rtpCon;
    }


    void onMessageReceived(const sp < AMessage > & msg) {
        switch(msg->what()) {
            /*
            case kWhatRtpPacket:
            {

                sp<ABuffer> rtpPacket;
                msg->findBuffer("packet",&rtpPacket);
                //mPacketNotify->setSize("rtp_flag",1);
                int32_t flag = imsma_rtp_packet;
                sp<AMessage> meta = rtpPacket->meta();

                meta->findInt32("flag",&flag);

                int32_t seqNum = rtpPacket->int32Data();

                sp<AMessage> response;
                msg->findMessage("response",&response);

                ALOGD("listener:kWhatRtpPacket,rtpPacket(0x%x),seqNum(%d),flag(%d),response(0x%x)",\
                    rtpPacket.get(),seqNum,flag,response.get());
                if((flag == imsma_rtp_packet) && rtpPacket.get()){
                    ALOGD("listener receiver one packet");
                    printf("listener receiver one packet \n");
                    mRtpCon->queuePacket(rtpPacket);
                    response->post();
                    ALOGD("post response");
                }
                if((flag == imsma_rtcp_packet) && rtpPacket.get()){
                    ALOGD("listener receiver one rtcp packet");
                    printf("listener receiver one rtcp packet \n");
                    mRtpCon->queuePacket(rtpPacket);
                    //response->post();

                }
                break;
            }
            */
        case kWhatAccu: {
            ALOGD("listener receiver one accu");
            printf("listener receiver one accu \n");
            accu_count++;

            sp<ABuffer> accessUnit;
            msg->findBuffer("access-unit", &accessUnit);

            sp<AMessage> accu_meta = accessUnit->meta();

            int64_t mediaTimeUs = 0;
            int64_t ntp_time = 0;

            accu_meta->findInt64("timeUs", &mediaTimeUs);
            accu_meta->findInt64("ntp-time",&ntp_time);

            int32_t ccw_rotation = 0;
            int32_t camera_facing = 0;
            int32_t flip = 0;
            accu_meta->findInt32("ccw_rotation",&ccw_rotation);
            accu_meta->findInt32("camera_facing",&camera_facing);
            accu_meta->findInt32("flip",&flip);

            ALOGI("RTPControllerListener,kWhatAccu,timeUs(%" PRId64 ")us,ntp-time(%" PRId64 ")us",\
                  mediaTimeUs,ntp_time);

            ALOGI("RTPControllerListener,kWhatAccu,ccw_rotation(%d),camera_facing(%d),flip(%d)",\
                  ccw_rotation,camera_facing,flip);
            /*

            sp<AMessage> response;
            msg->findMessage("response",&response);

            response->post();
            //test
            response->post();


            if(accu_count == 50){
                printf("stop downlink in other thread \n");
                ALOGI("stop downlink in other thread");
                mRtpCon->stop(IMSMA_RTP_DOWNLINK);

                printf("remove downlink in other thread \n");
                ALOGI("remove downlink in other thread ");
                mRtpCon->removeStream(IMSMA_RTP_DOWNLINK);

                printf("remove downlink in other thread done \n");
                ALOGI("remove downlink in other thread done");

            }

            //test fir
            if(accu_count == 10){
                printf("test decode err fir\n");
                ALOGI("test decode err fir");
                sp<AMessage> errInfo = new AMessage();
                errInfo->setInt32("err",IRTPController::DecErr_FullIntraRequest);
                mRtpCon->feedBackDecoderError(errInfo);
            }
            if(accu_count == 50){
                printf("test decode receiver refresh point\n");
                ALOGI("test decode receiver refresh point");

                mRtpCon->recvDecoderRefreshPoint();
            }

            //test AVPF during hold on
            //test pli
            if(accu_count == 70){
                printf("test decode err pli\n");
                ALOGI("test decode err pli");
                sp<AMessage> errInfo = new AMessage();
                errInfo->setInt32("err",IRTPController::DecErr_PictureLossIndication);
                mRtpCon->feedBackDecoderError(errInfo);
            }

            //test pli
            if(accu_count == 160){
                printf("test decode err pli\n");
                ALOGI("test decode err pli");
                sp<AMessage> errInfo = new AMessage();
                errInfo->setInt32("err",IRTPController::DecErr_PictureLossIndication);
                mRtpCon->feedBackDecoderError(errInfo);
            }
            //test sli
            if(accu_count == 170){
                printf("test decode err sli\n");
                ALOGI("test decode err sli");
                sp<AMessage> errInfo = new AMessage();
                sp<ABuffer> buffer = new ABuffer(4);
                uint8_t* data = buffer->data();
                //first=1,number=10,pictureid=30
                data[0] = 0;
                data[1] = 8;
                data[2] = 2;
                data[3] = 0x9E;

                errInfo->setBuffer("sli",buffer);
                errInfo->setInt32("err",IRTPController::DecErr_SliceLossIndication);
                mRtpCon->feedBackDecoderError(errInfo);
            }

            //test fir
            if(accu_count == 180){
                printf("test decode err fir\n");
                ALOGI("test decode err fir");
                sp<AMessage> errInfo = new AMessage();
                errInfo->setInt32("err",IRTPController::DecErr_FullIntraRequest);
                mRtpCon->feedBackDecoderError(errInfo);
            }
            /*
            if(accu_count == 150){
                //test updateConfig
                //new config param
                printf("test update config\n");
                ALOGI("test update config");
                List<rtp_rtcp_capability_t*> rtp_caps;

                rtp_caps = IRTPController::getCapability(IMSMA_RTP_VIDEO);
                rtp_rtcp_capability_t* rtp_cap_first = *(rtp_caps.begin());

                rtp_rtcp_config_t rtp_config;
                rtp_config.media_type = rtp_cap_first->media_type;
                rtp_config.mime_Type = rtp_cap_first->mime_Type;
                rtp_config.rtp_payload_type = payload_type_value;
                rtp_config.rtp_profile = IMSMA_RTP_AVPF;
                rtp_config.sample_rate = rtp_cap_first->sample_rate;
                rtp_config.rtp_packet_bandwidth = 2000; // 2Mbps

                rtp_config.packetize_mode = IMSMA_NON_INTERLEAVED_MODE;

                //change to not support CVO
                rtp_config.rtp_header_extension_num = 0;
                memset(rtp_config.rtp_ext_map,0,sizeof(rtp_cap_first->rtp_ext_map));

                //change rtcp bw
                rtp_config.rtcp_sender_bandwidth = 2000; // 2Kbps
                rtp_config.rtcp_receiver_bandwidth = 1000;//1Kbps

                rtp_config.rtcp_reduce_size = rtp_cap_first->rtcp_reduce_size;
                rtp_config.rtcp_fb_param_num = rtp_cap_first->rtcp_fb_param_num;
                memcpy(rtp_config.rtcp_fb_type,rtp_cap_first->rtcp_fb_type,sizeof(rtp_cap_first->rtcp_fb_type));

                network_info_t ni;
                ni.ebi = 0;
                ni.interface_type = 0;
                ni.dscp = 0;
                ni.soc_priority = 0;

                ni.remote_rtp_port = 40000;
                ni.remote_rtcp_port = 40001;
                ni.local_rtp_port = 40000;
                ni.local_rtcp_port = 40001;

                //ni.remote_rtp_address = {127,0,0,1};
                ni.remote_rtp_address[0] = 127;
                ni.remote_rtp_address[1] = 0;
                ni.remote_rtp_address[2] = 0;
                ni.remote_rtp_address[3] = 1;


                //ni.remote_rtcp_address = {127,0,0,1};
                ni.remote_rtcp_address[0] = 127;
                ni.remote_rtcp_address[1] = 0;
                ni.remote_rtcp_address[2] = 0;
                ni.remote_rtcp_address[3] = 1;

                //ni.local_rtp_address = {127,0,0,1};
                ni.local_rtp_address[0] = 127;
                ni.local_rtp_address[1] = 0;
                ni.local_rtp_address[2] = 0;
                ni.local_rtp_address[3] = 1;

                //ni.local_rtcp_address = {127,0,0,1};
                ni.local_rtcp_address[0] = 127;
                ni.local_rtcp_address[1] = 0;
                ni.local_rtcp_address[2] = 0;
                ni.local_rtcp_address[3] = 1;

                ni.remote_addr_type = ViLTE_IPv4;
                ni.local_addr_type = ViLTE_IPv4;

                ni.network_id = 0;
                ni.rtp_direction = ViLTE_RTP_DIRECTION_SENDRECV;

                //ni.socket_fds = {-1,-1,-1,...}
                memset(ni.socket_fds,-1,sizeof(ni.socket_fds));

                memcpy(&rtp_config.network_info,&ni,sizeof(ni));

                mRtpCon->updateConfigParams(&rtp_config);
            }
            */
            break;
        }
        case kWhatNotify: {
            ALOGD("listener receiver RTPController notify");
            printf("listener receiver RTPController notify \n");
            int32_t what = 0;
            msg->findInt32("what",&what);

            if(what == IRTPController::kWhatPLI) {
                printf("inform encoder PLI\n");
                ALOGI("inform encoder PLI");
            }

            if(what == IRTPController::kWhatFIR) {
                printf("inform encoder FIR\n");
                ALOGI("inform encoder FIR");
            }

            if(what == IRTPController::kWhatSLI) {
                printf("inform encoder SLI\n");
                ALOGI("inform encoder SLI");
                sp<ABuffer> sli_buffer;
                msg->findBuffer("sli",&sli_buffer);
                uint8_t* data = sli_buffer->data();

                uint32_t first = 0;
                uint32_t number = 0;
                uint8_t pid = 0;
                first = (data[0]<< 5) | ((data[1]& 0xF8) >> 3);
                number = ((data[1] & 0x07) << 10) | (data[2] << 2) | (data[3] >> 6);
                pid = data[3] & 0x3F;
                ALOGI("sli:first=%d,number=%d,pid=%d",first,number,pid);
                printf("sli:first=%d,number=%d,pid=%d\n",first,number,pid);
            }

            break;
        }

        default:
            ALOGE("should not be here,what(%d)",msg->what());
            break;
        }
    }

    sp<IRTPController> mRtpCon;
    uint32_t accu_count;
    //sp<ALooper> mLooper;
};

void print_help()
{
    printf("#@#RTPController test app!\n");
    //printf("example:RTPControllerTest filePath \n" );
    printf("example: RTPControllerTest -accu filepath \n");
    printf("example: RTPControllerTest -pcap xxx.pcap 99 \n");
    printf("example: RTPControllerTest -pcap xxx.pcap 99 -lost 200000 0 1000000 1 1000000 2 1000000 3 ...\n");
}

static uint16_t u16at(const uint8_t *data)
{
    return data[0] << 8 | data[1];
}

//int test_adaptation(){
//
//  test_DL_Path(char* file_name,char* payload_type,List<>)
//}
struct LostRate {
    uint32_t durUs;
    int32_t lostRate;
};

int test_DL_Path(char* file_name,char* payload_type,List<LostRate> lostRateList)
{
    ALOGI("file_name=%s",file_name);
    printf("file_name=%s \n",file_name);

    payload_type_value = atoi(payload_type);

    List<rtp_rtcp_capability_t*> rtp_caps;

    rtp_caps = IRTPController::getCapability(IMSMA_RTP_VIDEO);
    printf("get caps done \n");
    ALOGI("get Caps done");

    //sp<RTPControllerListener> rtpConListener = new RTPControllerListener();
    //rtpConListener = new RTPControllerListener();
    sp<RTPControllerListener> rtpConListener = new RTPControllerListener();
    ALOGI("new RTPControllerListener(0x%x)",rtpConListener.get());


    sp<ALooper> mLooper = new ALooper();
    mLooper->setName("Test listener looper");
    mLooper->registerHandler(rtpConListener);
    mLooper->start();

    const sp<AMessage> notify = new AMessage(kWhatNotify,rtpConListener);
    printf("new RTPController \n");
    ALOGI("new RTPController");
    //sp<RTPController> rtpCon = new RTPController();
    sp<IRTPController> rtpCon = IRTPController::createRTPController(0,1);

    //sp<IRTPController> irtpCon = dynamic_cast<IRTPController*>(rtpCon.get());
    rtpConListener->setRTPController(rtpCon);
    rtpCon->setEventNotify(notify);

    rtp_rtcp_capability_t* rtp_cap_first = * (rtp_caps.begin());

    rtp_rtcp_config_t rtp_config;
    rtp_config.media_type = rtp_cap_first->media_type;
    rtp_config.mime_Type = rtp_cap_first->mime_Type;

    rtp_config.rtp_payload_type = payload_type_value;
    rtp_config.rtp_profile = IMSMA_RTP_AVPF;
    rtp_config.sample_rate = rtp_cap_first->sample_rate;
    rtp_config.rtp_packet_bandwidth = 2000; // 2Mbps

    rtp_config.packetize_mode = IMSMA_NON_INTERLEAVED_MODE;
    rtp_config.rtp_header_extension_num = rtp_cap_first->rtp_header_extension_num;
    memcpy(rtp_config.rtp_ext_map,rtp_cap_first->rtp_ext_map,sizeof(rtp_cap_first->rtp_ext_map));
    rtp_config.rtcp_sender_bandwidth = 8000; // 8Kbps
    rtp_config.rtcp_receiver_bandwidth = 6000;//6Kbps
    rtp_config.rtcp_reduce_size = rtp_cap_first->rtcp_reduce_size;
    rtp_config.rtcp_fb_param_num = rtp_cap_first->rtcp_fb_param_num;
    memcpy(rtp_config.rtcp_fb_type,rtp_cap_first->rtcp_fb_type,sizeof(rtp_cap_first->rtcp_fb_type));

    network_info_t ni;
    ni.ebi = 0;
    ni.interface_type = 0;
    ni.dscp = 0;
    ni.soc_priority = 0;

    ni.remote_rtp_port = 40000;
    ni.remote_rtcp_port = 40001;
    ni.local_rtp_port = 40000;
    ni.local_rtcp_port = 40001;

    //ni.remote_rtp_address = {127,0,0,1};
    ni.remote_rtp_address[0] = 127;
    ni.remote_rtp_address[1] = 0;
    ni.remote_rtp_address[2] = 0;
    ni.remote_rtp_address[3] = 1;


    //ni.remote_rtcp_address = {127,0,0,1};
    ni.remote_rtcp_address[0] = 127;
    ni.remote_rtcp_address[1] = 0;
    ni.remote_rtcp_address[2] = 0;
    ni.remote_rtcp_address[3] = 1;

    //ni.local_rtp_address = {127,0,0,1};
    ni.local_rtp_address[0] = 127;
    ni.local_rtp_address[1] = 0;
    ni.local_rtp_address[2] = 0;
    ni.local_rtp_address[3] = 1;

    //ni.local_rtcp_address = {127,0,0,1};
    ni.local_rtcp_address[0] = 127;
    ni.local_rtcp_address[1] = 0;
    ni.local_rtcp_address[2] = 0;
    ni.local_rtcp_address[3] = 1;

    ni.remote_addr_type = ViLTE_IPv4;
    ni.local_addr_type = ViLTE_IPv4;

    ni.network_id = 0;
    strcpy(ni.ifname, "ccmni4");
    ni.rtp_direction = ViLTE_RTP_DIRECTION_SENDRECV;

    //ni.socket_fds = {-1,-1,-1,...}
    memset(ni.socket_fds,-1,sizeof(ni.socket_fds));

    memcpy(&rtp_config.network_info,&ni,sizeof(ni));

    rtpCon->setConfigParams(&rtp_config,IMSMA_RTP_VIDEO);

    /*
    //test another getCapability interface
    rtp_rtcp_capability_t* pRTPCap;
    uint8_t num = 0;
    IRTPController::getCapability(&pRTPCap,&num,IMSMA_RTP_VIDEO);
    ALOGD("getCapability,video pRTPCap(0x%x),num(%d)",pRTPCap,num);

    rtpCon->setConfigParams(pRTPCap,IMSMA_RTP_VIDEO);
    //end of test
    */
    const sp<AMessage> accuNotify = new AMessage(kWhatAccu,rtpConListener);

    rtpCon->setAccuNotify(accuNotify,IMSMA_RTP_VIDEO);

    rtpCon->addStream();// UP and down link

    printf("start RTPContorller \n");
    ALOGI("start RTPContorller");

    rtpCon->start();

    //create socket info
    //create socket for sending data to  127.0.0.1,port:40000
    //will confuse if two socket bind to same address and port
    //use socketWrapper directly

    sp<SocketWrapper> socket_Wrapper = ((RTPController*)(rtpCon.get()))->getVideoRTPSocketWrapper();
    //sp<SocketWrapper> socket_Wrapper = rtpCon->getVideoRTPSocketWrapper();

    //read a pcap file
    printf("open %s \n",file_name);
    ALOGI("open %s",file_name);
    int Fd = -1;
    Fd = open(file_name, O_LARGEFILE | O_RDONLY);

    if(Fd < 0) {
        printf("open %s fail \n",file_name);
        return -1;
    }

    off_t offset = 0;
    ssize_t real_size = 0;
    off_t mLength = lseek(Fd, 0, SEEK_END);
    printf("file lenth = 0x%x \n",mLength);
    ALOGI("file lenth = 0x%x",mLength);

    //read all the file to data
    sp<ABuffer> file_data = new ABuffer(mLength);
    off_t result = lseek(Fd, 0, SEEK_SET);
    real_size = read(Fd,file_data->data(),mLength);

    printf("read whole file to buffer %d \n",real_size);

    ALOGI("read whole file to buffer %d",real_size);
    file_data->setRange(file_data->offset(),real_size);

    const uint8_t *data = file_data->data();
    size_t size = file_data->size();

    //parse pcap
    offset += 24; //pcap file header

    uint32_t pcap_num = 0;
    uint64_t last_time_us = 0;
    uint64_t first_time_us = 0;

    bool apply_lost = false;
    List<LostRate>::iterator i;
    int32_t current_lostRate = 0;
    uint32_t current_dur_end = 0;

    if(lostRateList.size() > 0) {
        apply_lost = true;
        i = lostRateList.begin();
        current_lostRate = (*i).lostRate;
        current_dur_end = (*i).durUs;
    }

    float packet_num_with_lost = 1.0;

    while(offset < mLength) {
        ALOGI("offset=%d",offset);
        printf("offset=%d\n",offset);
        pcap_num++;

        ALOGI("pcap:0x%x,0x%x,0x%x,0X%x",data[offset],data[offset+1],data[offset+2],data[offset+3]);

        //parse one pcap packet
        //uint32_t time_s_part = (uint32_t)(data[offset]); //time s
        //uint32_t time_us_part = (uint32_t)(data[offset + 4]);//time us
        //uint64_t time_us = time_s_part * 1000000 + time_us_part;
        uint32_t time_s_part = (uint32_t)(data[offset + 3] << 24 | data[offset + 2] << 16 | data[offset + 1] << 8 | data[offset]);      //time s
        uint32_t time_us_offset = offset + 4;

        uint32_t time_us_part = (uint32_t)(data[time_us_offset + 3] << 24 | data[time_us_offset + 2] << 16 | data[time_us_offset + 1] << 8 | data[time_us_offset]);      //time us
        uint64_t time_us = time_s_part * 1000000 + time_us_part;
        ALOGI("time_s_part=%d,time_us_par=%d,time_us=%lld",time_s_part,time_us_part,time_us);

        if(pcap_num == 1) {
            last_time_us = time_us;
            first_time_us = time_us;
        }

        int64_t delay_us = time_us - last_time_us;
        ALOGI("pcap num =%d,delta from previous frame = %lld us",pcap_num,delay_us);
        printf("pcap num =%d,delta from previous frame = %lld us\n",pcap_num,delay_us);

        int64_t dur_from_first = time_us - first_time_us;

        uint32_t pcap_len_offset = offset + 8;
        uint32_t pcap_packet_length = (uint32_t)(data[pcap_len_offset + 3] << 24 | data[pcap_len_offset + 2] << 16 | data[pcap_len_offset + 1] << 8 | data[pcap_len_offset]);
        ALOGI("pcap_packet_length = %d",pcap_packet_length);
        printf("pcap_packet_length = %d\n",pcap_packet_length);

        uint32_t ip_offset = offset + 16 + 16; //16 for pcap packet header,16 for linux cooked capture
        ALOGI("ip_offset = %d",ip_offset);
        printf("ip_offset = %d\n",ip_offset);
        ALOGI("data[ip_offset + 4] = 0x%x,data[ip_offset + 5]=0x%x.",data[ip_offset + 4],data[ip_offset + 5]);
        uint16_t ip_payload_length = (data[ip_offset + 4] << 8) | data[ip_offset + 5];

        ALOGI("pcap num =%d,ip payload length = %d",pcap_num,ip_payload_length);
        printf("pcap num =%d,ip payload length = %d\n",pcap_num,ip_payload_length);
        //ip header
        //uint32_t udp_offset =

        uint32_t rtp_offset = offset + 16 + 16 + 40 + 8;////16 for pcap packet header,16 for linux cooked captur,40 for ipv6,8 for udp

        uint32_t rtp_size = ip_payload_length - 8;

        sp<ABuffer> buffer = new ABuffer(rtp_size);
        memcpy(buffer->data(),data + rtp_offset,rtp_size);

        const uint8_t *rtp_data = buffer->data();
        uint16_t seqN = u16at(&rtp_data[2]);
        printf("seqN = %d\n",seqN);
        //test hold->resume, seq number became small case
        /*
        if(seqN == 200){
            ALOGD("peerPausedSendStream");
            printf("peerPausedSendStream\n");
            rtpCon->peerPausedSendStream();
        }*/

        if(delay_us<0)
            delay_us = 0;

        usleep(delay_us);

        //test PeerResumeSendStream
        /*
        if(seqN == 300){
            ALOGD("peerResumedSendStream");
            printf("peerResumedSendStream\n");
            rtpCon->peerResumedSendStream();
        }
        */
        //for monitor packet lost
        bool drop = false;

        if(apply_lost) {
            //update current lost rate
            if((i != lostRateList.end()) && (dur_from_first > current_dur_end)) {
                i = lostRateList.erase(i);

                if(i != lostRateList.end()) {
                    current_dur_end += (*i).durUs;
                    current_lostRate = (*i).lostRate;
                    ALOGI("current_dur_end =%d,current_lostRate=%d",current_dur_end,current_lostRate);
                } else {
                    current_lostRate = 0;
                }
            }

            float factor = (float) 100.0 / (100 - current_lostRate);

            if(pcap_num != ((uint32_t) packet_num_with_lost)) {
                drop = true;
            } else {
                packet_num_with_lost += factor;
            }
        }

        if(!drop) {
            printf("write to Socket\n");
            //write to socket
            int write_size = 0;

            if(socket_Wrapper.get()) {
                write_size = socket_Wrapper->writeSock(buffer);
            }

            if(write_size < 0) {
                ALOGE("%s,packet send fail",__FUNCTION__);
                //no need re-send for rtcp
            } else if((uint32_t) write_size != (buffer->size())) {
                ALOGE("%s,writeSock write partial data:%d/%d",\
                      __FUNCTION__,write_size,buffer->size());
            }
        }

        last_time_us = time_us;
        offset += (16 + pcap_packet_length);

    }

    printf("file read done \n");
    ALOGI("file read done");
    sleep(10);    // sleep 10s

    printf("stop RTPController \n");
    ALOGD("stop RTPController");
    rtpCon->stop();
    printf("stop listener looper \n");
    ALOGD("stop listener looper");

    rtpCon->removeStream();

    mLooper->stop();
    mLooper->unregisterHandler(rtpConListener->id());
    printf("release listener and controller \n");
    ALOGD("release listener and controller");
    rtpCon = NULL;
    rtpConListener = NULL;
    printf("return \n");
    ALOGD("main return");
    sleep(1);
    return 0;



}
int test_UL_DL_Path(char* file_name)
{
    ALOGI("file_name=%s",file_name);
    printf("file_name=%s \n",file_name);


    List<rtp_rtcp_capability_t*> rtp_caps;

    rtp_caps = IRTPController::getCapability(IMSMA_RTP_VIDEO);
    printf("get caps done \n");
    ALOGI("get Caps done");

    //sp<RTPControllerListener> rtpConListener = new RTPControllerListener();
    //rtpConListener = new RTPControllerListener();
    sp<RTPControllerListener> rtpConListener = new RTPControllerListener();
    ALOGI("new RTPControllerListener(0x%x)",rtpConListener.get());


    sp<ALooper> mLooper = new ALooper();
    mLooper->setName("Test listener looper");
    mLooper->registerHandler(rtpConListener);
    mLooper->start();

    const sp<AMessage> notify = new AMessage(kWhatNotify,rtpConListener);
    printf("new RTPController \n");
    ALOGI("new RTPController");
    //sp<RTPController> rtpCon = new RTPController(notify);
    sp<IRTPController> rtpCon = IRTPController::createRTPController(0,1);

    rtpConListener->setRTPController(rtpCon);
    rtpCon->setEventNotify(notify);

    rtp_rtcp_capability_t* rtp_cap_first = * (rtp_caps.begin());

    rtp_rtcp_config_t rtp_config;
    rtp_config.media_type = rtp_cap_first->media_type;
    rtp_config.mime_Type = IMSMA_RTP_VIDEO_HEVC;
    rtp_config.rtp_payload_type = payload_type_value;
    rtp_config.rtp_profile = IMSMA_RTP_AVPF;
    rtp_config.sample_rate = rtp_cap_first->sample_rate;
    rtp_config.rtp_packet_bandwidth = 2000; // 2Mbps

    rtp_config.packetize_mode = IMSMA_NON_INTERLEAVED_MODE;
    rtp_config.rtp_header_extension_num = rtp_cap_first->rtp_header_extension_num;
    memcpy(rtp_config.rtp_ext_map,rtp_cap_first->rtp_ext_map,sizeof(rtp_cap_first->rtp_ext_map));
    rtp_config.rtcp_sender_bandwidth = 8000; // 8Kbps
    rtp_config.rtcp_receiver_bandwidth = 6000;//6Kbps
    rtp_config.rtcp_reduce_size = rtp_cap_first->rtcp_reduce_size;
    rtp_config.rtcp_fb_param_num = rtp_cap_first->rtcp_fb_param_num;
    memcpy(rtp_config.rtcp_fb_type,rtp_cap_first->rtcp_fb_type,sizeof(rtp_cap_first->rtcp_fb_type));

    network_info_t ni;
    ni.ebi = 0;
    ni.interface_type = 0;
    ni.dscp = 0;
    ni.soc_priority = 0;

    ni.remote_rtp_port = 40000;
    ni.remote_rtcp_port = 40001;
    ni.local_rtp_port = 40000;
    ni.local_rtcp_port = 40001;

    //ni.remote_rtp_address = {127,0,0,1};
    ni.remote_rtp_address[0] = 127;
    ni.remote_rtp_address[1] = 0;
    ni.remote_rtp_address[2] = 0;
    ni.remote_rtp_address[3] = 1;


    //ni.remote_rtcp_address = {127,0,0,1};
    ni.remote_rtcp_address[0] = 127;
    ni.remote_rtcp_address[1] = 0;
    ni.remote_rtcp_address[2] = 0;
    ni.remote_rtcp_address[3] = 1;

    //ni.local_rtp_address = {127,0,0,1};
    ni.local_rtp_address[0] = 127;
    ni.local_rtp_address[1] = 0;
    ni.local_rtp_address[2] = 0;
    ni.local_rtp_address[3] = 1;

    //ni.local_rtcp_address = {127,0,0,1};
    ni.local_rtcp_address[0] = 127;
    ni.local_rtcp_address[1] = 0;
    ni.local_rtcp_address[2] = 0;
    ni.local_rtcp_address[3] = 1;

    ni.remote_addr_type = ViLTE_IPv4;
    ni.local_addr_type = ViLTE_IPv4;

    ni.network_id = 0;
    strcpy(ni.ifname, "ccmni4");
    ni.rtp_direction = ViLTE_RTP_DIRECTION_SENDRECV;

    //ni.socket_fds = {-1,-1,-1,...}
    memset(ni.socket_fds,-1,sizeof(ni.socket_fds));

    memcpy(&rtp_config.network_info,&ni,sizeof(ni));

    rtpCon->setConfigParams(&rtp_config,IMSMA_RTP_VIDEO);

    /*
    //test another getCapability interface
    rtp_rtcp_capability_t* pRTPCap;
    uint8_t num = 0;
    IRTPController::getCapability(&pRTPCap,&num,IMSMA_RTP_VIDEO);
    ALOGD("getCapability,video pRTPCap(0x%x),num(%d)",pRTPCap,num);

    rtpCon->setConfigParams(pRTPCap,IMSMA_RTP_VIDEO);
    //end of test
    */
    const sp<AMessage> accuNotify = new AMessage(kWhatAccu,rtpConListener);

    rtpCon->setAccuNotify(accuNotify,IMSMA_RTP_VIDEO);

    rtpCon->addStream();// UP and down link

    printf("start RTPContorller \n");
    ALOGI("start RTPContorller");

    rtpCon->start();

    //read a bitstream file
    printf("open %s \n",file_name);
    ALOGI("open %s",file_name);
    int Fd = -1;
    Fd = open(file_name, O_LARGEFILE | O_RDONLY);

    if(Fd < 0) {
        printf("open %s fail \n",file_name);
        return -1;
    }

    // for overflow test
    int64_t timestampUs = 47719878844;//0;
    off_t offset = 0;
    ssize_t real_size = 0;
    off_t mLength = lseek(Fd, 0, SEEK_END);
    printf("file lenth = 0x%x \n",mLength);
    ALOGI("file lenth = 0x%x",mLength);

    //read all the file to data
    sp<ABuffer> file_data = new ABuffer(mLength);
    off_t result = lseek(Fd, 0, SEEK_SET);
    real_size = read(Fd,file_data->data(),mLength);

    printf("read whole file to buffer %d \n",real_size);

    ALOGI("read whole file to buffer %d",real_size);
    file_data->setRange(file_data->offset(),real_size);

    const uint8_t *data = file_data->data();
    size_t size = file_data->size();
    printf("buffer size =%d \n",size);
    const uint8_t *nalStart = NULL;
    size_t nalSize = 0;
    uint32_t nal_num = 0;

    while(getNextNALUnit(
                &data, &size, &nalStart, &nalSize,
                true /* startCodeFollows */) == OK) {

        printf("get one NAL(num:%d,offset:0x%x,size:%d \n)",nal_num,nalStart,nalSize);
        ALOGI("get one NAL(num:%d,offset:0x%x,size:%d)",nal_num,nalStart,nalSize);

        sp<ABuffer> accu = new ABuffer(nalSize + 4);
        uint8_t* nal_data = accu->data();
        memcpy(nal_data, "\x00\x00\x00\x01", 4);

        memcpy(nal_data+4,nalStart,nalSize);

        sp<AMessage> accu_meta = accu->meta();
        accu_meta->setInt64("timeUs",timestampUs);
        accu_meta->setInt32("ccw_rotation",180);

        timestampUs = timestampUs + 33000l;
        rtpCon->queueAccessUnit(accu);

        usleep(33000);    //sleep 33ms

        /*
        nal_num ++;
        if(nal_num == 100){
            //test hold on
            printf("test hold start\n");
            ALOGI("test hold start");
            rtpCon->stop();
            sleep(10);

            printf("test hold back\n");
            ALOGI("test hold back");
            rtpCon->start();
        }*/
    }

    printf("file read done \n");
    ALOGI("file read done");
    sleep(5);    // sleep 5s

    printf("stop RTPController \n");
    ALOGD("stop RTPController");
    rtpCon->stop();
    printf("stop listener looper \n");
    ALOGD("stop listener looper");

    rtpCon->removeStream();

    mLooper->stop();
    mLooper->unregisterHandler(rtpConListener->id());
    printf("release listener and controller \n");
    ALOGD("release listener and controller");
    rtpCon = NULL;
    rtpConListener = NULL;
    printf("return \n");
    ALOGD("main return");
    sleep(1);
    return 0;

}
//static RTPControllerListener* rtpConListener;
int main(int argc, char **argv)
{
    /*
    if(argc != 3)
    {
        print_help();
        return 0;
    }
    */
    //ToCheck: need this?
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    char* file_name = argv[2];

    bool apply_lost = false;

    int j = 0;

    for(j = 0; j < argc; j++) {
        if(strcmp(argv[j],"-lost") == 0) {
            break;
        }
    }

    if(j < (argc-2)) {
        apply_lost = true;
    }

    List<LostRate> lostRateList;

    if(apply_lost) {
        //List<LostRate> lostRateList;
        for(int i = 0; i < (argc - j-1) /2; i++) {
            LostRate lr;
            lr.durUs = atoi(argv[j+1+i*2]);
            lr.lostRate = atoi(argv[j+1+i*2+1]);
            lostRateList.push_back(lr);
        }

        if(strcmp(argv[1],"-pcap") == 0) {
            printf("simuluate lost rate \n");
            printf("example: RTPControllerTest -pcap xxx.pcap 99 -lost 200000 0 1000000 1 1000000 2 1000000 3 ...\n");
            test_DL_Path(file_name,argv[3],lostRateList);
        }
    } else {
        //example: RTPControllerTest -pcap xxx.pcap 99

        if(strcmp(argv[1],"-pcap") == 0) {
            test_DL_Path(file_name,argv[3],lostRateList);

        } else if(strcmp(argv[1],"-accu") == 0) {
            test_UL_DL_Path(file_name);
        } else {
            print_help();
        }
    }

    printf("test finish!\n");
    exit(0);
    return 0;

}





