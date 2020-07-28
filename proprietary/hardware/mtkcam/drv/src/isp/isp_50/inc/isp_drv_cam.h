/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _ISP_DRV_CAM_H_
#define _ISP_DRV_CAM_H_
/**
    this module contains CAM_A, CAM_B in abstract method + UNI_A cq
    note:
        UNI_A phy is not included
*/
#include <isp_drv.h>
#include <imem_drv.h>
#include "uni_drv.h"

/**
    patch for hw design prob:
    under twin mode, dmx_id/dmx_sel have no DB, need to use master cam's timing to ctrl slave cam's CQ.
    (setting is only valid at  v-blanking )
*/
#define TWIN_CQ_SW_WORKAROUND   1

/**
    descriptor range & dummy reg for descriptor
*/
#define CQ_DES_RANGE    0xFFFF
#define CQ_DUMMY_REG    0x4ffc
///////////////////////////////////////////////////////////
/**
    CAM CQ descriptor
    its a abstract descriptor , cam_a & cam_b use the same enum
*/
typedef enum {
    //put slave cam's ctrl 1st, for pipe master cam & slave cam.
    TWIN_CQ0_BASEADDRESS = 0,         // 1    6198   //must be allocated before salve cam's descriptor which is for salve cam's cq-trig
    TWIN_CQ1_BASEADDRESS,               // 1    61A4   //must be allocated before salve cam's descriptor which is for salve cam's cq-trig
    TWIN_CQ10_BASEADDRESS,             // 1    6210   //must be allocated before salve cam's descriptor which is for salve cam's cq-trig
#if TWIN_CQ_SW_WORKAROUND
    CAM_CQ0_EN_,                   // 1    4194/6194   //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    CAM_CQ0_TRIG_,               // 1    4000/6000
    CAM_CQ1_EN_,                   // 1    41A0/61A0   //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    CAM_CQ1_TRIG_,               // 1    4000/6000
    CAM_CQ10_EN_,                 // 1    420C/620C   //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    CAM_CQ10_TRIG_,             // 1    4000/6000
#endif
    CAM_CTL_EN_,             // 2    4004~4008
    CAM_CTL_FMT_,                // 1    400C
    CAM_CTL_SEL_,                // 1    4010
    CAM_CTL_MISC_,               // 1    4014
    CAM_CTL_EN2_,                // 1    4018
    CAM_CTL_INT_EN_,             // 1    4020
    CAM_CTL_INT2_EN_,            // 1    4030
    CAM_CTL_DONE_SEL_,           // 3    4044~404c
    CAM_TWIN_INFO,               // 2    4050~4054
    CAM_CQ_COUNTER,              // 1    4058
    CAM_CTL_SW_DONE_SEL_,        // 1    405C
    CAM_CTL_INT3_EN_,            // 1    40C0
    CAM_FBC_IMGO_,               // 1    4110
    CAM_FBC_RRZO_,               // 1    4118
    CAM_FBC_UFEO_,               // 1    4120
    CAM_FBC_LCSO_,               // 1    4128
    CAM_FBC_AFO_,                 // 1    4130
    CAM_FBC_AAO_,                // 1    4138
    CAM_FBC_PDO_,                // 1    4140
    CAM_FBC_PSO_,                // 1    4148
    CAM_FBC_FLKO_,               // 1    4150
    CAM_FBC_LMVO_,               // 1    4158
    CAM_FBC_RSSO_,               // 1    4160
    CAM_FBC_UFGO_,               // 1    4168
    CAM_CTL_DMA_V_FLIP_,         // 1    500C
    CAM_CTL_SPECIAL_FUN_,        // 1    5018
    //DMA CON1 & CON2 & CON3 can't be wriitten by CQ
    CAM_DMA_IMGO_BA,             // 1    5020
    CAM_DMA_IMGO_BA_OFST,        // 1    5028
    CAM_DMA_IMGO_,               // 3    5030~5038
    CAM_DMA_IMGO_CROP_,          // 1    5048
    CAM_DMA_RRZO_BA,             // 1    5050
    CAM_DMA_RRZO_BA_OFST,        // 1    5058
    CAM_DMA_RRZO_,               // 3    5060~5068
    CAM_DMA_RRZO_CROP_,          // 1    5078
    CAM_DMA_AAO_BA,              // 1    5080
    CAM_DMA_AAO_,                // 3    5090~5098
    CAM_DMA_AFO_BA,              // 1    50B0
    CAM_DMA_AFO_BA_OFST,         // 1    50B8
    CAM_DMA_AFO_,                // 3    50C0~50C8
    CAM_DMA_LCSO_BA,             // 1    50E0
    CAM_DMA_LCSO_BA_OFST,        // 1    50E8
    CAM_DMA_LCSO_,               // 3    50F0~50F8
    CAM_DMA_UFEO_BA,             // 1    5110
    CAM_DMA_UFEO_BA_OFST,        // 1    5118
    CAM_DMA_UFEO_,               // 3    5120~5128
    CAM_DMA_PDO_BA,              // 1    5140
    CAM_DMA_PDO_BA_OFST,         // 1    5148
    CAM_DMA_PDO_,                // 3    5150~5158
    CAM_DMA_PSO_BA,              // 1    5260
    CAM_DMA_PSO_BA_OFST,         // 1    5264
    CAM_DMA_PSO_,                // 3    5270~5278
    CAM_DMA_FLKO_BA,             // 1    52C0
    CAM_DMA_FLKO_BA_OFST,        // 1    52C4
    CAM_DMA_FLKO_,               // 3    52D0~52D8
    CAM_DMA_LMVO_BA,             // 1    5290
    CAM_DMA_LMVO_BA_OFST,        // 1    5294
    CAM_DMA_LMVO_,               // 3    52A0~52A8
    CAM_DMA_RSSO_A_BA,           // 1    52F0
    CAM_DMA_RSSO_A_BA_OFST,      // 1    52F4
    CAM_DMA_RSSO_A_,             // 3    5300~5308
    CAM_DMA_UFGO_BA,             // 1    5320
    CAM_DMA_UFGO_BA_OFST,        // 1    5324
    CAM_DMA_UFGO_,               // 3    5330~5338
    CAM_DMA_BPCI_,               // 7    5170~5188
    CAM_DMA_CACI_,               // 7    51A0~51B8
    CAM_DMA_LSCI_,               // 7    51D0~51E8
    CAM_DMA_PDI_,                // 12   5230~525C
    CAM_DMA_RSV1_,               // 1    53B0
    CAM_MAGIC_NUM_,              // 1    53B8
    CAM_DATA_PAT_,               // 1    53BC
    CAM_DMA_RSV6_,               // 1    53C4
    CAM_DMA_FH_EN_,              // 1    5400
    CAM_DMA_FH_IMGO_,            // 1    5404
    CAM_DMA_FH_IMGO_SPARE_,      // 15   5434~546C
    CAM_DMA_FH_RRZO_,            // 1    5408
    CAM_DMA_FH_RRZO_SPARE_,      // 15   5474~54AC
    CAM_DMA_FH_AAO_,             // 1    540C
    CAM_DMA_FH_AAO_SPARE_CQ0_,   // 8    54B4
    CAM_DMA_FH_AAO_SPARE_,       // 3   54D0~54DC
    CAM_DMA_FH_AFO_,             // 1    5410
    CAM_DMA_FH_AFO_SPARE_CQ0_,   // 8    54F4
    CAM_DMA_FH_AFO_SPARE_,       // 3   5510~551C
    CAM_DMA_FH_LCSO_,            // 1    5414
    CAM_DMA_FH_LCSO_SPARE_,      // 15   5534~556C
    CAM_DMA_FH_UFEO_,            // 1    5418
    CAM_DMA_FH_UFEO_SPARE_,      // 15   5574~55AC
    CAM_DMA_FH_PDO_,             // 1    541C
    CAM_DMA_FH_PDO_SPARE_CQ0_,   // 8    55B4~55CC
    CAM_DMA_FH_PDO_SPARE_,       // 3    55D0~55DC
    CAM_DMA_FH_PSO_,             // 1    5420
    CAM_DMA_FH_PSO_SPARE_CQ0_,   // 1    55F4
    CAM_DMA_FH_PSO_SPARE_,       // 15   55F8~5630
    CAM_DMA_FH_FLKO_,            // 1    5428
    CAM_DMA_FH_FLKO_SPARE_CQ0_,  // 1    5674
    CAM_DMA_FH_FLKO_SPARE_,      // 14   5678~56AC
    CAM_DMA_FH_LMVO_,            // 1    5424
    CAM_DMA_FH_LMVO_SPARE_,      // 7   5634~554C
    CAM_DMA_FH_LMVO_SPARE_2,    // 5   5654~5564
    CAM_DMA_FH_RSSO_A_,          // 1    542C
    CAM_DMA_FH_RSSO_A_SPARE_,    // 3   56B4~56BC
    CAM_DMA_FH_RSSO_A_SPARE_2,    // 4   56E0~56EC
    CAM_DMA_FH_RSSO_A_SPARE_3,    // 5   56F4~5704
    CAM_DMA_FH_UFGO_,            // 1    5430
    CAM_DMA_FH_UFGO_SPARE_,      // 15   5714~574C
    CAM_CQ_THRE0_ADDR_,          // 1    4198
    CAM_CQ_THRE0_SIZE_,          // 1    419C
    CAM_CQ_THRE1_ADDR_,          // 1    41A4
    CAM_CQ_THRE1_SIZE_,          // 1    41A8
    CAM_CQ_THRE2_ADDR_,          // 1    41B0
    CAM_CQ_THRE2_SIZE_,          // 1    41B4
    CAM_CQ_THRE3_ADDR_,          // 1    41BC
    CAM_CQ_THRE3_SIZE_,          // 1    41C0
    CAM_CQ_THRE4_ADDR_,          // 1    41C8
    CAM_CQ_THRE4_SIZE_,          // 1    41CC
    CAM_CQ_THRE5_ADDR_,          // 1    41D4
    CAM_CQ_THRE5_SIZE_,          // 1    41D8
    CAM_CQ_THRE6_ADDR_,          // 1    41E0
    CAM_CQ_THRE6_SIZE_,          // 1    41E4
    CAM_CQ_THRE7_ADDR_,          // 1    41EC
    CAM_CQ_THRE7_SIZE_,          // 1    41F0
    CAM_CQ_THRE8_ADDR_,          // 1    41F8
    CAM_CQ_THRE8_SIZE_,          // 1    41FC
    CAM_CQ_THRE9_ADDR_,          // 1    4204
    CAM_CQ_THRE9_SIZE_,          // 1    4208
    CAM_CQ_THRE10_ADDR_,         // 1    4210
    CAM_CQ_THRE10_SIZE_,         // 1    4214
    CAM_CQ_THRE11_ADDR_,         // 1    421C
    CAM_CQ_THRE11_SIZE_,         // 1    4220
    CAM_CQ_THRE12_ADDR_,         // 1    4228
    CAM_CQ_THRE12_SIZE_,         // 1    422C
    CAM_ISP_DMX_,                // 3    4330~4338
    CAM_ISP_PBN_,                // 3    4AA0~4AA8
    CAM_ISP_DBN_,                // 2    4A90~4A94
    CAM_ISP_BIN_,                // 3    4A70~4A78
    CAM_ISP_DBS_,                // 13   4AC0~4AF0
    CAM_ISP_OBC_,                // 8    43F0~440C
    CAM_ISP_RMG_,                // 3    4350~4358
    CAM_ISP_BNR_,                // 32   4420~449C
    CAM_ISP_STM_,                // 2    44A0~44A4
    CAM_ISP_SCM_,                // 2    44B0~44B4
    CAM_ISP_RMM_,                // 9    4390~43B0
    CAM_ISP_BMX_,                // 3    4580~4588
    CAM_ISP_LSC_,                // 11   45D0~45F8
    CAM_ISP_RCP_,                // 2    4800~4804
    CAM_ISP_RPG_,                // 6    44C0~44D4
    CAM_ISP_RRZ_,                // 14   44E0~4514
    CAM_ISP_RMX_,                // 3    4540~4548
    CAM_ISP_SGG5_,               // 3    4CB0~4CB8
    CAM_ISP_UFE_,                // 1    4D20
    CAM_ISP_RCP3_,               // 2    4AB0~4AB4
    CAM_ISP_SGM_,                // 12   4FA0~4FCC
    CAM_ISP_CPG_,                // 6    49E0~49F4
    CAM_ISP_QBN1_,               // 1    49D0
    CAM_ISP_PS_AWB_,          // 8   4BD0~4BEC
    CAM_ISP_PS_AE_,             // 6   4BF0~4C04
    CAM_ISP_SL2F_,               // 12   4B00~4B2C
    CAM_ISP_PCP_CROP_,           // 2    4B80~4B84
    CAM_ISP_SGG2_,               // 3    4B90~4B98
    CAM_ISP_SL2J_,               // 12   4B40~4B6C
    CAM_ISP_HLR_,                // 12   4710~473C
    CAM_ISP_PSB_,                // 2    4BA0~4BA4
    CAM_ISP_PDE_,                // 2    4BB0~4BB4
    CAM_ISP_LCS_,                // 18   4790~47D4
    CAM_ISP_AE_,                 // 29   4900~4970
    CAM_ISP_VBN_,                // 4    4A30~4A3C
    CAM_ISP_AMX_,                // 3    4A50~4A58
    CAM_ISP_AWB_,                // 42   4830~48D4
    CAM_ISP_SGG1_,               // 3    4810~4818
    CAM_ISP_AF_,                 // 47   4610~46C8
    CAM_ISP_UFEG_,               // 1    45C0
    CAM_ISP_RMB_,                // 1    4BC0
    CAM_ISP_SGG3_,               // 3   4C40~4C48
    CAM_ISP_FLK_A_,             // 4   4C50~4C5C
    CAM_ISP_LMV_,                 // 9   4C70~4C90
    CAM_ISP_HDS_,                // 1    4CC0
    CAM_ISP_RSS_A_,            // 9    4CE0~4D00
    CAM_ISP_ADBS_,              // 21  4D50~4DA0
    CAM_ISP_DCPN_,              // 8    4DB0~4DCC
    CAM_ISP_CPN_,                // 16  4DF0~4E2C

    //UNI
    UNI_CAM_XMX_SET_,       // 1   0x13000
    UNI_CAM_XMX_CLR_,      // 1   0x33000
    UNI_CTL_EN_,                 // 2    3010~3014
    UNI_CTL_FMT_,                // 1    301C
    UNI_DMA_V_FLIP_,             // 1    3104
    UNI_DMA_RAWI_,               // 7    3120~3138
    UNI_ISP_UNP2_,               // 1    3200
    CAM_DUMMY_,                  // 1    4ffc
    CAM_CTL_FBC_RCNT_INC_,       // 1    406C   //must be allocated after all FBC_en descirptor
    CAM_CQ_EN_,                  // 1    4160
    CAM_NEXT_Thread_,            // 1    must at the end of descriptor  , or apb will be crashed
    CAM_CAM_END_,                                    //must be kept at the end of descriptor
    CAM_CAM_MODULE_MAX
}E_CAM_MODULE;

#define RingBuf_MODULE_MAX   (4+1)  // +1 for cq inner baseaddr when high-speed mode

//special deifne
//write 2 cq inner address directly!!!!!
#define CAM_CQ_THRE0_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x14198           // for CQ access only
#define CAM_CQ_THRE1_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x141A4           // for CQ access only
#define CAM_CQ_THRE2_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x141B0           // for CQ access only
#define CAM_CQ_THRE3_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x141BC           // for CQ access only
#define CAM_CQ_THRE4_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x141C8           // for CQ access only
#define CAM_CQ_THRE5_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x141D4           // for CQ access only
#define CAM_CQ_THRE6_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x141E0           // for CQ access only
#define CAM_CQ_THRE7_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x141EC           // for CQ access only
#define CAM_CQ_THRE8_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x141F8           // for CQ access only
#define CAM_CQ_THRE9_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x14204           // for CQ access only
#define CAM_CQ_THRE10_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x14210           // for CQ access only
#define CAM_CQ_THRE11_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x1421C           // for CQ access only
#define CAM_CQ_THRE12_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x14228           // for CQ access only
///////////////////////////////////////////////////////////////////////////


/**
    RW marco, for coding convenience

    note1: cam + uni module only (uni is included because of cq programming range.

            if __offset over 0x1000, its a special design for CQ  baseaddr.
            target reg address is at current offset - 0x1000 + 0x8000. (0x1000 is for roll back, 0x8000 is mapping to set&clr domain.
    note2: caller here don't care, CAM_A or CAM_B r all acceptable!
*/


//user no need to write reg address, drv will fill the address automatically
#define CAM_BURST_READ_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    ret=IspDrvPtr->readRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,CAM_A);\
    ret;\
})

//user no need to write reg address, drv will fill the address automatically
#define CAM_BURST_WRITE_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL __ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    __ret=IspDrvPtr->writeRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,CAM_A);\
    __ret;\
})

#define CAM_READ_REG(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readReg(__offset,CAM_A);\
})

#define CAM_READ_BITS(IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,CAM_A);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})

#define CAM_WRITE_REG(IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->writeReg(__offset, Value,CAM_A);\
}while(0);

#define CAM_WRITE_BITS(IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = IspDrvPtr->readReg(__offset,CAM_A);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    IspDrvPtr->writeReg(__offset, ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw,CAM_A);\
}while(0);

#define CAM_REG_ADDR(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    __offset;\
})

///////////////////////////////////////////////////////////
/**
    CAM hw CQ
*/
typedef enum
{
    ISP_DRV_CQ_THRE0 = 0,
    ISP_DRV_CQ_THRE1,
    ISP_DRV_CQ_THRE2,
    ISP_DRV_CQ_THRE3,
    ISP_DRV_CQ_THRE4,
    ISP_DRV_CQ_THRE5,
    ISP_DRV_CQ_THRE6,
    ISP_DRV_CQ_THRE7,
    ISP_DRV_CQ_THRE8,
    ISP_DRV_CQ_THRE9,
    ISP_DRV_CQ_THRE10,
    ISP_DRV_CQ_THRE11,
    ISP_DRV_CQ_THRE12,
    ISP_DRV_CAM_BASIC_CQ_NUM,    //baisc set, pass1 cqs
    ISP_DRV_CQ_NONE = ISP_DRV_CAM_BASIC_CQ_NUM
}E_ISP_CAM_CQ;

/**
    CQ trig mode
*/
typedef enum
{
    CQ_SINGLE_IMMEDIATE_TRIGGER = 0,
    CQ_SINGLE_EVENT_TRIGGER,
    CQ_CONTINUOUS_EVENT_TRIGGER,
    CQ_TRIGGER_MODE_NUM
}E_ISP_CAM_CQ_TRIGGER_MODE;


/**
    CQ trig source
*/
typedef enum
{
    CQ_TRIG_BY_START = 0,
    CQ_TRIG_BY_PASS1_DONE,
    CQ_TRIG_BY_IMGO_DONE,
    CQ_TRIG_BY_RRZO_DONE,
    CQ_TRIG_SRC_NUM,
    CQ_TRIG_BY_NONE = CQ_TRIG_SRC_NUM
}E_ISP_CAM_CQ_TRIGGER_SOURCE;

///////////////////////////////////////////////////////////
/**
    CAM bufctrl CmdQ special descriptor, and ob special descriptor
*/
typedef enum{
    BUF_CTRL_ENQUE  = 0,
    BUF_CTRL_ENQUE_HIGH_SPEED = 1,
    BUF_CTRL_CLEAR,
    SET_FH_SPARE,
    GET_RING_DEPTH,
}E_BUF_CTRL;

typedef struct{
    E_BUF_CTRL      ctrl;
    MUINT32         dma_PA;
    MUINT32         dma_FH_PA;
    MUINT32*        pDma_fh_spare;
}CQ_RingBuf_ST;

typedef struct _cq_cmd_st_CAM_
{
    unsigned int descriptor;
    unsigned int data;
}CQ_CMD_ST_CAM;

typedef struct _cq_info_rtbc_st_CAM_
{
    CQ_CMD_ST_CAM   dma;
    CQ_CMD_ST_CAM   dma_fh;
    CQ_CMD_ST_CAM   fh_spare;
    CQ_CMD_ST_CAM   next_Thread_addr;
    CQ_CMD_ST_CAM   next_Thread_addr_inner;
    CQ_CMD_ST_CAM   end;
    MUINT32         dma_base_pa;
    MUINT32         dma_fh_base_pa;
    MUINT32         dma_fh_spare[CAM_IMAGE_HEADER];
}CQ_INFO_RTBC_ST_CAM;

typedef struct _cq_ring_cmd_st_CAM_
{
    CQ_INFO_RTBC_ST_CAM cq_rtbc;
    unsigned long next_pa;
    struct _cq_ring_cmd_st_CAM_ *pNext;
}CQ_RING_CMD_ST_CAM;

typedef struct _cq_rtbc_ring_st_CAM_
{
    CQ_RING_CMD_ST_CAM rtbc_ring[MAX_RING_SIZE];   //ring depth: sw maximum: 16 [hw maximus is depended on hw bits]
    unsigned int   dma_ring_size;
}CQ_RTBC_RING_ST_CAM;

//CQ0B for AE smoothing, set obc_gain0~3
typedef struct _cq0b_info_rtbc_st_CAM_
{
    CQ_CMD_ST_CAM ob;
    CQ_CMD_ST_CAM end;
}CQ0B_INFO_RTBC_ST_CAM;

typedef struct _cq0b_ring_cmd_st_CAM_
{
    CQ0B_INFO_RTBC_ST_CAM cq0b_rtbc;
    unsigned long next_pa;
    struct _cq0b_ring_cmd_st_CAM_ *pNext;
}CQ0B_RING_CMD_ST_CAM;

typedef struct _cq0b_rtbc_ring_st_CAM_
{
    CQ0B_RING_CMD_ST_CAM rtbc_ring;
}CQ0B_RTBC_RING_ST_CAM;

//////////////////////////////////////////////////////////////////
/**
    vir cq definition
*/
#define CmdQ_Cache  0   //CmdQ using by Cache is 1, Non-cache is 0
#define Max_PageNum 64  //e.g. nBurst * nDup <= Max_PageNum
/**
    define the order of vir_reg.
    if 1
        cam 1st , uni 2nd
    else if 0
        uni 1st . cam 2nd
*/
#define Vir_Buf_Order   1
/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
//control range: CAM + UNI_VIR only
class ISP_DRV_CAM : public IspDrvVir, public IspDrv
{
    public:
        ~ISP_DRV_CAM(){};
        ISP_DRV_CAM(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx);
    public:

        static  IspDrvVir*  createInstance(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx,const char* userName);

        virtual void    destroyInstance(void);


        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   start(void);
        virtual MBOOL   stop(void);

        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq);
        virtual MBOOL   registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq);
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq);

        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);

        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);

        virtual MBOOL   cqAddModule(MUINT32 moduleId);
        virtual MBOOL   cqAddModule(MUINT32 moduleId, ISP_HW_MODULE slave_cam);
        virtual MBOOL   cqAddModule(MUINT32 moduleId, MUINT32 extModuleId, ISP_HW_MODULE slave_cam=CAM_MAX);
        virtual MBOOL   cqDelModule(MUINT32 moduleId);
        virtual MBOOL   cqNopModule(MUINT32 moduleId);
        virtual MBOOL   cqApbModule(MUINT32 moduleId);
        virtual MBOOL   dumpCQTable(void);


        virtual MUINT32*    getCQDescBufPhyAddr(void);
        virtual MUINT32*    getCQDescBufVirAddr(void);
                MUINT32     getCQDescBufSize(void);
                MINT32      getCQIonFD(void);
        virtual MUINT32*    getIspVirRegPhyAddr(void);
        virtual MUINT32*    getIspVirRegVirAddr(void);
                MUINT32     getIspVirRegSize(void);
                MINT32      getIspVirIonFD(void);

        virtual MBOOL       getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize);
        virtual MBOOL       flushCmdQ(void);

        IspDrv*             getPhyObj(void);

        MBOOL               getCurObjInfo(ISP_HW_MODULE* p_module,E_ISP_CAM_CQ* p_cq,MUINT32* p_page);
        MBOOL               DumpReg(MBOOL bPhy);

        //for CQ those descriptor is static, not programable
        MUINT32             cqRingBuf(CQ_RingBuf_ST *pBuf_ctrl);

        //set cq in fixed format in which each reg have it's own descriptor with data length:1 only.  e.g.:CQ11...
        MBOOL               CQ_SetContent(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count);

        //read cq registers of each CqModule
        MBOOL               readCQModuleInfo(MUINT32 moduleId, void **ptr, MUINT32 size);

    private:
                MBOOL   FSM_CHK(MUINT32 op,const char* caller);
                //
                MBOOL   CQ_Allocate_method1(MUINT32 step);
                MBOOL   CQ_Allocate_method2(MUINT32 step);
                //
                MBOOL   updateEnqCqRingBuf(CQ_RingBuf_ST *pBuf_ctrl);
                MUINT32*    update_FH_Spare(CQ_RingBuf_ST *pBuf_ctrl);

                MUINT32 VirReg_OFFSET(MUINT32 hwModule);
                MBOOL   VirReg_ValidRW(void);
                MUINT32 twinPath_CQ_Ctrl(MUINT32 moduleId, ISP_HW_MODULE slave_cam); //this is for master cam to control salve cam under twin mode
    public:
        ISP_HW_MODULE   m_HWmasterModule; //To get system time of master when run twin.

    private:
        volatile MINT32         m_UserCnt;
        mutable android::Mutex           IspRegMutex;
        //
        MUINT32*        m_pIspVirRegAddr_va;
        MUINT32*        m_pIspVirRegAddr_pa;

        //
        ISP_DRV_CQ_CMD_DESC_STRUCT*         m_pIspDescript_vir;
        MUINT32*                            m_pIspDescript_phy;

        //
        MUINT32         m_FSM;
        //imem
        IMemDrv*        m_pMemDrv;
        IMEM_BUF_INFO   m_ispVirRegBufInfo;
        IMEM_BUF_INFO   m_ispCQDescBufInfo;

        E_ISP_CAM_CQ    m_CQ;
        ISP_HW_MODULE   m_HWModule;
        ISP_HW_MODULE   m_Module;           //phy + virtual hw module
        MUINT32         m_pageIdx;
        char            m_useName[32];

        IspDrv*         m_pIspDrvImp;       //ptr for access CAM_A/CAM_B ...etc. phy
};





#endif //_ISP_DRV_CAM_H_
