/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*****************************************************************************
 *
 * Filename:
 * ---------
 *  HEVCAssembler.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   Assemble RTP packet to HEVC NAL
 *
 * Author:
 * -------
 *   Haizhen.Wang(mtk80691)
 *
 ****************************************************************************/
//#define LOG_NDEBUG 0
#define LOG_TAG "[VT][RTP]HEVCAssembler"
#include <utils/Log.h>

#include "HEVCAssembler.h"

#include "RTPSource.h"
#include <cutils/properties.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/hexdump.h>

#include <arpa/inet.h>
#include <stdint.h>
#include <inttypes.h>

#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>

namespace imsma
{

// static
HEVCAssembler::HEVCAssembler(const sp<AMessage> notify)
    : mNotifyMsg(notify->dup()),
      //mAccessUnitRTPTime(0),
      mNextExpectedSeqNoValid(false),
      mNextExpectedSeqNo(0),
      mAccessUnitDamaged(false)
{
    ALOGI("%s",__FUNCTION__);

    mAccuCount = 0;
    mpNALFragmentInfo = NULL;

    mLastLost = -1;

    mLostCount = 0;
    mIDamageNum = 0;

#ifdef DEBUG_DUMP_ACCU
    mDumpAcuu = 0;
    mAccuFd = -1;
    ALOGD("dump downlink accu init=%" PRId64 "", mDumpAcuu);
    char dump_param[PROPERTY_VALUE_MAX];
    memset(dump_param,0,sizeof(dump_param));

    //int64_t dump_value;
    if(property_get("vendor.vt.imsma.dump_downlink_accu", dump_param, NULL)) {
        mDumpAcuu = atol(dump_param);
        ALOGD("dump downlink accu =%" PRId64 "", mDumpAcuu);
    }

    if(mDumpAcuu > 0) {
        const char* accu_filename = "/sdcard/downlink_accu_data.raw";
        mAccuFd = open(accu_filename, O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
        ALOGD("open %s,accuFd(%d)",accu_filename,mAccuFd);
    }

#endif
}

HEVCAssembler::~HEVCAssembler()
{
    ALOGI("%s",__FUNCTION__);
#ifdef DEBUG_DUMP_ACCU

    if(mAccuFd >= 0) {
        ALOGD("close accuFd(%d)",mAccuFd);
        close(mAccuFd);
        mAccuFd = -1;
    }

#endif
}

RTPAssembler::AssemblyStatus HEVCAssembler::addNALUnit(
    const sp<RTPSource> &source)
{
    ATRACE_CALL();
    ALOGV("%s",__FUNCTION__);
    List<sp<ABuffer> > *queue = source->queue();

    if(queue->empty()) {
        ALOGW("%s,source queue is empty",__FUNCTION__);
        return NOT_ENOUGH_DATA;
    }

    if(mNextExpectedSeqNoValid) {
        List<sp<ABuffer> >::iterator it = queue->begin();

        while(it != queue->end()) {
            if((uint32_t)(*it)->int32Data() >= mNextExpectedSeqNo) {
                break;
            }

            ALOGD("%s,drop unexpected SeqNo(%d) of source queue, mNextExpectedSeqNo(%d)", \
                  __FUNCTION__, (uint32_t)(*it)->int32Data(), mNextExpectedSeqNo);
            it = queue->erase(it);
        }

        if(queue->empty()) {
            return NOT_ENOUGH_DATA;
        }
    }

    sp<ABuffer> buffer = *queue->begin();

    if(!mNextExpectedSeqNoValid) {
        mNextExpectedSeqNoValid = true;
        mNextExpectedSeqNo = (uint32_t) buffer->int32Data();
        ALOGI("%s,first seq = %d",__FUNCTION__,mNextExpectedSeqNo);
    } else if((uint32_t) buffer->int32Data() != mNextExpectedSeqNo) {
        AssemblyStatus assemble_status = getAssembleStatus(queue, mNextExpectedSeqNo);
        ALOGW("%s,%d is not the sequence number %d I expected,status=%d",
              __FUNCTION__,buffer->int32Data(), mNextExpectedSeqNo,assemble_status);
        return assemble_status;
        //return WRONG_SEQUENCE_NUMBER;
    }

    //ATRACE_INT64("RTR:AVCAsmb:deqSqN",(int64_t)(uint32_t)(buffer->int32Data()));

    sp<AMessage> buffer_meta = buffer->meta();
    //use seqNum before extending
    int32_t seqNum = 0;
    buffer_meta->findInt32("token",&seqNum);
    ATRACE_ASYNC_END("RTR-MAR:SeqN",seqNum);

    const uint8_t *data = buffer->data();
    size_t size = buffer->size();

    if(size < 1 || (data[0] & 0x80)) {
        // Corrupt.

        ALOGW("Ignoring corrupt buffer.");
        queue->erase(queue->begin());

        ++mNextExpectedSeqNo;
        return MALFORMED_PACKET;
    }

    unsigned nalType = (data[0] & 0x7E) >> 1;
    ALOGV("%s,nalType = %d",__FUNCTION__, nalType);

    if((/*nalType >= 0 && */nalType <= 9)
            || (nalType >= 15 && nalType <= 21)
            || (nalType >= 32 && nalType <= 40)) {
        ALOGV("%s,single NAL",__FUNCTION__);

        //may be the last one~several FU-A lost before this NAL
        if(mpNALFragmentInfo.get()) {
            //ALOGW("mpNALFragmentInfo has FU-As before this signal NAL");
            if(mAccessUnitDamaged) {
                mpNALFragmentInfo->mIsDamaged = true;
            }

            sp<ABuffer> accu = assembleToNAL(mpNALFragmentInfo);
            submitAccessUnit(accu);

            mAccessUnitDamaged = false;
            mpNALFragmentInfo = NULL;
        }

        ATRACE_ASYNC_BEGIN("RTR-MAR",mAccuCount);
        buffer_meta->setInt32("EarliestPacket_token",seqNum);
        buffer_meta->setInt32("FirstPacket_token",seqNum);
        buffer_meta->setInt32("latestPacekt_token",seqNum);

        addSingleNALUnit(buffer);
        queue->erase(queue->begin());
        ++mNextExpectedSeqNo;
        return OK;
    } else if(nalType == 49) {
        ALOGV("%s,FU-A",__FUNCTION__);

        // FU-A
        return addFragmentedNALUnit(queue);
    } else if(nalType == 48) {
        // STAP-A
        ALOGV("%s,STAP-A",__FUNCTION__);

        //may be the last one~several FU-A lost before this NAL
        if(mpNALFragmentInfo.get()) {
            //ALOGW("mpNALFragmentInfo has FU-As before this STAP-A");
            if(mAccessUnitDamaged) {
                mpNALFragmentInfo->mIsDamaged = true;
            }

            sp<ABuffer> accu = assembleToNAL(mpNALFragmentInfo);
            submitAccessUnit(accu);

            mAccessUnitDamaged = false;
            mpNALFragmentInfo = NULL;
        }

        bool success = addSingleTimeAggregationPacket(buffer);
        queue->erase(queue->begin());
        ++mNextExpectedSeqNo;
        return success ? OK : MALFORMED_PACKET;
    } else if(nalType == 50) {
        ALOGE("%s,not support PACI nal type.",__FUNCTION__);

        queue->erase(queue->begin());
        ++mNextExpectedSeqNo;
        return MALFORMED_PACKET;
    } else {
        ALOGW("%s,Ignoring unsupported buffer (nalType=%d)",__FUNCTION__,nalType);

        queue->erase(queue->begin());
        ++mNextExpectedSeqNo;
        return MALFORMED_PACKET;
    }
}

void HEVCAssembler::addSingleNALUnit(const sp<ABuffer> &buffer)
{
    //ALOGD("addSingleNALUnit of size %zu", buffer->size());
    submitAccessUnit(buffer);
    return;
}

bool HEVCAssembler::addSingleTimeAggregationPacket(const sp<ABuffer> &buffer)
{
    const uint8_t *data = buffer->data();
    size_t size = buffer->size();
    //ALOGD("%s,buffer size(%d)",__FUNCTION__,size);

    if(size < 6) {
        ALOGV("Discarding too small STAP-A packet.");
        return false;
    }

    sp<AMessage> meta = buffer->meta();
    int32_t token = 0;
    meta->findInt32("token",&token);

    data += 2;
    size -= 2;

    while(size >= 2) {
        size_t nalSize = (data[0] << 8) | data[1];

        if(size < nalSize + 2) {
            ALOGV("Discarding malformed STAP-A packet.");
            return false;
        }

        ATRACE_ASYNC_BEGIN("RTR-MAR",mAccuCount);


        ALOGV("%s type=%d nalSize=%zu", __FUNCTION__, (data[3] & 0x7E) >> 1, nalSize);

        sp<ABuffer> unit = new ABuffer(nalSize);
        memcpy(unit->data(), &data[2], nalSize);

        CopyMetas(unit,buffer);

        data += 2 + nalSize;
        size -= 2 + nalSize;

        if(size > 2) {
            //this is not the last nal of STAP
            //re-clear the marker bit flag
            sp<AMessage> unit_meta = unit->meta();
            unit_meta->setInt32("M",0);
        }

        unit->meta()->setInt32("EarliestPacket_token",token);
        unit->meta()->setInt32("FirstPacket_token",token);
        unit->meta()->setInt32("latestPacekt_token",token);

        addSingleNALUnit(unit);
    }

    if(size != 0) {
        ALOGV("Unexpected padding at end of STAP-A packet.");
    }

    return true;
}

RTPAssembler::AssemblyStatus HEVCAssembler::addFragmentedNALUnit(
    List<sp<ABuffer> > *queue)
{
    ATRACE_CALL();
    CHECK(!queue->empty());

    sp<ABuffer> buffer = *queue->begin();
    const uint8_t *data = buffer->data();
    size_t size = buffer->size();
    ALOGV("%s,buffer size(%zu)",__FUNCTION__,size);


    ALOGV("%s,buffer 0=%x 1=%x 2=%x",__FUNCTION__,data[0], data[1],data[2]);

    CHECK(size > 0);

    if(size < 3) {
        ALOGW("Ignoring malformed FU buffer (size = %zu)", size);

        queue->erase(queue->begin());
        ++mNextExpectedSeqNo;
        return MALFORMED_PACKET;
    }

    uint32_t nalType = (data[0] & 0x7E) >> 1;

    if(!mpNALFragmentInfo.get()) {
        if(!(data[2] & 0x80)) {
            ALOGV("Start bit not set on first buffer");

            queue->erase(queue->begin());
            ++mNextExpectedSeqNo;
            return MALFORMED_PACKET;
        } else {
            ALOGV("This is a new start fragment");
            //need re-set mAccessUnitDamaged for new NAL
            //avoid seting "damage" for new  completed NAL by mistake because of the last whole NAL has lost

            mAccessUnitDamaged = false;
        }
    }

    if(mpNALFragmentInfo.get()) {
        if(data[2] & 0x80) {
            ALOGV("This is a new start fragment,submit the last nal");

            //may be the end nal fragment of last NAL is lost
            //may be the end flag not set for the end fragement of the last NAL
            if(mAccessUnitDamaged) {
                mpNALFragmentInfo->mIsDamaged = true;

                if(nalType == 19 || nalType == 20) {
                    mIDamageNum++;
                }
            }

            //maybe the end of NAL fragment is not set for the last NAL
            //submitAccu();
            sp<ABuffer> accu = assembleToNAL(mpNALFragmentInfo);
            submitAccessUnit(accu);
            //because receive a start fragment, re-set mAccessUnitDamaged
            mAccessUnitDamaged = false;
            mpNALFragmentInfo = NULL;
        } else {
            ALOGV("append fragment to NAL");

            if(nalType != mpNALFragmentInfo->mNALType) {
                ALOGE("Ignoring malformed FU buffer(fragment nal_type(%d) != %d)",\
                      nalType,mpNALFragmentInfo->mNALType);
                queue->erase(queue->begin());
                ++mNextExpectedSeqNo;
                return MALFORMED_PACKET;
            }
        }
    }

    if(!mpNALFragmentInfo.get()) {
        ALOGV("new one NALFragMentsInfo");
        mpNALFragmentInfo = new NALFragMentsInfo();
        mpNALFragmentInfo->mNALType = nalType;
    }

    mpNALFragmentInfo->mNALFragments.push_back(buffer);
    mpNALFragmentInfo->mNALSize += size - 3;
    mpNALFragmentInfo->mTotalCount++;
    ALOGV("%s,Nal-FU-A(count:%d,total_size(%d))",\
          __FUNCTION__,mpNALFragmentInfo->mTotalCount,mpNALFragmentInfo->mNALSize);

    if(data[2] & 0x40) {
        ALOGV("This is end fragment.");

        if(mAccessUnitDamaged) {
            mpNALFragmentInfo->mIsDamaged = true;

            if(nalType == 19 || nalType == 20) {
                mIDamageNum++;
            }
        }

        //maybe the end of NAL fragment is not set for the last NAL
        //submitAccu();
        sp<ABuffer> accu = assembleToNAL(mpNALFragmentInfo);
        submitAccessUnit(accu);
        mAccessUnitDamaged = false;
        mpNALFragmentInfo = NULL;
    }

    queue->erase(queue->begin());
    ++mNextExpectedSeqNo;
    ALOGV("%s,mNextExpectedSeqNo(%d)",__FUNCTION__,mNextExpectedSeqNo);

    return OK;
}
sp<ABuffer> HEVCAssembler::assembleToNAL(sp<NALFragMentsInfo> nalFragmentInfo)
{
    uint32_t nal_size = nalFragmentInfo->mNALSize + 2;// for 1 byte NAL header


    List<sp<ABuffer> > *queue = & (nalFragmentInfo->mNALFragments);   //ToDo: can we assigne list this way
    ALOGI("%s,nal fragments in list(num = %zu, total_size=%d)",__FUNCTION__,queue->size(),nal_size);

    ATRACE_ASYNC_BEGIN("RTR-MAR",mAccuCount);

    sp<ABuffer> unit = new ABuffer(nal_size);

    if(nalFragmentInfo->mIsDamaged) {
        unit->meta()->setInt32("damaged", true);
    }

    sp<ABuffer> beginFragment = *queue->begin();
    sp<ABuffer> endFragment = *--queue->end();
    uint8_t* data = beginFragment->data();

    uint32_t nalType = data[2] & 0x1f;

    CopyMetas(unit,endFragment);

    unit->data() [0] = data[0];
    unit->data() [0] &= 0x81;
    unit->data() [0] |= (nalType << 1);
    unit->data() [1] = data[1];

    ALOGV("%s,nal fragments nalType=%d, header1=0x%x header2=0x%x",
          __FUNCTION__,nalType,unit->data() [0],unit->data() [1]);

    size_t offset = 2;
    List<sp<ABuffer> >::iterator it = queue->begin();
    sp<AMessage> meta = (*it)->meta();
    int64_t earliest_recv_time = 0;
    meta->findInt64("recv-time",&earliest_recv_time);

    int32_t tokenF = 0;
    (*it)->meta()->findInt32("token",&tokenF);
    unit->meta()->setInt32("FirstPacket_token",tokenF);

    while(it!= queue->end()) {
        const sp<ABuffer> &buffer = *it;

        sp<AMessage> meta = buffer->meta();
        int64_t recv_time = 0;
        meta->findInt64("recv-time",&recv_time);

        int32_t token = 0;
        meta->findInt32("token",&token);

        if(recv_time <= earliest_recv_time) {
            unit->meta()->setInt32("EarliestPacket_token",token);
            earliest_recv_time = recv_time;
        }

        memcpy(unit->data() + offset, buffer->data() + 3, buffer->size() - 3);
        offset += buffer->size() - 3;

        it = queue->erase(it);

        if(it == queue->end()) {
            unit->meta()->setInt32("latestPacekt_token",token);
            ALOGV("F=%d L=%d", tokenF, token);
        }
    }

    unit->setRange(0, nal_size);    //ToDo: assign uint32_t to size_t,ok?
    //ALOGD("successfully assembled a NAL unit(size:%d) from fragments.",nal_size);
    return unit;
}
void HEVCAssembler::submitAccessUnit(const sp<ABuffer>& accessUnit)
{
    /*
    if (mAccessUnitDamaged) {
        accessUnit->meta()->setInt32("damaged", true);
    }

    mAccessUnitDamaged = false;
    */
    ATRACE_CALL();
    ALOGV("%s,notify(0x%x)",__FUNCTION__,mNotifyMsg->what());
    sp<AMessage> msg = mNotifyMsg->dup();
    msg->setInt32("what",kWhatAccessUnit);
    msg->setBuffer("access-unit", accessUnit);

    sp<AMessage> accu_meta = accessUnit->meta();
    int32_t marker_bit = 0;
    accu_meta->findInt32("M", &marker_bit);

    if(marker_bit > 0) {
        ALOGV("%s,last accu of frame",__FUNCTION__);
    }

    accu_meta->setInt32("token",mAccuCount);
    accu_meta->setInt32("needStartCode", 1);
    //ALOGD("%s,accu count(%d) needStartCode",__FUNCTION__,mAccuCount);

    int32_t iEarliestPak = 0;
    int32_t iLatestPak = 0;
    accu_meta->findInt32("EarliestPacket_token",&iEarliestPak);
    accu_meta->findInt32("latestPacekt_token",&iLatestPak);
    ATRACE_INT("RTR-MAR:EpkSeqN",iEarliestPak);
    ATRACE_INT("RTR-MAR:LpkSeqN",iLatestPak);

    mAccuCount++;

    //ATRACE_ASYNC_BEGIN("RTR-MAR",mAccuCount);
    //ATRACE_INT("RTR:AVCAsmb:sbAU",mAccuCount);
#ifdef DEBUG_DUMP_ACCU

    if(mAccuFd >= 0) {
        size_t real_write = write(mAccuFd,"\x00\x00\x00\x01",4);
        real_write = write(mAccuFd,accessUnit->data(),accessUnit->size());
        //ALOGD("write to file,real_write(%d)",real_write);
    }

#endif
    msg->post();
}

RTPAssembler::AssemblyStatus HEVCAssembler::assembleMore(
    const sp<RTPSource> source)
{
    AssemblyStatus status = addNALUnit(source);

    if(status == MALFORMED_PACKET) {
        mAccessUnitDamaged = true;
    }

    return status;
}

void HEVCAssembler::packetLost()
{
    CHECK(mNextExpectedSeqNoValid);

    if(mLastLost <= 0) {
        mLastLost = ALooper::GetNowUs();
    } else {
        if(ALooper::GetNowUs() - mLastLost < 200000ll) {
            //ALOGW("packetLost  don't submit");
            return ;
        }
    }

    //don't submit~~~
    if(mPackLostList.size() == 0) {
        //ALOGW("not packetLost ");
        return ;
    }

    int LostCount = mPackLostList.size();
    ALOGW("packetLost  size=%d", LostCount);

    sp<ABuffer> TmBuf = new ABuffer(4*LostCount);

    uint8_t * pTmBuf = TmBuf->base();

    int count = 0;

    for(List<uint32_t>::iterator i = mPackLostList.begin(); i != mPackLostList.end(); i++) {
        //ALOGW("packetLost  count=%d  seq=%d", count, *i);
        ((uint32_t *) pTmBuf) [count++] = *i;
    }

    sp<AMessage> msg = mNotifyMsg->dup();
    msg->setInt32("what",kWhatPacketLost);
    msg->setInt32("lostcount",LostCount);
    msg->setBuffer("lostpointer",TmBuf);
    msg->post();

    mLastLost = ALooper::GetNowUs();

    mPackLostList.clear();
}

void HEVCAssembler::packetLostRegister()
{
    //ALOGW("packetLostRegister (expected %d)", mNextExpectedSeqNo);

    if(mPackLostList.size() >= PACKETLOSTRECORDNUM) {
        List<uint32_t>::iterator i = mPackLostList.begin();
        mPackLostList.erase(i);
    }

    mPackLostList.push_back(mNextExpectedSeqNo);

    ++mNextExpectedSeqNo;
    mAccessUnitDamaged = true;

    mLostCount++;
}


void HEVCAssembler::flushQueue()
{
    //because RTPSource will lock the operation with packetReceive
    //so Assembler need not lock
    if(mpNALFragmentInfo.get()) {
        mpNALFragmentInfo->mNALFragments.clear();
        mpNALFragmentInfo = NULL;
    }
}
void HEVCAssembler::reset()
{
    //flushQueue();
    mNextExpectedSeqNoValid = false;
    mNextExpectedSeqNo = 0;
    mAccessUnitDamaged = false;
    mLostCount = 0;
    mIDamageNum = 0;

    //maybe we should add lock,but now nobody use the interface
    //mPackLostList.clear();
    mLastLost = -1;
}

}  // namespace android
