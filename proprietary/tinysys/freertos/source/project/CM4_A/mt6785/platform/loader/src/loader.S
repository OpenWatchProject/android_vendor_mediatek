/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <loader.h>

  .file "loader.s"
  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

   .section  .isr_vector,"a",%progbits

  .word  _estack
  .word  SCPLoader
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler
  .word  DefExcepHandler

  /* External Interrupts */
  .word DefExcepHandler /*IPC0_IRQHandler */
  .word DefExcepHandler /* IPC1_IRQHandler */
  .word DefExcepHandler /* IPC2_IRQHandler */
  .word DefExcepHandler /* IPC3_IRQHandler */
  .word DefExcepHandler /* SPM_IRQHandler  */
  .word DefExcepHandler /* CIRQ_IRQHandler */
  .word DefExcepHandler /* EINT_IRQHandler */
  .word DefExcepHandler /* PMIC_IRQHandler */
  .word DefExcepHandler /* UART0_IRQHandler*/
  .word DefExcepHandler /* UART1_IRQHandler*/
  .word DefExcepHandler /* I2C0_IRQHandler */
  .word DefExcepHandler /* I2C1_IRQHandler */
  .word DefExcepHandler /* I2C2_IRQHandler */
  .word DefExcepHandler /* CLK_CTRL_IRQHandler*/
  .word DefExcepHandler /* MAD_FIFO_IRQHandler*/
  .word DefExcepHandler /* TIMER0_IRQHandler  */
  .word DefExcepHandler /* TIMER1_IRQHandler  */
  .word DefExcepHandler /* TIMER2_IRQHandler  */
  .word DefExcepHandler /* TIMER3_IRQHandler  */
  .word DefExcepHandler /* TIMER4_IRQHandler  */
  .word DefExcepHandler /* TIMER5_IRQHandler  */
  .word DefExcepHandler /* 21: TIMER_IRQ_STATUS_IRQn */
  .word DefExcepHandler /* 22: UART0_RX_IRQHandler */
  .word DefExcepHandler /* 23: UART1_RX_IRQHandler */
  .word DefExcepHandler /* 24: DMA_IRQHandler */
  .word DefExcepHandler /* 25: AUDIO_IRQHandler */
  .word DefExcepHandler /* 26: FD216_IRQn */
  .word DefExcepHandler /* 27: MD_IRQHandler */
  .word DefExcepHandler /* 28: C2K_IRQn */
  .word DefExcepHandler /* 29: SPI0_IRQn */
  .word DefExcepHandler /* 30: SPI1_IRQn */
  .word DefExcepHandler /* 31: SPI2_IRQn */
  .word DefExcepHandler /* 32 ~ 36 IRQn */
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler /* 37 ~ 41 IRQn */
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler /* 42 ~ 46 IRQn */
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler /* 47 ~ 51 IRQn */
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler /* 52 ~ 56 IRQn */
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler /* 57 ~ 63 IRQn */
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler
  .word DefExcepHandler

    .section  .text

.macro IRQenable
  cpsie  i
.endm

.macro IRQdisable
  cpsid  i
.endm

/*
 * map dram start address (ap view) to 0x2AAB0000
 */
SRAMRemap:
  ldr  r1, =REMAP_VAL1
  and  r1, r1, #0xFFFFFFF0

  ldr  r0, =RegionInfo
  ldr  r0, [r0, #OFF_LOADER_START]
  lsr  r0, r0, #28
  orr  r1, r1, r0

  ldr  r0, =REMAP_CFG1
  str  r1, [r0]

  ldr  r1, =REMAP_VAL2
  ldr  r0, =REMAP_CFG2
  str  r1, [r0]
  ldr  r1, =REMAP_VAL3
  ldr  r0, =REMAP_CFG3
  str  r1, [r0]
  bx  lr

/*
 * r0: tmp
 * r1: tmp
 */
TCMLock:
  ldr  r0, =_sImage
  ldr  r1, =RegionInfo
  ldr  r1, [r1, #OFF_FW_SIZE]
  add  r1, r1, r0
  lsr  r1, r1, #TCM_LOCK_UNIT
  add  r1, r1, #1
  lsl  r1, r1, #TCM_LOCK_SHIFT
  ldr  r0, =TCM_LOCK_CFG
  str  r1, [r0]

  ldr  r0, =SCP_SECURE_CRTL
  ldr  r1, [r0]
  orr  r1, r1, #TCM_EN_BIT
  str  r1, [r0]

#if TCM_FORCE_LOCK
  ldr  r0, =SCP_ONE_TIME_LOCK
  ldr  r1, [r0]
  orr  r1, r1, #TCM_EN_BIT
  str  r1, [r0]
#endif
  bx  lr


SRAMRemapRestore:
  ldr  r1, =REMAP_VAL1
  ldr  r0, =REMAP_CFG1
  str  r1, [r0]

  ldr  r1, =REMAP_VAL2
  ldr  r0, =REMAP_CFG2
  str  r1, [r0]

  ldr  r1, =REMAP_VAL3
  ldr  r0, =REMAP_CFG3
  str  r1, [r0]
  bx  lr

/*
 * r4: dram address
 * r5: tmp
 */
ConvDRAMAddr:
  ldr  r5, =DRAM_REMAP_ADR
  lsr  r5, r5, #28
  bfi  r4, r5, #28, #4
  bx  lr

/*
 * r0: dram start
 * r1: len
 * output r0 = 0 if success
 */
VerifyImg:
  mov  r0, #0
  bx  lr

WdtSetup:
#if WDT_SUPPORT
  ldr  r0, =WDT_INTV_MASK
  ldr  r1, =WDT_INTV
  and  r1, r1, r0
  orr  r1, r1, #WDT_EN
  ldr  r0, =WDT_REG_BASE
  str  r1, [r0]
#endif
  bx  lr

WdtKick:
#if WDT_SUPPORT
  ldr  r0, =WDT_REG_BASE
  ldr  r1, =WDT_TICK
  str  r1, [r0, #4]
#endif
  bx  lr

LogError:
  b  .

    .section  .text.SCPLoader
  .weak  SCPLoader
  .type  SCPLoader, %function
SCPLoader:
  IRQdisable
  /* DRAM remapping */
  bl  SRAMRemap
#if TCM_LOCK_SUPPORT
  /* lock TCM */
  bl TCMLock
#endif

/*
  r0: dram start
  r1: length
  r2: start
  r3: end
 */
  bl  WdtSetup
  ldr  r2, =RegionInfo
  ldr  r4, [r2, #OFF_FW_START]
  bl  ConvDRAMAddr
  mov  r0, r4
  ldr  r1, [r2, #OFF_FW_SIZE]
  bl  VerifyImg
  cmp  r0, #0
  beq  LoadImgInit
  b  LogError

    .size  SCPLoader, .-SCPLoader

EnableFPU:
  ldr  r0, =CPACR
  ldr  r1, [r0]
  orr  r1, r1, #CPACR_VAL
  str  r1, [r0]
  bx  lr


#if MPU_PROTECT
SetupMPURegion:
  bic  r0, r0, #0x1F
  bfi  r0, r1, #0, #4
  orr  r0, r0, #0x10
  ldr  r1, =MPU_RBAR
  str  r0, [r1]

  and  r0, r6, #0x01
  ubfx  r1, r6, #1, #1
  bfi  r0, r1, #28, #1   /*XN bit(always 0)*/
  bfi  r0, r2, #1, #5    /*SIZE*/
  bfi  r0, r3, #24, #3   /*AP*/
  bfi  r0, r4, #16, #6   /*TEX SCB*/
  bfi  r0, r5, #8, #8    /*SRD*/
  ldr  r1, =MPU_RASR
  str  r0, [r1]
  bx  lr

DisableMPURegion:
  and  r0, r0, #0xF
  orr  r0, r0, #0x10
  ldr  r1, =MPU_RBAR
  str  r0, [r1]

  mov  r0, #0
  ldr  r1, =MPU_RASR
  str  r0, [r1]
  bx  lr

SetupMPU:
  ldr  r0, =MPU_TYPE
  ldr  r1, [r0]
  /* if MPU not exists, return it */
  cmp  r1, #0
  bcc  SetVTOR

  /* if MPU exists, disable it first */
  ldr  r1, =MPU_CTRL
  mov  r2, #0
  str  r2, [r1]

  /* region for loader */
  ldr  r0, =LOADER_BASE /* base */
  mov  r1, #7           /* id */
  ldr  r2, =LOADER_SIZE /* size */
  mov  r3, 0x0          /* AP = 0, forbidden */
  mov  r4, 0x2          /* MemAttrib = 2, write-through, no write allocate */
  mov  r5, 0x00         /* no disable sub-region */
  mov  r6, 0x1          /* enable */
  bl  SetupMPURegion
  /* region for MPU register */
  ldr  r0, =MPU_BASE
  mov  r1, #6
  ldr  r2, =MPU_SIZE
  mov  r3, 0x5          /* AP = 5, privilege read-only */
  mov  r4, 0x0          /* MemAttrib = 0, strongly-ordered */
  mov  r5, 0x00
  mov  r6, 0x1
  bl  SetupMPURegion
  mov  r0, #5
  bl  DisableMPURegion
  mov  r0, #4
  bl  DisableMPURegion
  mov  r0, #3
  bl  DisableMPURegion
  ldr  r0, =SRAM_BASE
  mov  r1, #2
  ldr  r2, =SRAM_SIZE
  mov  r3, 0x3          /* AP = 3, all read-write */
  mov  r4, 0x2          /* MemAttrib = 2, write-through, no write allocate */
  mov  r5, 0x00
  mov  r6, 0x1
  bl  SetupMPURegion
  ldr  r0, =SRAM_BASE
  mov  r1, #1
  ldr  r2, =UNUSED_REGION
  mov  r3, 0x0          /* AP = 0, forbidden */
  mov  r4, 0x2          /* MemAttrib = 2, write-through, no write allocate */
  mov  r5, 0x00         /* no disable sub-region */
  mov  r6, 0x1
  bl  SetupMPURegion
  /* region for FreeRTOS */
  ldr  r0, =MEM_BASE
  mov  r1, #0
  ldr  r2, =MEM_SPACE
  mov  r3, 0x3          /* AP = 3, all read-write */
  mov  r4, 0x0          /* MemAttrib = 0, strongly-ordered */
  mov  r5, 0x00
  mov  r6, 0x1
  bl  SetupMPURegion
  b  SetVTOR
#endif
/*MPU_PROTECT end*/

/*
  r0: start
  r1: length
  r2: index
  r3: dram start
 */
LoadImgInit:
  mov  r2, #0
  ldr  r0, =RegionInfo
  ldr  r4, [r0, #OFF_FW_START]
  bl  ConvDRAMAddr
  mov  r3, r4
  ldr  r1, [r0, #OFF_FW_SIZE]
  ldr  r0, =_sImage
  b  LoopLoadImg

/*
  r2: index
  r4: tmp
 */
LoadImg:
  ldr  r4, [r3, r2]
  str  r4, [r0, r2]
  add  r2, r2, #4

LoopLoadImg:
  cmp  r2, r1
  bcc  LoadImg
  bl SRAMRemapRestore
  /* kick watch dog before we leave */
  bl  WdtKick
  bl EnableFPU

 /* NOTE: this will lock FPU register */
#if MPU_PROTECT
  b SetupMPU
#endif

SetVTOR:
  /* set vtor */
  ldr  r0, =_sImage
  ldr  r1, =VTOR
  str  r0, [r1]
  ldr  sp, [r0]
  ldr  r4, [r0, #4]
  IRQenable
#if MPU_PROTECT
   /* Enable MPU
   * NOTE: this will lock FPU register */
  ldr  r1, =MPU_CTRL
  mov  r2, #1
  str  r2, [r1]
#endif
  bx  r4
/*  program end  */

    .section  .text.DefExcepHandler
  .weak  DefExcepHandler
  .type  DefExcepHandler, %function
DefExcepHandler:
    b  LogError

.size  DefExcepHandler, .-DefExcepHandler

    .section .region_info,"a",%progbits
RegionInfo:
  .word 0x00000000  /* loader start on dram */
  .word 0x00000000  /* loader length */
  .word 0x00000000  /* firmware start on dram */
  .word 0x00000000  /* firmware length */
  .word 0x00000000  /* dram start address */
  .word 0x00000000  /* dram code size */
  .word 0x00000000  /* dram backup */
  .word 0x00000000  /* structure size */
  .word 0x00000000  /* log through ap uart */
  .word 0x00000000  /* task Context pointer */
  .word CFG_L1C_ICON    /* L1C setting */
  .word CFG_L1C_DCON    /* L1C setting */
  .word 0x00000000  /* scpctl */
  .word 0x00000000  /* param start address */


