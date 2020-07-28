
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
#define MTK_OMX_LOGD(this, fmt, arg...) \
    do { \
        if (OmxMVAManager::mEnableMoreLog == OMX_TRUE) { \
            ALOGD("[0x%08x] " fmt, this, ##arg); \
        } \
    } while(0)
#define MTK_OMX_LOGE(fmt, arg...)       ALOGE("[0x%08x] " fmt, this, ##arg)

//-------- MtkOmxMVAMap --------
//OmxMVAEntry
int OmxMVAEntry::setToVencBS(VENC_DRV_PARAM_BS_BUF_T *bsBuf)
{
    bsBuf->rBSAddr.u4VA = (VAL_UINT32_T)va;
    bsBuf->rBSAddr.u4PA = (VAL_UINT32_T)pa;
    bsBuf->rBSAddr.u4Size = (VAL_UINT32_T)size;
    bsBuf->u4BSStartVA = (VAL_UINT32_T)va;
    return 1;
}

int OmxMVAEntry::setToVencFrm(VENC_DRV_PARAM_FRM_BUF_T *frmBuf)
{
    frmBuf->rFrmBufAddr.u4VA = (VAL_UINT32_T)va;
    frmBuf->rFrmBufAddr.u4PA = (VAL_UINT32_T)pa;
    frmBuf->rFrmBufAddr.u4Size = (VAL_UINT32_T)size;
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
    bufInfo->ionBufHndl = -1;
    return 1;
}

//OmxIonMVAEntry
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
    bufInfo->ionBufHndl = ionBufHndl;
    MTK_OMX_LOGD(this, "[ION][setToVBufInfo] va = 0x%x, pa = 0x%x, size = %d, pNativeHandle = 0x%x, ionFd = %d, ionBufHndl = 0x%x\n",
                 bufInfo->u4OriVA, bufInfo->u4PA, bufInfo->u4BuffSize, bufInfo->pNativeHandle, bufInfo->iIonFd, bufInfo->ionBufHndl);
    return 1;
}

bool OmxIonMVAEntry::hasThisKey(int key)
{
    return (ionBufHndl == (ion_user_handle_t)key);
}

//OmxIonMVAMap
int OmxIonMVAMap::mIonHandleCount = 0;

OmxIonMVAMap::OmxIonMVAMap(const char *ionName)
    : mIonDevFd(-1),
      mIsMCIMode(OMX_FALSE),
      mIsClientLocally(OMX_FALSE),
      mIsVdec(OMX_FALSE),
      mIsSecure(OMX_FALSE)
{
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
}

OmxIonMVAMap::~OmxIonMVAMap()
{
    --mIonHandleCount;
    if (-1 != mIonDevFd)
    {
        close(mIonDevFd);
        mIonDevFd = -1;
    }
}

int OmxIonMVAMap::setBoolProperty(const char *key, OMX_BOOL value)
{
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
    else if (!strncmp(key, "SEC", strlen("SEC")))
    {
        mIsSecure = value;
    }
    else
    {
        MTK_OMX_LOGD(this, "[ION] unsupport key:%s", key);
    }
    return 1;
}

#if 0
int OmxIonMVAMap::setCharProperty(const char *key, const char *value)
{
    if (!strncmp(key, "NAME", strlen("NAME")))
    {
        if (-1 != mIonDevFd)
        {
            close(mIonDevFd);
            mIonDevFd = -1;
            --mIonHandleCount;
        }

        mIonDevFd = mt_ion_open(value);
        if (mIonDevFd < 0)
        {
            MTK_OMX_LOGE("[ERROR] cannot open ION device.(%d)", mIonDevFd);
            mIonDevFd = -1;
            return mIonDevFd;
        }
        else
        {
            ++mIonHandleCount;
        }
    }
    else
    {
        MTK_OMX_LOGD(this, "[ION] unsupport key:%s", key);
    }
    return 1;
}
#endif

void *OmxIonMVAMap::newEntry(int align, int size, void *bufHdr)
{
    OmxIonMVAEntry *newone = new(std::nothrow) OmxIonMVAEntry;

    if (newone == NULL)
    {
        MTK_OMX_LOGE("allocate entry fail!");
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

    MTK_OMX_LOGD(this, "[ION][AllocateBuffer] va = 0x%x, pa = 0x%x, size = 0x%x\n",
                 newone->va, newone->pa, newone->size);

    return newone;
EXIT:
    if (newone != NULL)
    {
        delete newone;
    }
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
    OmxIonMVAEntry *newone = new(std::nothrow) OmxIonMVAEntry;

    if (newone == NULL)
    {
        MTK_OMX_LOGE("allocate entry fail!");
        return NULL;
    }

    newone->bufHdr = bufHdr;
    buffer_handle_t _handle = (buffer_handle_t)handle;
    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &newone->size);
    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ION_FD, &newone->srcFd);
    newone->handle = handle;

    newone->fd = -1;

    if (mapIon(newone, 0) < 0)
    {
        goto EXIT;
    }

    MTK_OMX_LOGD(this, "IonBuffer: entry(0x%08x), va(0x%08x), mva:(0x%08x), ion_buf_handle (0x%08X)",
                 (unsigned int)newone, (unsigned int)newone->va,
                 (unsigned int)newone->pa, (unsigned int)newone->ionBufHndl);

    return newone;
EXIT:
    if (newone != NULL)
    {
        delete newone;
    }
    return NULL;
}

void OmxIonMVAMap::freeEntry(void *entry)
{
    OmxIonMVAEntry  *one = (OmxIonMVAEntry *)entry;

    MTK_OMX_LOGD(this, "[ION][FreeBuffer] entry=0x%lx, va=0x%lx, pa=0x%lx,"
                 "size=0x%lx, srcFd=0x%08X, fd=0x%08X, bufHdr=0x%08X\n",
                 one, one->va, one->pa, one->size, one->srcFd, one->fd, one->bufHdr);
    if(!mIsSecure)
    {
        //clear ion
        ion_munmap(mIonDevFd, (void *)one->va, one->size);
    }

    // free ION buffer fd
    if (one->fd >= 0)
    {
        if (ion_share_close(mIonDevFd, one->fd))
        {
            MTK_OMX_LOGE("[ERROR] ion_share_close failed");
        }
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
    OmxIonMVAEntry  *one = (OmxIonMVAEntry *)entry;
    MTK_OMX_LOGD(this, "entry=0x%x, va=0x%x, pa=0x%x, hdr=0x%x, hndl=0x%x, size=%d, fd=%d, srcfd=%d, ionhndl=%d",
                 one, one->va, one->pa, one->bufHdr, one->handle, one->size, one->fd, one->srcFd, one->ionBufHndl);
}

int OmxIonMVAMap::getIonFd(){
    return mIonDevFd;
}

int OmxIonMVAMap::findKeyForSearch(void *handle)
{
    int tmpSrcFd = -1;
    ion_user_handle_t key = -1;
    buffer_handle_t _handle = (buffer_handle_t)handle;
    if (NULL == handle)
    {
        return -1;
    }

    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ION_FD, &tmpSrcFd);
    if (tmpSrcFd > 0)
    {
        if (ion_import(mIonDevFd, tmpSrcFd, &key))
        {
            MTK_OMX_LOGE("[ERROR] ion_import failed, LINE: %d", __LINE__);
        }
    }
    return key;
}

int OmxIonMVAMap::deleteKeyAfterSearch(void *handle, int key)
{
    if (key != -1)
   {
       if (ion_free(mIonDevFd, key))
       {
            MTK_OMX_LOGE("[ERROR] ion_free failed in deleteKeyAfterSearch");
       }
   }
    return 1;
}

bool OmxIonMVAMap::configIonBuffer(int ion_fd, ion_user_handle_t handle)
{

    struct ion_mm_data mm_data;
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = handle;
    mm_data.config_buffer_param.eModuleID = eVideoGetM4UModuleID(VAL_MEM_CODEC_FOR_VDEC);
    mm_data.config_buffer_param.security = (mIsMCIMode == OMX_TRUE) ? 1 : 0;
    mm_data.config_buffer_param.coherent = (mIsMCIMode == OMX_TRUE) ? 1 : 0;

    if (ion_custom_ioctl(ion_fd, ION_CMD_MULTIMEDIA, &mm_data))
    {
        MTK_OMX_LOGE("[ERROR] cannot configure buffer");
        return false;
    }
    return true;
}

uint32_t OmxIonMVAMap::getIonPhysicalAddress(int ion_fd, ion_user_handle_t handle)
{
    // query physical address
    struct ion_sys_data sys_data;
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = handle;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data))
    {
        MTK_OMX_LOGE("[ERROR] cannot get buffer physical address");
        return 0;
    }
    return (uint32_t)sys_data.get_phys_param.phy_addr;
}

int OmxIonMVAMap::mapIon(OmxIonMVAEntry *pItem, int align)
{
    if (pItem->srcFd > 0)
    {
        if (ion_import(mIonDevFd, pItem->srcFd, &pItem->ionBufHndl))
        {
            MTK_OMX_LOGE("[ERROR] ion_import failed, LINE: %d", __LINE__);
            goto EXIT;
        }
    }
    else
    {
        uint32_t flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;//it's cacheable, 0 --> non-cacheable
        int ret = ion_alloc_mm(mIonDevFd, pItem->size, align, flags, &pItem->ionBufHndl);
        if (0 != ret)
        {
            MTK_OMX_LOGE("[ERROR] ion_alloc_mm failed (%d), LINE:%d", ret, __LINE__);
            goto EXIT;
        }
    }

    if (ion_share(mIonDevFd, pItem->ionBufHndl, &pItem->fd))
    {
        MTK_OMX_LOGE("[ERROR] ion_share failed, LINE:%d", __LINE__);
        goto EXIT;
    }
    //secure ion do not support map va if it is not eng load.
    if(mIsSecure)
    {
        pItem->va = 0;
        MTK_OMX_LOGE("[Info] We don't map va when SVP");
    }
    else
    {
        // map virtual address
        pItem->va = (void *)ion_mmap(mIonDevFd, NULL, pItem->size, PROT_READ | PROT_WRITE, MAP_SHARED, pItem->fd, 0);
        if (pItem->va == 0 || (uint32_t)pItem->va == 0xffffffff)
        {
            MTK_OMX_LOGE("[ERROR] ion_mmap failed, LINE:%d", __LINE__);
            close(pItem->fd);
            goto EXIT;
        }
        MTK_OMX_LOGD(this, "ION share_fd %d, size (%d), va (0x%08X)", pItem->fd, pItem->size, pItem->va);
    }
    // configure buffer
    configIonBuffer(mIonDevFd, pItem->ionBufHndl);
    pItem->pa = (void *)getIonPhysicalAddress(mIonDevFd, pItem->ionBufHndl);

#if 0
    // Try to close fd early.
    if (pItem->fd >= 0)
    {
        if (ion_share_close(mIonDevFd, pItem->fd))
        {
            MTK_OMX_LOGE("[ERROR] [Early_Close] ion_share_close failed");
        }
        else
        {
            pItem->fd = -1; // Reset as the initial value after ion_share_close() successfully.
        }
    }
#endif
    return 1;
EXIT:
    return -1;
}

//-------- MtkOmxMVAMgr --------
//OmxMVAManager
OMX_BOOL OmxMVAManager::mEnableMoreLog = OMX_FALSE;
OmxMVAManager::OmxMVAManager()
    : mType(NULL),
      mMap(NULL)
{
    init("ion");
    INIT_MUTEX(mEntryLock);
}

OmxMVAManager::OmxMVAManager(const char *type)
    : mType(NULL),
      mMap(NULL)
{
    init(type);
    INIT_MUTEX(mEntryLock);
}

OmxMVAManager::OmxMVAManager(const char *type, const char *String)
    : mType(NULL),
      mMap(NULL)
{
    init(type, String);
    INIT_MUTEX(mEntryLock);
}

OmxMVAManager::~OmxMVAManager()
{
    MTK_OMX_LOGD(this, "remove %s MVA Map, count %d", mType, mEntryList.size());
    if (mType != NULL)
    {
        MTK_OMX_FREE(mType);
    }

    //check if list is empty, if not, clear them all!
    while (mEntryList.size() > 0)
    {
        mMap->freeEntry(mEntryList.itemAt(0));
        mEntryList.removeAt(0);
    }

    delete mMap;
    DESTROY_MUTEX(mEntryLock);
}

int OmxMVAManager::setBoolProperty(const char *key, OMX_BOOL value)
{
    return mMap->setBoolProperty(key, value);
}

#if 0
int OmxMVAManager::setCharProperty(const char *key, const char *value)
{
    return mMap->setCharProperty(key, value);
}
#endif

int OmxMVAManager::newOmxMVAwithVA(void *va, int size, void *bufHdr)
{
    OmxMVAEntry *newone = (OmxMVAEntry *)mMap->newEntryWithVa(va, size, bufHdr);

    if (NULL != newone)
    {
        LOCK(mEntryLock);
        mEntryList.push(newone);
        UNLOCK(mEntryLock);
        //dumpList();
        return 1;
    }

    return -1;
}

int OmxMVAManager::newOmxMVAandVA(int align, int size, void *bufHdr, void **pVa)
{
    OmxMVAEntry *newone = (OmxMVAEntry *)mMap->newEntry(align, size, bufHdr);

    if (NULL != newone)
    {
        LOCK(mEntryLock);
        mEntryList.push(newone);
        UNLOCK(mEntryLock);
        *pVa = newone->va;
        //dumpList();
        return 1;
    }

    return -1;
}

int OmxMVAManager::newOmxMVAwithHndl(void *handle, void *bufHdr)
{
    OmxMVAEntry *newone = (OmxMVAEntry *)mMap->newEntryWithHndl(handle, bufHdr);

    if (NULL != newone)
    {
        LOCK(mEntryLock);
        mEntryList.push(newone);
        UNLOCK(mEntryLock);
        //dumpList();
        return 1;
    }

    return -1;
}

int OmxMVAManager::freeOmxMVAAll()
{
    MTK_OMX_LOGD(this, "Remove %s MVA Map, remain buffer count=%d", mType, mEntryList.size());

    //check if list is empty, if not, clear them all!
    LOCK(mEntryLock);
    while (mEntryList.size() > 0)
    {
        mMap->freeEntry(mEntryList.itemAt(0));
        mEntryList.removeAt(0);
    }
    UNLOCK(mEntryLock);

    //dumpList();
    return 1;
}
int OmxMVAManager::freeOmxMVAByVa(void *va)
{
    LOCK(mEntryLock);
    int index = getMapIndexFromVa(va);
    if (index < 0)
    {
        UNLOCK(mEntryLock);
        return -1;
    }
    else
    {
        mMap->freeEntry(mEntryList.itemAt(index));
        mEntryList.removeAt(index);
    }
    //dumpList();
    UNLOCK(mEntryLock);
    return 1;
}

int OmxMVAManager::freeOmxMVAByHndl(void *handle)
{
    LOCK(mEntryLock);
    int index = getMapIndexFromHndl(handle);
    if (index < 0)
    {
        UNLOCK(mEntryLock);
        return -1;
    }
    else
    {
        mMap->freeEntry(mEntryList.itemAt(index));
        mEntryList.removeAt(index);
    }
    //dumpList();
    UNLOCK(mEntryLock);
    return 1;
}

int OmxMVAManager::getOmxMVAFromVAToVencBS(void *va, VENC_DRV_PARAM_BS_BUF_T *bsBuf)
{
    LOCK(mEntryLock);
    int index = 0;
    index = getMapIndexFromVa(va);
    if (index >= 0)//get MVA
    {
        OmxMVAEntry *one = mEntryList.itemAt(index);
        one->setToVencBS(bsBuf);
        bsBuf->rSecMemHandle = 0;
        UNLOCK(mEntryLock);
        return 1;
    }
    else
    {
        UNLOCK(mEntryLock);
        return -1;
    }
}

int OmxMVAManager::getOmxMVAFromVAToVencFrm(void *va, VENC_DRV_PARAM_FRM_BUF_T *frmBuf)
{
    LOCK(mEntryLock);
    int index = 0;
    index = getMapIndexFromVa(va);
    if (index >= 0)//get MVA
    {
        OmxMVAEntry *one = mEntryList.itemAt(index);
        one->setToVencFrm(frmBuf);
        frmBuf->rSecMemHandle = 0;
        UNLOCK(mEntryLock);
        return 1;
    }
    else
    {
        UNLOCK(mEntryLock);
        return -1;
    }
}

int OmxMVAManager::getOmxMVAFromVA(void *va, uint32_t *mva)
{
    LOCK(mEntryLock);
    int index = 0;
    index = getMapIndexFromVa(va);
    if (index >= 0)//get MVA
    {
        OmxMVAEntry *one = mEntryList.itemAt(index);
        *mva = (uint32_t)one->pa;
        UNLOCK(mEntryLock);
        return 1;
    }
    else
    {
        UNLOCK(mEntryLock);
        return -1;
    }
}

int OmxMVAManager::getOmxInfoFromVA(void* va, VBufInfo* info)
{
    LOCK(mEntryLock);
    int index = 0;
    index = getMapIndexFromVa(va);
    if (index >= 0)//get MVA
    {
        OmxMVAEntry *one = mEntryList.itemAt(index);
        one->setToVBufInfo(info);
        UNLOCK(mEntryLock);
        return 1;
    }
    else
    {
        UNLOCK(mEntryLock);
        return -1;
    }
}

int OmxMVAManager::getOmxInfoFromHndl(void *handle, VBufInfo* info)
{
    LOCK(mEntryLock);
    int index = 0;
    index = getMapIndexFromHndl(handle);
    if (index >= 0)//get MVA
    {
        OmxMVAEntry *one = mEntryList.itemAt(index);
        one->setToVBufInfo(info);
        UNLOCK(mEntryLock);
        return 1;
    }
    else
    {
        UNLOCK(mEntryLock);
        return -1;
    }
}

int OmxMVAManager::getOmxMVAFromHndlToVencBS(void *handle, VENC_DRV_PARAM_BS_BUF_T *bsBuf)
{
    LOCK(mEntryLock);
    int index = 0;
    index = getMapIndexFromHndl(handle);
    if (index >= 0)//get MVA
    {
        OmxMVAEntry *one = mEntryList.itemAt(index);
        one->setToVencBS(bsBuf);
        bsBuf->rSecMemHandle = 0;
        UNLOCK(mEntryLock);
        return 1;
    }
    else
    {
        UNLOCK(mEntryLock);
        return -1;
    }
}

int OmxMVAManager::getOmxMVAFromHndlToVencFrm(void *handle, VENC_DRV_PARAM_FRM_BUF_T *frmBuf)
{
    LOCK(mEntryLock);
    int index = 0;
    index = getMapIndexFromHndl(handle);
    if (index >= 0)//get MVA
    {
        OmxMVAEntry *one = mEntryList.itemAt(index);
        one->setToVencFrm(frmBuf);
        frmBuf->rSecMemHandle = 0;
        UNLOCK(mEntryLock);
        return 1;
    }
    else
    {
        UNLOCK(mEntryLock);
        return -1;
    }
}

//private
void OmxMVAManager::init(const char *type, const char *ionName)
{
    char buildType[PROPERTY_VALUE_MAX];
    char enableMoreLog[PROPERTY_VALUE_MAX];
    property_get("ro.build.type", buildType, "eng");
    if (!strcmp(buildType, "user") || !strcmp(buildType, "userdebug")){
        property_get("vendor.mtk.omx.enable.mvamgr.log", enableMoreLog, "0");
        mEnableMoreLog = (OMX_BOOL) atoi(enableMoreLog);
    }

    mType = (char *)MTK_OMX_ALLOC(VENC_ROUND_N(strlen(type) + 1, 16));
    if (mType == NULL)
    {
        MTK_OMX_LOGD(this, "alloc type fail!");
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

    MTK_OMX_LOGD(this, "Create %s MVA Map!", mType);
}

void OmxMVAManager::syncBufferCacheFrm(void *buffer, unsigned int operation)
{
    struct ion_sys_data sys_data;
    VBufInfo info;

    if (0 == this->getOmxInfoFromVA(buffer, &info))
    {
        MTK_OMX_LOGE("Failed to getOmxInfoFromVA(). buffer: %p", buffer);
        return ;
    }

    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle = info.ionBufHndl;
    sys_data.cache_sync_param.sync_type = (ION_CACHE_SYNC_TYPE)operation;
    if (operation == ION_CACHE_FLUSH_ALL)
    {
        sys_data.cache_sync_param.size = 0xffffffff;
        MTK_OMX_LOGD(this, "[syncBufferCacheFrm] ION_CACHE_FLUSH_ALL handle: 0x%x \n", info.ionBufHndl);
    }
    else
    {
        sys_data.cache_sync_param.va = buffer;
        sys_data.cache_sync_param.size = info.u4BuffSize;
        MTK_OMX_LOGD(this, "[syncBufferCacheFrm] ION_CACHE_INVALID/FLUSH_BY_RANGE(%d) VA: 0x%x size %d handle: 0x%x \n", operation, buffer, info.u4BuffSize, info.ionBufHndl);
    }

    if(ion_custom_ioctl(mMap->getIonFd(), ION_CMD_SYSTEM, &sys_data))
    {
        //config error
        MTK_OMX_LOGE("ion_custom_ioctl error\n");
    }
}

int OmxMVAManager::getMapIndexFromVa(void *va)
{
    VAL_UINT32_T i;
    for (i = 0; i < mEntryList.size(); ++i)
    {
        OmxMVAEntry *one = mEntryList.itemAt(i);
        if (one->va == va)
        {
            return i;
        }
    }
    MTK_OMX_LOGD(this, "[WARNING] Can not find MVA from VA: 0x%x\n", (unsigned int)va);
    return -1;
}

int OmxMVAManager::getMapIndexFromHndl(void *handle)
{
    VAL_UINT32_T i;
    int key = mMap->findKeyForSearch(handle);
    for (i = 0; i < mEntryList.size(); ++i)
    {
        OmxMVAEntry *one = mEntryList.itemAt(i);
        if (one->hasThisKey(key))
        {
            mMap->deleteKeyAfterSearch(handle, key);
            return i;
        }
    }
    MTK_OMX_LOGD(this, "[INFO] Don't find MVA from Handle: 0x%x\n", (unsigned int)handle);
    mMap->deleteKeyAfterSearch(handle, key);
    return -1;
}
#if 0
int OmxMVAManager::getMapHndlFromIndex(int index, void *handle)
{
    if(index <= mEntryList.size())
    {
        OmxMVAEntry *one = mEntryList.itemAt(index);
        handle = one->handle;
        return 1;
    }
    else
    {
        MTK_OMX_LOGE("[ERROR] : invalid index %d\n", index);
        return -1;
    }
}

int OmxMVAManager::getMapIndexFromBufHdr(void *bufHdr)
{
    VAL_UINT32_T i;
    for (i = 0; i < mEntryList.size(); ++i)
    {
        OmxMVAEntry *one = mEntryList.itemAt(i);
        if (one->bufHdr == bufHdr)
        {
            return i;
        }
    }
    MTK_OMX_LOGD(this, "[INFO] Don't find MVA from bufferHeader: 0x%x\n", (unsigned int)bufHdr);
    return -1;
}

int OmxMVAManager::setSecHandleFromVa(void *va, unsigned long secHandle)
{
    VAL_UINT32_T i;
    for (i = 0; i < mEntryList.size(); ++i)
    {
        OmxMVAEntry *one = mEntryList.itemAt(i);
        if (one->va == va)
        {
            one->rSecMemHandle = secHandle;
            return 1;
        }
    }
    MTK_OMX_LOGE("[ERROR] Can not set Handle from VA: 0x%x\n", (unsigned int)va);
    return -1;
}

int OmxMVAManager::setBufHdrFromVa(void *va, void *bufHdr)
{
    VAL_UINT32_T i;
    for (i = 0; i < mEntryList.size(); ++i)
    {
        OmxMVAEntry *one = mEntryList.itemAt(i);
        if (one->va == va)
        {
            one->bufHdr = bufHdr;
            return 1;
        }
    }
    MTK_OMX_LOGE("[ERROR] Can not set bufHdr from VA: 0x%x\n", (unsigned int)va);
    return -1;
}
#endif
void OmxMVAManager::dumpList()
{
    VAL_UINT32_T i;
    for (i = 0; i < mEntryList.size(); ++i)
    {
        mMap->dumpEntry(mEntryList.itemAt(i));
    }
    MTK_OMX_LOGD(this, "%s", mType);
}

