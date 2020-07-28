#ifndef _CAM_CROP_H_
#define _CAM_CROP_H_

#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <queue>
#include <semaphore.h>
#include <pthread.h>

#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/threads.h>

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>



#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>



class CROP_TEST{
public:
    typedef enum{
        CROP_NONE       = 0,
        CROP_SYSMATRIC  = 1,
        CROP_LEFT_ONLY  = 2,
        CROP_RIGHT_ONLY = 3,
        CROP_SPECIFIC   = 4,
        MAX             = 5,
    }ECROP;

    MVOID   InitCfg(NSCam::MSize PipeLineIn,ECROP func,NSCam::MSize tgSize);

    MBOOL   GetCropInfo(NSCam::NSIoPipe::PortID iport,NSCam::EImageFormat ifmt,NSCam::MRect& oStart,MBOOL bPure = MFALSE);
    MBOOL   GetScaleInfo(NSCam::NSIoPipe::PortID iport,NSCam::EImageFormat ifmt,NSCam::MSize& oSize,MUINT32 type,MUINT32 factor);

    MBOOL   SetInfo(NSCam::NSIoPipe::PortID iport,NSCam::EImageFormat ifmt,NSCam::MSize oSize);

    MBOOL   PatchPipeLineIn(NSCam::MSize size);
private:
    MBOOL   Generator_IMGO(NSCam::EImageFormat ifmt,NSCam::MRect& oStart,MBOOL bPure);
    MBOOL   Generator_RRZO(NSCam::EImageFormat ifmt,NSCam::MRect& oStart);
    MBOOL   Generator_RRZO(NSCam::EImageFormat ifmt,NSCam::MSize& oStart,MUINT32 type,MUINT32 factor);
private:
    NSCam::MSize   mInput;
    NSCam::MSize   mTG;
    ECROP   mFunc;

    //
    typedef enum{
        _e_none = 0,
        _e_set,
        _e_crop,
        _e_scale,
    }E_FLOW;
    E_FLOW  m_status_imgo;
    E_FLOW  m_status_rrzo;
    NSCam::MRect   m_Crop_imgo;
    NSCam::MRect   m_Crop_rrzo;

    //
    NSCam::EImageFormat fmt_imgo;
    NSCam::EImageFormat fmt_rrzo;

};
#endif
