/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.settings.sim;

import android.app.Activity;
/// M: Record the dialog to dismiss it when activity destroyed.
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
/// M: Dismis the dialog when receiving back key or cancel event. @{
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnKeyListener;
/// @}
import android.content.res.Resources;
import android.graphics.Paint;
import android.graphics.drawable.ShapeDrawable;
import android.graphics.drawable.shapes.OvalShape;
import android.os.Bundle;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
/// M: Dismiss the dialog when receiving back key or cancel event.
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.appcompat.app.AlertDialog;

import com.android.settings.R;
import com.android.settings.Utils;

/// M: Add for SIM settings plugin.
import com.mediatek.settings.UtilsExt;
import com.mediatek.settings.ext.ISimManagementExt;
/// @}
/// M: Add for supporting SIM hot swap. @{
import com.mediatek.settings.sim.SimHotSwapHandler;
import com.mediatek.settings.sim.SimHotSwapHandler.OnSimHotSwapListener;
/// @}

public class SimPreferenceDialog extends Activity {

    private static final String TAG = "SimPreferenceDialog";

    private Context mContext;
    private SubscriptionInfo mSubInfoRecord;
    private int mSlotId;
    private int[] mTintArr;
    private String[] mColorStrings;
    private int mTintSelectorPos;
    private SubscriptionManager mSubscriptionManager;
    AlertDialog.Builder mBuilder;
    View mDialogLayout;
    private final String SIM_NAME = "sim_name";
    private final String TINT_POS = "tint_pos";

    /// M: Add for SIM settings plugin.
    private ISimManagementExt mSimManagementExt;

    /// M: Add for supporting SIM hot swap.
    private SimHotSwapHandler mSimHotSwapHandler;

    /// M: Record the dialog to dismiss it when activity destroyed.
    private Dialog mDialog;

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        mContext = this;
        final Bundle extras = getIntent().getExtras();
        mSlotId = extras.getInt(SimSettings.EXTRA_SLOT_ID, -1);
        mSubscriptionManager = SubscriptionManager.from(mContext);
        mSubInfoRecord = mSubscriptionManager.getActiveSubscriptionInfoForSimSlotIndex(mSlotId);

        /// M: For ALPS02368883, the sub info may be null when hot swap happened. @{
        if (mSubInfoRecord == null) {
            Log.w(TAG, "Sub info record is null, finish the activity.");
            finish();
            return;
        }
        /// @}

        mTintArr = mContext.getResources().getIntArray(com.android.internal.R.array.sim_colors);
        mColorStrings = mContext.getResources().getStringArray(R.array.color_picker);
        mTintSelectorPos = 0;

        mBuilder = new AlertDialog.Builder(mContext);
        LayoutInflater inflater = (LayoutInflater)mContext
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mDialogLayout = inflater.inflate(R.layout.multi_sim_dialog, null);
        mBuilder.setView(mDialogLayout);

        /// M: Add for SIM settings plugin.
        mSimManagementExt = UtilsExt.getSimManagementExt(getApplicationContext());

        createEditDialog(bundle);

        /// M: Add for supporting SIM hot swap. @{
        mSimHotSwapHandler = new SimHotSwapHandler(getApplicationContext());
        mSimHotSwapHandler.registerOnSimHotSwap(new OnSimHotSwapListener() {
            @Override
            public void onSimHotSwap() {
                Log.d(TAG, "onSimHotSwap, finish Activity.");
                finish();
            }
        });
        /// @}
    }

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        savedInstanceState.putInt(TINT_POS, mTintSelectorPos);

        final EditText nameText = (EditText)mDialogLayout.findViewById(R.id.sim_name);
        savedInstanceState.putString(SIM_NAME, nameText.getText().toString());

        super.onSaveInstanceState(savedInstanceState);

    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);

        int pos = savedInstanceState.getInt(TINT_POS);
        final Spinner tintSpinner = (Spinner) mDialogLayout.findViewById(R.id.spinner);
        tintSpinner.setSelection(pos);
        mTintSelectorPos = pos;

        EditText nameText = (EditText)mDialogLayout.findViewById(R.id.sim_name);
        nameText.setText(savedInstanceState.getString(SIM_NAME));
        Utils.setEditTextCursorPosition(nameText);
    }

    private void createEditDialog(Bundle bundle) {
        final Resources res = mContext.getResources();
        EditText nameText = (EditText)mDialogLayout.findViewById(R.id.sim_name);
        nameText.setText(mSubInfoRecord.getDisplayName());
        Utils.setEditTextCursorPosition(nameText);
        final Spinner tintSpinner = (Spinner) mDialogLayout.findViewById(R.id.spinner);
        SelectColorAdapter adapter = new SelectColorAdapter(mContext,
                R.layout.settings_color_picker_item, mColorStrings);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        tintSpinner.setAdapter(adapter);

        for (int i = 0; i < mTintArr.length; i++) {
            if (mTintArr[i] == mSubInfoRecord.getIconTint()) {
                tintSpinner.setSelection(i);
                mTintSelectorPos = i;
                break;
            }
        }

        tintSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view,
                                       int pos, long id){
                tintSpinner.setSelection(pos);
                mTintSelectorPos = pos;
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        final TelephonyManager tm = (TelephonyManager) mContext.getSystemService(
                Context.TELEPHONY_SERVICE);
        TextView numberView = (TextView)mDialogLayout.findViewById(R.id.number);
        final String rawNumber =  tm.getLine1Number(mSubInfoRecord.getSubscriptionId());
        if (TextUtils.isEmpty(rawNumber)) {
            numberView.setText(res.getString(com.android.internal.R.string.unknownName));
        } else {
            numberView.setText(PhoneNumberUtils.formatNumber(rawNumber));
        }

        String simCarrierName = tm.getSimOperatorName(mSubInfoRecord.getSubscriptionId());
        TextView carrierView = (TextView)mDialogLayout.findViewById(R.id.carrier);
        carrierView.setText(!TextUtils.isEmpty(simCarrierName) ? simCarrierName :
                mContext.getString(com.android.internal.R.string.unknownName));

        mBuilder.setTitle(String.format(res.getString(R.string.sim_editor_title),
                (mSubInfoRecord.getSimSlotIndex() + 1)));

        mBuilder.setPositiveButton(R.string.okay, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int whichButton) {
                final EditText nameText = (EditText)mDialogLayout.findViewById(R.id.sim_name);
                Utils.setEditTextCursorPosition(nameText);
                String displayName = nameText.getText().toString();
                int subId = mSubInfoRecord.getSubscriptionId();
                mSubInfoRecord.setDisplayName(displayName);
                mSubscriptionManager.setDisplayName(displayName, subId,
                        SubscriptionManager.NAME_SOURCE_USER_INPUT);

                final int tintSelected = tintSpinner.getSelectedItemPosition();
                int subscriptionId = mSubInfoRecord.getSubscriptionId();
                int tint = mTintArr[tintSelected];
                mSubInfoRecord.setIconTint(tint);
                mSubscriptionManager.setIconTint(tint, subscriptionId);
                dialog.dismiss();
            }
        });

        mBuilder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int whichButton) {
                dialog.dismiss();
            }
        });

        /// M: Dismiss the dialo when receiving back key or cancel event. @{
        mBuilder.setOnKeyListener(new OnKeyListener() {
            @Override
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                if (keyCode == KeyEvent.KEYCODE_BACK) {
                    dialog.dismiss();
                    return true;
                }
                return false;
            }
        });

        mBuilder.setOnCancelListener(new OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                dialog.dismiss();
            }
        });
        /// @}

        mBuilder.setOnDismissListener(new DialogInterface.OnDismissListener() {
           @Override
           public void onDismiss(DialogInterface dialogInListener) {
               finish();
           }
        });

        /// M: Add for SIM settings plugin.
        mSimManagementExt.hideSimEditorView(mDialogLayout, mContext);

        /// M: Record the dialog to dismiss it when activity destroyed. @{
        mDialog = mBuilder.create();
        mDialog.show();
        /// @}
    }

    private class SelectColorAdapter extends ArrayAdapter<CharSequence> {
        private Context mContext;
        private int mResId;

        public SelectColorAdapter(
                Context context, int resource, String[] arr) {
            super(context, resource, arr);
            mContext = context;
            mResId = resource;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LayoutInflater inflater = (LayoutInflater)
                    mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

            View rowView;
            final ViewHolder holder;
            Resources res = mContext.getResources();
            int iconSize = res.getDimensionPixelSize(R.dimen.color_swatch_size);
            int strokeWidth = res.getDimensionPixelSize(R.dimen.color_swatch_stroke_width);

            if (convertView == null) {
                // Cache views for faster scrolling
                rowView = inflater.inflate(mResId, null);
                holder = new ViewHolder();
                ShapeDrawable drawable = new ShapeDrawable(new OvalShape());
                drawable.setIntrinsicHeight(iconSize);
                drawable.setIntrinsicWidth(iconSize);
                drawable.getPaint().setStrokeWidth(strokeWidth);
                holder.label = (TextView) rowView.findViewById(R.id.color_text);
                holder.icon = (ImageView) rowView.findViewById(R.id.color_icon);
                holder.swatch = drawable;
                rowView.setTag(holder);
            } else {
                rowView = convertView;
                holder = (ViewHolder) rowView.getTag();
            }

            holder.label.setText(getItem(position));
            holder.swatch.getPaint().setColor(mTintArr[position]);
            holder.swatch.getPaint().setStyle(Paint.Style.FILL_AND_STROKE);
            holder.icon.setVisibility(View.VISIBLE);
            holder.icon.setImageDrawable(holder.swatch);
            return rowView;
        }

        @Override
        public View getDropDownView(int position, View convertView, ViewGroup parent) {
            View rowView = getView(position, convertView, parent);
            final ViewHolder holder = (ViewHolder) rowView.getTag();

            if (mTintSelectorPos == position) {
                holder.swatch.getPaint().setStyle(Paint.Style.FILL_AND_STROKE);
            } else {
                holder.swatch.getPaint().setStyle(Paint.Style.STROKE);
            }
            holder.icon.setVisibility(View.VISIBLE);
            return rowView;
        }

        private class ViewHolder {
            TextView label;
            ImageView icon;
            ShapeDrawable swatch;
        }
    }

    @Override
    protected void onDestroy() {
        /// M: Dismiss the dialog when activity destroyed. @{
        if (mDialog != null && mDialog.isShowing()) {
            mDialog.dismiss();
            mDialog = null;
        }
        /// @}

        /// M: Add for supporting SIM hot swap. @{
        if (mSimHotSwapHandler != null) {
            mSimHotSwapHandler.unregisterOnSimHotSwap();
        }
        /// @}

        super.onDestroy();
    };
}
