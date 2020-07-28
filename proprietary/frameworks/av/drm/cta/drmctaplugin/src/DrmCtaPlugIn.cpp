/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "DrmCtaPlugIn"
#include <utils/Log.h>

#include <DrmCtaPlugIn.h>
#include <DrmCtaUtil.h>
#include <Cta5File.h>
#include <DrmDef.h>
#include <DrmTypeDef.h>
#include <DrmUtil.h>

#include <DrmRequestType.h>
#include <DrmCtaUtil.h>
#include <Cta5FileFactory.h>
#include <binder/IPCThreadState.h>

#include <drm/DrmMtkUtil.h>
#include <drm/DrmConstraints.h>
#include <drm/DrmConvertedStatus.h>
#include <drm/drm_framework_common.h>
#include <drm/DrmInfo.h>
#include <drm/DrmInfoEvent.h>
#include <drm/DrmInfoRequest.h>
#include <drm/DrmInfoStatus.h>
#include <drm/DrmMetadata.h>
#include <drm/DrmSupportInfo.h>

#include <cutils/properties.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define _DRM_DEBUG

// the invalid offset
#define INVALID_OFFSET 0x7fffffff

using namespace android;

namespace android
{
void *encryptThread(void *arg);
void *decryptThread(void *arg);
} //namespace


// This extern "C" is mandatory to be managed by TPlugInManager
extern "C" IDrmEngine* create()
{
    return new DrmCtaPlugIn();
}

// This extern "C" is mandatory to be managed by TPlugInManager
extern "C" void destroy(IDrmEngine* pPlugIn)
{
    delete pPlugIn;
    pPlugIn = NULL;
}

// Needed for event callout from implementation object
Vector<DrmCtaUtil::Listener> *DrmCtaPlugIn::sJavaAPIListeners = NULL;

DrmCtaPlugIn::DrmCtaPlugIn() :
        DrmEngineBase(), mIsEncryptThreadCreated(false), mIsDecryptThreadCreated(false)
{

    // we check according to the property "drm.forwardlock.only"
    if (DrmUtil::sDDebug) ALOGD("DrmCtaPlugIn() : DrmCtaPlugin");
    sJavaAPIListeners = new Vector<DrmCtaUtil::Listener>();
    sem_init(&mAvailEncryptSem, 0, 0);
    sem_init(&mAvailDecryptSem, 0, 0);
    mEncryptThreadId = 0;
    mDecryptThreadId = 0;
    mEncryptThreadNeedCanceled = false;
    mDecryptThreadNeedCanceled = false;
    mTokenMap = new KeyedVector<String8, String8>();
}

DrmCtaPlugIn::~DrmCtaPlugIn()
{
    if (DrmUtil::sDDebug) ALOGD("~DrmCtaPlugIn");
    //make thread exit
    mEncryptThreadNeedCanceled = true;
    mDecryptThreadNeedCanceled = true;
    //pthread_join(mEncryptThreadId, NULL);
    //pthread_join(mDecryptThreadId, NULL);
    sem_destroy(&mAvailEncryptSem);
    sem_destroy(&mAvailDecryptSem);
    delete sJavaAPIListeners;
    delete mTokenMap;
}

DrmConstraints* DrmCtaPlugIn::onGetConstraints(int uniqueId __attribute__((unused)),
                                               const String8* path __attribute__((unused)),
                                               int action __attribute__((unused)))
{
   if (DrmUtil::sDDebug) ALOGD("DrmCtaPlugIn() : onGetConstraints");
    return NULL;
}

DrmMetadata* DrmCtaPlugIn::onGetMetadata(int uniqueId, const String8* path)
{

   if (DrmUtil::sDDebug) ALOGD("onGetMetadata() [%d]", uniqueId);
    if (NULL == path)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]onGetMetadata() : error, the path is NULL");
        return NULL;
    }

    DrmMetadata* drmMetadata = new DrmMetadata();
    return drmMetadata;
}

status_t DrmCtaPlugIn::onInitialize(int uniqueId)
{
    if (DrmUtil::sDDebug) ALOGV("onInitialize() : uniqueId : %d", uniqueId);
    createEncryptThread();
    createDecryptThread();
    return DRM_NO_ERROR;
}

/**
 * Terminate the plug-in
 * and release resource bound to plug-in
 *
 * @param[in] uniqueId Unique identifier for a session
 * @return status_t
 *     Returns DRM_NO_ERROR for success, DRM_ERROR_UNKNOWN for failure
 */
status_t DrmCtaPlugIn::onTerminate(int uniqueId)
{
    if (DrmUtil::sDDebug) ALOGV("onTerminate():uniqueId : %d", uniqueId);

    for (size_t i = 0; i < sJavaAPIListeners->size(); i++)
    {
        if ((*sJavaAPIListeners)[i].GetUniqueId() == uniqueId)
        {
            sJavaAPIListeners->removeAt(i);
            break;
        }
    }

    return DRM_NO_ERROR;
}

status_t DrmCtaPlugIn::onSetOnInfoListener(int uniqueId,
        const IDrmEngine::OnInfoListener* infoListener)
{
    if (DrmUtil::sDDebug) ALOGV("onSetOnInfoListener(): uniqueId : %d", uniqueId);

    DrmCtaUtil::Listener newListener = DrmCtaUtil::Listener(
            const_cast<IDrmEngine::OnInfoListener *>(infoListener), uniqueId);
    bool found = false;

    const int nativeUniqueIdFlag = 0x1000;
    if (uniqueId & nativeUniqueIdFlag)
    {
        if (DrmUtil::sDDebug) ALOGV("onSetOnInfoListener(): native");
    } else
    {
        // Replace old listener for this id if it exists
        for (size_t i = 0; i < sJavaAPIListeners->size(); i++)
        {
            if ((*sJavaAPIListeners)[i].GetUniqueId() == uniqueId)
            {
                if (DrmUtil::sDDebug) ALOGV("onSetOnInfoListener():  replace");
                sJavaAPIListeners->replaceAt(newListener, i);
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (DrmUtil::sDDebug) ALOGV("onSetOnInfoListener() :  add");
            sJavaAPIListeners->push(newListener);
        }
    }
    return DRM_NO_ERROR;
}

bool DrmCtaPlugIn::onCanHandle(int uniqueId, const String8& path)
{
   if (DrmUtil::sDDebug) ALOGD("onCanHandle() uniqueId: [%d], file path: %s ", uniqueId, path.string());
    int fd_in = open(path.string(), O_RDONLY);
    if (-1 == fd_in)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]onCanHandle() : failed to open file, reason [%s]", strerror(errno));
        return false;
    }
    bool isCta5 = Cta5File::isCta5File(fd_in);
    close(fd_in);
   if (DrmUtil::sDDebug) ALOGD("onCanHandle isCta5:result [%d]", isCta5);
    return isCta5;
}

DrmInfoStatus* DrmCtaPlugIn::onProcessDrmInfo(int uniqueId __attribute__((unused)),
                                              const DrmInfo* drmInfo __attribute__((unused)))
{
    DrmInfoStatus* drmInfoStatus = NULL;
    return drmInfoStatus;
}

status_t DrmCtaPlugIn::onSaveRights(int uniqueId __attribute__((unused)),
                                    const DrmRights& drmRights __attribute__((unused)),
        const String8& rightsPath __attribute__((unused)), const String8& contentPath __attribute__((unused)))
{
    return DRM_NO_ERROR;
}

DrmInfo* DrmCtaPlugIn::onAcquireDrmInfo(int uniqueId, const DrmInfoRequest* drmInfoRequest)
{
   if (DrmUtil::sDDebug) ALOGD("EYES onAcquireDrmInfo():uniqueId : %d", uniqueId);

    DrmInfo* drmInfo = NULL;

    if (NULL != drmInfoRequest)
    {
        String8 result; // the result of the operation
        int type = drmInfoRequest->getInfoType();
        String8 action = drmInfoRequest->get(DrmRequestType::KEY_ACTION);
        if (DrmUtil::sDDebug) ALOGD("onAcquireDrmInfo(): execute action %s", action.string());
        if (DrmRequestType::TYPE_SET_DRM_INFO == type)
        {
            bool actionResult = false;
            if (DrmRequestType::ACTION_CTA5_ENCRYPT == action)
            {
               if (DrmUtil::sDDebug) ALOGD("onAcquireDrmInfo() ACTION_CTA5_ENCRYPT");
                actionResult = encrypt(drmInfoRequest);
            } else if (DrmRequestType::ACTION_CTA5_DECRYPT == action)
            {
               if (DrmUtil::sDDebug) ALOGD("onAcquireDrmInfo() ACTION_CTA5_DECRYPT");
                actionResult = decryptInfR(drmInfoRequest);
            } else if (DrmRequestType::ACTION_CTA5_CANCEL == action)
            {
               if (DrmUtil::sDDebug) ALOGD("onAcquireDrmInfo() ACTION_CTA5_DECRYPT");
                actionResult = cancel(drmInfoRequest);
            }else if (DrmRequestType::ACTION_CTA5_SETKEY == action)
            {
                actionResult = setKey(drmInfoRequest);
            }else if(DrmRequestType::ACTION_CTA5_CHANGEPASSWORD == action)
            {
                if (DrmUtil::sDDebug) ALOGD("onAcquireDrmInfo() ACTION_CTA5_CHANGEPASSWORD");
                actionResult = ChangeKey(drmInfoRequest);

            }else if(DrmRequestType::ACTION_CTA5_GETPROGESS == action)
            {
                actionResult = getProgress(drmInfoRequest);
            }else if(DrmRequestType::ACTION_CTA5_ISCTAFILE == action)
            {
                actionResult = isCta5File(drmInfoRequest);
            } else
            {
                if (DrmUtil::sDDebug) ALOGD("Not valid action %s", action.string());
            }
            if (actionResult == true)
            {
                result = DrmRequestType::RESULT_SUCCESS;
            } else
            {
                result = DrmRequestType::RESULT_FAILURE;
                if (DrmUtil::sDDebug) ALOGE("[ERROR]onAcquireDrmInfo:result is false, call back");
                String8 errorMsg = DrmCtaUtil::getErrorCallbackMsg(String8("error"), DrmDef::CTA5_ERROR);
                DrmCtaUtil::notify(sJavaAPIListeners, errorMsg);

            }
        } else if (DrmRequestType::TYPE_GET_DRM_INFO == type)
        {
            if (DrmRequestType::ACTION_CTA5_GETTOKEN == action)
            {
                result = getToken(drmInfoRequest);
            } else if(DrmRequestType::ACTION_CTA5_CHECKTOKEN == action)
            {
                result = isTokenValid(drmInfoRequest) ?
                        DrmRequestType::RESULT_SUCCESS : DrmRequestType::RESULT_FAILURE;
            } else if(DrmRequestType::ACTION_CTA5_CLEARTOKEN == action)
            {
                result = clearToken(drmInfoRequest) ?
                        DrmRequestType::RESULT_SUCCESS : DrmRequestType::RESULT_FAILURE;
            }
        }

        // the result value is passed back to caller side.
        int length = result.length();
        char* data = new char[length];
        memcpy(data, result.string(), length);

        drmInfo = new DrmInfo(drmInfoRequest->getInfoType(), DrmBuffer(data, length),
                drmInfoRequest->getMimeType());
        //ALOGD("DrmCtaPlugIn hongen::onAcquireDrmInfo : mime type=[%s]", drmInfoRequest->getMimeType().string());
    }
    if (DrmUtil::sDDebug) ALOGD("DrmCtaPlugIn hongen::onAcquireDrmInfo end: ");
    return drmInfo;
}

String8 DrmCtaPlugIn::onGetOriginalMimeType(int uniqueId, const String8& path, int fd)
{
   if (DrmUtil::sDDebug) ALOGD("onGetOriginalMimeType() rui [%d],fd = %d, path = %s", uniqueId, fd, path.string());
    String8 mime;
    /*if (mKey.isEmpty())
    {
        ALOGE("[ERROR]onGetOriginalMimeType - The key is not set by user");
        return mime;
    }*/
    if (path.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]file path invalid(empty)");
        return mime;
    }
    int fd_in = open(path.string(), O_RDONLY);
    if (fd_in < 0)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]Canot open file");
        return mime;
    }
    Cta5File *pCta5File = Cta5FileFactory::createCta5File(fd_in, mKey);
    if(pCta5File == NULL)
    {
        close(fd_in);
        if (DrmUtil::sDDebug) ALOGE("[ERROR]A bad cta5 file");
        return mime;
    }
    mime = pCta5File->getOriginalMimeType();
    delete pCta5File;
    pCta5File = NULL;

    return mime;
}

int DrmCtaPlugIn::onGetDrmObjectType(int uniqueId __attribute__((unused)), const String8& path __attribute__((unused)),
                                     const String8& mimeType __attribute__((unused)))
{
    return DrmObjectType::UNKNOWN;
}

int DrmCtaPlugIn::onCheckRightsStatus(int uniqueId __attribute__((unused)),
                                      const String8& path __attribute__((unused)), int action)
{
    // For cta5 encrypt file, it can be play, display and transfer, but could not be printed
    // and setted as ringtone/wallpaper.
    switch (action) {
        case Action::PLAY:
        case Action::DISPLAY:
        case Action::TRANSFER:
        {
            return RightsStatus::RIGHTS_VALID;
        }

        case Action::RINGTONE:
        case OmaDrmAction::WALLPAPER:
        case OmaDrmAction::PRINT:
        {
            return RightsStatus::RIGHTS_INVALID;
        }

        default:
        {
           if (DrmUtil::sDDebug) ALOGD("checkRightsStatus: unsupport other actions");
            return RightsStatus::RIGHTS_INVALID;
        }
    }
}

status_t DrmCtaPlugIn::onConsumeRights(int uniqueId __attribute__((unused)),
                                       sp<DecryptHandle>& decryptHandle __attribute__((unused)),
                                       int action __attribute__((unused)),
        bool reserve __attribute__((unused)))
{
    return DRM_NO_ERROR;
}

status_t DrmCtaPlugIn::onSetPlaybackStatus(int uniqueId __attribute__((unused)),
                                           sp<DecryptHandle>& decryptHandle __attribute__((unused)),
        int playbackStatus __attribute__((unused)), int64_t position __attribute__((unused)))
{
   // if (DrmUtil::sDDebug) ALOGD("DrmCtaPlugIn::onSetPlaybackStatus playbackStatus = %d,position=%lld", playbackStatus,position);
    return DRM_NO_ERROR;
}

bool DrmCtaPlugIn::onValidateAction(int uniqueId __attribute__((unused)), const String8& path __attribute__((unused)),
                                    int action __attribute__((unused)),
        const ActionDescription& description __attribute__((unused)))
{
    return 0;
}

status_t DrmCtaPlugIn::onRemoveRights(int uniqueId __attribute__((unused)), const String8& path __attribute__((unused)))
{
    return DRM_NO_ERROR;
}

status_t DrmCtaPlugIn::onRemoveAllRights(int uniqueId __attribute__((unused)))
{
    return DRM_NO_ERROR;
}

status_t DrmCtaPlugIn::onOpenConvertSession(int uniqueId __attribute__((unused)), int convertId __attribute__((unused)))
{
    return DRM_NO_ERROR;
}

DrmConvertedStatus* DrmCtaPlugIn::onConvertData(int uniqueId __attribute__((unused)), int convertId __attribute__((unused)),
        const DrmBuffer* inputData __attribute__((unused)))
{
    return NULL;
}

DrmConvertedStatus* DrmCtaPlugIn::onCloseConvertSession(int uniqueId __attribute__((unused)), int convertId __attribute__((unused)))
{
    return NULL;
}

DrmSupportInfo* DrmCtaPlugIn::onGetSupportInfo(int uniqueId)
{
   if (DrmUtil::sDDebug) ALOGD("DrmCtaPlugIn() : onGetSupportInfo : %d", uniqueId);
    DrmSupportInfo* drmSupportInfo = new DrmSupportInfo();
    drmSupportInfo->addMimeType(DrmDef::MIME_CTA5_MESSAGE);
    // Add File Suffixes
    drmSupportInfo->addFileSuffix(DrmDef::EXT_CTA5_FILE);
    // Add plug-in description
    drmSupportInfo->setDescription(DrmDef::CTA5_PLUGIN_DESCRIPTION);
    return drmSupportInfo;
    //return NULL;
}

status_t DrmCtaPlugIn::onOpenDecryptSession(int uniqueId, sp<DecryptHandle>& decryptHandle, int fd,
        off64_t offset, off64_t length)
{
   if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() [%d], fd = %d, offset = %ld, length = %ld",
            uniqueId, fd, (long) offset, (long) length);

    if (mKey.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]onOpenDecryptSession - The key is not set by user");
        return DRM_ERROR_CANNOT_HANDLE;
    }
    status_t result = DRM_ERROR_CANNOT_HANDLE;
    if (decryptHandle)
    {

        if (fd < 0)
        {
            if (DrmUtil::sDDebug) ALOGE("[ERROR]onOpenDecryptSession() : invalid input file descriptor [%d]", fd);
            return DRM_ERROR_CANNOT_HANDLE;
        }
        if (Cta5File::isCta5File(fd) == false)
        {
            if (DrmUtil::sDDebug) ALOGE("Not a cta5 file");
            return DRM_ERROR_CANNOT_HANDLE;
        }
        // check decrypt session map
        int fileDesc = -1;
        if (!m_decryptSessionMap.isCreated(decryptHandle->decryptId))
        {
            // use the file descriptor: the original raw fd is owned by binder,
            // shall use dup() to get our own copy.
            fileDesc = fcntl(fd, F_DUPFD, 0);
            if (-1 == fileDesc)
            {
                if (DrmUtil::sDDebug) ALOGE( "[ERROR]Failed to duplicate the file descriptor, reason [%s]", strerror(errno));
                return DRM_ERROR_CANNOT_HANDLE;
            }
        } else
        {
            if (DrmUtil::sDDebug) ALOGE("[ERROR]onOpenDecryptSession() : decrypt session map error.");
            return DRM_ERROR_CANNOT_HANDLE;
        }
        String8 pid_str = decryptHandle->extendedData.valueFor(String8("ClientPid"));
       if (DrmUtil::sDDebug) ALOGD("pid_str = %s", pid_str.string());
        pid_t pid = -1;
        sscanf(pid_str.string(), "%d", &pid);
       if (DrmUtil::sDDebug) ALOGD("pid = %d", pid);
        String8 processName = DrmMtkUtil::getProcessName(pid);
        bool isTrustClient = DrmCtaUtil::isTrustCtaClient(processName);
       if (DrmUtil::sDDebug) ALOGD("isTrustClient = %d", isTrustClient);
        if(isTrustClient == false)
        {
            if (DrmUtil::sDDebug) ALOGE("[ERROR]Not a trusted client");
            return DRM_ERROR_CANNOT_HANDLE;
        }

        // DataProtection will call opendecryptsession to decode thumbnail, not to check token
        // Some time it will happen the file path get from fd is difference from the one which
        // app use to get token, we need handle this case, convert the path and check again
        if (processName != "com.mediatek.dataprotection")
        {
            String8 filePath = DrmCtaUtil::getPathFromFd(fd);
            filePath = String8("file://") + filePath;
            String8 mntMediaPath = String8("file:///mnt/media_rw");
            String8 dataMediaPath = String8("file:///data/media");
            ssize_t index = mTokenCheckedMap.indexOfKey(filePath);
            // if check fail, convert path and check again
            if (index < 0 || index >= mTokenCheckedMap.size()) {
                if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession check token don't find path:%s, convert path and try again",
                        filePath.string());
                if (0 == strncasecmp(filePath.string(), mntMediaPath.string(), mntMediaPath.length())) {
                    filePath = String8("file:///storage") + String8(filePath.string() + mntMediaPath.length());
                    index = mTokenCheckedMap.indexOfKey(filePath);
                } else if (0 == strncasecmp(filePath.string(), dataMediaPath.string(), dataMediaPath.length())) {
                    filePath = String8("file:///storage/emulated") + String8(filePath.string()
                            + dataMediaPath.length());
                    index = mTokenCheckedMap.indexOfKey(filePath);
                }
            }
            if (index < 0 || index >= mTokenCheckedMap.size()) {
                if (DrmUtil::sDDebug) ALOGE("onOpenDecryptSession check token don't find path:%s", filePath.string());
                return DRM_ERROR_CANNOT_HANDLE;
            }

        }

        Cta5File *pCta5File = Cta5FileFactory::createCta5File(fileDesc, mKey);
        if(pCta5File == NULL)
        {
            close(fileDesc);
            if (DrmUtil::sDDebug) ALOGE("[ERROR]Not a cta5 file, create file failed");
            return DRM_ERROR_CANNOT_HANDLE;
        }

        if (pCta5File->isKeyValid(mKey) == false)
        {
            if (DrmUtil::sDDebug) ALOGE("[ERROR]Bad key");
            delete pCta5File;
            pCta5File = NULL;
            //TODO return a special error
            //CTA_ERROR_BAD_KEY
            return DRM_ERROR_CANNOT_HANDLE;
        }

        decryptHandle->mimeType = pCta5File->getMimeType();
        decryptHandle->decryptApiType = DecryptApiType::CONTAINER_BASED;
        decryptHandle->status = RightsStatus::RIGHTS_VALID;
        decryptHandle->decryptInfo = new DecryptInfo();
        decryptHandle->decryptInfo->decryptBufferLength = pCta5File->getContentLen();
//        lseek(fd, 0, SEEK_SET);
//        uint64_t totalSize = lseek(fd, 0, SEEK_END);
//        decryptHandle->decryptInfo->decryptBufferLength = totalSize - pCta5File->getHeader().length();
        // add to decrypt session map
        DecryptSession* pDecryptSession = new DecryptSession(pCta5File);
        m_decryptSessionMap.addValue(decryptHandle->decryptId, pDecryptSession);
        result = DRM_NO_ERROR;
    } else
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]onOpenDecryptSession() : invalid decryptHandle [%p]", decryptHandle.get());
    }
   if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : result [%d]", result);
    return result;
}

status_t DrmCtaPlugIn::onOpenDecryptSession(int uniqueId __attribute__((unused)),
                                            sp<DecryptHandle>& decryptHandle __attribute__((unused)),
        const char* uri)
{
   if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() with uri return DRM_ERROR_CANNOT_HANDLE");
    if(uri)
    {
       if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession uri = %s",uri);
    }
    return DRM_ERROR_CANNOT_HANDLE;
}

status_t DrmCtaPlugIn::onCloseDecryptSession(int uniqueId __attribute__((unused)), sp<DecryptHandle>& decryptHandle)
{
    if (decryptHandle)
    {
        m_decryptSessionMap.removeValue(decryptHandle->decryptId);
        if (NULL != decryptHandle->decryptInfo)
        {
            delete decryptHandle->decryptInfo;
            decryptHandle->decryptInfo = NULL;
        }
        decryptHandle = NULL;
    }
    return DRM_NO_ERROR;
}

status_t DrmCtaPlugIn::onInitializeDecryptUnit(int uniqueId __attribute__((unused)),
                                               sp<DecryptHandle>& decryptHandle __attribute__((unused)),
        int decryptUnitId __attribute__((unused)), const DrmBuffer* headerInfo __attribute__((unused)))
{
    return DRM_NO_ERROR;
}

status_t DrmCtaPlugIn::onDecrypt(int uniqueId __attribute__((unused)), sp<DecryptHandle>& decryptHandle __attribute__((unused)),
                                 int decryptUnitId __attribute__((unused)), const DrmBuffer* encBuffer __attribute__((unused)),
                                 DrmBuffer** decBuffer __attribute__((unused)), DrmBuffer* IV __attribute__((unused)))
{
    return DRM_NO_ERROR;
}

status_t DrmCtaPlugIn::onFinalizeDecryptUnit(int uniqueId __attribute__((unused)),
                                             sp<DecryptHandle>& decryptHandle __attribute__((unused)),
        int decryptUnitId __attribute__((unused)))
{
    return DRM_NO_ERROR;
}

ssize_t DrmCtaPlugIn::onPread(int uniqueId __attribute__((unused)), sp<DecryptHandle>& decryptHandle, void* buffer,
        ssize_t numBytes, off64_t offset)
{
    if (DrmUtil::sDDebug) ALOGV("onPread() hongen : enter--.");
    ssize_t bytesRead = -1; // returns the actual bytes read.
    if (decryptHandle && NULL != buffer && numBytes > -1 && offset > -1)
    {
        DecryptSession* pDecryptSession = m_decryptSessionMap.getValue(decryptHandle->decryptId);
        if (NULL != pDecryptSession)
        {
            Cta5File *pCta5File = pDecryptSession->pCta5File;
            if (pCta5File != NULL)
            {
                bytesRead = pCta5File->pread(buffer, numBytes, offset);
            } else
            {
                if (DrmUtil::sDDebug) ALOGE("[ERROR]onPread() : error, pCta5File is NULL.");
            }
        } else
        {
            if (DrmUtil::sDDebug) ALOGE("[ERROR]onPread() : error, invalid decrypt session.");
        }
    } else
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]onPread() error, parameter invalid");
    }

    if (DrmUtil::sDDebug) ALOGV("onPread() hongen : enter--.bytesRead = %d", (int) bytesRead);

    return bytesRead;
}

bool DrmCtaPlugIn::encrypt(const DrmInfoRequest *drmInfoRequest)
{
    if(mKey.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]encrypt - The key is not set by user");
        return false;
    }
    String8 file_mime = drmInfoRequest->get(String8("CTA5rawMime"));
    String8 clear_fd_str = drmInfoRequest->get(String8("CTA5clearFd"));
    String8 cipher_fd_str = drmInfoRequest->get(String8("CTA5cipherFd"));
    String8 key_str = drmInfoRequest->get(String8("CTA5key"));
    int clear_fd = -1;
    int cipher_fd = -1;
    sscanf(clear_fd_str.string(), "%d", &clear_fd);
    sscanf(cipher_fd_str.string(), "%d", &cipher_fd);
   // ALOGD( "clear_fd_str : %s, cipher_fd_str : %s", clear_fd_str.string(), cipher_fd_str.string());
   // ALOGD("clear_fd : %d, cipher_fd : %d", clear_fd, cipher_fd);
    // content length

    if (-1 == lseek(clear_fd, 0, SEEK_SET))
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]encrypt() : failed to seek set clear_fd, reason [%s]", strerror(errno));
        return false;
    }
    int contentLength = lseek(clear_fd, 0, SEEK_END);
    if (-1 == contentLength)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]encrypt() : failed to seek end clear_fd, reason [%s]", strerror(errno));
        return false;
    }
   // ALOGD("hongen encrypt :content length: [%ld]", contentLength);
    int dupClearFd = dup(clear_fd);
    if (-1 == dupClearFd)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]encrypt() : failed to dup clear_fd, reason [%s]", strerror(errno));
        return false;
    }
    int dupCipherFd = dup(cipher_fd);
    if (-1 == dupCipherFd)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]encrypt() : failed to dup cipher_fd, reason [%s]", strerror(errno));
        close(dupClearFd);
        return false;
    }
    // if (DrmUtil::sDDebug) ALOGD("encrypt mKey = %s",mKey.string());
    bool isDcfFile = Cta5FileUtil::isDcfFile(dupClearFd);
    Cta5File *pCta5File = Cta5FileFactory::createCta5File(file_mime, contentLength, mKey, isDcfFile);
    if(pCta5File == NULL)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]Create Cta5 file failed");
        close(dupClearFd);
        close(dupCipherFd);
        return false;
    }
    pCta5File->setClearFd(dupClearFd);
    pCta5File->setCipherFd(dupCipherFd);
    {
        Mutex::Autolock lock(mEncryptMutex);
        mAvailEncryptFiles.push_back(pCta5File);
    }

    sem_post(&mAvailEncryptSem);
    return true;
}

bool DrmCtaPlugIn::decryptInfR(const DrmInfoRequest *drmInfoRequest)
{
    pid_t callerPid = IPCThreadState::self()->getCallingPid();
    String8 processName = DrmMtkUtil::getProcessName(callerPid);
    bool isTrustedClient = DrmCtaUtil::IsCtaTrustedGetTokenClient(processName);
    if (isTrustedClient == false)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]no rights to execute decrypt");
        return false;
    }
    String8 key_str = drmInfoRequest->get(String8("CTA5key"));

    if (mKey.isEmpty() && key_str.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]decrypt - The key is not set by user");
        return false;
    }
    //ALOGD("EYES %s mKey = %s, key_str = %s", __FUNCTION__, mKey.string(), key_str.string());
    String8 clear_fd_str = drmInfoRequest->get(String8("CTA5clearFd"));
    String8 cipher_fd_str = drmInfoRequest->get(String8("CTA5cipherFd"));
    int clear_fd = -1;
    int cipher_fd = -1;
    sscanf(clear_fd_str.string(), "%d", &clear_fd);
    sscanf(cipher_fd_str.string(), "%d", &cipher_fd);
    //ALOGD( "clear_fd_str %s, cipher_fd_str : %s", clear_fd_str.string(), cipher_fd_str.string());
    //ALOGD("clear_fd  %d, cipher_fd : %d", clear_fd, cipher_fd);

    int dupClearFd = dup(clear_fd);
    if (dupClearFd == -1)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]decrypt - dup clear_fd failed, because %s", strerror(errno));
        return false;
    }
    int dupCipherFd = dup(cipher_fd);
    if (dupCipherFd == -1)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]decrypt - dup cipher_fd failed, because %s", strerror(errno));
        close(dupClearFd);
        return false;
    }
   // ALOGD("dupClearFd : %d, dupCipherFd : %d", dupClearFd, dupCipherFd);
    Cta5File *pCta5File = NULL;
    if(!key_str.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGD("Use the key provide by user");
        pCta5File = Cta5FileFactory::createCta5File(dupCipherFd, key_str);
    }else
    {
        pCta5File = Cta5FileFactory::createCta5File(dupCipherFd, mKey);
        key_str = mKey;
    }
    if(pCta5File == NULL)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]Create Cta5 file failed");
        close(dupClearFd);
        close(dupCipherFd);
        return false;
    }
    pCta5File->setClearFd(dupClearFd);
    pCta5File->setCipherFd(dupCipherFd);
    if (!pCta5File->isKeyValid(key_str))
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]decrypt() key is invalid.");
        String8 path = Cta5FileUtil::getPathFromFd(pCta5File->getCipherFd());

        String8 r("data_s:0::cnt_s:0::path:");
        r.append(path);
        r.append("::result:");
        r.append(DrmDef::CTA5_KEY_ERROR);
        pCta5File->notify(sJavaAPIListeners, r);
        delete pCta5File;
        pCta5File = NULL;
        return true;
    }
    {
        Mutex::Autolock lock(mDecryptMutex);
        mAvailDecryptFiles.push_back(pCta5File);
    }

    sem_post(&mAvailDecryptSem);
    return true;
}

bool DrmCtaPlugIn::cancel(const DrmInfoRequest *drmInfoRequest)
{
    String8 fd_str = drmInfoRequest->get(String8("CTA5Fd"));
    bool canceled = false;
    int fd = -1;
    sscanf(fd_str.string(), "%d", &fd);
    String8 cancelPath = Cta5FileUtil::getPathFromFd(fd);
    String8 ctaPath("");
   if (DrmUtil::sDDebug) ALOGD( "hongen cancel filePath:%s", cancelPath.string());
    //check decrypt vector
    if (canceled == false)
    {
        Mutex::Autolock lock(mDecryptMutex);
        for (Vector<Cta5File *>::iterator iter = mAvailDecryptFiles.begin();
                iter != mAvailDecryptFiles.end(); ++iter)
        {
            Cta5File *PCta5File = (*iter);
            if (Cta5FileUtil::getPathFromFd(PCta5File->getCipherFd()) == cancelPath)
            {
               if (DrmUtil::sDDebug) ALOGD("cancel:hongen found decrypt file");
                PCta5File->cancel();
                canceled = true;
                return true;
            }
        }
    }

    //check encrypt vector
    if (canceled == false)
    {
        Mutex::Autolock lock(mEncryptMutex);
        for (Vector<Cta5File *>::iterator iter = mAvailEncryptFiles.begin();
                iter != mAvailEncryptFiles.end(); ++iter)
        {
            Cta5File *PCta5File = (*iter);
            //ALOGD( "hongen cancel encrypt, orin path:%s", Cta5FileUtil::getPathFromFd(PCta5File->getClearFd()).string());
            if (Cta5FileUtil::getPathFromFd(PCta5File->getClearFd()) == cancelPath)
            {
                if (DrmUtil::sDDebug) ALOGD("cancel:hongen found encrypt file");
                PCta5File->cancel();
                canceled = true;
                return true;
            }
        }
    }

    if (DrmUtil::sDDebug) ALOGW("hongen cancel no file found in vector.");
    return false;
}

bool DrmCtaPlugIn::setKey(const DrmInfoRequest *drmInfoRequest)
{
    pid_t callerPid = IPCThreadState::self()->getCallingPid();
    String8 processName = DrmMtkUtil::getProcessName(callerPid);
    bool isTrustedClient = DrmCtaUtil::IsCtaTrustedGetTokenClient(processName);
    if (isTrustedClient == false)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]no rights to execute setKey");
        return false;
    }
    mKey = drmInfoRequest->get(String8("CTA5key"));
    //ALOGD("key length = %d", mKey.length());
    return true;
}

bool DrmCtaPlugIn::ChangeKey(const DrmInfoRequest *drmInfoRequest)
{
    if (DrmUtil::sDDebug) ALOGD("ChangeKey");
    if (mKey.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]ChangeKey - The key is not set by user");
        return false;
    }
    pid_t callerPid = IPCThreadState::self()->getCallingPid();
    String8 processName = DrmMtkUtil::getProcessName(callerPid);
    bool isTrustedClient = DrmCtaUtil::IsCtaTrustedGetTokenClient(processName);
    if (isTrustedClient == false)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]no rights to execute ChangeKey");
        return false;
    }
    String8 oldKey = drmInfoRequest->get(String8("CTA5oldKey"));
    String8 newKey = drmInfoRequest->get(String8("CTA5newKey"));

    String8 fd_str = drmInfoRequest->get(String8("CTA5Fd"));
    int fd = -1;
    sscanf(fd_str.string(), "%d", &fd);
    if(fd == -1)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]ChangeKey Bad fd");
        return false;
    }

    if (Cta5File::isCta5File(fd) == false)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]ChangeKey Not a cta5 file");
        return false;
    }

    Cta5File *pCta5File = Cta5FileFactory::createCta5File(fd, oldKey);
    if (pCta5File == NULL)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]Not a cta5 file, create file failed");
        return false;
    }

    if (pCta5File->isKeyValid(oldKey) == false)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]ChangeKey Bad key");
        delete pCta5File;
        pCta5File = NULL;
        return false;
    }
    pCta5File->changeKey(oldKey, newKey);
    this->mKey = newKey;

    delete pCta5File;
    pCta5File = NULL;
    return true;
}

bool DrmCtaPlugIn::isCta5File(const DrmInfoRequest *drmInfoRequest)
{
    if (DrmUtil::sDDebug) ALOGD("isCta5File");
    if (mKey.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]isCta5File - The key is not set by user");
        return false;
    }
    String8 fd_str = drmInfoRequest->get(String8("CTA5Fd"));
    String8 path_str = drmInfoRequest->get(DrmRequestType::KEY_CTA5_FILEPATH);
    int path_in = -1;
    int fd_in = -1;
    path_in = open(path_str.string(), O_RDONLY);
    sscanf(fd_str.string(), "%d", &fd_in);
    if ((fd_in == -1) && (path_in == -1))
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]isCta5File Bad fd");
        return false;
    }
    int fd = (fd_in == -1) ? path_in : fd_in;
    bool result = Cta5File::isCta5File(fd);
    if (path_in != -1)
    {
        close(path_in);
    } else {
        // maybe open file failed, but fd_in is ok, it will go to here.
        if (DrmUtil::sDDebug) ALOGW("[Warning] open file[%s] failed, reason: %s", path_str.string(), strerror(errno));
    }
    return result;

}

bool DrmCtaPlugIn::getProgress(const DrmInfoRequest *drmInfoRequest)
{
    if (DrmUtil::sDDebug) ALOGD("getProgress");
    String8 fd_str = drmInfoRequest->get(String8("CTA5Fd"));
    int fd = -1;
    sscanf(fd_str.string(), "%d", &fd);
    if (fd == -1)
    {
        if (DrmUtil::sDDebug) ALOGE("getProgress Bad fd");
        return false;
    }
    return true;
}

bool DrmCtaPlugIn::createEncryptThread()
{
    if (mIsEncryptThreadCreated == true)
    {
        if (DrmUtil::sDDebug) ALOGV("encrypt thread has being created");
        return true;
    }
    int ret = pthread_create(&mEncryptThreadId, NULL, encryptThread, this);
    if (DrmUtil::sDDebug) ALOGD("createEncryptThread pid = %ld", mEncryptThreadId);
    if (ret)
    {
        mIsEncryptThreadCreated = false;
        if (DrmUtil::sDDebug) ALOGE("[ERROR]createEncryptThread() : createEncryptThread error =[%d]", ret);
        return false;
    }
    mIsEncryptThreadCreated = true;
    return true;
}

bool DrmCtaPlugIn::createDecryptThread()
{
    if (mIsDecryptThreadCreated == true)
    {
        if (DrmUtil::sDDebug) ALOGV("decrypt thread has being created");
        return true;
    }
    int ret = pthread_create(&mDecryptThreadId, NULL, decryptThread, this);
    if (DrmUtil::sDDebug) ALOGD("createDecryptThread pid = %ld", mDecryptThreadId);
    if (ret)
    {
        mIsDecryptThreadCreated = false;
        if (DrmUtil::sDDebug) ALOGE("[ERROR]createDecryptThread() : createDecryptThread error =[%d]", ret);
        return false;
    }
    mIsDecryptThreadCreated = true;
    return true;
}

String8 DrmCtaPlugIn::getToken(const DrmInfoRequest *drmInfoRequest)
{
    if (DrmUtil::sDDebug) ALOGD("getToken");
    String8 result("error");
    pid_t callerPid = IPCThreadState::self()->getCallingPid();
    String8 processName = DrmMtkUtil::getProcessName(callerPid);
    bool isTrustedClient = DrmCtaUtil::IsCtaTrustedGetTokenClient(processName);
    if (isTrustedClient == false)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]no rights to execute getToken");
        return result;
    }
    if (mKey.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]getToken - The key is not set by user");
        return result;
    }
    String8 path = drmInfoRequest->get(DrmRequestType::KEY_CTA5_FILEPATH);
    if (path.length() <= 0)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]getToken Bad file path:%s",path.string());
        return result;
    }
    int max = 90000;
    int min = 10000;
    int target = 1;
    srand (time(NULL));
    target = rand() % (max - min + 1) + min;

    char cnt_size_str[32] = { 0 };
    snprintf(cnt_size_str, sizeof(cnt_size_str), "%d", target);
    String8 cnt_str8("");
    result = cnt_size_str;
    if (DrmUtil::sDDebug) ALOGD("getToken file file path:%s",path.string());
    mTokenMap->add(path,result);
    return result;
}

bool DrmCtaPlugIn::isTokenValid(const DrmInfoRequest *drmInfoRequest)
{
    if (DrmUtil::sDDebug) ALOGD("isTokenValid");
    bool result = false;
    pid_t callerPid = IPCThreadState::self()->getCallingPid();
    String8 processName = DrmMtkUtil::getProcessName(callerPid);
    bool isTrustedClient = DrmCtaUtil::IsCtaTrustedCheckTokenClient(processName);
    if (isTrustedClient == false)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]isTokenValid - Not a trusted client: %s",processName.string() );
        return false;
    }
    if (mKey.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]isTokenValid - The key is not set by user");
        return false;
    }
    String8 path = drmInfoRequest->get(DrmRequestType::KEY_CTA5_FILEPATH);
    if (path.length() <= 0)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]isTokenVlid Bad file path");
        return result;
    }
    String8 token = drmInfoRequest->get(DrmRequestType::KEY_CTA5_TOKEN);

    ssize_t index = mTokenMap->indexOfKey(path);
    if (index >= 0)
    {
        String8 valueget = mTokenMap->valueAt(index);
        //mTokenMap->removeItemsAt(index);
        result = (valueget == token);
    }else
    {
        if (DrmUtil::sDDebug) ALOGE("isTokenVlid: can not find fd or token is error");
        return false;
    }
    // add checked path to mTokenCheckedMap
    if (result == true)
    {
        //std::map<String8, String8>::value_type item(path, processName);
        mTokenCheckedMap.add(path, processName);
    }

    return result;
}

bool DrmCtaPlugIn::clearToken(const DrmInfoRequest *drmInfoRequest)
{
    if (DrmUtil::sDDebug) ALOGD("clearToken");
    bool result = false;
    pid_t callerPid = IPCThreadState::self()->getCallingPid();
    String8 processName = DrmMtkUtil::getProcessName(callerPid);
    bool isTrustedClient = DrmCtaUtil::isTrustCtaClient(processName);
    if (isTrustedClient == false)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]clearToken - Not a trusted client: %s", processName.string());
        return false;
    }
    if (mKey.isEmpty())
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]clearToken - The key is not set by user");
        return false;
    }
    String8 path = drmInfoRequest->get(DrmRequestType::KEY_CTA5_FILEPATH);
    if (DrmUtil::sDDebug) ALOGD("hongen clearToken file file path:%s",path.string());
    if (path.length() <= 0)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]clearToken Bad file path");
        return result;
    }
    // clear token
    String8 token = drmInfoRequest->get(DrmRequestType::KEY_CTA5_TOKEN);
    if (DrmUtil::sDDebug) ALOGD("hongen clearToken file token:%s",token.string());
    ssize_t index = mTokenMap->indexOfKey(path);
    if (index >= 0)
    {
        String8 valueget = mTokenMap->valueAt(index);
        if (valueget == token)
        {
            mTokenMap->removeItemsAt(index);
            result = true;
        }
        if (DrmUtil::sDDebug) ALOGD("hongen valueget 1 is:%s",valueget.string());

    } else
    {
        if (DrmUtil::sDDebug) ALOGE("clearToken: can not find fd or token is error");
        result = false;
    }

    // clear checked map
    //std::map<String8, String8>::iterator it = mTokenCheckedMap.find(path);
    ssize_t indexInCheckedMap = mTokenCheckedMap.indexOfKey(path);
    if (indexInCheckedMap > -1 && indexInCheckedMap < mTokenCheckedMap.size())
    {
        if (DrmUtil::sDDebug) ALOGD("hongen delete checkedmap path:%s", path.string());
        mTokenCheckedMap.removeItem(path);
        result = true;
    } else
    {
        if (DrmUtil::sDDebug) ALOGE("clearToken: can not find file path or token is error");
        result = false;
    }

    return result;
}

namespace android
{
void * encryptThread(void *arg)
{
    DrmCtaPlugIn *plugin = (DrmCtaPlugIn *) arg;
    if (arg == NULL)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]create encryptThread fail, arg is null");
        return NULL;
    }
    //Now we can call every member function in DrmCtaPlugIn
    while (true)
    {
        if (DrmUtil::sDDebug) ALOGD("Wait to encrypt");
        sem_wait(&(plugin->mAvailEncryptSem));
        if (DrmUtil::sDDebug) ALOGD("do encrypt");
        if (plugin->mEncryptThreadNeedCanceled == true)
        {
            if (DrmUtil::sDDebug) ALOGD("encryptThread Exit(cancled)");
            return NULL;
        }
        //Do something
        Cta5File *PCta5File = NULL;
        {
            Mutex::Autolock lock(plugin->mEncryptMutex);
            PCta5File = plugin->mAvailEncryptFiles.top();
        }

        if (PCta5File != NULL)
        {
            bool result = PCta5File->encrypt(PCta5File->getClearFd(), PCta5File->getCipherFd(),
                    plugin->sJavaAPIListeners);
            if (!result)
            {
                if (DrmUtil::sDDebug) ALOGE("[ERROR]encryptThread:result is false.");
                String8 errorMsg = DrmCtaUtil::getErrorCallbackMsg(String8("error"), DrmDef::CTA5_ERROR);
                DrmCtaUtil::notify(plugin->sJavaAPIListeners, errorMsg);
            }
        }
        {
            Mutex::Autolock lock(plugin->mEncryptMutex);
            // check file is canceled, after file encrypt is done
            if (PCta5File->isNeedCancel())
            {
                if (DrmUtil::sDDebug) ALOGD("hongen cancel after encrypt is done");
                String8 path = Cta5FileUtil::getPathFromFd(PCta5File->getClearFd());
                String8 r("data_s:0::cnt_s:0::path:");
                r.append(path);
                r.append("::result:");
                r.append(DrmDef::CTA5_CANCEL_DONE);
                PCta5File->notify(plugin->sJavaAPIListeners, r);
            }
            plugin->mAvailEncryptFiles.erase(plugin->mAvailEncryptFiles.begin());
        }
        delete PCta5File;
        PCta5File = NULL;
    }
    if (DrmUtil::sDDebug) ALOGD("encryptThread Exit");
    return NULL;
}

void * decryptThread(void *arg)
{
    DrmCtaPlugIn *plugin = (DrmCtaPlugIn *) arg;
    if (arg == NULL)
    {
        if (DrmUtil::sDDebug) ALOGE("[ERROR]create decryptThread fail, arg is null");
        return NULL;
    }
    //Now we can call every member function in DrmCtaPlugIn
    while (true)
    {
        if (DrmUtil::sDDebug) ALOGD("Wait to decrypt");
        sem_wait(&(plugin->mAvailDecryptSem));
        if (DrmUtil::sDDebug) ALOGD("do decrypt");
        if (plugin->mDecryptThreadNeedCanceled == true)
        {
            if (DrmUtil::sDDebug) ALOGD("decryptThread Exit(cancled)");
            return NULL;
        }
        //Do something
        Cta5File *PCta5File = NULL;
        {
            Mutex::Autolock lock(plugin->mEncryptMutex);
            PCta5File = plugin->mAvailDecryptFiles.top();
        }
        if (PCta5File != NULL)
        {
            bool result = PCta5File->decrypt(PCta5File->getClearFd(), plugin->sJavaAPIListeners);
            if (!result)
            {
                if (DrmUtil::sDDebug) ALOGE("[ERROR]decryptThread:result is false.");
                String8 errorMsg = DrmCtaUtil::getErrorCallbackMsg(String8("error"), DrmDef::CTA5_ERROR);
                DrmCtaUtil::notify(plugin->sJavaAPIListeners, errorMsg);
            }
        }
        //remove it from vector
        {
            Mutex::Autolock lock(plugin->mEncryptMutex);
            // check file is canceled, after file encrypt is done
            if (PCta5File->isNeedCancel())
            {
                if (DrmUtil::sDDebug) ALOGD("hongen cancel after decrypt is done");
                String8 path = Cta5FileUtil::getPathFromFd(PCta5File->getCipherFd());
                String8 r("data_s:0::cnt_s:0::path:");
                r.append(path);
                r.append("::result:");
                r.append(DrmDef::CTA5_CANCEL_DONE);
                PCta5File->notify(plugin->sJavaAPIListeners, r);
            }
            plugin->mAvailDecryptFiles.erase(plugin->mAvailDecryptFiles.begin());
        }

        delete PCta5File;
        PCta5File = NULL;
    }
    ALOGD("encryptThread Exit");
    return NULL;
}
} //namespace
