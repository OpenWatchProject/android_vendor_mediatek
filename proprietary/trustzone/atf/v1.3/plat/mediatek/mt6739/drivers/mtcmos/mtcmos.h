/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __MTCMOS_H__
#define __MTCMOS_H__

/*
 * This function will turn off all the little core's power except cpu 0. The
 * cores in cluster 0 are all powered when the system power on. The System
 * Power Manager (SPM) will do nothing if it found the core's power was on
 * during CPU_ON psci call.
 */
void mtcmos_little_cpu_off(void);
#define spm_read(addr)          mmio_read_32(addr)
#define spm_write(addr, val)    mmio_write_32(addr, val)

/* Define MTCMOS power control */
#define PWR_RST_B                        (0x1 << 0)
#define PWR_ISO                          (0x1 << 1)
#define PWR_ON                           (0x1 << 2)
#define PWR_ON_2ND                       (0x1 << 3)
#define PWR_CLK_DIS                      (0x1 << 4)
#define SRAM_CKISO                       (0x1 << 5)
#define SRAM_ISOINT_B                    (0x1 << 6)
#define SLPB_CLAMP                       (0x1 << 7)

/* Define MTCMOS Bus Protect Mask */
#define MFG1_PROT_BIT_MASK               ((0x1 << 4) \
					  |(0x1 << 5) \
					  |(0x1 << 6))
#define MFG1_PROT_BIT_ACK_MASK           ((0x1 << 4) \
					  |(0x1 << 5) \
					  |(0x1 << 6))
#define C2K_PROT_BIT_MASK                ((0x1 << 22) \
					  |(0x1 << 23) \
					  |(0x1 << 24) \
					  |(0x1 << 25))
#define C2K_PROT_BIT_ACK_MASK            ((0x1 << 22) \
					  |(0x1 << 23) \
					  |(0x1 << 24) \
					  |(0x1 << 25))
#define MD1_PROT_BIT_MASK                ((0x1 << 16) \
					  |(0x1 << 17) \
					  |(0x1 << 18) \
					  |(0x1 << 19) \
					  |(0x1 << 21) \
					  |(0x1 << 25) \
					  |(0x1 << 28))
#define MD1_PROT_BIT_ACK_MASK            ((0x1 << 16) \
					  |(0x1 << 17) \
					  |(0x1 << 18) \
					  |(0x1 << 19) \
					  |(0x1 << 21) \
					  |(0x1 << 25))
#define DPY_CH0_PROT_BIT_MASK            ((0x1 << 0))
#define DPY_CH0_PROT_BIT_ACK_MASK        ((0x1 << 0))
#define DPY_CH1_PROT_BIT_MASK            ((0x1 << 1))
#define DPY_CH1_PROT_BIT_ACK_MASK        ((0x1 << 1))
#define DPY_CH2_PROT_BIT_MASK            ((0x1 << 2))
#define DPY_CH2_PROT_BIT_ACK_MASK        ((0x1 << 2))
#define DPY_CH3_PROT_BIT_MASK            ((0x1 << 3))
#define DPY_CH3_PROT_BIT_ACK_MASK        ((0x1 << 3))
#define INFRA_PROT_BIT_MASK              ((0x1 << 14) \
					  |(0x1 << 15))
#define INFRA_PROT_BIT_ACK_MASK          ((0x1 << 14) \
					  |(0x1 << 15))
#define INFRA_PROT_BIT_2ND_MASK          ((0x1 << 8) \
					  |(0x1 << 9) \
					  |(0x1 << 10) \
					  |(0x1 << 25))
#define INFRA_PROT_BIT_ACK_2ND_MASK      ((0x1 << 8) \
					  |(0x1 << 9) \
					  |(0x1 << 10) \
					  |(0x1 << 25))
#define INFRA_PROT_BIT_3RD_MASK          ((0x1 << 0) \
					  |(0x1 << 1) \
					  |(0x1 << 16) \
					  |(0x1 << 17) \
					  |(0x1 << 24) \
					  |(0x1 << 25))
#define INFRA_PROT_BIT_ACK_3RD_MASK      ((0x1 << 0) \
					  |(0x1 << 1) \
					  |(0x1 << 16) \
					  |(0x1 << 17) \
					  |(0x1 << 24) \
					  |(0x1 << 25))
#define PERI_PROT_BIT_MASK               ((0x1 << 12) \
					  |(0x1 << 13) \
					  |(0x1 << 23))
#define PERI_PROT_BIT_ACK_MASK           ((0x1 << 12) \
					  |(0x1 << 13) \
					  |(0x1 << 23))
#define MM0_PROT_BIT_MASK                ((0x1 << 8) \
					  |(0x1 << 9))
#define MM0_PROT_BIT_ACK_MASK            ((0x1 << 8) \
					  |(0x1 << 9))
#define MM0_PROT_BIT_2ND_MASK            ((0x1 << 4) \
					  |(0x1 << 6) \
					  |(0x1 << 8) \
					  |(0x1 << 10) \
					  |(0x1 << 12) \
					  |(0x1 << 14))
#define MM0_PROT_BIT_ACK_2ND_MASK        ((0x1 << 4) \
					  |(0x1 << 6) \
					  |(0x1 << 8) \
					  |(0x1 << 10) \
					  |(0x1 << 12) \
					  |(0x1 << 14))
#define CAM_PROT_BIT_MASK                ((0x1 << 19))
#define CAM_PROT_BIT_ACK_MASK            ((0x1 << 19))
#define IPU_PROT_BIT_MASK                ((0x1 << 7) \
					  |(0x1 << 24))
#define IPU_PROT_BIT_ACK_MASK            ((0x1 << 7) \
					  |(0x1 << 24))
#define IPU_PROT_BIT_2ND_MASK            ((0x1 << 18) \
					  |(0x1 << 20))
#define IPU_PROT_BIT_ACK_2ND_MASK        ((0x1 << 18) \
					  |(0x1 << 20))
#define ISP_PROT_BIT_MASK                ((0x1 << 21))
#define ISP_PROT_BIT_ACK_MASK            ((0x1 << 21))
#define VEN_PROT_BIT_MASK                ((0x1 << 31))
#define VEN_PROT_BIT_ACK_MASK            ((0x1 << 31))
#define MCU0_PROT_BIT_MASK               ((0x1 << 11) \
					  |(0x1 << 16) \
					  |(0x1 << 17) \
					  |(0x1 << 30) \
					  |(0x1 << 31))
#define MCU0_PROT_BIT_ACK_MASK           ((0x1 << 11) \
					  |(0x1 << 16) \
					  |(0x1 << 17) \
					  |(0x1 << 30) \
					  |(0x1 << 31))
#define MCU0_PROT_BIT_2ND_MASK           ((0x1 << 20) \
					  |(0x1 << 21) \
					  |(0x1 << 22) \
					  |(0x1 << 23) \
					  |(0x1 << 24))
#define MCU0_PROT_BIT_ACK_2ND_MASK       ((0x1 << 20) \
					  |(0x1 << 21) \
					  |(0x1 << 22) \
					  |(0x1 << 23) \
					  |(0x1 << 24))
#define MCU1_PROT_BIT_MASK               ((0x1 << 11) \
					  |(0x1 << 16) \
					  |(0x1 << 17) \
					  |(0x1 << 30) \
					  |(0x1 << 31))
#define MCU1_PROT_BIT_ACK_MASK           ((0x1 << 11) \
					  |(0x1 << 16) \
					  |(0x1 << 17) \
					  |(0x1 << 30) \
					  |(0x1 << 31))
#define MCU1_PROT_BIT_2ND_MASK           ((0x1 << 20) \
					  |(0x1 << 21) \
					  |(0x1 << 22) \
					  |(0x1 << 23) \
					  |(0x1 << 24))
#define MCU1_PROT_BIT_ACK_2ND_MASK       ((0x1 << 20) \
					  |(0x1 << 21) \
					  |(0x1 << 22) \
					  |(0x1 << 23) \
					  |(0x1 << 24))
#define MCU2_PROT_BIT_MASK               ((0x1 << 0) \
					  |(0x1 << 1) \
					  |(0x1 << 2) \
					  |(0x1 << 3))
#define MCU2_PROT_BIT_ACK_MASK           ((0x1 << 0) \
					  |(0x1 << 1) \
					  |(0x1 << 2) \
					  |(0x1 << 3))
#define MCU7_PROT_BIT_MASK               ((0x1 << 4) \
					  |(0x1 << 5) \
					  |(0x1 << 6) \
					  |(0x1 << 7))
#define MCU7_PROT_BIT_ACK_MASK           ((0x1 << 4) \
					  |(0x1 << 5) \
					  |(0x1 << 6) \
					  |(0x1 << 7))
#define MCU12_PROT_BIT_MASK              ((0x1 << 8) \
					  |(0x1 << 9) \
					  |(0x1 << 10) \
					  |(0x1 << 11) \
					  |(0x1 << 12))
#define MCU12_PROT_BIT_ACK_MASK          ((0x1 << 8) \
					  |(0x1 << 9) \
					  |(0x1 << 10) \
					  |(0x1 << 11) \
					  |(0x1 << 12))
#define MCU15_PROT_BIT_MASK              ((0x1 << 12) \
					  |(0x1 << 13) \
					  |(0x1 << 14) \
					  |(0x1 << 15))
#define MCU15_PROT_BIT_ACK_MASK          ((0x1 << 12) \
					  |(0x1 << 13) \
					  |(0x1 << 14) \
					  |(0x1 << 15))

/* Define MTCMOS Power Status Mask */
#define MCU0_CPU_PWR_STA_MASK            (0x1 << 0)
#define MCU1_CPU_PWR_STA_MASK            (0x1 << 1)
#define MCU2_CPU_PWR_STA_MASK            (0x1 << 2)
#define MCU3_CPU_PWR_STA_MASK            (0x1 << 3)
#define MCU4_CPU_PWR_STA_MASK            (0x1 << 4)
#define MCU5_CPU_PWR_STA_MASK            (0x1 << 5)
#define MCU6_CPU_PWR_STA_MASK            (0x1 << 6)
#define MCU7_CPU_PWR_STA_MASK            (0x1 << 7)
#define MCU8_CPU_PWR_STA_MASK            (0x1 << 8)
#define MCU9_CPU_PWR_STA_MASK            (0x1 << 9)
#define MCU10_CPU_PWR_STA_MASK           (0x1 << 10)
#define MCU11_CPU_PWR_STA_MASK           (0x1 << 11)
#define MCU12_CPU_PWR_STA_MASK           (0x1 << 12)
#define MCU13_CPU_PWR_STA_MASK           (0x1 << 13)
#define MCU14_CPU_PWR_STA_MASK           (0x1 << 14)
#define MCU15_CPU_PWR_STA_MASK           (0x1 << 15)
#define MCU16_CPU_PWR_STA_MASK           (0x1 << 16)
#define MCU17_CPU_PWR_STA_MASK           (0x1 << 17)

#define MFG0_PWR_STA_MASK                (0x1 << 1)
#define MFG1_PWR_STA_MASK                (0x1 << 2)
#define MFG2_PWR_STA_MASK                (0x1 << 3)
#define MFG3_PWR_STA_MASK                (0x1 << 4)
#define C2K_PWR_STA_MASK                 (0x1 << 5)
#define MD1_PWR_STA_MASK                 (0x1 << 6)
#define DPY_CH0_PWR_STA_MASK             (0x1 << 7)
#define DPY_CH1_PWR_STA_MASK             (0x1 << 8)
#define DPY_CH2_PWR_STA_MASK             (0x1 << 9)
#define DPY_CH3_PWR_STA_MASK             (0x1 << 10)
#define INFRA_PWR_STA_MASK               (0x1 << 11)
#define PERI_PWR_STA_MASK                (0x1 << 12)
#define AUD_PWR_STA_MASK                 (0x1 << 13)
#define MJC_PWR_STA_MASK                 (0x1 << 14)
#define MM0_PWR_STA_MASK                 (0x1 << 15)
#define MM1_PWR_STA_MASK                 (0x1 << 16)
#define CAM_PWR_STA_MASK                 (0x1 << 17)
#define IPU_PWR_STA_MASK                 (0x1 << 18)
#define ISP_PWR_STA_MASK                 (0x1 << 19)
#define VEN_PWR_STA_MASK                 (0x1 << 20)
#define VDE_PWR_STA_MASK                 (0x1 << 21)
#define DUMMY0_PWR_STA_MASK              (0x1 << 22)
#define DUMMY1_PWR_STA_MASK              (0x1 << 23)
#define DUMMY2_PWR_STA_MASK              (0x1 << 24)
#define DUMMY3_PWR_STA_MASK              (0x1 << 25)

/* Define CPU SRAM Mask */
#define MCU0_SRAM_PDN                    (0x1 << 8)
#define MCU0_SRAM_PDN_ACK                (0x1 << 24)
#define MCU0_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU0_SRAM_SLEEP_B                (0x1 << 12)
#define MCU0_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU0_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU1_SRAM_PDN                    (0x1 << 8)
#define MCU1_SRAM_PDN_ACK                (0x1 << 24)
#define MCU1_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU1_SRAM_SLEEP_B                (0x1 << 12)
#define MCU1_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU1_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU2_SRAM_PDN                    (0x1 << 8)
#define MCU2_SRAM_PDN_ACK                (0x1 << 24)
#define MCU2_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU2_SRAM_SLEEP_B                (0x1 << 12)
#define MCU2_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU2_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU3_SRAM_PDN                    (0x1 << 8)
#define MCU3_SRAM_PDN_ACK                (0x1 << 24)
#define MCU3_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU3_SRAM_SLEEP_B                (0x1 << 12)
#define MCU3_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU3_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU4_SRAM_PDN                    (0x1 << 8)
#define MCU4_SRAM_PDN_ACK                (0x1 << 24)
#define MCU4_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU4_SRAM_SLEEP_B                (0x1 << 12)
#define MCU4_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU4_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU5_SRAM_PDN                    (0x1 << 8)
#define MCU5_SRAM_PDN_ACK                (0x1 << 24)
#define MCU5_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU5_SRAM_SLEEP_B                (0x1 << 12)
#define MCU5_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU5_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU6_SRAM_PDN                    (0x1 << 8)
#define MCU6_SRAM_PDN_ACK                (0x1 << 24)
#define MCU6_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU6_SRAM_SLEEP_B                (0x1 << 12)
#define MCU6_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU6_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU7_SRAM_PDN                    (0x1 << 8)
#define MCU7_SRAM_PDN_ACK                (0x1 << 24)
#define MCU7_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU7_SRAM_SLEEP_B                (0x1 << 12)
#define MCU7_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU7_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU8_SRAM_PDN                    (0x1 << 8)
#define MCU8_SRAM_PDN_ACK                (0x1 << 24)
#define MCU8_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU8_SRAM_SLEEP_B                (0x1 << 12)
#define MCU8_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU8_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU9_SRAM_PDN                    (0x1 << 8)
#define MCU9_SRAM_PDN_ACK                (0x1 << 24)
#define MCU9_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MCU9_SRAM_SLEEP_B                (0x1 << 12)
#define MCU9_SRAM_SLEEP_B_ACK            (0x1 << 28)
#define MCU9_SRAM_SLEEP_B_ACK_BIT0       (0x1 << 28)
#define MCU10_SRAM_PDN                   (0x1 << 8)
#define MCU10_SRAM_PDN_ACK               (0x1 << 24)
#define MCU10_SRAM_PDN_ACK_BIT0          (0x1 << 24)
#define MCU10_SRAM_SLEEP_B               (0x1 << 12)
#define MCU10_SRAM_SLEEP_B_ACK           (0x1 << 28)
#define MCU10_SRAM_SLEEP_B_ACK_BIT0      (0x1 << 28)
#define MCU11_SRAM_PDN                   (0x1 << 8)
#define MCU11_SRAM_PDN_ACK               (0x1 << 24)
#define MCU11_SRAM_PDN_ACK_BIT0          (0x1 << 24)
#define MCU11_SRAM_SLEEP_B               (0x1 << 12)
#define MCU11_SRAM_SLEEP_B_ACK           (0x1 << 28)
#define MCU11_SRAM_SLEEP_B_ACK_BIT0      (0x1 << 28)
#define MCU12_SRAM_PDN                   (0x1 << 8)
#define MCU12_SRAM_PDN_ACK               (0x1 << 24)
#define MCU12_SRAM_PDN_ACK_BIT0          (0x1 << 24)
#define MCU12_SRAM_SLEEP_B               (0x1 << 12)
#define MCU12_SRAM_SLEEP_B_ACK           (0x1 << 28)
#define MCU12_SRAM_SLEEP_B_ACK_BIT0      (0x1 << 28)
#define MCU13_SRAM_PDN                   (0x1 << 8)
#define MCU13_SRAM_PDN_ACK               (0x1 << 24)
#define MCU13_SRAM_PDN_ACK_BIT0          (0x1 << 24)
#define MCU13_SRAM_SLEEP_B               (0x1 << 12)
#define MCU13_SRAM_SLEEP_B_ACK           (0x1 << 28)
#define MCU13_SRAM_SLEEP_B_ACK_BIT0      (0x1 << 28)
#define MCU14_SRAM_PDN                   (0x1 << 8)
#define MCU14_SRAM_PDN_ACK               (0x1 << 24)
#define MCU14_SRAM_PDN_ACK_BIT0          (0x1 << 24)
#define MCU14_SRAM_SLEEP_B               (0x1 << 12)
#define MCU14_SRAM_SLEEP_B_ACK           (0x1 << 28)
#define MCU14_SRAM_SLEEP_B_ACK_BIT0      (0x1 << 28)
#define MCU15_SRAM_PDN                   (0x1 << 8)
#define MCU15_SRAM_PDN_ACK               (0x1 << 24)
#define MCU15_SRAM_PDN_ACK_BIT0          (0x1 << 24)
#define MCU15_SRAM_SLEEP_B               (0x1 << 12)
#define MCU15_SRAM_SLEEP_B_ACK           (0x1 << 28)
#define MCU15_SRAM_SLEEP_B_ACK_BIT0      (0x1 << 28)
#define MCU16_SRAM_PDN                   (0x1 << 8)
#define MCU16_SRAM_PDN_ACK               (0x1 << 24)
#define MCU16_SRAM_PDN_ACK_BIT0          (0x1 << 24)
#define MCU16_SRAM_SLEEP_B               (0x1 << 12)
#define MCU16_SRAM_SLEEP_B_ACK           (0x1 << 28)
#define MCU16_SRAM_SLEEP_B_ACK_BIT0      (0x1 << 28)
#define MCU17_SRAM_PDN                   (0x1 << 8)
#define MCU17_SRAM_PDN_ACK               (0x1 << 24)
#define MCU17_SRAM_PDN_ACK_BIT0          (0x1 << 24)
#define MCU17_SRAM_SLEEP_B               (0x1 << 12)
#define MCU17_SRAM_SLEEP_B_ACK           (0x1 << 28)
#define MCU17_SRAM_SLEEP_B_ACK_BIT0      (0x1 << 28)

/* Define Non-CPU SRAM Mask */
#define MFG1_SRAM_PDN                    (0xF << 8)
#define MFG1_SRAM_PDN_ACK                (0xF << 24)
#define MFG1_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MFG1_SRAM_PDN_ACK_BIT1           (0x1 << 25)
#define MFG1_SRAM_PDN_ACK_BIT2           (0x1 << 26)
#define MFG1_SRAM_PDN_ACK_BIT3           (0x1 << 27)
#define MFG2_SRAM_PDN                    (0xF << 8)
#define MFG2_SRAM_PDN_ACK                (0xF << 24)
#define MFG2_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MFG2_SRAM_PDN_ACK_BIT1           (0x1 << 25)
#define MFG2_SRAM_PDN_ACK_BIT2           (0x1 << 26)
#define MFG2_SRAM_PDN_ACK_BIT3           (0x1 << 27)
#define MFG3_SRAM_PDN                    (0xF << 8)
#define MFG3_SRAM_PDN_ACK                (0xF << 24)
#define MFG3_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define MFG3_SRAM_PDN_ACK_BIT1           (0x1 << 25)
#define MFG3_SRAM_PDN_ACK_BIT2           (0x1 << 26)
#define MFG3_SRAM_PDN_ACK_BIT3           (0x1 << 27)
#define MD1_SRAM_PDN                     (0x7 << 8)
#define MD1_SRAM_PDN_ACK                 (0x0 << 24)
#define MD1_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define MD1_SRAM_PDN_ACK_BIT1            (0x1 << 25)
#define MD1_SRAM_PDN_ACK_BIT2            (0x1 << 26)
#define DPY_CH0_SRAM_PDN                 (0xF << 8)
#define DPY_CH0_SRAM_PDN_ACK             (0x1 << 12)
#define DPY_CH0_SRAM_PDN_ACK_BIT0        (0x1 << 12)
#define DPY_CH0_SRAM_PDN_ACK_BIT1        (0x1 << 13)
#define DPY_CH0_SRAM_PDN_ACK_BIT2        (0x1 << 14)
#define DPY_CH0_SRAM_PDN_ACK_BIT3        (0x1 << 15)
#define DPY_CH1_SRAM_PDN                 (0xF << 8)
#define DPY_CH1_SRAM_PDN_ACK             (0x2 << 12)
#define DPY_CH1_SRAM_PDN_ACK_BIT0        (0x1 << 12)
#define DPY_CH1_SRAM_PDN_ACK_BIT1        (0x1 << 13)
#define DPY_CH1_SRAM_PDN_ACK_BIT2        (0x1 << 14)
#define DPY_CH1_SRAM_PDN_ACK_BIT3        (0x1 << 15)
#define DPY_CH2_SRAM_PDN                 (0xF << 8)
#define DPY_CH2_SRAM_PDN_ACK             (0x4 << 12)
#define DPY_CH2_SRAM_PDN_ACK_BIT0        (0x1 << 12)
#define DPY_CH2_SRAM_PDN_ACK_BIT1        (0x1 << 13)
#define DPY_CH2_SRAM_PDN_ACK_BIT2        (0x1 << 14)
#define DPY_CH2_SRAM_PDN_ACK_BIT3        (0x1 << 15)
#define DPY_CH3_SRAM_PDN                 (0xF << 8)
#define DPY_CH3_SRAM_PDN_ACK             (0x8 << 12)
#define DPY_CH3_SRAM_PDN_ACK_BIT0        (0x1 << 12)
#define DPY_CH3_SRAM_PDN_ACK_BIT1        (0x1 << 13)
#define DPY_CH3_SRAM_PDN_ACK_BIT2        (0x1 << 14)
#define DPY_CH3_SRAM_PDN_ACK_BIT3        (0x1 << 15)
#define INFRA_SRAM_PDN                   (0xF << 8)
#define INFRA_SRAM_PDN_ACK               (0xF << 24)
#define INFRA_SRAM_PDN_ACK_BIT0          (0x1 << 24)
#define INFRA_SRAM_PDN_ACK_BIT1          (0x1 << 25)
#define INFRA_SRAM_PDN_ACK_BIT2          (0x1 << 26)
#define INFRA_SRAM_PDN_ACK_BIT3          (0x1 << 27)
#define PERI_SRAM_PDN                    (0xF << 8)
#define PERI_SRAM_PDN_ACK                (0xF << 24)
#define PERI_SRAM_PDN_ACK_BIT0           (0x1 << 24)
#define PERI_SRAM_PDN_ACK_BIT1           (0x1 << 25)
#define PERI_SRAM_PDN_ACK_BIT2           (0x1 << 26)
#define PERI_SRAM_PDN_ACK_BIT3           (0x1 << 27)
#define AUD_SRAM_PDN                     (0xF << 8)
#define AUD_SRAM_PDN_ACK                 (0xF << 24)
#define AUD_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define AUD_SRAM_PDN_ACK_BIT1            (0x1 << 25)
#define AUD_SRAM_PDN_ACK_BIT2            (0x1 << 26)
#define AUD_SRAM_PDN_ACK_BIT3            (0x1 << 27)
#define MJC_SRAM_PDN                     (0x1 << 8)
#define MJC_SRAM_PDN_ACK                 (0x1 << 24)
#define MJC_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define MM0_SRAM_PDN                     (0x1 << 8)
#define MM0_SRAM_PDN_ACK                 (0x1 << 24)
#define MM0_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define MM1_SRAM_PDN                     (0x1 << 8)
#define MM1_SRAM_PDN_ACK                 (0x1 << 24)
#define MM1_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define CAM_SRAM_PDN                     (0x3 << 8)
#define CAM_SRAM_PDN_ACK                 (0x3 << 24)
#define CAM_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define CAM_SRAM_PDN_ACK_BIT1            (0x1 << 25)
#define IPU_SRAM_PDN                     (0x1 << 8)
#define IPU_SRAM_PDN_ACK                 (0x1 << 24)
#define IPU_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define ISP_SRAM_PDN                     (0x3 << 8)
#define ISP_SRAM_PDN_ACK                 (0x3 << 24)
#define ISP_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define ISP_SRAM_PDN_ACK_BIT1            (0x1 << 25)
#define VEN_SRAM_PDN                     (0xF << 8)
#define VEN_SRAM_PDN_ACK                 (0xF << 24)
#define VEN_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define VEN_SRAM_PDN_ACK_BIT1            (0x1 << 25)
#define VEN_SRAM_PDN_ACK_BIT2            (0x1 << 26)
#define VEN_SRAM_PDN_ACK_BIT3            (0x1 << 27)
#define VDE_SRAM_PDN                     (0x1 << 8)
#define VDE_SRAM_PDN_ACK                 (0x1 << 24)
#define VDE_SRAM_PDN_ACK_BIT0            (0x1 << 24)
#define DUMMY0_SRAM_PDN                  (0xF << 8)
#define DUMMY0_SRAM_PDN_ACK              (0xF << 24)
#define DUMMY0_SRAM_PDN_ACK_BIT0         (0x1 << 24)
#define DUMMY0_SRAM_PDN_ACK_BIT1         (0x1 << 25)
#define DUMMY0_SRAM_PDN_ACK_BIT2         (0x1 << 26)
#define DUMMY0_SRAM_PDN_ACK_BIT3         (0x1 << 27)
#define DUMMY1_SRAM_PDN                  (0xF << 8)
#define DUMMY1_SRAM_PDN_ACK              (0xF << 24)
#define DUMMY1_SRAM_PDN_ACK_BIT0         (0x1 << 24)
#define DUMMY1_SRAM_PDN_ACK_BIT1         (0x1 << 25)
#define DUMMY1_SRAM_PDN_ACK_BIT2         (0x1 << 26)
#define DUMMY1_SRAM_PDN_ACK_BIT3         (0x1 << 27)
#define DUMMY2_SRAM_PDN                  (0xF << 8)
#define DUMMY2_SRAM_PDN_ACK              (0xF << 24)
#define DUMMY2_SRAM_PDN_ACK_BIT0         (0x1 << 24)
#define DUMMY2_SRAM_PDN_ACK_BIT1         (0x1 << 25)
#define DUMMY2_SRAM_PDN_ACK_BIT2         (0x1 << 26)
#define DUMMY2_SRAM_PDN_ACK_BIT3         (0x1 << 27)
#define DUMMY3_SRAM_PDN                  (0xF << 8)
#define DUMMY3_SRAM_PDN_ACK              (0xF << 24)
#define DUMMY3_SRAM_PDN_ACK_BIT0         (0x1 << 24)
#define DUMMY3_SRAM_PDN_ACK_BIT1         (0x1 << 25)
#define DUMMY3_SRAM_PDN_ACK_BIT2         (0x1 << 26)
#define DUMMY3_SRAM_PDN_ACK_BIT3         (0x1 << 27)

extern unsigned int cpu_bitmask;
int spm_mtcmos_ctrl_mp0_cputop_shut_down(int state);
int spm_mtcmos_ctrl_mp1_cputop_shut_down(int state);
int mtcmos_little_core_onoff(int cpu_id, int state, unsigned int chk_wfi);
#endif /* __MTCMOS_H__ */
