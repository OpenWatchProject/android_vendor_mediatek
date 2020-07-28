package com.mediatek.contacts.aas;

import android.app.ActionBar;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.android.contacts.R;
import com.android.contacts.activities.RequestPermissionsActivity;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.contacts.aas.AlertDialogFragment.EditTextDialogFragment;
import com.mediatek.contacts.aas.AlertDialogFragment.EditTextDialogFragment.EditTextDoneListener;
import com.mediatek.contacts.aas.MessageAlertDialogFragment.AlertConfirmedListener;
import com.mediatek.contacts.aassne.SimAasSneUtils;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.internal.telephony.phb.AlphaTag;

public class AasTagActivity extends Activity {
    private static final String TAG = "AasTagActivity";

    private static final String CREATE_AAS_DIALOG = "create_aas_dialog";
    private static final String EDIT_AAS_DIALOG = "edit_aas_dialog";
    private static final String DELETE_CONFIRM_DIALOG = "delete_confirm_dialog";
    private static final String EDIT_CONFIRM_DIALOG = "edit_confirm_dialog";

    private AasTagInfoAdapter mAasAdapter = null;
    private int mSubId = -1;
    private View mActionBarEdit = null;
    private TextView mSelectedView = null;
    private ToastHelper mToastHelper = null;
    private AlphaTag mCurrentEditAlphaTag = null;

    private static final String ADAPTER_MODE = "adapter_mode";
    private static final String ADAPTER_CHECKED_ARRAY = "adapter_checked_array";
    private static final String CURRENT_EDIT_ALPHATAG = "current_edit_alphaTag";

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        Log.d(TAG, "[onSaveInstanceState] mode() = " + mAasAdapter.getMode());
        outState.putInt(ADAPTER_MODE, mAasAdapter.getMode());
        outState.putIntArray(ADAPTER_CHECKED_ARRAY, mAasAdapter.getCheckedIndexArray());
        outState.putParcelable(CURRENT_EDIT_ALPHATAG, mCurrentEditAlphaTag);
        super.onSaveInstanceState(outState);
    }

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "[onCreate]");

        // Move registerReceiver to top to ensure unregister correctly in onDestroy().
        registerReceiver(mPhbStateListener, new IntentFilter(
                TelephonyIntents.ACTION_PHB_STATE_CHANGED));
        // skip onCreate() if permission is not granted to void permission JE.
        if (RequestPermissionsActivity.startPermissionActivityIfNeeded(this)) {
            Log.d(TAG, "[onCreate] return directly due to no permissions !");
            return;
        }
        PhbInfoUtils.getActiveUsimPhbInfoMap();
        setContentView(R.layout.custom_aas);

        Intent intent = getIntent();
        if (intent != null) {
            mSubId = intent.getIntExtra(SimAasSneUtils.KEY_SUB_ID, -1);
        }

        if (mSubId == -1) {
            Log.e(TAG, "[onCreate] Eorror slotId=-1, finish the AasTagActivity");
            finish();
        }
        ListView listView = (ListView) findViewById(R.id.custom_aas);
        mAasAdapter = new AasTagInfoAdapter(this, mSubId);
        mAasAdapter.updateAlphaTags();
        listView.setAdapter(mAasAdapter);
        if (savedInstanceState != null) {
            mAasAdapter.setMode(savedInstanceState.getInt(ADAPTER_MODE));
            mAasAdapter.setCheckedByIndexArray(
                    savedInstanceState.getIntArray(ADAPTER_CHECKED_ARRAY));
            mCurrentEditAlphaTag = savedInstanceState.getParcelable(CURRENT_EDIT_ALPHATAG);
        }
        mToastHelper = new ToastHelper(this);
        listView.setOnItemClickListener(new ListItemClickListener());

        initActionBar();
    }

    public void initActionBar() {
        ActionBar actionBar = getActionBar();
        LayoutInflater inflate = getLayoutInflater();
        View customView = inflate.inflate(R.layout.custom_aas_action_bar, null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM
                | ActionBar.DISPLAY_SHOW_TITLE | ActionBar.DISPLAY_HOME_AS_UP);

        mActionBarEdit = customView.findViewById(R.id.action_bar_edit);
        mSelectedView = (TextView) customView.findViewById(R.id.selected);
        ImageView selectedIcon = (ImageView) customView.findViewById(R.id.selected_icon);
        selectedIcon.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setMode(AasTagInfoAdapter.MODE_NORMAL);
                updateActionBar();
            }
        });
        actionBar.setCustomView(customView);

        updateActionBar();
    }

    public void updateActionBar() {
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_NORMAL)) {
                actionBar.setDisplayOptions(ActionBar.DISPLAY_HOME_AS_UP,
                        ActionBar.DISPLAY_SHOW_TITLE | ActionBar.DISPLAY_HOME_AS_UP);

                actionBar.setDisplayHomeAsUpEnabled(true);
                actionBar.setDisplayShowTitleEnabled(true);
                actionBar.setTitle(R.string.aas_custom_title);
                mActionBarEdit.setVisibility(View.GONE);
            } else {
                actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                        ActionBar.DISPLAY_SHOW_CUSTOM);

                actionBar.setDisplayHomeAsUpEnabled(false);
                actionBar.setDisplayShowTitleEnabled(false);
                mActionBarEdit.setVisibility(View.VISIBLE);
                String select = getResources().getString(R.string.selected_item_count,
                        mAasAdapter.getCheckedItemCount());
                mSelectedView.setText(select);
            }
        }
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        Log.d(TAG, "[onPrepareOptionsMenu]");
        MenuInflater inflater = getMenuInflater();
        menu.clear();
        if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_NORMAL)) {
            inflater.inflate(R.menu.custom_normal_menu, menu);
        } else {
            inflater.inflate(R.menu.custom_edit_menu, menu);
        }

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        super.onOptionsItemSelected(item);
        Log.d(TAG, "[onOptionsItemSelected]");
        if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_NORMAL)) {
            switch (item.getItemId()) {
            case R.id.menu_add_new:
                if (!mAasAdapter.isFull()) {
                    showNewAasDialog();
                } else {
                    mToastHelper.showToast(R.string.aas_usim_full);
                }
                break;
            case R.id.menu_deletion:
                setMode(AasTagInfoAdapter.MODE_EDIT);
                break;
            case android.R.id.home:
                finish();
                break;
            default:
            }
        } else {
            switch (item.getItemId()) {
            case R.id.menu_select_all:
                mAasAdapter.setAllChecked(true);
                updateActionBar();
                break;
            case R.id.menu_disselect_all:
                mAasAdapter.setAllChecked(false);
                updateActionBar();
                break;
            case R.id.menu_delete:
                // mAasAdapter.deleteCheckedAasTag();
                if (mAasAdapter.getCheckedItemCount() == 0) {
                    mToastHelper.showToast(R.string.multichoice_no_select_alert);
                } else {
                    showDeleteAlertDialog();
                }
                break;
            default:
                break;
            }
        }
        return true;
    }

    public void setMode(int mode) {
        mAasAdapter.setMode(mode);
        updateActionBar();
        invalidateOptionsMenu();
    }

    @Override
    public void onBackPressed() {
        if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_EDIT)) {
            setMode(AasTagInfoAdapter.MODE_NORMAL);
        } else {
            super.onBackPressed();
        }
    }

    public EditTextDoneListener getEditTextDoneListener(String dialogTag) {
        if (dialogTag.equals(CREATE_AAS_DIALOG)) {
            return new NewAlpahTagListener();
        } else if (dialogTag.equals(EDIT_AAS_DIALOG)) {
            return new EditAlpahTagListener(mCurrentEditAlphaTag);
        } else {
            throw new IllegalArgumentException("unknown dialogTag!");
        }
    }

    protected void showNewAasDialog() {
        EditTextDialogFragment createItemDialogFragment = EditTextDialogFragment.newInstance(
                R.string.aas_new_dialog_title, android.R.string.cancel, android.R.string.ok, "");
        createItemDialogFragment.show(getFragmentManager(), CREATE_AAS_DIALOG);
    }

    final private class NewAlpahTagListener implements EditTextDoneListener {

        @Override
        public void onEditTextDone(String text) {
            Log.d("NewAlpahTagListener", "[onEditTextDone] text = " + Log.anonymize(text));
            if (mAasAdapter.isExist(text)) {
                mToastHelper.showToast(R.string.aas_name_exist);
            } else if (!SimAasSneUtils.isAasTextValid(text, mSubId)) {
                mToastHelper.showToast(R.string.aas_name_invalid);
            } else {
                int aasIndex = SimAasSneUtils.insertUSIMAAS(mSubId, text);
                Log.d("NewAlpahTagListener", "[onEditTextDone] aasIndex = " + aasIndex);
                if (aasIndex > 0) {
                    mAasAdapter.updateAlphaTags();
                } else {
                    mToastHelper.showToast(R.string.aas_new_fail);
                }
            }
        }
    }

    protected void showEditAasDialog(AlphaTag alphaTag) {
        mCurrentEditAlphaTag = alphaTag;
        if (alphaTag == null) {
            Log.e(TAG, "[showEditAasDialog] alphaTag is null,");
            return;
        }
        final String text = alphaTag.getAlphaTag();
        EditTextDialogFragment editDialogFragment = EditTextDialogFragment.newInstance(
                R.string.ass_rename_dialog_title, android.R.string.cancel,
                android.R.string.ok, text);
        editDialogFragment.show(getFragmentManager(), EDIT_AAS_DIALOG);
    }

    final private class EditAlpahTagListener implements EditTextDoneListener {
        private AlphaTag mAlphaTag;

        public EditAlpahTagListener(AlphaTag alphaTag) {
            mAlphaTag = alphaTag;
        }

        @Override
        public void onEditTextDone(String text) {
            Log.e("EditAlpahTagListener", "[onEditTextDone] text=" + Log.anonymize(text)
                    + ", mAlphaTag.getAlphaTag()=" + Log.anonymize(mAlphaTag.getAlphaTag()));
            if (mAlphaTag.getAlphaTag().equals(text)) {
                return;
            }
            if (mAasAdapter.isExist(text)) {
                mToastHelper.showToast(R.string.aas_name_exist);
            } else if (!SimAasSneUtils.isAasTextValid(text, mSubId)) {
                mToastHelper.showToast(R.string.aas_name_invalid);
            } else {
                showEditAssertDialog(mAlphaTag, text);
            }
        }
    }

    public AlertConfirmedListener getAlertConfirmedListener(String dialogTag) {
        Log.d(TAG, "[getAlertConfirmedListener] dialogTag = " + dialogTag);
        if (dialogTag.equals(EDIT_CONFIRM_DIALOG)) {
            return new EditAssertListener(mCurrentEditAlphaTag);
        } else if (dialogTag.equals(DELETE_CONFIRM_DIALOG)) {
            return new DeletionListener();
        } else {
            throw new IllegalArgumentException("unknown dialogTag!");
        }
    }

    private void showEditAssertDialog(AlphaTag alphaTag, String targetName) {
        MessageAlertDialogFragment editAssertDialogFragment = MessageAlertDialogFragment
                .newInstance(android.R.string.dialog_alert_title,
                        R.string.ass_edit_assert_message, true, targetName);
        editAssertDialogFragment.show(getFragmentManager(), EDIT_CONFIRM_DIALOG);
    }

    final private class EditAssertListener implements AlertConfirmedListener {
        private AlphaTag mAlphaTag = null;

        public EditAssertListener(AlphaTag alphaTag) {
            mAlphaTag = alphaTag;
        }

        @Override
        public void onMessageAlertConfirmed(String text) {
            Log.d("EditAssertListener", "[onMessageAlertConfirmed] text = "
                    + Log.anonymize(text));
            boolean flag = SimAasSneUtils.updateUSIMAAS(mSubId, mAlphaTag.getRecordIndex(),
                    mAlphaTag.getPbrIndex(), text);
            if (flag) {
                mAasAdapter.updateAlphaTags();
            } else {
                String msg = getResources().getString(R.string.aas_edit_fail,
                        mAlphaTag.getAlphaTag());
                mToastHelper.showToast(msg);
            }
        }
    }

    protected void showDeleteAlertDialog() {
        MessageAlertDialogFragment deleteDialogFragment = MessageAlertDialogFragment.newInstance(
                android.R.string.dialog_alert_title, R.string.aas_delele_dialog_message, true, "");
        deleteDialogFragment.show(getFragmentManager(), DELETE_CONFIRM_DIALOG);
    }

    final private class DeletionListener implements AlertConfirmedListener {
        @Override
        public void onMessageAlertConfirmed(String text) {
            Log.d("DeletionListener", "[onMessageAlertConfirmed]");
            mAasAdapter.deleteCheckedAasTag();
            setMode(AasTagInfoAdapter.MODE_NORMAL);
        }
    }

    public class ListItemClickListener implements OnItemClickListener {

        @Override
        public void onItemClick(AdapterView<?> view, View v, int pos, long arg3) {
            if (mAasAdapter.isMode(AasTagInfoAdapter.MODE_NORMAL)) {
                showEditAasDialog(mAasAdapter.getItem(pos).mAlphaTag);
            } else {
                mAasAdapter.updateChecked(pos);
                invalidateOptionsMenu();
                updateActionBar();
            }
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mPhbStateListener);
    }

    private BroadcastReceiver mPhbStateListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, -1);
            boolean phbReady = intent.getBooleanExtra("ready", true);
            Log.d(TAG, "[onReceive] mPhbStateListener subId:" + subId + ",phbReady:" + phbReady);
            // for phb state change
            if (subId == mSubId && !phbReady) {
                Log.d(TAG, "[onReceive] subId: " + subId);
                finish();
            }
        }
    };

    /**
     * Add for ALPS02613851, do not destroy when configuration change.
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }
}
