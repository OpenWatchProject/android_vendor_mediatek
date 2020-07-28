#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include <ion/ion.h> //system/core/libion/include/ion.h
#include <linux/ion_drv.h>
#include <ion.h> //libion_mtk/include/ion.h
#include <mt_iommu_port.h>
#include <sys/mman.h>

#include "apusysMem.h"
#include "apusys_drv.h"
#include "ionAllocator.h"
#include "apusysCmn.h"

ionAllocator::ionAllocator(int devFd)
{
    /* check apusys fd */
    if (devFd <= 0)
    {
        LOG_ERR("invalid dev fd(%d)\n", devFd);
        assert(devFd >= 0);
    }

    /* allocate ion fd */
    mIonFd = mt_ion_open(__FILE__);
    if (mIonFd < 0)
    {
        LOG_ERR("create ion fd fail\n");
        assert(mIonFd >= 0);
    }

    mMemList.clear();
    mDevFd = devFd;

    LOG_DEBUG("ion allocator init done\n");

    return;
}

ionAllocator::~ionAllocator()
{
    ion_close(mIonFd);

    return;
}

IApusysMem * ionAllocator::alloc(uint32_t size, uint32_t align, uint32_t cache)
{
    struct apusys_mem ioctlMem;
    apusysIonMem * ionMem = nullptr;
    ion_user_handle_t bufHandle;
    int shareFd = 0;
    int ret = 0;
    uint64_t uva;
    struct ion_mm_data mmData;
    struct ion_sys_data sysData;

    DEBUG_TAG;

    if((align > PAGE_ALIGN) || ( PAGE_ALIGN % align !=0 ))
    {
        LOG_ERR("fail to get ion buffer align must be %d Not %d\n", PAGE_ALIGN, align);
        return nullptr;
    }


    if(cache > 0)
    {
        /* alloc ion memory cached */
        if(ion_alloc_mm(mIonFd, size, PAGE_ALIGN, 3, &bufHandle))
        {
            LOG_ERR("fail to get ion buffer, (devFd=%d, buf_handle = %d, len=%d)\n", mIonFd, bufHandle, size);
            return nullptr;
        }
    } else
    {
        /* alloc ion memory non-cached*/
        if(ion_alloc_mm(mIonFd, size, PAGE_ALIGN, 0, &bufHandle))
        {
            LOG_ERR("fail to get ion buffer, (devFd=%d, buf_handle = %d, len=%d)\n", mIonFd, bufHandle, size);
            return nullptr;
        }
    }


    /* set ion memory shared */
    if(ion_share(mIonFd, bufHandle, &shareFd))
    {
        LOG_ERR("fail to get ion buffer handle (devFd=%d, shared_fd = %d, len=%d)\n", mIonFd, shareFd, size);
        goto shareFdFail;
    }

    /* map user va */
    uva = (uint64_t)ion_mmap(mIonFd, NULL, (size_t)size, PROT_READ|PROT_WRITE, MAP_SHARED, shareFd, 0);
    if (!uva)
    {
        LOG_ERR("get uva failed.\n");
        goto mmapFail;
    }

    /* config buffer */
    memset(&mmData, 0, sizeof(mmData));
    mmData.mm_cmd = ION_MM_CONFIG_BUFFER;
    mmData.config_buffer_param.handle      = bufHandle;
    mmData.config_buffer_param.eModuleID   = M4U_PORT_L21_APU_FAKE_DATA;
    mmData.config_buffer_param.security    = 0;
    mmData.config_buffer_param.coherent    = 0;
    if(ion_custom_ioctl(mIonFd, ION_CMD_MULTIMEDIA, &mmData))
    {
        LOG_ERR("ion_config_buffer: ion_custom_ioctl ION_CMD_MULTIMEDIA Config Buffer failed\n");
        goto out;
    }

    /* query mva */
    memset(&sysData, 0, sizeof(sysData));
    sysData.sys_cmd = ION_SYS_GET_PHYS;
    sysData.get_phys_param.handle = bufHandle;
    if(ion_custom_ioctl(mIonFd, ION_CMD_SYSTEM, &sysData))
    {
        LOG_ERR("ion_config_buffer: ion_custom_ioctl ION_CMD_SYSTEM query buffer mva failed\n");
        goto out;
    }

    memset(&ioctlMem, 0, sizeof(struct apusys_mem));
    /* get kva from kernel */
    ioctlMem.kva = 0;
    ioctlMem.iova = sysData.get_phys_param.phy_addr;
    ioctlMem.size = size;
    ioctlMem.mem_type = APUSYS_MEM_DRAM_ION;
    ioctlMem.align = align;
    ioctlMem.ion_data.ion_share_fd = shareFd;
    ioctlMem.ctl_data.cmd = APUSYS_MAP;
    ioctlMem.ctl_data.map_param.map_type = APUSYS_MAP_KVA;


    LOG_DEBUG("ioctlMem(%d/%d/0x%x/0x%llx/0x%llx/%d)\n",mDevFd, mIonFd, ioctlMem.iova, ioctlMem.uva, ioctlMem.kva, ioctlMem.size);

    ret = ioctl(mDevFd, APUSYS_IOCTL_MEM_CTL, &ioctlMem);
    if(ret)
    {
        LOG_ERR("ioctl mem alloc fail(%d)\n", ret);
        goto out;
    }

    /* allocate apusysMem */
    ionMem = new apusysIonMem;
    if (ionMem == nullptr)
    {
        LOG_ERR("allocate apusys ion mem struct fail\n");
        goto out;
    }

    ionMem->va = uva;
    ionMem->kva = ioctlMem.kva;
    ionMem->iova = ioctlMem.iova;
    ionMem->offset = 0;
    ionMem->size = ioctlMem.size;
    ionMem->ion_share_fd = shareFd;
    ionMem->ion_khandle = ioctlMem.ion_data.ion_khandle;
    ionMem->ion_uhandle = bufHandle;


    LOG_DEBUG("ionMem(0x%x/0x%llx/0x%llx/%d)\n",ionMem->iova, ionMem->va, ionMem->kva, ionMem->size);
    LOG_DEBUG("ionMem ion handle(%d/%d/%d)\n",ionMem->ion_share_fd, ionMem->ion_khandle, ionMem->ion_uhandle);

    mMemList.push_back(ionMem);

    return (IApusysMem *)ionMem;
out:
    if(ion_munmap(mIonFd, (void*)uva, size))
    {
        LOG_ERR("ion unmap fail\n");
    }
mmapFail:
    if(ion_share_close(mIonFd, shareFd))
    {
        LOG_ERR("ion close share fail\n");
    }
shareFdFail:
    if(ion_free(mIonFd, bufHandle))
    {
        LOG_ERR("ion free fail, fd = %d, len = %d\n", mIonFd, size);
    }
    return nullptr;
}

bool ionAllocator::free(IApusysMem * mem)
{
    apusysIonMem * ionMem = (apusysIonMem *)mem;
    ion_user_handle_t bufHandle;
    struct apusys_mem ioctlMem;
    int ret = 0;
    std::vector<IApusysMem *>::iterator iter;

    /* check argument */
    if (mem == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return false;
    }

    /* delete from mem list */
    for (iter = mMemList.begin(); iter != mMemList.end(); iter++)
    {
        if (ionMem == *iter)
        {
            mMemList.erase(iter);
            break;
        }
    }
    if (ionMem != *iter)
    {
        LOG_ERR("find ionMem from mem list fail(%p)\n", ionMem);
    }

    memset(&ioctlMem, 0, sizeof(struct apusys_mem));
    /* get kva from kernel */
    ioctlMem.kva = ionMem->kva;
    ioctlMem.iova = ionMem->iova;
    ioctlMem.size = ionMem->size;
    ioctlMem.align = ionMem->align;
    ioctlMem.mem_type = APUSYS_MEM_DRAM_ION;
    ioctlMem.ion_data.ion_khandle = ionMem->ion_khandle;
    ioctlMem.ion_data.ion_share_fd = ionMem->ion_share_fd;
    ioctlMem.ctl_data.cmd = APUSYS_MAP;
    ioctlMem.ctl_data.map_param.map_type = APUSYS_UNMAP_KVA;

    LOG_DEBUG("ionMem(%d/%d/0x%x/0x%llx/0x%llx/%d)\n",mDevFd, mIonFd, ioctlMem.iova, ioctlMem.uva, ioctlMem.kva, ioctlMem.size);
    ret = ioctl(mDevFd, APUSYS_IOCTL_MEM_CTL, &ioctlMem);
    if(ret)
    {
        LOG_ERR("ioctl mem free fail(%d)\n", ret);
    }

    /* free */
    if(ion_munmap(mIonFd, (void*)ionMem->va, ionMem->size))
    {
        LOG_ERR("ion unmap fail\n");
        return false;
    }
    if(ion_share_close(mIonFd, ionMem->ion_share_fd))
    {
        LOG_ERR("ion close share fail\n");
        return false;
    }
    if(ion_free(mIonFd, ionMem->ion_uhandle))
    {
        LOG_ERR("ion free fail, fd = %d, len = %d\n", mIonFd, ionMem->size);
        return false;
    }

    delete ionMem;
    DEBUG_TAG;

    return true;
}

bool ionAllocator::flush(IApusysMem * mem)
{

    apusysIonMem * ionMem = (apusysIonMem *)mem;
    struct ion_sys_data sys_data;

    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = ionMem->ion_uhandle;
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_BY_RANGE;
    sys_data.cache_sync_param.va = (void*) ionMem->va;
    sys_data.cache_sync_param.size = ionMem->size;

    if (ion_custom_ioctl(mIonFd, ION_CMD_SYSTEM, &sys_data)) {
        LOG_ERR("ion flush fail\n");
        return false;
    }

    return true;
}

bool ionAllocator::invalidate(IApusysMem * mem)
{
    apusysIonMem * ionMem = (apusysIonMem *)mem;
    struct ion_sys_data sys_data;


    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = ionMem->ion_uhandle;
    sys_data.cache_sync_param.sync_type = ION_CACHE_INVALID_BY_RANGE;
    sys_data.cache_sync_param.va = (void*) ionMem->va;
    sys_data.cache_sync_param.size = ionMem->size;

    if (ion_custom_ioctl(mIonFd, ION_CMD_SYSTEM, &sys_data)) {
        LOG_ERR("ion invalidate fail\n");
        return false;
    }

    return true;
}

IApusysMem * ionAllocator::import(int shareFd, uint32_t size)
{
    struct apusys_mem ioctlMem;
    apusysIonMem * ionMem = nullptr;
    ion_user_handle_t bufHandle;
    int ret = 0;
    uint64_t uva;
    struct ion_mm_data mmData;
    struct ion_sys_data sysData;

    DEBUG_TAG;

    /* set ion memory shared */
    if(ion_import(mIonFd, shareFd, &bufHandle))
    {
        LOG_ERR("fail to get ion buffer by import (devFd=%d, shared_fd = %d)\n", mIonFd, shareFd);
        goto importFdFail;
    }

    /* map user va */
    uva = (uint64_t)ion_mmap(mIonFd, NULL, (size_t)size, PROT_READ|PROT_WRITE, MAP_SHARED, shareFd, 0);
    if (!uva)
    {
        LOG_ERR("get uva failed.\n");
        goto mmapFail;
    }

    /* config buffer */
    memset(&mmData, 0, sizeof(mmData));
    mmData.mm_cmd = ION_MM_CONFIG_BUFFER;
    mmData.config_buffer_param.handle      = bufHandle;
    mmData.config_buffer_param.eModuleID   = M4U_PORT_L21_APU_FAKE_DATA;
    mmData.config_buffer_param.security    = 0;
    mmData.config_buffer_param.coherent    = 0;
    if(ion_custom_ioctl(mIonFd, ION_CMD_MULTIMEDIA, &mmData))
    {
        LOG_ERR("ion_config_buffer: ion_custom_ioctl ION_CMD_MULTIMEDIA Config Buffer failed\n");
        goto out;
    }

    /* query mva */
    memset(&sysData, 0, sizeof(sysData));
    sysData.sys_cmd = ION_SYS_GET_PHYS;
    sysData.get_phys_param.handle = bufHandle;
    if(ion_custom_ioctl(mIonFd, ION_CMD_SYSTEM, &sysData))
    {
        LOG_ERR("ion_config_buffer: ion_custom_ioctl ION_CMD_SYSTEM query buffer mva failed\n");
        goto out;
    }

    memset(&ioctlMem, 0, sizeof(struct apusys_mem));
    /* get kva from kernel */
    ioctlMem.kva = 0;
    ioctlMem.iova = sysData.get_phys_param.phy_addr;
    ioctlMem.size = size;
    ioctlMem.mem_type = APUSYS_MEM_DRAM_ION;
    ioctlMem.align = 0;
    ioctlMem.ion_data.ion_share_fd = shareFd;
    ioctlMem.ctl_data.cmd = APUSYS_MAP;
    ioctlMem.ctl_data.map_param.map_type = APUSYS_MAP_KVA;


    LOG_DEBUG("ioctlMem(%d/%d/0x%x/0x%llx/0x%llx/%d)\n",mDevFd, mIonFd, ioctlMem.iova, ioctlMem.uva, ioctlMem.kva, ioctlMem.size);

    ret = ioctl(mDevFd, APUSYS_IOCTL_MEM_CTL, &ioctlMem);
    if(ret)
    {
        LOG_ERR("ioctl mem alloc fail(%d)\n", ret);
        goto out;
    }

    /* allocate apusysMem */
    ionMem = new apusysIonMem;
    if (ionMem == nullptr)
    {
        LOG_ERR("allocate apusys ion mem struct fail\n");
        goto out;
    }

    ionMem->va = uva;
    ionMem->kva = ioctlMem.kva;
    ionMem->iova = ioctlMem.iova;
    ionMem->offset = 0;
    ionMem->size = ioctlMem.size;
    ionMem->ion_share_fd = shareFd;
    ionMem->ion_khandle = ioctlMem.ion_data.ion_khandle;
    ionMem->ion_uhandle = bufHandle;


    LOG_DEBUG("ionMem(0x%x/0x%llx/0x%llx/%d)\n",ionMem->iova, ionMem->va, ionMem->kva, ionMem->size);

    mMemList.push_back(ionMem);

    return (IApusysMem *)ionMem;
out:
    if(ion_munmap(mIonFd, (void*)uva, size))
    {
        LOG_ERR("ion unmap fail\n");
    }
mmapFail:
    if(ion_free(mIonFd, bufHandle))
    {
        LOG_ERR("ion free fail, fd = %d, len = %d\n", mIonFd, size);
    }

importFdFail:
    return nullptr;
}

bool ionAllocator::unimport(IApusysMem * mem)
{
    apusysIonMem * ionMem = (apusysIonMem *)mem;
    ion_user_handle_t bufHandle;
    struct apusys_mem ioctlMem;
    int ret = 0;
    std::vector<IApusysMem *>::iterator iter;

    /* check argument */
    if (mem == nullptr)
    {
        LOG_ERR("invalid argument\n");
        return false;
    }

    /* delete from mem list */
    for (iter = mMemList.begin(); iter != mMemList.end(); iter++)
    {
        if (ionMem == *iter)
        {
            mMemList.erase(iter);
            break;
        }
    }
    if (ionMem != *iter)
    {
        LOG_ERR("find ionMem from mem list fail(%p)\n", ionMem);
    }

    memset(&ioctlMem, 0, sizeof(struct apusys_mem));
    /* get kva from kernel */
    ioctlMem.kva = ionMem->kva;
    ioctlMem.iova = ionMem->iova;
    ioctlMem.size = ionMem->size;
    ioctlMem.align = ionMem->align;
    ioctlMem.mem_type = APUSYS_MEM_DRAM_ION;
    ioctlMem.ion_data.ion_khandle = ionMem->ion_khandle;
    ioctlMem.ion_data.ion_share_fd = ionMem->ion_share_fd;
    ioctlMem.ctl_data.cmd = APUSYS_MAP;
    ioctlMem.ctl_data.map_param.map_type = APUSYS_UNMAP_KVA;

    LOG_DEBUG("ionMem(%d/%d/0x%x/0x%llx/0x%llx/%d)\n",mDevFd, mIonFd, ioctlMem.iova, ioctlMem.uva, ioctlMem.kva, ioctlMem.size);
    ret = ioctl(mDevFd, APUSYS_IOCTL_MEM_CTL, &ioctlMem);
    if(ret)
    {
        LOG_ERR("ioctl mem free fail(%d)\n", ret);
    }

    /* free */
    if(ion_munmap(mIonFd, (void*)ionMem->va, ionMem->size))
    {
        LOG_ERR("ion unmap fail\n");
        return false;
    }
    if(ion_free(mIonFd, ionMem->ion_uhandle))
    {
        LOG_ERR("ion free fail, fd = %d, len = %d\n", mIonFd, ionMem->size);
        return false;
    }

    delete ionMem;
    DEBUG_TAG;

    return true;
}
