package com.mediatek.mms.util;

import java.util.ArrayList;

import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;
import com.mediatek.mms.ui.PermissionCheckActivity;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.util.Log;
import android.widget.Toast;

import com.android.mms.R;

public class PermissionCheckUtil {
    private static final String TAG = "PermissionCheckUtil";
    public static final int REQUIRED_PERMISSIONS_REQUEST_CODE = 666;
    public static final String[] ALL_PERMISSIONS = {
      //  Manifest.permission.READ_EXTERNAL_STORAGE,
      //  Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.CALL_PHONE,
        Manifest.permission.READ_PHONE_STATE,
        Manifest.permission.READ_CONTACTS,
        Manifest.permission.WRITE_CONTACTS,
        Manifest.permission.RECEIVE_SMS,
        Manifest.permission.RECEIVE_MMS,
        Manifest.permission.SEND_SMS,
        Manifest.permission.READ_SMS,
        Manifest.permission.WRITE_SMS,
        Manifest.permission.RECEIVE_WAP_PUSH,
    };

    public static final String[] REQUIRED_PERMISSIONS = {
        Manifest.permission.CALL_PHONE,
        Manifest.permission.READ_PHONE_STATE,
        Manifest.permission.READ_CONTACTS,
        Manifest.permission.WRITE_CONTACTS,
        Manifest.permission.RECEIVE_SMS,
        Manifest.permission.RECEIVE_MMS,
        Manifest.permission.SEND_SMS,
        Manifest.permission.READ_SMS,
        Manifest.permission.WRITE_SMS,
        Manifest.permission.RECEIVE_WAP_PUSH,
    };

    public static final String[] ALL_CTA_PERMISSIONS = {
//        com.mediatek.Manifest.permission.CTA_ENABLE_BT,
        com.mediatek.Manifest.permission.CTA_SEND_MMS,
     //   Manifest.permission.READ_EXTERNAL_STORAGE,
     //   Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.CALL_PHONE,
        Manifest.permission.READ_PHONE_STATE,
        Manifest.permission.READ_CONTACTS,
        Manifest.permission.WRITE_CONTACTS,
        Manifest.permission.RECEIVE_SMS,
        Manifest.permission.RECEIVE_MMS,
        Manifest.permission.SEND_SMS,
        Manifest.permission.READ_SMS,
        Manifest.permission.WRITE_SMS,
        Manifest.permission.RECEIVE_WAP_PUSH,
    };

    public static final String[] REQUIRED_CTA_PERMISSIONS = {
  //      com.mediatek.Manifest.permission.CTA_ENABLE_BT,
        com.mediatek.Manifest.permission.CTA_SEND_MMS,
        Manifest.permission.CALL_PHONE,
        Manifest.permission.READ_PHONE_STATE,
        Manifest.permission.READ_CONTACTS,
        Manifest.permission.WRITE_CONTACTS,
        Manifest.permission.RECEIVE_SMS,
        Manifest.permission.RECEIVE_MMS,
        Manifest.permission.SEND_SMS,
        Manifest.permission.READ_SMS,
        Manifest.permission.WRITE_SMS,
        Manifest.permission.RECEIVE_WAP_PUSH,
    };

    public static final String[] ALL_CTA_PERMISSIONS_NO_SEND_MMS = {
   //     com.mediatek.Manifest.permission.CTA_ENABLE_BT,
     //   Manifest.permission.READ_EXTERNAL_STORAGE,
     //   Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.CALL_PHONE,
        Manifest.permission.READ_PHONE_STATE,
        Manifest.permission.READ_CONTACTS,
        Manifest.permission.WRITE_CONTACTS,
        Manifest.permission.RECEIVE_SMS,
        Manifest.permission.RECEIVE_MMS,
        Manifest.permission.SEND_SMS,
        Manifest.permission.READ_SMS,
        Manifest.permission.WRITE_SMS,
        Manifest.permission.RECEIVE_WAP_PUSH,
    };

    public static final String[] REQUIRED_CTA_PERMISSIONS_NO_SEND_MMS = {
    //    com.mediatek.Manifest.permission.CTA_ENABLE_BT,
        Manifest.permission.CALL_PHONE,
        Manifest.permission.READ_PHONE_STATE,
        Manifest.permission.READ_CONTACTS,
        Manifest.permission.WRITE_CONTACTS,
        Manifest.permission.RECEIVE_SMS,
        Manifest.permission.RECEIVE_MMS,
        Manifest.permission.SEND_SMS,
        Manifest.permission.READ_SMS,
        Manifest.permission.WRITE_SMS,
        Manifest.permission.RECEIVE_WAP_PUSH,
    };

    public static final String PREVIOUS_ACTIVITY_INTENT = "previous_intent";
    public static final String MISSING_PERMISSIONS = "missing_permissions";
    public static int sPermissionsActivityStarted = 0;
    private static final CtaManager sManager = CtaManagerFactory.getInstance().makeCtaManager();

    public static boolean requestAllPermissions(Activity activity) {
        if (sManager.isCtaSupported()) {
            return requestPermissions(activity, ALL_CTA_PERMISSIONS);
        } else {
            return requestPermissions(activity, ALL_PERMISSIONS);
        }
    }

    public static boolean requestRequiredPermissions(Activity activity) {
        if (sManager.isCtaSupported()) {
            return requestPermissions(activity, REQUIRED_CTA_PERMISSIONS);
        } else {
            return requestPermissions(activity, REQUIRED_PERMISSIONS);
        }
    }

    private static boolean requestPermissions(Activity activity, String[] permissions) {
        ArrayList<String> missingList = getMissingPermissions(activity, permissions);
        return requestPermissions(activity, missingList);
    }

    public static ArrayList<String> getMissingPermissions(
            Activity activity, String[] requiredPermissions) {
        final ArrayList<String> missingList = new ArrayList<String>();

        for (int i = 0; i < requiredPermissions.length; i++) {
            if (!hasPermission(activity, requiredPermissions[i])) {
                missingList.add(requiredPermissions[i]);
            }
        }

        return missingList;
    }

    public static boolean hasNeverGrantedPermissions(
            Activity activity, ArrayList<String> permissionList) {
        boolean isNeverGranted = false;
        for (int i = 0; i < permissionList.size(); i++) {
            if (isNeverGrantedPermission(activity, permissionList.get(i))) {
                isNeverGranted = true;
                Log.d(TAG, " hasNeverGrantedPermissions "
                        + permissionList.get(i) + " is always denied");
                break;
            }
        }

        return isNeverGranted;
    }

    public static boolean isNeverGrantedPermission(Activity activity, String permission) {
        return !activity.shouldShowRequestPermissionRationale(permission);
    }

    private static boolean requestPermissions(Activity activity, ArrayList<String> missingList) {
        if (missingList.size() == 0) {
            Log.d(TAG, " requestPermissions all permissions granted");
            return false;
        }

        final String[] missingArray = new String[missingList.size()];
        missingList.toArray(missingArray);

        Intent intentPermissions = new Intent(activity, PermissionCheckActivity.class);
        intentPermissions.putExtra(PREVIOUS_ACTIVITY_INTENT, activity.getIntent());
        intentPermissions.putExtra(MISSING_PERMISSIONS, missingArray);

        activity.startActivity(intentPermissions);
        activity.finish();

        return true;
    }

    public static boolean checkAllPermissions(Context context) {
        if (sManager.isCtaSupported()) {
            return checkPermissions(context, ALL_CTA_PERMISSIONS);
        } else {
            return checkPermissions(context, ALL_PERMISSIONS);
        }
    }

    public static boolean checkRequiredPermissions(Context context) {
        if (sManager.isCtaSupported()) {
            return checkPermissions(context, REQUIRED_CTA_PERMISSIONS);
        } else {
            return checkPermissions(context, REQUIRED_PERMISSIONS);
        }
    }

    public static boolean checkPermissions(Context context, String[] permissions) {
        for (int i = 0; i < permissions.length; i++) {
            if (!hasPermission(context, permissions[i])) {
                Log.d(TAG, "checkPermissions false : " + permissions[i]);
                return false;
            }
        }

        return true;
    }

    public static boolean onRequestPermissionsResult(
            Activity activity, int requestCode, String[] permissions,
            int[] grantResults, boolean needFinish) {
        for (int i = 0; i < permissions.length; i++) {
            if (!hasPermission(activity, permissions[i])) {
                // Show toast
                if (isRequiredPermission(permissions[i])
                        || isNeverGrantedPermission(activity, permissions[i])) {
                    showNoPermissionsToast(activity);
                    if (needFinish) {
                        activity.finish();
                    }
                }
                Log.d(TAG, "onRequestPermissionsResult return false");
                return false;
            }
        }
        String[] requiredPermissions;
        if (sManager.isCtaSupported()) {
            requiredPermissions = REQUIRED_CTA_PERMISSIONS;
        } else {
            requiredPermissions = REQUIRED_PERMISSIONS;
        }
        for (int i = 0; i < requiredPermissions.length; i++) {
            if (!hasPermission(activity, requiredPermissions[i])) {
                // Show toast
                if (isRequiredPermission(requiredPermissions[i])
                        || isNeverGrantedPermission(activity, requiredPermissions[i])) {
                    if (!isPermissionChecking()) {
                        showNoPermissionsToast(activity);
                    }
                    if (needFinish) {
                        activity.finish();
                    }
                }
                Log.d(TAG, "onRequestPermissionsResult return false");
                return false;
            }
        }
        Log.d(TAG, "onRequestPermissionsResult return true");
        return true;
    }

    public static boolean hasPermission(Context context, String permission) {
        return context.checkSelfPermission(permission) == PackageManager.PERMISSION_GRANTED;
    }

    public static void showNoPermissionsToast(Context context) {
        Toast.makeText(context, R.string.missing_required_permission,
                Toast.LENGTH_LONG).show();

    }

    public static boolean isPermissionChecking() {
        Log.d(TAG, " isPermissionChecking Activity Count: " + sPermissionsActivityStarted);
        return sPermissionsActivityStarted > 0;
    }

    /*
     * It means permission activity would be finished if startActivity is false.
     */
    public static void setPermissionActivityCount(boolean startActivity) {
        if (startActivity) {
            if (sPermissionsActivityStarted < 0) {
                sPermissionsActivityStarted = 0;
            }
            sPermissionsActivityStarted++;
        } else {
            sPermissionsActivityStarted--;
            if (sPermissionsActivityStarted < 0) {
                sPermissionsActivityStarted = 0;
            }
        }
        Log.d(TAG, "setPermissionActivityCount: "
                + sPermissionsActivityStarted + ", start: " + startActivity);
    }

    public static boolean isRequiredPermission(String permission) {
        String[] requiredPermissions;
        if (sManager.isCtaSupported()) {
            requiredPermissions = REQUIRED_CTA_PERMISSIONS;
        } else {
            requiredPermissions = REQUIRED_PERMISSIONS;
        }
        for (int i = 0; i < requiredPermissions.length; i++) {
            if (requiredPermissions[i].equals(permission)) {
                Log.d(TAG, "isRequiredPermission: " + permission);
                return true;
            }
        }
        return false;
    }
}
