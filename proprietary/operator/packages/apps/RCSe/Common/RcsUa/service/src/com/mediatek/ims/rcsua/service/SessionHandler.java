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

package com.mediatek.ims.rcsua.service;

import android.content.Context;
import com.mediatek.ims.rcsua.service.utils.Logger;
import com.mediatek.ims.rcsua.service.utils.Utils;

public class SessionHandler {

    private static SessionHandler INSTANCE;
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * IMS network interface
     */
    private static final int REGISTRATION_TIMER = 1000;
    private static final int MAX_REGISTRATION_TIMER = 20000;

    private static int mInstanceCount  = -1;

    public static SessionHandler getInstance() {
        if (INSTANCE == null)
            INSTANCE = new SessionHandler();

        return INSTANCE;
    }

    /**
     * Instantiates a new rcs session handler.
     */
    private SessionHandler() {
        logger.debug("constructor");
    }

    public synchronized void initInstanceCount(){
        //for MD 93
        boolean result = Utils.sendAtCommand("AT+EIMSRCSCONN=254");
        logger.debug("initInstanceCount(): send init AT command, result=" + result);

        //for MD 91/92
        //RuaAdapter.RcsUaEvent event = new RuaAdapter.RcsUaEvent(mRcsUaAdapter.CMD_RDS_NOTIFY_RCS_CONN_INIT);
        //mRcsUaAdapter.writeEvent(event);

        mInstanceCount = 0;
    }

    public synchronized void incrementInstanceCount(){
        //for MD 93
        boolean result = Utils.sendAtCommand("AT+EIMSRCSCONN=1");
        logger.debug("incrementInstanceCount(): send active AT command, result=" + result);

        //for MD 91/92
        //RuaAdapter.RcsUaEvent event = new RuaAdapter.RcsUaEvent(mRcsUaAdapter.CMD_RDS_NOTIFY_RCS_CONN_ACTIVE);
        //mRcsUaAdapter.writeEvent(event);

        mInstanceCount++;
    }

    public synchronized void decrementInstanceCount(){
        //for MD 93
        boolean result = Utils.sendAtCommand("AT+EIMSRCSCONN=0");
        logger.debug("decrementInstanceCount(): send inActive AT command, result=" + result);

        //for MD 91/92
        //RuaAdapter.RcsUaEvent event = new RuaAdapter.RcsUaEvent(mRcsUaAdapter.CMD_RDS_NOTIFY_RCS_CONN_INACTIVE);
        //mRcsUaAdapter.writeEvent(event);

        mInstanceCount--;
    }

    public synchronized void deInitInstanceCount(){
        logger.debug("deInitInstanceCount(): send deInit");

        mInstanceCount = -1;
    }
}
