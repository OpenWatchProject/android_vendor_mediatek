/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "WpfaRingBuffer.h"

#define WPFA_D_LOG_TAG "WpfaRingBuffer"

WpfaRingBuffer::WpfaRingBuffer() {
    mtkLogD(WPFA_D_LOG_TAG, "-new()");
}

WpfaRingBuffer::~WpfaRingBuffer() {
    pthread_mutex_destroy(&mControlPara.mutex);
    pthread_cond_destroy(&mControlPara.cond_can_write);
    pthread_cond_destroy(&mControlPara.cond_can_read);
    mtkLogD(WPFA_D_LOG_TAG, "-del()");
}

void WpfaRingBuffer::initRingBuffer() {
    mtkLogD(WPFA_D_LOG_TAG, "+ initRingBuffer()");

    // buffer init
    ringBuffer_memset(mRingBuffer, 0, RING_BUFFER_SIZE);

    // control parameter init
    mControlPara.mWriteIdx = 10;
    mControlPara.mReadIdx = 10;
    mControlPara.mReadDataSize = 0;
    mControlPara.mTempReadIdx = INVALID_INDEX;
    mControlPara.mTempReadDataSize = 0;
    setState(RING_BUFFER_STATE_NONE);

    mControlPara.mutex = PTHREAD_MUTEX_INITIALIZER;
    mControlPara.cond_can_write = PTHREAD_COND_INITIALIZER;
    mControlPara.cond_can_read = PTHREAD_COND_INITIALIZER;

    dumpParam();
    mtkLogD(WPFA_D_LOG_TAG, "- initRingBuffer()");
}

void WpfaRingBuffer::lock(const char* user) {
    pthread_mutex_lock(&mControlPara.mutex);
    mtkLogD(WPFA_D_LOG_TAG, "----[RB] lock success (%s)", user);
}

void WpfaRingBuffer::unlock(const char* user) {
    pthread_mutex_unlock(&mControlPara.mutex);
    mtkLogD(WPFA_D_LOG_TAG, "----[RB] unlock success (%s)", user);
}

void WpfaRingBuffer::waitCanWrite(const char* user, uint16_t dataSize) {
    mtkLogD(WPFA_D_LOG_TAG, "----[RB] wait can write .... (%s)", user);
    //Kuncheng: add for A2M buffer is full, assert in IT phase.
    if(isFull() || (getFreeSize() < dataSize)){
        mtkLogD(WPFA_D_LOG_TAG, "The Buffer is Full");
    }
    while(isFull() || (getFreeSize() < dataSize)) {
        // wait until some data packets are consumed
        pthread_cond_wait(&mControlPara.cond_can_write, &mControlPara.mutex);
    }
    mtkLogD(WPFA_D_LOG_TAG, "----[RB] wait can write success (%s)", user);
}

void WpfaRingBuffer::signalCanWrite(const char* user) {
    pthread_cond_signal(&mControlPara.cond_can_write);
    mtkLogD(WPFA_D_LOG_TAG, "----[RB] signal can write success (%s)", user);
}

void WpfaRingBuffer::waitCanRead(const char* user) {
    mtkLogD(WPFA_D_LOG_TAG, "----[RB] wait can read .... (%s)", user);
    while(isEmpty()) {
        // wait for new data packets to be appended to the buffer
        pthread_cond_wait(&mControlPara.cond_can_read, &mControlPara.mutex);
    }
    mtkLogD(WPFA_D_LOG_TAG, "----[RB] wait can read success (%s)", user);
}

void WpfaRingBuffer::signalCanRead(const char* user) {
    pthread_cond_signal(&mControlPara.cond_can_read);
    mtkLogD(WPFA_D_LOG_TAG, "----[RB] signal can read success (%s)", user);
}

int WpfaRingBuffer::writeDataToRingBuffer(const void *src, uint16_t dataSize) {
    char *p_src = (char *)src;

    // check free size
    if (dataSize <= getFreeSize()) {
        // memcpy
        if (mControlPara.mWriteIdx >= mControlPara.mReadIdx) {
            uint16_t w2e = RING_BUFFER_SIZE - mControlPara.mWriteIdx; // remain size from writeIdx to eof
            //cout << "dataSize:" << dataSize << " w2e:" << w2e << endl;
            if (dataSize <= w2e) {
                ringBuffer_memcpy(mRingBuffer + mControlPara.mWriteIdx, src, dataSize);
                toString();

                // update mReadDataSize before update wirte index
                updateReadDataSizeByState(dataSize);

                // update wirte index
                mControlPara.mWriteIdx += dataSize;
                if (mControlPara.mWriteIdx == RING_BUFFER_SIZE) {
                    mControlPara.mWriteIdx = 0;
                }
            } else {
                ringBuffer_memcpy(mRingBuffer + mControlPara.mWriteIdx, src, w2e);
                ringBuffer_memcpy(mRingBuffer, (uint8_t *)src + w2e, dataSize - w2e);
                toString();

                // update mReadDataSize before update wirte index
                updateReadDataSizeByState(dataSize);

                // update wirte index
                mControlPara.mWriteIdx = dataSize - w2e;
            }
        } else { // readIdx > writeIdx
            ringBuffer_memcpy(mRingBuffer + mControlPara.mWriteIdx, src, dataSize);
            toString();

            // update mReadDataSize before update wirte index
            updateReadDataSizeByState(dataSize);

            // update wirte index
            mControlPara.mWriteIdx += dataSize;
        }
        dumpParam();
    } else {
        //wait
        mtkLogD(WPFA_D_LOG_TAG, "wait()");
        return 0;
    }

    /* You can't notify here, case callback fuction is blocking call!!
    // send notify to reader and update state
    if (mControlPara.mState == RING_BUFFER_STATE_NONE) {
        mControlPara.mState = RING_BUFFER_STATE_READING;
        notifyToReader(mControlPara.mReadIdx, mControlPara.mReadDataSize);
    }
    */
    //cout << "- WpfaRingBuffer-writeDataToRingBuffer()" << endl;
    return dataSize;
}

void WpfaRingBuffer::readDataFromRingBuffer(void *des, uint32_t readIdx, uint16_t dataSize) {
    char *p_src = (char *)des;
    uint32_t currentWriteIdx = (readIdx + dataSize)%RING_BUFFER_SIZE;

    if ((mControlPara.mReadIdx == readIdx) && (dataSize == mControlPara.mReadDataSize)) {
        if (readIdx <= currentWriteIdx) {
            ringBuffer_memcpy(des, mRingBuffer + readIdx, dataSize);
        } else {
            uint16_t r2e = RING_BUFFER_SIZE - readIdx;
            //cout << "r2e:" << r2e << endl;
            if (dataSize > r2e) {
                ringBuffer_memcpy(des, mRingBuffer + readIdx, r2e);
                ringBuffer_memcpy((uint8_t *)des + r2e, mRingBuffer, dataSize - r2e);
            } else {
                ringBuffer_memcpy(des, mRingBuffer + readIdx, dataSize);
            }
        }
    } else {
        //error
        mtkLogE(WPFA_D_LOG_TAG, "read error");
    }
    //cout << "- WpfaRingBuffer-readDataFromRingBuffer()" << endl;
}

void WpfaRingBuffer::readDataWithoutRegionCheck(void *des, uint32_t readIdx, uint16_t dataSize) {
    char *p_src = (char *)des;
    uint32_t currentWriteIdx = (readIdx + dataSize)%RING_BUFFER_SIZE;

    // 1. the *des should be a linear
    // 2. user needs to make sure read index and data size is the same with
    //    the return of getRegionInfoForReader().
    mtkLogD(WPFA_D_LOG_TAG,"DataPktCopy+");
    //if ((mControlPara.mReadIdx == readIdx) && (dataSize == mControlPara.mReadDataSize)) {
        if (readIdx <= currentWriteIdx) {  // Ex: RING_BUFFER_SIZE = 10, readIdx=5 and currentWriteIdx=9
            ringBuffer_memcpy(des, mRingBuffer + readIdx, dataSize);
            dump_hex((unsigned char *)des, dataSize);
        } else {
            // Ex: RING_BUFFER_SIZE = 10, readIdx=9 and currentWriteIdx=5, r2e=1
            uint16_t r2e = RING_BUFFER_SIZE - readIdx;
            //cout << "r2e:" << r2e << endl;
            if (dataSize > r2e) {
                ringBuffer_memcpy(des, mRingBuffer + readIdx, r2e);
                dump_hex((unsigned char *)des, r2e);
                ringBuffer_memcpy((uint8_t *)des + r2e, mRingBuffer, dataSize - r2e);
                dump_hex((unsigned char *)des + r2e, dataSize - r2e);
            } else {
                ringBuffer_memcpy(des, mRingBuffer + readIdx, dataSize);
                dump_hex((unsigned char *)des, dataSize);
            }
        }
    mtkLogD(WPFA_D_LOG_TAG,"DataPktCopy-");
    //} else {
    //    mtkLogE(WPFA_D_LOG_TAG, "read error");
    //}
    //cout << "- WpfaRingBuffer-readDataFromRingBuffer()" << endl;
}


/*
int WpfaRingBuffer::notifyToReader(uint32_t readIdx, uint16_t dataSize) {
    int ret = 0;
    WpfaDriver *mWpfaDriver = WpfaDriver::getInstance();

    // update state first
    mControlPara.mRbState = RING_BUFFER_STATE_READING;

    //invoke callback function of Wpfa
    region_info_t* mRegion = NULL;
    mRegion = (region_info_t *)malloc(sizeof(region_info_t));
    mRegion->read_idx = mControlPara.mReadIdx;
    mRegion->data_size = mControlPara.mReadDataSize;
    ret = mWpfaDriver->notifyCallback(WPFA_EVENT_READ_DATA_PTK, mRegion);

    return ret;
}
*/

void WpfaRingBuffer::getRegionInfoForReader(region_info_t* mRegion) {
    mRegion->read_idx = mControlPara.mReadIdx;
    mRegion->data_size = mControlPara.mReadDataSize;
}


void WpfaRingBuffer::readDone() {
    mtkLogD(WPFA_D_LOG_TAG, "-readDone()");
    //dumpParam();

    if (mControlPara.mTempReadIdx == INVALID_INDEX) {
        // no more data after notify
        if (mControlPara.mReadIdx <= mControlPara.mWriteIdx) {
            mControlPara.mReadIdx += mControlPara.mReadDataSize;
        } else {
            uint16_t r2e = RING_BUFFER_SIZE - mControlPara.mReadIdx;
            if (mControlPara.mReadDataSize > r2e) {
                mControlPara.mReadIdx = mControlPara.mReadDataSize - r2e;
            } else {
                mControlPara.mReadIdx += mControlPara.mReadDataSize;
                if (mControlPara.mReadIdx == RING_BUFFER_SIZE) {
                    mControlPara.mReadIdx = 0;
                }
            }
        }
        mControlPara.mReadDataSize = 0;

    } else {
        // more data packet needs to send
        mControlPara.mReadIdx = mControlPara.mTempReadIdx;
        mControlPara.mReadDataSize = mControlPara.mTempReadDataSize;

        mControlPara.mTempReadIdx = INVALID_INDEX;
        mControlPara.mTempReadDataSize = 0;
    }
    mControlPara.mRbState = RING_BUFFER_STATE_NONE;
    dumpParam();
    //mtkLogD(WPFA_D_LOG_TAG, "- WpfaRingBuffer-readDone()");
}

void WpfaRingBuffer::updateReadDataSizeByState(uint16_t dataSize) {
    if (mControlPara.mRbState == RING_BUFFER_STATE_READING) {
        if (mControlPara.mTempReadIdx == INVALID_INDEX) {
            // set tempReadIdx at first time
            mControlPara.mTempReadIdx = mControlPara.mWriteIdx;
        }
        mControlPara.mTempReadDataSize += dataSize;
    } else {
        mControlPara.mReadDataSize += dataSize;
    }
}

void WpfaRingBuffer::setState(int state) {
    mControlPara.mRbState = state;
}

int WpfaRingBuffer::getState() {
    return mControlPara.mRbState;
}

void WpfaRingBuffer::ringBuffer_memset(void *des, uint8_t value, uint16_t size) {
    char *p_des = (char *)des;
    uint16_t i = 0;

    for (i = 0; i < size; i++) {
        p_des[i] = value;
    }
}


void WpfaRingBuffer::ringBuffer_memcpy(void *des, const void *src, uint16_t size) {
    char *p_src = (char *)src;
    char *p_des = (char *)des;
    uint16_t i = 0;

    for (i = 0; i < size; i++) {
        p_des[i] = p_src[i];
    }
}

bool WpfaRingBuffer::isEmpty() {
    if (mControlPara.mWriteIdx == mControlPara.mReadIdx) {
        return true;
    }
    return false;
}

bool WpfaRingBuffer::isFull() {
    if ((mControlPara.mWriteIdx+1)%RING_BUFFER_SIZE == mControlPara.mReadIdx) {
        return true;
    }
    return false;
}

uint16_t WpfaRingBuffer::getFreeSize() {
    return RING_BUFFER_SIZE - mControlPara.mReadDataSize - mControlPara.mTempReadDataSize;
}

uint16_t WpfaRingBuffer::getDataCount() {
    return (mControlPara.mReadDataSize + mControlPara.mTempReadDataSize);
}

void WpfaRingBuffer::toString() {
    /*cout << "+ RingBuffer: ";
    uint16_t i = 0;
    for (i = 0; i < RING_BUFFER_SIZE; i++ ) {
        cout << i << "=" << mRingBuffer[i]<< ",";
    }
    cout << endl;    */
}

void WpfaRingBuffer::dumpParam() {
    mtkLogD(WPFA_D_LOG_TAG, " W_Idx:%d,R_idx:%d,R_Size:%d,State:%d,T_Idx:%d,T_Size:%d",
            mControlPara.mWriteIdx, mControlPara.mReadIdx,
            mControlPara.mReadDataSize, mControlPara.mRbState,
            mControlPara.mTempReadIdx, mControlPara.mTempReadDataSize);
}

int WpfaRingBuffer::dump_hex(unsigned char *data, int len) {
    int i,counter ,rest;
    char * dumpbuffer;
    char  printbuf[1024];

    dumpbuffer = (char*)malloc(16*1024);
    if (!dumpbuffer) {
        mtkLogD(WPFA_D_LOG_TAG, "DUMP_HEX ALLOC memory fail \n");
        return -1;
    }

    if (len >8*1024 ){
        mtkLogD(WPFA_D_LOG_TAG, "trac the packet \n");
        len = 8*1024;
    }

    //memset((void *)dumpbuffer,0,16*1024);
    memset(dumpbuffer, 0, 16*1024);
    //mtkLogD(UPLINK_LOG_TAG, "dumpbuffer size =%d \n",(int)sizeof(*dumpbuffer));

    for (i = 0 ; i < len ; i++) {
       sprintf(&dumpbuffer[i*2],"%02x",data[i]);
    }
    dumpbuffer[i*2] = '\0' ;

    // android log buffer =1024bytes, need to splite the log
    counter = len/300 ;
    rest = len - counter*300 ;

    mtkLogD(WPFA_D_LOG_TAG, " Data Length = %d ,counter =%d ,rest =%d", len ,counter,rest);

    mtkLogD(WPFA_D_LOG_TAG, " NFQUEU Data: ");
    for (i = 0 ; i < counter ; i++) {
        memset(printbuf, 0, sizeof(printbuf));
        memcpy(printbuf ,dumpbuffer+i*600 , 300*2);
        printbuf[600]='\0';
        mtkLogD(WPFA_D_LOG_TAG, "data:%s",printbuf);
        mtkLogD(WPFA_D_LOG_TAG, "~");
    }

    //for rest data
    memset(printbuf, 0, sizeof(printbuf));
    memcpy(printbuf ,dumpbuffer+counter*600 , rest*2);
    printbuf[rest*2]='\0';
    mtkLogD(WPFA_D_LOG_TAG, "%s",printbuf);

    free(dumpbuffer);
    return 1;
}