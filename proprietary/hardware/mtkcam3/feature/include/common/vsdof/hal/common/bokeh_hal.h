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
#ifndef BOKEH_HAL_H_
#define BOKEH_HAL_H_

#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>

namespace StereoHAL {

struct BOKEH_HAL_INIT_PARAMS
{
    MUINT32 dsV            = 4;  //4, 8, 16, 32
    MUINT32 dsH            = 4;  //4, 8, 16, 32
    MUINT32 iteration      = 1;  //1~5
    MUINT32 interpolation  = 0;  //0, 1
    MUINT32 core           = 8;  //1~8(MT6757) or 1~10(MT6799)
    MUINT32 coreDetails[3] = {4, 4, 0};
};

struct BOKEH_HAL_PARAMS
{
    char                *extraData         = NULL;
    MINT32              requestNumber      = 0;
    time_t              timestamp          = 0;

    NSCam::IImageBuffer *cleanImage        = NULL;  //NV21
    NSCam::IImageBuffer *packedDepthmap    = NULL;  //Y-only, size: DMW*4

    //For FF lens, AF lens does not need to set
    //Value: -1000~1000
    NSCam::MPoint       touchPos;

    //For ABF
    MUINT32             iso                = 0;

    //For background service, we have to cache settings
    MUINT32             logicalDeviceID    = StereoSettingProvider::getLogicalDeviceID();

    NSCam::TuningUtils::FILE_DUMP_NAMING_HINT *dumpHint = NULL;
};

struct BOKEH_HAL_OUTPUT
{
    NSCam::IImageBuffer *bokehImage     = NULL;     //NV21
};

class BOKEH_HAL
{
public:
    /**
     * \brief Create a new instance
     * \details Create instance, callers should call destroyInstance after used
     * \return A new instance
     */
    static BOKEH_HAL *createInstance(BOKEH_HAL_INIT_PARAMS *initParams = NULL);

    /**
     * \brief [brief description]
     * \details [long description]
     */
    virtual void destroyInstance() = 0;

    /**
     * \brief Run software bokeh algorithm
     * \details Run software bokeh algorithm
     *
     * \param param software bokeh input parameter
     * \param output software bokeh output parameter
     *
     * \return True if success
     */
    virtual bool Run(BOKEH_HAL_PARAMS &param, BOKEH_HAL_OUTPUT &output) = 0;
protected:
    BOKEH_HAL() {}
    virtual ~BOKEH_HAL() {}
};
};  //namespace StereoHAL
#endif