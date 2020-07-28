#pragma once
#include <vector>

#define PAGE_ALIGN (4096)
#define NONCACHED (0)

class apusysIonMem : public IApusysMem{
public:
    int ion_share_fd;
    uint64_t ion_khandle;
    int ion_uhandle;
private:
    uint32_t ionLen;
    uint64_t ionIova; //ion's iova is 64 bit

};

class ionAllocator : public IMemAllocator {
private:
    std::vector<IApusysMem *> mMemList;

    int mDevFd;
    int mIonFd;

public:
    ionAllocator(int devFd);
    ~ionAllocator();

    IApusysMem * alloc(uint32_t size, uint32_t align, uint32_t cache);
    bool free(IApusysMem * mem);
    bool flush(IApusysMem * mem);
    bool invalidate(IApusysMem * mem);
    IApusysMem * import(int shareFd, uint32_t size);
    bool unimport(IApusysMem * mem);
};
