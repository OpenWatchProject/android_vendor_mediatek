#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <error.h>

#include "apusysMem.h"
#include "apusys_drv.h"
#include "dmaAllocator.h"
#include "apusysCmn.h"

dmaAllocator::dmaAllocator(int devFd)
{

    /* check apusys fd */
    if (devFd <= 0)
    {
        LOG_ERR("invalid dev fd(%d)\n", devFd);
        assert (devFd >= 0);
    }

    mDevFd = devFd;

    LOG_DEBUG("Done\n");

    return;
}

dmaAllocator::~dmaAllocator()
{

    LOG_DEBUG("Done\n");
    return;
}

IApusysMem * dmaAllocator::alloc(uint32_t size)
{
    return this->alloc(size, PAGE_ALIGN);
}
IApusysMem * dmaAllocator::alloc(uint32_t size, uint32_t align)
{
    struct apusys_mem ioctlMem;
    int ret = 0;
    uint64_t uva;
    apusysDmaMem * dmaMem = nullptr;



    ioctlMem.kva = 0;
    ioctlMem.iova = 0;
    ioctlMem.size = size;
    ioctlMem.mem_type = APUSYS_MEM_DRAM_DMA;

    ret = ioctl (mDevFd, APUSYS_IOCTL_MEM_ALLOC, &ioctlMem);
    if (ret)
    {
        LOG_ERR("ioctl mem alloc fail(%d)\n", ret);
        return NULL;
    }
    uva = (uint64_t) mmap (NULL, ioctlMem.size, PROT_READ | PROT_WRITE,
                           MAP_SHARED, mDevFd, (size_t) ioctlMem.iova);

    if ((void*) uva == MAP_FAILED)
    {
        LOG_ERR("mmap() fail, error: %d, %s\n", errno, strerror (errno));
        goto release_mem;
    }

    dmaMem = new apusysDmaMem;
    if (dmaMem == nullptr)
    {
        LOG_ERR("allocate apusys dma mem struct fail\n");
        goto unmap_mem;
    }

    dmaMem->va = uva;
    dmaMem->kva = ioctlMem.kva;
    dmaMem->iova = ioctlMem.iova;
    dmaMem->offset = 0;
    dmaMem->size = ioctlMem.size;
    LOG_DEBUG("alloc dmaMem(0x%x/0x%llx/0x%llx/%d)\n", dmaMem->iova, dmaMem->va,
              dmaMem->kva, dmaMem->size);

    mMemList.push_back (dmaMem);

    return (IApusysMem *) dmaMem;

unmap_mem:
    ret = munmap ((void*) uva, (size_t) ioctlMem.size);
    if (ret)
    {
        LOG_ERR("munmap fail(%d)\n", ret);
    }

release_mem:
    ret = ioctl (mDevFd, APUSYS_IOCTL_MEM_FREE, &ioctlMem);
    if (ret)
    {
        LOG_ERR("ioctl mem free fail(%d)\n", ret);
    }
    return NULL;
}

bool dmaAllocator::free(IApusysMem * mem)
{
    apusysDmaMem * dmaMem = nullptr;
    struct apusys_mem ioctlMem;
    int ret = 0;
    auto iter = mMemList.begin();
    apusysDmaMem * dmaMemIt;
    bool find = false;

    dmaMem = (apusysDmaMem *) mem;

    /* delete from mem list */
    for (iter = mMemList.begin(); iter != mMemList.end(); iter++)
    {
        if (dmaMem == *iter)
        {
            mMemList.erase(iter);
            break;
        }
    }
    if (dmaMem != *iter)
    {
        LOG_ERR("find dmaMem from mem list fail(%p)\n", dmaMem);
    }

    ioctlMem.kva = dmaMem->kva;
    ioctlMem.iova = dmaMem->iova;
    ioctlMem.size = dmaMem->size;
    ioctlMem.mem_type = APUSYS_MEM_DRAM_DMA;

    ret = munmap ((void*) dmaMem->va, (size_t) dmaMem->size);
    if (ret)
    {
        LOG_ERR("munmap fail(%d)\n", ret);
        return false;
    }

    ret = ioctl (mDevFd, APUSYS_IOCTL_MEM_FREE, &ioctlMem);
    if (ret)
    {
        LOG_ERR("ioctl mem alloc fail(%d)\n", ret);
        return false;
    }

    LOG_DEBUG("free dmaMem(0x%x/0x%llx/0x%llx/%d)\n", dmaMem->iova, dmaMem->va,
              dmaMem->kva, dmaMem->size);
    delete dmaMem;

    return true;

}

bool dmaAllocator::flush(IApusysMem * mem)
{
    apusysDmaMem * dmaMem = nullptr;

    dmaMem = (apusysDmaMem *) mem;
    LOG_DEBUG("dmaMem(0x%x/0x%llx/0x%llx/%d)\n", dmaMem->iova, dmaMem->va,
            dmaMem->kva, dmaMem->size);
    msync ((void*) dmaMem->va, (size_t) dmaMem->size, MS_INVALIDATE | MS_SYNC);
    LOG_DEBUG("Done\n");
    return true;
}

bool dmaAllocator::invalidate(IApusysMem * mem)
{
    apusysDmaMem * dmaMem = nullptr;

    dmaMem = (apusysDmaMem *) mem;
    msync ((void*) dmaMem->va, (size_t) dmaMem->size, MS_INVALIDATE);
    LOG_DEBUG("Done\n");
    return true;
}

IApusysMem * dmaAllocator::import(int shareFd, uint32_t size)
{
    LOG_ERR("DMA Not Support Import!\n");
    return nullptr;
}
