#include "sighandler.h"
#include "../include/recorder.h"


static void dumpBtTable(PUBRD pUBRD){
    size_t i, j;
    size_t numEntries;
    uintptr_t *backtrace;
    char *module_name = pUBRD->mConfig.module_name;
    PUBRD_BtTable pBtTable = &pUBRD->mBtTable;
    PUBRD_BtEntry pBtEntry;

    ubrd_info_log("[%s] dumpBtTable count:%zu\n", module_name, pBtTable->count);
    for (i = 0; i < BT_HASH_TABLE_SIZE; i++) {
        pBtEntry = pBtTable->slots[i];
        while (pBtEntry) {
            numEntries = pBtEntry->numEntries;
            backtrace = pBtEntry->backtrace;
            ubrd_info_log("[%s] pBtEntry:%p, size:%zu, allocations:%zu, free_refereced:%zu, backtrace\n",
                module_name, pBtEntry, pBtEntry->size, pBtEntry->allocations, pBtEntry->free_referenced);
            for (j = 0; j < numEntries; j++) {
                ubrd_info_log("[%s] bt:%p\n", module_name, (void *)pBtEntry->backtrace[j]);
            }
            pBtEntry = pBtEntry->next;
        }
    }
}

static void dumpHashTable(PUBRD pUBRD){
    int i = 0;
    char *module_name = pUBRD->mConfig.module_name;
    PUBRD_HashTable pHashTable = &pUBRD->mHashTable;
    PUBRD_HashEntry pHashEntry;
    PUBRD_EntryInfo pEntryInfo;

    if (!pUBRD->mConfig.mRecordWithRingBuf) {
        ubrd_info_log("[%s] dumpHashTable count:%zu\n", module_name, pHashTable->mCount);
        for (i = 0; i < HASH_TABLE_SIZE; i++) {
            pHashEntry = pHashTable->mBase[i];
            while (pHashEntry) {
                pEntryInfo = pHashEntry->mPEntryInfo;
                ubrd_info_log("[%s] pHashEntry:%p, addr:%p, size:%zu, pBtEntry:%p\n",
                    module_name, pHashEntry, pEntryInfo->mAddr, pEntryInfo->mBytes, pHashEntry->mPBtEntry);
                if (pEntryInfo->mExtraInfoLen && pEntryInfo->mExtraInfo) {
                    ubrd_info_log("[%s] mExtraInfoLen:%zu, mExtraInfo:%s\n",
                        module_name, pEntryInfo->mExtraInfoLen, (char *)pEntryInfo->mExtraInfo);
                }
                pHashEntry = pHashEntry->next;
            }
        }
    }
}

static void dumpRingBuffer(PUBRD pUBRD) {
    size_t i, j;
    size_t numEntries;
    uintptr_t *backtrace;
    char *module_name = pUBRD->mConfig.module_name;
    PUBRD_HashEntry pHashEntry;
    PUBRD_EntryInfo pEntryInfo;
    PUBRD_BtEntry pBtEntry;
    PUBRD_BT pBt;
    size_t RingBufferSize = pUBRD->mConfig.mRingBufferSize;

    if (RingBufferSize) {
        ubrd_info_log("[%s] dumpRingBuffer RingBufferSize:%zu\n", module_name, RingBufferSize);
        for (i = 0; i < RingBufferSize; i++) {
            pHashEntry = pUBRD->mRingBuffer.mBase[i];
            if (pHashEntry) {
                pEntryInfo = pHashEntry->mPEntryInfo;
                ubrd_info_log("[%s] pHashEntry:%p, addr:%p, size:%zu, pBtEntry:%p\n",
                    module_name, pHashEntry, pEntryInfo->mAddr, pEntryInfo->mBytes, pHashEntry->mPBtEntry);
                if (pEntryInfo->mExtraInfoLen && pEntryInfo->mExtraInfo) {
                    ubrd_info_log("[%s] mExtraInfoLen:%zu, mExtraInfo:%s\n",
                        module_name, pEntryInfo->mExtraInfoLen, (char *)pEntryInfo->mExtraInfo);
                }
                pBtEntry = pHashEntry->mPBtEntry;
                if (pBtEntry) {
                    ubrd_info_log("[%s] dump malloc backtrace\n", module_name);
                    numEntries = pBtEntry->numEntries;
                    backtrace = pBtEntry->backtrace;
                    for (j = 0; j < numEntries; j++) {
                        ubrd_info_log("[%s] bt:%p\n", module_name, (void *)backtrace[j]);
                    }
                }

                pBt = pHashEntry->mBt;
                if (pBt) {
                    ubrd_info_log("[%s] dump free backtrace\n", module_name);
                    numEntries = pHashEntry->mBt->numEntries;
                    backtrace = pHashEntry->mBt->backtrace;
                    for (j = 0; j < numEntries; j++) {
                        ubrd_info_log("[%s] bt:%p\n", module_name, (void *)backtrace[j]);
                    }
                }
            }
        }
    }
}

void dumpUBRD(PUBRD pUBRD) {
    char *module_name = pUBRD->mConfig.module_name;
    ubrd_info_log("[%s] pUBRD->mMspace: %p\n", module_name, pUBRD->mMspace);
    ubrd_info_log("[%s] pUBRD->mConfig.mDebugMspaceSize: 0x%x\n", module_name, pUBRD->mConfig.mDebugMspaceSize);
    ubrd_info_log("[%s] pUBRD->mConfig.mHisBufferSize: 0x%x\n", module_name, pUBRD->mConfig.mRingBufferSize);

    dumpBtTable(pUBRD);
    dumpHashTable(pUBRD);
    dumpRingBuffer(pUBRD);
}
