#include <iostream>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <assert.h>

#include "apusysCmd.h"
#include "apusysMem.h"
#include "apusysCmn.h"

#include "cmdFormat.h"

static TYPE_APUSYS_MAGIC getMagicNumFromCmdbuf(uint64_t cmdBufVa)
{
    return *(TYPE_APUSYS_MAGIC *)(cmdBufVa + OFFSET_APUSYS_MAGIC);
}

static TYPE_APUSYS_CMD_ID getCmdIdFromCmdbuf(uint64_t cmdBufVa)
{
    return *(TYPE_APUSYS_CMD_ID *)(cmdBufVa + OFFSET_APUSYS_CMD_ID);
}

static TYPE_APUSYS_CMD_VERSION getCmdVersionFromCmdbuf(uint64_t cmdBufVa)
{
    return *(TYPE_APUSYS_CMD_VERSION *)(cmdBufVa + OFFSET_APUSYS_CMD_VERSION);
}

static TYPE_PRIORITY getPriorityFromCmdbuf(uint64_t cmdBufVa)
{
    return *(TYPE_PRIORITY *)(cmdBufVa + OFFSET_PRIORITY);
}

static TYPE_TARGET_TIME getTargetTimeFromCmdbuf(uint64_t cmdBufVa)
{
    return *(TYPE_TARGET_TIME *)(cmdBufVa + OFFSET_TARGET_TIME);
}

static TYPE_NUM_OF_SUBGRAPH getNumSubcmdFromCmdbuf(uint64_t cmdBufVa)
{
    return *(TYPE_NUM_OF_SUBGRAPH *)(cmdBufVa + OFFSET_NUM_OF_SUBGRAPH);
}

static TYPE_OFFSET_TO_DEPENDENCY_INFO_LIST getOffsetDependencyFromCmdbuf(uint64_t cmdBufVa)
{
    return *(TYPE_OFFSET_TO_DEPENDENCY_INFO_LIST *)(cmdBufVa + OFFSET_OFFSET_TO_DEPENDENCY_INFO_LIST);
}

apusysSubCmd::apusysSubCmd()
{
    id = -1;
    subCmdBuf = nullptr;

    dependency.clear();
    beDepend.clear();
    pack.clear();
    valid = false;

    return;
}

apusysSubCmd::~apusysSubCmd()
{
    return;
}

/*
 * private functions
 */
uint32_t apusysCmd::calcCmdSize(void)
{
    uint32_t numSubgraph = 0;
    uint32_t numPmuInfo = 0;
    uint32_t cmdSize = 0;

    /* header + dependency + numsubgraphs + subgraphs + pmu infos */
    numSubgraph = mSubCmdList.size();
    if (numSubgraph == 0)
    {
        return 0;
    }

    cmdSize = SIZE_APUSYS_HEADER + mDependencySize + numSubgraph * SIZE_SUBGRAPH_INFO_POINTER + numSubgraph * SIZE_SUBGRAPH_ELEMENT;

    /* sample/vpu command add uint32_t for pack cmd */
    LOG_DEBUG("subcmd device num sample(%d) dsp(%d)\n", mTypeCmdNum.at(APUSYS_DEVICE_SAMPLE), mTypeCmdNum.at(APUSYS_DEVICE_VPU));
    cmdSize += (SIZE_SUBGRAPH_PACK * (mTypeCmdNum.at(APUSYS_DEVICE_SAMPLE) + mTypeCmdNum.at(APUSYS_DEVICE_VPU)));

    LOG_DEBUG("cmd size : %d:(%d/%d/%d*%d/%d*%d/4*%d)\n", cmdSize, SIZE_APUSYS_HEADER, mDependencySize, numSubgraph, SIZE_SUBGRAPH_INFO_POINTER, numSubgraph, SIZE_SUBGRAPH_ELEMENT, (mTypeCmdNum.at(APUSYS_DEVICE_SAMPLE) + mTypeCmdNum.at(APUSYS_DEVICE_VPU)));

    return cmdSize;
}

void apusysCmd::printCmd()
{
    apusysSubCmd * subCmd = nullptr;
    int i = 0, j = 0;

    for (i < 0; i < mSubCmdList.size(); i++)
    {
        subCmd = mSubCmdList.at(i);
        LOG_DEBUG("(%d)#%d subcmd: \n", i, subCmd->id);
        LOG_DEBUG("    dpList: ");
        for (j = 0; j < subCmd->dependency.size(); j++)
        {
            LOG_CON(" %d", subCmd->dependency.at(j));
        }
        LOG_CON("\n");
        LOG_DEBUG("    beDpList: ");
        for (j = 0; j < subCmd->beDepend.size(); j++)
        {
            LOG_CON(" %d", subCmd->beDepend.at(j));
        }
        LOG_CON("\n");
        LOG_DEBUG("    pack: ");
        for (j = 0; j < subCmd->pack.size(); j++)
        {
            LOG_CON(" %d", subCmd->pack.at(j));
        }
        LOG_CON("\n");
        LOG_DEBUG("    ctx id: %d\n", subCmd->ctxId);
        LOG_CON("\n");
    }

    return;
}

bool apusysCmd::constructCmd()
{
    std::unique_lock<std::mutex> mutexLock(mMtx);
    uint32_t cmdBufSize = 0, numSubCmd = 0;
    int i = 0, j = 0;
    apusysSubCmd * subCmd = nullptr;
    uint64_t subCmdVa = 0, subCmdKva = 0;
    uint64_t dependencyKva = 0, dependencyVa = 0, tempDependecyAddr = 0;
    uint64_t cmdBufKva = 0, cmdBufVa = 0;
    uint32_t subCmdListSize = 0;

    /* check cmdbuf */
    if (mDirty == false)
    {
        return true;
    }

    /* print debug cmd info */
    //printCmd();

    /* calculate total apusys size for alloc */
    cmdBufSize = calcCmdSize();
    if (cmdBufSize == 0)
    {
        LOG_ERR("apusys no subcmd\n");
        return false;
    }

    if (mCmdBuf != nullptr)
    {
        if (mEngine->memFree(mCmdBuf) == false)
        {
            LOG_ERR("re-construct cmd fail, free origin cmdbuf error\n");
            return false;
        }
        else
        {
            /* re-construct apusys cmd */
            mEngine = nullptr;
            return constructCmd();
        }
    }

    /* allocate cmdBuf */
    mCmdBuf = mEngine->memAlloc(cmdBufSize);
    if (mCmdBuf == nullptr)
    {
        LOG_ERR("allocate cmdbuf(%d) fail\n", cmdBufSize);
        return false;
    }

    /* num of subcmd */
    numSubCmd = mSubCmdList.size();
    subCmdListSize = numSubCmd * SIZE_SUBGRAPH_INFO_POINTER;

    /* fill header */
    cmdBufVa = mCmdBuf->va;
    cmdBufKva = mCmdBuf->kva;
    /* don't consider mdla buffer size */
    //dependencyKva = cmdBufKva + SIZE_APUSYS_HEADER + numSubCmd * SIZE_SUBGRAPH_INFO_POINTER + numSubCmd * SIZE_SUBGRAPH_ELEMENT;
    //dependencyVa = cmdBufVa + SIZE_APUSYS_HEADER + numSubCmd * SIZE_SUBGRAPH_INFO_POINTER + numSubCmd * SIZE_SUBGRAPH_ELEMENT;


    *(TYPE_APUSYS_MAGIC *)(cmdBufVa + OFFSET_APUSYS_MAGIC) = APUSYS_MAGIC_NUMBER;
    *(TYPE_APUSYS_CMD_ID *)(cmdBufVa + OFFSET_APUSYS_CMD_ID) = mCmdId;
    *(TYPE_APUSYS_CMD_VERSION *)(cmdBufVa + OFFSET_APUSYS_CMD_VERSION) = APUSYS_CMD_VERSION;
    *(TYPE_PRIORITY *)(cmdBufVa + OFFSET_PRIORITY) = mPriority;
    *(TYPE_TARGET_TIME *)(cmdBufVa + OFFSET_TARGET_TIME) = mEstimateMs;
    *(TYPE_DEADLINE *)(cmdBufVa + OFFSET_DEADLINE) = mTargetMs;
    *(TYPE_NUM_OF_SUBGRAPH *)(cmdBufVa + OFFSET_NUM_OF_SUBGRAPH) = numSubCmd;
    //*(TYPE_OFFSET_TO_DEPENDENCY_INFO_LIST *)(cmdBufVa + OFFSET_OFFSET_TO_DEPENDENCY_INFO_LIST) = dependencyKva - cmdBufKva;

    /* fill subgraph info & subgraph */
    subCmdKva = (uint64_t)(cmdBufKva + OFFSET_SUBGRAPH_INFO_OFFSET_LIST + subCmdListSize);
    subCmdVa = (uint64_t)(cmdBufVa + OFFSET_SUBGRAPH_INFO_OFFSET_LIST + subCmdListSize);
    for (i = 0; i < numSubCmd; i++)
    {
        subCmd = mSubCmdList.at(i);
        /* fill addr */
        *(TYPE_SUBGRAPH_INFO_POINTER *)(cmdBufVa + OFFSET_SUBGRAPH_INFO_OFFSET_LIST + i*SIZE_SUBGRAPH_INFO_POINTER) = (uint32_t)(subCmdKva - cmdBufKva); //offset
        LOG_DEBUG("subcmd(%d) offset = 0x%x, (0x%llx/0x%llx)\n", i, subCmdKva - cmdBufKva, subCmdKva, cmdBufKva);
        LOG_DEBUG("subcmd(%d) (%d/%d/%d/%d/%d)\n", i, subCmd->type, subCmd->estimateMs, subCmd->tcmUsage, subCmd->tcmForce, subCmd->subCmdBuf->size);

        /* fill subgraph */
        *(TYPE_SUBGRAPH_TYPE *)(subCmdVa + OFFSET_SUBGRAPH_TYPE) = subCmd->type;
        *(TYPE_SUBGRAPH_TARGET_TIME *)(subCmdVa + OFFSET_SUBGRAPH_TARGET_TIME) = subCmd->estimateMs;
        *(TYPE_SUBGRAPH_UP_TURNAROUND *)(subCmdVa + OFFSET_SUBGRAPH_UP_TURNAROUND) = 0;
        *(TYPE_SUBGRAPH_DRIVER_TURNAROUND *)(subCmdVa + OFFSET_SUBGRAPH_DRIVER_TURNAROUND) = 0;
        *(TYPE_SUBGRAPH_TCM_USAGE *)(subCmdVa + OFFSET_SUBGRAPH_TCM_USAGE) = subCmd->tcmUsage;
        *(TYPE_SUBGRAPH_TCM_FORCE *)(subCmdVa + OFFSET_SUBGRAPH_TCM_FORCE) = subCmd->tcmForce;
        *(TYPE_SUBGRAPH_CTX_ID *)(subCmdVa + OFFSET_SUBGRAPH_CTX_ID) = subCmd->ctxId;
        *(TYPE_SUBGRAPH_SIZE *)(subCmdVa + OFFSET_SUBGRAPH_SIZE) = subCmd->subCmdBuf->size;
        *(TYPE_SUBGRAPH_ADDR *)(subCmdVa + OFFSET_SUBGRAPH_ADDR) = subCmd->subCmdBuf->kva;

        /* handle pack id */
        if (subCmd->type == APUSYS_DEVICE_SAMPLE || subCmd->type == APUSYS_DEVICE_VPU)
        {
            if ((subCmd->pack).size() > 1)
            {
                std::vector<int>::iterator it = std::find((subCmd->pack).begin(), (subCmd->pack).end(), i);
                std::vector<int>::iterator itNext;
                if (it != (subCmd->pack).end())
                {
                    itNext = ++it;
                    if (itNext == (subCmd->pack).end())
                    {
                        *(TYPE_SUBGRAPH_PACK *)(subCmdVa + OFFSET_SUBGRAPH_PACK) = (subCmd->pack).front();
                    }
                    else
                    {
                        *(TYPE_SUBGRAPH_PACK *)(subCmdVa + OFFSET_SUBGRAPH_PACK) = *it;
                    }
                    LOG_DEBUG("set #%d subcmd pack id (%d)\n", i, *(TYPE_SUBGRAPH_PACK *)(subCmdVa + OFFSET_SUBGRAPH_PACK));
                }
                else
                {
                    *(TYPE_SUBGRAPH_PACK *)(subCmdVa + OFFSET_SUBGRAPH_PACK) = VALUE_SUBGAPH_PACK_ID_NONE;
                }
            }
            else
            {
                *(TYPE_SUBGRAPH_PACK *)(subCmdVa + OFFSET_SUBGRAPH_PACK) = VALUE_SUBGAPH_PACK_ID_NONE;
            }

            subCmdKva = (uint64_t)(subCmdKva + SIZE_SUBGRAPH_ELEMENT + SIZE_SUBGRAPH_PACK);
            subCmdVa = (uint64_t)(subCmdVa + SIZE_SUBGRAPH_ELEMENT + SIZE_SUBGRAPH_PACK);
        }
        else
        {
            subCmdKva = (uint64_t)(subCmdKva + SIZE_SUBGRAPH_ELEMENT);
            subCmdVa = (uint64_t)(subCmdVa + SIZE_SUBGRAPH_ELEMENT);
        }
    }

    /* fill dependency list */
    dependencyKva = subCmdKva;
    dependencyVa = subCmdVa;
    *(TYPE_OFFSET_TO_DEPENDENCY_INFO_LIST *)(cmdBufVa + OFFSET_OFFSET_TO_DEPENDENCY_INFO_LIST) = dependencyKva - cmdBufKva;
    LOG_DEBUG("magic(0x%llx),cmdId(0x%llx),cmdBufSize(%d),numSubCmd(%d),subCmdListSize(%d),cmdBufVa(0x%llx),dependencyKva(0x%llx)\n",
        APUSYS_MAGIC_NUMBER,mCmdId,cmdBufSize,numSubCmd,subCmdListSize,cmdBufVa,dependencyKva);

    tempDependecyAddr = dependencyVa;
    for (i = 0; i<numSubCmd; i++)
    {
        /* setup self subcmd id */
        subCmd = mSubCmdList.at(i);
        *(TYPE_DEPENDENCY_ELEMENT *)tempDependecyAddr = (TYPE_DEPENDENCY_ELEMENT)subCmd->dependency.size();
        tempDependecyAddr += SIZE_DEPENDENCY_ELEMENT;

        LOG_DEBUG("subcmd #%d, dependency size(%d)\n", i, subCmd->dependency.size());

        /* setup dependency id */
        for (j = 0; j < subCmd->dependency.size(); j++)
        {
            *(TYPE_DEPENDENCY_ELEMENT *)tempDependecyAddr = (TYPE_DEPENDENCY_ELEMENT)subCmd->dependency.at(j);
            LOG_DEBUG("subcmd #%d, dependency(%d)/%d)\n", i, j, subCmd->dependency.at(j));
            tempDependecyAddr += SIZE_DEPENDENCY_ELEMENT;
        }
    }

    /* mark this buffer is not dirty */
    mDirty = false;

    return true;
}

/*
 * public functions
 */
apusysCmd::apusysCmd(IApusysEngine * engine)
{
    assert(engine != nullptr);

    /* init private member */
    mCmdId = (uint64_t)this;
    mSubCmdList.clear();
    mEmptyIdxList.clear();
    mCmdBuf = 0;
    mDirty = false;
    mDependencySize = 0;
    mTypeCmdNum.assign(APUSYS_DEVICE_MAX, 0);
    mEngine = engine; // for memory allocator
    mMtxCtxId = VALUE_SUBGAPH_CTX_ID_NONE;

    /* user may set */
    mTargetMs = 0;
    mEstimateMs = 0;
    mPriority = APUSYS_PRIORITY_NORMAL;

    return;
}

apusysCmd::~apusysCmd()
{
    apusysSubCmd * subcmd = nullptr;

    /* delete all subcmd */
    while(!mSubCmdList.empty())
    {
        subcmd = mSubCmdList.back();
        if (subcmd == nullptr)
        {
            LOG_ERR("release all subcmd fail, iter null\n");
            break;
        }

        delete subcmd;
        mSubCmdList.pop_back();
    }

    /* free apusys cmdbuf */
    if (mCmdBuf != nullptr)
    {
        if (mEngine->memFree(mCmdBuf) == false)
        {
            LOG_ERR("release cmd fail, free origin cmdbuf error\n");
        }
        else
        {
            mCmdBuf == nullptr;
        }
    }

    mTypeCmdNum.clear();

    return;
}

/* cmd functions */
bool apusysCmd::setCmdProperty(struct apusysProp &prop)
{
    /* critical session */
    std::unique_lock<std::mutex> mutexLock(mMtx);

    if(prop.priority >=0 && prop.priority < APUSYS_PRIORITY_MAX)
    {
        mPriority = prop.priority;
    }
    if(prop.targetMs >= 0)
    {
        mTargetMs = prop.targetMs;
    }
    if(prop.estimateMs >= 0)
    {
        mEstimateMs = prop.estimateMs;
    }

    return true;
}

int apusysCmd::addSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType, std::vector<int> & dependency)
{
    return addSubCmd(subCmdBuf, deviceType, dependency, 0, VALUE_SUBGAPH_CTX_ID_NONE);
}

int apusysCmd::addSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType, std::vector<int> & dependency, uint32_t estimateMs)
{
    return addSubCmd(subCmdBuf, deviceType, dependency, estimateMs, VALUE_SUBGAPH_CTX_ID_NONE);
}

int apusysCmd::addSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType, std::vector<int> & dependency, uint32_t estimateMs, uint32_t ctxId)
{
    int subCmdId = -1, i = 0;
    apusysSubCmd * subCmd = nullptr;
    apusysSubCmd * tmpSubCmd = nullptr;

    if (deviceType >= APUSYS_DEVICE_MAX)
    {
        LOG_ERR("invalid device type(%d)\n\n", deviceType);
        return -1;
    }

    /* check argument valid */
    if(subCmdBuf == nullptr)
    {
        LOG_ERR("allocate subCmd fail\n");
        return -1;
    }

    /* check ctx id valid */
    if (ctxId != VALUE_SUBGAPH_CTX_ID_NONE)
    {
        if (ctxId > mSubCmdList.size())
        {
            LOG_ERR("ctx id(%d) is bigger than #subCmdBuf num(%d)\n", ctxId, mSubCmdList.size());
            return -1;
        }

        if (mMtxCtxId == VALUE_SUBGAPH_CTX_ID_NONE)
        {
            mMtxCtxId = ctxId;
        }
        else
        {
            mMtxCtxId = mMtxCtxId > ctxId ? mMtxCtxId : ctxId;
        }
    }

    /* new subCmd */
    subCmd = new apusysSubCmd();
    if(subCmd == nullptr)
    {
        LOG_ERR("allocate subCmd fail\n");
        return -1;
    }


    /* set variable */
    subCmd->type = deviceType;
    subCmd->subCmdBuf = subCmdBuf;
    subCmd->valid = true;
    subCmd->ctxId = ctxId;
    subCmd->dependency = dependency;
    subCmd->estimateMs = estimateMs;

    /* critical session */
    std::unique_lock<std::mutex> mutexLock(mMtx);
    /* get subCmd idx */
    if(!mEmptyIdxList.empty())
    {
        subCmdId = mEmptyIdxList.at(0);
        mEmptyIdxList.erase(mEmptyIdxList.begin());
    }
    else
    {
        subCmdId = mSubCmdList.size();
    }

    subCmd->id = subCmdId;
    (subCmd->pack).push_back(subCmdId);

    /* add sub cmd to private list */
    mSubCmdList.push_back(subCmd);
    mTypeCmdNum.at(deviceType)++;

    /* add be-dependency list */
    for (i = 0; i < dependency.size(); i++)
    {
        tmpSubCmd = mSubCmdList.at(dependency.at(i));
        tmpSubCmd->beDepend.push_back(subCmd->id);
    }

    /* mark this apusys cmd has been changed, should construct apusys cmd again before fire cmd */
    mDirty = true;

    /* record total dependency size for easy to calculate total cmdbuf size */
    mDependencySize += ((1 + dependency.size()) * SIZE_DEPENDENCY_ELEMENT);

    LOG_DEBUG("subCmdId = %d, size = %d\n",subCmdId, subCmd->subCmdBuf->size);

    return subCmdId;
}

bool apusysCmd::deleteSubCmd(uint32_t idx)
{
    LOG_ERR("don't support deleteSubCmd\n");
    return false;
}

bool apusysCmd::modifySubCmd(uint32_t idx, IApusysMem * subCmdBuf)
{
    LOG_ERR("don't support modifySubCmd\n");
    return false;
}

int apusysCmd::packSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType , int subCmdIdx)
{
    apusysSubCmd * subCmd = nullptr;
    apusysSubCmd * targetSubCmd = nullptr, * tempSubCmd = nullptr;
    int retIdx = 0;
    int i = 0, j = 0;

    /* check argument */
    if (deviceType != APUSYS_DEVICE_VPU && deviceType != APUSYS_DEVICE_SAMPLE)
    {
        LOG_ERR("device(%d) command not support pack function\n", deviceType);
        return -1;
    }

    if (subCmdBuf == nullptr)
    {
        LOG_ERR("invalid sub-command buffer\n");
        return -1;
    }

    if (subCmdIdx > mSubCmdList.size()-1)
    {
        LOG_ERR("pack idx over list number(%d/%d)\n", subCmdIdx, mSubCmdList.size());
        return -1;
    }

    /* critical session */
    std::unique_lock<std::mutex> mutexLock(mMtx);

    targetSubCmd = mSubCmdList.at(subCmdIdx);
    if (targetSubCmd->type != APUSYS_DEVICE_VPU && targetSubCmd->type != APUSYS_DEVICE_SAMPLE)
    {
        LOG_ERR("target subcmd(%d) type(%d) is not support pack function\n", subCmdIdx, targetSubCmd->type);
        return -1;
    }

    /* new subCmd */
    subCmd = new apusysSubCmd();
    if(subCmd == nullptr)
    {
        LOG_ERR("allocate subCmd fail\n");
        return -1;
    }

    /* set variable */
    subCmd->type = deviceType;
    subCmd->subCmdBuf = subCmdBuf;
    subCmd->valid = true;
    subCmd->dependency = targetSubCmd->dependency;
    subCmd->beDepend = targetSubCmd->beDepend;
    subCmd->estimateMs = targetSubCmd->estimateMs;
    subCmd->pack = targetSubCmd->pack;
    /* packed cmd's ctx id should be the same */
    if (targetSubCmd->ctxId == VALUE_SUBGAPH_CTX_ID_NONE)
    {
        targetSubCmd->ctxId = ++mMtxCtxId;
    }
    subCmd->ctxId = targetSubCmd->ctxId;

    retIdx = mSubCmdList.size();
    mSubCmdList.push_back(subCmd);
    mTypeCmdNum.at(deviceType)++;

    mDependencySize += ((1 + subCmd->dependency.size()) * SIZE_DEPENDENCY_ELEMENT);
    /* add dependency list to next subcmd */
    for (i = 0;i<subCmd->beDepend.size(); i++)
    {
        tempSubCmd = mSubCmdList.at(subCmd->beDepend.at(i));
        tempSubCmd->dependency.push_back(retIdx);
        std::sort(tempSubCmd->dependency.begin(), tempSubCmd->dependency.end());
        mDependencySize += SIZE_DEPENDENCY_ELEMENT;
    }

    /* add be dependency to prev subcmd */
    for (i = 0;i<subCmd->dependency.size(); i++)
    {
        tempSubCmd = mSubCmdList.at(subCmd->dependency.at(i));
        tempSubCmd->beDepend.push_back(retIdx);
        std::sort(tempSubCmd->beDepend.begin(), tempSubCmd->beDepend.end());
    }

    /* push to target & self */
    targetSubCmd->pack.push_back(retIdx);
    subCmd->pack.push_back(retIdx);

    /* sorting */
    std::sort(targetSubCmd->pack.begin(),targetSubCmd->pack.end());
    std::sort(subCmd->pack.begin(),subCmd->pack.end());

    LOG_DEBUG("pack subCmd idx = %d, to %d\n", retIdx, subCmdIdx);
    LOG_DEBUG("target subCmd pack list size (%d/%d)\n", (subCmd->pack).size(), (targetSubCmd->pack).size());

    /* mark all packed command */
    for (i = 0; i < (targetSubCmd->pack).size();i++)
    {
        if ((targetSubCmd->pack).at(i)!= retIdx && (targetSubCmd->pack).at(i)!= subCmdIdx)
        {
            LOG_DEBUG("cmd = %d\n", (targetSubCmd->pack).at(i));
            tempSubCmd = mSubCmdList.at((targetSubCmd->pack).at(i));
            tempSubCmd->pack.push_back(retIdx);

            std::sort(tempSubCmd->pack.begin(),tempSubCmd->pack.end());
            for (j = 0; j < tempSubCmd->pack.size(); j++)
            {
                LOG_INFO("packing function: %d/%d \n", targetSubCmd->pack.at(i), tempSubCmd->pack.at(j));
            }
        }
    }

    return true;
}

bool apusysCmd::unpackSubCmd(IApusysMem * subCmdBuf)
{
    LOG_ERR("not support\n");
    return false;
}

IApusysMem * apusysCmd::getCmdBuf(void)
{
    std::unique_lock<std::mutex> mutexLock(mMtx);

    if(mDirty == true)
    {
        if(constructCmd() == false)
        {
            return nullptr;
        }
    }

    if(mCmdBuf == nullptr)
    {
        LOG_ERR("cmd buffer invalid(%p)\n", mCmdBuf);
        return nullptr;
    }

    return mCmdBuf;
}

bool apusysCmd::checkCmdValid(IApusysMem * cmdBuf)
{
    uint64_t cmdBufVa = 0;

    LOG_INFO("check apusys cmd valid...\n");

    /* check cmd buffer */
    if (cmdBuf == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return false;
    }

    cmdBufVa = cmdBuf->va;
    if (cmdBufVa == 0)
    {
        LOG_ERR("invalid cmdbuf addr(0x%llx)\n", cmdBufVa);
        return false;
    }

    /* check header*/
    if (getMagicNumFromCmdbuf(cmdBufVa) != APUSYS_MAGIC_NUMBER)
    {
        LOG_ERR("invalid magic number\n");
        return false;
    }

    if (getCmdIdFromCmdbuf(cmdBufVa) == 0)
    {
        LOG_ERR("invalid cmd id\n");
        return false;
    }

    if (getCmdVersionFromCmdbuf(cmdBufVa) != APUSYS_CMD_VERSION)
    {
        LOG_ERR("invalid cmd version(%d/%d)\n", getCmdVersionFromCmdbuf(cmdBufVa), APUSYS_CMD_VERSION);
        return false;
    }

    if (getPriorityFromCmdbuf(cmdBufVa) >= APUSYS_PRIORITY_MAX)
    {
        LOG_ERR("invalid priority(%d)\n", getPriorityFromCmdbuf(cmdBufVa));
        return false;
    }

    if (getNumSubcmdFromCmdbuf(cmdBufVa) == 0)
    {
        LOG_ERR("invalid num of subcmd(%d)\n", getNumSubcmdFromCmdbuf(cmdBufVa));
        return false;
    }

    LOG_INFO("check apusys cmd valid done\n");

    return true;;
}

void apusysCmd::printCmd(IApusysMem * cmdBuf)
{
    uint64_t cmdBufVa = 0;

    LOG_DEBUG("print apusys cmd valid...\n");

    /* check cmd buffer */
    if (cmdBuf == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return;
    }

    cmdBufVa = cmdBuf->va;
    if (cmdBufVa == 0)
    {
        LOG_ERR("invalid cmdbuf addr(0x%llx)\n", cmdBufVa);
        return;
    }

    /* print header*/
    LOG_DEBUG("======================================\n");
    LOG_DEBUG(" apusys command header\n");
    LOG_DEBUG("--------------------------------------\n");
    LOG_DEBUG(" magic number = 0x%llx\n", getMagicNumFromCmdbuf(cmdBufVa));
    LOG_DEBUG(" cmd id       = 0x%llx\n", getCmdIdFromCmdbuf(cmdBufVa));
    LOG_DEBUG(" cmd version  = 0x%x\n", getCmdVersionFromCmdbuf(cmdBufVa));
    LOG_DEBUG(" priority     = %d\n", getPriorityFromCmdbuf(cmdBufVa));
    LOG_DEBUG(" #sub cmd     = 0x%llx\n", getNumSubcmdFromCmdbuf(cmdBufVa));
    LOG_DEBUG("======================================\n");

    return;
}
