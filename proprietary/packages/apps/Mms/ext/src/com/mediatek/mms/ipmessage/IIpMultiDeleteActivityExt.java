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

public interface IIpMultiDeleteActivityExt {

    /**
     * called on initPlugin
     * @param context: this activity
     * @return boolean
     * @internal
     */
    public boolean multiDeleteActivityInit(Activity context, IMultiDeleteActivityCallback callback);

    /**
     * called on MultiDeleteMsgListener
     * @param handler: AsyncQueryHandler
     * @param token: int
     * @param deleteLocked: deleteLocked
     * @return boolean
     * @internal
     */
    public boolean onIpMultiDeleteClick(AsyncQueryHandler handler, int token, Object cookie,
                                                    int deleteRunningCount, boolean deleteLocked);

    /**
     * called on MultiDeleteMsgListener
     * @param msgId: sms Id
     * @return boolean
     * @internal
     */
    public boolean onIpDeleteLockedIpMsg(long msgId);

    /**
     * called on mMessageListItemHandler
     * @param item: IIpMessageItemExt
     * @param ipMessageId: ipMessageId
     * @param isSelected: isSelected
     * @param msgId: msgId
     * @return boolean
     * @internal
     */
    public boolean onIpHandleItemClick(IIpMessageItemExt item, long ipMessageId, boolean isSelected,
                                        long msgId);

    /**
     * called on markCheckedState
     * @param cursor: Cursor
     * @param checkedState: boolean
     * @return boolean
     * @internal
     */
    public boolean onIpMarkCheckedState(Cursor cursor, boolean checkedState);

    /**
     * called on markCheckedState
     * @param checkedState: checkedState
     * @param msgId: msgId
     * @param ipMessageId: ipMessageId
     * @return boolean
     * @internal
     */
    public boolean onAddSelectedIpMessageId(boolean checkedState, long msgId, long ipMessageId);

    /**
     * called on MultiDeleteMsgListener
     * @param threads: threads list
     * @param maxSmsId: maxSmsId
     * @return boolean
     * @internal
     */
    public boolean onIpDeleteThread(Collection<Long> threads, int maxSmsId);

    /**
     * called on onCreateActionMode
     * @param mode: ActionMode
     * @param menu: Menu
     * @return boolean
     * @internal
     */
    public boolean onCreateIpActionMode(final ActionMode mode, Menu menu);

    /**
     * called on onPrepareActionMode
     * @param mode: ActionMode
     * @param menu: Menu
     * @return boolean
     * @internal
     */
    public boolean onPrepareIpActionMode(
            ActionMode mode, Menu menu, int selectNum, int ForwardMenuId);

    /**
     * called on onActionItemClicked
     * @param mode: ActionMode
     * @param menu: Menu
     * @param selectedIds: all selected message ids
     * @return boolean
     * @internal
     */
    public boolean onIpActionItemClicked(ActionMode mode, MenuItem item, long[][] selectedIds,
            String[] Contacts, Cursor cursor);

    public boolean ipForwardOneMms(Uri mUri);

    /**
     * called on onActionItemClicked
     * @param mode: ActionMode
     * @param menu: Menu
     * @param selectedIds: all selected message ids
     * @return boolean
     * @internal
     */
    public boolean forwardTextMessage(ArrayList<Long> smsList, int maxLength);

    /**
     * startMsgListQuery.
     * @param mQueryHandler
     * @param token
     * @param cookie
     * @param uri
     * @param projection
     * @param selection
     * @param selectionArgs
     * @param orderBy
     * @return return true if cosume, else return false
     * @internal
     */
    public boolean startMsgListQuery(AsyncQueryHandler mQueryHandler, int token, Object cookie,
            Uri uri, String[]projection, String selection, String[] selectionArgs, String orderBy);

    /**
     * onIpParseDeleteMsg.
     * @param key
     * @return if handled return true, else return false.
     * @internal
     */
    public boolean onIpParseDeleteMsg(long key);

    /**
     * InitMessageList.
     * @param adapter IIpMessageListAdapterExt
     * @internal
     */
    public void initMessageList(IIpMessageListAdapterExt adapter);

    /**
     * MarkAsLocked.
     * @param ids long[][]
     * @param lock boolean
     * @internal
     */
    public void markAsLocked(final long[][] ids, final boolean lock);

    /**
     * GetSelectedMsgIds.
     * @param selectMap
     * @param size
     * @return long[][]
     * @internal
     */
    public long[][] getSelectedMsgIds(Map<Long, Boolean> selectMap, int size);
}

