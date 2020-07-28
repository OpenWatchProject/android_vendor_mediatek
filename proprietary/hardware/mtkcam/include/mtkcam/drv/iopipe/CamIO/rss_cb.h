#ifndef _RSS_CB_H_
#define _RSS_CB_H_

#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

class RssP1Cb;

extern RssP1Cb* getRssP1CbImpl(int, const char*) __attribute__((weak));

class RssP1Cb : public P1_TUNING_NOTIFY
{
public:
    static RssP1Cb* createInstance(int sensorID, const char* name)
    {
        RssP1Cb* (*null_fp)(int, const char*) = 0;
        return (getRssP1CbImpl == null_fp) ? NULL : getRssP1CbImpl(sensorID, name);
    }

    RssP1Cb(){cropsize = {0, 0, 0, 0};};
    virtual ~RssP1Cb(){};
    virtual void registerP1Notify(INormalPipe *pipe){};
    virtual void p1TuningNotify(MVOID* pInput,MVOID *pOutput){};
    virtual const char* TuningName() { return "Update_RSS"; };
    virtual void SetCropSize(RSS_CROP_SIZE size){};
    RSS_CROP_SIZE   cropsize;
 };

#endif

