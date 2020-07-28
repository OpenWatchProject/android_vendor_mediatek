/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.miravision.ui;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.RemoteException;
import android.support.v4.app.ActionBarDrawerToggle;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.util.Log;
import android.view.IDockedStackListener;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManagerGlobal;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import java.io.IOException;
import java.util.ArrayList;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import com.mediatek.miravision.setting.MiraVisionJni;
import com.mediatek.miravision.ui.ResetDialogFragment.ResetListener;
import com.mediatek.miravision.utils.Utils;

/**
 * MiraVision application.
 */
public class MiraVisionActivity extends Activity implements ListView.OnItemClickListener {

    private static final String TAG = "MiraVisionActivity";
    private static final int AAL_ID = 0;
    private static final int PICTURE_MODE_ID = 1;
    private static final int BASIC_TUNING_ID = 2;
    private static final int ADVANCE_TUNING_ID = 3;
    private static final int OD_ID = 4;
    private static final int VIDEO_ENHANCEMENT_ID = 5;
    private static final int BLULIGHT_DEFENDER_ID = 7;

    private DrawerLayout mDrawerLayout;
    private ListView mDrawerList;
    private ActionBarDrawerToggle mDrawerToggle;
    static DrawerListViewAdapter sDrawerListAdapter;

    private static final boolean ENG_MODE_STATUS = android.os.Build.TYPE.equals("eng");
    private final Fragment[] mFragments = { new AalSettingsFragment(), new PictureModeFragment(),
            new BasicColorTuningFragment(), new AdvanceColorTuningFragment(),
            new OverDriveFragment(), new DynamicContrastFragment(), new ResetDialogFragment(),
            new ViewingComfortFragment() };

    // List item status
    private static final int ITEM_HEADER = 0x1;
    private static final int ITEM_DIVIDER = 0x2;
    private static final int ITEM_INVISIBLE = 0x4;
    private static final int ITEM_DISENABLE = 0x8;

    private static DockDividerVisibilityListener sDockDividerVisibilityListener;
    private static boolean sUpdateOrientation = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.miravision_activity);
        mDrawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        mDrawerList = (ListView) findViewById(R.id.left_drawer);

        // set a custom shadow that overlays the main content when the drawer
        // opens
        mDrawerLayout.setDrawerShadow(R.drawable.drawer_shadow, GravityCompat.START);
        // set up the drawer's list view with items and click listener
        sDrawerListAdapter = new DrawerListViewAdapter(this);
        mDrawerList.setAdapter(sDrawerListAdapter);
        mDrawerList.setOnItemClickListener(this);
        mDrawerList.setDivider(null);

        // enable ActionBar app icon to behave as action to toggle nav drawer
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setHomeButtonEnabled(true);
        getActionBar().setTitle(R.string.mira_vision_tm);

        // ActionBarDrawerToggle ties together the the proper interactions
        // between the sliding drawer and the action bar app icon
        mDrawerToggle = new ActionBarDrawerToggle(this, /* host Activity */
        mDrawerLayout, /* DrawerLayout object */
        R.drawable.ic_drawer, /* nav drawer image to replace 'Up' caret */
        R.string.drawer_open, /* "open drawer" description for accessibility */
        R.string.drawer_close /* "close drawer" description for accessibility */
        ) {
            public void onDrawerClosed(View view) {
                invalidateOptionsMenu(); // creates call to
                // onPrepareOptionsMenu()
            }

            public void onDrawerOpened(View drawerView) {
                invalidateOptionsMenu(); // creates call to
                // onPrepareOptionsMenu()
            }
        };
        mDrawerLayout.setDrawerListener(mDrawerToggle);

        if (savedInstanceState == null) {
            show(getFragmentManager(), new IntroductionFragment(),
                    ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        }

        if (sDockDividerVisibilityListener == null) {
            Log.d(TAG, "registerDockedStackListener");
            sDockDividerVisibilityListener = new DockDividerVisibilityListener(this);
            registerDockedStackListener(sDockDividerVisibilityListener);
        }
    }

    private void registerDockedStackListener(IDockedStackListener listener) {
        try {
            WindowManagerGlobal.getWindowManagerService().registerDockedStackListener(listener);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void resetPQ() {
        mDrawerLayout.openDrawer(mDrawerList);
        sDrawerListAdapter.setResetBoolean(true);
        sDrawerListAdapter.notifyDataSetChanged();
        sDrawerListAdapter.setResetBoolean(false);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // The action bar home/up action should open or close the drawer.
        // ActionBarDrawerToggle will take care of this.
        if (mDrawerToggle.onOptionsItemSelected(item)) {
            return true;
        }
        return true;
    }

    /* The click listner for ListView in the navigation drawer */
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        selectItem(position);
    }

    private void selectItem(int position) {
        Log.d(TAG, "selectItem position: " + position);
        DrawerListViewEntry entry = (DrawerListViewEntry) sDrawerListAdapter.getItem(position);
        Fragment fragment = mFragments[(int) entry.mId]; // default fragment
        int orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE; // default
        // LANDSCAPE
        sUpdateOrientation = true;

        if (fragment instanceof ResetDialogFragment) {
            ResetDialogFragment resetDialog = (ResetDialogFragment) fragment;
            resetDialog.setResetListener(new ResetListener() {
                @Override
                public void reset() {
                    resetPQ();
                    getActionBar().setTitle(R.string.mira_vision_tm);
                    show(getFragmentManager(), new IntroductionFragment(),
                            ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
                }
            });
            resetDialog.show(getFragmentManager(), null);
            mDrawerList.setItemChecked(position, true);
            mDrawerLayout.closeDrawer(mDrawerList);
            return;
        }

        getActionBar().setCustomView(null);
        // AAL and picture mode is UNSPECIFIED
        if (fragment instanceof AalSettingsFragment || fragment instanceof PictureModeFragment) {
            orientation = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;
            sUpdateOrientation = false;
        }
        // Basic and Advance tuning need know which item being selected
        if (fragment instanceof BasicColorTuningFragment) {
            fragment = new BasicColorTuningFragment(entry.mTitle);
        } else if (fragment instanceof AdvanceColorTuningFragment) {
            fragment = new AdvanceColorTuningFragment(entry.mTitle);
        } else if (fragment instanceof DynamicContrastFragment) {
            fragment = new DynamicContrastFragment(entry.mTitle);
        } else if (fragment instanceof ViewingComfortFragment) {
            fragment = new ViewingComfortFragment(entry.mTitle);
        }
        show(getFragmentManager(), fragment, orientation);

        mDrawerList.setItemChecked(position, true);
        setTitle(entry.mFragmentTitle);
        mDrawerLayout.closeDrawer(mDrawerList);
        Log.d(TAG, "selectItem mUpdateOrientation: " + sUpdateOrientation);
    }

    private void show(FragmentManager fm, Fragment fragment, int orientation) {
        setRequestedOrientation(orientation);
        final FragmentTransaction ft = fm.beginTransaction();
        ft.replace(R.id.content_frame, fragment);
        ft.commitAllowingStateLoss();
    }

    @Override
    public void setTitle(CharSequence title) {
        getActionBar().setTitle(title);
    }

    @Override
    public void onBackPressed() {
        if (mDrawerLayout.isDrawerOpen(mDrawerList)) {
            mDrawerLayout.closeDrawer(mDrawerList);
            return;
        }

        if (getFragmentManager().findFragmentById(R.id.content_frame) instanceof IntroductionFragment) {
            super.onBackPressed();
        } else {
            mDrawerLayout.openDrawer(mDrawerList);
            show(getFragmentManager(), new IntroductionFragment(),
                    ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        }
    }

    /**
     * When using the ActionBarDrawerToggle, you must call it during
     * onPostCreate() and onConfigurationChanged()...
     */
    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        // Sync the toggle state after onRestoreInstanceState has occurred.
        mDrawerToggle.syncState();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // Pass any configuration change to the drawer toggls
        mDrawerToggle.onConfigurationChanged(newConfig);
    }

    /**
     *
     * Drawer list adapter
     *
     */
    static class DrawerListViewAdapter extends BaseAdapter {
        private ArrayList<DrawerListViewEntry> mDrawerListEntries;
        private Context mContext;
        private LayoutInflater inflater;
        private boolean mIsReset;

        public DrawerListViewAdapter(Context context) {
            mContext = context;
            inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            updateDrawerListEntry();
            updateListItemsStatus();
        }

        public void setResetBoolean(boolean isReset) {
            mIsReset = isReset;
        }

        public int getCount() {
            return mDrawerListEntries != null ? mDrawerListEntries.size() : 0;
        }

        public boolean isEnabled(int position) {
            return (mDrawerListEntries.get(position).mStatus & ITEM_HEADER) == 0
                    && (mDrawerListEntries.get(position).mStatus & ITEM_DISENABLE) == 0;
        }

        public Object getItem(int position) {
            return mDrawerListEntries.get(position);
        }

        public long getItemId(int position) {
            return mDrawerListEntries.get(position).mId;
        }

        public View getView(int position, View convertView, ViewGroup parent) {

            ItemViewHolder itemHolder = null;
            DrawerListViewEntry entry = (DrawerListViewEntry) getItem(position);
            if (convertView == null) {
                itemHolder = new ItemViewHolder();
                convertView = inflater.inflate(R.layout.drawer_list_item, null);
                itemHolder.header = (TextView) convertView.findViewById(R.id.header);
                itemHolder.title = (TextView) convertView.findViewById(R.id.title);
                itemHolder.currentMode = (TextView) convertView.findViewById(R.id.current_mode);
                itemHolder.divider = convertView.findViewById(R.id.divider);
                itemHolder.contentLayout = (LinearLayout) convertView
                        .findViewById(R.id.item_content);
                convertView.setTag(itemHolder);
            } else {
                itemHolder = (ItemViewHolder) convertView.getTag();
            }

            // update item base on entry
            itemHolder.header.setVisibility((entry.mStatus & ITEM_HEADER) != 0 ? View.VISIBLE
                    : View.GONE);
            itemHolder.contentLayout
                    .setVisibility((entry.mStatus & ITEM_HEADER) == 0 ? View.VISIBLE : View.GONE);
            itemHolder.title.setVisibility((entry.mStatus & ITEM_HEADER) == 0 ? View.VISIBLE
                    : View.GONE);
            itemHolder.divider.setVisibility((entry.mStatus & ITEM_HEADER) == 0 ? View.VISIBLE
                    : View.GONE);

            if ((entry.mStatus & ITEM_HEADER) != 0) {
                itemHolder.header.setText(entry.mTitle);
            } else {
                itemHolder.title.setText(entry.mTitle);
                itemHolder.title.setEnabled(isEnabled(position));
                if (entry.mId == PICTURE_MODE_ID) {
                    int mode = MiraVisionJni.getLibStatus() ? MiraVisionJni.nativeGetPictureMode()
                            : -1;
                    int resId = -1;
                    if (mode == MiraVisionJni.PIC_MODE_STANDARD) {
                        resId = R.string.picture_mode_standard;
                    } else if (mode == MiraVisionJni.PIC_MODE_USER_DEF) {
                        resId = R.string.picture_mode_user;
                    } else if (mode == MiraVisionJni.PIC_MODE_VIVID) {
                        resId = R.string.picture_mode_vivid;
                    }
                    if (resId != -1) {
                        itemHolder.currentMode.setText(resId);
                    }
                }
                itemHolder.currentMode.setVisibility(entry.mId == PICTURE_MODE_ID ? View.VISIBLE
                        : View.GONE);
                itemHolder.divider
                        .setVisibility((entry.mStatus & ITEM_DIVIDER) == 0 ? View.INVISIBLE
                                : View.VISIBLE);
            }
            return convertView;
        }

        /**
         * Add list entry from xml
         */
        private void updateDrawerListEntry() {
            Log.d(TAG, "updateDrawerListEntry");
            XmlPullParser parser = mContext.getResources().getXml(R.xml.mira_vision_features);
            if (parser == null) {
                Log.e(TAG, "updateDrawerListEntry the package has no features xml ");
                return;
            }
            if (mDrawerListEntries == null) {
                mDrawerListEntries = new ArrayList<DrawerListViewEntry>();
            } else {
                mDrawerListEntries.clear();
            }
            try {
                int xmlEventType;
                int title = 0;
                int fragTitle = 0;
                int status = -1;
                int id = -1;
                while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                    String name = parser.getName();
                    if (xmlEventType == XmlPullParser.START_TAG && "Item".equals(name)) {
                        title = getIntStringId(parser.getAttributeValue(null, "ListItemTitle"));
                        fragTitle = getIntStringId(parser.getAttributeValue(null, "FragmentTitle"));
                        try {
                            status = Integer.valueOf(parser.getAttributeValue(null, "Status"));
                            id = Integer.valueOf(parser.getAttributeValue(null, "Id"));
                        } catch (NumberFormatException e) {
                            id = -1;
                        }
                        Log.d(TAG, "updateDrawerListEntry title = " + title + " , fragTitle = "
                                + fragTitle + " ,status = " + status + " ,id = " + id);
                    } else if (xmlEventType == XmlPullParser.END_TAG && "Item".equals(name)
                            && id != -1) {
                        mDrawerListEntries
                                .add(new DrawerListViewEntry(title, fragTitle, status, id));
                    }
                }
            } catch (XmlPullParserException e) {
                Log.e(TAG, "updateDrawerListEntry Got execption XmlPullParserException", e);
            } catch (IOException e) {
                Log.e(TAG, "updateDrawerListEntry Got execption IOException", e);
            } catch (Exception e) {
                Log.e(TAG, "updateDrawerListEntry Got execption. ", e);
            }
        }

        /**
         * use reflect get string id
         */
        private int getIntStringId(String idStr) {
            String classRStr = mContext.getPackageName() + "."
                    + idStr.substring(0, idStr.indexOf("."));
            String classStringStr = idStr.substring(idStr.indexOf("."), idStr.lastIndexOf("."));
            String filedStr = idStr.substring(idStr.lastIndexOf(".") + 1);
            Log.d(TAG, "getIntStringId classRStr = " + classRStr + ", classStringStr = "
                    + classStringStr + ", filedStr = " + filedStr);
            Class classR = null;
            Class classString = null;
            int result = -1;
            try {
                classR = Class.forName(classRStr);
            } catch (ClassNotFoundException e) {
                Log.d(TAG, "getIntStringId ClassNotFoundException");
                return result;
            }
            Class[] classes = classR.getClasses();
            for (int i = 0; i < classes.length; i++) {
                if (classes[i].getSimpleName().equals("string")) {
                    classString = classes[i];
                    break;
                }
            }
            Log.d(TAG, "getIntStringId classR = " + classR + " , classString = " + classString);
            try {
                result = classString.getField(filedStr).getInt(null);
            } catch (IllegalArgumentException e) {
                Log.d(TAG, "getIntStringId IllegalArgumentException");
            } catch (IllegalAccessException e) {
                Log.d(TAG, "getIntStringId IllegalAccessException");
            } catch (SecurityException e) {
                Log.d(TAG, "getIntStringId SecurityException");
            } catch (NoSuchFieldException e) {
                Log.d(TAG, "getIntStringId NoSuchFieldException");
            } catch (NullPointerException e) {
                Log.d(TAG, "getIntStringId NullPointerException");
            }
            return result;
        }

        /**
         * Base on eng mode and picture mode, update items of list
         */
        public void updateListItemsStatus() {
            if (mDrawerListEntries == null || mDrawerListEntries.size() == 0) {
                return;
            }
            ArrayList<DrawerListViewEntry> removeIds = new ArrayList<DrawerListViewEntry>();
            for (DrawerListViewEntry entry : mDrawerListEntries) {
                Log.d(TAG, "updateListItemsStatus title: " + entry.mTitle);
                Log.d(TAG, "updateListItemsStatus id: " + entry.mId);
                if (!MiraVisionJni.getLibStatus()) {
                    entry.mStatus = entry.mStatus | ITEM_DISENABLE;
                    continue;
                } else {
                    entry.mStatus = entry.mStatus & ~ITEM_DISENABLE;
                }
                switch ((int) entry.mId) {
                case AAL_ID:
                    String aal = Utils.getSysProperty("ro.vendor.mtk_aal_support", null);
                    if (aal == null || !aal.equals("1") || !ENG_MODE_STATUS) {
                        entry.mStatus = entry.mStatus | ITEM_INVISIBLE;
                        removeIds.add(entry);
                    }
                    break;
                case OD_ID:
                 // case DYNAMIC_CONTRAST_ID: // DC is visible on user load
                    // ALPS01679756
                    Log.d(TAG, "OD support: " + MiraVisionJni.nativeEnableODDemo(2));
                    if (!MiraVisionJni.nativeEnableODDemo(2) || !ENG_MODE_STATUS) {
                        entry.mStatus = entry.mStatus | ITEM_INVISIBLE;
                        removeIds.add(entry);
                    }
                    break;
                case BASIC_TUNING_ID:
                case ADVANCE_TUNING_ID:
                    if (!mIsReset
                       && MiraVisionJni.nativeGetPictureMode() == MiraVisionJni.PIC_MODE_USER_DEF) {
                        entry.mStatus = entry.mStatus & ~ITEM_DISENABLE;
                    } else {
                        entry.mStatus = entry.mStatus | ITEM_DISENABLE;
                    }
                    if (R.string.advance_color_item_temperature == entry.mTitle) {
                        // Factory gamma and Color temperature are mutually exclusive.
                        String gamma = Utils.getSysProperty("ro.mtk_factory_gamma_support", null);
                        if (gamma != null && gamma.equals("1")) {
                            entry.mStatus = entry.mStatus | ITEM_INVISIBLE;
                            removeIds.add(entry);
                            // if no color temperature, color sharpness no divider
                            for (DrawerListViewEntry subEntry : mDrawerListEntries) {
                                if (R.string.advance_color_item_sharpness == subEntry.mTitle) {
                                    subEntry.mStatus = subEntry.mStatus & ~ITEM_DIVIDER;
                                    break;
                                }
                            }
                        }
                    }
                    // When globalpq feature support,need remove sharpness item
                    if (R.string.advance_color_item_sharpness == entry.mTitle) {
                        String globalPq = Utils.getSysProperty("ro.globalpq.support", null);
                        if (globalPq != null && globalPq.equals("1")) {
                            entry.mStatus = entry.mStatus | ITEM_INVISIBLE;
                            removeIds.add(entry);
                        }
                    }
                    break;
                case VIDEO_ENHANCEMENT_ID:
                    // MDP color support
                    int mpdType =
                        Integer.valueOf(Utils.getSysProperty("ro.vendor.mtk_pq_color_mode", "0"));
                    if (R.string.color_effect_item == entry.mTitle) {
                        if (mpdType < 2) {
                            entry.mStatus = entry.mStatus | ITEM_INVISIBLE;
                            removeIds.add(entry);
                            // if no color effect, dynamic contrast no divider
                            for (DrawerListViewEntry subEntry : mDrawerListEntries) {
                                if (R.string.contrast_item == subEntry.mTitle) {
                                    subEntry.mStatus = subEntry.mStatus & ~ITEM_DIVIDER;
                                    break;
                                }
                            }
                        }
                    }
                    // When globalpq feature support,need remove Dynamic contrast item
                    String globalPq = Utils.getSysProperty("ro.globalpq.support", null);
                    if (mpdType < 2) {
                        if (globalPq != null && globalPq.equals("1")) {
                            // if no color effect and golobalpq support , remove Video Enhancement
                            entry.mStatus = entry.mStatus | ITEM_INVISIBLE;
                            removeIds.add(entry);
                        }
                    } else {
                        if (R.string.contrast_item == entry.mTitle) {
                            if (globalPq != null && globalPq.equals("1")) {
                                // if color effect exist , and golobalpq support
                                // remove dynamic contrast
                                entry.mStatus = entry.mStatus | ITEM_INVISIBLE;
                                removeIds.add(entry);
                            }
                        }
                    }
                    break;
                case BLULIGHT_DEFENDER_ID:
                    // BluLight defender & Chameleon Display
                    String blueLight =
                         Utils.getSysProperty("ro.vendor.mtk_blulight_def_support", null);
                    boolean bluLightSupport = (blueLight != null && blueLight.equals("1"));
                    String chameleon =
                         Utils.getSysProperty("ro.vendor.mtk_chameleon_support", null);
                    boolean chameleonDisplaySupport = (chameleon != null && chameleon.equals("1"));;
                    if (R.string.eye_care_header == entry.mTitle) {
                        if (!bluLightSupport && !chameleonDisplaySupport) {
                            removeIds.add(entry);
                        }
                    }
                    if (R.string.blue_light_item == entry.mTitle) {
                        if (!bluLightSupport) {
                            entry.mStatus = entry.mStatus | ITEM_INVISIBLE;
                            removeIds.add(entry);
                        } else {
                            if (!chameleonDisplaySupport) {
                                entry.mStatus = entry.mStatus & ~ITEM_DIVIDER;
                            }
                        }
                    }
                    if (R.string.chameleon_display_item == entry.mTitle) {
                        if (!chameleonDisplaySupport) {
                            removeIds.add(entry);
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            mDrawerListEntries.removeAll(removeIds);
        }

        @Override
        public void notifyDataSetChanged() {
            super.notifyDataSetChanged();
            Log.d(TAG, "notifyDataSetChanged");
            updateListItemsStatus();
        }
    }

    static class ItemViewHolder {
        TextView header;
        TextView title;
        TextView currentMode;
        View divider;
        LinearLayout contentLayout;
    }

    static class DrawerListViewEntry {
        public DrawerListViewEntry(int title, int fragTitle, int status, long id) {
            mTitle = title;
            mFragmentTitle = fragTitle;
            mStatus = status;
            mId = id;
            Log.d(TAG, "DrawerListViewEntry mTitle: " + mTitle + " mStatus: " + mStatus
                    + " mFragmentTitle: " + mFragmentTitle + " mId " + mId);
        }

        long mId;
        int mTitle;
        int mFragmentTitle;
        int mStatus;
    }

    class DockDividerVisibilityListener extends IDockedStackListener.Stub {

        private Activity mActivity;

        public DockDividerVisibilityListener(Activity activity) {
            mActivity = activity;
        }

        @Override
        public void onDividerVisibilityChanged(boolean visible) throws RemoteException {
            Log.i(TAG, "window divivider changed, visible: " + visible + " mUpdateOrientation : "
                    + sUpdateOrientation);
            if (!visible) {
                if (sUpdateOrientation) {
                    mActivity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                } else {
                    mActivity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
                }
            }
        }

        @Override
        public void onDockedStackExistsChanged(boolean exists) throws RemoteException {
        }

        @Override
        public void onDockedStackMinimizedChanged(boolean minimized, long animDuration,
                            boolean isHomeStackResizable)
                throws RemoteException {
        }

        @Override
        public void onAdjustedForImeChanged(boolean adjustedForIme, long animDuration)
                throws RemoteException {

        }

        @Override
        public void onDockSideChanged(final int newDockSide) throws RemoteException {
        }
    }
}
