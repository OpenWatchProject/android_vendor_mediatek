#include "WpfaShmAccessController.h"

extern "C" {
#include <hardware/ccci_intf.h>
}

#define WPFA_D_LOG_TAG "WpfaShmAccessController"

WpfaShmAccessController::WpfaShmAccessController() {
    mtkLogD(WPFA_D_LOG_TAG, "-new()");

    mCcciShareMemoryHandler = -1;
    pShareMemoryBase = NULL;
    mShareMemoryLength = 0;
    pShareMemory = NULL;

    mDlBufferSize = 0;
    mUlBufferSize = 0;

    mShmAccessMutex = PTHREAD_MUTEX_INITIALIZER;
    init();
}

WpfaShmAccessController::~WpfaShmAccessController() {
    mtkLogD(WPFA_D_LOG_TAG, "-del()");
}

void WpfaShmAccessController::init() {
    int retValue = 0;
    retValue = openShareMemory();
    if (retValue >= 0 ) {
        retValue = formatShareMemory();
        if (retValue == 0) {
            dumpShmLayout();
        } else {
            mtkLogE(WPFA_D_LOG_TAG, "format share memory fail!!");
        }
    } else {
        mtkLogE(WPFA_D_LOG_TAG, "open share memory fail!!");
    }
    mtkLogD(WPFA_D_LOG_TAG, "-init()");
}

int WpfaShmAccessController::resetShareMemoryIndex() {
    int retValue = 0;
    return retValue;
}
/*
uint32_t WpfaShmAccessController::writeApDataToShareMemory(WpfaRingBuffer *ringBuffer) {
    uint32_t newIdx = -1;
    lock("writeApDataToShareMemory");

    // checkfree space
    if (size <= getFreeSizeOfApToMd()) {
        // get base address of DL_DATA region
        uint8_t *p_buf = ((uint8_t *)pShareMemory) + mApRingBufferCtrlParam.offset;

        if (mApRingBufferCtrlParam.ap_write_index >= mApRingBufferCtrlParam.md_read_index) {
            // remain size from writeIdx to eof
            uint32_t w2e = WPFA_SHM_DATA_SIZE - mApRingBufferCtrlParam.ap_write_index;

            mtkLogD(WPFA_D_LOG_TAG,"writeApDataToShareMemory dataSize:%d w2e:%d", size, w2e);
            if (size <= w2e) {
                shm_memcpy(p_buf + mApRingBufferCtrlParam.ap_write_index, src, size);
                // update wirte index
                mApRingBufferCtrlParam.ap_write_index += size;
                if (mApRingBufferCtrlParam.ap_write_index == WPFA_SHM_DATA_SIZE) {
                    mApRingBufferCtrlParam.ap_write_index = 0;
                }
            } else {
                shm_memcpy(p_buf + mApRingBufferCtrlParam.ap_write_index, src, w2e);
                shm_memcpy(p_buf, (uint8_t *)src + w2e, size - w2e);
                // update wirte index
                mApRingBufferCtrlParam.ap_write_index = size - w2e;
            }
        } else {   // readIdx > writeIdx
            shm_memcpy(p_buf + mApRingBufferCtrlParam.ap_write_index, src, size);
            mApRingBufferCtrlParam.ap_write_index += size;
        }
        newIdx = mApRingBufferCtrlParam.ap_write_index;
    } else {
        mtkLogE(WPFA_D_LOG_TAG,"writeApDataToShareMemory: no space");
    }

    unlock("writeApDataToShareMemory");
    return newIdx;
}
*/
uint32_t WpfaShmAccessController::writeApDataToShareMemory(WpfaRingBuffer *ringBuffer) {
    uint32_t newIdx = -1;
    lock("writeApDataToShareMemory");

    uint32_t mRingBuffer_ReadIdx = ringBuffer->getReadIndex();
    uint32_t mRingBuffer_DataSize = ringBuffer->getReadDataSize();

    // check free space
    if (mRingBuffer_DataSize <= getFreeSizeOfApToMd()) {
        mtkLogD(WPFA_D_LOG_TAG,"writeApDataToShareMemory ap_write_index=%d md_read_index=%d"
                    " mRingBuffer_ReadIdx=%d mRingBuffer_DataSize=%d",
                    mApRingBufferCtrlParam.ap_write_index, mApRingBufferCtrlParam.md_read_index,
                    mRingBuffer_ReadIdx, mRingBuffer_DataSize);
        // get base address of DL_DATA region
        uint8_t *p_buf = ((uint8_t *)pShareMemory) + mApRingBufferCtrlParam.offset;

        // Ex: WPFA_SHM_DATA_SIZE=11,
        // [case1]
        //     ap_write_index=5, md_read_index=2,
        //     md_read_size=3, free size = 8,
        //     mRingBuffer_DataSize=7
        //     w2e=11-5=6
        // [case2]
        //     ap_write_index=5, md_read_index=2,
        //     md_read_size=3, free size = 8,
        //     mRingBuffer_DataSize=3
        //     w2e=11-5=6
        // [case3]
        //     ap_write_index=5, md_read_index=8,
        //     md_read_size=8, free size = 3,
        //     mRingBuffer_DataSize=3
        //     w2e is no needed

        if (mApRingBufferCtrlParam.ap_write_index >= mApRingBufferCtrlParam.md_read_index) {
            // remain size from writeIdx to eof in SHM
            // [case1,2]: w2e=11-5=6
            uint32_t w2e = mDlBufferSize - mApRingBufferCtrlParam.ap_write_index;
            if (mRingBuffer_DataSize <= w2e) {
                // [case2]: mRingBuffer_DataSize=3, w2e=6
                mtkLogD(WPFA_D_LOG_TAG,"[case2]writeApDataToShareMemory dataSize:%d w2e:%d",
                        mRingBuffer_DataSize, w2e);
                ringBuffer->readDataWithoutRegionCheck(p_buf + mApRingBufferCtrlParam.ap_write_index, mRingBuffer_ReadIdx, mRingBuffer_DataSize);
                // update wirte index
                // [case2]: ap_write_index=5+3=8
                mApRingBufferCtrlParam.ap_write_index += mRingBuffer_DataSize;
                if (mApRingBufferCtrlParam.ap_write_index == mDlBufferSize) {
                    mApRingBufferCtrlParam.ap_write_index = 0;
                }
            } else {
                // [case1]: mRingBuffer_DataSize=7, w2e=6
                mtkLogD(WPFA_D_LOG_TAG,"[case1]writeApDataToShareMemory dataSize:%d w2e:%d",
                        mRingBuffer_DataSize, w2e);
                ringBuffer->readDataWithoutRegionCheck(p_buf + mApRingBufferCtrlParam.ap_write_index, mRingBuffer_ReadIdx ,w2e);
                ringBuffer->readDataWithoutRegionCheck(p_buf, ((mRingBuffer_ReadIdx + w2e)%RING_BUFFER_SIZE), mRingBuffer_DataSize - w2e);
                // update wirte index
                // [case1]: ap_write_index=7-6=1
                mApRingBufferCtrlParam.ap_write_index = mRingBuffer_DataSize - w2e;
            }
        } else {   // readIdx > writeIdx
            // [case3]: mRingBuffer_DataSize=3, w2e=5
            mtkLogD(WPFA_D_LOG_TAG,"[case3]writeApDataToShareMemory dataSize:%d",
                    mRingBuffer_DataSize);
            ringBuffer->readDataWithoutRegionCheck(p_buf + mApRingBufferCtrlParam.ap_write_index, mRingBuffer_ReadIdx, mRingBuffer_DataSize);
            // [case3]: ap_write_index=5+3=8
            mApRingBufferCtrlParam.ap_write_index += mRingBuffer_DataSize;
        }
        newIdx = mApRingBufferCtrlParam.ap_write_index;
        dumpApRingBufferCtrlParam();
    } else {
        mtkLogE(WPFA_D_LOG_TAG,"writeApDataToShareMemory: no free space!!");
    }

    unlock("writeApDataToShareMemory");
    return newIdx;
}

int WpfaShmAccessController::openShareMemory() {
    int retValue = 0;

    // TODO: confrim with CCCI owner
    CCCI_MD ccci_md = MD_SYS1;
    CCCI_USER ccci_user = USR_CCCI_WIFI_PROXY;

    // get share momoey address
    // TODO: CCCI not ready the USR_SMEM_RAW_AUDIO is for speech not for WAFA
    mCcciShareMemoryHandler = ccci_smem_get(ccci_md, ccci_user,
                  &pShareMemoryBase, &mShareMemoryLength);

    if (mCcciShareMemoryHandler < 0) {
        mtkLogE(WPFA_D_LOG_TAG, "ccci_smem_get(%d) fail mCcciShareMemoryHandler: %d, errno: %d",
              ccci_md, (int32_t)mCcciShareMemoryHandler, errno);
        retValue = -1;
        return retValue;
    }

    if (pShareMemoryBase == NULL || mShareMemoryLength == 0) {
        mtkLogE(WPFA_D_LOG_TAG, "pShareMemoryBase(%p) == NULL || mShareMemoryLength(%u) == 0",
              pShareMemoryBase, (uint32_t)mShareMemoryLength);
        closeShareMemory();
        retValue = -1;
        return retValue;
    }

    if (mShareMemoryLength < sizeof(wpfa_shm_t)) {
        mtkLogE(WPFA_D_LOG_TAG, "mShareMemoryLength(%u) < sizeof(wpfa_shm_t): %u",
              (uint32_t)mShareMemoryLength, (uint32_t)sizeof(wpfa_shm_t));
        closeShareMemory();
        retValue = -1;
        return retValue;
    }
    mtkLogD(WPFA_D_LOG_TAG, "%s(), ccci_md: %d, mCcciShareMemoryHandler: %d, pShareMemoryBase: %p,"
          " mShareMemoryLength: %u", __FUNCTION__,
          ccci_md, mCcciShareMemoryHandler, pShareMemoryBase,
          (uint32_t)mShareMemoryLength);

    return mCcciShareMemoryHandler;
}

int WpfaShmAccessController::closeShareMemory() {
    int retValue = 0;
    mtkLogD(WPFA_D_LOG_TAG, "%s(), mCcciShareMemoryHandler: %d, pShareMemoryBase: %p, mShareMemoryLength: %u",
          __FUNCTION__, mCcciShareMemoryHandler, pShareMemoryBase, mShareMemoryLength);

    if (mCcciShareMemoryHandler >= 0) {
        // TODO: CCCI not ready
        ccci_smem_put(mCcciShareMemoryHandler, pShareMemoryBase, mShareMemoryLength);
        mCcciShareMemoryHandler = -1;
        pShareMemoryBase = NULL;
        mShareMemoryLength = 0;
        pShareMemory = NULL;
    }

    return retValue;
}

int WpfaShmAccessController::formatShareMemory() {
    int retValue = 0;
    lock("formatShareMemory");

    if (pShareMemoryBase == NULL || mShareMemoryLength < sizeof(wpfa_shm_t)) {
        mtkLogE(WPFA_D_LOG_TAG, " pShareMemoryBase==NULL || mShareMemoryLength<sizeof(sph_shm_t)");
        unlock("formatShareMemory");
        retValue = -1;
        return retValue;
    }

    pShareMemory = (wpfa_shm_t *)pShareMemoryBase;

    // only format share memory once after boot
    //if (get_uint32_from_property(kPropertyKeyShareMemoryInit) != 0) {
    //    goto FORMAT_SHARE_MEMORY_DONE;
    //}

    // 4 bytes gurard region header
    //pShareMemory->guard_region_header is 0x57694669;

    // 8 bytes for UL control param (MD write/AP read)
    //pShareMemory->ul_control_param.index = 0;
    //pShareMemory->ul_control_param.size = 0;

    // 8 bytes for DL control param (AP write/MD read)
    // TODO: need to confrim with modem of initial value(=begin address of ap data region or =0)
    pShareMemory->dl_control_param.index = 0;
    pShareMemory->dl_control_param.size = 0;

    switch (CURRENT_SHM_CONFIG_MODE) {
        case SHM_CONFIG_DL_ONLY:
            shm_memset(pShareMemory->data_region.u.dlOnlyDataBuffer.dl_data, 0,
                    WPFA_SHM_MAX_DATA_BUFFER_SIZE);
            mDlBufferSize = WPFA_SHM_MAX_DATA_BUFFER_SIZE;
            mUlBufferSize = 0;
            break;

        case SHM_CONFIG_UL_ONLY:
            /* not supported */
            retValue = -1;
            //shm_memset(pShareMemory->data_region.u.shareDataBuffer.ul_data, 0,
            //        WPFA_SHM_MAX_DATA_BUFFER_SIZE/2);
            //mUlBufferSize = WPFA_SHM_MAX_DATA_BUFFER_SIZE;
            //mDlBufferSize = 0;
            break;

        case SHM_CONFIG_SHARE:
            shm_memset(pShareMemory->data_region.u.shareDataBuffer.dl_data, 0,
                    WPFA_SHM_MAX_DATA_BUFFER_SIZE/2);

            mUlBufferSize = WPFA_SHM_MAX_DATA_BUFFER_SIZE/2;
            mDlBufferSize = WPFA_SHM_MAX_DATA_BUFFER_SIZE/2;
            break;

        default:
            mtkLogE(WPFA_D_LOG_TAG, "Error: unknown CURRENT_SHM_CONFIG_MODE");
            retValue = -1;
    }

    // set AP control params
    if (CURRENT_SHM_CONFIG_MODE == SHM_CONFIG_DL_ONLY) {
        mApRingBufferCtrlParam.offset =
            (uint8_t *)pShareMemory->data_region.u.dlOnlyDataBuffer.dl_data - (uint8_t *)pShareMemory;
        mtkLogD(WPFA_D_LOG_TAG, "mApRingBufferCtrlParam.offset: %d", mApRingBufferCtrlParam.offset);
    } else if (CURRENT_SHM_CONFIG_MODE == SHM_CONFIG_SHARE) {
        mApRingBufferCtrlParam.offset =
            (uint8_t *)pShareMemory->data_region.u.shareDataBuffer.dl_data - (uint8_t *)pShareMemory;
        mtkLogD(WPFA_D_LOG_TAG, "mApRingBufferCtrlParam.offset: %d", mApRingBufferCtrlParam.offset);
    } else {
        mtkLogE(WPFA_D_LOG_TAG, "share memory config not supported!!");
        retValue = -1;
    }
    mApRingBufferCtrlParam.ap_write_index = 0;
    mApRingBufferCtrlParam.md_read_index = 0;  //mapping to index in DL ctrl param(AP write/MD read)
    mApRingBufferCtrlParam.md_read_size = 0;  //mapping to size in DL ctrl param (AP write/MD read)
    mApRingBufferCtrlParam.temp_read_size = 0;

    // 4 bytes gurard region footer
    //pShareMemory->guard_region_footer is 0x96649675;

    unlock("formatShareMemory");
    return retValue;
}

void WpfaShmAccessController::lock(const char* user) {
    pthread_mutex_lock(&mShmAccessMutex);
    mtkLogD(WPFA_D_LOG_TAG, "[ShmAccessCtrl] lock success (%s)", user);
}

void WpfaShmAccessController::unlock(const char* user) {
    pthread_mutex_unlock(&mShmAccessMutex);
    mtkLogD(WPFA_D_LOG_TAG, "[ShmAccessCtrl] unlock success (%s)", user);
}

void WpfaShmAccessController::shm_memset(void *des, uint8_t value, uint16_t size) {
    char *p_des = (char *)des;
    uint16_t i = 0;

    for (i = 0; i < size; i++) {
        p_des[i] = value;
    }
}

void WpfaShmAccessController::shm_memcpy(void *des, const void *src, uint16_t size) {
    char *p_src = (char *)src;
    char *p_des = (char *)des;
    uint16_t i = 0;

    for (i = 0; i < size; i++) {
        p_des[i] = p_src[i];
    }
}

uint32_t WpfaShmAccessController::getFreeSizeOfApToMd() {
    return mDlBufferSize - mApRingBufferCtrlParam.md_read_size -
            mApRingBufferCtrlParam.temp_read_size;
}

void WpfaShmAccessController::dumpApRingBufferCtrlParam() {
    mtkLogD(WPFA_D_LOG_TAG,"dumpApRingBufferCtrlParam ap_write_index=%d,md_read_index=%d,"
            "md_read_size=%d,temp_read_size=%d",
            mApRingBufferCtrlParam.ap_write_index,
            mApRingBufferCtrlParam.md_read_index,
            mApRingBufferCtrlParam.md_read_size,
            mApRingBufferCtrlParam.temp_read_size);
}

void WpfaShmAccessController::dumpShmLayout() {
    mtkLogD(WPFA_D_LOG_TAG, "mCcciShareMemoryHandler: %d,"
            " pShareMemoryBase: %p, mShareMemoryLength: %u",
            mCcciShareMemoryHandler,
            pShareMemoryBase,
            (uint32_t)mShareMemoryLength);

    mtkLogD(WPFA_D_LOG_TAG, "pShareMemory: %p, pGuard_region_header: %p,"
            " ul_control_param: %p, dl_control_param: %p,"
            " data_region: %p, guard_region_footer: %p",
            (void*)pShareMemory,
            (void*)&(pShareMemory->guard_region_header),
            (void*)&(pShareMemory->ul_control_param),
            (void*)&(pShareMemory->dl_control_param),
            (void*)&(pShareMemory->data_region),
            (void*)&(pShareMemory->guard_region_footer));

    switch (CURRENT_SHM_CONFIG_MODE) {
        case SHM_CONFIG_DL_ONLY:
            mtkLogD(WPFA_D_LOG_TAG, "DL_ONLY dl_data: %p",
                    (void*)&(pShareMemory->data_region.u.dlOnlyDataBuffer.dl_data));
            break;

        case SHM_CONFIG_UL_ONLY:
            mtkLogD(WPFA_D_LOG_TAG, "UL_ONLY ul_data:: %p",
                    (void*)&(pShareMemory->data_region.u.ulOnlyDataBuffer.ul_data));
            break;

        case SHM_CONFIG_SHARE:
            mtkLogD(WPFA_D_LOG_TAG, "SHARE ul_data: %p, dl_data: %p",
                    (void*)&(pShareMemory->data_region.u.shareDataBuffer.ul_data),
                    (void*)&(pShareMemory->data_region.u.shareDataBuffer.dl_data));
            break;

        default:
            mtkLogE(WPFA_D_LOG_TAG, "Error: unknown CURRENT_SHM_CONFIG_MODE");
    }
}

void WpfaShmAccessController::dumpShmDLCtrParm() {
    mtkLogD(WPFA_D_LOG_TAG,"dumpShmDLCtrParm: pShareMemory: [%p]", (void*) &pShareMemory);
    dumpApRingBufferCtrlParam();
    mtkLogD(WPFA_D_LOG_TAG,"dumpShmDLCtrParm dl_control_param.index=%d,dl_control_param.size=%d,",
            pShareMemory->dl_control_param.index,
            pShareMemory->dl_control_param.size);
}

void WpfaShmAccessController::dumpShmWriteDataInShm(uint32_t index, uint32_t size) {
    // get base address of DL_DATA region
    //uint8_t *p_buf = ((uint8_t *)pShareMemory) + mApRingBufferCtrlParam.offset;
}

int WpfaShmAccessController::dump_hex(unsigned char *data, int len) {
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
