#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>


#include "isp_drv_cam.h"
#include "uni_drv.h"
#include "FakeDrvSensor.h"


//++ for alska ldvt non-swo #include "isp_function_cam.h"
#undef LOG_TAG
#define LOG_TAG "Test_IspDrv"


#define LOG_VRB(fmt, arg...)        printf("[%s]" fmt "\n", __FUNCTION__, ##arg)
#define LOG_DBG(fmt, arg...)        printf("[%s]" fmt "\n", __FUNCTION__, ##arg)
#define LOG_INF(fmt, arg...)        printf("[%s]" fmt "\n", __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)        printf("[%s]" fmt "\n", __FUNCTION__, ##arg)
#define LOG_ERR(fmt, arg...)        printf("error:[%s]" fmt "\n", __FUNCTION__, ##arg)

enum {
    _DMAI_TBL_LSCI = 0,
    _DMAI_TBL_BPCI,
    _DMAI_TBL_RAWI,
    _DMAI_TBL_PDI,
    _DMAI_TBL_CQI,
    _DMAI_TBL_NUM,
} _DMAI_TBL_INDEX;

struct TestInputInfo {
    struct DmaiTableInfo {
        const unsigned char     *pTblAddr;
        MUINT32                 tblLength;
        MUINT32                 simAddr;
    };
    MUINT32                 tgNum;
    MUINT32                 tg_enable[CAM_MAX];
    IMEM_BUF_INFO           ****pImemBufs;
    IMEM_BUF_INFO           rawiImemBuf;
    IMEM_BUF_INFO           CQImemBuf;
    DmaiTableInfo           DmaiTbls[CAM_MAX][_DMAI_TBL_NUM];

    TestInputInfo()
        {
            int i = 0, cam = 0;

            tgNum = 1;
            pImemBufs = NULL;
            for (cam = 0; cam < CAM_MAX; cam++) {
                tg_enable[cam] = 1;
                for (i = 0; i < _DMAI_TBL_NUM; i++) {
                    DmaiTbls[cam][i].pTblAddr = NULL;
                    DmaiTbls[cam][i].tblLength = 0;
                    DmaiTbls[cam][i].simAddr = 0;
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

    LOG_INF("cq_tbl = 0x%x tbl_size = 0x%x \n", cq_tbl, tbl_size);
    LOG_INF("simPA = 0x%x realPA = 0x%x delta = 0x%x\n", sim_pa, real_pa, delta_pa);
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


MUINT32 CamIdMap[] = {CAM_A, CAM_B, CAM_C};


MINT32 LDVT_DRV_INIT(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR linkpath)
{
    ISP_DRV_CAM** ptr = (ISP_DRV_CAM**)_ptr;

    switch(length){
    case 3:// no break if length >= 2
        ptr[CAM_C] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_C,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_C");
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
    case 2:// no break if length >= 2
        ptr[CAM_B] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_B");
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
        ptr[CAM_A] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_A");
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
        for(MUINT32 i=0;i<length;i++){
            ptr[i] = NULL;
        }
        LOG_INF("##############################\n");
        LOG_ERR("un-supported hw (%d)\n",(int)length);
        LOG_INF("##############################\n");
        return 1;
        break;
    }

    switch(linkpath){
        case CAM_A:
            if(ptr[CAM_A] != NULL){
                *(UniDrvImp**)_uni = (UniDrvImp*)UniDrvImp::createInstance(UNI_A);

                (*(UniDrvImp**)_uni)->init("Test_IspDrvCam_A",(IspDrvImp *)ptr[CAM_A]->getPhyObj());
            }
            else{
                LOG_ERR(" CAM_A is not allocated\n");
                return 1;
            }
            break;
        case CAM_B:
            if(ptr[CAM_B] != NULL){
                *(UniDrvImp**)_uni = (UniDrvImp*)UniDrvImp::createInstance(UNI_A);

                (*(UniDrvImp**)_uni)->init("Test_IspDrvCam_B",(IspDrvImp *)ptr[CAM_A]->getPhyObj());
            }
            else{
                LOG_ERR(" CAM_B is not allocated\n");
                return 1;
            }
            break;
        case CAM_C:
            if(ptr[CAM_C] != NULL){
                *(UniDrvImp**)_uni = (UniDrvImp*)UniDrvImp::createInstance(UNI_A);

                (*(UniDrvImp**)_uni)->init("Test_IspDrvCam_C",(IspDrvImp *)ptr[CAM_C]->getPhyObj());
            }
            else{
                LOG_ERR(" CAM_C is not allocated\n");
                return 1;
            }
            break;
        case CAM_MAX:
            *(UniDrvImp**)_uni = NULL;
            LOG_INF("uni is no needed\n");
            break;
    }


    for (int i = 0; i < (int)length; i++) {
        LOG_INF("Reseting CAM_%c(%d)", 'A'+i, CamIdMap[i]);
        if (ptr[CamIdMap[i]]) {
            ptr[CamIdMap[i]]->setDeviceInfo(_SET_RESET_HW_MOD, NULL);
        }

        LOG_INF("CAM_%c clear ctl/dma enable reg\n", 'A'+i);
        CAM_WRITE_REG((ptr[CamIdMap[i]]->getPhyObj()), CAM_CTL_EN, 0);
        CAM_WRITE_REG((ptr[CamIdMap[i]]->getPhyObj()), CAM_CTL_DMA_EN, 0);
    }

    return 0;
}

MINT32 MMU_INIT(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR linkpath)
{
    ISP_DRV_CAM** ptr = (ISP_DRV_CAM**)_ptr;
    _uni; linkpath;

    for (int i = 0; i < (int)length; i++) {
        if (ptr[i]) {
            ISP_LARB_MMU_STRUCT larbInfo;
            int idx = 0;
            // P1 HW dmao not through M4U

            //LOG_INF("Config MMU Larb to PA at CAM_%c .............", 'A'+i);

            larbInfo.regVal = 0;
            larbInfo.LarbNum = 0;
            //LOG_INF("config larb=%d offset=0x%x", larbInfo.LarbNum, larbInfo.regOffset);
            for (idx = 0; idx < 32; idx++) {
                larbInfo.regOffset = 0x380 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
            }

            larbInfo.LarbNum = 6;
            //LOG_INF("config larb=%d offset=0x%x", larbInfo.LarbNum, larbInfo.regOffset);
            for (idx = 0; idx < 31; idx++) {
                larbInfo.regOffset = 0x380 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

                larbInfo.regOffset = 0xf80 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

            }

            break; //only need once
        }
    }

    for (int i = 0; i < (int)length; i++) {
        LOG_INF("Reseting CAM_%c", 'A'+i);
        if (ptr[i]) {
            ptr[i]->setDeviceInfo(_SET_RESET_HW_MOD, NULL);
        }
    }

    return 0;
}


MINT32 Pattern_Start_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    UINT32 DMA_EN = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf;
    IMemDrv* pImemDrv = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;


    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;


    #define HEADER_MEM_ALLOC(Module,STR,STR2,STR3) \
        MUINT32 _tmp = (DMA_EN & STR3##_EN_);   \
        MUINT32 _cnt = 0;        \
        while(_tmp != 0){        \
            _cnt++;              \
            _tmp >>= 1;          \
        }                       \
        pBuf[Module][_cnt][1]->size = 4*16;   \
        LOG_INF("%s buf allocate: idx:%d, size:64 bytes\n",#STR,_cnt);    \
        if (pImemDrv->allocVirtBuf(pBuf[Module][_cnt][1]) < 0) {       \
            LOG_ERR(" imem alloc fail at %s\n",#STR);   \
            return 1;                                       \
        }                                                   \
        if (pImemDrv->mapPhyAddr(pBuf[Module][_cnt][1]) < 0) {         \
            LOG_ERR(" imem map fail at %s\n",#STR);     \
            return 1;                                       \
        }                                                   \
        LOG_INF("%s PA:%p VA:%p\n",#STR,pBuf[Module][_cnt][1]->phyAddr,pBuf[Module][_cnt][1]->virtAddr);  \
        STR2##_WRITE_REG(ptr,CAM_##STR##_FH_BASE_ADDR,pBuf[Module][_cnt][1]->phyAddr);


    //aao/pso/bpci's stride is useless. need to use xsize to cal. total memory size
    #define MEM_ALLOC(Module,STR,STR2,STR3) \
        MUINT32 tmp = (DMA_EN & STR3##_EN_), _dma_en = (DMA_EN & STR3##_EN_);   \
        MUINT32 cnt = 0;        \
        while(tmp != 0){        \
            cnt++;              \
            tmp >>= 1;          \
        }                       \
        if(0/*(_dma_en & IMGO_EN_)|| (_dma_en & RRZO_EN_)*/){\
            MUINT32 __tmp = (STR2##_READ_BITS(ptr,CAM_##STR##_XSIZE,XSIZE) + 1 );\
            STR2##_WRITE_BITS(ptr,CAM_##STR##_STRIDE,STRIDE,__tmp);\
            pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * STR2##_READ_BITS(ptr,CAM_##STR##_STRIDE,STRIDE);   \
        }\
        else{\
            if((_dma_en & BPCI_EN_)||(_dma_en & PSO_EN_)||(_dma_en & AAO_EN_)){\
                LOG_INF("%s 1D-DMA using xsize instead of stride",#STR);\
                pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * (STR2##_READ_BITS(ptr,CAM_##STR##_XSIZE,XSIZE)+1);   \
            }\
            else{\
                pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * STR2##_READ_BITS(ptr,CAM_##STR##_STRIDE,STRIDE);   \
            }\
        }\
        LOG_INF("%s buf allocate: idx:%d, size:%d\n",#STR,cnt,pBuf[Module][cnt][0]->size);    \
        if (pImemDrv->allocVirtBuf(pBuf[Module][cnt][0]) < 0) {       \
            LOG_ERR(" imem alloc fail at %s\n",#STR);   \
            return 1;                                       \
        }                                                   \
        if (pImemDrv->mapPhyAddr(pBuf[Module][cnt][0]) < 0) {         \
            LOG_ERR(" imem map fail at %s\n",#STR);     \
            return 1;                                       \
        }                                                   \
        if(0/*(_dma_en & IMGO_EN_)|| (_dma_en & RRZO_EN_)*/){\
        }\
        else\
            memset((MUINT8*)pBuf[Module][cnt][0]->virtAddr,0x0,pBuf[Module][cnt][0]->size);\
        LOG_INF("%s PA:%p  VA:%p\n",#STR,pBuf[Module][cnt][0]->phyAddr,pBuf[Module][cnt][0]->virtAddr); \
        STR2##_WRITE_REG(ptr,CAM_##STR##_BASE_ADDR,pBuf[Module][cnt][0]->phyAddr);      \

    #define MEM_ALLOC_1D(Module,STR,STR2,STR3) \
        MUINT32 tmp = (DMA_EN & STR3##_EN_), _dma_en = (DMA_EN & STR3##_EN_);   \
        MUINT32 cnt = 0;        \
        while(tmp != 0){        \
            cnt++;              \
            tmp >>= 1;          \
        }                       \
        if((_dma_en & BPCI_EN_)||(_dma_en & PSO_EN_)||(_dma_en & AAO_EN_)||(_dma_en & EISO_EN_)||(_dma_en & FLKO_EN_)){\
            LOG_INF("%s 1D-DMA using xsize instead of stride",#STR);\
            pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * (STR2##_READ_BITS(ptr,CAM_##STR##_XSIZE,XSIZE)+1);   \
            LOG_INF("%s buf allocate: idx:%d, size:%d\n",#STR,cnt,pBuf[Module][cnt][0]->size);    \
            if (pImemDrv->allocVirtBuf(pBuf[Module][cnt][0]) < 0) {       \
                LOG_ERR(" imem alloc fail at %s\n",#STR);   \
                return 1;                                       \
            }                                                   \
            if (pImemDrv->mapPhyAddr(pBuf[Module][cnt][0]) < 0) {         \
                LOG_ERR(" imem map fail at %s\n",#STR);     \
                return 1;                                       \
            }                                                   \
            memset((MUINT8*)pBuf[Module][cnt][0]->virtAddr,0x0,pBuf[Module][cnt][0]->size);\
            LOG_INF("%s PA:%p  VA:%p\n",#STR,pBuf[Module][cnt][0]->phyAddr,pBuf[Module][cnt][0]->virtAddr); \
            STR2##_WRITE_REG(ptr,CAM_##STR##_BASE_ADDR,pBuf[Module][cnt][0]->phyAddr);      \
        }\
        else{\
            LOG_INF("%s error not 1D-DMAO\n",#STR); \
        }\


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

        pInputInfo->CQImemBuf.size = pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength;

        if (pImemDrv->allocVirtBuf(&pInputInfo->CQImemBuf) < 0) {
            LOG_ERR(" imem alloc fail at %s\n", "CQI");
            return 1;
        }
        if (pImemDrv->mapPhyAddr(&pInputInfo->CQImemBuf) < 0) {
            LOG_ERR(" imem map fail at %s\n", "CQI");
            return 1;
        }

        LOG_INF("CQI srcTable=%p size=%d pa=%p, va=%p\n",
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

        for (MUINT32 c = 0;(c < length) && (c < sizeof(CamIdMap)/sizeof(CamIdMap[0]));c++) {
            drvPtr[c] = ((ISP_DRV_CAM**)_ptr)[CamIdMap[c]]->getPhyObj();
            cq0_base_addr[c] = CAM_READ_REG(drvPtr[c],CAM_CQ_THR0_BASEADDR);
        }

        LOG_INF("CQ0_BASE A/B/C : 0x%x/0x%x/0x%x\n", cq0_base_addr[0], cq0_base_addr[1], cq0_base_addr[2]);

        for (MUINT32 c = 0;(c < length) && (c < sizeof(CamIdMap)/sizeof(CamIdMap[0]));c++) {
            if (CAM_READ_BITS(drvPtr[c],CAM_CQ_THR0_CTL,CQ_THR0_EN)) {
                MUINT32 _offset = cq0_base_addr[c]-pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr;

                CAM_WRITE_REG(drvPtr[c],CAM_CQ_THR0_BASEADDR,pInputInfo->CQImemBuf.phyAddr+_offset);

                LOG_INF("update cam_%c cq0_base offset = 0x%x, new_base = 0x%x\n",
                    'a'+c, _offset, CAM_READ_REG(drvPtr[c],CAM_CQ_THR0_BASEADDR));

                if (CAM_READ_BITS(drvPtr[c],CAM_CQ_THR0_CTL,CQ_THR0_MODE) == 1/*immediate*/) {
                    LOG_INF("Trig cam_%c CQ0 \n", 'a'+c);
                    CAM_WRITE_BITS(drvPtr[c], CAM_CTL_START, CQ_THR0_START, 1);
                }
            }
        }

        LOG_INF("Wait CQ trig done, press to continue enable VF/RAWI ...\n");
        getchar();
    }


    for(MUINT32 c = 0;(c < length) && (c < sizeof(CamIdMap)/sizeof(CamIdMap[0]));c++){
        MUINT32 i = CamIdMap[c];
        ptr = ((ISP_DRV_CAM**)_ptr)[i]->getPhyObj();
        DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);
        LOG_INF("module_%d:  enabled DMA:0x%x\n",i,DMA_EN);
        if(DMA_EN & IMGO_EN_){
            MEM_ALLOC(i,IMGO,CAM,IMGO);
            HEADER_MEM_ALLOC(i,IMGO,CAM,IMGO);
        }
        if(DMA_EN & UFEO_EN_){
            MUINT32 _orgxsize = CAM_READ_BITS(ptr,CAM_UFEO_XSIZE,XSIZE);
            MUINT32 _orgstride =  CAM_READ_BITS(ptr,CAM_UFEO_STRIDE,STRIDE);
            CAM_WRITE_BITS(ptr,CAM_UFEO_XSIZE,XSIZE,_orgstride-1);
            MEM_ALLOC(i,UFEO,CAM,UFEO);
            HEADER_MEM_ALLOC(i,UFEO,CAM,UFEO);
            CAM_WRITE_BITS(ptr,CAM_UFEO_XSIZE,XSIZE,_orgxsize);
        }
        if(DMA_EN & UFGO_EN_){
            MEM_ALLOC(i,UFGO,CAM,UFGO);
            HEADER_MEM_ALLOC(i,UFGO,CAM,UFGO);
        }
        if(DMA_EN & RRZO_EN_){
            MEM_ALLOC(i,RRZO,CAM,RRZO);
            HEADER_MEM_ALLOC(i,RRZO,CAM,RRZO);
        }
        if(DMA_EN & AFO_EN_){
            MEM_ALLOC(i,AFO,CAM,AFO);
            HEADER_MEM_ALLOC(i,AFO,CAM,AFO);
        }
        if(DMA_EN & LCSO_EN_){
            MEM_ALLOC(i,LCSO,CAM,LCSO);
            HEADER_MEM_ALLOC(i,LCSO,CAM,LCSO);
        }
        if(DMA_EN & AAO_EN_){
            MEM_ALLOC_1D(i,AAO,CAM,AAO);
            HEADER_MEM_ALLOC(i,AAO,CAM,AAO);
        }
        if(DMA_EN & PSO_EN_){
            MEM_ALLOC_1D(i,PSO,CAM,PSO);
            HEADER_MEM_ALLOC(i,PSO,CAM,PSO);
        }

        if(DMA_EN & FLKO_EN_){
            MEM_ALLOC_1D(i,FLKO,CAM,FLKO);
            HEADER_MEM_ALLOC(i,FLKO,CAM,FLKO);
        }
        if(DMA_EN & EISO_EN_){
            MEM_ALLOC_1D(i,LMVO,CAM,EISO);
            HEADER_MEM_ALLOC(i,LMVO,CAM,EISO);
        }
        if(DMA_EN & RSSO_EN_){
            MEM_ALLOC(i,RSSO_A,CAM,RSSO);
            HEADER_MEM_ALLOC(i,RSSO_A,CAM,RSSO);
        }

        if(DMA_EN & BPCI_EN_){
            #if 1
            if (pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI].pTblAddr && pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI].tblLength) {
                MUINT32 tmp = (DMA_EN & BPCI_EN_), cnt = 0;
                while (tmp != 0) {
                    cnt++;
                    tmp >>= 1;
                }

                pBuf[i][cnt][0]->size = pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI].tblLength;

                if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem alloc fail at %s\n", "LSCI");
                    return 1;
                }
                if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem map fail at %s\n", "LSCI");
                    return 1;
                }

                CAM_WRITE_REG(ptr,CAM_BPCI_BASE_ADDR,pBuf[i][cnt][0]->phyAddr);

                LOG_INF("BPCI srcTable=%p size=%d pa=0x%x, va=%p\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI].tblLength,
                    pBuf[i][cnt][0]->phyAddr, (MUINT32 *)pBuf[i][cnt][0]->virtAddr);
                LOG_INF("BPCI xsize=0x%x ysize=0x%x stride=0x%x\n",
                    CAM_READ_BITS(ptr,CAM_BPCI_XSIZE,XSIZE), CAM_READ_BITS(ptr,CAM_BPCI_YSIZE,YSIZE),
                    CAM_READ_BITS(ptr,CAM_BPCI_STRIDE,STRIDE));
                LOG_INF("PDC_EN:%d,  PDC_OUT:%d\n",CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN),CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_OUT));

                memcpy((MUINT8*)pBuf[i][cnt][0]->virtAddr, (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI].tblLength);
            }
            else {
                LOG_ERR("BPCI table not found va(%p) size(%d)!!\n", pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_BPCI].tblLength);
                LOG_ERR("press to continue...\n");
                getchar();
            }

            #else
            MUINT32 table_size = 0;
            MUINT8* pTable = NULL;
            MUINT32 _orgsize = CAM_READ_BITS(ptr,CAM_BPCI_XSIZE,XSIZE);
            //overwrite bpci dma size
            if(CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN)){
                //#include "Emulation/table/pdo_bpci_table.h" //this table is from everest test code
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
        if(DMA_EN & LSCI_EN_){
            #if 1
            if (pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI].pTblAddr && pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI].tblLength) {
                MUINT32 tmp = (DMA_EN & LSCI_EN_), cnt = 0;
                while (tmp != 0) {
                    cnt++;
                    tmp >>= 1;
                }

                pBuf[i][cnt][0]->size = pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI].tblLength;

                if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem alloc fail at %s\n", "LSCI");
                    return 1;
                }
                if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem map fail at %s\n", "LSCI");
                    return 1;
                }

                CAM_WRITE_REG(ptr,CAM_LSCI_BASE_ADDR,pBuf[i][cnt][0]->phyAddr);

                LOG_INF("LSCI srcTable=%p size=%d pa=0x%x, va=%p\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI].tblLength,
                    pBuf[i][cnt][0]->phyAddr, (MUINT32 *)pBuf[i][cnt][0]->virtAddr);
                LOG_INF("LSCI xsize=0x%x ysize=0x%x stride=0x%x\n",
                    CAM_READ_BITS(ptr,CAM_LSCI_XSIZE,XSIZE), CAM_READ_BITS(ptr,CAM_LSCI_YSIZE,YSIZE),
                    CAM_READ_BITS(ptr,CAM_LSCI_STRIDE,STRIDE));

                memcpy((MUINT8*)pBuf[i][cnt][0]->virtAddr, (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI].tblLength);
            }
            else {
                LOG_ERR("LSCI table not found va(%p) size(%d)!!\n", pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_LSCI].tblLength);
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

        if (DMA_EN & PDI_EN_) {
          if (pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr && pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength) {
              MUINT32 tmp = (DMA_EN & PDI_EN_), cnt = 0;
              while (tmp != 0) {
                  cnt++;
                  tmp >>= 1;
              }
               pBuf[i][cnt][0]->size = pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength;
                if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem alloc fail at %s\n", "PDI");
                    return 1;
                }
                if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem map fail at %s\n", "PDI");
                    return 1;
                }

                CAM_WRITE_REG(ptr,CAM_PDI_BASE_ADDR,pBuf[i][cnt][0]->phyAddr);

                LOG_INF("PDI srcTable=%p size=%d pa=0x%x, va=%p\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength,
                    pBuf[i][cnt][0]->phyAddr, (MUINT32 *)pBuf[i][cnt][0]->virtAddr);
                LOG_INF("PDI xsize=0x%x ysize=0x%x stride=0x%x\n",
                    CAM_READ_BITS(ptr,CAM_PDI_XSIZE,XSIZE), CAM_READ_BITS(ptr,CAM_PDI_YSIZE,YSIZE),
                    CAM_READ_BITS(ptr,CAM_PDI_STRIDE,STRIDE));

                memcpy((MUINT8*)pBuf[i][cnt][0]->virtAddr, (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength);
            }
            else {
                LOG_ERR("LSCI table not found va(%p) size(%d)!!\n", pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength);
                LOG_ERR("press to continue...\n");
                getchar();
            }
        }

        if(DMA_EN & PDO_EN_){
            MEM_ALLOC(i,PDO,CAM,PDO);
            HEADER_MEM_ALLOC(i,PDO,CAM,PDO);
        }
    }

    ptr = (UniDrvImp*)_uni;
    DMA_EN = UNI_READ_REG(ptr, CAM_UNI_TOP_DMA_EN);
    LOG_INF("module_UNI:  enabled DMA:0x%x\n",DMA_EN);

    if(DMA_EN & RAWI_A_EN_){
        if (pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].pTblAddr &&
            pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].tblLength) {
            MUINT32 tmp = (DMA_EN & RAWI_A_EN_), cnt = 0;
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

            UNI_WRITE_REG(ptr,CAM_UNI_RAWI_BASE_ADDR,pInputInfo->rawiImemBuf.phyAddr);

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

    if (DMA_EN & RAWI_A_EN_) {
        LOG_INF("RAWI not enable VF, enable rawi\n");
        ptr = (UniDrvImp*)_uni;

        UNI_WRITE_BITS(ptr,CAM_UNI_TOP_RAWI_TRIG,TOP_RAWI_TRIG,1);
    }
    else {
        //
        for (int i = 0; i < pInputInfo->tgNum; i++) {
            if(pInputInfo->tg_enable[i] == 1) {
                ptr = ((ISP_DRV_CAM**)_ptr)[CamIdMap[i]]->getPhyObj();
                CAM_WRITE_BITS(ptr,CAM_TG_TIME_STAMP_CTL,TG_TIME_STAMP_SEL,0);// for local timestamp from TM
                CAM_WRITE_BITS(ptr,CAM_TG_VF_CON,SINGLE_MODE,0);
                CAM_WRITE_BITS(ptr,CAM_TG_VF_CON,VFDATA_EN,1);
            }
        }


        //enlarge test pattern if pattern size == TG size
        //cam will be fail when pattern size == TG size under continuous mode
        {
            CAM_REG_TG_SEN_GRAB_LIN tg;
            ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
            tg.Raw = CAM_READ_REG(ptr,CAM_TG_SEN_GRAB_LIN);
            TS_FakeDrvSensor().adjustPatternSize(tg.Bits.LIN_E - tg.Bits.LIN_S);
        }
    }
    return 0;
}

MINT32 Pattern_Stop_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
#if 1
    MUINT32 snrAry = 0, DMA_EN = 0;
    IspDrv* ptr = NULL;

    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;

    ptr = (UniDrvImp*)_uni;
    DMA_EN = UNI_READ_REG(ptr, CAM_UNI_TOP_DMA_EN);


    LOG_INF("Not yet support IRQ in current bitfile!!");
    LOG_INF("press any key to continuous or stop CPU here\n");
    getchar();
#else
    ISP_WAIT_IRQ_ST irq;
    _uni;linkpath;
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

    if (DMA_EN & RAWI_A_EN_) {
        LOG_INF("press any key to wait for rawi fetch\n");
        getchar();
    }
    else {
        //CAM_WRITE_BITS(((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN,0);
        for (int i = 0; i < pInputInfo->tgNum; i++) {
            ptr = ((ISP_DRV_CAM**)_ptr)[CamIdMap[i]]->getPhyObj();
            CAM_WRITE_BITS(ptr,CAM_TG_VF_CON,VFDATA_EN,0);
        }

        LOG_INF("press any key to wait for VF_OFF fetch\n");
        getchar();

        TS_FakeDrvSensor().powerOff("fake", 1, &snrAry);
        if (pInputInfo->tgNum > 1) {
            MUINT32 snrArySub = 1;
            TS_FakeDrvSensor().powerOff("fake1", 1, &snrArySub);
        }
    }

    return 0;
}

MINT32 Pattern_Loading_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    LOG_ERR("########## TO_DO Fix Cannon_ep ##########\n");
    const MUINT32 seninf[] = {
        //#include "Emulation/CAM_UFO/CAM_UFO_SENINF.h"
        0xffffffff
    };

    static const unsigned char pattern_1_bpci_tbl[] = {
            #include "Emulation/CAM_SINGLE/bpci.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_1_bpci_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_1_bpci_tbl)/sizeof(pattern_1_bpci_tbl[0]);

    static const unsigned char pattern_1_rawi_tbl[] = {
            #include "Emulation/CAM_SINGLE/rawi.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].pTblAddr = pattern_1_rawi_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_RAWI].tblLength = sizeof(pattern_1_rawi_tbl)/sizeof(pattern_1_rawi_tbl[0]);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/CAM_SINGLE/CAM_SINGLE_CAMA.h"
    //cannon_ep reg CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,0);

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_SINGLE/CAM_SINGLE_UNI.h"

    #if 0 //cannon_ep
    UNI_WRITE_REG(ptr, CAM_UNI_TOP_MOD_DCM_DIS, 0);
    UNI_WRITE_REG(ptr, CAM_UNI_TOP_DMA_DCM_DIS, 0);

    CAM_WRITE_REG(ptrCam, CAM_CTL_RAW_DCM_DIS, 0);
    CAM_WRITE_REG(ptrCam, CAM_CTL_DMA_DCM_DIS, 0);
    CAM_WRITE_REG(ptrCam, CAM_CTL_TOP_DCM_DIS, 0);

    CAM_WRITE_BITS(ptrCam, CAM_TG_FLASHA_CTL, FLASH_EN, 1);
    CAM_WRITE_BITS(ptrCam, CAM_TG_SEN_MODE, CMOS_EN, 1);

    //put here for pix mode alignment
    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf);
    }
    #endif

    return 0;
}

MINT32 Pattern_Loading_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_TWIN_ABC_ALLON/CAM_TWIN_ABC_ALLON_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_TWIN_ABC_ALLON/CAM_TWIN_ABC_ALLON_CAMB.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_TWIN_ABC_ALLON/CAM_TWIN_ABC_ALLON_CAMC.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_TWIN_ABC_ALLON/CAM_TWIN_ABC_ALLON_UNI.h"

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
        #include "Emulation/CAM_TWIN_ABC_ALLON/CAM_TWIN_ABC_ALLON_SENINF1.h"
        0xdeadbeef
    };

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);

    return 0;
}

MINT32 Pattern_Loading_2(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;

#if CANNON_LSCI
    static const unsigned char pattern_2_lsci_tbl_a[] = {
        0//#include "Emulation/CAM_SIMUL/CAM_SIMUL_LSCI_A.h"
    };
    static const unsigned char pattern_2_lsci_tbl_b[] = {
        0//#include "Emulation/CAM_SIMUL/CAM_SIMUL_LSCI_B.h"
    };
#endif
    static const unsigned char pattern_2_bpci_tbl_a[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_BPCI_A.h"
    };
    static const unsigned char pattern_2_bpci_tbl_b[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_BPCI_B.h"
    };

    LOG_INF("###########################");
    LOG_INF("###########################");
    LOG_INF("this case can't support bit-true\n");
    LOG_INF("DE can't sim. this simultaneous case\n");
    LOG_INF("###########################");
    LOG_INF("###########################");

    /* save dmai buffer location, for latter memory allocation and loading */
#if CANNON_LSCI
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_2_lsci_tbl_a;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_2_lsci_tbl_a)/sizeof(pattern_2_lsci_tbl_a[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_2_lsci_tbl_b;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_2_lsci_tbl_b)/sizeof(pattern_2_lsci_tbl_b[0]);
#endif
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_2_bpci_tbl_a;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_2_bpci_tbl_a)/sizeof(pattern_2_bpci_tbl_a[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_2_bpci_tbl_b;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_2_bpci_tbl_b)/sizeof(pattern_2_bpci_tbl_b[0]);



    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_SIMUL/CAM_SIMUL_CAMA.h"
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    //CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,0);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_SIMUL/CAM_SIMUL_CAMB.h"
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    //CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,0);

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_SIMUL/CAM_SIMUL_UNI.h"
    UNI_WRITE_BITS(ptr,CAM_UNI_TOP_MISC,DB_EN,1);

    //put here for pix mode alignment

    const MUINT32 seninf0_golden[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_SENINF.h"
        0xdeadbeef
    };

    const MUINT32 seninf1_golden[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_SENINF1.h"
        0xdeadbeef
    };

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf0_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf1_golden);
    }

    return 0;
}


MINT32 Pattern_Loading_3(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    MUINT32 snrAry = 0;
    inputInfo;

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_TWIN_AB_ALLON/CAM_TWIN_AB_ALLON_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_TWIN_AB_ALLON/CAM_TWIN_AB_ALLON_CAMB.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_TWIN_AB_ALLON/CAM_TWIN_AB_ALLON_UNI.h"

    //put here for pix mode alignment

    const MUINT32 seninf0_golden[] = {
        #include "Emulation/CAM_TWIN_AB_ALLON/CAM_TWIN_AB_ALLON_SENINF1.h"
        0xdeadbeef
    };

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf0_golden);

    return 0;
}


MINT32 Pattern_Loading_4(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    const MUINT32 seninf[] = {
        //include "Emulation/CAM_BIN/CAM_BIN_SENINF.h"
        0xffffffff
    };

    static const unsigned char pattern_4_lsci_tbl[] = {
        0//#include "Emulation/CAM_BIN/CAM_BIN_LSCI.h"
    };
    static const unsigned char pattern_4_bpci_tbl[] = {
        0//#include "Emulation/CAM_BIN/CAM_BIN_BPCI.h"
    };

    length;

    /* save dmai buffer location, for latter memory allocation and loading */
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_4_lsci_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_4_lsci_tbl)/sizeof(pattern_4_lsci_tbl[0]);

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_4_bpci_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_4_bpci_tbl)/sizeof(pattern_4_bpci_tbl[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_4_lsci_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_4_lsci_tbl)/sizeof(pattern_4_lsci_tbl[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_4_bpci_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_4_bpci_tbl)/sizeof(pattern_4_bpci_tbl[0]);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    //#include <Emulation/CAM_BIN/CAM_BIN_CAMA.h>
    //pattern have no enable cmos
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,1);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    //#include "Emulation/CAM_BIN/CAM_BIN_CAMB.h"
    //
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,1);

    //
    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    ptr = (UniDrvImp*)_uni;
    //#include "Emulation/CAM_BIN/CAM_BIN_UNI.h"
    //
    UNI_WRITE_BITS(ptr,CAM_UNI_TOP_MISC,DB_EN,1);

    //put here for pix mode alignment
    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf);
    }

    return 0;
}

MINT32 Pattern_Loading_5(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_TWIN_AC_ALLON/CAM_TWIN_AC_ALLON_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_TWIN_AC_ALLON/CAM_TWIN_AC_ALLON_CAMB.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_TWIN_AC_ALLON/CAM_TWIN_AC_ALLON_CAMC.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_TWIN_AC_ALLON/CAM_TWIN_AC_ALLON_UNI.h"

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
        #include "Emulation/CAM_TWIN_AC_ALLON/CAM_TWIN_AC_ALLON_SENINF1.h"
        0xdeadbeef
    };

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);

    return 0;
}

MINT32 Pattern_Loading_6(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;


    static const unsigned char pattern_bpci_a_tbl[] = {
            #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/dat/bpci_a.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_a_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_a_tbl)/sizeof(pattern_bpci_a_tbl[0]);


    static const unsigned char pattern_bpci_b_tbl[] = {
            #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/dat/bpci_b.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_b_tbl)/sizeof(pattern_bpci_b_tbl[0]);

    static const unsigned char pattern_bpci_c_tbl[] = {
            #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/dat/bpci_c.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_c_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_c_tbl)/sizeof(pattern_bpci_c_tbl[0]);

    static const unsigned char pattern_lsci_b_tbl[] = {
            #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/dat/lsci_b.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_lsci_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_lsci_b_tbl)/sizeof(pattern_lsci_b_tbl[0]);

    static const unsigned char pattern_pdi_a_tbl[] = {
            #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/dat/pdi_a.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_a_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_a_tbl)/sizeof(pattern_pdi_a_tbl[0]);

    static const unsigned char pattern_pdi_b_tbl[] = {
            #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/dat/pdi_b.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_b_tbl)/sizeof(pattern_pdi_b_tbl[0]);


    static const unsigned char pattern_cq_dat_tbl[] = {
            #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/dat/cq_data.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr = pattern_cq_dat_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength = sizeof(pattern_cq_dat_tbl)/sizeof(pattern_cq_dat_tbl[0]);
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr = 0x56c11800; //from image.list in pattern from DE

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER_CAMB.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER_CAMC.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    //ptr = (UniDrvImp*)_uni;
    //#include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER_CAM_UNI.h"

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER_CAM_SENINF1.h"
        0xdeadbeef
    };
    const MUINT32 seninf2_golden[] = {
    #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER_CAM_SENINF2.h"
        0xdeadbeef
    };

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    return 0;
}

MINT32 Pattern_Loading_15(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;

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

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }

    return 0;
}

MINT32 Pattern_Loading_16(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;

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

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    return 0;
}

MINT32 Pattern_Loading_13(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;


    static const unsigned char pattern_bpci_a_tbl[] = {
            #include "Emulation/CAM_RAW10_25M_3RAW_TEST/dat/bpci_a.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_a_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_a_tbl)/sizeof(pattern_bpci_a_tbl[0]);


    static const unsigned char pattern_bpci_b_tbl[] = {
            #include "Emulation/CAM_RAW10_25M_3RAW_TEST/dat/bpci_b.dat"
        };
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_b_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_b_tbl)/sizeof(pattern_bpci_b_tbl[0]);

    static const unsigned char pattern_bpci_c_tbl[] = {
            #include "Emulation/CAM_RAW10_25M_3RAW_TEST/dat/bpci_c.dat"
        };
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].pTblAddr = pattern_bpci_c_tbl;
    pInputInfo->DmaiTbls[CAM_C][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_bpci_c_tbl)/sizeof(pattern_bpci_c_tbl[0]);

    static const unsigned char pattern_pdi_a_tbl[] = {
            #include "Emulation/CAM_RAW10_25M_3RAW_TEST/dat/pdi_a.dat"
        };
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].pTblAddr = pattern_pdi_a_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].tblLength = sizeof(pattern_pdi_a_tbl)/sizeof(pattern_pdi_a_tbl[0]);

    static const unsigned char pattern_cq_dat_tbl[] = {
            #include "Emulation/CAM_RAW10_25M_3RAW_TEST/dat/cq_data.dat"
        };
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].pTblAddr = pattern_cq_dat_tbl;
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].tblLength = sizeof(pattern_cq_dat_tbl)/sizeof(pattern_cq_dat_tbl[0]);
    pInputInfo->DmaiTbls[0][_DMAI_TBL_CQI].simAddr = 0x511e3e00; //from image.list in pattern from DE

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_RAW10_25M_3RAW_TEST/CAM_RAW10_25M_3RAW_TEST_CAMA.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_RAW10_25M_3RAW_TEST/CAM_RAW10_25M_3RAW_TEST_CAMB.h"

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    #include "Emulation/CAM_RAW10_25M_3RAW_TEST/CAM_RAW10_25M_3RAW_TEST_CAMC.h"

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    //put here for pix mode alignment

    const MUINT32 seninf1_golden[] = {
    #include "Emulation/CAM_RAW10_25M_3RAW_TEST/CAM_RAW10_25M_3RAW_TEST_CAM_SENINF1.h"
        0xdeadbeef
    };

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);

    return 0;
}

MINT32 Pattern_Loading_14_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;


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

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    return 0;
}


MINT32 Pattern_Loading_14_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;


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

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    return 0;
}

MINT32 Pattern_Loading_15_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;


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

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    return 0;
}


MINT32 Pattern_Loading_15_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;


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

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    return 0;
}

MINT32 Pattern_Loading_15_2(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;


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

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf1_golden);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub,seninf2_golden);
    }
    return 0;
}


MINT32 Pattern_Loading_30(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;

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

    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry,seninf0_golden);

    return 0;
}


MBOOL Beyond_Compare(MUINT32 module, MUINT32 _DMA_EN, MUINT32 _DMA_BIT, IMEM_BUF_INFO**** pBuf,
        const unsigned char *_table, char *dma_name, MUINT32 dma_addr_offset = 0,  std::vector<skipPairSt> *pSkips = NULL)
{
    MBOOL   ret = MTRUE;
    MUINT32 DMA_EN = 0,_tmp=0,_cnt=0, skip_itr = 0, unit_size;
    MUINT32 *pMem = NULL, *pTable = (MUINT32*)_table;

    unit_size = sizeof(*pMem);
    _tmp = (_DMA_EN & _DMA_BIT);
    _cnt = 0;

    while(_tmp != 0){
        _cnt++;
        _tmp >>= 1;
    }

    if (pBuf[module][_cnt][0]->size == 0) {
        LOG_INF("%s no buf for bit true, bypass", dma_name);
        goto _EXIT_COMPARE;
    }
    if (unit_size & (unit_size-1) != 0) {
        LOG_INF("not yet support non 2'pow unit size\n");
        goto _EXIT_COMPARE;
    }

    pMem = (MUINT32*)(pBuf[module][_cnt][0]->virtAddr + dma_addr_offset);
    _tmp = (pBuf[module][_cnt][0]->size - dma_addr_offset)/unit_size;

    LOG_INF("###########################\nmodule:%d dmx_bit:%d verify#:%d offset:0x%x\n",
        module, _cnt, pBuf[module][_cnt][0]->size,dma_addr_offset);

    for (_cnt = 0; _cnt < _tmp; ) {
        if (pSkips) {
            if (skip_itr < pSkips->size()) {
                if ((_cnt << 2) == pSkips->at(skip_itr).offset) {
                    #if 0 //debug only
                    LOG_INF("%s matching skip region: offset:%d(row:%d)_bytes:%d jump row: %d\n", dma_name,
                        pSkips->at(skip_itr).offset, pSkips->at(skip_itr).offset/unit_size,
                        pSkips->at(skip_itr).nbyte, pSkips->at(skip_itr).nbyte/unit_size);
                    #endif
                    if ((pSkips->at(skip_itr).nbyte & (unit_size-1)) || (pSkips->at(skip_itr).offset & (unit_size-1))) {
                        LOG_ERR("%s error: currently only support %dbyte don't care data pattern\n", dma_name, unit_size);
                    }
                    else {
                        _cnt    += (pSkips->at(skip_itr).nbyte/unit_size);
                        pMem    += (pSkips->at(skip_itr).nbyte/unit_size);
                        pTable  += (pSkips->at(skip_itr).nbyte/unit_size);

                        skip_itr++;

                        continue;
                    }
                }
            }
        }

        if ((*pMem) != (*pTable)) {
            LOG_ERR("%s bit true fail,@src=%p golden=%p\n",dma_name,pMem,pTable);
            LOG_ERR("%s bit true fail offset=0x%x 0x%x_0x%x\n",dma_name,_cnt,*pMem,*pTable);
            if (_cnt >= 1)
                LOG_ERR("%s bit true fail,prev1: 0x%x_0x%x\n",dma_name,*(pMem-1),*(pTable-1));
            if (_cnt >= 2)
                LOG_ERR("%s bit true fail,prev2: 0x%x_0x%x\n",dma_name,*(pMem-2),*(pTable-2));
            break;
        }
        _cnt++;
        pMem++;
        pTable++;

    }
    if(_cnt != _tmp){
        ret = MFALSE;
        LOG_ERR("%s bit true fail,%d_%d\n",dma_name,_cnt,_tmp);
    }
    else{
        ret = MTRUE;
        LOG_INF("%s bit true pass\n",dma_name);
    }

    LOG_INF("###########################\n");

_EXIT_COMPARE:

    return ret;
}


MINT32 Pattern_BitTrue_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;

    static const unsigned char golden_flko[] = {
        #include "Emulation/CAM_SINGLE/Golden/flko_a_o.dhex"
    };
    static const unsigned char golden_imgo[] = {
        #include "Emulation/CAM_SINGLE/Golden/imgo_a_o.dhex"
    };
    static const unsigned char golden_lcso[] = {
        #include "Emulation/CAM_SINGLE/Golden/lcso_a_o.dhex"
    };
    static const unsigned char golden_lmvo[] = {
        #include "Emulation/CAM_SINGLE/Golden/lmvo_a_o.dhex"
    };
    static const unsigned char golden_pso[] = {
        #include "Emulation/CAM_SINGLE/Golden/pso_a_o.dhex"
    };
    static const unsigned char golden_rrzo[] = {
        #include "Emulation/CAM_SINGLE/Golden/rrzo_a_o.dhex"
    };
    static const unsigned char golden_rsso[] = {
        #include "Emulation/CAM_SINGLE/Golden/rsso_a_o.dhex"
    };
    static const unsigned char golden_ufeo[] = {
        #include "Emulation/CAM_SINGLE/Golden/ufeo_a_o.dhex"
    };
    static const unsigned char golden_ufgo[] = {
        #include "Emulation/CAM_SINGLE/Golden/ufgo_a_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,LCSO_EN_,pBuf,golden_lcso,"LCSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_pso,"PSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_EN_,pBuf,golden_ufeo,"UFEO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_ufgo,"UFGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_imgo,"IMGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_rrzo,"RRZO") == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_flko,"FLKO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_lmvo,"EISO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_rsso,"RSSO") == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

    return ret;
}


MINT32 Pattern_BitTrue_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;


    static const unsigned char golden_1_aao_a[] = {
        #include "Emulation/CAM_TWIN_ABC_ALLON/Golden/aao_a_o.dhex"
    };
    static const unsigned char golden_1_imgo_a[] = {
        #include "Emulation/CAM_TWIN_ABC_ALLON/Golden/imgo_a_o.dhex"
    };
    static const unsigned char golden_1_pso_a[] = {
        #include "Emulation/CAM_TWIN_ABC_ALLON/Golden/pso_a_o.dhex"
    };
    static const unsigned char golden_1_rrzo_a[] = {
        #include "Emulation/CAM_TWIN_ABC_ALLON/Golden/rrzo_a_o.dhex"
    };
    static const unsigned char golden_1_ufgo_a[] = {
        #include "Emulation/CAM_TWIN_ABC_ALLON/Golden/ufgo_a_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,AAO_EN_,pBuf,golden_1_aao_a,"AAO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_1_imgo_a,"IMGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_1_pso_a,"PSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_1_rrzo_a,"RRZO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_1_ufgo_a,"UFGO") == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

    return ret;
}

MINT32 Pattern_BitTrue_3(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;

    static const unsigned char golden_flko_a[] = {
        #include "Emulation/CAM_TWIN_AB_ALLON/Golden/flko_a_o.dhex"
    };
    static const unsigned char golden_imgo_a[] = {
        #include "Emulation/CAM_TWIN_AB_ALLON/Golden/imgo_a_o.dhex"
    };
    static const unsigned char golden_pso_a[] = {
        #include "Emulation/CAM_TWIN_AB_ALLON/Golden/pso_a_o.dhex"
    };
    static const unsigned char golden_rrzo_a[] = {
        #include "Emulation/CAM_TWIN_AB_ALLON/Golden/rrzo_a_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_flko_a,"FLKO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_imgo_a,"IMGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_pso_a,"PSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_rrzo_a,"RRZO") == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

    return ret;
}

MINT32 Pattern_BitTrue_2(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;

    static const unsigned char golden_flko[] = {
        #include "Emulation/CAM_SIMUL/golden/flko_a_o.dhex"
    };
    static const unsigned char golden_imgo[] = {
        #include "Emulation/CAM_SIMUL/golden/imgo_a_o.dhex"
    };
    static const unsigned char golden_lcso[] = {
        #include "Emulation/CAM_SIMUL/golden/lcso_a_o.dhex"
    };
    static const unsigned char golden_lmvo[] = {
        #include "Emulation/CAM_SIMUL/golden/lmvo_a_o.dhex"
    };
    static const unsigned char golden_pso[] = {
        #include "Emulation/CAM_SIMUL/golden/pso_a_o.dhex"
    };
    static const unsigned char golden_rrzo[] = {
        #include "Emulation/CAM_SIMUL/golden/rrzo_a_o.dhex"
    };
    static const unsigned char golden_rsso[] = {
        #include "Emulation/CAM_SIMUL/golden/rsso_a_o.dhex"
    };
    static const unsigned char golden_ufeo[] = {
        #include "Emulation/CAM_SIMUL/golden/ufeo_a_o.dhex"
    };
    static const unsigned char golden_ufgo[] = {
        #include "Emulation/CAM_SIMUL/golden/ufgo_a_o.dhex"
    };

    static const unsigned char golden_flko_b[] = {
        #include "Emulation/CAM_SIMUL/golden/flko_b_o.dhex"
    };
    static const unsigned char golden_imgo_b[] = {
        #include "Emulation/CAM_SIMUL/golden/imgo_b_o.dhex"
    };
    static const unsigned char golden_lcso_b[] = {
        #include "Emulation/CAM_SIMUL/golden/lcso_b_o.dhex"
    };
    static const unsigned char golden_lmvo_b[] = {
        #include "Emulation/CAM_SIMUL/golden/lmvo_b_o.dhex"
    };
    static const unsigned char golden_pso_b[] = {
        #include "Emulation/CAM_SIMUL/golden/pso_b_o.dhex"
    };
    static const unsigned char golden_rrzo_b[] = {
        #include "Emulation/CAM_SIMUL/golden/rrzo_b_o.dhex"
    };
    static const unsigned char golden_rsso_b[] = {
        #include "Emulation/CAM_SIMUL/golden/rsso_b_o.dhex"
    };
    static const unsigned char golden_ufeo_b[] = {
        #include "Emulation/CAM_SIMUL/golden/ufeo_b_o.dhex"
    };
    static const unsigned char golden_ufgo_b[] = {
        #include "Emulation/CAM_SIMUL/golden/ufgo_b_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);
    if (Beyond_Compare(CAM_A,DMA_EN,LCSO_EN_,pBuf,golden_lcso,"LCSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_pso,"PSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_EN_,pBuf,golden_ufeo,"UFEO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_ufgo,"UFGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_imgo,"IMGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_rrzo,"RRZO") == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_flko,"FLKO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_lmvo,"EISO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_rsso,"RSSO") == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);
    if (Beyond_Compare(CAM_B,DMA_EN,LCSO_EN_,pBuf,golden_lcso_b,"LCSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_pso_b,"PSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_EN_,pBuf,golden_ufeo_b,"UFEO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_EN_,pBuf,golden_ufgo_b,"UFGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_imgo_b,"IMGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_rrzo_b,"RRZO") == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_EN_,pBuf,golden_flko_b,"FLKO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_lmvo_b,"EISO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_EN_,pBuf,golden_rsso_b,"RSSO") == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

    return ret;
}

MINT32 Pattern_BitTrue_4(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;

    static const unsigned char golden_4_aao[] = {
    0//#include "Emulation/CAM_BIN/Golden/aao_golden.dhex"
    };
    static const unsigned char golden_4_lcso[] = {
    0//#include "Emulation/CAM_BIN/Golden/lcso_golden.dhex"
    };
    static const unsigned char golden_4_afo_a[] = {
    0//#include "Emulation/CAM_BIN/Golden/afo_a_golden.dhex"
    };
    static const unsigned char golden_4_afo_b[] = {
    0//#include "Emulation/CAM_BIN/Golden/afo_b_golden.dhex"
    };


#if 0 //code size issue, do not build-in. also, imgo/rrzo's stride is not the same as golden
    static const unsigned char golden_4_imgo_a[] = {
    #include "Emulation/CAM_TWIN_NO_DBN/Golden/imgo_a_golden.dhex"
    };
    static const unsigned char golden_4_imgo_b[] = {
    #include "Emulation/CAM_TWIN_NO_DBN/Golden/imgo_b_golden.dhex"
    };
    static const unsigned char golden_4_rrzo_a[] = {
    #include "Emulation/CAM_TWIN_NO_DBN/Golden/rrzo_a_golden.dhex"
    };
    static const unsigned char golden_4_rrzo_b[] = {
    #include "Emulation/CAM_TWIN_NO_DBN/Golden/rrzo_b_golden.dhex"
    };
#endif

    #define COMPARE(module,STR,STR2)\
            _tmp = (DMA_EN & STR##_EN_);\
            _cnt = 0;\
            while(_tmp != 0){\
                _cnt++;\
                _tmp >>= 1;\
            }\
            pTable = (MUINT32*)golden_4_##STR2;\
            pMem = (MUINT32*)pBuf[module][_cnt][0]->virtAddr;\
            _tmp = pBuf[module][_cnt][0]->size/sizeof(MUINT32);\
            _cnt = 0;\
            LOG_INF("###########################\n");\
            while((*pMem++ == *pTable++) && (_cnt++ != _tmp)){\
            }\
            if(_cnt != (_tmp+1)){\
                ret++;\
                LOG_ERR("%s bit true fail,%d_%d\n",#STR,_cnt,(_tmp+1));\
            }\
            else{\
                LOG_INF("%s bit true pass\n",#STR);\
            }\
            LOG_INF("###########################\n");\


    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    COMPARE(CAM_A,AAO,aao);
    COMPARE(CAM_A,LCSO,lcso);
    COMPARE(CAM_A,AFO,afo_a);

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    COMPARE(CAM_B,AFO,afo_b);
    LOG_INF("press any key continuous\n");
    getchar();
    return ret;
}



MINT32 Pattern_BitTrue_5(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;

    static const unsigned char golden_5_imgo_a[] = {
        #include "Emulation/CAM_TWIN_AC_ALLON/Golden/imgo_a_o.dhex"
    };
    static const unsigned char golden_5_pso_a[] = {
        #include "Emulation/CAM_TWIN_AC_ALLON/Golden/pso_a_o.dhex"
    };
    static const unsigned char golden_5_rrzo_a[] = {
        #include "Emulation/CAM_TWIN_AC_ALLON/Golden/rrzo_a_o.dhex"
    };
    static const unsigned char golden_5_rsso_a[] = {
        #include "Emulation/CAM_TWIN_AC_ALLON/Golden/rsso_a_o.dhex"
    };
    static const unsigned char golden_5_ufgo_a[] = {
        #include "Emulation/CAM_TWIN_AC_ALLON/Golden/ufgo_a_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_5_imgo_a,"IMGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_5_pso_a,"PSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_5_rrzo_a,"RRZO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_5_rsso_a,"RSSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_5_ufgo_a,"UFGO") == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();
    return ret;
}

MINT32 Pattern_BitTrue_6(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;


    static const unsigned char golden_6_aao_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/aao_a_o.dhex"
    };
    static const unsigned char golden_6_aao_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/aao_b_o.dhex"
    };

    static const unsigned char golden_6_afo_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/afo_a_o.dhex"
    };
    static const unsigned char golden_6_afo_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/afo_b_o.dhex"
    };
    static const unsigned char golden_6_afo_c[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/afo_c_o.dhex"
    };

    static const unsigned char golden_6_flko_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/flko_a_o.dhex"
    };
    static const unsigned char golden_6_flko_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/flko_b_o.dhex"
    };

    static const unsigned char golden_6_imgo_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/imgo_a_o.dhex"
    };
    static const unsigned char golden_6_imgo_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/imgo_b_o.dhex"
    };
    static const unsigned char golden_6_imgo_c[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/imgo_c_o.dhex"
    };

    static const unsigned char golden_6_lcso_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/lcso_a_o.dhex"
    };
    static const unsigned char golden_6_lcso_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/lcso_b_o.dhex"
    };

    static const unsigned char golden_6_lmvo_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/lmvo_a_o.dhex"
    };
    static const unsigned char golden_6_lmvo_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/lmvo_b_o.dhex"
    };

    static const unsigned char golden_6_pso_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/pso_a_o.dhex"
    };
    static const unsigned char golden_6_pso_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/pso_b_o.dhex"
    };

    static const unsigned char golden_6_rrzo_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/rrzo_a_o.dhex"
    };
    static const unsigned char golden_6_rrzo_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/rrzo_b_o.dhex"
    };
    static const unsigned char golden_6_rrzo_c[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/rrzo_c_o.dhex"
    };

    static const unsigned char golden_6_rsso_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/rsso_a_o.dhex"
    };
    static const unsigned char golden_6_rsso_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/rsso_b_o.dhex"
    };

    static const unsigned char golden_6_ufeo_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/ufeo_b_o.dhex"
    };

    static const unsigned char golden_6_ufgo_a[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/ufgo_a_o.dhex"
    };
    static const unsigned char golden_6_ufgo_b[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/ufgo_b_o.dhex"
    };
    static const unsigned char golden_6_ufgo_c[] = {
        #include "Emulation/CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER/Golden/ufgo_c_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,AAO_EN_,pBuf,golden_6_aao_a,"AAO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,AFO_EN_,pBuf,golden_6_afo_a,"AFO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_6_flko_a,"FLKO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_6_imgo_a,"IMGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCSO_EN_,pBuf,golden_6_lcso_a,"LCSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_6_lmvo_a,"LMVO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_6_pso_a,"PSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_6_rrzo_a,"RRZO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_6_rsso_a,"RSSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_6_ufgo_a,"UFGO") == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);


    if (Beyond_Compare(CAM_B,DMA_EN,AAO_EN_,pBuf,golden_6_aao_b,"AAO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,AFO_EN_,pBuf,golden_6_afo_b,"AFO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_EN_,pBuf,golden_6_flko_b,"FLKO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_6_imgo_b,"IMGO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCSO_EN_,pBuf,golden_6_lcso_b,"LCSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_6_lmvo_b,"LMVO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_6_pso_b,"PSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_6_rrzo_b,"RRZO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_EN_,pBuf,golden_6_rsso_b,"RSSO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_EN_,pBuf,golden_6_ufeo_b,"UFEO") == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_EN_,pBuf,golden_6_ufgo_b,"UFGO") == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_EN_,pBuf,golden_6_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_C,DMA_EN,IMGO_EN_,pBuf,golden_6_imgo_c,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_C,DMA_EN,UFGO_EN_,pBuf,golden_6_ufgo_c,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

    return ret;
}

MINT32 Pattern_BitTrue_13(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0;
    MUINT32 *pTable = NULL,*pMem = NULL;


    static const unsigned char golden_13_aao_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/aao_a_o.dhex"
    };

    static const unsigned char golden_13_afo_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/afo_a_o.dhex"
    };

    static const unsigned char golden_13_flko_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/flko_a_o.dhex"
    };

    static const unsigned char golden_13_imgo_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/imgo_a_o.dhex"
    };
    static const unsigned char golden_13_imgo_b[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/imgo_b_o.dhex"
    };
    static const unsigned char golden_13_imgo_c[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/imgo_c_o.dhex"
    };

    static const unsigned char golden_13_lcso_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/lcso_a_o.dhex"
    };

    static const unsigned char golden_13_lmvo_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/lmvo_a_o.dhex"
    };


    static const unsigned char golden_13_pso_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/pso_a_o.dhex"
    };

    static const unsigned char golden_13_rrzo_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/rrzo_a_o.dhex"
    };
    static const unsigned char golden_13_rrzo_b[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/rrzo_b_o.dhex"
    };
    static const unsigned char golden_13_rrzo_c[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/rrzo_c_o.dhex"
    };

    static const unsigned char golden_13_rsso_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/rsso_a_o.dhex"
    };

    static const unsigned char golden_13_ufgo_a[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/ufgo_a_o.dhex"
    };
    static const unsigned char golden_13_ufgo_b[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/ufgo_b_o.dhex"
    };
    static const unsigned char golden_13_ufgo_c[] = {
        #include "Emulation/CAM_RAW10_25M_3RAW_TEST/Golden/ufgo_c_o.dhex"
    };

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_A,DMA_EN,AAO_EN_,pBuf,golden_13_aao_a,"AAO",CAM_READ_BITS(ptr,CAM_AAO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,AFO_EN_,pBuf,golden_13_afo_a,"AFO",CAM_READ_BITS(ptr,CAM_AFO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_13_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_13_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCSO_EN_,pBuf,golden_13_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_13_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_13_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_13_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_13_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_13_ufgo_a,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_13_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_13_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_EN_,pBuf,golden_13_ufgo_b,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);


    if (Beyond_Compare(CAM_C,DMA_EN,IMGO_EN_,pBuf,golden_13_imgo_c,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_EN_,pBuf,golden_13_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_C,DMA_EN,UFGO_EN_,pBuf,golden_13_ufgo_c,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

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
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_14_0_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
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
    if (Beyond_Compare(CAM_A,DMA_EN,LCSO_EN_,pBuf,golden_14_0_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_14_0_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_14_0_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_14_0_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_14_0_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_EN_,pBuf,golden_14_0_ufeo_a,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_14_0_ufgo_a,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
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
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_EN_,pBuf,golden_14_0_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
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
    if (Beyond_Compare(CAM_B,DMA_EN,LCSO_EN_,pBuf,golden_14_0_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_14_0_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_14_0_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_14_0_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_EN_,pBuf,golden_14_0_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_EN_,pBuf,golden_14_0_ufeo_b,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_EN_,pBuf,golden_14_0_ufgo_b,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

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
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_14_1_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_14_1_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCSO_EN_,pBuf,golden_14_1_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_14_1_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_14_1_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_14_1_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_14_1_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_EN_,pBuf,golden_14_1_ufeo_a,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_14_1_ufgo_a,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
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
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_EN_,pBuf,golden_14_1_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_14_1_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCSO_EN_,pBuf,golden_14_1_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_14_1_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_14_1_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_14_1_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_EN_,pBuf,golden_14_1_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_EN_,pBuf,golden_14_1_ufeo_b,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_EN_,pBuf,golden_14_1_ufgo_b,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

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
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_EN_,pBuf,golden_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCSO_EN_,pBuf,golden_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_EN_,pBuf,golden_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
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
    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_EN_,pBuf,golden_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

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
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_EN_,pBuf,golden_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCSO_EN_,pBuf,golden_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_EN_,pBuf,golden_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
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
    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_EN_,pBuf,golden_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

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
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_15_0_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_15_0_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCSO_EN_,pBuf,golden_15_0_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_15_0_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_15_0_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_15_0_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_15_0_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
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
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_EN_,pBuf,golden_15_0_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_15_0_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCSO_EN_,pBuf,golden_15_0_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_15_0_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_15_0_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_15_0_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_EN_,pBuf,golden_15_0_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_EN_,pBuf,golden_15_0_ufeo_b,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_EN_,pBuf,golden_15_0_ufgo_b,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_C,DMA_EN,IMGO_EN_,pBuf,golden_15_0_imgo_c,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_EN_,pBuf,golden_15_0_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

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
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_15_1_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_15_1_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCSO_EN_,pBuf,golden_15_1_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_15_1_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_15_1_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_15_1_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_15_1_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_EN_,pBuf,golden_15_1_ufeo_a,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_15_1_ufgo_a,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
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
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_EN_,pBuf,golden_15_1_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_15_1_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCSO_EN_,pBuf,golden_15_1_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_15_1_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_15_1_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_15_1_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_EN_,pBuf,golden_15_1_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFEO_EN_,pBuf,golden_15_1_ufeo_b,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,UFGO_EN_,pBuf,golden_15_1_ufgo_b,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

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
    if (Beyond_Compare(CAM_A,DMA_EN,FLKO_EN_,pBuf,golden_15_2_flko_a,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,IMGO_EN_,pBuf,golden_15_2_imgo_a,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,LCSO_EN_,pBuf,golden_15_2_lcso_a,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,EISO_EN_,pBuf,golden_15_2_lmvo_a,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,PSO_EN_,pBuf,golden_15_2_pso_a,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RRZO_EN_,pBuf,golden_15_2_rrzo_a,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,RSSO_EN_,pBuf,golden_15_2_rsso_a,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFEO_EN_,pBuf,golden_15_2_ufeo_a,"UFEO",CAM_READ_BITS(ptr,CAM_UFEO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_A,DMA_EN,UFGO_EN_,pBuf,golden_15_2_ufgo_a,"UFGO",CAM_READ_BITS(ptr,CAM_UFGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
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
    if (Beyond_Compare(CAM_B,DMA_EN,FLKO_EN_,pBuf,golden_15_2_flko_b,"FLKO",CAM_READ_BITS(ptr,CAM_FLKO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,IMGO_EN_,pBuf,golden_15_2_imgo_b,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,LCSO_EN_,pBuf,golden_15_2_lcso_b,"LCSO",CAM_READ_BITS(ptr,CAM_LCSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,EISO_EN_,pBuf,golden_15_2_lmvo_b,"LMVO",CAM_READ_BITS(ptr,CAM_LMVO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,PSO_EN_,pBuf,golden_15_2_pso_b,"PSO",CAM_READ_BITS(ptr,CAM_PSO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RRZO_EN_,pBuf,golden_15_2_rrzo_b,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }
    if (Beyond_Compare(CAM_B,DMA_EN,RSSO_EN_,pBuf,golden_15_2_rsso_b,"RSSO",CAM_READ_BITS(ptr,CAM_RSSO_A_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_C]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    if (Beyond_Compare(CAM_C,DMA_EN,IMGO_EN_,pBuf,golden_15_2_imgo_c,"IMGO",CAM_READ_BITS(ptr,CAM_IMGO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    if (Beyond_Compare(CAM_C,DMA_EN,RRZO_EN_,pBuf,golden_15_2_rrzo_c,"RRZO",CAM_READ_BITS(ptr,CAM_RRZO_OFST_ADDR,OFFSET_ADDR)) == MFALSE) {
        ret++;
    }

    LOG_INF("press any key continuous\n");
    getchar();

    return ret;
}

MINT32 Pattern_freeBuf(MUINT32* _ptr,MUINTPTR inputInfo,MUINT32* _uni,MUINTPTR linkpath)
{
    IMEM_BUF_INFO**** pBuf;
    IMemDrv* pImemDrv = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;

    pImemDrv = IMemDrv::createInstance();
    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    for(MUINT32 j=CAM_A;j<(MAX_ISP_HW_MODULE+1);j++){
        for(MUINT32 i=0;i<32; i++){
            for(MUINT32 n=0;n<2;n++){
                if(pBuf[j][i][n]->size != 0)
                    pImemDrv->freeVirtBuf(pBuf[j][i][n]);
            }
        }
    }
    if (pInputInfo->rawiImemBuf.size) {
        pImemDrv->freeVirtBuf(&pInputInfo->rawiImemBuf);
    }

    pImemDrv->uninit();
    pImemDrv->destroyInstance();

    return 0;
}

MINT32 Pattern_release(MUINT32* _ptr,MUINTPTR inputInfo,MUINT32* _uni,MUINTPTR linkpath)
{
    IMEM_BUF_INFO**** pBuf;
    IMemDrv* pImemDrv = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;

    pImemDrv = IMemDrv::createInstance();
    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;
    for(MUINT32 j=CAM_A;j<(MAX_ISP_HW_MODULE+1);j++){
        for(MUINT32 i=0;i<32; i++){
            for(MUINT32 n=0;n<2;n++){
                if(pBuf[j][i][n]->size != 0)
                    pImemDrv->freeVirtBuf(pBuf[j][i][n]);
            }
        }
    }
    if (pInputInfo->rawiImemBuf.size) {
        pImemDrv->freeVirtBuf(&pInputInfo->rawiImemBuf);
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

    if(_uni){
        if(linkpath == CAM_A)
            ((UniDrvImp*)_uni)->uninit("Test_IspDrvCam_A");
        else if (linkpath == CAM_B)
            ((UniDrvImp*)_uni)->uninit("Test_IspDrvCam_B");
        else if (linkpath == CAM_C)
            ((UniDrvImp*)_uni)->uninit("Test_IspDrvCam_C");

        ((UniDrvImp*)_uni)->destroyInstance();
    }

    return 0;
}




#define CASE_OP     7   // 5 is for 1: isp drv init|fakeSensor,2:load MMU setting, 3:loading pattern(APMCU or CQ loading). 4:mem allocate + start, 5:stop, 6:deallocate

typedef MINT32 (*LDVT_CB)(MUINT32*,MUINTPTR,MUINT32*,MUINTPTR);

static enum CASE_OP_LIST
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

LDVT_CB CB_TBL[][CASE_OP] = {
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_0,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_1,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_1,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_2,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_2,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_3,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_3,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_4,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_4,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_5,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_5,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_6,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_6,Pattern_release},
    [13] = {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_13,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_13,Pattern_release},
    [30] = {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_30,Pattern_Start_1,Pattern_Stop_1,NULL,Pattern_release}
};

LDVT_CB MULTIFRAME_CB_TBL_CASE14[][CASE_OP] = {
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_14_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_14_0,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_14_1,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_14_1,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_14_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_14_0,Pattern_release},
};

LDVT_CB MULTIFRAME_CASE15_CB_TBL[][CASE_OP] = {
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_15_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15_0,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_15_1,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15_1,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_15_2,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15_2,Pattern_freeBuf},
    {NULL         ,NULL    ,Pattern_Loading_15_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15_0,Pattern_release},
};

LDVT_CB CB_TC15[][CASE_OP] = {
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_15,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15,Pattern_freeBuf},
    {         NULL,    NULL,Pattern_Loading_16,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_16,Pattern_freeBuf},
    {         NULL,    NULL,Pattern_Loading_15,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_15,Pattern_release},
};

int IspDrvCam_LDVT(void)
{
    int ret = 0;
    char s[32];
    MUINT32 test_case, _raw_num;
    ISP_DRV_CAM* pDrvCam = NULL;
    ISP_DRV_CAM* pDrvCam2 = NULL;
    ISP_DRV_CAM* ptr = NULL;
    IMEM_BUF_INFO ****pimgBuf = NULL;
    ISP_DRV_CAM*    pIspDrv[CAM_MAX] = {NULL,NULL};
    UniDrvImp*      UniDrv;
    TestInputInfo   testInput;

    if(pimgBuf == NULL){
        pimgBuf = (IMEM_BUF_INFO****)malloc(sizeof(IMEM_BUF_INFO***)*(MAX_ISP_HW_MODULE+1));
        for(MUINT32 i=0;i<(MAX_ISP_HW_MODULE+1);i++){
            pimgBuf[i] = (IMEM_BUF_INFO***)malloc(sizeof(IMEM_BUF_INFO**)*32);
            for(MUINT32 j=0;j<32;j++){
                pimgBuf[i][j] = (IMEM_BUF_INFO**)malloc(sizeof(IMEM_BUF_INFO*)*2);
                for(MUINT32 k=0;k<2;k++){
                    pimgBuf[i][j][k] = new IMEM_BUF_INFO();//calls default constructor
                }
            }
        }
    }
    testInput.pImemBufs= pimgBuf;

    LOG_INF("##############################\n");
    LOG_INF("case 0: P1_CAM_SINGLE\n");
    LOG_INF("case 1: P1_CAM_TWIN_ABC_ALLON\n");
    LOG_INF("case 2: P1 MULTI\n");
    LOG_INF("case 3: P1_CAM_TWIN_AB_ALLON\n");
    LOG_INF("case 4: P1 BIN\n");
    LOG_INF("case 5: P1_CAM_TWIN_AC_ALLON\n");
    LOG_INF("case 6: P1_CAM_RAW10_20M_16M_SIML_CQ_FRAME_HEADER\n");
    LOG_INF("case 13: P1_CAM_RAW10_25M_3RAW_TEST\n");
    LOG_INF("case 14: CAM_MULTI_SIMUL_BINNING\n");
    LOG_INF("case 15: CAM_MULTI_DYNAMIC_TWIN_BINNING\n");
    LOG_INF("case 30: SINGLE TM\n");
    LOG_INF("case 31: P1_CAM_MULTI_TWIN_BC_BINNING\n");
    LOG_INF("##############################\n");
    scanf("%s", s);
    test_case = atoi(s);
    getchar();

    LOG_INF("start case:%d\n",test_case);
    switch(test_case){
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 13:
        case 30:
            if((ret = CB_TBL[test_case][LDVT_DRV_INIT_OP]((MUINT32*)pIspDrv,3,(MUINT32*)&UniDrv,CAM_A)) != 0){
                LOG_ERR(" case_%d step_0 fail\n",test_case);
                return 1;
            }

            if((ret = CB_TBL[test_case][MMU_ININ_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,CAM_A)) != 0){
                LOG_ERR(" case_%d step_1 fail\n",test_case);
                return 1;
            }

            if((ret = CB_TBL[test_case][PATTENR_LOADING_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv, (MUINTPTR)&testInput)) != 0){
                LOG_ERR(" case_%d step_2 fail\n",test_case);
                return 1;
            }
            switch (test_case) {
            case 0:
                testInput.tgNum = 0;
                _raw_num = 1;
                break;
            case 2: //simul
                testInput.tgNum = 2;
                _raw_num = 2;
                break;
            case 3: //twin 2
            case 5:
                testInput.tgNum = 1;
                _raw_num = 2;
                break;
            case 1: //twin 3
                testInput.tgNum = 1;
                _raw_num = 3;
                break;
            case 6:
                testInput.tgNum = 2;
                _raw_num = 3;
                break;
            case 13:
                testInput.tgNum = 1;
                _raw_num = 3;
                break;
            default:
                LOG_ERR("Unknown test case : %d\n", test_case);
                goto _EMU_EXIT;
            }
            LOG_INF("tg_num = %d, raw_num=%d\n", testInput.tgNum, _raw_num);

            if ((ret = CB_TBL[test_case][PATTERN_START_OP]((MUINT32*)pIspDrv,_raw_num,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                LOG_ERR(" case_%d step_3 fail\n",test_case);
                return 1;
            }

            if((ret = CB_TBL[test_case][PATTERN_STOP_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                LOG_ERR(" case_%d step_4 fail\n",test_case);
                return 1;
            }

            if(CB_TBL[test_case][PATTERN_BITTRUE_OP] != NULL){
                if((ret = CB_TBL[test_case][PATTERN_BITTRUE_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_5 fail\n",test_case);
                }
            }
            else{
                LOG_INF("######################################\n");
                LOG_INF("this case_%d support no bit-true flow\n");
                LOG_INF("######################################\n");
            }

            if((ret = CB_TBL[test_case][PATTERN_RELEASE_OP]((MUINT32*)pIspDrv,(MUINTPTR)&testInput,(MUINT32*)UniDrv,CAM_A)) != 0){
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
                    if((ret = MULTIFRAME_CB_TBL_CASE14[frame][LDVT_DRV_INIT_OP]((MUINT32*)pIspDrv,3,(MUINT32*)&UniDrv, CAM_A)) != 0){
                        LOG_ERR(" case_%d step_0 fail\n",test_case);
                    }
                }else{
                    LOG_INF(" MULTIFRAME_CB_TBL_CASE14[%d][LDVT_DRV_INIT_OP] = NULL\n",frame);
                }

                if(MULTIFRAME_CB_TBL_CASE14[frame][MMU_ININ_OP] != NULL){
                    if((ret = MULTIFRAME_CB_TBL_CASE14[frame][MMU_ININ_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv, CAM_A)) != 0){
                        LOG_ERR(" case_%d step_1 fail\n",test_case);
                    }
                }else{
                    LOG_INF(" MULTIFRAME_CB_TBL_CASE14[%d][MMU_ININ_OP] = NULL\n",frame);
                }

                if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTENR_LOADING_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv, (MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_2 fail\n",test_case);
                    return 1;
                }

                if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_START_OP]((MUINT32*)pIspDrv,_raw_num,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_3 fail\n",test_case);
                    return 1;
                }

                if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_STOP_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_4 fail\n",test_case);
                    return 1;
                }

                if(MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_BITTRUE_OP] != NULL){
                    if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_BITTRUE_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                        LOG_ERR(" case_%d step_5 fail\n",test_case);
                    }
                }
                else{
                    LOG_INF("######################################\n");
                    LOG_INF("this case_%d support no bit-true flow\n");
                    LOG_INF("######################################\n");
                }

                if((ret = MULTIFRAME_CB_TBL_CASE14[frame][PATTERN_RELEASE_OP]((MUINT32*)pIspDrv,(MUINTPTR)&testInput,(MUINT32*)UniDrv,CAM_A)) != 0){
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
                    if((ret = MULTIFRAME_CASE15_CB_TBL[frame][LDVT_DRV_INIT_OP]((MUINT32*)pIspDrv,3,(MUINT32*)&UniDrv, CAM_A)) != 0){
                        LOG_ERR(" case_%d step_0 fail\n",test_case);
                    }
                }else{
                    LOG_INF(" MULTIFRAME_CASE15_CB_TBL[%d][LDVT_DRV_INIT_OP] = NULL\n",frame);
                }

                if(MULTIFRAME_CASE15_CB_TBL[frame][MMU_ININ_OP] != NULL){
                    if((ret = MULTIFRAME_CASE15_CB_TBL[frame][MMU_ININ_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv, CAM_A)) != 0){
                        LOG_ERR(" case_%d step_1 fail\n",test_case);
                    }
                }else{
                    LOG_INF(" MULTIFRAME_CASE15_CB_TBL[%d][MMU_ININ_OP] = NULL\n",frame);
                }

                if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTENR_LOADING_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv, (MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_2 fail\n",test_case);
                    return 1;
                }

                if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_START_OP]((MUINT32*)pIspDrv,_raw_num,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_3 fail\n",test_case);
                    return 1;
                }

                if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_STOP_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_4 fail\n",test_case);
                    return 1;
                }

                if(MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_BITTRUE_OP] != NULL){
                    if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_BITTRUE_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                        LOG_ERR(" case_%d step_5 fail\n",test_case);
                    }
                }
                else{
                    LOG_INF("######################################\n");
                    LOG_INF("this case_%d support no bit-true flow\n");
                    LOG_INF("######################################\n");
                }

                if((ret = MULTIFRAME_CASE15_CB_TBL[frame][PATTERN_RELEASE_OP]((MUINT32*)pIspDrv,(MUINTPTR)&testInput,(MUINT32*)UniDrv,CAM_A)) != 0){
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
                    if((ret = CB_TC15[f][LDVT_DRV_INIT_OP]((MUINT32*)pIspDrv,3,(MUINT32*)&UniDrv,CAM_A)) != 0){
                        LOG_ERR(" case_%d step_0 fail\n",test_case);
                        return 1;
                    }

                if(CB_TC15[f][MMU_ININ_OP] != NULL)
                    if((ret = CB_TC15[f][MMU_ININ_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,CAM_A)) != 0){
                        LOG_ERR(" case_%d step_1 fail\n",test_case);
                        return 1;
                    }

                if((ret = CB_TC15[f][PATTENR_LOADING_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv, (MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_2 fail\n",test_case);
                    return 1;
                }
                if ((ret = CB_TC15[f][PATTERN_START_OP]((MUINT32*)pIspDrv,_raw_num,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_3 fail\n",test_case);
                    return 1;
                }

                if((ret = CB_TC15[f][PATTERN_STOP_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_4 fail\n",test_case);
                    return 1;
                }

                if(CB_TC15[f][PATTERN_BITTRUE_OP] != NULL){
                    if((ret = CB_TC15[f][PATTERN_BITTRUE_OP]((MUINT32*)pIspDrv,3,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                        LOG_ERR(" case_%d step_5 frame_%d fail\n",test_case, f);
                    }
                } else {
                    LOG_INF("######################################\n");
                    LOG_INF("this case_%d support no bit-true flow\n");
                    LOG_INF("######################################\n");
                }

                if((ret = CB_TC15[f][PATTERN_RELEASE_OP]((MUINT32*)pIspDrv,(MUINTPTR)&testInput,(MUINT32*)UniDrv,CAM_A)) != 0){
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
            for(MUINT32 k=0;k<2;k++)
                delete pimgBuf[i][j][k];
            free(pimgBuf[i][j]);
        }
        free(pimgBuf[i]);
    }
    pimgBuf = NULL;
    return ret;
}
