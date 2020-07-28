#pragma once

#include "apusys.h"

class IMemAllocator {
private:
    int type;
public:
    virtual ~IMemAllocator(){};

    virtual IApusysMem * alloc(uint32_t size, uint32_t align, uint32_t cache) = 0;
    virtual bool free(IApusysMem * mem) = 0;
    virtual bool flush(IApusysMem * mem) = 0;
    virtual bool invalidate(IApusysMem * mem) = 0;
    virtual IApusysMem * import(int shareFd, uint32_t size) = 0;
    virtual bool unimport(IApusysMem * mem) = 0;
};
