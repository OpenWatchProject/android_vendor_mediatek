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

package com.android.dialer.util;

import static android.Manifest.permission.ACCESS_COARSE_LOCATION;
import static android.Manifest.permission.ACCESS_FINE_LOCATION;
import static android.Manifest.permission.ADD_VOICEMAIL;
import static android.Manifest.permission.CALL_PHONE;
import static android.Manifest.permission.MODIFY_PHONE_STATE;
import static android.Manifest.permission.READ_CALL_LOG;
import static android.Manifest.permission.READ_CONTACTS;
import static android.Manifest.permission.READ_PHONE_STATE;
import static android.Manifest.permission.READ_VOICEMAIL;
import static android.Manifest.permission.SEND_SMS;
import static android.Manifest.permission.WRITE_CALL_LOG;
import static android.Manifest.permission.WRITE_CONTACTS;
import static android.Manifest.permission.WRITE_VOICEMAIL;

import android.Manifest.permission;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
///M: @{
import android.content.pm.PackageInfo;
/// @}
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PermissionInfo;
import android.support.annotation.NonNull;
import android.support.annotation.VisibleForTesting;
import android.support.v4.content.ContextCompat;
import android.support.v4.content.LocalBroadcastManager;
import android.widget.Toast;
import com.android.dialer.common.LogUtil;
import com.android.dialer.storage.StorageComponent;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
//M: @{
import java.util.concurrent.ConcurrentHashMap;
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;
// @}
import java.util.List;

/** Utility class to help with runtime permissions. */
public class PermissionsUtil {

  @VisibleForTesting
  public static final String PREFERENCE_CAMERA_ALLOWED_BY_USER = "camera_allowed_by_user";

  private static final String PERMISSION_PREFERENCE = "dialer_permissions";
  private static final String CEQUINT_PERMISSION = "com.cequint.ecid.CALLER_ID_LOOKUP";


  ///M: Add for multi-party conference call permission
  private static final String CTA_CONFERENCE_CALL = "com.mediatek.permission.CTA_CONFERENCE_CALL";
  private static final String TAG = PermissionsUtil.class.getSimpleName();

  // Permissions list retrieved from application manifest.
  // Starting in Android O Permissions must be explicitly enumerated:
  // https://developer.android.com/preview/behavior-changes.html#rmp

  ///M: if CTA feature open need apply permission  for multi-party conference,
  ///cant add to permission group when CTA feature close, or will happen error@{
  public static List<String> allPhoneGroupPermissionsUsedInDialer =
      new ArrayList<>(Arrays.asList(
          READ_CALL_LOG,
          WRITE_CALL_LOG,
          READ_PHONE_STATE,
          MODIFY_PHONE_STATE,
          SEND_SMS,
          CALL_PHONE,
          ADD_VOICEMAIL,
          WRITE_VOICEMAIL,
          READ_VOICEMAIL,
          /**M:*/permission.PROCESS_OUTGOING_CALLS));
 //comment out CTA that is not ready
  static {
    CtaManager ctaManager = CtaManagerFactory.getInstance().makeCtaManager();
    if (ctaManager.isCtaSupported()) {
      allPhoneGroupPermissionsUsedInDialer.add(CTA_CONFERENCE_CALL);
    }
  }

  /// @}

  public static final List<String> allContactsGroupPermissionsUsedInDialer =
      Collections.unmodifiableList(Arrays.asList(READ_CONTACTS, WRITE_CONTACTS,
        /**M:*/permission.GET_ACCOUNTS));

  public static final List<String> allLocationGroupPermissionsUsedInDialer =
      Collections.unmodifiableList(Arrays.asList(ACCESS_FINE_LOCATION, ACCESS_COARSE_LOCATION));

  public static boolean hasPhonePermissions(Context context) {
    return hasPermission(context,/* M:checking full group */PHONE_FULL_GROUP);
  }

  public static boolean hasReadPhoneStatePermissions(Context context) {
    return hasPermission(context, permission.READ_PHONE_STATE);
  }

  public static boolean hasContactsReadPermissions(Context context) {
    return hasPermission(context,/* M:checking full group */CONTACTS_FULL_GROUP);
  }

  public static boolean hasContactsWritePermissions(Context context) {
    return hasPermission(context, permission.WRITE_CONTACTS);
  }

  public static boolean hasLocationPermissions(Context context) {
    return hasPermission(context,/* M:checking full group */LOCATION_FULL_GROUP);
  }

  public static boolean hasCameraPermissions(Context context) {
    return hasPermission(context, permission.CAMERA);
  }

  public static boolean hasMicrophonePermissions(Context context) {
    return hasPermission(context, permission.RECORD_AUDIO);
  }

  public static boolean hasCallLogReadPermissions(Context context) {
    return hasPermission(context, permission.READ_CALL_LOG);
  }

  public static boolean hasCallLogWritePermissions(Context context) {
    return hasPermission(context, permission.WRITE_CALL_LOG);
  }

  public static boolean hasCequintPermissions(Context context) {
    return hasPermission(context, CEQUINT_PERMISSION);
  }

  public static boolean hasReadVoicemailPermissions(Context context) {
    return hasPermission(context, permission.READ_VOICEMAIL);
  }

  public static boolean hasWriteVoicemailPermissions(Context context) {
    return hasPermission(context, permission.WRITE_VOICEMAIL);
  }

  public static boolean hasAddVoicemailPermissions(Context context) {
    return hasPermission(context, permission.ADD_VOICEMAIL);
  }

  public static boolean hasSendSmsPermissions(Context context) {
    return hasPermission(context, permission.SEND_SMS);
  }

  /// M: To improve performance, cache permission to avoid query many times. @{
  private static ConcurrentHashMap<String, Boolean> mPermissionCache
      = new ConcurrentHashMap<String, Boolean>();
  public static boolean hasPermission(Context context, String permission) {
    if (mPermissionCache.containsKey(permission)) {
      return true;
    } else {
      boolean result = ContextCompat.checkSelfPermission(context, permission)
          == PackageManager.PERMISSION_GRANTED;
      if (result) {
        mPermissionCache.put(permission, true);
      }
    return result;
    }
  } /// @}

  /**
   * Checks {@link android.content.SharedPreferences} if a permission has been requested before.
   *
   * <p>It is important to note that this method only works if you call {@link
   * PermissionsUtil#permissionRequested(Context, String)} in {@link
   * android.app.Activity#onRequestPermissionsResult(int, String[], int[])}.
   */
  public static boolean isFirstRequest(Context context, String permission) {
    return context
        .getSharedPreferences(PERMISSION_PREFERENCE, Context.MODE_PRIVATE)
        .getBoolean(permission, true);
  }

  /**
   * Records in {@link android.content.SharedPreferences} that the specified permission has been
   * requested at least once.
   *
   * <p>This method should be called in {@link android.app.Activity#onRequestPermissionsResult(int,
   * String[], int[])}.
   */
  public static void permissionRequested(Context context, String permission) {
    context
        .getSharedPreferences(PERMISSION_PREFERENCE, Context.MODE_PRIVATE)
        .edit()
        .putBoolean(permission, false)
        .apply();
  }

  /**
   * Rudimentary methods wrapping the use of a LocalBroadcastManager to simplify the process of
   * notifying other classes when a particular fragment is notified that a permission is granted.
   *
   * <p>To be notified when a permission has been granted, create a new broadcast receiver and
   * register it using {@link #registerPermissionReceiver(Context, BroadcastReceiver, String)}
   *
   * <p>E.g.
   *
   * <p>final BroadcastReceiver receiver = new BroadcastReceiver() { @Override public void
   * onReceive(Context context, Intent intent) { refreshContactsView(); } }
   *
   * <p>PermissionsUtil.registerPermissionReceiver(getActivity(), receiver, READ_CONTACTS);
   *
   * <p>If you register to listen for multiple permissions, you can identify which permission was
   * granted by inspecting {@link Intent#getAction()}.
   *
   * <p>In the fragment that requests for the permission, be sure to call {@link
   * #notifyPermissionGranted(Context, String)} when the permission is granted so that any
   * interested listeners are notified of the change.
   */
  public static void registerPermissionReceiver(
      Context context, BroadcastReceiver receiver, String permission) {
    LogUtil.i("PermissionsUtil.registerPermissionReceiver", permission);
    final IntentFilter filter = new IntentFilter(permission);
    LocalBroadcastManager.getInstance(context).registerReceiver(receiver, filter);
  }

  public static void unregisterPermissionReceiver(Context context, BroadcastReceiver receiver) {
    LogUtil.i("PermissionsUtil.unregisterPermissionReceiver", null);
    LocalBroadcastManager.getInstance(context).unregisterReceiver(receiver);
  }

  public static void notifyPermissionGranted(Context context, String permission) {
    LogUtil.i("PermissionsUtil.notifyPermissionGranted", permission);
    final Intent intent = new Intent(permission);
    LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
  }

  /**
   * Returns a list of permissions currently not granted to the application from the supplied list.
   *
   * @param context - The Application context.
   * @param permissionsList - A list of permissions to check if the current application has been
   *     granted.
   * @return An array of permissions that are currently DENIED to the application; a subset of
   *     permissionsList.
   */
  @NonNull
  public static String[] getPermissionsCurrentlyDenied(
      @NonNull Context context, @NonNull List<String> permissionsList) {
    List<String> permissionsCurrentlyDenied = new ArrayList<>();
    for (String permission : permissionsList) {
      if (!hasPermission(context, permission)) {
        permissionsCurrentlyDenied.add(permission);
      }
    }
    return permissionsCurrentlyDenied.toArray(new String[permissionsCurrentlyDenied.size()]);
  }

  /**
   * Since we are granted the camera permission automatically as a first-party app, we need to show
   * a toast to let users know the permission was granted for privacy reasons.
   *
   * @return true if we've already shown the camera privacy toast.
   */
  public static boolean hasCameraPrivacyToastShown(@NonNull Context context) {
    return StorageComponent.get(context)
        .unencryptedSharedPrefs()
        .getBoolean(PREFERENCE_CAMERA_ALLOWED_BY_USER, false);
  }

  public static void showCameraPermissionToast(@NonNull Context context) {
    Toast.makeText(context, context.getString(R.string.camera_privacy_text), Toast.LENGTH_LONG)
        .show();
    setCameraPrivacyToastShown(context);
  }

  public static void setCameraPrivacyToastShown(@NonNull Context context) {
    StorageComponent.get(context)
        .unencryptedSharedPrefs()
        .edit()
        .putBoolean(PREFERENCE_CAMERA_ALLOWED_BY_USER, true)
        .apply();
  }


  /// M: Mediatek start.
  /// M: MTK modified the AOSP permission group logic.
  /// Now, add permissions full list for real permission group checking
  /// instead of AOSP. This is more logically, and meet the requirement of CTA. @{
  /**
   * The Phone group permissions defined by AOSP.
   */
  public static final String[] PHONE_FULL_GROUP = allPhoneGroupPermissionsUsedInDialer
      .toArray(new String[allPhoneGroupPermissionsUsedInDialer.size()]);
  /**
   * The CONTACTS group permissions defined by AOSP.
   */
  public static final String[] CONTACTS_FULL_GROUP = new String[] {
    permission.READ_CONTACTS, permission.WRITE_CONTACTS,
    permission.GET_ACCOUNTS
  };
  /**
   * The LOCATION group permissions defined by AOSP.
   */
  public static final String[] LOCATION_FULL_GROUP = new String[] {
    permission.ACCESS_FINE_LOCATION, permission.ACCESS_COARSE_LOCATION
  };

  /**
   * M: Retrieve all the permissions in the phone group defined by system
   * @param context
   * @param groupName
   * @return permissions array or null
   */
  public static String[] getAllPermissionsByGroup(Context context, String groupName) {
    final List<PermissionInfo> permissions;
    try {
      permissions = context.getPackageManager().queryPermissionsByGroup(groupName,
          PackageManager.GET_META_DATA);
    } catch (NameNotFoundException e) {
      return null;
    }

    if (permissions == null || permissions.size() == 0) {
      return null;
    }

    final ArrayList<String> permissionNames = new ArrayList<String>();
    for (PermissionInfo permission : permissions) {
      permissionNames.add(permission.name);
    }
    return permissionNames.toArray(new String[permissionNames.size()]);
  }

  private static boolean isNeedGetPackagePermissionList = true;
  private static List<String> requestedPermissions;

  public static boolean hasPermission(Context context, String[] permissions) {
    if (isNeedGetPackagePermissionList) {
      LogUtil.i(TAG, "hasPermission GetPackagePermissionList start ...");
      PackageInfo packageInfo = null;
      try {
        packageInfo = context.getPackageManager().getPackageInfo(
            context.getApplicationInfo().packageName, PackageManager.GET_PERMISSIONS);
      } catch (NameNotFoundException e) {
        LogUtil.e(TAG, "NameNotFoundException NOT SUPPORTED : " + e.toString());
      } catch (UnsupportedOperationException e) {
        // some mock context such as testcase can't support this.
        LogUtil.e(TAG, "NOT SUPPORTED : " + e.toString());
      }
      if (packageInfo != null && packageInfo.requestedPermissions != null) {
        requestedPermissions = Arrays.asList(packageInfo.requestedPermissions);
      }
      isNeedGetPackagePermissionList = false;
      LogUtil.i(TAG, "hasPermission GetPackagePermissionList end ...");
    }

    if (requestedPermissions == null) {
      LogUtil.d(TAG, "requestedPermissions == null ");
      for (String permission : permissions) {
        // Only the permissions were requested need to be checking granted or not.
        if (!hasPermission(context, permission)) {
          LogUtil.e(TAG, "NOT GRANTED : " + permission);
          return false;
        }
      }
      return true;
    }

    for (String permission : permissions) {
       // Only the permissions were requested need to be checking granted or not.
       if (requestedPermissions.contains(permission) && !hasPermission(context, permission)) {
         LogUtil.e(TAG, "NOT GRANTED : " + permission);
         return false;
       }
     }
     return true;
    }

  public static void registerPermissionReceiver(Context context, BroadcastReceiver receiver,
      String[] permissions) {
    IntentFilter filter = null;
    for (String permission : permissions) {
      filter = new IntentFilter(permission);
      LocalBroadcastManager.getInstance(context).registerReceiver(receiver, filter);
    }
  }

  public static void notifyPermissionGranted(Context context, String[] permissions) {
    Intent intent = null;
    for (String permission : permissions) {
      intent = new Intent(permission);
      LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }
  }
  /// @}
  /// M: Mediatek end.
}
