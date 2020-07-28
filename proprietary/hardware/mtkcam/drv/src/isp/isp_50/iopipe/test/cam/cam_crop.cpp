#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG "main_crop"

#include "cam_crop.h"


#define LOG_INF(fmt, arg...)    printf("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_ERR(fmt, arg...)    printf("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)

using namespace NSCam;
using namespace NSCam::Utils;
using namespace android;
using namespace std;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
static INormalPipeModule* getNormalPipeModule()
{
    static auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        LOG_ERR("INormalPipeModule::get() fail");
    }
    return pModule;
}

static MUINT32 selectNormalPipeVersion(MUINT32 sensorIdx)
{
    MUINT32 selectedVersion = 0;
    auto pModule = getNormalPipeModule();

    if  ( ! pModule ) {
        LOG_ERR("getNormalPipeModule() fail");
        return selectedVersion;
    }

    MUINT32 const* version = NULL;
    size_t count = 0;
    int err = pModule->get_sub_module_api_version(&version, &count, sensorIdx);
    if  ( err < 0 || ! count || ! version ) {
        LOG_ERR(
            "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
            sensorIdx, err, count, version
        );
        return selectedVersion;
    }

    selectedVersion = *(version + count - 1); //Select max. version
    LOG_INF("[%d] count:%zu Selected CamIO Version:%0#x", sensorIdx, count, selectedVersion);
    return selectedVersion;
}


MVOID CROP_TEST::InitCfg(MSize PipeLineIn,ECROP func,MSize tgSize)
{
    char str[CROP_TEST::MAX][32] = {
        {"CROP_NONE"},
        {"CROP_SYSMATRIC"},
        {"CROP_LEFT_ONLY"},
        {"CROP_RIGHT_ONLY"},
        };

    this->mInput = PipeLineIn;
    this->mFunc = func;
    this->mTG = tgSize;

    LOG_INF("crop input:%d_%d,crop method:%s",this->mInput.w,this->mInput.h,str[this->mFunc]);
}

MBOOL CROP_TEST::PatchPipeLineIn(MSize size)
{
    this->mInput = size;

    if(this->mFunc == CROP_SPECIFIC){
        if(this->SetInfo(PORT_RRZO,this->fmt_rrzo,this->m_Crop_rrzo.s) == MFALSE)
            return MFALSE;
        if(this->SetInfo(PORT_IMGO,this->fmt_imgo,this->m_Crop_imgo.s) == MFALSE)
            return MFALSE;
    }

    return MTRUE;
}


MBOOL CROP_TEST::Generator_IMGO(EImageFormat ifmt,MRect& oStart,MBOOL bPure)
{
    #define CropRatio 8/10

    NormalPipe_QueryInfo    qry;
    NormalPipe_QueryIn      input;
    MSize _size;
    MSize _input;

    if(bPure){
        _input = this->mTG;
    }
    else{
        _input = this->mInput;
    }

    switch(this->mFunc){
        case CROP_NONE:
            //
            oStart.p.x = oStart.p.y = 0;

            //
            _size = _input;

            //
            input.width = _size.w;
            getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_X_PIX, ifmt, input, qry);
            oStart.s.w = qry.x_pix;
            oStart.s.h = _size.h;

            break;
        case CROP_SYSMATRIC:
            //
            oStart.p.x = _input.w / 4;
            oStart.p.y = _input.h / 4;

            input.width = oStart.p.x;
            getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_CROP_START_X, ifmt, input, qry);
            oStart.p.x = qry.crop_x;
            //
            _size = _input;
            _size.w /= 2;
            _size.w = _size.w*CropRatio;
            _size.h /= 2;
            _size.h = _size.h*CropRatio;

            //
            input.width = _size.w;
            getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_CROP_X_PIX, ifmt, input, qry);
            oStart.s.w = qry.x_pix;
            oStart.s.h = _size.h;

            break;
        case CROP_LEFT_ONLY:
            //
            oStart.p.x = oStart.p.y = 0;

            //
            _size = _input;
            _size.w /= 2;
            _size.w = _size.w*CropRatio;
            _size.h = _size.h*CropRatio;

            //
            input.width = _size.w;
            getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_X_PIX, ifmt, input, qry);
            oStart.s.w = qry.x_pix;
            oStart.s.h = _size.h;
            break;
        case CROP_RIGHT_ONLY:
            //
            oStart.p.x = _input.w / 2;
            oStart.p.y = 0;

            //
            input.width = oStart.p.x;
            getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_CROP_START_X, ifmt, input, qry);
            oStart.p.x = qry.crop_x;
            //
            _size = _input;
            _size.w /= 2;
            _size.w = _size.w*CropRatio;
            _size.h = _size.h*CropRatio;

            //
            input.width = _size.w;
            getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_CROP_X_PIX, ifmt, input, qry);
            oStart.s.w = qry.x_pix;
            oStart.s.h = _size.h;
            break;
         case CROP_SPECIFIC:
            //
            oStart = this->m_Crop_imgo;

            break;
        default:
            LOG_ERR("unspported func:%d\n",this->mFunc);
            break;
    }
    LOG_INF("IMGO: crop:%d_%d_%d_%d,isPure(%d)\n",oStart.p.x,oStart.p.y,oStart.s.w,oStart.s.h,bPure);
    return MTRUE;
}

MBOOL CROP_TEST::Generator_RRZO(EImageFormat ifmt,MRect& oStart)
{
    #define CropRatio 8/10

    NormalPipe_QueryInfo    qry;
    NormalPipe_QueryIn      input;
    MSize _size;

    switch(this->mFunc){
        case CROP_NONE:
            //
            oStart.p.x = oStart.p.y = 0;

            //
            _size = this->mInput;

            //
            input.width = _size.w;
            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_X_PIX, ifmt, input, qry);
            oStart.s.w = qry.x_pix;
            oStart.s.h = _size.h;
            break;
        case CROP_SYSMATRIC:
            //
            oStart.p.x = this->mInput.w / 4;
            oStart.p.y = this->mInput.h / 4;
            input.width = oStart.p.x;
            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_CROP_START_X, ifmt, input, qry);
            oStart.p.x = qry.crop_x;
            //
            _size = this->mInput;
            _size.w /= 2;
            _size.w = _size.w*CropRatio;
            _size.h /= 2;
            _size.h = _size.h*CropRatio;

            //
            input.width = _size.w;
            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_CROP_X_PIX, ifmt, _size.w, qry);
            oStart.s.w = qry.x_pix;
            oStart.s.h = _size.h;

            break;
        case CROP_LEFT_ONLY:
            //
            oStart.p.x = oStart.p.y = 0;

            //
            _size = this->mInput;
            _size.w /= 2;
            _size.w = _size.w*CropRatio;
            _size.h = _size.h*CropRatio;

            //
            input.width = _size.w;
            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_CROP_X_PIX, ifmt, input, qry);
            oStart.s.w = qry.x_pix;
            oStart.s.h = _size.h;
            break;
        case CROP_RIGHT_ONLY:
            //
            oStart.p.x = this->mInput.w / 2;
            oStart.p.y = 0;

            input.width = oStart.p.x;
            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_CROP_START_X, ifmt, input, qry);
            oStart.p.x = qry.crop_x;

            //
            _size = this->mInput;
            _size.w /= 2;
            _size.w = _size.w*CropRatio;
            _size.h = _size.h*CropRatio;

            //
            input.width = _size.w;
            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_CROP_X_PIX, ifmt, input, qry);
            oStart.s.w = qry.x_pix;
            oStart.s.h = _size.h;
            break;
        case CROP_SPECIFIC:
            //
            oStart = this->m_Crop_rrzo;
            break;
        default:
            LOG_ERR("unspported func:%d\n",this->mFunc);
            break;
    }
    LOG_INF("RRZO: crop:%d_%d_%d_%d\n",oStart.p.x,oStart.p.y,oStart.s.w,oStart.s.h);
    return MTRUE;
}

MBOOL CROP_TEST::Generator_RRZO(EImageFormat ifmt,MSize& oStart,MUINT32 type,MUINT32 factor)
{
    #define CropRatio 8/10

    NormalPipe_QueryInfo    qry;
    NormalPipe_QueryIn      input;
    MSize _size;
    MUINT32 ratio;

    //
    _size = this->m_Crop_rrzo.s;
    if(this->fmt_rrzo != ifmt){
        LOG_ERR("rrzo fmt can't be diff from crop & scale(%d_%d)\n",this->fmt_rrzo,ifmt);
        return MFALSE;
    }
    input.width = _size.w;
    getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_BS_RATIO, ifmt, input, qry);

    switch(type){
        case 0:  ratio = 100;          break;
        case 1:  ratio = qry.bs_ratio; break;
        case 2:  ratio = factor;       break;
        default: ratio = 100;          break;
    }

    switch(this->mFunc){
        case CROP_SPECIFIC:
            if( (_size.w * 100 / this->mInput.w) < ratio){
                LOG_ERR("H-scaling ratio is not supported:target:%d,max%d\n",(_size.w * 100 / this->mInput.w),ratio);
                return MFALSE;
            }
            if( (_size.h * 100 / this->mInput.h) < ratio){
                LOG_ERR("V-scaling ratio is not supported:target:%d,max%d\n",(_size.h * 100 / this->mInput.h),ratio);
                return MFALSE;
            }

            oStart = this->m_Crop_rrzo.s;
            break;
        default:
            int _tmp;
            _tmp = (_size.w*ratio/100);
            if (_tmp > _size.w) {
                _tmp = _size.w;
            }
            input.width = _tmp;
            _tmp = (_size.h*ratio/100);
            if (_tmp > _size.h) {
                _tmp = _size.h;
            }
            _size.h = _tmp;

            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_X_PIX, ifmt, input, qry);
            oStart.w = qry.x_pix;
            oStart.h = _size.h;
            break;
    }


    LOG_INF("RRZO: scale:%d_%d\n",oStart.w,oStart.h);
    return MTRUE;
}

MBOOL CROP_TEST::GetCropInfo(PortID iport,EImageFormat ifmt,MRect& oStart,MBOOL bPure)
{
    switch(iport.index){
        case NSImageio::NSIspio::EPortIndex_IMGO:
            this->Generator_IMGO(ifmt,oStart,bPure);
            this->m_status_imgo = _e_crop;
            this->m_Crop_imgo = oStart;
            this->fmt_imgo = ifmt;
            break;
        case NSImageio::NSIspio::EPortIndex_RRZO:
            this->Generator_RRZO(ifmt,oStart);
            this->m_status_rrzo = _e_crop;
            this->m_Crop_rrzo = oStart;
            this->fmt_rrzo = ifmt;
            break;
        default:
            LOG_ERR("unspported dmao:%d\n",iport.index);
            return MFALSE;
            break;
    }

    return MTRUE;
}

MBOOL CROP_TEST::GetScaleInfo(PortID iport,EImageFormat ifmt,MSize& oSize,MUINT32 type,MUINT32 factor)
{
    switch(iport.index){
        case NSImageio::NSIspio::EPortIndex_IMGO:
            if(this->m_status_imgo == _e_crop){
                //no scaler
                oSize = this->m_Crop_imgo.s;
            }
            else{
                LOG_ERR("flow err, need to crop 1st before sacle\n");
                return MFALSE;
            }

            this->m_status_imgo = _e_scale;
            break;
        case NSImageio::NSIspio::EPortIndex_RRZO:
            if(this->m_status_rrzo == _e_crop){
                this->Generator_RRZO(ifmt,oSize,type,factor);
            }
            else{
                LOG_ERR("flow err, need to crop 1st before sacle\n");
                return MFALSE;
            }

            break;
        default:
            LOG_ERR("unspported dmao:%d\n",iport.index);
            return MFALSE;
            break;
    }

    return MTRUE;
}



MBOOL CROP_TEST::SetInfo(PortID iport,EImageFormat ifmt,MSize iSize)
{
    NormalPipe_QueryInfo    qry;
    NormalPipe_QueryIn      input;

    switch(iport.index){
        case NSImageio::NSIspio::EPortIndex_IMGO:
            this->m_status_imgo = _e_set;

            this->m_Crop_imgo.p.x = this->m_Crop_imgo.p.y = 0;
            this->m_Crop_imgo.s = this->mInput;

            LOG_INF("imgo:%d_%d_%d_%d(%d)\n",this->m_Crop_imgo.p.x,this->m_Crop_imgo.p.y,this->m_Crop_imgo.s.w,this->m_Crop_imgo.s.h,this->m_status_imgo);
            break;
        case NSImageio::NSIspio::EPortIndex_RRZO:

            if(iSize.w > this->mInput.w){
                LOG_ERR("support no scaling-up:%d_%d\n",iSize.w,this->mInput.w);
                return MFALSE;
            }
            if(iSize.h > this->mInput.h){
                LOG_ERR("support no scaling-up:%d_%d\n",iSize.h,this->mInput.h);
                return MFALSE;
            }

            this->m_status_rrzo = _e_set;
            this->m_Crop_rrzo.p.x = this->m_Crop_rrzo.p.y = 0;

            input.width = iSize.w;
            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_X_PIX, ifmt, input, qry);
            this->m_Crop_rrzo.s.w = qry.x_pix;
            //
            this->m_Crop_rrzo.s.h = iSize.h;

            LOG_INF("rrzo:%d_%d_%d_%d(%d)\n",this->m_Crop_rrzo.p.x,this->m_Crop_rrzo.p.y,this->m_Crop_rrzo.s.w,this->m_Crop_rrzo.s.h,this->m_status_rrzo);
            break;
        default:
            LOG_ERR("unspported dmao:%d\n",iport.index);
            return MFALSE;
            break;
    }
    return MTRUE;
}

