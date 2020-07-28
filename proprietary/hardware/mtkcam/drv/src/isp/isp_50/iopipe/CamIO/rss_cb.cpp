
#define MTK_LOG_ENABLE 1
#undef LOG_TAG
#define LOG_TAG "RssCB"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <cutils/properties.h>
#include <math.h>
#include <mtkcam/def/common.h>
#include <Cam_Notify_datatype.h>
#include <mtkcam/drv/iopipe/CamIO/rss_cb.h>
#include <isp_reg.h>    // For ISP register structures.
#include <mtkcam/drv/iopipe/PostProc/RscUtility.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_NORMAL_PIPE);

#define RSS_CB_DEBUG

#ifdef RSS_CB_DEBUG
#undef __func__
#define __func__ __FUNCTION__

#define RSS_LOG(fmt, arg...)     CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define RSS_INF(fmt, arg...)      CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define RSS_WRN(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define RSS_ERR(fmt, arg...)     CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else
#define RSS_LOG(a,...)
#define RSS_INF(a,...)
#define RSS_WRN(a,...)
#define RSS_ERR(a,...)

#endif

#define RSS_DRV_COEFFICIENT_TABLE  (15) //recommeded by HW
#define RSS_DRV_RZ_UINT                      (1048576) //2^20

using namespace NSCam;
using namespace NSIoPipe;

/*******************************************************************************
* Global variable
********************************************************************************/
static MINT32 g_debugDump = MTRUE; // Dynamic debug


/*******************************************************************************
*
********************************************************************************/
class RssP1CbImpl : public RssP1Cb
{
public:
    RssP1CbImpl();
    virtual ~RssP1CbImpl(){};
    virtual void registerP1Notify(INormalPipe *pipe);
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);
    virtual void SetCropSize(RSS_CROP_SIZE size);
};

static RssP1CbImpl mRssP1CbImpl;

/*******************************************************************************
*
********************************************************************************/
RssP1CbImpl::RssP1CbImpl() : RssP1Cb()
{
#ifdef RSS_DRV_DEBUG
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.rssdrv.dump", value, NULL);
    g_debugDump = atoi(value);
#endif
    cropsize = {0, 0, 0, 0};
}

/*******************************************************************************
*
********************************************************************************/
RssP1Cb* getRssP1CbImpl(int sensorID, const char *name)
{
    return &mRssP1CbImpl;
}

/*******************************************************************************
*
********************************************************************************/
void RssP1CbImpl::registerP1Notify(INormalPipe *pipe)
{
    pipe->sendCommand(ENPipeCmd_SET_RSS_CBFP, (MINTPTR)&mRssP1CbImpl, -1, -1);
}

/*******************************************************************************
*
********************************************************************************/
void RssP1CbImpl::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    MUINT32    SizeOut_H, SizeOut_V;
    MUINT32    pCoeffStep_H, pCoeffStep_V;
    MUINT32    Mm1_H, Mm1_V, Nm1_H, Nm1_V;
    MUINT32    OffsetInt_H, OffsetSub_H, OffsetInt_V, OffsetSub_V;

    RSS_CFG* Output = (RSS_CFG*)pOut;
    RSS_INPUT_INFO* Input = (RSS_INPUT_INFO*)pIn;

    // To get max input w size of RSC
    RSCDMAPort rscport = DMA_RSC_IMGI_C;
    SizeOut_H = RSCQueryRscInDMAWidthMax(rscport); //max width size for input of RSC
    if(SizeOut_H == -1){
        RSS_ERR("Get max width of RSC error\n");
        return;
    }

    // Calculate Outsize.v accroding to input.w to input.h.
    SizeOut_V = (SizeOut_H * Input->rrz_out_h) / (Input->rrz_out_w);

    if(SizeOut_V < 22)
        SizeOut_V = 22;
    else if(SizeOut_V > 511)
        SizeOut_V = 511;

    if((SizeOut_H > Input->rss_in_w) || (SizeOut_V > Input->rss_in_h)) {
        RSS_ERR("RSS output size is bigger than input size\n");
        return;
    }

    if(((cropsize.w_start + cropsize.w_size) > Input->rss_in_w) || ((cropsize.h_start + cropsize.h_size) > Input->rss_in_h)) {
        RSS_ERR("Cropsize is bigger than input size\n");
        return;
    }

    if(!cropsize.w_size && !cropsize.h_size) {
        Mm1_V = Input->rss_in_h - 1;
        Mm1_H = Input->rss_in_w - 1;
        Output->cfg_rss_hori_int_ofst = 0;
        Output->cfg_rss_hori_sub_ofst = 0;
        Output->cfg_rss_vert_int_ofst = 0;
        Output->cfg_rss_vert_sub_ofst = 0;
    } else {
        Mm1_V = cropsize.h_size - 1;
        Mm1_H = cropsize.w_size - 1;
    }

    // Calculate Nm1 = SizeOut - 1.
    Nm1_H = SizeOut_H - 1;
    Nm1_V = SizeOut_V - 1;

    //Calculate CoefStep.
    pCoeffStep_H = (MUINT32)((Nm1_H*RSS_DRV_RZ_UINT + Mm1_H - 1) / Mm1_H);
    pCoeffStep_V = (MUINT32)((Nm1_V*RSS_DRV_RZ_UINT + Mm1_V - 1) / Mm1_V);

    //Coefficient table 15 is recommended by HW.
    Output->cfg_rss_ctrl_hori_tbl_sel = RSS_DRV_COEFFICIENT_TABLE;
    Output->cfg_rss_ctrl_vert_tbl_sel = RSS_DRV_COEFFICIENT_TABLE;
    Output->cfg_rss_ctrl_hori_en = MTRUE;
    Output->cfg_rss_ctrl_vert_en = MTRUE;

    //Calculate Luma for VERT and HORI
    if((cropsize.w_size > 0) && (cropsize.h_size > 0)) {
        OffsetInt_H = floor(cropsize.w_start);
        OffsetSub_H = RSS_DRV_RZ_UINT * (cropsize.w_start - floor(cropsize.w_start));
        OffsetInt_V = floor(cropsize.h_start);
        OffsetSub_V = RSS_DRV_RZ_UINT * (cropsize.h_start - floor(cropsize.h_start));

        Output->cfg_rss_hori_int_ofst = (OffsetInt_H * pCoeffStep_H + OffsetSub_H * pCoeffStep_H / RSS_DRV_RZ_UINT) / RSS_DRV_RZ_UINT;
        Output->cfg_rss_hori_sub_ofst = (OffsetInt_H * pCoeffStep_H + OffsetSub_H * pCoeffStep_H / RSS_DRV_RZ_UINT) % RSS_DRV_RZ_UINT;
        Output->cfg_rss_vert_int_ofst = (OffsetInt_V * pCoeffStep_H + OffsetSub_V * pCoeffStep_H / RSS_DRV_RZ_UINT) / RSS_DRV_RZ_UINT;
        Output->cfg_rss_vert_sub_ofst = (OffsetInt_V * pCoeffStep_H + OffsetSub_V * pCoeffStep_H / RSS_DRV_RZ_UINT) % RSS_DRV_RZ_UINT;

        if(g_debugDump) {
            RSS_LOG("OffsetInt_H=%x, OffsetSub_H=%x, OffsetInt_V=%x, OffsetSub_V=%x\n", \
                OffsetInt_H, OffsetSub_H, OffsetInt_V, OffsetSub_V);
        }
    }

    Output->cfg_rss_in_img = ((Input->rss_in_h << 16) | (Input->rss_in_w));
    Output->cfg_rss_out_img = ((SizeOut_V << 16) | (SizeOut_H));
    Output->cfg_rss_hori_step = pCoeffStep_H;
    Output->cfg_rss_vert_step = pCoeffStep_V;
    Output->bypassRSS = MFALSE;
    Output->enRSS = MTRUE;

    if(g_debugDump) {
        RSS_LOG("bRSS_EN=%x, bRSS_Bypass=%x, VERT_STEP=%x, HORI_STEP=%x, OUT_IMG=%x, IN_IMG=%x, CTRL_1=%x_%x_%x_%x, HORI_INT_OFST=%x, HORI_SUB_OFST=%x, VERT_INT_OFST=%x, VERT_SUB_OFST=%x\n", \
            Output->enRSS, Output->bypassRSS, Output->cfg_rss_vert_step, Output->cfg_rss_hori_step, \
            Output->cfg_rss_out_img, Output->cfg_rss_in_img, Output->cfg_rss_ctrl_hori_tbl_sel, Output->cfg_rss_ctrl_vert_tbl_sel, Output->cfg_rss_ctrl_hori_en, Output->cfg_rss_ctrl_vert_en, \
            Output->cfg_rss_hori_int_ofst, Output->cfg_rss_hori_sub_ofst, Output->cfg_rss_vert_int_ofst, Output->cfg_rss_vert_sub_ofst);
    }
}

void RssP1CbImpl::SetCropSize(RSS_CROP_SIZE size)
{
    if(size.w_start < 0 || size.h_start < 0) {
        if(g_debugDump) {
            RSS_ERR("Set wrong crop size: w_start:%f, h_start:%f, w_size=%d, h_size=%d\n", size.w_start, size.h_start, size.w_size, size.h_size);
        }
        return;
    }

    cropsize = size;

    if(g_debugDump)
        RSS_LOG("w_start=%f, h_start=%f, w_size=%d, h_size=%d\n", cropsize.w_start, cropsize.h_start, cropsize.w_size, cropsize.h_size);
}

