#ifndef _SEC_MGR_H_
#define _SEC_MGR_H_

#include <vector>
#include <isp_drv.h>
#include "isp_datatypes.h"
#include <ispio_pipe_ports.h>
#include <semaphore.h>

using namespace NSCam::NSIoPipe;

#define DAPC_NUM_CQ 18

typedef struct{
    MUINT32 Num_of_Cam;
    MUINT32 CAM_CTL_EN_CTL;
    MUINT32 CAM_CTL_EN;
    MUINT32 CAM_CTL_EN2;
    MUINT32 CAM_CTL_EN3;
    MUINT32 CAM_CTL_EN4;
    MUINT32 CAM_CTL_DMA_EN;
    MUINT32 CAM_CTL_DMA2_EN;
    MUINT32 CAM_CTL_SEL;
    MUINT32 CAM_CTL_SEL2;
    MUINT32 AA_AWB_W_HSIZE;               //AA_R1_AA_AWB_WIN_SIZE
    MUINT32 AA_AWB_W_VSIZE;
    MUINT32 LCES_OUT_HT;                  //LCES_R1_LCES_OUT_SIZE
    MUINT32 LCES_OUT_WD;
    MUINT32 FLK_SIZE_X;                   //FLK_R1_FLK_SIZE
    MUINT32 FLK_SIZE_Y;
    MUINT32 LTMS_IN_HSIZE;                //LTMS_R1_LTMS_IN_SIZE
    MUINT32 LTMS_IN_VSIZE;
    MUINT32 LTMS_OUT_HSIZE;               //LTMS_R1_LTMS_OUT_SIZE
    MUINT32 LTMS_OUT_VSIZE;
    MUINT32 LTMS_ACT_WIN_X_START;         //LTMS_R1_LTMS_ACT_WINDOW_X
    MUINT32 LTMS_ACT_WIN_X_END;
    MUINT32 LTMS_ACT_WIN_Y_START;         //LTMS_R1_LTMS_ACT_WINDOW_Y    
    MUINT32 LTMS_ACT_WIN_Y_END;
    MUINT32 TSFS_W_VSIZE;                 //TSFS_R1_TSFS_SIZE
    MUINT32 TSFS_W_HSIZE;
    MUINT32 AF_PROT_BLK_XSIZE;            //AF_R1_AF_BLK_PROT
    MUINT32 AF_PROT_BLK_YSIZE;
    MUINT32 SecReg_ADDR_CQ[DAPC_NUM_CQ];     //record all 2nd DAPC register address
    MUINT32 SecOn;
}SecMgr_QueryInfo;

typedef struct{
    MUINT32 dapc_cq[DAPC_NUM_CQ];
}SecMgr_RegInfo;

enum SECMGRCmd {
    SECMGRCmd_QUERY_CTL_EN_CONSTRAINT              = 0x0001,
    SECMGRCmd_QUERY_CTL_EN2_CONSTRAINT             = 0x0002,
    SECMGRCmd_QUERY_CTL_EN3_CONSTRAINT             = 0x0003,
    SECMGRCmd_QUERY_CTL_EN4_CONSTRAINT             = 0x0004,
    SECMGRCmd_QUERY_DMA_EN_CONSTRAINT              = 0x0005,
    SECMGRCmd_QUERY_DMA2_EN_CONSTRAINT             = 0x0006,
    SECMGRCmd_QUERY_CTL_SEL_CONSTRAINT             = 0x0007,
    SECMGRCmd_QUERY_CTL_SEL2_CONSTRAINT            = 0x0008,
    SECMGRCmd_QUERY_AWB_WIN_SIZE_CONSTRAINT        = 0x0009,
    SECMGRCmd_QUERY_LCES_OUT_SIZE_CONSTRAINT       = 0x000A,
    SECMGRCmd_QUERY_FLK_SIZE_CONSTRAINT            = 0x000B,
    SECMGRCmd_QUERY_LTMS_IN_SIZE_CONSTRAINT        = 0x000C,
    SECMGRCmd_QUERY_LTMS_OUT_SIZE_CONSTRAINT       = 0x000D,
    SECMGRCmd_QUERY_LTMS_ACT_WINDOW_X_CONSTRAINT   = 0x000E,
    SECMGRCmd_QUERY_LTMS_ACT_WINDOW_Y_CONSTRAINT   = 0x000F,
    SECMGRCmd_QUERY_TSFS_SIZE_CONSTRAINT           = 0x0010,
    SECMGRCmd_QUERY_AF_BLK_PROT_CONSTRAINT         = 0x0011,
    SECMGRCmd_QUERY_NUM_CAM_CONSTRAINT             = 0x0012,
    SECMGRCmd_QUERY_CAP_CONSTRAINT                 = 0x0013,
    SECMGRCmd_MAX
};

typedef enum{
    SECMEM_CQ_DESCRIPTOR_TABLE = 0,
    SECMEM_VIRTUAL_REG_TABLE,
    SECMEM_FRAME_HEADER,
    SECMEM_LSC,
    SECMEM_BPC,
    SECMEM_TWIN_FRAME_HEADER,
    SECMEM_MAX
}SEC_MEM_TYPE;

typedef enum {
    CAM_PORT_IMGO_C = 0,
    CAM_PORT_RRZO_C,
    CAM_PORT_LSCI_C,
    CAM_PORT_BPCI_C,
    CAM_PORT_YUVO_C,
    CAM_PORT_UFDI_R2_C,
    CAM_PORT_RAWI_R2_C,
    CAM_PORT_RAWI_R5_C,
    CAM_PORT_CAMSV_1,
    CAM_PORT_CAMSV_2,
    CAM_PORT_CAMSV_3,
    CAM_PORT_CAMSV_4,
    CAM_PORT_CAMSV_5,
    CAM_PORT_CAMSV_6,
    CAM_PORT_AAO_C,
    CAM_PORT_AFO_C,
    CAM_PORT_FLKO_C,
    CAM_PORT_LCESO_C,
    CAM_PORT_CRZO_C,
    CAM_PORT_LTMSO_C,
    CAM_PORT_RSSO_C,
    CAM_PORT_CCUI,
    CAM_PORT_CCUO,
    CAM_PORT_FAKE,
    CAM_PORT_IMGO_A,
    CAM_PORT_RRZO_A,
    CAM_PORT_LSCI_A,
    CAM_PORT_BPCI_A,
    CAM_PORT_YUVO_A,
    CAM_PORT_UFDI_A,
    CAM_PORT_RAWI_R2_A,
    CAM_PORT_RAWI_R5_A,
    CAM_PORT_IMGO_B,
    CAM_PORT_RRZO_B,
    CAM_PORT_LSCI_B,
    CAM_PORT_BPCI_B,
    CAM_PORT_YUVO_B,
    CAM_PORT_UFDI_B,
    CAM_PORT_RAWI_R2_B,
    CAM_PORT_RAWI_R5_B,
    CAM_PORT_CAMSV_0,
    CAM_PORT_AAO_A,
    CAM_PORT_AFO_A,
    CAM_PORT_FLKO_A,
    CAM_PORT_LCESO_A,
    CAM_PORT_CRZO_A,
    CAM_PORT_AAO_B,
    CAM_PORT_AFO_B,
    CAM_PORT_FLKO_B,
    CAM_PORT_LCESO_B,
    CAM_PORT_CRZO_B,
    CAM_PORT_LTMSO_A,
    CAM_PORT_RSSO_A,
    CAM_PORT_LTMSO_B,
    CAM_PORT_RSSO_B,
    CAM_PORT_MAX
}SEC_DMA_PORT;

typedef enum {
    SECTABLE_CQ_DESCRIPTOR = 0,
    SECTABLE_VIRTUAL_REG,
    SECTABLE_FRAME_HEADER,
    SECTABLE_LSC,
    SECTABLE_BPC,
    SECTABLE_MAX
}SEC_TABLE_TYPE;

typedef enum{
    SEC_A = 0,
    SEC_B,
    MAX_SEC_HW_MODULE
}SEC_HW_MODULE;

typedef enum {
  TEE_PROTECT = 0,
  TEE_SECURE,
  TEE_MAX
} TEE_MEM_TYPE;

typedef struct{
    SEC_MEM_TYPE type;
    MUINT32 buff_size;
    uint64_t buff_va;
    MUINT32 buff_sec_mva;
    MINT32  memID;
}SecMgr_SubSecInfo;

typedef struct{
MUINT32 LTM_R1_LTM_TILE_SIZE;
MUINT32 LTM_R1_LTM_TILE_EDGE;
MUINT32 LTM_R1_LTM_TILE_CROP;
MUINT32 LTMS_R1_LTMS_BLK_NUM;
MUINT32 LTMS_R1_LTMS_BLK_SZ;
MUINT32 LTMS_R1_LTMS_IN_SIZE;       //secure
MUINT32 LTMS_R1_LTMS_OUT_SIZE;      //secure
MUINT32 LTMS_R1_LTMS_ACT_WINDOW_X;  //secure
MUINT32 LTMS_R1_LTMS_ACT_WINDOW_Y;  //secure
MUINT32 LTMS_R1_LTMS_OUT_DATA_NUM;
MUINT32 LTMS_R1_LTMS_SRAM_CFG;
MUINT32 LTMSO_R1_LTMSO_XSIZE;
MUINT32 LTMSO_R1_LTMSO_YSIZE;
MUINT32 LTMSO_R1_LTMSO_STRIDE;
}PreSet_Reg;

typedef struct{
    SEC_MEM_TYPE type;
    MUINT32 module;
    MUINT32 cq;
    MUINT32 dummy;
    MUINT32 dupq;
    MUINT32 buff_size;
    uint64_t buff_va;
    MUINT32 port;
    MUINT32 buff_sec_mva;
    MINT32  memID;
    SecMgr_SubSecInfo sub;
}SecMgr_SecInfo;

typedef struct{
    MUINT32 CamModule;
    MUINT32 TwinCamModule;
    MUINT32 SecTG;
    MUINT32 DevID;
    uint64_t chk_handle;
}SecMgr_CamInfo;

typedef enum{
    SEC_CAM_A   = 0,
    SEC_CAM_B,
    SEC_CAM_C,
    SEC_CAM_MAX,  //3
    SEC_CAMSV_START = SEC_CAM_MAX,
    SEC_CAMSV_0 = SEC_CAMSV_START,
    SEC_CAMSV_1,
    SEC_CAMSV_2,
    SEC_CAMSV_3,
    SEC_CAMSV_4,
    SEC_CAMSV_5,
    SEC_CAMSV_MAX, //9
    MAX_SEC_ISP_HW_MODULE = SEC_CAMSV_MAX
}SEC_ISP_HW_MODULE;

typedef enum{
    SEC_STATE_NONE          = 0X00,
    SEC_STATE_READY2RUN     = 0X01,
    SEC_STATE_IDLE          = 0x02,
}SEC_STATE;


typedef enum{
    SET_NONE    = 0,
    SET_TUNING,
    SET_DEFAULT,
    SET_READY
}PRE_REG_STATE;

class SecMgr
{
public:
    SecMgr(){}
    virtual ~SecMgr(){}

    static SecMgr* SecMgr_Init();

    static SecMgr* SecMgr_GetMgrObj();

    static MUINT32 SecMgr_GetSecureCamStatus(ISP_HW_MODULE module); //For CQ check

    static MBOOL SecMgr_GetSecurePortStatus(ISP_HW_MODULE module,MUINT32 port);//For frm header check

    virtual MBOOL SecMgr_GetSecureTwinStatus(ISP_HW_MODULE module);

    virtual MBOOL SecMgr_detach(MUINT32 tg);

    virtual MBOOL SecMgr_attach(MUINT32 tg,MUINT32 sen_idx,vector<MUINT32> sec_port,MUINT32 burstq,MUINT32 tee_type,MUINT64 chk);

    virtual MBOOL SecMgr_attach_twin(ISP_HW_MODULE module);

    virtual MBOOL SecMgr_QuerySecMVAFromHandle(MUINT64 HandlePA,MUINT32* SecMemPA,MUINT64 chk);

    virtual MBOOL SecMgr_QueryFHSecMVA(SecMgr_SecInfo secinfo,MUINT32* SecMemPA);

    virtual MBOOL SecMgr_P1RegisterShareMem(SecMgr_SecInfo* secinfo);

    virtual MBOOL SecMgr_P1MigrateTable(SecMgr_SecInfo* secinfo);

    virtual MBOOL SecMgr_CheckSecReg(ISP_HW_MODULE module);

    virtual MBOOL SecMgr_SetDapcReg(ISP_HW_MODULE module);

    virtual MBOOL SecMgr_SecConfig(MUINT32 dma_port,MUINT32 tg_idx);

    virtual MBOOL SecMgr_start();

    virtual MBOOL SecMgr_stop();

    virtual MBOOL SecMgr_DumpSecBuffer(MUINT64 HandlePA, void *dst_buf, MUINT32 size);

    virtual MBOOL SecMgr_sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    virtual MBOOL SecMgr_SetDapcRegAddr(ISP_HW_MODULE module);

    virtual MBOOL SecMgr_SecCamEnable();

    virtual MBOOL SecMgr_GetPreReg(vector<MUINT32> *reg_addr, ISP_HW_MODULE module);

    virtual MBOOL SecMgr_SetPreReg(PreSet_Reg* dreg, MUINT32 IN_HSIZE, MUINT32 IN_VSIZE, MUINT32 RESIZE_MODE, ISP_HW_MODULE module);

    virtual PRE_REG_STATE SecMgr_GetPreRegStatus(ISP_HW_MODULE module);
};
#endif

