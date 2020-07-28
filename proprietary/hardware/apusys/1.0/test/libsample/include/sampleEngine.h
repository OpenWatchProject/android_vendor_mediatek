#pragma once

#include <cstring>
#include <vector>
#include "apusysEngine.h"


class ISampleCmd {
public:
    virtual ~ISampleCmd(){};

    virtual bool setup(std::string & name, uint32_t algoId, uint32_t delayMs) = 0;
    virtual bool setProp(uint8_t priority, uint32_t targetMs) = 0;
    virtual bool setNotDirty() = 0;
};

class ISampleEngine : public apusysEngine {
public:
    ISampleEngine(const char * userName);
    virtual ~ISampleEngine(){};

    /* for normal user trigger directly */
    virtual ISampleCmd * getCmd() = 0;
    virtual bool releaseCmd(ISampleCmd * ICmd) = 0;
    virtual bool runSync(ISampleCmd * ICmd) = 0;

    /* for user who want to contruct big apusys cmd */
    virtual IApusysMem * getSubCmdBuf(ISampleCmd * ICmd) = 0;

    static ISampleEngine * createInstance(const char * userName);
    static bool deleteInstance(ISampleEngine * engine);
};

