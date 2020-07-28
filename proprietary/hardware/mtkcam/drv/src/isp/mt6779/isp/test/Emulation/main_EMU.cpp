#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include <errno.h>
#include <fcntl.h>


#include "isp_drv_cam.h"
#include "isp_drv_camsv.h"
#include "FakeDrvSensor.h"
#include <sys/stat.h>


//++ for alska ldvt non-swo #include "isp_function_cam.h"
#undef LOG_TAG
#define LOG_TAG "Test_IspDrv"
typedef signed char             MINT8;


#define LOG_VRB(fmt, arg...)        printf("[%s]" fmt "\n", __FUNCTION__, ##arg)
#define LOG_DBG(fmt, arg...)        printf("[%s]" fmt "\n", __FUNCTION__, ##arg)
#define LOG_INF(fmt, arg...)        printf("[%s]" fmt "\n", __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)        printf("[%s]" fmt "\n", __FUNCTION__, ##arg)
#define LOG_ERR(fmt, arg...)        printf("error:[%s]" fmt "\n", __FUNCTION__, ##arg)

#define LOG_VRBL(fmt, arg...)       printf("[%s:%d]" fmt "\n", __FUNCTION__,__LINE__, ##arg)
#define LOG_DBGL(fmt, arg...)       printf("[%s:%d]" fmt "\n", __FUNCTION__,__LINE__, ##arg)
#define LOG_INFL(fmt, arg...)       printf("[%s:%d]" fmt "\n", __FUNCTION__,__LINE__, ##arg)
#define LOG_WRNL(fmt, arg...)       printf("[%s:%d]" fmt "\n", __FUNCTION__,__LINE__, ##arg)
#define LOG_ERRL(fmt, arg...)       printf("error:[%s:%d]" fmt "\n", __FUNCTION__,__LINE__, ##arg)

#define SHOW_MATCH_SKIP_REGION   0
#define DUMP_REG_TO_FILE         0
#define DUMP_REG_TO_FILE_DISPLAY 0
#define SUMSAMPLE_NUM_MAX        32

#define ISP_CQ_APB_INST             0x0
#define ISP_CQ_NOP_INST             0x1
#define ISP_DRV_CQ_END_TOKEN        0x1C000000
const MUINT32 ISP_DRV_CQ_END_TOKEN_VALUE = ISP_DRV_CQ_END_TOKEN;
#define ISP_DRV_CQ_DUMMY_TOKEN(Module)   ((ISP_CQ_NOP_INST<<26)|((CQ_DUMMY_REG + Module*CAM_BASE_RANGE) & CQ_DES_RANGE))
#define ISP_CQ_DUMMY_PA             0x88100000

#define DESCRIPTOR_TOKEN(reg_oft,reg_num,dram_oft)  ({\
    MUINT32 tmp;                                      \
    tmp = ((reg_oft + dram_oft)&0xffff)  |            \
          (((reg_num-1)&0x3ff)<<16)      |            \
          ((ISP_CQ_APB_INST)<<26)        |            \
          (((reg_oft + dram_oft)&0x70000)<<(29-16));  \
    tmp;                                              \
})


#define CAMSV_GET_REG_OFFSET(IspDrvPtr,RegName,...)({                              \
    MUINT32 __offset;                                                              \
    __offset = (MUINT8*)(&( ((camsv_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
        (MUINT8*)(IspDrvPtr->m_pIspRegMap);                                        \
   __offset;                                                                       \
})

#define CAM_GET_REG_OFFSET(IspDrvPtr,RegName,...)({                              \
    MUINT32 __offset;                                                            \
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
        (MUINT8*)(IspDrvPtr->m_pIspRegMap);                                      \
   __offset;                                                                     \
})

MBOOL   Pattern_Loading34_Flag = MFALSE;
enum {
    _DMAI_TBL_LSCI_R1 = 0,
    _DMAI_TBL_LSCI_R2,
    _DMAI_TBL_UFDI_R1,
    _DMAI_TBL_UFDI_R2,
    _DMAI_TBL_BPCI_R1,
    _DMAI_TBL_BPCI_R2,
    _DMAI_TBL_RAWI,
    _DMAI_TBL_RAWI_R2,
    _DMAI_TBL_PDI_R1,
    _DMAI_TBL_PDI_R2,
    _DMAI_TBL_CQI,
    _DMAI_TBL_NUM,
} _DMAI_TBL_INDEX;

enum {
    _SUBSAMPLE_0 = 0 , _SUBSAMPLE_1 , _SUBSAMPLE_2 , _SUBSAMPLE_3,
    _SUBSAMPLE_4     , _SUBSAMPLE_5 , _SUBSAMPLE_6 , _SUBSAMPLE_7,
    _SUBSAMPLE_8     , _SUBSAMPLE_9 , _SUBSAMPLE_10, _SUBSAMPLE_11,
    _SUBSAMPLE_12    , _SUBSAMPLE_13, _SUBSAMPLE_14, _SUBSAMPLE_15,
    _SUBSAMPLE_16    , _SUBSAMPLE_17, _SUBSAMPLE_18, _SUBSAMPLE_19,
    _SUBSAMPLE_20    , _SUBSAMPLE_21, _SUBSAMPLE_22, _SUBSAMPLE_23,
    _SUBSAMPLE_24    , _SUBSAMPLE_25, _SUBSAMPLE_26, _SUBSAMPLE_27,
    _SUBSAMPLE_28    , _SUBSAMPLE_29, _SUBSAMPLE_30, _SUBSAMPLE_31,
    _SUBSAMPLE_NUM,
} _SUBSAMPLE_TBL_INDEX;

typedef enum {
  DMA_EN_GROUP0       = 0,
  CQ_NORMAL_PAGE_TYPE = DMA_EN_GROUP0,  // for _pCamsvCQVirtRegBuf , _pCamsvImgBuf
  DMA_EN_GROUP1       = 1,
  CQ_DUMMY_PAGE_TYPE  = DMA_EN_GROUP1,  // for _pCamsvCQVirtRegBuf , _pCamsvImgBuf
  DMA_EN_GROUP_NUM    = 2,
  CQ_PAGE_TYPE_NUM    = DMA_EN_GROUP_NUM,
}EDMA_EN_GROUPS;

typedef EDMA_EN_GROUPS ECQ_PAGE_TYPE_GROUP;

typedef enum {
  CAMSV_FBC_OFF_GROUP = 0,
  CQ_NORMAL_PAGE0     = CAMSV_FBC_OFF_GROUP,
  CQ_DUMMY_PAGE0      = CQ_NORMAL_PAGE0,
  CQ_PAGE0            = CQ_DUMMY_PAGE0,
  CAMSV_FBC_ON_GROUP  = 1,
  CQ_NORMAL_PAGE1     = CAMSV_FBC_ON_GROUP,
  CQ_DUMMY_PAGE1      = CQ_NORMAL_PAGE1,
  CQ_PAGE1            = CQ_DUMMY_PAGE1,
  CAMSV_FBC_GROUP_NUM = 2,
  CQ_NORMALPAGE_NUM   = CAMSV_FBC_GROUP_NUM,
  CQ_DUMMY_PAGE_NUM   = CQ_NORMALPAGE_NUM,
  CQ_PAGE_NUM         = CQ_DUMMY_PAGE_NUM,
} ECAMSV_FBC_GROUP;

typedef  ECAMSV_FBC_GROUP ECQ_NORMAL_PAGE_NUM_GROUP;
typedef  ECAMSV_FBC_GROUP ECQ_DUMMY_PAGES_NUM_GROUP;
typedef  ECAMSV_FBC_GROUP ECQ_PAGES_NUM_GROUP;


typedef enum {
  CamsvVF_OFF           = 0,
  CQ_IMGO_ENQ_ADDR_INX  = CamsvVF_OFF,
  CamsvVF_ON            = 1,
  CQ_IMGO_ENQ_HADDR_INX = CamsvVF_ON,
  CamsvVF_STATUS_NUM    = 2,
  CamsvVF_ENQ_ADDR_NUM  = CamsvVF_STATUS_NUM,
}ECAMSV_VF_STATUS;
typedef ECAMSV_VF_STATUS ECAMSV_ENQ_ADDR_TYPE;

typedef enum{
    TG_EN              = (1L<<0),
    PAK_EN             = (1L<<2),
    CAMSV_CTL_IMGO_EN_ = (1L<<4),
    DOWN_SAMPLE_EN     = (1L<<5),
    CAMSV_CTL_UFE_EN_  = (1L<<6),
    UFEO_EN            = (1L<<7),
    DB_EN              = (1L<<30),
}ENUM_CAMSV_CTL_DMA_EN;

typedef enum {
    CAMSV_FBC_USE_CQ_IMGO_ENQ_ADDR_INX,
    CAMSV_FBC_USE_CQ_IMGO_ENQ_HADDR_INX,
    CAMSV_FBC_USE_CQ_IMGO_RCNT_INC_STATUS_INX,
    CAMSV_FBC_USE_CQ_IMGO_R1_IMGO_FH_BASE_ADDR_DUMMY_PAGE0_INX,
    CAMSV_FBC_USE_CQ_IMGO_R1_IMGO_FH_BASE_ADDR_DUMMY_PAGE1_INX,
    CHANGE_BASEADDR_TO_MultiCamsvCQDummyDescriptorBuf_DUMMY_PAGE0_INX,
    CHANGE_BASEADDR_TO_MultiCamsvCQDummyDescriptorBuf_DUMMY_PAGE1_INX,
    CAMSV_FBC_USE_CQ_ITEMS_NUM
} ECAMSV_FBC_USE_CQ_CREATE_ITEMS;

static MUINT32 *gCamsvCQVirtualRegisterPhyAddr   = NULL;

static MUINT32 *gCamsvCQVirtualRegisterPhyAddr_chageDummyBaseAddress   = NULL;

IMEM_BUF_INFO    *_pCamsvCQDescriptorBuf         = NULL;
// _pCamsvImgBuf:
// [HW_MODULE][_SUBSAMPLE][CamsvVF_STATUS][DMA_EN_GROUP][CAMSV_FBC_GROUP]
// [HW_MODULE][_SUBSAMPLE][CamsvVF_STATUS][CQ_PAGE_TYPE][ECQ_PAGES_NUM]
IMEM_BUF_INFO ******_pCamsvImgBuf                = NULL;
// _pCamsvCQVirtRegBuf:
// [HW_MODULE][_SUBSAMPLE][CamsvVF_STATUS][DMA_EN_GROUP][CAMSV_FBC_GROUP]
// [HW_MODULE][_SUBSAMPLE][CamsvVF_STATUS][CQ_PAGE_TYPE][ECQ_PAGES_NUM]
IMEM_BUF_INFO ******_pCamsvCQVirtRegBuf          = NULL;
// _pCamsvCQVirtRegFBCFHAddrBuf:
// [_SUBSAMPLE_NUM]
IMEM_BUF_INFO     **_pCamsvCQVirtRegFBCFHAddrBuf = NULL;
// gpMultiCamsvCQDescriptorBuf:
// [HW_MODULE][CQ_NORMAL_PAGE_TYPE]
IMEM_BUF_INFO    ***gpMultiCamsvCQDescriptorBuf        = NULL;
// gpMultiCamsvCQDummyDescriptorBuf:
// [HW_MODULE][CQ_DUMMY_PAGE_TYPE]
IMEM_BUF_INFO    ***gpMultiCamsvCQDummyDescriptorBuf   = NULL;
// gpMultiCamsvCQVirtRegFBCFHAddrBuf:
// [HW_MODULE][_SUBSAMPLE_NUM][PAGEINX]
IMEM_BUF_INFO   ****gpMultiCamsvCQVirtRegFBCFHAddrBuf   = NULL;



struct TestInputInfo {
    struct DmaiTableInfo {
        const unsigned char     *pTblAddr;
        MUINT32                 tblLength;
        MUINT32                 simAddr;
    };
    MUINT32                 tgNum;
    MUINT32                 tg_enable[CAM_MAX];
    IMEM_BUF_INFO           *****pImemBufs;
    // m_pCamsvImemBufs:
    // [HW_MODULE][_SUBSAMPLE][CamsvVF_STATUS][DMA_EN_GROUP][CAMSV_FBC_GROUP]
    // [HW_MODULE][_SUBSAMPLE][CamsvVF_STATUS][CQ_PAGE_TYPE][ECQ_PAGES_NUM]
    IMEM_BUF_INFO           ******m_pCamsvImemBufs;
    // m_pCamsvCQVirtualRegisterBuf:
    // [HW_MODULE][_SUBSAMPLE][CamsvVF_STATUS][DMA_EN_GROUP][CAMSV_FBC_GROUP]
    // [HW_MODULE][_SUBSAMPLE][CamsvVF_STATUS][CQ_PAGE_TYPE][ECQ_PAGES_NUM]
    IMEM_BUF_INFO           ******m_pCamsvCQVirtualRegisterBuf;
    IMEM_BUF_INFO           rawiImemBuf;
    IMEM_BUF_INFO           CQImemBuf;
    IMEM_BUF_INFO           *m_pCamsvCQDescriptorBuf;
    // m_pMultiCamsvCQDescriptorBuf:
    // [HW_MODULE][CQ_NORMAL_PAGE_NUM]
    IMEM_BUF_INFO           ***m_pMultiCamsvCQDescriptorBuf;
    // m_pMultiCamsvCQDummyDescriptorBuf:
    // [HW_MODULE][CQ_DUMMY_PAGE_NUM]
    IMEM_BUF_INFO           ***m_pMultiCamsvCQDummyDescriptorBuf;
    // m_pCamsvCQVirRegFBCFHAddrBuf:
    // [_SUBSAMPLE_NUM]
    IMEM_BUF_INFO           **m_pCamsvCQVirRegFBCFHAddrBuf;
    // m_pMultiCamsvCQVirRegFBCFHAddrBuf:
    // [HW_MODULE][_SUBSAMPLE_NUM][PageInx]
    IMEM_BUF_INFO           ****m_pMultiCamsvCQVirRegFBCFHAddrBuf;

    DmaiTableInfo           DmaiTbls[CAM_MAX][_DMAI_TBL_NUM];

    DmaiTableInfo           SubsampleTbls[CAMSV_MAX][_SUBSAMPLE_NUM]
                                         [CAMSV_FBC_GROUP_NUM];

    DmaiTableInfo           SubsampleDummyTbls[CAMSV_MAX][_SUBSAMPLE_NUM]
                                              [CQ_DUMMY_PAGE_NUM];
    MUINT32                 m_CamsvNum;
    MUINT32                 m_DCIF; // 1:DCIF on
    MUINT32                 m_DCIF_ONE_RAWI_R2_CASE; // 1:DCIF_only One Rawi_R2
    TestInputInfo()
        {
            int i = 0, cam = 0;

            tgNum = 1;
            pImemBufs                    = NULL;
            m_pCamsvImemBufs             = NULL;
            m_pCamsvCQVirtualRegisterBuf = NULL;
            m_pCamsvCQDescriptorBuf      = NULL;
            m_pCamsvCQVirRegFBCFHAddrBuf = NULL;
            m_pMultiCamsvCQDescriptorBuf = NULL;
            m_pMultiCamsvCQDummyDescriptorBuf = NULL;
            m_pMultiCamsvCQVirRegFBCFHAddrBuf = NULL;
            m_DCIF = 0;
            m_DCIF_ONE_RAWI_R2_CASE = 0;
            this->m_CamsvNum = 0;
            for (cam = 0; cam < CAM_MAX; cam++) {
                tg_enable[cam] = 1;
                for (i = 0; i < _DMAI_TBL_NUM; i++) {
                    DmaiTbls[cam][i].pTblAddr = NULL;
                    DmaiTbls[cam][i].tblLength = 0;
                    DmaiTbls[cam][i].simAddr = 0;
                }
            }
            for (cam = 0; cam < CAMSV_MAX; cam++) {
              for (i = 0; i < _SUBSAMPLE_NUM; i++) {
                for(MUINT32 j = 0; j < CAMSV_FBC_GROUP_NUM; j++){
                    SubsampleTbls[cam][i][j].pTblAddr = NULL;
                    SubsampleTbls[cam][i][j].tblLength = 0;
                    SubsampleTbls[cam][i][j].simAddr = 0;
                }
              }
            }

            for (cam = 0; cam < CAMSV_MAX; cam++) {
              for (i = 0; i < _SUBSAMPLE_NUM; i++) {
                for(MUINT32 j = 0; j < CQ_DUMMY_PAGE_NUM; j++){
                    SubsampleDummyTbls[cam][i][j].pTblAddr = NULL;
                    SubsampleDummyTbls[cam][i][j].tblLength = 0;
                    SubsampleDummyTbls[cam][i][j].simAddr = 0;
                }
              }
            }

        }
};

struct skipPairSt {
    unsigned int offset;
    unsigned int nbyte;
public:
    skipPairSt (int _ofst = 0, int _nb = 0)
        : offset(_ofst), nbyte(_nb) {}
};

typedef enum{
    IMGO_R1,
    RAWI_R2,
}ENUM_CAM_CTL_DMA;

MUINT32
AllocCamsvSubsampleBuffer(
    IMEM_BUF_INFO ******pCamsvBuf,
    ISP_HW_MODULE hwModule,
    MUINT32 subSampleNum,
    EDMA_EN_GROUPS dmaEnGroup,
    ECAMSV_VF_STATUS camsvVFstatus,
    MUINT32 bufSize,
    ECAMSV_FBC_GROUP camsvFbcstatus);

void
CopyCamsvSubsampleTblDataToCQVirtRegBuf(
    IMEM_BUF_INFO ******_pCamsvCQTokenBuf,
    TestInputInfo *pInputInfo,
    ISP_HW_MODULE hwModule, MUINT32 subsampleInx,
    ECAMSV_VF_STATUS camsvVFstatus, EDMA_EN_GROUPS dmaEnGroup,
    ECAMSV_FBC_GROUP camsvFbcGroup);

void
CopyCamsvSubsampleTblDataToCQDummyVirtRegBuf(
    IMEM_BUF_INFO ******_pCamsvCQDummyVirtRegBuf,
    TestInputInfo *pInputInfo,
    ISP_HW_MODULE hwModule,
    MUINT32 subsampleInx,
    ECAMSV_VF_STATUS camsvVFstatus,
    ECQ_PAGE_TYPE_GROUP pageType,
    ECQ_DUMMY_PAGES_NUM_GROUP cqDummyPageInx,
    MUINT32 dummyPageValue);

MBOOL
Beyond_Compare_Subsamples(
    ISP_HW_MODULE module,
    MUINT32 subsampleInx,
    IMEM_BUF_INFO ******pBuf,
    const unsigned char *_table,
    EDMA_EN_GROUPS _DMA_EN_GROUP,
    ECAMSV_FBC_GROUP camsvFbcGroup,
    MUINT32 dma_addr_offset = 0,
    std::vector<skipPairSt> *pSkips = NULL);

MBOOL
Beyond_Compare_Subsamples_MultiCQ(
    ISP_HW_MODULE module,
    MUINT32 subsampleInx,
    IMEM_BUF_INFO ******pBuf,
    const unsigned char *_table,
    EDMA_EN_GROUPS _DMA_EN_GROUP,/* ECQ_PAGE_TYPE_GROUP */
    ECAMSV_FBC_GROUP camsvFbcGroup,/* ECQ_PAGES_NUM_GROUP */
    MUINT32 dma_addr_offset,
    std::vector<skipPairSt> *pSkips,
    ECAMSV_ENQ_ADDR_TYPE enqAddrType /*ECAMSV_ENQ_ADDR_TYPE */);

int RelocateCqTableBase(unsigned int *cq_tbl, int tbl_size, int descriptor_size,
        unsigned int sim_pa, unsigned int real_pa, unsigned int *real_va)
{
#define CQ_END_TOKEN        (0x1c000000)
#define max(x,y) (((x) > (y)) ? (x) : (y))
#define min(x,y) (((x) < (y)) ? (x) : (y))
#define offset(x,y) (max(x,y) - min(x,y))
    unsigned int delta_pa, cur_pa;
    unsigned int desc_words, tbl_words, i;
    unsigned int *ptr, *dst;

    delta_pa = offset(real_pa, sim_pa);
    desc_words = descriptor_size / 4;
    tbl_words = tbl_size / 4;

    LOG_INF("cq_tbl = %p tbl_size = 0x%x \n", cq_tbl, tbl_size);

    LOG_INF("simPA = 0x%x realPA = 0x%x delta = 0x%x\n", sim_pa, real_pa,
            delta_pa);

    LOG_INF("desc_words = 0x%x tbl_words = 0x%x\n", desc_words, tbl_words);

    if (desc_words & 1) {
        LOG_INF("token num should be even: %d\n", desc_words);
    }

    // relocate token
    ptr = cq_tbl;
    dst = real_va;
    for (i = 0; i < desc_words; i += 2) {

        switch (*ptr) {
        case CQ_END_TOKEN:
        case 0x0:
            //LOG_INF("skip token 0x%08x_0x%08x\n", *ptr, *(ptr+1));
            *dst++ = *ptr++; // 4byte inc
            *dst++ = *ptr++;
            break;
        default:
            //LOG_INF("i:%d ptr: 0x%08x dst: 0x%08x\n", i, ptr, dst);
            //LOG_INF("*ptr: 0x%08x > *dst: 0x%08x\n", *(ptr), *(dst));
            *dst++ = *ptr++; // 4byte inc
            if(real_pa > sim_pa)
                *dst++ = *ptr++ + delta_pa;
            else
                *dst++ = *ptr++ - delta_pa;
            //LOG_INF("PA: 0x%08x > 0x%08x\n", *(ptr-1), *(dst-1));
            break;
        }
    }

    LOG_INF("Copy vir reg\n");
    memcpy(dst, ptr, tbl_size - descriptor_size);

    return 0;
}


MUINT32 CamIdMap[]   = {CAM_A,   CAM_B,   CAM_C};
MUINT32 CamSVIdMap[] = {CAMSV_0, CAMSV_1, CAMSV_2, CAMSV_3, CAMSV_4, CAMSV_5};
TS_FakeDrvSensor fakedrvsensor;
int TS_FakeDrvSensor::mInitCount = 0;



MUINT32
AllocCamsvSubsampleBuffer(
    IMEM_BUF_INFO ******pCamsvBuf,
    ISP_HW_MODULE hwModule,
    MUINT32 subSampleNum,
    EDMA_EN_GROUPS dmaEnGroup, /*ECQ_PAGE_TYPE_GROUP*/
    ECAMSV_VF_STATUS camsvVFstatus,
    MUINT32 bufSize,
    ECAMSV_FBC_GROUP camsvFbcstatus) /* ECQ_PAGES_NUM_GROUP*/
{
   IMemDrv* pImemDrv = NULL;
   pImemDrv = IMemDrv::createInstance();
   if(pImemDrv->init() < 0){
       LOG_ERR(" imem init fail\n");
       return 1;
   }

   for(MUINT32 subSampleInx = 0; subSampleInx < subSampleNum; subSampleInx++) {

        pCamsvBuf[hwModule][subSampleInx][camsvVFstatus][dmaEnGroup]
            [camsvFbcstatus]->size = bufSize;

        LOG_INFL("CAMSV subsample buf allocate: subSampleInx:%d, size:0x%x bytes\n",
                 subSampleInx,
                 pCamsvBuf[hwModule][subSampleInx][camsvVFstatus][dmaEnGroup]
                 [camsvFbcstatus]->size);

        if (pImemDrv->allocVirtBuf(pCamsvBuf[hwModule][subSampleInx]
                [camsvVFstatus][dmaEnGroup][camsvFbcstatus]) < 0) {

            LOG_ERR("CAMSV subsample imem alloc fail at subSampleInx 0x%x\n",
                subSampleInx);

            return 0;
        }
        if (pImemDrv->mapPhyAddr(pCamsvBuf[hwModule][subSampleInx]
            [camsvVFstatus][dmaEnGroup][camsvFbcstatus]) < 0) {

            LOG_ERR(" CAMSV subsample imem map fail at subSampleInx %02d\n",
                subSampleInx);

            return 0;
        }

        memset((MUINT8*)pCamsvBuf[hwModule][subSampleInx][camsvVFstatus]
            [dmaEnGroup][camsvFbcstatus]->virtAddr, 0xFF,
            pCamsvBuf[hwModule][subSampleInx][camsvVFstatus][dmaEnGroup]
            [camsvFbcstatus]->size);


        LOG_INFL("CAMSV subsampe PA(value):0x%lx "
                 "VA(value):0x%lx "
                 "hwModule:0x%x subSampleInx:0x%x camsvFbcstatus:0x%x"
                 "dmaEnGroup:0x%x camsvFbcstatus:0x%x\n",
                 pCamsvBuf[hwModule][subSampleInx][camsvVFstatus][dmaEnGroup]
                 [camsvFbcstatus]->phyAddr,
                 pCamsvBuf[hwModule][subSampleInx][camsvVFstatus][dmaEnGroup]
                 [camsvFbcstatus]->virtAddr,
                 hwModule, subSampleInx, camsvFbcstatus,
                 dmaEnGroup, camsvFbcstatus);
   }

   pImemDrv->uninit();
   pImemDrv->destroyInstance();
   return 0;
}


MUINT32
AllocCamsvSubsampleCQDescriptorBuffer(
    char const szUsrName[32],
    IMEM_BUF_INFO *pCamsvCQBuf,
    ISP_HW_MODULE hwModule,
    MUINT32 _bufSize)
{
    (void)szUsrName;
    IMemDrv* pImemDrv = NULL;
    pImemDrv = IMemDrv::createInstance();
    if(pImemDrv->init() < 0){
        LOG_ERR(" imem init fail\n");
        return 1;
    }

    pCamsvCQBuf->size = _bufSize;

    LOG_INFL("CQ buf allocate: size:0x%x bytes\n",
        pCamsvCQBuf->size );

    if (pImemDrv->allocVirtBuf(pCamsvCQBuf) < 0) {
        LOG_ERR("CQ Buf imem alloc fail\n");
        return 0;
    }
    if (pImemDrv->mapPhyAddr(pCamsvCQBuf) < 0) {
        LOG_ERR("CQ Buf subsample imem map fail\n" );
        return 0;
    }

    memset((MUINT8*)pCamsvCQBuf->virtAddr, 0x00, pCamsvCQBuf->size);


    LOG_INFL("CQ PA(value_address):0x%lx VA(value_address):0x%lx "
             "hwModule:0x%x\n",
             pCamsvCQBuf->phyAddr,
             pCamsvCQBuf->virtAddr,
             hwModule);

    pImemDrv->uninit();
    pImemDrv->destroyInstance();

    return 0;
}

void
CopyCamsvSubsampleTblDataToCQVirtRegBuf(
    IMEM_BUF_INFO ******_pCamsvCQVirtRegBuf,
    TestInputInfo *pInputInfo,
    ISP_HW_MODULE hwModule, MUINT32 subsampleInx,
    ECAMSV_VF_STATUS camsvVFstatus, EDMA_EN_GROUPS dmaEnGroup,
    ECAMSV_FBC_GROUP camsvFbcGroup)
{
    enum {
         CAMSV_IMGO_FH_SPARE_1_INX = 0 , CAMSV_IMGO_FH_SPARE_2_INX ,
         CAMSV_IMGO_FH_SPARE_3_INX     , CAMSV_IMGO_FH_SPARE_4_INX ,
         CAMSV_IMGO_FH_SPARE_5_INX     , CAMSV_IMGO_FH_SPARE_6_INX ,
         CAMSV_IMGO_FH_SPARE_7_INX     , CAMSV_IMGO_FH_SPARE_8_INX ,
         CAMSV_IMGO_FH_SPARE_9_INX     , CAMSV_IMGO_FH_SPARE_10_INX,
         CAMSV_IMGO_FH_SPARE_11_INX    , CAMSV_IMGO_FH_SPARE_12_INX,
         CAMSV_IMGO_FH_SPARE_13_INX    , CAMSV_IMGO_FH_SPARE_14_INX,
         CAMSV_IMGO_FH_SPARE_15_INX    , CAMSV_IMGO_FH_SPARE_16_INX,
         CAMSV_IMGO_FH_BASE_ADDR_INX   , CAMSV_CAMSV_IMGO_FBC_INX  ,
         CAMSV_TEST_CAM_CQ_ITEM_NUM    ,
      } ECAMSV_TEST_CAM_CQ_ITEM_NUM    ;

    memcpy((MUINT32 *)_pCamsvCQVirtRegBuf[hwModule][subsampleInx][camsvVFstatus]
           [dmaEnGroup][camsvFbcGroup]->virtAddr,
           (MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
           [camsvFbcGroup].pTblAddr), (4 * 16));

    for(MUINT32 i = CAMSV_IMGO_FH_SPARE_1_INX;
        i <=CAMSV_IMGO_FH_SPARE_16_INX; i++){
        LOG_INFL("_pCamsvCQVirtRegBuf[%02d][%02d][%02d][%02d][%02d]"
                 "->virAddr_%02d=0x%lx "
                 "->*virAddr_%02d=0x%x ",
                 hwModule, subsampleInx, camsvVFstatus, dmaEnGroup,camsvFbcGroup,
                 i,
                 ((_pCamsvCQVirtRegBuf[hwModule][subsampleInx]
                 [camsvVFstatus][dmaEnGroup][camsvFbcGroup]->virtAddr) + i),
                 i,
                 *(((MUINT32 *)_pCamsvCQVirtRegBuf[hwModule][subsampleInx]
                 [camsvVFstatus][dmaEnGroup][camsvFbcGroup]->virtAddr) + i));
    }
}

void
CopyCamsvSubsampleTblDataToCQDummyVirtRegBuf(
    IMEM_BUF_INFO ******_pCamsvCQDummyVirtRegBuf,
    TestInputInfo *pInputInfo,
    ISP_HW_MODULE hwModule,
    MUINT32 subsampleInx,
    ECAMSV_VF_STATUS camsvVFstatus,
    ECQ_PAGE_TYPE_GROUP pageType,
    ECQ_DUMMY_PAGES_NUM_GROUP cqDummyPageInx,
    MUINT32 dummyPageValue)
{
    static MUINT32   lDummyPageValue = 0;
    static MUINT32 *pCamsvImgoFbcRcntInc1Value  = &lDummyPageValue;
    (void)pInputInfo;
    lDummyPageValue = dummyPageValue;
    enum {
         IMGO_R1_IMGO_FH_SPARE_01_INX = 0 , IMGO_R1_IMGO_FH_SPARE_02_INX ,
         IMGO_R1_IMGO_FH_SPARE_03_INX     , IMGO_R1_IMGO_FH_SPARE_04_INX ,
         IMGO_R1_IMGO_FH_SPARE_05_INX     , IMGO_R1_IMGO_FH_SPARE_06_INX ,
         IMGO_R1_IMGO_FH_SPARE_07_INX     , IMGO_R1_IMGO_FH_SPARE_08_INX ,
         IMGO_R1_IMGO_FH_SPARE_09_INX     , IMGO_R1_IMGO_FH_SPARE_10_INX ,
         IMGO_R1_IMGO_FH_SPARE_11_INX     , IMGO_R1_IMGO_FH_SPARE_12_INX ,
         IMGO_R1_IMGO_FH_SPARE_13_INX     , IMGO_R1_IMGO_FH_SPARE_14_INX ,
         IMGO_R1_IMGO_FH_SPARE_15_INX     , IMGO_R1_IMGO_FH_SPARE_16_INX ,
         IMGO_R1A_IMGO_FH_BASE_ADDR_INX   ,
         CAM_TEST_CQ_ITEM_NUM             ,
      } ECAM_TEST_CQ_ITEM_NUM;



    for(MUINT32 inx = IMGO_R1_IMGO_FH_SPARE_02_INX;
        inx <= IMGO_R1_IMGO_FH_SPARE_16_INX; inx++) {

        memcpy((MUINT32 *)_pCamsvCQDummyVirtRegBuf[hwModule][subsampleInx]
        [camsvVFstatus][pageType][cqDummyPageInx]->virtAddr + inx,
        pCamsvImgoFbcRcntInc1Value,
        sizeof(pCamsvImgoFbcRcntInc1Value));

        LOG_INFL("_pCamsvCQDummyVirtRegBuf[%02d][%02d][%02d][%02d]"
                 "->virAddr_%02d=0x%lx "
                 "->*virAddr_%02d=0x%x ",
                 hwModule, subsampleInx, camsvVFstatus, pageType,
                 inx,
                 ((_pCamsvCQDummyVirtRegBuf[hwModule][subsampleInx]
                 [camsvVFstatus][pageType][cqDummyPageInx]->virtAddr) + inx),
                 inx,
                 *(((MUINT32 *)_pCamsvCQDummyVirtRegBuf[hwModule][subsampleInx]
                 [camsvVFstatus][pageType][cqDummyPageInx]->virtAddr) + inx));
    }
}


MINT32 LDVT_DRV_INIT(MUINT32* _ptr,MUINTPTR _raw_num,MUINT32* _camsv,MUINTPTR linkpath)
{
    ISP_DRV_CAM** ptr = (ISP_DRV_CAM**)_ptr;
    IspDrvCamsv** pCamsv = (IspDrvCamsv**)_camsv;

    switch(_raw_num){
    case 3:// no break if _raw_num >= 2

        ptr[CAM_C] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_C,
                        ISP_DRV_CQ_THRE0, 0, "Test_IspDrvCam_C");

        if(ptr[CAM_C] == NULL){
            LOG_ERR("CAM_C create fail\n");
            return -1;
        }

        if(ptr[CAM_C]->init("Test_IspDrvCam_C") == MFALSE){
            ptr[CAM_C]->destroyInstance();
            LOG_ERR("CAM_C init failure\n");
            ptr[CAM_C] = NULL;
            return -1;
        }
    case 2:// no break if _raw_num >= 2

        ptr[CAM_B] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,
                        ISP_DRV_CQ_THRE0, 0, "Test_IspDrvCam_B");

        if(ptr[CAM_B] == NULL){
            LOG_ERR("CAM_B create fail\n");
            return -1;
        }

        if(ptr[CAM_B]->init("Test_IspDrvCam_B") == MFALSE){
            ptr[CAM_B]->destroyInstance();
            LOG_ERR("CAM_B init failure\n");
            ptr[CAM_B] = NULL;
            return -1;
        }
    case 1:
        ptr[CAM_A] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,
                        ISP_DRV_CQ_THRE0, 0, "Test_IspDrvCam_A");

        if(ptr[CAM_A] == NULL){
            LOG_ERR("CAM_A create fail\n");
            return -1;
        }

        if(ptr[CAM_A]->init("Test_IspDrvCam_A") == MFALSE){
            ptr[CAM_A]->destroyInstance();
            LOG_ERR("CAM_A init failure\n");
            ptr[CAM_A] = NULL;
            return -1;
        }
        break;
    default:
        for(MUINT32 i=0;i<_raw_num;i++){
            ptr[i] = NULL;
        }
        LOG_INF("##############################\n");
        LOG_ERR("un-supported hw (%d)\n",(int)_raw_num);
        LOG_INF("##############################\n");
        return 1;
        break;
    }

    switch(linkpath){
        case CAM_A:
            break;
        case CAM_B:
            break;
        case CAM_C:
            break;
        case CAM_MAX:
            break;
        case CAMSV_MAX:
            pCamsv[CAMSV_0-CAM_MAX] =
                (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_0);

            if (pCamsv[CAMSV_0-CAM_MAX] == NULL) {
                LOG_ERR("CAMSV_0 create fail\n");
                return -1;
            }

            if (pCamsv[CAMSV_0-CAM_MAX]->init("Test_IspDrvCamsv_0") == MFALSE) {
                pCamsv[CAMSV_0-CAM_MAX]->destroyInstance();
                LOG_ERR("Camsv_0 init failure\n");
                pCamsv[CAMSV_0-CAM_MAX] = NULL;
                return -1;
            }

            pCamsv[CAMSV_1-CAM_MAX] =
                (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);

            if (pCamsv[CAMSV_1-CAM_MAX] == NULL) {
                LOG_ERR("CAMSV_1 create fail\n");
                return -1;
            }

            if (pCamsv[CAMSV_1-CAM_MAX]->init("Test_IspDrvCamsv_1") == MFALSE) {
                pCamsv[CAMSV_1-CAM_MAX]->destroyInstance();
                LOG_ERR("Camsv_1 init failure\n");
                pCamsv[CAMSV_1-CAM_MAX] = NULL;
                return -1;
            }
    }


    for (int i = 0; i < (int)_raw_num; i++) {
        LOG_INF("Reseting CAM_%c(%d)", 'A'+i, CamIdMap[i]);
        if (ptr[CamIdMap[i]]) {
            ptr[CamIdMap[i]]->setDeviceInfo(_SET_RESET_HW_MOD, NULL);
        }

        LOG_INF("CAM_%c clear ctl/dma enable reg\n", 'A'+i);
        CAM_WRITE_REG((ptr[CamIdMap[i]]->getPhyObj()), CAMCTL_R1_CAMCTL_EN, 0);
        CAM_WRITE_REG((ptr[CamIdMap[i]]->getPhyObj()), CAMCTL_R1_CAMCTL_DMA_EN, 0);
    }

    for (int i = 0; i < 2; i++) {

        LOG_INF("Reseting CAMSV_%d, pCamsv[%d]=%p",
            i, CamSVIdMap[i]-CAM_MAX, pCamsv[CamSVIdMap[i]-CAM_MAX]);

        if (pCamsv[CamSVIdMap[i]-CAM_MAX]) {
            pCamsv[CamSVIdMap[i]-CAM_MAX]->setDeviceInfo(_SET_RESET_HW_MOD, NULL);
        }

        LOG_INF("CAMSV_%d clear ctl/dma enable reg\n", i);


        CAMSV_WRITE_REG((pCamsv[CamSVIdMap[i]-CAM_MAX]),
            CAMSV_MODULE_EN, 0);
    }

    return 0;
}

MINT32 MMU_INIT(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR linkpath)
{
    ISP_DRV_CAM** ptr = (ISP_DRV_CAM**)_ptr;
    _pCamsv; linkpath;
    #if 1 //ioncarve out and CVD setting to 1
    for (int i = 0; i < (int)length; i++) {
        if (ptr[i]) {
            ISP_LARB_MMU_STRUCT larbInfo;
            int idx = 0;
            // P1 HW dmao not through M4U

            //LOG_INF("Config MMU Larb to PA at CAM_%c .............", 'A'+i);

            larbInfo.regVal = 0;

            larbInfo.LarbNum = 0;
            for (idx = 0; idx < 26; idx++) {
                larbInfo.regOffset = 0x380 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
                //LOG_INF("config larb=%d offset=0x%x",
                //        larbInfo.LarbNum, larbInfo.regOffset);
            }

            larbInfo.LarbNum = 9;
            for (idx = 0; idx < 24; idx++) {
                larbInfo.regOffset = 0x380 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
                //LOG_INF("config larb=%d offset=0x%x",
                //        larbInfo.LarbNum, larbInfo.regOffset);
            }

            larbInfo.LarbNum = 10;
            for (idx = 0; idx < 31; idx++) {
                larbInfo.regOffset = 0x380 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
                //LOG_INF("config larb=%d offset=0x%x",
                //        larbInfo.LarbNum, larbInfo.regOffset);
            }
            break; //only need once
        }
    }
    #endif
    for (int i = 0; i < (int)length; i++) {
        LOG_INF("Reseting CAM_%c", ('A'+i));
        if (ptr[i]) {
            ptr[i]->setDeviceInfo(_SET_RESET_HW_MOD, NULL);
        }
    }
    getchar();
    return 0;
}


MINT32 Pattern_Start_1(MUINT32* _ptr,MUINTPTR _raw_num,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    MUINT32 CamDmaEnSt = 0,CamDma2EnSt = 0,CamsvDamEnSt = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO***** pBuf;
    IMemDrv* pImemDrv = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 CamsvNum = 0;

    pBuf = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    CamsvNum = pInputInfo->m_CamsvNum;

    #define HEADER_MEM_ALLOC(HwModule,DMA,DMASYS,DMA_RX,DMA_EN_ST,DMA_EN_GROUP)\
        MUINT32 _tmp = (DMA_EN_ST & DMA_RX##_EN_);                             \
        MUINT32 _cnt = 0;                                                      \
        while(_tmp != 0){                                                      \
            _cnt++;                                                            \
            _tmp >>= 1;                                                        \
        }                                                                      \
        pBuf[HwModule][_cnt][1][DMA_EN_GROUP]->size = 4*16;                    \
                                                                               \
        LOG_INF("HEADER-%s HEADER-buf allocate: idx:%d, size:64 bytes\n",      \
            #DMA_RX,_cnt);                                                     \
                                                                               \
        if (pImemDrv->allocVirtBuf(pBuf[HwModule][_cnt][1][DMA_EN_GROUP]) < 0) \
        {                                                                      \
            LOG_ERR(" imem alloc fail at %s\n",#DMA_RX);                       \
            return 1;                                                          \
        }                                                                      \
        if (pImemDrv->mapPhyAddr(pBuf[HwModule][_cnt][1][DMA_EN_GROUP]) < 0) { \
            LOG_ERR(" imem map fail at %s\n",#DMA_RX);                         \
            return 1;                                                          \
        }                                                                      \
                                                                               \
        LOG_INF("HEADER-%s PA:0x%lx VA:0x%lx\n",#DMA_RX,                       \
                pBuf[HwModule][_cnt][1][DMA_EN_GROUP]->phyAddr,                \
                pBuf[HwModule][_cnt][1][DMA_EN_GROUP]->virtAddr);              \
                                                                               \
        DMASYS##_WRITE_REG(ptr,DMA_RX##_##DMA##_FH_BASE_ADDR,                  \
                pBuf[HwModule][_cnt][1][DMA_EN_GROUP]->phyAddr);               \


    //aao/pso/bpci's stride is useless. need to use xsize to cal. total memory size
    #define MEM_ALLOC(HwModule,DMA,DMASYS,DMA_RX,DMA_EN_ST,DMA_EN_GROUP)       \
        MUINT32 tmp     = (DMA_EN_ST & DMA_RX##_EN_);                          \
        MUINT32 _dma_en = (DMA_EN_ST & DMA_RX##_EN_);                          \
        MUINT32 dmaEnGroup = DMA_EN_GROUP;                                     \
        MUINT32 cnt = 0;                                                       \
        while(tmp != 0){                                                       \
            cnt++;                                                             \
            tmp >>= 1;                                                         \
        }                                                                      \
        if(0/*(_dma_en & IMGO_EN_)|| (_dma_en & RRZO_R1_EN_)*/){               \
            MUINT32 __tmp =                                                    \
                (DMASYS##_READ_BITS(ptr,                                       \
                    DMA_RX##_##DMA##_XSIZE,DMA##_XSIZE) + 1 );                 \
                DMASYS##_WRITE_BITS(ptr,                                       \
                    DMA_RX##_##DMA##_STRIDE,DMA##_STRIDE,__tmp);               \
                                                                               \
            pBuf[HwModule][cnt][0][DMA_EN_GROUP]->size =                       \
                ((DMASYS##_READ_BITS(ptr,                                      \
                    DMA_RX##_##DMA##_YSIZE,DMA##_YSIZE)+1) *                   \
                 (DMASYS##_READ_BITS(ptr,                                      \
                    DMA_RX##_##DMA##_STRIDE,DMA##_STRIDE)));                   \
        }                                                                      \
        else{                                                                  \
            if(((_dma_en & BPCI_R1_EN_)  && (DMA_EN_GROUP0 == dmaEnGroup)) ||  \
               ((_dma_en & TSFSO_R1_EN_) && (DMA_EN_GROUP0 == dmaEnGroup)) ||  \
               ((_dma_en & AAO_R1_EN_)   && (DMA_EN_GROUP0 == dmaEnGroup)) ||  \
               ((_dma_en & FLKO_R1_EN_)  && (DMA_EN_GROUP0 == dmaEnGroup)) ||  \
               ((_dma_en & LMVO_R1_EN_)  && (DMA_EN_GROUP0 == dmaEnGroup))){   \
                LOG_INF("%s 1D-DMA using xsize instead of stride",#DMA);       \
                                                                               \
                pBuf[HwModule][cnt][0][DMA_EN_GROUP]->size =                   \
                    ((DMASYS##_READ_BITS(ptr,                                  \
                        DMA_RX##_##DMA##_YSIZE,DMA##_YSIZE) + 1) *             \
                     (DMASYS##_READ_BITS(ptr,                                  \
                        DMA_RX##_##DMA##_XSIZE,DMA##_XSIZE) + 1));             \
            }                                                                  \
            else{                                                              \
                pBuf[HwModule][cnt][0][DMA_EN_GROUP]->size =                   \
                    ((DMASYS##_READ_BITS(ptr,                                  \
                        DMA_RX##_##DMA##_YSIZE,DMA##_YSIZE)+1) *               \
                     (DMASYS##_READ_BITS(ptr,                                  \
                        DMA_RX##_##DMA##_STRIDE,DMA##_STRIDE)));               \
            }                                                                  \
        }                                                                      \
        LOG_INF("%s buf allocate: idx:%d, size:%d bytes\n",                    \
                #DMA_RX,cnt,pBuf[HwModule][cnt][0][DMA_EN_GROUP]->size);       \
                                                                               \
        if (pImemDrv->allocVirtBuf(pBuf[HwModule][cnt][0][DMA_EN_GROUP]) < 0) {\
            LOG_ERR(" imem alloc fail at %s\n",#DMA_RX);                       \
            return 1;                                                          \
        }                                                                      \
        if (pImemDrv->mapPhyAddr(pBuf[HwModule][cnt][0][DMA_EN_GROUP]) < 0) {  \
            LOG_ERR(" imem map fail at %s\n",#DMA_RX);                         \
            return 1;                                                          \
        }                                                                      \
        if(0/*(_dma_en & IMGO_EN_)|| (_dma_en & RRZO_R1_EN_)*/){               \
        }                                                                      \
        else{                                                                  \
            memset((MUINT8*)pBuf[HwModule][cnt][0][DMA_EN_GROUP]->virtAddr,0x0,\
                pBuf[HwModule][cnt][0][DMA_EN_GROUP]->size);                   \
        }                                                                      \
                                                                               \
        LOG_INF("%s PA:0x%lx  VA:0x%lx\n",#DMA_RX,                             \
            pBuf[HwModule][cnt][0][DMA_EN_GROUP]->phyAddr,                     \
            pBuf[HwModule][cnt][0][DMA_EN_GROUP]->virtAddr);                   \
                                                                               \
        DMASYS##_WRITE_REG(ptr,DMA_RX##_##DMA##_BASE_ADDR,                     \
            pBuf[HwModule][cnt][0][DMA_EN_GROUP]->phyAddr);                    \

    #define MEM_ALLOC_1D(HwModule,DMA,DMASYS,DMA_RX,DMA_EN_ST,DMA_EN_GROUP)    \
        MUINT32 tmp     = (DMA_EN_ST & DMA_RX##_EN_);                          \
        MUINT32 _dma_en = (DMA_EN_ST & DMA_RX##_EN_);                          \
        MUINT32 cnt = 0;                                                       \
        while(tmp != 0){                                                       \
            cnt++;                                                             \
            tmp >>= 1;                                                         \
        }                                                                      \
        if((_dma_en & BPCI_R1_EN_) || (_dma_en & TSFSO_R1_EN_) ||              \
           (_dma_en & AAO_EN_)     || (_dma_en & FLKO_R1_EN_)){                \
            LOG_INF("%s 1D-DMA using xsize instead of stride",#DMA);           \
                                                                               \
            pBuf[HwModule][cnt][0][DMA_EN_GROUP]->size =                       \
                ((DMASYS##_READ_BITS(ptr,                                      \
                    DMA_RX##_##DMA##_YSIZE,DMA##_YSIZE)+1) *                   \
                 (DMASYS##_READ_BITS(ptr,                                      \
                    DMA_RX##_##DMA##_XSIZE,DMA##_XSIZE)+1));                   \
                                                                               \
            LOG_INF("1D-%s 1D-buf allocate: idx:%d, 1D-size:%d bytes\n",#DMA,  \
                cnt,pBuf[HwModule][cnt][0][DMA_EN_GROUP]->size);               \
                                                                               \
            if (pImemDrv->allocVirtBuf(pBuf[HwModule][cnt][0][DMA_EN_GROUP]) < \
                0) {                                                           \
                LOG_ERR(" imem alloc fail at %s\n",#DMA_RX);                   \
                return 1;                                                      \
            }                                                                  \
            if (pImemDrv->mapPhyAddr(pBuf[HwModule][cnt][0][DMA_EN_GROUP]) < 0)\
            {                                                                  \
                LOG_ERR(" imem map fail at %s\n",#DMA_RX);                     \
                return 1;                                                      \
            }                                                                  \
                                                                               \
            memset((MUINT8*)pBuf[HwModule][cnt][0][DMA_EN_GROUP]->virtAddr,0x0,\
                pBuf[HwModule][cnt][0][DMA_EN_GROUP]->size);                   \
                                                                               \
            LOG_INF("1D-%s 1D-PA:0x%x  1D-VA:0x%x\n",#DMA,                     \
                pBuf[HwModule][cnt][0][DMA_EN_GROUP]->phyAddr,                 \
                pBuf[HwModule][cnt][0][DMA_EN_GROUP]->virtAddr);               \
                                                                               \
            DMASYS##_WRITE_REG(ptr,DMA_RX##_##DMA##_BASE_ADDR,                 \
                pBuf[HwModule][cnt][0][DMA_EN_GROUP]->phyAddr);                \
        }                                                                      \
        else{                                                                  \
            LOG_INF("%s error not 1D-DMAO\n",#DMA);                            \
        }                                                                      \

    #define CAMSV_HEADER_MEM_ALLOC(HwModule,DMA,DMASYS,DMA_FIELD,DMA_EN_ST,DMA_EN_GROUP)\
        MUINT32 _tmp = (DMA_EN_ST & CAMSV_CTL_##DMA_FIELD##_EN_);              \
        MUINT32 _cnt = 0;                                                      \
        while(_tmp != 0){                                                      \
            _cnt++;                                                            \
            _tmp >>= 1;                                                        \
        }                                                                      \
        pBuf[HwModule][_cnt][1][DMA_EN_GROUP]->size = 4*16;                    \
                                                                               \
        LOG_INF("CAMSV HEADER-%s HEADER-buf allocate: idx:%d, size:64 bytes\n",\
            #DMA_FIELD,_cnt);                                                  \
                                                                               \
        if (pImemDrv->allocVirtBuf(pBuf[HwModule][_cnt][1][DMA_EN_GROUP]) < 0) \
        {                                                                      \
            LOG_ERR(" imem alloc fail at %s\n",#DMA_FIELD);                    \
            return 1;                                                          \
        }                                                                      \
        if (pImemDrv->mapPhyAddr(pBuf[HwModule][_cnt][1][DMA_EN_GROUP]) < 0) { \
            LOG_ERR(" imem map fail at %s\n",#DMA_FIELD);                      \
            return 1;                                                          \
        }                                                                      \
                                                                               \
        LOG_INF("CAMSV HEADER-%s PA:0x%lx VA:0x%lx\n",#DMA_FIELD,              \
                pBuf[HwModule][_cnt][1][DMA_EN_GROUP]->phyAddr,                \
                pBuf[HwModule][_cnt][1][DMA_EN_GROUP]->virtAddr);              \
                                                                               \
        DMASYS##_WRITE_REG(ptr,DMASYS##_##DMA##_FH_BASE_ADDR,                  \
                pBuf[HwModule][_cnt][1][DMA_EN_GROUP]->phyAddr);               \

    //aao/pso/bpci's stride is useless. need to use xsize to cal. total memory size
    #define CAMSV_MEM_ALLOC(Module,DMA,DMASYS,DMA_FIELD,DMA_EN_ST,DMA_EN_GROUP)\
        MUINT32 tmp     = (DMA_EN_ST & CAMSV_CTL_##DMA_FIELD##_EN_);           \
        MUINT32 _dma_en = (DMA_EN_ST & CAMSV_CTL_##DMA_FIELD##_EN_);           \
        MUINT32 cnt = 0;                                                       \
        while(tmp != 0){                                                       \
            cnt++;                                                             \
            tmp >>= 1;                                                         \
        }                                                                      \
        if(0/*(_dma_en & IMGO_EN_)|| (_dma_en & RRZO_EN_)*/){                  \
            MUINT32 __tmp =                                                    \
                (DMASYS##_READ_BITS(ptr,CAMSV_##DMA##_XSIZE,XSIZE) + 1 );      \
                                                                               \
            DMASYS##_WRITE_BITS(ptr,CAMSV_##DMA##_STRIDE,STRIDE,__tmp);        \
                                                                               \
            pBuf[Module][cnt][0][DMA_EN_GROUP]->size =                         \
                ((DMASYS##_READ_BITS(ptr,CAMSV_##DMA##_YSIZE,YSIZE)+1) *       \
                 (DMASYS##_READ_BITS(ptr,CAMSV_##DMA##_STRIDE,STRIDE)));       \
        }                                                                      \
        else{                                                                  \
            if(0/* (_dma_en & BPCI_EN_)||(_dma_en & TSFSO_R1_EN_)||            \
                   (_dma_en & AAO_EN_) */){                                    \
                LOG_INF("%s 1D-DMA using xsize instead of stride",#DMA);       \
                                                                               \
                pBuf[Module][cnt][0][DMA_EN_GROUP]->size =                     \
                    ((DMASYS##_READ_BITS(ptr,CAMSV_##DMA##_YSIZE,YSIZE)+1) *   \
                     (DMASYS##_READ_BITS(ptr,CAMSV_##DMA##_XSIZE,XSIZE)+1));   \
            }                                                                  \
            else{                                                              \
                pBuf[Module][cnt][0][DMA_EN_GROUP]->size =                     \
                    ((DMASYS##_READ_BITS(ptr,CAMSV_##DMA##_YSIZE,YSIZE)+1) *   \
                     (DMASYS##_READ_BITS(ptr,CAMSV_##DMA##_STRIDE,STRIDE)));   \
            }                                                                  \
        }                                                                      \
        LOG_INF("CAMSV-%s buf allocate: idx:%d, size:%d bytes\n",              \
            #DMA,cnt,pBuf[Module][cnt][0][DMA_EN_GROUP]->size);                \
                                                                               \
        if (pImemDrv->allocVirtBuf(pBuf[Module][cnt][0][DMA_EN_GROUP]) < 0) {  \
            LOG_ERR(" CAMSV imem alloc fail at %s\n",#DMA);                    \
            return 1;                                                          \
        }                                                                      \
        if (pImemDrv->mapPhyAddr(pBuf[Module][cnt][0][DMA_EN_GROUP]) < 0) {    \
            LOG_ERR(" CAMSV imem map fail at %s\n",#DMA);                      \
            return 1;                                                          \
        }                                                                      \
        if(0/*(_dma_en & IMGO_EN_)|| (_dma_en & RRZO_EN_)*/){                  \
        }                                                                      \
        else{                                                                  \
            memset((MUINT8*)pBuf[Module][cnt][0][DMA_EN_GROUP]->virtAddr,      \
                0x0,pBuf[Module][cnt][0][DMA_EN_GROUP]->size);                 \
        }                                                                      \
                                                                               \
        LOG_INF("CAMSV-%s PA(value_address):0x%lx VA(value_address):0x%lx "    \
                "Module:%d cnt:%d\n ",                                         \
                #DMA,                                                          \
                pBuf[Module][cnt][0][DMA_EN_GROUP]->phyAddr,                   \
                pBuf[Module][cnt][0][DMA_EN_GROUP]->virtAddr,                  \
                Module,                                                        \
                cnt);                                                          \
                                                                               \
        DMASYS##_WRITE_REG(ptr,CAMSV_##DMA##_BASE_ADDR,                        \
            pBuf[Module][cnt][0][DMA_EN_GROUP]->phyAddr);                      \



    pImemDrv = IMemDrv::createInstance();
    if(pImemDrv->init() < 0){
        LOG_ERR(" imem init fail\n");
        return 1;
    }

    if (pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr &&
        pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength) {
        MUINT32 cq0_base_addr[3];
        IspDrv* drvPtr[3];

        drvPtr[0] = drvPtr[1] = drvPtr[2] = NULL;
        cq0_base_addr[0]= cq0_base_addr[1] = cq0_base_addr[2] = 0;

        pInputInfo->CQImemBuf.size =
            pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength;

        if (pImemDrv->allocVirtBuf(&pInputInfo->CQImemBuf) < 0) {
            LOG_ERR(" imem alloc fail at %s\n", "CQI");
            return 1;
        }
        if (pImemDrv->mapPhyAddr(&pInputInfo->CQImemBuf) < 0) {
            LOG_ERR(" imem map fail at %s\n", "CQI");
            return 1;
        }

        LOG_INF("CQI srcTable=%p size=%d pa=%lx, va=%lx\n",
            pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr,
            pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength,
            pInputInfo->CQImemBuf.phyAddr, pInputInfo->CQImemBuf.virtAddr);

        RelocateCqTableBase(
            (unsigned int*)pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr,
            pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength,
            0x3000,
            pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr,//0x56c11800,
            pInputInfo->CQImemBuf.phyAddr,
            (unsigned int*)pInputInfo->CQImemBuf.virtAddr);

        for (MUINT32 c = 0;
            (c < _raw_num) && (c < sizeof(CamIdMap)/sizeof(CamIdMap[0]));
            c++) {
            drvPtr[c] = ((ISP_DRV_CAM**)_ptr)[CamIdMap[c]]->getPhyObj();

            cq0_base_addr[c] =
                CAM_READ_REG(drvPtr[c],CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR);
        }

        LOG_INF("CQ0_BASE A/B/C : 0x%x/0x%x/0x%x\n",
            cq0_base_addr[0], cq0_base_addr[1], cq0_base_addr[2]);

        for (MUINT32 c = 0;
            (c < _raw_num) && (c < sizeof(CamIdMap)/sizeof(CamIdMap[0]));
            c++) {
            if (CAM_READ_BITS(drvPtr[c],CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
                    CAMCQ_CQ_THR0_EN)) {
                MUINT32 _offset =
                    (cq0_base_addr[c]-
                     pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr);

                CAM_WRITE_REG(drvPtr[c],CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,
                    pInputInfo->CQImemBuf.phyAddr+_offset);

                LOG_INF("update cam_%c cq0_base offset = 0x%x,"
                    "new_base = 0x%x\n",
                    'a'+c, _offset, CAM_READ_REG(drvPtr[c],
                    CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR));

                if (CAM_READ_BITS(drvPtr[c],
                        CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
                        CAMCQ_CQ_THR0_MODE) == 1/*immediate*/) {
                    LOG_INF("Trig cam_%c CQ0 \n", 'a'+c);
                    CAM_WRITE_BITS(drvPtr[c], CAMCTL_R1_CAMCTL_START,
                        CAMCTL_CQ_THR0_START, 1);
                }
            }
        }

        LOG_INF("Wait CQ trig done, press to continue enable VF/RAWI ...\n");
        getchar();
    }

    // Camsv DMAOs allocate Buffers
    for(MUINT32 c = 0;
        ((c < CamsvNum) && (c < (sizeof(CamSVIdMap) / sizeof(CamSVIdMap[0]))));
        c++){
        MUINT32 i    = CamSVIdMap[c];

        LOG_INF("CamsvNum = 0x%x, c = 0x%x, i = 0x%x, _pCamsv=%p, "
                "((IspDrvCamsv**)_pCamsv)[%d]=%p",
                 CamsvNum, c, i, _pCamsv,
                (i-CAM_MAX),
                ((IspDrvCamsv**)_pCamsv)[i-CAM_MAX]);


        ptr          = ((IspDrvCamsv**)_pCamsv)[i-CAM_MAX];

        LOG_INFL("_pCamsv = %p\n"
                 "((IspDrvCamsv**)_pCamsv)[%d] = %p\n"
                 "((IspDrvCamsv**)_pCamsv)[%d] = %p",
                 _pCamsv, c, ((IspDrvCamsv**)_pCamsv)[i-CAM_MAX], c, ptr);

        if ((NULL == ptr) || (NULL == _pCamsv) ||
            (NULL == ((ISP_DRV_CAM**)_pCamsv)[i-CAM_MAX])) {
            return 1;
        }

        CamsvDamEnSt = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);

        LOG_INF("module_%d:  enabled CAMSV_DMA:0x%x\n", i, CamsvDamEnSt);

        if (CamsvDamEnSt & CAMSV_CTL_IMGO_EN_) {
            unsigned int group;
            switch(i) {
                case CAMSV_0:
                    group = DMA_EN_GROUP0;
                    break;
                case CAMSV_1:
                    group = DMA_EN_GROUP1;
                    break;
                default:
                    LOG_INFL("Camsv%d No Support Yet", (i-CAM_MAX));
                    break;
            }
            LOG_INFL("CamsvDamEnSt=0x%x CAMSV_CTL_IMGO_EN_=0x%x "
                     "CamsvDamEnSt & CAMSV_CTL_IMGO_EN_=0x%x ",
                     CamsvDamEnSt, CAMSV_CTL_IMGO_EN_,
                     CamsvDamEnSt & CAMSV_CTL_IMGO_EN_);

            CAMSV_MEM_ALLOC(i, IMGO, CAMSV, IMGO, CamsvDamEnSt, group);
            CAMSV_HEADER_MEM_ALLOC(i, IMGO, CAMSV, IMGO, CamsvDamEnSt, group);

            // CAM_SV0 IMGO baseAddress Must equal CAMSV_1  IMGO BaseAddress
            if(1 == pInputInfo->m_DCIF_ONE_RAWI_R2_CASE){
                MUINT32 tmp     = CamsvDamEnSt & CAMSV_CTL_IMGO_EN_;
                MUINT32 cnt = 0;
                while(tmp != 0){
                    cnt++;
                    tmp >>= 1;
                }
                CAMSV_WRITE_REG(ptr,CAMSV_IMGO_BASE_ADDR,
                    pBuf[CAMSV_0][cnt][0][DMA_EN_GROUP0]->phyAddr);

                LOG_INF("DCIF_ONE_RAWI_R2_CASE:"
                        "camsv_%d CAMSV_IMGO_BASE_ADDR=0x%x",
                        c, CAMSV_READ_REG(ptr, CAMSV_IMGO_BASE_ADDR));
            }
        }

        if (CamsvDamEnSt & CAMSV_CTL_UFE_EN_)
        {
            unsigned int group;
            switch(i) {
                case CAMSV_0:
                    group = DMA_EN_GROUP0;
                    break;
                case CAMSV_1:
                    group = DMA_EN_GROUP1;
                    break;
                default:
                    LOG_INFL("Camsv%d UFE No Support Yet", (i-CAM_MAX));
                    break;
            }
            LOG_INFL("CamsvDamEnSt=0x%x CAMSV_CTL_UFE_EN_=0x%x "
                     "CamsvDamEnSt & CAMSV_CTL_UFE_EN_=0x%x ",
                     CamsvDamEnSt, CAMSV_CTL_UFE_EN_,
                     CamsvDamEnSt & CAMSV_CTL_UFE_EN_);

            CAMSV_MEM_ALLOC(i, UFEO, CAMSV, UFE, CamsvDamEnSt, group);
            CAMSV_HEADER_MEM_ALLOC(i, UFEO, CAMSV, UFE, CamsvDamEnSt, group);

             // CAM_SV0 UFEO baseAddress Must equal CAMSV_1 UFEO BaseAddress
            if(1 == pInputInfo->m_DCIF_ONE_RAWI_R2_CASE) {
                MUINT32 tmp     = CamsvDamEnSt & CAMSV_CTL_UFE_EN_;
                MUINT32 cnt = 0;
                while(tmp != 0){
                    cnt++;
                    tmp >>= 1;
                }
                CAMSV_WRITE_REG(ptr, CAMSV_UFEO_BASE_ADDR,
                    pBuf[CAMSV_0][cnt][0][DMA_EN_GROUP0]->phyAddr);

                LOG_INF("DCIF_ONE_RAWI_R2_CASE:"
                        "camsv_%d CAMSV_UFEO_BASE_ADDR=0x%x",
                        c, CAMSV_READ_REG(ptr, CAMSV_UFEO_BASE_ADDR));
            }
        }
    }

    MUINT32 camsvInx = 0;
    // Cam DMAs allocate Buffers
    for (MUINT32 c = 0, camsvInx = 0;
         (c < _raw_num) && (c < sizeof(CamIdMap)/sizeof(CamIdMap[0])),
         ((camsvInx < CamsvNum) &&
         (camsvInx < (sizeof(CamSVIdMap) / sizeof(CamSVIdMap[0]))));
         c++, camsvInx++){
        MUINT32 i         = CamIdMap[c];
        MUINT32 camsvName = CamSVIdMap[camsvInx];

        ptr           = ((ISP_DRV_CAM**)_ptr)[i]->getPhyObj();

        LOG_INFL("_ptr = %p\n"
                 "((ISP_DRV_CAM**)_ptr)[%d] = %p\n"
                 "((ISP_DRV_CAM**)_ptr)[%d]->getPhyObj()= %p",
                     _ptr, i, ((IspDrvCamsv**)_ptr)[i], i, ptr);

        if ((NULL == ptr) || (NULL == _ptr) ||
            (NULL == ((ISP_DRV_CAM**)_ptr)[i])) {
            return 1;
        }

        CamDmaEnSt    = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
        CamDma2EnSt   = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

        LOG_INF("module_%d:  enabled DMA:0x%x, enabled DMA2:0x%x\n",
            i, CamDmaEnSt, CamDma2EnSt);


        if (CamDmaEnSt & IMGO_R1_EN_) {
            MEM_ALLOC(i,IMGO,CAM,IMGO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,IMGO,CAM,IMGO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & LTMSO_R1_EN_) {
            MEM_ALLOC(i,LTMSO,CAM,LTMSO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,LTMSO,CAM,LTMSO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & RRZO_R1_EN_) {
            MEM_ALLOC(i,RRZO,CAM,RRZO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,RRZO,CAM,RRZO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & LCESO_R1_EN_) {
            MEM_ALLOC(i,LCESO,CAM,LCESO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,LCESO,CAM,LCESO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & AAO_R1_EN_) {
            MEM_ALLOC(i,AAO,CAM,AAO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,AAO,CAM,AAO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & FLKO_R1_EN_) {
            MEM_ALLOC(i,FLKO,CAM,FLKO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,FLKO,CAM,FLKO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & UFEO_R1_EN_) {
            MEM_ALLOC(i,UFEO,CAM,UFEO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,UFEO,CAM,UFEO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & AFO_R1_EN_) {
            MEM_ALLOC(i,AFO,CAM,AFO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,AFO,CAM,AFO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & UFGO_R1_EN_) {
            MEM_ALLOC(i,UFGO,CAM,UFGO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,UFGO,CAM,UFGO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & RSSO_R1_EN_) {
            MEM_ALLOC(i,RSSO,CAM,RSSO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,RSSO,CAM,RSSO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & LMVO_R1_EN_) {
            MEM_ALLOC(i,LMVO,CAM,LMVO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,LMVO,CAM,LMVO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & YUVBO_R1_EN_) {
            MEM_ALLOC(i,YUVBO,CAM,YUVBO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,YUVBO,CAM,YUVBO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & TSFSO_R1_EN_) {
            MEM_ALLOC(i,TSFSO,CAM,TSFSO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,TSFSO,CAM,TSFSO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & PDO_R1_EN_) {
            MEM_ALLOC(i,PDO,CAM,PDO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,PDO,CAM,PDO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & MQEO_R1_EN_) {
            //MEM_ALLOC(i, MQEO, CAM, MQEO_R1, CamDmaEnSt);
            //HEADER_MEM_ALLOC(i, MQEO, CAM, MQEO_R1, CamDmaEnSt);
        }

        if (CamDmaEnSt & MAEO_R1_EN_) {
            //MEM_ALLOC(i, MAEO, CAM, MAEO_R1, CamDmaEnSt);
            //HEADER_MEM_ALLOC(i, MAEO, CAM, MAEO_R1, CamDmaEnSt);
        }

        if (CamDmaEnSt & CRZO_R1_EN_) {
            MEM_ALLOC(i,CRZO,CAM,CRZO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,CRZO,CAM,CRZO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & CRZBO_R1_EN_) {
            MEM_ALLOC(i,CRZBO,CAM,CRZBO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,CRZBO,CAM,CRZBO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }

        if (CamDmaEnSt & BPCO_R1_EN_) {
            //MEM_ALLOC(i, BPCO, CAM, BPCO_R1, CamDmaEnSt);
            //HEADER_MEM_ALLOC(i, BPCO, CAM, BPCO_R1, CamDmaEnSt);
        }

        if (CamDmaEnSt & LHSO_R1_EN_) {
            //MEM_ALLOC(i, LHSO, CAM, LHSO_R1, CamDmaEnSt);
            //HEADER_MEM_ALLOC(i, LHSO, CAM, LHSO_R1, CamDmaEnSt);
        }

        if (CamDmaEnSt & YUVCO_R1_EN_) {
            MEM_ALLOC(i,YUVCO,CAM,YUVCO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,YUVCO,CAM,YUVCO_R1,CamDmaEnSt,DMA_EN_GROUP0);
        }
        //CamDma2EnSt
        if (CamDma2EnSt & LCESHO_R1_EN_) {
            //MEM_ALLOC(i, LCESHO, CAM, LCESHO_R1, CamDma2EnSt);
            //HEADER_MEM_ALLOC(i, LCESHO, CAM,LCESHO_R1, CamDma2EnSt);
        }

        if (CamDma2EnSt & CRZO_R2_EN_) {
            MEM_ALLOC(i,CRZO,CAM,CRZO_R2,CamDma2EnSt,DMA_EN_GROUP1);
            HEADER_MEM_ALLOC(i,CRZO,CAM,CRZO_R2,CamDma2EnSt,DMA_EN_GROUP1);
        }

        if (CamDma2EnSt & CRZBO_R2_EN_) {
            MEM_ALLOC(i, CRZBO, CAM, CRZBO_R2, CamDma2EnSt, DMA_EN_GROUP1);
            HEADER_MEM_ALLOC(i, CRZBO, CAM, CRZBO_R2, CamDma2EnSt, DMA_EN_GROUP1);
        }

        if (CamDma2EnSt & RSSO_R2_EN_) {
            MEM_ALLOC(i,RSSO,CAM,RSSO_R2,CamDma2EnSt,DMA_EN_GROUP1);
            HEADER_MEM_ALLOC(i,RSSO,CAM,RSSO_R2,CamDma2EnSt,DMA_EN_GROUP1);
        }

        if (CamDma2EnSt & MLSCI_R1_EN_) {
            //MEM_ALLOC(i, MLSCI,CAM, MLSCI_R1, CamDma2EnSt, DMA_EN_GROUP1);
            //HEADER_MEM_ALLOC(i, MLSCI, CAM,MLSCI_R1, DMA_EN_GROUP1);
        }

        if (CamDma2EnSt & YUVO_R1_EN_) {
            MEM_ALLOC(i,YUVO,CAM,YUVO_R1,CamDma2EnSt,DMA_EN_GROUP1);
            HEADER_MEM_ALLOC(i,YUVO,CAM,YUVO_R1,CamDma2EnSt,DMA_EN_GROUP1);
        }

        #if 0
        if (CamDmaEnSt & UFEO_R1_EN_) {
            MUINT32 _orgxsize = CAM_READ_BITS(ptr,
                    UFEO_R1_UFEO_XSIZE,UFEO_XSIZE);

            MUINT32 _orgstride =  CAM_READ_BITS(ptr,
                    UFEO_R1_UFEO_STRIDE,UFEO_STRIDE);

            CAM_WRITE_BITS(ptr,UFEO_R1_UFEO_XSIZE,UFEO_XSIZE,_orgstride-1);
            MEM_ALLOC(i,UFEO,CAM,UFEO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            HEADER_MEM_ALLOC(i,UFEO,CAM,UFEO_R1,CamDmaEnSt,DMA_EN_GROUP0);
            CAM_WRITE_BITS(ptr,UFEO_R1_UFEO_XSIZE,UFEO_XSIZE,_orgxsize);
        }
        #endif

        if (CamDmaEnSt & RAWI_R2_EN_) {
            if (pInputInfo->m_DCIF == 1) { //DCIF case
                MUINT32 BASE_ADDR1 = 0;
                MUINT32 tmp = (CamsvDamEnSt & CAMSV_CTL_IMGO_EN_), cnt = 0;
                while (tmp != 0) {
                   cnt++;
                   tmp >>= 1;
                }
                CAM_WRITE_REG(ptr, RAWI_R2_RAWI_BASE_ADDR,
                    pBuf[camsvName][cnt][0][DMA_EN_GROUP0]->phyAddr);

                BASE_ADDR1 = CAM_READ_REG(ptr, RAWI_R2_RAWI_BASE_ADDR);

                LOG_INF("RAWI_R2_RAWI_BASE_ADDR=(0x%x) "
                        "pBuf[%d][%d][0]->phyAddr=(0x%lx) \n",
                        BASE_ADDR1, camsvName, cnt,
                        pBuf[camsvName][cnt][0][DMA_EN_GROUP0]->phyAddr);
            } else {

                if (pInputInfo->DmaiTbls[i][_DMAI_TBL_RAWI_R2].pTblAddr &&
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_RAWI_R2].tblLength) {
                    MUINT32 tmp = (CamDmaEnSt & RAWI_R2_EN_), cnt = 0;
                    while (tmp != 0) {
                        cnt++;
                        tmp >>= 1;
                    }

                    pBuf[i][cnt][0][DMA_EN_GROUP0]->size =
                        pInputInfo->DmaiTbls[i][_DMAI_TBL_RAWI_R2].tblLength;

                    if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0)
                    {
                        LOG_ERR(" imem alloc fail at %s\n", "BPCI_R1");
                        return 1;
                    }
                    if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0) {
                        LOG_ERR(" imem map fail at %s\n", "BPCI_R1");
                        return 1;
                    }

                    CAM_WRITE_REG(ptr,RAWI_R2_RAWI_BASE_ADDR,
                        pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr);

                    LOG_INF("RAW_R2 srcTable=%p size=%d pa=0x%lx, va=0x%lx\n",
                        pInputInfo->DmaiTbls[i][_DMAI_TBL_RAWI_R2].pTblAddr,
                        pInputInfo->DmaiTbls[i][_DMAI_TBL_RAWI_R2].tblLength,
                        pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr,
                        pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr);

                    LOG_INF("RAWI_R2 xsize=0x%x ysize=0x%x stride=0x%x\n",
                        CAM_READ_BITS(ptr, RAWI_R2_RAWI_XSIZE, RAWI_XSIZE),
                        CAM_READ_BITS(ptr, RAWI_R2_RAWI_YSIZE, RAWI_YSIZE),
                        CAM_READ_BITS(ptr, RAWI_R2_RAWI_STRIDE, RAWI_STRIDE));

                    //EigerE2-MUSTFIX: LOG_INF("PDC_EN:%d,  PDC_OUT:%d\n",
                    //EigerE2-MUSTFIX:  CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN),
                    //EigerE2-MUSTFIX:  CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_OUT));

                    memcpy((MUINT8*)pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr,
                        (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_RAWI_R2].pTblAddr,
                        pInputInfo->DmaiTbls[i][_DMAI_TBL_RAWI_R2].tblLength);
                }
                else {
                    LOG_ERR("RAW_R2 table not found va(%p) size(%d)!!\n",
                        pInputInfo->DmaiTbls[i][_DMAI_TBL_RAWI_R2].pTblAddr,
                        pInputInfo->DmaiTbls[i][_DMAI_TBL_RAWI_R2].tblLength);
                    LOG_ERR("press to continue...\n");
                    getchar();
                }
            }
        }

        if (CamDmaEnSt & BPCI_R1_EN_) {
            #if 1
            if (pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R1].pTblAddr &&
                pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R1].tblLength) {
                MUINT32 tmp = (CamDmaEnSt & BPCI_R1_EN_), cnt = 0;
                while (tmp != 0) {
                    cnt++;
                    tmp >>= 1;
                }

                pBuf[i][cnt][0][DMA_EN_GROUP0]->size =
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R1].tblLength;

                if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0)
                {
                    LOG_ERR(" imem alloc fail at %s\n", "BPCI_R1");
                    return 1;
                }
                if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0) {
                    LOG_ERR(" imem map fail at %s\n", "BPCI_R1");
                    return 1;
                }

                CAM_WRITE_REG(ptr,BPCI_R1_BPCI_BASE_ADDR,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr);

                LOG_INF("BPCI_R1 srcTable=%p size=%d pa=0x%lx, va=0x%lx\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R1].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R1].tblLength,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr);

                LOG_INF("BPCI_R1 xsize=0x%x ysize=0x%x stride=0x%x\n",
                    CAM_READ_BITS(ptr, BPCI_R1_BPCI_XSIZE, BPCI_XSIZE),
                    CAM_READ_BITS(ptr, BPCI_R1_BPCI_YSIZE, BPCI_YSIZE),
                    CAM_READ_BITS(ptr, BPCI_R1_BPCI_STRIDE, BPCI_STRIDE));

                //EigerE2-MUSTFIX: LOG_INF("PDC_EN:%d,  PDC_OUT:%d\n",
                //EigerE2-MUSTFIX:  CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN),
                //EigerE2-MUSTFIX:  CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_OUT));

                memcpy((MUINT8*)pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr,
                    (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R1].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R1].tblLength);
            }
            else {
                LOG_ERR("BPCI_R1 table not found va(%p) size(%d)!!\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R1].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R1].tblLength);
                LOG_ERR("press to continue...\n");
                getchar();
            }

            #else
            MUINT32 table_size = 0;
            MUINT8* pTable = NULL;
            MUINT32 _orgsize = CAM_READ_BITS(ptr,CAM_BPCI_XSIZE,XSIZE);
            //overwrite bpci dma size
            if(CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN)){
                //#include "Emulation/table/pdo_bpci_table.h" //this table is from MT6797 test code
                #include "Emulation/table/bpci.h"
                LOG_INF("using pdo_bpci_table(release from DE)\n");
                table_size = sizeof(bpci_array_s5k2x8)/sizeof(MUINT8);
                pTable = (MUINT8*)bpci_array_s5k2x8;
                LOG_INF("bpci_array_s5k2x8=%p size=%d\n", bpci_array_s5k2x8, table_size);
            }
            else{
                #include "Emulation/table/bpci_table.h"
                LOG_INF("using bpci_table\n");
                table_size = sizeof(g_bpci_array)/sizeof(MUINT8);
                pTable = (MUINT8*)g_bpci_array;
                LOG_INF("g_bpci_array=%p %d\n", g_bpci_array, table_size);
            }
            CAM_WRITE_BITS(ptr,CAM_BPCI_XSIZE,XSIZE,table_size-1);//this is for buf-allocate


            MEM_ALLOC(i,BPCI,CAM,BPCI);
            if (CAM_READ_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN) == 1) {
                CAM_WRITE_BITS(ptr,CAM_BPCI_XSIZE,XSIZE,_orgsize);
                LOG_INF("Twin cases need write-back xsize: BPCI\n");
            }
            else {
                LOG_INF("Non-twin cases no need write-back xsize: BPCI\n");
            }
            LOG_INF("PDC_EN:%d,  PDC_OUT:%d\n",CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN),CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_OUT));
            memcpy((MUINT8*)pBuf[i][cnt][0]->virtAddr,pTable,table_size);
            LOG_INF("BPCI table=%p size=%d pa=0x%x va=%p\n",
                pTable, table_size, pBuf[i][cnt][0]->phyAddr, (MUINT32 *)pBuf[i][cnt][0]->virtAddr);
            #endif

        }

        if (CamDmaEnSt & BPCI_R2_EN_) {
            #if 1
            if (pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R2].pTblAddr &&
                pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R2].tblLength) {
                MUINT32 tmp = (CamDmaEnSt & BPCI_R2_EN_), cnt = 0;
                while (tmp != 0) {
                    cnt++;
                    tmp >>= 1;
                }

                pBuf[i][cnt][0][DMA_EN_GROUP0]->size =
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R2].tblLength;

                if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0)
                {
                    LOG_ERR(" imem alloc fail at %s\n", "BPCI_R2");
                    return 1;
                }
                if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0) {
                    LOG_ERR(" imem map fail at %s\n", "BPCI_R2");
                    return 1;
                }

                CAM_WRITE_REG(ptr,BPCI_R2_BPCI_BASE_ADDR,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr);

                LOG_INF("BPCI_R2 srcTable=%p size=%d pa=0x%lx, va=0x%lx\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R2].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R2].tblLength,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr);

                LOG_INF("BPCI_R2 xsize=0x%x ysize=0x%x stride=0x%x\n",
                    CAM_READ_BITS(ptr, BPCI_R2_BPCI_XSIZE, BPCI_XSIZE),
                    CAM_READ_BITS(ptr, BPCI_R2_BPCI_YSIZE, BPCI_YSIZE),
                    CAM_READ_BITS(ptr, BPCI_R2_BPCI_STRIDE, BPCI_STRIDE));

                //EigerE2-MUSTFIX: LOG_INF("PDC_EN:%d,  PDC_OUT:%d\n",
                //EigerE2-MUSTFIX:     CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN),
                //EigerE2-MUSTFIX:     CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_OUT));

                memcpy((MUINT8*)pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr,
                    (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R2].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R2].tblLength);
            }
            else {
                LOG_ERR("BPCI_R2 table not found va(%p) size(%d)!!\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R2].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI_R2].tblLength);

                LOG_ERR("press to continue...\n");
                getchar();
            }

            #else
            MUINT32 table_size = 0;
            MUINT8* pTable = NULL;
            MUINT32 _orgsize = CAM_READ_BITS(ptr,CAM_BPCI_XSIZE,XSIZE);
            //overwrite bpci dma size
            if(CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN)){
                //#include "Emulation/table/pdo_bpci_table.h" //this table is from MT6797 test code
                #include "Emulation/table/bpci.h"
                LOG_INF("using pdo_bpci_table(release from DE)\n");
                table_size = sizeof(bpci_array_s5k2x8)/sizeof(MUINT8);
                pTable = (MUINT8*)bpci_array_s5k2x8;
                LOG_INF("bpci_array_s5k2x8=%p size=%d\n", bpci_array_s5k2x8, table_size);
            }
            else{
                #include "Emulation/table/bpci_table.h"
                LOG_INF("using bpci_table\n");
                table_size = sizeof(g_bpci_array)/sizeof(MUINT8);
                pTable = (MUINT8*)g_bpci_array;
                LOG_INF("g_bpci_array=%p %d\n", g_bpci_array, table_size);
            }
            CAM_WRITE_BITS(ptr,CAM_BPCI_XSIZE,XSIZE,table_size-1);//this is for buf-allocate


            MEM_ALLOC(i,BPCI,CAM,BPCI);
            if (CAM_READ_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN) == 1) {
                CAM_WRITE_BITS(ptr,CAM_BPCI_XSIZE,XSIZE,_orgsize);
                LOG_INF("Twin cases need write-back xsize: BPCI\n");
            }
            else {
                LOG_INF("Non-twin cases no need write-back xsize: BPCI\n");
            }
            LOG_INF("PDC_EN:%d,  PDC_OUT:%d\n",CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN),CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_OUT));
            memcpy((MUINT8*)pBuf[i][cnt][0]->virtAddr,pTable,table_size);
            LOG_INF("BPCI table=%p size=%d pa=0x%x va=%p\n",
                pTable, table_size, pBuf[i][cnt][0]->phyAddr, (MUINT32 *)pBuf[i][cnt][0]->virtAddr);
            #endif
        }

        if (CamDmaEnSt & LSCI_R1_EN_) {
            #if 1
            if (pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI_R1].pTblAddr &&
                pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI_R1].tblLength) {
                MUINT32 tmp = (CamDmaEnSt & LSCI_R1_EN_), cnt = 0;
                while (tmp != 0) {
                    cnt++;
                    tmp >>= 1;
                }

                pBuf[i][cnt][0][DMA_EN_GROUP0]->size =
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI_R1].tblLength;

                if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0)
                {
                    LOG_ERR(" imem alloc fail at %s\n", "LSCI");
                    return 1;
                }
                if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0) {
                    LOG_ERR(" imem map fail at %s\n", "LSCI");
                    return 1;
                }

                CAM_WRITE_REG(ptr,LSCI_R1_LSCI_BASE_ADDR,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr);

                LOG_INF("LSCI srcTable=%p size=%d pa=0x%lx, va=0x%lx\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI_R1].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI_R1].tblLength,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr);

                LOG_INF("LSCI xsize=0x%x ysize=0x%x stride=0x%x\n",
                    CAM_READ_BITS(ptr, LSCI_R1_LSCI_XSIZE, LSCI_XSIZE),
                    CAM_READ_BITS(ptr, LSCI_R1_LSCI_YSIZE, LSCI_YSIZE),
                    CAM_READ_BITS(ptr, LSCI_R1_LSCI_STRIDE, LSCI_STRIDE));

                memcpy((MUINT8*)pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr,
                    (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI_R1].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI_R1].tblLength);
            }
            else {
                LOG_ERR("LSCI table not found va(%p) size(%d)!!\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI_R1].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI_R1].tblLength);

                LOG_ERR("press to continue...\n");
                getchar();
            }
            #else
            MUINT32 table_size = 0;
            MUINT32 _orgsize = CAM_READ_BITS(ptr,CAM_LSCI_XSIZE,XSIZE);
            //table size is don't case, pipeline won't be crashed by table content
            #include "Emulation/table/lsc_table.h"
            table_size = sizeof(g_lsci_array)/sizeof(MUINT8);
            table_size = ( table_size + (CAM_READ_BITS(ptr,CAM_LSCI_YSIZE,YSIZE)) ) / (CAM_READ_BITS(ptr,CAM_LSCI_YSIZE,YSIZE) + 1);
            CAM_WRITE_BITS(ptr,CAM_LSCI_XSIZE,XSIZE,table_size-1);

            MEM_ALLOC(i,LSCI,CAM,LSCI);
            if (CAM_READ_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN) == 1) {
                CAM_WRITE_BITS(ptr,CAM_LSCI_XSIZE,XSIZE,_orgsize);
                LOG_INF("Twin cases need write-back xsize: LSCI\n");
            }
            else {
                LOG_INF("Non-twin cases no need write-back xsize: LSCI\n");
            }
            LOG_INF("LSCI table=%p size=%d pa=0x%x, va=%p\n",
                g_lsci_array, table_size, pBuf[i][cnt][0]->phyAddr, (MUINT32 *)pBuf[i][cnt][0]->virtAddr);
            memcpy((MUINT8*)pBuf[i][cnt][0]->virtAddr,(MUINT8*)g_lsci_array,sizeof(g_lsci_array));
            #endif
        }

        if (CamDmaEnSt & PDI_R1_EN_) {
          if (pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI_R1].pTblAddr &&
              pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI_R1].tblLength) {
              MUINT32 tmp = (CamDmaEnSt & PDI_R1_EN_), cnt = 0;
              while (tmp != 0) {
                  cnt++;
                  tmp >>= 1;
              }
              pBuf[i][cnt][0][DMA_EN_GROUP0]->size =
                pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI_R1].tblLength;

              if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0) {
                  LOG_ERR(" imem alloc fail at %s\n", "PDI");
                  return 1;
              }
              if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0) {
                  LOG_ERR(" imem map fail at %s\n", "PDI");
                  return 1;
              }

              CAM_WRITE_REG(ptr, PDI_R1_PDI_BASE_ADDR,
                pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr);

              LOG_INF("PDI srcTable=%p size=%d pa=0x%lx, va=%lx\n",
                  pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI_R1].pTblAddr,
                  pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI_R1].tblLength,
                  pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr,
                  pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr);

              LOG_INF("PDI xsize=0x%x ysize=0x%x stride=0x%x\n",
                  CAM_READ_BITS(ptr, PDI_R1_PDI_XSIZE, PDI_XSIZE),
                  CAM_READ_BITS(ptr, PDI_R1_PDI_YSIZE, PDI_YSIZE),
                  CAM_READ_BITS(ptr, PDI_R1_PDI_STRIDE, PDI_STRIDE));

              memcpy((MUINT8*)pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr,
                  (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI_R1].pTblAddr,
                  pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI_R1].tblLength);
            }
            else {
                LOG_ERR("PDI table not found va(%p) size(%d)!!\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI_R1].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI_R1].tblLength);
                LOG_ERR("press to continue...\n");
                getchar();
            }
        }

         if (CamDmaEnSt & UFDI_R2_EN_) {
            if (pInputInfo->m_DCIF == 1) { //DCIF case
            MUINT32 BASE_ADDR1 = 0;
            MUINT32 tmp = (CamsvDamEnSt & CAMSV_CTL_UFE_EN_), cnt = 0;
            while (tmp != 0) {
               cnt++;
               tmp >>= 1;
            }
            CAM_WRITE_REG(ptr, UFDI_R2_UFDI_BASE_ADDR,
                pBuf[camsvName][cnt][0][DMA_EN_GROUP0]->phyAddr);

            BASE_ADDR1 = CAM_READ_REG(ptr, UFDI_R2_UFDI_BASE_ADDR);

            LOG_INF("UFDI_R2_UFDI_BASE_ADDR=(0x%x) "
                    "pBuf[%d][%d][0]->phyAddr=(0x%lx) \n",
                    BASE_ADDR1, camsvName, cnt,
                    pBuf[camsvName][cnt][0][DMA_EN_GROUP0]->phyAddr);
            } else {

              if (pInputInfo->DmaiTbls[i][_DMAI_TBL_UFDI_R2].pTblAddr &&
                  pInputInfo->DmaiTbls[i][_DMAI_TBL_UFDI_R2].tblLength) {
                  MUINT32 tmp = (CamDmaEnSt & UFDI_R2_EN_), cnt = 0;
                  while (tmp != 0) {
                      cnt++;
                      tmp >>= 1;
                  }
                  pBuf[i][cnt][0][DMA_EN_GROUP0]->size =
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_UFDI_R2].tblLength;

                  if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0) {
                      LOG_ERR(" imem alloc fail at %s\n", "PDI");
                      return 1;
                  }
                  if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0][DMA_EN_GROUP0]) < 0) {
                      LOG_ERR(" imem map fail at %s\n", "PDI");
                      return 1;
                  }

                  CAM_WRITE_REG(ptr, UFDI_R2_UFDI_BASE_ADDR,
                    pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr);

                  LOG_INF("UFDI srcTable=%p size=%d pa=0x%lx, va=%lx\n",
                      pInputInfo->DmaiTbls[i][_DMAI_TBL_UFDI_R2].pTblAddr,
                      pInputInfo->DmaiTbls[i][_DMAI_TBL_UFDI_R2].tblLength,
                      pBuf[i][cnt][0][DMA_EN_GROUP0]->phyAddr,
                      pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr);

                  LOG_INF("UFDI xsize=0x%x ysize=0x%x stride=0x%x\n",
                      CAM_READ_BITS(ptr, UFDI_R2_UFDI_XSIZE, UFDI_XSIZE),
                      CAM_READ_BITS(ptr, UFDI_R2_UFDI_YSIZE, UFDI_YSIZE),
                      CAM_READ_BITS(ptr, UFDI_R2_UFDI_STRIDE, UFDI_STRIDE));

                  memcpy((MUINT8*)pBuf[i][cnt][0][DMA_EN_GROUP0]->virtAddr,
                      (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_UFDI_R2].pTblAddr,
                      pInputInfo->DmaiTbls[i][_DMAI_TBL_UFDI_R2].tblLength);
                }
                else {
                    LOG_ERR("UFDI table not found va(%p) size(%d)!!\n",
                        pInputInfo->DmaiTbls[i][_DMAI_TBL_UFDI_R2].pTblAddr,
                        pInputInfo->DmaiTbls[i][_DMAI_TBL_UFDI_R2].tblLength);
                    LOG_ERR("press to continue...\n");
                    getchar();
                }
          }
        }
    }

    #if 0 //Uni
    ptr    = (UniDrvImp*)_pCamsv;
    CamDmaEnSt = UNI_READ_REG(ptr, CAM_UNI_TOP_DMA_EN);
    LOG_INF("module_UNI:  enabled DMA:0x%x\n",CamDmaEnSt);

    if(CamDmaEnSt & RAWI_A_EN_){
        if (pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].pTblAddr &&
            pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].tblLength) {
            MUINT32 tmp = (CamDmaEnSt & RAWI_A_EN_), cnt = 0;
            while (tmp != 0) {
                cnt++;
                tmp >>= 1;
            }

            pInputInfo->rawiImemBuf.size = pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].tblLength;

            if (pImemDrv->allocVirtBuf(&pInputInfo->rawiImemBuf) < 0) {
                LOG_ERR(" imem alloc fail at %s\n", "RAWI");
                return 1;
            }
            if (pImemDrv->mapPhyAddr(&pInputInfo->rawiImemBuf) < 0) {
                LOG_ERR(" imem map fail at %s\n", "RAWI");
                return 1;
            }

            //UNI_WRITE_REG(ptr,CAM_UNI_RAWI_BASE_ADDR,pInputInfo->rawiImemBuf.phyAddr);

            LOG_INF("RAWI srcTable=%p size=%d pa=%p, va=%p\n",
                pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].pTblAddr,
                pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].tblLength,
                pInputInfo->rawiImemBuf.phyAddr, pInputInfo->rawiImemBuf.virtAddr);

            memcpy((MUINT8*)pInputInfo->rawiImemBuf.virtAddr,
                   (MUINT8*)pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].pTblAddr,
                   pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].tblLength);
        }
        else {
            LOG_ERR("RAWI table not found va(%p) size(%d)!!\n",
                    pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].pTblAddr,
                    pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].tblLength);
            LOG_ERR("press to continue...\n");
            getchar();
        }
    }
    #endif

    if (CamsvNum) {
        LOG_INF("Camsv_0x%x before enable Camsv VF\n", (CamsvNum - 1));
        getchar();
        for(MUINT32 c = 0;
        (c < CamsvNum) && (c < (sizeof(CamSVIdMap) / sizeof(CamSVIdMap[0])));
        c++){
            MUINT32 i = CamSVIdMap[c];
            ptr       = ((IspDrvCamsv**)_pCamsv)[i-CAM_MAX];
            CAMSV_WRITE_BITS(ptr, CAMSV_TG_VF_CON, SINGLE_MODE, 0);
            //CAMSV_WRITE_BITS(ptr, CAMSV_TG_VF_CON, SINGLE_MODE, 1);
            CAMSV_WRITE_BITS(ptr, CAMSV_TG_VF_CON, VFDATA_EN, 1);
        }
        LOG_INF("Camsv_0x%x after enable Camsv VF\n", (CamsvNum - 1));
        getchar();
        //if(MTRUE == ) {
        //    MUINT sofstatus = 0;
        //    ptr       = ((IspDrvCamsv**)_ptr)[CAM_A]->getPhyObj();
        //    while (1) {
        //        sofstatus = CAM_READ_BITS(ptr, CAMCTL_R1_CAMCTL_INT_STATUS,
        //                                CAMCTL_SOF_INT_ST);
        //
        //        if (1 == sofstatus ) {
        //            Pattern_Loading_4_AfterCamsvVFOn(_ptr,
        //                    _raw_num, _pCamsv, inputInfo);
        //        }
        //    }
        //}
    } else {
        for (MUINT32 i = 0; i < pInputInfo->tgNum; i++) {
            if(pInputInfo->tg_enable[i] == 1) {
                ptr = ((ISP_DRV_CAM**)_ptr)[CamIdMap[i]]->getPhyObj();

                // for local timestamp from TM
                CAM_WRITE_BITS(ptr, TG_R1_TG_TIME_STAMP_CTL,
                    TG_TG_TIME_STAMP_SEL, 0);

                CAM_WRITE_BITS(ptr, TG_R1_TG_VF_CON, TG_SINGLE_MODE, 0);
                CAM_WRITE_BITS(ptr, TG_R1_TG_VF_CON, TG_VFDATA_EN, 1);
            }
        }

        //enlarge test pattern if pattern size == TG size
        //cam will be fail when pattern size == TG size under continuous mode
        {
            REG_TG_R1_TG_SEN_GRAB_LIN tg;
            ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
            tg.Raw = CAM_READ_REG(ptr, TG_R1_TG_SEN_GRAB_LIN);
            fakedrvsensor.adjustPatternSize(tg.Bits.TG_LIN_E - tg.Bits.TG_LIN_S);
        }
    }
    return 0;
}

MINT32 Pattern_Stop_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
#if 1
    MUINT32 snrAry = 0, DMA_EN = 0;
    IspDrv* ptr = NULL;
    IspDrv* ptrCam[MAX_ISP_HW_MODULE] = {NULL};

    UINT32 CamsvNum = 0;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    (void)length;
    CamsvNum = pInputInfo->m_CamsvNum;
    LOG_INF("Not yet support IRQ in current bitfile!!");
    LOG_INF("press any key to continuous or stop CPU here\n");
    //getchar();
#else
    ISP_WAIT_IRQ_ST irq;
    _pCamsv;linkpath;
    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = SIGNAL_INT;
    irq.Status = SW_PASS1_DON_ST;
    irq.Timeout = 3000;

    if(((ISP_DRV_CAM**)_ptr)[CAM_A]){
        LOG_INF("start wait cam_a sw p1 done\n");
        if(((ISP_DRV_CAM**)_ptr)[CAM_A]->waitIrq(&irq) == MFALSE){
            LOG_ERR(" wait CAM_A p1 done fail\n");
        }
    }
    irq.Timeout = 3000;
    if(((ISP_DRV_CAM**)_ptr)[CAM_B]){
        LOG_INF("start wait cam_b sw p1 done\n");
        if(((ISP_DRV_CAM**)_ptr)[CAM_B]->waitIrq(&irq) == MFALSE){
            LOG_ERR(" wait CAM_B p1 done fail\n");
        }
    }
#endif

    if (CamsvNum) {
        MUINT32 DMA_EN = 0;
        MUINT32 DMA2_EN = 0;


//        ///
//       ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
//       ptrCam[CAM_A] = ptr;
//       ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
//       ptrCam[CAM_B] = ptr;

//       for(MUINT32 hwModule = CAM_A; hwModule <= CAM_B; hwModule++) {
//           LOG_INFL("Before Setting  cam_%c CQ0 Continue Mode \n", 'a'+hwModule);
//           getchar();
//           if (CAM_READ_BITS(ptrCam[hwModule], CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
//                   CAMCQ_CQ_THR0_MODE) != 0 /*continue Mode*/) {
//               LOG_INFL("cam_%c CQ0 continue Mode \n", 'a'+hwModule);
//               CAM_WRITE_BITS(ptrCam[hwModule], CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
//                    CAMCQ_CQ_THR0_MODE, 0);
//           }
//       }

//       ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
//       ptrCam[CAM_A] = ptr;
//       CAM_WRITE_REG(ptr, CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,
//                 gpMultiCamsvCQDescriptorBuf[CAMSV_0][CQ_NORMAL_PAGE1]
//                 ->phyAddr);

//       DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
//       DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

//       LOG_INFL("Change CAM_A CQ0 BaseAddress to CQDescriptor Normal Page1\n");
//       getchar();

//       ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
//       ptrCam[CAM_B] = ptr;
//       CAM_WRITE_REG(ptr, CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,
//                   gpMultiCamsvCQDescriptorBuf[CAMSV_1][CQ_NORMAL_PAGE1]
//                   ->phyAddr);

//       DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
//       DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

//       LOG_INFL("Change CAM_B CQ0 BaseAddress to CQDescriptor Normal Page1\n");
//       LOG_INFL("Before Close TestModel\n");
//       getchar();

        LOG_INF("press any key to wait for Camsv VF OFF(FPGA 90s)\n");
        getchar();
        for(MUINT32 c = 0;
        (c < CamsvNum) && (c < (sizeof(CamSVIdMap) / sizeof(CamSVIdMap[0])));
        c++){
            MUINT32 i = CamSVIdMap[c];
            ptr       = ((IspDrvCamsv**)_pCamsv)[i-CAM_MAX];
            CAMSV_WRITE_BITS(ptr, CAMSV_TG_VF_CON, VFDATA_EN, 0);
        }

        for (MUINT32 i = 0; i < pInputInfo->tgNum; i++) {
            ptr = ((ISP_DRV_CAM**)_ptr)[CamIdMap[i]]->getPhyObj();
            CAM_WRITE_BITS(ptr, TG_R1_TG_VF_CON, TG_VFDATA_EN, 0);
        }
        ///


        LOG_INF("press any key to wait for VF_OFF fetch\n");
        getchar();



        //getchar();
        MUINT32 snrAry[5] = {0,1,2,3,4};
        fakedrvsensor.powerOff("fake0", 1, &snrAry[0]);
        fakedrvsensor.powerOff("fake1", 1, &snrAry[1]);
        fakedrvsensor.powerOff("fake2", 1, &snrAry[2]);
        fakedrvsensor.powerOff("fake3", 1, &snrAry[3]);
        fakedrvsensor.powerOff("fake4", 1, &snrAry[4]);

//        if (pInputInfo->tgNum > 1) {
//            MUINT32 snrArySub = 1;
//            fakedrvsensor.powerOff("fake1", 1, &snrArySub);
//        }
        getchar();
    }
    else {
        //CAM_WRITE_BITS(((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj(),
        //               CAM_TG_VF_CON, VFDATA_EN, 0);

        for (MUINT32 i = 0; i < pInputInfo->tgNum; i++) {
            ptr = ((ISP_DRV_CAM**)_ptr)[CamIdMap[i]]->getPhyObj();
            CAM_WRITE_BITS(ptr, TG_R1_TG_VF_CON, TG_VFDATA_EN, 0);
        }

        LOG_INF("press any key to wait for VF_OFF fetch\n");
        getchar();

        fakedrvsensor.powerOff("fake", 1, &snrAry);
        if (pInputInfo->tgNum > 1) {
            MUINT32 snrArySub = 1;
            fakedrvsensor.powerOff("fake1", 1, &snrArySub);
        }
    }

    return 0;
}

MINT32 Pattern_Loading_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry1 = 0, snrAry4 = 3;
    (void)_ptr;(void)length;(void)_pCamsv;
    return 0;
}

MINT32 Pattern_Loading_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry[5] = {0,1,2,3,4};
    (void)length;
    //
    static const unsigned char bpci_r1a_i_0[] = {
            #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/dat/bpci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].pTblAddr  = bpci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].tblLength =
        (sizeof(bpci_r1a_i_0) / sizeof(bpci_r1a_i_0[0]));

    //
    static const unsigned char bpci_r2a_i_0[] = {
            #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/dat/bpci_r2a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].pTblAddr  = bpci_r2a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].tblLength =
        (sizeof(bpci_r2a_i_0) / sizeof(bpci_r2a_i_0[0]));

    //
    static const unsigned char lsci_r1a_i_0[] = {
            #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/dat/lsci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].pTblAddr  = lsci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].tblLength =
        (sizeof(lsci_r1a_i_0) / sizeof(lsci_r1a_i_0[0]));

    //
    static const unsigned char pdi_r1a_i_0[] = {
            #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/dat/pdi_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].pTblAddr  = pdi_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].tblLength =
        (sizeof(pdi_r1a_i_0) / sizeof(pdi_r1a_i_0[0]));


    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    ptrCam = ptr;
    #include "Emulation/1camsv_cq_pat_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern-CAMSV_0.h"

    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/whole_chip_pattern-CAMA.h"


    const MUINT32 seninf0_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/whole_chip_pattern-SENINF0.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/whole_chip_pattern-SENINF1.h"
        0xdeadbeef
    };

    const MUINT32 seninf2_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF2.h"
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/whole_chip_pattern-SENINF2.h"
        0xdeadbeef
    };

    const MUINT32 seninf3_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF3.h"
        0xdeadbeef
    };

    const MUINT32 seninf4_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF4.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
    fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
    fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
    fakedrvsensor.powerOn("fake3", 1, &snrAry[3], seninf3_golden);
    fakedrvsensor.powerOn("fake4", 1, &snrAry[4], seninf4_golden);

    return 0;
}

MINT32 Pattern_Loading_2(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry[3] = {0,1,2};
    (void)length;(void)_uni;(void)inputInfo;
    //
    static const unsigned char bpci_r1a_i_0[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/dat/bpci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].pTblAddr  = bpci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].tblLength =
        sizeof(bpci_r1a_i_0)/sizeof(bpci_r1a_i_0[0]);

    //
    static const unsigned char bpci_r2a_i_0[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/dat/bpci_r2a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].pTblAddr  = bpci_r2a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].tblLength =
        sizeof(bpci_r2a_i_0)/sizeof(bpci_r2a_i_0[0]);

    //
    static const unsigned char lsci_r1a_i_0[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/dat/lsci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].pTblAddr  = lsci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].tblLength =
        sizeof(lsci_r1a_i_0)/sizeof(lsci_r1a_i_0[0]);

    static const unsigned char pattern_cq_dat_tbl[] = {
            #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/dat/cq_data_0.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr  = pattern_cq_dat_tbl;
    // from image.list in pattern from DE
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr   = 0x54e7ee00;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength =
        (sizeof(pattern_cq_dat_tbl) / sizeof(pattern_cq_dat_tbl[0]));

    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/whole_chip_pattern-CAM_A-CQ.h"

    const MUINT32 seninf0_golden[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/whole_chip_pattern-SENINF0.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/whole_chip_pattern-SENINF1.h"
        0xdeadbeef
    };

    const MUINT32 seninf2_golden[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/whole_chip_pattern-SENINF2.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
    fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
    fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
    return 0;

}


MINT32 Pattern_Loading_3(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry[5] = {0,1,2,3,4};
    IMEM_BUF_INFO ******_pCamsvimgBuf = NULL;
    MUINT32 subSampleNum = 0;
    _pCamsvimgBuf = pInputInfo->m_pCamsvImemBufs;
    ISP_HW_MODULE hwModule = CAMSV_0;
    (void)length;

    //
    static const unsigned char bpci_r1a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/bpci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].pTblAddr  = bpci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].tblLength =
        (sizeof(bpci_r1a_i_0) / sizeof(bpci_r1a_i_0[0]));

    //
    static const unsigned char bpci_r2a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/bpci_r2a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].pTblAddr  = bpci_r2a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].tblLength =
        (sizeof(bpci_r2a_i_0) / sizeof(bpci_r2a_i_0[0]));

    //
    static const unsigned char lsci_r1a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/lsci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].pTblAddr  = lsci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].tblLength =
        (sizeof(lsci_r1a_i_0) / sizeof(lsci_r1a_i_0[0]));

    //
    static const unsigned char pdi_r1a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/pdi_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].pTblAddr  = pdi_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].tblLength =
        (sizeof(pdi_r1a_i_0) / sizeof(pdi_r1a_i_0[0]));


    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/isp_top_single_fbc_pat_whole_chip_pattern_CAM_A.h"


    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];

    LOG_INFL("ptr=%p _pCamsv=%p "
             "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p\n",
             ptr, _pCamsv,
             ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]);

    ptrCam = ptr;
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/isp_top_single_fbc_pat_camsv_whole_chip_pattern_CAMSV0.h"


    subSampleNum = CAMSV_READ_BITS(ptr, CAMSV_SPARE1,
         for_DCIF_subsample_number);

    LOG_INFL("ptr=%p _pCamsv=%p "
             "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p "
             "subSampleNum = %d",
             ptr, _pCamsv,
             ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX],
             subSampleNum);

    //subSampleNum = 8;
    AllocCamsvSubsampleBuffer(_pCamsvimgBuf, hwModule, subSampleNum,
                              DMA_EN_GROUP0, CamsvVF_OFF, (4 * 16 * 2),
                              CAMSV_FBC_ON_GROUP);

    LOG_INFL("ptr=%p _pCamsv=%p "
            "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p\n",
            ptr, _pCamsv,
            ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]);


    typedef const unsigned char (*subsample)[64];

    static const unsigned char subsample_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_0.dat"
    };
    static const unsigned char subsample_1[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_1.dat"
    };
    static const unsigned char subsample_2[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_2.dat"
    };
    static const unsigned char subsample_3[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_3.dat"
    };
    static const unsigned char subsample_4[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_4.dat"
    };
    static const unsigned char subsample_5[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_5.dat"
    };
    static const unsigned char subsample_6[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_6.dat"
    };
    static const unsigned char subsample_7[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_7.dat"
    };
    static const unsigned char subsample_8[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_8.dat"
    };
    static const unsigned char subsample_9[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_9.dat"
    };
    static const unsigned char subsample_10[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_10.dat"
    };
    static const unsigned char subsample_11[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_11.dat"
    };
    static const unsigned char subsample_12[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_12.dat"
    };
    static const unsigned char subsample_13[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_13.dat"
    };
    static const unsigned char subsample_14[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_14.dat"
    };
    static const unsigned char subsample_15[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_15.dat"
    };

    static subsample SUBSUMPLE_TAL[]={
        &subsample_0,  &subsample_1,  &subsample_2,  &subsample_3,
        &subsample_4,  &subsample_5,  &subsample_6,  &subsample_7,
        &subsample_8,  &subsample_9,  &subsample_10, &subsample_11,
        &subsample_12, &subsample_13, &subsample_14, &subsample_15,
        &subsample_0,  &subsample_1,  &subsample_2,  &subsample_3,
        &subsample_4,  &subsample_5,  &subsample_6,  &subsample_7,
        &subsample_8,  &subsample_9,  &subsample_10, &subsample_11,
        &subsample_12, &subsample_13, &subsample_14, &subsample_15,

        };

    for(MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {

        pInputInfo->SubsampleTbls[hwModule][subsampleInx][CAMSV_FBC_ON_GROUP].
            pTblAddr  = ((const unsigned char *)(SUBSUMPLE_TAL[subsampleInx]));

        pInputInfo->SubsampleTbls[hwModule][subsampleInx][CAMSV_FBC_ON_GROUP].
            tblLength = 16;


        LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                 "pTblAddr = %p *pTblAddr = 0x%x tblLength = 0x%x ",
                 hwModule, subsampleInx,
                 pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                 [CAMSV_FBC_ON_GROUP].pTblAddr,
                 *(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                 [CAMSV_FBC_ON_GROUP].pTblAddr),
                 pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                 [CAMSV_FBC_ON_GROUP].tblLength);
    }
    //AllocCamsvSubsampleBufferAndCopyData(pInputInfo, hwModule,subSampleNum);
     enum {
       CAMSV_IMGO_FH_SPARE_1_INX = 0 , CAMSV_IMGO_FH_SPARE_2_INX ,
       CAMSV_IMGO_FH_SPARE_3_INX     , CAMSV_IMGO_FH_SPARE_4_INX ,
       CAMSV_IMGO_FH_SPARE_5_INX     , CAMSV_IMGO_FH_SPARE_6_INX ,
       CAMSV_IMGO_FH_SPARE_7_INX     , CAMSV_IMGO_FH_SPARE_8_INX ,
       CAMSV_IMGO_FH_SPARE_9_INX     , CAMSV_IMGO_FH_SPARE_10_INX,
       CAMSV_IMGO_FH_SPARE_11_INX    , CAMSV_IMGO_FH_SPARE_12_INX,
       CAMSV_IMGO_FH_SPARE_13_INX    , CAMSV_IMGO_FH_SPARE_14_INX,
       CAMSV_IMGO_FH_SPARE_15_INX    , CAMSV_IMGO_FH_SPARE_16_INX,
       CAMSV_TEST_CAM_CQ_ITEM_NUM    ,
    } ECAMSV_TEST_CAM_CQ_ITEM_NUM    ;

    for(MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {
        for(MUINT32 i = CAMSV_IMGO_FH_SPARE_2_INX;
            i < CAMSV_TEST_CAM_CQ_ITEM_NUM; i++) {
          LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                   "Write to CAMSV_IMGO_FH_SPARE_%d = 0x%x\n",
                   hwModule, subsampleInx,
                   (i + 1),
                   *(((MUINT32 *)(pInputInfo
                   ->SubsampleTbls[hwModule][subsampleInx][CAMSV_FBC_ON_GROUP].
                   pTblAddr)) + (i)));

       }

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_2,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_2_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_3,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_3_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_4,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_4_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_5,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_5_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_6,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_6_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_7,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_7_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_8,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_8_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_9,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_9_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_10,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_10_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_11,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_11_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_12,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_12_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_13,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_13_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_14,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_14_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_15,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_15_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_16,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_16_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_BASE_ADDR,
            pInputInfo->m_pCamsvImemBufs[hwModule][subsampleInx][CamsvVF_OFF]
            [DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr);

        getchar();
        CAMSV_WRITE_BITS(ptr, CAMSV_IMGO_FBC, RCNT_INC1, 1);
    }

    const MUINT32 seninf0_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-SENINF0.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-SENINF1.h"
        0xdeadbeef
    };

    const MUINT32 seninf2_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF2.h"
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-SENINF2.h"
        0xdeadbeef
    };

    const MUINT32 seninf3_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF3.h"
        0xdeadbeef
    };

    const MUINT32 seninf4_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF4.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
    fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
    fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
    fakedrvsensor.powerOn("fake3", 1, &snrAry[3], seninf3_golden);
    fakedrvsensor.powerOn("fake4", 1, &snrAry[4], seninf4_golden);

    return 0;
}

MINT32 Pattern_Loading_4(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry[5] = {0,1,2,3,4};
    (void)length;
    MUINT32 subSampleNum = 0;

    _pCamsvImgBuf                = pInputInfo->m_pCamsvImemBufs;
    _pCamsvCQVirtRegBuf          = pInputInfo->m_pCamsvCQVirtualRegisterBuf;
    _pCamsvCQDescriptorBuf       = pInputInfo->m_pCamsvCQDescriptorBuf;
    _pCamsvCQVirtRegFBCFHAddrBuf = pInputInfo->m_pCamsvCQVirRegFBCFHAddrBuf;

    ISP_HW_MODULE                     hwModule = CAMSV_0;
    static MUINT32   camsvImgoFbcRcntInc1Value = 0;
    static MUINT32 *pCamsvImgoFbcRcntInc1Value = &camsvImgoFbcRcntInc1Value;

    //
    static const unsigned char bpci_r1a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/dat/bpci_r1a_i_0.dat"
    };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].pTblAddr  = bpci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].tblLength =
        (sizeof(bpci_r1a_i_0) / sizeof(bpci_r1a_i_0[0]));

    //
    static const unsigned char bpci_r2a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/dat/bpci_r2a_i_0.dat"
    };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].pTblAddr  = bpci_r2a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].tblLength =
        (sizeof(bpci_r2a_i_0) / sizeof(bpci_r2a_i_0[0]));

    //
    static const unsigned char lsci_r1a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/dat/lsci_r1a_i_0.dat"
    };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].pTblAddr  = lsci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].tblLength =
        (sizeof(lsci_r1a_i_0) / sizeof(lsci_r1a_i_0[0]));

    //
    static const unsigned char pdi_r1a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/dat/pdi_r1a_i_0.dat"
    };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].pTblAddr  = pdi_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].tblLength =
        (sizeof(pdi_r1a_i_0) / sizeof(pdi_r1a_i_0[0]));


    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/isp_top_single_fbc_pat_whole_chip_pattern_CAM_A.h"
    if ((NULL == ptr) || (NULL == _ptr) ||
        (NULL == ((ISP_DRV_CAM**)_ptr)[CAM_A])) {
        LOG_INFL("_ptr=%p\n"
                 "((ISP_DRV_CAM**)_ptr)[CAM_A]=%p\n"
                 "((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj()=%p",
                  _ptr, ((ISP_DRV_CAM**)_ptr)[CAM_A], ptr);
        return 1;
    }

    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    ptrCam = ptr;

    if ((NULL == ptr) || (NULL == _pCamsv) ||
        (NULL == ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX])) {
        LOG_INFL("_pCamsv=%p \n"
                 "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p\n"
                 "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p",
                 _pCamsv, ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX], ptr);
        return 1;
    }

    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/camsv_whole_chip_pattern/isp_top_single_fbc_pat_camsv_whole_chip_pattern_CAMSV0.h"

    subSampleNum =
        CAMSV_READ_BITS(ptr, CAMSV_SPARE1, for_DCIF_subsample_number);

    AllocCamsvSubsampleBuffer(_pCamsvImgBuf, hwModule, subSampleNum,
        DMA_EN_GROUP0, CamsvVF_OFF, (4 * 16 * 2), CAMSV_FBC_ON_GROUP);

    AllocCamsvSubsampleBuffer(_pCamsvImgBuf, hwModule, subSampleNum,
        DMA_EN_GROUP0, CamsvVF_OFF, (4 * 16 * 2), CAMSV_FBC_OFF_GROUP);

    LOG_INFL("_pCamsvImgBuf:%p ", _pCamsvImgBuf);

    AllocCamsvSubsampleBuffer(_pCamsvCQVirtRegBuf, hwModule, subSampleNum,
        DMA_EN_GROUP0, CamsvVF_OFF, (4 * 16), CAMSV_FBC_ON_GROUP);

    LOG_INFL("_pCamsvCQVirtRegBuf:%p", _pCamsvCQVirtRegBuf);

    AllocCamsvSubsampleCQDescriptorBuffer("_pCamsvCQDescriptorBuf",
        _pCamsvCQDescriptorBuf , CAMSV_0, 0x4000);

    LOG_INFL("_pCamsvCQDescriptorBuf PA:0x%lx VA:0x%lx SIZE:%d",
        _pCamsvCQDescriptorBuf->phyAddr, _pCamsvCQDescriptorBuf->virtAddr,
        _pCamsvCQDescriptorBuf->size);


    for(MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {

        AllocCamsvSubsampleCQDescriptorBuffer("_pCamsvCQVirtRegFBCFHAddrBuf",
        _pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx] , CAMSV_0, 0x10);

        for(MUINT32 i = CAMSV_FBC_USE_CQ_IMGO_ENQ_ADDR_INX;
            i < CAMSV_FBC_USE_CQ_ITEMS_NUM;
            i++) {
            LOG_INFL("_pCamsvCQVirtRegFBCFHAddrBuf[%d]"
                     " PA_%d:0x%lx VA_%d:0x%lx SIZE:%d",
                subsampleInx,
                i, (_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]->phyAddr + i),
                i, (_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]->virtAddr + i),
                _pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]->size);
        }
    }

    typedef const unsigned char (*subsample)[64];

    static const unsigned char subsample_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_0.dat"
    };
    static const unsigned char subsample_1[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_1.dat"
    };
    static const unsigned char subsample_2[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_2.dat"
    };
    static const unsigned char subsample_3[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_3.dat"
    };
    static const unsigned char subsample_4[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_4.dat"
    };
    static const unsigned char subsample_5[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_5.dat"
    };
    static const unsigned char subsample_6[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_6.dat"
    };
    static const unsigned char subsample_7[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_7.dat"
    };
    static const unsigned char subsample_8[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_8.dat"
    };
    static const unsigned char subsample_9[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_9.dat"
    };
    static const unsigned char subsample_10[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_10.dat"
    };
    static const unsigned char subsample_11[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_11.dat"
    };
    static const unsigned char subsample_12[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_12.dat"
    };
    static const unsigned char subsample_13[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_13.dat"
    };
    static const unsigned char subsample_14[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_14.dat"
    };
    static const unsigned char subsample_15[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_dat/subsample_15.dat"
    };

    static subsample SUBSUMPLE_TAL[] = {
        &subsample_0 , &subsample_1 , &subsample_2 , &subsample_3 ,
        &subsample_4 , &subsample_5 , &subsample_6 , &subsample_7 ,
        &subsample_8 , &subsample_9 , &subsample_10, &subsample_11,
        &subsample_12, &subsample_13, &subsample_14, &subsample_15,
        &subsample_0 , &subsample_1 , &subsample_2 , &subsample_3 ,
        &subsample_4 , &subsample_5 , &subsample_6 , &subsample_7 ,
        &subsample_8 , &subsample_9 , &subsample_10, &subsample_11,
        &subsample_12, &subsample_13, &subsample_14, &subsample_15,
    };

    enum {
         CAMSV_FBC_IMGO_ENQ_HEADER_TIMESTAMP_INX = 0 ,
         CAMSV_FBC_IMGO_ENQ_HEADER_0_INX, CAMSV_FBC_IMGO_ENQ_HEADER_1_INX ,
         CAMSV_FBC_IMGO_ENQ_HEADER_2_INX, CAMSV_FBC_IMGO_ENQ_HEADER_3_INX ,
         CAMSV_FBC_IMGO_ENQ_HEADER_4_INX, CAMSV_FBC_IMGO_ENQ_HEADER_5_INX ,
         CAMSV_FBC_IMGO_ENQ_ADDR_INX    , CAMSV_FBC_IMGO_ENQ_HADDR_INX    ,
         CAMSV_CAMSV_IMGO_FBC_INX       , REG_CAMSV_IMGO_FH_BASE_ADDR_INX ,
         CAMSV_TEST_CAM_CQ_ITEM_NUM     ,
    } ECAMSV_TEST_CAM_CQ_ITEM_NUM;

    for(MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {

        pInputInfo->SubsampleTbls[hwModule][subsampleInx][CAMSV_FBC_ON_GROUP].
            pTblAddr = ((const unsigned char *)(SUBSUMPLE_TAL[subsampleInx]));

        pInputInfo->SubsampleTbls[hwModule][subsampleInx][CAMSV_FBC_ON_GROUP].
            tblLength = 16;

        for(MUINT32 i = CAMSV_FBC_IMGO_ENQ_HEADER_0_INX;
            i <= CAMSV_FBC_IMGO_ENQ_HEADER_5_INX;
            i++) {
            LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                     "pTblAddr_%d = %p *pTblAddr_%d = 0x%x tblLength = 0x%x\n",
                     hwModule, subsampleInx,
                     i,
                     (((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule]
                     [subsampleInx][CAMSV_FBC_ON_GROUP].pTblAddr))+ i),
                     i,
                     *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule]
                     [subsampleInx][CAMSV_FBC_ON_GROUP].pTblAddr))+ i),
                     pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                     [CAMSV_FBC_ON_GROUP].tblLength);
        }
    }

    for(MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {
        CopyCamsvSubsampleTblDataToCQVirtRegBuf(_pCamsvCQVirtRegBuf,
            pInputInfo, hwModule, subsampleInx, CamsvVF_OFF, DMA_EN_GROUP0,
            CAMSV_FBC_ON_GROUP);
    }
    //AllocCamsvSubsampleBufferAndCopyData(pInputInfo, hwModule,subSampleNum);

    MUINT32 write2CQTokenNum = 0;
    for (MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {
        MUINT32 _offset[CAMSV_TEST_CAM_CQ_ITEM_NUM] = {0x0};
        MUINT32 _token[CAMSV_TEST_CAM_CQ_ITEM_NUM]  = {0x0};

        _offset[CAMSV_FBC_IMGO_ENQ_HEADER_TIMESTAMP_INX]  =  0;

        _offset[CAMSV_FBC_IMGO_ENQ_HEADER_0_INX]  =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_0);

        _offset[CAMSV_FBC_IMGO_ENQ_HEADER_1_INX]  =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_1);

        _offset[CAMSV_FBC_IMGO_ENQ_HEADER_2_INX]  =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_2);

        _offset[CAMSV_FBC_IMGO_ENQ_HEADER_3_INX]  =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_3);

        _offset[CAMSV_FBC_IMGO_ENQ_HEADER_4_INX]  =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_4);

        _offset[CAMSV_FBC_IMGO_ENQ_HEADER_5_INX]  =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_5);

        _offset[CAMSV_FBC_IMGO_ENQ_ADDR_INX] =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_FBC_IMGO_ENQ_ADDR);

        _offset[CAMSV_FBC_IMGO_ENQ_HADDR_INX] =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_FBC_IMGO_ENQ_HADDR);

        _offset[REG_CAMSV_IMGO_FH_BASE_ADDR_INX] =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_IMGO_FH_BASE_ADDR);

        _offset[CAMSV_CAMSV_IMGO_FBC_INX] =
            CAMSV_GET_REG_OFFSET(ptr, CAMSV_IMGO_FBC);

        for (MUINT32 i = CAMSV_FBC_IMGO_ENQ_HEADER_0_INX;
            i <= CAMSV_FBC_IMGO_ENQ_HEADER_5_INX; i++) {

             gCamsvCQVirtualRegisterPhyAddr =
                (((MUINT32 *)(_pCamsvCQVirtRegBuf[hwModule][subsampleInx]
                [CamsvVF_OFF][DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr))+(i));

            LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                     "CAMSV_FBC_IMGO_ENQ_HEADER_%d_offset = 0x%x, "
                     "Dram address = 0x%lx Dram Data = 0x%x\n "
                     "CAMSV_FBC_IMGO_ENQ_HEADER_%d_Token = 0x%x",
                     hwModule, subsampleInx,
                     (i - 1), _offset[i],
                     ((_pCamsvCQVirtRegBuf[hwModule]
                     [subsampleInx][CamsvVF_OFF][DMA_EN_GROUP0]
                     [CAMSV_FBC_ON_GROUP]->phyAddr) + i - 1),
                     *((MUINT32 *)(_pCamsvCQVirtRegBuf[hwModule]
                     [subsampleInx][CamsvVF_OFF][DMA_EN_GROUP0]
                     [CAMSV_FBC_ON_GROUP]->virtAddr) + i - 1),
                     (i - 1), DESCRIPTOR_TOKEN(_offset[i], 1, CAMSV_0_BASE_HW));
        }

        LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                 "CAMSV_FBC_IMGO_ENQ_ADDR_INX_offset = 0x%x, "
                 "Dram address = %p Dram Data = 0x%lx\n "
                 "CAMSV_FBC_IMGO_ENQ_ADDR_INX_Token = 0x%x",
                 hwModule, subsampleInx,
                 _offset[CAMSV_FBC_IMGO_ENQ_ADDR_INX],
                 &_pCamsvImgBuf[hwModule][subsampleInx][CamsvVF_OFF]
                 [DMA_EN_GROUP0][CAMSV_FBC_OFF_GROUP]->phyAddr,
                 _pCamsvImgBuf[hwModule][subsampleInx][CamsvVF_OFF]
                 [DMA_EN_GROUP0][CAMSV_FBC_OFF_GROUP]->phyAddr,
                 DESCRIPTOR_TOKEN(_offset[CAMSV_FBC_IMGO_ENQ_ADDR_INX],
                                  1, CAMSV_0_BASE_HW));

        LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                 "REG_CAMSV_IMGO_FH_BASE_ADDR_INX = 0x%x, "
                 "Dram address = %p Dram Data = 0x%lx\n "
                 "REG_CAMSV_IMGO_FH_BASE_ADDR_INX_Token = 0x%x",
                 hwModule, subsampleInx,
                 _offset[REG_CAMSV_IMGO_FH_BASE_ADDR_INX],
                 &_pCamsvImgBuf[hwModule][subsampleInx][CamsvVF_OFF]
                 [DMA_EN_GROUP0][CAMSV_FBC_OFF_GROUP]->phyAddr,
                 _pCamsvImgBuf[hwModule][subsampleInx][CamsvVF_OFF]
                 [DMA_EN_GROUP0][CAMSV_FBC_OFF_GROUP]->phyAddr,
                 DESCRIPTOR_TOKEN(_offset[REG_CAMSV_IMGO_FH_BASE_ADDR_INX],
                                  1, CAMSV_0_BASE_HW));

        LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                 "CAMSV_FBC_IMGO_ENQ_HADDR_INX = 0x%x, "
                 "Dram address = %p Dram Data = 0x%lx\n "
                 "CAMSV_FBC_IMGO_ENQ_HADDR_INX_Token = 0x%x",
                 hwModule, subsampleInx,
                 _offset[CAMSV_FBC_IMGO_ENQ_HADDR_INX],
                 &_pCamsvImgBuf[hwModule][subsampleInx][CamsvVF_OFF]
                 [DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr,
                 _pCamsvImgBuf[hwModule][subsampleInx][CamsvVF_OFF]
                 [DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr,
                 DESCRIPTOR_TOKEN(_offset[CAMSV_FBC_IMGO_ENQ_HADDR_INX],
                                  1, CAMSV_0_BASE_HW));

        LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                 "CAMSV_CAMSV_IMGO_FBC_offset = 0x%x, "
                 "VA=%p DramData = 0x%x\n "
                 "CAMSV_CAMSV_IMGO_FBC_Token = 0x%x ",
                 hwModule, subsampleInx,
                 _offset[CAMSV_CAMSV_IMGO_FBC_INX],
                 pCamsvImgoFbcRcntInc1Value,
                 camsvImgoFbcRcntInc1Value,
                 DESCRIPTOR_TOKEN(_offset[CAMSV_CAMSV_IMGO_FBC_INX],
                                  1, CAMSV_0_BASE_HW));
        getchar();

        //write CAMSV_FBC_IMGO_ENQ_HEADER_X to CamsvCQDescriptorBuf
        _token[CAMSV_FBC_IMGO_ENQ_HEADER_TIMESTAMP_INX] = 0;
        for(MUINT32 i = CAMSV_FBC_IMGO_ENQ_HEADER_0_INX;
            i <= CAMSV_FBC_IMGO_ENQ_HEADER_5_INX ; i++) {

            _token[i] = DESCRIPTOR_TOKEN(_offset[i], 1, CAMSV_0_BASE_HW);

            memcpy((((MUINT32 *)(_pCamsvCQDescriptorBuf->virtAddr)) +
                write2CQTokenNum), &_token[i], sizeof(_token[i]));

            write2CQTokenNum += 1;

            gCamsvCQVirtualRegisterPhyAddr =
                (((MUINT32 *)(_pCamsvCQVirtRegBuf[hwModule][subsampleInx]
                [CamsvVF_OFF][DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr))+
                (i - 1));

            memcpy((((MUINT32 *)(_pCamsvCQDescriptorBuf->virtAddr)) +
                    write2CQTokenNum), &gCamsvCQVirtualRegisterPhyAddr,
                    sizeof(((MUINT32 *)(_pCamsvCQVirtRegBuf[hwModule]
                    [subsampleInx][CamsvVF_OFF][DMA_EN_GROUP0]
                    [CAMSV_FBC_ON_GROUP]->virtAddr))));

            write2CQTokenNum += 1;
        }

        //write CAMSV_FBC_IMGO_ENQ_ADDR to CamsvCQDescriptorBuf
        _token[CAMSV_FBC_IMGO_ENQ_ADDR_INX] =
            DESCRIPTOR_TOKEN(_offset[CAMSV_FBC_IMGO_ENQ_ADDR_INX], 1,
            CAMSV_0_BASE_HW);

        memcpy((((MUINT32 *)(_pCamsvCQDescriptorBuf->virtAddr)) +
            write2CQTokenNum), &_token[CAMSV_FBC_IMGO_ENQ_ADDR_INX],
            sizeof(_token[CAMSV_FBC_IMGO_ENQ_ADDR_INX]));

        write2CQTokenNum += 1;

        //copy Data to _pCamsvCQVirtRegFBCFHAddrBuf
        memcpy(((MUINT32 *)_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]
               ->virtAddr + CAMSV_FBC_USE_CQ_IMGO_ENQ_ADDR_INX),
               (MUINT32 *)&(_pCamsvImgBuf[hwModule][subsampleInx]
                [CamsvVF_OFF][DMA_EN_GROUP0][CAMSV_FBC_OFF_GROUP]->phyAddr),
               sizeof((MUINT32 *)(_pCamsvImgBuf[hwModule][subsampleInx]
               [CamsvVF_OFF][DMA_EN_GROUP0][CAMSV_FBC_OFF_GROUP]->phyAddr)));

        // copy _pCamsvCQVirtRegFBCFHAddrBuf to _pCamsvCQDescriptorBuf
        gCamsvCQVirtualRegisterPhyAddr =
            (((MUINT32 *)(_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]
            ->phyAddr)) + CAMSV_FBC_USE_CQ_IMGO_ENQ_ADDR_INX);

        memcpy((((MUINT32 *)(_pCamsvCQDescriptorBuf->virtAddr)) +
               write2CQTokenNum), &gCamsvCQVirtualRegisterPhyAddr,
               sizeof((MUINT32 *)_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]
               ->phyAddr));

        write2CQTokenNum += 1;


        //write REG_CAMSV_FBC_IMGO_ENQ_HADDR to CamsvCQDescriptorBuf
        _token[CAMSV_FBC_IMGO_ENQ_HADDR_INX] =
            DESCRIPTOR_TOKEN(_offset[CAMSV_FBC_IMGO_ENQ_HADDR_INX], 1,
            CAMSV_0_BASE_HW);

        memcpy((((MUINT32 *)(_pCamsvCQDescriptorBuf->virtAddr)) +
            write2CQTokenNum), &_token[CAMSV_FBC_IMGO_ENQ_HADDR_INX],
            sizeof(_token[CAMSV_FBC_IMGO_ENQ_HADDR_INX]));

        write2CQTokenNum += 1;

        // copy data to _pCamsvCQVirtRegFBCFHAddrBuf
        memcpy(((MUINT32 *)_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]
               ->virtAddr + CAMSV_FBC_USE_CQ_IMGO_ENQ_HADDR_INX),
               (MUINT32 *)&(_pCamsvImgBuf[hwModule][subsampleInx]
               [CamsvVF_OFF][DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr),
               sizeof((MUINT32 *)(_pCamsvImgBuf[hwModule][subsampleInx]
               [CamsvVF_OFF][DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr)));

        // copy _pCamsvCQVirtRegFBCFHAddrBuf to _pCamsvCQDescriptorBuf
        gCamsvCQVirtualRegisterPhyAddr = (((MUINT32 *)(_pCamsvCQVirtRegFBCFHAddrBuf
               [subsampleInx]->phyAddr)) + CAMSV_FBC_USE_CQ_IMGO_ENQ_HADDR_INX);

        memcpy((((MUINT32 *)(_pCamsvCQDescriptorBuf->virtAddr)) +
                write2CQTokenNum), &gCamsvCQVirtualRegisterPhyAddr,
                sizeof((MUINT32 *)(&_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]
                ->phyAddr)));

        write2CQTokenNum += 1;

        //write CAMSV_CAMSV_IMGO_FBC to CamsvCQDescriptorBuf
        _token[CAMSV_CAMSV_IMGO_FBC_INX] =
            DESCRIPTOR_TOKEN(_offset[CAMSV_CAMSV_IMGO_FBC_INX], 1,
            CAMSV_0_BASE_HW);

        memcpy((((MUINT32 *)(_pCamsvCQDescriptorBuf->virtAddr)) +
            write2CQTokenNum), &_token[CAMSV_CAMSV_IMGO_FBC_INX],
            sizeof(_token[CAMSV_CAMSV_IMGO_FBC_INX]));

        write2CQTokenNum += 1;

        camsvImgoFbcRcntInc1Value = CAMSV_READ_REG(ptr, CAMSV_IMGO_FBC);

        LOG_INFL("subsampleInx = 0x%x camsvImgoFbcRcntInc1Value Before |= 0x%x ",
            subsampleInx, camsvImgoFbcRcntInc1Value);

        camsvImgoFbcRcntInc1Value = camsvImgoFbcRcntInc1Value | 0x00000001;

        LOG_INFL("subsampleInx = 0x%x camsvImgoFbcRcntInc1Value after |= 0x%x ",
            subsampleInx, camsvImgoFbcRcntInc1Value);

        memcpy(((MUINT32 *)_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]
               ->virtAddr + CAMSV_FBC_USE_CQ_IMGO_RCNT_INC_STATUS_INX),
               (MUINT32 *)&(camsvImgoFbcRcntInc1Value),
               sizeof(camsvImgoFbcRcntInc1Value));

        gCamsvCQVirtualRegisterPhyAddr =
            (((MUINT32 *)(_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]
            ->phyAddr)) + CAMSV_FBC_USE_CQ_IMGO_RCNT_INC_STATUS_INX);

        memcpy((((MUINT32 *)(_pCamsvCQDescriptorBuf->virtAddr)) +
            write2CQTokenNum), &gCamsvCQVirtualRegisterPhyAddr,
            sizeof((MUINT32 *)(&_pCamsvCQVirtRegFBCFHAddrBuf[subsampleInx]
            ->phyAddr)));

        write2CQTokenNum += 1;
    }

    getchar();
    //write ISP_DRV_CQ_END_TOKEN to CamsvCQDescriptorBuf
    memcpy((((MUINT32 *)(_pCamsvCQDescriptorBuf->virtAddr)) + write2CQTokenNum),
           &ISP_DRV_CQ_END_TOKEN_VALUE, sizeof(ISP_DRV_CQ_END_TOKEN_VALUE));

    write2CQTokenNum += 2;

    LOG_INFL("write2CQTokenNum = 0x%x ", write2CQTokenNum);

    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    CAM_WRITE_REG(ptr, CAMCQ_R1_CAMCQ_CQ_THR0_DESC_SIZE,
        (write2CQTokenNum * 4));

    CAM_WRITE_REG(ptr, CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,
        _pCamsvCQDescriptorBuf->phyAddr);

    LOG_INFL("Before Trig cam_%c CQ0 \n", 'a');
    getchar();
    if (CAM_READ_BITS(ptr, CAMCQ_R1_CAMCQ_CQ_THR0_CTL, CAMCQ_CQ_THR0_MODE) == 1
        /*immediate*/) {
        LOG_INFL("Trig cam_%c CQ0 \n", 'a');
        CAM_WRITE_BITS(ptr, CAMCTL_R1_CAMCTL_START,
                 CAMCTL_CQ_THR0_START, 1);
    }

    const MUINT32 seninf0_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/whole_chip_pattern-SENINF0.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/whole_chip_pattern-SENINF1.h"
        0xdeadbeef
    };

    const MUINT32 seninf2_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF2.h"
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/whole_chip_pattern-SENINF2.h"
        0xdeadbeef
    };

    const MUINT32 seninf3_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF3.h"
        0xdeadbeef
    };

    const MUINT32 seninf4_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF4.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
    fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
    fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
    fakedrvsensor.powerOn("fake3", 1, &snrAry[3], seninf3_golden);
    fakedrvsensor.powerOn("fake4", 1, &snrAry[4], seninf4_golden);

    return 0;

}

MINT32 Pattern_Loading_4_AfterCamsvVFOn(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry[5] = {0,1,2,3,4};
    IMEM_BUF_INFO ******_pCamsvimgBuf = NULL;
    MUINT32 subSampleNum = 0;
    _pCamsvimgBuf = pInputInfo->m_pCamsvImemBufs;
    ISP_HW_MODULE hwModule = CAMSV_0;
    (void)length;
    //
    static const unsigned char bpci_r1a_i_0[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/bpci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].pTblAddr  = bpci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].tblLength =
        (sizeof(bpci_r1a_i_0) / sizeof(bpci_r1a_i_0[0]));

    //
    static const unsigned char bpci_r2a_i_0[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/bpci_r2a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].pTblAddr  = bpci_r2a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].tblLength =
        (sizeof(bpci_r2a_i_0) / sizeof(bpci_r2a_i_0[0]));

    //
    static const unsigned char lsci_r1a_i_0[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/lsci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].pTblAddr  = lsci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].tblLength =
        (sizeof(lsci_r1a_i_0) / sizeof(lsci_r1a_i_0[0]));

    //
    static const unsigned char pdi_r1a_i_0[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/pdi_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].pTblAddr  = pdi_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].tblLength =
        (sizeof(pdi_r1a_i_0) / sizeof(pdi_r1a_i_0[0]));


    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];

    LOG_INFL("ptr=%p _pCamsv=%p "
             "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p\n",
             ptr, _pCamsv,
             ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]);

    ptrCam = ptr;
#include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-CAMSV_0.h"


    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
#include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-CAMA.h"
    //#include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/whole_chip_pattern-CAM_A-CQ.h"

    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];

    LOG_INFL("ptr=%p _pCamsv=%p "
             "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p\n",
             ptr, _pCamsv,
             ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]);

    subSampleNum = CAMSV_READ_BITS(ptr,CAMSV_SPARE1,for_DCIF_subsample_number);
    LOG_INFL("ptr=%p _pCamsv=%p "
             "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p\n",
             ptr, _pCamsv,
             ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]);


    AllocCamsvSubsampleBuffer(_pCamsvimgBuf, hwModule, subSampleNum,
                              DMA_EN_GROUP0, CamsvVF_ON, (4 * 16 * 2),
                              CAMSV_FBC_ON_GROUP);

    LOG_INFL("ptr=%p _pCamsv=%p "
            "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p\n",
            ptr, _pCamsv,
            ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]);


    typedef const unsigned char (*subsample)[64];

    static const unsigned char subsample_0[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_0.dat"
    };
    static const unsigned char subsample_1[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_1.dat"
    };
    static const unsigned char subsample_2[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_2.dat"
    };
    static const unsigned char subsample_3[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_3.dat"
    };
    static const unsigned char subsample_4[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_4.dat"
    };
    static const unsigned char subsample_5[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_5.dat"
    };
    static const unsigned char subsample_6[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_6.dat"
    };
    static const unsigned char subsample_7[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_7.dat"
    };
    static const unsigned char subsample_8[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_8.dat"
    };
    static const unsigned char subsample_9[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_9.dat"
    };
    static const unsigned char subsample_10[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_10.dat"
    };
    static const unsigned char subsample_11[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_11.dat"
    };
    static const unsigned char subsample_12[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_12.dat"
    };
    static const unsigned char subsample_13[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_13.dat"
    };
    static const unsigned char subsample_14[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_14.dat"
    };
    static const unsigned char subsample_15[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_15.dat"
    };

    static subsample SUBSUMPLE_TAL[]={
        &subsample_0,  &subsample_1,  &subsample_2,  &subsample_3,
        &subsample_4,  &subsample_5,  &subsample_6,  &subsample_7,
        &subsample_8,  &subsample_9,  &subsample_10, &subsample_11,
        &subsample_12, &subsample_13, &subsample_14, &subsample_15,
        &subsample_0,  &subsample_1,  &subsample_2,  &subsample_3,
        &subsample_4,  &subsample_5,  &subsample_6,  &subsample_7,
        &subsample_8,  &subsample_9,  &subsample_10, &subsample_11,
        &subsample_12, &subsample_13, &subsample_14, &subsample_15,
        };

    for(MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {
        pInputInfo->SubsampleTbls[hwModule][subsampleInx][CAMSV_FBC_ON_GROUP].
            pTblAddr = ((const unsigned char *)(SUBSUMPLE_TAL[subsampleInx]));

        pInputInfo->SubsampleTbls[hwModule][subsampleInx][CAMSV_FBC_ON_GROUP].
            tblLength = 16;


        LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                 "pTblAddr = %p *pTblAddr = 0x%x tblLength = 0x%x ",
                 hwModule, subsampleInx,
                 pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                 [CAMSV_FBC_ON_GROUP].pTblAddr,
                 *(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                 [CAMSV_FBC_ON_GROUP].pTblAddr),
                 pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                 [CAMSV_FBC_ON_GROUP].tblLength);
    }
    //AllocCamsvSubsampleBufferAndCopyData(pInputInfo, hwModule,subSampleNum);
     enum {
       CAMSV_IMGO_FH_SPARE_1_INX = 0 , CAMSV_IMGO_FH_SPARE_2_INX ,
       CAMSV_IMGO_FH_SPARE_3_INX     , CAMSV_IMGO_FH_SPARE_4_INX ,
       CAMSV_IMGO_FH_SPARE_5_INX     , CAMSV_IMGO_FH_SPARE_6_INX ,
       CAMSV_IMGO_FH_SPARE_7_INX     , CAMSV_IMGO_FH_SPARE_8_INX ,
       CAMSV_IMGO_FH_SPARE_9_INX     , CAMSV_IMGO_FH_SPARE_10_INX,
       CAMSV_IMGO_FH_SPARE_11_INX    , CAMSV_IMGO_FH_SPARE_12_INX,
       CAMSV_IMGO_FH_SPARE_13_INX    , CAMSV_IMGO_FH_SPARE_14_INX,
       CAMSV_IMGO_FH_SPARE_15_INX    , CAMSV_IMGO_FH_SPARE_16_INX,
       CAMSV_TEST_CAM_CQ_ITEM_NUM    ,
    } ECAMSV_TEST_CAM_CQ_ITEM_NUM    ;

    for(MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {
        for(MUINT32 i = CAMSV_IMGO_FH_SPARE_2_INX;
            i < CAMSV_TEST_CAM_CQ_ITEM_NUM; i++) {
          LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                   "Write to CAMSV_IMGO_FH_SPARE_%d = 0x%x\n",
                   hwModule, subsampleInx,
                   (i + 1),
                   *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule]
                   [subsampleInx][CAMSV_FBC_ON_GROUP].pTblAddr)) + (i)));
       }

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_2,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_2_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_3,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_3_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_4,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_4_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_5,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_5_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_6,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_6_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_7,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_7_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_8,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_8_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_9,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_9_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_10,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_10_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_11,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_11_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_12,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_12_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_13,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_13_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_14,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_14_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_15,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_15_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_SPARE_16,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_IMGO_FH_SPARE_16_INX));

        CAMSV_WRITE_REG(ptr, CAMSV_IMGO_FH_BASE_ADDR,
            pInputInfo->m_pCamsvImemBufs[hwModule][subsampleInx][CamsvVF_ON]
            [DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr);

        CAMSV_WRITE_BITS(ptr, CAMSV_IMGO_FBC, RCNT_INC1, 1);
    }

    const MUINT32 seninf0_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-SENINF0.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-SENINF1.h"
        0xdeadbeef
    };

    const MUINT32 seninf2_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF2.h"
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-SENINF2.h"
        0xdeadbeef
    };

    const MUINT32 seninf3_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF3.h"
        0xdeadbeef
    };

    const MUINT32 seninf4_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF4.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
    fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
    fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
    fakedrvsensor.powerOn("fake3", 1, &snrAry[3], seninf3_golden);
    fakedrvsensor.powerOn("fake4", 1, &snrAry[4], seninf4_golden);

    return 0;
}


MINT32 Pattern_Loading_5(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry[5] = {0,1,2,3,4};
    IMEM_BUF_INFO ******_pCamsvImgBuf = NULL;
    MUINT32 subSampleNum = 0;
    _pCamsvImgBuf = pInputInfo->m_pCamsvImemBufs;
    ISP_HW_MODULE hwModule = CAMSV_0;
    (void)length;

    //
    static const unsigned char bpci_r1a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/bpci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].pTblAddr  = bpci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R1].tblLength =
        (sizeof(bpci_r1a_i_0) / sizeof(bpci_r1a_i_0[0]));

    //
    static const unsigned char bpci_r2a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/bpci_r2a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].pTblAddr  = bpci_r2a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI_R2].tblLength =
        (sizeof(bpci_r2a_i_0) / sizeof(bpci_r2a_i_0[0]));

    //
    static const unsigned char lsci_r1a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/lsci_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].pTblAddr  = lsci_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI_R1].tblLength =
        (sizeof(lsci_r1a_i_0) / sizeof(lsci_r1a_i_0[0]));

    //
    static const unsigned char pdi_r1a_i_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/dat/pdi_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].pTblAddr  = pdi_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].tblLength =
        (sizeof(pdi_r1a_i_0) / sizeof(pdi_r1a_i_0[0]));


    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/isp_top_single_fbc_pat_whole_chip_pattern_CAM_A.h"



    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];

    LOG_INFL("ptr=%p _pCamsv=%p "
             "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p\n",
             ptr, _pCamsv,
             ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]);

    ptrCam = ptr;
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/isp_top_single_fbc_pat_camsv_whole_chip_pattern_CAMSV0.h"


    subSampleNum =
        CAMSV_READ_BITS(ptr, CAMSV_SPARE1, for_DCIF_subsample_number);

    LOG_INFL("ptr=%p _pCamsv=%p "
             "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p "
             "subSampleNum=%d",
             ptr, _pCamsv,
             ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX],
             subSampleNum);

    AllocCamsvSubsampleBuffer(_pCamsvImgBuf, hwModule, subSampleNum,
                              DMA_EN_GROUP0, CamsvVF_OFF, (4 * 16 * 2),
                              CAMSV_FBC_ON_GROUP);

    AllocCamsvSubsampleBuffer(_pCamsvImgBuf, hwModule, subSampleNum,
                              DMA_EN_GROUP0, CamsvVF_OFF, (4 * 16 * 2),
                              CAMSV_FBC_OFF_GROUP);

    LOG_INFL("ptr=%p _pCamsv=%p "
            "((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]=%p\n",
            ptr, _pCamsv,
            ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]);


    typedef const unsigned char (*subsample)[64];

    static const unsigned char subsample_0[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_0.dat"
    };
    static const unsigned char subsample_1[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_1.dat"
    };
    static const unsigned char subsample_2[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_2.dat"
    };
    static const unsigned char subsample_3[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_3.dat"
    };
    static const unsigned char subsample_4[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_4.dat"
    };
    static const unsigned char subsample_5[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_5.dat"
    };
    static const unsigned char subsample_6[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_6.dat"
    };
    static const unsigned char subsample_7[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_7.dat"
    };
    static const unsigned char subsample_8[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_8.dat"
    };
    static const unsigned char subsample_9[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_9.dat"
    };
    static const unsigned char subsample_10[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_10.dat"
    };
    static const unsigned char subsample_11[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_11.dat"
    };
    static const unsigned char subsample_12[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_12.dat"
    };
    static const unsigned char subsample_13[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_13.dat"
    };
    static const unsigned char subsample_14[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_14.dat"
    };
    static const unsigned char subsample_15[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_dat/subsample_15.dat"
    };


    static subsample SUBSUMPLE_TAL[]={
        &subsample_0,  &subsample_1,  &subsample_2,  &subsample_3,
        &subsample_4,  &subsample_5,  &subsample_6,  &subsample_7,
        &subsample_8,  &subsample_9,  &subsample_10, &subsample_11,
        &subsample_12, &subsample_13, &subsample_14, &subsample_15,
        &subsample_0,  &subsample_1,  &subsample_2,  &subsample_3,
        &subsample_4,  &subsample_5,  &subsample_6,  &subsample_7,
        &subsample_8,  &subsample_9,  &subsample_10, &subsample_11,
        &subsample_12, &subsample_13, &subsample_14, &subsample_15
        };

    for(MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {

        pInputInfo->SubsampleTbls[hwModule][subsampleInx][CAMSV_FBC_ON_GROUP].
            pTblAddr  = ((const unsigned char *)(SUBSUMPLE_TAL[subsampleInx]));

        pInputInfo->SubsampleTbls[hwModule][subsampleInx][CAMSV_FBC_ON_GROUP].
            tblLength = 16;


        LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                 "pTblAddr = %p *pTblAddr = 0x%x tblLength = 0x%x ",
                 hwModule, subsampleInx,
                 pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                 [CAMSV_FBC_ON_GROUP].pTblAddr,
                 *(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                 [CAMSV_FBC_ON_GROUP].pTblAddr),
                 pInputInfo->SubsampleTbls[hwModule][subsampleInx]
                 [CAMSV_FBC_ON_GROUP].tblLength);
    }
    //AllocCamsvSubsampleBufferAndCopyData(pInputInfo, hwModule,subSampleNum);
     enum {
       CAMSV_FBC_IMGO_ENQ_HEADER_TIMESTAMP_INX = 0 ,
       CAMSV_FBC_IMGO_ENQ_HEADER_0_INX ,  CAMSV_FBC_IMGO_ENQ_HEADER_1_INX ,
       CAMSV_FBC_IMGO_ENQ_HEADER_2_INX ,  CAMSV_FBC_IMGO_ENQ_HEADER_3_INX ,
       CAMSV_FBC_IMGO_ENQ_HEADER_4_INX ,  CAMSV_FBC_IMGO_ENQ_HEADER_5_INX ,
       CAMSV_TEST_CAM_CQ_ITEM_NUM      ,
    } ECAMSV_TEST_CAM_CQ_ITEM_NUM    ;

    for(MUINT32 subsampleInx = 0; subsampleInx < subSampleNum; subsampleInx++) {
        for(MUINT32 i = CAMSV_FBC_IMGO_ENQ_HEADER_0_INX;
            i < CAMSV_TEST_CAM_CQ_ITEM_NUM; i++) {

          LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                   "Write to CAMSV_FBC_IMGO_ENQ_HEADER_%d = 0x%x\n",
                   hwModule, subsampleInx,
                   (i-1),
                   *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule]
                   [subsampleInx][CAMSV_FBC_ON_GROUP].pTblAddr)) + (i - 1)));

       }

        CAMSV_WRITE_REG(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_0,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_FBC_IMGO_ENQ_HEADER_0_INX -
            1 ));

        CAMSV_WRITE_REG(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_1,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_FBC_IMGO_ENQ_HEADER_1_INX -
            1));

        CAMSV_WRITE_REG(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_2,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_FBC_IMGO_ENQ_HEADER_2_INX -
            1));

        CAMSV_WRITE_REG(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_3,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_FBC_IMGO_ENQ_HEADER_3_INX -
            1));

        CAMSV_WRITE_REG(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_4,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_FBC_IMGO_ENQ_HEADER_4_INX -
            1));

        CAMSV_WRITE_REG(ptr, CAMSV_FBC_IMGO_ENQ_HEADER_5,
            *(((MUINT32 *)(pInputInfo->SubsampleTbls[hwModule][subsampleInx]
            [CAMSV_FBC_ON_GROUP].pTblAddr)) + CAMSV_FBC_IMGO_ENQ_HEADER_5_INX -
            1));

        CAMSV_WRITE_REG(ptr, CAMSV_FBC_IMGO_ENQ_ADDR,
            pInputInfo->m_pCamsvImemBufs[hwModule][subsampleInx][CamsvVF_OFF]
            [DMA_EN_GROUP0][CAMSV_FBC_OFF_GROUP]->phyAddr);

        CAMSV_WRITE_REG(ptr, CAMSV_FBC_IMGO_ENQ_HADDR,
            pInputInfo->m_pCamsvImemBufs[hwModule][subsampleInx][CamsvVF_OFF]
            [DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr);


        LOG_INFL("hwModule = 0x%x, subsampleInx = 0x%x, "
                 "Write to CAMSV_FBC_IMGO_ENQ_ADDR = 0x%lx "
                 "Write to CAMSV_FBC_IMGO_ENQ_HADDR = 0x%lx ",
                 hwModule, subsampleInx,
                 pInputInfo->m_pCamsvImemBufs[hwModule][subsampleInx]
                 [CamsvVF_OFF][DMA_EN_GROUP0][CAMSV_FBC_OFF_GROUP]->phyAddr,
                 pInputInfo->m_pCamsvImemBufs[hwModule][subsampleInx]
                 [CamsvVF_OFF][DMA_EN_GROUP0][CAMSV_FBC_ON_GROUP]->phyAddr);

        getchar();
        CAMSV_WRITE_BITS(ptr, CAMSV_IMGO_FBC, RCNT_INC1, 1);
    }

    const MUINT32 seninf0_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-SENINF0.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-SENINF1.h"
        0xdeadbeef
    };

    const MUINT32 seninf2_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF2.h"
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/whole_chip_pattern-SENINF2.h"
        0xdeadbeef
    };

    const MUINT32 seninf3_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF3.h"
        0xdeadbeef
    };

    const MUINT32 seninf4_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF4.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
    fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
    fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
    fakedrvsensor.powerOn("fake3", 1, &snrAry[3], seninf3_golden);
    fakedrvsensor.powerOn("fake4", 1, &snrAry[4], seninf4_golden);

    return 0;
}

MINT32 Pattern_Loading_6(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry[5] = {0,1,2,3,4};
    (void)length;

    //
    static const unsigned char ufdi_r2b_i_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/dat/ufdi_r2b_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_UFDI_R2].pTblAddr  = ufdi_r2b_i_0;

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_UFDI_R2].tblLength =
        (sizeof(ufdi_r2b_i_0) / sizeof(ufdi_r2b_i_0[0]));

    //
    static const unsigned char ufdi_r2a_i_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/dat/ufdi_r2a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_UFDI_R2].pTblAddr  = ufdi_r2a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_UFDI_R2].tblLength =
        (sizeof(ufdi_r2a_i_0) / sizeof(ufdi_r2a_i_0[0]));

    //
    static const unsigned char rawi_r2b_i_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/dat/rawi_r2b_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_RAWI_R2].pTblAddr  = rawi_r2b_i_0;

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_RAWI_R2].tblLength =
        (sizeof(rawi_r2b_i_0) / sizeof(rawi_r2b_i_0[0]));

    //
    static const unsigned char rawi_r2a_i_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/dat/rawi_r2a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_RAWI_R2].pTblAddr  = rawi_r2a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_RAWI_R2].tblLength =
        (sizeof(rawi_r2a_i_0) / sizeof(rawi_r2a_i_0[0]));

     //
    static const unsigned char pdi_r1b_i_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/dat/pdi_r1b_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI_R1].pTblAddr  = pdi_r1b_i_0;

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI_R1].tblLength =
        (sizeof(pdi_r1b_i_0) / sizeof(pdi_r1b_i_0[0]));

    //
    static const unsigned char pdi_r1a_i_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/dat/pdi_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].pTblAddr  = pdi_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].tblLength =
        (sizeof(pdi_r1a_i_0) / sizeof(pdi_r1a_i_0[0]));

    //
    static const unsigned char pattern_cq_dat_tbl[] = {
            #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/dat/cq_data_0.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr  = pattern_cq_dat_tbl;
    // from image.list in pattern from DE
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr   = 0x5d834800;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength =
        (sizeof(pattern_cq_dat_tbl) / sizeof(pattern_cq_dat_tbl[0]));


    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/whole_chip_pattern-CAM_A-CQ.h"
    //#include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/whole_chip_pattern-CAM_A-CQ.h"

    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/whole_chip_pattern-CAM_B-CQ.h"


    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    ptrCam = ptr;
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern-CAMSV_0.h"

    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_1-CAM_MAX];
    ptrCam = ptr;
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern-CAMSV_1.h"

    const MUINT32 seninf0_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/whole_chip_pattern-SENSOR0.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/whole_chip_pattern-SENSOR1.h"
        0xdeadbeef
    };

    const MUINT32 seninf2_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF2.h"
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/whole_chip_pattern-SENSOR2.h"
        0xdeadbeef
    };

    const MUINT32 seninf3_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF3.h"
        0xdeadbeef
    };

    const MUINT32 seninf4_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF4.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
    fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
    fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
    fakedrvsensor.powerOn("fake3", 1, &snrAry[3], seninf3_golden);
    fakedrvsensor.powerOn("fake4", 1, &snrAry[4], seninf4_golden);
    return 0;
}

MINT32 Pattern_Loading_15(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char pattern_bpci_b_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/bpci_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_b_tbl)/sizeof(pattern_bpci_b_tbl[0]);

    static const unsigned char pattern_bpci_c_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/bpci_c_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_c_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_c_tbl)/sizeof(pattern_bpci_c_tbl[0]);

    static const unsigned char pattern_lsci_b_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/lsci_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_b_tbl)/sizeof(pattern_lsci_b_tbl[0]);

    static const unsigned char pattern_lsci_c_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/lsci_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_c_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_c_tbl)/sizeof(pattern_lsci_c_tbl[0]);

    static const unsigned char pattern_pdi_b_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/pdi_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_b_tbl)/sizeof(pattern_pdi_b_tbl[0]);

    static const unsigned char pattern_cq_dat_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/cq_data_0.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr = pattern_cq_dat_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength = sizeof(pattern_cq_dat_tbl)/sizeof(pattern_cq_dat_tbl[0]);
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr = 0x56fac600; //from image.list in pattern from DE

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/CAM_A_0.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/CAM_B_0.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/CAM_C_0.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/SENINF1_0.h"
        0xdeadbeef
    };
    const MUINT32 seninf2_golden[] = {
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/SENINF2_0.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        fakedrvsensor.powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    #endif
    return 0;
}

MINT32 Pattern_Loading_16(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char pattern_bpci_b_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/bpci_b_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_b_tbl)/sizeof(pattern_bpci_b_tbl[0]);

    static const unsigned char pattern_bpci_c_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/bpci_c_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_c_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_c_tbl)/sizeof(pattern_bpci_c_tbl[0]);

    static const unsigned char pattern_lsci_b_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/lsci_b_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_b_tbl)/sizeof(pattern_lsci_b_tbl[0]);

    static const unsigned char pattern_lsci_c_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/lsci_c_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_c_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_c_tbl)/sizeof(pattern_lsci_c_tbl[0]);

    static const unsigned char pattern_pdi_b_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/pdi_b_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_b_tbl)/sizeof(pattern_pdi_b_tbl[0]);

    static const unsigned char pattern_cq_dat_tbl[] = {
            #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/dat/cq_data_1.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr = pattern_cq_dat_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength = sizeof(pattern_cq_dat_tbl)/sizeof(pattern_cq_dat_tbl[0]);
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr = 0x53cab000; //from image.list in pattern from DE

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/CAM_A_1.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/CAM_B_1.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/CAM_C_1.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/SENINF1_1.h"
        0xdeadbeef
    };
    const MUINT32 seninf2_golden[] = {
    #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/cfg/SENINF2_1.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        fakedrvsensor.powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    #endif
    return 0;
}

MINT32 Pattern_Loading_7(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
     TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
     MUINT32 snrAry[5] = {0,1,2,3,4};
     (void)length;(void)_uni;
     //
     static const unsigned char pattern_cq_dat_tbl[] = {
         #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/dat/cq_data_0.dat"
     };
     pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr  = pattern_cq_dat_tbl;
     // from image.list in pattern from DE
     pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr   = 0x508e3400;
     pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength =
         (sizeof(pattern_cq_dat_tbl) / sizeof(pattern_cq_dat_tbl[0]));


    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/whole_chip_pattern-CAM_A-CQ.h"

    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/whole_chip_pattern-CAM_B-CQ.h"

    const MUINT32 seninf0_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/whole_chip_pattern-SENINF0.h"
         0xdeadbeef
     };

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/whole_chip_pattern-SENINF1.h"
         0xdeadbeef
     };

    const MUINT32 seninf2_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/whole_chip_pattern-SENINF2.h"
         0xdeadbeef
     };


     fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
     fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
     fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
     return 0;

}

MINT32 Pattern_Loading_8(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    IspDrv* ptrCam[MAX_ISP_HW_MODULE] = {NULL, NULL};
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry[5] = {0,1,2,3,4};
    const MUINT32 SUBSAMPLE_MAX = 32;
    (void)length;
    // Loading DE Setting

    static const unsigned char pdi_r1a_i_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/dat/2camsv_cq_1raw_ufo_fbc_on_pat__release_pdi_r1a_i_0.dat"
    };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].pTblAddr  = pdi_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].tblLength =
        (sizeof(pdi_r1a_i_0) / sizeof(pdi_r1a_i_0[0]));


    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam[CAM_A] = ptr;
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/2camsv_cq_1raw_ufo_fbc_on_pat__release_whole_chip_pattern-CAM_A_NOCQ.h"

    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    ptrCam[CAM_B] = ptr;
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/2camsv_cq_1raw_ufo_fbc_on_pat__release_whole_chip_pattern-CAM_B_NOCQ.h"


    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    ptrCam[CAMSV_0] = ptr;
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/camsv_whole_chip_pattern/2camsv_cq_1raw_ufo_fbc_on_pat_camsv_whole_chip_pattern_CAMSV_0.h"

    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_1-CAM_MAX];
    ptrCam[CAMSV_1] = ptr;
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/camsv_whole_chip_pattern/2camsv_cq_1raw_ufo_fbc_on_pat_camsv_whole_chip_pattern_CAMSV_1.h"

    LOG_INF("Loading Finish DE Setting!!!!!!!!!!!!!");
    getchar();

    #if 1
    // SW Control Flow Setting
    typedef const unsigned char (*subsample)[64];

    enum {
         CAMSV_FBC_IMGO_ENQ_HEADER_TIMESTAMP_INX = 0 ,
         CAMSV_FBC_IMGO_ENQ_HEADER_0_INX, CAMSV_FBC_IMGO_ENQ_HEADER_1_INX ,
         CAMSV_FBC_IMGO_ENQ_HEADER_2_INX, CAMSV_FBC_IMGO_ENQ_HEADER_3_INX ,
         CAMSV_FBC_IMGO_ENQ_HEADER_4_INX, CAMSV_FBC_IMGO_ENQ_HEADER_5_INX ,
         CAMSV_FBC_IMGO_ENQ_ADDR_INX    , CAMSV_FBC_IMGO_ENQ_HADDR_INX    ,
         CAMSV_CAMSV_IMGO_FBC_INX       , REG_CAMSV_IMGO_FH_BASE_ADDR_INX ,
         CAMSV_TEST_CAM_CQ_ITEM_NUM     ,
    }ECAMSV_TEST_CAM_CQ_ITEM_NUM;


    enum {
         IMGO_R1_IMGO_FH_SPARE_01_INX = 0 , IMGO_R1_IMGO_FH_SPARE_02_INX ,
         IMGO_R1_IMGO_FH_SPARE_03_INX     , IMGO_R1_IMGO_FH_SPARE_04_INX ,
         IMGO_R1_IMGO_FH_SPARE_05_INX     , IMGO_R1_IMGO_FH_SPARE_06_INX ,
         IMGO_R1_IMGO_FH_SPARE_07_INX     , IMGO_R1_IMGO_FH_SPARE_08_INX ,
         IMGO_R1_IMGO_FH_SPARE_09_INX     , IMGO_R1_IMGO_FH_SPARE_10_INX ,
         IMGO_R1_IMGO_FH_SPARE_11_INX     , IMGO_R1_IMGO_FH_SPARE_12_INX ,
         IMGO_R1_IMGO_FH_SPARE_13_INX     , IMGO_R1_IMGO_FH_SPARE_14_INX ,
         IMGO_R1_IMGO_FH_SPARE_15_INX     , IMGO_R1_IMGO_FH_SPARE_16_INX ,
         IMGO_R1_IMGO_FH_BASE_ADDR_INX    ,
         CAM_CQ_CHANGE_BASEADDR_TO_DUMMY_DESCRIPTOR_INX,
         CAM_TEST_CQ_ITEM_NUM             ,
    }ECAM_TEST_CQ_ITEM_NUM;

    MUINT32 _camsv_offset[MAX_ISP_HW_MODULE][CAMSV_TEST_CAM_CQ_ITEM_NUM] = { };
    MUINT32 _cam_offset[MAX_ISP_HW_MODULE][CAM_TEST_CQ_ITEM_NUM]         = { };
    MUINT32 _camsv_token[MAX_ISP_HW_MODULE][CAMSV_TEST_CAM_CQ_ITEM_NUM]  = { };
    MUINT32 _cam_token[MAX_ISP_HW_MODULE][CAM_TEST_CQ_ITEM_NUM]          = { };
    MUINT32 write2CQTokenNum[MAX_ISP_HW_MODULE][CQ_NORMALPAGE_NUM]       = { };

    MUINT32 write2CQDummyTokenNum[MAX_ISP_HW_MODULE][CQ_DUMMY_PAGE_NUM]
                                 [SUBSAMPLE_MAX]  = { };
    MUINT32 subSampleNum = 0;

    _pCamsvImgBuf                = pInputInfo->m_pCamsvImemBufs;
    _pCamsvCQVirtRegBuf          = pInputInfo->m_pCamsvCQVirtualRegisterBuf;
    gpMultiCamsvCQDescriptorBuf  = pInputInfo->m_pMultiCamsvCQDescriptorBuf;

    gpMultiCamsvCQDummyDescriptorBuf  =
        pInputInfo->m_pMultiCamsvCQDummyDescriptorBuf;

    gpMultiCamsvCQVirtRegFBCFHAddrBuf =
        pInputInfo->m_pMultiCamsvCQVirRegFBCFHAddrBuf;

    static MUINT32   camsvImgoFbcRcntInc1Value[MAX_ISP_HW_MODULE]  = {0};
    static MUINT32 *pCamsvImgoFbcRcntInc1Value[MAX_ISP_HW_MODULE]  = {NULL};
    for(MUINT32 inx = CAM_A; inx < MAX_ISP_HW_MODULE; inx++){

        pCamsvImgoFbcRcntInc1Value[inx] = &camsvImgoFbcRcntInc1Value[inx];
    }

    // NormalPage0
    static const unsigned char NormalPage0_subsample_0[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage0/NormalPage0_subsample_0.dat"
    };
    static const unsigned char NormalPage0_subsample_1[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage0/NormalPage0_subsample_1.dat"
    };
    static const unsigned char NormalPage0_subsample_2[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage0/NormalPage0_subsample_2.dat"
    };
    static const unsigned char NormalPage0_subsample_3[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage0/NormalPage0_subsample_3.dat"
    };
    static const unsigned char NormalPage0_subsample_4[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage0/NormalPage0_subsample_4.dat"
    };
    static const unsigned char NormalPage0_subsample_5[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage0/NormalPage0_subsample_5.dat"
    };
    static const unsigned char NormalPage0_subsample_6[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage0/NormalPage0_subsample_6.dat"
    };
    static const unsigned char NormalPage0_subsample_7[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage0/NormalPage0_subsample_7.dat"
    };


    static subsample NormalPage0_SUBSUMPLE_TAL[] = {
        &NormalPage0_subsample_0 , &NormalPage0_subsample_1 ,
        &NormalPage0_subsample_2 , &NormalPage0_subsample_3 ,
        &NormalPage0_subsample_4 , &NormalPage0_subsample_5 ,
        &NormalPage0_subsample_6 , &NormalPage0_subsample_7 ,
        &NormalPage0_subsample_0 , &NormalPage0_subsample_1 ,
        &NormalPage0_subsample_2 , &NormalPage0_subsample_3 ,
        &NormalPage0_subsample_4 , &NormalPage0_subsample_5 ,
        &NormalPage0_subsample_6 , &NormalPage0_subsample_7 ,
        &NormalPage0_subsample_0 , &NormalPage0_subsample_1 ,
        &NormalPage0_subsample_2 , &NormalPage0_subsample_3 ,
        &NormalPage0_subsample_4 , &NormalPage0_subsample_5 ,
        &NormalPage0_subsample_6 , &NormalPage0_subsample_7 ,
        &NormalPage0_subsample_0 , &NormalPage0_subsample_1 ,
        &NormalPage0_subsample_2 , &NormalPage0_subsample_3 ,
        &NormalPage0_subsample_4 , &NormalPage0_subsample_5 ,
        &NormalPage0_subsample_6 , &NormalPage0_subsample_7 ,
    };

    // NormalPage1
    static const unsigned char NormalPage1_subsample_0[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage1/NormalPage1_subsample_0.dat"
    };
    static const unsigned char NormalPage1_subsample_1[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage1/NormalPage1_subsample_1.dat"
    };
    static const unsigned char NormalPage1_subsample_2[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage1/NormalPage1_subsample_2.dat"
    };
    static const unsigned char NormalPage1_subsample_3[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage1/NormalPage1_subsample_3.dat"
    };
    static const unsigned char NormalPage1_subsample_4[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage1/NormalPage1_subsample_4.dat"
    };
    static const unsigned char NormalPage1_subsample_5[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage1/NormalPage1_subsample_5.dat"
    };
    static const unsigned char NormalPage1_subsample_6[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage1/NormalPage1_subsample_6.dat"
    };
    static const unsigned char NormalPage1_subsample_7[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_dat/NormalPage1/NormalPage1_subsample_7.dat"
    };


    static subsample NormalPage1_SUBSUMPLE_TAL[] = {
        &NormalPage1_subsample_0 , &NormalPage1_subsample_1 ,
        &NormalPage1_subsample_2 , &NormalPage1_subsample_3 ,
        &NormalPage1_subsample_4 , &NormalPage1_subsample_5 ,
        &NormalPage1_subsample_6 , &NormalPage1_subsample_7 ,
        &NormalPage1_subsample_0 , &NormalPage1_subsample_1 ,
        &NormalPage1_subsample_2 , &NormalPage1_subsample_3 ,
        &NormalPage1_subsample_4 , &NormalPage1_subsample_5 ,
        &NormalPage1_subsample_6 , &NormalPage1_subsample_7 ,
        &NormalPage1_subsample_0 , &NormalPage1_subsample_1 ,
        &NormalPage1_subsample_2 , &NormalPage1_subsample_3 ,
        &NormalPage1_subsample_4 , &NormalPage1_subsample_5 ,
        &NormalPage1_subsample_6 , &NormalPage1_subsample_7 ,
        &NormalPage1_subsample_0 , &NormalPage1_subsample_1 ,
        &NormalPage1_subsample_2 , &NormalPage1_subsample_3 ,
        &NormalPage1_subsample_4 , &NormalPage1_subsample_5 ,
        &NormalPage1_subsample_6 , &NormalPage1_subsample_7 ,
    };

    subSampleNum = CAMSV_READ_BITS(ptrCam[CAMSV_0], CAMSV_SPARE1,
                                for_DCIF_subsample_number);

    MUINT32 camsvHwModule = CAMSV_0, camHwModule = CAM_A;
    MUINT32 camsvHwBase = 0x0, camHwBase = 0x0;

    for(camsvHwModule = CAMSV_0,    camHwModule = CAM_A;
        camsvHwModule <= CAMSV_1 && camHwModule <= CAM_B ;
        camsvHwModule++,camHwModule++) {

        switch(camsvHwModule){
            case CAMSV_0:
                camsvHwBase = CAMSV_0_BASE_HW;
                break;
            case CAMSV_1:
                camsvHwBase = CAMSV_1_BASE_HW;
                break;
            default:
                camsvHwBase = 0x0;
                break;
        }

        switch(camHwModule){
            case CAM_A:
                camHwBase = CAM_A_BASE_HW;
                break;
            case CAM_B:
                camHwBase = CAM_B_BASE_HW;
                break;
            default:
                camHwBase = 0x0;
                break;
        }

         for(MUINT32 pageInx = CQ_PAGE0;
             pageInx < CQ_PAGE_NUM;
             pageInx++) {

            // create Normal & Dummy of _pCamsvImgBuf & _pCamsvCQVirtRegBuf
            for(MUINT32 pageTypeInx = CQ_NORMAL_PAGE_TYPE;
                pageTypeInx < CQ_PAGE_TYPE_NUM;
                pageTypeInx++) {
                MUINT32 local_IMGO_YSIZE = 0,local_IMGO_XSIZE = 0;
                MUINT32 local_IMGO_STRIDE = 0;
                local_IMGO_XSIZE = CAMSV_READ_BITS(ptr, CAMSV_IMGO_XSIZE, XSIZE);
                local_IMGO_YSIZE = CAMSV_READ_BITS(ptr, CAMSV_IMGO_YSIZE, YSIZE);
                local_IMGO_STRIDE = CAMSV_READ_BITS(ptr, CAMSV_IMGO_STRIDE, STRIDE);

                AllocCamsvSubsampleBuffer(_pCamsvImgBuf,
                    (ISP_HW_MODULE)camsvHwModule, subSampleNum,
                    (ECQ_PAGE_TYPE_GROUP)pageTypeInx, CQ_IMGO_ENQ_ADDR_INX,
                    (2 * (local_IMGO_STRIDE * (local_IMGO_YSIZE + 1))),
                    (ECQ_PAGES_NUM_GROUP)pageInx);

                LOG_INFL("camsvHwModule=%02d subSampleNum=%02d "
                         "pageTypeInx=%02d pageInx=%02d CQ_IMGO_ENQ_ADDR_INX=%02d "
                         "_pCamsvImgBuf[%02d][%02d][%02d][%02d][%2d]=%p "
                         "PA:0x%lx VA:0x%lx Size:%02d ",
                         camsvHwModule, subSampleNum,
                         pageTypeInx, pageInx, CQ_IMGO_ENQ_ADDR_INX,
                         camsvHwModule, subSampleNum,
                         pageTypeInx, pageInx, CQ_IMGO_ENQ_ADDR_INX,
                         _pCamsvImgBuf[camsvHwModule][subSampleNum][CQ_IMGO_ENQ_ADDR_INX]
                         [pageTypeInx][pageInx],
                         _pCamsvImgBuf[camsvHwModule][subSampleNum][CQ_IMGO_ENQ_ADDR_INX]
                         [pageTypeInx][pageInx]->phyAddr,
                         _pCamsvImgBuf[camsvHwModule][subSampleNum][CQ_IMGO_ENQ_ADDR_INX]
                         [pageTypeInx][pageInx]->virtAddr,
                         _pCamsvImgBuf[camsvHwModule][subSampleNum][CQ_IMGO_ENQ_ADDR_INX]
                         [pageTypeInx][pageInx]->size);

                AllocCamsvSubsampleBuffer(_pCamsvImgBuf,
                    (ISP_HW_MODULE)camsvHwModule, subSampleNum,
                    (ECQ_PAGE_TYPE_GROUP)pageTypeInx, CQ_IMGO_ENQ_HADDR_INX,
                    (4 * 16 * 2),
                    (ECQ_PAGES_NUM_GROUP)pageInx);

                LOG_INFL("camsvHwModule=%02d subSampleNum=%02d "
                         "pageTypeInx=%02d pageInx=%02d CQ_IMGO_ENQ_HADDR_INX=%02d "
                         "_pCamsvImgBuf[%02d][%02d][%02d][%02d][%2d]=%p "
                         "PA:0x%lx VA:0x%lx Size:%02d ",
                         camsvHwModule, subSampleNum,
                         pageTypeInx, pageInx, CQ_IMGO_ENQ_HADDR_INX,
                         camsvHwModule, subSampleNum,
                         pageTypeInx, pageInx, CQ_IMGO_ENQ_HADDR_INX,
                         _pCamsvImgBuf[camsvHwModule][subSampleNum][CQ_IMGO_ENQ_HADDR_INX]
                         [pageTypeInx][pageInx],
                         _pCamsvImgBuf[camsvHwModule][subSampleNum][CQ_IMGO_ENQ_HADDR_INX]
                         [pageTypeInx][pageInx]->phyAddr,
                         _pCamsvImgBuf[camsvHwModule][subSampleNum][CQ_IMGO_ENQ_HADDR_INX]
                         [pageTypeInx][pageInx]->virtAddr,
                         _pCamsvImgBuf[camsvHwModule][subSampleNum][CQ_IMGO_ENQ_HADDR_INX]
                         [pageTypeInx][pageInx]->size);

                AllocCamsvSubsampleBuffer(_pCamsvCQVirtRegBuf,
                    (ISP_HW_MODULE)camsvHwModule, subSampleNum,
                    (ECQ_PAGE_TYPE_GROUP)pageTypeInx, CamsvVF_OFF, (4 * 16),
                    (ECQ_PAGES_NUM_GROUP)pageInx);

                LOG_INFL("camsvHwModule=%02d subSampleNum=%02d "
                         "pageTypeInx=%02d pageInx=%02d "
                         "_pCamsvCQVirtRegBuf[%02d][%02d][%02d][%02d]=%p "
                         "PA:0x%lx VA:0x%lx SIZE:%02d ",
                         camsvHwModule, subSampleNum,
                         pageTypeInx, pageInx,
                         camsvHwModule, subSampleNum,
                         pageTypeInx, pageInx,
                         _pCamsvCQVirtRegBuf[camsvHwModule][subSampleNum]
                         [CamsvVF_OFF][pageTypeInx][pageInx],
                         _pCamsvCQVirtRegBuf[camsvHwModule][subSampleNum]
                         [CamsvVF_OFF][pageTypeInx][pageInx]->phyAddr,
                         _pCamsvCQVirtRegBuf[camsvHwModule][subSampleNum]
                         [CamsvVF_OFF][pageTypeInx][pageInx]->virtAddr,
                         _pCamsvCQVirtRegBuf[camsvHwModule][subSampleNum]
                         [CamsvVF_OFF][pageTypeInx][pageInx]->size);
            }

            // create gpMultiCamsvCQDescriptorBuf
            AllocCamsvSubsampleCQDescriptorBuffer("gpMultiCamsvCQDescriptorBuf",
                gpMultiCamsvCQDescriptorBuf[camsvHwModule][pageInx] ,
                (ISP_HW_MODULE)camsvHwModule, 0x4000);

            LOG_INFL("camsvHwModule:%02d pageInx:%02d "
                     "gpMultiCamsvCQDescriptorBuf[%02d][%02d] "
                     "PA:0x%lx VA:0x%lx SIZE:%02d ",
                     (ISP_HW_MODULE)camsvHwModule, (ECQ_PAGES_NUM_GROUP)pageInx,
                     (ISP_HW_MODULE)camsvHwModule, (ECQ_PAGES_NUM_GROUP)pageInx,
                     gpMultiCamsvCQDescriptorBuf[camsvHwModule][pageInx]
                     ->phyAddr,
                     gpMultiCamsvCQDescriptorBuf[camsvHwModule][pageInx]
                     ->virtAddr,
                     gpMultiCamsvCQDescriptorBuf[camsvHwModule][pageInx]
                     ->size);

            // create gpMultiCamsvCQDummyDescriptorBuf
            AllocCamsvSubsampleCQDescriptorBuffer(
                       "gpMultiCamsvCQDummyDescriptorBuf",
                       gpMultiCamsvCQDummyDescriptorBuf[camsvHwModule][pageInx],
                       (ISP_HW_MODULE)camsvHwModule, 0x4000);

            LOG_INFL("camsvHwModule:%02d pageInx:%02d "
                     "gpMultiCamsvCQDummyDescriptorBuf[%02d][%02d] "
                     "PA:0x%lx VA:0x%lx SIZE:%02d",
                     (ISP_HW_MODULE)camsvHwModule, (ECQ_PAGES_NUM_GROUP)pageInx,
                     (ISP_HW_MODULE)camsvHwModule, (ECQ_PAGES_NUM_GROUP)pageInx,
                     gpMultiCamsvCQDummyDescriptorBuf[camsvHwModule][pageInx]
                     ->phyAddr,
                     gpMultiCamsvCQDummyDescriptorBuf[camsvHwModule][pageInx]
                     ->virtAddr,
                     gpMultiCamsvCQDummyDescriptorBuf[camsvHwModule][pageInx]
                     ->size);

        }
        for(MUINT32 pageInx = CQ_PAGE0; pageInx < CQ_PAGE_NUM; pageInx++) {
            for(MUINT32 subsampleInx = 0;
                subsampleInx < subSampleNum;
                subsampleInx++) {
                // create Normal page gpMultiCamsvCQVirtRegFBCFHAddrBuf
                AllocCamsvSubsampleCQDescriptorBuffer(
                    "_pCamsvCQVirtRegFBCFHAddrBuf",
                    gpMultiCamsvCQVirtRegFBCFHAddrBuf[camsvHwModule][subsampleInx][pageInx] ,
                    (ISP_HW_MODULE)camsvHwModule, 0x10);

                for(MUINT32 inx = CAMSV_FBC_USE_CQ_IMGO_ENQ_ADDR_INX;
                    inx < CAMSV_FBC_USE_CQ_ITEMS_NUM;
                    inx++) {

                    LOG_INFL("subsampleInx=%02d _pCamsvCQVirtRegFBCFHAddrBuf[%02d] "
                             " PA_%d:0x%lx VA_%d:0x%lx SIZE:%02d",
                             subsampleInx, subsampleInx,
                             inx,
                             (gpMultiCamsvCQVirtRegFBCFHAddrBuf[camsvHwModule]
                             [subsampleInx][pageInx]->phyAddr + inx),
                             inx,
                             (gpMultiCamsvCQVirtRegFBCFHAddrBuf[camsvHwModule]
                             [subsampleInx][pageInx]->virtAddr + inx),
                             gpMultiCamsvCQVirtRegFBCFHAddrBuf[camsvHwModule]
                             [subsampleInx][pageInx]->size);
                }
            }
        }

        // copy Data from outside txt to inside Tbls.
        for(MUINT32 subsampleInx = 0;
            subsampleInx < subSampleNum;
            subsampleInx++) {

             for(MUINT32 pageInx = CQ_NORMAL_PAGE0;
                 pageInx < CQ_NORMALPAGE_NUM;
                 pageInx++) {

                    switch(pageInx) {
                        case (CQ_NORMAL_PAGE0):

                            pInputInfo->SubsampleTbls[camsvHwModule]
                                [subsampleInx][pageInx].pTblAddr =
                                ((const unsigned char *)
                                (NormalPage0_SUBSUMPLE_TAL[subsampleInx]));

                            break;
                        case (CQ_NORMAL_PAGE1):

                            pInputInfo->SubsampleTbls[camsvHwModule]
                                [subsampleInx][pageInx].pTblAddr =
                                ((const unsigned char *)
                                (NormalPage1_SUBSUMPLE_TAL[subsampleInx]));

                            break;
                        default:

                            pInputInfo->SubsampleTbls[camsvHwModule]
                                [subsampleInx][pageInx].pTblAddr = NULL;

                            break;
                    }

                    pInputInfo->SubsampleTbls[camsvHwModule][subsampleInx]
                        [pageInx].tblLength = 16;

                    for(MUINT32 headerInx = CAMSV_FBC_IMGO_ENQ_HEADER_0_INX;
                        headerInx <= CAMSV_FBC_IMGO_ENQ_HEADER_5_INX;
                        headerInx++) {

                        LOG_INFL("NormalPage%02d: camsvHwModule=0x%x "
                                 "subsampleInx=0x%x headerInx=%02d "
                                 "pTblAddr_%02d=%p *pTblAddr_%02d=0x%x "
                                 "tblLength=0x%x\n",
                                 pageInx, (ISP_HW_MODULE)camsvHwModule,
                                 subsampleInx, headerInx, headerInx,
                                 ((MUINT32 *)((pInputInfo->SubsampleTbls
                                 [camsvHwModule][subsampleInx][pageInx].
                                 pTblAddr)) + headerInx),
                                 headerInx,
                                 *(((MUINT32 *)(pInputInfo->SubsampleTbls
                                 [camsvHwModule][subsampleInx][pageInx].
                                 pTblAddr)) + headerInx),
                                 pInputInfo->SubsampleTbls[camsvHwModule]
                                 [subsampleInx][pageInx].tblLength);
                    }
             }
        }


        // copy Data form Tbls to Normal _pCamsvCQVirtRegBuf
        // write Data to Dummy _pCamsvCQVirtRegBuf
        for(MUINT32 subsampleInx = 0;
            subsampleInx < subSampleNum;
            subsampleInx++) {

           for(MUINT32 pageInx = CQ_PAGE0;
               pageInx < CQ_PAGE_NUM;
               pageInx++) {
               MUINT32 dummyValue = 0xFFFF;
               // copy data to CQ_NORMAL_PAGE_TYPE
               CopyCamsvSubsampleTblDataToCQVirtRegBuf(_pCamsvCQVirtRegBuf,
                    pInputInfo, (ISP_HW_MODULE)camsvHwModule, subsampleInx,
                    CamsvVF_OFF, CQ_NORMAL_PAGE_TYPE,
                    (ECQ_PAGES_NUM_GROUP)pageInx);

               // write to CQ_DUMMY_PAGE_TYPE
               switch(pageInx){
                    case (CQ_PAGE0):
                        dummyValue = 0x2;
                        break;
                    case (CQ_PAGE1):
                        dummyValue = 0x4;
                        break;
                    default:
                        break;
               }
               LOG_INFL("subsampleInx=%02d pageInx=%02d dummyValue=%02d ",
                    subsampleInx, pageInx, dummyValue);

               CopyCamsvSubsampleTblDataToCQDummyVirtRegBuf(_pCamsvCQVirtRegBuf,
                    pInputInfo, (ISP_HW_MODULE)camsvHwModule, subsampleInx,
                    CamsvVF_OFF, CQ_DUMMY_PAGE_TYPE,
                    (ECQ_PAGES_NUM_GROUP)pageInx, dummyValue);
           }
        }


        //AllocCamsvSubsampleBufferAndCopyData(pInputInfo, camsvHwModule,subSampleNum);

        // Prepare _camsv_offset of cam register
        for (MUINT32 subsampleInx = 0;
             subsampleInx < subSampleNum;
             subsampleInx++) {

                _cam_offset[camHwModule][IMGO_R1_IMGO_FH_BASE_ADDR_INX]  =
                    CAM_GET_REG_OFFSET(ptrCam[camHwModule],
                    IMGO_R1_IMGO_FH_BASE_ADDR);

                _cam_offset[camHwModule][IMGO_R1_IMGO_FH_SPARE_02_INX]  =
                    CAM_GET_REG_OFFSET(ptrCam[camHwModule],
                    IMGO_R1_IMGO_FH_SPARE_2);

                _cam_offset[camHwModule]
                    [CAM_CQ_CHANGE_BASEADDR_TO_DUMMY_DESCRIPTOR_INX]  =
                    CAM_GET_REG_OFFSET(ptrCam[camHwModule],
                    CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR);

                LOG_INFL("subsampleInx=%02d camHwModule=%02d "
                         "_cam_offset[%02d][IMGO_R1_IMGO_FH_BASE_ADDR_INX]=0x%x \n"
                         "_cam_offset[%02d][IMGO_R1_IMGO_FH_SPARE_02_INX]=0x%x "
                         "_cam_offset[%02d]"
                         "[CAM_CQ_CHANGE_BASEADDR_TO_DUMMY_DESCRIPTOR_INX]=0x%x ",
                        subsampleInx, camHwModule,
                        camHwModule,
                        _cam_offset[camHwModule][IMGO_R1_IMGO_FH_BASE_ADDR_INX],
                        camHwModule,
                        _cam_offset[camHwModule][IMGO_R1_IMGO_FH_SPARE_02_INX],
                        camHwModule,
                        _cam_offset[camHwModule]
                        [CAM_CQ_CHANGE_BASEADDR_TO_DUMMY_DESCRIPTOR_INX]);
        }

        for (MUINT32 subsampleInx = 0;
             subsampleInx < subSampleNum;
             subsampleInx++) {

            // Prepare camsv register _camsv_offset
            _camsv_offset[camsvHwModule]
                [CAMSV_FBC_IMGO_ENQ_HEADER_TIMESTAMP_INX]  =  0;

            _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HEADER_0_INX]  =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_FBC_IMGO_ENQ_HEADER_0);

            _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HEADER_1_INX]  =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_FBC_IMGO_ENQ_HEADER_1);

            _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HEADER_2_INX]  =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_FBC_IMGO_ENQ_HEADER_2);

            _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HEADER_3_INX]  =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_FBC_IMGO_ENQ_HEADER_3);

            _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HEADER_4_INX]  =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_FBC_IMGO_ENQ_HEADER_4);

            _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HEADER_5_INX]  =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_FBC_IMGO_ENQ_HEADER_5);

            _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_ADDR_INX] =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_FBC_IMGO_ENQ_ADDR);

            _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HADDR_INX] =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_FBC_IMGO_ENQ_HADDR);

            _camsv_offset[camsvHwModule][REG_CAMSV_IMGO_FH_BASE_ADDR_INX] =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_IMGO_FH_BASE_ADDR);

            _camsv_offset[camsvHwModule][CAMSV_CAMSV_IMGO_FBC_INX] =
                CAMSV_GET_REG_OFFSET(ptrCam[camsvHwModule],
                CAMSV_IMGO_FBC);

            // show Normal Page _pCamsvCQVirtRegBuf Inf
            for (MUINT32 headerInx = CAMSV_FBC_IMGO_ENQ_HEADER_0_INX;
                headerInx <= CAMSV_FBC_IMGO_ENQ_HEADER_5_INX;
                headerInx++) {

                for(MUINT32 pageInx = CQ_NORMAL_PAGE0;
                    pageInx <= CQ_NORMAL_PAGE1;
                    pageInx++) {

                    gCamsvCQVirtualRegisterPhyAddr =
                        (((MUINT32 *)(_pCamsvCQVirtRegBuf[camsvHwModule]
                        [subsampleInx][CamsvVF_OFF][CQ_NORMAL_PAGE_TYPE]
                        [pageInx]->phyAddr)) +
                        (headerInx));

                    LOG_INFL("NormalPage%02d: camsvHwModule = 0x%x "
                             "subsampleInx=0x%x "
                             "CAMSV_FBC_IMGO_ENQ_HEADER_%02d_offset=0x%x "
                             "DramAddress=0x%lx DramData=0x%x\n "
                             "CAMSV_FBC_IMGO_ENQ_HEADER_%02d_Token=0x%x",
                             pageInx, (ISP_HW_MODULE)camsvHwModule,
                             subsampleInx, (headerInx - 1),
                             _camsv_offset[camsvHwModule][headerInx],
                             ((_pCamsvCQVirtRegBuf[camsvHwModule]
                             [subsampleInx][CamsvVF_OFF][CQ_NORMAL_PAGE_TYPE]
                             [pageInx]->phyAddr) + headerInx - 1),
                             *((MUINT32 *)(_pCamsvCQVirtRegBuf[camsvHwModule]
                             [subsampleInx][CamsvVF_OFF][CQ_NORMAL_PAGE_TYPE]
                             [pageInx]->virtAddr) + headerInx - 1),
                             (headerInx - 1),
                             DESCRIPTOR_TOKEN(
                             _camsv_offset[camsvHwModule][headerInx], 1,
                             camsvHwBase));
                }
            }

            for(MUINT32 pageInx = CQ_NORMAL_PAGE0;
                pageInx < CQ_NORMALPAGE_NUM;
                pageInx++) {

                LOG_INFL("NormalPage%02d: camsvHwModule=0x%x subsampleInx=0x%x "
                         "CAMSV_FBC_IMGO_ENQ_ADDR_INX_offset=0x%x, "
                         "DramAddress=%p DramData=0x%lx\n "
                         "CAMSV_FBC_IMGO_ENQ_ADDR_INX_Token=0x%x ",
                         pageInx, (ISP_HW_MODULE)camsvHwModule, subsampleInx,
                         _camsv_offset[camsvHwModule]
                         [CAMSV_FBC_IMGO_ENQ_ADDR_INX],
                         &_pCamsvImgBuf[camsvHwModule][subsampleInx]
                         [CamsvVF_OFF][DMA_EN_GROUP0][pageInx]->phyAddr,
                         _pCamsvImgBuf[camsvHwModule][subsampleInx]
                         [CamsvVF_OFF][DMA_EN_GROUP0][pageInx]->phyAddr,
                         DESCRIPTOR_TOKEN(
                         _camsv_offset[camsvHwModule]
                         [CAMSV_FBC_IMGO_ENQ_ADDR_INX], 1, camsvHwBase));

                LOG_INFL("NormalPage%02d: camsvHwModule=0x%x subsampleInx=0x%x "
                         "REG_CAMSV_IMGO_FH_BASE_ADDR_INX=0x%x "
                         "DramAddress=%p DramData=0x%lx\n "
                         "REG_CAMSV_IMGO_FH_BASE_ADDR_INX_Token=0x%x ",
                         pageInx, (ISP_HW_MODULE)camsvHwModule, subsampleInx,
                         _camsv_offset[camsvHwModule]
                         [REG_CAMSV_IMGO_FH_BASE_ADDR_INX],
                         &_pCamsvImgBuf[camsvHwModule][subsampleInx]
                         [CamsvVF_OFF][DMA_EN_GROUP0][pageInx]->phyAddr,
                         _pCamsvImgBuf[camsvHwModule][subsampleInx]
                         [CamsvVF_OFF][DMA_EN_GROUP0][pageInx]->phyAddr,
                         DESCRIPTOR_TOKEN(
                         _camsv_offset[camsvHwModule]
                         [REG_CAMSV_IMGO_FH_BASE_ADDR_INX], 1, camsvHwBase));

                LOG_INFL("NormalPage%02d: camsvHwModule=0x%x subsampleInx=0x%x "
                         "CAMSV_FBC_IMGO_ENQ_HADDR_INX=0x%x "
                         "DramAddress=%p DramData=0x%lx\n "
                         "CAMSV_FBC_IMGO_ENQ_HADDR_INX_Token=0x%x ",
                         pageInx, (ISP_HW_MODULE)camsvHwModule, subsampleInx,
                         _camsv_offset[camsvHwModule]
                         [CAMSV_FBC_IMGO_ENQ_HADDR_INX],
                         &_pCamsvImgBuf[camsvHwModule][subsampleInx]
                         [CamsvVF_OFF][DMA_EN_GROUP0][pageInx]->phyAddr,
                         _pCamsvImgBuf[camsvHwModule][subsampleInx][CamsvVF_OFF]
                         [DMA_EN_GROUP0][pageInx]->phyAddr,
                         DESCRIPTOR_TOKEN(
                         _camsv_offset[camsvHwModule]
                         [CAMSV_FBC_IMGO_ENQ_HADDR_INX], 1, camsvHwBase));

                LOG_INFL("NormalPage%02d: camsvHwModule=0x%x subsampleInx=0x%x "
                         "CAMSV_CAMSV_IMGO_FBC_offset=0x%x, "
                         "DramAddress=%p DramData=0x%x\n "
                         "CAMSV_CAMSV_IMGO_FBC_Token=0x%x ",
                         pageInx, (ISP_HW_MODULE)camsvHwModule, subsampleInx,
                         _camsv_offset[camsvHwModule][CAMSV_CAMSV_IMGO_FBC_INX],
                         pCamsvImgoFbcRcntInc1Value[camsvHwModule],
                         camsvImgoFbcRcntInc1Value[camsvHwModule],
                         DESCRIPTOR_TOKEN(
                         _camsv_offset[camsvHwModule][CAMSV_CAMSV_IMGO_FBC_INX],
                         1, camsvHwBase));
            }

            //getchar();

            //write CAM_IMGO_R1_IMGO_FH_SPARE_X to CamsvCQDummyDescriptorBuf
            for(MUINT32 headerInx = IMGO_R1_IMGO_FH_SPARE_02_INX;
                headerInx <= IMGO_R1_IMGO_FH_SPARE_02_INX;
                headerInx++) {

                for(MUINT32 pageInx = CQ_DUMMY_PAGE0;
                    pageInx < CQ_DUMMY_PAGE_NUM;
                    pageInx++) {

                        _cam_token[camHwModule][headerInx] =
                            DESCRIPTOR_TOKEN(_cam_offset[camHwModule][headerInx],
                            1, camHwBase);

                        LOG_INFL("NormalPage%02d: camHwModule=0x%x "
                                 "headerInx=0x%x "
                                 "_cam_token[%02d][%02d]=0x%x "
                                 "_cam_offset[%02d][%02d]=0x%x\n"
                                 "write2CQDummyTokenNum[%02d][%02d][%02d]=0x%x",
                                 pageInx,camHwModule,
                                 headerInx,
                                 camHwModule, headerInx,
                                 _cam_token[camHwModule][headerInx],
                                 camHwModule, headerInx,
                                 _cam_offset[camHwModule][headerInx],
                                 camsvHwModule, pageInx, subsampleInx,
                                 write2CQDummyTokenNum[camsvHwModule]
                                 [pageInx][subsampleInx]);

                        memcpy((((MUINT32 *)(gpMultiCamsvCQDummyDescriptorBuf
                            [camsvHwModule][pageInx]->virtAddr)) +
                            write2CQDummyTokenNum[camsvHwModule][pageInx]
                            [subsampleInx]),
                            &_cam_token[camHwModule][headerInx],
                            sizeof(_cam_token[camHwModule][headerInx]));

                        write2CQDummyTokenNum[camsvHwModule][pageInx]
                            [subsampleInx] += 1;

                        gCamsvCQVirtualRegisterPhyAddr =
                            (((MUINT32 *)(_pCamsvCQVirtRegBuf[camsvHwModule]
                            [subsampleInx][CamsvVF_OFF][CQ_DUMMY_PAGE_TYPE]
                            [pageInx]->phyAddr))+ (headerInx - 1));

                        memcpy((((MUINT32 *)(gpMultiCamsvCQDummyDescriptorBuf
                            [camsvHwModule][pageInx]->virtAddr)) +
                            write2CQDummyTokenNum[camsvHwModule][pageInx]
                            [subsampleInx]),
                            &gCamsvCQVirtualRegisterPhyAddr,
                            sizeof(((MUINT32 *)(_pCamsvCQVirtRegBuf[camHwModule]
                            [subsampleInx][CamsvVF_OFF][CQ_DUMMY_PAGE_TYPE]
                            [pageInx]->virtAddr))));

                        write2CQDummyTokenNum[camsvHwModule][pageInx]
                            [subsampleInx] += 1;
                }
            }

            //write IMGO_R1_IMGO_FH_BASE_ADDR_INX to CamsvCQDummyDescriptorBuf
            for(MUINT32 pageInx = CQ_DUMMY_PAGE0;
                pageInx < CQ_DUMMY_PAGE_NUM;
                pageInx++) {

                //write IMGO_R1_IMGO_FH_BASE_ADDR_INX to CamsvCQDummyDescriptorBuf
                _cam_token[camHwModule][IMGO_R1_IMGO_FH_BASE_ADDR_INX] =
                    DESCRIPTOR_TOKEN(
                    _cam_offset[camHwModule][IMGO_R1_IMGO_FH_BASE_ADDR_INX],
                    1, camHwBase);

                LOG_INFL("NormalPage%02d: camHwModule=0x%x "
                         "_cam_token[%02d][18]=0x%x "
                         "_cam_offset[%02d][18]=0x%x\n"
                         "write2CQDummyTokenNum[%02d][%02d][%02d]=0x%x",
                         pageInx, camHwModule,
                         camHwModule,
                         _cam_token[camHwModule][IMGO_R1_IMGO_FH_BASE_ADDR_INX],
                         camHwModule,
                         _cam_offset[camHwModule][IMGO_R1_IMGO_FH_BASE_ADDR_INX],
                         camsvHwModule, pageInx, subsampleInx,
                         write2CQDummyTokenNum[camsvHwModule][pageInx]
                         [subsampleInx]);

                memcpy((((MUINT32 *)(gpMultiCamsvCQDummyDescriptorBuf
                    [camsvHwModule][pageInx]->virtAddr)) +
                    write2CQDummyTokenNum[camsvHwModule][pageInx][subsampleInx]),
                    &_cam_token[camHwModule][IMGO_R1_IMGO_FH_BASE_ADDR_INX],
                    sizeof(_cam_token[camHwModule]
                    [IMGO_R1_IMGO_FH_BASE_ADDR_INX]));

                write2CQDummyTokenNum[camsvHwModule][pageInx][subsampleInx] += 1;

                // copy data to _pCamsvCQVirtRegFBCFHAddrBuf
                memcpy(((MUINT32 *)gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->virtAddr +
                    CAMSV_FBC_USE_CQ_IMGO_R1_IMGO_FH_BASE_ADDR_DUMMY_PAGE0_INX +
                    pageInx),
                    (MUINT32 *)&(_pCamsvImgBuf[camsvHwModule][subsampleInx]
                    [CamsvVF_OFF][CQ_DUMMY_PAGE_TYPE][pageInx]->phyAddr),
                    sizeof((MUINT32 *)(_pCamsvImgBuf[camsvHwModule]
                    [subsampleInx][CamsvVF_OFF][CQ_DUMMY_PAGE_TYPE][pageInx]
                    ->phyAddr)));

                // copy _pCamsvCQVirtRegFBCFHAddrBuf to _pCamsvCQDummyDescriptorBuf
                gCamsvCQVirtualRegisterPhyAddr =
                    (((MUINT32 *)(gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->phyAddr)) +
                    CAMSV_FBC_USE_CQ_IMGO_R1_IMGO_FH_BASE_ADDR_DUMMY_PAGE0_INX +
                    pageInx);

                memcpy((((MUINT32 *)(gpMultiCamsvCQDummyDescriptorBuf
                    [camsvHwModule][pageInx]->virtAddr)) +
                    write2CQDummyTokenNum[camsvHwModule][pageInx][subsampleInx]),
                    &gCamsvCQVirtualRegisterPhyAddr,
                    sizeof((MUINT32 *)(&gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->phyAddr)));

                write2CQDummyTokenNum[camsvHwModule][pageInx][subsampleInx] += 1;


                //write ISP_DRV_CQ_END_TOKEN to gpMultiCamsvCQDummyDescriptorBuf
                memcpy((((MUINT32 *)(gpMultiCamsvCQDummyDescriptorBuf
                    [camsvHwModule][pageInx]->virtAddr)) +
                    write2CQDummyTokenNum[camsvHwModule][pageInx][subsampleInx]),
                    &ISP_DRV_CQ_END_TOKEN_VALUE,
                    sizeof(ISP_DRV_CQ_END_TOKEN_VALUE));

                write2CQDummyTokenNum[camsvHwModule][pageInx][subsampleInx] += 2;

            }


            //write CAMSV_FBC_IMGO_ENQ_HEADER_X to CamsvCQDescriptorBuf
            _camsv_token[camsvHwModule]
                [CAMSV_FBC_IMGO_ENQ_HEADER_TIMESTAMP_INX] = 0;

            for(MUINT32 headerInx = CAMSV_FBC_IMGO_ENQ_HEADER_0_INX;
                headerInx <= CAMSV_FBC_IMGO_ENQ_HEADER_5_INX ;
                headerInx++) {

                 for(MUINT32 pageInx = CQ_NORMAL_PAGE0;
                    pageInx <= CQ_NORMAL_PAGE1; pageInx++) {

                        _camsv_token[camsvHwModule][headerInx] =
                            DESCRIPTOR_TOKEN(
                            _camsv_offset[camsvHwModule][headerInx], 1,
                            camsvHwBase);

                        memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf
                            [camsvHwModule][pageInx]->virtAddr)) +
                            write2CQTokenNum[camsvHwModule][pageInx]),
                            &_camsv_token[camsvHwModule][headerInx],
                            sizeof(_camsv_token[camsvHwModule][headerInx]));

                        write2CQTokenNum[camsvHwModule][pageInx] += 1;

                        gCamsvCQVirtualRegisterPhyAddr =
                            (((MUINT32 *)(_pCamsvCQVirtRegBuf[camsvHwModule]
                            [subsampleInx][CamsvVF_OFF][CQ_NORMAL_PAGE_TYPE]
                            [pageInx]->phyAddr))+ (headerInx - 1));

                        memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf
                            [camsvHwModule][pageInx]->virtAddr)) +
                            write2CQTokenNum[camsvHwModule][pageInx]),
                            &gCamsvCQVirtualRegisterPhyAddr,
                            sizeof(((MUINT32 *)(_pCamsvCQVirtRegBuf
                            [camsvHwModule][subsampleInx][CamsvVF_OFF]
                            [CQ_NORMAL_PAGE_TYPE][pageInx]->virtAddr))));

                        write2CQTokenNum[camsvHwModule][pageInx] += 1;
                  }
            }

            for(MUINT32 pageInx = CQ_NORMAL_PAGE0;
                pageInx <= CQ_NORMAL_PAGE1;
                pageInx++) {

                //write CAMSV_FBC_IMGO_ENQ_ADDR to CamsvCQDescriptorBuf
                _camsv_token[camsvHwModule][CAMSV_FBC_IMGO_ENQ_ADDR_INX] =
                    DESCRIPTOR_TOKEN(
                    _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_ADDR_INX],
                    1, camsvHwBase);

                memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf
                    [camsvHwModule][pageInx]->virtAddr)) +
                    write2CQTokenNum[camsvHwModule][pageInx]),
                    &_camsv_token[camsvHwModule][CAMSV_FBC_IMGO_ENQ_ADDR_INX],
                    sizeof(_camsv_token[camsvHwModule]
                    [CAMSV_FBC_IMGO_ENQ_ADDR_INX]));

                write2CQTokenNum[camsvHwModule][pageInx] += 1;

                //copy Data to _pCamsvCQVirtRegFBCFHAddrBuf
                memcpy(((MUINT32 *)gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->virtAddr +
                    CAMSV_FBC_USE_CQ_IMGO_ENQ_ADDR_INX),
                    (MUINT32 *)&(_pCamsvImgBuf[CAMSV_0][subsampleInx]
                    [CQ_IMGO_ENQ_ADDR_INX][CQ_NORMAL_PAGE_TYPE][pageInx]
                    ->phyAddr),
                    sizeof((MUINT32 *)(_pCamsvImgBuf[CAMSV_0]
                    [subsampleInx][CQ_IMGO_ENQ_ADDR_INX][pageInx]
                    [CQ_NORMAL_PAGE1]->phyAddr)));

                // copy _pCamsvCQVirtRegFBCFHAddrBuf to _pCamsvCQDescriptorBuf
                gCamsvCQVirtualRegisterPhyAddr =
                    (((MUINT32 *)(gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->phyAddr)) +
                    CAMSV_FBC_USE_CQ_IMGO_ENQ_ADDR_INX);

                memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf
                    [camsvHwModule][pageInx]->virtAddr)) +
                    write2CQTokenNum[camsvHwModule][pageInx]),
                    &gCamsvCQVirtualRegisterPhyAddr,
                    sizeof((MUINT32 *)gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->phyAddr));

                write2CQTokenNum[camsvHwModule][pageInx] += 1;


                //write REG_CAMSV_FBC_IMGO_ENQ_HADDR to CamsvCQDescriptorBuf
                _camsv_token[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HADDR_INX] =
                    DESCRIPTOR_TOKEN(
                    _camsv_offset[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HADDR_INX],
                    1, camsvHwBase);

                memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf
                    [camsvHwModule][pageInx]->virtAddr)) +
                    write2CQTokenNum[camsvHwModule][pageInx]),
                    &_camsv_token[camsvHwModule][CAMSV_FBC_IMGO_ENQ_HADDR_INX],
                    sizeof(_camsv_token[camsvHwModule]
                    [CAMSV_FBC_IMGO_ENQ_HADDR_INX]));

                write2CQTokenNum[camsvHwModule][pageInx] += 1;

                // copy data to _pCamsvCQVirtRegFBCFHAddrBuf
                memcpy(((MUINT32 *)gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->virtAddr +
                    CAMSV_FBC_USE_CQ_IMGO_ENQ_HADDR_INX),
                    (MUINT32 *)&(_pCamsvImgBuf[camsvHwModule][subsampleInx]
                    [CQ_IMGO_ENQ_HADDR_INX][CQ_NORMAL_PAGE_TYPE][pageInx]
                    ->phyAddr),
                    sizeof((MUINT32 *)(_pCamsvImgBuf[camsvHwModule]
                    [subsampleInx][CQ_IMGO_ENQ_HADDR_INX][pageInx]
                    [pageInx]->phyAddr)));

                // copy _pCamsvCQVirtRegFBCFHAddrBuf to _pCamsvCQDescriptorBuf
                gCamsvCQVirtualRegisterPhyAddr =
                    (((MUINT32 *)(gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->phyAddr)) +
                    CAMSV_FBC_USE_CQ_IMGO_ENQ_HADDR_INX);

                memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf
                    [camsvHwModule][pageInx]->virtAddr)) +
                    write2CQTokenNum[camsvHwModule][pageInx]),
                    &gCamsvCQVirtualRegisterPhyAddr,
                    sizeof((MUINT32 *)(&gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->phyAddr)));

                write2CQTokenNum[camsvHwModule][pageInx] += 1;

                //write CAMSV_CAMSV_IMGO_FBC to CamsvCQDescriptorBuf
                _camsv_token[camsvHwModule][CAMSV_CAMSV_IMGO_FBC_INX] =
                    DESCRIPTOR_TOKEN(
                    _camsv_offset[camsvHwModule][CAMSV_CAMSV_IMGO_FBC_INX],
                    1, camsvHwBase);

                memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf
                    [camsvHwModule][pageInx]->virtAddr)) +
                    write2CQTokenNum[camsvHwModule][pageInx]),
                    &_camsv_token[CAMSV_0][CAMSV_CAMSV_IMGO_FBC_INX],
                    sizeof(_camsv_token[camsvHwModule]
                    [CAMSV_CAMSV_IMGO_FBC_INX]));

                write2CQTokenNum[camsvHwModule][pageInx] += 1;

                camsvImgoFbcRcntInc1Value[camsvHwModule] =
                    CAMSV_READ_REG(ptrCam[camsvHwModule], CAMSV_IMGO_FBC);

                LOG_INFL("subsampleInx=0x%x camsvImgoFbcRcntInc1Value "
                         "Before |= 0x%x ",
                         subsampleInx,
                         camsvImgoFbcRcntInc1Value[camsvHwModule]);

                switch(camsvHwModule) {
                    case(CAMSV_0):
                        camsvImgoFbcRcntInc1Value[camsvHwModule] =
                            camsvImgoFbcRcntInc1Value[camsvHwModule] | 0x00000001;
                        break;
                    case(CAMSV_1):
                        camsvImgoFbcRcntInc1Value[CAMSV_0] =
                            CAMSV_READ_REG(ptrCam[CAMSV_0], CAMSV_IMGO_FBC);

                        camsvImgoFbcRcntInc1Value[CAMSV_0] =
                            camsvImgoFbcRcntInc1Value[CAMSV_0] | 0x00000004;
                        break;
                    default:
                        LOG_INFL("No Support Camsv:%d", camsvHwModule);
                        break;
                }


                LOG_INFL("subsampleInx=0x%x camsvImgoFbcRcntInc1Value "
                         "after |= 0x%x ",
                    subsampleInx, camsvImgoFbcRcntInc1Value[camsvHwModule]);

                memcpy(((MUINT32 *)gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->virtAddr +
                    CAMSV_FBC_USE_CQ_IMGO_RCNT_INC_STATUS_INX),
                    (MUINT32 *)&(camsvImgoFbcRcntInc1Value[CAMSV_0]),
                    sizeof(camsvImgoFbcRcntInc1Value[CAMSV_0]));

                gCamsvCQVirtualRegisterPhyAddr =
                    (((MUINT32 *)(gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->phyAddr)) +
                    CAMSV_FBC_USE_CQ_IMGO_RCNT_INC_STATUS_INX);

                memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf[camsvHwModule]
                    [pageInx]->virtAddr)) +
                    write2CQTokenNum[camsvHwModule][pageInx]),
                    &gCamsvCQVirtualRegisterPhyAddr,
                    sizeof((MUINT32 *)(&gpMultiCamsvCQVirtRegFBCFHAddrBuf
                    [camsvHwModule][subsampleInx][pageInx]->phyAddr)));

                write2CQTokenNum[camsvHwModule][pageInx] += 1;

            }// for(MUINT32 pageInx = CQ_NORMAL_PAGE0;
        }// for (MUINT32 subsampleInx = 0;

        getchar();

        for(MUINT32 pageInx = CQ_NORMAL_PAGE0;
            pageInx < CQ_NORMALPAGE_NUM;
            pageInx++) {

            //CAM_CQ_CHANGE_BASEADDR_TO_DUMMY_DESCRIPTOR_INX
            _cam_token[camHwModule]
                [CAM_CQ_CHANGE_BASEADDR_TO_DUMMY_DESCRIPTOR_INX] =
                DESCRIPTOR_TOKEN(_cam_offset[camHwModule]
                [CAM_CQ_CHANGE_BASEADDR_TO_DUMMY_DESCRIPTOR_INX],
                1, camHwBase);

            memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf
                [camsvHwModule][pageInx]->virtAddr)) +
                write2CQTokenNum[camsvHwModule][pageInx]),
                &_cam_token[camHwModule]
                [CAM_CQ_CHANGE_BASEADDR_TO_DUMMY_DESCRIPTOR_INX],
                sizeof(_cam_token[camHwModule]
                [CAM_CQ_CHANGE_BASEADDR_TO_DUMMY_DESCRIPTOR_INX]));

            write2CQTokenNum[camsvHwModule][pageInx] += 1;



            gCamsvCQVirtualRegisterPhyAddr =
                ((MUINT32 *)(gpMultiCamsvCQDummyDescriptorBuf[camsvHwModule]
                [pageInx]->phyAddr));

            memcpy(((MUINT32 *)gpMultiCamsvCQVirtRegFBCFHAddrBuf
                  [camsvHwModule][subSampleNum-1][pageInx]->virtAddr +
                  CHANGE_BASEADDR_TO_MultiCamsvCQDummyDescriptorBuf_DUMMY_PAGE0_INX
                  + pageInx),
                  (MUINT32 *)&(gCamsvCQVirtualRegisterPhyAddr),
                  sizeof(gCamsvCQVirtualRegisterPhyAddr));

            gCamsvCQVirtualRegisterPhyAddr_chageDummyBaseAddress =
                ((MUINT32 *)gpMultiCamsvCQVirtRegFBCFHAddrBuf
                 [camsvHwModule][subSampleNum-1][pageInx]->phyAddr +
                 CHANGE_BASEADDR_TO_MultiCamsvCQDummyDescriptorBuf_DUMMY_PAGE0_INX
                 + pageInx);

            memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf
                [camsvHwModule][pageInx]->virtAddr)) +
                write2CQTokenNum[camsvHwModule][pageInx]),
                &gCamsvCQVirtualRegisterPhyAddr_chageDummyBaseAddress,
                sizeof((MUINT32 *)
                (&gCamsvCQVirtualRegisterPhyAddr_chageDummyBaseAddress)));

            write2CQTokenNum[camsvHwModule][pageInx] += 1;

            //write ISP_DRV_CQ_END_TOKEN to CamsvCQDescriptorBuf
            memcpy((((MUINT32 *)(gpMultiCamsvCQDescriptorBuf[camsvHwModule]
                [pageInx]->virtAddr)) +
                write2CQTokenNum[camsvHwModule][pageInx]),
                &ISP_DRV_CQ_END_TOKEN_VALUE,
                sizeof(ISP_DRV_CQ_END_TOKEN_VALUE));

            write2CQTokenNum[camsvHwModule][pageInx] += 2;

            LOG_INFL("camsvHwModule=%02d pageIn=%02d "
                     "write2CQTokenNum[%02d][%02d]=%02d ",
                     (ISP_HW_MODULE)camsvHwModule, pageInx,
                     (ISP_HW_MODULE)camsvHwModule, pageInx,
                     write2CQTokenNum[camsvHwModule][pageInx]);

            // write only CQ_NORMAL_PAGE0 to CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR
            // before Camsv VF_data on
            CAM_WRITE_REG(ptrCam[camHwModule],
                CAMCQ_R1_CAMCQ_CQ_THR0_DESC_SIZE,
                (write2CQTokenNum[camsvHwModule][CQ_NORMAL_PAGE0] * 4));

            CAM_WRITE_REG(ptrCam[camHwModule],
                CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,
                gpMultiCamsvCQDescriptorBuf[camsvHwModule][CQ_NORMAL_PAGE0]
                ->phyAddr);

            LOG_INFL("Write to CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR "
                     "ptrCam[%d]=%p pageInx=%d"
                     "write2CQTokenNum[%d][%d]=%02d "
                     "gpMultiCamsvCQDescriptorBuf[%d][%d]"
                     "->phyAddr=0x%lx ",
                      camHwModule,
                      ptrCam[camHwModule], pageInx,
                      camHwModule, CQ_NORMAL_PAGE0,
                      write2CQTokenNum[camsvHwModule][CQ_NORMAL_PAGE0],
                      camHwModule, CQ_NORMAL_PAGE0,
                      gpMultiCamsvCQDescriptorBuf[camsvHwModule][CQ_NORMAL_PAGE0]
                      ->phyAddr);



        }
    } //for(camsvHwModule = CAMSV_0,    camHwModule = CAM_A;


//    for(MUINT32 hwModule = CAM_A; hwModule <= CAM_B; hwModule++) {
//        LOG_INFL("Before Setting  cam_%c CQ0 Continue Mode \n", 'a'+hwModule);
//        getchar();
//        if (CAM_READ_BITS(ptrCam[hwModule], CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
//                CAMCQ_CQ_THR0_MODE) != 0 /*continue Mode*/) {
//            LOG_INFL("cam_%c CQ0 continue Mode \n", 'a'+hwModule);
//            CAM_WRITE_BITS(ptrCam[hwModule], CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
//                     CAMCQ_CQ_THR0_MODE, 0);
//        }
//    }


    for(MUINT32 hwModule = CAM_A; hwModule <= CAM_B; hwModule++) {
        LOG_INFL("Before Trig cam_%c CQ0 \n", 'a'+hwModule);
        if (CAM_READ_BITS(ptrCam[hwModule], CAMCQ_R1_CAMCQ_CQ_THR0_CTL, CAMCQ_CQ_THR0_MODE) == 1
            /*immediate*/) {
            LOG_INFL("Trig cam_%c CQ0 \n", 'a'+hwModule);
            getchar();
            CAM_WRITE_BITS(ptrCam[hwModule], CAMCTL_R1_CAMCTL_START,
                     CAMCTL_CQ_THR0_START, 1);
        }
    }


    ///
//    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
//    ptrCam[CAM_A] = ptr;
//    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
//    ptrCam[CAM_B] = ptr;

    for(MUINT32 hwModule = CAM_A; hwModule <= CAM_B; hwModule++) {
        LOG_INFL("Before Setting  cam_%c CQ0 Continue Mode \n", 'a'+hwModule);
        getchar();
        if (CAM_READ_BITS(ptrCam[hwModule], CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
                CAMCQ_CQ_THR0_MODE) != 0 /*continue Mode*/) {
            LOG_INFL("cam_%c CQ0 continue Mode \n", 'a'+hwModule);
            CAM_WRITE_BITS(ptrCam[hwModule], CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
                 CAMCQ_CQ_THR0_MODE, 0);
        }
    }

    #endif
    const MUINT32 seninf0_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/whole_chip_pattern-SENSOR0.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/whole_chip_pattern-SENSOR1.h"
        0xdeadbeef
    };

    const MUINT32 seninf2_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF2.h"
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/whole_chip_pattern-SENSOR2.h"
        0xdeadbeef
    };

    const MUINT32 seninf3_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF3.h"
        0xdeadbeef
    };

    const MUINT32 seninf4_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/camsv_whole_chip_pattern/camsv_whole_chip_pattern-SENINF4.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
    fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
    fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
    fakedrvsensor.powerOn("fake3", 1, &snrAry[3], seninf3_golden);
    fakedrvsensor.powerOn("fake4", 1, &snrAry[4], seninf4_golden);
    return 0;
}


MINT32 Pattern_Loading_9(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry[5] = {0,1,2,3,4};
    (void)length;

    //
    static const unsigned char pdi_r1a_i_0[] = {
        #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/dat/pdi_r1a_i_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].pTblAddr  = pdi_r1a_i_0;

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI_R1].tblLength =
        (sizeof(pdi_r1a_i_0) / sizeof(pdi_r1a_i_0[0]));

    //
    static const unsigned char pattern_cq_dat_tbl[] = {
            #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/dat/cq_data_0.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr  = pattern_cq_dat_tbl;
    // from image.list in pattern from DE
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr   = 0x5df7d800;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength =
        (sizeof(pattern_cq_dat_tbl) / sizeof(pattern_cq_dat_tbl[0]));


    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/whole_chip_pattern_CAM_A_CQ.h"
    //#include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/whole_chip_pattern-CAM_A-CQ.h"

    ptr    = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/whole_chip_pattern_CAM_B_CQ.h"


    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    ptrCam = ptr;
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern_CAMSV_0.h"

    ptr    = ((IspDrvCamsv**)_pCamsv)[CAMSV_1-CAM_MAX];
    ptrCam = ptr;
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern_CAMSV_1.h"

    const MUINT32 seninf0_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/whole_chip_pattern_seninf_0.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/whole_chip_pattern_seninf_1.h"
        0xdeadbeef
    };

    const MUINT32 seninf2_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern_SENINF2.h"
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/whole_chip_pattern_seninf_2.h"
        0xdeadbeef
    };

    const MUINT32 seninf3_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern_SENINF3.h"
        0xdeadbeef
    };

    const MUINT32 seninf4_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/camsv_whole_chip_pattern_SENINF4.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake0", 1, &snrAry[0], seninf0_golden);
    fakedrvsensor.powerOn("fake1", 1, &snrAry[1], seninf1_golden);
    fakedrvsensor.powerOn("fake2", 1, &snrAry[2], seninf2_golden);
    fakedrvsensor.powerOn("fake3", 1, &snrAry[3], seninf3_golden);
    fakedrvsensor.powerOn("fake4", 1, &snrAry[4], seninf4_golden);
    return 0;
}

MINT32 Pattern_Loading_14_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char pattern_bpci_a_0_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame0/bpci_a_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_a_0_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_a_0_tbl)/sizeof(pattern_bpci_a_0_tbl[0]);

    static const unsigned char pattern_bpci_b_0_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame0/bpci_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_b_0_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_b_0_tbl)/sizeof(pattern_bpci_b_0_tbl[0]);

    static const unsigned char pattern_lsci_a_0_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame0/lsci_a_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_a_0_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_a_0_tbl)/sizeof(pattern_lsci_a_0_tbl[0]);

    static const unsigned char pattern_lsci_b_0_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame0/lsci_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_b_0_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_b_0_tbl)/sizeof(pattern_lsci_b_0_tbl[0]);

    static const unsigned char pattern_pdi_a_0_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame0/pdi_a_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_a_0_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_a_0_tbl)/sizeof(pattern_pdi_a_0_tbl[0]);

    static const unsigned char pattern_pdi_b_0_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame0/pdi_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_b_0_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_b_0_tbl)/sizeof(pattern_pdi_b_0_tbl[0]);

    static const unsigned char pattern_cq_dat_0_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame0/cq_data_0.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr = pattern_cq_dat_0_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength = sizeof(pattern_cq_dat_0_tbl)/sizeof(pattern_cq_dat_0_tbl[0]);
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr = 0x53170800; //from image.list in pattern from DE

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame0/CAM_MULTI_SIMUL_BINNING_FRAME0_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame0/CAM_MULTI_SIMUL_BINNING_FRAME0_CAMB.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame0/CAM_MULTI_SIMUL_BINNING_FRAME0_CAMC.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame0/CAM_MULTI_SIMUL_BINNING_FRAME0_SENINF1.h"
        0xdeadbeef
    };
    const MUINT32 seninf2_golden[] = {
     #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame0/CAM_MULTI_SIMUL_BINNING_FRAME0_SENINF2.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        fakedrvsensor.powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    #endif
    return 0;
}


MINT32 Pattern_Loading_14_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char pattern_bpci_a_1_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame1/bpci_a_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_a_1_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_a_1_tbl)/sizeof(pattern_bpci_a_1_tbl[0]);

    static const unsigned char pattern_bpci_b_1_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame1/bpci_b_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_b_1_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_b_1_tbl)/sizeof(pattern_bpci_b_1_tbl[0]);

    static const unsigned char pattern_lsci_a_1_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame1/lsci_a_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_a_1_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_a_1_tbl)/sizeof(pattern_lsci_a_1_tbl[0]);

    static const unsigned char pattern_lsci_b_1_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame1/lsci_b_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_b_1_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_b_1_tbl)/sizeof(pattern_lsci_b_1_tbl[0]);

    static const unsigned char pattern_pdi_a_1_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame1/pdi_a_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_a_1_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_a_1_tbl)/sizeof(pattern_pdi_a_1_tbl[0]);

    static const unsigned char pattern_pdi_b_1_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame1/pdi_b_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_b_1_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_b_1_tbl)/sizeof(pattern_pdi_b_1_tbl[0]);

    static const unsigned char pattern_cq_dat_1_tbl[] = {
            #include "Emulation/CAM_MULTI_SIMUL_BINNING/dat/frame1/cq_data_1.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr = pattern_cq_dat_1_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength = sizeof(pattern_cq_dat_1_tbl)/sizeof(pattern_cq_dat_1_tbl[0]);
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr = 0x5453c600; //from image.list in pattern from DE

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame1/CAM_MULTI_SIMUL_BINNING_FRAME1_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame1/CAM_MULTI_SIMUL_BINNING_FRAME1_CAMB.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame1/CAM_MULTI_SIMUL_BINNING_FRAME1_CAMC.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame1/CAM_MULTI_SIMUL_BINNING_FRAME1_SENINF1.h"
        0xdeadbeef
    };
    const MUINT32 seninf2_golden[] = {
     #include "Emulation/CAM_MULTI_SIMUL_BINNING/inc/frame1/CAM_MULTI_SIMUL_BINNING_FRAME1_SENINF2.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        fakedrvsensor.powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    #endif
    return 0;
}

MINT32 Pattern_Loading_15_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char pattern_bpci_a_15_0_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame0/bpci_a_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_a_15_0_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_a_15_0_tbl)/sizeof(pattern_bpci_a_15_0_tbl[0]);

    static const unsigned char pattern_bpci_b_15_0_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame0/bpci_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_b_15_0_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_b_15_0_tbl)/sizeof(pattern_bpci_b_15_0_tbl[0]);

    static const unsigned char pattern_bpci_c_15_0_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame0/bpci_c_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_c_15_0_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_c_15_0_tbl)/sizeof(pattern_bpci_c_15_0_tbl[0]);

    static const unsigned char pattern_lsci_a_15_0_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame0/lsci_a_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_a_15_0_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_a_15_0_tbl)/sizeof(pattern_lsci_a_15_0_tbl[0]);

    static const unsigned char pattern_lsci_b_15_0_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame0/lsci_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_b_15_0_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_b_15_0_tbl)/sizeof(pattern_lsci_b_15_0_tbl[0]);

    static const unsigned char pattern_lsci_c_15_0_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame0/lsci_c_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_c_15_0_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_c_15_0_tbl)/sizeof(pattern_lsci_c_15_0_tbl[0]);

    static const unsigned char pattern_pdi_a_15_0_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame0/pdi_a_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_a_15_0_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_a_15_0_tbl)/sizeof(pattern_pdi_a_15_0_tbl[0]);

    static const unsigned char pattern_pdi_b_15_0_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame0/pdi_b_0.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_b_15_0_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_b_15_0_tbl)/sizeof(pattern_pdi_b_15_0_tbl[0]);

    static const unsigned char pattern_cq_dat_15_0_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame0/cq_data_0.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr = pattern_cq_dat_15_0_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength = sizeof(pattern_cq_dat_15_0_tbl)/sizeof(pattern_cq_dat_15_0_tbl[0]);
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr = 0x56656000; //from image.list in pattern from DE

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame0/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME0_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame0/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME0_CAMB.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame0/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME0_CAMC.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame0/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME0_SENINF1.h"
        0xdeadbeef
    };
    const MUINT32 seninf2_golden[] = {
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame0/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME0_SENINF2.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        fakedrvsensor.powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    #endif
    return 0;
}


MINT32 Pattern_Loading_15_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char pattern_bpci_a_15_1_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame1/bpci_a_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_a_15_1_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_a_15_1_tbl)/sizeof(pattern_bpci_a_15_1_tbl[0]);

    static const unsigned char pattern_bpci_b_15_1_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame1/bpci_b_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_b_15_1_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_b_15_1_tbl)/sizeof(pattern_bpci_b_15_1_tbl[0]);

    static const unsigned char pattern_lsci_a_15_1_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame1/lsci_a_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_a_15_1_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_a_15_1_tbl)/sizeof(pattern_lsci_a_15_1_tbl[0]);

    static const unsigned char pattern_lsci_b_15_1_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame1/lsci_b_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_b_15_1_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_b_15_1_tbl)/sizeof(pattern_lsci_b_15_1_tbl[0]);

    static const unsigned char pattern_pdi_a_15_1_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame1/pdi_a_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_a_15_1_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_a_15_1_tbl)/sizeof(pattern_pdi_a_15_1_tbl[0]);

    static const unsigned char pattern_pdi_b_15_1_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame1/pdi_b_1.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_b_15_1_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_b_15_1_tbl)/sizeof(pattern_pdi_b_15_1_tbl[0]);

    static const unsigned char pattern_cq_dat_15_1_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame1/cq_data_1.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr = pattern_cq_dat_15_1_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength = sizeof(pattern_cq_dat_15_1_tbl)/sizeof(pattern_cq_dat_15_1_tbl[0]);
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr = 0x549b5800; //from image.list in pattern from DE

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame1/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME1_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame1/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME1_CAMB.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame1/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME1_CAMC.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame1/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME1_SENINF1.h"
        0xdeadbeef
    };
    const MUINT32 seninf2_golden[] = {
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame1/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME1_SENINF2.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        fakedrvsensor.powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    #endif
    return 0;
}

MINT32 Pattern_Loading_15_2(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char pattern_bpci_a_15_2_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame2/bpci_a_2.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_a_15_2_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_a_15_2_tbl)/sizeof(pattern_bpci_a_15_2_tbl[0]);

    static const unsigned char pattern_bpci_b_15_2_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame2/bpci_b_2.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_b_15_2_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_b_15_2_tbl)/sizeof(pattern_bpci_b_15_2_tbl[0]);

    static const unsigned char pattern_bpci_c_15_2_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame2/bpci_c_2.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_c_15_2_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_c_15_2_tbl)/sizeof(pattern_bpci_c_15_2_tbl[0]);

    static const unsigned char pattern_lsci_a_15_2_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame2/lsci_a_2.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_a_15_2_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_a_15_2_tbl)/sizeof(pattern_lsci_a_15_2_tbl[0]);

    static const unsigned char pattern_lsci_b_15_2_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame2/lsci_b_2.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_b_15_2_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_b_15_2_tbl)/sizeof(pattern_lsci_b_15_2_tbl[0]);

    static const unsigned char pattern_lsci_c_15_2_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame2/lsci_c_2.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_c_15_2_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_c_15_2_tbl)/sizeof(pattern_lsci_c_15_2_tbl[0]);

    static const unsigned char pattern_pdi_a_15_2_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame2/pdi_a_2.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_a_15_2_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_a_15_2_tbl)/sizeof(pattern_pdi_a_15_2_tbl[0]);

    static const unsigned char pattern_pdi_b_15_2_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame2/pdi_b_2.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_b_15_2_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_b_15_2_tbl)/sizeof(pattern_pdi_b_15_2_tbl[0]);

    static const unsigned char pattern_cq_dat_15_2_tbl[] = {
            #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/dat/frame2/cq_data_2.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr = pattern_cq_dat_15_2_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength = sizeof(pattern_cq_dat_15_2_tbl)/sizeof(pattern_cq_dat_15_2_tbl[0]);
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr = 0x52493800; //from image.list in pattern from DE

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame2/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME2_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame2/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME2_CAMB.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame2/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME2_CAMC.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame2/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME2_SENINF1.h"
        0xdeadbeef
    };
    const MUINT32 seninf2_golden[] = {
    #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/inc/frame2/CAM_MULTI_DYNAMIC_TWIN_BINNING_FRAME2_SENINF2.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        fakedrvsensor.powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    #endif
    return 0;
}


MINT32 Pattern_Loading_30(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char pattern_bpci_tbl[] = {
        #include "Emulation/CAM_SINGLE_TM/CAM_SINGLE_TM_BPCI_A.h"
    };
    LOG_INF("Loading pattern 30\n");

    /* save dmai buffer location, for latter memory allocation and loading */

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_tbl)/sizeof(pattern_bpci_tbl[0]);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_SINGLE_TM/CAM_SINGLE_TM_CAMA.h"
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,0);

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_SINGLE_TM/CAM_SINGLE_TM_UNI.h"
    UNI_WRITE_BITS(ptr,CAM_UNI_TOP_MISC,DB_EN,1);

    //put here for pix mode alignment

    const MUINT32 seninf0_golden[] = {
        #include "Emulation/CAM_SINGLE_TM/CAM_SINGLE_TM_SENINF.h"
        0xdeadbeef
    };

    fakedrvsensor.powerOn("fake", 1, &snrAry,seninf0_golden);

    #endif
    return 0;
}
char * ModuleName(MUINT32 module)
{
    char *_name;

    switch(module) {
    case CAM_A:
        _name = (char *)"CAM_A-";
        break;
    case CAM_B:
        _name = (char *)"CAM_B-";
        break;
    case CAM_C:
        _name = (char *)"CAM_C-";
        break;
    default:
        _name = (char *)"CAM_Unknown-";
        break;
    }

    return _name;
}


MBOOL Beyond_Compare(MUINT32 module, MUINT32 _DMA_EN, MUINT32 _DMA_BIT,
                     IMEM_BUF_INFO***** pBuf,
                     const unsigned char *_table, const char *dma_name,
                     EDMA_EN_GROUPS _DMA_EN_GROUP,
                     MUINT32 dma_addr_offset = 0,
                     std::vector<skipPairSt> *pSkips = NULL)
{
    MBOOL   ret = MTRUE;
    MUINT32 DMA_EN = 0, _tmp = 0, _cnt = 0, skip_itr = 0, unit_size;
    MUINT32 currentRow = 0, currentByte = 0, totalRow = 0;
    MUINT32 *pMem = NULL, *pTable = (MUINT32*)_table;

    MUINT32  ComPareMASK = 0xFFFFFFFF;

    typedef enum{
        OUT_OF_SPECIAL_SKIP_AREA = 0,
        ENTER_SPECIAL_SKIP_AREA = 1,
        SPACIAL_SKIP_AREA_STASUS_NUM = 2,
    } E_SPCIAL_SKIP_AERA_STATUS;

    MUINT32 specialSkipAreaFlag = OUT_OF_SPECIAL_SKIP_AREA;
    MUINT32 nextCurrentByte = 0;
    MINT32 different1 = 0;
    MINT32 different2 = 0;
    MBOOL   logicalA = 0;
    MBOOL   logicalB = 0;
    MUINT32 Remain = 0;
    #if DUMP_REG_TO_FILE
    char    *pMemOneByte   = NULL;
    char    pDirName[64]   = "TestCase-";
    char    pFileName[256] = {0};
    char    *pModuleName   = NULL;
    FILE    *fp;
    pModuleName = ModuleName(module);
    strncat(pDirName, pModuleName, strlen(pModuleName));
    strncat(pFileName, pDirName, strlen(pDirName));
    strncat(pFileName, dma_name, strlen(dma_name));
    LOG_INF("SaveFileName = %s\n",pFileName);
    fp = fopen(pFileName, "w+");
    if (NULL == fp) {
        LOG_ERR("Open DumpRegFile Error\n");
    }
    #endif

    unit_size = sizeof(*pMem);
    _tmp = (_DMA_EN & _DMA_BIT);
    _cnt = 0;

    while(_tmp != 0){
        _cnt++;
        _tmp >>= 1;
    }

    if (pBuf[module][_cnt][0][_DMA_EN_GROUP]->size == 0) {
        LOG_INF("%s no buf for bit true, bypass", dma_name);
        goto _EXIT_COMPARE;
    }
    if ((unit_size & (unit_size-1)) != 0) {
        LOG_INF("not yet support non 2'pow unit size\n");
        goto _EXIT_COMPARE;
    }

    pMem = (MUINT32*)(pBuf[module][_cnt][0][_DMA_EN_GROUP]->virtAddr +
                dma_addr_offset);

    totalRow = ((pBuf[module][_cnt][0][_DMA_EN_GROUP]->size - dma_addr_offset) /
                    unit_size);

    #if DUMP_REG_TO_FILE
    pMemOneByte = (char *)pMem;
    #endif

    LOG_INF("##################\nmodule:%d dmx_bit:%d verify#:%d offset:0x%x\n",
        module, _cnt, pBuf[module][_cnt][0][_DMA_EN_GROUP]->size,
        dma_addr_offset);


    for (currentRow = 0; currentRow < totalRow; ) {
        if (pSkips) {
            if (skip_itr < pSkips->size()) {
                currentByte = (currentRow << 2);
                nextCurrentByte = (currentByte + 4);
                if ( currentByte == pSkips->at(skip_itr).offset) {
                    ComPareMASK = 0xFFFFFFFF;
                    #if SHOW_MATCH_SKIP_REGION //debug only
                    LOG_INFL("%s matching skip region: "
                        "offset:%d(row:%d)_bytes:%d jump row: %d\n",
                        dma_name,
                        pSkips->at(skip_itr).offset,
                        pSkips->at(skip_itr).offset/unit_size,
                        pSkips->at(skip_itr).nbyte,
                        pSkips->at(skip_itr).nbyte/unit_size);
                    #endif
                    if ((pSkips->at(skip_itr).nbyte  & (unit_size-1)) ||
                        (pSkips->at(skip_itr).offset & (unit_size-1))) {
                        LOG_ERRL("%s error: currently only support %dbyte "
                            "don't care data pattern\n", dma_name, unit_size);
                    }
                    else {
                        currentRow += (pSkips->at(skip_itr).nbyte/unit_size);
                        pMem       += (pSkips->at(skip_itr).nbyte/unit_size);
                        pTable     += (pSkips->at(skip_itr).nbyte/unit_size);

                        skip_itr++;

                        continue;
                    }
                } else {

                   ComPareMASK = 0xFFFFFFFF;

                   #if SHOW_MATCH_SKIP_REGION //debug only
                   LOG_INFL("%s No Complete Skip Match Area "
                            "currentRow=%d "
                            "currentByte=%d "
                            "nextCurrentByte=%d "
                            "skip_itr=%d "
                            "pSkips->at(%d).offset=%d \n"
                            "pSkips->at(%d).nbyte=%d "
                            "(*src)=0x%x (*golden)=0x%x "
                            "@src=0x%x golden=0x%x ",
                            dma_name,
                            currentRow,
                            currentByte,
                            nextCurrentByte,
                            skip_itr,
                            skip_itr, pSkips->at(skip_itr).offset,
                            skip_itr, pSkips->at(skip_itr).nbyte,
                            (*pMem), (*pTable), pMem, pTable);
                    #endif

                    different1 = (pSkips->at(skip_itr).offset - currentByte);
                    different2 = (nextCurrentByte - pSkips->at(skip_itr).offset);
                    logicalA = (different1 >= 0);
                    logicalB = (different2 >= 0);

                    if((different1 > 0) && (different2 > 0)) {

                       specialSkipAreaFlag = ENTER_SPECIAL_SKIP_AREA;

                       Remain = (pSkips->at(skip_itr).offset % currentByte);
                       switch(Remain) {
                           case(1):
                               ComPareMASK = 0x000000FF;
                               break;
                           case(2):
                               ComPareMASK = 0x0000FFFF;
                               break;
                           case(3):
                               ComPareMASK = 0x00FFFFFF;
                               break;
                           default:
                               ComPareMASK = 0xFFFFFFFF;
                               break;
                       }
                       #if SHOW_MATCH_SKIP_REGION //debug only
//                       LOG_INFL("%s Sepcial Skip Match Area1:"
//                                "different1=%d different2=%d "
//                                "logicalA=%d logicalB=%d "
//                                "logicalA&&logicalB=%d \n"
//                                "currentRow=%d "
//                                "currentByte=%d "
//                                "nextCurrentByte=%d "
//                                "skip_itr=%d "
//                                "pSkips->at(%d).offset=%d \n"
//                                "pSkips->at(%d).nbyte=%d "
//                                "ComPareMASK=0x%x "
//                                "(ComPareMASK)&(*src)=0x%x "
//                                "(ComPareMASK)&(*golden)=0x%x "
//                                "(*src)=0x%x (*golden)=0x%x "
//                                "@src=0x%x golden=0x%x ",
//                                dma_name, different1, different2,
//                                logicalA, logicalB, (logicalA&&logicalB),
//                                currentRow,
//                                currentByte,
//                                nextCurrentByte,
//                                skip_itr,
//                                skip_itr, pSkips->at(skip_itr).offset,
//                                skip_itr, pSkips->at(skip_itr).nbyte,
//                                ComPareMASK,
//                                ((ComPareMASK)&(*pMem)),
//                                ((ComPareMASK)&(*pTable)),
//                                (*pMem), (*pTable), pMem, pTable);
                       #endif
                       if (((ComPareMASK)&(*pMem)) != ((ComPareMASK)&(*pTable))) {

                           LOG_ERRL("%s Sepcial Skip Match Area1 bit true fail: "
                                    "currentRow=%d "
                                    "currentByte=%d "
                                    "nextCurrentByte=%d "
                                    "skip_itr=%d "
                                    "pSkips->at(%d).offset=%d \n"
                                    "pSkips->at(%d).nbyte=%d "
                                    "ComPareMASK=0x%x "
                                    "(ComPareMASK)&(*src)=0x%x "
                                    "(ComPareMASK)&(*golden)=0x%x "
                                    "(*src)=0x%x (*golden)=0x%x "
                                    "@src=%p golden=%p ",
                                    dma_name,
                                    currentRow,
                                    currentByte,
                                    nextCurrentByte,
                                    skip_itr,
                                    skip_itr, pSkips->at(skip_itr).offset,
                                    skip_itr, pSkips->at(skip_itr).nbyte,
                                    ComPareMASK,
                                    ((ComPareMASK)&(*pMem)),
                                    ((ComPareMASK)&(*pTable)),
                                    (*pMem), (*pTable), pMem, pTable);

                           break;
                       }
                       currentRow++;
                       pMem++;
                       pTable++;
                       continue;
                  }
                  if(ENTER_SPECIAL_SKIP_AREA == specialSkipAreaFlag) {
                      if(currentByte < (pSkips->at(skip_itr).offset + 4)) {
                        ComPareMASK = 0x00000000;
                        #if SHOW_MATCH_SKIP_REGION //debug only
                        LOG_INFL("%s Sepcial Skip Match Area2:"
                                 "currentRow=%d "
                                 "currentByte=%d "
                                 "nextCurrentByte=%d "
                                 "skip_itr=%d "
                                 "pSkips->at(%d).offset=%d \n"
                                 "pSkips->at(%d).nbyte=%d "
                                 "ComPareMASK=0x%x "
                                 "(ComPareMASK)&(*src)=0x%x "
                                 "(ComPareMASK)&(*golden)=0x%x "
                                 "(*src)=0x%x (*golden)=0x%x "
                                 "@src=0x%x golden=0x%x ",
                                 dma_name,
                                 currentRow,
                                 currentByte,
                                 nextCurrentByte,
                                 skip_itr,
                                 skip_itr, pSkips->at(skip_itr).offset,
                                 skip_itr, pSkips->at(skip_itr).nbyte,
                                 ComPareMASK,
                                 ((ComPareMASK)&(*pMem)),
                                 ((ComPareMASK)&(*pTable)),
                                 (*pMem), (*pTable), pMem, pTable);
                        #endif

                        if (((ComPareMASK)&(*pMem)) != ((ComPareMASK)&(*pTable))) {

                            LOG_ERRL("%s Sepcial Skip Match Area2 bit true fail: "
                                    "currentRow=%d "
                                    "currentByte=%d "
                                    "nextCurrentByte=%d "
                                    "skip_itr=%d "
                                    "pSkips->at(%d).offset=%d \n"
                                    "pSkips->at(%d).nbyte=%d "
                                    "ComPareMASK=0x%x "
                                    "(ComPareMASK)&(*src)=0x%x "
                                    "(ComPareMASK)&(*golden)=0x%x "
                                    "(*src)=0x%x (*golden)=0x%x "
                                    "@src=%p golden=%p ",
                                    dma_name,
                                    currentRow,
                                    currentByte,
                                    nextCurrentByte,
                                    skip_itr,
                                    skip_itr, pSkips->at(skip_itr).offset,
                                    skip_itr, pSkips->at(skip_itr).nbyte,
                                    ComPareMASK,
                                    ((ComPareMASK)&(*pMem)),
                                    ((ComPareMASK)&(*pTable)),
                                    (*pMem), (*pTable), pMem, pTable);
                             break;
                        }
                        currentRow += ((pSkips->at(skip_itr).nbyte / unit_size));
                        pMem       += ((pSkips->at(skip_itr).nbyte / unit_size));
                        pTable     += ((pSkips->at(skip_itr).nbyte / unit_size));
                        skip_itr++;
                        specialSkipAreaFlag = OUT_OF_SPECIAL_SKIP_AREA; //Leave SpceialSkipArea.
                        continue;
                   }
                 } // if(1 == specialSkipAreaFlag)
              } // if ( currentByte == pSkips->at(skip_itr).offset)
            }// if (skip_itr < pSkips->size())
        }// if (pSkips)

        if (((ComPareMASK)&(*pMem)) != ((ComPareMASK)&(*pTable))) {

            LOG_ERRL("%s Normal Area bit true fail: "
                    "currentRow=%d "
                    "currentByte=%d "
                    "nextCurrentByte=%d "
                    "skip_itr=%d "
                    "ComPareMASK=0x%x "
                    "(ComPareMASK)&(*src)=0x%x "
                    "(ComPareMASK)&(*golden)=0x%x \n"
                    "(*src)=0x%x (*golden)=0x%x "
                    "@src=%p golden=%p ",
                    dma_name,
                    currentRow,
                    currentByte,
                    nextCurrentByte,
                    skip_itr,
                    ComPareMASK,
                    ((ComPareMASK)&(*pMem)),
                    ((ComPareMASK)&(*pTable)),
                    (*pMem), (*pTable), pMem, pTable);


            if (currentRow >= 1){
                LOG_ERRL("%s bit true fail,prev1: 0x%x_0x%x\n",
                    dma_name, *(pMem-1), *(pTable-1));
            }
            if (currentRow >= 2){
                LOG_ERRL("%s bit true fail,prev2: 0x%x_0x%x\n",
                    dma_name, *(pMem-2), *(pTable-2));
            }
            break;
        }

        currentRow++;
        pMem++;
        pTable++;

    }// for (currentRow = 0; currentRow < totalRow; )

    if(currentRow != totalRow){
        ret = MFALSE;
        LOG_ERR("%s bit true fail,(currentRow_totalRow)=(%d_%d)\n",
            dma_name, currentRow, totalRow);
    }
    else{
        ret = MTRUE;
        LOG_INF("%s bit true pass\n",dma_name);
    }

    #if DUMP_REG_TO_FILE
    if (NULL != fp) {
        LOG_INF("%d", _tmp);
        for (MUINT32 i = 0; i < _tmp * 4; i+=4) {
            fprintf(fp, "0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
                    *(pMemOneByte+i),   *(pMemOneByte+i+1),
                    *(pMemOneByte+i+2), *(pMemOneByte+i+3));
        #if DUMP_REG_TO_FILE_DISPLAY
        LOG_INF("i = %ld ,0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", i,
                *(pMemOneByte+i),   *(pMemOneByte+i+1),
                *(pMemOneByte+i+2), *(pMemOneByte+i+3));
        #endif
        }
        fclose(fp);
    }
    #endif
    LOG_INF("###########################\n");

_EXIT_COMPARE:

    return ret;
}

MBOOL
Beyond_Compare_Subsamples(
    ISP_HW_MODULE module,
    MUINT32 subsampleInx,
    IMEM_BUF_INFO ******pBuf,
    const unsigned char *_table,
    EDMA_EN_GROUPS _DMA_EN_GROUP,/* ECQ_PAGE_TYPE_GROUP */
    ECAMSV_FBC_GROUP camsvFbcGroup,/* ECQ_PAGES_NUM_GROUP */
    MUINT32 dma_addr_offset,
    std::vector<skipPairSt> *pSkips)
{
    MBOOL   ret = MTRUE;
    MUINT32 DMA_EN = 0, _tmp = 0, _cnt = 0, skip_itr = 0, unit_size;
    MUINT32 currentRow = 0, currentByte = 0, totalRow = 0;
    MUINT32 *pMem = NULL, *pTable = (MUINT32*)_table;

    MUINT32  ComPareMASK = 0xFFFFFFFF;
    #if DUMP_REG_TO_FILE
    char    *pMemOneByte   = NULL;
    char    pDirName[64]   = "TestCase-";
    char    pFileName[256] = {0};
    char    *pModuleName   = NULL;
    FILE    *fp;
    pModuleName = ModuleName(module);
    strncat(pDirName, pModuleName, strlen(pModuleName));
    strncat(pFileName, pDirName, strlen(pDirName));
    //strncat(pFileName, dma_name, strlen(dma_name));
    LOG_INF("SaveFileName = %s\n",pFileName);
    fp = fopen(pFileName, "w+");
    if (NULL == fp) {
        LOG_ERR("Open DumpRegFile Error\n");
    }
    #endif

    unit_size = sizeof(*pMem);

    if (pBuf[module][subsampleInx][0][_DMA_EN_GROUP][camsvFbcGroup]->size == 0) {
        LOG_INF("subsampleInx %d no buf for bit true, bypass", subsampleInx);
        goto _EXIT_COMPARE;
    }
    if ((unit_size & (unit_size-1)) != 0) {
        LOG_INF("not yet support non 2'pow unit size\n");
        goto _EXIT_COMPARE;
    }

    pMem = (MUINT32*)(pBuf[module][subsampleInx][0][_DMA_EN_GROUP]
        [camsvFbcGroup]->virtAddr + dma_addr_offset);

    totalRow = ((pBuf[module][subsampleInx][0][_DMA_EN_GROUP][camsvFbcGroup]
                ->size - dma_addr_offset) / unit_size);

    #if DUMP_REG_TO_FILE
    pMemOneByte = (char *)pMem;
    #endif
    LOG_INF("###########################\n"
            "module:%d subsampleInx:%d verify#:%d offset:0x%x\n",
            module, subsampleInx,
            pBuf[module][subsampleInx][0][_DMA_EN_GROUP][camsvFbcGroup]->size,
            dma_addr_offset);

    for (currentRow = 0; currentRow < totalRow; ) {
        if (pSkips) {
            if (skip_itr < pSkips->size()) {
                currentByte = currentRow << 2;
                if ( currentByte == pSkips->at(skip_itr).offset) {
                    ComPareMASK = 0xFFFFFFFF;
                    #if SHOW_MATCH_SKIP_REGION //debug only
                    LOG_INF("subsampleInx %d matching skip region: "
                        "offset:%d(row:%d)_bytes:%d jump row: %d\n",
                        subsampleInx,
                        pSkips->at(skip_itr).offset,
                        pSkips->at(skip_itr).offset/unit_size,
                        pSkips->at(skip_itr).nbyte,
                        pSkips->at(skip_itr).nbyte/unit_size);
                    #endif
                    if ((pSkips->at(skip_itr).nbyte  & (unit_size-1)) ||
                        (pSkips->at(skip_itr).offset & (unit_size-1))) {
                        LOG_ERR("subsampleInx %d error: currently only support %dbyte "
                            "don't care data pattern\n", subsampleInx, unit_size);
                    }
                    else {
                        currentRow += (pSkips->at(skip_itr).nbyte/unit_size);
                        pMem       += (pSkips->at(skip_itr).nbyte/unit_size);
                        pTable     += (pSkips->at(skip_itr).nbyte/unit_size);
                        skip_itr++;

                        continue;
                    }
                } else {

                #if 0
                    MUINT32 _offset = pSkips->at(skip_itr).offset - currentByte;
                    switch(_offset){
                        case 3:
                        case 1:
                           break;
                        case 2:
                            LOG_INF("currentRow=0x%x, currentByte=0x%x, "
                                    "skip_itr=0x%x, "
                                    "pSkips->at(0x%x).offset=0x%x "
                                    "pSkips->at(0x%x).nbyte =0x%x\n",
                                    currentRow, (currentRow << 2),
                                    skip_itr, skip_itr,
                                    pSkips->at(skip_itr).offset,
                                    skip_itr,
                                    pSkips->at(skip_itr).nbyte);

                            ComPareMASK = 0x0000FFFF;
                            if ((pSkips->at(skip_itr).nbyte +
                                 pSkips->at(skip_itr).offset) & (unit_size - 1)) {
                                LOG_ERR("subsampleInx %d error: "
                                        "currently only support TOTAL %dbyte "
                                        "don't care data pattern\n",
                                        subsampleInx, unit_size);
                            } else {

                                if (((ComPareMASK)&(*pMem)) !=
                                    ((ComPareMASK)&(*pTable))) {
                                    LOG_ERR("subsampleInx %d bit true fail, "
                                            "@src=0x%x golden=0x%x "
                                            "ComPareMask = 0x%x\n",
                                            subsampleInx, pMem, pTable, ComPareMASK);

                                    LOG_ERR("subsampleInx %d bit true fail "
                                            "offset=0x%x 0x%x_0x%x skip_itr=0x%x\n",
                                            subsampleInx, currentRow,
                                            ((ComPareMASK)&(*pMem)),
                                            ((ComPareMASK)&(*pTable)), skip_itr);

                                    if (currentRow >= 1){
                                        LOG_ERR("subsampleInx %d bit true fail, "
                                                "prev1: 0x%x_0x%x\n",
                                                subsampleInx, *(pMem-1), *(pTable-1));
                                    }
                                    if (currentRow >= 2){
                                        LOG_ERR("subsampleInx %d bit true fail, "
                                                "prev2: 0x%x_0x%x\n",
                                                subsampleInx, *(pMem-2), *(pTable-2));
                                    }
                                } else {
                                }

                                currentRow += (((pSkips->at(skip_itr).nbyte+2)/unit_size)+1);
                                pMem       += (((pSkips->at(skip_itr).nbyte+2)/unit_size)+1);
                                pTable     += (((pSkips->at(skip_itr).nbyte+2)/unit_size)+1);

                                skip_itr++;

                                continue;
                            }
                        break;
                      default:
                        break;
                    }
                    #endif
                }
            }
        }

        if (((ComPareMASK)&(*pMem)) != ((ComPareMASK)&(*pTable))) {
            LOG_ERR("subsampleInx %d bit true fail,@src=%p golden=%p ComPareMask = 0x%x\n",
                    subsampleInx, pMem, pTable, ComPareMASK);

            LOG_ERR("subsampleInx %d bit true fail offset=0x%x 0x%x_0x%x skip_itr=0x%x\n",
                    subsampleInx, currentRow, ((ComPareMASK)&(*pMem)),
                    ((ComPareMASK)&(*pTable)), skip_itr);

            if (currentRow >= 1){
                LOG_ERR("subsampleInx %d bit true fail,prev1: 0x%x_0x%x\n",
                    subsampleInx, *(pMem-1), *(pTable-1));
            }
            if (currentRow >= 2){
                LOG_ERR("subsampleInx %d bit true fail,prev2: 0x%x_0x%x\n",
                    subsampleInx, *(pMem-2), *(pTable-2));
            }
            break;
        }else{
            LOG_INF("ComPareMASK = 0x%x, *pMem = 0x%x, *pTable = 0x%x",
                     ComPareMASK, *pMem, *pTable);
        }

        currentRow++;
        pMem++;
        pTable++;

    }
    if(currentRow != totalRow){
        ret = MFALSE;
        LOG_ERR("subsampleInx %d bit true fail,(currentRow_totalRow)=(%d_%d)\n",
            subsampleInx, currentRow, totalRow);
    }
    else{
        ret = MTRUE;
        LOG_INF("subsampleInx %d bit true pass\n",subsampleInx);
    }

    #if DUMP_REG_TO_FILE
    if (NULL != fp) {
        LOG_INF("%d", _tmp);
        for (MUINT32 i = 0; i < _tmp * 4; i+=4) {
            fprintf(fp, "0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
                    *(pMemOneByte+i),   *(pMemOneByte+i+1),
                    *(pMemOneByte+i+2), *(pMemOneByte+i+3));
        #if DUMP_REG_TO_FILE_DISPLAY
        LOG_INF("i = %ld ,0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", i,
                *(pMemOneByte+i),   *(pMemOneByte+i+1),
                *(pMemOneByte+i+2), *(pMemOneByte+i+3));
        #endif
        }
        fclose(fp);
    }
    #endif
    LOG_INF("###########################\n");

_EXIT_COMPARE:

    return ret;
}

MBOOL
Beyond_Compare_Subsamples_MultiCQ(
    ISP_HW_MODULE module,
    MUINT32 subsampleInx,
    IMEM_BUF_INFO ******pBuf,
    const unsigned char *_table,
    EDMA_EN_GROUPS _DMA_EN_GROUP,/* ECQ_PAGE_TYPE_GROUP */
    ECAMSV_FBC_GROUP camsvFbcGroup,/* ECQ_PAGES_NUM_GROUP */
    MUINT32 dma_addr_offset,
    std::vector<skipPairSt> *pSkips,
    ECAMSV_ENQ_ADDR_TYPE enqAddrType /*ECAMSV_ENQ_ADDR_TYPE */)
{
    MBOOL   ret = MTRUE;
    MUINT32 DMA_EN = 0, _tmp = 0, _cnt = 0, skip_itr = 0, unit_size;
    MUINT32 currentRow = 0, currentByte = 0, totalRow = 0;
    MUINT32 *pMem = NULL, *pTable = (MUINT32*)_table;

    MUINT32  ComPareMASK = 0xFFFFFFFF;
    #if DUMP_REG_TO_FILE
    char    *pMemOneByte   = NULL;
    char    pDirName[64]   = "TestCase-";
    char    pFileName[256] = {0};
    char    *pModuleName   = NULL;
    FILE    *fp;
    pModuleName = ModuleName(module);
    strncat(pDirName, pModuleName, strlen(pModuleName));
    strncat(pFileName, pDirName, strlen(pDirName));
    //strncat(pFileName, dma_name, strlen(dma_name));
    LOG_INF("SaveFileName = %s\n",pFileName);
    fp = fopen(pFileName, "w+");
    if (NULL == fp) {
        LOG_ERR("Open DumpRegFile Error\n");
    }
    #endif

    unit_size = sizeof(*pMem);

    if (pBuf[module][subsampleInx][enqAddrType][_DMA_EN_GROUP][camsvFbcGroup]->size == 0) {
        LOG_INF("subsampleInx %d no buf for bit true, bypass", subsampleInx);
        goto _EXIT_COMPARE;
    }
    if ((unit_size & (unit_size-1)) != 0) {
        LOG_INF("not yet support non 2'pow unit size\n");
        goto _EXIT_COMPARE;
    }

    pMem = (MUINT32*)(pBuf[module][subsampleInx][enqAddrType][_DMA_EN_GROUP]
        [camsvFbcGroup]->virtAddr + dma_addr_offset);

    totalRow = ((pBuf[module][subsampleInx][enqAddrType][_DMA_EN_GROUP][camsvFbcGroup]
                ->size - dma_addr_offset) / unit_size);

    #if DUMP_REG_TO_FILE
    pMemOneByte = (char *)pMem;
    #endif
    LOG_INF("###########################\n"
            "module:%d subsampleInx:%d verify#:%d offset:0x%x\n",
            module, subsampleInx,
            pBuf[module][subsampleInx][enqAddrType][_DMA_EN_GROUP][camsvFbcGroup]->size,
            dma_addr_offset);

    for (currentRow = 0; currentRow < totalRow; ) {
        if (pSkips) {
            if (skip_itr < pSkips->size()) {
                currentByte = currentRow << 2;
                if ( currentByte == pSkips->at(skip_itr).offset) {
                    ComPareMASK = 0xFFFFFFFF;
                    #if SHOW_MATCH_SKIP_REGION //debug only
                    LOG_INF("subsampleInx %d matching skip region: "
                        "offset:%d(row:%d)_bytes:%d jump row: %d\n",
                        subsampleInx,
                        pSkips->at(skip_itr).offset,
                        pSkips->at(skip_itr).offset/unit_size,
                        pSkips->at(skip_itr).nbyte,
                        pSkips->at(skip_itr).nbyte/unit_size);
                    #endif
                    if ((pSkips->at(skip_itr).nbyte  & (unit_size-1)) ||
                        (pSkips->at(skip_itr).offset & (unit_size-1))) {
                        LOG_ERR("subsampleInx %d error: currently only support %dbyte "
                            "don't care data pattern\n", subsampleInx, unit_size);
                    }
                    else {
                        currentRow += (pSkips->at(skip_itr).nbyte/unit_size);
                        pMem       += (pSkips->at(skip_itr).nbyte/unit_size);
                        pTable     += (pSkips->at(skip_itr).nbyte/unit_size);
                        skip_itr++;

                        continue;
                    }
                } else {

                #if 0
                    MUINT32 _offset = pSkips->at(skip_itr).offset - currentByte;
                    switch(_offset){
                        case 3:
                        case 1:
                           break;
                        case 2:
                            LOG_INF("currentRow=0x%x, currentByte=0x%x, "
                                    "skip_itr=0x%x, "
                                    "pSkips->at(0x%x).offset=0x%x "
                                    "pSkips->at(0x%x).nbyte =0x%x\n",
                                    currentRow, (currentRow << 2),
                                    skip_itr, skip_itr,
                                    pSkips->at(skip_itr).offset,
                                    skip_itr,
                                    pSkips->at(skip_itr).nbyte);

                            ComPareMASK = 0x0000FFFF;
                            if ((pSkips->at(skip_itr).nbyte +
                                 pSkips->at(skip_itr).offset) & (unit_size - 1)) {
                                LOG_ERR("subsampleInx %d error: "
                                        "currently only support TOTAL %dbyte "
                                        "don't care data pattern\n",
                                        subsampleInx, unit_size);
                            } else {

                                if (((ComPareMASK)&(*pMem)) !=
                                    ((ComPareMASK)&(*pTable))) {
                                    LOG_ERR("subsampleInx %d bit true fail, "
                                            "@src=0x%x golden=0x%x "
                                            "ComPareMask = 0x%x\n",
                                            subsampleInx, pMem, pTable, ComPareMASK);

                                    LOG_ERR("subsampleInx %d bit true fail "
                                            "offset=0x%x 0x%x_0x%x skip_itr=0x%x\n",
                                            subsampleInx, currentRow,
                                            ((ComPareMASK)&(*pMem)),
                                            ((ComPareMASK)&(*pTable)), skip_itr);

                                    if (currentRow >= 1){
                                        LOG_ERR("subsampleInx %d bit true fail, "
                                                "prev1: 0x%x_0x%x\n",
                                                subsampleInx, *(pMem-1), *(pTable-1));
                                    }
                                    if (currentRow >= 2){
                                        LOG_ERR("subsampleInx %d bit true fail, "
                                                "prev2: 0x%x_0x%x\n",
                                                subsampleInx, *(pMem-2), *(pTable-2));
                                    }
                                } else {
                                }

                                currentRow += (((pSkips->at(skip_itr).nbyte+2)/unit_size)+1);
                                pMem       += (((pSkips->at(skip_itr).nbyte+2)/unit_size)+1);
                                pTable     += (((pSkips->at(skip_itr).nbyte+2)/unit_size)+1);

                                skip_itr++;

                                continue;
                            }
                        break;
                      default:
                        break;
                    }
                    #endif
                }
            }
        }

        if (((ComPareMASK)&(*pMem)) != ((ComPareMASK)&(*pTable))) {
            LOG_ERR("subsampleInx %d bit true fail,@src=%p golden=%p ComPareMask = 0x%x\n",
                    subsampleInx, pMem, pTable, ComPareMASK);

            LOG_ERR("subsampleInx %d bit true fail offset=0x%x 0x%x_0x%x skip_itr=0x%x\n",
                    subsampleInx, currentRow, ((ComPareMASK)&(*pMem)),
                    ((ComPareMASK)&(*pTable)), skip_itr);

            if (currentRow >= 1){
                LOG_ERR("subsampleInx %d bit true fail,prev1: 0x%x_0x%x\n",
                    subsampleInx, *(pMem-1), *(pTable-1));
            }
            if (currentRow >= 2){
                LOG_ERR("subsampleInx %d bit true fail,prev2: 0x%x_0x%x\n",
                    subsampleInx, *(pMem-2), *(pTable-2));
            }
            break;
        }else{
            LOG_INF("ComPareMASK = 0x%x, *pMem = 0x%x, *pTable = 0x%x",
                     ComPareMASK, *pMem, *pTable);
        }

        currentRow++;
        pMem++;
        pTable++;

    }
    if(currentRow != totalRow){
        ret = MFALSE;
        LOG_ERR("subsampleInx %d bit true fail,(currentRow_totalRow)=(%d_%d)\n",
            subsampleInx, currentRow, totalRow);
    }
    else{
        ret = MTRUE;
        LOG_INF("subsampleInx %d bit true pass\n",subsampleInx);
    }

    #if DUMP_REG_TO_FILE
    if (NULL != fp) {
        LOG_INF("%d", _tmp);
        for (MUINT32 i = 0; i < _tmp * 4; i+=4) {
            fprintf(fp, "0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
                    *(pMemOneByte+i),   *(pMemOneByte+i+1),
                    *(pMemOneByte+i+2), *(pMemOneByte+i+3));
        #if DUMP_REG_TO_FILE_DISPLAY
        LOG_INF("i = %ld ,0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", i,
                *(pMemOneByte+i),   *(pMemOneByte+i+1),
                *(pMemOneByte+i+2), *(pMemOneByte+i+3));
        #endif
        }
        fclose(fp);
    }
    #endif
    LOG_INF("###########################\n");

_EXIT_COMPARE:

    return ret;
}


MINT32 Pattern_BitTrue_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    MINT32 ret  = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO***** pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0, _tmp=0, _cnt=0;
    MUINT32 *pTable = NULL, *pMem = NULL;
    (void)_ptr;(void)length;(void)_pCamsv;(void)inputInfo;

    return ret;
}


MINT32 Pattern_BitTrue_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    MINT32  ret = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO***** pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0, DMA2_EN = 0, CAMSV_DMA_EN = 0, _tmp=0, _cnt=0;
    MUINT32 *pTable = NULL, *pMem = NULL;
    std::vector<skipPairSt> skips;
    length;

    static const unsigned char crzbo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/crzbo_r1a_o_golden.dhex"
    };
    static const unsigned char crzbo_r2a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/crzbo_r2a_o_golden.dhex"
    };
    static const unsigned char crzo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/crzo_r1a_o_golden.dhex"
    };
    static const unsigned char crzo_r2a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/crzo_r2a_o_golden.dhex"
    };
    static const unsigned char imgo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/imgo_r1a_o_golden.dhex"
    };
    static const unsigned char lmvo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/lmvo_r1a_o_golden.dhex"
    };
    static const unsigned char pdo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/pdo_r1a_o_golden.dhex"
    };
    static const unsigned int pdo_r1a_o_golden_skippair[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/pdo_r1a_o_golden_skippair.txt"
    };

    static const unsigned char rrzo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/rrzo_r1a_o_golden.dhex"
    };
    static const unsigned char rsso_r2a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/rsso_r2a_o_golden.dhex"
    };
    static const unsigned char ufeo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/ufeo_r1a_o_golden.dhex"
    };
    static const unsigned char ufgo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/ufgo_r1a_o_golden.dhex"
    };
    static const unsigned char yuvbo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/yuvbo_r1a_o_golden.dhex"
    };
    static const unsigned char yuvco_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/yuvco_r1a_o_golden.dhex"
    };
    static const unsigned char yuvo_r1a_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/whole_chip_pattern/golden/yuvo_r1a_o_golden.dhex"
    };

    static const unsigned char top0_camsv0_imgo_o_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass/camsv_whole_chip_pattern/golden/top0_camsv0_imgo_o_golden.dhex"
    };

    pBuf    = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

    if (Beyond_Compare(CAM_A, DMA_EN, CRZBO_R1_EN_, pBuf, crzbo_r1a_o_golden,
        "crzbo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZBO_R1_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, CRZBO_R2_EN_, pBuf, crzbo_r2a_o_golden,
        "crzbo_r2a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZBO_R2_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, CRZO_R1_EN_, pBuf, crzo_r1a_o_golden,
        "crzo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZO_R1_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, CRZO_R2_EN_, pBuf, crzo_r2a_o_golden,
        "crzo_r2a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZO_R2_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, IMGO_R1_EN_, pBuf, imgo_r1a_o_golden,
        "imgo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, IMGO_R1_IMGO_OFST_ADDR, IMGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, LMVO_R1_EN_, pBuf, lmvo_r1a_o_golden,
        "lmvo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, LMVO_R1_LMVO_OFST_ADDR, LMVO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    skips.clear();
    for (MUINT32 i = 0;
        i < (sizeof(pdo_r1a_o_golden_skippair) /
             sizeof(pdo_r1a_o_golden_skippair[0]));
        i+=2) {
        skips.push_back(skipPairSt(pdo_r1a_o_golden_skippair[i],
            pdo_r1a_o_golden_skippair[i+1]));
    }

    LOG_INF("pdo_r1a_o_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    if (Beyond_Compare(CAM_A, DMA_EN, PDO_R1_EN_, pBuf, pdo_r1a_o_golden,
        "pdo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, PDO_R1_PDO_OFST_ADDR, PDO_OFST_ADDR),
        &skips ) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, RRZO_R1_EN_, pBuf, rrzo_r1a_o_golden,
        "rrzo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, RRZO_R1_RRZO_OFST_ADDR, RRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, RSSO_R2_EN_, pBuf, rsso_r2a_o_golden,
        "rsso_r2a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, RSSO_R2_RSSO_OFST_ADDR, RSSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, UFEO_R1_EN_, pBuf, ufeo_r1a_o_golden,
        "ufeo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFEO_R1_UFEO_OFST_ADDR, UFEO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, UFGO_R1_EN_, pBuf, ufgo_r1a_o_golden,
        "ufgo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFGO_R1_UFGO_OFST_ADDR, UFGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, YUVBO_R1_EN_, pBuf, yuvbo_r1a_o_golden,
        "yuvbo_r1a", DMA_EN_GROUP0) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, YUVCO_R1_EN_, pBuf, yuvco_r1a_o_golden,
        "yuvco_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, YUVCO_R1_YUVCO_OFST_ADDR, YUVCO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, YUVO_R1_EN_, pBuf, yuvo_r1a_o_golden,
        "yuvo_r1a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, YUVO_R1_YUVO_OFST_ADDR, YUVO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    ptr          = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);

    if (Beyond_Compare(CAMSV_0, CAMSV_DMA_EN, CAMSV_CTL_IMGO_EN_, pBuf,
        top0_camsv0_imgo_o_golden, "CAMSV_IMGO", DMA_EN_GROUP0) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

    return ret;

}

MINT32 Pattern_BitTrue_2(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32  ret = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO***** pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0;
    std::vector<skipPairSt> skips;
    (void)length;(void)_uni;

    static const unsigned char imgo_r1a_o_golden[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/golden/imgo_r1a_o_golden.dhex"
    };
    static const unsigned int imgo_r1a_o_golden_skippair[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/golden/imgo_r1a_o_golden_skippair.txt"
    };

    static const unsigned char rrzo_r1a_o_golden[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/golden/rrzo_r1a_o_golden.dhex"
    };
    static const unsigned int rrzo_r1a_o_golden_skippair[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/golden/rrzo_r1a_o_golden_skippair.txt"
    };

    static const unsigned char ufeo_r1a_o_golden[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/golden/ufeo_r1a_o_golden.dhex"
    };
    static const unsigned int ufeo_r1a_o_golden_skippair[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/golden/ufeo_r1a_o_golden_skippair.txt"
    };
    static const unsigned char ufgo_r1a_o_golden[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/golden/ufgo_r1a_o_golden.dhex"
    };
    static const unsigned int ufgo_r1a_o_golden_skippair[] = {
        #include "Emulation/p1_ufe_raw14_4p_ufg_raw12_1p/whole_chip_pattern/golden/ufgo_r1a_o_golden_skippair.txt"
    };


    pBuf    = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);


    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(imgo_r1a_o_golden_skippair) /
         sizeof(imgo_r1a_o_golden_skippair[0]))) < 0);
         i+=2) {

        skips.push_back(skipPairSt(imgo_r1a_o_golden_skippair[i],
            imgo_r1a_o_golden_skippair[i+1]));
    }

    LOG_INF("imgo_r1a_o_golden_skippair total dontcare pairs: %ld\n",
        skips.size());

    if (Beyond_Compare(CAM_A, DMA_EN, IMGO_R1_EN_, pBuf, imgo_r1a_o_golden,
        "imgo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, IMGO_R1_IMGO_OFST_ADDR, IMGO_OFST_ADDR),
        &skips) == MFALSE) {
        ret++;
    }

    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(rrzo_r1a_o_golden_skippair) /
         sizeof(rrzo_r1a_o_golden_skippair[0]))) < 0);
         i+=2) {

        skips.push_back(skipPairSt(rrzo_r1a_o_golden_skippair[i],
            rrzo_r1a_o_golden_skippair[i+1]));
    }

    LOG_INF("rrzo_r1a_o_golden_skippair total dontcare pairs: %ld\n",
        skips.size());

    if (Beyond_Compare(CAM_A, DMA_EN, RRZO_R1_EN_, pBuf, rrzo_r1a_o_golden,
        "rrzo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, RRZO_R1_RRZO_OFST_ADDR, RRZO_OFST_ADDR),
        &skips) == MFALSE) {
        ret++;
    }

    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(ufeo_r1a_o_golden_skippair) /
         sizeof(ufeo_r1a_o_golden_skippair[0]))) < 0);
         i+=2) {

        skips.push_back(skipPairSt(ufeo_r1a_o_golden_skippair[i],
            ufeo_r1a_o_golden_skippair[i+1]));
    }

    LOG_INF("ufeo_r1a_o_golden_skippair total dontcare pairs: %ld\n",
        skips.size());

    if (Beyond_Compare(CAM_A, DMA_EN, UFEO_R1_EN_, pBuf, ufeo_r1a_o_golden,
        "ufeo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFEO_R1_UFEO_OFST_ADDR, UFEO_OFST_ADDR),
        &skips) == MFALSE) {
        ret++;
    }

    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(ufgo_r1a_o_golden_skippair) /
         sizeof(ufgo_r1a_o_golden_skippair[0]))) < 0);
         i+=2) {

        skips.push_back(skipPairSt(ufgo_r1a_o_golden_skippair[i],
            ufgo_r1a_o_golden_skippair[i+1]));
    }

    LOG_INF("ufgo_r1a_o_golden_skippair total dontcare pairs: %ld\n",
        skips.size());

    if (Beyond_Compare(CAM_A, DMA_EN, UFGO_R1_EN_, pBuf, ufgo_r1a_o_golden,
        "ufgo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFGO_R1_UFGO_OFST_ADDR, UFGO_OFST_ADDR),
        &skips) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

    return ret;

}

MINT32 Pattern_BitTrue_3(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    MINT32  ret = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO ******pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0, DMA2_EN = 0, CAMSV_DMA_EN = 0, _tmp=0, _cnt=0,subsampleNum = 0;
    MUINT32 *pTable = NULL, *pMem = NULL;
    std::vector<skipPairSt> skips;
    length;
    typedef const unsigned char (*subsample_golden)[128];

    static const unsigned int subsample_skippair[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_skip.txt"
    };
    static const unsigned char subsample_0_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_0.dhex"
    };
    static const unsigned char subsample_1_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_1.dhex"
    };
    static const unsigned char subsample_2_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_2.dhex"
    };
    static const unsigned char subsample_3_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_3.dhex"
    };
    static const unsigned char subsample_4_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_4.dhex"
    };
    static const unsigned char subsample_5_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_5.dhex"
    };
    static const unsigned char subsample_6_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_6.dhex"
    };
    static const unsigned char subsample_7_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_7.dhex"
    };
    static const unsigned char subsample_8_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_8.dhex"
    };
    static const unsigned char subsample_9_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_9.dhex"
    };
    static const unsigned char subsample_10_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_10.dhex"
    };
    static const unsigned char subsample_11_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_11.dhex"
    };
    static const unsigned char subsample_12_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_12.dhex"
    };
    static const unsigned char subsample_13_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_13.dhex"
    };
    static const unsigned char subsample_14_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_14.dhex"
    };
    static const unsigned char subsample_15_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_15.dhex"
    };

    subsample_golden SUBSUMPLE_Golden_TAL[]=
    {   &subsample_0_golden,  &subsample_1_golden,   &subsample_2_golden,
        &subsample_3_golden,  &subsample_4_golden,   &subsample_5_golden,
        &subsample_6_golden,  &subsample_7_golden,   &subsample_8_golden,
        &subsample_9_golden,  &subsample_10_golden,  &subsample_11_golden,
        &subsample_12_golden, &subsample_13_golden,  &subsample_14_golden,
        &subsample_15_golden,
        &subsample_0_golden,  &subsample_1_golden,   &subsample_2_golden,
        &subsample_3_golden,  &subsample_4_golden,   &subsample_5_golden,
        &subsample_6_golden,  &subsample_7_golden,   &subsample_8_golden,
        &subsample_9_golden,  &subsample_10_golden,  &subsample_11_golden,
        &subsample_12_golden, &subsample_13_golden,  &subsample_14_golden,
        &subsample_15_golden
    };

    pBuf    = (IMEM_BUF_INFO******)pInputInfo->m_pCamsvImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

    ptr     = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);
    subsampleNum = CAMSV_READ_BITS(ptr,CAMSV_SPARE1,for_DCIF_subsample_number);


    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(subsample_skippair) /
         sizeof(subsample_skippair[0]))) < 0);
         i+=2) {
        skips.push_back(skipPairSt(subsample_skippair[i],
            subsample_skippair[i+1]));
    }

    LOG_INF("subsample_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    for (MUINT32 subsampleInx = 0; subsampleInx < subsampleNum; subsampleInx++) {
        if (Beyond_Compare_Subsamples(CAMSV_0, subsampleInx, pBuf,
            *SUBSUMPLE_Golden_TAL[subsampleInx],
            DMA_EN_GROUP0, CAMSV_FBC_ON_GROUP, 4, &skips) == MFALSE) {
            ret++;
        }
    }


    LOG_INF("press any key continuous\n");
    getchar();

    return ret;

}

MINT32 Pattern_BitTrue_4(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    MINT32  ret = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO ******pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0, DMA2_EN = 0, CAMSV_DMA_EN = 0, _tmp=0, _cnt=0,subsampleNum = 0;
    MUINT32 *pTable = NULL, *pMem = NULL;
    std::vector<skipPairSt> skips;
    length;
    typedef const unsigned char (*subsample_golden)[128];

    static const unsigned int subsample_skippair[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_skip.txt"
    };
    static const unsigned char subsample_0_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_0.dhex"
    };
    static const unsigned char subsample_1_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_1.dhex"
    };
    static const unsigned char subsample_2_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_2.dhex"
    };
    static const unsigned char subsample_3_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_3.dhex"
    };
    static const unsigned char subsample_4_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_4.dhex"
    };
    static const unsigned char subsample_5_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_5.dhex"
    };
    static const unsigned char subsample_6_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_6.dhex"
    };
    static const unsigned char subsample_7_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_7.dhex"
    };
    static const unsigned char subsample_8_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_8.dhex"
    };
    static const unsigned char subsample_9_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_9.dhex"
    };
    static const unsigned char subsample_10_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_10.dhex"
    };
    static const unsigned char subsample_11_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_11.dhex"
    };
    static const unsigned char subsample_12_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_12.dhex"
    };
    static const unsigned char subsample_13_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_13.dhex"
    };
    static const unsigned char subsample_14_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_14.dhex"
    };
    static const unsigned char subsample_15_golden[] = {
    #include "Emulation/1camsv_cq_pat_pass_FrameHeader_CQ/whole_chip_pattern/subsample_golden/subsample_15.dhex"
    };

    subsample_golden SUBSUMPLE_Golden_TAL[]=
    {   &subsample_0_golden,   &subsample_1_golden,   &subsample_2_golden,
        &subsample_3_golden,   &subsample_4_golden,   &subsample_5_golden,
        &subsample_6_golden,   &subsample_7_golden,   &subsample_8_golden,
        &subsample_9_golden,   &subsample_10_golden,  &subsample_11_golden,
        &subsample_12_golden,  &subsample_13_golden,  &subsample_14_golden,
        &subsample_15_golden,
        &subsample_0_golden,   &subsample_1_golden,   &subsample_2_golden,
        &subsample_3_golden,   &subsample_4_golden,   &subsample_5_golden,
        &subsample_6_golden,   &subsample_7_golden,   &subsample_8_golden,
        &subsample_9_golden,   &subsample_10_golden,  &subsample_11_golden,
        &subsample_12_golden,  &subsample_13_golden,  &subsample_14_golden,
        &subsample_15_golden,
    };




    pBuf    = (IMEM_BUF_INFO******)pInputInfo->m_pCamsvImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

    ptr     = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);
    subsampleNum = CAMSV_READ_BITS(ptr,CAMSV_SPARE1,for_DCIF_subsample_number);


    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(subsample_skippair) /
         sizeof(subsample_skippair[0]))) < 0);
         i+=2) {
        skips.push_back(skipPairSt(subsample_skippair[i],
            subsample_skippair[i+1]));
    }

    LOG_INF("subsample_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    for (MUINT32 subsampleInx = 0; subsampleInx < subsampleNum; subsampleInx++) {
        if (Beyond_Compare_Subsamples(CAMSV_0, subsampleInx, pBuf,
            *SUBSUMPLE_Golden_TAL[subsampleInx],
            DMA_EN_GROUP0, CAMSV_FBC_ON_GROUP, 4, &skips) == MFALSE) {
            ret++;
        }
    }


    LOG_INF("press any key continuous\n");
    getchar();

    return ret;

}


MINT32 Pattern_BitTrue_5(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    MINT32  ret = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO ******pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0, DMA2_EN = 0, CAMSV_DMA_EN = 0, _tmp=0, _cnt=0,subsampleNum = 0;
    MUINT32 *pTable = NULL, *pMem = NULL;
    std::vector<skipPairSt> skips;
    length;
    typedef const unsigned char (*subsample_golden)[128];

    static const unsigned int subsample_skippair[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_skip.txt"
    };
    static const unsigned char subsample_0_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_0.dhex"
    };
    static const unsigned char subsample_1_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_1.dhex"
    };
    static const unsigned char subsample_2_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_2.dhex"
    };
    static const unsigned char subsample_3_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_3.dhex"
    };
    static const unsigned char subsample_4_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_4.dhex"
    };
    static const unsigned char subsample_5_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_5.dhex"
    };
    static const unsigned char subsample_6_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_6.dhex"
    };
    static const unsigned char subsample_7_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_7.dhex"
    };
    static const unsigned char subsample_8_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_8.dhex"
    };
    static const unsigned char subsample_9_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_9.dhex"
    };
    static const unsigned char subsample_10_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_10.dhex"
    };
    static const unsigned char subsample_11_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_11.dhex"
    };
    static const unsigned char subsample_12_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_12.dhex"
    };
    static const unsigned char subsample_13_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_13.dhex"
    };
    static const unsigned char subsample_14_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_14.dhex"
    };
    static const unsigned char subsample_15_golden[] = {
        #include "Emulation/1camsv_cq_pat_pass_FrameHeader/whole_chip_pattern/subsample_golden/subsample_15.dhex"
    };

    subsample_golden SUBSUMPLE_Golden_TAL[]=
    {   &subsample_0_golden,  &subsample_1_golden,   &subsample_2_golden,
        &subsample_3_golden,  &subsample_4_golden,   &subsample_5_golden,
        &subsample_6_golden,  &subsample_7_golden,   &subsample_8_golden,
        &subsample_9_golden,  &subsample_10_golden,  &subsample_11_golden,
        &subsample_12_golden, &subsample_13_golden,  &subsample_14_golden,
        &subsample_15_golden,
        &subsample_0_golden,  &subsample_1_golden,   &subsample_2_golden,
        &subsample_3_golden,  &subsample_4_golden,   &subsample_5_golden,
        &subsample_6_golden,  &subsample_7_golden,   &subsample_8_golden,
        &subsample_9_golden,  &subsample_10_golden,  &subsample_11_golden,
        &subsample_12_golden, &subsample_13_golden,  &subsample_14_golden,
        &subsample_15_golden
    };

    pBuf    = (IMEM_BUF_INFO******)pInputInfo->m_pCamsvImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

    ptr     = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);
    subsampleNum = CAMSV_READ_BITS(ptr,CAMSV_SPARE1,for_DCIF_subsample_number);


    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(subsample_skippair) /
         sizeof(subsample_skippair[0]))) < 0);
         i+=2) {
        skips.push_back(skipPairSt(subsample_skippair[i],
            subsample_skippair[i+1]));
    }

    LOG_INF("subsample_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    for (MUINT32 subsampleInx = 0; subsampleInx < subsampleNum; subsampleInx++) {
        if (Beyond_Compare_Subsamples(CAMSV_0, subsampleInx, pBuf,
            *SUBSUMPLE_Golden_TAL[subsampleInx],
            DMA_EN_GROUP0, CAMSV_FBC_ON_GROUP, 4, &skips) == MFALSE) {
            ret++;
        }
    }


    LOG_INF("press any key continuous\n");
    getchar();

    return ret;
}

MINT32 Pattern_BitTrue_6(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    MINT32  ret = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO***** pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0, DMA2_EN = 0, CAMSV_DMA_EN = 0, _tmp=0, _cnt=0;
    MUINT32 *pTable = NULL, *pMem = NULL;
    std::vector<skipPairSt> skips;
    length;
    // golden

    // aao_r1
    static const unsigned char aao_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/aao_r1a_o_golden.dhex"
    };
    static const unsigned char aao_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/aao_r1b_o_golden.dhex"
    };

    // crzbo_r1
    static const unsigned char crzbo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/crzbo_r1a_o_golden.dhex"
    };
    static const unsigned char crzbo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/crzbo_r1b_o_golden.dhex"
    };

    // crzbo_r2
    static const unsigned char crzbo_r2a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/crzbo_r2a_o_golden.dhex"
    };
    static const unsigned char crzbo_r2b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/crzbo_r2b_o_golden.dhex"
    };

    // crzo_r1
    static const unsigned char crzo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/crzo_r1a_o_golden.dhex"
    };
    static const unsigned char crzo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/crzo_r1b_o_golden.dhex"
    };

    // crzo_r2a
    static const unsigned char crzo_r2a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/crzo_r2a_o_golden.dhex"
    };
    static const unsigned char crzo_r2b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/crzo_r2b_o_golden.dhex"
    };

    // imgo_r1a
    static const unsigned char imgo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/imgo_r1a_o_golden.dhex"
    };
    static const unsigned char imgo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/imgo_r1b_o_golden.dhex"
    };

    // lceso_r1a
    static const unsigned char lceso_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/lceso_r1a_o_golden.dhex"
    };
    static const unsigned char lceso_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/lceso_r1b_o_golden.dhex"
    };

    // ltmso_r1
    static const unsigned char ltmso_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/ltmso_r1a_o_golden.dhex"
    };
    static const unsigned char ltmso_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/ltmso_r1b_o_golden.dhex"
    };

    // pdo_r1
    static const unsigned char pdo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/pdo_r1a_o_golden.dhex"
    };
    static const unsigned char pdo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/pdo_r1b_o_golden.dhex"
    };
    static const unsigned int pdo_r1a_o_golden_skippair[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/pdo_r1a_o_golden_skippair.txt"
    };
    static const unsigned int pdo_r1b_o_golden_skippair[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/pdo_r1b_o_golden_skippair.txt"
    };

    // rrzo_r1
    static const unsigned char rrzo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/rrzo_r1a_o_golden.dhex"
    };
    static const unsigned char rrzo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/rrzo_r1b_o_golden.dhex"
    };

    // rsso_r2
    static const unsigned char rsso_r2a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/rsso_r2a_o_golden.dhex"
    };
    static const unsigned char rsso_r2b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/rsso_r2b_o_golden.dhex"
    };

    // tsfso_r1
    static const unsigned char tsfso_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/tsfso_r1a_o_golden.dhex"
    };
    static const unsigned char tsfso_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/tsfso_r1b_o_golden.dhex"
    };

    // ufgo_r1
    static const unsigned char ufgo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/ufgo_r1a_o_golden.dhex"
    };
    static const unsigned char ufgo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/ufgo_r1b_o_golden.dhex"
    };

    // yuvbo_r1
    static const unsigned char yuvbo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/yuvbo_r1a_o_golden.dhex"
    };
    static const unsigned char yuvbo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/yuvbo_r1b_o_golden.dhex"
    };

    // yuvco_r1
    static const unsigned char yuvco_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/yuvco_r1a_o_golden.dhex"
    };
    static const unsigned char yuvco_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/yuvco_r1b_o_golden.dhex"
    };

    // yuvo_r1
    static const unsigned char yuvo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/yuvo_r1a_o_golden.dhex"
    };
    static const unsigned char yuvo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/whole_chip_pattern/golden/yuvo_r1b_o_golden.dhex"
    };

    // CAMSV Golden
    static const unsigned char top0_camsv0_imgo_o[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/camsv_whole_chip_pattern/golden/top0_camsv0_imgo_o.dhex"
    };
    static const unsigned char top0_camsv1_imgo_o[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass/camsv_whole_chip_pattern/golden/top0_camsv1_imgo_o.dhex"
    };

    // CAM_A compare
    pBuf    = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);
#if 1
    if (Beyond_Compare(CAM_A, DMA_EN, AAO_R1_EN_, pBuf, aao_r1a_o_golden,
        "aao_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, AAO_R1_AAO_OFST_ADDR, AAO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, CRZBO_R1_EN_, pBuf, crzbo_r1a_o_golden,
        "crzbo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZBO_R1_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, CRZBO_R2_EN_, pBuf, crzbo_r2a_o_golden,
        "crzbo_r2a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZBO_R2_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, CRZO_R1_EN_, pBuf, crzo_r1a_o_golden,
        "crzo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZO_R1_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, CRZO_R2_EN_, pBuf, crzo_r2a_o_golden,
        "crzo_r2a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZO_R2_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, IMGO_R1_EN_, pBuf, imgo_r1a_o_golden,
        "imgo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, IMGO_R1_IMGO_OFST_ADDR, IMGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, LCESO_R1_EN_, pBuf, lceso_r1a_o_golden,
        "lceso_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, LCESO_R1_LCESO_OFST_ADDR, LCESO_OFST_ADDR),
        &skips ) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, LTMSO_R1_EN_, pBuf, ltmso_r1a_o_golden,
        "ltmso_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, LTMSO_R1_LTMSO_OFST_ADDR, LTMSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    #endif
    skips.clear();
    for (MUINT32 i = 0;
        i < (sizeof(pdo_r1a_o_golden_skippair) /
             sizeof(pdo_r1a_o_golden_skippair[0]));
        i+=2) {
        skips.push_back(skipPairSt(pdo_r1a_o_golden_skippair[i],
            pdo_r1a_o_golden_skippair[i+1]));
    }
    LOG_INF("pdo_r1a_o_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    if (Beyond_Compare(CAM_A, DMA_EN, PDO_R1_EN_, pBuf, pdo_r1a_o_golden,
        "pdo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, PDO_R1_PDO_OFST_ADDR, PDO_OFST_ADDR),
        &skips) == MFALSE) {
        ret++;
    }
    skips.clear();
#if 1
    if (Beyond_Compare(CAM_A, DMA_EN, RRZO_R1_EN_, pBuf, rrzo_r1a_o_golden,
        "rrzo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, RRZO_R1_RRZO_OFST_ADDR, RRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, RSSO_R2_EN_, pBuf, rsso_r2a_o_golden,
        "rsso_r2a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, RSSO_R2_RSSO_OFST_ADDR, RSSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, UFGO_R1_EN_, pBuf, ufgo_r1a_o_golden,
        "ufgo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFGO_R1_UFGO_OFST_ADDR, UFGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, YUVBO_R1_EN_, pBuf, yuvbo_r1a_o_golden,
        "yuvbo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, YUVBO_R1_YUVBO_OFST_ADDR, YUVBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, YUVCO_R1_EN_, pBuf, yuvco_r1a_o_golden,
        "yuvco_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, YUVCO_R1_YUVCO_OFST_ADDR, YUVCO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, YUVO_R1_EN_, pBuf, yuvo_r1a_o_golden,
       "yuvo_r1a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, YUVO_R1_YUVO_OFST_ADDR, YUVO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, TSFSO_R1_EN_, pBuf, tsfso_r1a_o_golden,
        "tsfso_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, TSFSO_R1_TSFSO_OFST_ADDR, TSFSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    // CAM_B compare
    pBuf    = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

    if (Beyond_Compare(CAM_B, DMA_EN, AAO_R1_EN_, pBuf, aao_r1b_o_golden,
        "aao_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, AAO_R1_AAO_OFST_ADDR, AAO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, CRZBO_R1_EN_, pBuf, crzbo_r1b_o_golden,
        "crzbo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZBO_R1_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA2_EN, CRZBO_R2_EN_, pBuf, crzbo_r2b_o_golden,
        "crzbo_r2b", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZBO_R2_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, CRZO_R1_EN_, pBuf, crzo_r1b_o_golden,
        "crzo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZO_R1_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA2_EN, CRZO_R2_EN_, pBuf, crzo_r2b_o_golden,
        "crzo_r2b", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZO_R2_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, IMGO_R1_EN_, pBuf, imgo_r1b_o_golden,
        "imgo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, IMGO_R1_IMGO_OFST_ADDR, IMGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, LCESO_R1_EN_, pBuf, lceso_r1b_o_golden,
        "lceso_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, LCESO_R1_LCESO_OFST_ADDR, LCESO_OFST_ADDR),
        &skips ) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, LTMSO_R1_EN_, pBuf, ltmso_r1b_o_golden,
        "ltmso_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, LTMSO_R1_LTMSO_OFST_ADDR, LTMSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    skips.clear();
    for (MUINT32 i = 0;
        i < (sizeof(pdo_r1b_o_golden_skippair) /
             sizeof(pdo_r1b_o_golden_skippair[0]));
        i+=2) {
        skips.push_back(skipPairSt(pdo_r1b_o_golden_skippair[i],
            pdo_r1b_o_golden_skippair[i+1]));
    }

    LOG_INFL("pdo_r1b_o_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    if (Beyond_Compare(CAM_B, DMA_EN, PDO_R1_EN_, pBuf, pdo_r1b_o_golden,
        "pdo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, PDO_R1_PDO_OFST_ADDR, PDO_OFST_ADDR),
        &skips) == MFALSE) {
        ret++;
    }
    skips.clear();


    if (Beyond_Compare(CAM_B, DMA_EN, RRZO_R1_EN_, pBuf, rrzo_r1b_o_golden,
        "rrzo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, RRZO_R1_RRZO_OFST_ADDR, RRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA2_EN, RSSO_R2_EN_, pBuf, rsso_r2b_o_golden,
        "rsso_r2b", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, RSSO_R2_RSSO_OFST_ADDR, RSSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, UFGO_R1_EN_, pBuf, ufgo_r1b_o_golden,
        "ufgo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFGO_R1_UFGO_OFST_ADDR, UFGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, YUVBO_R1_EN_, pBuf, yuvbo_r1b_o_golden,
        "yuvbo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, YUVBO_R1_YUVBO_OFST_ADDR, YUVBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_B, DMA_EN, YUVCO_R1_EN_, pBuf, yuvco_r1b_o_golden,
        "yuvco_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, YUVCO_R1_YUVCO_OFST_ADDR, YUVCO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA2_EN, YUVO_R1_EN_, pBuf, yuvo_r1b_o_golden,
        "yuvo_r1b", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, YUVO_R1_YUVO_OFST_ADDR, YUVO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, TSFSO_R1_EN_, pBuf, tsfso_r1b_o_golden,
        "tsfso_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, TSFSO_R1_TSFSO_OFST_ADDR, TSFSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    // CAMSV0 compare
    ptr          = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);

    if (Beyond_Compare(CAMSV_0, CAMSV_DMA_EN, CAMSV_CTL_IMGO_EN_, pBuf,
        top0_camsv0_imgo_o, "CAMSV0_IMGO", DMA_EN_GROUP0,
        CAMSV_READ_REG(ptr, CAMSV_IMGO_OFST_ADDR, OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    // CAMSV1 compare
    ptr          = ((IspDrvCamsv**)_pCamsv)[CAMSV_1-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);

    if (Beyond_Compare(CAMSV_1, CAMSV_DMA_EN, CAMSV_CTL_IMGO_EN_, pBuf,
        top0_camsv1_imgo_o, "CAMSV1_IMGO", DMA_EN_GROUP1,
        CAMSV_READ_REG(ptr, CAMSV_IMGO_OFST_ADDR, OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
#endif
    LOG_INF("press any key continuous\n");
    getchar();
    return ret;

}

MINT32 Pattern_BitTrue_7(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32  ret = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO***** pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0, DMA2_EN = 0, CAMSV_DMA_EN = 0, _tmp=0, _cnt=0;
    MUINT32 *pTable = NULL, *pMem = NULL;
    std::vector<skipPairSt> skips;
    length;(void)_uni;
    // CAM golden

    //aao_r1
    static const unsigned char aao_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/aao_r1a_o_golden.dhex"
    };

    //afo_r1
    static const unsigned char afo_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/afo_r1a_o_golden.dhex"
    };

    //flko_r1
    static const unsigned char flko_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/flko_r1a_o_golden.dhex"
    };

    //imgo_r1
    static const unsigned char imgo_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/imgo_r1a_o_golden.dhex"
    };
    static const unsigned char imgo_r1b_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/imgo_r1b_o_golden.dhex"
    };
    static const unsigned int imgo_r1b_o_golden_skippair[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/imgo_r1b_o_golden_skippair.txt"
    };

    //lceso_r1
    static const unsigned char lceso_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/lceso_r1a_o_golden.dhex"
    };

    //ltmso_r1
    static const unsigned char ltmso_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/ltmso_r1a_o_golden.dhex"
    };

    //rrzo_r1
    static const unsigned char rrzo_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/rrzo_r1a_o_golden.dhex"
    };
    static const unsigned char rrzo_r1b_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/rrzo_r1b_o_golden.dhex"
    };
    static const unsigned int rrzo_r1b_o_golden_skippair_Modify[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/rrzo_r1b_o_golden_skippair_Modify.txt"
    };

    //tsfso_r1
    static const unsigned char tsfso_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/tsfso_r1a_o_golden.dhex"
    };

    //ufeo_r1
    static const unsigned char ufeo_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/ufeo_r1a_o_golden.dhex"
    };

    static const unsigned char ufeo_r1b_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/ufeo_r1b_o_golden.dhex"
    };
    static const unsigned int ufeo_r1b_o_golden_skippair__Modify[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/ufeo_r1b_o_golden_skippair_Modify.txt"
    };

    //yuvo_r1
    static const unsigned char yuvo_r1a_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/yuvo_r1a_o_golden.dhex"
    };
    static const unsigned char yuvo_r1b_o_golden[] = {
    #include "Emulation/TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS/whole_chip_pattern/golden/yuvo_r1b_o_golden.dhex"
    };

    // CAM_A compare
    pBuf    = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

    if (Beyond_Compare(CAM_A, DMA_EN, AAO_R1_EN_, pBuf, aao_r1a_o_golden,
        "aao_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, AAO_R1_AAO_OFST_ADDR, AAO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, AFO_R1_EN_, pBuf, afo_r1a_o_golden,
        "afo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, AFO_R1_AFO_OFST_ADDR, AFO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, FLKO_R1_EN_, pBuf, flko_r1a_o_golden,
        "flko_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, FLKO_R1_FLKO_OFST_ADDR, FLKO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_A, DMA_EN, IMGO_R1_EN_, pBuf, imgo_r1a_o_golden,
        "imgo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, IMGO_R1_IMGO_OFST_ADDR, IMGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, LTMSO_R1_EN_, pBuf, ltmso_r1a_o_golden,
        "ltmso_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, LTMSO_R1_LTMSO_OFST_ADDR, LTMSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_A, DMA_EN, RRZO_R1_EN_, pBuf, rrzo_r1a_o_golden,
        "rrzo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, RRZO_R1_RRZO_OFST_ADDR, RRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, LCESO_R1_EN_, pBuf, lceso_r1a_o_golden,
        "lceso_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, LCESO_R1_LCESO_OFST_ADDR, LCESO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_A, DMA_EN, TSFSO_R1_EN_, pBuf, tsfso_r1a_o_golden,
        "tsfso_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, TSFSO_R1_TSFSO_OFST_ADDR, TSFSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_A, DMA_EN, UFEO_R1_EN_, pBuf, ufeo_r1a_o_golden,
        "ufeo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFEO_R1_UFEO_OFST_ADDR, UFEO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, YUVO_R1_EN_, pBuf, yuvo_r1a_o_golden,
        "yuvo_r1a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, YUVO_R1_YUVO_OFST_ADDR, YUVO_OFST_ADDR)) == MFALSE) {
        ret++;
    }


    // CAM_B compare
    pBuf    = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

    skips.clear();
    for (MUINT32 i = 0;
       i < (sizeof(imgo_r1b_o_golden_skippair) /
            sizeof(imgo_r1b_o_golden_skippair[0]));
       i+=2) {
       skips.push_back(skipPairSt(imgo_r1b_o_golden_skippair[i],
           imgo_r1b_o_golden_skippair[i+1]));
    }
    LOG_INF("imgo_r1b_o_golden_skippair total dontcare pairs: %ld\n",
           skips.size());

    if (Beyond_Compare(CAM_B, DMA_EN, IMGO_R1_EN_, pBuf, imgo_r1b_o_golden,
        "imgo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, IMGO_R1_IMGO_OFST_ADDR, IMGO_OFST_ADDR),
        &skips) == MFALSE) {
        ret++;
    }

    skips.clear();
    for (MUINT32 i = 0;
       i < (sizeof(rrzo_r1b_o_golden_skippair_Modify) /
            sizeof(rrzo_r1b_o_golden_skippair_Modify[0]));
       i+=2) {
       skips.push_back(skipPairSt(rrzo_r1b_o_golden_skippair_Modify[i],
           rrzo_r1b_o_golden_skippair_Modify[i+1]));
    }
    LOG_INF("rrzo_r1b_o_golden_skippair_Modify total dontcare pairs: %lu\n",
           skips.size());
    if (Beyond_Compare(CAM_B, DMA_EN, RRZO_R1_EN_, pBuf, rrzo_r1b_o_golden,
        "rrzo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, RRZO_R1_RRZO_OFST_ADDR, RRZO_OFST_ADDR),
        &skips) == MFALSE) {
        ret++;
    }

    skips.clear();
    for (MUINT32 i = 0;
       i < (sizeof(ufeo_r1b_o_golden_skippair__Modify) /
            sizeof(ufeo_r1b_o_golden_skippair__Modify[0]));
       i+=2) {
       skips.push_back(skipPairSt(ufeo_r1b_o_golden_skippair__Modify[i],
           ufeo_r1b_o_golden_skippair__Modify[i+1]));
    }
    LOG_INF("ufeo_r1b_o_golden_skippair__Modify total dontcare pairs: %lu\n",
           skips.size());
    if (Beyond_Compare(CAM_B, DMA_EN, UFEO_R1_EN_, pBuf, ufeo_r1b_o_golden,
        "ufeo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFEO_R1_UFEO_OFST_ADDR, UFEO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    skips.clear();

    if (Beyond_Compare(CAM_B, DMA2_EN, YUVO_R1_EN_, pBuf, yuvo_r1b_o_golden,
        "yuvo_r1b", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, YUVO_R1_YUVO_OFST_ADDR, YUVO_OFST_ADDR)) == MFALSE) {
        ret++;
    }


    LOG_INF("press any key continuous\n");
    getchar();
    return ret;
}

MINT32 Pattern_BitTrue_8(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    MINT32  ret = 0;
    IspDrv* ptr = NULL;
    IspDrv* ptrCam[MAX_ISP_HW_MODULE] = {NULL};
    IMEM_BUF_INFO ******pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0, DMA2_EN = 0, CAMSV_DMA_EN = 0, _tmp=0, _cnt=0,subsampleNum = 0;
    MUINT32 *pTable = NULL, *pMem = NULL;
    std::vector<skipPairSt> skips;
    length;
    typedef const unsigned char (*subsample_golden)[128];

    static const unsigned int NormalPage0_subsample_skippair[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage0/subsample_skip.txt"
    };
    static const unsigned char NormalPage0_subsample_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage0/NormalPage0_subsample_0.dhex"
    };
    static const unsigned char NormalPage0_subsample_1[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage0/NormalPage0_subsample_1.dhex"
    };
    static const unsigned char NormalPage0_subsample_2[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage0/NormalPage0_subsample_2.dhex"
    };
    static const unsigned char NormalPage0_subsample_3[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage0/NormalPage0_subsample_3.dhex"
    };
    static const unsigned char NormalPage0_subsample_4[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage0/NormalPage0_subsample_4.dhex"
    };
    static const unsigned char NormalPage0_subsample_5[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage0/NormalPage0_subsample_5.dhex"
    };
    static const unsigned char NormalPage0_subsample_6[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage0/NormalPage0_subsample_6.dhex"
    };
    static const unsigned char NormalPage0_subsample_7[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage0/NormalPage0_subsample_7.dhex"
    };


    subsample_golden NormalPage0_SUBSUMPLE_Golden_TAL[]=
    {   &NormalPage0_subsample_0,  &NormalPage0_subsample_1,
        &NormalPage0_subsample_2,  &NormalPage0_subsample_3,
        &NormalPage0_subsample_4,  &NormalPage0_subsample_5,
        &NormalPage0_subsample_6,  &NormalPage0_subsample_7,
        &NormalPage0_subsample_0,  &NormalPage0_subsample_1,
        &NormalPage0_subsample_2,  &NormalPage0_subsample_3,
        &NormalPage0_subsample_4,  &NormalPage0_subsample_5,
        &NormalPage0_subsample_6,  &NormalPage0_subsample_7,
        &NormalPage0_subsample_0,  &NormalPage0_subsample_1,
        &NormalPage0_subsample_2,  &NormalPage0_subsample_3,
        &NormalPage0_subsample_4,  &NormalPage0_subsample_5,
        &NormalPage0_subsample_6,  &NormalPage0_subsample_7,
        &NormalPage0_subsample_0,  &NormalPage0_subsample_1,
        &NormalPage0_subsample_2,  &NormalPage0_subsample_3,
        &NormalPage0_subsample_4,  &NormalPage0_subsample_5,
        &NormalPage0_subsample_6,  &NormalPage0_subsample_7,

    };

    static const unsigned int NormalPage1_subsample_skippair[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage1/subsample_skip.txt"
    };
    static const unsigned char NormalPage1_subsample_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage1/NormalPage1_subsample_0.dhex"
    };
    static const unsigned char NormalPage1_subsample_1[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage1/NormalPage1_subsample_1.dhex"
    };
    static const unsigned char NormalPage1_subsample_2[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage1/NormalPage1_subsample_2.dhex"
    };
    static const unsigned char NormalPage1_subsample_3[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage1/NormalPage1_subsample_3.dhex"
    };
    static const unsigned char NormalPage1_subsample_4[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage1/NormalPage1_subsample_4.dhex"
    };
    static const unsigned char NormalPage1_subsample_5[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage1/NormalPage1_subsample_5.dhex"
    };
    static const unsigned char NormalPage1_subsample_6[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage1/NormalPage1_subsample_6.dhex"
    };
    static const unsigned char NormalPage1_subsample_7[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/NormalPage1/NormalPage1_subsample_7.dhex"
    };


    subsample_golden NormalPage1_SUBSUMPLE_Golden_TAL[]=
    {   &NormalPage1_subsample_0,  &NormalPage1_subsample_1,
        &NormalPage1_subsample_2,  &NormalPage1_subsample_3,
        &NormalPage1_subsample_4,  &NormalPage1_subsample_5,
        &NormalPage1_subsample_6,  &NormalPage1_subsample_7,
        &NormalPage1_subsample_0,  &NormalPage1_subsample_1,
        &NormalPage1_subsample_2,  &NormalPage1_subsample_3,
        &NormalPage1_subsample_4,  &NormalPage1_subsample_5,
        &NormalPage1_subsample_6,  &NormalPage1_subsample_7,
        &NormalPage1_subsample_0,  &NormalPage1_subsample_1,
        &NormalPage1_subsample_2,  &NormalPage1_subsample_3,
        &NormalPage1_subsample_4,  &NormalPage1_subsample_5,
        &NormalPage1_subsample_6,  &NormalPage1_subsample_7,
        &NormalPage1_subsample_0,  &NormalPage1_subsample_1,
        &NormalPage1_subsample_2,  &NormalPage1_subsample_3,
        &NormalPage1_subsample_4,  &NormalPage1_subsample_5,
        &NormalPage1_subsample_6,  &NormalPage1_subsample_7,

    };

    static const unsigned int DummyPage0_subsample_skip[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage0/DummyPage0_subsample_skip.txt"
    };
    static const unsigned char DummyPage0_subsample_0[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage0/DummyPage0_subsample_0.dhex"
    };
    static const unsigned char DummyPage0_subsample_1[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage0/DummyPage0_subsample_1.dhex"
    };
    static const unsigned char DummyPage0_subsample_2[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage0/DummyPage0_subsample_2.dhex"
    };
    static const unsigned char DummyPage0_subsample_3[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage0/DummyPage0_subsample_3.dhex"
    };
    static const unsigned char DummyPage0_subsample_4[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage0/DummyPage0_subsample_4.dhex"
    };
    static const unsigned char DummyPage0_subsample_5[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage0/DummyPage0_subsample_5.dhex"
    };
    static const unsigned char DummyPage0_subsample_6[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage0/DummyPage0_subsample_6.dhex"
    };
    static const unsigned char DummyPage0_subsample_7[] = {
        #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage0/DummyPage0_subsample_7.dhex"
    };


    subsample_golden DummyPage0_SUBSUMPLE_Golden_TAL[]=
    {   &DummyPage0_subsample_0,  &DummyPage0_subsample_1,
        &DummyPage0_subsample_2,  &DummyPage0_subsample_3,
        &DummyPage0_subsample_4,  &DummyPage0_subsample_5,
        &DummyPage0_subsample_6,  &DummyPage0_subsample_7,
        &DummyPage0_subsample_0,  &DummyPage0_subsample_1,
        &DummyPage0_subsample_2,  &DummyPage0_subsample_3,
        &DummyPage0_subsample_4,  &DummyPage0_subsample_5,
        &DummyPage0_subsample_6,  &DummyPage0_subsample_7,
        &DummyPage0_subsample_0,  &DummyPage0_subsample_1,
        &DummyPage0_subsample_2,  &DummyPage0_subsample_3,
        &DummyPage0_subsample_4,  &DummyPage0_subsample_5,
        &DummyPage0_subsample_6,  &DummyPage0_subsample_7,
        &DummyPage0_subsample_0,  &DummyPage0_subsample_1,
        &DummyPage0_subsample_2,  &DummyPage0_subsample_3,
        &DummyPage0_subsample_4,  &DummyPage0_subsample_5,
        &DummyPage0_subsample_6,  &DummyPage0_subsample_7,

    };


    static const unsigned int DummyPage1_subsample_skip[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage1/DummyPage1_subsample_skip.txt"
    };
    static const unsigned char DummyPage1_subsample_0[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage1/DummyPage1_subsample_0.dhex"
    };
    static const unsigned char DummyPage1_subsample_1[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage1/DummyPage1_subsample_1.dhex"
    };
    static const unsigned char DummyPage1_subsample_2[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage1/DummyPage1_subsample_2.dhex"
    };
    static const unsigned char DummyPage1_subsample_3[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage1/DummyPage1_subsample_3.dhex"
    };
    static const unsigned char DummyPage1_subsample_4[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage1/DummyPage1_subsample_4.dhex"
    };
    static const unsigned char DummyPage1_subsample_5[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage1/DummyPage1_subsample_5.dhex"
    };
    static const unsigned char DummyPage1_subsample_6[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage1/DummyPage1_subsample_6.dhex"
    };
    static const unsigned char DummyPage1_subsample_7[] = {
    #include "Emulation/2camsv_cq_ufo_pat_0521_pass_ECO/whole_chip_pattern/subsample_golden/DummyPage1/DummyPage1_subsample_7.dhex"
    };


    subsample_golden DummyPage1_SUBSUMPLE_Golden_TAL[]=
    {   &DummyPage1_subsample_0,  &DummyPage1_subsample_1,
        &DummyPage1_subsample_2,  &DummyPage1_subsample_3,
        &DummyPage1_subsample_4,  &DummyPage1_subsample_5,
        &DummyPage1_subsample_6,  &DummyPage1_subsample_7,
        &DummyPage1_subsample_0,  &DummyPage1_subsample_1,
        &DummyPage1_subsample_2,  &DummyPage1_subsample_3,
        &DummyPage1_subsample_4,  &DummyPage1_subsample_5,
        &DummyPage1_subsample_6,  &DummyPage1_subsample_7,
        &DummyPage1_subsample_0,  &DummyPage1_subsample_1,
        &DummyPage1_subsample_2,  &DummyPage1_subsample_3,
        &DummyPage1_subsample_4,  &DummyPage1_subsample_5,
        &DummyPage1_subsample_6,  &DummyPage1_subsample_7,
        &DummyPage1_subsample_0,  &DummyPage1_subsample_1,
        &DummyPage1_subsample_2,  &DummyPage1_subsample_3,
        &DummyPage1_subsample_4,  &DummyPage1_subsample_5,
        &DummyPage1_subsample_6,  &DummyPage1_subsample_7,

    };


    for(MUINT32 hwModule = CAM_A; hwModule <= CAM_B; hwModule++) {
        LOG_INFL("Before Setting  cam_%c CQ0 Continue Mode \n", 'a'+hwModule);
        getchar();
        if (CAM_READ_BITS(ptrCam[hwModule], CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
                CAMCQ_CQ_THR0_MODE) != 0 /*continue Mode*/) {
            LOG_INFL("cam_%c CQ0 continue Mode \n", 'a'+hwModule);
            CAM_WRITE_BITS(ptrCam[hwModule], CAMCQ_R1_CAMCQ_CQ_THR0_CTL,
                     CAMCQ_CQ_THR0_MODE, 0);
        }
    }

    pBuf    = (IMEM_BUF_INFO******)pInputInfo->m_pCamsvImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam[CAM_A] = ptr;
    CAM_WRITE_REG(ptr, CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,
                  gpMultiCamsvCQDescriptorBuf[CAMSV_0][CQ_NORMAL_PAGE1]
                  ->phyAddr);

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

    LOG_INF("Change CAM_A CQ0 BaseAddress to CQDescriptor Normal Page1\n");
    getchar();

    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    ptrCam[CAM_B] = ptr;
    CAM_WRITE_REG(ptr, CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR,
                  gpMultiCamsvCQDescriptorBuf[CAMSV_1][CQ_NORMAL_PAGE1]
                  ->phyAddr);

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);

    LOG_INF("Change CAM_B CQ0 BaseAddress to CQDescriptor Normal Page1\n");
    getchar();

    ptr     = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);
    subsampleNum = CAMSV_READ_BITS(ptr,CAMSV_SPARE1,for_DCIF_subsample_number);
    LOG_INFL("BitTrue: subsampleNum=%d\n",subsampleNum);


    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(NormalPage0_subsample_skippair) /
         sizeof(NormalPage0_subsample_skippair[0]))) < 0);
         i+=2) {
        skips.push_back(skipPairSt(NormalPage0_subsample_skippair[i],
            NormalPage0_subsample_skippair[i+1]));
    }

    LOG_INF("NormalPage0_subsample_skippair total dontcare pairs:%02ld\n",
            skips.size());

    for (MUINT32 subsampleInx = 0; subsampleInx < subsampleNum; subsampleInx++) {
        if (Beyond_Compare_Subsamples_MultiCQ(CAMSV_0, subsampleInx, pBuf,
            *NormalPage0_SUBSUMPLE_Golden_TAL[subsampleInx],
            CQ_NORMAL_PAGE_TYPE, CQ_PAGE0, 4, &skips, CQ_IMGO_ENQ_HADDR_INX) == MFALSE) {
            ret++;
        }
    }

    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(NormalPage1_subsample_skippair) /
         sizeof(NormalPage1_subsample_skippair[0]))) < 0);
         i+=2) {
        skips.push_back(skipPairSt(NormalPage1_subsample_skippair[i],
            NormalPage1_subsample_skippair[i+1]));
    }

    LOG_INF("NormalPage1_subsample_skippair total dontcare pairs:%02ld\n",
            skips.size());

    for (MUINT32 subsampleInx = 0; subsampleInx < subsampleNum; subsampleInx++) {
        if (Beyond_Compare_Subsamples_MultiCQ(CAMSV_0, subsampleInx, pBuf,
            *NormalPage1_SUBSUMPLE_Golden_TAL[subsampleInx],
            CQ_NORMAL_PAGE_TYPE, CQ_PAGE1, 4, &skips, CQ_IMGO_ENQ_HADDR_INX) == MFALSE) {
            ret++;
        }
    }

    ptr     = ((IspDrvCamsv**)_pCamsv)[CAMSV_1-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);
    subsampleNum = CAMSV_READ_BITS(ptr,CAMSV_SPARE1,for_DCIF_subsample_number);


    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(NormalPage0_subsample_skippair) /
         sizeof(NormalPage0_subsample_skippair[0]))) < 0);
         i+=2) {
        skips.push_back(skipPairSt(NormalPage0_subsample_skippair[i],
            NormalPage0_subsample_skippair[i+1]));
    }

    LOG_INF("NormalPage0_subsample_skippair total dontcare pairs:%02ld\n",
            skips.size());

    for (MUINT32 subsampleInx = 0; subsampleInx < subsampleNum; subsampleInx++) {
        if (Beyond_Compare_Subsamples_MultiCQ(CAMSV_1, subsampleInx, pBuf,
            *NormalPage0_SUBSUMPLE_Golden_TAL[subsampleInx],
            DMA_EN_GROUP0, CQ_PAGE0, 4, &skips, CQ_IMGO_ENQ_HADDR_INX) == MFALSE) {
            ret++;
        }
    }

    skips.clear();
    for (MUINT32 i = 0;
         ((i - (sizeof(NormalPage1_subsample_skippair) /
         sizeof(NormalPage1_subsample_skippair[0]))) < 0);
         i+=2) {
        skips.push_back(skipPairSt(NormalPage1_subsample_skippair[i],
            NormalPage1_subsample_skippair[i+1]));
    }

    LOG_INF("NormalPage1_subsample_skippair total dontcare pairs:%02ld\n",
            skips.size());

    for (MUINT32 subsampleInx = 0; subsampleInx < subsampleNum; subsampleInx++) {
        if (Beyond_Compare_Subsamples_MultiCQ(CAMSV_1, subsampleInx, pBuf,
            *NormalPage1_SUBSUMPLE_Golden_TAL[subsampleInx],
            DMA_EN_GROUP0, CQ_PAGE1, 4, &skips, CQ_IMGO_ENQ_HADDR_INX) == MFALSE) {
            ret++;
        }
    }

    LOG_INF("press any key continuous\n");
    getchar();

    return ret;
}

MINT32 Pattern_BitTrue_9(MUINT32* _ptr,MUINTPTR length,MUINT32* _pCamsv,MUINTPTR inputInfo)
{
    MINT32  ret = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO***** pBuf   = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0, DMA2_EN = 0, CAMSV_DMA_EN = 0, _tmp=0, _cnt=0;
    MUINT32 *pTable = NULL, *pMem = NULL;
    std::vector<skipPairSt> skips;
    length;
    // golden

    // aao_r1
    static const unsigned char aao_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/aao_r1a_o_golden.dhex"
    };

    // afo_r1
    static const unsigned char afo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/afo_r1a_o_golden.dhex"
    };

    // crzbo_r1
    static const unsigned char crzbo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/crzbo_r1a_o_golden.dhex"
    };
    static const unsigned char crzbo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/crzbo_r1b_o_golden.dhex"
    };

    // crzbo_r2
    static const unsigned char crzbo_r2a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/crzbo_r2a_o_golden.dhex"
    };
    static const unsigned char crzbo_r2b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/crzbo_r2b_o_golden.dhex"
    };

    // crzo_r1
    static const unsigned char crzo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/crzo_r1a_o_golden.dhex"
    };
    static const unsigned char crzo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/crzo_r1b_o_golden.dhex"
    };

    // crzo_r2a
    static const unsigned char crzo_r2a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/crzo_r2a_o_golden.dhex"
    };
    static const unsigned char crzo_r2b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/crzo_r2b_o_golden.dhex"
    };

    // imgo_r1a
    static const unsigned char imgo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/imgo_r1a_o_golden.dhex"
    };
    static const unsigned char imgo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/imgo_r1b_o_golden.dhex"
    };

    // lceso_r1a
    static const unsigned char lceso_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/lceso_r1a_o_golden.dhex"
    };


    // ltmso_r1
    static const unsigned char ltmso_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/ltmso_r1a_o_golden.dhex"
    };

    // pdo_r1
    static const unsigned char pdo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/pdo_r1a_o_golden.dhex"
    };

    static const unsigned int pdo_r1a_o_golden_skippair[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/pdo_r1a_o_golden_skippair.txt"
    };

    // rrzo_r1
    static const unsigned char rrzo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/rrzo_r1a_o_golden.dhex"
    };
    static const unsigned char rrzo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/rrzo_r1b_o_golden.dhex"
    };

    // rsso_r2
    static const unsigned char rsso_r2a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/rsso_r2a_o_golden.dhex"
    };
    static const unsigned char rsso_r2b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/rsso_r2b_o_golden.dhex"
    };

    // tsfso_r1
    static const unsigned char tsfso_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/tsfso_r1a_o_golden.dhex"
    };


    // ufgo_r1
    static const unsigned char ufgo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/ufgo_r1a_o_golden.dhex"
    };
    static const unsigned char ufgo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/ufgo_r1b_o_golden.dhex"
    };

    // yuvbo_r1
    static const unsigned char yuvbo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/yuvbo_r1a_o_golden.dhex"
    };
    static const unsigned char yuvbo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/yuvbo_r1b_o_golden.dhex"
    };

    // yuvco_r1
    static const unsigned char yuvco_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/yuvco_r1a_o_golden.dhex"
    };
    static const unsigned char yuvco_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/yuvco_r1b_o_golden.dhex"
    };

    // yuvo_r1
    static const unsigned char yuvo_r1a_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/yuvo_r1a_o_golden.dhex"
    };
    static const unsigned char yuvo_r1b_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/whole_chip_pattern/golden/yuvo_r1b_o_golden.dhex"
    };

    // CAMSV Golden
    static const unsigned char top0_camsv0_imgo_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/golden/top0_camsv0_imgo_o_golden.dhex"
    };
    static const unsigned int top0_camsv0_imgo_o_golden_skippair[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/golden/top0_camsv0_imgo_o_golden_skippair.txt"
    };
    static const unsigned char top0_camsv0_ufeo_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/golden/top0_camsv0_ufeo_o_golden.dhex"
    };
    static const unsigned int top0_camsv0_ufeo_o_golden_skippair[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/golden/top0_camsv0_ufeo_o_golden_skippair.txt"
    };
    static const unsigned char top0_camsv1_imgo_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/golden/top0_camsv1_imgo_o_golden.dhex"
    };
    static const unsigned int top0_camsv1_imgo_o_golden_skippair[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/golden/top0_camsv1_imgo_o_golden_skippair.txt"
    };
    static const unsigned char top0_camsv1_ufeo_o_golden[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/golden/top0_camsv1_ufeo_o_golden.dhex"
    };
    static const unsigned int top0_camsv1_ufeo_o_golden_skippair2[] = {
    #include "Emulation/2camsv_cq_1raw_ufo_pat_pass/camsv_whole_chip_pattern/golden/top0_camsv1_ufeo_o_golden_skippair2.txt"
    };
    // CAM_A compare
    pBuf    = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);
#if 1
    if (Beyond_Compare(CAM_A, DMA_EN, AAO_R1_EN_, pBuf, aao_r1a_o_golden,
        "aao_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, AAO_R1_AAO_OFST_ADDR, AAO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_A, DMA_EN, AFO_R1_EN_, pBuf, afo_r1a_o_golden,
        "afo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, AFO_R1_AFO_OFST_ADDR, AFO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_A, DMA_EN, CRZBO_R1_EN_, pBuf, crzbo_r1a_o_golden,
        "crzbo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZBO_R1_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, CRZBO_R2_EN_, pBuf, crzbo_r2a_o_golden,
        "crzbo_r2a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZBO_R2_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, CRZO_R1_EN_, pBuf, crzo_r1a_o_golden,
        "crzo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZO_R1_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, CRZO_R2_EN_, pBuf, crzo_r2a_o_golden,
        "crzo_r2a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZO_R2_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, IMGO_R1_EN_, pBuf, imgo_r1a_o_golden,
        "imgo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, IMGO_R1_IMGO_OFST_ADDR, IMGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, LCESO_R1_EN_, pBuf, lceso_r1a_o_golden,
        "lceso_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, LCESO_R1_LCESO_OFST_ADDR, LCESO_OFST_ADDR),
        &skips ) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, LTMSO_R1_EN_, pBuf, ltmso_r1a_o_golden,
        "ltmso_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, LTMSO_R1_LTMSO_OFST_ADDR, LTMSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    #endif
    skips.clear();
    for (MUINT32 i = 0;
        i < (sizeof(pdo_r1a_o_golden_skippair) /
             sizeof(pdo_r1a_o_golden_skippair[0]));
        i+=2) {
        skips.push_back(skipPairSt(pdo_r1a_o_golden_skippair[i],
            pdo_r1a_o_golden_skippair[i+1]));
    }
    LOG_INF("pdo_r1a_o_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    if (Beyond_Compare(CAM_A, DMA_EN, PDO_R1_EN_, pBuf, pdo_r1a_o_golden,
        "pdo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, PDO_R1_PDO_OFST_ADDR, PDO_OFST_ADDR),
        &skips) == MFALSE) {
        ret++;
    }
    skips.clear();
#if 1
    if (Beyond_Compare(CAM_A, DMA_EN, RRZO_R1_EN_, pBuf, rrzo_r1a_o_golden,
        "rrzo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, RRZO_R1_RRZO_OFST_ADDR, RRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, RSSO_R2_EN_, pBuf, rsso_r2a_o_golden,
        "rsso_r2a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, RSSO_R2_RSSO_OFST_ADDR, RSSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, UFGO_R1_EN_, pBuf, ufgo_r1a_o_golden,
        "ufgo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFGO_R1_UFGO_OFST_ADDR, UFGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, YUVBO_R1_EN_, pBuf, yuvbo_r1a_o_golden,
        "yuvbo_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, YUVBO_R1_YUVBO_OFST_ADDR, YUVBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, YUVCO_R1_EN_, pBuf, yuvco_r1a_o_golden,
        "yuvco_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, YUVCO_R1_YUVCO_OFST_ADDR, YUVCO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA2_EN, YUVO_R1_EN_, pBuf, yuvo_r1a_o_golden,
       "yuvo_r1a", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, YUVO_R1_YUVO_OFST_ADDR, YUVO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A, DMA_EN, TSFSO_R1_EN_, pBuf, tsfso_r1a_o_golden,
        "tsfso_r1a", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, TSFSO_R1_TSFSO_OFST_ADDR, TSFSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    // CAM_B compare
    pBuf    = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    ptr     = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();

    DMA_EN  = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA_EN);
    DMA2_EN = CAM_READ_REG(ptr, CAMCTL_R1_CAMCTL_DMA2_EN);


    if (Beyond_Compare(CAM_B, DMA_EN, CRZBO_R1_EN_, pBuf, crzbo_r1b_o_golden,
        "crzbo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZBO_R1_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA2_EN, CRZBO_R2_EN_, pBuf, crzbo_r2b_o_golden,
        "crzbo_r2b", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZBO_R2_CRZBO_OFST_ADDR, CRZBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, CRZO_R1_EN_, pBuf, crzo_r1b_o_golden,
        "crzo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, CRZO_R1_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA2_EN, CRZO_R2_EN_, pBuf, crzo_r2b_o_golden,
        "crzo_r2b", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, CRZO_R2_CRZO_OFST_ADDR, CRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, IMGO_R1_EN_, pBuf, imgo_r1b_o_golden,
        "imgo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, IMGO_R1_IMGO_OFST_ADDR, IMGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_B, DMA_EN, RRZO_R1_EN_, pBuf, rrzo_r1b_o_golden,
        "rrzo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, RRZO_R1_RRZO_OFST_ADDR, RRZO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA2_EN, RSSO_R2_EN_, pBuf, rsso_r2b_o_golden,
        "rsso_r2b", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, RSSO_R2_RSSO_OFST_ADDR, RSSO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, UFGO_R1_EN_, pBuf, ufgo_r1b_o_golden,
        "ufgo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, UFGO_R1_UFGO_OFST_ADDR, UFGO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA_EN, YUVBO_R1_EN_, pBuf, yuvbo_r1b_o_golden,
        "yuvbo_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, YUVBO_R1_YUVBO_OFST_ADDR, YUVBO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_B, DMA_EN, YUVCO_R1_EN_, pBuf, yuvco_r1b_o_golden,
        "yuvco_r1b", DMA_EN_GROUP0,
        CAM_READ_BITS(ptr, YUVCO_R1_YUVCO_OFST_ADDR, YUVCO_OFST_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B, DMA2_EN, YUVO_R1_EN_, pBuf, yuvo_r1b_o_golden,
        "yuvo_r1b", DMA_EN_GROUP1,
        CAM_READ_BITS(ptr, YUVO_R1_YUVO_OFST_ADDR, YUVO_OFST_ADDR)) == MFALSE) {
        ret++;
    }

    // CAMSV0 compare
    ptr          = ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);

    // top0_camsv0_imgo_o_golden_skippair
    skips.clear();
    for (MUINT32 i = 0;
        i < (sizeof(top0_camsv0_imgo_o_golden_skippair) /
             sizeof(top0_camsv0_imgo_o_golden_skippair[0]));
        i+=2) {
        skips.push_back(skipPairSt(top0_camsv0_imgo_o_golden_skippair[i],
            top0_camsv0_imgo_o_golden_skippair[i+1]));
    }
    LOG_INF("top0_camsv0_imgo_o_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    if (Beyond_Compare(CAMSV_0, CAMSV_DMA_EN, CAMSV_CTL_IMGO_EN_, pBuf,
        top0_camsv0_imgo_o_golden, "CAMSV0_IMGO", DMA_EN_GROUP0,
        CAMSV_READ_REG(ptr, CAMSV_IMGO_OFST_ADDR, OFFSET_ADDR),
        &skips) == MFALSE) {
        ret++;
    }

    // top0_camsv0_ufeo_o_golden_skippair
    skips.clear();
    for (MUINT32 i = 0;
        i < (sizeof(top0_camsv0_ufeo_o_golden_skippair) /
             sizeof(top0_camsv0_ufeo_o_golden_skippair[0]));
        i+=2) {
        skips.push_back(skipPairSt(top0_camsv0_ufeo_o_golden_skippair[i],
            top0_camsv0_ufeo_o_golden_skippair[i+1]));
    }
    LOG_INF("top0_camsv0_ufeo_o_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    if (Beyond_Compare(CAMSV_0, CAMSV_DMA_EN, CAMSV_CTL_UFE_EN_, pBuf,
        top0_camsv0_ufeo_o_golden, "CAMSV0_UFEO", DMA_EN_GROUP0,
        CAMSV_READ_REG(ptr, CAMSV_UFEO_OFST_ADDR, OFFSET_ADDR),
        &skips) == MFALSE) {
        ret++;
    }

    // CAMSV1 compare

    // 1 == pInputInfo->m_DCIF_ONE_RAWI_R2_CASE
    // IMGO_0 baseAddress = IMGO_1 baseAddress
    // UFEO_0 baseAddress = UFEO_1 baseAddress
    // so must compare with IMGO_0/UFEO_0 baseAddress
    ptr          = ((IspDrvCamsv**)_pCamsv)[CAMSV_1-CAM_MAX];
    CAMSV_DMA_EN = CAMSV_READ_REG(ptr, CAMSV_MODULE_EN);

    // top0_camsv1_imgo_o_golden_skippair
    skips.clear();
    for (MUINT32 i = 0;
        i < (sizeof(top0_camsv1_imgo_o_golden_skippair) /
             sizeof(top0_camsv1_imgo_o_golden_skippair[0]));
        i+=2) {
        skips.push_back(skipPairSt(top0_camsv1_imgo_o_golden_skippair[i],
            top0_camsv1_imgo_o_golden_skippair[i+1]));
    }
    LOG_INF("top0_camsv1_imgo_o_golden_skippair total dontcare pairs: %ld\n",
            skips.size());

    if (Beyond_Compare(CAMSV_0, CAMSV_DMA_EN, CAMSV_CTL_IMGO_EN_, pBuf,
        top0_camsv1_imgo_o_golden, "CAMSV1_IMGO", DMA_EN_GROUP0,
        CAMSV_READ_REG(ptr, CAMSV_IMGO_OFST_ADDR, OFFSET_ADDR),
        &skips) == MFALSE) {
        ret++;
    }

    // top0_camsv1_ufeo_o_golden_skippair2
    skips.clear();
    for (MUINT32 i = 0;
        i < (sizeof(top0_camsv1_ufeo_o_golden_skippair2) /
             sizeof(top0_camsv1_ufeo_o_golden_skippair2[0]));
        i+=2) {
        skips.push_back(skipPairSt(top0_camsv1_ufeo_o_golden_skippair2[i],
            top0_camsv1_ufeo_o_golden_skippair2[i+1]));
    }
    LOG_INF("top0_camsv1_ufeo_o_golden_skippair2 total dontcare pairs: %ld\n",
            skips.size());

    if (Beyond_Compare(CAMSV_0, CAMSV_DMA_EN, CAMSV_CTL_UFE_EN_, pBuf,
        top0_camsv1_ufeo_o_golden, "CAMSV1_UFEO", DMA_EN_GROUP0,
        CAMSV_READ_REG(ptr, CAMSV_UFEO_OFST_ADDR, OFFSET_ADDR),
        &skips) == MFALSE) {
        ret++;
    }
    skips.clear();

#endif
    LOG_INF("press any key continuous\n");
    getchar();
    return ret;

}


MINT32 Pattern_BitTrue_14_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;
    std::vector<skipPairSt> skips;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char golden_14_0_aao_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/aao_a_o.dhex"
    };
    static const unsigned char golden_14_0_aao_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/aao_b_o.dhex"
    };

    static const unsigned char golden_14_0_afo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/afo_a_o.dhex"
    };
    static const unsigned char golden_14_0_afo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/afo_b_o.dhex"
    };

    static const unsigned char golden_14_0_flko_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/flko_a_o.dhex"
    };
    static const unsigned char golden_14_0_flko_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/flko_b_o.dhex"
    };

    static const unsigned char golden_14_0_imgo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/imgo_a_o.dhex"
    };
    static const unsigned char golden_14_0_imgo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/imgo_b_o.dhex"
    };

    static const unsigned int dontcare_14_0_imgo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/imgo_a_o_dontcare.dhex"
    };
    static const unsigned int dontcare_14_0_imgo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/imgo_b_o_dontcare.dhex"
    };

    static const unsigned char golden_14_0_lcso_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/lcso_a_o.dhex"
    };
    static const unsigned char golden_14_0_lcso_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/lcso_b_o.dhex"
    };

    static const unsigned char golden_14_0_lmvo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/lmvo_a_o.dhex"
    };
    static const unsigned char golden_14_0_lmvo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/lmvo_b_o.dhex"
    };

    static const unsigned char golden_14_0_pso_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/pso_a_o.dhex"
    };
    static const unsigned char golden_14_0_pso_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/pso_b_o.dhex"
    };

    static const unsigned char golden_14_0_rrzo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/rrzo_a_o.dhex"
    };
    static const unsigned char golden_14_0_rrzo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/rrzo_b_o.dhex"
    };

    static const unsigned char golden_14_0_rsso_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/rsso_a_o.dhex"
    };
    static const unsigned char golden_14_0_rsso_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/rsso_b_o.dhex"
    };

    static const unsigned char golden_14_0_ufeo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/ufeo_a_o.dhex"
    };
    static const unsigned char golden_14_0_ufeo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/ufeo_b_o.dhex"
    };

    static const unsigned char golden_14_0_ufgo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/ufgo_a_o.dhex"
    };
    static const unsigned char golden_14_0_ufgo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame0/ufgo_b_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,AAO_EN_,pBuf,golden_14_0_aao_a,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,AFO_EN_,pBuf,golden_14_0_afo_a,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_R1_EN_,pBuf,golden_14_0_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    skips.clear();
    for (int i = 0; i < sizeof(dontcare_14_0_imgo_a)/sizeof(dontcare_14_0_imgo_a[0]); i+=2) {
        skips.push_back(skipPairSt(dontcare_14_0_imgo_a[i], dontcare_14_0_imgo_a[i+1]));
    }
    LOG_INF("total dontcare pairs: %d\n", skips.size());
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_14_0_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR),
        &skips) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCESO_R1_EN_,pBuf,golden_14_0_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_14_0_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_14_0_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_R1_EN_,pBuf,golden_14_0_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_R1_EN_,pBuf,golden_14_0_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_R1_EN_,pBuf,golden_14_0_ufeo_a,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_R1_EN_,pBuf,golden_14_0_ufgo_a,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_B,DMA_EN,AAO_EN_,pBuf,golden_14_0_aao_b,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,AFO_EN_,pBuf,golden_14_0_afo_b,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_R1_EN_,pBuf,golden_14_0_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    skips.clear();
    for (int i = 0; i < sizeof(dontcare_14_0_imgo_b)/sizeof(dontcare_14_0_imgo_b[0]); i+=2) {
        skips.push_back(skipPairSt(dontcare_14_0_imgo_b[i], dontcare_14_0_imgo_b[i+1]));
    }
    LOG_INF("total dontcare pairs: %d\n", skips.size());
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_14_0_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR),
        &skips) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCESO_R1_EN_,pBuf,golden_14_0_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_14_0_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_14_0_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_R1_EN_,pBuf,golden_14_0_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_R1_EN_,pBuf,golden_14_0_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_R1_EN_,pBuf,golden_14_0_ufeo_b,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_R1_EN_,pBuf,golden_14_0_ufgo_b,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();
    #endif
    return ret;
}


MINT32 Pattern_BitTrue_14_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char golden_14_1_aao_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/aao_a_o.dhex"
    };
    static const unsigned char golden_14_1_aao_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/aao_b_o.dhex"
    };

    static const unsigned char golden_14_1_afo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/afo_a_o.dhex"
    };
    static const unsigned char golden_14_1_afo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/afo_b_o.dhex"
    };

    static const unsigned char golden_14_1_flko_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/flko_a_o.dhex"
    };
    static const unsigned char golden_14_1_flko_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/flko_b_o.dhex"
    };

    static const unsigned char golden_14_1_imgo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/imgo_a_o.dhex"
    };
    static const unsigned char golden_14_1_imgo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/imgo_b_o.dhex"
    };

    static const unsigned char golden_14_1_lcso_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/lcso_a_o.dhex"
    };
    static const unsigned char golden_14_1_lcso_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/lcso_b_o.dhex"
    };

    static const unsigned char golden_14_1_lmvo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/lmvo_a_o.dhex"
    };
    static const unsigned char golden_14_1_lmvo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/lmvo_b_o.dhex"
    };

    static const unsigned char golden_14_1_pso_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/pso_a_o.dhex"
    };
    static const unsigned char golden_14_1_pso_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/pso_b_o.dhex"
    };

    static const unsigned char golden_14_1_rrzo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/rrzo_a_o.dhex"
    };
    static const unsigned char golden_14_1_rrzo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/rrzo_b_o.dhex"
    };

    static const unsigned char golden_14_1_rsso_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/rsso_a_o.dhex"
    };
    static const unsigned char golden_14_1_rsso_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/rsso_b_o.dhex"
    };

    static const unsigned char golden_14_1_ufeo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/ufeo_a_o.dhex"
    };
    static const unsigned char golden_14_1_ufeo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/ufeo_b_o.dhex"
    };

    static const unsigned char golden_14_1_ufgo_a[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/ufgo_a_o.dhex"
    };
    static const unsigned char golden_14_1_ufgo_b[] = {
        #include "Emulation/CAM_MULTI_SIMUL_BINNING/Golden/frame1/ufgo_b_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,AAO_EN_,pBuf,golden_14_1_aao_a,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,AFO_EN_,pBuf,golden_14_1_afo_a,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_R1_EN_,pBuf,golden_14_1_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_14_1_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCESO_R1_EN_,pBuf,golden_14_1_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_14_1_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_14_1_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_R1_EN_,pBuf,golden_14_1_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_R1_EN_,pBuf,golden_14_1_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_R1_EN_,pBuf,golden_14_1_ufeo_a,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_R1_EN_,pBuf,golden_14_1_ufgo_a,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_B,DMA_EN,AAO_EN_,pBuf,golden_14_1_aao_b,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,AFO_EN_,pBuf,golden_14_1_afo_b,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_R1_EN_,pBuf,golden_14_1_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_14_1_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCESO_R1_EN_,pBuf,golden_14_1_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_14_1_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_14_1_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_R1_EN_,pBuf,golden_14_1_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_R1_EN_,pBuf,golden_14_1_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_R1_EN_,pBuf,golden_14_1_ufeo_b,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_R1_EN_,pBuf,golden_14_1_ufgo_b,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();
    #endif
    return ret;
}

MINT32 Pattern_BitTrue_15(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char golden_aao_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/aao_b_o.dhex"
    };

    static const unsigned char golden_afo_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/afo_b_o.dhex"
    };

    static const unsigned char golden_afo_c[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/afo_c_o.dhex"
    };

    static const unsigned char golden_flko_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/flko_b_o.dhex"
    };

    static const unsigned char golden_imgo_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/imgo_b_o.dhex"
    };
    static const unsigned char golden_imgo_c[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/imgo_c_o.dhex"
    };

    static const unsigned char golden_lcso_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/lcso_b_o.dhex"
    };

    static const unsigned char golden_lmvo_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/lmvo_b_o.dhex"
    };

    static const unsigned char golden_pso_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/pso_b_o.dhex"
    };

    static const unsigned char golden_rrzo_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/rrzo_b_o.dhex"
    };
    static const unsigned char golden_rrzo_c[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/rrzo_c_o.dhex"
    };

    static const unsigned char golden_rsso_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.0/rsso_b_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_B,DMA_EN,AAO_EN_,pBuf,golden_aao_b,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,AFO_EN_,pBuf,golden_afo_b,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_R1_EN_,pBuf,golden_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCESO_R1_EN_,pBuf,golden_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_R1_EN_,pBuf,golden_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_R1_EN_,pBuf,golden_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_C,DMA_EN,AFO_EN_,pBuf,golden_afo_c,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_C,DMA_EN,IMGO_EN_,pBuf,golden_imgo_c,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_R1_EN_,pBuf,golden_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();
    #endif
    return ret;
}

MINT32 Pattern_BitTrue_16(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char golden_aao_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/aao_b_o.dhex"
    };

    static const unsigned char golden_afo_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/afo_b_o.dhex"
    };

    static const unsigned char golden_afo_c[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/afo_c_o.dhex"
    };

    static const unsigned char golden_flko_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/flko_b_o.dhex"
    };

    static const unsigned char golden_imgo_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/imgo_b_o.dhex"
    };
    static const unsigned char golden_imgo_c[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/imgo_c_o.dhex"
    };

    static const unsigned char golden_lcso_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/lcso_b_o.dhex"
    };

    static const unsigned char golden_pso_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/pso_b_o.dhex"
    };

    static const unsigned char golden_rrzo_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/rrzo_b_o.dhex"
    };
    static const unsigned char golden_rrzo_c[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/rrzo_c_o.dhex"
    };

    static const unsigned char golden_rsso_b[] = {
        #include "Emulation/CAM_MULTI_TWIN_BC_BINNING/Golden/frame.1/rsso_b_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_B,DMA_EN,AAO_EN_,pBuf,golden_aao_b,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,AFO_EN_,pBuf,golden_afo_b,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_R1_EN_,pBuf,golden_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCESO_R1_EN_,pBuf,golden_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_R1_EN_,pBuf,golden_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_R1_EN_,pBuf,golden_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_C,DMA_EN,AFO_EN_,pBuf,golden_afo_c,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_C,DMA_EN,IMGO_EN_,pBuf,golden_imgo_c,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_R1_EN_,pBuf,golden_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();
    #endif
    return ret;
}

MINT32 Pattern_BitTrue_15_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char golden_15_0_aao_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/aao_a_o.dhex"
    };
    static const unsigned char golden_15_0_aao_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/aao_b_o.dhex"
    };

    static const unsigned char golden_15_0_afo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/afo_a_o.dhex"
    };
    static const unsigned char golden_15_0_afo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/afo_b_o.dhex"
    };

    static const unsigned char golden_15_0_flko_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/flko_a_o.dhex"
    };
    static const unsigned char golden_15_0_flko_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/flko_b_o.dhex"
    };

    static const unsigned char golden_15_0_imgo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/imgo_a_o.dhex"
    };
    static const unsigned char golden_15_0_imgo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/imgo_b_o.dhex"
    };
    static const unsigned char golden_15_0_imgo_c[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/imgo_c_o.dhex"
    };

    static const unsigned char golden_15_0_lcso_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/lcso_a_o.dhex"
    };
    static const unsigned char golden_15_0_lcso_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/lcso_b_o.dhex"
    };

    static const unsigned char golden_15_0_lmvo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/lmvo_a_o.dhex"
    };
    static const unsigned char golden_15_0_lmvo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/lmvo_b_o.dhex"
    };

    static const unsigned char golden_15_0_pso_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/pso_a_o.dhex"
    };
    static const unsigned char golden_15_0_pso_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/pso_b_o.dhex"
    };

    static const unsigned char golden_15_0_rrzo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/rrzo_a_o.dhex"
    };
    static const unsigned char golden_15_0_rrzo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/rrzo_b_o.dhex"
    };
    static const unsigned char golden_15_0_rrzo_c[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/rrzo_c_o.dhex"
    };

    static const unsigned char golden_15_0_rsso_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/rsso_a_o.dhex"
    };
    static const unsigned char golden_15_0_rsso_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/rsso_b_o.dhex"
    };

    static const unsigned char golden_15_0_ufeo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/ufeo_b_o.dhex"
    };

    static const unsigned char golden_15_0_ufgo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame0/ufgo_b_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,AAO_EN_,pBuf,golden_15_0_aao_a,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,AFO_EN_,pBuf,golden_15_0_afo_a,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_R1_EN_,pBuf,golden_15_0_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_15_0_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCESO_R1_EN_,pBuf,golden_15_0_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_15_0_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_15_0_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_R1_EN_,pBuf,golden_15_0_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_R1_EN_,pBuf,golden_15_0_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }


    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_B,DMA_EN,AAO_EN_,pBuf,golden_15_0_aao_b,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,AFO_EN_,pBuf,golden_15_0_afo_b,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_R1_EN_,pBuf,golden_15_0_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_15_0_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCESO_R1_EN_,pBuf,golden_15_0_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_15_0_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_15_0_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_R1_EN_,pBuf,golden_15_0_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_R1_EN_,pBuf,golden_15_0_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_R1_EN_,pBuf,golden_15_0_ufeo_b,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_R1_EN_,pBuf,golden_15_0_ufgo_b,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_C,DMA_EN,IMGO_EN_,pBuf,golden_15_0_imgo_c,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_R1_EN_,pBuf,golden_15_0_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();
    #endif
    return ret;
}

MINT32 Pattern_BitTrue_15_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char golden_15_1_aao_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/aao_a_o.dhex"
    };
    static const unsigned char golden_15_1_aao_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/aao_b_o.dhex"
    };

    static const unsigned char golden_15_1_afo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/afo_a_o.dhex"
    };
    static const unsigned char golden_15_1_afo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/afo_b_o.dhex"
    };

    static const unsigned char golden_15_1_flko_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/flko_a_o.dhex"
    };
    static const unsigned char golden_15_1_flko_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/flko_b_o.dhex"
    };

    static const unsigned char golden_15_1_imgo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/imgo_a_o.dhex"
    };
    static const unsigned char golden_15_1_imgo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/imgo_b_o.dhex"
    };

    static const unsigned char golden_15_1_lcso_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/lcso_a_o.dhex"
    };
    static const unsigned char golden_15_1_lcso_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/lcso_b_o.dhex"
    };

    static const unsigned char golden_15_1_lmvo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/lmvo_a_o.dhex"
    };
    static const unsigned char golden_15_1_lmvo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/lmvo_b_o.dhex"
    };

    static const unsigned char golden_15_1_pso_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/pso_a_o.dhex"
    };
    static const unsigned char golden_15_1_pso_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/pso_b_o.dhex"
    };

    static const unsigned char golden_15_1_rrzo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/rrzo_a_o.dhex"
    };
    static const unsigned char golden_15_1_rrzo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/rrzo_b_o.dhex"
    };

    static const unsigned char golden_15_1_rsso_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/rsso_a_o.dhex"
    };
    static const unsigned char golden_15_1_rsso_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/rsso_b_o.dhex"
    };

    static const unsigned char golden_15_1_ufeo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/ufeo_a_o.dhex"
    };
    static const unsigned char golden_15_1_ufeo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/ufeo_b_o.dhex"
    };

    static const unsigned char golden_15_1_ufgo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/ufgo_a_o.dhex"
    };
    static const unsigned char golden_15_1_ufgo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame1/ufgo_b_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,AAO_EN_,pBuf,golden_15_1_aao_a,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,AFO_EN_,pBuf,golden_15_1_afo_a,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_R1_EN_,pBuf,golden_15_1_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_15_1_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCESO_R1_EN_,pBuf,golden_15_1_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_15_1_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_15_1_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_R1_EN_,pBuf,golden_15_1_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_R1_EN_,pBuf,golden_15_1_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_R1_EN_,pBuf,golden_15_1_ufeo_a,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_R1_EN_,pBuf,golden_15_1_ufgo_a,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_B,DMA_EN,AAO_EN_,pBuf,golden_15_1_aao_b,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,AFO_EN_,pBuf,golden_15_1_afo_b,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_R1_EN_,pBuf,golden_15_1_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_15_1_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCESO_R1_EN_,pBuf,golden_15_1_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_15_1_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_15_1_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_R1_EN_,pBuf,golden_15_1_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_R1_EN_,pBuf,golden_15_1_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_R1_EN_,pBuf,golden_15_1_ufeo_b,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_R1_EN_,pBuf,golden_15_1_ufgo_b,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();
    #endif
    return ret;
}


MINT32 Pattern_BitTrue_15_2(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;
    (void)_ptr;(void)length;(void)_uni;
    #if 0
    static const unsigned char golden_15_2_aao_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/aao_a_o.dhex"
    };
    static const unsigned char golden_15_2_aao_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/aao_b_o.dhex"
    };

    static const unsigned char golden_15_2_afo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/afo_a_o.dhex"
    };
    static const unsigned char golden_15_2_afo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/afo_b_o.dhex"
    };

    static const unsigned char golden_15_2_flko_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/flko_a_o.dhex"
    };
    static const unsigned char golden_15_2_flko_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/flko_b_o.dhex"
    };

    static const unsigned char golden_15_2_imgo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/imgo_a_o.dhex"
    };
    static const unsigned char golden_15_2_imgo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/imgo_b_o.dhex"
    };
    static const unsigned char golden_15_2_imgo_c[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/imgo_c_o.dhex"
    };

    static const unsigned char golden_15_2_lcso_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/lcso_a_o.dhex"
    };
    static const unsigned char golden_15_2_lcso_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/lcso_b_o.dhex"
    };

    static const unsigned char golden_15_2_lmvo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/lmvo_a_o.dhex"
    };
    static const unsigned char golden_15_2_lmvo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/lmvo_b_o.dhex"
    };

    static const unsigned char golden_15_2_pso_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/pso_a_o.dhex"
    };
    static const unsigned char golden_15_2_pso_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/pso_b_o.dhex"
    };

    static const unsigned char golden_15_2_rrzo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/rrzo_a_o.dhex"
    };
    static const unsigned char golden_15_2_rrzo_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/rrzo_b_o.dhex"
    };
    static const unsigned char golden_15_2_rrzo_c[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/rrzo_c_o.dhex"
    };

    static const unsigned char golden_15_2_rsso_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/rsso_a_o.dhex"
    };
    static const unsigned char golden_15_2_rsso_b[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/rsso_b_o.dhex"
    };

    static const unsigned char golden_15_2_ufeo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/ufeo_a_o.dhex"
    };

    static const unsigned char golden_15_2_ufgo_a[] = {
        #include "Emulation/CAM_MULTI_DYNAMIC_TWIN_BINNING/Golden/frame2/ufgo_a_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,AAO_EN_,pBuf,golden_15_2_aao_a,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,AFO_EN_,pBuf,golden_15_2_afo_a,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_R1_EN_,pBuf,golden_15_2_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_15_2_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCESO_R1_EN_,pBuf,golden_15_2_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_15_2_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_15_2_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_R1_EN_,pBuf,golden_15_2_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_R1_EN_,pBuf,golden_15_2_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_R1_EN_,pBuf,golden_15_2_ufeo_a,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_R1_EN_,pBuf,golden_15_2_ufgo_a,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_B,DMA_EN,AAO_EN_,pBuf,golden_15_2_aao_b,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,AFO_EN_,pBuf,golden_15_2_afo_b,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_R1_EN_,pBuf,golden_15_2_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_15_2_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCESO_R1_EN_,pBuf,golden_15_2_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_15_2_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_15_2_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_R1_EN_,pBuf,golden_15_2_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_R1_EN_,pBuf,golden_15_2_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_C,DMA_EN,IMGO_EN_,pBuf,golden_15_2_imgo_c,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_R1_EN_,pBuf,golden_15_2_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();
    #endif
    return ret;
}

MINT32 Pattern_freeBuf(MUINT32* _ptr,MUINTPTR inputInfo,MUINT32* _uni,MUINTPTR linkpath)
{
    IMEM_BUF_INFO *****pBuf, ******pCamsvBuf;
    IMEM_BUF_INFO ******pCamsvCQVirtualRegisterBuf, *pCamsvCQImemBuf;
    IMEM_BUF_INFO **pCamsvCQVirRegFBCFHAddrBuf;
    IMEM_BUF_INFO ***pMultiCamsvCQDescriptorBuf;
    IMEM_BUF_INFO ***pMultiCamsvCQDummyDescriptorBuf;
    IMEM_BUF_INFO ****pMultiCamsvCQVirRegFBCFHAddrBuf;

    IMemDrv* pImemDrv = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    (void)_ptr;
    (void)_uni;
    (void)linkpath;

    pImemDrv  = IMemDrv::createInstance();
    pBuf      = (IMEM_BUF_INFO *****)pInputInfo->pImemBufs;
    pCamsvBuf = (IMEM_BUF_INFO ******)pInputInfo->m_pCamsvImemBufs;

    pCamsvCQVirtualRegisterBuf =
        (IMEM_BUF_INFO ******)pInputInfo->m_pCamsvCQVirtualRegisterBuf;

    pCamsvCQImemBuf  = (IMEM_BUF_INFO *)pInputInfo->m_pCamsvCQDescriptorBuf;

    pCamsvCQVirRegFBCFHAddrBuf =
        (IMEM_BUF_INFO **)pInputInfo->m_pCamsvCQVirRegFBCFHAddrBuf;

    // For Twin Case
    pMultiCamsvCQDescriptorBuf =
        (IMEM_BUF_INFO ***)pInputInfo->m_pMultiCamsvCQDescriptorBuf;

    pMultiCamsvCQDummyDescriptorBuf =
        (IMEM_BUF_INFO ***)pInputInfo->m_pMultiCamsvCQDummyDescriptorBuf;

    pMultiCamsvCQVirRegFBCFHAddrBuf =
        (IMEM_BUF_INFO ****)pInputInfo->m_pMultiCamsvCQVirRegFBCFHAddrBuf;


    for(MUINT32 j=CAM_A;j<(MAX_ISP_HW_MODULE+1);j++){
        for(MUINT32 i=0;i<32; i++){
            for(MUINT32 n=0;n<2;n++){
                for(MUINT32 m=0;m<DMA_EN_GROUP_NUM;m++){
                    if(pBuf[j][i][n][m]->size != 0)
                        pImemDrv->freeVirtBuf(pBuf[j][i][n][m]);
                }
            }
        }
    }

    for(MUINT32 j=CAM_A;j<(MAX_ISP_HW_MODULE+1);j++){
        for(MUINT32 i=0;i<_SUBSAMPLE_NUM; i++){
            for(MUINT32 n=0;n<2;n++){
                for(MUINT32 m=0;m<DMA_EN_GROUP_NUM;m++){
                    for(MUINT32 o=0; o<CAMSV_FBC_GROUP_NUM;o++){
                        if(pCamsvBuf[j][i][n][m][o]->size != 0){
                            pImemDrv->freeVirtBuf(pCamsvBuf[j][i][n][m][o]);
                        }
                    }
                }
            }
        }
    }

    for(MUINT32 j=CAM_A;j<(MAX_ISP_HW_MODULE+1);j++){
        for(MUINT32 i=0;i<_SUBSAMPLE_NUM; i++){
            for(MUINT32 n=0;n<2;n++){
                for(MUINT32 m=0;m<DMA_EN_GROUP_NUM;m++){
                    for(MUINT32 o=0;o<CAMSV_FBC_GROUP_NUM;o++){
                        if(pCamsvCQVirtualRegisterBuf[j][i][n][m][o]->size != 0){
                            pImemDrv->freeVirtBuf(
                                pCamsvCQVirtualRegisterBuf[j][i][n][m][o]);
                        }
                    }
                }
            }
        }
    }


    for(MUINT32 i=0;i<_SUBSAMPLE_NUM; i++) {
        if(pCamsvCQVirRegFBCFHAddrBuf[i]->size != 0){
            pImemDrv->freeVirtBuf(pCamsvCQVirRegFBCFHAddrBuf[i]);
        }
    }

    for(MUINT32 i=0;i<(MAX_ISP_HW_MODULE + 1); i++) {
        for(MUINT32 j=0; j<CQ_NORMALPAGE_NUM; j++) {
            if(pMultiCamsvCQDescriptorBuf[i][j]->size != 0){
                pImemDrv->freeVirtBuf(pMultiCamsvCQDescriptorBuf[i][j]);
            }
        }
    }

    for(MUINT32 i=CAM_A;i<(MAX_ISP_HW_MODULE + 1); i++) {
        for(MUINT32 j=0; j<CQ_DUMMY_PAGE_NUM; j++) {
            if(pMultiCamsvCQDummyDescriptorBuf[i][j]->size != 0){
                pImemDrv->freeVirtBuf(pMultiCamsvCQDummyDescriptorBuf[i][j]);
            }
        }
    }

    for(MUINT32 i=0; i<(MAX_ISP_HW_MODULE + 1); i++) {
        for(MUINT32 j=0; j<_SUBSAMPLE_NUM; j++) {
            for(MUINT32 k=0; k<CQ_PAGE_NUM; k++){
                if(pMultiCamsvCQVirRegFBCFHAddrBuf[i][j][k]->size != 0) {
                    pImemDrv->freeVirtBuf(pMultiCamsvCQVirRegFBCFHAddrBuf[i][j][k]);
                }
            }
        }
    }

    if (pInputInfo->rawiImemBuf.size) {
        pImemDrv->freeVirtBuf(&pInputInfo->rawiImemBuf);
    }

    if(0 != pCamsvCQImemBuf->size) {
        pImemDrv->freeVirtBuf(pCamsvCQImemBuf);
    }

    pImemDrv->uninit();
    pImemDrv->destroyInstance();

    return 0;
}

MINT32 Pattern_release(MUINT32* _ptr,MUINTPTR inputInfo,MUINT32* _pCamsv,MUINTPTR linkpath)
{
    IMEM_BUF_INFO *****pBuf, ******pCamsvBuf;
    IMEM_BUF_INFO ******pCamsvCQVirtualRegisterBuf, *pCamsvCQImemBuf;
    IMEM_BUF_INFO **pCamsvCQVirRegFBCFHAddrBuf;
    IMEM_BUF_INFO ***pMultiCamsvCQDescriptorBuf;
    IMEM_BUF_INFO ***pMultiCamsvCQDummyDescriptorBuf;
    IMEM_BUF_INFO ****pMultiCamsvCQVirRegFBCFHAddrBuf;

    IMemDrv* pImemDrv = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    (void)_ptr;
    (void)linkpath;

    pImemDrv  = IMemDrv::createInstance();
    pBuf      = (IMEM_BUF_INFO*****)pInputInfo->pImemBufs;
    pCamsvBuf = (IMEM_BUF_INFO******)pInputInfo->m_pCamsvImemBufs;

    pCamsvCQVirtualRegisterBuf =
        (IMEM_BUF_INFO******)pInputInfo->m_pCamsvCQVirtualRegisterBuf;

    pCamsvCQImemBuf  = (IMEM_BUF_INFO *)pInputInfo->m_pCamsvCQDescriptorBuf;

    pCamsvCQVirRegFBCFHAddrBuf =
        (IMEM_BUF_INFO **)pInputInfo->m_pCamsvCQVirRegFBCFHAddrBuf;

    // For Twin Case
    pMultiCamsvCQDescriptorBuf =
        (IMEM_BUF_INFO ***)pInputInfo->m_pMultiCamsvCQDescriptorBuf;

    pMultiCamsvCQDummyDescriptorBuf =
        (IMEM_BUF_INFO ***)pInputInfo->m_pMultiCamsvCQDummyDescriptorBuf;

    pMultiCamsvCQVirRegFBCFHAddrBuf =
        (IMEM_BUF_INFO ****)pInputInfo->m_pMultiCamsvCQVirRegFBCFHAddrBuf;


    for(MUINT32 j=CAM_A;j<(MAX_ISP_HW_MODULE+1);j++){
        for(MUINT32 i=0;i<32; i++){
            for(MUINT32 n=0;n<2;n++){
                for(MUINT32 m=0;m<DMA_EN_GROUP_NUM;m++){
                    if(pBuf[j][i][n][m]->size != 0)
                        pImemDrv->freeVirtBuf(pBuf[j][i][n][m]);
                }
            }
        }
    }

    for(MUINT32 j=CAM_A;j<(MAX_ISP_HW_MODULE+1);j++){
        for(MUINT32 i=0;i<_SUBSAMPLE_NUM; i++){
            for(MUINT32 n=0;n<2;n++){
                for(MUINT32 m=0;m<DMA_EN_GROUP_NUM;m++){
                    for(MUINT32 o=0; o<CAMSV_FBC_GROUP_NUM;o++){
                        if(pCamsvBuf[j][i][n][m][o]->size != 0){
                            pImemDrv->freeVirtBuf(pCamsvBuf[j][i][n][m][o]);
                        }
                    }
                }
            }
        }
    }

    for(MUINT32 j=CAM_A;j<(MAX_ISP_HW_MODULE+1);j++){
        for(MUINT32 i=0;i<_SUBSAMPLE_NUM; i++){
            for(MUINT32 n=0;n<2;n++){
                for(MUINT32 m=0;m<DMA_EN_GROUP_NUM;m++){
                    for(MUINT32 o=0;o<CAMSV_FBC_GROUP_NUM;o++){
                        if(pCamsvCQVirtualRegisterBuf[j][i][n][m][o]->size != 0){
                            pImemDrv->freeVirtBuf(
                                pCamsvCQVirtualRegisterBuf[j][i][n][m][o]);
                        }
                    }
                }
            }
        }
    }

    for(MUINT32 i=0;i<_SUBSAMPLE_NUM; i++){
        if(pCamsvCQVirRegFBCFHAddrBuf[i]->size != 0){
            pImemDrv->freeVirtBuf(pCamsvCQVirRegFBCFHAddrBuf[i]);
        }
    }

    for(MUINT32 i=0;i<(MAX_ISP_HW_MODULE+1); i++){
        for(MUINT32 j=0;j<CQ_NORMALPAGE_NUM; j++){
            if(pMultiCamsvCQDescriptorBuf[i][j]->size != 0){
                pImemDrv->freeVirtBuf(pMultiCamsvCQDescriptorBuf[i][j]);
            }
        }
    }

    for(MUINT32 i=0; i<(MAX_ISP_HW_MODULE+1); i++) {
        for(MUINT32 j=0; j<_SUBSAMPLE_NUM; j++){
            for(MUINT32 k=0; k<CQ_PAGE_NUM;k++){
                if(pMultiCamsvCQVirRegFBCFHAddrBuf[i][j][k]->size != 0){
                    pImemDrv->freeVirtBuf(pMultiCamsvCQVirRegFBCFHAddrBuf[i][j][k]);
                }
            }
        }
    }

    for(MUINT32 i=CAM_A;i<(MAX_ISP_HW_MODULE+1);i++){
        for(MUINT32 j=0;j<CQ_DUMMY_PAGE_NUM; j++){
            if(pMultiCamsvCQDummyDescriptorBuf[i][j]->size != 0){
                pImemDrv->freeVirtBuf(pMultiCamsvCQDummyDescriptorBuf[i][j]);
            }
        }
    }

    if (pInputInfo->rawiImemBuf.size) {
        pImemDrv->freeVirtBuf(&pInputInfo->rawiImemBuf);
    }

    if(NULL != pCamsvCQImemBuf) {
        pImemDrv->freeVirtBuf(pCamsvCQImemBuf);
    }

    pImemDrv->uninit();
    pImemDrv->destroyInstance();

    //
    if(((ISP_DRV_CAM**)_ptr)[CAM_C]){
        ((ISP_DRV_CAM**)_ptr)[CAM_C]->uninit("Test_IspDrvCam_C");
        ((ISP_DRV_CAM**)_ptr)[CAM_C]->destroyInstance();
    }
    if(((ISP_DRV_CAM**)_ptr)[CAM_B]){
        ((ISP_DRV_CAM**)_ptr)[CAM_B]->uninit("Test_IspDrvCam_B");
        ((ISP_DRV_CAM**)_ptr)[CAM_B]->destroyInstance();
    }
    if(((ISP_DRV_CAM**)_ptr)[CAM_A]){
        ((ISP_DRV_CAM**)_ptr)[CAM_A]->uninit("Test_IspDrvCam_A");
        ((ISP_DRV_CAM**)_ptr)[CAM_A]->destroyInstance();
    }

    if(_pCamsv){
       switch(linkpath){
           case CAMSV_MAX:
                ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]
                    ->uninit("Test_IspDrvCamsv_0");

                ((IspDrvCamsv**)_pCamsv)[CAMSV_1-CAM_MAX]
                    ->uninit("Test_IspDrvCamsv_1");

                ((IspDrvCamsv**)_pCamsv)[CAMSV_0-CAM_MAX]->destroyInstance();
                ((IspDrvCamsv**)_pCamsv)[CAMSV_1-CAM_MAX]->destroyInstance();
                break;
           default:
                break;
       }
    }

    return 0;
}




#define CASE_OP     7   // 1: isp drv init|fakeSensor,
                        // 2: load MMU setting,
                        // 3: loading pattern(APMCU or CQ loading).
                        // 4:mem allocate + start,
                        // 5:stop, 6:deallocate

typedef MINT32 (*LDVT_CB)(MUINT32*,MUINTPTR,MUINT32*,MUINTPTR);

enum CASE_OP_LIST
{
    LDVT_DRV_INIT_OP = 0,
    MMU_ININ_OP,
    PATTENR_LOADING_OP,
    PATTERN_START_OP,
    PATTERN_STOP_OP,
    PATTERN_BITTRUE_OP,
    PATTERN_RELEASE_OP,
    CASE_OP_NUM,
};

LDVT_CB CB_TBL[][CASE_OP_NUM] = {
    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_0, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_0, Pattern_release},

    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_1, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_1, Pattern_release},

    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_2, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_2, Pattern_release},

    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_3, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_3, Pattern_release},

    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_4, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_4, Pattern_release},

    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_5, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_5, Pattern_release},

    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_6, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_6, Pattern_release},

    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_7, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_7 , Pattern_release},

    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_8, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_8, Pattern_release},

    {LDVT_DRV_INIT , MMU_INIT         , Pattern_Loading_9, Pattern_Start_1,
     Pattern_Stop_1, Pattern_BitTrue_9, Pattern_release},

    [30] = {LDVT_DRV_INIT , MMU_INIT , Pattern_Loading_30, Pattern_Start_1,
            Pattern_Stop_1, NULL     , Pattern_release}
};

LDVT_CB MULTIFRAME_CB_TBL_CASE14[][CASE_OP_NUM] = {
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_14_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_14_0,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_14_1,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_14_1,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_14_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_14_0,Pattern_release},
};

LDVT_CB MULTIFRAME_CASE15_CB_TBL[][CASE_OP_NUM] = {
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_15_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15_0,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_15_1,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15_1,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_15_2,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15_2,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_15_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15_0,Pattern_release},
};

LDVT_CB CB_TC15[][CASE_OP_NUM] = {
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_15,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15,Pattern_freeBuf},
    {         NULL,    NULL,Pattern_Loading_16,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_16,Pattern_freeBuf},
    {         NULL,    NULL,Pattern_Loading_15,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15,Pattern_release},
};

int IspDrvCam_LDVT(void)
{
    int ret = 0;
    char s[32];
    MUINT32 test_case, _raw_num;
    IMEM_BUF_INFO *****pimgBuf                       = NULL;
    IMEM_BUF_INFO ******pCamsvimgBuf                 = NULL;
    IMEM_BUF_INFO ******pCamsvCQVirtualRegisterBuf   = NULL;
    IMEM_BUF_INFO *pCamsvCQDescriptorBuf             = NULL;
    IMEM_BUF_INFO **pCamsvCQVirRegFBCFHAddrBuf       = NULL;
    IMEM_BUF_INFO ***pMultiCamsvCQDescriptorBuf      = NULL;
    IMEM_BUF_INFO ***pMultiCamsvCQDummyDescriptorBuf = NULL;
    IMEM_BUF_INFO ****pMultiCamsvCQVirRegFBCFHAddrBuf = NULL;

    ISP_DRV_CAM*    pIspDrv[CAM_MAX] = {NULL,NULL};

    IspDrvCamsv*    pCamsvIspDrv[CAMSV_MAX-CAM_MAX] =
                        {NULL,NULL,NULL,NULL,NULL,NULL};

    TestInputInfo   testInput;

    if (pimgBuf == NULL) {
        pimgBuf =
            (IMEM_BUF_INFO*****)malloc(
                sizeof(IMEM_BUF_INFO****) * (MAX_ISP_HW_MODULE + 1));

        for (MUINT32 i = 0; i < (MAX_ISP_HW_MODULE + 1); i++) {
            pimgBuf[i] = (IMEM_BUF_INFO****)malloc(sizeof(IMEM_BUF_INFO***) * 32);
            for (MUINT32 j = 0; j < 32; j++) {
                pimgBuf[i][j] =
                    (IMEM_BUF_INFO***)malloc(sizeof(IMEM_BUF_INFO**) * 2);

                for (MUINT32 k = 0; k < 2; k++) {
                     pimgBuf[i][j][k] =
                        (IMEM_BUF_INFO**)malloc(sizeof(IMEM_BUF_INFO*) * 2);

                     for (MUINT32 l = 0; l < DMA_EN_GROUP_NUM; l++) {
                         //calls default constructor
                         pimgBuf[i][j][k][l] = new IMEM_BUF_INFO();
                     }
                }
            }
        }
    }

    if (pCamsvimgBuf == NULL) {
        pCamsvimgBuf =
            (IMEM_BUF_INFO******)malloc(
                sizeof(IMEM_BUF_INFO*****) * (MAX_ISP_HW_MODULE + 1));

        for (MUINT32 i = 0; i < (MAX_ISP_HW_MODULE + 1); i++) {

            pCamsvimgBuf[i] = (IMEM_BUF_INFO*****)malloc(
                sizeof(IMEM_BUF_INFO****) * _SUBSAMPLE_NUM);


            for (MUINT32 j = 0; j < _SUBSAMPLE_NUM; j++) {

                pCamsvimgBuf[i][j] =
                    (IMEM_BUF_INFO****)malloc(
                        sizeof(IMEM_BUF_INFO***) * CamsvVF_STATUS_NUM);

                for (MUINT32 k = 0; k < CamsvVF_STATUS_NUM; k++) {

                     pCamsvimgBuf[i][j][k] =
                        (IMEM_BUF_INFO***)malloc(
                        sizeof(IMEM_BUF_INFO**) * DMA_EN_GROUP_NUM);

                     for (MUINT32 l = 0; l < DMA_EN_GROUP_NUM; l++) {

                        pCamsvimgBuf[i][j][k][l]  =
                            (IMEM_BUF_INFO**)malloc(
                                sizeof(IMEM_BUF_INFO*) * CAMSV_FBC_GROUP_NUM);

                         for (MUINT32 m = 0; m < CAMSV_FBC_GROUP_NUM; m++) {

                            //calls default constructor
                            pCamsvimgBuf[i][j][k][l][m] = new IMEM_BUF_INFO();
                         }
                     }
                }
            }
        }
    }

    if (pCamsvCQVirtualRegisterBuf == NULL) {

        pCamsvCQVirtualRegisterBuf =
            (IMEM_BUF_INFO******)malloc(
                sizeof(IMEM_BUF_INFO*****) * (MAX_ISP_HW_MODULE + 1));

        for (MUINT32 i = 0; i < (MAX_ISP_HW_MODULE + 1); i++) {

            pCamsvCQVirtualRegisterBuf[i] =
                (IMEM_BUF_INFO*****)malloc(
                sizeof(IMEM_BUF_INFO****) * _SUBSAMPLE_NUM);

            for (MUINT32 j = 0; j < _SUBSAMPLE_NUM; j++) {

                pCamsvCQVirtualRegisterBuf[i][j] =
                    (IMEM_BUF_INFO****)malloc(
                        sizeof(IMEM_BUF_INFO***) * CamsvVF_STATUS_NUM);

                for (MUINT32 k = 0; k < CamsvVF_STATUS_NUM; k++) {

                    pCamsvCQVirtualRegisterBuf[i][j][k] =
                        (IMEM_BUF_INFO***)malloc(
                        sizeof(IMEM_BUF_INFO**) * DMA_EN_GROUP_NUM);

                    for (MUINT32 l = 0; l < DMA_EN_GROUP_NUM; l++) {

                        pCamsvCQVirtualRegisterBuf[i][j][k][l]  =
                            (IMEM_BUF_INFO**)malloc(
                            sizeof(IMEM_BUF_INFO*) * CAMSV_FBC_GROUP_NUM);

                        for(MUINT32 m = 0;
                            m < CAMSV_FBC_GROUP_NUM;
                            m++) {
                           //calls default constructor
                           pCamsvCQVirtualRegisterBuf[i][j][k][l][m] =
                            new IMEM_BUF_INFO();
                        }
                    }
                }
            }
        }
    }

    if (pCamsvCQVirRegFBCFHAddrBuf == NULL) {
        pCamsvCQVirRegFBCFHAddrBuf =
            (IMEM_BUF_INFO**)malloc(
                sizeof(IMEM_BUF_INFO*) * _SUBSAMPLE_NUM);

        for (MUINT32 i = 0; i < _SUBSAMPLE_NUM; i++) {
            //calls default constructor
            pCamsvCQVirRegFBCFHAddrBuf[i] = new IMEM_BUF_INFO();
        }
    }

    if (pMultiCamsvCQDescriptorBuf == NULL) {

        pMultiCamsvCQDescriptorBuf =
            (IMEM_BUF_INFO***)malloc(
                sizeof(IMEM_BUF_INFO**) * (MAX_ISP_HW_MODULE + 1));

        for (MUINT32 i = 0; i < (MAX_ISP_HW_MODULE + 1); i++) {

            pMultiCamsvCQDescriptorBuf[i] =
                (IMEM_BUF_INFO**)malloc(
                sizeof(IMEM_BUF_INFO*) * CQ_NORMALPAGE_NUM);

            for (MUINT32 j = 0; j < CQ_NORMALPAGE_NUM; j++) {
                //calls default constructor
                pMultiCamsvCQDescriptorBuf[i][j] = new IMEM_BUF_INFO();

            }
        }
    }

    if (pMultiCamsvCQDummyDescriptorBuf == NULL) {

        pMultiCamsvCQDummyDescriptorBuf =
            (IMEM_BUF_INFO***)malloc(
                sizeof(IMEM_BUF_INFO**) * (MAX_ISP_HW_MODULE + 1));

        for (MUINT32 i = 0; i < (MAX_ISP_HW_MODULE + 1); i++) {

            pMultiCamsvCQDummyDescriptorBuf[i] =
                (IMEM_BUF_INFO**)malloc(
                sizeof(IMEM_BUF_INFO*) * CQ_DUMMY_PAGE_NUM);

            for (MUINT32 j = 0; j < CQ_DUMMY_PAGE_NUM; j++) {
                //calls default constructor
                pMultiCamsvCQDummyDescriptorBuf[i][j] = new IMEM_BUF_INFO();

            }
        }
    }



    if (pMultiCamsvCQVirRegFBCFHAddrBuf == NULL) {

        pMultiCamsvCQVirRegFBCFHAddrBuf =
            (IMEM_BUF_INFO****)malloc(
                sizeof(IMEM_BUF_INFO***) * (MAX_ISP_HW_MODULE + 1));

        for (MUINT32 i = 0; i < (MAX_ISP_HW_MODULE + 1); i++) {

            pMultiCamsvCQVirRegFBCFHAddrBuf[i] =
                (IMEM_BUF_INFO***)malloc(
                sizeof(IMEM_BUF_INFO**) * _SUBSAMPLE_NUM);

            for (MUINT32 j = 0; j < _SUBSAMPLE_NUM; j++) {

                pMultiCamsvCQVirRegFBCFHAddrBuf[i][j] =
                    (IMEM_BUF_INFO**)malloc(
                        sizeof(IMEM_BUF_INFO*) * CQ_PAGE_NUM);

                    for(MUINT32 k = 0; k < CQ_PAGE_NUM; k++){
                        //calls default constructor
                        pMultiCamsvCQVirRegFBCFHAddrBuf[i][j][k] = new IMEM_BUF_INFO();
                   }

            }
        }
    }

    if (pCamsvCQDescriptorBuf == NULL) {
        pCamsvCQDescriptorBuf = new IMEM_BUF_INFO();
    }

    testInput.pImemBufs                    = pimgBuf;
    testInput.m_pCamsvImemBufs             = pCamsvimgBuf;
    testInput.m_pCamsvCQDescriptorBuf      = pCamsvCQDescriptorBuf;
    testInput.m_pCamsvCQVirtualRegisterBuf = pCamsvCQVirtualRegisterBuf;
    testInput.m_pCamsvCQVirRegFBCFHAddrBuf = pCamsvCQVirRegFBCFHAddrBuf;

    testInput.m_pMultiCamsvCQDescriptorBuf      = pMultiCamsvCQDescriptorBuf;
    testInput.m_pMultiCamsvCQDummyDescriptorBuf = pMultiCamsvCQDummyDescriptorBuf;
    testInput.m_pMultiCamsvCQVirRegFBCFHAddrBuf = pMultiCamsvCQVirRegFBCFHAddrBuf;

    LOG_INF("##############################\n");
    LOG_INF("case 1: 1camsv_cq_pat_pass\n");
    LOG_INF("case 2: p1_ufe_raw14_4p_ufg_raw12_1p\n");
    LOG_INF("case 4: 1camsv_cq_pat_pass_FrameHeader-CQ\n");
    LOG_INF("case 5: 1camsv_cq_pat_pass_FrameHeader-CPU-withTimeStamp\n");
    LOG_INF("case 6: 2camsv_cq_ufo_pat_0521_pass      (BitFile:fpga_0616_camsys_postcts_0604_2raw_with_irq)-Cancel\n");
    LOG_INF("case 7: TWIN_ALL_ON_FPGA_TESTMDL_2X_PASS (BitFile:fpga_0616_camsys_postcts_0604_2raw_with_irq)\n");
    LOG_INF("case 8: 2camsv_cq_ufo_pat_0521_pass-ECO  (BitFile:fpga_0616_camsys_postcts_0604_2raw_with_irq)\n");
    LOG_INF("case 9: 2camsv_cq_1raw_ufo_pat_pass      (BitFile:fpga_0616_camsys_postcts_0604_2raw_with_irq)\n");
    LOG_INF("##############################\n");
    scanf("%s", s);
    test_case = atoi(s);
    getchar();

    LOG_INF("start case:%d\n",test_case);
    switch(test_case){
        case 0:
        case 1:
        case 2:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:

            switch (test_case) {
            case 0:
                testInput.tgNum = 0;
                _raw_num = 2;
                break;
            case 1:
                testInput.tgNum = 0;
                testInput.m_CamsvNum = 1;
                _raw_num = 1;
                testInput.m_DCIF = 1;
                break;
            case 2:
                testInput.tgNum = 1;
                testInput.m_CamsvNum = 0;
                _raw_num = 1;
                break;
           case 4:
                testInput.tgNum = 0;
                testInput.m_CamsvNum = 1;
                _raw_num = 1;
                break;
           case 5:
                testInput.tgNum = 0;
                testInput.m_CamsvNum = 1;
                _raw_num = 1;
                break;
           case 6:
                testInput.tgNum = 2;
                testInput.m_CamsvNum = 2;
                _raw_num = 2;
                break;
           case 7:
                testInput.tgNum = 1;
                testInput.m_CamsvNum = 0;
                _raw_num = 2;
                testInput.m_DCIF = 0;
                testInput.m_DCIF_ONE_RAWI_R2_CASE = 0;
                break;
           case 8:
                testInput.tgNum = 2;
                testInput.m_CamsvNum = 2;
                _raw_num = 2;
                testInput.m_DCIF = 1;
                testInput.m_DCIF_ONE_RAWI_R2_CASE = 1;
                break;
           case 9:
                testInput.tgNum = 2;
                testInput.m_CamsvNum = 2;
                _raw_num = 2;
                testInput.m_DCIF = 1;
                testInput.m_DCIF_ONE_RAWI_R2_CASE = 1;
                break;
            default:
                LOG_ERR("Unknown test case : %d\n", test_case);
                goto _EMU_EXIT;
            }
            LOG_INF("tg_num = %d, raw_num=%d\n", testInput.tgNum, _raw_num);

            if((ret = CB_TBL[test_case][LDVT_DRV_INIT_OP](
                        (MUINT32*)pIspDrv, _raw_num,
                        (MUINT32*)&pCamsvIspDrv, CAMSV_MAX)) != 0) {
                LOG_ERR(" case_%d step_0 fail\n",test_case);
                return 1;
            }

            if((ret = CB_TBL[test_case][MMU_ININ_OP](
                        (MUINT32*)pIspDrv, _raw_num,
                        (MUINT32*)pCamsvIspDrv, CAM_A)) != 0) {
                LOG_ERR(" case_%d step_1 fail\n",test_case);
                return 1;
            }

            if((ret = CB_TBL[test_case][PATTENR_LOADING_OP](
                        (MUINT32*)pIspDrv, _raw_num,
                        (MUINT32*)pCamsvIspDrv, (MUINTPTR)&testInput)) != 0) {
                LOG_ERR(" case_%d step_2 fail\n",test_case);
                return 1;
            }

            if ((ret = CB_TBL[test_case][PATTERN_START_OP](
                        (MUINT32*)pIspDrv, _raw_num,
                        (MUINT32*)pCamsvIspDrv, (MUINTPTR)&testInput)) != 0) {
                LOG_ERR(" case_%d step_3 fail\n",test_case);
                return 1;
            }

            if ((ret = CB_TBL[test_case][PATTERN_STOP_OP](
                        (MUINT32*)pIspDrv, 3,
                        (MUINT32*)pCamsvIspDrv, (MUINTPTR)&testInput)) != 0) {
                LOG_ERR(" case_%d step_4 fail\n",test_case);
                return 1;
            }

            if (CB_TBL[test_case][PATTERN_BITTRUE_OP] != NULL) {
                if ((ret = CB_TBL[test_case][PATTERN_BITTRUE_OP](
                    (MUINT32*)pIspDrv, 3,
                    (MUINT32*)pCamsvIspDrv, (MUINTPTR)&testInput)) != 0) {
                    LOG_ERR(" case_%d step_5 fail\n",test_case);
                }
            }
            else {
                LOG_INF("######################################\n");
                LOG_INF("this case_%d support no bit-true flow\n", test_case);
                LOG_INF("######################################\n");
            }

            if((ret = CB_TBL[test_case][PATTERN_RELEASE_OP](
                        (MUINT32*)pIspDrv, (MUINTPTR)&testInput,
                        (MUINT32*)pCamsvIspDrv, CAMSV_MAX)) != 0) {
                LOG_ERR(" case_%d step_6 fail\n",test_case);
                return 1;
            }
            break;
        case 14:
            int case14_frame_num;
            case14_frame_num = 3;
            testInput.tgNum = 2;
            _raw_num        = 3;

            LOG_INF("tg_num = %d, raw_num=%d\n", testInput.tgNum, _raw_num);

            for(int frame = 0; frame < case14_frame_num; frame++){
                LOG_INF("######################################\n");
                LOG_INF("frame_%d \n",frame);
                LOG_INF("######################################\n");


                if(MULTIFRAME_CB_TBL_CASE14[frame][LDVT_DRV_INIT_OP] != NULL){
                    if((ret = MULTIFRAME_CB_TBL_CASE14[frame][LDVT_DRV_INIT_OP]((MUINT32*)pIspDrv,3,(MUINT32*)&pCamsvIspDrv, CAM_A)) != 0){
                        LOG_ERR(" case_%d step_0 fail\n",test_case);
                    }
                }else{
                    LOG_INF(" MULTIFRAME_CB_TBL_CASE14[%d][LDVT_DRV_INIT_OP] = NULL\n",frame);
                }

                if(MULTIFRAME_CB_TBL_CASE14[frame][MMU_ININ_OP] != NULL){
                    if((ret = MULTIFRAME_CB_TBL_CASE14[frame][MMU_ININ_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv, CAM_A)) != 0){
                        LOG_ERR(" case_%d step_1 fail\n",test_case);
                    }
                }else{
                    LOG_INF(" MULTIFRAME_CB_TBL_CASE14[%d][MMU_ININ_OP] = NULL\n",frame);
                }

                if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTENR_LOADING_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv, (MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_2 fail\n",test_case);
                    return 1;
                }

                if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_START_OP]((MUINT32*)pIspDrv,_raw_num,(MUINT32*)pCamsvIspDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_3 fail\n",test_case);
                    return 1;
                }

                if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_STOP_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_4 fail\n",test_case);
                    return 1;
                }

                if(MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_BITTRUE_OP] != NULL){
                    if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_BITTRUE_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv,(MUINTPTR)&testInput)) != 0){
                        LOG_ERR(" case_%d step_5 fail\n",test_case);
                    }
                }
                else{
                    LOG_INF("######################################\n");
                    LOG_INF("this case_%d support no bit-true flow\n", test_case);
                    LOG_INF("######################################\n");
                }

                if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_RELEASE_OP]((MUINT32*)pIspDrv,(MUINTPTR)&testInput,(MUINT32*)pCamsvIspDrv,CAM_A)) != 0){
                    LOG_ERR(" case_%d step_6 fail\n",test_case);
                    return 1;
                }
            }
            break;
        case 15:
            int case15_frame_num;
            case15_frame_num = 4;
            testInput.tgNum  = 2;
            _raw_num         = 3;
            LOG_INF("tg_num = %d, raw_num=%d\n", testInput.tgNum, _raw_num);

            for(int frame = 0; frame < case15_frame_num; frame++){
                LOG_INF("######################################\n");
                LOG_INF("frame_%d \n",frame);
                LOG_INF("######################################\n");


                if(MULTIFRAME_CASE15_CB_TBL[frame][LDVT_DRV_INIT_OP] != NULL){
                    if((ret = MULTIFRAME_CASE15_CB_TBL[frame][LDVT_DRV_INIT_OP]((MUINT32*)pIspDrv,3,(MUINT32*)&pCamsvIspDrv, CAM_A)) != 0){
                        LOG_ERR(" case_%d step_0 fail\n",test_case);
                    }
                }else{
                    LOG_INF(" MULTIFRAME_CASE15_CB_TBL[%d][LDVT_DRV_INIT_OP] = NULL\n",frame);
                }

                if(MULTIFRAME_CASE15_CB_TBL[frame][MMU_ININ_OP] != NULL){
                    if((ret = MULTIFRAME_CASE15_CB_TBL[frame][MMU_ININ_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv, CAM_A)) != 0){
                        LOG_ERR(" case_%d step_1 fail\n",test_case);
                    }
                }else{
                    LOG_INF(" MULTIFRAME_CASE15_CB_TBL[%d][MMU_ININ_OP] = NULL\n",frame);
                }

                if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTENR_LOADING_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv, (MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_2 fail\n",test_case);
                    return 1;
                }

                if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_START_OP]((MUINT32*)pIspDrv,_raw_num,(MUINT32*)pCamsvIspDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_3 fail\n",test_case);
                    return 1;
                }

                if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_STOP_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_4 fail\n",test_case);
                    return 1;
                }

                if(MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_BITTRUE_OP] != NULL){
                    if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_BITTRUE_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv,(MUINTPTR)&testInput)) != 0){
                        LOG_ERR(" case_%d step_5 fail\n",test_case);
                    }
                }
                else{
                    LOG_INF("######################################\n");
                    LOG_INF("this case_%d support no bit-true flow\n", test_case);
                    LOG_INF("######################################\n");
                }

                if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_RELEASE_OP]((MUINT32*)pIspDrv,(MUINTPTR)&testInput,(MUINT32*)pCamsvIspDrv,CAM_A)) != 0){
                    LOG_ERR(" case_%d step_6 fail\n",test_case);
                    return 1;
                }

            }
            break;
        case 31:
            int frame_num;
            frame_num = 2;
            testInput.tgNum = 2;
            testInput.tg_enable[CAM_A] = 0;
            testInput.tg_enable[CAM_C] = 0;
            _raw_num = 3;

            for(int f = 0; f < frame_num; f++){

                if(CB_TC15[f][LDVT_DRV_INIT_OP] != NULL)
                    if((ret = CB_TC15[f][LDVT_DRV_INIT_OP]((MUINT32*)pIspDrv,3,(MUINT32*)&pCamsvIspDrv,CAM_A)) != 0){
                        LOG_ERR(" case_%d step_0 fail\n",test_case);
                        return 1;
                    }

                if(CB_TC15[f][MMU_ININ_OP] != NULL)
                    if((ret = CB_TC15[f][MMU_ININ_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv,CAM_A)) != 0){
                        LOG_ERR(" case_%d step_1 fail\n",test_case);
                        return 1;
                    }

                if((ret = CB_TC15[f][PATTENR_LOADING_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv, (MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_2 fail\n",test_case);
                    return 1;
                }
                if ((ret = CB_TC15[f][PATTERN_START_OP]((MUINT32*)pIspDrv,_raw_num,(MUINT32*)pCamsvIspDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_3 fail\n",test_case);
                    return 1;
                }

                if((ret = CB_TC15[f][PATTERN_STOP_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_4 fail\n",test_case);
                    return 1;
                }

                if(CB_TC15[f][PATTERN_BITTRUE_OP] != NULL){
                    if((ret = CB_TC15[f][PATTERN_BITTRUE_OP]((MUINT32*)pIspDrv,3,(MUINT32*)pCamsvIspDrv,(MUINTPTR)&testInput)) != 0){
                        LOG_ERR(" case_%d step_5 frame_%d fail\n",test_case, f);
                    }
                } else {
                    LOG_INF("######################################\n");
                    LOG_INF("this case_%d support no bit-true flow\n", test_case);
                    LOG_INF("######################################\n");
                }

                if((ret = CB_TC15[f][PATTERN_RELEASE_OP]((MUINT32*)pIspDrv,(MUINTPTR)&testInput,(MUINT32*)pCamsvIspDrv,CAMSV_MAX)) != 0){
                    LOG_ERR(" case_%d step_6 fail\n",test_case);
                    return 1;
                }
            }
            break;

        default:
            LOG_ERR("unsupported case(%d)\n",test_case);
            return 1;
            break;
    }

_EMU_EXIT:
    for(MUINT32 i=0;i<(MAX_ISP_HW_MODULE+1);i++){
        for(MUINT32 j=0;j<32;j++){
            for(MUINT32 k=0;k<2;k++){
                for(MUINT32 l=0;l<DMA_EN_GROUP_NUM; l++){
                    delete pimgBuf[i][j][k][l];
                }
                free(pimgBuf[i][j][k]);
            }
            free(pimgBuf[i][j]);
        }
        free(pimgBuf[i]);
    }
    pimgBuf = NULL;

    for(MUINT32 i=0;i<(MAX_ISP_HW_MODULE+1);i++){
        for(MUINT32 j=0;j<_SUBSAMPLE_NUM;j++){
            for(MUINT32 k=0;k<CamsvVF_STATUS_NUM;k++){
                for(MUINT32 l=0;l<DMA_EN_GROUP_NUM; l++){
                    for(MUINT32 m=0;m<CAMSV_FBC_GROUP_NUM; m++){
                        delete pCamsvimgBuf[i][j][k][l][m];
                    }
                    free(pCamsvimgBuf[i][j][k][l]);
                }
                free(pCamsvimgBuf[i][j][k]);
            }
            free(pCamsvimgBuf[i][j]);
        }
        free(pCamsvimgBuf[i]);
    }
    pCamsvimgBuf = NULL;

    for(MUINT32 i=0;i<(MAX_ISP_HW_MODULE+1);i++){
        for(MUINT32 j=0;j<_SUBSAMPLE_NUM;j++){
            for(MUINT32 k=0;k<CamsvVF_STATUS_NUM;k++){
                for(MUINT32 l=0;l<DMA_EN_GROUP_NUM; l++){
                    for(MUINT32 m=0;m<CAMSV_FBC_GROUP_NUM; m++){
                        delete pCamsvCQVirtualRegisterBuf[i][j][k][l][m];
                    }
                    free(pCamsvCQVirtualRegisterBuf[i][j][k][l]);
                }
                free(pCamsvCQVirtualRegisterBuf[i][j][k]);
            }
            free(pCamsvCQVirtualRegisterBuf[i][j]);
        }
        free(pCamsvCQVirtualRegisterBuf[i]);
    }
    pCamsvCQVirtualRegisterBuf = NULL;

    for(MUINT32 j=0;j<_SUBSAMPLE_NUM;j++){

        delete pCamsvCQVirRegFBCFHAddrBuf[j];
    }
    pCamsvCQVirRegFBCFHAddrBuf = NULL;


    for(MUINT32 i=0;i<(MAX_ISP_HW_MODULE+1);i++){
        for(MUINT32 j=0;j<CQ_NORMALPAGE_NUM;j++){
            delete pMultiCamsvCQDescriptorBuf[i][j];
        }
    }
    pMultiCamsvCQDescriptorBuf = NULL;

    for(MUINT32 i=0;i<(MAX_ISP_HW_MODULE+1);i++){
        for(MUINT32 j=0;j<CQ_NORMALPAGE_NUM;j++){
            delete pMultiCamsvCQDummyDescriptorBuf[i][j];
        }
    }
    pMultiCamsvCQDummyDescriptorBuf = NULL;


    for(MUINT32 i=0;i<(MAX_ISP_HW_MODULE+1);i++){
        for(MUINT32 j=0;j<_SUBSAMPLE_NUM;j++){
            for(MUINT32 k=0;k<CQ_PAGE_NUM;k++){
                delete pMultiCamsvCQVirRegFBCFHAddrBuf[i][j][k];
            }
        }
    }
    pMultiCamsvCQVirRegFBCFHAddrBuf = NULL;

    return ret;
}
