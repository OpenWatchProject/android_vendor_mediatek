#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>


#include "isp_drv_cam.h"
#include "isp_drv_dip.h"
#include "isp_drv_dip_phy.h"
#include "uni_drv.h"
#include "isp_drv_camsv.h"
#include "FakeDrvSensor.h"
#include <mtkcam/drv/IHalSensor.h>
#include <cutils/properties.h>  // For property_get().

#undef LOG_TAG
#define LOG_TAG "Test_IspDrv"


#define LOG_VRB(fmt, arg...)        printf("[%s][#%d] " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define LOG_DBG(fmt, arg...)        printf("[%s][%d] " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define LOG_INF(fmt, arg...)        printf("[%s][%d] " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define LOG_WRN(fmt, arg...)        printf("[%s][%d] " fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)        printf("[%s][%d] error:" fmt "\n", __FUNCTION__, __LINE__, ##arg)

#define LPDVT_USING_SENINF      (0)
#define LPDVT_USING_CAMSV       (0)

#define UNUSED(expr) do { (void)(expr); } while (0)

enum {
    _DMAI_TBL_LSCI = 0,
    _DMAI_TBL_BPCI,
    _DMAI_TBL_PDI,
    _DMAI_TBL_NUM,
} _DMAI_TBL_INDEX;

struct TestInputInfo {
    struct DmaiTableInfo {
        const unsigned char     *pTblAddr;
        MUINT32                 tblLength;
    };

    IMEM_BUF_INFO           ****pImemBufs;
    DmaiTableInfo           DmaiTbls[CAM_MAX][_DMAI_TBL_NUM];

    TestInputInfo()
        {
            int i = 0, cam = 0;

            pImemBufs = NULL;
            for (cam = 0; cam < CAM_MAX; cam++) {
                for (i = 0; i < _DMAI_TBL_NUM; i++) {
                    DmaiTbls[cam][i].pTblAddr = NULL;
                    DmaiTbls[cam][i].tblLength = 0;
                }
            }
        }
};


#if LPDVT_USING_SENINF == 0
NSCam::IHalSensorList* pHalSensorList = NULL;
NSCam::SensorStaticInfo mSensorInfo[1];
NSCam::IHalSensor* pSensorHalObj[1] = {NULL};
#endif

#define PDO_END_TOKEN (0x4000)

static MUINT32 lpdvt_pdn=0;

MBOOL WaitVsync(IspDrv* ptr)
{
    MUINT32 count =0;

    while(1)
    {
        usleep(1000);

        if (CAM_READ_REG(ptr, CAM_CTL_RAW_INT_STATUS) & 0x1)
            break;

        if (++count >= 200)
            return MTRUE;
    }

    return MFALSE;
}

MBOOL WaitP1Done(IspDrv* ptr)
{
    MUINT32 count =0;

    while(1)
    {
        if (CAM_READ_REG(ptr, CAM_CTL_RAW_INT_STATUS) & 0x0800)
            break;

        if(++count > 100)
            return MTRUE;

        usleep(1000);
    }

    return MFALSE;
}

MINT32 LDVT_DRV_INIT(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR linkpath)
{
    ISP_DRV_CAM** ptr = (ISP_DRV_CAM**)_ptr;
    switch(length-1){
        case CAM_B:// no break if length >= 2
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
        case CAM_A:
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
        case CAM_MAX:
            *(UniDrvImp**)_uni = NULL;
            LOG_INF("uni is no needed\n");
            break;
    }


    for (int i = 0; i < (int)length; i++) {
        LOG_INF("Reseting CAM_%c", 'A'+i);
        if (ptr[i]) {
            ptr[i]->setDeviceInfo(_SET_RESET_HW_MOD, NULL);
        }
    }

    LOG_INF("CAMA clear ctl/dma enable reg\n");
    CAM_WRITE_REG((ptr[CAM_A]->getPhyObj()), CAM_CTL_EN, 0);
    CAM_WRITE_REG((ptr[CAM_A]->getPhyObj()), CAM_CTL_DMA_EN, 0);
    if (length > 1) {
        LOG_INF("CAMB clear ctl/dma enable reg\n");
        CAM_WRITE_REG((ptr[CAM_B]->getPhyObj()), CAM_CTL_EN, 0);
        CAM_WRITE_REG((ptr[CAM_B]->getPhyObj()), CAM_CTL_DMA_EN, 0);
    }

    return 0;
}

MINT32 MMU_INIT(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR linkpath)
{
    ISP_DRV_CAM** ptr = (ISP_DRV_CAM**)_ptr;
    _uni; linkpath;

    for (int i = 0; i < (int)length; i++) {
        if (0) { //(ptr[i]) {
            ISP_LARB_MMU_STRUCT larbInfo;
            int idx = 0;

            //LOG_INF("Config MMU Larb to PA at CAM_%c .............", 'A'+i);

            larbInfo.regVal = 0;

            larbInfo.LarbNum = 2;
            //LOG_INF("config larb=%d offset=0x%x", larbInfo.LarbNum, larbInfo.regOffset);
            for (idx = 0; idx < 3; idx++) {
                larbInfo.regOffset = 0x380 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

                larbInfo.regOffset = 0xf80 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
            }

            larbInfo.LarbNum = 3;
            for (idx = 0; idx < 5; idx++) {
                larbInfo.regOffset = 0x380 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

                larbInfo.regOffset = 0xf80 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
            }

            larbInfo.LarbNum = 5;
            //LOG_INF("config larb=%d offset=0x%x", larbInfo.LarbNum, larbInfo.regOffset);
            for (idx = 0; idx < 12; idx++) {
                larbInfo.regOffset = 0x380 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

                larbInfo.regOffset = 0xf80 + (idx << 2);
                ptr[i]->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
            }

            larbInfo.LarbNum = 6;
            //LOG_INF("config larb=%d offset=0x%x", larbInfo.LarbNum, larbInfo.regOffset);
            for (idx = 0; idx < 19; idx++) {
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
        LOG_INF("%s PA:0x%x\n",#STR,(unsigned int)pBuf[Module][_cnt][1]->phyAddr);       \
        STR2##_WRITE_REG(ptr,CAM_##STR##_FH_BASE_ADDR,pBuf[Module][_cnt][1]->phyAddr);


    //aao/pso/bpci/flko's stride is useless. need to use xsize to cal. total memory size
    /* Disable force change the stride of IMGO/RRZO to (xsize+1)
        MUINT32 __tmp = (STR2##_READ_BITS(ptr,CAM_##STR##_XSIZE,XSIZE) + 1 );
        STR2##_WRITE_BITS(ptr,CAM_##STR##_STRIDE,STRIDE,__tmp);
    */
    #define MEM_ALLOC(Module,STR,STR2,STR3) \
        MUINT32 tmp = (DMA_EN & STR3##_EN_), _dma_en = (DMA_EN & STR3##_EN_);   \
        MUINT32 cnt = 0;        \
        while(tmp != 0){        \
            cnt++;              \
            tmp >>= 1;          \
        }                       \
        MBOOL bUNI = ((Module == UNI_A) || (Module == UNI_B));\
        if( (!bUNI) && ((_dma_en & IMGO_EN_)|| (_dma_en & RRZO_EN_))){\
            pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * STR2##_READ_BITS(ptr,CAM_##STR##_STRIDE,STRIDE);   \
        }\
        else{\
            if((!bUNI) && ((_dma_en & BPCI_EN_)||(_dma_en & PSO_EN_)||(_dma_en & AAO_EN_))){\
                pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * STR2##_READ_BITS(ptr,CAM_##STR##_XSIZE,XSIZE);   \
            }\
            else if(bUNI && (_dma_en & FLKO_A_EN_)){\
                pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * STR2##_READ_BITS(ptr,CAM_##STR##_XSIZE,XSIZE);   \
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
        if((_dma_en & IMGO_EN_)|| (_dma_en & RRZO_EN_)){\
        }\
        else\
            memset((MUINT8*)pBuf[Module][cnt][0]->virtAddr,0x0,pBuf[Module][cnt][0]->size);\
        LOG_INF("%s PA:0x%x VA:0x%x, cnt=%d, Module=%d\n",#STR,(unsigned int)pBuf[Module][cnt][0]->phyAddr, (unsigned int)pBuf[Module][cnt][0]->virtAddr, cnt, Module);       \
        STR2##_WRITE_REG(ptr,CAM_##STR##_BASE_ADDR,pBuf[Module][cnt][0]->phyAddr);      \


    pImemDrv = IMemDrv::createInstance();
    if(pImemDrv->init() < 0){
        LOG_ERR(" imem init fail\n");
        return 1;
    }


    for(MUINT32 i=CAM_A;i<=CAM_B;i++){
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
            MEM_ALLOC(i,AAO,CAM,AAO);
            HEADER_MEM_ALLOC(i,AAO,CAM,AAO);
        }
        if(DMA_EN & PSO_EN_){
            MEM_ALLOC(i,PSO,CAM,PSO);
            HEADER_MEM_ALLOC(i,PSO,CAM,PSO);
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

                LOG_INF("BPCI srcTable=%p size=%d pa=0x%lx, va=%p\n",
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
                if(getchar() < 0)
                    LOG_ERR("getchar error");
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
        if(DMA_EN & PDI_EN_){
            if (pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr && pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength) {
                MUINT32 tmp = (DMA_EN & PDI_EN_), cnt = 0;
                while (tmp != 0) {
                    cnt++;
                    tmp >>= 1;
                }

                pBuf[i][cnt][0]->size = pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength;

                if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem alloc fail at %s\n", "LSCI");
                    return 1;
                }
                if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem map fail at %s\n", "LSCI");
                    return 1;
                }

                CAM_WRITE_REG(ptr,CAM_PDI_BASE_ADDR,pBuf[i][cnt][0]->phyAddr);

                LOG_INF("PDI srcTable=%p size=%d pa=0x%lx, va=%p\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength,
                    pBuf[i][cnt][0]->phyAddr, (MUINT32 *)pBuf[i][cnt][0]->virtAddr);
                LOG_INF("PDI xsize=0x%x ysize=0x%x stride=0x%x\n",
                    CAM_READ_BITS(ptr,CAM_PDI_XSIZE,XSIZE), CAM_READ_BITS(ptr,CAM_PDI_YSIZE,YSIZE),
                    CAM_READ_BITS(ptr,CAM_PDI_STRIDE,STRIDE));
                LOG_INF("PDC_EN:%d,  PDC_OUT:%d\n",CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN),CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_OUT));

                memcpy((MUINT8*)pBuf[i][cnt][0]->virtAddr, (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength);
            }
            else {
                LOG_ERR("PDI table not found va(%p) size(%d)!!\n", pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength);
                LOG_ERR("press to continue...\n");
                if(getchar() < 0)
                    LOG_ERR("getchar error");
            }
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

                LOG_INF("LSCI srcTable=%p size=%d pa=0x%lx, va=%p\n",
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
                if(getchar() < 0){
                    LOG_ERR("getchar error");
                }
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
        if(DMA_EN & PDO_EN_){
            MEM_ALLOC(i,PDO,CAM,PDO);
            HEADER_MEM_ALLOC(i,PDO,CAM,PDO);
        }
    }


    ptr = (UniDrvImp*)_uni;
    DMA_EN = UNI_READ_REG(ptr, CAM_UNI_TOP_DMA_EN);
    LOG_INF("module_UNI_A:  enabled DMA:0x%x\n",DMA_EN);
    if(DMA_EN & FLKO_A_EN_){
        MEM_ALLOC(UNI_A,UNI_FLKO,UNI,FLKO_A);
        HEADER_MEM_ALLOC(UNI_A,UNI_FLKO,UNI,FLKO_A);
    }
    if(DMA_EN & EISO_A_EN_){
        MEM_ALLOC(UNI_A,UNI_EISO,UNI,EISO_A);
        HEADER_MEM_ALLOC(UNI_A,UNI_EISO,UNI,EISO_A);
    }
    if(DMA_EN & RSSO_A_EN_){
        MEM_ALLOC(UNI_A,UNI_RSSO_A,UNI,RSSO_A);
        HEADER_MEM_ALLOC(UNI_A,UNI_RSSO_A,UNI,RSSO_A);
    }
    DMA_EN = UNI_READ_REG(ptr, CAM_UNI_B_TOP_DMA_EN);
    LOG_INF("module_UNI_B:  enabled DMA:0x%x\n",DMA_EN);
    if(DMA_EN & FLKO_A_EN_){
        MEM_ALLOC(UNI_B,UNI_B_FLKO,UNI,FLKO_A);
        HEADER_MEM_ALLOC(UNI_B,UNI_B_FLKO,UNI,FLKO_A);
    }
    if(DMA_EN & EISO_A_EN_){
        MEM_ALLOC(UNI_B,UNI_B_EISO,UNI,EISO_A);
        HEADER_MEM_ALLOC(UNI_B,UNI_B_EISO,UNI,EISO_A);
    }
    if(DMA_EN & RSSO_A_EN_){
        MEM_ALLOC(UNI_B,UNI_B_RSSO_A,UNI,RSSO_A);
        HEADER_MEM_ALLOC(UNI_B,UNI_B_RSSO_A,UNI,RSSO_A);
    }

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    CAM_WRITE_BITS(ptr,CAM_TG_VF_CON,SINGLE_MODE,0);
    CAM_WRITE_BITS(ptr,CAM_TG_VF_CON,VFDATA_EN,1);
    if (length > 1) {
        ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
        CAM_WRITE_BITS(ptr,CAM_TG_VF_CON,SINGLE_MODE,0);
        CAM_WRITE_BITS(ptr,CAM_TG_VF_CON,VFDATA_EN,1);
    }


    //enlarge test pattern if pattern size == TG size
    //cam will be fail when pattern size == TG size under continuous mode
    {
        CAM_REG_TG_SEN_GRAB_LIN tg;
        ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
        tg.Raw = CAM_READ_REG(ptr,CAM_TG_SEN_GRAB_LIN);
        TS_FakeDrvSensor().adjustPatternSize(tg.Bits.LIN_E - tg.Bits.LIN_S);
    }
    return 0;
}

#define HEADER_MEM_ALLOC6(Module,STR,STR2,STR3) \
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
    LOG_INF("%s PA:0x%x\n",#STR,(unsigned int)pBuf[Module][_cnt][1]->phyAddr);       \
    STR2##_WRITE_REG(ptr,CAM_##STR##_FH_BASE_ADDR,pBuf[Module][_cnt][1]->phyAddr);

//aao/pso/bpci's stride is useless. need to use xsize to cal. total memory size
#define MEM_ALLOC6(Module,STR,STR2,STR3) \
    MUINT32 tmp = (DMA_EN & STR3##_EN_), _dma_en = (DMA_EN & STR3##_EN_);   \
    MUINT32 cnt = 0;        \
    while(tmp != 0){        \
        cnt++;              \
        tmp >>= 1;          \
    }                       \
    MBOOL bUNI = ((Module == UNI_A) || (Module == UNI_B));\
    if( (!bUNI) && ((_dma_en & IMGO_EN_)|| (_dma_en & RRZO_EN_))){\
        pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * STR2##_READ_BITS(ptr,CAM_##STR##_STRIDE,STRIDE);   \
    }\
    else{\
        if((!bUNI) && ((_dma_en & BPCI_EN_)||(_dma_en & PSO_EN_)||(_dma_en & AAO_EN_))){\
            pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * STR2##_READ_BITS(ptr,CAM_##STR##_XSIZE,XSIZE);   \
        }\
        else if(bUNI && (_dma_en & FLKO_A_EN_)){\
            pBuf[Module][cnt][0]->size = (STR2##_READ_BITS(ptr,CAM_##STR##_YSIZE,YSIZE)+1) * STR2##_READ_BITS(ptr,CAM_##STR##_XSIZE,XSIZE);   \
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
    LOG_INF("dmao %s buf size:%d P:0x%x V:0x%lx\n", #STR, pBuf[Module][cnt][0]->size, \
        (unsigned int)pBuf[Module][cnt][0]->phyAddr, pBuf[Module][cnt][0]->virtAddr); \
    STR2##_WRITE_REG(ptr,CAM_##STR##_BASE_ADDR,pBuf[Module][cnt][0]->phyAddr);      \

MINT32 Pattern_Start_6(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    UINT32 DMA_EN = 0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf;
    IMemDrv* pImemDrv = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UNUSED(length);

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;

    pImemDrv = IMemDrv::createInstance();
    if(pImemDrv->init() < 0){
        LOG_ERR(" imem init fail\n");
        return 1;
    }


    for(MUINT32 i=CAM_A;i<=CAM_B;i++){
        ptr = ((ISP_DRV_CAM**)_ptr)[i]->getPhyObj();
        DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);
        LOG_INF("module_%d:  enabled DMA:0x%x\n",i,DMA_EN);
        if(DMA_EN & IMGO_EN_){
            MEM_ALLOC6(i,IMGO,CAM,IMGO);
            HEADER_MEM_ALLOC6(i,IMGO,CAM,IMGO);
        }
        if(DMA_EN & UFEO_EN_){
            MUINT32 _orgxsize = CAM_READ_BITS(ptr,CAM_UFEO_XSIZE,XSIZE);
            MUINT32 _orgstride =  CAM_READ_BITS(ptr,CAM_UFEO_STRIDE,STRIDE);
            MEM_ALLOC6(i,UFEO,CAM,UFEO);
            HEADER_MEM_ALLOC6(i,UFEO,CAM,UFEO);
        }
        if(DMA_EN & RRZO_EN_){
            MEM_ALLOC6(i,RRZO,CAM,RRZO);
            HEADER_MEM_ALLOC6(i,RRZO,CAM,RRZO);
        }
        if(DMA_EN & AFO_EN_){
            MEM_ALLOC6(i,AFO,CAM,AFO);
            HEADER_MEM_ALLOC6(i,AFO,CAM,AFO);
        }
        if(DMA_EN & LCSO_EN_){
            MEM_ALLOC6(i,LCSO,CAM,LCSO);
            HEADER_MEM_ALLOC6(i,LCSO,CAM,LCSO);
        }
        if(DMA_EN & AAO_EN_){
            MEM_ALLOC6(i,AAO,CAM,AAO);
            HEADER_MEM_ALLOC6(i,AAO,CAM,AAO);
        }
        if(DMA_EN & PSO_EN_){
            MEM_ALLOC6(i,PSO,CAM,PSO);
            HEADER_MEM_ALLOC6(i,PSO,CAM,PSO);
        }
        if(DMA_EN & BPCI_EN_){
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

                LOG_INF("BPCI srcTable=%p size=%d pa=0x%lx, va=%p\n",
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
                if(getchar() < 0)
                    LOG_ERR("getchar error");
            }

        }
        if(DMA_EN & PDI_EN_){
            if (pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr && pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength) {
                MUINT32 tmp = (DMA_EN & PDI_EN_), cnt = 0;
                while (tmp != 0) {
                    cnt++;
                    tmp >>= 1;
                }

                pBuf[i][cnt][0]->size = pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength;

                if (pImemDrv->allocVirtBuf(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem alloc fail at %s\n", "LSCI");
                    return 1;
                }
                if (pImemDrv->mapPhyAddr(pBuf[i][cnt][0]) < 0) {
                    LOG_ERR(" imem map fail at %s\n", "LSCI");
                    return 1;
                }

                CAM_WRITE_REG(ptr,CAM_PDI_BASE_ADDR,pBuf[i][cnt][0]->phyAddr);

                LOG_INF("PDI srcTable=%p size=%d pa=0x%lx, va=%p\n",
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength,
                    pBuf[i][cnt][0]->phyAddr, (MUINT32 *)pBuf[i][cnt][0]->virtAddr);
                LOG_INF("PDI xsize=0x%x ysize=0x%x stride=0x%x\n",
                    CAM_READ_BITS(ptr,CAM_PDI_XSIZE,XSIZE), CAM_READ_BITS(ptr,CAM_PDI_YSIZE,YSIZE),
                    CAM_READ_BITS(ptr,CAM_PDI_STRIDE,STRIDE));
                LOG_INF("PDC_EN:%d,  PDC_OUT:%d\n",CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_EN),CAM_READ_BITS(ptr,CAM_BNR_PDC_CON,PDC_OUT));

                memcpy((MUINT8*)pBuf[i][cnt][0]->virtAddr, (MUINT8*)pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength);
            }
            else {
                LOG_ERR("PDI table not found va(%p) size(%d)!!\n", pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].pTblAddr,
                    pInputInfo->DmaiTbls[i][_DMAI_TBL_PDI].tblLength);
                LOG_ERR("press to continue...\n");
                if(getchar() < 0)
                    LOG_ERR("getchar error");
            }
        }
        if(DMA_EN & LSCI_EN_){
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

                LOG_INF("LSCI srcTable=%p size=%d pa=0x%lx, va=%p\n",
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
                if(getchar() < 0) {
                    LOG_ERR("getchar error");
                }
            }
        }
        if(DMA_EN & PDO_EN_){
            MEM_ALLOC6(i,PDO,CAM,PDO);
            HEADER_MEM_ALLOC6(i,PDO,CAM,PDO);
        }
    }


    ptr = (UniDrvImp*)_uni;
    DMA_EN = UNI_READ_REG(ptr, CAM_UNI_TOP_DMA_EN);
    LOG_INF("module_UNI:  enabled DMA:0x%x\n",DMA_EN);
    if(DMA_EN & FLKO_A_EN_){
        MEM_ALLOC6(UNI_A,UNI_FLKO,UNI,FLKO_A);
        HEADER_MEM_ALLOC6(UNI_A,UNI_FLKO,UNI,FLKO_A);
    }
    if(DMA_EN & EISO_A_EN_){
        MEM_ALLOC6(UNI_A,UNI_EISO,UNI,EISO_A);
        HEADER_MEM_ALLOC6(UNI_A,UNI_EISO,UNI,EISO_A);
    }
    if(DMA_EN & RSSO_A_EN_){
        MEM_ALLOC6(UNI_A,UNI_RSSO_A,UNI,RSSO_A);
        HEADER_MEM_ALLOC6(UNI_A,UNI_RSSO_A,UNI,RSSO_A);
    }
    DMA_EN = UNI_READ_REG(ptr, CAM_UNI_B_TOP_DMA_EN);
    LOG_INF("module_UNI_B:  enabled DMA:0x%x\n",DMA_EN);
    if(DMA_EN & FLKO_A_EN_){
        MEM_ALLOC(UNI_B,UNI_B_FLKO,UNI,FLKO_A);
        HEADER_MEM_ALLOC(UNI_B,UNI_B_FLKO,UNI,FLKO_A);
    }
    if(DMA_EN & EISO_A_EN_){
        MEM_ALLOC(UNI_B,UNI_B_EISO,UNI,EISO_A);
        HEADER_MEM_ALLOC(UNI_B,UNI_B_EISO,UNI,EISO_A);
    }
    if(DMA_EN & RSSO_A_EN_){
        MEM_ALLOC(UNI_B,UNI_B_RSSO_A,UNI,RSSO_A);
        HEADER_MEM_ALLOC(UNI_B,UNI_B_RSSO_A,UNI,RSSO_A);
    }

    //enlarge test pattern if pattern size == TG size
    //cam will be fail when pattern size == TG size under continuous mode
    if (0) {
        CAM_REG_TG_SEN_GRAB_LIN tg;
        ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
        tg.Raw = CAM_READ_REG(ptr,CAM_TG_SEN_GRAB_LIN);
        TS_FakeDrvSensor().adjustPatternSize(tg.Bits.LIN_E - tg.Bits.LIN_S);
    }
    else {
    }

    /*
    * Enable sensor test pattern
    */
    if(lpdvt_pdn) {
        MINT32 u32Enable = 1;
        pSensorHalObj[0]->sendCommand((MUINT)NSCam::SENSOR_DEV_MAIN, NSCam::SENSOR_CMD_SET_TEST_PATTERN_OUTPUT, (MUINTPTR)&u32Enable, 0, 0);
        LOG_INF("Enable sensor test_pattern for lpdvt pdn test.");
    }

    /*
     * Enable VF for CAM(Twin) & CAMSV
     */
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    CAM_WRITE_BITS(ptr,CAM_TG_VF_CON,SINGLE_MODE,0);
    CAM_WRITE_BITS(ptr,CAM_TG_VF_CON,VFDATA_EN,1);

    #if (LPDVT_USING_CAMSV == 1)
    {
        IspDrvCamsv *pCamsvDrv0, *pCamsvDrv1;

        pCamsvDrv0 = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_0);
        pCamsvDrv0->init("CamSV_0");

        pCamsvDrv1 = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);
        pCamsvDrv1->init("CamSV_1");

        LOG_INF("Enable CAMSV0/1 VF");
        CAMSV_WRITE_BITS(pCamsvDrv0, CAMSV_TG_VF_CON, VFDATA_EN, 1);
        CAMSV_WRITE_BITS(pCamsvDrv1, CAMSV_TG_VF_CON, VFDATA_EN, 1);

        pCamsvDrv0->uninit("CamSV_0");
        pCamsvDrv0->destroyInstance();

        pCamsvDrv1->uninit("CamSV_1");
        pCamsvDrv1->destroyInstance();
    }
    #endif

    return 0;
}


MINT32 Pattern_Stop_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR linkpath)
{
#if 1
    MUINT32 snrAry = 0;
    _uni; linkpath;


    LOG_INF("Not yet support IRQ in current bitfile!!");
    LOG_INF("press any key to continuous or stop CPU here\n");
    if(getchar() < 0){
        LOG_ERR("getchar error");
    }

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

    CAM_WRITE_BITS(((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN,0);
    if (length > 1) {
        CAM_WRITE_BITS(((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN,0);
    }
    LOG_INF("press any key to wait for VF_OFF fetch\n");
    if(getchar() == EOF) {
        LOG_ERR("getchar is error");
    }

    TS_FakeDrvSensor().powerOff("fake", 1, &snrAry);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOff("fake2", 1, &snrArySub);
    }

    return 0;
}

MINT32 Pattern_Stop_6(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR linkpath)
{
    MUINT32 snrAry = 0;
    _uni; linkpath;

    UNUSED(length);
    LOG_INF("CAM under streaming .........");
    LOG_INF("press any key to stop CAM streaming");
    if(getchar() < 0){
        LOG_ERR("getchar error");
    }

    {
        IspDrvCamsv *pCamsvDrv0, *pCamsvDrv1;

        pCamsvDrv0 = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_0);
        pCamsvDrv0->init("CamSV_0");

        pCamsvDrv1 = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);
        pCamsvDrv1->init("CamSV_1");

        LOG_INF("Disable CAMSV0/1 VF");
        CAMSV_WRITE_BITS(pCamsvDrv0, CAMSV_TG_VF_CON, VFDATA_EN, 0);
        CAMSV_WRITE_BITS(pCamsvDrv1, CAMSV_TG_VF_CON, VFDATA_EN, 0);

        pCamsvDrv0->uninit("CamSV_0");
        pCamsvDrv0->destroyInstance();

        pCamsvDrv1->uninit("CamSV_1");
        pCamsvDrv1->destroyInstance();
    }

    CAM_WRITE_BITS(((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN,0);

    LOG_INF("CAM stopped .........");
    LOG_INF("press any key to exit test case");
    if(getchar() < 0){
        LOG_ERR("getchar error");
    }

    #if LPDVT_USING_SENINF == 1
    TS_FakeDrvSensor().powerOff("fake", 1, &snrAry);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOff("fake1", 1, &snrArySub);
    }
    #else
    pSensorHalObj[0]->powerOff("LPDVT", 1, &snrAry);
    #endif

    return 0;
}

MINT32 Pattern_Stop_7(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR linkpath)
{
    MUINT32 snrAry = 0;
    _uni; linkpath;_ptr;

    UNUSED(length);

    return 0;
}

MINT32 Pattern_Loading_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL, *ptrCam = NULL;
    MUINT32 snrAry = 0;
    const MUINT32 seninf[] = {
        #include "Emulation/CAM_UFO/CAM_UFO_SENINF.h"
        0xffffffff
    };

    inputInfo;


    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    ptrCam = ptr;
    #include "Emulation/CAM_UFO/CAM_UFO_CAMA.h"
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,0);

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }

    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_UFO/CAM_UFO_UNI.h"


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

    return 0;
}

MINT32 Pattern_Loading_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    const MUINT32 seninf[] = {
        #include "Emulation/CAM_TWIN/CAM_TWIN_SENINF.h"
        0xffffffff
    };

    static const unsigned char pattern_1_lsci_tbl[] = {
        #include "Emulation/CAM_TWIN/CAM_TWIN_LSCI_A.h"
    };
    static const unsigned char pattern_1_bpci_tbl[] = {
        #include "Emulation/CAM_TWIN/CAM_TWIN_BPCI_A.h"
    };


    /* save dmai buffer location, for latter memory allocation and loading */
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_1_lsci_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_1_lsci_tbl)/sizeof(pattern_1_lsci_tbl[0]);

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_1_bpci_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_1_bpci_tbl)/sizeof(pattern_1_bpci_tbl[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_1_lsci_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_1_lsci_tbl)/sizeof(pattern_1_lsci_tbl[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_1_bpci_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_1_bpci_tbl)/sizeof(pattern_1_bpci_tbl[0]);



    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include <Emulation/CAM_TWIN/CAM_TWIN_CAMA.h>
    //pattern have no enable cmos
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,1);


    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_TWIN/CAM_TWIN_CAMB.h"
    //
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,1);

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_TWIN/CAM_TWIN_UNI.h"
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

MINT32 Pattern_Loading_2(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    const MUINT32 seninf[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_SENINF.h"
        0xffffffff
    };

    static const unsigned char pattern_2_lsci_tbl_a[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_LSCI_A.h"
    };
    static const unsigned char pattern_2_lsci_tbl_b[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_LSCI_B.h"
    };
    static const unsigned char pattern_2_bpci_tbl_a[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_BPCI_A.h"
    };
    static const unsigned char pattern_2_bpci_tbl_b[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_BPCI_B.h"
    };
    static const unsigned char pattern_2_pdi_tbl_a[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_PDI_A.h"
    };
    static const unsigned char pattern_2_pdi_tbl_b[] = {
        #include "Emulation/CAM_SIMUL/CAM_SIMUL_PDI_B.h"
    };

    /* save dmai buffer location, for latter memory allocation and loading */
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_2_lsci_tbl_a;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_2_lsci_tbl_a)/sizeof(pattern_2_lsci_tbl_a[0]);

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_2_bpci_tbl_a;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_2_bpci_tbl_a)/sizeof(pattern_2_bpci_tbl_a[0]);

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].pTblAddr = pattern_2_pdi_tbl_a;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].tblLength = sizeof(pattern_2_pdi_tbl_a)/sizeof(pattern_2_pdi_tbl_a[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_2_lsci_tbl_b;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_2_lsci_tbl_b)/sizeof(pattern_2_lsci_tbl_b[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_2_bpci_tbl_b;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_2_bpci_tbl_b)/sizeof(pattern_2_bpci_tbl_b[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_2_pdi_tbl_b;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_2_pdi_tbl_b)/sizeof(pattern_2_pdi_tbl_b[0]);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_SIMUL/CAM_SIMUL_CAMA.h"
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,0);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_SIMUL/CAM_SIMUL_CAMB.h"
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
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
    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry, seninf);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOn("fake2", 1, &snrArySub);
    }

    return 0;
}


MINT32 Pattern_Loading_3(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    MUINT32 snrAry = 0;
    inputInfo;

    const MUINT32 seninf[] = {
        #include "Emulation/CAM_TWIN_RCP3/CAM_TWIN_RCP3_SENINF.h"
        0xffffffff
    };


    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    #include <Emulation/CAM_TWIN_RCP3/CAM_TWIN_RCP3_CAMA.h>
    //pattern have no enable cmos
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,1);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_TWIN_RCP3/CAM_TWIN_RCP3_CAMB.h"
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
    #include "Emulation/CAM_TWIN_RCP3/CAM_TWIN_RCP3_UNI.h"
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


MINT32 Pattern_Loading_4(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    MUINT32 snrAry = 0;
    const MUINT32 seninf[] = {
        #include "Emulation/CAM_BIN/CAM_BIN_SENINF.h"
        0xffffffff
    };

    static const unsigned char pattern_4_lsci_tbl[] = {
        #include "Emulation/CAM_BIN/CAM_BIN_LSCI.h"
    };
    static const unsigned char pattern_4_bpci_tbl[] = {
        #include "Emulation/CAM_BIN/CAM_BIN_BPCI.h"
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

    #include <Emulation/CAM_BIN/CAM_BIN_CAMA.h>
    //pattern have no enable cmos
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,1);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_BIN/CAM_BIN_CAMB.h"
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
    #include "Emulation/CAM_BIN/CAM_BIN_UNI.h"
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
    const MUINT32 seninf[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/CAM_TWIN_NO_DBN_SENINF.h"
        0xffffffff
    };


    static const unsigned char pattern_5_lsci_tbl[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/CAM_TWIN_NO_DBN_LSCI_A.h"
    };
    static const unsigned char pattern_5_bpci_tbl[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/CAM_TWIN_NO_DBN_BPCI_A.h"
    };


    /* save dmai buffer location, for latter memory allocation and loading */
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_5_lsci_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_5_lsci_tbl)/sizeof(pattern_5_lsci_tbl[0]);

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_5_bpci_tbl;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_5_bpci_tbl)/sizeof(pattern_5_bpci_tbl[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_5_lsci_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_5_lsci_tbl)/sizeof(pattern_5_lsci_tbl[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_5_bpci_tbl;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_5_bpci_tbl)/sizeof(pattern_5_bpci_tbl[0]);



    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include <Emulation/CAM_TWIN_NO_DBN/CAM_TWIN_NO_DBN_CAMA.h>
    //pattern have no enable cmos
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,1);


    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_TWIN_NO_DBN/CAM_TWIN_NO_DBN_CAMB.h"
    //
    CAM_WRITE_BITS(ptr,CAM_CTL_MISC,DB_EN,1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);
    CAM_WRITE_BITS(ptr,CAM_CTL_TWIN_STATUS,TWIN_EN,1);

    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_TWIN_NO_DBN/CAM_TWIN_NO_DBN_UNI.h"
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

MINT32 Pattern_Loading_6(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    IspDrv* ptr = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    IspDrvImp* ptrImp = NULL;
    NSCam::IHalSensor::ConfigParam m_sensorConfigParam;
    NSCam::MSize TgSize;
    uint32_t sensorArray[1];
    MUINT32 snrAry = 0, pixelMode = 0;
    CAM_REG_TG_SEN_GRAB_PXL tg_pix;
    CAM_REG_TG_SEN_GRAB_LIN tg_lin;
    MUINT sensorNum = 0;
    #if LPDVT_USING_SENINF == 1
    static const MUINT32 seninf[] = {
        #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_3SENINF.h"
        0xffffffff
    };
    #endif

    IMemDrv* pImemDrv = NULL;
    IMEM_BUF_INFO ibuf;
    IspDrvCamsv *pCamsvDrv0, *pCamsvDrv1;
    MUINT32  ui32Width, ui32Height,ui32Width_2, ui32Height_2, regval, stride, height;

    static const unsigned char pattern_6_lsci_tbl_a[] = {
        #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_LSCI_A.h"
    };
    static const unsigned char pattern_6_lsci_tbl_b[] = {
        #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_LSCI_B.h"
    };
    static const unsigned char pattern_6_bpci_tbl_a[] = {
        #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_BPCI_A.h"
    };
    static const unsigned char pattern_6_bpci_tbl_b[] = {
        #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_BPCI_B.h"
    };
    static const unsigned char pattern_6_pdi_tbl_a[] = {
        #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_PDI_A.h"
    };
    static const unsigned char pattern_6_pdi_tbl_b[] = {
        #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_PDI_B.h"
    };

    UNUSED(length);
    pImemDrv = IMemDrv::createInstance();
    if(pImemDrv->init() < 0){
        LOG_ERR(" imem init fail\n");
        return 1;
    }

    #if (LPDVT_USING_CAMSV == 1)
    LOG_INF("Create CAMSV0");
    pCamsvDrv0 = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_0);
    pCamsvDrv0->init("CamSV_0");

    LOG_INF("Create CAMSV1");
    pCamsvDrv1 = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);
    pCamsvDrv1->init("CamSV_1");
    #endif

    /* save dmai buffer location, for latter memory allocation and loading */
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].pTblAddr = pattern_6_lsci_tbl_a;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_6_lsci_tbl_a)/sizeof(pattern_6_lsci_tbl_a[0]);

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].pTblAddr = pattern_6_bpci_tbl_a;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_6_bpci_tbl_a)/sizeof(pattern_6_bpci_tbl_a[0]);

    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].pTblAddr = pattern_6_pdi_tbl_a;
    pInputInfo->DmaiTbls[CAM_A][_DMAI_TBL_PDI].tblLength = sizeof(pattern_6_pdi_tbl_a)/sizeof(pattern_6_pdi_tbl_a[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].pTblAddr = pattern_6_lsci_tbl_b;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_LSCI].tblLength = sizeof(pattern_6_lsci_tbl_b)/sizeof(pattern_6_lsci_tbl_b[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].pTblAddr = pattern_6_bpci_tbl_b;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_BPCI].tblLength = sizeof(pattern_6_bpci_tbl_b)/sizeof(pattern_6_bpci_tbl_b[0]);

    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].pTblAddr = pattern_6_pdi_tbl_b;
    pInputInfo->DmaiTbls[CAM_B][_DMAI_TBL_PDI].tblLength = sizeof(pattern_6_pdi_tbl_b)/sizeof(pattern_6_pdi_tbl_b[0]);


    /*
     * Start loading setting
     */
    LOG_INF("Loading setting ...\n");
    if(_uni == NULL){
        LOG_ERR(" uni is null\n");
        return 1;
    }
    ptr = (UniDrvImp*)_uni;
    #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_0UNI.h"
    //UNI_WRITE_BITS(ptr,CAM_UNI_TOP_MISC,DB_EN,1);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_1CAMA.h"
    CAM_WRITE_BITS(ptr, CAM_TG_SEN_MODE, CMOS_EN, 1);
    CAM_WRITE_BITS(ptr, CAM_CTL_RAW_INT_EN, SOF_INT_EN, 1);
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);

    tg_pix.Raw = CAM_READ_REG(ptr,CAM_TG_SEN_GRAB_PXL);
    tg_lin.Raw = CAM_READ_REG(ptr,CAM_TG_SEN_GRAB_LIN);

    switch ((CAM_READ_BITS(ptr, CAM_TG_SEN_MODE, DBL_DATA_BUS1) << 1) | CAM_READ_BITS(ptr, CAM_TG_SEN_MODE, DBL_DATA_BUS)) {
    case 0x0:
        pixelMode = 1;
        break;
    case 0x1:
        pixelMode = 2;
        break;
    case 0x3:
        pixelMode = 4;
        break;
    default:
        LOG_ERR("Error: wrong pixel mode %d",
            ((CAM_READ_BITS(ptr, CAM_TG_SEN_MODE, DBL_DATA_BUS1) << 1)| CAM_READ_BITS(ptr, CAM_TG_SEN_MODE, DBL_DATA_BUS)));
        return 1;
    }
    LOG_INF("Queried TG pixel mode: %d", pixelMode);

    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    #include "Emulation/CAM_LPDVT/CAM_LPDVT_BIG_2CAMAB.h"
    CAM_WRITE_REG(ptr,CAM_CTL_RAW_INT_EN,0x7fffffff);

    #if (LPDVT_USING_SENINF == 1)
    TS_FakeDrvSensor().powerOn("fake", 1, &snrAry, seninf);
    #endif

#if (LPDVT_USING_CAMSV == 1)
    ui32Width = tg_pix.Bits.PXL_E - tg_pix.Bits.PXL_S;
    ui32Height = tg_lin.Bits.LIN_E - tg_lin.Bits.LIN_S;
    ui32Width_2 = tg_pix.Bits.PXL_E - tg_pix.Bits.PXL_S;
    ui32Height_2 = tg_lin.Bits.LIN_E - tg_lin.Bits.LIN_S;

    LOG_INF("Camsv size0:%d_%d size1:%d_%d", ui32Width, ui32Height,ui32Width_2, ui32Height_2);

    /*
     *  LOADING CAMSV0 SETTING
     */
    regval = CAMSV_READ_REG(pCamsvDrv0, CAMSV_MODULE_EN);
    regval |= ((1<<4)| (1<<3)| (1<<0));
    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_MODULE_EN, regval);

    regval = CAMSV_READ_REG(pCamsvDrv0, CAMSV_CLK_EN);
    regval |= ((1<<0)| (1<<2)| (1<<15));
    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_CLK_EN, regval);

    #if 1
    LOG_INF("########################################");
    LOG_INF(" Force raw8");
    LOG_INF("########################################");
    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_FMT_SEL, ((1)<<24 | (2<<16) | 0));
    #else
    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_FMT_SEL, ((1)<<24 | (2<<16) | 7));
    #endif

    CAMSV_WRITE_BITS(pCamsvDrv0, CAMSV_INT_EN, INT_WCLR_EN, 0);
    CAMSV_WRITE_BITS(pCamsvDrv0, CAMSV_INT_EN, TG_SOF_INT_EN, 1);

    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_IMGO_XSIZE, ui32Width-1);
    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_IMGO_YSIZE, ui32Height-1);
    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_IMGO_STRIDE, ((1)<<16 | 2<<20 | (1)<<23 | (1)<<24 | ui32Width));

    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_TG_SEN_GRAB_PXL, (ui32Width<<16));
    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_TG_SEN_GRAB_LIN, (ui32Height<<16));

    CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_PAK, ((1)<<4 | 1));
    CAMSV_WRITE_BITS(pCamsvDrv0, CAMSV_TG_SEN_MODE, CMOS_EN, 1);
    switch(pixelMode) {
    case 1:
        CAMSV_WRITE_BITS(pCamsvDrv0,CAMSV_TG_SEN_MODE,DBL_DATA_BUS,0);
        CAMSV_WRITE_BITS(pCamsvDrv0,CAMSV_TG_SEN_MODE,DBL_DATA_BUS1,0);
        break;
    case 2:
        CAMSV_WRITE_BITS(pCamsvDrv0,CAMSV_TG_SEN_MODE,DBL_DATA_BUS,1);
        CAMSV_WRITE_BITS(pCamsvDrv0,CAMSV_TG_SEN_MODE,DBL_DATA_BUS1,0);
        break;
    case 4:
    default:
        CAMSV_WRITE_BITS(pCamsvDrv0,CAMSV_TG_SEN_MODE,DBL_DATA_BUS,1);
        CAMSV_WRITE_BITS(pCamsvDrv0,CAMSV_TG_SEN_MODE,DBL_DATA_BUS1,1);
        break;
    }

    stride = CAMSV_READ_BITS(pCamsvDrv0, CAMSV_IMGO_STRIDE, STRIDE);
    height = CAMSV_READ_BITS(pCamsvDrv0, CAMSV_IMGO_YSIZE, YSIZE) + 1;
    LOG_INF("Allocate CAMSV0 buf with stride:%d height:%d\n", stride, height);

    ibuf.size = stride * height;
    ibuf.useNoncache = 1;
    if (pImemDrv->allocVirtBuf(&ibuf) < 0) {
        LOG_ERR("Alloc imem failed [%d]\n", __LINE__);
        return 1;
    }
    if (pImemDrv->mapPhyAddr(&ibuf) < 0) {
        LOG_ERR("Map imem failed [%d]\n", __LINE__);
        return 1;
    }
    LOG_INF("CAMSV0 imgo PA:%x VA:0x%lx size:%d\n", (MUINT32)ibuf.virtAddr, ibuf.phyAddr, ibuf.size);
    CAMSV_WRITE_REG(pCamsvDrv0,CAMSV_IMGO_BASE_ADDR,ibuf.phyAddr);
    CAMSV_WRITE_REG(pCamsvDrv0,CAMSV_FBC_IMGO_ENQ_ADDR,ibuf.phyAddr);
    CAMSV_WRITE_REG(pCamsvDrv0,CAMSV_IMGO_FH_BASE_ADDR,ibuf.phyAddr);
    //////////////CAMSV_0_IMGO_BASE_ADDR

    /*
     *  LOADING CAMSV1 SETTING
     */
    regval = CAMSV_READ_REG(pCamsvDrv1, CAMSV_MODULE_EN);
    regval |= ((1<<4)| (1<<3)| (1<<0));
    CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_MODULE_EN, regval);

    regval = CAMSV_READ_REG(pCamsvDrv1, CAMSV_CLK_EN);
    regval |= ((1<<0)| (1<<2)| (1<<15));
    CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_CLK_EN, regval);

    CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_FMT_SEL, ((2*1+1)<<24 | (0<<16) | 1));

    CAMSV_WRITE_BITS(pCamsvDrv1, CAMSV_INT_EN, INT_WCLR_EN, 0);

    CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_IMGO_XSIZE, (ui32Width_2*12/8-1));
    CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_IMGO_YSIZE, ui32Height_2-1);
    CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_IMGO_STRIDE, ((2*1+1)<<16 | 0<<20 | (1)<<23 | (1)<<24 | (ui32Width_2*12/8)));

    CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_TG_SEN_GRAB_PXL, (ui32Width_2<<16));
    CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_TG_SEN_GRAB_LIN, (ui32Height_2<<16));

    CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_PAK, ((1)<<4 | 2));
    CAMSV_WRITE_BITS(pCamsvDrv1, CAMSV_TG_SEN_MODE, CMOS_EN, 1);
    switch(pixelMode) {
    case 1:
        CAMSV_WRITE_BITS(pCamsvDrv1,CAMSV_TG_SEN_MODE,DBL_DATA_BUS,0);
        CAMSV_WRITE_BITS(pCamsvDrv1,CAMSV_TG_SEN_MODE,DBL_DATA_BUS1,0);
        break;
    case 2:
        CAMSV_WRITE_BITS(pCamsvDrv1,CAMSV_TG_SEN_MODE,DBL_DATA_BUS,1);
        CAMSV_WRITE_BITS(pCamsvDrv1,CAMSV_TG_SEN_MODE,DBL_DATA_BUS1,0);
        break;
    case 4:
    default:
        CAMSV_WRITE_BITS(pCamsvDrv1,CAMSV_TG_SEN_MODE,DBL_DATA_BUS,1);
        CAMSV_WRITE_BITS(pCamsvDrv1,CAMSV_TG_SEN_MODE,DBL_DATA_BUS1,1);
        break;
    }

    stride = CAMSV_READ_BITS(pCamsvDrv1, CAMSV_IMGO_STRIDE, STRIDE);
    height = CAMSV_READ_BITS(pCamsvDrv1, CAMSV_IMGO_YSIZE, YSIZE) + 1;
    LOG_INF("Allocate CAMSV0 buf with stride:%d height:%d\n", stride, height);

    ibuf.size = stride * height;
    ibuf.useNoncache = 1;
    if (pImemDrv->allocVirtBuf(&ibuf) < 0) {
        LOG_ERR("Alloc imem failed [%d]\n", __LINE__);
        return 1;
    }
    if (pImemDrv->mapPhyAddr(&ibuf) < 0) {
        LOG_ERR("Map imem failed [%d]\n", __LINE__);
        return 1;
    }
    LOG_INF("CAMSV1 imgo PA:%x VA:0x%lx size:%d\n", (MUINT32)ibuf.virtAddr, ibuf.phyAddr, ibuf.size);
    CAMSV_WRITE_REG(pCamsvDrv1,CAMSV_IMGO_BASE_ADDR,ibuf.phyAddr);
    CAMSV_WRITE_REG(pCamsvDrv1,CAMSV_FBC_IMGO_ENQ_ADDR,ibuf.phyAddr);
    CAMSV_WRITE_REG(pCamsvDrv1,CAMSV_IMGO_FH_BASE_ADDR,ibuf.phyAddr);
    //////////////CAMSV_1_IMGO_BASE_ADDR

    pCamsvDrv0->uninit("CamSV_0");
    pCamsvDrv0->destroyInstance();

    pCamsvDrv1->uninit("CamSV_1");
    pCamsvDrv1->destroyInstance();

    pImemDrv->uninit();
    pImemDrv->destroyInstance();
#endif //LPDVT_USING_CAMSV == 1

    #if (LPDVT_USING_SENINF == 0)

    pHalSensorList = NSCam::IHalSensorList::get();

    LOG_INF("search sensor...\n");
    pHalSensorList->searchSensors();
    sensorNum = pHalSensorList->queryNumberOfSensors();
    if(0 == sensorNum){
        LOG_ERR("sensorNum = 0\n");
        return 1;
    } else
        LOG_INF("sensorNum:%x\n", sensorNum);

    LOG_INF("query sensor\n");
    pHalSensorList->querySensorStaticInfo(pHalSensorList->querySensorDevIdx(0), &mSensorInfo[0]);

    LOG_INF("power-on sensor\n");

    sensorArray[0] = 0; //main cam
    pSensorHalObj[0] = pHalSensorList->createSensor((char*)"LPDVT", 0);
    if (pSensorHalObj[0] == NULL) {
        LOG_INF("mpSensorHalObj is NULL");
        return 1;
    }
    pSensorHalObj[0]->powerOn((char*)"LPDVT", 1, &sensorArray[0]);

    TgSize.w = mSensorInfo[0].captureWidth;
    TgSize.h = mSensorInfo[0].captureHeight;

    NSCam::IHalSensor::ConfigParam sensorCfg =
    {
       sensorArray[0],   /* index            */
       TgSize,           /* crop             */
       NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE, /* scenarioId       */
       0,                /* isBypassScenario */
       1,                /* isContinuous     */
       MFALSE,           /* iHDROn           */
       0,                /* framerate        */
       0,                /* two pixel on     */
       0,                /* debugmode        */
    };

    sensorCfg.twopixelOn = 1;

    LOG_INF("start config TG: %d_%d\n", TgSize.w, TgSize.h);

    pSensorHalObj[0]->configure(1, &sensorCfg);


    ptrImp = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);
    if (NULL == ptrImp) {
        LOG_ERR("Error: IspDrv CreateInstace fail");
        return 1;
    }
    if (ptrImp->init("LPDVT") < 0) {
        LOG_ERR("Error: IspDrv init fail");
        return 1;
    }
    ptrImp->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only

    LOG_INF("Set seninf top cam mux 0 for CAMVS 0x%x def:0x%x",
        SENINF_TOP_CAM_MUX_CTRL, ptrImp->readReg(SENINF_TOP_CAM_MUX_CTRL - 0x18040000));
    ptrImp->writeReg((SENINF_TOP_CAM_MUX_CTRL - 0x18040000), 0);

    ptrImp->setRWMode(ISP_DRV_RW_MMAP);
    ptrImp->uninit("LPDVT");
    ptrImp->destroyInstance();


    #endif // LPDVT_USING_SENINF == 0

    return 0;
}

MINT32 Pattern_BitTrue_0(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0, cnt_1=0, _stride=0, _xsize=0, _preMSB=0;
    MUINT32 *pTable = NULL,*pMem = NULL;
    //byte compare
    MUINT8 *pTableB = NULL,*pMemB = NULL;
    char    szFileName[256];
    FILE*   pFp = NULL;

    static const unsigned char golden_2_ufeo_a[] = {
        #include "Emulation/CAM_UFO/Golden/ufeo_a_golden.dhex"
    };

    UNUSED(length);
    UNUSED(_uni);

//code size issue, do not build-in
#undef  _RRZO_BITTRUE_
#undef  _IMGO_BITTRUE_
#define _IMGO_BITTRUE_ (1)
#define _RRZO_BITTRUE_ (1)

#if (_IMGO_BITTRUE_ == 1)
    static const unsigned char golden_2_imgo_a[] = {
        #include "Emulation/CAM_UFO/Golden/imgo_a_golden.dhex"
    };
#endif
#if (_RRZO_BITTRUE_ == 1)
    static const unsigned char golden_2_rrzo_a[] = {
        #include "Emulation/CAM_UFO/Golden/rrzo_a_golden.dhex"
    };
#endif

    #undef COMPARE
    #define COMPARE(module,STR,STR2,STR3,STR4)\
            _tmp = (DMA_EN & STR##_EN_);\
            _cnt = 0;\
            while(_tmp != 0){\
                _cnt++;\
                _tmp >>= 1;\
            }\
            pTable = (MUINT32*)golden_2_##STR4;\
            pMem = (MUINT32*)pBuf[module][_cnt][0]->virtAddr;\
            _tmp = pBuf[module][_cnt][0]->size/sizeof(MUINT32);\
            _cnt = 0;\
            _stride = (STR2##_READ_BITS(ptr,CAM_##STR3##_STRIDE,STRIDE))/sizeof(MUINT32);\
            LOG_INF("###########################\n");\
            for (_cnt = 0, cnt_1 = 0; _cnt < _tmp; _cnt++) { \
                if ((*pMem) != (*pTable)) {\
                    LOG_ERR("%s:%s bit true fail,@src=%p golden=%p\n",#module,#STR,pMem,pTable);\
                    LOG_ERR("%s:%s bit true fail offset=0x%x 0x%x_0x%x\n",#module,#STR,_cnt,*pMem,*pTable);\
                    if(_cnt > 1) {\
                        LOG_ERR("%s:%s bit true fail,prev1: 0x%x_0x%x\n",#module,#STR,*(pMem-1),*(pTable-1));\
                        LOG_ERR("%s:%s bit true fail,prev2: 0x%x_0x%x\n",#module,#STR,*(pMem-2),*(pTable-2));\
                    }\
                    break;\
                }\
                pMem++;\
                pTable++;\
                cnt_1++;\
            }\
            if(_cnt != _tmp){\
                ret++;\
                LOG_ERR("%s:%s bit true fail,%d_%d\n",#module,#STR,_cnt,_tmp);\
            }\
            else{\
                LOG_INF("%s:%s bit true pass,%d\n",#module,#STR,_tmp);\
            }\
            LOG_INF("###########################\n");\

    #undef COMPARE_BYTE/*for IMGO skip xsize~stride*/
    #define COMPARE_BYTE(module,STR,STR2,STR3,STR4)\
            _tmp = (DMA_EN & STR##_EN_);\
            _cnt = 0;\
            while(_tmp != 0){\
                _cnt++;\
                _tmp >>= 1;\
            }\
            pTableB = (MUINT8*)golden_2_##STR4;\
            pMemB = (MUINT8*)pBuf[module][_cnt][0]->virtAddr;\
            _tmp = pBuf[module][_cnt][0]->size;\
            _xsize = (STR2##_READ_BITS(ptr,CAM_##STR3##_XSIZE,XSIZE));\
            _stride = (STR2##_READ_BITS(ptr,CAM_##STR3##_STRIDE,STRIDE));\
            LOG_INF("###########################\n");\
            for (_cnt = 0, cnt_1 = 0; _cnt < _tmp; _cnt++) { \
                if ((*pMemB) != (*pTableB)) {\
                    LOG_ERR("%s:%s bit true fail,@src=%p golden=%p\n",#module,#STR,pMemB,pTableB);\
                    LOG_ERR("%s:%s bit true fail offset=0x%x 0x%x_0x%x\n",#module,#STR,_cnt,*pMemB,*pTableB);\
                    if(_cnt > 1) {\
                        LOG_ERR("%s:%s bit true fail,prev1: 0x%x_0x%x\n",#module,#STR,*(pMemB-1),*(pTableB-1));\
                        LOG_ERR("%s:%s bit true fail,prev2: 0x%x_0x%x\n",#module,#STR,*(pMemB-2),*(pTableB-2));\
                    }\
                    break;\
                }\
                if (cnt_1 == _xsize){\
                    cnt_1 = _stride-_xsize; \
                    pMem+=cnt_1; pTable+=cnt_1;_cnt+=(cnt_1-1);\
                    cnt_1 = 0;\
                    continue;\
                }\
                pMemB++;\
                pTableB++;\
                cnt_1++;\
            }\
            if(_cnt != _tmp){\
                ret++;\
                LOG_ERR("%s:%s bit true fail,%d_%d\n",#module,#STR,_cnt,_tmp);\
            }\
            else{\
                LOG_INF("%s:%s bit true pass,%d\n",#module,#STR,_tmp);\
            }\
            LOG_INF("###########################\n");\

    //aao/pso/bpci's stride is useless. need to use xsize to cal. total memory size
    #undef DUMPIMG
    #define DUMPIMG(module,STR,STR2,STR3)\
        _tmp = (DMA_EN & STR3##_EN_);\
        cnt_1 = 0;\
        while(_tmp != 0){\
            cnt_1++;\
            _tmp >>= 1;\
        }\
        LOG_INF("%s:%s, VA:0x(%lx), size(%d)", #module,#STR,pBuf[module][cnt_1][0]->virtAddr,pBuf[module][cnt_1][0]->size);\
        LOG_INF("%s:%s, XSIZE:%d,YSIZE:%d,STRIDE:%d",#module,#STR,STR2##_READ_REG(ptr,CAM_##STR##_XSIZE)+1,\
                STR2##_READ_REG(ptr,CAM_##STR##_YSIZE)+1,STR2##_READ_BITS(ptr,CAM_##STR##_STRIDE,STRIDE));\
        sprintf(szFileName, "%s/%s_%s.raw" , "/data",#module,#STR);\
        pFp = fopen(szFileName, "wb");\
        if (NULL == pFp ) {\
            LOG_ERR("Can't open file to save image");\
        }\
        else {\
            fwrite((MUINT8 *)pBuf[module][cnt_1][0]->virtAddr, 1, pBuf[module][cnt_1][0]->size, pFp);\
            fflush(pFp);\
            if (0 != fsync(fileno(pFp))) {\
                LOG_ERR("%s:%s,fync fail",#module,#STR);\
                fclose(pFp);\
            }else{\
                LOG_INF("%s:%s,Save image: %s",#module,#STR,szFileName);\
                fclose(pFp);\
            }\
        }\

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

#if (_IMGO_BITTRUE_ == 1)
    COMPARE_BYTE(CAM_A,IMGO,CAM,IMGO,imgo_a);
    DUMPIMG(CAM_A,IMGO,CAM,IMGO);
#endif
#if (_RRZO_BITTRUE_ == 1)
    COMPARE_BYTE(CAM_A,RRZO,CAM,RRZO,rrzo_a);
    DUMPIMG(CAM_A,RRZO,CAM,RRZO);
#endif
    COMPARE(CAM_A,UFEO,CAM,UFEO,ufeo_a);
    DUMPIMG(CAM_A,UFEO,CAM,UFEO);
    //

    LOG_INF("press any key continuous\n");
    if(getchar() < 0) {
        LOG_ERR("getchar error");
    }
    return ret;
}

MINT32 Pattern_BitTrue_1(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0, cnt_1;
    MUINT32 *pTable = NULL,*pMem = NULL;

    char    szFileName[256];
    FILE*   pFp = NULL;

    UNUSED(length);
    UNUSED(_uni);
    static const unsigned char golden_1_aao[] = {
        #include "Emulation/CAM_TWIN/Golden/aao_golden.dhex"
    };
    static const unsigned char golden_1_lcso[] = {
        #include "Emulation/CAM_TWIN/Golden/lcso_golden.dhex"
    };
    static const unsigned char golden_1_pso[] = {
        #include "Emulation/CAM_TWIN/Golden/pso_golden.dhex"
    };
    static const unsigned char golden_1_afo_a[] = {
        #include "Emulation/CAM_TWIN/Golden/afo_a_golden.dhex"
    };
    static const unsigned char golden_1_afo_b[] = {
        #include "Emulation/CAM_TWIN/Golden/afo_b_golden.dhex"
    };
    static const unsigned char golden_1_pdo_a[] = {
        #include "Emulation/CAM_TWIN/Golden/pdo_a_golden.dhex"
    };
    static const unsigned char golden_1_pdo_b[] = {
        #include "Emulation/CAM_TWIN/Golden/pdo_b_golden.dhex"
    };


#if 0 //code size issue, do not build-in. also, imgo/rrzo's stride is not the same as golden
    static const unsigned char golden_1_imgo_a[] = {
#include "Emulation/CAM_TWIN_NO_DBN/Golden/imgo_a_golden.dhex"
    };
    static const unsigned char golden_1_imgo_b[] = {
#include "Emulation/CAM_TWIN_NO_DBN/Golden/imgo_b_golden.dhex"
    };
    static const unsigned char golden_1_rrzo_a[] = {
#include "Emulation/CAM_TWIN_NO_DBN/Golden/rrzo_a_golden.dhex"
    };
    static const unsigned char golden_1_rrzo_b[] = {
#include "Emulation/CAM_TWIN_NO_DBN/Golden/rrzo_b_golden.dhex"
    };
#endif
    #undef COMPARE
    #define COMPARE(module,STR,STR2)\
            _tmp = (DMA_EN & STR##_EN_);\
            _cnt = 0;\
            while(_tmp != 0){\
                _cnt++;\
                _tmp >>= 1;\
            }\
            pTable = (MUINT32*)golden_1_##STR2;\
            pMem = (MUINT32*)pBuf[module][_cnt][0]->virtAddr;\
            _tmp = pBuf[module][_cnt][0]->size/sizeof(MUINT32);\
            LOG_INF("###########################\n");\
            for (_cnt = 0; _cnt < _tmp; _cnt++) { \
                if ((*pMem) != (*pTable)) {\
                    LOG_ERR("%s bit true fail,@src=%p golden=%p\n",#STR,pMem,pTable);\
                    LOG_ERR("%s bit true fail offset=0x%x 0x%x_0x%x\n",#STR,_cnt,*pMem,*pTable);\
                    if(_cnt > 1) {\
                        LOG_ERR("%s bit true fail,prev1: 0x%x_0x%x\n",#STR,*(pMem-1),*(pTable-1));\
                        LOG_ERR("%s bit true fail,prev2: 0x%x_0x%x\n",#STR,*(pMem-2),*(pTable-2));\
                    }\
                    break;\
                }\
                pMem++;\
                pTable++;\
            }\
            if(_cnt != _tmp){\
                ret++;\
                LOG_ERR("%s bit true fail,%d_%d\n",#STR,_cnt,_tmp);\
            }\
            else{\
                LOG_INF("%s bit true pass\n",#STR);\
            }\
            LOG_INF("###########################\n");\


    #define DUMPIMG(module,STR)\
        _tmp = (DMA_EN & STR##_EN_);\
        cnt_1 = 0;\
        while(_tmp != 0){\
            cnt_1++;\
            _tmp >>= 1;\
        }\
        LOG_INF("%s:%s, VA:0x(%lx), size(%d)", #module,#STR,pBuf[module][cnt_1][0]->virtAddr,pBuf[module][cnt_1][0]->size);\
        LOG_INF("%s:%s XSIZE:%d,YSIZE:%d,STRIDE:%d",#module,#STR,CAM_READ_REG(ptr,CAM_##STR##_XSIZE)+1,\
        CAM_READ_REG(ptr,CAM_##STR##_YSIZE)+1,CAM_READ_BITS(ptr,CAM_##STR##_STRIDE,STRIDE));\
        sprintf(szFileName, "%s/%s_%s.raw" , "/data",#module,#STR);\
        pFp = fopen(szFileName, "wb");\
        if (NULL == pFp ) {\
            LOG_ERR("Can't open file to save image");\
        }\
        else {\
            fwrite((MUINT8 *)pBuf[module][cnt_1][0]->virtAddr, 1, pBuf[module][cnt_1][0]->size, pFp);\
            fflush(pFp);\
            if (0 != fsync(fileno(pFp))) {\
                LOG_ERR("fync fail");\
                fclose(pFp);\
            }else{\
                LOG_INF("Save image: %s", szFileName);\
                fclose(pFp);\
            }\
        }\


    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    COMPARE(CAM_A,AAO,aao);
    COMPARE(CAM_A,PSO,pso);
    COMPARE(CAM_A,LCSO,lcso);
    COMPARE(CAM_A,AFO,afo_a);
    COMPARE(CAM_A,PDO,pdo_a);

    DUMPIMG(CAM_A,IMGO);
    DUMPIMG(CAM_A,RRZO);

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    COMPARE(CAM_B,AFO,afo_b);
    COMPARE(CAM_B,PDO,pdo_b);

    DUMPIMG(CAM_B,IMGO);
    DUMPIMG(CAM_B,RRZO);

    LOG_INF("press any key continuous\n");
    if(getchar() < 0) {
        LOG_ERR("getchar error");
    }
    return ret;
}

MINT32 Pattern_BitTrue_2(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0, cnt_1=0, _stride=0, _xsize=0, _preMSB=0;
    MUINT32 *pTable = NULL,*pMem = NULL;
    //byte compare
    MUINT8 *pTableB = NULL,*pMemB = NULL;
    char    szFileName[256];
    FILE*   pFp = NULL;

    static const unsigned char golden_2_aao_a[] = {
        #include "Emulation/CAM_SIMUL/Golden/aao_a_golden.dhex"
    };
    static const unsigned char golden_2_aao_b[] = {
        #include "Emulation/CAM_SIMUL/Golden/aao_b_golden.dhex"
    };
    static const unsigned char golden_2_pso_a[] = {
        #include "Emulation/CAM_SIMUL/Golden/pso_a_golden.dhex"
    };
    static const unsigned char golden_2_pso_b[] = {
        #include "Emulation/CAM_SIMUL/Golden/pso_b_golden.dhex"
    };
    static const unsigned char golden_2_lcso_a[] = {
        #include "Emulation/CAM_SIMUL/Golden/lcso_a_golden.dhex"
    };
    static const unsigned char golden_2_lcso_b[] = {
        #include "Emulation/CAM_SIMUL/Golden/lcso_b_golden.dhex"
    };
    static const unsigned char golden_2_afo_a[] = {
        #include "Emulation/CAM_SIMUL/Golden/afo_a_golden.dhex"
    };
    static const unsigned char golden_2_afo_b[] = {
        #include "Emulation/CAM_SIMUL/Golden/afo_b_golden.dhex"
    };
    static const unsigned char golden_2_pdo_a[] = {
        #include "Emulation/CAM_SIMUL/Golden/pdo_a_golden.dhex"
    };
    static const unsigned char golden_2_pdo_b[] = {
        #include "Emulation/CAM_SIMUL/Golden/pdo_b_golden.dhex"
    };
    static const unsigned char golden_2_rsso_a[] = {
        #include "Emulation/CAM_SIMUL/Golden/rsso_a_golden.dhex"
    };
    static const unsigned char golden_2_rsso_b[] = {
        #include "Emulation/CAM_SIMUL/Golden/rsso_b_golden.dhex"
    };
    static const unsigned char golden_2_flko_a[] = {
        #include "Emulation/CAM_SIMUL/Golden/flko_a_golden.dhex"
    };
    static const unsigned char golden_2_flko_b[] = {
        #include "Emulation/CAM_SIMUL/Golden/flko_b_golden.dhex"
    };
    UNUSED(length);
    UNUSED(_uni);

//code size issue, do not build-in
#define _IMGO_BITTRUE_ (0)
#define _RRZO_BITTRUE_ (0)

#if (_IMGO_BITTRUE_ == 1)
    static const unsigned char golden_2_imgo_a[] = {
        #include "Emulation/CAM_SIMUL/Golden/imgo_a_golden.dhex"
    };
    static const unsigned char golden_2_imgo_b[] = {
        #include "Emulation/CAM_SIMUL/Golden/imgo_b_golden.dhex"
    };
#endif
#if (_RRZO_BITTRUE_ == 1)
    static const unsigned char golden_2_rrzo_a[] = {
        #include "Emulation/CAM_SIMUL/Golden/rrzo_a_golden.dhex"
    };
    static const unsigned char golden_2_rrzo_b[] = {
        #include "Emulation/CAM_SIMUL/Golden/rrzo_b_golden.dhex"
    };
#endif

    #undef COMPARE
    /* NOTE: skip xsize~stride
       FLKO is 1-D data, only compare data with (xsize) bytes
       PDO will skip comparation after PDO_END_TOKEN of each line, then jump to next line
    */
    #define COMPARE(module,STR,STR2,STR3,STR4)\
            _tmp = (DMA_EN & STR##_EN_);\
            _cnt = 0;\
            while(_tmp != 0){\
                _cnt++;\
                _tmp >>= 1;\
            }\
            pTable = (MUINT32*)golden_2_##STR4;\
            pMem = (MUINT32*)pBuf[module][_cnt][0]->virtAddr;\
            if ((module == UNI_A) || (module == UNI_B)) {\
                if (STR##_EN_ == FLKO_A_EN_)\
                {\
                    _tmp = (STR2##_READ_BITS(ptr,CAM_##STR3##_YSIZE,YSIZE)+1) * STR2##_READ_BITS(ptr,CAM_##STR3##_XSIZE,XSIZE);\
                    _tmp /= sizeof(MUINT32);\
                } else \
                    _tmp = pBuf[module][_cnt][0]->size/sizeof(MUINT32);\
            } else \
                _tmp = pBuf[module][_cnt][0]->size/sizeof(MUINT32);\
            _cnt = 0;\
            _stride = (STR2##_READ_BITS(ptr,CAM_##STR3##_STRIDE,STRIDE))/sizeof(MUINT32);\
            LOG_INF("###########################\n");\
            for (_cnt = 0, cnt_1 = 0; _cnt < _tmp; _cnt++) { \
                if (STR##_EN_ == PDO_EN_ && (module == CAM_A || module == CAM_B)) \
                {\
                    if (((*pMem) & 0xFFFF) == PDO_END_TOKEN){\
                        if(cnt_1 > _stride) cnt_1 = _stride-2;\
                        cnt_1 = _stride-cnt_1-1;\
                        pMem+=cnt_1; pTable+=cnt_1;_cnt+=(cnt_1-1);\
                        cnt_1 = 0; _preMSB = 0;\
                        continue;\
                    } else if ((((*pMem)>>16) & 0xFFFF) == PDO_END_TOKEN){\
                        if(cnt_1 > _stride) cnt_1 = _stride-2;\
                        cnt_1 = _stride-cnt_1-1;\
                        pMem+=cnt_1; pTable+=cnt_1;_cnt+=(cnt_1-1);\
                        cnt_1 = 0; _preMSB = 1;\
                        continue;\
                    } else if (cnt_1 == 0 && _preMSB == 1 && (*pMem) != (*pTable)) {\
                        if(((*pMem) & 0xFFFF) == ((*pTable) & 0xFFFF)){\
                            pMem++; pTable++;cnt_1++;\
                            continue;\
                        }\
                    }\
                }\
                if ((*pMem) != (*pTable)) {\
                    LOG_ERR("%s:%s bit true fail,@src=%p golden=%p\n",#module,#STR,pMem,pTable);\
                    LOG_ERR("%s:%s bit true fail offset=0x%x 0x%x_0x%x\n",#module,#STR,_cnt,*pMem,*pTable);\
                    if(_cnt > 1) {\
                        LOG_ERR("%s:%s bit true fail,prev1: 0x%x_0x%x\n",#module,#STR,*(pMem-1),*(pTable-1));\
                        LOG_ERR("%s:%s bit true fail,prev2: 0x%x_0x%x\n",#module,#STR,*(pMem-2),*(pTable-2));\
                    }\
                    break;\
                }\
                pMem++;\
                pTable++;\
                cnt_1++;\
            }\
            if(_cnt != _tmp){\
                ret++;\
                LOG_ERR("%s:%s bit true fail,%d_%d\n",#module,#STR,_cnt,_tmp);\
            }\
            else{\
                LOG_INF("%s:%s bit true pass,%d\n",#module,#STR,_tmp);\
            }\
            LOG_INF("###########################\n");\

    #undef COMPARE_BYTE/*for IMGO skip xsize~stride*/
    #define COMPARE_BYTE(module,STR,STR2,STR3,STR4)\
            _tmp = (DMA_EN & STR##_EN_);\
            _cnt = 0;\
            while(_tmp != 0){\
                _cnt++;\
                _tmp >>= 1;\
            }\
            pTableB = (MUINT8*)golden_2_##STR4;\
            pMemB = (MUINT8*)pBuf[module][_cnt][0]->virtAddr;\
            _tmp = pBuf[module][_cnt][0]->size;\
            _xsize = (STR2##_READ_BITS(ptr,CAM_##STR3##_XSIZE,XSIZE));\
            _stride = (STR2##_READ_BITS(ptr,CAM_##STR3##_STRIDE,STRIDE));\
            LOG_INF("###########################\n");\
            for (_cnt = 0, cnt_1 = 0; _cnt < _tmp; _cnt++) { \
                if ((*pMemB) != (*pTableB)) {\
                    LOG_ERR("%s:%s bit true fail,@src=%p golden=%p\n",#module,#STR,pMemB,pTableB);\
                    LOG_ERR("%s:%s bit true fail offset=0x%x 0x%x_0x%x\n",#module,#STR,_cnt,*pMemB,*pTableB);\
                    if(_cnt > 1) {\
                        LOG_ERR("%s:%s bit true fail,prev1: 0x%x_0x%x\n",#module,#STR,*(pMemB-1),*(pTableB-1));\
                        LOG_ERR("%s:%s bit true fail,prev2: 0x%x_0x%x\n",#module,#STR,*(pMemB-2),*(pTableB-2));\
                    }\
                    break;\
                }\
                if (cnt_1 == _xsize){\
                    cnt_1 = _stride-_xsize; \
                    pMem+=cnt_1; pTable+=cnt_1;_cnt+=(cnt_1-1);\
                    cnt_1 = 0;\
                    continue;\
                }\
                pMemB++;\
                pTableB++;\
                cnt_1++;\
            }\
            if(_cnt != _tmp){\
                ret++;\
                LOG_ERR("%s:%s bit true fail,%d_%d\n",#module,#STR,_cnt,_tmp);\
            }\
            else{\
                LOG_INF("%s:%s bit true pass,%d\n",#module,#STR,_tmp);\
            }\
            LOG_INF("###########################\n");\

    //aao/pso/bpci's stride is useless. need to use xsize to cal. total memory size
    #undef DUMPIMG
    #define DUMPIMG(module,STR,STR2,STR3)\
        _tmp = (DMA_EN & STR3##_EN_);\
        cnt_1 = 0;\
        while(_tmp != 0){\
            cnt_1++;\
            _tmp >>= 1;\
        }\
        LOG_INF("%s:%s, VA:0x(%lx), size(%d)", #module,#STR,pBuf[module][cnt_1][0]->virtAddr,pBuf[module][cnt_1][0]->size);\
        LOG_INF("%s:%s, XSIZE:%d,YSIZE:%d,STRIDE:%d",#module,#STR,STR2##_READ_REG(ptr,CAM_##STR##_XSIZE)+1,\
                STR2##_READ_REG(ptr,CAM_##STR##_YSIZE)+1,STR2##_READ_BITS(ptr,CAM_##STR##_STRIDE,STRIDE));\
        sprintf(szFileName, "%s/%s_%s.raw" , "/data",#module,#STR);\
        pFp = fopen(szFileName, "wb");\
        if (NULL == pFp ) {\
            LOG_ERR("Can't open file to save image");\
        }\
        else {\
            fwrite((MUINT8 *)pBuf[module][cnt_1][0]->virtAddr, 1, pBuf[module][cnt_1][0]->size, pFp);\
            fflush(pFp);\
            if (0 != fsync(fileno(pFp))) {\
                LOG_ERR("%s:%s,fync fail",#module,#STR);\
                fclose(pFp);\
            }else{\
                LOG_INF("%s:%s,Save image: %s",#module,#STR,szFileName);\
                fclose(pFp);\
            }\
        }\

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

#if (_IMGO_BITTRUE_ == 1)
    COMPARE_BYTE(CAM_A,IMGO,CAM,IMGO,imgo_a);
    DUMPIMG(CAM_A,IMGO,CAM,IMGO);
#endif
#if (_RRZO_BITTRUE_ == 1)
    COMPARE_BYTE(CAM_A,RRZO,CAM,RRZO,rrzo_a);
    DUMPIMG(CAM_A,RRZO,CAM,RRZO);
#endif

    COMPARE(CAM_A,AAO,CAM,AAO,aao_a);
    COMPARE(CAM_A,AFO,CAM,AFO,afo_a);
    COMPARE(CAM_A,PSO,CAM,PSO,pso_a);
    COMPARE(CAM_A,LCSO,CAM,LCSO,lcso_a);
    COMPARE(CAM_A,PDO,CAM,PDO,pdo_a);

    DUMPIMG(CAM_A,AAO,CAM,AAO);
    DUMPIMG(CAM_A,AFO,CAM,AFO);
    DUMPIMG(CAM_A,PSO,CAM,PSO);
    DUMPIMG(CAM_A,LCSO,CAM,LCSO);
    DUMPIMG(CAM_A,PDO,CAM,PDO);

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

#if (_IMGO_BITTRUE_ == 1)
    COMPARE_BYTE(CAM_B,IMGO,CAM,IMGO,imgo_b);
    DUMPIMG(CAM_B,IMGO,CAM,IMGO);
#endif
#if (_RRZO_BITTRUE_ == 1)
    COMPARE_BYTE(CAM_B,RRZO,CAM,RRZO,rrzo_b);
    DUMPIMG(CAM_B,RRZO,CAM,RRZO);
#endif

    COMPARE(CAM_B,AAO,CAM,AAO,aao_b);
    COMPARE(CAM_B,AFO,CAM,AFO,afo_b);
    COMPARE(CAM_B,PSO,CAM,PSO,pso_b);
    COMPARE(CAM_B,LCSO,CAM,LCSO,lcso_b);
    COMPARE(CAM_B,PDO,CAM,PDO,pdo_b);

    DUMPIMG(CAM_B,AAO,CAM,AAO);
    DUMPIMG(CAM_B,AFO,CAM,AFO);
    DUMPIMG(CAM_B,PSO,CAM,PSO);
    DUMPIMG(CAM_B,LCSO,CAM,LCSO);
    DUMPIMG(CAM_B,PDO,CAM,PDO);

    ptr = (UniDrvImp*)_uni;
    DMA_EN = UNI_READ_REG(ptr, CAM_UNI_TOP_DMA_EN);

    COMPARE(UNI_A,FLKO_A,UNI,UNI_FLKO,flko_a);
    COMPARE(UNI_A,RSSO_A,UNI,UNI_RSSO_A,rsso_a);

    DUMPIMG(UNI_A,UNI_FLKO,UNI,FLKO_A);
    DUMPIMG(UNI_A,UNI_RSSO_A,UNI,RSSO_A);

    DMA_EN = UNI_READ_REG(ptr, CAM_UNI_B_TOP_DMA_EN);

    COMPARE(UNI_B,FLKO_A,UNI,UNI_B_FLKO,flko_b);
    COMPARE(UNI_B,RSSO_A,UNI,UNI_B_RSSO_A,rsso_b);

    DUMPIMG(UNI_B,UNI_B_FLKO,UNI,FLKO_A);
    DUMPIMG(UNI_B,UNI_B_RSSO_A,UNI,RSSO_A);

    LOG_INF("press any key continuous\n");
    if(getchar() < 0) {
        LOG_ERR("getchar error");
    }
    return ret;
}


MINT32 Pattern_BitTrue_4(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    IMEM_BUF_INFO**** pBuf = NULL;
    TestInputInfo *pInputInfo = (TestInputInfo *)inputInfo;
    UINT32 DMA_EN = 0,_tmp=0,_cnt=0, cnt_1=0;
    MUINT32 *pTable = NULL,*pMem = NULL;

    char    szFileName[256];
    FILE*   pFp = NULL;

    static const unsigned char golden_4_aao[] = {
    #include "Emulation/CAM_BIN/Golden/aao_golden.dhex"
    };
    static const unsigned char golden_4_lcso[] = {
    #include "Emulation/CAM_BIN/Golden/lcso_golden.dhex"
    };
    static const unsigned char golden_4_afo_a[] = {
    #include "Emulation/CAM_BIN/Golden/afo_a_golden.dhex"
    };
    static const unsigned char golden_4_afo_b[] = {
    #include "Emulation/CAM_BIN/Golden/afo_b_golden.dhex"
    };

    UNUSED(length);
    UNUSED(_uni);
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
    #undef COMPARE
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
            for (_cnt = 0; _cnt < _tmp; _cnt++) { \
                if ((*pMem) != (*pTable)) {\
                    LOG_ERR("%s bit true fail,@src=%p golden=%p\n",#STR,pMem,pTable);\
                    LOG_ERR("%s bit true fail offset=0x%x 0x%x_0x%x\n",#STR,_cnt,*pMem,*pTable);\
                    if(_cnt > 1) {\
                        LOG_ERR("%s bit true fail,prev1: 0x%x_0x%x\n",#STR,*(pMem-1),*(pTable-1));\
                        LOG_ERR("%s bit true fail,prev2: 0x%x_0x%x\n",#STR,*(pMem-2),*(pTable-2));\
                    }\
                    break;\
                }\
                pMem++;\
                pTable++;\
            }\
            if(_cnt != _tmp){\
                ret++;\
                LOG_ERR("%s bit true fail,%d_%d\n",#STR,_cnt,_tmp);\
            }\
            else{\
                LOG_INF("%s bit true pass\n",#STR);\
            }\
            LOG_INF("###########################\n");\

    #define DUMPIMG(module,STR)\
        _tmp = (DMA_EN & STR##_EN_);\
        cnt_1 = 0;\
        while(_tmp != 0){\
            cnt_1++;\
            _tmp >>= 1;\
        }\
        LOG_INF("%s:%s, VA:0x(%lx), size(%d)", #module,#STR,pBuf[module][cnt_1][0]->virtAddr,pBuf[module][cnt_1][0]->size);\
        LOG_INF("%s:%s XSIZE:%d,YSIZE:%d,STRIDE:%d",#module,#STR,CAM_READ_REG(ptr,CAM_##STR##_XSIZE)+1,\
        CAM_READ_REG(ptr,CAM_##STR##_YSIZE)+1,CAM_READ_BITS(ptr,CAM_##STR##_STRIDE,STRIDE));\
        sprintf(szFileName, "%s/%s_%s.raw" , "/data",#module,#STR);\
        pFp = fopen(szFileName, "wb");\
        if (NULL == pFp ) {\
            LOG_ERR("Can't open file to save image");\
        }\
        else {\
            fwrite((MUINT8 *)pBuf[module][cnt_1][0]->virtAddr, 1, pBuf[module][cnt_1][0]->size, pFp);\
            fflush(pFp);\
            if (0 != fsync(fileno(pFp))) {\
                LOG_ERR("fync fail");\
                fclose(pFp);\
            }else{\
                LOG_INF("Save image: %s", szFileName);\
                fclose(pFp);\
            }\
        }\

    pBuf = (IMEM_BUF_INFO****)pInputInfo->pImemBufs;

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    COMPARE(CAM_A,AAO,aao);
    COMPARE(CAM_A,LCSO,lcso);
    COMPARE(CAM_A,AFO,afo_a);

    DUMPIMG(CAM_A,IMGO);
    DUMPIMG(CAM_A,RRZO);

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    COMPARE(CAM_B,AFO,afo_b);
    LOG_INF("press any key continuous\n");
    if(getchar() < 0){
        LOG_ERR("getchar error");
    }
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

    static const unsigned char golden_5_aao[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/Golden/aao_golden.dhex"
    };
    static const unsigned char golden_5_pso[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/Golden/pso_golden.dhex"
    };
    static const unsigned char golden_5_lcso[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/Golden/lcso_golden.dhex"
    };
    static const unsigned char golden_5_afo_a[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/Golden/afo_a_golden.dhex"
    };
    static const unsigned char golden_5_afo_b[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/Golden/afo_b_golden.dhex"
    };
    UNUSED(length);
    UNUSED(_uni);

#if 0 //code size issue, do not build-in. also, imgo/rrzo's stride is not the same as golden
    static const unsigned char golden_5_imgo_a[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/Golden/imgo_a_golden.dhex"
    };
    static const unsigned char golden_5_imgo_b[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/Golden/imgo_b_golden.dhex"
    };
    static const unsigned char golden_5_rrzo_a[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/Golden/rrzo_a_golden.dhex"
    };
    static const unsigned char golden_5_rrzo_b[] = {
        #include "Emulation/CAM_TWIN_NO_DBN/Golden/rrzo_b_golden.dhex"
    };
#endif
    #undef COMPARE
    #define COMPARE(module,STR,STR2)\
            _tmp = (DMA_EN & STR##_EN_);\
            _cnt = 0;\
            while(_tmp != 0){\
                _cnt++;\
                _tmp >>= 1;\
            }\
            pTable = (MUINT32*)golden_5_##STR2;\
            pMem = (MUINT32*)pBuf[module][_cnt][0]->virtAddr;\
            _tmp = pBuf[module][_cnt][0]->size/sizeof(MUINT32);\
            _cnt = 0;\
            LOG_INF("###########################\n");\
            for (_cnt = 0; _cnt < _tmp; _cnt++) { \
                if ((*pMem) != (*pTable)) {\
                    LOG_ERR("%s bit true fail,@src=%p golden=%p\n",#STR,pMem,pTable);\
                    LOG_ERR("%s bit true fail offset=0x%x 0x%x_0x%x\n",#STR,_cnt,*pMem,*pTable);\
                    if(_cnt > 1) {\
                        LOG_ERR("%s bit true fail,prev1: 0x%x_0x%x\n",#STR,*(pMem-1),*(pTable-1));\
                        LOG_ERR("%s bit true fail,prev2: 0x%x_0x%x\n",#STR,*(pMem-2),*(pTable-2));\
                    }\
                    break;\
                }\
                pMem++;\
                pTable++;\
            }\
            if(_cnt != _tmp){\
                ret++;\
                LOG_ERR("%s bit true fail,%d_%d\n",#STR,_cnt,_tmp);\
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
    COMPARE(CAM_A,PSO,pso);
    COMPARE(CAM_A,LCSO,lcso);
    COMPARE(CAM_A,AFO,afo_a);

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_B]->getPhyObj();
    DMA_EN = CAM_READ_REG(ptr, CAM_CTL_DMA_EN);

    COMPARE(CAM_B,AFO,afo_b);

    LOG_INF("press any key continuous\n");
    if(getchar() < 0) {
        LOG_ERR("getchar error");
    }
    return ret;
}

MINT32 Pattern_BitTrue_7(MUINT32* _ptr,MUINTPTR length,MUINT32* _uni,MUINTPTR inputInfo)
{
    MINT32 ret=0;
    IspDrv* ptr = NULL;
    MBOOL Golden_Record = MFALSE;
    MUINT32 IMGO_Golden, IMGO_checksum;
    MUINT32 RRZO_Golden, RRZO_checksum;
    MUINT32 snrAry = 0;
    inputInfo;

    UNUSED(length);
    UNUSED(_uni);

    #if (LPDVT_USING_CAMSV == 1)
    IspDrvCamsv *pCamsvDrv0, *pCamsvDrv1;

    pCamsvDrv0 = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_0);
    pCamsvDrv0->init("CamSV_0");

    pCamsvDrv1 = (IspDrvCamsv*)IspDrvCamsv::createInstance(CAMSV_1);
    pCamsvDrv1->init("CamSV_1");
    #endif

    //
    ptr = ((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj();

    while(1) {
        #if (LPDVT_USING_CAMSV == 1)
        CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_TG_VF_CON, (CAMSV_READ_REG(pCamsvDrv0, CAMSV_TG_VF_CON) & ~0x1)); //CAMSV_0 VF off
        CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_TG_VF_CON, (CAMSV_READ_REG(pCamsvDrv1, CAMSV_TG_VF_CON) & ~0x1)); //CAMSV_1 VF off
        #endif
        CAM_WRITE_REG(ptr, CAM_TG_VF_CON, 0x00001002); //VF off

        usleep(50*1000);

        #if (LPDVT_USING_CAMSV == 1)
        CAMSV_WRITE_REG(pCamsvDrv0, CAMSV_TG_VF_CON, (CAMSV_READ_REG(pCamsvDrv0, CAMSV_TG_VF_CON) | 0x1)); //CAMSV_0 VF off
        CAMSV_WRITE_REG(pCamsvDrv1, CAMSV_TG_VF_CON, (CAMSV_READ_REG(pCamsvDrv1, CAMSV_TG_VF_CON) | 0x1)); //CAMSV_1 VF off
        #endif
        CAM_WRITE_REG(ptr, CAM_TG_VF_CON, 0x00001003); //VF on

        if(!WaitVsync(ptr)){
            LOG_INF("CAMSYS VSync Pass.\n");
        } else{
            LOG_ERR("CAMSYS VSync Fail, Bit-true Fail.\n");
            LOG_ERR("infinite loop for CVD debug\n");
            while(WaitVsync(ptr)){};
        }

        if(!WaitP1Done(ptr)){
            LOG_INF("CAMSYS P1_DONE Pass.\n");
        } else{
            LOG_ERR("CAMSYS P1_DONE Fail, Bit-true Fail.\n");
            LOG_ERR("infinite loop for CVD debug\n");
            while(WaitP1Done(ptr)){};
        }

        if(!Golden_Record){
            CAM_WRITE_REG(ptr, CAM_DMA_DEBUG_SEL, 0x080B); //debug sel for imgo
            /*Set CTL_DBG_SET*/
            CAM_WRITE_REG(ptr, CAM_CTL_DBG_SET, 0x00FF);
            /*Read CTL_DBG_PORT*/
            IMGO_Golden = CAM_READ_REG(ptr, CAM_CTL_DBG_PORT);

            CAM_WRITE_REG(ptr, CAM_DMA_DEBUG_SEL, 0x0C0B); //debug sel for rrzo
            /*Set CTL_DBG_SET*/
            CAM_WRITE_REG(ptr, CAM_CTL_DBG_SET, 0x00FF);
            /*Read CTL_DBG_PORT*/
            RRZO_Golden = CAM_READ_REG(ptr, CAM_CTL_DBG_PORT);

            LOG_INF("Golden_Checksum IMGO=0x%x, RRZO=0x%x", IMGO_Golden, RRZO_Golden);
            Golden_Record = MTRUE;
        } else {
            CAM_WRITE_REG(ptr, CAM_DMA_DEBUG_SEL, 0x080B); //debug sel for imgo
            /*Set CTL_DBG_SET*/
            CAM_WRITE_REG(ptr, CAM_CTL_DBG_SET, 0x00FF);
            /*Read CTL_DBG_PORT*/
            IMGO_checksum = CAM_READ_REG(ptr, CAM_CTL_DBG_PORT);

            CAM_WRITE_REG(ptr, CAM_DMA_DEBUG_SEL, 0x0C0B); //debug sel for rrzo
            /*Set CTL_DBG_SET*/
            CAM_WRITE_REG(ptr, CAM_CTL_DBG_SET, 0x00FF);
            /*Read CTL_DBG_PORT*/
            RRZO_checksum = CAM_READ_REG(ptr, CAM_CTL_DBG_PORT);

            if (((MUINT64)IMGO_Golden + (MUINT64)RRZO_Golden) == ((MUINT64)IMGO_checksum + (MUINT64)RRZO_checksum)) {
                LOG_INF("CAMSYS Bit-true Pass. (0x%lx_0x%lx)", ((MUINT64)IMGO_Golden + (MUINT64)RRZO_Golden), ((MUINT64)IMGO_checksum + (MUINT64)RRZO_checksum));
            } else {
                LOG_ERR("CAMSYS Bit-true Fail. (0x%lx_0x%lx)", ((MUINT64)IMGO_Golden + (MUINT64)RRZO_Golden), ((MUINT64)IMGO_checksum + (MUINT64)RRZO_checksum));
                return 1;
            }

            if(lpdvt_pdn == 2) {
                LOG_INF("Loop one time");
                lpdvt_pdn = 0;
                break;
            }
        }
    }

    #if (LPDVT_USING_CAMSV == 1)
    LOG_INF("Disable CAMSV0/1 VF");
    CAMSV_WRITE_BITS(pCamsvDrv0, CAMSV_TG_VF_CON, VFDATA_EN, 0);
    CAMSV_WRITE_BITS(pCamsvDrv1, CAMSV_TG_VF_CON, VFDATA_EN, 0);

    pCamsvDrv0->uninit("CamSV_0");
    pCamsvDrv0->destroyInstance();

    pCamsvDrv1->uninit("CamSV_1");
    pCamsvDrv1->destroyInstance();
    #endif

    CAM_WRITE_BITS(((ISP_DRV_CAM**)_ptr)[CAM_A]->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN,0);

#if LPDVT_USING_SENINF == 1
    TS_FakeDrvSensor().powerOff("fake", 1, &snrAry);
    if (length > 1) {
        MUINT32 snrArySub = 1;
        TS_FakeDrvSensor().powerOff("fake1", 1, &snrArySub);
    }
#else
    pSensorHalObj[0]->powerOff("LPDVT", 1, &snrAry);
#endif

    return ret;
}



MINT32 Pattern_release(MUINT32* _ptr,MUINTPTR BufPtr,MUINT32* _uni,MUINTPTR linkpath)
{
    IMEM_BUF_INFO**** pBuf;
    IMemDrv* pImemDrv = NULL;

    pImemDrv = IMemDrv::createInstance();
    pBuf = (IMEM_BUF_INFO****)BufPtr;
    for(MUINT32 j=CAM_A;j<(MAX_UNI_HW_MODULE);j++){
        for(MUINT32 i=0;i<32; i++){
            for(MUINT32 n=0;n<2;n++){
                if(pBuf[j][i][n]->size != 0)
                    pImemDrv->freeVirtBuf(pBuf[j][i][n]);
            }
        }
    }

    pImemDrv->uninit();
    pImemDrv->destroyInstance();

    //
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
        else
            ((UniDrvImp*)_uni)->uninit("Test_IspDrvCam_B");

        ((UniDrvImp*)_uni)->destroyInstance();
    }

    return 0;
}

#define CASE_OP     7   // 5 is for 1: isp drv init|fakeSensor,2:load MMU setting, 3:loading pattern(APMCU or CQ loading). 4:mem allocate + start, 5:stop, 6:deallocate
#define CASE_TC     8

typedef MINT32 (*LDVT_CB)(MUINT32*,MUINTPTR,MUINT32*,MUINTPTR);

LDVT_CB CB_TBL[CASE_TC][CASE_OP] = {
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_0,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_0,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_1,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_1,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_2,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_2,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_3,Pattern_Start_1,Pattern_Stop_1,NULL,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_4,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_4,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_5,Pattern_Start_1,Pattern_Stop_1,Pattern_BitTrue_5,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_6,Pattern_Start_6,Pattern_Stop_6,NULL,Pattern_release},
    {LDVT_DRV_INIT,MMU_INIT,Pattern_Loading_6,Pattern_Start_6,Pattern_Stop_7,Pattern_BitTrue_7,Pattern_release},
};

int IspDrvCam_LDVT(void)
{
    int ret = 0;
    int s;
    MUINT32 test_case;
    ISP_DRV_CAM* pDrvCam = NULL;
    ISP_DRV_CAM* pDrvCam2 = NULL;
    ISP_DRV_CAM* ptr = NULL;
    IMEM_BUF_INFO ****pimgBuf = NULL;
    ISP_DRV_CAM*    pIspDrv[CAM_MAX] = {NULL,NULL};
    UniDrvImp*      UniDrv;
    TestInputInfo   testInput;

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("camsys.lpdvt.pdn", value, "0");
    lpdvt_pdn = atoi(value);

    if(pimgBuf == NULL){
        pimgBuf = (IMEM_BUF_INFO****)malloc(sizeof(IMEM_BUF_INFO***)*(MAX_UNI_HW_MODULE));
        for(MUINT32 i=0;i<(MAX_UNI_HW_MODULE);i++){
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

    if (!lpdvt_pdn) {
        LOG_INF("##############################\n");
        LOG_INF("case 0: UFE-FG\n");
        LOG_INF("case 1: P1 TWIN\n");
        LOG_INF("case 2: P1 MULTI 2UNI\n");
        LOG_INF("case 3: P1TWIN_RCP3\n");
        LOG_INF("case 4: P1 BIN\n");
        LOG_INF("case 5: p1_Twin_no_dbn\n");
        LOG_INF("case 6: LPDVT IMAX Vcore\n");
        LOG_INF("case 7: LPDVT IMAX/MSSV");
        LOG_INF("##############################\n");
        s = getchar();
        test_case = atoi((const char*)&s);
        if(getchar() == EOF) {
            LOG_ERR("getchar is error");
        }

        if(test_case >= CASE_TC){
            LOG_ERR("case num error! cur:%d max:%d\n", test_case, CASE_TC);
            return 1;
        }
    } else {
        test_case = 7;
    }

    LOG_INF("start case:%d\n",test_case);
    switch(test_case){
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            if((ret = CB_TBL[test_case][0]((MUINT32*)pIspDrv,2,(MUINT32*)&UniDrv,CAM_A)) != 0){
                LOG_ERR(" case_%d step_0 fail\n",test_case);
                return 1;
            }

            if((ret = CB_TBL[test_case][1]((MUINT32*)pIspDrv,2,(MUINT32*)UniDrv,CAM_A)) != 0){
                LOG_ERR(" case_%d step_1 fail\n",test_case);
                return 1;
            }

            if((ret = CB_TBL[test_case][2]((MUINT32*)pIspDrv,2,(MUINT32*)UniDrv, (MUINTPTR)&testInput)) != 0){
                LOG_ERR(" case_%d step_2 fail\n",test_case);
                return 1;
            }
            if (test_case == 2) {
                LOG_INF("Case %d multi cam...\n", test_case);
                if ((ret = CB_TBL[test_case][3]((MUINT32*)pIspDrv,2,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_3 fail\n",test_case);
                    return 1;
                }
            }
            else {
                if((ret = CB_TBL[test_case][3]((MUINT32*)pIspDrv,1,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_3 fail\n",test_case);
                    return 1;
                }
            }


            if((ret = CB_TBL[test_case][4]((MUINT32*)pIspDrv,2,(MUINT32*)UniDrv,CAM_A)) != 0){
                LOG_ERR(" case_%d step_4 fail\n",test_case);
                return 1;
            }

            if(CB_TBL[test_case][5] != NULL){
                if((ret = CB_TBL[test_case][5]((MUINT32*)pIspDrv,2,(MUINT32*)UniDrv,(MUINTPTR)&testInput)) != 0){
                    LOG_ERR(" case_%d step_5 fail\n",test_case);
                }
            }
            else{
                LOG_INF("######################################\n");
                LOG_INF("this case_%d support no bit-true flow\n", test_case);
                LOG_INF("######################################\n");
            }

            if((ret = CB_TBL[test_case][6]((MUINT32*)pIspDrv,(MUINTPTR)pimgBuf,(MUINT32*)UniDrv,CAM_A)) != 0){
                LOG_ERR(" case_%d step_6 fail\n",test_case);
                return 1;
            }
            break;
        default:
            LOG_ERR("unsupported case(%d)\n",test_case);
            return 1;
            break;
    }

    for(MUINT32 i=0;i<(MAX_UNI_HW_MODULE);i++){
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
