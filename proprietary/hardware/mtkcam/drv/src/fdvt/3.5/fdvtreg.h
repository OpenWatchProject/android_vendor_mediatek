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
#ifndef _FDVTREG_MT6573_
#define _FDVTREG_MT6573_

//register map
#define FDVT_START                 (0x0)
#define FDVT_ENABLE                (0x4)
#define FDVT_RS                    (0x8)
#define FDVT_RSCON_ADR             (0xC)
#define FDVT_RGB2Y0                (0x10)
#define FDVT_RGB2Y1                (0x14)
#define FDVT_INVG0                 (0x18)
#define FDVT_INVG1                 (0x1C)
#define FDVT_INVG2                 (0x20)
#define FD_FNUM_0                  (0x24)
#define FD_FNUM_1                  (0x28)
#define FD_FNUM_2                  (0x2C)
#define FD_FNUM_3                  (0x30)
#define VT_FNUM_0                  (0x34)
#define VT_FNUM_1                  (0x38)
#define VT_FNUM_2                  (0x3C)
#define VT_FNUM_3                  (0x40)
#define FE_FNUM_0                  (0x44)
#define FE_FNUM_1                  (0x48)
#define FE_FNUM_2                  (0x4C)
#define FE_FNUM_3                  (0x50)
#define LD_BADR_0                  (0x54)
#define LD_BADR_1                  (0x58)
#define LD_BADR_2                  (0x5C)
#define LD_BADR_3                  (0x60)
#define LD_BADR_4                  (0x64)
#define LD_BADR_5                  (0x68)
#define LD_BADR_6                  (0x6C)
#define LD_BADR_7                  (0x70)
#define FDVT_RMAP_0                (0x74)
#define FDVT_RMAP_1                (0x78)
#define FDVT_FD_SET                (0x7C)
#define FDVT_FD_CON_BASE_ADR       (0x80)
#define FDVT_GFD                   (0x84)
#define FDVT_LFD                   (0x88)
#define FDVT_GFD_POSITION_0        (0x8C)
#define FDVT_GFD_POSITION_1        (0x90)
#define FDVT_GFD_DET0              (0x94)
#define FDVT_GFD_DET1              (0x98)
#define FDVT_FD_RLT_BASE_ADR       (0x9C)
#define FDVT_TC_RLT_BASE_ADR       (0xA0)
#define LFD_CTRL_0                 (0xA4)
#define LFD_TL_0                   (0xA8)
#define LFD_BR_0                   (0xAC)
#define LFD_CTRL_1                 (0xB0)
#define LFD_TL_1                   (0xB4)
#define LFD_BR_1                   (0xB8)
#define LFD_CTRL_2                 (0xBC)
#define LFD_TL_2                   (0xC0)
#define LFD_BR_2                   (0xC4)
#define LFD_CTRL_3                 (0xC8)
#define LFD_TL_3                   (0xCC)
#define LFD_BR_3                   (0xD0)
#define LFD_CTRL_4                 (0xD4)
#define LFD_TL_4                   (0xD8)
#define LFD_BR_4                   (0xDC)
#define LFD_CTRL_5                 (0xE0)
#define LFD_TL_5                   (0xE4)
#define LFD_BR_5                   (0xE8)
#define LFD_CTRL_6                 (0xEC)
#define LFD_TL_6                   (0xF0)
#define LFD_BR_6                   (0xF4)
#define LFD_CTRL_7                 (0xF8)
#define LFD_TL_7                   (0xFC)
#define LFD_BR_7                   (0x100)
#define LFD_CTRL_8                 (0x104)
#define LFD_TL_8                   (0x108)
#define LFD_BR_8                   (0x10C)
#define LFD_CTRL_9                 (0x110)
#define LFD_TL_9                   (0x114)
#define LFD_BR_9                   (0x118)
#define LFD_CTRL_10                (0x11C)
#define LFD_TL_10                  (0x120)
#define LFD_BR_10                  (0x124)
#define LFD_CTRL_11                (0x128)
#define LFD_TL_11                  (0x12C)
#define LFD_BR_11                  (0x130)
#define LFD_CTRL_12                (0x134)
#define LFD_TL_12                  (0x138)
#define LFD_BR_12                  (0x13C)
#define LFD_CTRL_13                (0x140)
#define LFD_TL_13                  (0x144)
#define LFD_BR_13                  (0x148)
#define LFD_CTRL_14                (0x14C)
#define LFD_TL_14                  (0x150)
#define LFD_BR_14                  (0x154)
#define FDVT_RESULT                (0x158)
#define FDVT_INT_EN                (0x15C)
#define FDVT_SRC_WD_HT             (0x160)
#define FDVT_SRC_IMG_BASE_ADDR     (0x164)
#define FDVT_INT                   (0x168)
#define DEBUG_INFO_1               (0x16C)
#define DEBUG_INFO_2               (0x170)
#define DEBUG_INFO_3               (0x174)
#define FDVT_RESULTNUM             (0x178)

//SubBit
//0x0004
#define FF_ENABLE   0x00000100
#define FD_ENABLE   0x00000010
#define RS_ENABLE   0x00000001

#endif