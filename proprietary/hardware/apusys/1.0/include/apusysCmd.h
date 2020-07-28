#pragma once

#include <cstdint>
#include <vector>
#include <deque>
#include <string>
#include <mutex>

#include "apusys.h"

class apusysSubCmd {
public:
    apusysSubCmd();
    ~apusysSubCmd();

    APUSYS_DEVICE_E type;
    int id;
    IApusysMem * subCmdBuf; // sub cmd
    std::vector<int> dependency;
    std::vector<int> beDepend; //which subcmd dependent on this subcmd
    std::vector<int> pack;

    uint32_t ctxId;
    uint64_t estimateMs;
    uint32_t tcmUsage;
    uint32_t tcmForce;

    bool valid;
};

class apusysCmd : public IApusysCmd {
private:
    uint64_t mCmdId;
    uint32_t mDependencySize; // recore all subcmd's dependency size

    IApusysMem * mCmdBuf; // whole apusys cmd
    std::vector<apusysSubCmd *> mSubCmdList;
    std::vector<int> mEmptyIdxList; //record subCmdList empty idx

    std::vector<int> mTypeCmdNum; // record how many type subcmd in this cmd

    std::mutex mMtx;
    bool mDirty;

    /* for memory allocator */
    IApusysEngine * mEngine;

    /* set by user */
    uint8_t mPriority;
    int mTargetMs; // deadline
    int mEstimateMs; // execute time from trail run

    uint32_t mMtxCtxId;

    /* calculate total apusys cmd size for allocate memory */
    uint32_t calcCmdSize(void); //

    /* mCmdBuf operation functions */
    bool constructCmd();
    void printCmd();

public:
    apusysCmd(IApusysEngine * engine);
    ~apusysCmd();

    /* cmd functions */
    bool setCmdProperty(struct apusysProp &prop);
    int addSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType, std::vector<int> & dependency);
    int addSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType, std::vector<int> & dependency, uint32_t estimateMs);
    int addSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType, std::vector<int> & dependency, uint32_t estimateMs, uint32_t ctxId);
    bool deleteSubCmd(uint32_t idx);
    bool modifySubCmd(uint32_t idx, IApusysMem * subCmdBuf);
    int packSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType , int subCmdIdx);
    bool unpackSubCmd(IApusysMem * subCmdBuf);

    /* for apusys engine only */
    IApusysMem * getCmdBuf(void);

    static bool checkCmdValid(IApusysMem * cmdBuf);
    static void printCmd(IApusysMem * cmdBuf);
};
