
package com.mediatek.gallery3d.video;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.widget.Toast;

import com.mediatek.gallery3d.util.Log;
import com.mediatek.galleryportable.RuntimePermissionUtils;

import java.util.ArrayList;

public class RequestPermissionActivity extends Activity {

    private static final String TAG = "VP_PermissionActivity";
    public static final String PREVIOUS_ACTIVITY_INTENT = "previous_intent";
    /** Whether the permissions activity was already started. */
    public static final String STARTED_PERMISSIONS_ACTIVITY = "started_permissions_activity";
    private static final int PERMISSIONS_REQUEST_ALL_PERMISSIONS = 0;

    private Intent mPreviousActivityIntent;
    private ArrayList<String> mPermissionsNeeded = new ArrayList<String>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.v(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        mPreviousActivityIntent = (Intent) getIntent().getExtras().get(PREVIOUS_ACTIVITY_INTENT);
        // Only start a requestPermissions() flow when first starting this
        // activity the first time. The process is likely to be restarted
        // during the permission flow (necessary to enable permissions)
        // so this is important to track.
        if (savedInstanceState == null) {
            requestPermissions(mPreviousActivityIntent.getData());
        }
    }

    /**
     * If any permissions the videoplayer app needs are missing, open an
     * Activity to prompt the user for these permissions. Moreover, finish the
     * current activity. This is designed to be called inside
     * {@link android.app.Activity#onCreate}
     */
    public static boolean startPermissionActivity(Activity activity) {
        Log.v(TAG, "startPermissionActivity from " + activity);
        boolean ret = false;
        Uri uri = activity.getIntent().getData();
        if (!hasPermissions(activity, uri)) {
            final Intent intent = new Intent(activity, RequestPermissionActivity.class);
            activity.getIntent().putExtra(STARTED_PERMISSIONS_ACTIVITY, true);
            intent.putExtra(PREVIOUS_ACTIVITY_INTENT, activity.getIntent());
            activity.startActivity(intent);
            activity.finish();
            ret = true;
        }
        Log.v(TAG, "startPermissionActivity return " + ret);
        return ret;
    }

    private static boolean hasPermissions(Context context, Uri uri) {
        boolean ret = true;
        // Only media provider or file uri need grant permissions at here.
        // if the uri come from other provider, the uri should already granted,
        // otherwise, need check by other apps.
        if (uri != null && !uri.toString().startsWith("content://media") &&
                !uri.toString().startsWith("file")) {
            Log.d(TAG, "no need check permission, directly return");
            return true;
        }
        // get permissions needed in current scenario
        ArrayList<String> permissionsNeeded = new ArrayList<String>();
        permissionsNeeded.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        permissionsNeeded.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        // check status of all needed permissions
        for (String permission : permissionsNeeded) {
            if (ContextCompat.checkSelfPermission(context, permission)
                != PackageManager.PERMISSION_GRANTED) {
                Log.v(TAG, "permission[" + permission + "] is required");
                ret = false;
            }
        }
        Log.v(TAG, "hasPermissions return " + ret);
        return ret;
    }

    private void requestPermissions(Uri uri) {
        Log.v(TAG, "requestPermissions  uri = " + uri);
        // get permissions needed in current scenario
        mPermissionsNeeded.clear();
        mPermissionsNeeded.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        mPermissionsNeeded.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        // Construct a list of missing permissions
        final ArrayList<String> unsatisfiedPermissions = new ArrayList<String>();
        for (String permission : mPermissionsNeeded) {
            if (ContextCompat.checkSelfPermission(this, permission)
                != PackageManager.PERMISSION_GRANTED) {
                unsatisfiedPermissions.add(permission);
            }
        }
        if (unsatisfiedPermissions.size() == 0) {
            throw new RuntimeException("Request permission activity was called even"
                    + " though all permissions are satisfied.");
        }
        ActivityCompat.requestPermissions(
                this,
                unsatisfiedPermissions.toArray(new String[unsatisfiedPermissions.size()]),
                PERMISSIONS_REQUEST_ALL_PERMISSIONS);
    }

    private boolean isAllGranted(String permissions[], int[] grantResult) {
        boolean ret = true;
        for (int i = 0; i < permissions.length; i++) {
            if (grantResult[i] != PackageManager.PERMISSION_GRANTED
                    && isPermissionRequired(permissions[i])) {
                ret = false;
                break;
            }
        }
        Log.v(TAG, "isAllGranted, return " + ret);
        return ret;
    }

    private boolean isPermissionRequired(String p) {
        boolean ret = mPermissionsNeeded.contains(p);
        Log.v(TAG, "isPermissionRequired, permission = " + p + ", ret = " + ret);
        return ret;
    }

    @Override
    public void onRequestPermissionsResult(
            int requestCode, String permissions[], int[] grantResults) {
        if (permissions != null && permissions.length > 0
                && isAllGranted(permissions, grantResults)) {
            Log.v(TAG, "onRequestPermissionsResult, start previous activity");
            mPreviousActivityIntent.setFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
            startActivity(mPreviousActivityIntent);
            finish();
            overridePendingTransition(0, 0);
        } else {
            Log.v(TAG, "onRequestPermissionsResult, deny request permission");
            try {
                Toast.makeText(this, RuntimePermissionUtils.getDeniedPermissionString(this),
                        Toast.LENGTH_SHORT).show();
            } catch (Exception e) {
                Log.e(TAG, "resource 'denied_required_permission' not existed");
                e.printStackTrace();
            }
            finish();
        }
    }
}
