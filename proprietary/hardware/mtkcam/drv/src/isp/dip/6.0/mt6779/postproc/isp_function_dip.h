#ifndef __ISP_FUNCTION_DIP_H__
#define __ISP_FUNCTION_DIP_H__
//
#include <vector>
#include <map>
#include <list>
using namespace std;
//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
//
#include <tpipe_config.h>

#include <mtkcam/drv/def/ispio_port_index.h>
#include "isp_datatypes.h"
#include "camera_dip.h" //For Isp function ID
#include "crz_drv.h"
#include "isp_drv.h"
#include "imem_drv.h"
#include <ispio_pipe_scenario.h>    // For enum EDrvScenario.
#include "isp_drv_dip.h"
#include "isp_drv_dip_phy.h"
#include "mdp_mgr.h"
#include "DipRingBuffer.h"
#include <mtkcam/drv/def/ICam_type.h>
#include <mtkcam/utils/std/Trace.h>

/*/////////////////////////////////////////////////////////////////////////////
    ISP Function ID
  /////////////////////////////////////////////////////////////////////////////*/
// for crop function
#define CROP_FLOAT_PECISE_BIT   31    // precise 31 bit
#define CROP_TPIPE_PECISE_BIT   15
#define CROP_CDP_HARDWARE_BIT   8

//
enum {
    ENUM_DIP_TOP = 0,    //0
    ENUM_DIP_RAW,
    ENUM_DIP_RGB,
    ENUM_DIP_YUV,
    ENUM_DIP_MDP,
    ENUM_DIP_CDP,        //5
    ENUM_DIP_TDRI,
    ENUM_DIP_DMA_CQI,
    ENUM_DIP_DMA_TDRI,
    ENUM_DIP_DMA_IMGI,
    ENUM_DIP_DMA_IMGBI,  //10
    ENUM_DIP_DMA_IMGCI,
    ENUM_DIP_DMA_UFDI,
    ENUM_DIP_DMA_LCEI,   
    ENUM_DIP_DMA_VIPI,
    ENUM_DIP_DMA_VIP2I,  //VIPBI 15
    ENUM_DIP_DMA_VIP3I,  //VIPCI
    ENUM_DIP_DMA_DMGI,
    ENUM_DIP_DMA_DEPI,
    ENUM_DIP_DMA_TIMGO,  //old MFBO PAK2O
    ENUM_DIP_DMA_IMG3O,  //20
    ENUM_DIP_DMA_IMG3BO,
    ENUM_DIP_DMA_IMG3CO, 
    ENUM_DIP_DMA_FEO,
    ENUM_DIP_DMA_IMG2O,  //CRZO
    ENUM_DIP_DMA_IMG2BO, //CRZBO 25
    ENUM_DIP_DMA_DCESO,
    ENUM_DIP_DMA_WDMAO,
    ENUM_DIP_DMA_WROTO,
    ENUM_DIP_DMA_JPEGO,
    ENUM_DIP_DMA_VENC_STREAMO, //30
    ENUM_DIP_BUFFER,
    ENUM_DIP_TURNING,
    ENUM_DIP_CRZ,
    ENUM_DIP_FUNCTION_MAX_NUM
};


enum EIspRetStatus
{
    eIspRetStatus_VSS_NotReady  = 1,
    eIspRetStatus_Success       = 0,
    eIspRetStatus_Failed        = -1,
};

 class CrspCfg
 {
 public:
     MUINT32 enCrsp;
     IspSize out;
 };

 class MfbCfg
 {
 public:
     MUINT32 bld_mode;
     IspSize mfb_out;        //actually the same with mfbo
     MUINT32 out_xofst;
     MUINT32 bld_ll_db_en;   //set default from imageio, would update via tuning
     MUINT32 bld_ll_brz_en;  //set default from imageio, would update via tuning
 };

 class SrzSizeCfg
 {
  public:
      MUINT32 in_w;
      MUINT32 in_h;
      MUINT32 out_w;
      MUINT32 out_h;
     SrzSizeCfg()
         : in_w(0x0)
         , in_h(0x0)
         , out_w(0x0)
         , out_h(0x0)
     {
     }
 };

 class SrzCfg
 {
 public:
 	 MUINT32 ctrl;
     SrzSizeCfg inout_size;
     IspRect crop;
     MUINT32 h_step;
     MUINT32 v_step;

     SrzCfg()
    : ctrl(0x0)
    , h_step(0x0)
    , v_step(0x0)
     {}
 };


 /******************************************************************************
  *
  * @struct _CQ_CONFIG_INFO_
  * @brief config info for dip frame
  * @details
  *
  ******************************************************************************/
 struct _CQ_CONFIG_INFO_
 {
  public:
        MINT32 hwModule;
        MINT32 moduleIdx;
        MINT32 dipTh;
        MINT32 dipCQ_dupIdx;
        MINT32 dipCQ_burstIdx;
        MBOOL isDipOnly;
        DipWorkingBuffer* pDipWBuf;

     _CQ_CONFIG_INFO_()
         : hwModule(0)
         , moduleIdx(0)
         , dipTh(0)
         , dipCQ_dupIdx(0)
         , dipCQ_burstIdx(0)
         , isDipOnly(MFALSE)
         , pDipWBuf(NULL)
     {}
 };


//
/**/
class IspFunctionDip_B;
class DIP_ISP_PIPE;


 class IspTopModuleMappingTable{
 public:
    MUINT32 ctrlByte;  // isp register byte
    DIP_A_MODULE_ENUM eCqThrFunc;
    MINT32 (*default_func)(DIP_ISP_PIPE &imageioPackage); // run individual function
 };


/*/////////////////////////////////////////////////////////////////////////////
    Isp driver object
/////////////////////////////////////////////////////////////////////////////*/
class IspDrv_B
{    public:
        IspDrv_B(){}
        virtual ~IspDrv_B(){}

    public:
        //
        friend class DIP_ISP_PIPE;

        //Raw
		static MINT32 setUNP_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setUFD_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSMT_D1(DIP_ISP_PIPE &imageioPackage); // new
		static MINT32 setBPC_D1(DIP_ISP_PIPE &imageioPackage); // new
		static MINT32 setOBC_D1(DIP_ISP_PIPE &imageioPackage);	  //obc(ob2) --> obc_D1
		static MINT32 setZFUS_D1(DIP_ISP_PIPE &imageioPackage); // new
		static MINT32 setDGN_D1(DIP_ISP_PIPE &imageioPackage); // new
		static MINT32 setLSC_D1(DIP_ISP_PIPE &imageioPackage);	//LSC2 -->LSC 
		static MINT32 setWB_D1(DIP_ISP_PIPE &imageioPackage);	   //new	 
		static MINT32 setHLR_D1(DIP_ISP_PIPE &imageioPackage);	//new 
		static MINT32 setLTM_D1(DIP_ISP_PIPE &imageioPackage);		//skip CRP / PAK 
		static MINT32 setDM_D1(DIP_ISP_PIPE &imageioPackage);	//UDM --> DM
		static MINT32 setLDNR_D1(DIP_ISP_PIPE &imageioPackage); //new
		static MINT32 setCRP_D2(DIP_ISP_PIPE &imageioPackage);	//new 
		static MINT32 setPAK_D1(DIP_ISP_PIPE &imageioPackage);	  //new
		static MINT32 setSLK_D1(DIP_ISP_PIPE &imageioPackage);	   
		static MINT32 setSLK_D6(DIP_ISP_PIPE &imageioPackage);		




        //Rgb
		static MINT32 setFLC_D1(DIP_ISP_PIPE &imageioPackage);	//FLC2 --> FLC
		static MINT32 setCCM_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setGGM_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setWSYNC_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setCCM_D2(DIP_ISP_PIPE &imageioPackage);	// no DIP_A_CCM_D2
		static MINT32 setGGM_D2(DIP_ISP_PIPE &imageioPackage);	// To Do: all of below (20180619)
		static MINT32 setSMT_D4(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setUNP_D4(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setPAK_D4(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setMCRP_D2(DIP_ISP_PIPE &imageioPackage);


        //Yuv
		static MINT32 setG2CX_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setC42_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setMIX_D3(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setYNR_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setNDG_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setMIX_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setC24_D3(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setC2G_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setIGGM_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setCCM_D3(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setLCE_D1(DIP_ISP_PIPE &imageioPackage);	 
		static MINT32 setGGM_D3(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setDCE_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setDCES_D1(DIP_ISP_PIPE &imageioPackage); 		
		static MINT32 setG2C_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setC42_D2(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setEE_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSMT_D2(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setUNP_D2(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setPAK_D2(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setCNR_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setNDG_D2(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSMT_D3(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setUNP_D3(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setPAK_D3(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setCOLOR_D1(DIP_ISP_PIPE &imageioPackage);		 
		static MINT32 setMIX_D2(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setMIX_D4(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setCRP_D1(DIP_ISP_PIPE &imageioPackage);

		static MINT32 setNR3D_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setCRSP_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setPAK_D6(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setPAK_D7(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setPAK_D8(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setC24_D2(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setMCRP_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setCRZ_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSRZ_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSRZ_D3(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSRZ_D4(DIP_ISP_PIPE &imageioPackage); 
		static MINT32 setUNP_D6(DIP_ISP_PIPE &imageioPackage); 
		static MINT32 setUNP_D7(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setUNP_D8(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setC02_D2(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setC24_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setUNP_D9(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setUNP_D10(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setUNP_D11(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setC02_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSLK_D2(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSLK_D3(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSLK_D4(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setSLK_D5(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setFM_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setLPCNR_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setBS_D1(DIP_ISP_PIPE &imageioPackage);
		static MINT32 setDFE_D1(DIP_ISP_PIPE &imageioPackage);

};

/*/////////////////////////////////////////////////////////////////////////////
    Isp driver shell
/////////////////////////////////////////////////////////////////////////////*/
class IspDrvShell:virtual public IspDrv_B
{
    protected:
        IspDrvShell()
        {
            m_pDipRingBuf = NULL;
            m_pDipCrzDrv = NULL;
            m_pMdpMgr = NULL;
            m_pDrvDipPhy = NULL;
            for(int i=DIP_HW_A; i<DIP_HW_MAX; i++) {
                m_pDrvDip[i] = NULL;
            }
            m_bInitAllocSmxBuf = false;
            m_pIMemDrv= NULL;
            m_iDipMetEn = 0;
            m_bDCMEn = 0;
            m_dumpDipHwRegEn = 0;
            m_dumpDipHwRegBySceID = -1;
            m_dumpDipHwTpipeReg = 1;
        }
        virtual                 ~IspDrvShell() {};
    public:
        static IspDrvShell*     createInstance();
        virtual void            destroyInstance(void) = 0;
        virtual MBOOL           init(const char* userName="", MUINT32 secTag=0) = 0;
        virtual MBOOL           uninit(const char* userName="") = 0;
        virtual DipRingBuffer*  getDipRingBufMgr() = 0;
        //
        friend class IspFunctionDip_B;

        DipRingBuffer*         m_pDipRingBuf; /* RingBufMgr */
        CrzDrv*                m_pDipCrzDrv; /* CRZ Driver */
        MdpMgr*                m_pMdpMgr; /* MdpMgr */
        int                    m_iDipMetEn;
        int                    m_bDCMEn;
        int                    m_dumpDipHwRegEn;
        int                    m_dumpDipHwRegBySceID;
        int                    m_dumpDipHwTpipeReg;

        PhyDipDrv*             m_pDrvDipPhy;
        VirDipDrv*             m_pDrvDip[DIP_HW_MAX];
        
        bool                   m_bInitAllocSmxBuf;

    public://commandQ operation
        mutable Mutex   mLock;
        DipIMemDrv*               m_pIMemDrv ; /* IMEM */

};


/*/////////////////////////////////////////////////////////////////////////////
    IspFunctionDip_B
  /////////////////////////////////////////////////////////////////////////////*/
class IspFunctionDip_B
{
private:
    mutable Mutex           mLock;
protected://isp driver operation

public://isp driver operation

    int                     bypass;
    static IspDrvShell*     m_pIspDrvShell;
public:
    IspFunctionDip_B(): bypass(0){};
    virtual ~IspFunctionDip_B(){};
public:
    virtual unsigned long   id( void ) = 0;
    virtual const char*     name_Str( void ) = 0;   //Name string of module
    virtual int             is_bypass( void ) {return bypass;}

public: //isp driver operationr
    inline int              waitIrq( DIP_WAIT_IRQ_ST* pWaitIrq ){return (int)m_pIspDrvShell->m_pDrvDipPhy->waitIrq( pWaitIrq );}
    mutable Mutex           queHwLock;

public:
    virtual int     config( unsigned int dipidx ) = 0;
    virtual int     enable( void* pParam ) = 0;
    virtual int     disable( void* pParam ) = 0;
    virtual int     write2CQ( unsigned int dipidx ) = 0 ;

};

/*/////////////////////////////////////////////////////////////////////////////
    DMA
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_DMA:public IspFunctionDip_B
{
public:

    IspDMACfg   dma_cfg;
        int dma_ID;

private:

public: /*ctor/dtor*/
        DIP_DMA(const int& dmaID):dma_ID(-1)
            {dma_cfg.pixel_byte = 1; dma_ID = dmaID;};

public:/*[IspFunctionDip_B]*/
    virtual int config( unsigned int dipidx );
    virtual int enable( void* pParam ){(void)pParam; return 0;}
    virtual int disable( void* pParam ){(void)pParam; return 0;}
    virtual int write2CQ( unsigned int dipidx );
    virtual unsigned long id( void ) {return this->dma_ID;}
    virtual const char*   name_Str( void )
    {   //ONLY FOR DEBUG USAGE
        switch(this->dma_ID)
        {
            case ENUM_DIP_DMA_CQI:
                return     "DIP_DMA_CQI";
            case ENUM_DIP_DMA_TDRI:
                return     "DIP_DMA_TDRI";
            case ENUM_DIP_DMA_IMGI:
                return     "DIP_DMA_IMGI";
            case ENUM_DIP_DMA_IMGBI:
                return     "DIP_DMA_IMGBI";
            case ENUM_DIP_DMA_IMGCI:
                return     "DIP_DMA_IMGCI";
            case ENUM_DIP_DMA_UFDI:
                return     "DIP_DMA_UFDI";
            case ENUM_DIP_DMA_LCEI:
                return     "DIP_DMA_LCEI";
            case ENUM_DIP_DMA_VIPI:
                return     "DIP_DMA_VIPI";
            case ENUM_DIP_DMA_VIP2I:
                return     "DIP_DMA_VIP2I";
            case ENUM_DIP_DMA_VIP3I:
                return     "DIP_DMA_VIP3I";
            case ENUM_DIP_DMA_DMGI:
                return     "DIP_DMA_DMGI";
            case ENUM_DIP_DMA_DEPI:
                return     "DIP_DMA_DEPI";
            case ENUM_DIP_DMA_TIMGO:
                return     "DIP_DMA_PAK2O";
            case ENUM_DIP_DMA_IMG3O:
                return     "DIP_DMA_IMG3O";
            case ENUM_DIP_DMA_IMG3BO:
                return     "DIP_DMA_IMG3BO";
            case ENUM_DIP_DMA_IMG3CO:
                return     "DIP_DMA_IMG3CO";
            case ENUM_DIP_DMA_FEO:
                return     "DIP_DMA_FEO";
            case ENUM_DIP_DMA_IMG2O:
                return     "DIP_DMA_IMG2O";
            case ENUM_DIP_DMA_IMG2BO:
                return     "DIP_DMA_IMG2BO";
            case ENUM_DIP_DMA_DCESO:
                return     "DIP_DMA_DCESO";
            default:
                return     "WRONG......";
        }
    }
    public: /*[DIP_DMA]*/

    private: /*[DIP_DMA]*/

};

class Crz_Cfg
{
public:
    int conf_cdrz;
    IspSize crz_in;
    IspSize crz_out;
    IspRect crz_crop;
    MUINT32 hAlgo,vAlgo;
    MUINT32 hTable, vTable;
    MUINT32 hCoeffStep,vCoeffStep;
    int tpipeMode;

    Crz_Cfg():conf_cdrz(0),hAlgo(0),vAlgo(0),hTable(0),vTable(0),hCoeffStep(0),vCoeffStep(0),tpipeMode(CRZ_DRV_MODE_TPIPE){};
};


/*/////////////////////////////////////////////////////////////////////////////
    DIP_TOP_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_ISP_PIPE : public IspFunctionDip_B
{
public:
    //
    struct ST_DIP_TOP_CTRL  isp_top_ctl;

    MINT32 irqUserKey; // get the user key when call function "registerIrq" for each dip, and user use this user key to call function "waitirq"

    MBOOL isDipOnly;
    unsigned int dipTh;
    unsigned int moduleIdx;
    //unsigned int dupCqIdx;
    unsigned int RingBufIdx;
    unsigned int burstQueIdx; // for burst queue number
    MBOOL isApplyTuning;
    dip_x_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
    UFDG_META_INFO *pUfdParam;
    MUINT32 isSecureFra;

    //common
    int             m_isp_function_count;
    IspFunctionDip_B*   m_isp_function_list[ENUM_DIP_FUNCTION_MAX_NUM];

    DIP_DMA      DMAImgi{ENUM_DIP_DMA_IMGI};
    DIP_DMA      DMAImgbi{ENUM_DIP_DMA_IMGBI};
    DIP_DMA      DMAImgci{ENUM_DIP_DMA_IMGCI};
    DIP_DMA      DMAVipi{ENUM_DIP_DMA_VIPI};
    DIP_DMA      DMAVip2i{ENUM_DIP_DMA_VIP2I};
    DIP_DMA      DMAVip3i{ENUM_DIP_DMA_VIP3I};
    DIP_DMA      DMADepi{ENUM_DIP_DMA_DEPI};
    DIP_DMA      DMADmgi{ENUM_DIP_DMA_DMGI};
    DIP_DMA      DMAUfdi{ENUM_DIP_DMA_UFDI};
    DIP_DMA      DMALcei{ENUM_DIP_DMA_LCEI};
    DIP_DMA      DMAImg2o{ENUM_DIP_DMA_IMG2O};
    DIP_DMA      DMAImg2bo{ENUM_DIP_DMA_IMG2BO};
    DIP_DMA      DMAImg3o{ENUM_DIP_DMA_IMG3O};
    DIP_DMA      DMAImg3bo{ENUM_DIP_DMA_IMG3BO};
    DIP_DMA      DMAImg3co{ENUM_DIP_DMA_IMG3CO};
    DIP_DMA      DMATimgo{ENUM_DIP_DMA_TIMGO};
    DIP_DMA      DMAFeo{ENUM_DIP_DMA_FEO};
    DIP_DMA      DMADceso{ENUM_DIP_DMA_DCESO};

    CrspCfg crspCfg;
    MfbCfg mfb_cfg;
    SrzCfg srz1Cfg;
    SrzCfg srz2Cfg;
    SrzCfg srz3Cfg;
    SrzCfg srz4Cfg;
    SrzCfg srz5Cfg;

    Crz_Cfg crzPipe;
    //
    PhyDipDrv* pDrvDipPhy;
    VirDipDrv* pDrvDip;

    int dip_cq_thr_ctl;
    int tdr_ctl;
    int tdr_sel;
    int tdr_tcm_en;
    int tdr_tcm2_en;
    int tdr_tcm3_en;
    int tdr_tcm4_en;
    int tdr_tcm5_en;
    int tdr_tcm6_en;

    MUINTPTR pRegiAddr;
    unsigned int regCount;
    unsigned int* pReadAddrList;

    DipWorkingBuffer* m_pDipWBuf;

public:
    DIP_ISP_PIPE():irqUserKey(0),isDipOnly(MFALSE),dipTh(ISP_DRV_DIP_CQ_THRE0),moduleIdx(DIP_HW_A),RingBufIdx(0),burstQueIdx(0),isApplyTuning(MFALSE), pTuningIspReg(NULL),pUfdParam(NULL),isSecureFra(0),m_isp_function_count(0),pDrvDipPhy(NULL),pDrvDip(NULL),dip_cq_thr_ctl(0),tdr_ctl(0),tdr_sel(0),
        tdr_tcm_en(0),tdr_tcm2_en(0),tdr_tcm3_en(0),tdr_tcm4_en(0),tdr_tcm5_en(0),tdr_tcm6_en(0),pRegiAddr(0x0),regCount(0),pReadAddrList(NULL),m_pDipWBuf(NULL),MDPCallback(NULL),MDPparam(NULL){

        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&(*this);
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImgi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImgbi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImgci;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAUfdi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMALcei;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAVipi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAVip2i;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAVip3i;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMADepi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMADmgi;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMATimgo;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg2o;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg2bo;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg3o;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg3bo;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAImg3co;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMAFeo;
        m_isp_function_list[m_isp_function_count++] = (IspFunctionDip_B*)&DMADceso;
    };
    ~DIP_ISP_PIPE()
    {

    }
    virtual unsigned long id( void )                    {   return ENUM_DIP_TOP;  }
    virtual const char*   name_Str( void )              {   return "DIP_TOP";}
    int configTpipeData( void );
    int ConfigDipPipe( DipWorkingBuffer* pDipWBuf );

public:
    virtual int config( unsigned int dipidx );
    virtual int startMDP(MINT32 const p2CQ, MUINT32 const RingBufIdx, MINT32 const burstQIdx, MUINT32 const frameNum);
    virtual int enable( void* pParam );
    virtual int disable( void* pParam );
    virtual int write2CQ( unsigned int dipidx );
    void (*MDPCallback)(void *param);
    void *MDPparam;
    
    virtual MBOOL   CalAlgoAndCStep(
        CRZ_DRV_MODE_ENUM       eFrameOrTpipeOrVrmrg,
        CRZ_DRV_RZ_ENUM         eRzName,
        MUINT32                 SizeIn_H,
        MUINT32                 SizeIn_V,
        MUINT32                 u4CroppedSize_H,
        MUINT32                 u4CroppedSize_V,
        MUINT32                 SizeOut_H,
        MUINT32                 SizeOut_V,
        CRZ_DRV_ALGO_ENUM       *pAlgo_H,
        CRZ_DRV_ALGO_ENUM       *pAlgo_V,
        MUINT32                 *pTable_H,
        MUINT32                 *pTable_V,
        MUINT32                 *pCoeffStep_H,
        MUINT32                 *pCoeffStep_V);


protected:
    IspFunctionDip_B**  isp_function_list()  {   return m_isp_function_list; }
    int             isp_function_count() {return m_isp_function_count; }
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_MDP
  /////////////////////////////////////////////////////////////////////////////*/
class DIP_MDP_PIPE:public IspFunctionDip_B
{
public:
    //IspRect src_crop;
    MdpRotDMACfg wroto_out;
    MdpRotDMACfg wdmao_out;
    //MdpRotDMACfg jpego_out;
    MdpRotDMACfg imgxo_out;
    MdpRotDMACfg venco_out;
    int mdp_imgxo_p2_en;
    unsigned int yuv2_en;

    MBOOL lastframe;  //last frame in enque package or not
    //unsigned int lastframe_dupCqIdx;
    //unsigned int lastframe_burstQueIdx; // for burst queue number
    //unsigned int lastframe_dipTh;
    unsigned int dip_dma_en;
    unsigned int dip_dma2_en;
    int drvScenario;
    MBOOL isWaitBuf;
    int dipCtlDmaEn;
    int dipCtlDma2En;
    //
    EImageFormat p2MdpSrcFmt;
    unsigned int p2MdpSrcW;
    unsigned int p2MdpSrcH;
    unsigned int p2MdpSrcYStride;
    unsigned int p2MdpSrcUVStride;
    unsigned int p2MdpSrcSize;
    unsigned int p2MdpSrcCSize;
    unsigned int p2MdpSrcVSize;
    unsigned int p2MdpSrcPlaneNum;
    IspDMACfg   imgi_dma_cfg;
    //MVOID       *DCParam;
    //MVOID       *CZParam; // For clear zoom
    //MVOID       *VSDOFPQParam;
    MVOID       *WDMAPQParam;
    MVOID       *WROTPQParam;

    unsigned int dipTh;
    //unsigned int dupCqIdx;
    unsigned int burstQueIdx; // for burst queue number
    unsigned int RingBufIdx;    
    dip_x_reg_t *pTuningIspReg;  // p2 tuning data set by tuning provider
    MBOOL isDipOnly;

    unsigned int framemode_total_in_w;  //total width of in-dma for frame mode only
    unsigned int framemode_total_out_w; //total width od out-dma for frame mode only
    unsigned int framemode_h;       //height for frame mode only
    unsigned int total_data_size;     // add for bandwidth

    // TODO: HungWen temp.
    //ISP_TPIPE_CONFIG_STRUCT m_LastTpipeStructure;
    //unsigned int m_LastVencPortEn;
    MUINT32 isSecureFra;
public:
    //DIP_MDP_PIPE():mdp_imgxo_p2_en(0),yuv2_en(0),lastframe(MFALSE),lastframe_dupCqIdx(0),lastframe_burstQueIdx(0),lastframe_dipTh(ISP_DRV_DIP_CQ_THRE0),
    DIP_MDP_PIPE():mdp_imgxo_p2_en(0),yuv2_en(0),lastframe(MFALSE),
    dip_dma_en(0),drvScenario(-1),isWaitBuf(MTRUE),dipCtlDmaEn(0),p2MdpSrcFmt(eImgFmt_YV12),p2MdpSrcW(0),p2MdpSrcH(0),p2MdpSrcYStride(0),p2MdpSrcUVStride(0),p2MdpSrcSize(0),p2MdpSrcCSize(0),
    p2MdpSrcVSize(0),p2MdpSrcPlaneNum(0),WDMAPQParam(NULL),WROTPQParam(NULL),dipTh(0),burstQueIdx(0),RingBufIdx(0),pTuningIspReg(NULL),isDipOnly(MFALSE),framemode_total_in_w(0),
    framemode_total_out_w(0), framemode_h(0), total_data_size(0), isSecureFra(0)
        {
            //m_LastTpipeStructure.drvinfo.tpipeTableVa=NULL;
            //m_LastTpipeStructure.drvinfo.frameflag=0xffff; //initial value
            //m_LastVencPortEn = 0;
            memset(&imgi_dma_cfg, 0, sizeof(imgi_dma_cfg));
            //memset(&m_LastTpipeStructure, 0, sizeof(m_LastTpipeStructure));


        };

    ~DIP_MDP_PIPE()
        {
            //m_LastTpipeStructure.drvinfo.tpipeTableVa=NULL;
            //m_LastTpipeStructure.drvinfo.frameflag=0xffff; //initial value
            //m_LastVencPortEn = 0;
        };

public:
    virtual unsigned long id( void )                    {   return ENUM_DIP_MDP;  }
    virtual const char*   name_Str( void )              {   return "DIP_MDP";}

    int configMDP( DipWorkingBuffer* pDipWBuf );

public:
    virtual int config( unsigned int dipidx ){(void)dipidx; return 0;}
    virtual int enable( void* pParam ){(void)pParam; return 0;}
    virtual int disable( void* pParam ){(void)pParam; return 0;}
    virtual int write2CQ( unsigned int dipidx ){(void)dipidx; return 0;}

public:
    MBOOL startVencLink( MINT32 fps, MINT32 wd,MINT32 ht);
    MBOOL stopVencLink();
};


//
class DIP_BUF_CTRL : public IspFunctionDip_B
{
//friend class IspDrvShellImp;
friend class DIP_TOP_CTRL;

private:

public:
    // TODO: hungwen modify
    MDPMGR_CFG_STRUCT* m_pMdpMgrCfgData;
public:
    DIP_BUF_CTRL():m_pMdpMgrCfgData(NULL)
        {};
    virtual unsigned long id( void )                    {   return ENUM_DIP_BUFFER;  }
    virtual const char*   name_Str( void )              {   return "DIP_BUFFER";}

    /*
        * dequeueMdpFrameEnd
        *       set mdp deque frame end for each frame end
        */
    //virtual MINT32 dequeueMdpFrameEnd( MINT32 dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const moduleIdx);
    virtual MINT32 dequeueMdpFrameEnd();
    virtual EIspRetStatus debugDump(DipWorkingBuffer* pDipWBuf, MUINT32 ispDmaEnBit, MUINT32 i, MUINT32 drvScenario);
    virtual EIspRetStatus dequeHwBuf( MBOOL& isVencContained, DipWorkingBuffer* pDipWBuf);
private:

protected:
    virtual int     config( unsigned int dipidx ) {(void)dipidx; return 0;}
    virtual int     enable( void* pParam ) {(void)pParam; return 0;}
    virtual int     disable( void* pParam ) {(void)pParam; return 0;}
    virtual int     write2CQ( unsigned int dipidx ) {(void)dipidx; return 0;}

};

////////////////////////////////////////////////////////////////////////////////


#endif /*__ISP_FUNCTION_DIP_H__*/




