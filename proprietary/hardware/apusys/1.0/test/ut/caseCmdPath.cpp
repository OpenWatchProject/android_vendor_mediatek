#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
#include <random>

#include "sampleEngine.h"
#include "caseCmdPath.h"
#include "testCmn.h"
#include "apusys.h"

#define UT_SUBCMD_MAX 10

enum {
    UT_CMDPATH_DIRECTLY,
    UT_CMDPATH_SEQUENTIAL_SYNC,
    UT_CMDPATH_SEQUENTIAL_ASYNC,
    UT_CMDPATH_RANDOM_SYNC,
    UT_CMDPATH_RANDOM_ASYNC,
    UT_CMDPATH_RANDOM_PACK,

    UT_CMDPATH_MAX,
};

caseCmdPath::caseCmdPath()
{
    return;
}

caseCmdPath::~caseCmdPath()
{
    return;
}

static bool createSampleCmd(caseCmdPath * caseInst, IApusysCmd * apusysCmd, std::vector<std::vector<int>> &cmdOrder, std::vector<ISampleCmd *> &cmdList)
{
    int i = 0, j = 0;
    ISampleCmd * cmd = nullptr;
    IApusysMem * cmdBuf = nullptr;
    std::string name;

    if (caseInst == nullptr || apusysCmd == nullptr)
    {
        LOG_ERR("invalid arguments\n");
        return false;
    }

    name.assign("command test");

    cmdList.clear();
    LOG_DEBUG("sample construct cmd size(%d), list size(%d)\n", cmdOrder.size(), cmdList.size());

    for (i = 0; i < cmdOrder.size(); i++)
    {
        cmd = caseInst->getEngine()->getCmd();
        if (cmd == nullptr)
        {
            LOG_ERR("fail to get sample command(%d)\n", i);
            return false;
        }

        cmd->setup(name, i, 50);
        cmdBuf = caseInst->getEngine()->getSubCmdBuf(cmd);
        if (cmdBuf == nullptr)
        {
            LOG_ERR("fail to get sample command buffer(%d)\n", i);
            return false;
        }

        if (apusysCmd->addSubCmd(cmdBuf, APUSYS_DEVICE_SAMPLE, cmdOrder.at(i), 0, i) < 0)
        {
            LOG_ERR("fail to add command buffer to apusys command(%d)\n", i);
            return false;
        }

        cmdList.push_back(cmd);
    }

    return true;
}

static bool getDpList(std::vector<std::vector<int>> & dpList, int num , bool random)
{
    std::vector<int> instDp;
    std::random_device rd;
    std::default_random_engine gen = std::default_random_engine(rd());
    std::uniform_int_distribution<int> dis(1, num);
    int i = 0, j = 0;
    int randomNum = 0;

    dpList.clear();

    if (random == true)
    {
        /* random */
        for (i = 0; i < num ; i++)
        {
            instDp.clear();
            if(i != 0)
            {
                randomNum = dis(gen) % i;
                LOG_DEBUG("randomNum = %d/%d\n", i, randomNum);
                for (j = i-1; j >= i-randomNum; j--)
                {
                    instDp.push_back(j);
                }
            }

            dpList.push_back(instDp);
        }
    }
    else
    {
        /* sequentail dependency list */
        for (i = 0; i < num ; i++)
        {
            instDp.clear();
            if (i != 0)
            {
                instDp.push_back(i-1);
            }

            dpList.push_back(instDp);
        }
    }

    LOG_INFO("APUSYS CMD Dependency(%d): [ ", num);
    /* print dependency */
    for (i = 0; i < dpList.size(); i++)
    {
        LOG_CON(" {");
        for (j = 0; j < dpList.at(i).size(); j++)
        {
            LOG_CON("%d ", dpList.at(i).at(j));
        }
        LOG_CON("}");
    }
    LOG_CON(" ]\n");

    return true;
}

static bool deleteSampleCmd(caseCmdPath * caseInst, std::vector<ISampleCmd *> &cmdList)
{
    int i = 0;
    int size = cmdList.size();
    ISampleCmd * cmd = nullptr;

    for (i = 0; i < size; i++)
    {
        cmd = cmdList.back();
        cmdList.pop_back();

        if (cmd->setNotDirty() == false)
        {
            LOG_ERR("#%d subcmd wasn't executed\n", size-i-1);
            return false;
        }

        if (caseInst->getEngine()->releaseCmd(cmd) == false)
        {
            LOG_ERR("release sample command list(%d/%d) fail\n", i, size);
            return false;
        }
    }

    return true;
}

static bool syncRun(caseCmdPath * caseInst, uint32_t loopNum, bool random)
{
    std::vector<std::vector<int>> dpList;
    IApusysEngine * apusysEngine = nullptr;
    IApusysCmd * apusysCmd = nullptr;
    std::vector<ISampleCmd *> cmdList;
    int num = 0, i = 0;
    bool ret = true;

    std::random_device rd;
    std::default_random_engine gen = std::default_random_engine(rd());
    std::uniform_int_distribution<int> dis(1, UT_SUBCMD_MAX);

    apusysEngine = IApusysEngine::createInstance("syncRun");
    if (apusysEngine == nullptr)
    {
        LOG_ERR("create apusys engine instance fail\n");
        return false;
    }

    for (i = 0; i < loopNum; i++)
    {
        apusysCmd = apusysEngine->initCmd();
        if (apusysCmd == nullptr)
        {
            LOG_ERR("init apusys command fail\n");
            ret = false;
            goto out;
        }

        dpList.clear();
        num = dis(gen);

        if (getDpList(dpList, num, random) == false)
        {
            LOG_ERR("get dependency fail, num(%d), round(%d/%d)\n", num, i, loopNum);
            ret = false;
            goto out;
        }

        if (createSampleCmd(caseInst, apusysCmd, dpList, cmdList) == false)
        {
            LOG_ERR("fail to create sample command list(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        if (apusysEngine->runCmd(apusysCmd) == false)
        {
            LOG_ERR("run command list fail(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        if (deleteSampleCmd(caseInst, cmdList) == false)
        {
            LOG_ERR("fail to delete sample command list(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        if (apusysEngine->destroyCmd(apusysCmd) == false)
        {
            LOG_ERR("delete apusys command fail(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }
    }

out:
    if (IApusysEngine::deleteInstance(apusysEngine) == false)
    {
        LOG_ERR("delete instance fail\n");
        ret = false;
    }

    return ret;
}

static bool asyncRun(caseCmdPath * caseInst, uint32_t loopNum, bool random)
{
    std::vector<std::vector<int>> dpList;
    IApusysEngine * apusysEngine = nullptr;
    IApusysCmd * apusysCmd = nullptr;
    std::vector<ISampleCmd *> cmdList;
    int num = 0, i = 0;
    bool ret = true;

    std::random_device rd;
    std::default_random_engine gen = std::default_random_engine(rd());
    std::uniform_int_distribution<int> dis(1, UT_SUBCMD_MAX);

    apusysEngine = IApusysEngine::createInstance("asyncRun");
    if (apusysEngine == nullptr)
    {
        LOG_ERR("create apusys engine instance fail\n");
        return false;
    }

    for (i = 0; i < loopNum; i++)
    {
        apusysCmd = apusysEngine->initCmd();
        if (apusysCmd == nullptr)
        {
            LOG_ERR("init apusys command fail\n");
            ret = false;
            goto out;
        }

        dpList.clear();
        num = dis(gen);

        if (getDpList(dpList, num, random) == false)
        {
            LOG_ERR("get dependency fail, num(%d), round(%d/%d)\n", num, i, loopNum);
            ret = false;
            goto out;
        }

        if (createSampleCmd(caseInst, apusysCmd, dpList, cmdList) == false)
        {
            LOG_ERR("fail to create sample command list(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        if (apusysEngine->runCmdAsync(apusysCmd) == false)
        {
            LOG_ERR("run command async list fail(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        if (apusysEngine->waitCmd(apusysCmd) == false)
        {
            LOG_ERR("wait cmd fail(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        if (deleteSampleCmd(caseInst, cmdList) == false)
        {
            LOG_ERR("fail to delete sample command list(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        if (apusysEngine->destroyCmd(apusysCmd) == false)
        {
            LOG_ERR("delete apusys command fail(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }
    }

out:
    if (IApusysEngine::deleteInstance(apusysEngine) == false)
    {
        LOG_ERR("delete instance fail\n");
        ret = false;
    }

    return ret;
}

static bool directlyRun(caseCmdPath * caseInst, uint32_t loopNum)
{
    ISampleCmd * cmd = nullptr;
    std::string name;
    char nameChar[32];
    int i = 0;

    for (i=0; i<loopNum; i++)
    {
        cmd = caseInst->getEngine()->getCmd();
        if (cmd == nullptr)
        {
           LOG_ERR("caseCmdPath-directlyRun: cmd path get cmd fail\n");
           return false;
        }

        name.assign("caseCmdPath:directlyRun");

        if (cmd->setup(name, 0x9453, 0) == false)
        {
           goto runFalse;
        }

        if (cmd->setProp(APUSYS_PRIORITY_NORMAL, 500) == false)
        {
           goto runFalse;
        }

        if (caseInst->getEngine()->runSync(cmd) == false)
        {
           goto runFalse;
        }

        if (cmd->setNotDirty() == false)
        {
            LOG_ERR("caseCmdPath-directlyRun: driver done's bit is not set\n");
            goto runFalse;
        }

        if (caseInst->getEngine()->releaseCmd(cmd) == false)
        {
            goto runFalse;
        }
    }

    return true;

runFalse:
    if (caseInst->getEngine()->releaseCmd(cmd) == false)
    {
        LOG_ERR("caseCmdPath-directlyRun: release cmd fail\n");
    }
    return false;
}

static bool packRun(caseCmdPath * caseInst, uint32_t loopNum)
{
    std::vector<std::vector<int>> dpList;
    IApusysEngine * apusysEngine = nullptr;
    IApusysCmd * apusysCmd = nullptr;
    std::vector<ISampleCmd *> cmdList;
    std::vector<ISampleCmd *> packCmdList;
    ISampleCmd * packCmd = nullptr;
    IApusysMem * packCmdBuf = nullptr;
    int num = 0, i = 0, j = 0, packNum = 0, packTarget = 0;
    bool ret = true;
    std::string name;

    std::random_device rd;
    std::default_random_engine gen = std::default_random_engine(rd());
    std::uniform_int_distribution<int> dis(1, UT_SUBCMD_MAX);

    name.assign("pack command");

    apusysEngine = IApusysEngine::createInstance("asyncRun");
    if (apusysEngine == nullptr)
    {
        LOG_ERR("create apusys engine instance fail\n");
        return false;
    }

    for (i = 0; i < loopNum; i++)
    {
        apusysCmd = apusysEngine->initCmd();
        if (apusysCmd == nullptr)
        {
            LOG_ERR("init apusys command fail\n");
            ret = false;
            goto out;
        }

        dpList.clear();
        num = dis(gen);

        if (getDpList(dpList, num, true) == false)
        {
            LOG_ERR("get dependency fail, num(%d), round(%d/%d)\n", num, i, loopNum);
            ret = false;
            goto out;
        }

        if (createSampleCmd(caseInst, apusysCmd, dpList, cmdList) == false)
        {
            LOG_ERR("fail to create sample command list(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        /* packing... */
        packCmdList.clear();

        packNum = dis(gen) % num;
        LOG_ERR("pack command num = %d\n", packNum);
        for (j = 0; j < packNum; j++)
        {
            packCmd = caseInst->getEngine()->getCmd();
            if (packCmd == nullptr)
            {
                LOG_ERR("fail to get sample command for packing(%d/%d/%d)\n", j, i, loopNum);
                return false;
            }

            packCmdList.push_back(packCmd);

            packCmd->setup(name, i+j, 100);
            packCmdBuf = caseInst->getEngine()->getSubCmdBuf(packCmd);
            if (packCmdBuf == nullptr)
            {
                LOG_ERR("fail to get sample command buffer(%d)\n", i);
                return false;
            }

            packTarget = dis(gen) % num;
            LOG_INFO("pack command to %d subCmd\n", packTarget);
            if (apusysCmd->packSubCmd(packCmdBuf, APUSYS_DEVICE_SAMPLE, packTarget) == false)
            {
                LOG_ERR("pack command fail(%d)\n", packTarget);
                return false;
            }
        }

        if (apusysEngine->runCmd(apusysCmd) == false)
        {
            LOG_ERR("run command async list fail(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        /* release packing cmd */
        for (j = packNum-1; j >= 0; j--)
        {
            packCmd = packCmdList.back();
            packCmdList.pop_back();

            if (packCmd->setNotDirty() == false)
            {
                LOG_ERR("packing subCmd(%d) is not set\n", j);
                return false;
            }

            if (caseInst->getEngine()->releaseCmd(packCmd) == false)
            {
                LOG_ERR("release pack command list(%d/%d) fail\n", j, packNum);
                return false;
            }
        }

        if (deleteSampleCmd(caseInst, cmdList) == false)
        {
            LOG_ERR("fail to delete sample command list(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }

        if (apusysEngine->destroyCmd(apusysCmd) == false)
        {
            LOG_ERR("delete apusys command fail(%d/%d)\n", i, loopNum);
            ret = false;
            goto out;
        }
    }

out:
    if (IApusysEngine::deleteInstance(apusysEngine) == false)
    {
        LOG_ERR("delete instance fail\n");
        ret = false;
    }

    return ret;
}

bool caseCmdPath::runCase(uint32_t subCaseIdx, uint32_t loopNum)
{
    if(loopNum == 0)
    {
        LOG_ERR("caseCmdPath: loop number(%d), do nothing\n", loopNum);
        return false;
    }

    switch(subCaseIdx)
    {
        case UT_CMDPATH_DIRECTLY:
            return directlyRun(this, loopNum);
            break;

        case UT_CMDPATH_SEQUENTIAL_SYNC:
            return syncRun(this, loopNum, false);
            break;

        case UT_CMDPATH_SEQUENTIAL_ASYNC:
            return asyncRun(this, loopNum, false);
            break;

        case UT_CMDPATH_RANDOM_SYNC:
            return syncRun(this, loopNum, true);
            break;

        case UT_CMDPATH_RANDOM_ASYNC:
            return asyncRun(this, loopNum, true);
            break;

        case UT_CMDPATH_RANDOM_PACK:
            return packRun(this, loopNum);
            break;

        default:
            LOG_ERR("caseCmdPath: no subtest(%d)\n", subCaseIdx);
    }

    return false;
}

void caseCmdPathInfo(void)
{
    LOG_CON("   <testCase> = %d : command test\n", APUSYS_UT_CMDPATH);
    LOG_CON("     <subtestIdx> = %-2d : directly run\n", UT_CMDPATH_DIRECTLY);
    LOG_CON("     <subtestIdx> = %-2d : sequential sync run\n", UT_CMDPATH_SEQUENTIAL_SYNC);
    LOG_CON("     <subtestIdx> = %-2d : sequential async run\n", UT_CMDPATH_SEQUENTIAL_ASYNC);
    LOG_CON("     <subtestIdx> = %-2d : random sync run\n", UT_CMDPATH_RANDOM_SYNC);
    LOG_CON("     <subtestIdx> = %-2d : random async run\n", UT_CMDPATH_RANDOM_ASYNC);
    LOG_CON("     <subtestIdx> = %-2d : random packing run\n", UT_CMDPATH_RANDOM_PACK);

    return;
}

