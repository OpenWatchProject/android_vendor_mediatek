/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ims.rcsua;

import android.util.Log;

/**
 * Callback indicate IMS registration status changed
 */
public abstract class ImsEventCallback extends AppCallback {

    public void onRegistering(@RcsUaService.RegistrationMode int mode) {

    }

    public void onRegistered(@RcsUaService.RegistrationMode int mode) {

    }

    public void onDeregistering(@RcsUaService.RegistrationMode int mode) {

    }

    public void onDeregistered(@RcsUaService.RegistrationMode int mode) {

    }

    public void onDeregStart(@RcsUaService.RegistrationMode int mode) {

    }

    public void onReregistered(@RcsUaService.RegistrationMode int mode) {

    }

    public void onVopsIndication(int vops) {

    }

    class Runner extends BaseRunner<Integer> {

        Runner(Integer... params) {
            super(params);
        }

        @Override
        void exec(Integer... params) {
            int regState = params[0];
            int regMode = params[1];

            Log.i("ImsEventCallback", "Runner->exec:" + regState + "," + regMode);

            switch (regState) {
                case RcsUaService.REG_STATE_REGISTERING:
                    onRegistering(regMode);
                    break;

                case RcsUaService.REG_STATE_REGISTERED:
                    onRegistered(regMode);
                    break;

                case RcsUaService.REG_STATE_DEREGISTERING:
                    onDeregistering(regMode);
                    break;

                case RcsUaService.REG_STATE_NOT_REGISTERED:
                    onDeregistered(regMode);
                    break;

                // TODO: remove magic constant
                case 128:
                    onDeregStart(regMode);
                    break;
                case 256:
                    onReregistered(regMode);
                    break;
                case 512:
                    int vops = params[2];
                    onVopsIndication(vops);
                    break;
                default:
                    /* Nothing to do */
                    break;
            }

        }
    }
}
