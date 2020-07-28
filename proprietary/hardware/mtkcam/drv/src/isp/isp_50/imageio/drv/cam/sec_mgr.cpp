#define LOG_TAG "SecMgr"

#include "sec_mgr.h"
#include "isp_function_cam.h"

//#include <unistd.h> //for usleep
#include <stdlib.h>
#include <dlfcn.h>

//#include <utils/Vector.h>
//#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//#include <core/iopipe/CamIO/PortMap.h>
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

//#if 0 //if TEE is not ready
#define MTK_SEC_ISP_LIB            "libispcameraca.so"
#define MTK_SEC_ISP_CREATE         "MtkSecISP_tlcHandleCreate"
#define MTK_SEC_ISP_OPEN           "MtkSecISP_tlcOpen"
#define MTK_SEC_ISP_CLOSE          "MtkSecISP_tlcClose"
#define MTK_SEC_ISP_INIT           "MtkSecISP_tlcInit"
#define MTK_SEC_ISP_UNINIT         "MtkSecISP_tlcUnInit"
#define MTK_SEC_ISP_CONFIG         "MtkSecISP_tlcSecConfig"
#define MTK_SEC_ISP_QUERYPA        "MtkSecISP_tlcQueryMVAFromHandle"  //MtkSecISP_tlcQueryPAFromHandle for debug
#define MTK_SEC_ISP_QUERYFHPA      "MtkSecISP_tlcQueryFHPA"
#define MTK_SEC_ISP_QUERYHWINFO    "MtkSecISP_tlcQueryHWInfo"
#define MTK_SEC_ISP_REGISTER_MEM   "MtkSecISP_tlcRegisterShareMem"
#define MTK_SEC_ISP_MIGRATE_TABLE  "MtkSecISP_tlcMigrateTable"
#define MTK_SEC_ISP_DUMP_SECBUFFER "MtkSecISP_tlcDumpSecmem"
#define MTK_SEC_ISP_SETSECCAM      "MtkSecISP_tlcSetSecCam"

typedef void *ca_create_ptr();
typedef int ca_open_ptr(void*);
typedef int ca_close_ptr(void*);
typedef int ca_init_ptr(void*);
typedef int ca_uninit_ptr(void*);
typedef int ca_config_ptr(void*,MINT32,MINT32);
typedef int ca_querypa_ptr(void*,MUINT64,MUINT32*,TEE_MEM_TYPE);
typedef int ca_queryfhpa_ptr(void*,SecMgr_SecInfo,MUINT32*);
typedef int ca_queryinfo_ptr(void*,SecMgr_QueryInfo*);
typedef int ca_registersharemem_ptr(void*,SecMgr_SecInfo*);
typedef int ca_migratetable_ptr(void*,SecMgr_SecInfo*);
typedef int ca_dumpsecbuffer_ptr(void*,MUINT64,void*,MUINT32);
typedef int ca_setcaminfo_ptr(void*,SecMgr_CamInfo);

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
//#endif


//DMA port support
#define MAX_DMA_PORT_NUM 2

PortID support_dma_port[MAX_DMA_PORT_NUM]=
{
    PORT_RRZO,
    PORT_IMGO,
    //PORT_UFO,
    //PORT_RRZO_D,
    //PORT_IMGO_D,
    //PORT_CAMSV_IMGO,
    //PORT_CAMSV2_IMGO
};

//#define PORT_RRZO_D 17
//#define PORT_IMGO_D 16
//#define PORT_RRZO   15
//#define PORT_UFO    14
//#define PORT_IMGO   13

#define PortMap(PortIdx) ({\
    static MUINT32 _idx = 0;\
    if (PortIdx == PORT_IMGO.index)\
        _idx = ((unsigned int)1 << 0);\
    else if (PortIdx == PORT_RRZO.index)\
        _idx = ((unsigned int)1 << 2);\
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
   }\
   _str_dmao;\
})

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

    virtual MUINT32 SecMgr_GetSecureCamStatus(ISP_HW_MODULE module); //For CQ protection

    virtual MBOOL SecMgr_GetSecurePortStatus(ISP_HW_MODULE module,MUINT32 port); //For frame header protection

    virtual MBOOL SecMgr_SetDapcReg();

    virtual MUINT32 SecMgr_GetDummySecMVA(MUINT32 dup);
private:
    SecMgr_QueryInfo SecHwInfo[2];//0 for secure constraint; 1 for non-secure constraint

    ISP_HW_MODULE mModule[2]; //which CAM initialize secure CAM

    SecMgr_CamInfo CamInfo[2];

    IspDrvImp* m_pIspDrv;

    static MUINT32 Num_SecCam;

    MUINT32 dummy_sec_mva[3];

    MUINT32 fh_sec_mva[MAX_DMA_PORT_NUM][16];

    uint64_t fh_sec_mva_idx[MAX_DMA_PORT_NUM][16];

    vector<MUINT32> buf_sec_mva;

    vector<uint64_t> buf_sec_mva_idx;

    vector<MUINT32> lsc_buf_sec_mva;

    vector<uint64_t> lsc_buff_idx;  //lsc buff's VA

    vector<MUINT32> bpc_buf_sec_mva;

    vector<uint64_t> bpc_buff_idx;  //bpc buff's VA

    TEE_MEM_TYPE tee_mem_type;

    vector<MUINT32> enable_secport;

    mutable Mutex m_lock;  // for multi-thread
};

static SecMgrImp gSecMgr;
static void* ispHandle;
static void* m_isp_ca;

MUINT32 SecMgrImp::Num_SecCam = 0;

SecMgrImp::SecMgrImp()
{
    memset(&SecHwInfo[0], 0, sizeof(SecMgr_QueryInfo));
    memset(&SecHwInfo[1], 0, sizeof(SecMgr_QueryInfo));
    memset(&CamInfo[0], 0, sizeof(SecMgr_CamInfo));
    memset(&CamInfo[1], 0, sizeof(SecMgr_CamInfo));
    this->mModule[0] = CAM_MAX;
    this->mModule[1] = CAM_MAX;
    this->m_pIspDrv = NULL;
    this->dummy_sec_mva[0] = 0;
    this->dummy_sec_mva[1] = 0;
    this->dummy_sec_mva[2] = 0;

    this->tee_mem_type = TEE_MAX;

    this->buf_sec_mva.clear();
    this->buf_sec_mva_idx.clear();
    this->lsc_buf_sec_mva.clear();
    this->lsc_buff_idx.clear();
    this->bpc_buf_sec_mva.clear();
    this->bpc_buff_idx.clear();
    this->enable_secport.clear();

    for(MUINT32 i=0;i<MAX_DMA_PORT_NUM;i++){
        for(MUINT32 j=0;j<16;j++){
            fh_sec_mva[i][j] = 0;
            fh_sec_mva_idx[i][j] = 0;
        }
    }

}


MBOOL SecMgrImp::SecMgr_sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int ret = 0; //0 for ok; -1 for fail
    SecMgr_INF("+ (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", cmd, arg1, arg2, arg3);
    switch ( cmd ) {
        case SECMGRCmd_QUERY_AF_BLK2_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].AF_BLK2_XSIZE;
                *(MUINT32*)arg2 = this->SecHwInfo[0].AF_BLK2_YSIZE;
                break;
            }
        case SECMGRCmd_QUERY_AWB_WIN_SIZE_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].AWB_W_HSIZE;
                *(MUINT32*)arg2 = this->SecHwInfo[0].AWB_W_VSIZE;
                break;
            }
        case SECMGRCmd_QUERY_AWB_WIN_PIT_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].AWB_W_HPIT;
                *(MUINT32*)arg2 = this->SecHwInfo[0].AWB_W_VPIT;
                break;
            }

        case SECMGRCmd_QUERY_PS_AWB_WIN_SIZE_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].PS_AWB_W_HSIZE;
                *(MUINT32*)arg2 = this->SecHwInfo[0].PS_AWB_W_VSIZE;
                break;
            }
        case SECMGRCmd_QUERY_PS_AWB_WIN_PIT_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].PS_AWB_W_HPIT;
                *(MUINT32*)arg2 = this->SecHwInfo[0].PS_AWB_W_VPIT;
                break;
            }

        case SECMGRCmd_QUERY_FLK_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].FLK_SIZE_X;
                *(MUINT32*)arg2 = this->SecHwInfo[0].FLK_SIZE_Y;
                break;
            }
        case SECMGRCmd_QUERY_MODULE_EN_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_EN;
                break;
            }
        case SECMGRCmd_QUERY_DMA_EN_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_DMA_EN;
                break;
            }
        case SECMGRCmd_QUERY_MODULE_EN2_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_EN2;
                break;
            }
        case SECMGRCmd_QUERY_SEL_CONSTRAINT:
            {
                *(MUINT32*)arg1 = this->SecHwInfo[0].CAM_CTL_SEL;
                break;
            }
        case SECMGRCmd_QUERY_NUM_CAM_CONSTRAINT:
            {
                 *(MUINT32*)arg1 = this->SecHwInfo[0].Num_of_Cam;
                 break;
            }
        case SECMGRCmd_QUERY_CAP_CONSTRAINT:
            {
                 *(MBOOL*)arg1 = MFALSE;
                 break;
            }
        default:
            {
                SecMgr_ERR("Not support SecMgr command!");
                ret = -1;
                break;
            }
    }

    if( ret != 0)
    {
        SecMgr_ERR("sendCommand(0x%x) error!",cmd);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_LoadCA()
{
    SecMgr_INF("+");
//#if 0 //if TEE is not ready for
    MBOOL ret = MTRUE;
    m_isp_ca = dlopen(MTK_SEC_ISP_LIB, RTLD_NOW);

    if(nullptr == m_isp_ca)
    {
        SecMgr_ERR("ISP CA LIB open failed (%s)",dlerror());
        return MFALSE;
    }

    SecMgr_INF("ISP CA LIB open success");
    if (nullptr != m_isp_ca)
    {
        if (nullptr == g_isp_ca_handlecreate)
        {
            g_isp_ca_handlecreate = (ca_create_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_CREATE);
            if (nullptr == g_isp_ca_handlecreate)
            {
                SecMgr_ERR("Load CA create handle failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_open)
        {
            g_isp_ca_open = (ca_open_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_OPEN);
            if (nullptr == g_isp_ca_open)
            {
                SecMgr_ERR("Load CA open failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_close)
        {
            g_isp_ca_close = (ca_close_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_CLOSE);
            if (nullptr == g_isp_ca_close)
            {
                SecMgr_ERR("Load CA close failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_init)
        {
            g_isp_ca_init = (ca_init_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_INIT);
            if (nullptr == g_isp_ca_init)
            {
                SecMgr_ERR("Load CA init failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_uninit)
        {
            g_isp_ca_uninit = (ca_uninit_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_UNINIT);
            if (nullptr == g_isp_ca_uninit)
            {
                SecMgr_ERR("Load CA uninit failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_config)
        {
            g_isp_ca_config = (ca_config_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_CONFIG);
            if (nullptr == g_isp_ca_config)
            {
                SecMgr_ERR("Load CA config failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_querypa)
        {
            g_isp_ca_querypa = (ca_querypa_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_QUERYPA);
            if (nullptr == g_isp_ca_querypa)
            {
                SecMgr_ERR("Load CA querypa failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_queryfhpa)
        {
            g_isp_ca_queryfhpa = (ca_queryfhpa_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_QUERYFHPA);
            if (nullptr == g_isp_ca_queryfhpa)
            {
                SecMgr_ERR("Load CA querypa failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_queryinfo)
        {
            g_isp_ca_queryinfo = (ca_queryinfo_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_QUERYHWINFO);
            if (nullptr == g_isp_ca_queryinfo)
            {
                SecMgr_ERR("Load CA queryinfo failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_registersharemem)
        {
            g_isp_ca_registersharemem = (ca_registersharemem_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_REGISTER_MEM);
            if (nullptr == g_isp_ca_registersharemem)
            {
                SecMgr_ERR("Load CA register share mem failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_migratetable)
        {
            g_isp_ca_migratetable = (ca_migratetable_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_MIGRATE_TABLE);
            if (nullptr == g_isp_ca_migratetable)
            {
                SecMgr_ERR("Load CA migrate table failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_dumpsecbuffer)
        {
            g_isp_ca_dumpsecbuffer = (ca_dumpsecbuffer_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_DUMP_SECBUFFER);
            if (nullptr == g_isp_ca_dumpsecbuffer)
            {
                SecMgr_ERR("Load CA dump secbuffer failed!");
                ret = MFALSE;
                goto Exit;
            }
        }

        if (nullptr == g_isp_ca_setcaminfo)
        {
            g_isp_ca_setcaminfo = (ca_setcaminfo_ptr*) dlsym(m_isp_ca, MTK_SEC_ISP_SETSECCAM);
            if (nullptr == g_isp_ca_setcaminfo)
            {
                SecMgr_ERR("Load CA set caminfo failed!");
                ret = MFALSE;
                goto Exit;
            }
        }
    }

//#endif
Exit:
    if (ret == MFALSE)
    {
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
    SecMgr_INF("SecMgr_OpenCA+ (0x%x)",ispHandle);
    if (!g_isp_ca_open(ispHandle))
    {
        SecMgr_ERR("ISP CA open failed!");
        return MFALSE;
    }
    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_InitCA()
{
    SecMgr_INF("SecMgr_InitCA+ (0x%x)",ispHandle);
    if (!g_isp_ca_init(ispHandle))
    {
        SecMgr_ERR("ISP CA init failed!");
        return MFALSE;
    }
    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_QuerySecMVAFromHandle(MUINT64 HandlePA,MUINT32* SecMemPA)
{
    SecMgr_INF("SecMgr_QuerySecMVAFromHandle+ HandlePA:0x%lx",(MUINT32)HandlePA);

    if(this->buf_sec_mva.size() < 1){
        if (!g_isp_ca_querypa(ispHandle, HandlePA, SecMemPA, this->tee_mem_type))
        {
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

         if(find == MFALSE) {
             if (!g_isp_ca_querypa(ispHandle, HandlePA, SecMemPA, this->tee_mem_type))
             {
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
    SecMgr_INF("+");

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
        default:
            SecMgr_ERR("Unsupport port type (%d)",secinfo.port);
            return MFALSE;
            break;
    }

    for(MUINT32 i=0; i < 16; i++)
    {
        if(this->fh_sec_mva_idx[_port][i] == sinfo.buff_va)
            *SecMemPA = this->fh_sec_mva[_port][i];
    }
#if 0
    if (!g_isp_ca_queryfhpa(ispHandle, sinfo, SecMemPA))
    {
        SecMgr_ERR("Query secmem FH PA failed!");
        return MFALSE;
    }
#endif
    SecMgr_INF("DMA port:%s FH_SecMemPA:0x%x",str_dmao(secinfo.port),*SecMemPA);

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_GetHWCapability()
{
    SecMgr_INF("+ (0x%x)",ispHandle);

    this->SecHwInfo[0].SecOn = 1;
    if (!g_isp_ca_queryinfo(ispHandle,&this->SecHwInfo[0]))
    {
        SecMgr_ERR("ISP CA get secure HW capability failed!");
        return MFALSE;
    }

    this->SecHwInfo[1].SecOn = 0;
    if (!g_isp_ca_queryinfo(ispHandle,&this->SecHwInfo[1]))
    {
        SecMgr_ERR("ISP CA get non-secure HW capability failed!");
        return MFALSE;
    }
    SecMgr_INF("SecHwInfo[0].Num_of_Cam:0x%x",this->SecHwInfo[0].Num_of_Cam);
    SecMgr_INF("SecHwInfo[0].CAM_CTL_DMA_EN:0x%x",this->SecHwInfo[0].CAM_CTL_DMA_EN);
    SecMgr_INF("SecHwInfo[0].CAM_CTL_EN:0x%x",this->SecHwInfo[0].CAM_CTL_EN);
    SecMgr_INF("SecHwInfo[0].CAM_CTL_EN2:0x%x",this->SecHwInfo[0].CAM_CTL_EN2);
    SecMgr_INF("SecHwInfo[0].CAM_CTL_SEL:0x%x",this->SecHwInfo[0].CAM_CTL_SEL);
    for(MUINT i=0;i<10;i++)
        SecMgr_INF("SecReg[%d]:0x%x",i,this->SecHwInfo[0].SecReg[i]);
#if 0
    this->SecHwInfo[0].CAM_CTL_DMA_EN = 0x00000005; //Constraint of secure DMA port
    this->SecHwInfo[0].CAM_CTL_EN     = 0x0;
    this->SecHwInfo[0].CAM_CTL_EN2    = 0x0;
    this->SecHwInfo[0].CAM_CTL_SEL    = 0x0;
    this->SecHwInfo[1].CAM_CTL_DMA_EN = 0x00000000; //Constraint of non-secure DMA port

    this->SecHwInfo[0].SecReg[0] = 0x004;
    this->SecHwInfo[0].SecReg[1] = 0x008;
    this->SecHwInfo[0].SecReg[2] = 0x010;
    this->SecHwInfo[0].SecReg[3] = 0x018;
    this->SecHwInfo[0].SecReg[4] = 0x6C8;
    this->SecHwInfo[0].SecReg[5] = 0x834;
    this->SecHwInfo[0].SecReg[6] = 0x838;
    this->SecHwInfo[0].SecReg[7] = 0xBD4;
    this->SecHwInfo[0].SecReg[8] = 0xBD8;
    this->SecHwInfo[0].SecReg[9] = 0xC58;
#endif
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
   MUINT32 _port = 0;

   switch(secinfo->type)
   {
       case SECMEM_CQ_DESCRIPTOR_TABLE:
       case SECMEM_VIRTUAL_REG_TABLE:
            SecMgr_INF("+ module:0x%x cq:%d dup:%d dmy:%d type:%s buff_va:0x%llx size:0x%x memID:0x%x",
            secinfo->module,secinfo->cq,secinfo->dupq,secinfo->dummy,str(secinfo->type),secinfo->buff_va,secinfo->buff_size,secinfo->memID);
            SecMgr_INF("+ module:0x%x cq:%d dup:%d dmy:%d sub-type:%s buff_va:0x%llx size:0x%x memID:0x%x",
            secinfo->module,secinfo->cq,secinfo->dupq,secinfo->dummy,str(secinfo->sub.type),secinfo->sub.buff_va,secinfo->sub.buff_size,secinfo->sub.memID);

            if (!g_isp_ca_registersharemem(ispHandle,secinfo))
            {
                SecMgr_ERR("Share mem register failed (module:0x%x type:%s buff:0x%llx size:0x%x)",
                            secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                return MFALSE;
            }
            break;
       case SECMEM_FRAME_HEADER:
            SecMgr_INF("+ module:0x%x type:%s DMA port:%d",secinfo->module,str(secinfo->type),secinfo->port);
            if (!g_isp_ca_registersharemem(ispHandle,secinfo))
            {
                SecMgr_ERR("Share mem register failed (module:0x%x type:%s buff:0x%llx size:0x%x)",
                            secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                return MFALSE;
            }

            if (secinfo->port == PORT_IMGO.index){
                _port = 0;
            }else if (secinfo->port == PORT_RRZO.index){
                _port = 1;
            }else {
                SecMgr_ERR("Unsupport port type (%d)",secinfo->port);
                return MFALSE;
            }

            this->fh_sec_mva[_port][fh_idx] = secinfo->buff_sec_mva;
            this->fh_sec_mva_idx[_port][fh_idx] = secinfo->buff_va;
            SecMgr_INF("(%s_%d)FH va:0x%llx sec_mva:0x%x",str_dmao(_port),fh_idx,this->fh_sec_mva_idx[_port][fh_idx],this->fh_sec_mva[_port][fh_idx]);
            fh_idx++;
            if (fh_idx == 16)
                fh_idx = 0;
            break;
       case SECTABLE_LSC:
            SecMgr_INF("+ module:0x%x type:%s buff:0x%llx size:0x%x memID:0x%x",
              secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size,secinfo->memID);

            if(this->lsc_buff_idx.size() < 1){
                if (!g_isp_ca_registersharemem(ispHandle,secinfo))
                {
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

                if(find == MFALSE) {
                    if (!g_isp_ca_registersharemem(ispHandle,secinfo))
                    {
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
                if (!g_isp_ca_registersharemem(ispHandle,secinfo))
                {
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

                if(find == MFALSE) {
                    if (!g_isp_ca_registersharemem(ispHandle,secinfo))
                    {
                        SecMgr_ERR("Share mem register failed (module:0x%x type:%s buff:0x%llx size:0x%x)",
                                secinfo->module,str(secinfo->type),secinfo->buff_va,secinfo->buff_size);
                        return MFALSE;
                    }

                    this->bpc_buff_idx.push_back(secinfo->buff_va);
                    this->bpc_buf_sec_mva.push_back(secinfo->buff_sec_mva);
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


MUINT32 SecMgrImp::SecMgr_GetDummySecMVA(MUINT32 dup)
{
    SecMgr_INF("+ dup:%d sec_mva:0x%x",dup,dummy_sec_mva[dup]);
#if 0
    switch(dup)
    {
        case 0:
            return this->dummy_sec_mva[2];
            break;
        case 1:
            return this->dummy_sec_mva[0];
            break;
        case 2:
            return this->dummy_sec_mva[1];
            break;
        default:
            SecMgr_INF("Wrong dup");
            break;
    }
#endif
    return this->dummy_sec_mva[dup];
}

MBOOL SecMgrImp::SecMgr_P1MigrateTable(SecMgr_SecInfo* secinfo)
{
   SecMgr_INF("+ module:0x%x type:%s",secinfo->module,str(secinfo->type));
 
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
            SecMgr_INF("cq:%d dup:%d dummy:%d sub-type:%s",
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
                default:
                    SecMgr_ERR("Unsupport port type (%d)",secinfo->port);
                    return MFALSE;
                    break;
            }

            SecMgr_INF("DMA port:%s buff:0x%llx",str_dmao(secinfo->port),secinfo->buff_va);
            break;
       case SECTABLE_LSC:
       case SECTABLE_BPC:
            sinfo.buff_va = secinfo->buff_va;
            break;
   }

   if (!g_isp_ca_migratetable(ispHandle,&sinfo))
   {
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

    if (this->SecHwInfo[0].CAM_CTL_DMA_EN & PortMap(dma_port))
    {
        if (dma_port == PORT_RRZO.index)
            _port = CAM_PORT_RRZO;
        else if (dma_port == PORT_IMGO.index)
            _port = CAM_PORT_IMGO;

        if (!g_isp_ca_config(ispHandle,_port,tg_idx))
        {
            SecMgr_ERR("ISP CA Config DMA Port(%d) failed!",dma_port);
            return MFALSE;
        }
    }
    else {
        SecMgr_INF("Not support DMA port:(%d)",dma_port);
        return MFALSE;
    }

    return MTRUE;
}

vector<PortID> SecMgrImp::SecMgr_GetSupportPort(MBOOL SecEnable)
{
    vector<PortID> result;
    //SecMgr_INF("Run CheckPort, DMA port:%d(Constraint:0x%x check:0x%x)",dma_port,this->SecHwInfo[0].CAM_CTL_DMA_EN,PortMap(dma_port));
    if(SecEnable){
        for(int i=0;i<MAX_DMA_PORT_NUM;i++)
        {
            if(this->SecHwInfo[0].CAM_CTL_DMA_EN & PortMap(support_dma_port[i].index))
                result.push_back(support_dma_port[i]);
        }
    }else{
        for(int i=0;i<MAX_DMA_PORT_NUM;i++)
        {
            if(this->SecHwInfo[1].CAM_CTL_DMA_EN & PortMap(support_dma_port[i].index))
                result.push_back(support_dma_port[i]);
        }
    }
    return result;
}

MBOOL SecMgrImp::SecMgr_SetDapcReg()
{
    SecMgr_INF("+");

    vector<MUINT32> reg_list;
    this->m_pIspDrv = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);
    //this->m_pIspDrv->init("Hwsync");

    for(MUINT32 i=0;i<DAPC_NUM;i++)
        reg_list.push_back(this->SecHwInfo[0].SecReg[i]);

    this->m_pIspDrv->setDapcReg(reg_list);
    //this->m_pIspDrv->uninit("Hwsync");
    //this->m_pIspDrv->destroyInstance();

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_DumpSecBuffer(MUINT64 HandlePA, void *dst_buf, MUINT32 size)
{
    SecMgr_INF("+ HandlePA(0x%llx) dst(0x%llx) size(0x%x)",HandlePA,dst_buf,size);

    if (!g_isp_ca_dumpsecbuffer(ispHandle,HandlePA,dst_buf,size))
    {
        SecMgr_ERR("ISP CA dump sec buff failed!");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_attach(MUINT32 tg,MUINT32 sen_idx,vector<MUINT32> sec_port,MUINT32 burstq,MUINT32 tee_type,MUINT64 chk)
{
    SecMgr_INF("+ TG:0x%x SenID:0x%x TEE_Type:0x%x chk:0x%llx",tg,sen_idx,tee_type,chk);

    if(burstq > 1)
    {
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
        case 0:
            this->CamInfo[0].CamModule = SEC_CAM_A;
            this->CamInfo[0].TwinCamModule = SEC_CAM_MAX;
            this->CamInfo[0].SecTG = tg;
            this->CamInfo[0].DevID = sen_idx;
            this->CamInfo[0].chk_handle = chk;

            if (!g_isp_ca_setcaminfo(ispHandle,this->CamInfo[0]))
            {
                SecMgr_ERR("ISP CA set caminfo failed(0x%x)",tg);
                return MFALSE;
            }

            this->Num_SecCam++;
            SecMgr_INF("Secure Cam(0x%x) attached(0x%x)",this->CamInfo[0].CamModule,this->Num_SecCam);
            break;

        case 1:
            this->CamInfo[1].CamModule = SEC_CAM_B;
            this->CamInfo[1].TwinCamModule = SEC_CAM_MAX;
            this->CamInfo[1].SecTG = tg;
            this->CamInfo[1].DevID = sen_idx;
            this->CamInfo[1].chk_handle = chk;

            if (!g_isp_ca_setcaminfo(ispHandle,this->CamInfo[1]))
            {
                SecMgr_ERR("ISP CA set caminfo failed(0x%x)",tg);
                return MFALSE;
            }

            this->Num_SecCam++;
            SecMgr_INF("Secure Cam(0x%x) attached(0x%x)",this->CamInfo[1].CamModule,this->Num_SecCam);
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

    if (MFALSE == this->SecMgr_SetDapcReg()){
        SecMgr_ERR("Set DAPC Reg failure");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL SecMgrImp::SecMgr_detach(MUINT32 tg)
{
    SecMgr_INF("+ SenIdx:0x%x",tg);

    if(this->Num_SecCam == 1){
        if (!g_isp_ca_uninit(ispHandle))
        {
            SecMgr_ERR("ISP CA unint failed!");
            return MFALSE;
        }

        if (!g_isp_ca_close(ispHandle))
        {
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

        memset(&SecHwInfo[0], 0, sizeof(SecMgr_QueryInfo));
        memset(&SecHwInfo[1], 0, sizeof(SecMgr_QueryInfo));
        memset(&CamInfo[0], 0, sizeof(SecMgr_CamInfo));
        memset(&CamInfo[1], 0, sizeof(SecMgr_CamInfo));
        this->mModule[0] = CAM_MAX;
        this->mModule[1] = CAM_MAX;

        this->dummy_sec_mva[0] = 0;
        this->dummy_sec_mva[1] = 0;
        this->dummy_sec_mva[2] = 0;

        this->m_pIspDrv = NULL;
        this->Num_SecCam = 0;
        this->tee_mem_type = TEE_MAX;
        this->buf_sec_mva.clear();
        this->buf_sec_mva_idx.clear();
        this->lsc_buf_sec_mva.clear();
        this->lsc_buff_idx.clear();
        this->bpc_buf_sec_mva.clear();
        this->bpc_buff_idx.clear();
        this->enable_secport.clear();

        for(MUINT32 i=0;i<MAX_DMA_PORT_NUM;i++){
            for(MUINT32 j=0;j<16;j++){
                fh_sec_mva[i][j] = 0;
                fh_sec_mva_idx[i][j] = 0;
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

    switch(module){
        default:
            //in 50, once if security is enabled, all cams' cq need to be locked
            if(this->Num_SecCam > 0)
               lock = 1;
            break;
    }
    return lock;
}

MBOOL SecMgrImp::SecMgr_GetSecurePortStatus(ISP_HW_MODULE module,MUINT32 port)
{
    MUINT32 dma_port = 0;

    if((this->CamInfo[0].CamModule == module) || (this->CamInfo[1].CamModule == module)){
        SecMgr_INF("+ (0x%x)module:0x%x dma port:%s",ispHandle,module,str_dmao(port));
        switch(port)
        {
            case _imgo_:
                dma_port = PORT_IMGO.index;
                break;
            case _rrzo_:
                dma_port = PORT_RRZO.index;
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
        SecMgr_DBG("Secure Cam (0x%x) is not enabled",module);
        return MFALSE;
    }

        return MTRUE;
}


SecMgr* SecMgr::SecMgr_Init()
{
    MINT32 ret = 0;

    SecMgr_INF("+ (0x%x)",ispHandle);

    if (MFALSE == gSecMgr.SecMgr_LoadCA()){
        SecMgr_ERR("Load CA library failure");
        return NULL;
    }

    if (MFALSE == gSecMgr.SecMgr_OpenCA()){
        SecMgr_ERR("Open CA session failure");
        return NULL;
    }

    if (MFALSE == gSecMgr.SecMgr_InitCA()){
        SecMgr_ERR("Init CA failure");
        return NULL;
    }


    if (MFALSE == gSecMgr.SecMgr_GetHWCapability()){
        SecMgr_ERR("Get secure constraints failure");
        return NULL;
    }

    return (SecMgr*)&gSecMgr;
}

SecMgr* SecMgr::SecMgr_GetMgrObj()
{
    SecMgr_DBG("+ (0x%x)",ispHandle);
    SecMgr_DBG("gSecMgrf:0x%x",&gSecMgr);
    return (SecMgr*)&gSecMgr;
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

//Check whether this Register is locked by 2nd DAPC
MBOOL SecMgrImp::SecMgr_CheckSecReg(ISP_HW_MODULE module, MUINT32 Addr)
{
    for(MUINT32 i=0;i<DAPC_NUM;i++){
        if(this->SecHwInfo[0].SecReg[i] & Addr)
            return MTRUE;
    }

    return MFALSE;
}

