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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "libmdmonitor.h"
#include <assert.h>
#include <string>
using namespace libmdmonitor;

enum ICDCodecApiReturnCode {
    ICD_CODEC_RET_E_FILE_NOT_EXIST = -1000,
    ICD_CODEC_RET_E_DB_INVALID = -900,
    ICD_CODEC_RET_E_DB_STRUCT_UNDEFINED,
    ICD_CODEC_RET_E_DB_STRUCT_INVALID,
    ICD_CODEC_RET_E_DB_FIELD_TYPE_UNDEFINED,
    ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE = -800,
    ICD_CODEC_RET_E_HEADER_UNSUPPORTED_TIME_TYPE,
    ICD_CODEC_RET_E_HEADER_ICD_TYPE_UNDEFINED,
    ICD_CODEC_RET_E_HEADER_CHECKSUM_ERROR,
    ICD_CODEC_RET_E_PAYLOAD_ICD_CODE_UNDEFINED = -700,
    ICD_CODEC_RET_E_PAYLOAD_INVALID_BUFFER_SIZE,
    ICD_CODEC_RET_E_PAYLOAD_FIELD_UNDEFINED,
    ICD_CODEC_RET_E_PAYLOAD_FIELD_VALUE_OUT_OF_RANGE,
    ICD_CODEC_RET_E_OUTPUT_BUFFER_SIZE_TOO_SMALL = - 600,

    ICD_CODEC_RET_E_INVALID_ARGUMENTS = -500,
    ICD_CODEC_RET_E_NOT_SUPPORT,
    ICD_CODEC_RET_E_FAIL = 0,
    ICD_CODEC_RET_S_OK = 1
};

typedef struct {
    unsigned char type:4;               //4bit
    unsigned char version:4;            //4bit
    unsigned short total_len;           //16bit
    unsigned char timestamp_type:4;     //4bit
    unsigned char protocol_id:4;        //4bit
    unsigned short event_code;          //16bit
    unsigned short header_checksum;     //16bit
    unsigned long long timestamp;       //1 or 2 or 4 or 8 byte
    unsigned int size;  //an integer to record the actually byte-usage of this ICD packet
} IcdEventHeader;

typedef struct {
    unsigned char type:4;           //4bit
    unsigned char version:4;        //4bit
    unsigned int total_len:24;      //24bit
    unsigned char timestamp_type:4; //4bit
    unsigned char protocol_id:4;    //4bit
    unsigned char reserved;         //8bit
    unsigned short record_code;     //16bit
    unsigned int header_checksum;   //32bit
    unsigned long long timestamp;   //8byte or 4 byte
    unsigned int size;  //an integer to record the actually byte-usage of this ICD packet
} IcdRecordHeader;

class IcdDecoder {
public:
    // IcdDecoder();
    IcdDecoder();
    virtual ~IcdDecoder();
    bool ParseTrapInfo(TRAP_TYPE trapType, uint64_t rtosTime, const unsigned char *pData,
            size_t len, FRAME_INFO &frameInfo, const unsigned char *&payload,
            uint16_t &frameNumber);

private:
    ICDCodecApiReturnCode GetICDVersion(const unsigned char *packet, unsigned int &version);
    ICDCodecApiReturnCode GetICDType(const unsigned char *packet, unsigned int &type);
    ICDCodecApiReturnCode GetICDTimestamp(const unsigned char *packet,
            const unsigned int packetSize,unsigned long long &timestamp);
    ICDCodecApiReturnCode DecodeICDPacketHeader(const unsigned char *packet,
            const unsigned int packetSize, unsigned int &code, unsigned char &protocolID,
            char &token, const unsigned char **payload, unsigned int &payloadSize);
    ICDCodecApiReturnCode DecodeICDEventPacketHeader(const unsigned char *buffer,
            const unsigned int bufferLen, IcdEventHeader *outICDEventHeader);
    ICDCodecApiReturnCode DecodeICDRecordPacketHeader(const unsigned char *buffer,
            const unsigned int bufferLen, IcdRecordHeader *outICDRecordHeader);
};