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
#ifndef _STEREO_FD_PROXY_H_
#define _STEREO_FD_PROXY_H_

#include <vector>
#include <utils/Mutex.h>
#include <faces.h>  //for MtkCameraFaceMetadata
#include <mtkcam/def/common.h>

class StereoFDProxy
{
public:
    /**
     * \brief Set FD result from FDClient for FF lens in GF
     *
     * \param faces face list
     */
    static void setFaces(MtkCameraFaceMetadata *faces);

    /**
     * \brief Get FD results
     * \details FD result are sorted by the distance to center by default
     * \return FD result
     */
    static std::vector<MtkCameraFace> getFaces();

    /**
     * \brief Set the face which is used to focus for FF lens
     * \details If size is 0x0, then focus is not from FD
     *
     * \param face The face used as focus point
     */
    static void setFocusedFace(MtkCameraFace &face);

    /**
     * \brief Get focused face for FF lens
     * \details Size will be 0x0 if no face is focused
     * \return Focused face
     */
    static MtkCameraFace getFocusedFace();

    /**
     * \brief If not focused on face, need to reset focused face
     * \details Size will set to 0x0
     */
    static void resetFocusedFace();

    /**
     * \brief Check if it is focused on face
     * \return true if it is focused on face
     */
    static bool isFocusedOnFace();

    /**
     * \brief Face to MRect
     * \details Transfer (left, top, right, bottom) to (x, y) wxh in [-1000, 1000] domain
     *
     * \param face Face to transform
     * \return rectangle
     */
    static NSCam::MRect faceToMRect(MtkCameraFace &face);

    /**
     * \brief Get rect of focused face in [-1000, 1000] domain
     * \details If no face is focuesd, return with size of 0x0
     * \return rect of focused face in [-1000, 1000] domain
     */
    static NSCam::MRect getFocusedFaceRect();

    /**
     * \brief Reset to release memory
     */
    static void reset();

private:
    static android::Mutex               __faceLock;
    static std::vector<MtkCameraFace>   __faces;

    static android::Mutex               __focusLock;
    static MtkCameraFace                __focusedFace;
};

#endif  // _STEREO_FD_PROXY_H_
