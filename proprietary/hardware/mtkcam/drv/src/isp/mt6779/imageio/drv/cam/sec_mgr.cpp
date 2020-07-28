#define LOG_TAG "SecMgr"

#include "sec_mgr.h"
#include "isp_function_cam.h"

#include <stdlib.h>
#include <dlfcn.h>

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace std;
using namespace NSCam::NSIoPipe;

DECLARE_DBG_LOG_VARIABLE(SecMgr);

// Clear previous define, use our own define.
#undef SecMgr_VRB
#undef SecMgr_DBG
#undef SecMgr_INF
#undef SecMgr_WRN
#undef SecMgr_ERR
#undef SecMgr_AST
#define SecMgr_VRB(fmt, arg...)        do { if (SecMgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define SecMgr_DBG(fmt, arg...)        do { if (SecMgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define SecMgr_INF(fmt, arg...)        do { if (SecMgr_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define SecMgr_WRN(fmt, arg...)        do { if (SecMgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define SecMgr_ERR(fmt, arg...)        do { if (SecMgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define SecMgr_AST(cond, fmt, arg...)  do { if (SecMgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define MTK_SEC_ISP_LIB            "libispcameraca.so"
#define MTK_SEC_ISP_CREATE         "MtkSecISP_tlcHandleCreate"
#define MTK_SEC_ISP_OPEN           "MtkSecISP_tlcOpen"
#define MTK_SEC_ISP_CLOSE          "MtkSecISP_tlcClose"
#define MTK_SEC_ISP_INIT           "MtkSecISP_tlcInit"
#define MTK_SEC_ISP_UNINIT         "MtkSecISP_tlcUnInit"
#define MTK_SEC_ISP_CONFIG         "MtkSecISP_tlcSecConfig"
#define MTK_SEC_ISP_QUERYPA        "MtkSecISP_tlcQueryMVAFromHandle"
#define MTK_SEC_ISP_QUERYFHPA      "MtkSecISP_tlcQueryFHPA"
#define MTK_SEC_ISP_QUERYHWINFO    "MtkSecISP_tlcQueryHWInfo"
#define MTK_SEC_ISP_REGISTER_MEM   "MtkSecISP_tlcRegisterShareMem"
#define MTK_SEC_ISP_MIGRATE_TABLE  "MtkSecISP_tlcMigrateTable"
#define MTK_SEC_ISP_DUMP_SECBUFFER "MtkSecISP_tlcDumpSecmem"
#define MTK_SEC_ISP_SETSECCAM      "MtkSecISP_tlcSetSecCam"
#define MTK_SEC_ISP_SETDAPCREG     "MtkSecISP_tlcSetDapcReg"

typedef void *ca_create_ptr();
typedef int ca_open_ptr(void*);
typedef int ca_close_ptr(void*);
typedef int ca_init_ptr(void*);
typedef int ca_uninit_ptr(void*);
typedef int ca_config_ptr(void*,MINT32,MINT32);
typedef int ca_querypa_ptr(void*,MUINT64,MUINT32*,TEE_MEM_TYPE,MUINT64);
typedef int ca_queryfhpa_ptr(void*,SecMgr_SecInfo,MUINT32*);
typedef int ca_queryinfo_ptr(void*,SecMgr_QueryInfo*);
typedef int ca_registersharemem_ptr(void*,SecMgr_SecInfo*);
typedef int ca_migratetable_ptr(void*,SecMgr_SecInfo*);
typedef int ca_dumpsecbuffer_ptr(void*,MUINT64,void*,MUINT32);
typedef int ca_setcaminfo_ptr(void*,SecMgr_CamInfo);
typedef int ca_setdapcreg_ptr(void*,SecMgr_RegInfo*,MUINT32);

ca_create_ptr           *g_isp_ca_handlecreate     = nullptr;
ca_open_ptr             *g_isp_ca_open             = nullptr;
ca_close_ptr            *g_isp_ca_close            = nullptr;
ca_init_ptr             *g_isp_ca_init             = nullptr;
ca_uninit_ptr           *g_isp_ca_uninit           = nullptr;
ca_config_ptr           *g_isp_ca_config           = nullptr;
ca_querypa_ptr          *g_isp_ca_querypa          = nullptr;
ca_queryfhpa_ptr        *g_isp_ca_queryfhpa        = nullptr;
ca_queryinfo_ptr        *g_isp_ca_queryinfo        = nullptr;
ca_registersharemem_ptr *g_isp_ca_registersharemem = nullptr;
ca_migratetable_ptr     *g_isp_ca_migratetable     = nullptr;
ca_dumpsecbuffer_ptr    *g_isp_ca_dumpsecbuffer    = nullptr;
ca_setcaminfo_ptr       *g_isp_ca_setcaminfo       = nullptr;
ca_setdapcreg_ptr       *g_isp_ca_setdapcreg       = nullptr;


#define MAX_DMA_PORT_NUM            (3)   //imgo, rrzo, crzo_r1
#define IMG_HEADER_BUF_NUM          (16)
#define CAMSV_HEADER_BUF_NUM        (2)

#define PortMap(PortIdx) ({\
    static MUINT32 _idx = 0;\
    if (PortIdx == PORT_IMGO.index)\
        _idx = ((unsigned int)1 << 0);\
    else if (PortIdx == PORT_RRZO.index)\
        _idx = ((unsigned int)1 << 2);\
    else if (PortIdx == PORT_CRZO_R1.index)\
        _idx = ((unsigned int)1 << 26);\
    else\
        SecMgr_ERR("Not support PortIdx:(%d)",PortIdx);\
    _idx;\
})

#define str_dmao(x)  ({\
   static char _str_dmao[10];\
   _str_dmao[0] = '\0';\
   switch(x){\
       case _imgo_:\
           sprintf(_str_dmao,"imgo");\
           break;\
       case _rrzo_:\
           sprintf(_str_dmao,"rrzo");\
           break;\
       case _lcso_:\
           sprintf(_str_dmao,"lcso");\
           break;\
       case _lmvo_:\
           sprintf(_str_dmao,"lmvo");\
           break;\
       case _rsso_:\
           sprintf(_str_dmao,"rsso");\
           break;\
       case _crzo_:\
           sprintf(_str_dmao,"crzo");\
           break;\
   }\
   _str_dmao;\
})

#define PRE_REG_NUM 14
static MUINT32 pre_reg_addr[PRE_REG_NUM] = {
	0x6AC,
	0x6B0,
	0x6B4,
	0x8C4,
	0x8C8,
	0x930,
	0x934,
	0x938,
	0x93C,
	0x940,
	0x948,
        0x44FC,
        0x4500,
        0x4504
};

class SecMgrImp : public SecMgr
{
public:
    SecMgrImp();
    ~SecMgrImp(){}

    virtual MBOOL SecMgr_LoadCA();

    virtual MBOOL SecMgr_InitCA();

    virtual MBOOL SecMgr_OpenCA();

    virtual MBOOL SecMgr_GetHWCapability();

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

    virtual MUINT32 SecMgr_GetSecureCamStatus(ISP_HW_MODULE module);

    virtual MBOOL SecMgr_GetSecurePortStatus(ISP_HW_MODULE module,MUINT32 port);

    virtual MBOOL SecMgr_GetSecureTwinStatus(ISP_HW_MODULE module);

    virtual MBOOL SecMgr_SetDapcRegAddr(ISP_HW_MODULE module);

    virtual MBOOL SecMgr_SecCamEnable();

    virtual MBOOL SecMgr_GetPreReg(vector<MUINT32> *reg_addr,ISP_HW_MODULE module);

    virtual MBOOL SecMgr_SetPreReg(PreSet_Reg* dreg, MUINT32 IN_HSIZE, MUINT32 IN_VSIZE, MUINT32 RESIZE_MODE, ISP_HW_MODULE module);

    virtual PRE_REG_STATE SecMgr_GetPreRegStatus(ISP_HW_MODULE module);

private:
    SecMgr_QueryInfo SecHwInfo[2];//0 for secure constraint; 1 for non-secure constraint

    SecMgr_CamInfo CamInfo[SEC_CAM_MAX];

    ISP_DRV_CAM* m_pDrv[SEC_CAM_MAX];

    IspDrvImp* m_pIspDrv[SEC_CAM_MAX];

    static MUINT32 Num_SecCam;

    static SecMgr_RegInfo regval[SEC_CAM_MAX];

    MUINT32 fh_sec_mva[SEC_CAM_MAX][MAX_DMA_PORT_NUM][IMG_HEADER_BUF_NUM];

    uint64_t fh_sec_mva_idx[SEC_CAM_MAX][MAX_DMA_PORT_NUM][IMG_HEADER_BUF_NUM];

    vector<MUINT32> buf_sec_mva;

    vector<uint64_t> buf_sec_mva_idx;

    vector<MUINT32> lsc_buf_sec_mva;

    vector<uint64_t> lsc_buff_idx;  //lsc buff's VA

    vector<MUINT32> bpc_buf_sec_mva;

    vector<uint64_t> bpc_buff_idx;  //bpc buff's VA

    vector<MUINT32> twin_buf_sec_mva[MAX_DMA_PORT_NUM];

    vector<uint64_t> twin_buff_idx[MAX_DMA_PORT_NUM];  //bpc buff's VA

    TEE_MEM_TYPE tee_mem_type;

    vector<MUINT32> enable_secport;

    sem_t mSemSecDone;

    SEC_STATE mState[SEC_CAM_MAX];

    SEC_STATE mInitState;

    PRE_REG_STATE pre_reg_status;

};

static SecMgrImp gSecMgr;
static void* ispHandle;
static void* m_isp_ca;

MUINT32 SecMgrImp::Num_SecCam = 0;
SecMgr_RegInfo SecMgrImp::regval[SEC_CAM_MAX];

SecMgrImp::SecMgrImp()
{
    memset(&SecHwInfo[0], 0, sizeof(SecMgr_QueryInfo));
    memset(&SecHwInfo[1], 0, sizeof(SecMgr_QueryInfo));
    memset(&this->mSemSecDone, 0, sizeof(sem_t));

    this->tee_mem_type = TEE_MAX;
    this->mInitState = SEC_STATE_NONE;

    this->buf_sec_mva.clear();
    this->buf_sec_mva_idx.clear();
    this->lsc_buf_sec_mva.clear();
    this->lsc_buff_idx.clear();
    this->bpc_buf_sec_mva.clear();
    this->bpc_buff_idx.clear();
    this->enable_secport.clear();
    this->pre_reg_status = SET_NONE;

    for (MUINT32 i=0;i<SEC_CAM_MAX;i++){
        memset(&CamInfo[i], 0, sizeof(SecMgr_CamInfo));
        memset(&regval[i],0, sizeof(SecMgr_RegInfo));
        this->CamInfo[i].CamModule = PHY_CAM;
        this->CamInfo[i].TwinCamModule = PHY_CAM;
        this->m_pDrv[i] = NULL;
        this->m_pIspDrv[i] = NULL;
        this->mState[i] = SEC_STATE_NONE;

        for(MUINT32 j=0;j<MAX_DMA_PORT_NUM;j++){
            twin_buf_sec_mva[j].clear();
            twin_buff_idx[j].clear();
            for(MUINT32 k=0;k<IMG_HEADER_BUF_NUM;k++){
                fh_sec_mva[i][j][k] = 0;
                fh_sec_mva_idx[i][j][k] = 0;
            }
        }
    }
}

MBOOL SecMgrImp::SecMgr_sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int ret = 0; //0 for ok; -1 for fail
    SecMgr_INF("+ (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", cmd, arg1, arg2, arg3);
    switch ( cmd ) {
        case SECMGRCmd_QUERY_CTL_EN_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_EN;
                break;
            }
        case SECMGRCmd_QUERY_CTL_EN2_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_EN2;
                break;
            }
        case SECMGRCmd_QUERY_CTL_EN3_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_EN3;
                break;
            }
        case SECMGRCmd_QUERY_CTL_EN4_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_EN;
                break;
            }
        case SECMGRCmd_QUERY_DMA_EN_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_DMA_EN;
                break;
            }
        case SECMGRCmd_QUERY_DMA2_EN_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_DMA2_EN;
                break;
            }
        case SECMGRCmd_QUERY_CTL_SEL_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_SEL;
                break;
            }
        case SECMGRCmd_QUERY_CTL_SEL2_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_SEL2;
                break;
            }
        case SECMGRCmd_QUERY_AWB_WIN_SIZE_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].AA_AWB_W_HSIZE;
                *(MUINT32*)arg2 = this->SecHwInfo[0].AA_AWB_W_VSIZE;
                break;
            }
        case SECMGRCmd_QUERY_LCES_OUT_SIZE_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].LCES_OUT_HT;
                *(MUINT32*)arg2 = this->SecHwInfo[0].LCES_OUT_WD;
                break;
            }
        case SECMGRCmd_QUERY_FLK_SIZE_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].FLK_SIZE_X;
                *(MUINT32*)arg2 = this->SecHwInfo[0].FLK_SIZE_X;
                break;
            }

        case SECMGRCmd_QUERY_LTMS_IN_SIZE_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].LTMS_IN_HSIZE;
                *(MUINT32*)arg2 = this->SecHwInfo[0].LTMS_IN_VSIZE;
                break;
            }
        case SECMGRCmd_QUERY_LTMS_OUT_SIZE_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].LTMS_OUT_HSIZE;
                *(MUINT32*)arg2 = this->SecHwInfo[0].LTMS_OUT_VSIZE;
                break;
            }

        case SECMGRCmd_QUERY_LTMS_ACT_WINDOW_X_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].LTMS_ACT_WIN_X_START;
                *(MUINT32*)arg2 = this->SecHwInfo[0].LTMS_ACT_WIN_X_END;
                break;
            }
        case SECMGRCmd_QUERY_LTMS_ACT_WINDOW_Y_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].LTMS_ACT_WIN_Y_START;
                *(MUINT32*)arg2 = this->SecHwInfo[0].LTMS_ACT_WIN_Y_END;
                break;
            }
        case SECMGRCmd_QUERY_TSFS_SIZE_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].TSFS_W_VSIZE;
                *(MUINT32*)arg2 = this->SecHwInfo[0].TSFS_W_HSIZE;
                break;
            }
        case SECMGRCmd_QUERY_AF_BLK_PROT_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].AF_PROT_BLK_XSIZE;
                *(MUINT32*)arg2 = this->SecHwInfo[0].AF_PROT_BLK_YSIZE;
                break;
            }
        case SECMGRCmd_QUERY_NUM_CAM_CONSTRAINT:
            {
                 *(MUINT32*)arg1 = this->SecHwInfo[0].Num_of_Cam;
                 break;
            }
        case SECMGRCmd_QUERY_CAP_CONSTRAINT:
            {
                 *(MBOOL*)arg1 = MTRUE;
                 break;
            }
        default:
            {
                SecMgr_ERR("Not support SecMgr command!");
                ret = -1;
                break;
            }
    }

    if ( ret != 0){
        SecMgr_ERR("sendCommand(0x%x) error!",cmd);
        return MFALSE;
    }

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_LoadCA()
{
    SecMgr_DBG("+");
    MBOOL ret = MTRUE;
    m_isp_ca = dlopen(MTK_SEC_ISP_LIB, RTLD_NOW);

    if(nullptr == m_isp_ca){
        SecMgr_ERR("ISP CA LIB open failed (%s)",dlerror());
        return MFALSE;
    }

    SecMgr_INF("ISP CA LIB open success");
    if(nullptr != m_isp_ca){
        if(nullptr == g_isp_ca_handlecreate){
            g_isp_ca_handlecreate = (ca_create_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_CREATE);
            if(nullptr == g_isp_ca_handlecreate){
                SecMgr_ERR("Load CA create handle failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_open){
            g_isp_ca_open = (ca_open_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_OPEN);
            if(nullptr == g_isp_ca_open){
                SecMgr_ERR("Load CA open failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_close){
            g_isp_ca_close = (ca_close_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_CLOSE);
            if (nullptr == g_isp_ca_close){
                SecMgr_ERR("Load CA close failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_init){
            g_isp_ca_init = (ca_init_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_INIT);
            if(nullptr == g_isp_ca_init){
                SecMgr_ERR("Load CA init failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_uninit){
            g_isp_ca_uninit = (ca_uninit_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_UNINIT);
            if(nullptr == g_isp_ca_uninit){
                SecMgr_ERR("Load CA uninit failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_config){
            g_isp_ca_config = (ca_config_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_CONFIG);
            if(nullptr == g_isp_ca_config){
                SecMgr_ERR("Load CA config failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_querypa){
            g_isp_ca_querypa = (ca_querypa_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_QUERYPA);
            if(nullptr == g_isp_ca_querypa){
                SecMgr_ERR("Load CA querypa failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_queryfhpa){
            g_isp_ca_queryfhpa = (ca_queryfhpa_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_QUERYFHPA);
            if(nullptr == g_isp_ca_queryfhpa){
                SecMgr_ERR("Load CA querypa failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_queryinfo){
            g_isp_ca_queryinfo = (ca_queryinfo_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_QUERYHWINFO);
            if(nullptr == g_isp_ca_queryinfo){
                SecMgr_ERR("Load CA queryinfo failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_registersharemem){
            g_isp_ca_registersharemem = (ca_registersharemem_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_REGISTER_MEM);
            if(nullptr == g_isp_ca_registersharemem){
                SecMgr_ERR("Load CA register share mem failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_migratetable){
            g_isp_ca_migratetable = (ca_migratetable_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_MIGRATE_TABLE);
            if(nullptr == g_isp_ca_migratetable){
                SecMgr_ERR("Load CA migrate table failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_dumpsecbuffer){
            g_isp_ca_dumpsecbuffer = (ca_dumpsecbuffer_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_DUMP_SECBUFFER);
            if(nullptr == g_isp_ca_dumpsecbuffer){
                SecMgr_ERR("Load CA dump secbuffer failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_setcaminfo){
            g_isp_ca_setcaminfo = (ca_setcaminfo_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_SETSECCAM);
            if(nullptr == g_isp_ca_setcaminfo){
                SecMgr_ERR("Load CA set caminfo failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if(nullptr == g_isp_ca_setdapcreg){
            g_isp_ca_setdapcreg = (ca_setdapcreg_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_SETDAPCREG);
            if(nullptr == g_isp_ca_setdapcreg){
                SecMgr_ERR("Load CA set dapareg failed!");
                return MFALSE;
            }
        }
    }

Exit:
    if(ret == MFALSE){
        dlclose(m_isp_ca);
        m_isp_ca = NULL;
    }

    return ret;
}


MBOOL SecMgrImp::SecMgr_OpenCA()
{
    ispHandle = g_isp_ca_handlecreate();
    if(ispHandle == NULL){
        SecMgr_ERR("ISP CA handle create failed!");
        return MFALSE;
    }
    SecMgr_DBG("SecMgr_OpenCA+ (0x%x)",ispHandle);

    if(!g_isp_ca_open(ispHandle)){
        SecMgr_ERR("ISP CA open failed!");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_InitCA()
{
    SecMgr_DBG("SecMgr_InitCA+ (0x%x)",ispHandle);

    if(!g_isp_ca_init(ispHandle)){
        SecMgr_ERR("ISP CA init failed!");
        return MFALSE;
    }

    this->mInitState = SEC_STATE_READY2RUN;
    ::sem_post(&this->mSemSecDone);

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_QuerySecMVAFromHandle(MUINT64 HandlePA,MUINT32* SecMemPA,MUINT64 chk)
{
    SecMgr_INF("SecMgr_QuerySecMVAFromHandle+ HandlePA:0x%lx chk:0x%llx",(MUINT32)HandlePA,chk);

    if(this->buf_sec_mva.size() < 1){
        if(!g_isp_ca_querypa(ispHandle, HandlePA, SecMemPA, this->tee_mem_type,chk)){
            SecMgr_ERR("Query secmem PA failed!");
            return MFALSE;
        }

       this->buf_sec_mva.push_back(*SecMemPA);
       this->buf_sec_mva_idx.push_back(HandlePA);

    }else{
         MBOOL find = MFALSE;
         for(MUINT32 i=0; i<this->buf_sec_mva.size(); i++){
             if(HandlePA == this->buf_sec_mva_idx.at(i)){
                 *SecMemPA = this->buf_sec_mva.at(i);
                 find = MTRUE;
                 break;
             }
         }

         if(find == MFALSE){
             if(!g_isp_ca_querypa(ispHandle, HandlePA, SecMemPA, this->tee_mem_type,chk)){
                 SecMgr_ERR("Query secmem PA failed!");
                 return MFALSE;
             }
             this->buf_sec_mva.push_back(*SecMemPA);
             this->buf_sec_mva_idx.push_back(HandlePA);
         }

    }
    SecMgr_INF("SecMemPA:0x%x",*SecMemPA);

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_QueryFHSecMVA(SecMgr_SecInfo secinfo,MUINT32* SecMemPA)
{
    SecMgr_DBG("+");

    SecMgr_SecInfo sinfo;
    sinfo.type = secinfo.type;
    sinfo.module = secinfo.module;
    sinfo.buff_size = secinfo.buff_size;
    sinfo.buff_va = secinfo.buff_va;
    MUINT32 _port = 0;

    switch(secinfo.port)
    {
        case _imgo_:
            sinfo.port = PORT_IMGO.index;
            _port = 0;
            break;
        case _rrzo_:
            sinfo.port = PORT_RRZO.index;
            _port = 1;
            break;
        case _crzo_:
            sinfo.port = PORT_CRZO_R1.index;
            _port = 2;
            break;
        default:
            SecMgr_ERR("Unsupport port type (%d)",secinfo.port);
            return MFALSE;
            break;
    }

    for(MUINT32 i=0; i < IMG_HEADER_BUF_NUM; i++){
        if(this->fh_sec_mva_idx[sinfo.module][_port][i] == sinfo.buff_va)
            *SecMemPA = this->fh_sec_mva[sinfo.module][_port][i];
    }

    SecMgr_DBG("DMA port:%s FH_SecMemPA:0x%x",str_dmao(secinfo.port),*SecMemPA);

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_GetHWCapability()
{
    SecMgr_INF("+ (0x%x)",ispHandle);

    this->SecHwInfo[0].SecOn = 1;
    if(!g_isp_ca_queryinfo(ispHandle,&this->SecHwInfo[0])){
        SecMgr_ERR("ISP CA get secure HW capability failed!");
        return MFALSE;
    }

    this->SecHwInfo[1].SecOn = 0;
    if(!g_isp_ca_queryinfo(ispHandle,&this->SecHwInfo[1])){
        SecMgr_ERR("ISP CA get non-secure HW capability failed!");
        return MFALSE;
    }

    if(this->SecHwInfo[0].Num_of_Cam == 0 || this->SecHwInfo[0].CAM_CTL_DMA_EN == 0){
        SecMgr_ERR("Get wrong HW Capability!");
        return MFALSE;
    }

    SecMgr_INF("SecHwInfo[0].Num_of_Cam:0x%x",this->SecHwInfo[0].Num_of_Cam);
    SecMgr_INF("SecHwInfo[0].CAM_CTL_DMA_EN:0x%x",this->SecHwInfo[0].CAM_CTL_DMA_EN);
    SecMgr_DBG("SecHwInfo[0].CAM_CTL_EN:0x%x",this->SecHwInfo[0].CAM_CTL_EN);
    SecMgr_DBG("SecHwInfo[0].CAM_CTL_EN2:0x%x",this->SecHwInfo[0].CAM_CTL_EN2);
    SecMgr_DBG("SecHwInfo[0].CAM_CTL_SEL:0x%x",this->SecHwInfo[0].CAM_CTL_SEL);
    for(MUINT i=0;i<DAPC_NUM_CQ;i++)
        SecMgr_DBG("SecReg[%d]:0x%x",i,this->SecHwInfo[0].SecReg_ADDR_CQ[i]);

    return MTRUE;
}

#define str(x)  ({\
   static char _str[48];\
   _str[0] = '\0';\
   switch(x){\
       case SECMEM_CQ_DESCRIPTOR_TABLE:\
           sprintf(_str,"SECMEM_CQ_DESCRIPTOR_TABLE");\
           break;\
       case SECMEM_VIRTUAL_REG_TABLE:\
           sprintf(_str,"SECMEM_VIRTUAL_REG_TABLE");\
           break;\
       case SECMEM_FRAME_HEADER:\
           sprintf(_str,"SECMEM_FRAME_HEADER");\
           break;\
       case SECMEM_TWIN_FRAME_HEADER:\
           sprintf(_str,"SECMEM_TWIN_FRAME_HEADER");\
           break;\
       case SECMEM_LSC:\
           sprintf(_str,"SECMEM_LSC");\
           break;\
       case SECMEM_BPC:\
           sprintf(_str,"SECMEM_BPC");\
           break;\
   }\
   _str;\
})

MBOOL SecMgrImp::SecMgr_P1RegisterShareMem(SecMgr_SecInfo* secinfo)
{
   static MUINT32 fh_idx = 0;
   MUINT32 _port = 0, _port_ = 0;

   switch(secinfo->type)
   {
       case SECMEM_CQ_DESCRIPTOR_TABLE:
       case SECMEM_VIRTUAL_REG_TABLE:
            SecMgr_INF("+ module:0x%x cq:%d dup:%d dmy:%d type:%s buff_va:0x%llx size:0x%x memID:0x%x",
            secinfo->module,secinfo->cq,secinfo->dupq,secinfo->dummy,str(secinfo->type),secinfo->buff_va,secinfo->buff_size,secinfo->memID);
            SecMgr_INF("+ module:0x%x cq:%d dup:%d dmy:%d sub-type:%s buff_va:0x%llx size:0x%x memID:0x%x",
            secinfo->module,secinfo->cq,secinfo->dupq,secinfo->dummy,str(secinfo->sub.type),secinfo->sub.buff_va,secinfo->sub.buff_size,secinfo->sub.memID);

            if(!g_isp_ca_registersharemem(ispHandle,secinfo)){
                SecMgr_ERR("Share mem register failed (module:0x%x type:%s buff:0x%llx size:0x%x)",
                            secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                return MFALSE;
            }
            break;
       case SECMEM_FRAME_HEADER:
            SecMgr_DBG("+ module:0x%x type:%s DMA port:%d",secinfo->module,str(secinfo->type),secinfo->port);
            if(!g_isp_ca_registersharemem(ispHandle,secinfo)){
                SecMgr_ERR("Share mem register failed (module:0x%x type:%s buff:0x%llx size:0x%x)",
                            secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                return MFALSE;
            }

            if(secinfo->port == PORT_IMGO.index){
                _port = 0;
                _port_ = _imgo_;
            }else if(secinfo->port == PORT_RRZO.index){
                _port = 1;
                _port_ = _rrzo_;
            }else if(secinfo->port == PORT_CRZO_R1.index){
                _port = 1;
                _port_ = _crzo_;
            }else{
                SecMgr_ERR("Unsupport port type (%d)",secinfo->port);
                return MFALSE;
            }

            this->fh_sec_mva[secinfo->module][_port][fh_idx] = secinfo->buff_sec_mva;
            this->fh_sec_mva_idx[secinfo->module][_port][fh_idx] = secinfo->buff_va;
            SecMgr_INF("(%s_%d)FH va:0x%llx sec_mva:0x%x",str_dmao(_port_),fh_idx,
                       this->fh_sec_mva_idx[secinfo->module][_port][fh_idx],this->fh_sec_mva[secinfo->module][_port][fh_idx]);
            fh_idx++;
            if (fh_idx == IMG_HEADER_BUF_NUM)
                fh_idx = 0;
            break;
       case SECTABLE_LSC:
            SecMgr_INF("+ module:0x%x type:%s buff:0x%llx size:0x%x memID:0x%x",
              secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size,secinfo->memID);

            if(this->lsc_buff_idx.size() < 1){
                if(!g_isp_ca_registersharemem(ispHandle,secinfo)){
                    SecMgr_ERR("Share mem register failed (module:0x%x type:%s buff:0x%llx size:0x%x)",
                            secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                    return MFALSE;
                }

                this->lsc_buff_idx.push_back(secinfo->buff_va);
                this->lsc_buf_sec_mva.push_back(secinfo->buff_sec_mva);

            }else{
                MBOOL find = MFALSE;
                for(MUINT32 i=0; i<this->lsc_buff_idx.size(); i++){
                    if(secinfo->buff_va == this->lsc_buff_idx.at(i)){
                        SecMgr_INF("Find LSC buffer:%d_0x%llx sec_mva:0x%x",i,this->lsc_buff_idx.at(i),this->lsc_buf_sec_mva.at(i));
                        secinfo->buff_sec_mva = this->lsc_buf_sec_mva.at(i);
                        find = MTRUE;
                        break;
                    }
                }

                if(find == MFALSE){
                    if (!g_isp_ca_registersharemem(ispHandle,secinfo)){
                        SecMgr_ERR("Share mem register failed (module:0x%x type:%s buff:0x%llx size:0x%x)",
                                secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                        return MFALSE;
                    }

                    this->lsc_buff_idx.push_back(secinfo->buff_va);
                    this->lsc_buf_sec_mva.push_back(secinfo->buff_sec_mva);
                }

           }
           break;
       case SECTABLE_BPC:
            SecMgr_INF("+ module:0x%x type:%s buff:0x%llx size:0x%x memID:0x%x",
              secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size,secinfo->memID);

            if(this->bpc_buff_idx.size() < 1){
                if(!g_isp_ca_registersharemem(ispHandle,secinfo)){
                    SecMgr_ERR("Share mem register failed (module:0x%x type:%s buff:0x%llx size:0x%x)",
                            secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                    return MFALSE;
                }

                this->bpc_buff_idx.push_back(secinfo->buff_va);
                this->bpc_buf_sec_mva.push_back(secinfo->buff_sec_mva);
            }else{
                MBOOL find = MFALSE;
                for(MUINT32 i=0; i<this->bpc_buff_idx.size(); i++){
                    if(secinfo->buff_va == this->bpc_buff_idx.at(i)){
                        SecMgr_INF("Find BPC buffer:%d_0x%llx sec_mva:0x%x",i,this->bpc_buff_idx.at(i),this->bpc_buf_sec_mva.at(i));
                        secinfo->buff_sec_mva = this->bpc_buf_sec_mva.at(i);
                        find = MTRUE;
                        break;
                    }
                }

                if(find == MFALSE){
                    if (!g_isp_ca_registersharemem(ispHandle,secinfo)){
                        SecMgr_ERR("Share mem register failed (module:0x%x type:%s buff:0x%llx size:0x%x)",
                                secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                        return MFALSE;
                    }

                    this->bpc_buff_idx.push_back(secinfo->buff_va);
                    this->bpc_buf_sec_mva.push_back(secinfo->buff_sec_mva);
                }

            }
            break;
       case SECMEM_TWIN_FRAME_HEADER:
            if(secinfo->port == _imgo_){
                secinfo->port = PORT_IMGO.index;
                _port_ = _imgo_;
                _port = 0;
            }else if(secinfo->port == _rrzo_){
                secinfo->port = PORT_RRZO.index;
                _port_ = _rrzo_;
                _port = 1;
            }else if(secinfo->port == _crzo_){
                secinfo->port = PORT_CRZO_R1.index;
                _port_ = _crzo_;
                _port = 2;
            }else{
                SecMgr_ERR("Unsupport port type (%d)",secinfo->port);
                return MFALSE;
            }
            SecMgr_INF("+ Twin module:0x%x type:%s DMA port:%d",secinfo->module,str(secinfo->type),secinfo->port);

            secinfo->type = SECMEM_FRAME_HEADER;
            if(this->twin_buff_idx[_port].size() < 1){
                if(!g_isp_ca_registersharemem(ispHandle,secinfo)){
                    SecMgr_ERR("Share mem register failed (twin module:0x%x type:%s buff:0x%llx size:0x%x)",
                                secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                    return MFALSE;
                }
                this->twin_buff_idx[_port].push_back(secinfo->buff_va);
                this->twin_buf_sec_mva[_port].push_back(secinfo->buff_sec_mva);
                SecMgr_INF("(%s_%d)FH va:0x%llx sec_mva:0x%x",str_dmao(_port_),this->twin_buff_idx[_port].size()-1,
                           secinfo->buff_va,secinfo->buff_sec_mva);

            }else{
                MBOOL find = MFALSE;
                for(MUINT32 i=0; i<this->twin_buff_idx[_port].size(); i++){
                    if(secinfo->buff_va == this->twin_buff_idx[_port].at(i)){
                        SecMgr_INF("Find FH:%d_0x%llx sec_mva:0x%x",i,this->twin_buff_idx[_port].at(i),this->twin_buf_sec_mva[_port].at(i));
                        secinfo->buff_sec_mva = this->twin_buf_sec_mva[_port].at(i);
                        find = MTRUE;
                        break;
                    }
                }

                if(find == MFALSE){
                    if (!g_isp_ca_registersharemem(ispHandle,secinfo)){
                        SecMgr_ERR("Share mem register failed (twin module:0x%x type:%s buff:0x%llx size:0x%x)",
                                    secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                        return MFALSE;
                    }
                    this->twin_buff_idx[_port].push_back(secinfo->buff_va);
                    this->twin_buf_sec_mva[_port].push_back(secinfo->buff_sec_mva);
                    SecMgr_INF("(%s_%d)FH va:0x%llx sec_mva:0x%x",str_dmao(_port_),this->twin_buff_idx[_port].size()-1,
                               secinfo->buff_va,secinfo->buff_sec_mva);
                }

            }
            break;

       default:
            SecMgr_ERR("Unsupport table type (%d)",secinfo->type);
            return MFALSE;
            break;
   }

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_P1MigrateTable(SecMgr_SecInfo* secinfo)
{
   SecMgr_SecInfo sinfo;

   sinfo.type = secinfo->type;
   sinfo.module = secinfo->module;

   switch(secinfo->type)
   {
       case SECMEM_CQ_DESCRIPTOR_TABLE:
       case SECMEM_VIRTUAL_REG_TABLE:
            sinfo.cq = secinfo->cq;
            sinfo.dupq = secinfo->dupq;
            sinfo.dummy = secinfo->dummy;
            sinfo.sub.type = secinfo->sub.type;
            SecMgr_INF("module:0x%x type:%s cq:%d dup:%d dummy:%d sub-type:%s",
                        secinfo->module,str(secinfo->type),
                        secinfo->cq,secinfo->dupq,secinfo->dummy,str(secinfo->sub.type));
            break;
       case SECMEM_FRAME_HEADER:
            sinfo.buff_va = secinfo->buff_va;
            switch(secinfo->port)
            {
                case _imgo_:
                    sinfo.port = PORT_IMGO.index;
                    break;
                case _rrzo_:
                    sinfo.port = PORT_RRZO.index;
                    break;
                case _crzo_:
                    sinfo.port = PORT_CRZO_R1.index;
                    break;
                default:
                    SecMgr_ERR("Unsupport port type (%d)",secinfo->port);
                    return MFALSE;
                    break;
            }

            SecMgr_INF("module:0x%x type:%s DMA port:%s buff:0x%llx",
                        secinfo->module,str(secinfo->type),str_dmao(secinfo->port),secinfo->buff_va);
            break;
       case SECTABLE_LSC:
       case SECTABLE_BPC:
            sinfo.buff_va = secinfo->buff_va;
            break;
   }

   if (!g_isp_ca_migratetable(ispHandle,&sinfo)){
       SecMgr_ERR("Share mem migration failed (module:0x%x type:%s)",secinfo->module,str(secinfo->type));
       return MFALSE;
   }

   if(secinfo->type == SECMEM_FRAME_HEADER)
       SecMgr_INF("E_IMG_PA:0x%x",sinfo.buff_sec_mva);

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_SecConfig(MUINT32 dma_port,MUINT32 tg_idx)
{
    SEC_DMA_PORT _port = CAM_PORT_MAX;

    SecMgr_INF("+ DMA port:%d TG:%d",dma_port,tg_idx);

    if(this->SecHwInfo[0].CAM_CTL_DMA_EN & PortMap(dma_port)){
        if(dma_port == PORT_RRZO.index){
            switch(tg_idx){
                case TG_A:
                    _port = CAM_PORT_RRZO_A;
                    break;
                case TG_B:
                    _port = CAM_PORT_RRZO_B;
                    break;
                case TG_C:
                    _port = CAM_PORT_RRZO_C;
                    break;
                default:
                    SecMgr_ERR("Unsupport tg type (%d)",tg_idx);
                    return MFALSE;
                    break;
             }
        }else if(dma_port == PORT_IMGO.index){
             switch(tg_idx){
                case TG_A:
                    _port = CAM_PORT_IMGO_A;
                    break;
                case TG_B:
                    _port = CAM_PORT_IMGO_B;
                    break;
                case TG_C:
                    _port = CAM_PORT_IMGO_C;
                    break;
                default:
                    SecMgr_ERR("Unsupport tg type (%d)",tg_idx);
                    return MFALSE;
                    break;
             }
        }else if(dma_port == PORT_CRZO_R1.index){
             switch(tg_idx){
                case TG_A:
                    _port = CAM_PORT_CRZO_A;
                    break;
                case TG_B:
                    _port = CAM_PORT_CRZO_B;
                    break;
                case TG_C:
                    _port = CAM_PORT_CRZO_C;
                    break;
                default:
                    SecMgr_ERR("Unsupport tg type (%d)",tg_idx);
                    return MFALSE;
                    break;
             }
        }

        if(!g_isp_ca_config(ispHandle,_port,tg_idx)){
            SecMgr_ERR("ISP CA Config DMA Port(%d) failed!",dma_port);
            return MFALSE;
        }

    }else{
        SecMgr_INF("Not support DMA port:(%d)",dma_port);
        return MFALSE;
    }

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_SetDapcRegAddr(ISP_HW_MODULE module)
{
    SecMgr_DBG("+");

    vector<MUINT32> reg_list_cq;

    if(this->m_pDrv[module] == NULL){
        this->m_pDrv[module] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance((ISP_HW_MODULE)module,ISP_DRV_CQ_THRE0,0,"SecMgr");
        this->m_pIspDrv[module] = (IspDrvImp*)IspDrvImp::createInstance(module);

        for(MUINT32 i=0;i<DAPC_NUM_CQ;i++)
            reg_list_cq.push_back(this->SecHwInfo[0].SecReg_ADDR_CQ[i]);

        this->m_pDrv[module]->setDapcReg(reg_list_cq,module);  //updated by CQ
        this->m_pIspDrv[module]->setDapcReg(reg_list_cq);
    }

    if(this->mState[module] == SEC_STATE_IDLE){
        this->m_pDrv[module]->setDapcReg(reg_list_cq,module);
    }

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_GetPreReg(vector<MUINT32> *reg_addr, ISP_HW_MODULE module)
{
    SecMgr_DBG("+ module:0x%x");

    for(MUINT32 i=0;i<PRE_REG_NUM;i++){
        reg_addr->push_back(pre_reg_addr[i]);
        SecMgr_INF("LTM/LTMS addr:0x%x",pre_reg_addr[i]);
    }

    return MTRUE;
}

PRE_REG_STATE SecMgrImp::SecMgr_GetPreRegStatus(ISP_HW_MODULE module)
{
    SecMgr_DBG("+ module:0x%x");
    if(this->pre_reg_status == SET_NONE){
        this->pre_reg_status = SET_READY;
        return SET_DEFAULT;
    }else{
        SecMgr_DBG("LTM/LTMS initialization has been done");
        return SET_READY;
    }
}

MBOOL SecMgrImp::SecMgr_SetPreReg(PreSet_Reg* dreg, MUINT32 IN_HSIZE, MUINT32 IN_VSIZE, MUINT32 RESIZE_MODE, ISP_HW_MODULE module)
{
#define LTM_BLK_X_NUM 12
#define LTM_BLK_Y_NUM 9

    SecMgr_INF("+ module:0x%x IN_HSIZE:0x%x IN_VSIZE:0x%x RESIZE_MODE:0x%x",module,IN_HSIZE,IN_VSIZE,RESIZE_MODE);

    IMemDrv* mpIMem;

    if ((IN_HSIZE != 0) && (IN_VSIZE != 0)){

        MUINT32 OUT_HSIZE = (RESIZE_MODE == 0) ? (IN_HSIZE >> 1) : IN_HSIZE;
        MUINT32 OUT_VSIZE = (RESIZE_MODE == 2) ? (IN_VSIZE >> 2) : (IN_VSIZE >> 1);
        MUINT32 BLK_WIDTH = (OUT_HSIZE / LTM_BLK_X_NUM) + (OUT_HSIZE % LTM_BLK_X_NUM != 0);
        MUINT32 BLK_HEIGHT = (OUT_VSIZE / LTM_BLK_Y_NUM) + (OUT_VSIZE % LTM_BLK_Y_NUM != 0);

        dreg->LTMSO_R1_LTMSO_XSIZE = LTM_BLK_X_NUM * 20 * 4 - 1; //0x3bf
        dreg->LTMSO_R1_LTMSO_YSIZE = LTM_BLK_Y_NUM + 1 - 1;      //0x9
        dreg->LTMSO_R1_LTMSO_STRIDE = LTM_BLK_X_NUM * 20 * 4;    //0x3c0
        dreg->LTMS_R1_LTMS_OUT_DATA_NUM = LTM_BLK_X_NUM * 20;    //0xf0

        dreg->LTM_R1_LTM_TILE_SIZE = (IN_VSIZE << 16) + IN_HSIZE;
        dreg->LTM_R1_LTM_TILE_EDGE = 0xf;
        dreg->LTM_R1_LTM_TILE_CROP = 0x22;
        dreg->LTMS_R1_LTMS_BLK_NUM = 0x90c;
        dreg->LTMS_R1_LTMS_BLK_SZ = (BLK_HEIGHT << 16) + BLK_WIDTH;
        dreg->LTMS_R1_LTMS_SRAM_CFG = 0x110000;

        //protected
        dreg->LTMS_R1_LTMS_IN_SIZE = (IN_VSIZE << 16) + IN_HSIZE;
        dreg->LTMS_R1_LTMS_OUT_SIZE = (OUT_VSIZE << 16) + OUT_HSIZE;
        dreg->LTMS_R1_LTMS_ACT_WINDOW_X = (OUT_HSIZE - 1) << 16;
        dreg->LTMS_R1_LTMS_ACT_WINDOW_Y = (OUT_VSIZE - 1) << 16;

        SecMgr_INF("LTM_R1_LTM_TILE_SIZE:0x%x",dreg->LTM_R1_LTM_TILE_SIZE);
        SecMgr_INF("LTM_R1_LTM_TILE_EDGE:0x%x",dreg->LTM_R1_LTM_TILE_EDGE);
        SecMgr_INF("LTM_R1_LTM_TILE_CROP:0x%x",dreg->LTM_R1_LTM_TILE_CROP);
        SecMgr_INF("LTMS_R1_LTMS_BLK_NUM:0x%x",dreg->LTMS_R1_LTMS_BLK_NUM);
        SecMgr_INF("LTMS_R1_LTMS_BLK_SZ:0x%x",dreg->LTMS_R1_LTMS_BLK_SZ);
        SecMgr_INF("LTMS_R1_LTMS_IN_SIZE:0x%x",dreg->LTMS_R1_LTMS_IN_SIZE);
        SecMgr_INF("LTMS_R1_LTMS_OUT_SIZE:0x%x",dreg->LTMS_R1_LTMS_OUT_SIZE);
        SecMgr_INF("LTMS_R1_LTMS_ACT_WINDOW_X:0x%x",dreg->LTMS_R1_LTMS_ACT_WINDOW_X);
        SecMgr_INF("LTMS_R1_LTMS_ACT_WINDOW_Y:0x%x",dreg->LTMS_R1_LTMS_ACT_WINDOW_Y);
        SecMgr_INF("LTMS_R1_LTMS_OUT_DATA_NUM:0x%x",dreg->LTMS_R1_LTMS_OUT_DATA_NUM);
        SecMgr_INF("LTMS_R1_LTMS_SRAM_CFG:0x%x",dreg->LTMS_R1_LTMS_SRAM_CFG);
        SecMgr_INF("LTMSO_R1_LTMSO_XSIZE:0x%x",dreg->LTMSO_R1_LTMSO_XSIZE);
        SecMgr_INF("LTMSO_R1_LTMSO_YSIZE:0x%x",dreg->LTMSO_R1_LTMSO_YSIZE);
        SecMgr_INF("LTMSO_R1_LTMSO_STRIDE:0x%x",dreg->LTMSO_R1_LTMSO_STRIDE);

        return MTRUE;
    }else{
        SecMgr_ERR("IN_HSIZE/IN_VSIZE is null");
        return MFALSE;
    }
}

MBOOL SecMgrImp::SecMgr_DumpSecBuffer(MUINT64 HandlePA, void *dst_buf, MUINT32 size)
{
    SecMgr_INF("+ HandlePA(0x%llx) dst(0x%llx) size(0x%x)",HandlePA,dst_buf,size);

    if(!g_isp_ca_dumpsecbuffer(ispHandle,HandlePA,dst_buf,size)){
        SecMgr_ERR("ISP CA dump sec buff failed!");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_attach(MUINT32 tg,MUINT32 sen_idx,vector<MUINT32> sec_port,MUINT32 burstq,MUINT32 tee_type,MUINT64 chk)
{
    SecMgr_INF("+ TG:0x%x SenID:0x%x TEE_Type:0x%x init_chk:0x%llx",tg,sen_idx,tee_type,chk);

    if(burstq > 1){
        SecMgr_ERR("Failed - Secure Cam DO NOT support subsample mode(0x%x)",burstq);
        return MFALSE;
    }

    for(MUINT32 j=0;j<sec_port.size();j++){
        if (MFALSE == this->SecMgr_SecConfig(sec_port.at(j),tg)){
            SecMgr_ERR("Secure DMA port(0x%x) is not supported",sec_port.at(j));
            return NULL;
        }
        this->enable_secport.push_back(sec_port.at(j));
    }

    switch(tg)
    {
        case TG_A:
            this->CamInfo[0].CamModule = CAM_A;
            this->CamInfo[0].SecTG = tg;
            this->CamInfo[0].DevID = sen_idx;
            this->CamInfo[0].chk_handle = chk;

            if(!g_isp_ca_setcaminfo(ispHandle,this->CamInfo[0])){
                SecMgr_ERR("ISP CA set caminfo failed(0x%x)",tg);
                return MFALSE;
            }

            if(MFALSE == this->SecMgr_SetDapcRegAddr(CAM_A)){
                SecMgr_ERR("Set DAPC Reg failure at CAMA");
                return MFALSE;
            }

            SecMgr_DBG("Secure Cam(0x%x) attached",this->CamInfo[0].CamModule);
            break;

        case TG_B:
            this->CamInfo[1].CamModule = CAM_B;
            this->CamInfo[1].SecTG = tg;
            this->CamInfo[1].DevID = sen_idx;
            this->CamInfo[1].chk_handle = chk;

            if(!g_isp_ca_setcaminfo(ispHandle,this->CamInfo[1])){
                SecMgr_ERR("ISP CA set caminfo failed(0x%x)",tg);
                return MFALSE;
            }

            if(MFALSE == this->SecMgr_SetDapcRegAddr(CAM_B)){
                SecMgr_ERR("Set DAPC Reg at failure at CAMB");
                return MFALSE;
            }

            SecMgr_DBG("Secure Cam(0x%x) attached",this->CamInfo[1].CamModule);
            break;

        case TG_C:
            this->CamInfo[2].CamModule = CAM_C;
            this->CamInfo[2].SecTG = tg;
            this->CamInfo[2].DevID = sen_idx;
            this->CamInfo[2].chk_handle = chk;

            if(!g_isp_ca_setcaminfo(ispHandle,this->CamInfo[2])){
                SecMgr_ERR("ISP CA set caminfo failed(0x%x)",tg);
                return MFALSE;
            }

            if(MFALSE == this->SecMgr_SetDapcRegAddr(CAM_C)){
                SecMgr_ERR("Set DAPC Reg at failure at CAMC");
                return MFALSE;
            }

            SecMgr_DBG("Secure Cam(0x%x) attached",this->CamInfo[2].CamModule);
            break;

        default:
            SecMgr_ERR("Unsupport tg type (%d)",tg);
            return MFALSE;
            break;
    }

    switch(tee_type)
    {
        case 1:
            this->tee_mem_type = TEE_PROTECT;
            break;
        case 2:
            this->tee_mem_type = TEE_SECURE;
            break;
        default:
            SecMgr_ERR("Unsupport TEE mem type (%d)",tee_type);
            return MFALSE;
            break;
    }

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_attach_twin(ISP_HW_MODULE module)
{
    MUINT32 master = SEC_CAM_MAX;

    for(MUINT32 i=0;i<SEC_CAM_MAX;i++){
        if(this->CamInfo[i].chk_handle)
            master = i;
    }

    if(master == SEC_CAM_MAX){
        SecMgr_ERR("Master secure cam is not attached");
        return MFALSE;
    }

    SecMgr_INF("+ Sec Master :0x%x Slave:0x%x",master,module);

    switch(module)
    {
        case CAM_A:
            this->CamInfo[master].TwinCamModule = CAM_A;

            if(MFALSE == this->SecMgr_SetDapcRegAddr(CAM_A)){
                SecMgr_ERR("Set DAPC Reg addr failure at Slave CAMA");
                return MFALSE;
            }

            SecMgr_INF("Secure twin cam(0x%x) attached",this->CamInfo[master].TwinCamModule);
            break;
        case CAM_B:
            this->CamInfo[master].TwinCamModule = CAM_B;

            if(MFALSE == this->SecMgr_SetDapcRegAddr(CAM_B)){
                SecMgr_ERR("Set DAPC Reg addr failure at Slave CAMB");
                return MFALSE;
            }

            SecMgr_INF("Secure twin cam(0x%x) attached",this->CamInfo[master].TwinCamModule);
            break;
        default:
            //Only CAM_A or CAM_B can be slave cam
            SecMgr_ERR("Secure twin cam(0x%x) attached failure",module);
            return MFALSE;
            break;
    }
    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_detach(MUINT32 tg)
{
    SecMgr_INF("+ SenIdx:0x%x",tg);

    if(this->Num_SecCam == 1){
        if(!g_isp_ca_uninit(ispHandle)){
            SecMgr_ERR("ISP CA unint failed!");
            return MFALSE;
        }

        if(!g_isp_ca_close(ispHandle)){
            SecMgr_ERR("ISP CA close failed!");
            return MFALSE;
        }

        g_isp_ca_handlecreate     = nullptr;
        g_isp_ca_open             = nullptr;
        g_isp_ca_close            = nullptr;
        g_isp_ca_init             = nullptr;
        g_isp_ca_uninit           = nullptr;
        g_isp_ca_config           = nullptr;
        g_isp_ca_querypa          = nullptr;
        g_isp_ca_queryfhpa        = nullptr;
        g_isp_ca_queryinfo        = nullptr;
        g_isp_ca_registersharemem = nullptr;
        g_isp_ca_migratetable     = nullptr;
        g_isp_ca_dumpsecbuffer    = nullptr;
        g_isp_ca_setcaminfo       = nullptr;
        g_isp_ca_setdapcreg       = nullptr;

        memset(&SecHwInfo[0], 0, sizeof(SecMgr_QueryInfo));
        memset(&SecHwInfo[1], 0, sizeof(SecMgr_QueryInfo));
        memset(&this->mSemSecDone, 0, sizeof(sem_t));

        this->Num_SecCam = 0;
        this->tee_mem_type = TEE_MAX;
        this->buf_sec_mva.clear();
        this->buf_sec_mva_idx.clear();
        this->lsc_buf_sec_mva.clear();
        this->lsc_buff_idx.clear();
        this->bpc_buf_sec_mva.clear();
        this->bpc_buff_idx.clear();
        this->enable_secport.clear();
        this->pre_reg_status = SET_NONE;
        this->mInitState = SEC_STATE_NONE;

        for(MUINT32 i=0;i<SEC_CAM_MAX;i++){
            memset(&CamInfo[i], 0, sizeof(SecMgr_CamInfo));
            memset(&regval[i],0, sizeof(SecMgr_RegInfo));
            this->CamInfo[i].CamModule = PHY_CAM;
            this->CamInfo[i].TwinCamModule = PHY_CAM;
            this->m_pDrv[i] = NULL;
            this->m_pIspDrv[i] = NULL;
            this->mState[i] = SEC_STATE_NONE;

            for(MUINT32 j=0;j<MAX_DMA_PORT_NUM;j++){
                twin_buf_sec_mva[j].clear();
                twin_buff_idx[j].clear();
                for(MUINT32 k=0;k<IMG_HEADER_BUF_NUM;k++){
                    fh_sec_mva[i][j][k] = 0;
                    fh_sec_mva_idx[i][j][k] = 0;
                }
            }
        }

        ispHandle = NULL;
        if(m_isp_ca){
            dlclose(m_isp_ca);
           m_isp_ca = NULL;
        }

    }else if(this->Num_SecCam > 1){
        this->Num_SecCam--;
    }else{
        SecMgr_ERR("SecMgr detach failed:Secure Cam (0x%x) is not enabled before",tg);
        return MFALSE;
    }

    return MTRUE;
}

MUINT32 SecMgr::SecMgr_GetSecureCamStatus(ISP_HW_MODULE module)
{
    return gSecMgr.SecMgr_GetSecureCamStatus(module);
}

MBOOL SecMgr::SecMgr_GetSecurePortStatus(ISP_HW_MODULE module,MUINT32 port)
{
    return gSecMgr.SecMgr_GetSecurePortStatus(module,port);
}


MUINT32 SecMgrImp::SecMgr_GetSecureCamStatus(ISP_HW_MODULE module)
{
    SecMgr_DBG("+ (0x%x)",ispHandle);
    MUINT32 lock = 0;

    if(this->Num_SecCam > 0 && this->mInitState == SEC_STATE_NONE){
        SecMgr_INF("Wait Secure init ready");
        ::sem_wait(&this->mSemSecDone);
    }

    switch(module){
        default:
            //in 50, once if security is enabled, all cams' cq need to be locked
            if(this->Num_SecCam > 0)
               lock = 1;
            break;
    }
    return lock;
}

MBOOL SecMgrImp::SecMgr_GetSecureTwinStatus(ISP_HW_MODULE module)
{
    SecMgr_INF("+ (0x%x)module:0x%x",ispHandle,module);

    if((this->CamInfo[0].TwinCamModule == module) || (this->CamInfo[1].TwinCamModule == module))
        return MTRUE;
    else
        return MFALSE;
}

MBOOL SecMgrImp::SecMgr_GetSecurePortStatus(ISP_HW_MODULE module,MUINT32 port)
{
    MUINT32 dma_port = 0;

    if(this->Num_SecCam > 0){
        if((this->CamInfo[0].CamModule == module) || (this->CamInfo[1].CamModule == module) ||
           (this->CamInfo[2].CamModule == module) || (this->CamInfo[0].TwinCamModule == module) ||
           (this->CamInfo[1].TwinCamModule == module)){
            SecMgr_INF("+ (0x%x)module:0x%x dma port:%s",ispHandle,module,str_dmao(port));
            switch(port)
            {
                case _imgo_:
                    dma_port = PORT_IMGO.index;
                    break;
                case _rrzo_:
                    dma_port = PORT_RRZO.index;
                    break;
                case _crzo_:
                    dma_port = PORT_CRZO_R1.index;
                    break;
                default:
                    SecMgr_DBG("UnSupported DMA port (0x%x)",port);
                    return MFALSE;
                    break;
            }

            if(this->SecHwInfo[0].CAM_CTL_DMA_EN & PortMap(dma_port)){
                for(MUINT32 j=0;j<this->enable_secport.size();j++){
                   if(this->enable_secport.at(j) == dma_port)
                        return MTRUE;
                }
                SecMgr_INF("(0x%x)Secure DMA port (0x%x) is not enabled",module, port);
                return MFALSE;
            }else{
                SecMgr_INF("(0x%x)DMA port (0x%x) is not supported",module, port);
                return MFALSE;
            }
        }else{
            SecMgr_ERR("Secure Cam (0x%x) is not attached correctly",module);
            return MFALSE;
        }
    }else{
        SecMgr_DBG("Secure Cam (0x%x) is not enabled",module);
        return MFALSE;
    }

        return MTRUE;
}

MBOOL SecMgrImp::SecMgr_SecCamEnable()
{
    this->Num_SecCam++;
    if(this->Num_SecCam == 1)
        ::sem_init(&this->mSemSecDone, 0, 0);

    SecMgr_INF("Secure Cam is enabled(0x%x)",this->Num_SecCam);
    return MTRUE;
}

SecMgr* SecMgr::SecMgr_Init()
{
    MINT32 ret = 0;

    SecMgr_DBG("+ (0x%x)",ispHandle);

    if(gSecMgr.SecMgr_GetSecureCamStatus(CAM_A) == 0){
        gSecMgr.SecMgr_SecCamEnable();

        if(MFALSE == gSecMgr.SecMgr_LoadCA()){
            SecMgr_ERR("Load CA library failure");
            return NULL;
        }

        if(MFALSE == gSecMgr.SecMgr_OpenCA()){
            SecMgr_ERR("Open CA session failure");
            return NULL;
        }

        if(MFALSE == gSecMgr.SecMgr_InitCA()){
            SecMgr_ERR("Init CA failure");
            return NULL;
        }


        if(MFALSE == gSecMgr.SecMgr_GetHWCapability()){
            SecMgr_ERR("Get secure constraints failure");
            return NULL;
        }

    }else{
         gSecMgr.SecMgr_SecCamEnable();
    }

    return (SecMgr*)&gSecMgr;
}

SecMgr* SecMgr::SecMgr_GetMgrObj()
{
    SecMgr_DBG("+ (0x%x)",ispHandle);
    SecMgr_DBG("gSecMgrf:0x%x",&gSecMgr);
    return (SecMgr*)&gSecMgr;
}

MBOOL SecMgrImp::SecMgr_SetDapcReg(ISP_HW_MODULE module)
{
    SecMgr_INF("+ module:0x%x",module);
    vector<MUINT32> reg_cq;

    reg_cq = this->m_pDrv[module]->getDapcReg(module);
    if(reg_cq.size() != DAPC_NUM_CQ){
        SecMgr_ERR("The number of cq dapc reg(%d) is wrong(%d)",reg_cq.size(),DAPC_NUM_CQ);
        return MFALSE;
    }else{
        for(MUINT32 i=0;i<reg_cq.size();i++){
            regval[module].dapc_cq[i] = reg_cq.at(i);
            if(regval[module].dapc_cq[i])
                SecMgr_INF("Addr:0x%x Data:0x%x",this->SecHwInfo[0].SecReg_ADDR_CQ[i],regval[module].dapc_cq[i]);
        }
    }

    if(!g_isp_ca_setdapcreg(ispHandle,&regval[module],(MUINT32)module)){
        SecMgr_ERR("ISP CA set DAPC reg failed");
        return MFALSE;
    }

    this->mState[module] = SEC_STATE_IDLE;
    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_start()
{
    SecMgr_INF("+");

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_stop()
{
    SecMgr_INF("+");

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_CheckSecReg(ISP_HW_MODULE module)
{
    SecMgr_DBG("+");
    vector<MUINT32> reg_cq;

    if(this->mState[module] == SEC_STATE_IDLE){
        reg_cq = this->m_pDrv[module]->getDapcReg(module);
        if (reg_cq.size() != DAPC_NUM_CQ){
            SecMgr_ERR("The number of cq dapc reg(%d) is wrong(%d)",reg_cq.size(),DAPC_NUM_CQ);
            return MFALSE;
        }else{
            for(MUINT32 i=0;i<reg_cq.size();i++){
                if((reg_cq.at(i) != 0x0) && (regval[module].dapc_cq[i] != reg_cq.at(i)))
                    SecMgr_ERR("[0x%x][CQ]2nd DAPC Reg value change, add:0x%x value:0x%x - 0x%x",module,
                                this->SecHwInfo[0].SecReg_ADDR_CQ[i],regval[module].dapc_cq[i],reg_cq.at(i));
            }
        }
    }

    return MTRUE;
}

