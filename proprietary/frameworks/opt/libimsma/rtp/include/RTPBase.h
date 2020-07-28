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
* RTPBase.h
*
* Project:
* --------
*   Android
*
* Description:
* ------------
*  Define RTP capabilty struct for ViLTE
*  keep all the enum  in sync with UA, could not changing the value without sync with UA
*
* Author:
* -------
*   Haizhen.Wang(mtk80691)
*
****************************************************************************/

#ifndef _IMS_RTP_BASE_H_
#define _IMS_RTP_BASE_H_
#include <stdint.h>

//namespace imsma {
namespace android
{

//keep equal with VTSocketBind.h :MAX_PORT_RESERVED
#define VILTE_BIND_FD_NUMBER 32
//keep equal with SocketWrapper.h : VOLTE_IPADDR_LENGTH
#define VILTE_IPADDR_LENGTH 16

//track media type
/*
enum media_type {
    VIDEO = 0x01,
    AUDIO = 0x02,
    TRANSPORT_STREAM = 0x04,
    VIDEO_AUDIO = VIDEO|AUDIO,
};*/
enum media_type {
    IMSMA_RTP_VIDEO = 0x01,
    IMSMA_RTP_AUDIO = 0x02,
    IMSMA_RTP_TRANSPORT_STREAM = 0x04,
    IMSMA_RTP_VIDEO_AUDIO = IMSMA_RTP_VIDEO|IMSMA_RTP_AUDIO,
};

enum rtp_path {
    IMSMA_RTP_UPLINK = 0x01,
    IMSMA_RTP_DOWNLINK = 0x02,
    IMSMA_RTP_UP_DOWN_LINK = IMSMA_RTP_UPLINK|IMSMA_RTP_DOWNLINK,
};
/*
//media codec type
enum mime_Type{
    VIDEO_H264 = 1,
    VIDEO_H263 = 2,
    VIDEO_MPEG4 = 3,
    VIDEO_HEVC = 4,

    AUDIO_AMR = 11,
    AUDIO_AAC = 12,
};*/
enum mime_Type {
    IMSMA_RTP_VIDEO_H264 = 1,
    IMSMA_RTP_VIDEO_H263 = 2,
    IMSMA_RTP_VIDEO_MPEG4 = 3,
    IMSMA_RTP_VIDEO_HEVC = 4,

    IMSMA_RTP_AUDIO_AMR = 11,
    IMSMA_RTP_AUDIO_AAC = 12,
};

/*
enum rtp_profile {
    RTP_AVP = 0x01, //rtp Audio-Visual Profile
    RTP_AVPF = 0x02, //rtp Audio-Visual feedback profile
};*/
enum rtp_profile {
    IMSMA_RTP_AVP = 0x01, //rtp Audio-Visual Profile
    IMSMA_RTP_AVPF = 0x02, //rtp Audio-Visual feedback profile
};


//H264 packetization mode
enum packetization_mode {
    IMSMA_VIDEO_H264_RTP_PACK_MODE_START = 0,
    IMSMA_SINGLE_NAL_MODE = 0x01,
    IMSMA_NON_INTERLEAVED_MODE = 0x02,
    IMSMA_INTERLEAVED_MODE =0x04,
    IMSMA_VIDEO_H264_RTP_PACK_MODE_END,
};


enum rtcp_fd_id {
    IMSMA_ACK = 1,
    IMSMA_NACK = 2,
    IMSMA_TRR_INT = 3,
    IMSMA_CCM = 4,
    IMSMA_APP = 5,
};
enum rtcp_fb_param {
    IMSMA_NONE = 0,
    IMSMA_GENERIC_NACK,
    IMSMA_PLI,
    IMSMA_SLI,
    IMSMA_RPSI,
    IMSMA_FIR,
    IMSMA_TMMBR,
    IMSMA_TSTR,
    IMSMA_VBCM,
};

/*@ rtcp feed back message type
 *@ may be "ack rpsi" "nack pli" "nack tmmbr smaxpr=120"
 *@ rtcp_fb_param only has one param,
 *@ if support several params, need each rtcp_fb_type struct for each param
 */
typedef struct {
    uint16_t rtcp_fb_id; // such as "ack","nack","trr-int","ccm","app¡°
    uint16_t rtcp_fb_param; // such as generic nack, pli,sli,rpsi,fir,tmmbr,tstr,vbcm
    char rtcp_fb_sub_param[128]; // LEN 128 , such as smaxpr=120 of tmmbr
} rtcp_fb_param_type_t;

//TODO
//RTP header extensions: such as Orientation,gps
//such as orientation--CVO
//a=extmap:7 urn:3gpp:video-orientation--TS 26.114 6.2.3

typedef struct {
    uint8_t extension_id; //1-14 for one-bytes mode, only one-bytes mode used for CVO
    uint8_t direction; //maybe used future
    char extension_uri[256];// extension uri size maybe bigger than 256, discuss change to string
    //char extension_attributes[256] ;//maybe used future
} rtp_ext_map_t;

//keep align with UA definition
enum ViLTE_RTP_DIRECTION_e {
    ViLTE_RTP_DIRECTION_INACTIVE = 0,     ///< RTP session suspends. Inactive also is default value.
    ViLTE_RTP_DIRECTION_SENDONLY,         ///< Send only
    ViLTE_RTP_DIRECTION_RECVONLY,         ///< Receive only
    ViLTE_RTP_DIRECTION_SENDRECV,         ///< Send and Receive
    /*add here*/
    ViLTE_RTP_DIRECTION_Max,
};

enum ViLTE_IP_Version_e {
    ViLTE_IPv4 = 0,
    ViLTE_IPv6,
    /*add here*/
    ViLTE_IPv_Max,
};

/*network info
*/
typedef struct  {
    /*ebi*/
    uint32_t ebi;

    /*wifi or 4G*/
    uint16_t interface_type;/*0(LTE),1(WIFI)*/
    /*ip header needed info*/
    uint32_t dscp;
    uint32_t soc_priority;

    /*Socket paramerters*/
    uint16_t remote_rtp_port;
    uint16_t remote_rtcp_port;
    uint16_t local_rtp_port;
    uint16_t local_rtcp_port;
    uint8_t remote_rtp_address[VILTE_IPADDR_LENGTH];
    uint8_t remote_rtcp_address[VILTE_IPADDR_LENGTH];
    uint8_t local_rtp_address[VILTE_IPADDR_LENGTH];
    uint8_t local_rtcp_address[VILTE_IPADDR_LENGTH];
    uint8_t remote_addr_type;
    uint8_t local_addr_type;

    uint32_t network_id;
    char ifname[16];
    uid_t uid;
    /*Call Inactive? Send only? Recv Only? Send+Recv?
    used to calculate RTCP interval*/
    uint8_t rtp_direction;

    int32_t socket_fds[VILTE_BIND_FD_NUMBER];

    /*Network explicit indication MBR changed*/
    uint32_t MBR_DL;//in kbps
    uint32_t MBR_UL; // in kbps

    uint32_t tag; //for datausage
} network_info_t;


/*@ rtp_rtcp_capability represent the rtp and rtcp related capabilitys related to specific mimetype
 *@
 *@ contain rtp capability  represent by rtp_packet_param
 *@ and rtcp capability represent by rtcp_packet_para
 *@
 */
typedef struct {
    uint8_t media_type; //such as video/audio
    uint8_t mime_Type; // such as H.264
    //uint8_t rtp_payload_type;// num such as: 97,98, 100
    uint8_t rtp_profile;//supported rtp_profile for special media such as RTP/AVP,RTP/AVPF
    uint32_t sample_rate;//timescale, which use to calculte rtp timestamp

    /*@ rtp packet transmission param
    *@ contain b=AS,PT,packetization params related to the codec type
    *@ ----------------------------------------------------------------------
    *@consider the negotiated params also can set through rtp_rtcp_capability
    *@ rtp_rtcp_capability must be can for one explicit item
    *@ then should not contain alternatives in rtp_packetize_params,such as "packetization-moe=1;packetization-mode=0"
    *@ ----------------------------------------------------------------------
    */
    //uint32_t rtp_packet_bandwidth; //b=AS in kb/s
    uint32_t packetize_mode;

    //TODO
    //RTP header extensions: such as Orientation,gps
    //

    uint32_t rtp_header_extension_num;
    rtp_ext_map_t rtp_ext_map[4];//now only CVO support


    //uint64_t rtcp_sender_bandwidth; //b=RS in bps
    //uint64_t rtcp_receiver_bandwidth; //b=RR in bps
    uint32_t rtcp_reduce_size; //a=rtcp_rsize

    /*@ rtcp packet transmission param
     *@ contain b=RS,b=RR,and feedback message type
     *@ may be serveral feedback message supported like:
     *@ ---------sdp--------------
     *@ a=rtcp-fb:98 ccm tstr
     *@ a=rtcp-fb:98 ccm fir
     *@ a=rtcp-fb:* ccm tmmbr smaxpr=120
     *@ ---------------------------
     *@ so need a array of rtcp_fb_type_t to represent all thest feedback messages
     *@ rtcp_fb_param_num represent the size of the array
     *@ maybe different rtcp fb support for different codec
     */
    uint32_t rtcp_fb_param_num;
    rtcp_fb_param_type_t  rtcp_fb_type[8]; // SIZE  32
} rtp_rtcp_capability_t;

typedef struct {
    uint8_t media_type; //such as video/audio
    uint8_t mime_Type; // such as H.264
    uint8_t rtp_payload_type;// num such as: 97,98, 100
    uint8_t rtp_profile;//supported rtp_profile for special media such as RTP/AVP,RTP/AVPF
    uint32_t sample_rate;//timescale, which use to calculte rtp timestamp

    /*@ rtp packet transmission param
    *@ contain b=AS,PT,packetization params related to the codec type
    *@ ----------------------------------------------------------------------
    *@consider the negotiated params also can set through rtp_rtcp_capability
    *@ rtp_rtcp_capability must be can for one explicit item
    *@ then should not contain alternatives in rtp_packetize_params,such as "packetization-moe=1;packetization-mode=0"
    *@ ----------------------------------------------------------------------
    */
    uint32_t rtp_packet_bandwidth; //b=AS in kb/s
    uint32_t packetize_mode;

    //TODO
    //RTP header extensions: such as Orientation,gps
    //

    uint32_t rtp_header_extension_num;
    rtp_ext_map_t rtp_ext_map[4];//now only CVO support


    uint64_t rtcp_sender_bandwidth; //b=RS in bps
    uint64_t rtcp_receiver_bandwidth; //b=RR in bps
    uint32_t rtcp_reduce_size; //a=rtcp_rsize

    /*@ rtcp packet transmission param
     *@ contain b=RS,b=RR,and feedback message type
     *@ may be serveral feedback message supported like:
     *@ ---------sdp--------------
     *@ a=rtcp-fb:98 ccm tstr
     *@ a=rtcp-fb:98 ccm fir
     *@ a=rtcp-fb:* ccm tmmbr smaxpr=120
     *@ ---------------------------
     *@ so need a array of rtcp_fb_type_t to represent all thest feedback messages
     *@ rtcp_fb_param_num represent the size of the array
     */
    uint32_t rtcp_fb_param_num;
    rtcp_fb_param_type_t  rtcp_fb_type[8]; // SIZE  32

    /* network info
    */
    network_info_t network_info;
} rtp_rtcp_config_t;

enum {
    imsma_sourceID   = 0xdeadbeef,
};
enum {
    imsma_rtp_ReservedHeaderSize = 0,//16,
    imsma_rtp_MTU_size = 1200,
};
enum {
    IMSMA_RTP_IDEL,
    IMSMA_RTP_START,
    IMSMA_RTP_PAUSE,
    IMSMA_RTP_STOP,
};

enum {
    IMSMA_CAMERA_FACING_UNKNOW = 0,
    IMSMA_CAMERA_FACING_FRONT = 0,
    IMSMA_CAMERA_FACING_BACK = 1,
};
enum {
    IMSMA_CAMERA_NO_FLIP = 0,
    IMSMA_CAMERA_HORIZON_FLIP = 1,
};

enum {
    IMSMA_IP_IPSec_UDP_HEADER = 200, //200bytes
};
}
#endif

