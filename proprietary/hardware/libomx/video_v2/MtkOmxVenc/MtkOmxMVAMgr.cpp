
#include "MtkOmxMVAMgr.h"

#undef LOG_TAG
#define LOG_TAG "MtkOmxMVAMgr"

// for ion map
#include <linux/ion_drv.h>
#include <ion.h>
#include <ion/ion.h>
#include <sys/mman.h>
#include <hardware/gralloc.h>
#include <ui/gralloc_extra.h>

// for accessing system properites
#include <cutils/properties.h>

#define VENC_ROUND_N(X, N)   (((X) + ((N)-1)) & (~((N)-1))) //only for N is exponential of 2

#undef MTK_OMX_LOGD
#undef MTK_OMX_LOGE

#define MORE_DEBUG_LOG
#ifdef MORE_DEBUG_LOG
#define MTK_OMX_LOGD(fmt, arg...) \
    do { \
        if (OmxMVAManager::mEnableMoreLog >= 1) { \
            ALOGD("[0x%08x] " fmt, this, ##arg); \
        } \
    } while(0)
#define MTK_OMX_LOGV(fmt, arg...) \
    do { \
        if (OmxMVAManager::mEnableMoreLog >= 2) { \
            ALOGD("[0x%08x] " fmt, this, ##arg); \
        } \
    } while(0)
#define MTK_OMX_LOGE(fmt, arg...)       ALOGE("[0x%08x] " fmt, this, ##arg)

#define IN_FUNC() \
    MTK_OMX_LOGV("+ %s():%d\n", __func__, __LINE__)

#define OUT_FUNC() \
    MTK_OMX_LOGV("- %s():%d\n", __func__, __LINE__)
#else
#define MTK_OMX_LOGD(fmt, arg...)
#define MTK_OMX_LOGE(fmt, arg...)
#define IN_FUNC()
#define OUT_FUNC()
#endif

//-------- MtkOmxMVAMap --------
//OmxMVAEntry
int OmxMVAEntry::setToVencBS(VENC_DRV_PARAM_BS_BUF_T *bsBuf)
{
    bsBuf->rBSAddr.u4VA = (VAL_UINT32_T)va;
    bsBuf->rBSAddr.u4PA = (VAL_UINT32_T)pa;
    bsBuf->rBSAddr.u4Size = (VAL_UINT32_T)size;
    bsBuf->u4BSStartVA = (VAL_UINT32_T)va;
    bsBuf->pIonBufhandle = 0;
    bsBuf->u4IonDevFd = 0;
    bsBuf->u4IonShareFd = 0;
    return 1;
}

int OmxMVAEntry::setToVencFrm(VENC_DRV_PARAM_FRM_BUF_T *frmBuf)
{
    frmBuf->rFrmBufAddr.u4VA = (VAL_UINT32_T)va;
    frmBuf->rFrmBufAddr.u4PA = (VAL_UINT32_T)pa;
    frmBuf->rFrmBufAddr.u4Size = (VAL_UINT32_T)size;
    frmBuf->pIonBufhandle = 0;
    frmBuf->u4IonDevFd = 0;
    frmBuf->u4IonShareFd = 0;
    return 1;
}

int OmxMVAEntry::setToVBufInfo(VBufInfo *bufInfo)
{
    bufInfo->u4OriVA = (VAL_UINT32_T)va;
    bufInfo->u4VA = (VAL_UINT32_T)va;
    bufInfo->u4PA = (VAL_UINT32_T)pa;
    bufInfo->u4BuffSize = (VAL_UINT32_T)size;
    bufInfo->u4BuffHdr = (VAL_UINT32_T)bufHdr;
    bufInfo->pNativeHandle = (void*)handle;
    bufInfo->secure_handle = rSecMemHandle;
    bufInfo->iIonFd = -1;
    return 1;
}

//OmxIonMVAEntry
int OmxIonMVAEntry::setToVencBS(VENC_DRV_PARAM_BS_BUF_T *bsBuf)
{
    bsBuf->rBSAddr.u4VA = (VAL_UINT32_T)va;
    bsBuf->rBSAddr.u4PA = (VAL_UINT32_T)pa;
    bsBuf->rBSAddr.u4Size = (VAL_UINT32_T)size;
    bsBuf->u4BSStartVA = (VAL_UINT32_T)va;
    bsBuf->pIonBufhandle = ionBufHndl;
    bsBuf->u4IonDevFd = ionDevFd;
    bsBuf->u4IonShareFd = fd;
    return 1;
}

int OmxIonMVAEntry::setToVencFrm(VENC_DRV_PARAM_FRM_BUF_T *frmBuf)
{
    frmBuf->rFrmBufAddr.u4VA = (VAL_UINT32_T)va;
    frmBuf->rFrmBufAddr.u4PA = (VAL_UINT32_T)pa;
    frmBuf->rFrmBufAddr.u4Size = (VAL_UINT32_T)size;
    frmBuf->pIonBufhandle = ionBufHndl;
    frmBuf->u4IonDevFd = ionDevFd;
    frmBuf->u4IonShareFd = fd;
    return 1;
}

int OmxIonMVAEntry::setToVBufInfo(VBufInfo *bufInfo)
{
    bufInfo->u4OriVA = (VAL_UINT32_T)va;
    bufInfo->u4VA = (VAL_UINT32_T)va;
    bufInfo->u4PA = (VAL_UINT32_T)pa;
    bufInfo->u4BuffSize = (VAL_UINT32_T)size;
    bufInfo->u4BuffHdr = (VAL_UINT32_T)bufHdr;
    bufInfo->pNativeHandle = (void*)handle;
    bufInfo->secure_handle = rSecMemHandle;
    bufInfo->iIonFd = fd;
    return 1;
}


//OmxIonMVAMap
int OmxIonMVAMap::mIonHandleCount = 0;

OmxIonMVAMap::OmxIonMVAMap(const char *ionName)
    : mIonDevFd(-1),
      mIsMCIMode(OMX_FALSE),
      mIsClientLocally(OMX_FALSE),
      mIsVdec(OMX_FALSE)
{
    IN_FUNC();
    if (-1 == mIonDevFd)
    {
        //char buf[16];
        //sprintf(buf, "MtkOmxVenc%d", mIonHandleCount);
        //mIonDevFd = mt_ion_open(buf);
        if (ionName == NULL)
        {
            mIonDevFd = mt_ion_open("MtkOmxVcodec");
        }
        else
        {
            mIonDevFd = mt_ion_open(ionName);
        }

        if (mIonDevFd < 0)
        {
            MTK_OMX_LOGE("[ERROR] cannot open ION device.(%d)", mIonDevFd);
            mIonDevFd = -1;
        }
    }
    ++mIonHandleCount;
    OUT_FUNC();
}

OmxIonMVAMap::~OmxIonMVAMap()
{
    IN_FUNC();
    --mIonHandleCount;
    if (-1 != mIonDevFd)
    {
        close(mIonDevFd);
        mIonDevFd = -1;
    }
    OUT_FUNC();
}

int OmxIonMVAMap::setBoolProperty(const char *key, OMX_BOOL value)
{
    IN_FUNC();
    if (!strncmp(key, "MCI", strlen("MCI")))
    {
        mIsMCIMode = value;
    }
    else if (!strncmp(key, "Locally", strlen("Locally")))
    {
        mIsClientLocally = value;
    }
    else if (!strncmp(key, "VDEC", strlen("VDEC")))
    {
        mIsVdec = value;
    }
    else
    {
        MTK_OMX_LOGD("[ION] unsupport key:%s", key);
    }
    OUT_FUNC();
    return 1;
}

void *OmxIonMVAMap::newEntry(int align, int size, void *bufHdr)
{
    IN_FUNC();
    OmxIonMVAEntry *newone = new(std::nothrow) OmxIonMVAEntry;

    if (newone == NULL)
    {
        MTK_OMX_LOGE("allocate entry fail!");
        OUT_FUNC();
        return NULL;
    }

    newone->size = size;
    newone->bufHdr = bufHdr;

    newone->srcFd = -1;
    newone->handle = NULL;
    newone->allocated = true;

    if (mapIon(newone, align) < 0)
    {
        goto EXIT;
    }

    MTK_OMX_LOGD("[ION][AllocateBuffer] entry = 0x%x, va = 0x%x, pa = 0x%x, size = 0x%x\n",
                 newone, newone->va, newone->pa, newone->size);
    OUT_FUNC();
    return newone;
EXIT:
    if (newone != NULL)
    {
        delete newone;
    }
    OUT_FUNC();
    return NULL;
}

void *OmxIonMVAMap::newEntryWithVa(void *va, int size, void *bufHdr)
{
    (void)va;
    (void)size;
    (void)bufHdr;
    MTK_OMX_LOGE("[ERROR] Don't support newEntryWithVa");
    return NULL;
}

void *OmxIonMVAMap::newEntryWithHndl(void *handle, void *bufHdr)
{
    IN_FUNC();
    OmxIonMVAEntry *newone = new(std::nothrow) OmxIonMVAEntry;
    uint64_t usage1=0, usage2=0;

    /*if (newone == NULL)
    {
        MTK_OMX_LOGE("allocate entry fail!");
    }

    newone->bufHdr = bufHdr;*/
    buffer_handle_t _handle = (buffer_handle_t)handle;

    if (newone == NULL)
    {
        MTK_OMX_LOGE("allocate entry fail!");
        goto EXIT;
    }

    newone->bufHdr = bufHdr;
    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &newone->size);
    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ION_FD, &newone->srcFd);
    newone->handle = handle;

    newone->fd = -1;

    if (mapIon(newone, 0) < 0)
    {
        goto EXIT;
    }

    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_PRODUCER_USAGE, &usage1);
    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_CONSUMER_USAGE, &usage2);

    MTK_OMX_LOGD("IonBuffer: entry(0x%08x), va(0x%08x), mva:(0x%08x), ion_buf_handle (0x%08X), data size (%d)"
                ", usage 0x(%llx) 0x(%llx)",
                 (unsigned int)newone, (unsigned int)newone->va,
                 (unsigned int)newone->pa, (unsigned int)newone->ionBufHndl, (unsigned int)newone->size,
                 (long long)usage1,  (long long)usage2);
    OUT_FUNC();
    return newone;
EXIT:
    if (newone != NULL)
    {
        delete newone;
    }
    OUT_FUNC();
    return NULL;
}

void *OmxIonMVAMap::newEntryWithHndlwithSize(void *handle, void *bufHdr, unsigned int desireSize)
{
    IN_FUNC();
    OmxIonMVAEntry *newone = new(std::nothrow) OmxIonMVAEntry;

    buffer_handle_t _handle = (buffer_handle_t)handle;

    if (newone == NULL)
    {
        MTK_OMX_LOGE("allocate entry fail!");
        goto EXIT;
    }

    newone->bufHdr = bufHdr;
    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &newone->size);
    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ION_FD, &newone->srcFd);
    newone->handle = handle;

    newone->fd = -1;
    newone->srcFd = -1;

    MTK_OMX_LOGD("desireSize: %d vs old newone->size: %d", desireSize, newone->size);
    newone->size = (desireSize > newone->size)? desireSize : newone->size;

    if (mapIon(newone, 0) < 0)
    {
        goto EXIT;
    }

    MTK_OMX_LOGD("IonBuffer: entry(0x%08x), va(0x%08x), mva:(0x%08x), ion_buf_handle (0x%08X), data size (%d)",
                 (unsigned int)newone, (unsigned int)newone->va,
                 (unsigned int)newone->pa, (unsigned int)newone->ionBufHndl, (unsigned int)newone->size);
    OUT_FUNC();
    return newone;
EXIT:
    if (newone != NULL)
    {
        delete newone;
    }
    OUT_FUNC();
    return NULL;
}

void OmxIonMVAMap::freeEntry(void *entry)
{
    IN_FUNC();
    OmxIonMVAEntry  *one = (OmxIonMVAEntry *)entry;

    MTK_OMX_LOGD("[ION][FreeBuffer] entry=0x%lx, va=0x%lx, pa=0x%lx,"
                 "size=%d, srcFd=%d, fd=%d, bufHdr=0x%08X\n",
                 one, one->va, one->pa, one->size, one->srcFd, one->fd, one->bufHdr);
    //clear ion
    ion_munmap(mIonDevFd, (void *)one->va, one->size);

    // free ION buffer fd
    if ((one->fd >= 0) && (ion_share_close(mIonDevFd, one->fd)))
    {
        MTK_OMX_LOGE("[ERROR] ion_share_close failed");
    }
    if (one->allocated)
    {
        // free ION buffer fd
        if ((one->srcFd >= 0) && (OMX_FALSE == mIsClientLocally) && ion_share_close(mIonDevFd, one->srcFd))
        {
            MTK_OMX_LOGE("[ERROR] ion_share_close failed for source fd, LINE:%d", __LINE__);
        }
    }
    // free ION buffer handle
    if (ion_free(mIonDevFd, one->ionBufHndl))
    {
        MTK_OMX_LOGE("[ERROR] ion_free failed in FreeBuffer");
    }

    delete(OmxIonMVAEntry *)entry;
}

void OmxIonMVAMap::dumpEntry(void *entry)
{
    IN_FUNC();
    OmxIonMVAEntry  *one = (OmxIonMVAEntry *)entry;
    MTK_OMX_LOGD("entry=0x%x, va=0x%x, pa=0x%x, hdr=0x%x, hndl=0x%x, size=%d, fd=%d, srcfd=%d, ionhndl=%d",
                 one, one->va, one->pa, one->bufHdr, one->handle, one->size, one->fd, one->srcFd, one->ionBufHndl);
}

int OmxIonMVAMap::getIonFd() {
    return mIonDevFd;
}

bool OmxIonMVAMap::configIonBuffer(int ion_fd, ion_user_handle_t handle)
{
    IN_FUNC();
    struct ion_mm_data mm_data;
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = handle;
    mm_data.config_buffer_param.eModuleID = eVideoGetM4UModuleID(VAL_MEM_CODEC_FOR_VENC);
    mm_data.config_buffer_param.security = (mIsMCIMode == OMX_TRUE) ? 1 : 0;
    mm_data.config_buffer_param.coherent = (mIsMCIMode == OMX_TRUE) ? 1 : 0;

    if (ion_custom_ioctl(ion_fd, ION_CMD_MULTIMEDIA, &mm_data))
    {
        MTK_OMX_LOGE("[ERROR] cannot configure buffer");
        OUT_FUNC();
        return false;
    }
    OUT_FUNC();
    return true;
}

uint32_t OmxIonMVAMap::getIonPhysicalAddress(int ion_fd, ion_user_handle_t handle)
{
    IN_FUNC();
    // query physical address
    struct ion_sys_data sys_data;
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = handle;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data))
    {
        MTK_OMX_LOGE("[ERROR] cannot get buffer physical address");
        OUT_FUNC();
        return 0;
    }
    OUT_FUNC();
    return (uint32_t)sys_data.get_phys_param.phy_addr;
}

int OmxIonMVAMap::mapIon(OmxIonMVAEntry *pItem, int align)
{
    IN_FUNC();
    if (pItem->srcFd > 0)
    {
        MTK_OMX_LOGD("ion_import, mIonDevFd=%d, pItem->srcFd=%d", mIonDevFd, pItem->srcFd);

        if (ion_import(mIonDevFd, pItem->srcFd, &pItem->ionBufHndl))
        {
            MTK_OMX_LOGE("[ERROR] ion_import failed, LINE: %d", __LINE__);
            goto EXIT;
        }
    }
    else
    {
        uint32_t flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;//it's cacheable, 0 --> non-cacheable

        MTK_OMX_LOGD("ion_alloc_camera, mIonDevFd=%d, pItem->size=%d, align=%d, flags=0x%08x", mIonDevFd, pItem->size, align, flags);

        int ret = ion_alloc_camera(mIonDevFd, pItem->size, align, flags, &pItem->ionBufHndl);
        if (0 != ret)
        {
            MTK_OMX_LOGE("[ERROR] ion_alloc_mm failed (%d), LINE:%d", ret, __LINE__);
            goto EXIT;
        }
    }

    pItem->ionDevFd = mIonDevFd;
    if (ion_share(mIonDevFd, pItem->ionBufHndl, &pItem->fd))
    {
        MTK_OMX_LOGE("[ERROR] ion_share failed, LINE:%d", __LINE__);
        goto EXIT;
    }
    // map virtual address
    pItem->va = (void *)ion_mmap(mIonDevFd, NULL, pItem->size, PROT_READ | PROT_WRITE, MAP_SHARED, pItem->fd, 0);
    if (pItem->va == 0 || (uint32_t)pItem->va == 0xffffffff)
    {
        MTK_OMX_LOGE("[ERROR] ion_mmap failed, LINE:%d", __LINE__);
        close(pItem->fd);
        goto EXIT;
    }
    MTK_OMX_LOGD("ION share_fd %d, srcFd %d, IonDevFd: %d, size (%d), va (0x%08X)", pItem->fd, pItem->srcFd, pItem->ionDevFd, pItem->size, pItem->va);

    // configure buffer
    configIonBuffer(mIonDevFd, pItem->ionBufHndl);
    pItem->pa = (void *)getIonPhysicalAddress(mIonDevFd, pItem->ionBufHndl);

    OUT_FUNC();
    return 1;
EXIT:
    OUT_FUNC();
    MTK_OMX_LOGE("ION share_fd %d, srcFd %d, IonDevFd: %d, size (%d), va (0x%08X)",
        pItem->fd, pItem->srcFd, pItem->ionDevFd, pItem->size, pItem->va);
    return -1;
}

//-------- MtkOmxMVAMgr --------
//OmxMVAManager
OMX_U8 OmxMVAManager::mEnableMoreLog = 1;

OmxMVAManager::OmxMVAManager()
    : mType(NULL),
      mMap(NULL)
{
    init("ion");
}

OmxMVAManager::OmxMVAManager(const char *type)
    : mType(NULL),
      mMap(NULL)
{
    init(type);
}

OmxMVAManager::OmxMVAManager(const char *type, const char *String)
    : mType(NULL),
      mMap(NULL)
{
    init(type, String);
}

OmxMVAManager::~OmxMVAManager()
{
    MTK_OMX_LOGD("remove %s MVA Map, count %d", mType, mEntryList.size());
    if (mType != NULL)
    {
        MTK_OMX_FREE(mType);
    }

    //check if list is empty, if not, clear them all!
    pthread_mutex_lock(&mEntryListLock);
    while (mEntryList.size() > 0)
    {
        mMap->freeEntry(mEntryList.itemAt(0));
        mEntryList.removeAt(0);
    }
    pthread_mutex_unlock(&mEntryListLock);

    pthread_mutex_destroy(&mEntryListLock);

    delete mMap;
}

int OmxMVAManager::setBoolProperty(const char *key, OMX_BOOL value)
{
    return mMap->setBoolProperty(key, value);
}

int OmxMVAManager::newOmxMVAwithVA(void *va, int size, void *bufHdr)
{
    IN_FUNC();
    OmxMVAEntry *newone = (OmxMVAEntry *)mMap->newEntryWithVa(va, size, bufHdr);

    if (NULL != newone)
    {
        pthread_mutex_lock(&mEntryListLock);
        mEntryList.push(newone);
        pthread_mutex_unlock(&mEntryListLock);
        //dumpList();
        OUT_FUNC();
        return 1;
    }
    OUT_FUNC();
    return -1;
}

int OmxMVAManager::newOmxMVAandVA(int align, int size, void *bufHdr, void **pVa)
{
    IN_FUNC();
    OmxMVAEntry *newone = (OmxMVAEntry *)mMap->newEntry(align, size, bufHdr);

    if (NULL != newone)
    {
        pthread_mutex_lock(&mEntryListLock);
        mEntryList.push(newone);
        pthread_mutex_unlock(&mEntryListLock);
        *pVa = newone->va;
        //dumpList();
        OUT_FUNC();
        return 1;
    }
    OUT_FUNC();
    return -1;
}

int OmxMVAManager::newOmxMVAwithHndl(void *handle, void *bufHdr)
{
    IN_FUNC();
    OmxMVAEntry *newone = (OmxMVAEntry *)mMap->newEntryWithHndl(handle, bufHdr);

    if (NULL != newone)
    {
        pthread_mutex_lock(&mEntryListLock);
        mEntryList.push(newone);
        pthread_mutex_unlock(&mEntryListLock);
        //dumpList();
        OUT_FUNC();
        return 1;
    }
    OUT_FUNC();
    return -1;
}

int OmxMVAManager::newOmxMVAwithHndlwithSize(void *handle, void *bufHdr, unsigned int desireSize)
{
    IN_FUNC();
    OmxMVAEntry *newone = (OmxMVAEntry *)mMap->newEntryWithHndlwithSize(handle, bufHdr, desireSize);

    if (NULL != newone)
    {
        pthread_mutex_lock(&mEntryListLock);
        mEntryList.push(newone);
        pthread_mutex_unlock(&mEntryListLock);
        //dumpList();
        OUT_FUNC();
        return 1;
    }
    OUT_FUNC();
    return -1;
}

int OmxMVAManager::freeOmxMVAAll()
{
    IN_FUNC();
    MTK_OMX_LOGD("Remove %s MVA Map, remain buffer count=%d", mType, mEntryList.size());

    //check if list is empty, if not, clear them all!

    pthread_mutex_lock(&mEntryListLock);
    while (mEntryList.size() > 0)
    {
        mMap->freeEntry(mEntryList.itemAt(0));
        mEntryList.removeAt(0);
    }
    pthread_mutex_unlock(&mEntryListLock);


    //dumpList();
    OUT_FUNC();
    return 1;
}
int OmxMVAManager::freeOmxMVAByVa(void *va)
{
    IN_FUNC();
    pthread_mutex_lock(&mEntryListLock);
    int index = getMapIndexFromVa(va, false);
    if (index >= 0)
    {
        mMap->freeEntry(mEntryList.itemAt(index));
        mEntryList.removeAt(index);
    }
    pthread_mutex_unlock(&mEntryListLock);
    //dumpList();
    OUT_FUNC();
    return index;
}

int OmxMVAManager::freeOmxMVAByHndl(void *handle)
{
    IN_FUNC();
    pthread_mutex_lock(&mEntryListLock);
    int index = getMapIndexFromHndl(handle, false);
    if (index >= 0)
    {
        mMap->freeEntry(mEntryList.itemAt(index));
        mEntryList.removeAt(index);
    }
    pthread_mutex_unlock(&mEntryListLock);
    //dumpList();
    OUT_FUNC();
    return index;
}

int OmxMVAManager::getOmxMVAFromVAToVencBS(void *va, VENC_DRV_PARAM_BS_BUF_T *bsBuf)
{
    IN_FUNC();
    OmxMVAEntry *one = getMapFromVa(va);
    if (one != NULL)//get MVA
    {
        pthread_mutex_lock(&mEntryListLock);
        one->setToVencBS(bsBuf);
        pthread_mutex_unlock(&mEntryListLock);

        bsBuf->rSecMemHandle = 0;
        OUT_FUNC();
        return 1;
    }
    else
    {
        OUT_FUNC();
        return -1;
    }
}

int OmxMVAManager::getOmxMVAFromVAToVencFrm(void *va, VENC_DRV_PARAM_FRM_BUF_T *frmBuf)
{
    IN_FUNC();
    OmxMVAEntry *one = getMapFromVa(va);
    if (one != NULL)//get MVA
    {
        pthread_mutex_lock(&mEntryListLock);
        one->setToVencFrm(frmBuf);
        pthread_mutex_unlock(&mEntryListLock);

        frmBuf->rSecMemHandle = 0;
        OUT_FUNC();
        return 1;
    }
    else
    {
        OUT_FUNC();
        return -1;
    }
}

int OmxMVAManager::getOmxMVAFromVA(void *va, uint32_t *mva)
{
    IN_FUNC();
    OmxMVAEntry *one = getMapFromVa(va);
    if (one != NULL)//get MVA
    {
        pthread_mutex_lock(&mEntryListLock);
        *mva = (uint32_t)one->pa;
        pthread_mutex_unlock(&mEntryListLock);

        OUT_FUNC();
        return 1;
    }
    else
    {
        OUT_FUNC();
        return -1;
    }
}

int OmxMVAManager::getOmxInfoFromVA(void* va, VBufInfo* info)
{
    IN_FUNC();
    OmxMVAEntry *one = getMapFromVa(va);
    if (one != NULL)//get MVA
    {
        pthread_mutex_lock(&mEntryListLock);
        one->setToVBufInfo(info);
        pthread_mutex_unlock(&mEntryListLock);

        OUT_FUNC();
        return 1;
    }
    else
    {
        OUT_FUNC();
        return -1;
    }
}

int OmxMVAManager::getOmxInfoFromHndl(void *handle, VBufInfo* info)
{
    IN_FUNC();
    OmxMVAEntry *one = getMapFromHndl(handle);
    if (one != NULL)//get MVA
    {
        pthread_mutex_lock(&mEntryListLock);
        one->setToVBufInfo(info);
        pthread_mutex_unlock(&mEntryListLock);

        OUT_FUNC();
        return 1;
    }
    else
    {
        OUT_FUNC();
        return -1;
    }
}

int OmxMVAManager::getOmxMVAFromHndlToVencBS(void *handle, VENC_DRV_PARAM_BS_BUF_T *bsBuf)
{
    IN_FUNC();
    OmxMVAEntry *one = getMapFromHndl(handle);
    if (one != NULL)//get MVA
    {
        pthread_mutex_lock(&mEntryListLock);
        one->setToVencBS(bsBuf);
        pthread_mutex_unlock(&mEntryListLock);

        bsBuf->rSecMemHandle = 0;
        OUT_FUNC();
        return 1;
    }
    else
    {
        OUT_FUNC();
        return -1;
    }
}

int OmxMVAManager::getOmxMVAFromHndlToVencFrm(void *handle, VENC_DRV_PARAM_FRM_BUF_T *frmBuf)
{
    IN_FUNC();
    OmxMVAEntry *one = getMapFromHndl(handle);
    if (one != NULL)//get MVA
    {
        pthread_mutex_lock(&mEntryListLock);
        one->setToVencFrm(frmBuf);
        pthread_mutex_unlock(&mEntryListLock);

        frmBuf->rSecMemHandle = 0;
        OUT_FUNC();
        return 1;
    }
    else
    {
        OUT_FUNC();
        return -1;
    }
}

//private
void OmxMVAManager::init(const char *type, const char *ionName)
{
    IN_FUNC();
    mEnableMoreLog = (OMX_BOOL) MtkVenc::EnableMoreLog("0" /*userdebug/user default*/);

    pthread_mutex_init(&mEntryListLock, NULL);

    mType = (char *)MTK_OMX_ALLOC(VENC_ROUND_N(strlen(type) + 1, 16));
    if (mType == NULL)
    {
        MTK_OMX_LOGD("alloc type fail!");
        OUT_FUNC();
        return;
    }
    else
    {
        memset(mType, 0, VENC_ROUND_N(strlen(type) + 1, 16));
        strncpy(mType, type, strlen(type));
    }

    mMap = new(std::nothrow) OmxIonMVAMap(ionName);

    if (mMap == NULL)
    {
        MTK_OMX_LOGE("create map fail!");
    }

    MTK_OMX_LOGD("Create %s MVA Map!", mType);
    OUT_FUNC();
}

void OmxMVAManager::syncBufferCacheFrm(OMX_U8* buffer, bool flushAll)
{
    IN_FUNC();
    struct ion_sys_data sys_data;
    VENC_DRV_PARAM_FRM_BUF_T tempFrameBuf;

    this->getOmxMVAFromVAToVencFrm(buffer, &tempFrameBuf);
    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = tempFrameBuf.pIonBufhandle;
    if(!flushAll)
    {
        sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_BY_RANGE;
        sys_data.cache_sync_param.va = (void*)tempFrameBuf.rFrmBufAddr.u4VA;
        sys_data.cache_sync_param.size = tempFrameBuf.rFrmBufAddr.u4Size;
    }
    else
    {
        sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_ALL;
        sys_data.cache_sync_param.size = 0xffffffff;
    }
    if (ion_custom_ioctl(mMap->getIonFd(), ION_CMD_SYSTEM, &sys_data))
    {
        MTK_OMX_LOGE("[ERROR] cannot flush ion buffer");
    }
    OUT_FUNC();
}

void OmxMVAManager::syncBufferCacheFrm(OMX_U8* buffer)
{
    return syncBufferCacheFrm(buffer, false);
}

int OmxMVAManager::getMapIndexFromVa(void *va, bool lock=true)
{
    IN_FUNC();
    VAL_UINT32_T i;

    if(lock) pthread_mutex_lock(&mEntryListLock);
    for (i = 0; i < mEntryList.size(); ++i)
    {
        OmxMVAEntry *one = mEntryList.itemAt(i);
        if (one->va == va)
        {
            if(lock) pthread_mutex_unlock(&mEntryListLock);
            OUT_FUNC();
            return i;
        }
    }
    if(lock) pthread_mutex_unlock(&mEntryListLock);

    MTK_OMX_LOGE("[ERROR] Can not find MVA from VA: 0x%x\n", (unsigned int)va);
    OUT_FUNC();
    return -1;
}

int OmxMVAManager::getMapIndexFromHndl(void *handle, bool lock=true)
{
    IN_FUNC();
    VAL_UINT32_T i;

    if(lock) pthread_mutex_lock(&mEntryListLock);
    for (i = 0; i < mEntryList.size(); ++i)
    {
        OmxMVAEntry *one = mEntryList.itemAt(i);
        if (one->handle == handle)
        {
            if(lock) pthread_mutex_unlock(&mEntryListLock);
            return i;
        }
    }
    if(lock) pthread_mutex_unlock(&mEntryListLock);

    MTK_OMX_LOGD("[INFO] Don't find MVA from Handle: 0x%x\n", (unsigned int)handle);
    OUT_FUNC();
    return -1;
}

OmxMVAEntry* OmxMVAManager::getMapFromVa(void *va)
{
    IN_FUNC();
    VAL_UINT32_T i;

    OmxMVAEntry* one = NULL;
    pthread_mutex_lock(&mEntryListLock);
    for (i = 0; i < mEntryList.size(); ++i)
    {
        OmxMVAEntry* temp = mEntryList.itemAt(i);
        if (temp->va == va)
        {
            one = temp;
            break;
        }
    }
    pthread_mutex_unlock(&mEntryListLock);
    OUT_FUNC();

    return one;
}

OmxMVAEntry* OmxMVAManager::getMapFromHndl(void *handle)
{
    IN_FUNC();
    VAL_UINT32_T i;

    OmxMVAEntry* one = NULL;
    pthread_mutex_lock(&mEntryListLock);
    for (i = 0; i < mEntryList.size(); ++i)
    {
        OmxMVAEntry* temp = mEntryList.itemAt(i);
        if (temp->handle == handle)
        {
            one = temp;
            break;
        }
    }
    pthread_mutex_unlock(&mEntryListLock);
    OUT_FUNC();

    return one;
}

void OmxMVAManager::dumpList()
{
    IN_FUNC();
    VAL_UINT32_T i;

    pthread_mutex_lock(&mEntryListLock);
    for (i = 0; i < mEntryList.size(); ++i)
    {
        mMap->dumpEntry(mEntryList.itemAt(i));
    }
    pthread_mutex_unlock(&mEntryListLock);

    MTK_OMX_LOGD("%s", mType);
    OUT_FUNC();
}

