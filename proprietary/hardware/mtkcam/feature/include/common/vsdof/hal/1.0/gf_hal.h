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
#ifndef _GF_HAL_H_
#define _GF_HAL_H_

#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <vector>

namespace StereoHAL {

struct GF_HAL_IN_DATA
{
    MINT32  magicNumber           = -1;
    ENUM_STEREO_SCENARIO scenario = eSTEREO_SCENARIO_UNKNOWN;

    MINT32  dofLevel              = -1;

    MUINT8  *depthMap             = NULL;
    std::vector<NSCam::IImageBuffer *> images;

    MFLOAT  convOffset            = 0.0f; //from N3D HAL

    MINT32  touchPosX             = 0;
    MINT32  touchPosY             = 0;

    bool    isCapture             = false;
};

struct GF_HAL_OUT_DATA
{
    MUINT8 *dmbg     = NULL;
    MUINT8 *depthMap = NULL;   //Output size is configured in camera_custom_stereo.cpp of each target
};

class GF_HAL
{
public:
    /**
     * \brief Create a new instance
     * \details Callers should delete the instance by themselves:
     *          GF_HAL *gfHal = GF_HAL::createInstance();
     *          ...
     *          if(gfHal) delete gfHal;
     * \return A new instance
     */
    static GF_HAL *createInstance(ENUM_STEREO_SCENARIO eScenario);

    /**
     * \brief Default destructor
     * \details Callers should delete the instance by themselves
     */
    virtual ~GF_HAL() {}

    /**
     * \brief Run GF
     * \details Run GF algorithm and get result
     *
     * \param inData Input data
     * \param outData Output data
     *
     * \return True if success
     */
    virtual bool GFHALRun(GF_HAL_IN_DATA &inData, GF_HAL_OUT_DATA &outData) = 0;
    //
protected:
    GF_HAL() {}
};

};  //namespace StereoHAL
#endif