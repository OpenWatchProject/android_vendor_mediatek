#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <assert.h>

#include "sampleEngine.h"
#include "sampleCmn.h"
//#include "apusys.h"

#define SAMPLE_REQUEST_NAME_SIZE 32

struct sampleRequest {
    char name[SAMPLE_REQUEST_NAME_SIZE];
    uint32_t algoId;
    uint32_t delayMs;

    uint8_t driverDone;
};

class sampleCmd;

class sampleEngine : public ISampleEngine {
private:
    std::vector<sampleCmd *> mRequestList;
    std::mutex mListMtx;

public:
    sampleEngine(const char * userName);
    ~sampleEngine();
    ISampleCmd * getCmd();
    bool releaseCmd(ISampleCmd * cmd);
    bool runSync(ISampleCmd * cmd);

    /* for user who want to contruct big apusys cmd */
    IApusysMem * getSubCmdBuf(ISampleCmd * ICmd);
};

class sampleCmd : public ISampleCmd {
private:
    std::mutex mMtx;

    struct apusysProp mProp; // for mApusysCmd
    IApusysMem * mSubCmdBuf;

    sampleEngine * mEngine;

public:
    sampleCmd(sampleEngine * engine);
    ~sampleCmd();
    bool setup(std::string & name, uint32_t algoId, uint32_t delayMs);
    bool setProp(uint8_t priority, uint32_t targetMs);
    bool setNotDirty();
    IApusysMem * getCmdBuf();
};


//-----------------------------------
sampleCmd::sampleCmd(sampleEngine * engine)
{
    mSubCmdBuf = engine->memAlloc(sizeof(struct sampleRequest));
    assert(mSubCmdBuf != nullptr);

    LOG_DEBUG("mSubCmdBuf = %p, (0x%llx/0x%llx/0x%d)\n", mSubCmdBuf, mSubCmdBuf->va, mSubCmdBuf->kva, mSubCmdBuf->size);

    memset((void *)mSubCmdBuf->va, 0, sizeof(struct sampleRequest));
    mProp.priority = APUSYS_PRIORITY_NORMAL;
    mProp.targetMs = 0;

    mEngine = engine;

    return;
}

sampleCmd::~sampleCmd()
{
    if (mSubCmdBuf != nullptr)
    {
        if(mEngine->memFree(mSubCmdBuf) == false)
        {
            LOG_ERR("fail to free subcmd buf\n");
        }

    }
    return;
}

bool sampleCmd::setup(std::string & name, uint32_t algoId, uint32_t delayMs)
{
    struct sampleRequest * req = (struct sampleRequest *)mSubCmdBuf->va;
    std::vector<int> dependency;

    if (req == nullptr)
    {
        LOG_ERR("no valid request\n");
        return false;
    }

    dependency.clear();

    /* assign value */
    req->algoId = algoId;
    req->delayMs = delayMs;
    strncpy(req->name, name.c_str(), SAMPLE_REQUEST_NAME_SIZE - 1);

    LOG_DEBUG("setup request(%s/0x%x/%d)\n", req->name, req->algoId, req->delayMs);

    return true;
}

bool sampleCmd::setProp(uint8_t priority, uint32_t targetMs)
{
    struct apusysProp prop;

    if(priority >= APUSYS_PRIORITY_MAX)
    {
        return false;
    }

    mProp.priority = priority;
    mProp.targetMs = targetMs;

    return true;
}

bool sampleCmd::setNotDirty()
{
    struct sampleRequest * req = nullptr;

    if(mSubCmdBuf == nullptr)
    {
        LOG_ERR("invalid sub cmd buffer\n");
        return false;
    }

    req = (struct sampleRequest *)mSubCmdBuf->va;

    if (req->driverDone != 1)
    {
        LOG_ERR("driver dont execute this subcmd\n");
        return false;
    }

    req->driverDone = 0;

    return true;
}

IApusysMem * sampleCmd::getCmdBuf()
{
    struct sampleRequest * req = nullptr;

    if(mSubCmdBuf == nullptr)
    {
        LOG_ERR("invalid sub cmd buffer\n");
        return nullptr;
    }

    req = (struct sampleRequest *)mSubCmdBuf->va;
    LOG_INFO("==============================================================\n");
    LOG_INFO("| sample library getCmdBuf                                   |\n");
    LOG_INFO("-------------------------------------------------------------|\n");
    LOG_INFO("| name     = %-32s                |\n", req->name);
    LOG_INFO("| algo id  = 0x%-16x                              |\n", req->algoId);
    LOG_INFO("| delay ms = %-16d                                |\n", req->delayMs);
    LOG_INFO("| driver done(should be 0) = %-2d                              |\n", req->driverDone);
    LOG_INFO("==============================================================\n");

    return mSubCmdBuf;
}

//-----------------------------------
ISampleEngine::ISampleEngine(const char * userName):apusysEngine(userName)
{
    return;
}
//-----------------------------------

sampleEngine::sampleEngine(const char * userName):ISampleEngine(userName)
{
    mRequestList.clear();

    return;
}

sampleEngine::~sampleEngine()
{
    std::vector<sampleCmd*>::iterator iter;

    /* free all memory */
    std::unique_lock<std::mutex> mutexLock(mListMtx);
    while (!mRequestList.empty())
    {
        iter = mRequestList.end();
        delete(*iter);
        mRequestList.pop_back();
    }

    mRequestList.clear();

    return;
}

ISampleCmd * sampleEngine::getCmd()
{
    sampleCmd * cmd = new sampleCmd(this);

    if (cmd == nullptr)
    {
        LOG_ERR("allocate sample cmd fail\n");
        return nullptr;
    }

    mRequestList.push_back(cmd);

    return (ISampleCmd *)cmd;
}

/*
 * contruct sample's subCmd and return
 * it's used for user program contruct big apusys cmd
 */
IApusysMem * sampleEngine::getSubCmdBuf(ISampleCmd * ICmd)
{
    sampleCmd * cmd = (sampleCmd *)ICmd;
    std::vector<sampleCmd *>::iterator iter;

    if (cmd == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return nullptr;
    }

    std::unique_lock<std::mutex> mutexLock(mListMtx);
    for (iter = mRequestList.begin(); iter != mRequestList.end(); iter++)
    {
        if (*iter == ICmd)
        {
            return cmd->getCmdBuf();
        }
    }

    LOG_ERR("can't find this sample cmd\n");

    return nullptr;
}

bool sampleEngine::releaseCmd(ISampleCmd * ICmd)
{
    std::vector<sampleCmd *>::iterator iter;

    std::unique_lock<std::mutex> mutexLock(mListMtx);
    for (iter = mRequestList.begin(); iter != mRequestList.end(); iter++)
    {
        if (*iter == ICmd)
        {
            LOG_DEBUG(" subcmd = %p\n", *iter);
            mRequestList.erase(iter);
            delete (*iter);
            return true;
        }
    }

    return false;
}

bool sampleEngine::runSync(ISampleCmd * ICmd)
{
    IApusysCmd * apusysCmd = nullptr;
    sampleCmd * cmd = (sampleCmd *)ICmd;
    std::vector<int> dependency;
    IApusysMem * cmdBuf;

    /* check argument */
    if (cmd == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return false;
    }

    /* init apusysCmd */
    apusysCmd = initCmd();
    if (apusysCmd == nullptr)
    {
        LOG_ERR("init apusys cmd fail\n");
        return false;
    }

    /* get cmd buffer from sample command */
    cmd = (sampleCmd *)ICmd;
    cmdBuf = cmd->getCmdBuf();
    if (cmdBuf == nullptr)
    {
        destroyCmd(apusysCmd);
        return false;
    }

    /* assign to apusys cmd */
    apusysCmd->addSubCmd(cmdBuf, APUSYS_DEVICE_SAMPLE, dependency, 0, 0);

    /* run cmd */
    if(runCmd(apusysCmd) == false)
    {
        destroyCmd(apusysCmd);
        return false;
    }

    destroyCmd(apusysCmd);
    return true;
}

ISampleEngine * ISampleEngine::createInstance(const char * userName)
{
    sampleEngine * engine = new sampleEngine(userName);

    return (ISampleEngine *)engine;
}

bool ISampleEngine::deleteInstance(ISampleEngine * engine)
{
    if (engine == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return false;
    }

    delete (sampleEngine *)engine;
    return true;
}
