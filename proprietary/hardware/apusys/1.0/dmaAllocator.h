#pragma once
#include <vector>

#define PAGE_ALIGN (4096)

class apusysDmaMem : public IApusysMem{
public:
    int mMemFd;

};

class dmaAllocator : public IMemAllocator {
private:
    std::vector<IApusysMem *> mMemList;

    int mDevFd;

public:
    dmaAllocator(int devFd);
    ~dmaAllocator();

    IApusysMem * alloc(uint32_t size, uint32_t align);
    IApusysMem * alloc(uint32_t size);
    bool free(IApusysMem * mem);
    bool flush(IApusysMem * mem);
    bool invalidate(IApusysMem * mem);
    IApusysMem * import(int shareFd, uint32_t size);
};
