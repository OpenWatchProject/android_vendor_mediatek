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
package com.mediatek.contacts.list;

import android.content.Context;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;

import java.util.ArrayList;

public class DropMenu implements OnMenuItemClickListener {

    public static class DropDownMenu {
        private Button mButton;
        private PopupMenu mPopupMenu;
        private Menu mMenu;
        private boolean mIsPopupMenuShown;

        public DropDownMenu(Context context, Button selectItem, int menuId,
                OnMenuItemClickListener listener) {
            mButton = selectItem;
            mPopupMenu = new PopupMenu(context, mButton);
            mMenu = mPopupMenu.getMenu();
            // add the menu("Select all" or "Deselect all") on the button("XX selected").
            mPopupMenu.getMenuInflater().inflate(menuId, mMenu);
            mPopupMenu.setOnMenuItemClickListener(listener);
            mPopupMenu.setOnDismissListener(new PopupMenu.OnDismissListener() {
                @Override
                public void onDismiss(PopupMenu menu) {
                    mIsPopupMenuShown = false;
                }
            });
            mButton.setOnClickListener(new OnClickListener() {
                public void onClick(View v) {
                    // when click the button, show the popupMenu.
                    show();
                }
            });
        }

        /**
         * find menu item from menu id.
         * @param id
         * @return the menu
         */
        public MenuItem findItem(int id) {
            return mMenu.findItem(id);
        }

        public void show() {
            mIsPopupMenuShown = true;
            mPopupMenu.show();
        }

        public void dismiss() {
            mIsPopupMenuShown = false;
            mPopupMenu.dismiss();
        }

        public boolean isShown() {
            return mIsPopupMenuShown;
        }
    }

    private Context mContext;
    private ArrayList<DropDownMenu> mMenus;
    private OnMenuItemClickListener mListener;

    public DropMenu(Context context) {
        mContext = context;
        mMenus = new ArrayList<DropDownMenu>();
    }

    /**
     * new and add a menu on select_item.
     * @param select_item
     * @param menuId
     * @return
     */
    public DropDownMenu addDropDownMenu(Button selectItem, int menuId) {
        DropDownMenu menu = new DropDownMenu(mContext, selectItem, menuId, this);
        mMenus.add(menu);
        return menu;
    }

    public void setOnMenuItemClickListener(OnMenuItemClickListener listener) {
        mListener = listener;
    }

    public boolean onMenuItemClick(MenuItem item) {
        if (mListener != null) {
            return mListener.onMenuItemClick(item);
        }
        return false;
    }
}
