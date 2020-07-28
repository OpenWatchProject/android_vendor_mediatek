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

/* register map */
#define FDVT_START                 (0x0)
#define FDVT_ENABLE                (0x4)
#define FDVT_RS                    (0x8)
#define FDVT_RSCON_BASE_ADR        (0xC)
#define FDVT_RGB2Y0                (0x10)
#define FDVT_RGB2Y1                (0x14)
#define FDVT_INVG0                 (0x18)
#define FDVT_INVG1                 (0x1C)
#define FDVT_INVG2                 (0x20)
#define FDVT_FNUM_0                (0x24)
#define FDVT_FNUM_1                (0x28)
#define FDVT_FNUM_2                (0x2C)
#define FDVT_FNUM_3                (0x30)
#define FDVT_T_FNUM_4              (0x34)
#define FDVT_T_FNUM_5              (0x38)
#define FDVT_T_FNUM_6              (0x3C)
#define FDVT_T_FNUM_7              (0x40)
#define FDVT_FF_NUM_0              (0x44)
#define FDVT_FF_NUM_1              (0x48)
#define FDVT_FF_NUM_2              (0x4C)
#define FDVT_FF_NUM_3              (0x50)
#define FDVT_FF_BASE_ADR_0         (0x54)
#define FDVT_FF_BASE_ADR_1         (0x58)
#define FDVT_FF_BASE_ADR_2         (0x5C)
#define FDVT_FF_BASE_ADR_3         (0x60)
#define FDVT_FF_BASE_ADR_4         (0x64)
#define FDVT_FF_BASE_ADR_5         (0x68)
#define FDVT_FF_BASE_ADR_6         (0x6C)
#define FDVT_FF_BASE_ADR_7         (0x70)
#define FDVT_RMAP_0                (0x74)
#define FDVT_RMAP_1                (0x78)
#define FDVT_FD                    (0x7C)
#define FDVT_FD_CON_BASE_ADR       (0x80)
#define FDVT_GFD                   (0x84)
#define FDVT_LFD                   (0x88)
#define FDVT_GFD_POS_0        	   (0x8C)
#define FDVT_GFD_POS_1        	   (0x90)
#define FDVT_GFD_DET0              (0x94)
#define FDVT_GFD_DET1              (0x98)
#define FDVT_FD_RLT_BASE_ADR       (0x9C)
#define FDVT_LFD_INFO_CTRL_0       (0xA4)
#define FDVT_LFD_INFO_XPOS_0       (0xA8)
#define FDVT_LFD_INFO_YPOS_0       (0xAC)
#define FDVT_LFD_INFO_CTRL_1       (0xB0)
#define FDVT_LFD_INFO_XPOS_1       (0xB4)
#define FDVT_LFD_INFO_YPOS_1       (0xB8)
#define FDVT_LFD_INFO_CTRL_2       (0xBC)
#define FDVT_LFD_INFO_XPOS_2       (0xC0)
#define FDVT_LFD_INFO_YPOS_2       (0xC4)
#define FDVT_LFD_INFO_CTRL_3       (0xC8)
#define FDVT_LFD_INFO_XPOS_3       (0xCC)
#define FDVT_LFD_INFO_YPOS_3       (0xD0)
#define FDVT_LFD_INFO_CTRL_4       (0xD4)
#define FDVT_LFD_INFO_XPOS_4       (0xD8)
#define FDVT_LFD_INFO_YPOS_4       (0xDC)
#define FDVT_LFD_INFO_CTRL_5       (0xE0)
#define FDVT_LFD_INFO_XPOS_5       (0xE4)
#define FDVT_LFD_INFO_YPOS_5       (0xE8)
#define FDVT_LFD_INFO_CTRL_6       (0xEC)
#define FDVT_LFD_INFO_XPOS_6       (0xF0)
#define FDVT_LFD_INFO_YPOS_6       (0xF4)
#define FDVT_LFD_INFO_CTRL_7       (0xF8)
#define FDVT_LFD_INFO_XPOS_7       (0xFC)
#define FDVT_LFD_INFO_YPOS_7       (0x100)
#define FDVT_LFD_INFO_CTRL_8       (0x104)
#define FDVT_LFD_INFO_XPOS_8       (0x108)
#define FDVT_LFD_INFO_YPOS_8       (0x10C)
#define FDVT_LFD_INFO_CTRL_9       (0x110)
#define FDVT_LFD_INFO_XPOS_9       (0x114)
#define FDVT_LFD_INFO_YPOS_9       (0x118)
#define FDVT_LFD_INFO_CTRL_10      (0x11C)
#define FDVT_LFD_INFO_XPOS_10      (0x120)
#define FDVT_LFD_INFO_YPOS_10      (0x124)
#define FDVT_LFD_INFO_CTRL_11      (0x128)
#define FDVT_LFD_INFO_XPOS_11      (0x12C)
#define FDVT_LFD_INFO_YPOS_11      (0x130)
#define FDVT_LFD_INFO_CTRL_12      (0x134)
#define FDVT_LFD_INFO_XPOS_12      (0x138)
#define FDVT_LFD_INFO_YPOS_12      (0x13C)
#define FDVT_LFD_INFO_CTRL_13      (0x140)
#define FDVT_LFD_INFO_XPOS_13      (0x144)
#define FDVT_LFD_INFO_YPOS_13      (0x148)
#define FDVT_LFD_INFO_CTRL_14      (0x14C)
#define FDVT_LFD_INFO_XPOS_14      (0x150)
#define FDVT_LFD_INFO_YPOS_14      (0x154)
#define FDVT_TC_ENABLE_RESULT      (0x158)
#define FDVT_INT_EN                (0x15C)
#define FDVT_SRC_WD_HT             (0x160)
#define FDVT_INT                   (0x168)
#define FDVT_DEBUG_INFO_1          (0x16C)
#define FDVT_DEBUG_INFO_2          (0x170)
#define FDVT_DEBUG_INFO_3          (0x174)
#define FDVT_RESULT                (0x178)
#define FDVT_SPARE_CELL			   (0x198)
#define FDVT_CTRL				   (0x19C)
#define FDVT_VERSION			   (0x1A0)

#define FDVT_FF_NUM_4              (0x1C4)
#define FDVT_FF_NUM_5              (0x1C8)
#define FDVT_FF_NUM_6              (0x1CC)
#define FDVT_FF_NUM_7              (0x1D0)
#define FDVT_FF_NUM_8              (0x1D4)

#define FDVT_FF_BASE_ADR_8         (0x1A4)
#define FDVT_FF_BASE_ADR_9         (0x1A8)
#define FDVT_FF_BASE_ADR_10        (0x1AC)
#define FDVT_FF_BASE_ADR_11        (0x1B0)
#define FDVT_FF_BASE_ADR_12        (0x1B4)
#define FDVT_FF_BASE_ADR_13        (0x1B8)
#define FDVT_FF_BASE_ADR_14        (0x1BC)
#define FDVT_FF_BASE_ADR_15        (0x1C0)
#define FDVT_FF_BASE_ADR_16        (0x1D8)
#define FDVT_FF_BASE_ADR_17        (0x1DC)
//SubBit
//0x0004
#define FF_ENABLE   0x00000100
#define FD_ENABLE   0x00000010
#define RS_ENABLE   0x00000001

#endif