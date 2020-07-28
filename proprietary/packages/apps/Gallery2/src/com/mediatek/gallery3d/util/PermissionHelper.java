package com.mediatek.gallery3d.util;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.widget.Toast;

import com.mediatek.galleryportable.RuntimePermissionUtils;

import java.util.ArrayList;

/** Check and request permission in kinds of cases,
 * now used in GalleryActivity and FiltershowActivity.
 */
public class PermissionHelper {
    private static final String TAG = "MtkGallery2/PermissionHelper";

    /** Check WRITE_EXTERNAL_STORAGE/READ_EXTERNAL_STORAGE
     * permissions for gallery activity.
     * If all permissions are granted, return true.
     * If one of them is denied, request permissions and return false.
     * @param activity GalleryActivity
     * @return If all permissions are granted, return true.
     *         If one of them is denied, request permissions and return false.
     */
    public static boolean checkAndRequestForGallery(Activity activity) {
        // get permissions needed in current scenario
        ArrayList<String> permissionsNeeded = new ArrayList<String>();
        permissionsNeeded.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        permissionsNeeded.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        // check status of permissions, get which permissions need to request
        ArrayList<String> permissionsNeedRequest = new ArrayList<String>();
        for (String permission : permissionsNeeded) {
            if (ContextCompat.checkSelfPermission(activity, permission)
                    == PackageManager.PERMISSION_GRANTED) {
                continue;
            }
            permissionsNeedRequest.add(permission);
        }
        // request permissions
        if (permissionsNeedRequest.size() == 0) {
            Log.d(TAG, "<checkAndRequestForGallery> all permissions are granted");
            return true;
        } else {
            Log.d(TAG, "<checkAndRequestForGallery> not all permissions are granted, reuqest");
            String[] permissions = new String[permissionsNeedRequest.size()];
            permissions = permissionsNeedRequest.toArray(permissions);
            ActivityCompat.requestPermissions(activity, permissions, 0);
            return false;
        }
    }

    /** Check ACCESS_FINE_LOCATION permission for location cluster.
     * @param activity GalleryActivity
     * @return If permission is granted, return true, or else request permission and return false.
     */
    public static boolean checkAndRequestForLocationCluster(Activity activity) {
        if (ContextCompat.checkSelfPermission(activity, Manifest.permission.ACCESS_FINE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
            Log.d(TAG, "<checkAndRequestForLocationCluster> permission not granted, reuqest");
            ActivityCompat.requestPermissions(activity,
                    new String[] { Manifest.permission.ACCESS_FINE_LOCATION }, 0);
            return false;
        }
        Log.d(TAG, "<checkAndRequestForLocationCluster> all permissions are granted");
        return true;
    }

    /**Check WRITE_EXTERNAL_STORAGE/READ_EXTERNAL_STORAGE permission for filter show.
     * @param activity WidgetConfigure or WidgetClickHandler
     * @return If permission is granted, return true, or else request permission and return false.
     */
    public static boolean checkAndRequestForWidget(Activity activity) {
        if (!checkStoragePermission(activity)) {
            Log.d(TAG, "<checkAndRequestForWidget> permission not granted, reuqest");
            ActivityCompat.requestPermissions(activity, new String[] {
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
            }, 0);
            return false;
        }
        Log.d(TAG, "<checkAndRequestForWidget> all permissions are granted");
        return true;
    }

    /**Check WRITE_EXTERNAL_STORAGE/READ_EXTERNAL_STORAGE permission for filter show.
     * @param activity FilterShowActivity
     * @return If permission is granted, return true, or else finish activity and return false.
     */
    public static boolean checkForFilterShow(Activity activity) {
        if (!checkStoragePermission(activity)) {
            Log.d(TAG, "<checkForFilterShow> permission not granted, finish");
            PermissionHelper.showDeniedPrompt(activity);
            activity.finish();
            return false;
        }
        Log.d(TAG, "<checkForFilterShow> all permissions are granted");
        return true;
    }

    /** Check if all permissions in String[] are granted.
     * @param permissions A group of permissions
     * @param grantResults The granted status of permissions
     * @return If all permissions are granted, return true, or else return false.
     */
    public static boolean isAllPermissionsGranted(String[] permissions, int[] grantResults) {
        for (int i = 0; i < grantResults.length; i++) {
            if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    /**
     * Check if WRITE_EXTERNAL_STORAGE/READ_EXTERNAL_STORAGE is granted.
     * @param context Current application environment
     * @return WRITE_EXTERNAL_STORAGE/READ_EXTERNAL_STORAGE is granted or not.
     */
    public static boolean checkStoragePermission(Context context) {
        boolean writeGranted = ContextCompat.checkSelfPermission(context,
                Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
        boolean readGranted = ContextCompat.checkSelfPermission(context,
                Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
        Log.d(TAG, "<checkStoragePermission> writeGranted = " + writeGranted
                + ", readGranted = " + readGranted);
        return writeGranted && readGranted;
    }

    /**
     * Check if ACCESS_FINE_LOCATION is granted.
     * @param context Current application environment
     * @return ACCESS_FINE_LOCATION is granted or not.
     */
    public static boolean checkLocationPermission(Context context) {
        return ContextCompat.checkSelfPermission(context,
                Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED;
    }

    /**
     * Show toast after permission is denied.
     * @param context
     *            Current application environment
     */
    public static void showDeniedPrompt(Context context) {
        Toast.makeText(context, RuntimePermissionUtils.getDeniedPermissionString(context),
                Toast.LENGTH_SHORT).show();
    }

    /**
     * Show toast if permission is denied and "never ask again".
     * @param activity Current activity
     * @param permission The permission your app want to request
     * @return Whether toast has been shown.
     */
    public static boolean showDeniedPromptIfNeeded(Activity activity, String permission) {
        if (!ActivityCompat.shouldShowRequestPermissionRationale(activity, permission)) {
            showDeniedPrompt(activity.getApplicationContext());
            return true;
        }
        return false;
    }
}
