/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "StereoFDProxy"

#include <mtkcam3/feature/stereo/hal/stereo_fd_proxy.h>
#include <stdlib.h> //abs

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNC_START          MY_LOGD_IF(LOG_ENABLED, "+")
#define FUNC_END            MY_LOGD_IF(LOG_ENABLED, "-")

using namespace android;
using namespace NSCam;

Mutex StereoFDProxy::__faceLock;
std::vector<MtkCameraFace> StereoFDProxy::__faces;
Mutex StereoFDProxy::__focusLock;
MtkCameraFace StereoFDProxy::__focusedFace;

void
StereoFDProxy::setFaces(MtkCameraFaceMetadata *fdResult)
{
    Mutex::Autolock lock(__faceLock);
    __faces.clear();
    if(NULL == fdResult)
    {
        return;
    }

    for(int i = 0; i < fdResult->number_of_faces; ++i) {
        __faces.push_back(fdResult->faces[i]);
    }
}

std::vector<MtkCameraFace>
StereoFDProxy::getFaces()
{
    Mutex::Autolock lock(__faceLock);
    return __faces;
}

void
StereoFDProxy::setFocusedFace(MtkCameraFace &face)
{
    Mutex::Autolock lock(__focusLock);
    __focusedFace = face;
}

MtkCameraFace
StereoFDProxy::getFocusedFace()
{
    Mutex::Autolock lock(__focusLock);
    return __focusedFace;
}

void
StereoFDProxy::resetFocusedFace()
{
    Mutex::Autolock lock(__focusLock);
    ::memset(&__focusedFace, 0, sizeof(__focusedFace));
}

bool
StereoFDProxy::isFocusedOnFace()
{
    Mutex::Autolock lock(__focusLock);
    return (__focusedFace.rect[0] != 0 &&
            __focusedFace.rect[1] != 0 &&
            __focusedFace.rect[2] != 0 &&
            __focusedFace.rect[3] != 0);
}

MRect
StereoFDProxy::faceToMRect(MtkCameraFace &face)
{
    MRect result;

    //MtkCameraFace::rect is left, top, right, bottom, range is -1000~1000
    result.p.x = face.rect[0];
    result.p.y = face.rect[1];
    result.s.w = abs(face.rect[2]-face.rect[0]);
    result.s.h = abs(face.rect[3]-face.rect[1]);

    return result;
}

MRect
StereoFDProxy::getFocusedFaceRect()
{
    Mutex::Autolock lock(__focusLock);
    return faceToMRect(__focusedFace);
}

void
StereoFDProxy::reset()
{
    Mutex::Autolock lock(__faceLock);
    __faces.clear();
}
