#ifndef _SEC_MGR_H_
#define _SEC_MGR_H_

#include <vector>
#include <isp_drv.h>
#include "isp_datatypes.h"
#include <ispio_pipe_ports.h>

using namespace NSCam::NSIoPipe;

#define DAPC_NUM 10

typedef struct{
    MUINT32 Num_of_Cam;
    MUINT32 CAM_CTL_EN;
    MUINT32 CAM_CTL_DMA_EN;
    MUINT32 CAM_CTL_SEL;
    MUINT32 CAM_CTL_EN2;
    MUINT32 AWB_W_HSIZE;
    MUINT32 AWB_W_VSIZE;
    MUINT32 AWB_W_HPIT;
    MUINT32 AWB_W_VPIT;
    MUINT32 PS_AWB_W_HSIZE;
    MUINT32 PS_AWB_W_VSIZE;
    MUINT32 PS_AWB_W_HPIT;
    MUINT32 PS_AWB_W_VPIT;
    MUINT32 AF_BLK2_XSIZE;
    MUINT32 AF_BLK2_YSIZE;
    MUINT32 FLK_SIZE_X;
    MUINT32 FLK_SIZE_Y;
    MUINT32 SecReg[DAPC_NUM];
    MUINT32 SecOn;
}SecMgr_QueryInfo;

enum SECMGRCmd {
    SECMGRCmd_QUERY_AF_BLK2_CONSTRAINT         = 0x0001,
    SECMGRCmd_QUERY_AWB_WIN_SIZE_CONSTRAINT    = 0x0002,
    SECMGRCmd_QUERY_AWB_WIN_PIT_CONSTRAINT     = 0x0003,
    SECMGRCmd_QUERY_PS_AWB_WIN_SIZE_CONSTRAINT = 0x0004,
    SECMGRCmd_QUERY_PS_AWB_WIN_PIT_CONSTRAINT  = 0x0005,
    SECMGRCmd_QUERY_FLK_CONSTRAINT             = 0x0006,
    SECMGRCmd_QUERY_MODULE_EN_CONSTRAINT       = 0x0007,
    SECMGRCmd_QUERY_DMA_EN_CONSTRAINT          = 0x0008,
    SECMGRCmd_QUERY_MODULE_EN2_CONSTRAINT      = 0x0009,
    SECMGRCmd_QUERY_SEL_CONSTRAINT             = 0x000A,
    SECMGRCmd_QUERY_NUM_CAM_CONSTRAINT         = 0x000B,
    SECMGRCmd_QUERY_CAP_CONSTRAINT             = 0x000C,
    SECMGRCmd_MAX
};

typedef enum{
    SECMEM_CQ_DESCRIPTOR_TABLE = 0,
    SECMEM_VIRTUAL_REG_TABLE,
    SECMEM_FRAME_HEADER,
    SECMEM_LSC,
    SECMEM_BPC,
    SECMEM_MAX
}SEC_MEM_TYPE;

typedef enum {
    CAM_PORT_IMGO = 0,
    CAM_PORT_RRZO,
    CAM_PORT_AAO,
    CAM_PORT_AFO,
    CAM_PORT_LSCI0,
    CAM_PORT_LSCI1,
    CAM_PORT_PDO,
    CAM_PORT_BPCI,
    CAM_PORT_LCSO,
    CAM_PORT_CAM_RSSO_A,

    CAM_PORT_UFEO,
    CAM_PORT_SOCO,
    CAM_PORT_SOC1,
    CAM_PORT_SOC2,
    CAM_PORT_CCUI,
    CAM_PORT_CCUO,
    CAM_PORT_RAWI_A,
    CAM_PORT_CCUG,
    CAM_PORT_PSO,
    CAM_PORT_AFO_1,

    CAM_PORT_LSCI_2,
    CAM_PORT_PDI,
    CAM_PORT_FLKO,
    CAM_PORT_LMVO,
    CAM_PORT_UFGO,
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
}SEC_ISP_HW_MODULE;

class SecMgr
{
public:
    SecMgr(){}
    virtual ~SecMgr(){}

    static SecMgr* SecMgr_Init();

    static SecMgr* SecMgr_GetMgrObj();

    static MUINT32 SecMgr_GetSecureCamStatus(ISP_HW_MODULE module); //For CQ check

    static MBOOL SecMgr_GetSecurePortStatus(ISP_HW_MODULE module,MUINT32 port);//For frm header check

    virtual MBOOL SecMgr_detach(MUINT32 tg);

    virtual MBOOL SecMgr_attach(MUINT32 tg,MUINT32 sen_idx,vector<MUINT32> sec_port,MUINT32 burstq,MUINT32 tee_type,MUINT64 chk);

    virtual MBOOL SecMgr_QuerySecMVAFromHandle(MUINT64 HandlePA,MUINT32* SecMemPA);

    virtual MBOOL SecMgr_QueryFHSecMVA(SecMgr_SecInfo secinfo,MUINT32* SecMemPA);

    virtual MBOOL SecMgr_P1RegisterShareMem(SecMgr_SecInfo* secinfo);

    virtual MBOOL SecMgr_P1MigrateTable(SecMgr_SecInfo* secinfo);

    virtual MBOOL SecMgr_CheckSecReg(ISP_HW_MODULE module, MUINT32 Addr);

    virtual MBOOL SecMgr_SecConfig(MUINT32 dma_port,MUINT32 tg_idx);

    virtual MBOOL SecMgr_start();

    virtual MBOOL SecMgr_stop();

    virtual MBOOL SecMgr_DumpSecBuffer(MUINT64 HandlePA, void *dst_buf, MUINT32 size);

    virtual MBOOL SecMgr_sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    virtual vector<PortID> SecMgr_GetSupportPort(MBOOL SecEnable);

    virtual MBOOL SecMgr_SetDapcReg();

    virtual MUINT32 SecMgr_GetDummySecMVA(MUINT32 dup);
};
#endif

