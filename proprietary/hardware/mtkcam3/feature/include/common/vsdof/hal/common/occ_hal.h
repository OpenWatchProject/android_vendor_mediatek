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
#ifndef OCC_HAL_H_
#define OCC_HAL_H_

#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>

namespace StereoHAL {

struct OCC_HAL_PARAMS
{
    ENUM_STEREO_SCENARIO eScenario        = eSTEREO_SCENARIO_PREVIEW;

    NSCam::IImageBuffer     *imageMain1           = NULL;   //YV12/Y-only, output of N3D HAL
    NSCam::IImageBuffer     *imageMain1Bayer      = NULL;   //YV12, color version of imageMain1 if imageMain1 is Y-only
    NSCam::IImageBuffer     *imageMain2           = NULL;   //YV12, output of N3D HAL
    MUINT16                 *disparityLeftToRight = NULL;   //16-bit, output of DPE
    MUINT16                 *disparityRightToLEft = NULL;   //16-bit, output of DPE
    NSCam::IImageBuffer     *ldcMain1             = NULL;   //8-bit, output of N3D HAL
    NSCam::IImageBuffer     *confidenceMap        = NULL;   //8-bit, output of DPE
    MFLOAT                  cOffset               = 0.0f;

    MINT32                  requestNumber         = 0;  //deprecated, use dumpHint instead
    time_t                  timestamp             = 0;  //deprecated, use dumpHint instead

    NSCam::TuningUtils::FILE_DUMP_NAMING_HINT *dumpHint = NULL;
};

struct OCC_HAL_OUTPUT
{
    NSCam::IImageBuffer *downScaledImg = NULL;  //YV12      MY_S
    NSCam::IImageBuffer *depthMap      = NULL;  //Y only    DMH
    NSCam::IImageBuffer *occMap        = NULL;  //Y only
    NSCam::IImageBuffer *nocMap        = NULL;  //Y only
};

class OCC_HAL
{
public:
    /**
     * \brief Create a new instance
     * \details Create instance, callers should delete them after used
     * \return A new instance
     */
    static OCC_HAL *createInstance();

    /**
     * \brief Default destructor
     * \details Default destructor, callers should delete them after used
     */
    virtual ~OCC_HAL() {}

    /**
     * \brief Run OCC algorithm
     * \details Run OCC algorithm
     *
     * \param occHalParam OCC input parameter
     * \param occHalOutput OCC output parameter
     *
     * \return True if success
     */
    virtual bool OCCHALRun(OCC_HAL_PARAMS &occHalParam, OCC_HAL_OUTPUT &occHalOutput) = 0;
protected:
    OCC_HAL() {}
};

};  //namespace StereoHAL
#endif