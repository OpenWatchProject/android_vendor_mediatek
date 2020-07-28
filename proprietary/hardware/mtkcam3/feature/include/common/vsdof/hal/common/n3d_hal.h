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
#ifndef N3D_HAL_H_
#define N3D_HAL_H_

#include <vector>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>

namespace StereoHAL {

#define MAX_GEO_LEVEL (3)
#define WPE_PLANE_COUNT (3)

struct HWFEFM_DATA
{
    NSCam::TuningUtils::FILE_DUMP_NAMING_HINT *dumpHint    = NULL;   // For capture dump
    MUINT32 magicNumber[2]                                 = {0};     //0: main1, 1:main2
    NSCam::IImageBuffer *geoDataMain1[MAX_GEO_LEVEL]       = {NULL}; //FE
    NSCam::IImageBuffer *geoDataMain2[MAX_GEO_LEVEL]       = {NULL}; //FE
    NSCam::IImageBuffer *geoDataLeftToRight[MAX_GEO_LEVEL] = {NULL}; //FM
    NSCam::IImageBuffer *geoDataRightToLeft[MAX_GEO_LEVEL] = {NULL}; //FM
};

struct SWFEFM_DATA
{
    NSCam::IImageBuffer* geoSrcImgMain1[MAX_GEO_LEVEL] = {NULL};
    NSCam::IImageBuffer* geoSrcImgMain2[MAX_GEO_LEVEL] = {NULL};
};

struct EIS_DATA
{
    bool            isON = false;
    NSCam::MPoint   eisOffset;
    NSCam::MSize    eisImgSize;
};

//======================================================================
//                              Init Params
//======================================================================
struct N3D_HAL_INIT_PARAM_COMMON
{
    ENUM_STEREO_SCENARIO    eScenario = eSTEREO_SCENARIO_UNKNOWN;
    MUINT8                  fefmRound = 2;
};

struct N3D_HAL_INIT_PARAM : public N3D_HAL_INIT_PARAM_COMMON
{
    //The ring buffers for GPU warping of main2
    std::vector<NSCam::IImageBuffer *> inputImageBuffers;
    std::vector<NSCam::IImageBuffer *> outputImageBuffers;
    std::vector<NSCam::IImageBuffer *> outputMaskBuffers;
};

struct N3D_HAL_INIT_PARAM_WPE : public N3D_HAL_INIT_PARAM_COMMON
{
    //The ring buffers for GPU-WPE warping
    std::vector<NSCam::IImageBuffer *> outputWarpMapMain2[WPE_PLANE_COUNT];
    std::vector<NSCam::IImageBuffer *> outputWarpMapMain1[WPE_PLANE_COUNT];   //no need for VSDoF
};

//======================================================================
//                              Input Params
//======================================================================
struct N3D_HAL_PARAM_COMMON
{
    HWFEFM_DATA hwfefmData;
    NSCam::IImageBuffer *rectifyImgMain1 = NULL;
    NSCam::IImageBuffer *rectifyImgMain2 = NULL;

    NSCam::IImageBuffer *ccImage[2]      = {NULL};  //0: main1, 1: main2

    MUINT32 magicNumber[2]               = {0};     //0: main1, 1:main2
    MUINT32 requestNumber                = 0;
    time_t  timestamp                    = 0;

    NSCam::TuningUtils::FILE_DUMP_NAMING_HINT *dumpHint = NULL;
    int dumpLevel = 2;
};

struct N3D_HAL_PARAM : public N3D_HAL_PARAM_COMMON  //For Preview/VR
{
    EIS_DATA eisData;
    bool isAFTrigger                     = false;
    bool isDepthAFON                     = false;
    bool isDistanceMeasurementON         = false;

    //Digital zoom support
    NSCam::MRect    rrzoCrop;
    NSCam::MRect    mdpCrop;

    N3D_HAL_PARAM()
        : N3D_HAL_PARAM_COMMON()
    {
    }
};

struct N3D_HAL_PARAM_WPE
{
    MUINT32 magicNumber[2] = {0};     //0: main1, 1:main2
    MUINT32 requestNumber  = 0;

    NSCam::TuningUtils::FILE_DUMP_NAMING_HINT *dumpHint = NULL;
    int dumpLevel = 2;
};

//======================================================================
//                              Output Params
//======================================================================
struct N3D_HAL_OUTPUT_COMMON
{
    MFLOAT distance       = 0;
    MFLOAT convOffset     = 0;

    MINT32  *sceneInfo    = NULL;
    MUINT32 sceneInfoSize = 0;
};

struct N3D_HAL_OUTPUT : public N3D_HAL_OUTPUT_COMMON
{
    NSCam::IImageBuffer *rectifyImgMain1 = NULL;
    NSCam::IImageBuffer *maskMain1       = NULL;
    NSCam::IImageBuffer *ldcMain1        = NULL;

    NSCam::IImageBuffer *rectifyImgMain2 = NULL;
    NSCam::IImageBuffer *maskMain2       = NULL;
};

struct N3D_HAL_OUTPUT_WPE : public N3D_HAL_OUTPUT_COMMON
{
    NSCam::IImageBuffer *warpMapMain2[WPE_PLANE_COUNT] = {NULL};
    NSCam::IImageBuffer *warpMapMain1[WPE_PLANE_COUNT] = {NULL};
};

struct N3D_HAL_PARAM_CAPTURE : public N3D_HAL_PARAM_COMMON
{
    MUINT32 captureOrientation = 0;
    int touchPosX              = 0;
    int touchPosY              = 0;

    N3D_HAL_PARAM_CAPTURE()
        : N3D_HAL_PARAM_COMMON()
    {
    }
};

struct N3D_HAL_OUTPUT_CAPTURE : public N3D_HAL_OUTPUT
{
    //For de-noise
    //User need to alloc this array, size from StereoSettingProvider::getMaxWarpingMatrixBufferSizeInBytes()
    MFLOAT  *warpingMatrix    = NULL;
    MUINT32 warpingMatrixSize = 0; //Count of element in warpingMatrix

    MUINT8 *debugBuffer       = NULL;

    N3D_HAL_OUTPUT_CAPTURE()
        : N3D_HAL_OUTPUT()
    {
    }
};

//======================================================================
//                              N3D HAL
//======================================================================
class N3D_HAL
{
public:
    /**
     * \brief Create a new instance
     * \details Create instance, callers should delete them after used
     *
     * \param n3dInitParam Init parameter
     * \return A new instance
     */
    static N3D_HAL *createInstance(N3D_HAL_INIT_PARAM &n3dInitParam);

    /**
     * \brief Default destructor
     * \details Default destructor, callers should delete them after used
     */
    virtual ~N3D_HAL() {}

    /**
     * \brief Warp main1 image and provide mask
     *
     * \param main1Input Input main1 image image buffer, must align to the top-left corner
     * \param main1Output Output main1 image buffer
     * \param main1Mask Output mask of main1 image
     * \return reture if success
     */
    virtual bool N3DHALWarpMain1(NSCam::IImageBuffer *main1Input, NSCam::IImageBuffer *main1Output, NSCam::IImageBuffer *main1Mask) = 0;

    /**
     * \brief Run N3D HAL in preview/record scenario
     * \details Pass NULL FEFM data if no FEFM result for this run
     *
     * \param n3dParams Input parameter
     * \param n3dOutput Output parameter
     *
     * \return True if success
     */
    virtual bool N3DHALRun(N3D_HAL_PARAM &n3dParams, N3D_HAL_OUTPUT &n3dOutput) = 0;

    /**
     * \deprecated No capture flow now
     * \brief Run N3D HAL in capture scenario
     * \details Run N3D HAL in capture scenario
     *
     * \param n3dParams Input parameter
     * \param n3dOutput Output parameter
     *
     * \return True if success
     */
    virtual bool N3DHALRun(N3D_HAL_PARAM_CAPTURE &n3dParams, N3D_HAL_OUTPUT_CAPTURE &n3dOutput) = 0;

    /**
     * \deprecated No capture flow now
     * \brief Get stereo extra data in JSON format
     * \details Stereo extra data consists of parameters of stereo, such as JPS size, capture orientation, etc
     *          It also contains the mask for warpping. The mask is encoded with enhanced run-length-encoding(ERLE).
     *          The ERLE describes the valid area in the format [offset, length],
     *          e.g. [123, 1920] means the valid data starts from offset 123 with length 1920.
     *          User just needs to memset this area to 0xFF( ::memset(mask+123, 0xFF, 1920); ),
     *          then he can decode and get the warpping mask.
     * \return Stereo extra data in JSON format
     *
     * \see JSON_Util
     */
    virtual char *getStereoExtraData() = 0;

public:
    /**
     * \brief Create a new instance
     * \details Create instance, callers should delete them after used
     *
     * \param n3dInitParam Init parameter
     * \return A new instance
     */
    static N3D_HAL *createInstance(N3D_HAL_INIT_PARAM_WPE &n3dInitParam);

    /**
     * \brief Get warp map of main2
     * \details Warp map is input of WPE to warp an image
     *
     * \param n3dOutput Warp map and common result
     * \return true if success
     */
    virtual bool getWarpMapMain2(N3D_HAL_PARAM_WPE &n3dInput, N3D_HAL_OUTPUT_WPE &n3dOutput) = 0;

    /**
     * \brief Run N3D learning, must run before next time to call getWarpMapMain2
     * \details Tells N3D to run learning, since leanring is running in independent thread,
     *          this API will return immediately
     *
     * \param hwfefmData FEFM data
     * \return true if success
     */
    virtual bool runN3DLearning(HWFEFM_DATA &hwfefmData) = 0;

    /**
     * \brief Refine mask, call before and after WPE
     *
     * \param scenario scenario to use
     * \param mask mask buffer to refine
     * \param initMask Reset mask to 255 or not
     */
    static void refineMask(ENUM_STEREO_SCENARIO scenario, NSCam::IImageBuffer *mask, bool initMask);

protected:
    N3D_HAL() {}
};

};  //namespace StereoHAL
#endif  // N3D_HAL_H_