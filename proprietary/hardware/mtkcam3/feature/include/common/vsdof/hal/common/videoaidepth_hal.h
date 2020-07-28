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
#ifndef VIDEO_AI_DEPTH_HAL_H_
#define VIDEO_AI_DEPTH_HAL_H_

#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>

namespace StereoHAL {

struct VIDEO_AIDEPTH_HAL_PARAMS
{
    int                 mainCamPos     = 0;    //From StereoSettingProvider::getSensorRelativePosition(), 0 is lhs
    NSCam::IImageBuffer *imageMain1    = NULL;
    NSCam::IImageBuffer *imageMain2    = NULL;
    NSCam::IImageBuffer *disparityMap  = NULL;
    NSCam::IImageBuffer *confidenceMap = NULL;
    int                 iso            = 0;
    int64_t             exposureTime   = 0;

    //Dump buffers & settings
    NSCam::TuningUtils::FILE_DUMP_NAMING_HINT *dumpHint = NULL;
    int dumpLevel = 2;
};

struct VIDEO_AIDEPTH_HAL_OUTPUT
{
    NSCam::IImageBuffer *depthMap = NULL;
};

class VIDEO_AIDEPTH_HAL
{
public:
    /**
     * \brief Create a new instance
     * \details Create instance, callers should delete them after used
     * \return A new instance
     */
    static VIDEO_AIDEPTH_HAL *createInstance();

    /**
     * \brief Default destructor
     * \details Default destructor, callers should delete them after used
     */
    virtual ~VIDEO_AIDEPTH_HAL() {}

    /**
     * \brief Run Video AIDepth algorithm
     * \details Run Video AIDepth algorithm
     *
     * \param param Video AIDepth input parameter
     * \param output Video AIDepth output parameter
     *
     * \return True if success
     */
    virtual bool VideoAIDepthHALRun(VIDEO_AIDEPTH_HAL_PARAMS &param, VIDEO_AIDEPTH_HAL_OUTPUT &output) = 0;
protected:
    VIDEO_AIDEPTH_HAL() {}
};

};  //namespace StereoHAL
#endif