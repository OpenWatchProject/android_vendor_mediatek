#ifndef __ISP_FUNCTION_CAM_H__
#define __ISP_FUNCTION_CAM_H__

#include "isp_function.h"
#include "isp_function.timestamp.h"
#include "isp_drv_cam.h"
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include "ispio_stddef.h"

#include <Cam_Notify_datatype.h>

#include <Cam/buf_que_ctrl.h>
#include "cam_capibility.h"
#include <mtkcam/drv/def/ICam_type.h>

using namespace NSImageio::NSIspio;

//CQ sw wrok around for:
/**
a.after hw reset, cq under all modes,no matter what event trig/immediate or continuous, will be read at the very 1st p1_done after cq_en pulled high.
b.descriptor in cq can't be all nop cmd.
*/
#define CQ_SW_WORK_AROUND  1

/**
    this solution is for CAM_B always have sw p1 done under twin mode.
    And this behavior will break the CQ link-list.
    Side effect of current work around solution:
        sw only have (n-1)/n * (time of p1_sof to sw_p1_don) to program whole enque.
*/
#define TWIN_SW_P1_DONE_WROK_AROUND 1

/**
    For main cam support dynamic-twin with slave cam suspend/resume flow, hw physical regs will be
    overwritten by main cam's twin part. When resume, all regs on CQ needs to be reload. Enable
    this flag to support all p1 internal setting load from cq for every frame.
    (Tuning setting in internal module not included in this flag, due to 3A is resposible for load
    full tuning setting along with enque request come with resume API)
*/
#define P1_STANDBY_DTWIN_SUPPORT    1

/**
    top dcm must be disabeld to avoid abnormally TG grab error
*/
#define TOP_DCM_SW_WORK_AROUND  1

/**
    need to close subsample done function under drop frame, otherwise, subsample hw counter will work abnormally.
*/
#define SUBSAMPLE_SW_WORK_AROUND    1

/**
    patch hw default value.  hw didn't assign a right default value
*/
#define UFE_HW_DEFAULT_WORK_AROUND  0
/**
    For DCIF mode, camsv drop frame condition, cam cannot receive hw_p1_done
    Use CQ_START to trigger all CQ thread, causing STT port no dma done but CQ working
    Enable this flag to disable DB of top_ctrl_reg, and dma_en can be write through cmdQ in dummyPage
*/
// #define DCIF_STT_SW_WORK_AROUND    1

/**
    LMV has mux to select 2 different SOF source, but HW bug makes value always load from outter not inner
    workaround is LMV use CQ, and fix mux = 1
*/
#define LMV_DB_SW_WORK_AROUND 1

/**
    CCU uses aao line interrupt(HW capability) as triggered timing to do the job.
    Dut to aao fbc is disabled by cpu, in slow motion case, this may cause ccu's interrupt queue over-length.
    If aao_fbc_en enabled when stop, no data will be outputed to dram anymore.
*/
#define AAO_FBC_LOCK_EN 1

/**
    can't support arbitrary crop , plz reference to define:HW_TIMESTAMP_WORKAROUND.
    if output is cam_b only , there is no timestamp anymore.
*/
#if (HW_TIMESTAMP_WORKAROUND==1)
#define Arbitrary_Crop  (0)
#else
#define Arbitrary_Crop  (1)
#endif

/**
    [ISP 5.0 only]
    For the master cam that will be suspended,
    must disable it's BMX_EN & AMX_EN to avoid TG_overrun happend on another master cam.
*/
#define BMX_AMX_DTWIN_WROK_AROUND    (1)


/**
    Per-frame timeout
*/
#define MIN_GRPFRM_TIME_MS          (200)  //subsampled frame time minimum difference
#define MAX_RECENT_GRPFRM_TIME      (2)


/**
    Dynamic Bin callback control
*/
#define Dynamic_bin_Notify_Frame    (2)


#define UNSUPPORT_CAM CAM_C

/////////////////////////////////////////////////////////////////////////////
/*    ISP Function ID                                                      */
/////////////////////////////////////////////////////////////////////////////

/**
    individual sub-module existed in isp function.
*/
enum {
    CAM_TG = 0,           //0
    CAM_TOP,
    CAM_RAW,
    CAM_RGB,
    CAM_YUV,
    CAM_DMA_CQ0I,         //5
    CAM_DMA_CQ1I,
    CAM_DMA_CQ2I,
    CAM_DMA_CQ3I,
    CAM_DMA_CQ4I,
    CAM_DMA_CQ5I,         //10
    CAM_DMA_CQ6I,
    CAM_DMA_CQ7I,
    CAM_DMA_CQ8I,
    CAM_DMA_CQ9I,
    CAM_DMA_CQ10I,        //15
    CAM_DMA_CQ11I,
    CAM_DMA_CQ12I,
    CAM_DMA_CQ13I,
    CAM_DMA_CQ14I,
    CAM_DMA_CQ15I,        //20
    CAM_DMA_CQ16I,
    CAM_DMA_CQ17I,
    CAM_DMA_CQ18I,
    CAM_DMA_CQ19I,
    CAM_DMA_CQ20I,        //25
    CAM_DMA_CQ21I,
    CAM_DMA_CQ22I,
    CAM_DMA_CQ23I,
    CAM_DMA_CQ24I,
    CAM_DMA_CQ25I,        //30
    CAM_DMA_CQ26I,
    CAM_DMA_CQ27I,
    CAM_DMA_CQ28I,
    CAM_DMA_BPCI,
    CAM_DMA_LSCI,         //35
    CAM_DMA_CACI,
    CAM_DMA_PDI,
    CAM_DMA_IMGO,
    CAM_DMA_UFEO,
    CAM_DMA_RRZO,         //40
    CAM_DMA_AFO,
    CAM_DMA_AAO,
    CAM_DMA_PDO,
    CAM_DMA_LCSO,
    CAM_DMA_TSFSO,        //45
    CAM_DMA_LMVO,
    CAM_DMA_FLKO,
    CAM_DMA_RSSO,
    CAM_DMA_UFGO,
    CAM_DMA_YUVO,         //50
    CAM_DMA_YUVBO,
    CAM_DMA_YUVCO,
    CAM_DMA_CRZO_R1,
    CAM_DMA_CRZBO_R1,
    CAM_DMA_CRZO_R2,      //55
    CAM_DMA_CRZBO_R2,
    CAM_BUFFER_CTRL,
    CAM_TWIN,
    CAM_DMA_RAWI,
    CAM_DMA_UFDI,         //60
    CAM_DCIF,
    CAM_DYUV,
    CAM_FUNCTION_MAX_NUM,
};


typedef enum{
    IF_CAM_A,
    IF_CAM_B,
    IF_CAM_C,
    IF_RAWI_CAM_A,
    IF_RAWI_CAM_B,
    IF_RAWI_CAM_C,
    IF_CAM_MAX,
}ENUM_IF_CAM;

/******************************************************************************
 * @enum E_CamIMGSel
 *
 * @IMG select path, please reference block diagram.
 *
 ******************************************************************************/
typedef enum {
    eIMGSel_0 = 0,
    eIMGSel_1,
    eIMGSel_2,
} E_CamIMGSel;

/******************************************************************************
 * @enum E_CamUFESel
 *
 * @UFE select path, please reference block diagram.
 *
 ******************************************************************************/
typedef enum {
    eUFESel_0 = 0,
    eUFESel_1,
    eUFESel_2,
} E_CamUFESel;

/******************************************************************************
 * @enum E_CamPDOSel
 *
 * @PDO select path, please reference block diagram.
 *
 ******************************************************************************/
typedef enum {
    ePDOSel_0 = 0,
    ePDOSel_1,
} E_CamPDOSel;

/******************************************************************************
 * @enum E_LastP1DoneCtl
 *
 * @Last P1 done Ctl, please reference coda
 *
 ******************************************************************************/
typedef enum {
    eSwReset       = (1L<<0),
    eResetSOF_CamA = (1L<<1),
    eResetSOF_CamB = (1L<<2),
    eResetSOF_CamC = (1L<<3),
    eRefCamA       = (1L<<4),
    eRefCamB       = (1L<<5),
    eRefCamC       = (1L<<6),
} E_LastP1DoneCtl;

/******************************************************************************
 * @enum E_TWIN_DMAO
 *
 * @Twin's Damo
 *
 ******************************************************************************/
typedef enum {
    TWIN_IMGO_R1 = 0,
    TWIN_RRZO_R1,
    TWIN_UFEO_R1,
    TWIN_UFGO_R1,
    TWIN_AFO_R1,
    TWIN_YUVO_R1,
    TWIN_YUVBO_R1,
    TWIN_YUVCO_R1,
    TWIN_CRZO_R1,
    TWIN_CRZBO_R1,
    TWIN_CRZO_R2,
    TWIN_CRZBO_R2,
    TWIN_DMAO_MAX,
} E_TWIN_DMAO;

typedef enum {
    E_CRP_R3_SEL_0 = 0,
    E_CRP_R3_SEL_1,
    E_CRP_R3_SEL_2,
    E_CRP_R3_SEL_3,
    E_CRP_R3_SEL_4,
    E_CRP_R3_SEL_5,
    E_CRP_R3_SEL_6,
    E_CRP_R3_SEL_7
}E_CRP_R3_SEL;

typedef enum{
    E_Notify_Config  = 0, //first config before start
    E_Notify_Update  = 1, //perframe update after all CB is done
}E_SET_NOTIFY_OP;

#define SAVE_FBC(_pDrv,DMAO,_str) do{\
    if (CAM_READ_BITS(_pDrv,FBC_R1_FBC_##DMAO##_CTL1,FBC_##DMAO##_FBC_EN)){\
        char _tmp[32];\
        this->fbc_##DMAO.Raw = CAM_READ_REG(_pDrv,FBC_R1_FBC_##DMAO##_CTL2);\
        _str.append(#DMAO);\
        snprintf(_tmp, sizeof(_tmp), "_0x%x ", this->fbc_##DMAO.Raw);\
        _str.append(_tmp);\
    }\
} while(0)
#define PULL_RCNT_INC(_pDrv,DMAO,_str) do{\
    if (CAM_READ_BITS(_pDrv,FBC_R1_FBC_##DMAO##_CTL1,FBC_##DMAO##_FBC_EN)){\
        MUINT32 frm_cnt = 0;\
        for (frm_cnt = 0; frm_cnt < this->fbc_##DMAO.Bits.FBC_##DMAO##_FBC_CNT; frm_cnt++) {\
            char _tmp[32];\
            CAM_WRITE_BITS(_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC, CAMCTL_##DMAO##_RCNT_INC, 1);\
            _str.append(#DMAO);\
            snprintf(_tmp, sizeof(_tmp), "_0x%x ", CAM_READ_REG(_pDrv, FBC_R1_FBC_##DMAO##_CTL2));\
            _str.append(_tmp);\
        }\
    }\
} while(0)

#define GetP1DoneRef(HwModule) ({\
    MUINT32 P1DoneRef = 0;\
    switch(HwModule){\
        case CAM_A: P1DoneRef = (MUINT32)(eRefCamA | eResetSOF_CamA); break;\
        case CAM_B: P1DoneRef = (MUINT32)(eRefCamB | eResetSOF_CamB); break;\
        case CAM_C: P1DoneRef = (MUINT32)(eRefCamC | eResetSOF_CamC); break;\
        default: CAM_FUNC_ERR("HW module(%d) cant be a slave cam",HwModule); break;\
    }\
    P1DoneRef;\
})

#define SW_RESET(pDrv) do{\
    MUINT32 Last_P1Done = 0;\
    CAM_FUNC_DBG("TOP reset+\n");\
    CAM_WRITE_REG(pDrv,CAMCTL_R1_CAMCTL_SW_CTL,0x0);\
    CAM_WRITE_REG(pDrv,CAMCTL_R1_CAMCTL_SW_CTL,0x1);\
    while((CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_SW_CTL,CAMCTL_SW_RST_ST) != 0x1)){\
        CAM_FUNC_DBG("TOP reseting...SW_CTL:0x%x\n", CAM_READ_REG(pDrv,CAMCTL_R1_CAMCTL_SW_CTL));\
    }\
    Last_P1Done = CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_LAST_PASS1_DONE_CTL);\
    CAM_WRITE_REG(pDrv,CAMCTL_R1_CAMCTL_SW_CTL,0x4);\
    CAM_WRITE_REG(pDrv,CAMCTL_R1_CAMCTL_SW_CTL,0x0);\
    CAM_WRITE_BITS(pDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_LAST_PASS1_DONE_CTL,(Last_P1Done|0x1));\
    CAM_WRITE_BITS(pDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_LAST_PASS1_DONE_CTL,Last_P1Done);\
    CAM_FUNC_DBG("TOP reset-\n");\
} while(0)

template <typename _typ>
class RectData{
public:
    explicit RectData(){H = (_typ)0; V = (_typ)0;};
    _typ H;
    _typ V;
};

/*source form full image */
#define GET_FULL_BIN(pDrv,BIN,opt) do {                                 \
    switch(opt)                                                                 \
    {                                                                           \
        case 0: /*DBN*/                                                        \
            BIN.H = CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN);  \
            BIN.V = 0;                                                          \
            break;                                                              \
        case 1: /*DBN + BIN*/                                                  \
            BIN.H = CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN) + \
                    CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN);  \
            BIN.V = CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN);  \
            break;                                                              \
        case 2: /*DBN + BIN + FBND*/                                          \
            BIN.H = CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN) + \
                    CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN) + \
                    CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN); \
            BIN.V = CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN) + \
                    CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN); \
            break;                                                              \
        default: CAM_FUNC_ERR("unkonwn opt(%d)",opt);break;                     \
    }                                                                           \
} while(0)

/*source from seperated image*/
#define GET_SEP_BIN(pDrv,BIN) do {BIN.H = BIN.V = CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN);} while(0)

#define MAP_IFUNC_PIXMODE(src,_print)({ \
    E_PIX _pixmode = _1_pix_; \
    switch(src){ \
        case ePixMode_1: _pixmode = _1_pix_; break; \
        case ePixMode_2: _pixmode = _2_pix_; break; \
        case ePixMode_4: _pixmode = _4_pix_; break; \
        default: _print("[%d] unsupport pixel mode (%d)",__LINE__,src); break; \
    } \
    _pixmode; \
})

#define MAP_CAM_PIXMODE(src,_print)({ \
    E_CamPixelMode _pixmode = ePixMode_1; \
    switch(src){ \
        case _1_pix_: _pixmode = ePixMode_1; break; \
        case _2_pix_: _pixmode = ePixMode_2; break; \
        case _4_pix_: _pixmode = ePixMode_4; break; \
        default: _print("[%d] unsupport pixel mode (%d)",__LINE__,src); break; \
    } \
    _pixmode; \
})

#define XOR_WRITE_BITS(hwModule,...) do{\
    if(hwModule != UNSUPPORT_CAM){\
       CAM_WRITE_BITS(__VA_ARGS__);\
    }\
} while(0)
#define XOR_WRITE_REG(hwModule,...) do{\
    if(hwModule != UNSUPPORT_CAM){\
        CAM_WRITE_REG(__VA_ARGS__);\
    }\
} while(0)
#define XOR_ADD_MODULE(hwModule,pDrv,arg1) do{\
    if(hwModule != UNSUPPORT_CAM){\
        pDrv->cqAddModule(arg1);\
    }\
} while(0)

#define MAP_TWIN_DMAO(Dmao,opt,LogType) ({\
    MUINT32 tmp = 0;\
    if(opt){\
        switch(Dmao){\
        case TWIN_IMGO_R1:  tmp = _imgo_; break;\
        case TWIN_RRZO_R1:  tmp = _rrzo_; break;\
        case TWIN_UFEO_R1:  tmp = _ufeo_; break;\
        case TWIN_UFGO_R1:  tmp = _ufgo_; break;\
        case TWIN_AFO_R1:   tmp = _afo_; break;\
        case TWIN_YUVO_R1:  tmp = _yuvo_; break;\
        case TWIN_YUVBO_R1: tmp = _yuvbo_; break;\
        case TWIN_YUVCO_R1: tmp = _yuvco_; break;\
        case TWIN_CRZO_R1:  tmp = _crzo_; break;\
        case TWIN_CRZO_R2:  tmp = _crzo_r2_; break;\
        case TWIN_CRZBO_R1: tmp = _crzbo_; break;\
        case TWIN_CRZBO_R2: tmp = _crzbo_r2_; break;\
        default: LogType##_ERR("unsupport opt(%d) module(%d)",opt,Dmao);break;\
        }\
    } else {\
        switch(Dmao){\
        case _imgo_ :    tmp = TWIN_IMGO_R1; break;\
        case _rrzo_ :    tmp = TWIN_RRZO_R1; break;\
        case _ufeo_:     tmp = TWIN_UFEO_R1; break;\
        case _ufgo_:     tmp = TWIN_UFGO_R1; break;\
        case _afo_:      tmp = TWIN_AFO_R1; break;\
        case _yuvo_:     tmp = TWIN_YUVO_R1; break;\
        case _yuvbo_:    tmp = TWIN_YUVBO_R1; break;\
        case _yuvco_:    tmp = TWIN_YUVCO_R1; break;\
        case _crzo_:     tmp = TWIN_CRZO_R1; break;\
        case _crzo_r2_:  tmp = TWIN_CRZO_R2; break;\
        case _crzbo_:    tmp = TWIN_CRZBO_R1; break;\
        case _crzbo_r2_: tmp = TWIN_CRZBO_R2; break;\
        default: LogType##_ERR("unsupport opt(%d) module(%d)",opt,Dmao);break;\
        }\
    }\
    tmp;\
})

#define isThroughRRZ(pDrv) ({\
    MBOOL ret = MFALSE; \
    switch(CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DM_R1_SEL)){ \
    case 0: ret = MFALSE; break; \
    case 1: ret = CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_EN2, CAMCTL_RRZ_R1_EN) ? MTRUE : MFALSE; break; \
    default: CAM_FUNC_ERR("unknown sel_%d",CAM_READ_BITS(pDrv,CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DM_R1_SEL)); \
        break; \
    } \
    ret; \
})

#define MAP_MODULE_TG(input,op, _print) ({\
    MUINT32 output = 0; \
    if(op) { /*Map HwModule to TG*/ \
        switch(input){ \
        case CAM_A: output = (MUINT32)TG_A; break; \
        case CAM_B: output = (MUINT32)TG_B; break; \
        case CAM_C: output = (MUINT32)TG_C; break; \
        default: _print("unknown input_%d",input); \
            break; \
        } \
    } \
    else { /*Map TG to HwModule*/\
        switch(input){ \
        case TG_A: output = (MUINT32)CAM_A; break; \
        case TG_B: output = (MUINT32)CAM_B; break; \
        case TG_C: output = (MUINT32)CAM_C; break; \
        default: _print("unknown input_%d",input); \
            break; \
        } \
    } \
    output; \
})

#define MAP_RAW_TG_SEL(TG_X) ({\
    ENUM_RAW_TG_SEL RAW_TG_SEL = FROM_TG_A; \
    switch(TG_X){ \
    case TG_A: RAW_TG_SEL = FROM_TG_A; break; \
    case TG_B: RAW_TG_SEL = FROM_TG_B; break; \
    case TG_C: RAW_TG_SEL = FROM_TG_C; break; \
    default: CAM_FUNC_ERR("unknown TG_%d",TG_X); \
        break; \
    } \
    RAW_TG_SEL; \
})

#define CAM_READ_REG_INR(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
               (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readInnerReg(__offset,CAM_A);\
})

#define CAM_READ_BITS_INR(IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
               (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readInnerReg(__offset,CAM_A);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

///////////////////////////////////////////////////////////////////////////////
/**
    Class for TG module
*/
class CAM_TG_CTRL : public IspFunction_B
{
public:
    typedef struct _LINKED_TG {
        ISP_HW_MODULE hwModule   = PHY_CAM;
        E_INPUT       Input      = TG_CAM_MAX;
        IspDrv*       pIspPhyDrv = NULL;
        MBOOL         bSelfTG    = MTRUE;
    } LINKED_TG;

    CAM_TG_CTRL();

    virtual MUINT32     id( void )                    {   return CAM_TG;  }
    virtual const char* name_Str( void )              {   return "CAM_TG";}
            MINT32      pre_enable();
            MINT32      pre_disable();


protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam = NULL );
    virtual MINT32 _disable( void* pParam = NULL );
    virtual MINT32 _write2CQ( MUINT32 burstIndex ){(void)burstIndex;return 0;}

public:
    MBOOL             m_continuous;   //signle or continuous mode
    MUINT32           m_SubSample;    //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample
    IspRect           m_Crop;
    E_CamPixelMode    m_PixMode;
    E_CamPattern      m_Datapat;

private:
    ISP_DRV_CAM*    m_pDrv;     //for casting type , coding convenience only
};

///////////////////////////////////////////////////////////////////////////////
/**
    Class for CAM Top module
*/
class CAM_TOP_CTRL : public IspFunction_B
{
friend class CAM_TWIN_PIPE;
public:
    CAM_TOP_CTRL();

    virtual MUINT32     id( void )                    {   return CAM_TOP;  }
    virtual const char* name_Str( void )              {   return "CAM_TOP";}

            MBOOL       suspend(void){return MFALSE;}
            MBOOL       resume(void){return MFALSE;}
            MBOOL       HW_recover(MUINT32 step); //set 0:stop pipeline. set 1:restart pipeline
protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam  );
    virtual MINT32 _disable( void* pParam = NULL );
    virtual MINT32 _write2CQ( MUINT32 burstIndex );

public:
    ST_CAM_TOP_CTRL     cam_top_ctl;
    ENUM_IF_CAM         CAM_Path;       //only used for TG_IN or DMAI
    MUINT32             SubSample;      //signal subample . 0: no subsample. Vsync/SOF/P1_DONE use the sample subsample

private:
            ISP_DRV_CAM*    m_pDrv;     //for casting type , coding convenience only
            REG_FBC_R1_FBC_IMGO_R1_CTL2     fbc_IMGO_R1; //for recover fbc cnt
            REG_FBC_R1_FBC_RRZO_R1_CTL2     fbc_RRZO_R1;
            REG_FBC_R1_FBC_UFEO_R1_CTL2     fbc_UFEO_R1;
            REG_FBC_R1_FBC_UFGO_R1_CTL2     fbc_UFGO_R1;
            REG_FBC_R1_FBC_AFO_R1_CTL2      fbc_AFO_R1;
            REG_FBC_R1_FBC_AAO_R1_CTL2      fbc_AAO_R1;
			REG_FBC_R1_FBC_TSFSO_R1_CTL2    fbc_TSFSO_R1;
            REG_FBC_R1_FBC_LCESO_R1_CTL2    fbc_LCESO_R1;
            REG_FBC_R1_FBC_PDO_R1_CTL2      fbc_PDO_R1;
            REG_FBC_R1_FBC_FLKO_R1_CTL2     fbc_FLKO_R1;
            REG_FBC_R1_FBC_LMVO_R1_CTL2     fbc_LMVO_R1;
            REG_FBC_R1_FBC_RSSO_R1_CTL2     fbc_RSSO_R1;
            REG_FBC_R1_FBC_YUVO_R1_CTL2     fbc_YUVO_R1;
            REG_FBC_R1_FBC_YUVBO_R1_CTL2    fbc_YUVBO_R1;
            REG_FBC_R1_FBC_YUVCO_R1_CTL2    fbc_YUVCO_R1;
            REG_FBC_R1_FBC_CRZO_R1_CTL2     fbc_CRZO_R1;
            REG_FBC_R1_FBC_CRZO_R2_CTL2     fbc_CRZO_R2;
            REG_FBC_R1_FBC_CRZBO_R1_CTL2    fbc_CRZBO_R1;
            REG_FBC_R1_FBC_CRZBO_R2_CTL2    fbc_CRZBO_R2;
};

class CAM_DCIF_CTRL : public IspFunction_B
{
public:
    explicit CAM_DCIF_CTRL() : m_pDrv(NULL)
                                      , m_bRawInUF(MFALSE)
                                      , m_bSvTwin(MFALSE)
                                      {};
    virtual ~CAM_DCIF_CTRL(){};
    virtual MUINT32 id( void ) {   return CAM_DCIF;  }
    virtual const char* name_Str( void ) {   return "CAM_DCIF";}

protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam  );
    virtual MINT32 _disable( void* pParam = NULL );
    virtual MINT32 _write2CQ( MUINT32 burstIndex );

public:
    MBOOL           m_bRawInUF;
    MBOOL           m_bSvTwin;
    MSize           m_SrcSize;

protected:
    ISP_DRV_CAM*    m_pDrv;
};

typedef struct _T_DLP
{
    E_CamPixelMode RAW_SELO;
    E_CamPixelMode SEPI;
    E_CamPixelMode SEPO;

    E_CamPixelMode MRG_R3;
    E_CamPixelMode MRG_R12;

    E_CamPixelMode MRG_R7;
    E_CamPixelMode MRG_R11;

    E_CamPixelMode MRG_R13;
    E_CamPixelMode MRG_R14;

    E_CamPixelMode MRG_R8;
    E_CamPixelMode MRG_R6;

    _T_DLP()
    :RAW_SELO(ePixMode_1)
    ,SEPI(ePixMode_1)
    ,SEPO(ePixMode_1)
    ,MRG_R3(ePixMode_1)
    ,MRG_R12(ePixMode_1)
    ,MRG_R7(ePixMode_1)
    ,MRG_R11(ePixMode_1)
    ,MRG_R13(ePixMode_1)
    ,MRG_R14(ePixMode_1)
    ,MRG_R8(ePixMode_1)
    ,MRG_R6(ePixMode_1){}
}T_DLP; /*Direct Link Point*/

/**
    class for SEP contrl
*/
class CAM_RAW_PIPE;
class CAM_DYUV_PIPE;

template <typename _typ>
class CAM_DLP_CTRL
{
public:
    CAM_DLP_CTRL(){m_hwModule = CAM_MAX;m_pRaw_p = NULL;}
    virtual ~CAM_DLP_CTRL(){}
    inline MINT32 config(_typ* _this){
        this->m_pRaw_p = _this;
        this->m_hwModule = _this->m_hwModule;
        return this->_config();
    };

private:
    virtual MINT32 _config(void){return -1;}

protected:
    _typ*           m_pRaw_p;
    ISP_HW_MODULE   m_hwModule;
};
class CAM_SEP_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );
};
class CAM_MRG_R1_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );
};
class CAM_MRG_R2_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );
};
class CAM_MRG_R3_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );

};
class CAM_MRG_R4_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );
};
class CAM_MRG_R5_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );

};
class CAM_MRG_R6_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );
};
class CAM_MRG_R7_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );

};
class CAM_MRG_R8_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );

};
class CAM_MRG_R10_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );
};
class CAM_MRG_R13_CTRL:public CAM_DLP_CTRL<CAM_DYUV_PIPE>
{
private:
    virtual MINT32 _config( void );
};
class CAM_CRP_R1_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
private:
    virtual MINT32 _config( void );

};
class CAM_CRP_R3_CTRL:public CAM_DLP_CTRL<CAM_RAW_PIPE>
{
public:
    explicit CAM_CRP_R3_CTRL(){};
private:
    virtual MINT32 _config( void );
};
class CAM_CRP_R4_CTRL:public CAM_DLP_CTRL<CAM_DYUV_PIPE>
{
private:
    virtual MINT32 _config( void );
};

class CAM_CRSP_CTRL:public CAM_DLP_CTRL<CAM_DYUV_PIPE>
{
public:
    typedef enum {
        CRSP_R1 = 1,
        CRSP_R2 = 2,
    } E_CRSP_RX;

    CAM_CRSP_CTRL(E_CRSP_RX RX){mCRSP_RX = RX; mNameStr = "CRSP_R" + std::to_string(RX);}
    virtual ~CAM_CRSP_CTRL(){}
    virtual MUINT32     id( void ) {return mCRSP_RX;}
    virtual const char* name_Str( void ) {return mNameStr.c_str();}
private:
    virtual MINT32 _config( void );

private:
    E_CRSP_RX     mCRSP_RX;
    std::string   mNameStr;
};


typedef union {
    struct {
        MUINT32 PAK_DMAO         :  1; //0: Imgo 1:Yuvo 2:Rawi
        MUINT32 PAK_DNG          :  1; //0:non-DNG; 1:DNG
        MUINT32 PAK_BIT          :  1; //0:8bit; 1:10bit
        MUINT32 PAK_R3_ODR       :  2; //PAK_R3_ODR
        MUINT32 PAK_R4_ODR       :  2; //PAK_R4_ODR
        MUINT32 rsv              : 25;
    } Bits;
    MUINT32 Raw;
}Pak_Func;

class CAM_SUB_PIPE
{
public:
    CAM_SUB_PIPE(){m_hwModule = CAM_MAX;m_pMainPipe = NULL;}
    virtual ~CAM_SUB_PIPE(){}
    virtual MUINT32     id( void ){return 0;}
    virtual const char* name_Str( void ) {return "CAM_SUB_PIPE";}

    inline MBOOL        config(){return this->_config();}
    inline MBOOL        write2CQ(){return this->_write2CQ();}

private:
    virtual MBOOL       _config(){return MFALSE;}
    virtual MBOOL       _write2CQ(){return MFALSE;}

public:
    CAM_RAW_PIPE*   m_pMainPipe;

protected:
    ISP_HW_MODULE   m_hwModule;
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CAM  when input data type is DYUV
*/
class CAM_DYUV_PIPE: public CAM_SUB_PIPE
{
    friend class CAM_DLP_CTRL<CAM_DYUV_PIPE>;
    friend class CAM_MRG_R13_CTRL;
    friend class CAM_CRP_R4_CTRL;
    friend class CAM_CRSP_CTRL;

public:

    CAM_DYUV_PIPE(){m_pDrv = NULL;}
    virtual ~CAM_DYUV_PIPE(){}
    virtual MUINT32     id( void ) {return CAM_DYUV;}
    virtual const char* name_Str( void ) {return "CAM_DYUV";}
    MBOOL               setCRZ(MUINT32 RX);
    MBOOL               dynamicPak(const MUINT32 hwYuvoFmt, const Pak_Func Func);
    MUINT32             getCurPixMode(_isp_dma_enum_ dmao);
    MVOID               updateMrgInSize();
protected:
    virtual MBOOL       _config();
    virtual MBOOL       _write2CQ();

public:
    ISP_DRV_CAM*     m_pDrv;

protected:
    CAM_MRG_R13_CTRL m_MrgR13;
    CAM_CRP_R4_CTRL  m_CrpR4;
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CAM when input data type is Bayer, change raw pipe setting under streaming
*/
class CAM_RAW_PIPE:public IspFunction_B
{
public:
    friend class CAM_DLP_CTRL<CAM_RAW_PIPE>;
    friend class CAM_SUB_PIPE;
    friend class CAM_DYUV_PIPE;
	friend class CAM_CRZ_CTRL;
    friend class CAM_SEP_CTRL;
    friend class CAM_MRG_R1_CTRL;
    friend class CAM_MRG_R2_CTRL;
    friend class CAM_MRG_R3_CTRL;
    friend class CAM_MRG_R4_CTRL;
    friend class CAM_MRG_R5_CTRL;
    friend class CAM_MRG_R6_CTRL;
    friend class CAM_MRG_R7_CTRL;
    friend class CAM_MRG_R8_CTRL;
    friend class CAM_MRG_R10_CTRL;
    friend class CAM_CRP_R1_CTRL;
    friend class CAM_CRP_R3_CTRL;

public:
    typedef enum{
        E_FromTG    = 0,
        E_BeforLSC  = 1,
        E_AFTERLSC  = 2,
        E_BetweenLSCWB  = 3,
    }E_RawType;

    CAM_RAW_PIPE();
    virtual ~CAM_RAW_PIPE();

public:
    virtual MUINT32     id( void )                    {   return CAM_RAW;  }
    virtual const char* name_Str( void )              {   return "CAM_RAW";}

    virtual MINT32  setNotify(E_SET_NOTIFY_OP op);
            MBOOL   setLMV(MBOOL &bWdmaEn);                       //because of LMV have the requirement of real time sync with rrz. so, need additional process
            MBOOL   setLCS(MBOOL &bWdmaEn);                       //because of LCS of sync requirement when LCS croping r needed.
            MBOOL   setGSE(void);                  //sgg2 have 2 user:eis/rss, so using CB method the same as EIS/LCS. avoid Racing condition
            MBOOL   setRSS(MBOOL &bWdmaEn);
            MBOOL   setRRZ(void);                      //for dynamical bin switch & per-frame notify MW
            MBOOL   setCRZ(MUINT32 RX){return m_Dyuv.setCRZ(RX);}
            MBOOL   set3A(void);
            MBOOL   setIMG_SEL(void);
    inline  void    CBTimeChk(char *str, MBOOL bStartT);
            MUINT32 getCurPixMode(_isp_dma_enum_ dmao);
            MBOOL   dynamicPak(MUINT32 hwdmaoFmt, Pak_Func Func); // perframe control pak/unpak Imgo format
    virtual MINT32  DoneCollector(MBOOL En, E_INPUT InPut);
            MBOOL   configNotify(void);
            MBOOL   configCB(MBOOL bForce);
            MBOOL   setNotifyDone(void);

protected:
    virtual MINT32 _config( void );             //for initial only, work with _config() in CAM_TOP_CTRL
    virtual MINT32 _write2CQ( MUINT32 burstIndex );

    virtual MINT32 _enable( void* pParam  ) {(void)pParam;return 0;}
    virtual MINT32 _disable( void* pParam = NULL ) {(void)pParam;return 0;}
    inline  MBOOL  setRAW_SEL(void);

public:
    REG_TG_R1_TG_SEN_GRAB_PXL   m_TG_W; //LafitEP_TODO: need remove after fix tuning
    REG_TG_R1_TG_SEN_GRAB_LIN   m_TG_H; //LafitEP_TODO: need remove after fix tuning
	BIN_INPUT_INFO			  m_input;
    E_RawType                   m_RawType;
    ENUM_IF_CAM                 m_Source;
    P1_TUNING_NOTIFY*           m_pP1Tuning;
    T_DLP                       m_dlp;
    MBOOL                       m_bDYUV;
    MBOOL                       m_bBin;
    MBOOL                       m_bRRz;         //for 3-raw case, once if 3-raw + h-ratio zoomin,  rrz at main-cam will be disabled by twin drv.
                                                //use this member to keep scenario-level's rrz info
    E_CamPattern                m_DataPat;
    E_CamPattern                m_DataPat_config;//keep
    MSize                       m_SrcSize;
    E_INPUT                     m_SrcTG;
    MUINT32                     m_rrz_rlb_offset;

private:
    MUINT32                     m_NotifyFrame;

protected:
    ISP_DRV_CAM                *m_pDrv;
    CQ_DUMP_INFO                m_dumpInput; //for RegDumpCB use
    CAM_SEP_CTRL                m_Sep;
    CAM_MRG_R1_CTRL             m_MrgR1;
    CAM_MRG_R2_CTRL             m_MrgR2;
    CAM_MRG_R3_CTRL             m_MrgR3;
    CAM_MRG_R4_CTRL             m_MrgR4;
    CAM_MRG_R5_CTRL             m_MrgR5;
    CAM_MRG_R6_CTRL             m_MrgR6;
    CAM_MRG_R7_CTRL             m_MrgR7;
    CAM_MRG_R8_CTRL             m_MrgR8;
    CAM_MRG_R10_CTRL            m_MrgR10;
    CAM_CRP_R1_CTRL             m_RcpR1;
    CAM_CRP_R3_CTRL             m_RcpR3;
    CAM_DYUV_PIPE               m_Dyuv;
    MVOID                      *pIspReg;
    MBOOL                       m_CurBin;
    MUINT32                     m_CurQbn1Acc; //acc of qbin1 & rmb (pixel mode)
};

/////////////////////////////////////////////////////////////////////////////////////
class CAM_DPD_CTRL: public CAM_RAW_PIPE
{
protected:
    virtual MINT32 _config( void );
    virtual MINT32 _write2CQ( MUINT32 burstIndex );

    virtual MINT32 _enable( void* pParam  );
    virtual MINT32 _disable( void* pParam = NULL );
public:
            MINT32 PD_PATH_EN(MBOOL enable);        //dual-pd is not supported

public:
    typedef enum{
        _level_1    = 0,
        _level_2    = 1,
        _level_max  = 2,
    }E_Density;

            E_Density   m_Density;
private:
            static MBOOL       m_pdo_en[PHY_CAM];
};

/////////////////////////////////////////////////////////////////////////////////////
/**
    spare register of Frame Header mapping function
*/
#define E_HEADER_MAX    (16)
class HEADER_PARSER
{
protected:
    virtual ~HEADER_PARSER(){}
public:

    virtual void Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value) = 0;
    virtual void Header_Deque(MUINTPTR FH_VA,void* pOut) = 0;

public:
            ISP_HW_MODULE   m_hwModule;
};

/**
    spare register of Frame Header mapping function for imgo
*/
class Header_IMGO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_RAW_TYPE          = 3,    //spare4
        E_PIX_ID            = 4,    //spare5
        E_FMT               = 5,    //spare6
        E_ENQUE_CNT         = 6,    //spare7
        E_SRC_SIZE          = 7,    //spare8    [31]:dbn_on/off, [30:16]:size_h, [15:0]:size_w
        E_IQ_LEVEL          = 8,    //spare9    record high or low image quality, spare9 is shared by iq_level and is_ufe_fmt
        E_CROP_START        = 9,    //spare10   [31:16]:start_y, [15:0]:start_x
        E_CROP_SIZE         = 10,   //spare11   [31:16]:size_h, [15:0]:size_w
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_IS_UFE_FMT        = 13,   //spare9    record UFE fmt, per-frame changed
        E_MAX
    };
public:
    Header_IMGO()
    {
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for rrzo
*/
class Header_RRZO: public HEADER_PARSER
{
public:

    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_PIX_ID            = 3,    //spare4
        E_FMT               = 4,    //spare5
        E_ENQUE_CNT         = 5,    //spare6
        E_IS_UFG_FMT        = 6,    //spare7    record UFG fmt, per-frame changed
        E_IQ_LEVEL          = 7,    //spare8    record high or low image quality
        E_RRZ_CRP_START     = 8,    //spare9    [31:16]:start_y, [15:0]:start_x
        E_RRZ_CRP_SIZE      = 9,    //sapre10   [31:16]:size_h, [15:0]:size_w
        E_RRZ_DST_SIZE      = 10,   //spare11   [31:16]:dst_h,  [15:0]:dst_w
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
    static Mutex   m_tagAddrLock;
public:
    Header_RRZO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);    //for pipe chk use
     static MUINT32 GetTagAddr(MUINT32 Tag);
};

/**
    spare register of Frame Header mapping function for ufeo
*/
class Header_UFEO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_ENQUE_CNT         = 3,    //spare4
        E_FMT               = 4,    //spare5
        E_UFEO_OFFST_ADDR   = 5,    //spare6
        E_UFEO_XSIZE        = 6,    //spare7
        E_IMGO_OFFST_ADDR   = 7,    //spare8
        E_IQ_LEVEL          = 8,    //spare9    record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_UFEO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for ufeo
*/
class Header_UFGO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_ENQUE_CNT         = 3,    //spare4
        E_FMT               = 4,    //spare5
        E_UFGO_OFFST_ADDR   = 5,    //spare6
        E_UFGO_XSIZE        = 6,    //spare7
        E_RRZO_OFFST_ADDR   = 7,    //spare8
        E_IQ_LEVEL          = 8,    //spare9    record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_UFGO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for AAO
*/
class Header_AAO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_IQ_LEVEL          = 2,    //spare3    record high or low image quality
        E_SENSOR_ID         = 3,    //spare4
        E_ENQUE_SOF         = 9,    //spare10   SOF need to be first enum
        E_IMG_PA            = 10,   //spare11
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_AAO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};


/**
    spare register of Frame Header mapping function for AFO
*/
class Header_AFO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_STRIDE            = 2,    //spare3
        E_IQ_LEVEL          = 3,    //spare4    record high or low image quality
        E_SENSOR_ID         = 4,    //spare5
        E_ENQUE_SOF         = 9,    //spare10   SOF need to be first enum.
        E_IMG_PA            = 10,   //spare11
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_AFO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};


/**
    spare register of Frame Header mapping function for LCSO
*/
class Header_LCSO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2    buf_ctrl dbg log
        E_ENQUE_SOF         = 2,    //spare3
        E_ENQUE_CNT         = 3,    //spare4
        E_IQ_LEVEL          = 4,    //spare5    record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_LCSO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};

/**
    spare register of Frame Header mapping function for PDO
*/
class Header_PDO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_STRIDE            = 2,    //spare3
        E_SENSOR_ID         = 3,    //spare4
        E_IMG_PA_OFSET      = 7,    //spare8
        E_IQ_LEVEL          = 8,    //spare9    record high or low image quality
        E_ENQUE_SOF         = 9,    //spare10   SOF need to be first enum.
        E_IMG_PA            = 10,   //spare11
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_PDO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};

/**
    spare register of Frame Header mapping function for TSFSO
*/
class Header_TSFSO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2    buf_ctrl dbg log
        E_IQ_LEVEL          = 2,    //spare3    record high or low image quality
        E_SENSOR_ID         = 3,    //spare4
        E_ENQUE_SOF         = 9,    //spare10   SOF need to be first enum
        E_IMG_PA            = 10,   //spare11
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_TSFSO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};

/**
    spare register of Frame Header mapping function for EISO
*/
class Header_LMVO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_ENQUE_CNT         = 3,    //spare4
        E_IQ_LEVEL          = 4,    //spare5    record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_LMVO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};


/**
    spare register of Frame Header mapping function for FLKO
*/
class Header_FLKO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_IQ_LEVEL          = 2,    //spare3    record high or low image quality
        E_SENSOR_ID         = 3,    //spare4
        E_ENQUE_SOF         = 8,    //spare9    buf_ctrl dbg log
        E_IMG_PA            = 9,    //spare10
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_FLKO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};

/**
    spare register of Frame Header mapping function for RSSO
*/
class Header_RSSO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_ENQUE_CNT         = 3,    //spare4
        E_DST_SIZE          = 4,    //spare5    [31:16]:dst_h, [15:0]:dst_w
        E_IQ_LEVEL          = 5,    //spare6    record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_RSSO(){
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
};

/**
    spare register of Frame Header mapping function for YUVO
*/
class Header_YUVO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_PLANE_EN          = 3,    //spare4    b'0:yuvo_r1 b'1:yuvbo_r1 b'2:yuvco_r1
        E_SW_FMT            = 4,    //spare5
        E_FMT               = 5,    //spare6
        E_CROP_START        = 6,    //spare7    [31:16]:start_y, [15:0]: start_x
        E_CROP_SIZE         = 7,    //spare8    [31:16]:size_h, [15:0]: size_w
        E_ENQUE_CNT         = 8,    //spare9
        E_SRC_SIZE          = 9,    //spare10   [31:16]:size_h, [15:0]: size_w
        E_IQ_LEVEL          = 10,   //spare11   record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_YUVO()
    {
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for YUVBO
*/
class Header_YUVBO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_PIX_ID            = 4,    //spare5
        E_FMT               = 5,    //spare6
        E_CROP_START        = 6,    //spare7    [31:16]:start_y, [15:0]: start_x
        E_CROP_SIZE         = 7,    //spare8    [31:16]:size_h, [15:0]: size_w
        E_ENQUE_CNT         = 8,    //spare9
        E_SRC_SIZE          = 9,    //spare10   [31:16]:size_h, [15:0]: size_w
        E_IQ_LEVEL          = 10,   //spare11   record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_YUVBO()
    {
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for YUVCO
*/
class Header_YUVCO: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_PIX_ID            = 4,    //spare5
        E_FMT               = 5,    //spare6
        E_CROP_START        = 6,    //spare7    [31:16]:start_y, [15:0]: start_x
        E_CROP_SIZE         = 7,    //spare8    [31:16]:size_h, [15:0]: size_w
        E_ENQUE_CNT         = 8,    //spare9
        E_SRC_SIZE          = 9,    //spare10   [31:16]:size_h, [15:0]: size_w
        E_IQ_LEVEL          = 10,   //spare11   record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_YUVCO()
    {
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for CRZO_R1
*/
class Header_CRZO_R1: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_SW_FMT            = 3,    //spare4
        E_FMT               = 4,    //spare5
        E_CRZ_CROP_START    = 5,    //spare6    [31:16]:start_y, [15:0]: start_x
        E_CRZ_CROP_SIZE     = 6,    //spare7    [31:16]:size_h, [15:0]: size_w
        E_CRZ_DST_SIZE      = 7,    //spare8    [31:16]:size_h, [15:0]: size_w
        E_ENQUE_CNT         = 8,    //spare9
        E_PLANE_EN          = 9,    //spare10   b'0:crzo_r1 b'1:crzbo_r1
        E_IQ_LEVEL          = 10,   //spare11   record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_CRZO_R1()
    {
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for CRZBO_R1
*/
class Header_CRZBO_R1: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_PIX_ID            = 3,    //spare4
        E_FMT               = 4,    //spare5
        E_CRZ_CROP_START    = 5,    //spare6    [31:16]:start_y, [15:0]: start_x
        E_CRZ_CROP_SIZE     = 6,    //spare7    [31:16]:size_h, [15:0]: size_w
        E_CRZ_DST_SIZE      = 7,    //spare8    [31:16]:size_h, [15:0]: size_w
        E_ENQUE_CNT         = 8,    //spare9
        E_IQ_LEVEL          = 9,    //spare10   record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_CRZBO_R1()
    {
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for CRZO_R2
*/
class Header_CRZO_R2: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_SW_FMT            = 3,    //spare4
        E_FMT               = 4,    //spare5
        E_CRZ_CROP_START    = 5,    //spare6    [31:16]:start_y, [15:0]: start_x
        E_CRZ_CROP_SIZE     = 6,    //spare7    [31:16]:size_h, [15:0]: size_w
        E_CRZ_DST_SIZE      = 7,    //spare8    [31:16]:size_h, [15:0]: size_w
        E_ENQUE_CNT         = 8,    //spare9
        E_PLANE_EN          = 9,    //spare10   b'0:crzo_r2 b'1:crzbo_r2
        E_IQ_LEVEL          = 10,   //spare11   record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_CRZO_R2()
    {
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    spare register of Frame Header mapping function for CRZBO_R2
*/
class Header_CRZBO_R2: public HEADER_PARSER
{
public:
    enum {
        E_TimeStamp_LSB     = 0,    //spare1
        E_Magic             = 1,    //spare2
        E_ENQUE_SOF         = 2,    //spare3    buf_ctrl dbg log
        E_PIX_ID            = 3,    //spare4
        E_FMT               = 4,    //spare5
        E_CRZ_CROP_START    = 5,    //spare6    [31:16]:start_y, [15:0]: start_x
        E_CRZ_CROP_SIZE     = 6,    //spare7    [31:16]:size_h, [15:0]: size_w
        E_CRZ_DST_SIZE      = 7,    //spare8    [31:16]:size_h, [15:0]: size_w
        E_ENQUE_CNT         = 8,    //spare9
        E_IQ_LEVEL          = 9,    //spare10   record high or low image quality
        E_IMG_PA            = 11,   //spare12
        E_TimeStamp_MSB     = 12,   //spare13
        E_MAX
    };
public:
    Header_CRZBO_R2()
    {
        m_hwModule = CAM_MAX;
    }

    virtual void    Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value);
    virtual void    Header_Deque(MUINTPTR FH_VA,void* pOut);
            void    Header_Flash(MUINTPTR FH_VA);
            MUINT32 GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr);
};

/**
    class for RRZ contrl
*/
class CAM_RRZ_CTRL: public CAM_RAW_PIPE
{
public:
    class RRZ_PQ{
    public:
        RRZ_PQ():
            PRF_BLD(0),
            PRF(0),
            BLD_SL(16),
            CR_MODE(1),
            TH_MD(80),
            TH_HI(40),
            TH_LO(80),
            TH_MD2(0)
        {};
    public:
        const MUINT8  PRF_BLD;
        const MUINT8  PRF;
        const MUINT8  BLD_SL;
        const MUINT8  CR_MODE;
        const MUINT8  TH_MD;
        const MUINT8  TH_HI;
        const MUINT8  TH_LO;
        const MUINT8  TH_MD2;
    };

public:
    CAM_RRZ_CTRL(){rrz_rlb_offset = 0;};

    virtual MINT32 _config( void );
    virtual MINT32 _write2CQ( MUINT32 burstIndex ){(void)burstIndex;return 0;}
            MINT32 setDefault(void);

private:
     inline MUINT32 calCoefStep(MUINT32 in,MUINT32 crop,MUINT32 crop_ofst,MUINT32 crop_sub_ofst,MUINT32 out);
     inline MUINT32 getCoefNNIRTbl(MUINT32 tableSel);
            MUINT32 getCoefTbl(MUINT32 resize_in,MUINT32 resize_out);

public:
    IspRect         rrz_in_roi;
    IspSize         rrz_out_size;
    MUINT32         rrz_rlb_offset;
};

/**
    class for CRZ contrl
*/
class CAM_CRZ_CTRL: public CAM_DYUV_PIPE
{
#define MASK_TABLE_SELECT              (0x1F)
#define MASK_IMAGE_SIZE                (0x1FFF)
#define MASK_COEFF_STEP                (0x7FFFFF)
#define MASK_INT_OFFSET                (0x1FFF)
#define MASK_SUB_OFFSET                (0x1FFFFF)
#define RZ_4_TAP_RATIO_MAX             (32)
#define RZ_4_TAP_RATIO_MIN             (2)
#define RZ_TABLE_OFFSET                (20)
#define RZ_TABLE_AMOUNT                (6)
#define RZ_4N_TAP_RATIO_MAX            (2)
#define RZ_4N_TAP_RATIO_MIN            (8)
#define RZ_N_TAP_TABLE                 (0)
#define RZ_N_TAP_RATIO_MAX             (8)
#define RZ_N_TAP_RATIO_MIX             (256)

public:
    typedef enum {
        CRZ_R1 = 1,
        CRZ_R2 = 2,
    } E_CRZ_RX;

    typedef enum {
        ALGO_6_TAP, // For 95, only 6-tap.
        ALGO_N_TAP,
        ALGO_4N_TAP,
        ALGO_AMOUNT
    } E_CRZ_ALGO;
    explicit  CAM_CRZ_CTRL(E_CRZ_RX RX, CAM_RAW_PIPE* _this, MBOOL m_bIsCB);
    virtual MUINT32     id( void ) {return mCRZ_RX;}
    virtual const char* name_Str( void ) {return mNameStr.c_str();}
    virtual MINT32      _config( void );
    virtual MINT32      _write2CQ( void ){return 0;}
    MVOID               updateCrzInSize();

private:
    inline MBOOL CalOffset(E_CRZ_ALGO          Algo,
                              MBOOL               IsWidth,
                              MUINT32             CoeffStep,
                              MUINT32             Offset,
                              MUINT32*            pLumaInt,
                              MUINT32*            pLumaSub,
                              MUINT32*            pChromaInt,
                              MUINT32*            pChromaSub);
    MBOOL calCoefStep(MUINT32                 SizeIn_H,
                          MUINT32                 SizeIn_V,
                          MUINT32                 u4CroppedSize_H,
                          MUINT32                 u4CroppedSize_V,
                          MUINT32                 SizeOut_H,
                          MUINT32                 SizeOut_V,
                          E_CRZ_ALGO              *pAlgo_H,
                          E_CRZ_ALGO              *pAlgo_V,
                          MUINT32                 *pTable_H,
                          MUINT32                 *pTable_V,
                          MUINT32                 *pCoeffStep_H,
                          MUINT32                 *pCoeffStep_V);

public:
    MSize   crz_in;
    IspRect crz_in_roi;
    IspSize crz_out_size;
    MBOOL   m_bIsCB; //CRZ config is not like RRZ, only config once by CB

private:
    const MUINT32       kRzUint[ALGO_AMOUNT] = {32768, 1048576, 1048576};
    const MUINT32       kRzTbl[RZ_TABLE_AMOUNT] = {36409, 40961, 46812, 54614, 59579, 65537};
          E_CRZ_RX      mCRZ_RX;
          string        mNameStr;
          MBOOL         mFgVertFirst;
};

//////////////////////////////////////////////////////////////////////////////////
/**
    class for CAM under twin control
*/
#define _SLAVE_CAM_MAX_ (1)
class CAM_TWIN_PIPE:public CAM_RAW_PIPE
{
public:
    CAM_TWIN_PIPE();
public:
    virtual MUINT32     id( void )                    {   return CAM_TWIN;  }
    virtual const char* name_Str( void )              {   return "CAM_TWIN";}

    typedef enum {
        E_ENABLE_TWIN = 0,
        E_BEFORE_TWIN,
        E_AFTER_TWIN,
    } E_TWIN_UPDATE_OP;

    typedef struct{
        ISP_HW_MODULE   module;
        _isp_dma_enum_  channel;
        MUINT32         offsetAdr[ePlane_max];  // this is for patch header address
        MBOOL           bFrameEnd;
    }T_TWIN_DMA;
    typedef list<T_TWIN_DMA> L_T_TWIN_DMA; //this list is for multi-cam
            MINT32 get_RunTwinRst(MUINT32 dmachannel,vector<L_T_TWIN_DMA>& channel,MBOOL bDeque = MFALSE);
            MINT32 clr_RunTwinRst(MUINT32 dmachannel);

            MBOOL  HW_recover(MUINT32 step); //set 0:stop pipeline. set 1:restart pipeline
            MINT32 updateTwinInfo(MBOOL bEn, E_TWIN_UPDATE_OP op);    //update twin's info perframe

            MINT32 suspend( void ); //pause via CQ
            MINT32 resume( void );  //check slave cam which is really configured with setting of resume.Due to slave cam's page may be different at execution suspend() & resumse()

     static MBOOL  GetTwinRegAddr(ISP_HW_MODULE module,MUINT32* pAddr);

private:
    MINT32        update_TwinEn(MBOOL bEn);
    MINT32        update_beforeRunTwin(void);
    MVOID         update_before_twindrv_request(void); //this is for twin drv's SW request
    MINT32        update_afterRunTwin(void);
    MINT32        update_afterRunTwin_Dma(void);
    MUINT32       update_afterRunTwin_rlb(MUINT32 rlb_oft_prv,MUINT32 rrz_wd_prv,MUINT32 rrz_wd);
    MINT32        update_cq(void);
    MINT32        update_cq_suspend(void);//for suspend
    MBOOL         slaveNumCheck(MUINT32 &slave_num);
    ISP_HW_MODULE slaveModuleMap(MUINT32 slave_num, ISP_HW_MODULE MhwModule, ISP_HW_MODULE ShwModule);
public:
    MINT32        update_afterRunTwin_FH(MUINT32 slave_num, vector<ISP_DRV_CAM *> &v_slaveDrv);
    MINT32        DoneCollector(MBOOL En, E_INPUT InPut) override;

protected:
    virtual MINT32 _config( void );             //for initial only, work with _config() in CAM_TOP_CTRL, via CQ
    virtual MINT32 _enable( void* pParam  );    //via CQ
    virtual MINT32 _disable(void* pParam = NULL );  //write to phy reg directly
    virtual MINT32 _write2CQ( MUINT32 burstIndex );

public:
    //input info for ifunc_twin
    vector<ISP_DRV_CAM*>    m_pTwinIspDrv_v;
    MUINT32         m_subsample;

    //output info for twin drv
    MUINT32         m_rrz_out_wd;
    IspRect         m_rrz_roi;
    IspSize         m_rrz_in;

    MUINT32         m_rss_r2_out_wd;
    IspRect         m_rss_r2_roi;
    IspSize         m_rss_r2_in;

    MUINT32         m_crz_r1_out_wd;
    IspRect         m_crz_r1_roi;
    IspSize         m_crz_r1_in;

    MUINT32         m_crz_r2_out_wd;
    IspRect         m_crz_r2_roi;
    IspSize         m_crz_r2_in;

    MUINT32         m_af_vld_xstart;
    MUINT32         m_af_blk_xsize;
    MUINT32         m_af_x_win_num;

    MUINT32         m_lsc_lwidth;
    MUINT32         m_lsc_win_num_x;

    P1_TUNING_NOTIFY*       m_pTwinCB;
private:
    vector<L_T_TWIN_DMA>    m_dma_L[TWIN_DMAO_MAX]; //this list is for multi-frame,subsample is included

    MUINT32         m_rlb_oft;

    mutable Mutex   m_muList;   //protect lsit's push/pop
    MUINT32         m_ConfigDMAEnSt;
    MUINT32         m_ConfigDMA2EnSt;

    //for recover fbc cnt
    REG_FBC_R1_FBC_IMGO_R1_CTL2     fbc_IMGO_R1;
    REG_FBC_R1_FBC_RRZO_R1_CTL2     fbc_RRZO_R1;
    REG_FBC_R1_FBC_UFEO_R1_CTL2     fbc_UFEO_R1;
    REG_FBC_R1_FBC_UFGO_R1_CTL2     fbc_UFGO_R1;
    REG_FBC_R1_FBC_AFO_R1_CTL2      fbc_AFO_R1;
    REG_FBC_R1_FBC_YUVO_R1_CTL2     fbc_YUVO_R1;
    REG_FBC_R1_FBC_YUVBO_R1_CTL2    fbc_YUVBO_R1;
    REG_FBC_R1_FBC_YUVCO_R1_CTL2    fbc_YUVCO_R1;
    REG_FBC_R1_FBC_CRZO_R1_CTL2     fbc_CRZO_R1;
    REG_FBC_R1_FBC_CRZO_R2_CTL2     fbc_CRZO_R2;
    REG_FBC_R1_FBC_CRZBO_R1_CTL2    fbc_CRZBO_R1;
    REG_FBC_R1_FBC_CRZBO_R2_CTL2    fbc_CRZBO_R2;
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for CAM  when input data type is YUV
*/
class CAM_YUV_PIPE: public IspFunction_B
{
public:
    P1_TUNING_NOTIFY*   m_pP1Tuning;
public:
    CAM_YUV_PIPE()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_pP1Tuning = NULL;
    };

public:

    virtual MUINT32     id( void )                    {   return CAM_YUV;  }
    virtual const char* name_Str( void )              {   return "CAM_YUV";}

            MBOOL   setLMV(void);                       //because of LMV have the requirement of real time sync with rrz. so, need additional process
            MBOOL   setLCS(void);                       //because of LCS of sync requirement when LCS croping r needed.
            MBOOL   setGSE(void);                  //sgg2 have 2 user:eis/rss, so using CB method the same as EIS/LCS. avoid Racing condition
            MBOOL   setRSS(void){return MFALSE;}
            MUINT32 getCurPixMode(_isp_dma_enum_ dmao=_imgo_);

protected:
    virtual MINT32 _config( void );             //for initial only, work with _config() in CAM_TOP_CTRL
    virtual MINT32 _write2CQ( MUINT32 burstIndex );

    virtual MINT32 _enable( void* pParam  ) {(void)pParam;return 0;}
    virtual MINT32 _disable( void* pParam = NULL ) {(void)pParam;return 0;}

protected:
    ISP_DRV_CAM*    m_pDrv;

};

///////////////////////////////////////////////////////////////////////////////
/**
    class for magic number update
*/
class CAM_MAGIC_CTRL: public IspFunction_B
{
public:
    CAM_MAGIC_CTRL():
        m_nMagic(0x0fffffff)
    {
        m_pIspDrv = NULL;
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_hwModule = CAM_MAX;
    };

protected:
    virtual MINT32 _config( void );
    virtual MINT32 _write2CQ( MUINT32 burstIndex );

public:
    MUINT32         m_nMagic;
    IspDrvVir*      m_pIspDrv; // obj for new frame
    ISP_HW_MODULE   m_hwModule;

private:
    ISP_DRV_CAM*    m_pDrv;
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CAM tuning
*/
class CAM_TUNING_CTRL: public CAM_RAW_PIPE, public CAM_YUV_PIPE
{
public:
              CAM_TUNING_CTRL();
     virtual ~CAM_TUNING_CTRL();
public://_config defined as public member is to avoid abstract data type
    virtual MINT32  setNotify(E_SET_NOTIFY_OP op);
    virtual MINT32  _config( void );
    virtual MINT32  _enable( void* pParam );
    virtual MINT32  _disable( void* pParam = NULL );
            MINT32  _init(void);
            MINT32  _uninit(void);
            MINT32  update(MBOOL UpdateFLK,
                           MBOOL UpdatebyCB = MFALSE,
                           MINT32 CBmagic = 0);
            MINT32  update(void);
            MINT32  update_end(void);
            MBOOL   configNotify(void);
            MBOOL   configCB(MBOOL bForce);
            MBOOL   setAWB(void);
            MBOOL   setTuning(void);
            MBOOL   dumpCQReg(void);
            MBOOL   setSLK(void);
            MBOOL   setNotifyDone(void);
            void    updateTuningDrv(MUINT32 senDev);

protected:
            inline void CBTimeChk(char *str, MBOOL bStartT);
            void        dumpTuningRegBuf(void *vpTuningNodesPage,
                                         const char *moduleName,
                                         MUINT32 module_size,
                                         MUINT32 module_adr);
            MBOOL       setLTM(void);

            NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM
            tuningHwModuleMap(const ISP_HW_MODULE hwModule);
public:
            MUINT32* m_Magic;
            MUINT32  m_SenDev;
            MUINT32  m_BQNum;
            TUNINGCB_INPUT_INFO m_TuningCBinput;
            P1_TUNING_NOTIFY*   m_pP1Tuning;
            IspDrvVir*          m_pIspDrv;              // obj for new frame
            ISP_HW_MODULE       m_hwModule;
protected:
        enum _E_SLK_Update
        {
            SLK_R1_Update  = 0,
            SLK_R2_Update  = 1,
            SLK_Update_MAX = 2,
        };

        typedef struct _RRZ_PARAMETER{
            MBOOL   bRRZ_Bypass;
            MUINT32 src_x;
            MUINT32 src_y;
            MUINT32 src_w;
            MUINT32 src_h;
            MUINT32 crop_w;
            MUINT32 crop_h;
            MUINT32 tar_w;
            MUINT32 tar_h;
        } RRZ_PARAMETER;

        CQ_DUMP_INFO    m_dumpInput;//for RegDumpCB use
        MBOOL           SLKUpdateStatus[SLK_Update_MAX];

private:
    ISP_DRV_CAM*    m_pDrv;
    void*           m_pTuningDrv;
    unsigned long   m_UpdateFuncBit;
    MUINT32         m_NotifyFrame;
};

class STT_SEL_CTRL: public IspFunction_B //for stt set sel
{
public:
    typedef enum {
        E_FLK_SEL = 0,
        E_CRP_R1_SEL,
        E_STT_SEL_MAX,
    }E_STT_SEL;

    STT_SEL_CTRL():m_pDrv(NULL){};
    virtual ~STT_SEL_CTRL(){};

protected:
    virtual MINT32 _config( void );
    virtual MINT32 _write2CQ( MUINT32 burstIndex ){(void)burstIndex;return 0;}
    virtual MINT32 _enable( void* pParam  ) {(void)pParam;return 0;}
    virtual MINT32 _disable( void* pParam = NULL ) {(void)pParam;return 0;}

public:
    MUINT32 	  m_SttSel[E_STT_SEL_MAX];

private:
    ISP_DRV_CAM*    m_pDrv;
    const MUINT32 m_kSelMax[E_STT_SEL_MAX] = {5, 1}; //use for check sel max value
};

///////////////////////////////////////////////////////////////////////////////
/**
    Class for DMAI modules
*/
class DMAI_B:public DMA_B
{
public:
    DMAI_B();

public:
    virtual MBOOL BW_Meter(const Sig_Profile& prof,V_DMA_BW* pOut);
    virtual MINT32 setBaseAddr(void) {return -1;};

protected:
    virtual MINT32 _config( void );
    virtual MINT32 _enable( void* pParam );
    virtual MINT32 _disable( void* pParam = NULL );
    virtual MINT32 _write2CQ( MUINT32 burstIndex );
    virtual MBOOL   Init(void) {return MFALSE;};

protected:
    ISP_DRV_CAM*    m_pDrv;     //for casting type , coding convenience only
    MUINT32         m_updatecnt;//only workable in CQ0
};

///////////////////////////////////////////////////////////////////////////////
/**
    Class for DMAO modules
*/
class DMAO_B:public DMA_B
{
public:
    DMAO_B();

public:
    virtual MBOOL BW_Meter(const Sig_Profile& prof,V_DMA_BW* pOut);
    MUINT32 BusSizeCal(E_PIX& pixMode);
    MINT32 setBaseAddr(void);
    MBOOL Init(void); //for stt dmao

protected:
    virtual MINT32 _config( void ); //config all dma but baseaddress
    virtual MINT32 _enable( void* pParam );
    virtual MINT32 _disable( void* pParam = NULL );
    virtual MINT32 _write2CQ( MUINT32 burstIndex );

protected:
    ISP_DRV_CAM*    m_pDrv;     //for casting type , coding convenience only
public:
    MUINT32         Header_Addr;
    MUINT32         m_fps;
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for BPCI
*/
class DMA_BPCI:public DMAI_B
{
public:
    DMA_BPCI(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_BPCI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_BPCI";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for PDI
*/
class DMA_PDI:public DMAI_B
{
public:
    DMA_PDI(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_PDI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_PDI";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for LSCI
*/
class DMA_LSCI:public DMAI_B
{
public:
    DMA_LSCI(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_LSCI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_LSCI";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CACI
*/
class DMA_CACI:public DMAI_B
{
public:
    DMA_CACI(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CACI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CACI";}
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ0I
*/
class DMA_CQ0:public DMAI_B
{
public:
    DMA_CQ0(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_bSubsample = MFALSE;
        m_updatecnt = 0;
    };
    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
    typedef enum{
        _inner_,
        _outer_,
    }E_LoadMode;
    typedef enum{
        _cq_add_,
        _cq_delete_,
    }E_Write2CQ;
            MBOOL   setCQTriggerMode(E_MODE mode,void* ptr = NULL);
            MBOOL   TrigCQ(MBOOL bWait = MTRUE);
            MBOOL   setBaseAddr_byCQ(void);     //for cq link-list
            MBOOL   setSlaveAddr_byCQ(ISP_HW_MODULE slaveCam);    //master cam to program slave cam's baseaddress
            MBOOL   setDBLoadHold(MBOOL bHold, MBOOL bPhy);
            MBOOL   updateDB_byCQ(MBOOL byCQ);
            MBOOL   setLoadMode(E_LoadMode mode, MBOOL bPhysical);
            MBOOL   setDoneSubSample(ISP_DRV_CAM* pSrcObj,MBOOL bEn,MUINT32 nSub=0); //for high speed case
            MBOOL   disableAllDma(ISP_DRV_CAM* pSrcObj); //for DCIF STT work around
            MBOOL   SetCQupdateCnt(void);
            UINT32  GetCQupdateCnt(MBOOL bPhy);
            MINT32  IO2CQ(E_Write2CQ op, ISP_HW_MODULE slaveCam);

protected:
    virtual MINT32      _config( void ); //config all dma but baseaddress
    virtual MINT32      _write2CQ( MUINT32 burstIndex );

public:
    virtual MBOOL       Init(void);
    virtual MUINT32     id( void )                {    return     CAM_DMA_CQ0I;  }
    virtual const char* name_Str( void )          {    return     "CAM_DMA_CQ0I";}

#if CQ_SW_WORK_AROUND
            void    DummyDescriptor(void);
#endif

public:
            MBOOL   m_bSubsample;
            //for config
            static uintptr_t Addr[PHY_CAM];
            static ISP_DRV_CAM* pDrv[PHY_CAM];
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ1I
*/
class DMA_CQ1:public DMAI_B
{
public:
    DMA_CQ1(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_bSubsample = MFALSE;
    };
    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        //_continuouse_
    }E_MODE;
    typedef enum{
        _cq_add_,
        _cq_delete_,
    }E_Write2CQ;
            MBOOL   setCQTriggerMode(E_MODE mode,void* ptr = NULL);
            MBOOL   TrigCQ(MBOOL bWait = MTRUE);

            MBOOL   setCQContent(MUINTPTR arg1,MUINTPTR arg2);
            MBOOL   setBaseAddr_byCQ(void);     //for cq link-list
            MBOOL   setSlaveAddr_byCQ(ISP_HW_MODULE slaveCam);    //master cam to program slave cam's baseaddress
            MINT32  IO2CQ(E_Write2CQ op, ISP_HW_MODULE slaveCam);
public:
    virtual MUINT32     id( void )                {    return     CAM_DMA_CQ1I;  }
    virtual const char* name_Str( void )          {    return     "CAM_DMA_CQ1I";}

public:
            MBOOL   m_bSubsample;


};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ2I
*/
class DMA_CQ2:public DMAI_B
{
public:
    DMA_CQ2(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ2I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ2I";}
protected:
    virtual MINT32 _config( void ){return 1;}
    virtual MINT32 _enable( void* pParam ){(void)pParam;return 1;}
    virtual MINT32 _disable( void* pParam = NULL ){(void)pParam;return 1;}
    virtual MINT32 _write2CQ( MUINT32 burstIndex ){(void)burstIndex;return 1;}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ3I
*/
class DMA_CQ3:public DMAI_B
{
public:
    DMA_CQ3(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ3I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ3I";}
protected:
    virtual MINT32 _config( void ){return 1;}
    virtual MINT32 _enable( void* pParam ){(void)pParam;return 1;}
    virtual MINT32 _disable( void* pParam = NULL ){(void)pParam;return 1;}
    virtual MINT32 _write2CQ( MUINT32 burstIndex ){(void)burstIndex;return 1;}

};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ4I
*/
class DMA_CQ4:public DMAI_B
{
public:
    DMA_CQ4(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32     id( void )                {    return     CAM_DMA_CQ4I;  }
    virtual const char* name_Str( void )          {    return     "CAM_DMA_CQ4I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ5I
*/
class DMA_CQ5:public DMAI_B
{
public:
    DMA_CQ5(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ5I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ5I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ6I
*/
class DMA_CQ6:public DMAI_B
{
public:
    DMA_CQ6(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ6I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ6I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ7I
*/
class DMA_CQ7:public DMAI_B
{
public:
    DMA_CQ7(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ7I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ7I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ8I
*/
class DMA_CQ8:public DMAI_B
{
public:
    DMA_CQ8(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ8I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ8I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ9I
*/
class DMA_CQ9:public DMAI_B
{
public:
    DMA_CQ9(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ9I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ9I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ10I
    CQ10 is dedicated for CQ11, to loading CQ11 inner BA only under high-speed mode
    because of device tree, CPU can't program inner BA directly.

    but in D-Twin ON, CQ10 at slave cam will running in immediate mode, just like CQ1 at slave cam under D-Twin ON.
*/
class DMA_CQ10:public DMAI_B
{
public:
    DMA_CQ10(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _immediate_ = 1,
    }E_MODE;
    typedef enum{
        _cq_add_,
        _cq_delete_,
    }E_Write2CQ;
            MBOOL   setCQTriggerMode(E_MODE mode,void* ptr = NULL);
            MBOOL   setCQContent(MUINTPTR arg1,MUINTPTR arg2);
            MBOOL   TrigCQ(MBOOL bWait = MTRUE);
            MBOOL   setSlaveAddr_byCQ(ISP_HW_MODULE slaveCam);    //master cam to program slave cam's baseaddress
            MINT32  IO2CQ(E_Write2CQ op, ISP_HW_MODULE slaveCam);
public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ10I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ10I";}

protected:
    virtual MINT32  _write2CQ( MUINT32 burstIndex );
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ12I
*/
class DMA_CQ12:public DMAI_B
{
public:
    DMA_CQ12(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    };

    typedef enum{
        _p1_done_once_ = 0,
        _immediate_,
        _continuouse_
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

public:
    virtual MUINT32     id( void )                {    return     CAM_DMA_CQ12I;  }
    virtual const char* name_Str( void )          {    return     "CAM_DMA_CQ12I";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CQ28I
    CQ28 support only event_trig.  this is hw constraint.
*/
class DMA_CQ28:public DMAI_B
{
public:
    DMA_CQ28(){
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_prvSOF = 0xFFFF;
    };
    typedef enum{
        _event_trig = 0,
    }E_MODE;
            MBOOL   setCQTriggerMode(E_MODE mode);
            MBOOL   TrigCQ(void);

            MBOOL   setCQContent(MUINTPTR arg1,MUINTPTR arg2);

protected:
    virtual MINT32 _write2CQ( MUINT32 burstIndex ){(void)burstIndex;return 1;};
    virtual MINT32 _disable( void* pParam = NULL );

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CQ28I;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CQ28I";}

private:
            MUINT32 m_prvSOF;
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for IMGO
*/
class DMA_IMGO:public DMAO_B
{
public:
    DMA_IMGO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_IMGO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_IMGO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for RRZO
*/
class DMA_RRZO:public DMAO_B
{
public:
    DMA_RRZO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_RRZO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_RRZO";}
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for LCSO
*/
class DMA_LCSO:public DMAO_B
{
public:
    DMA_LCSO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_LCSO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_LCSO";}
};



///////////////////////////////////////////////////////////////////////////////
/**
    class for AAO
*/
class DMA_AAO:public DMAO_B
{
public:
    DMA_AAO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_AAO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_AAO";}
};



///////////////////////////////////////////////////////////////////////////////
/**
    class for AFO
*/
class DMA_AFO:public DMAO_B
{
public:
    DMA_AFO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_AFO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_AFO";}
};
///////////////////////////////////////////////////////////////////////////////
/**
    class for UFEO
*/
class DMA_UFEO:public DMAO_B
{
public:
    DMA_UFEO(){m_img_size = MSize(0, 0);};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_UFEO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_UFEO";}
    MVOID check(MBOOL& bUF_OFF);
    MVOID update_AU(void);

public:
    MSize m_img_size;
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for UFGO
*/
class DMA_UFGO:public DMAO_B
{
public:
    DMA_UFGO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_UFGO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_UFGO";}
    MVOID check(MBOOL& bUF_OFF);
    MVOID update_AU(void);
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for PDO
*/
class DMA_PDO:public DMAO_B
{
public:
    DMA_PDO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_PDO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_PDO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for TSFSO
*/
class DMA_TSFSO:public DMAO_B
{
public:
    DMA_TSFSO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_TSFSO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_TSFSO";}
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for EISO
*/
class DMA_EISO:public DMAO_B
{
public:
    DMA_EISO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_LMVO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_LMVO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for FLKO
*/
class DMA_FLKO:public DMAO_B
{
public:
    DMA_FLKO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_FLKO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_FLKO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for YUVO
*/
class DMA_YUVO:public DMAO_B
{
public:
    DMA_YUVO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_YUVO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_YUVO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for YUVO
*/
class DMA_YUVBO:public DMAO_B
{
public:
    DMA_YUVBO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_YUVBO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_YUVBO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for YUVCO
*/
class DMA_YUVCO:public DMAO_B
{
public:
    DMA_YUVCO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_YUVCO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_YUVCO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CRZO_R1
*/
class DMA_CRZO_R1:public DMAO_B
{
public:
    DMA_CRZO_R1(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CRZO_R1;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CRZO_R1";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CRZBO_R1
*/
class DMA_CRZBO_R1:public DMAO_B
{
public:
    DMA_CRZBO_R1(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CRZBO_R1;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CRZBO_R1";}
};


///////////////////////////////////////////////////////////////////////////////
/**
    class for CRZO_R2
*/
class DMA_CRZO_R2:public DMAO_B
{
public:
    DMA_CRZO_R2(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CRZO_R2;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CRZO_R2";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for CRZBO_R2
*/
class DMA_CRZBO_R2:public DMAO_B
{
public:
    DMA_CRZBO_R2(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_CRZBO_R2;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_CRZBO_R2";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for RSSO
*/
class DMA_RSSO:public DMAO_B
{
public:
    DMA_RSSO(){};

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_RSSO;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_RSSO";}
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for RAWI
*/
class DMA_RAWI:public DMAI_B
{
public:
    DMA_RAWI(){};

protected:
    virtual MINT32 _config( void );

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_RAWI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_RAWI";}
    virtual MINT32 setBaseAddr(void);
};

///////////////////////////////////////////////////////////////////////////////
/**
    class for UFDI
*/
class DMA_UFDI:public DMAI_B
{
public:
    DMA_UFDI(){};

protected:
    virtual MINT32 _config( void );

public:
    virtual MUINT32 id( void )                {    return     CAM_DMA_UFDI;  }
    virtual const char*   name_Str( void )          {    return     "CAM_DMA_UFDI";}
    virtual MINT32 setBaseAddr(void);
};

///////////////////////////////////////////////////////////////////////////////
typedef enum _E_PIPECHK_HW_PATH{
    E_PPC_UNKNOWN   = 0,
    E_PPC_DL,
    E_PPC_DC,
    E_PPC_RAWI, //bit true,fake stagger mode
    E_PPC_MAX,
}E_PIPECHK_HW_PATH;

class PIPE_CHECK
{
    public:
                PIPE_CHECK(void)
                    :m_pDrv(NULL)
                    ,m_hwModule(CAM_MAX){memset(&m_err_status,0,sizeof(m_err_status));}
                PIPE_CHECK(ISP_DRV_CAM* obj)
                    :m_pDrv(obj)
                    ,m_hwModule(CAM_MAX){memset(&m_err_status,0,sizeof(m_err_status));}
        virtual ~PIPE_CHECK(void){}

                MBOOL   Check_Start(void);
                MBOOL   Register_HW_PATH(E_PIPECHK_HW_PATH path);
    protected:
                MBOOL   MAIN_CHECK(ISP_DRV_CAM* masterobj = NULL){return this->_MAIN_CHECK(masterobj);}

                MBOOL   Front(MVOID* ptr = NULL){return this->_Front(ptr);}
                MBOOL   Middle(void){return this->_Middle();}
                MBOOL   End(void){return this->_End();}

    protected:
        virtual MBOOL   _Front(MVOID* ptr = NULL);          //before sep , branchs r inclueded.
                MBOOL   _Middle(void);                      //before rrz, branchs r not inclueded, rrz is not inclueded.
                MBOOL   _End(void);                        //after rrz, branchs r inclueded.

                MBOOL   RdyReq_Dump(void);
                MBOOL   DMAO_STATUS(void);
                MBOOL   UFE_CHECK(void);
                MBOOL   UFEG_CHECK(void);
                MBOOL   AA_CHECK(void);
                MBOOL   FLK_CHECK(void);
                MBOOL   LCS_CHECK(void);
                MBOOL   LMV_CHECK(void);
                MBOOL   RSS_CHECK(void);
                MBOOL   LSC_CHECK(void);
                MBOOL   AF_CHECK(void);
                MBOOL   PDO_CHECK(void);
                MBOOL   BPC_CHECK(void);
                MBOOL   LTM_CHECK(void);
                MBOOL   LTMS_CHECK(void);
                MBOOL   TG_CHECK(void);

                MBOOL   ENQUE_CHECK(void);
        virtual MBOOL   _MAIN_CHECK(ISP_DRV_CAM* masterobj);//top ctrl check, path ctrl r not incldued.
                MBOOL   CrossPath_check(void);               //check control which have interactive-effect between cams
                MBOOL   RRZ_CHECK(void);
                MBOOL   ME_CHECK(void);                     //mutully exclusive check
                MBOOL   Pattern_CHECK(void);                //check pattern
                MBOOL   PD_TABLE_CHECK(MUINTPTR va,MUINT32 tbl_stride);
                MBOOL   SEPMRG_CHECK(void);
                MBOOL   OBC_CHECK(void);
                MBOOL   TSF_CHECK(void);

                MBOOL   InPutSize_SEP(MUINT32& in_w,MUINT32& in_h);
                MBOOL   InPutSize_TG(MUINT32& in_w,MUINT32& in_h,MBOOL pure = MFALSE);

        virtual E_PIPECHK_HW_PATH   HW_Path(void){return E_PPC_DL;}

    public:
        ISP_DRV_CAM*    m_pDrv;
        static MUINTPTR m_BPCI_VA;
        static MUINTPTR m_PDI_VA;
    protected:
        static Mutex    m_lock[PHY_CAM];
        ISP_HW_MODULE   m_hwModule;
        struct ISP_RAW_INT_STATUS   m_err_status;
        static E_PIPECHK_HW_PATH    m_HWPath[PHY_CAM];
};

class TWIN_PIPECHK  : public PIPE_CHECK
{
    public:
                TWIN_PIPECHK(void);
        virtual ~TWIN_PIPECHK(void){}
                TWIN_PIPECHK(ISP_DRV_CAM* obj);
    protected:
        virtual MBOOL   _MAIN_CHECK(ISP_DRV_CAM* masterobj);    //top ctrl check, path ctrl r not incldued.
        virtual MBOOL   _Front(MVOID* ptr = NULL){return this->_Front(*(ISP_HW_MODULE*)ptr);}
    private:
                MBOOL   _Front(ISP_HW_MODULE master);           //before sep , branchs r inclueded.
};

class DYUV_PIPECHK : public PIPE_CHECK
{
    protected:
        typedef enum{
            E_FAIL = 0,
            E_PASS,
            E_BYPASS
        }E_YUV_STATE;
    public:
                DYUV_PIPECHK(void){};
        virtual ~DYUV_PIPECHK(void){}
                DYUV_PIPECHK(ISP_DRV_CAM* obj);
    public:
                MBOOL       CHECK_START(void);  //slave cam under twin mode is not supported. checked by twin drv
    protected:
        virtual const char* DMA_ID(void){return "DYUV";}
                MBOOL       InPutSize(E_YUV_STATE yuvo);
                MBOOL       CCM_CHECK(void);
        virtual MBOOL       GGM_CHECK(void);
        virtual MBOOL       G2C_CHECK(void);
        virtual MBOOL       CRZ_CHECK(void){return MTRUE;}
                MBOOL       DM_CHECK(void);
        virtual MBOOL       CRP_CHECK(void){return MTRUE;}
        virtual MBOOL       CRSP_CHECK(void){return MTRUE;}
        virtual MBOOL       C42_CHECK(void);
        virtual MBOOL       YNRS_CHECK(void);
        virtual MBOOL       DMAO_CHECK(void){return MFALSE;}
                MBOOL       SLK_CHECK(MUINT32 idx);         // idx:1 for R1, idx:2 for R2
        virtual MBOOL       BS_CHECK(void){return MTRUE;}

    protected:
                MSize       m_InPut;
};

class YUVO_PIPECHK : public DYUV_PIPECHK
{
    public:
                YUVO_PIPECHK(void){};
        virtual ~YUVO_PIPECHK(void){}
                YUVO_PIPECHK(ISP_DRV_CAM* obj);
        friend class DYUV_PIPECHK;
    protected:
        virtual const char* DMA_ID(void){return "YUVO";}
        virtual MBOOL       CRP_CHECK(void);
        virtual MBOOL       CRSP_CHECK(void);
                E_YUV_STATE ENABLE_CHECK(void);
        virtual MBOOL       DMAO_CHECK(void);
};

class CRZO_R2_PIPECHK : public DYUV_PIPECHK
{
    public:
                CRZO_R2_PIPECHK(void){};
        virtual ~CRZO_R2_PIPECHK(void){}
                CRZO_R2_PIPECHK(ISP_DRV_CAM* obj);
        friend class DYUV_PIPECHK;
    protected:
        virtual const char* DMA_ID(void){return "CRZO_R2";}
        virtual MBOOL       C42_CHECK(void);
        virtual MBOOL       CRZ_CHECK(void);
        virtual MBOOL       CRSP_CHECK(void);
                E_YUV_STATE ENABLE_CHECK(void);
        virtual MBOOL       DMAO_CHECK(void);
        virtual MBOOL       BS_CHECK(void);

};

class CRZO_R1_PIPECHK : public DYUV_PIPECHK
{
    public:
                CRZO_R1_PIPECHK(void){};
        virtual ~CRZO_R1_PIPECHK(void){}
                CRZO_R1_PIPECHK(ISP_DRV_CAM* obj);
        friend class DYUV_PIPECHK;
    protected:
        virtual const char* DMA_ID(void){return "CRZO_R1";}
        virtual MBOOL       CRZ_CHECK(void);
        virtual MBOOL       YNRS_CHECK(void){return MTRUE;}
                E_YUV_STATE ENABLE_CHECK(void);
        virtual MBOOL       DMAO_CHECK(void);
        virtual MBOOL       GGM_CHECK(void);
        virtual MBOOL       G2C_CHECK(void);
        virtual MBOOL       BS_CHECK(void);
};

class RAWI_PIPCHK : public PIPE_CHECK
{
    public:
                RAWI_PIPCHK(void);
        virtual ~RAWI_PIPCHK(void){}
                RAWI_PIPCHK(ISP_DRV_CAM* obj);
    protected:
        virtual MBOOL   _Front(MVOID* ptr = NULL);          //before sep , branchs r inclueded.
        virtual E_PIPECHK_HW_PATH   HW_Path(void){return E_PPC_RAWI;}

                MBOOL   RAWI_CHECK(void);
                MBOOL   UFDI_CHECK(void);
};

class RAWI_TWIN_PIPCHK : public TWIN_PIPECHK
{
    public:
                RAWI_TWIN_PIPCHK(void);
        virtual ~RAWI_TWIN_PIPCHK(void){}
                RAWI_TWIN_PIPCHK(ISP_DRV_CAM* obj);
    protected:
        virtual E_PIPECHK_HW_PATH   HW_Path(void){return E_PPC_RAWI;}
};


class DC_PIPECHK : public RAWI_PIPCHK
{
    public:
                DC_PIPECHK(void);
        virtual ~DC_PIPECHK(void){}
                DC_PIPECHK(ISP_DRV_CAM* obj);
    protected:
        virtual MBOOL   _Front(MVOID* ptr = NULL);          //before sep , branchs r inclueded.
        virtual E_PIPECHK_HW_PATH   HW_Path(void){return E_PPC_DC;}
};

class DC_TWIN_PIPECHK : public TWIN_PIPECHK
{
    public:
                DC_TWIN_PIPECHK(void);
        virtual ~DC_TWIN_PIPECHK(void){}
                DC_TWIN_PIPECHK(ISP_DRV_CAM* obj);
    protected:
        virtual MBOOL   _Front(MVOID* ptr = NULL){return this->_Front((ISP_DRV_CAM*)ptr);}
        virtual E_PIPECHK_HW_PATH   HW_Path(void){return E_PPC_DC;}

                MBOOL   _Front(ISP_DRV_CAM* masterobj);      //before sep , branchs r inclueded.
};

///////////////////////////////////////////////////////////////////////////////
typedef enum{
    eCmd_Fail = 0,          //fail
    eCmd_Pass = 1,          //ok
    eCmd_Stop_Pass,         //this state is for stopped already
    eCmd_Suspending_Pass,   //this state is for starting suspending
    eCmd_Dummy,             //this state is for dummy
}E_BC_STATUS;

/**
    class for CAM DAMO ctrl, support only deque/enque 1 image at 1 time.
*/
class CAM_BUF_CTRL : public IspFunction_B
{
public:
    CAM_BUF_CTRL(void);
    ~CAM_BUF_CTRL(void){};

    virtual MUINT32 id( void )                    {   return 0xffffffff;  }
    virtual const char*   name_Str( void )              {   return "CAM_BUFFER_CTRL";}

    /**
        check if any ready image on dram.
    */
    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);
    /**
        push empty buffer into hw
    */
    virtual E_BC_STATUS enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode = 0);

    /**
        retrieve available buffer on dram
    */
    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

    mutable Mutex   m_bufctrl_lock;//protect deque/enque behavior

private:

    #define MAX_DEPTH (64)
            MBOOL       reg_dump(const char* pStr);
protected:
        QueueMgr<ST_BUF_INFO>   m_Queue;
        QueueMgr<MUINTPTR>  m_Queue_deque_ptr;

protected:
    virtual MINT32  _config( void );                // this is "scenario-level" function.
    virtual MINT32  _enable( void* pParam  ) ;      // this is "frame-leve" fucntion, but statistic-ports r "partial scneraio-level"
    virtual MINT32  _disable( void* pParam = NULL ) ;// this is "frame-leve" fucntion, but statistic-ports r "partial scneraio-level"
    virtual MINT32  _write2CQ( MUINT32 burstIndex ) ;

            MBOOL   PipeCheck(void);

    virtual void    FBC_STATUS(IspDrv* ptr){(void)ptr;}
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr,ISP_HW_MODULE masterModule)
                                                        {(void)fh_va;(void)buf_info;(void)header_data;(void)pStr;masterModule;}
            MUINT32 estimateTimeout(MUINT32 subSample);

            MBOOL   ion_handle_lock(ISP_DRV_CAM* obj,MBOOL block,MUINT32 img_id,MUINT32 hd_id);

            #define buf_data_parsing(buf_info,plane,_buf) {\
                buf_info.memID[plane]          = _buf.image.mem_info.memID;\
                buf_info.u4BufSize[plane]      = _buf.image.mem_info.size;\
                buf_info.u4BufVA[plane]        = _buf.image.mem_info.va_addr;\
                buf_info.u4BufPA[plane]        = _buf.image.mem_info.pa_addr;\
                buf_info.bufCohe[plane]        = _buf.image.mem_info.bufCohe;\
                buf_info.bufSecu[plane]        = _buf.image.mem_info.bufSecu;\
                buf_info.Frame_Header.u4BufPA[plane]   = _buf.header.pa_addr;\
                buf_info.Frame_Header.u4BufVA[plane]   = _buf.header.va_addr;\
                buf_info.Frame_Header.u4BufSize[plane] = _buf.header.size;\
                buf_info.Frame_Header.memID[plane]     = _buf.header.memID;\
                buf_info.Frame_Header.bufCohe[plane]   = _buf.header.bufCohe;\
                buf_info.Frame_Header.bufSecu[plane]   = _buf.header.bufSecu;\
            }
public:
    static  MBOOL   updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset, ISP_HW_MODULE camModule);

    typedef enum {
        eCamState_runnable = 0,     //resume flow is ready(no wait)
        eCamState_stop,             //already stopped
        eCamState_suspending,       //starting suspend flow
        eCamState_suspend,          //already suspened, for dynamic twin.
        eState_forPDOnly,           //for pd only when switch raw_sel is crossed
        eCamState_max
    } E_CAM_STATE;
    typedef enum {
        eCamDmaType_main = 0,
        eCamDmaType_stt,
        eCamDmaType_max
    } E_CAM_DMA_TYPE;
    //virtual cam is not supported!(due to only phy cam is supported at waitbufready())
    typedef enum{
        eSus_HWOFF  = 0x01,
        eSus_HW_SW_STATE  = 0x02,
        eSus_SIGNAL = 0x04,
        eSus_ALL    = 0x07,
    }E_SUSPEND_OP;
    typedef enum{
        eRes_HW_STATE = 0x01,
        eRes_HWON     = 0x02,
        eRes_SW_STATE = 0x04,
        eRes_ALL      = 0x07,
    }E_RESUME_OP;
    static  MBOOL   suspend(ISP_HW_MODULE camModule, ISP_DRV_CAM *pDrvCam, ISP_HW_MODULE srcTgModule, ISP_DRV_CAM *pTgDrvCam, E_SUSPEND_OP op = eSus_ALL);
    static  MBOOL   resume(ISP_HW_MODULE camModule, ISP_DRV_CAM *pDrvCam, ISP_DRV_CAM *pTgDrvCam, E_RESUME_OP op = eRes_ALL);
    static  MBOOL   updateState(E_CAM_STATE camState, ISP_HW_MODULE camModule, E_CAM_DMA_TYPE dmaType);

public:
    static MUINT32          m_fps[CAM_MAX]; //fps here is sw operation frequency, m_fps = sensor fps / signal subsample.
    static MUINT32          m_recentFrmTimeMs[CAM_MAX][MAX_RECENT_GRPFRM_TIME]; //upadted through normalpipe enque, for dynamic change deque timeout
    static E_CAM_STATE      m_CamState[CAM_MAX];
    static E_CAM_STATE      m_SttState[CAM_MAX];

    CAM_TIMESTAMP*          m_pTimeStamp;


protected:
    ISP_DRV_CAM*    m_pDrv;

    MUINT32         m_buf_cnt;
};

class BUF_CTRL_IMGO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_IMGO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_IMGO";}
    virtual MUINT32     id(void)            {   return _imgo_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);


};

class BUF_CTRL_RRZO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_RRZO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_RRZO";}
    virtual MUINT32     id(void)            {   return _rrzo_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);


};

class BUF_CTRL_UFEO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_UFEO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_UFEO";}
    virtual MUINT32     id(void)            {   return _ufeo_;  }

    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_UFGO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_UFGO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_UFGO";}
    virtual MUINT32     id(void)            {   return _ufgo_;  }

    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_LCSO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_LCSO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_LCSO";}
    virtual MUINT32     id(void)            {   return _lcso_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_LMVO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_LMVO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_LMVO";}
    virtual MUINT32     id(void)            {   return _lmvo_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_RSSO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_RSSO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_RSSO";}
    virtual MUINT32     id(void)            {   return _rsso_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};


class BUF_CTRL_AAO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_AAO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_AAO";}
    virtual MUINT32     id(void)            {   return _aao_;  }

    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);

};

class BUF_CTRL_AFO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_AFO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_AFO";}
    virtual MUINT32     id(void)            {   return _afo_;  }

    using CAM_BUF_CTRL::waitBufReady;
    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
private:

};

class BUF_CTRL_FLKO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_FLKO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_FLKO";}
    virtual MUINT32     id(void)            {   return _flko_;  }

    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);

};

class BUF_CTRL_PDO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_PDO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
        m_enqCnt_pdo_suspending = 0;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_PDO";}
    virtual MUINT32     id(void)            {   return _pdo_;  }

    /**
        override due to raw_sel switched with different sensor type
        @waitBufReady
        @enqueueHwBuf
        @dequeueHwBuf
    */
    virtual E_BC_STATUS waitBufReady( CAM_STATE_NOTIFY *pNotify = NULL);
    virtual E_BC_STATUS enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode = 0);
    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual MINT32  _config( void );                // this is "scenario-level" function.

    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
private:
            MUINT32 m_enqCnt_pdo_suspending; //enqeu record after pdo suspending. flush after resume
};

class BUF_CTRL_TSFSO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_TSFSO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_TSFSO";}
    virtual MUINT32     id(void)            {   return _tsfso_;  }

    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL);

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);

};

class BUF_CTRL_YUVO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_YUVO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_YUVO";}
    virtual MUINT32     id(void)            {   return _yuvo_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_YUVBO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_YUVBO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_YUVBO";}
    virtual MUINT32     id(void)            {   return _yuvbo_;  }

    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_YUVCO: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_YUVCO()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_YUVCO";}
    virtual MUINT32     id(void)            {   return _yuvco_;  }

    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_CRZO_R1: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_CRZO_R1()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_CRZO_R1";}
    virtual MUINT32     id(void)            {   return _crzo_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_CRZBO_R1: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_CRZBO_R1()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_CRZBO_R1";}
    virtual MUINT32     id(void)            {   return _crzbo_;  }

    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};


class BUF_CTRL_CRZO_R2: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_CRZO_R2()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_CRZO_R2";}
    virtual MUINT32     id(void)            {   return _crzo_r2_;  }

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

class BUF_CTRL_CRZBO_R2: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_CRZBO_R2()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str(void)  {   return "BUF_CTRL_CRZBO_R2";}
    virtual MUINT32     id(void)            {   return _crzbo_r2_;  }

    virtual E_BC_STATUS dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual void    FBC_STATUS(IspDrv* ptr);
    virtual void    Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule);
};

/**
    buf_ctrl_rawi: fake frame buffer control, used in m2m related case(fake stagger mode/rawi)
*/
class BUF_CTRL_RAWI: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_RAWI()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str( void ) {return "BUF_CTRL_RAWI";}
    virtual MUINT32     id( void ) {return _rawi_;}
    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL){return eCmd_Pass;}
    virtual E_BC_STATUS enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode = 0);
    virtual E_BC_STATUS dequeueHwBuf( NSImageio::NSIspio::BufInfo& buf_info );


protected:
    virtual MINT32  _config( void );
    virtual MINT32  _enable( void* pParam ){(void)pParam;return 0;}
    virtual MINT32  _disable( void* pParam = NULL ){(void)pParam;return 0;}
    virtual MINT32  _write2CQ( MUINT32 burstIndex ){(void)burstIndex;return 0;}

};

/**
    buf_ctrl_ufdi: fake frame buffer control, used in m2m related case(fake stagger mode/rawi)
*/
class BUF_CTRL_UFDI: public CAM_BUF_CTRL
{
public:
    BUF_CTRL_UFDI()
    {
        m_pDrv = (ISP_DRV_CAM*)m_pIspDrv;
    }
    virtual const char* name_Str( void ) {return "BUF_CTRL_UFDI";}
    virtual MUINT32     id( void ) {return _ufdi_r2;}
    virtual E_BC_STATUS waitBufReady(CAM_STATE_NOTIFY *pNotify = NULL){return eCmd_Pass;}
    virtual E_BC_STATUS enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode = 0);
    virtual E_BC_STATUS dequeueHwBuf( NSImageio::NSIspio::BufInfo& buf_info );

protected:
    virtual MINT32  _config( void );
    virtual MINT32  _enable( void* pParam ){(void)pParam;return 0;}
    virtual MINT32  _disable( void* pParam = NULL ){(void)pParam;return 0;}
    virtual MINT32  _write2CQ( MUINT32 burstIndex ){(void)burstIndex;return 0;}

};


#endif
