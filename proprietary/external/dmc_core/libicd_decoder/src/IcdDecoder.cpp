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

#include <IcdDecoder.h>

#define UNUSED(expr) do { (void)(expr); } while (0)

IcdDecoder::IcdDecoder() {
    MDM_LOGD("Construct IcdDecoder.");
}

IcdDecoder::~IcdDecoder() {
    MDM_LOGD("IcdDecoder::~IcdDecoder");
}

bool IcdDecoder::ParseTrapInfo(
        TRAP_TYPE trapType,
        uint64_t rtosTime,
        const unsigned char *pData,
        size_t len,
        FRAME_INFO &frameInfo,
        const unsigned char *&payload,
        uint16_t &frameNumber) {
    UNUSED(rtosTime);

    switch (trapType) {
        case TRAP_TYPE_ICD_RECORD:
        case TRAP_TYPE_ICD_EVENT:
        {
            unsigned int payloadSize = 0;
            unsigned int icdCode = 0;
            unsigned int icdVer = -1;
            unsigned char protocolID = 0;
            unsigned long long timestamp = 0;
            std::string outBuffer;
            char token;

            if (ICD_CODEC_RET_S_OK != GetICDTimestamp(pData, len, timestamp)) {
                MDM_LOGE("IcdDecoder::GetICDTimestamp decode failed.");
                return false;
            }

            if (ICD_CODEC_RET_S_OK != DecodeICDPacketHeader(pData, len, icdCode, protocolID, token, &payload, payloadSize)) {
                MDM_LOGE("IcdDecoder::DecodeICDPacketHeader decode failed.");
                return false;
            }

            if (NULL != payload && ICD_CODEC_RET_S_OK != GetICDVersion(payload, icdVer)) {
                MDM_LOGE("IcdDecoder::GetICDVersion decode failed.");
                return false;
            }

            // print_buffer(payload, payloadSize);

            frameInfo.dhlFrameType = DHL_ICD_TUNNELING;
            frameInfo.trapType = trapType;
            frameInfo.simIdx = protocolID;
            frameInfo.msgID = icdCode;
            frameInfo.version = icdVer;
            // +--------------+---------------+--------------------+
            // | 10 Bytes     | 6 Bytes       | 48 Bytes           |
            // +--------------+---------------+--------------------|
            // | Frame Number | 16 US Counter | ms from 1970/01/01 |
            // +--------------+---------------+--------------------+
            //
            // Make timestamp to us
            // extract last 48 bytes to get timestamp in milliseconds
            uint64_t ms =         (timestamp & 0x0000FFFFFFFFFFFF);
            // extract 6 bytes to get 16 us counter
            uint64_t us_counter = (timestamp & 0x003F000000000000) >> 48;
            // extract first 10 bytes to get frame number field
            frameNumber =         (timestamp & 0xFFC0000000000000) >> 54;
            frameInfo.timestamp = ms*1000 + us_counter*16;
            return true;
        }
        default:
        {
            return false;
        }
    }
}

ICDCodecApiReturnCode IcdDecoder::GetICDType(const unsigned char *packet, unsigned int &type) {
    type = *packet & 0x0F;
    return ICD_CODEC_RET_S_OK;
}

ICDCodecApiReturnCode IcdDecoder::GetICDTimestamp(
        const unsigned char *packet, const unsigned int packetSize, unsigned long long &timestamp) {
    unsigned int type;

    if (GetICDType(packet, type) != ICD_CODEC_RET_S_OK) {
        return ICD_CODEC_RET_E_HEADER_ICD_TYPE_UNDEFINED;
    }

    switch (type)
    {
    case 0:
        {
            IcdEventHeader header;
            memset(&header, 0, sizeof(IcdEventHeader));
            ICDCodecApiReturnCode ret = DecodeICDEventPacketHeader(packet, packetSize, &header);
            if (ret != ICD_CODEC_RET_S_OK) {
                return ret; //failed to decode header
            }
            timestamp = header.timestamp;
        }
        break;
    case 1:
        {
            IcdRecordHeader header;
            memset(&header, 0, sizeof(IcdRecordHeader));
            ICDCodecApiReturnCode ret = DecodeICDRecordPacketHeader(packet, packetSize, &header);
            if (ret != ICD_CODEC_RET_S_OK)
                return ret;
            timestamp = header.timestamp;
        }
        break;
    default:
        return ICD_CODEC_RET_E_HEADER_ICD_TYPE_UNDEFINED;
    }

    return ICD_CODEC_RET_S_OK;
}

ICDCodecApiReturnCode IcdDecoder::GetICDVersion(const unsigned char *packet, unsigned int &version) {
    version = *packet;
    return ICD_CODEC_RET_S_OK;
}

ICDCodecApiReturnCode IcdDecoder::DecodeICDEventPacketHeader(
        const unsigned char *buffer, const unsigned int bufferLen, IcdEventHeader *outICDEventHeader) {
    unsigned int offset = 0;
    unsigned char dummy;

    //TYPE, read lower 4 bit of first byte
    //VERSION, read upper 4 bit of first byte
    if (bufferLen < offset + 1) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    outICDEventHeader->type = buffer[offset] & 0x0F;
    outICDEventHeader->version = (buffer[offset++] & 0xF0) >> 4;

    //TOTAL_LEN, read 2 byte
    if (bufferLen < offset + 2) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    outICDEventHeader->total_len = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;

    //TIMESTAMP_TYPE, read 4 bit
    //PROTOCOL_ID, read 4 bit
    if (bufferLen < offset + 1) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    dummy = buffer[offset++];
    outICDEventHeader->timestamp_type = dummy & 0x0F;      //lower 4 bit
    outICDEventHeader->protocol_id = (dummy & 0xF0) >> 4;  //upper 4 bit

    //EVENT_CODE, read 2 byte
    if (bufferLen < offset + 2) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    outICDEventHeader->event_code = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;

    //HEADER_CHECKSUM, read 2 byte
    if (bufferLen < offset + 2) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    //little-endian copy
    memcpy(&outICDEventHeader->header_checksum, &buffer[offset], 2);
    {
        unsigned short checksum = 0;
        unsigned int i = 0;
        // add bytes before check sum
        for (i = 0; i < offset; i += 2) {
            checksum += ((unsigned short)buffer[i] + (buffer[i+1] << 8));
        }
        if (checksum != outICDEventHeader->header_checksum) {
            return ICD_CODEC_RET_E_HEADER_CHECKSUM_ERROR;
        }
    }
    offset += 2;

    //TIMESTAMP, read 8 or 4 or 2 or 1 byte
    switch (outICDEventHeader->timestamp_type) {
    case 1: //1: Time Counter – Full 8 bytes
    case 8: //8: Wall Clock – Full 8 bytes
        if (bufferLen < offset + 8) {
            return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
        }
        {
            unsigned long long timestamp = 0;
            uint8_t *src = (uint8_t*)&buffer[offset];

            for (int i = sizeof(timestamp) - 1; i >= 0; i--) {
                timestamp = (timestamp << 8) | src[i];
            }

            outICDEventHeader->timestamp = timestamp;
            //memcpy(&outICDRecordHeader->timestamp, &buffer[offset], 8); //little-endian copy
        }
        offset += 8;
        break;
    case 2: //2: Time Counter – LSB 4 bytes
    case 9: //9: Wall Clock – LSB 4 bytes
        if (bufferLen < offset + 4) {
            return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
        }
        memcpy(&outICDEventHeader->timestamp, &buffer[offset], 4); //little-endian copy
        offset += 4;
        break;
    case 3: //3: Time Counter – LSB 2 bytes
    case 10: //10: Wall Clock – LSB 2 bytes
        if (bufferLen < offset + 2) {
            return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
        }
        outICDEventHeader->timestamp = buffer[offset] | (buffer[offset + 1] << 8);
        offset += 2;
        break;
    case 4: //4: Time Counter – LSB 1 byte
    case 11: //11: Wall Clock – LSB 1 byte
        if (bufferLen < offset + 1) {
            return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
        }
        outICDEventHeader->timestamp = buffer[offset++];
        break;
    case 0:
        break; //0: NULL
    default:
        return ICD_CODEC_RET_E_HEADER_UNSUPPORTED_TIME_TYPE;
    }

    //summary the header's size
    outICDEventHeader->size = offset;

    return ICD_CODEC_RET_S_OK;
}

ICDCodecApiReturnCode IcdDecoder::DecodeICDRecordPacketHeader(
        const unsigned char *buffer, const unsigned int bufferLen, IcdRecordHeader *outICDRecordHeader) {
    unsigned int offset = 0;
    unsigned char dummy;

    //TYPE, read lower 4 bit of first byte
    //VERSION, read upper 4 bit of first byte
    if (bufferLen < offset + 1) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    outICDRecordHeader->type = buffer[offset] & 0x0F;
    outICDRecordHeader->version = (buffer[offset++] & 0xF0) >> 4;

    //TOTAL_LEN, read 3 byte
    if (bufferLen < offset + 3) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    outICDRecordHeader->total_len = buffer[offset] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16);
    offset += 3;

    //TIMESTAMP_TYPE, read 4 bit
    //PROTOCOL_ID, read 4 bit
    if (bufferLen < offset + 1) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    dummy = buffer[offset++];
    outICDRecordHeader->timestamp_type = dummy & 0x0F;     //lower 4 bit
    outICDRecordHeader->protocol_id = (dummy & 0xF0) >> 4; //upper 4 bit

    //RESERVED, read 1 byte
    if (bufferLen < offset + 1) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    outICDRecordHeader->reserved = buffer[offset++];

    //RECORD_CODE, read 2 byte
    if (bufferLen < offset + 2) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    outICDRecordHeader->record_code = buffer[offset] | (buffer[offset + 1] << 8);
    offset += 2;

    //HEADER_CHECKSUM, read 4 byte
    if (bufferLen < offset + 4) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }
    memcpy(&outICDRecordHeader->header_checksum, &buffer[offset], 4); //little-endian copy
    //compare checksum
    {
        unsigned int checksum = 0;
        unsigned int i = 0;
        // add bytes before check sum
        for (i = 0; i < offset; i += 4) {
            checksum += ((unsigned int)buffer[i] + (buffer[i+1]<<8) + (buffer[i+2] << 16) + (buffer[i+3] << 24));
        }
        if (checksum != outICDRecordHeader->header_checksum) {
            return ICD_CODEC_RET_E_HEADER_CHECKSUM_ERROR;
        }
    }
    offset += 4;

    //TIMESTAMP, read 8 or 4 byte
    switch (outICDRecordHeader->timestamp_type) {
    case 0: //Time Counter
    case 8: //Wall Clock
        //Full 8 bytes
        if (bufferLen < offset + 8) {
            return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
        }
        {
            unsigned long long timestamp = 0;
            uint8_t *src = (uint8_t*)&buffer[offset];

            for (int i = sizeof(timestamp) - 1; i >= 0; i--) {
                timestamp = (timestamp << 8) | src[i];
            }

            outICDRecordHeader->timestamp = timestamp;
            //memcpy(&outICDRecordHeader->timestamp, &buffer[offset], 8); //little-endian copy
        }
        offset += 8;
        break;
    case 1: //Time Counter
    case 9: //Wall Clock
        //LSB 4 bytes
        if (bufferLen < offset + 4)
        {
            return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
        }
        memcpy(&outICDRecordHeader->timestamp, &buffer[offset], 4); //little-endian copy
        offset += 4;
        break;
    default:
        return ICD_CODEC_RET_E_HEADER_UNSUPPORTED_TIME_TYPE;
    }

    //summary the header's size
    outICDRecordHeader->size = offset;

    return ICD_CODEC_RET_S_OK;
}


ICDCodecApiReturnCode IcdDecoder::DecodeICDPacketHeader(
        const unsigned char *packet, const unsigned int packetSize, unsigned int &code,
        unsigned char &protocolID, char &token, const unsigned char **payload, unsigned int &payloadSize) {
    // since v1.25, we have type in first 4 bit of ICD protocol header
    if (packetSize < 1) {
        return ICD_CODEC_RET_E_HEADER_INVALID_BUFFER_SIZE;
    }

    switch (*packet & 0x0F) {
    case 0:
        {
            IcdEventHeader header;
            memset(&header, 0, sizeof(IcdEventHeader));
            ICDCodecApiReturnCode ret = DecodeICDEventPacketHeader(packet, packetSize, &header);
            if (ret != ICD_CODEC_RET_S_OK) {
                return ret;
            }
            code = header.event_code;
            protocolID =  header.protocol_id;
            token = -1;
            *payload = (packet) + header.size;
            payloadSize = header.total_len - header.size;
        }
        break;
    case 1:
        {
            IcdRecordHeader header;
            memset(&header, 0, sizeof(IcdRecordHeader));
            ICDCodecApiReturnCode ret = DecodeICDRecordPacketHeader(packet, packetSize, &header);
            if (ret != ICD_CODEC_RET_S_OK) {
                return ret;
            }
            code = header.record_code;
            protocolID = header.protocol_id;
            token = -1;
            *payload = (packet) + header.size;
            payloadSize = header.total_len - header.size;
        }
        break;
    default:
        return ICD_CODEC_RET_E_HEADER_ICD_TYPE_UNDEFINED;
    }


    return ICD_CODEC_RET_S_OK;
}

void print_buffer(const unsigned char *buffer, int len) {
    char temp[500];
    int offset = 0, ret;
    for (int i=0; i<len; ++i) {
        ret = sprintf(temp + offset, "%02X ", buffer[i]);
        if (ret <= 0) {
            break; //fail to write into 'temp'
        }
        offset += ret;
    }
    temp[offset] = '\0';
    // printf("%s\n", temp);
    MDM_LOGD("IcdDecoder::print_buffer len=[%d], temp=[%s]", len, temp);
}
