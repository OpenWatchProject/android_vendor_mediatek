/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.incallui.volte;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.android.incallui.ContactInfoCache;
import com.android.incallui.ContactInfoCache.ContactCacheEntry;
import com.android.incallui.R;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;

import java.util.ArrayList;
import java.util.List;

/**
 * [VoLTE Conference] ConferenceChildrenChangeHandler.
 */
public class ConferenceChildrenChangeHandler {

  private static final String LOG_TAG = "ConferenceChildrenChangeHandler";
  private static ConferenceChildrenChangeHandler sInstance =
      new ConferenceChildrenChangeHandler();
  private Context mContext;

  /**
   * ConferenceChildrenChangeHandler.
   * @return instance
   */
  public static synchronized ConferenceChildrenChangeHandler getInstance() {
    if (sInstance == null) {
      sInstance = new ConferenceChildrenChangeHandler();
    }
    return sInstance;
  }

  /**
   * setContext.
   * @param context Context
   */
  public void setContext(Context context) {
    mContext = context;
  }

  /**
   * clearContext.
   */
  public void clearContext() {
    mContext = null;
  }

  /**
   * getContext.
   * @return context
   */
  public Context getContext() {
    return mContext;
  }

  /**
   * handleChildrenChanged.
   * @param oldChildrenIds list
   * @param newChildrenIds list
   */
  public void handleChildrenChanged(List<String> oldChildrenIds, List<String> newChildrenIds) {
    // TODO: dump both list here for debug.
    log("handleChildrenChanged()...");
    // the two parameters are all ArrayList (not RandomAccess).
    // So here need feature check, whether children are really changed.(add or remove,
    // not order change)
    List<String> remianChildrenIds = new ArrayList<String>();
    List<String> removedChildrenIds = new ArrayList<String>();
    List<String> addedChildrenIds = new ArrayList<String>();

    removedChildrenIds.addAll(oldChildrenIds);
    addedChildrenIds.addAll(newChildrenIds);

    // find childrenIds which both in removedChildrenIds and addedChildrenIds.
    for (String childId : removedChildrenIds) {
      if (addedChildrenIds.contains(childId)) {
        remianChildrenIds.add(childId);
      }
    }

    // find "added" and "removed" childrenIds
    for (String childId : remianChildrenIds) {
      removedChildrenIds.remove(childId);
      addedChildrenIds.remove(childId);
    }

    // notify those children change.
    if (mContext != null) {
      for (String childId : addedChildrenIds) {
        final ContactCacheEntry contactCache = ContactInfoCache.getInstance(mContext)
            .getInfo(childId);
        if (contactCache != null) {
          ChildrenChangeNotifier.notifyChildChange(mContext,
              ChildrenChangeNotifier.NOTIFY_MEMBER_CHANGE_JOIN, contactCache.namePrimary);
        } else {
          // handle no contactInfoCache's case
          handleChildWithoutContactCache(childId,
              ChildrenChangeNotifier.NOTIFY_MEMBER_CHANGE_JOIN);
        }
      }

      for (String childId : removedChildrenIds) {
        final ContactCacheEntry contactCache =
            ContactInfoCache.getInstance(mContext).getInfo(childId);
        if (contactCache != null) {
          ChildrenChangeNotifier.notifyChildChange(mContext,
              ChildrenChangeNotifier.NOTIFY_MEMBER_CHANGE_LEAVE, contactCache.namePrimary);
        } else {
          // handle no contactInfoCache's case
          handleChildWithoutContactCache(childId,
              ChildrenChangeNotifier.NOTIFY_MEMBER_CHANGE_LEAVE);
        }
      }
    }
  }

  private void handleChildWithoutContactCache(String childId, int notifyType) {
    log("handleChildWithoutContactCache()...");
    DialerCall call = CallList.getInstance().getCallById(childId);
    if (call != null && mContext != null) {
      ContactInfoCache.getInstance(mContext).findInfo(call, false,
          new ContactLookupCallback(notifyType));
    }
  }

  public static class ContactLookupCallback implements ContactInfoCache.ContactInfoCacheCallback {
    private final int mNotifyType;

    public ContactLookupCallback(int notifyType) {
      mNotifyType = notifyType;
    }

    @Override
    public void onContactInfoComplete(String number, ContactCacheEntry entry) {
      Context context = ConferenceChildrenChangeHandler.getInstance().getContext();
      if (context != null) {
        ChildrenChangeNotifier.notifyChildChange(context, mNotifyType, entry.namePrimary);
      }
    }

    @Override
    public void onImageLoadComplete(String number, ContactCacheEntry entry) {
      // do nothing
    }
  }

  public static class ChildrenChangeNotifier {
    private static final String TAG = "ChildrenChangeNotifier";

    public static final int NOTIFY_MEMBER_CHANGE_JOIN = 300;
    public static final int NOTIFY_MEMBER_CHANGE_LEAVE = 301;
    public static final int NOTIFY_MEMBER_CHANGE_ADDING = 302;
    public static final int NOTIFY_MEMBER_CHANGE_ADD_FAILED = 303;

    public static void notifyChildChange(Context context, int notifyType, String name) {
      Log.d(TAG, "notifyMemberChange, notifyType = " + notifyType + ", name = " + name);
      String msg = "";
      switch (notifyType) {
        case NOTIFY_MEMBER_CHANGE_LEAVE:
          msg = context.getResources().getString(R.string.conference_member_leave, name);
          break;
        case NOTIFY_MEMBER_CHANGE_JOIN:
          msg = context.getResources().getString(R.string.conference_member_join, name);
          break;
        case NOTIFY_MEMBER_CHANGE_ADDING:
          msg = context.getResources().getString(R.string.conference_member_adding, name);
          break;
        case NOTIFY_MEMBER_CHANGE_ADD_FAILED:
          msg = context.getResources().getString(R.string.conference_member_add_fail, name);
          break;
        default:
          break;
      }
      if (!TextUtils.isEmpty(msg)) {
        Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
      }
    }
  }

  private void log(String msg) {
    Log.d(LOG_TAG, msg);
  }
}
