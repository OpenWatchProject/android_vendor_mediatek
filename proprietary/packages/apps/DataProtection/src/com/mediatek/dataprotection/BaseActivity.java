package com.mediatek.dataprotection;

import android.app.Activity;
import android.Manifest;
import android.os.Bundle;
import android.widget.Toast;

import com.mediatek.dataprotection.utils.DataProtectionLockPatternUtils;
import com.mediatek.dataprotection.utils.PermissionUtils;
import com.mediatek.dataprotection.utils.Log;

public class BaseActivity extends Activity {

    private static final String TAG = "BaseActivity";
    protected static final String TAG_CANCEL_DIALG = "cacel_dialog_main";
    public static final String EXPIRED = "ExPired";
    protected static final int REQUEST_READ_WRITE_STORAGE_PERMISSION_CODE = 3;
    protected boolean mHasPriviledge = false;
    protected DataProtectionService mService = null;
    protected long mCancelTaskId;
    protected int mCancelResId;
    protected boolean mNeedRestoreCancelDialog = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (DataProtectionLockPatternUtils.hasPattern(this) &&
            !PermissionUtils.hasStorageReadWritePermission(this) &&
            !PermissionUtils.showReadOrWriteRational(this)) {
                String toastStr = getString(R.string.denied_required_permission);
                Toast.makeText(getApplicationContext(), toastStr, Toast.LENGTH_LONG).show();
                finish();
        }

        if (!mHasPriviledge) {
            // if cancel dialog exists and no permission, need cancel dialog
            Log.d(TAG, "cancel dialog exists and no permission, need cancel dialog");
            mNeedRestoreCancelDialog = AlertDialogFragment.dismissCancelTaskDialog(
                    BaseActivity.this, mService, mCancelResId,
                    TAG_CANCEL_DIALG, mCancelTaskId);
        }
    }

    protected void showCancelTaskDialog() {
        if (mNeedRestoreCancelDialog) {
            Log.d(TAG, "onPatternVerifySuccess - show cancel dialog");
            AlertDialogFragment.showCancelTaskDialog(
                    BaseActivity.this, mService, mCancelResId,
                    TAG_CANCEL_DIALG, mCancelTaskId);
            mNeedRestoreCancelDialog = false;
        }
    }

    protected void setTaskIdAndResId(long taskId, int resId) {
        // record task id, res id
        Log.d(TAG, "onCancel - set task id: " + taskId + ", res id: " + resId);
        mCancelTaskId = taskId;
        mCancelResId = resId;
    }

    protected boolean isExpired() {
        Log.d(TAG, "T1: " + getIntent().getIntExtra(EXPIRED, 0) + " T2: " +
                android.os.Process.myPid());

        if (getIntent().getIntExtra(EXPIRED, 0) == android.os.Process.myPid()) {
            return false;
        } else {
            return true;
        }
    }

    protected boolean hasReadWritePermission() {
        return PermissionUtils.hasStorageReadPermission(this) &&
            PermissionUtils.hasStorageWritePermission(this);
    }

    protected void requestReadWritePermission() {
        PermissionUtils.requestReadWriteStoragePermission(this,
            REQUEST_READ_WRITE_STORAGE_PERMISSION_CODE);
    }
}
