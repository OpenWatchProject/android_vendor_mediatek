/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.mms.ipmessage;

import android.app.Activity;
import android.content.AsyncQueryHandler;
import android.database.Cursor;
import android.net.Uri;
import android.view.ActionMode;
import android.view.Menu;
import android.view.MenuItem;

import com.mediatek.mms.callback.IMultiDeleteActivityCallback;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

public class DefaultIpMultiDeleteActivityExt implements IIpMultiDeleteActivityExt {

    public boolean multiDeleteActivityInit(
            Activity context, IMultiDeleteActivityCallback callback) {
        return false;
    }

    public boolean onIpDeleteLockedIpMsg(long msgId) {
        return false;
    }

    public boolean onIpMultiDeleteClick(AsyncQueryHandler handler, int token, Object cookie,
            int deleteRunningCount, boolean deleteLocked) {
        return false;
    }

    public boolean onIpHandleItemClick(IIpMessageItemExt item, long ipMessageId, boolean isSelected,
            long msgId) {
        return false;
    }

    public boolean onIpMarkCheckedState(Cursor cursor, boolean checkedState) {
        return false;
    }

    public boolean onAddSelectedIpMessageId(boolean checkedState, long msgId, long ipMessageId) {
        return false;
    }

    public boolean onIpDeleteThread(Collection<Long> threads, int maxSmsId) {
        return false;
    }

    public boolean onCreateIpActionMode(final ActionMode mode, Menu menu) {
        return false;
    }

    public boolean onPrepareIpActionMode(
            ActionMode mode, Menu menu, int selectNum, int ForwardMenuId) {
        return false;
    }

    public boolean onIpActionItemClicked(ActionMode mode, MenuItem item, long[][] selectedIds,
            String[] Contacts, Cursor cursor) {
        return false;
    }

    public boolean ipForwardOneMms(Uri mUri) {
        return false;
    }

    public boolean forwardTextMessage(ArrayList<Long> smsList, int maxLength) {
        return false;
    }

    @Override
    public boolean startMsgListQuery(AsyncQueryHandler mQueryHandler, int token, Object cookie,
            Uri uri, String[]projection, String selection, String[] selectionArgs, String orderBy) {
        return false;
    }

    @Override
    public boolean onIpParseDeleteMsg(long key) {
        return false;
    }

    @Override
    public void initMessageList(IIpMessageListAdapterExt adapter) {
        //do nothing
    }

    @Override
    public void markAsLocked(final long[][] ids, final boolean lock) {
        // do nothing
    }

    @Override
    public long[][] getSelectedMsgIds(Map<Long, Boolean> selectMap, int size) {
        return null;
    }
}

