/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.dialer.ext;

import android.app.Fragment;
import android.content.Intent;
import android.support.v4.view.ViewPager;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.HorizontalScrollView;

public interface ICallLogActivityExt {

    /**
    * for OP02,OP09 multi tabs.
    * Init call Log tab
    * @param tabTitles tabTitles
    * @param viewPager viewPager
    */
    void initCallLogTab(CharSequence[] tabTitles, ViewPager viewPager);

     /**
      * for op09
      * called when CalllogActivity resume
      * @param activity the current activity
      */
    void onResume();

    /**
     * for OP02,OP09 multi tabs.
     * Get Tab Index Count
     * @return int
     */
   int getTabIndexCount(int defCount);

    /**
     * for op01.
     * called when host create menu, to add plug-in own menu here
     * @param menu Menu
     * @param tabs the ViewPagerTabs used in activity
     * @param callLogAction callback plug-in need if things need to be done by host
     * @internal
     */
    void createCallLogMenu(Menu menu, HorizontalScrollView tabs);

    /**
     * for op01.
     * called when host prepare menu, prepare plug-in own menu here
     * @param menu the Menu Created
     * @internal
     */
    public void prepareCallLogMenu(Menu menu);

     /**
      * for op02/op09
      * called when selec other page to clear missed call
      * @param index the index of fragment
      * @return boolean if plugin processed return true, or else return false to process in host
      */
     public boolean updateMissedCalls(int index);

  /**
    * for OP02,OP09 multi tabs.
    * Get the CallLog Item
    * @param position position
    * @return Fragment
    */
   Fragment getCallLogFragmentItem(int position);

   /**
     * for OP12, OP02,OP09 multi tabs.
     * Instantiate CallLog Item
     * @param position position
     * @param fragment fragment
     * @return Object if plugin init the item return obj, else return null
     */
   boolean instantiateItemByOp(int position, Fragment fragment);

     /**.
     * for OP02
     * plug-in handle Activity Result
     * @param requestCode requestCode
     * @param resultCode resultCode
     * @param data the intent return by setResult
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data);

    public void onDestroy();
}
