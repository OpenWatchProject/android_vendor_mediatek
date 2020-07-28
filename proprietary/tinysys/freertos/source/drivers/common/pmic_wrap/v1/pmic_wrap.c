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

#include "pmic_wrap.h"

/************** For timeout API *********************/
/*#define RUN_ON_FPGA*/
#if !defined RUN_ON_FPGA
static unsigned int _pwrap_get_current_time(void) {
    return 0;
}

static int _pwrap_timeout_ns(unsigned int start_time, unsigned int elapse_time) {
    return 0;
}

static unsigned int _pwrap_time2ns(unsigned int time_us) {
    return 0;
}
#else
static unsigned int _pwrap_get_current_time(void) {
    return (xTaskGetTickCount() * (1000 / portTICK_RATE_MS) * 1000);
}

static int _pwrap_timeout_ns (unsigned int start_time, unsigned int elapse_time) {
    unsigned int cur_time = 0 ;
    cur_time = (xTaskGetTickCount() * (1000 / portTICK_RATE_MS) * 1000);
    return (cur_time > (start_time + elapse_time));
}

static unsigned int _pwrap_time2ns (unsigned int time_us) {
    return (time_us * 1000);
}

#endif

/****************  channel API *****************/
typedef unsigned int (*loop_condition_fp)(unsigned int);

static inline unsigned int wait_for_fsm_idle(unsigned int x) {
    return (GET_WACS_P2P_FSM( x ) != 0 );
}
static inline unsigned int wait_for_fsm_vldclr(unsigned int x) {
    return (GET_WACS_P2P_FSM( x ) != 6);
}

static inline unsigned int  wait_for_state_idle(loop_condition_fp fp, unsigned int timeout_us, void * wacs_register,
            void * wacs_vldclr_register, unsigned int *read_reg) {
    unsigned long long start_time_ns = 0, timeout_ns = 0;
    unsigned int reg_rdata;

    start_time_ns = _pwrap_get_current_time();
    timeout_ns = _pwrap_time2ns(timeout_us);

    do {
        if (_pwrap_timeout_ns(start_time_ns, timeout_ns)) {
            PWRAPERR("wait_for_state_idle timeout when waiting for idle\n\r");
            return E_PWR_WAIT_IDLE_TIMEOUT;
        }
        reg_rdata = DRV_Reg32(wacs_register);
        PWRAPSBLOG("wait_for_state_idle%x 0x%x, 0x%x\n", wacs_register, DRV_Reg32(PMIC_WRAP_WACS_EN), DRV_Reg32(PMIC_WRAP_WACS_INIT_DONE));
        PWRAPSBLOG("wait_for_state_idle%x 0x%x, 0x%x\n", PMIC_WRAP_WACS_RDATA, DRV_Reg32(PMIC_WRAP_WACS_CMD), DRV_Reg32(PMIC_WRAP_WACS_RDATA));
        PWRAPSBLOG("wait_for_state_idle%x 0x%x, 0x%x\n", PMIC_WRAP_WACS_VLDCLR, DRV_Reg32(PMIC_WRAP_WACS_VLDCLR), reg_rdata);
        if (GET_WACS_INIT_DONE_P2P(reg_rdata) != WACS_INIT_DONE) {
            PWRAPERR("initialization isn't finished \n\r");
            return E_PWR_NOT_INIT_DONE;
        }
    } while (fp(reg_rdata));

    if (read_reg)
        *read_reg = reg_rdata;

    return 0;
}

static inline unsigned int wait_for_state_ready(loop_condition_fp fp, unsigned int timeout_us, void * wacs_register,
            unsigned int *read_reg) {

    unsigned long long start_time_ns = 0, timeout_ns = 0;
    unsigned int reg_rdata;

    start_time_ns = _pwrap_get_current_time();
    timeout_ns = _pwrap_time2ns(timeout_us);

    do {
        if (_pwrap_timeout_ns(start_time_ns, timeout_ns)) {
            PWRAPERR("timeout when waiting for idle\n\r");
            return E_PWR_WAIT_IDLE_TIMEOUT;
        }
        reg_rdata = DRV_Reg32(wacs_register);
        PWRAPSBLOG("wait_for_state_ready%x 0x%x, 0x%x\n", wacs_register, DRV_Reg32(PMIC_WRAP_WACS_EN), DRV_Reg32(PMIC_WRAP_WACS_INIT_DONE));
        PWRAPSBLOG("wait_for_state_ready%x 0x%x, 0x%x\n", PMIC_WRAP_WACS_RDATA, DRV_Reg32(PMIC_WRAP_WACS_CMD), DRV_Reg32(PMIC_WRAP_WACS_RDATA));
        PWRAPSBLOG("wait_for_state_ready%x 0x%x, 0x%x\n", PMIC_WRAP_WACS_VLDCLR, DRV_Reg32(PMIC_WRAP_WACS_VLDCLR), reg_rdata);

        if (GET_WACS_INIT_DONE_P2P(reg_rdata) != WACS_INIT_DONE) {
            PWRAPERR("initialization isn't finished \n\r");
            return E_PWR_NOT_INIT_DONE;
        }
    } while (fp(reg_rdata));

    if(read_reg)
        *read_reg = reg_rdata;

    return 0;
}

static signed int pwrap_scp(unsigned int  write, unsigned int  adr, unsigned int  wdata, unsigned int *rdata) {
        unsigned int reg_rdata = 0;
        unsigned int wacs_write = 0;
        unsigned int wacs_adr = 0;
        unsigned int wacs_cmd = 0;
        unsigned int return_value = 0;

        /* Check argument validation */
        if ((write & ~(0x1))    != 0)  return E_PWR_INVALID_RW;
        if ((adr   & ~(0xffff)) != 0)  return E_PWR_INVALID_ADDR;
        if ((wdata & ~(0xffff)) != 0)  return E_PWR_INVALID_WDAT;

        /* Argument process */
        wacs_write = write << 31;
        wacs_adr = (adr >> 1) << 16;

        /* Check IDLE & INIT_DONE in advance */
        return_value = wait_for_state_idle(wait_for_fsm_idle, TIMEOUT_WAIT_IDLE,
            (UINT32P)PMIC_WRAP_WACS_RDATA, (UINT32P)PMIC_WRAP_WACS_VLDCLR, 0);
        if (return_value != 0) {
            PWRAPERR("wait_for_fsm_idle fail,return_value=%d\n\r", return_value);
            goto FAIL;
        }

        wacs_cmd = wacs_write | wacs_adr | wdata;
        DRV_WriteReg32(PMIC_WRAP_WACS_CMD, wacs_cmd);

        if (write == 0) {
            if (NULL == rdata) {
                PWRAPERR("rdata is a NULL pointer\n\r");
                return_value= E_PWR_INVALID_ARG;
                goto FAIL;
            }
            return_value = wait_for_state_ready(wait_for_fsm_vldclr, TIMEOUT_READ,
                    (UINT32P)PMIC_WRAP_WACS_RDATA, &reg_rdata);
            if (return_value != 0) {
                PWRAPERR("wait_for_fsm_vldclr fail,return_value=%d\n\r", return_value);
                return_value += 1;
                goto FAIL;
            }

            *rdata = GET_WACS_P2P_RDATA(reg_rdata);
            DRV_WriteReg32(PMIC_WRAP_WACS_VLDCLR , 1);
        }
        PWRAPSBLOG("pwrap_scp write:0x%x, adr:0x%x, wdata:0x%x, rdata:0x%x\n\r", write, adr, wdata, *rdata);
    FAIL:
        if (return_value != 0) {
            PWRAPERR("pwrap_wacs2 fail,return_value=%d\n\r", return_value);
        }

    return return_value;
}


signed int pwrap_scp_read(unsigned int  adr, unsigned int *rdata) {
    return pwrap_scp(0, adr, 0, rdata);
}

signed int pwrap_scp_write(unsigned int  adr, unsigned int  wdata) {
    return pwrap_scp(1, adr, wdata, 0);
}

/*************************************************/


/*******************wacs2 channel API ******************************/
/******* For channel UT ********/
static unsigned int pwrap_read_test(void) {
    unsigned int rdata = 0;
    unsigned int return_value = 0;

    return_value = pwrap_scp_read(DEW_READ_TEST, &rdata);
    if (rdata != DEFAULT_VALUE_READ_TEST) {
        PWRAPERR("Read Test fail,rdata=0x%x, exp=0x5aa5,return_value=0x%x\n\r", rdata, return_value);
        return E_PWR_READ_TEST_FAIL;
    } else {
        PWRAPLOG("Read Test pass,return_value=%d\n\r", return_value);
    }


    return 0;
}

static unsigned int pwrap_write_test(void) {
    unsigned int rdata = 0;
    unsigned int sub_return = 0;
    unsigned int sub_return1 = 0;

    sub_return = pwrap_scp_write(DEW_WRITE_TEST, PWRAP_WRITE_TEST_VALUE);
    sub_return1 = pwrap_scp_read(DEW_WRITE_TEST, &rdata);
    if ((rdata != PWRAP_WRITE_TEST_VALUE) || (sub_return != 0) || (sub_return1 != 0)) {
        PWRAPERR("write test error, rdata = 0x%x, exp = 0xa55a, sub_return = 0x%x, sub_return1 = 0x%x\n\r",
            rdata, sub_return, sub_return1);
        return E_PWR_INIT_WRITE_TEST;
    } else {
        PWRAPLOG("write test pass\n\r");
    }
    return 0;
}

signed int pmic_wrap_init(void) {
    /* Init scp */
    DRV_WriteReg32(CLK_CTRL_BASE + 0x30, DRV_Reg32(CLK_CTRL_BASE + 0x30) | 0x40);
    PWRAPSBLOG("CLK_CTRL_BASE + 0x30:0x%x\n", DRV_Reg32(CLK_CTRL_BASE + 0x30));
    DRV_WriteReg32(PMIC_WRAP_WACS_EN, 0x1);
    DRV_WriteReg32(PMIC_WRAP_WACS_INIT_DONE, 0x1);

    PWRAPSBLOG("pmic_wrap_init:0x%x 0x%x, 0x%x\n", PMIC_WRAP_WACS_EN, DRV_Reg32(PMIC_WRAP_WACS_EN), DRV_Reg32(PMIC_WRAP_WACS_INIT_DONE));
    PWRAPSBLOG("pmic_wrap_init:0x%x 0x%x, 0x%x\n", PMIC_WRAP_WACS_RDATA, DRV_Reg32(PMIC_WRAP_WACS_CMD), DRV_Reg32(PMIC_WRAP_WACS_RDATA));
    PWRAPSBLOG("pmic_wrap_init:0x%x 0x%x, 0x%x\n", PMIC_WRAP_WACS_VLDCLR, DRV_Reg32(PMIC_WRAP_WACS_VLDCLR), 0);
    pwrap_read_test();
    pwrap_write_test();

    PWRAPLOG("pmic_wrap_init done\n");

    return 0;
}
