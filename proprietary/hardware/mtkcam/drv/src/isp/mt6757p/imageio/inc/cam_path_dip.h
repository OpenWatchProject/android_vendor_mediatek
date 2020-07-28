#ifndef __CAM_PATH_DIP_H__
#define __CAM_PATH_DIP_H__

#include "isp_function_dip.h"
#include "ispio_stddef.h"

//

#define DEBUG_STR_BEGIN "EEEEEEEEEEEEEEEEEEEEE"
#define DEBUG_STR_END   "XXXXXXXXXXXXXXXXXXXXX"

using android::Mutex;


enum EPathFreeBufMode {
    ePathFreeBufMode_SINGLE         = 0x0000,   //
    ePathFreeBufMode_ALL            = 0x0001,   //
};


#define CONFIG_FOR_SYSTRACE     0   // default:0
#if CONFIG_FOR_SYSTRACE
#define ATRACE_TAG ATRACE_TAG_CAMERA
#define ISP_TRACE_CALL() ATRACE_CALL()
#else
#define ISP_TRACE_CALL()
#endif // CONFIG_FOR_SYSTRACE


/*/////////////////////////////////////////////////////////////////////////////
    IspPath_B
  /////////////////////////////////////////////////////////////////////////////*/
class IspFunctionDip_B; //pre declaration

class DipPath_B
{
public:
    DIP_TUNNING_CTRL    ispTuningCtrl;
    DIP_TOP_CTRL        ispTopCtrl;
    DIP_BUF_CTRL        ispBufCtrl;
    int dipTh;
    //int CQ;

    //common
    DIP_RAW_PIPE        ispRawPipe;
    DIP_DMA_CQ          DMACQ;

    //for pass2
    DIP_RGB_PIPE        ispRgbPipe;
    DIP_YUV_PIPE        ispYuvPipe;
    DIP_MDP_PIPE        ispMdpPipe;
    DIP_CRZ_PIPE        crzPipe;
    DIP_TDRI_PIPE       tdriPipe;
    DIP_DMA_IMGI        DMAImgi;
    DIP_DMA_IMGBI       DMAImgbi;
    DIP_DMA_IMGCI       DMAImgci;
    DIP_DMA_TDRI        DMATdri;
    DIP_DMA_VIPI        DMAVipi;
    DIP_DMA_VIP2I       DMAVip2i;
    DIP_DMA_VIP3I       DMAVip3i;
    DIP_DMA_IMG2O       DMAImg2o;
    DIP_DMA_IMG2BO      DMAImg2bo;
    DIP_DMA_IMG3O       DMAImg3o;
    DIP_DMA_IMG3BO      DMAImg3bo;
    DIP_DMA_IMG3CO      DMAImg3co;
	DIP_DMA_DEPI        DMADepi;
    DIP_DMA_DMGI        DMADmgi;
    DIP_DMA_UFDI        DMAUfdi;
    DIP_DMA_LCEI        DMALcei;
    DIP_DMA_MFBO        DMAMfbo;
    DIP_DMA_FEO         DMAFeo;
private:
	//static Mutex   mCQBufferLock;  // for multi-thread
    //

public:
    DipPath_B();


    virtual ~DipPath_B(){};
public:
    int             start( void* pParam );
    int             stop( void* pParam );

protected:
    virtual IspFunctionDip_B**  isp_function_list() = 0;
    virtual int             isp_function_count() = 0;
public:
    virtual const char* name_Str(){     return  "IspPath";  }
protected:
    virtual int _config( void* pParam );
    virtual int _start( void* pParam );
    virtual int _stop( void* pParam );
    virtual int _waitIrq( int type, unsigned int irq , int userNumber) = 0;
    virtual int _registerIrq( NSImageio::NSIspio::Irq_t irq );
    virtual int _waitIrq( NSImageio::NSIspio::Irq_t irq );
    virtual int _end( void* pParam );
    virtual int _setZoom( void* pParam );
public:
    virtual int flushCqDescriptor( MUINT32 cq, MUINT32 burstQueIdx, MUINT32 dupCqIdx );
    //virtual int getNr3dGain(ESoftwareScenario softScenario, MINT32 magicNum);
    //virtual int setDMACurrBuf( MUINT32 const dmaChannel );
    //virtual int setDMANextBuf( MUINT32 const dmaChannel );
    virtual int enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo,MINT32 enqueCq=0,MINT32 dupCqIdx=0);
    virtual int enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data,MBOOL bImdMode );
     /**
      *@brief Free physical buffer
      *@param[in] mode : free buffer mode
      *@param[in] bufInfo : buffer info
      */
    virtual int freePhyBuf( MUINT32 const mode, stISP_BUF_INFO bufInfo );
    /**
      *@brief  Get p2 tuning top tag
      */
    //virtual MBOOL getEnTuningTag(MBOOL isTopEngine, E_ISP_DIP_CQ cq, MUINT32 drvScenario, MUINT32 subMode, MUINT32 &tuningEn1Tag, MUINT32 &tuningEn2Tag, MUINT32 &tuningDmaTag);

};


/*/////////////////////////////////////////////////////////////////////////////
    IspPathPass2
  /////////////////////////////////////////////////////////////////////////////*/
struct CamPathPass2Parameter
{
    unsigned int hwModule;
    unsigned int moduleIdx;
    unsigned int dupCqIdx;  // for hiding sw overhead
    unsigned int burstQueIdx; // for burst queue number
    bool lastframe; //last frame in enque package or not
    MBOOL isDipOnly;
    MBOOL isWaitBuf;
    int mdp_imgxo_p2_en;
    MBOOL isApplyTuning;
    int dipTh;
    int tpipe;
    int tdr_ctl;
    int tdr_tpipe;
    int tdr_tcm_en;
    int tdr_tcm2_en;
    int dip_cq_thr_ctl;
    int bypass_ispRawPipe;
    int bypass_ispRgbPipe;
    int bypass_ispYuvPipe;
//    int bypass_ispCdpPipe;
    int bypass_ispImg2o;
    //Read Register Base Address;
    unsigned int regCnt; //Read register count, the maximum value is 4.
    unsigned int* pReadAddrList;  // read base address
    MUINTPTR pRegiAddr;
    //float rrzRatio; // for sl2
    //unsigned int rrzCropX; // for sl2
    //unsigned int rrzCropY; // for sl2
    MBOOL isSl2eAheadCrz;
    MBOOL isLceAheadCrz;

    //enable table
    struct ST_DIP_TOP_CTRL  isp_top_ctl;
    //update function mask
    //struct stIspTopP2FmtSel fixed_mask_cdp_fmt;
    //
    //source ->  mem. in
    IspSize         src_img_size;
    IspRect         src_img_roi;
    //
    //IspSize         cdrz_out;
    //srz parameter
    SrzCfg srz1_cfg;
    SrzCfg srz2_cfg;
    SrzCfg srz3_cfg;
    SrzCfg srz4_cfg;
    SrzCfg srz5_cfg;
    //mfb parameter
    MfbCfg mfb_cfg;
    //
    C02Cfg	c02_cfg;
    C02Cfg	c02b_cfg;
    //nr3d
    //Nr3DCfg nr3d_cfg;
    /*===DMA===*/
    IspDMACfg tdri;
    IspDMACfg imgi;
    IspDMACfg imgbi;
    IspDMACfg imgci;
    IspDMACfg vipi;
    IspDMACfg vip2i;
    IspDMACfg vip3i;
    IspDMACfg lcei;
    IspDMACfg ufdi;
    IspDMACfg dmgi;
    IspDMACfg depi;
    //
    MdpRotDMACfg wroto;
    MdpRotDMACfg wdmao;
    MdpRotDMACfg jpego;
    MdpRotDMACfg imgxo;
    MdpRotDMACfg venco;
    //
    IspDMACfg img2o;
    IspDMACfg img2bo;
    IspDMACfg img3o;
    IspDMACfg img3bo;
    IspDMACfg img3co;
    IspDMACfg mfbo;
    //
    IspCrzCfg crz;
    IspMdpCropCfg croppara;
    IspDMACfg feo;
    IspCapTdriCfg capTdriCfg;
    IspBnrCfg bnr;
    IspLscCfg lsc;
    IspLceCfg lce;
    IspNbcCfg nbc;
    IspSeeeCfg seee;
    IspLcsoCfg lcso;
    //IspFeCfg fe;
    IspMfbCfg mfb;
    //IspCfaCfg cfa;
    IspSl2Cfg sl2;
    dip_x_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
    //MVOID *pDCParam;
    //MVOID *pCZParam; // For clear zoom
    //MVOID *pVSDOFPQParam;
    MVOID *WDMAPQParam;
    MVOID *WROTPQParam;
    MUINT32  pixIdP2;
	NSImageio::NSIspio::EDrvScenario drvScenario;   //driver scenario
};


class CamPathPass2:public DipPath_B
{
private:
    int             m_isp_function_count;
    IspFunctionDip_B*   m_isp_function_list[ENUM_DIP_FUNCTION_MAX_NUM];

public:
    CamPathPass2() :
        m_isp_function_count(0)
        {};
    virtual ~CamPathPass2(){};
private:
    virtual int _waitIrq( int type,unsigned int irq, int userNumber );
protected:
    virtual IspFunctionDip_B**  isp_function_list()  {   return m_isp_function_list; }
    virtual int             isp_function_count() {return m_isp_function_count; }
public:
    virtual const char* name_Str(){     return  "CamPathPass2";  }
public:
    int     config( struct CamPathPass2Parameter* p_parameter );
    int     setZoom( MUINT32 zoomRatio );
    EIspRetStatus  dequeueBuf( MUINT32 dmaChannel, MINT32 const eDrvSce,MINT32 dequeCq, MINT32 dequeDupCqIdx, MUINT32 dequeBurstQueIdx, MUINT32 moduleIdx );
    int     dequeueMdpFrameEnd( MINT32 const eDrvSce, MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const moduleIdx);
private:
    int     configTpipeData( struct CamPathPass2Parameter* p_parameter );
};

#endif







