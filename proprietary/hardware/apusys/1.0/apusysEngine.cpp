#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fcntl.h>

#include <unistd.h>

#include "apusys_drv.h"

#include <sys/ioctl.h>

#include "apusys.h"
#include "apusysEngine.h"
#include "apusysCmn.h"
#include "apusysCmd.h"

#if MTK_APSYS_ION_ENABLE
#include "ionAllocator.h"
#else
#include "dmaAllocator.h"
#endif

apusysDev::apusysDev()
{
    idx = -1;
    type = -1;
    hnd = 0;
    return;
}

apusysDev::~apusysDev()
{
    idx = -1;
    type = -1;
    hnd = 0;
    return;
}

apusysEngine::apusysEngine(const char * userName)
{
    int ret = 0, idx = 0;
    uint64_t devSup = 0;
    struct apusys_ioctl_hs ioctlHandShake;

    mFd = open("/dev/apusys", O_RDWR | O_SYNC);
    if (mFd < 0)
    {
        LOG_ERR("=========================================\n");
        LOG_ERR(" open apusys device node fail, errno(%d)\n", errno);
        LOG_ERR("=========================================\n");
        //throw std::invalid_argument("open apusys fail");
        assert("open apusys fail");
        return;
    }

    LOG_DEBUG("apusys dev node = %d\n", mFd);

    /* assign user information */
    //mUserName.assign(name);
    mUserPid = getpid();
    mCmdList.clear();
    mAllocDevList.clear();
    mAsyncList.clear();
    mDevNum.clear();

    memset(&ioctlHandShake, 0, sizeof(ioctlHandShake));
    ioctlHandShake.type = APUSYS_HANDSHAKE_BEGIN;
    /* handshake with driver */
    ret = ioctl(mFd, APUSYS_IOCTL_HANDSHAKE, &ioctlHandShake);
    if (ret)
    {
        LOG_ERR("apusys handshake fail(%d)\n", ret);
    }
    mMagicNum = ioctlHandShake.magic_num;
    mCmdVersion = ioctlHandShake.cmd_version;
    mDramType = ioctlHandShake.begin.mem_support;


    assert(ioctlHandShake.begin.dev_type_max == APUSYS_DEVICE_MAX);
    assert(mMagicNum == APUSYS_MAGIC_NUMBER);
    assert(mCmdVersion == APUSYS_CMD_VERSION);

    /* query all device num */
    devSup = ioctlHandShake.begin.dev_support;
    LOG_DEBUG("device support = 0x%llx\n", devSup);
    while (devSup && idx < APUSYS_DEVICE_MAX)
    {
        if (devSup & 1<<idx)
        {
            memset(&ioctlHandShake, 0, sizeof(ioctlHandShake));
            ioctlHandShake.type = APUSYS_HANDSHAKE_QUERY_DEV;
            ioctlHandShake.dev.type = idx;
            LOG_DEBUG("query device(%d/%d)\n", APUSYS_HANDSHAKE_QUERY_DEV, idx);
            ret = ioctl(mFd, APUSYS_IOCTL_HANDSHAKE, &ioctlHandShake);
            if (ret)
            {
                LOG_ERR("apusys handshake fail(%d)\n", ret);
            }
            LOG_INFO("device(%d) has (%d) core in this platform\n", idx, ioctlHandShake.dev.num);

            mDevNum.insert(std::pair<int, int>(idx, (int)ioctlHandShake.dev.num));

            devSup &= ~(1<<idx);
        }
        idx++;
    }

#if MTK_APSYS_ION_ENABLE
    LOG_DEBUG("ION Enable\n");
    /* init memory allocator */
    mMemAllocator = (IMemAllocator *)new ionAllocator(mFd); //should be easy to change memory source
#else //dma
    LOG_DEBUG("DMA Enable\n");
    mMemAllocator = (IMemAllocator *)new dmaAllocator(mFd); //should be easy to change memory source
#endif

    if(mMemAllocator == nullptr)
    {
       LOG_ERR("=========================================\n");
       LOG_ERR(" init apusys memory allocator fail\n");
       LOG_ERR("=========================================\n");
       //throw std::invalid_argument("init memory allocator fail");
       assert("init memory allocator fail");
       return;
    }

    if (userName == nullptr)
    {
        LOG_WARN("userName is null\n");
        mUserName.assign("defaultUser");
    }
    else
    {
        mUserName.assign(userName);
    }

    LOG_INFO("user(%s) create apusys engine\n", mUserName.c_str());

    return;
}

apusysEngine::apusysEngine():apusysEngine("defaultUser")
{
    return;
}

apusysEngine::~apusysEngine()
{
    delete(mMemAllocator);
    close(mFd);

    return;
}

IApusysCmd * apusysEngine::initCmd()
{
    IApusysCmd * cmd = (IApusysCmd *)new apusysCmd((IApusysEngine *)this);
    if(cmd == nullptr)
    {
        LOG_ERR("init apusys cmd fail\n");
        return nullptr;
    }

    return cmd;
}


bool apusysEngine::destroyCmd(IApusysCmd * cmd)
{
    delete (apusysCmd *)cmd;

    return true;
}

bool apusysEngine::runCmd(IApusysCmd * cmd)
{
    int ret = 0;
    apusysCmd * cmdImpl = (apusysCmd *)cmd;
    IApusysMem * cmdBuf = nullptr;
    struct apusys_ioctl_cmd ioctlCmd;

    if(cmd == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return false;
    }

    if(cmd->constructCmd() == false)
    {
        LOG_ERR("construct cmd buffer fail\n");
        return false;
    }

    memset(&ioctlCmd, 0, sizeof(ioctlCmd));
    cmdBuf = cmdImpl->getCmdBuf();
    if(cmdBuf == nullptr)
    {
        LOG_ERR("extract cmd buffer fail\n");
        return false;
    }

    LOG_DEBUG("ioctlCmd(%d/0x%llx/0x%llx/%d)\n",cmdBuf->iova, cmdBuf->va, cmdBuf->kva, cmdBuf->size);
    ioctlCmd.uva = cmdBuf->va;
    ioctlCmd.size = cmdBuf->size;
    ioctlCmd.iova = cmdBuf->iova;
    ioctlCmd.kva = cmdBuf->kva;

    ret = ioctl(mFd, APUSYS_IOCTL_RUN_CMD_SYNC, &ioctlCmd);
    if(ret)
    {
        LOG_ERR("run cmd sync fail(%d)\n", ret);
        return false;
    }

    return true;
}
bool apusysEngine::runCmdAsync(IApusysCmd * cmd)
{
    int ret = 0;
    apusysCmd * cmdImpl = (apusysCmd *)cmd;
    IApusysMem * cmdBuf = nullptr;
    struct apusys_ioctl_cmd ioctlCmd;

    if(cmd == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return false;
    }

    if(cmd->constructCmd() == false)
    {
        LOG_ERR("construct cmd buffer fail\n");
        return false;
    }

    memset(&ioctlCmd, 0, sizeof(ioctlCmd));
    cmdBuf = cmdImpl->getCmdBuf();
    if(cmdBuf == nullptr)
    {
        LOG_ERR("extract cmd buffer async fail\n");
        return false;
    }

    LOG_DEBUG("ioctlCmd(%d/0x%llx/0x%llx/%d)\n",cmdBuf->iova, cmdBuf->va, cmdBuf->kva, cmdBuf->size);
    ioctlCmd.uva = cmdBuf->va;
    ioctlCmd.size = cmdBuf->size;
    ioctlCmd.iova = cmdBuf->iova;
    ioctlCmd.kva = cmdBuf->kva;

    ret = ioctl(mFd, APUSYS_IOCTL_RUN_CMD_ASYNC, &ioctlCmd);
    if(ret)
    {
        LOG_ERR("run cmd async fail(%d)\n", ret);
        return false;
    }

    LOG_DEBUG("wait cmd async is : %p/0x%llx\n", cmdBuf, ioctlCmd.cmd_id);
    mAsyncList.insert({(uint64_t)cmdBuf, ioctlCmd.cmd_id});

    return true;
}
bool apusysEngine::waitCmd(IApusysCmd * cmd)
{
    IApusysMem * cmdBuf = nullptr;
    apusysCmd * cmdImpl = (apusysCmd *)cmd;
    struct apusys_ioctl_cmd ioctlCmd;
    uint64_t cmdId = 0;
    int ret = 0;


    cmdBuf = cmdImpl->getCmdBuf();
    if(cmdBuf == nullptr)
    {
        LOG_ERR("wait cmd fail, can't find key\n");
        return false;
    }

    auto got = mAsyncList.find((uint64_t)cmdBuf);
    if (got == mAsyncList.end())
    {
        LOG_ERR("wrong async cmd(%p) to wait\n", cmdBuf);
        return false;
    }

    cmdId = got->second;

    memset(&ioctlCmd, 0, sizeof(ioctlCmd));
    ioctlCmd.cmd_id = cmdId;

    ret = ioctl(mFd, APUSYS_IOCTL_WAIT_CMD, &ioctlCmd);
    if(ret)
    {
        LOG_ERR("wait cmd fail(%d)\n", ret);
        return false;
    }

    mAsyncList.erase(got);

    return true;
}

bool apusysEngine::runCmd(IApusysMem * cmdBuf)
{
    int ret = 0;
    struct apusys_ioctl_cmd ioctlCmd;

    if (cmdBuf == nullptr)
    {
        LOG_ERR("invalid argument\n");
    }

    memset(&ioctlCmd, 0, sizeof(ioctlCmd));
    LOG_DEBUG("ioctlCmd(%d/0x%llx/0x%llx/%d)\n",cmdBuf->iova, cmdBuf->va, cmdBuf->kva, cmdBuf->size);
    ioctlCmd.uva = cmdBuf->va;
    ioctlCmd.size = cmdBuf->size;
    ioctlCmd.iova = cmdBuf->iova;
    ioctlCmd.kva = cmdBuf->kva;

    ret = ioctl(mFd, APUSYS_IOCTL_RUN_CMD_SYNC, &ioctlCmd);
    if(ret)
    {
        LOG_ERR("run cmd sync fail(%d)\n", ret);
        return false;
    }

    return true;
}

bool apusysEngine::runCmdAsync(IApusysMem * cmdBuf)
{
    int ret = 0;
    struct apusys_ioctl_cmd ioctlCmd;

    if (cmdBuf == nullptr)
    {
        LOG_ERR("invalid argument\n");
    }

    memset(&ioctlCmd, 0, sizeof(ioctlCmd));
    LOG_DEBUG("ioctlCmd(%d/0x%llx/0x%llx/%d)\n",cmdBuf->iova, cmdBuf->va, cmdBuf->kva, cmdBuf->size);
    ioctlCmd.uva = cmdBuf->va;
    ioctlCmd.size = cmdBuf->size;
    ioctlCmd.iova = cmdBuf->iova;
    ioctlCmd.kva = cmdBuf->kva;

    ret = ioctl(mFd, APUSYS_IOCTL_RUN_CMD_ASYNC, &ioctlCmd);
    if(ret)
    {
        LOG_ERR("run cmd sync fail(%d)\n", ret);
        return false;
    }

    mAsyncList.insert({(uint64_t)cmdBuf, ioctlCmd.cmd_id});

    return true;
}

bool apusysEngine::waitCmd(IApusysMem * cmdBuf)
{
    struct apusys_ioctl_cmd ioctlCmd;
    uint64_t cmdId = 0;
    int ret = 0;

    if(cmdBuf == nullptr)
    {
        LOG_ERR("wait cmd fail, can't find key\n");
        return false;
    }

    auto got = mAsyncList.find((uint64_t)cmdBuf);
    if (got == mAsyncList.end())
    {
        LOG_ERR("wrong async cmd(%p) to wait\n", cmdBuf);
        return false;
    }

    cmdId = got->second;

    memset(&ioctlCmd, 0, sizeof(ioctlCmd));
    ioctlCmd.cmd_id = cmdId;

    ret = ioctl(mFd, APUSYS_IOCTL_WAIT_CMD, &ioctlCmd);
    if(ret)
    {
        LOG_ERR("wait cmd fail(%d)\n", ret);
        return false;
    }

    mAsyncList.erase(got);

    return true;
}

bool apusysEngine::checkCmdValid(IApusysMem * cmdBuf)
{
    return apusysCmd::checkCmdValid(cmdBuf);
}

/* memory functions */
IApusysMem * apusysEngine::memAlloc(size_t size, uint32_t align, uint32_t cache, APUSYS_USER_MEM_E type)
{
    IApusysMem * mem = nullptr;

    switch(type)
    {
        case APUSYS_USER_MEM_DRAM:
            mem = mMemAllocator->alloc(size, align, cache);
            break;
        case APUSYS_USER_MEM_VLM:
            LOG_ERR("don't support this memory type(%d)\n", type);
            return nullptr;
            break;
        default:
            return nullptr;
    }

    return mem;
}

IApusysMem * apusysEngine::memAlloc(size_t size, uint32_t align, uint32_t cache)
{
    return mMemAllocator->alloc(size, align, cache);
}
IApusysMem * apusysEngine::memAlloc(size_t size, uint32_t align)
{
    return mMemAllocator->alloc(size, align, NONCACHED);
}

IApusysMem * apusysEngine::memAlloc(size_t size)
{
    return mMemAllocator->alloc(size, PAGE_ALIGN, NONCACHED);
}

bool apusysEngine::memFree(IApusysMem * mem)
{
    return mMemAllocator->free(mem);
}

bool apusysEngine::memSync(IApusysMem * mem)
{
    return mMemAllocator->flush(mem);
}

bool apusysEngine::memInvalidate(IApusysMem * mem)
{
    return mMemAllocator->invalidate(mem);
}

IApusysMem * apusysEngine::memImport(int shareFd, uint32_t size)
{
    return mMemAllocator->import(shareFd, size);
}
bool apusysEngine::memUnImport(IApusysMem * mem)
{
    return mMemAllocator->unimport(mem);
}

int apusysEngine::getDeviceNum(APUSYS_DEVICE_E type)
{
    std::map<int, int>::iterator iter;

    iter = mDevNum.find(type);
    if (iter == mDevNum.end())
    {
        LOG_ERR("don't support this device type(%d)\n", type);
        return -1;
    }

    return iter->second;
}

IApusysDev * apusysEngine::devAlloc(APUSYS_DEVICE_E deviceType)
{
    struct apusys_ioctl_dev ioctlDev;
    apusysDev * userDev = nullptr;
    int ret = 0;

    /* check type */
    if (deviceType <= APUSYS_DEVICE_NONE || deviceType >= APUSYS_DEVICE_MAX)
    {
        LOG_ERR("invalid device type(%d)\n", deviceType);
        return nullptr;
    }

    ioctlDev.dev_type = deviceType;
    ioctlDev.num = 1;
    ioctlDev.dev_idx = -1;
    ioctlDev.handle = 0;

    ret = ioctl(mFd, APUSYS_IOCTL_DEVICE_ALLOC, &ioctlDev);
    if(ret || ioctlDev.handle == 0 || ioctlDev.dev_idx == -1)
    {
        LOG_ERR("alloc dev(%d) fail(%d/0x%llx/%d)\n", deviceType, ret, ioctlDev.handle, ioctlDev.dev_idx);
        return nullptr;
    }

    /* new virtual dev for user */
    userDev = new apusysDev;
    if (userDev == nullptr)
    {
        LOG_ERR("alloc userdev(%d) fail\n", deviceType);
        ret = ioctl(mFd, APUSYS_IOCTL_DEVICE_FREE, &ioctlDev);
        if(ret)
        {
            LOG_ERR("free dev(%d) fail(%d)\n", deviceType, ret);
            return nullptr;
        }
    }

    userDev->idx = ioctlDev.dev_idx;
    userDev->type = ioctlDev.dev_type;
    userDev->hnd = ioctlDev.handle;

    mAllocDevList.push_back(userDev);

    LOG_DEBUG("alloc dev(%d/%p) ok, idx(%d)\n", deviceType, userDev, ioctlDev.dev_idx);

    return (IApusysDev *)userDev;
}

bool apusysEngine::devFree(IApusysDev * idev)
{
    apusysDev * dev = (apusysDev *)idev;
    std::vector<apusysDev *>::iterator iter;
    struct apusys_ioctl_dev ioctlDev;
    int ret = 0;

    /* check argument */
    if (dev == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return false;
    }

    LOG_DEBUG("free dev(%p/%d)\n", dev, mAllocDevList.size());

    /* query dev from allocDevList */
    for (iter = mAllocDevList.begin(); iter != mAllocDevList.end(); iter++)
    {
        if (dev == *iter)
        {
            /* delete from allocDevList */
            mAllocDevList.erase(iter);
            /* call ioctl to notify kernel to free device */
            ioctlDev.dev_type = dev->type;
            ioctlDev.num = 1;
            ioctlDev.dev_idx = dev->idx;
            ioctlDev.handle = dev->hnd;
            ret = ioctl(mFd, APUSYS_IOCTL_DEVICE_FREE, &ioctlDev);
            if(ret)
            {
                LOG_ERR("free dev(%d/%d) fail(%d)\n", dev->type, dev->idx, ret);
                return false;
            }
            return true;
        }
    }

    return false;
}


bool apusysEngine::setPower(APUSYS_DEVICE_E deviceType, uint8_t boostValue)
{
    /* check type */
    if (deviceType <= APUSYS_DEVICE_NONE || deviceType >= APUSYS_DEVICE_MAX)
    {
        LOG_ERR("invalid device type(%d)\n", deviceType);
        return false;
    }

    return false;
}


//---------------------------------------------
IApusysEngine * IApusysEngine::createInstance(const char * userName)
{
    return new apusysEngine(userName);
}

bool IApusysEngine::deleteInstance(IApusysEngine * engine)
{
    if (engine == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return false;
    }

    delete (apusysEngine *)engine;
    return true;
}

