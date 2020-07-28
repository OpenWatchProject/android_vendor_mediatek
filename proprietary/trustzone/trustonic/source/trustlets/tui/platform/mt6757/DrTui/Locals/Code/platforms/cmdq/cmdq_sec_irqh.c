/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "drStd.h"
#include "DrApi/DrApi.h"

#include "drTuiCommon.h"
#include "cmdq_sec_reg.h"
#include "cmdq_sec_core.h"
#include "cmdq_sec_reg.h"
#include "cmdq_sec_platform.h"


DECLARE_STACK(cmdqIrqhStack, 2048);

/**
 * tbase IRQ register note
 *  .secure world IRQ ID is same as normal world (platform depenedent)
 *  .tbase IRQ porting desen't cover General Interrupt Control's IRQ init currently,
 *   whether the IRQ will be received in secure world only,
 *   we have to register IRQ_id to GIC in normalworld to ensure correct IRQ init.
 *   otherwise, me may receive IRQ at strange timing
 */

_NORETURN void cmdqIrqhLoop( void )
{
	/*
	 * timeout(ms) param of drApiWaitForIntr:
	 * 0: allowing to know if this interrupt alraedy tirggered or not
	 * TIME_INFINITE: block until IRQ is asserted
	 * 0 < x < 2 ^ 24
	 *
	 * timeout 1 sec = 1 * 1000
	 * p.s. before 301B, timeout base is ns.
	 */
	const uint32_t timeout_us = 100; /* 100 ms timeout */

	intrNo_t IRQ = 0;
	drApiResult_t drRet = DRAPI_OK;


	/* entry loop to wait irq */
	for (;;) {
#if (0 == CMDQ_DEBUG_POLL_TASK_DONE)
		/* suspend ISR till IPC thread wake up it to prepare waiting IRQ */
		if (cmdq_tz_suspend_worker_thread(DRIVER_THREAD_NO_IRQH, "ISR", "ISR")) {
			continue;
		}

		/* wait IRQ */
		drRet = drApiWaitForIntr(CMDQ_SECURE_IRQ_ID, timeout_us, &IRQ);

		if(DRAPI_OK != drRet){
			CMDQ_ERR("IRQ: drApiWaitForIntr timeout, IRQ:%d, timeout: %d, drRet:%x(%x, %x)\n",
				CMDQ_SECURE_IRQ_ID, timeout_us,
				drRet, DRAPI_ERROR_MAJOR(drRet), DRAPI_ERROR_DETAIL(drRet));

			CMDQ_ERR("IRQ:dump+\n");
			cmdq_tz_dump_status("IRQ");
			cmdq_tz_dump_thread(12, __LINE__);
			cmdq_tz_dump_thread(13, __LINE__);
			cmdq_tz_dump_thread(14, __LINE__);
			CMDQ_ERR("IRQ:dump-\n");

			/* wait IRQ timeout, go to IRQ handle to check if error IRQ and */
			/* resume IPC thread*/
		}
#else
		/* wait IRQ */
		drRet = drApiWaitForIntr(CMDQ_SECURE_IRQ_ID, timeout_us, &IRQ);
		CMDQ_LOG("IRQ: drApiWaitForIntr got, IRQ:%d, timeout: %d, drRet:%x(%x, %x)\n",
			CMDQ_SECURE_IRQ_ID, timeout_us,
			drRet, DRAPI_ERROR_MAJOR(drRet), DRAPI_ERROR_DETAIL(drRet));
		CMDQ_LOG("IRQ:dump+\n");
		cmdq_tz_dump_status("IRQ");
		cmdq_tz_dump_thread(12, __LINE__);
		cmdq_tz_dump_thread(13, __LINE__);
		cmdq_tz_dump_thread(14, __LINE__);
		CMDQ_LOG("IRQ:dump-\n");
		if(DRAPI_OK != drRet){
			continue;
		}
#endif

		// irq handle
		switch (IRQ) {
		case (CMDQ_SECURE_IRQ_ID):
			cmdq_tz_irq_handler();
			break;
		default:
			CMDQ_ERR("never go here: dose not handle irq[%d]\n", IRQ);
			break;
		}

	}
}


_THREAD_ENTRY void cmdqIrqhEntry( void )
{
	drApiResult_t attachedRet = drApiIntrAttach(CMDQ_SECURE_IRQ_ID, INTR_MODE_LOW_LEVEL);
	CMDQ_MSG("drApiIntrAttach, IRQ[%d], res = [%d]\n", CMDQ_SECURE_IRQ_ID, attachedRet);

	cmdqIrqhLoop();
}


void cmdqIrqhInit(void)
{
#if (0 == CMDQ_DEBUG_POLL_TASK_DONE)
/* Temp define for IRQ thread at first, will integrate to drTuiCommon.h after IRQ IT done */
#define DRIVER_THREAD_NO_IRQH     5

	CMDQ_MSG("cmdqIrqhInit, IRQ thread number:%d\n", DRIVER_THREAD_NO_IRQH);

	// ensure thread stack is clean
	clearStack(cmdqIrqhStack);

	// start irq handler thread
	drApiResult_t status = drApiStartThread(
					DRIVER_THREAD_NO_IRQH,
					FUNC_PTR(cmdqIrqhEntry),
					getStackTop(cmdqIrqhStack),
					IRQH_PRIORITY,
					DRIVER_THREAD_NO_EXCH);
	if (DRAPI_OK != status)
	{
		CMDQ_ERR("IrqhInit failed[%d]\n");
	}
#endif
}
