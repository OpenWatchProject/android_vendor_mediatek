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
#ifndef _STEREO_HAL_H_
#define _STEREO_HAL_H_

#include "common.h"
#include "stereo_hal_base.h"
#include "MTKStereoKernel.h"
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <stdlib.h>
#include <vector>

#include "camera_custom_nvram.h"
#include "mtkcam/featureio/IHal3A.h"
#include <mtkcam/featureio/fd_hal_base.h>
#include "stereo_setting_provider.h"    //For IMAGE_RESOLUTION_INFO_STRUCT
using android::Mutex;
using namespace NS3A;
/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
//#define NVRAM_LEN       1199		// this value should be the same as defined in StereoKernelCore.h
//#define OS_START        632			// the start position of one-shot info that stored in NVRAM
//#define OS_DATA_START		(OS_START+17)	// the start position of one-shot data (from module hosue) that stored in NVRAM

//// The crop size of image under IMAGE CAPTURE mode  (not horizontal binned)
//#define WIDTH_BEFORE_CROP	  1600		// the image width  before applying crop
//#define WIDTH_AFTER_CROP	  1600		// the image width  after  applying crop

//#define HEIGHT_BEFORE_CROP	1200		// the image height before applying crop
//#define HEIGHT_AFTER_CROP	  900			// the image height after  applying crop

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

typedef struct
{
	signed int      x_addr_start		;
	signed int      y_addr_start		;
	signed int      x_addr_end			;
	signed int      y_addr_end			;
	unsigned char   isPhysicallyRotate	;
	unsigned char   isSensorBinning_IC	;		// This is set according to the scenario of IMAGE CAPTURE mode // SETTING of isSensorBining: for Moudle House, 8M: 0, 2M: 0; for Phone Driver, 8M: 1, 2M: 0 (IMAGE CAPTURE MODE).
}SENSOR_SETTING_STRUCT ;


typedef struct
{
	unsigned char 			uSensorPos		; // 0: L:8M R:2M, 1: L:2M, R:8M
	unsigned char 			bIsUpscaling_PV	; // is 2M sensor has been scaled up in Preview Mode
	SENSOR_SETTING_STRUCT	sensorL			;
	SENSOR_SETTING_STRUCT	sensorR			;
} STEREO_SENSOR_SETTING_STRUCT ;

typedef struct
{
	signed int  setting[9] ;

	// Left  Part of Pattern
	signed int  pointsL[130] ;
	signed int  statusL[3] ;

	// Right Part of Pattern
	signed int  pointsR[130] ;
	signed int  statusR[3] ;
} ONE_SHOT_DATA_STRUCT ;

struct AF_WIN
{
	MUINT start_x ;
  MUINT start_y ;
  MUINT end_x ;
  MUINT end_y ;

public:     ////    Operations.
    AF_WIN()  { ::memset(this, 0, sizeof(AF_WIN)); }
};

struct RUN_LENGTH_DATA
{
    //Data is always 255
    MUINT32 offset;
    MUINT32 len;

    RUN_LENGTH_DATA(MUINT32 o, MUINT32 l) {
        offset = o;
        len = l;
    }
};
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class StereoHal: public StereoHalBase
{
protected:
    StereoHal();
    virtual ~StereoHal();

public:
    static StereoHal* createInstance(); //getInstance() + init()
    static StereoHal* getInstance();

    //=== stereo_hal_base ===
    virtual void destroyInstance();
    virtual bool init();
    virtual bool uninit();

    virtual void setParameters(sp<IParamsManager> spParamsMgr);

    //For middleware
    virtual MSize getMainSize(MSize const imgSize) const;
    virtual MSize getAlgoInputSize(MUINT32 const idx) const;
    virtual MSize getRrzSize(MUINT32 const idx) const;
    virtual MSize getFEImgSize() const  {   return mFEImgSize; };

    virtual bool STEREOGetRrzInfo(RRZ_DATA_STEREO_T &OutData) const;
    virtual bool STEREOGetInfo(HW_DATA_STEREO_T &OutData) const;
    virtual bool STEREOInit(INIT_DATA_STEREO_IN_T InData, INIT_DATA_STEREO_OUT_T &OutData);
    virtual bool STEREOSetParams(SET_DATA_STEREO_T RunData); //For GPU buffer should in one thread, if not can meger with STEREORun
    virtual bool STEREORun(OUT_DATA_STEREO_T &OutData, MBOOL bEnableAlgo = MTRUE);
    virtual bool STEREOGetDebugInfo(DBG_DATA_STEREO_T &DbgData);
    virtual bool STEREODestroy(void);
    virtual void STEREOSetFDInfo(std::vector<FD_DATA_STEREO_T> &fdInfos) { m_fdInfos = fdInfos; }
    //=== stereo_hal_base ===

private:
    void _setMainImgSize(MSize const& imgSize)     	{ mMainSize = imgSize; };
    void _setAlgoImgSize(MSize const& imgSize)     	{ mAlgoSize = imgSize; };
    MSize _getAlgoImgSize(MUINT32 const idx) const	{ return (idx == 0)? mAlgoSize: mAlgoAppendSize; };
    MSize _getMainImgSize() const            		{ return mMainSize; };
    //
    bool _loadFromNvram();
    bool _saveToNvram();
    //
	bool _initWorkBuf(const MUINT32 BUFFER_SIZE);
    //
    void _compressMask(std::vector<RUN_LENGTH_DATA> &compressedMask);
    void _encodeDebugInfo();

private:
    volatile MINT32         mUsers;
    mutable Mutex           mLock;
    //
    MTKStereoKernel*        m_pStereoDrv;
    STEREO_SCENARIO_ENUM    mScenario;
    //
    MSize                   mAlgoSize;
    MSize                   mAlgoAppendSize;
    MSize                   mMainSize;
    MSize                   mFEImgSize;

    IHal3A* mpHal3A;
    //
    NVRAM_CAMERA_GEOMETRY_STRUCT* m_pVoidGeoData;

    //
    MUINT8                 *m_pWorkBuf;
    MUINT8                 *m_pFullMask;
    MINT32                  m_nCapOrientation;
    //
    AF_WIN                  m_afWindow;
    AF_WIN                  m_rfsWindow;
    //
    MUINT                   m_uSensorRelativePosition;  // L-R 0: main-minor , 1: minor-main
    MUINT                   m_nMain1SensorIdx;
    MUINT                   m_nMain2SensorIdx;
    //
    MUINT                   m_nPrecropDegree;

    // for capture FD
    std::vector<FD_DATA_STEREO_T> m_fdInfos;

    char                   *m_debugInfo;
    std::vector<RUN_LENGTH_DATA> m_runLengthMaskData;
};

#endif  // _STEREO_HAL_H_

