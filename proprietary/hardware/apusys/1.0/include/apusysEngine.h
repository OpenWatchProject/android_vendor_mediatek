#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include "apusysCmd.h"
#include "apusysMem.h"
//#include "ionAllocator.h"

class apusysDev : public IApusysDev {
public:
    apusysDev();
    ~apusysDev();

    int type;
    int idx;

    uint64_t hnd;
};

class apusysEngine : public IApusysEngine {
private:
    int mFd; // device fd

    std::string mUserName;
    pid_t mUserPid;
    std::vector<apusysCmd> mCmdList;
    std::vector<apusysDev *> mAllocDevList;
    std::unordered_map<uint64_t, uint64_t> mAsyncList; //<iApusysMem *, cmd_id>
    std::map<int, int> mDevNum; // <device type, device num>

    /* from kernel */
    uint32_t mDramType;
    uint64_t mMagicNum;
    uint8_t mCmdVersion;

public:
    apusysEngine(const char * userName);
    apusysEngine();
    ~apusysEngine();

    IMemAllocator * mMemAllocator;

    /* cmd functions */
    IApusysCmd * initCmd();
    bool destroyCmd(IApusysCmd * cmd);
    bool runCmd(IApusysCmd * cmd);
    bool runCmdAsync(IApusysCmd * cmd);
    bool waitCmd(IApusysCmd * cmd);

    bool runCmd(IApusysMem * cmdBuf);
    bool runCmdAsync(IApusysMem * cmdBuf);
    bool waitCmd(IApusysMem * cmdBuf);
    bool checkCmdValid(IApusysMem * cmdBuf);

    /* memory functions */
    IApusysMem * memAlloc(size_t size);
    IApusysMem * memAlloc(size_t size, uint32_t align);
    IApusysMem * memAlloc(size_t size, uint32_t align, uint32_t cache);
    IApusysMem * memAlloc(size_t size, uint32_t align, uint32_t cache, APUSYS_USER_MEM_E type);
    bool memFree(IApusysMem * mem);
    bool memSync(IApusysMem * mem);
    bool memInvalidate(IApusysMem * mem);
    IApusysMem * memImport(int shareFd, uint32_t size);
    bool memUnImport(IApusysMem * mem);

    /* device */
    int getDeviceNum(APUSYS_DEVICE_E type);
    IApusysDev * devAlloc(APUSYS_DEVICE_E deviceType);
    bool devFree(IApusysDev * idev);

    bool setPower(APUSYS_DEVICE_E deviceType, uint8_t boostValue);
};
