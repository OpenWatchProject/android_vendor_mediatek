#include <cstdlib>
#include <iostream>
#include <exception>
#include <ctime>
#include <sys/mman.h>

#include "testCaseCmn.h"
#include "vpu_platform.h"
#include "multiCase.h"

#define DYNAMIC_LOAD_FOLDER "/data/local/tmp/vpu_test_multicore/core"
#define DYNAMIC_BIN_NAME "vpu_test_multicore"

//#define DYNAMIC_LOAD_FOLDER "/data/local/tmp/vpuDynamicLoadAlgo/core"
//#define DYNAMIC_BIN_NAME "vpu_dynamic_algo_test"

/* for dynamic load algo */
static int getTestAlgo(const char *path, char **algoBuf, unsigned long &len)
{
    FILE *file;
    char *buf;
    unsigned int fileLen = 0;

    if(path == nullptr)
    {
        return -1;
    }

    /* open file */
    file = fopen(path, "rb");
    if(!file)
    {
        MY_LOGE("open %s failed\n", path);
        return -1;
    }

    /* get file length */
    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* allocate buffer */
    buf = (char *)malloc(fileLen + 1);
    if(buf == 0)
    {
        MY_LOGE("[create] alloc memory for algo failed\n");
        fclose(file);
        return -1;
    }

    fread(buf, fileLen, 1, file);

    *algoBuf = buf;
    len = fileLen;

    fclose(file);

    return 0;
}

//---------------------------------------------

ionAllocator::ionAllocator()
{

    mFd = mt_ion_open(__FILE__);
    mAllocList.clear();
}

ionAllocator::~ionAllocator()
{
    int i = 0;
    struct ionBuffer * tmpBuf;

    while (!mAllocList.empty())
    {
        tmpBuf = mAllocList.back();
        memFree(tmpBuf);
    }

    ion_close(mFd);
}

bool ionAllocator::checkAllocated(struct ionBuffer * handle)
{
    auto iter = std::find(mAllocList.begin(), mAllocList.end(), handle);
    if (iter == mAllocList.end())
    {
        return false;
    }
    return true;
}


struct ionBuffer * ionAllocator::memAlloc(unsigned int size)
{
    ion_user_handle_t bufHandle;
    uint64_t va;
    int shareFd;
    struct ionBuffer * ionBuf;

    if(size == 0)
    {
        return nullptr;
    }

    // allocate ion buffer handle
    if(ion_alloc_mm(mFd, (size_t)size, 0, 0, &bufHandle))        // no alignment, non-cache
    {
        MY_LOGE("fail to allocate ion buffer[%d]\n", size);
        return nullptr;
    }

    // get ion buffer share handle
    if(ion_share(mFd, bufHandle, &shareFd))
    {
        MY_LOGE("fail to get ion buffer share handle");
        goto out;
    }

    // get buffer virtual address
    va = (uint64_t)ion_mmap(mFd, NULL, (size_t)size, PROT_READ|PROT_WRITE, MAP_SHARED, shareFd, 0);
    if(va == 0)
    {
        goto out;
    }

    ionBuf = (struct ionBuffer *)malloc(sizeof(struct ionBuffer));
    if(ionBuf == nullptr)
    {
        goto out;
    }

    ionBuf->va = va;
    ionBuf->shareFd = shareFd;
    ionBuf->len = size;

    mAllocList.push_back(ionBuf);

    return ionBuf;

out:
    if(ion_free(mFd, bufHandle) != 0)
    {
        MY_LOGE("fail to free buffer when alloc error, (%d/%d/%d)", mFd, shareFd, bufHandle);
    }
    return nullptr;
}


bool ionAllocator::memFree(struct ionBuffer * handle)
{
    ion_user_handle_t bufHandle;

    if(handle == nullptr)
    {
        return false;
    }

    if(ion_import(mFd, handle->shareFd, &bufHandle))
    {
        return false;
    }

    /* for ref count-- cause by ion_import */
    if(ion_free(mFd, bufHandle) != 0)
    {
        MY_LOGW("fail to free ion buffer ref count for free(%d/%d/%d)", mFd, handle->shareFd, bufHandle);
        return false;
    }

    /* delete from list */
    auto iter = std::find(mAllocList.begin(), mAllocList.end(), handle);
    if (iter == mAllocList.end())
    {
        return false;
    }
    mAllocList.erase(iter);

    /* free ion */
    if(ion_munmap(mFd, (void *)handle->va, handle->len))
    {
        return false;
    }

    if(ion_share_close(mFd, handle->shareFd))
    {
        return false;
    }

    if(ion_free(mFd, bufHandle))
    {
        return false;
    }

    free(handle);

    return true;
}

bool ionAllocator::memSync(struct ionBuffer * handle)
{
    struct ion_sys_data sysData;
    ion_user_handle_t bufHandle;

    if(handle == nullptr)
    {
        return false;
    }

    if (checkAllocated(handle) == false)
    {
        return false;
    }

    if(ion_import(mFd, handle->shareFd, &bufHandle))
    {
        return false;
    }

    /* for ref count-- cause by ion_import */
    if(ion_free(mFd, bufHandle) != 0)
    {
        MY_LOGE("fail to free ion buffer ref count for sync(%d/%d/%d)",mFd ,handle->shareFd, bufHandle);
        return false;
    }

    sysData.sys_cmd = ION_SYS_CACHE_SYNC;
    sysData.cache_sync_param.handle = bufHandle;
    sysData.cache_sync_param.va = (void *)handle->va;
    sysData.cache_sync_param.size = handle->len;

    if(ion_custom_ioctl(mFd, ION_CMD_SYSTEM, &sysData))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------

vpuTestMultiCore::vpuTestMultiCore(int coreNum)
{
    int i = 0;
    unsigned long algoLen = 0;
    std::string algoPath;

    /* check arguments */
    if(coreNum <=0 || coreNum > MAX_VPU_CORE_NUM)
    {
    }

    mCoreNum = coreNum;

    /* create vpu stream */
    mStream = VpuStream::createInstance();
    if (mStream == nullptr)
    {
        MY_LOGE("init vpu stream fail");
    }

    /* create algo */
    mAlgoBin.resize(mCoreNum);
    mCreateAlgoList.resize(mCoreNum);
    for(i = 0; i < mCoreNum; i++)
    {
        algoPath.clear();
        algoPath.append(DYNAMIC_LOAD_FOLDER);
        algoPath.append(std::to_string(i));
        algoPath.append("/");
        algoPath.append(DYNAMIC_BIN_NAME);

        MY_LOGD("algoPath[%d] = %s", i, algoPath.c_str());
        /* get algo bin from file system */
        if(getTestAlgo(algoPath.c_str(), &mAlgoBin.at(i), algoLen))
        {
            MY_LOGE("get algo buffer[%d] failed\n", i);
        }
        /* create vpu algo to vpu user */
        mCreateAlgoList.at(i) = mStream->createAlgo(DYNAMIC_BIN_NAME, mAlgoBin.at(i), algoLen, i);
        if(mCreateAlgoList.at(i) == 0)
        {
            MY_LOGE("create algo [%s] failed\n",DYNAMIC_BIN_NAME);
        }
    }

    /* get algo */
    mAlgo = mStream->getAlgo(DYNAMIC_BIN_NAME);
    if(mAlgo == nullptr)
    {
        MY_LOGE("fail to get algo[%s]", DYNAMIC_BIN_NAME);
    }

}

vpuTestMultiCore::~vpuTestMultiCore()
{
    int i = 0;
    VpuCreateAlgo tmpCreateAlgo;
    char * tmpBin;

    /* release algo created */
    while (!mCreateAlgoList.empty())
    {
        tmpCreateAlgo = mCreateAlgoList.back();
        mCreateAlgoList.pop_back();
        if(mStream->freeAlgo(tmpCreateAlgo) != true)
        {
            MY_LOGE("free algo fail");
        }
    }

    /* free algo bin */
    while (!mAlgoBin.empty())
    {
        tmpBin = mAlgoBin.back();
        mAlgoBin.pop_back();
        free(tmpBin);
    }

    delete mStream;
}

bool vpuTestMultiCore::execute(int loops, unsigned int algoDelayMs)
{
    int i = 0, j = 0;
    unsigned long totalIdx = 0x0;
    unsigned long randomPattern = 0x0;
    VpuBuffer syncBuffer;
    struct multicoreProperty prop;
    struct ionBuffer * buf;
    std::vector<VpuRequest *> requestList;
    bool ret = false;

    srand(time(NULL));
    randomPattern = rand() + (unsigned int )gettid();

    /* acquire request from algo */
    requestList.resize(mCoreNum);
    for(i=0;i<mCoreNum;i++)
    {
        requestList.at(i) = mStream->acquire(mAlgo, eModuleTypeCv);
        if(requestList.at(i) == nullptr)
        {
            MY_LOGE("acquire request fail");
            return false;
        }
        totalIdx |= (0x1 << i);
    }

    /* allocate sync buffer */
    buf = mMemAllocator.memAlloc(sizeof(struct syncFlagList));
    if(buf == nullptr)
    {
        MY_LOGE("allocate memory fail");
        return false;
    }

    /* setup vpu buffer */
    syncBuffer.planeCount = 1;
    syncBuffer.width = buf->len;
    syncBuffer.height = 1;
    syncBuffer.planes[0].fd = buf->shareFd;
    syncBuffer.planes[0].offset = 0;
    syncBuffer.planes[0].length = buf->len;
    syncBuffer.port_id = 1;
    syncBuffer.planes[0].stride = buf->len;

    /* add buffer to requests */
    for(i=0;i<mCoreNum;i++)
    {
        memset(&prop, 0, sizeof(struct multicoreProperty));

        prop.selfIdx = 1<<i;
        prop.totalIdx = totalIdx;
        prop.syncNum = mCoreNum;
        prop.syncPattern = randomPattern;
        MY_LOGD("set property[0x%x/0x%x/%d/0x%x]", prop.selfIdx,prop.totalIdx,prop.syncNum,prop.syncPattern);

        if(requestList.at(i)->setProperty(&prop, sizeof(struct multicoreProperty)) == false)
        {
            MY_LOGE("set property[%d] fail", i);
            ret = false;
            goto out;
        }

        if(requestList.at(i)->addBuffer(syncBuffer) == false)
        {
            MY_LOGE("addBuffer[%d] fail", i);
            ret = false;
            goto out;
        }
    }

    /* packing request */
    if(mStream->packRequest(requestList) == false)
    {
        ret = false;
        goto out;
    }

    /* enque vpu request */
    for(i=0;i<loops;i++)
    {
        memset((void *)buf->va, 0, sizeof(struct syncFlagList));
        MY_LOGI("enque %d packed request", mCoreNum);
        for(j=0;j<mCoreNum;j++)
        {
            if(mStream->enque(requestList.at(j)) == false)
            {
                MY_LOGE("enque %d packed request failed", j);
                return false;
            }
        }

        MY_LOGI("deque %d packed request...", mCoreNum);
        for(j=0;j<mCoreNum;j++)
        {
            if(mStream->deque() == nullptr)
            {
                MY_LOGE("deque %d request fail", j);
                return false;
            }
        }
        MY_LOGI("deque %d packed request ok!", mCoreNum);
    }

    /* unpacking request */
    if(mStream->unpackRequest(requestList) == false)
    {
        ret = false;
        goto out;
    }

    ret = true;

out:

    /* release test */
    if(mMemAllocator.memFree(buf) == false)
    {
        MY_LOGE("free memory fail");
    }

    for(i=0;i<mCoreNum;i++)
    {
        mStream->release(requestList.at(i));
    }

    return ret;
}

