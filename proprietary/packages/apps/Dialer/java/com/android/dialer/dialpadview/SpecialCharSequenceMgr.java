/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.android.dialer.dialpadview;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.DialogFragment;
import android.app.KeyguardManager;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Color;
import android.net.Uri;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.support.annotation.Nullable;
import android.support.annotation.VisibleForTesting;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import com.android.common.io.MoreCloseables;
import com.android.contacts.common.database.NoNullCursorAsyncQueryHandler;
import com.android.contacts.common.util.ContactDisplayUtils;
import com.android.contacts.common.widget.SelectPhoneAccountDialogFragment;
import com.android.contacts.common.widget.SelectPhoneAccountDialogFragment.SelectPhoneAccountListener;
import com.android.contacts.common.widget.SelectPhoneAccountDialogOptionsUtil;
import com.android.dialer.common.Assert;
import com.android.dialer.common.LogUtil;
import com.android.dialer.compat.telephony.TelephonyManagerCompat;
import com.android.dialer.oem.MotorolaUtils;
import com.android.dialer.oem.TranssionUtils;
import com.android.dialer.telecom.TelecomUtil;
import com.android.dialer.util.PermissionsUtil;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.dialer.compat.PowerManagerCompat;
import com.mediatek.dialer.ext.ExtensionManager;
import com.mediatek.dialer.util.DialerFeatureOptions;
import com.google.zxing.BarcodeFormat;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

/**
 * Helper class to listen for some magic character sequences that are handled specially by Dialer.
 */
public class SpecialCharSequenceMgr {
  private static final String TAG_SELECT_ACCT_FRAGMENT = "tag_select_acct_fragment";

  @VisibleForTesting static final String MMI_IMEI_DISPLAY = "*#06#";
  private static final String MMI_REGULATORY_INFO_DISPLAY = "*#07#";

  /// M: add for handle reboot meta secret code @{
  private static final String FK_SUPPORTED = "1";
  private static final String FK_REBOOT_META_SUPPORT = "ro.mtk_rebootmeta_support";
  private static final String MMI_USB_REBOOT_META_SECRET_CODE = "*#*#3641122#*#*";
  private static final String MMI_WIFI_REBOOT_META_SECRET_CODE = "*#*#3642233#*#*";
  private static final String ATM_FLAG_PROP = "ro.boot.atm";
  private static final String ATM_ENABLED = "enable";
  /// @}

  /** ***** This code is used to handle SIM Contact queries ***** */
  private static final String ADN_PHONE_NUMBER_COLUMN_NAME = "number";

  private static final String ADN_NAME_COLUMN_NAME = "name";
  private static final int ADN_QUERY_TOKEN = -1;

  /// M: [ALPS01764940]Add index to indicate the queried contacts @{
  private static final String ADN_ID_COLUMN_NAME = "index";
  /// @}
  /// M: Add for query SIM Contact additional Number, only used when SIM Contact phone type
  /// number is not set.
  private static final String ADN_ADDITIONAL_PHONE_NUMBER_COLUMN_NAME = "additionalNumber";
 
  ///M: add for log TAG
  private static final String TAG = "SpecialCharSequenceMgr";

  /**
   * Remembers the previous {@link QueryHandler} and cancel the operation when needed, to prevent
   * possible crash.
   *
   * <p>QueryHandler may call {@link ProgressDialog#dismiss()} when the screen is already gone,
   * which will cause the app crash. This variable enables the class to prevent the crash on {@link
   * #cleanup()}.
   *
   * <p>TODO: Remove this and replace it (and {@link #cleanup()}) with better implementation. One
   * complication is that we have SpecialCharSequenceMgr in Phone package too, which has *slightly*
   * different implementation. Note that Phone package doesn't have this problem, so the class on
   * Phone side doesn't have this functionality. Fundamental fix would be to have one shared
   * implementation and resolve this corner case more gracefully.
   */
  private static QueryHandler previousAdnQueryHandler;

  /** This class is never instantiated. */
  private SpecialCharSequenceMgr() {}

  public static boolean handleChars(Context context, String input, EditText textField) {
    // get rid of the separators so that the string gets parsed correctly
    String dialString = PhoneNumberUtils.stripSeparators(input);

    if (handleDeviceIdDisplay(context, dialString)
        || handleRegulatoryInfoDisplay(context, dialString)
        || handlePinEntry(context, dialString)
        || handleAdnEntry(context, dialString, textField)
        || handleSecretCode(context, dialString)
        /// M: for op09 *#0000# plug-in @{
        || ExtensionManager.getDialPadExtension().handleChars(context,
                dialString)) {
        /// @}
      return true;
    }

    if (MotorolaUtils.handleSpecialCharSequence(context, input)) {
      return true;
    }

    return false;
  }

  /**
   * Cleanup everything around this class. Must be run inside the main thread.
   *
   * <p>This should be called when the screen becomes background.
   */
  public static void cleanup() {
    Assert.isMainThread();

    if (previousAdnQueryHandler != null) {
      previousAdnQueryHandler.cancel();
      previousAdnQueryHandler = null;
    }
    ///M: Fix ALPS04085590. dismiss account select dialog. @{
    if (sSimContactQueryCookie != null && sSimContactQueryCookie.accountDialog != null) {
        sSimContactQueryCookie.accountDialog.dismissAllowingStateLoss();
        sSimContactQueryCookie.accountDialog = null;
    }
    /// @}
  }

  /**
   * Handles secret codes to launch arbitrary activities in the form of
   * *#*#<code>#*#* or *#<code_starting_with_number>#.
   *
   * @param context the context to use
   * @param input the text to check for a secret code in
   * @return true if a secret code was encountered and handled
   */
  static boolean handleSecretCode(Context context, String input) {
    /// M:start meta_info_activity in ATM mode @{
    String atmFlag = readATMFlag();
    if(ATM_ENABLED.equals(atmFlag) && MMI_WIFI_REBOOT_META_SECRET_CODE.equals(input)) {
      LogUtil.i(TAG,"start meta_info_activity");
      Intent intent = new Intent();
      intent.setAction("com.mediatek.atmwifimeta.Atm_Wifi_Activity");
      context.startActivity(intent);
      return true;
    }
    /// @}

    // Secret code specific to OEMs should be handled first.
    if (TranssionUtils.isTranssionSecretCode(input)) {
      TranssionUtils.handleTranssionSecretCode(context, input);
      return true;
    }

    // Secret codes are accessed by dialing *#*#<code>#*#* or "*#<code_starting_with_number>#"
    if (input.length() > 8 && input.startsWith("*#*#") && input.endsWith("#*#*")) {
      String secretCode = input.substring(4, input.length() - 4);
      TelephonyManagerCompat.handleSecretCode(context, secretCode);
      return true;
    }

    return false;
  }

  /**
   * Handle ADN requests by filling in the SIM contact number into the requested EditText.
   *
   * <p>This code works alongside the Asynchronous query handler {@link QueryHandler} and query
   * cancel handler implemented in {@link SimContactQueryCookie}.
   */
  static boolean handleAdnEntry(Context context, String input, EditText textField) {
    /* ADN entries are of the form "N(N)(N)#" */
    TelephonyManager telephonyManager =
        (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
    if (telephonyManager == null
        || (telephonyManager.getPhoneType() != TelephonyManager.PHONE_TYPE_GSM
                // M: support CDMA ADN requests
                && telephonyManager.getPhoneType() != TelephonyManager.PHONE_TYPE_CDMA)) {

      return false;
    }

    // if the phone is keyguard-restricted, then just ignore this
    // input.  We want to make sure that sim card contacts are NOT
    // exposed unless the phone is unlocked, and this code can be
    // accessed from the emergency dialer.
    KeyguardManager keyguardManager =
        (KeyguardManager) context.getSystemService(Context.KEYGUARD_SERVICE);
    if (keyguardManager.inKeyguardRestrictedInputMode()) {
      return false;
    }

    int len = input.length();
    if ((len > 1) && (len < 5) && (input.endsWith("#"))) {
      try {
        // get the ordinal number of the sim contact
        final int index = Integer.parseInt(input.substring(0, len - 1));

        /// M: Fix ALPS02287171. The index of ADN starts from 1. @{
        if (index <= 0) {
          return false;
        }
        /// @}

        // The original code that navigated to a SIM Contacts list view did not
        // highlight the requested contact correctly, a requirement for PTCRB
        // certification.  This behaviour is consistent with the UI paradigm
        // for touch-enabled lists, so it does not make sense to try to work
        // around it.  Instead we fill in the the requested phone number into
        // the dialer text field.

        // create the async query handler
        final QueryHandler handler = new QueryHandler(context.getContentResolver(),
            (Activity)context);

        // create the cookie object
        /// M: Query will return cursor with exact index no here.
        final SimContactQueryCookie sc =
            new SimContactQueryCookie(index/** - 1*/, handler, ADN_QUERY_TOKEN);

        /// M: Fix CR ALPS01863413. Record the ADN query cookie.
        sSimContactQueryCookie = sc;

        // setup the cookie fields
        /// M: No need to set.
        //sc.contactNum = index - 1;
        sc.setTextField(textField);

        // create the progress dialog
        sc.progressDialog = new ProgressDialog(context);
        sc.progressDialog.setTitle(R.string.simContacts_title);
        sc.progressDialog.setMessage(context.getText(R.string.simContacts_emptyLoading));
        sc.progressDialog.setIndeterminate(true);
        sc.progressDialog.setCancelable(true);
        sc.progressDialog.setOnCancelListener(sc);
        sc.progressDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);

        List<PhoneAccountHandle> subscriptionAccountHandles =
            TelecomUtil.getSubscriptionPhoneAccounts(context);
        Context applicationContext = context.getApplicationContext();
        boolean hasUserSelectedDefault =
            subscriptionAccountHandles.contains(
                TelecomUtil.getDefaultOutgoingPhoneAccount(
                    applicationContext, PhoneAccount.SCHEME_TEL));

        if (subscriptionAccountHandles.size() <= 1 || hasUserSelectedDefault) {
          /// M: to support CDMA ADN query, uri should change to PBR if CDMA sim @{
          /**
           * orignal code:
           * Uri uri = TelecomUtil.getAdnUriForPhoneAccount(applicationContext, null);
           */
          final TelecomManager telecomManager = (TelecomManager) context
              .getSystemService(Context.TELECOM_SERVICE);
          PhoneAccountHandle accountHandle = hasUserSelectedDefault ? telecomManager
              .getDefaultOutgoingPhoneAccount(PhoneAccount.SCHEME_TEL)
              : (subscriptionAccountHandles.size() > 0 ? subscriptionAccountHandles.get(0) : null);
          int subId = TelephonyManager.getDefault().getSubIdForPhoneAccount(
              telecomManager.getPhoneAccount(accountHandle));
          if (!SubInfoUtils.checkSubscriber(subId)) {
            return false;
          }
          Uri uri = SubInfoUtils.getIccProviderUri(subId);
          /// @}
          handleAdnQuery(handler, sc, uri);
        } else {
          SelectPhoneAccountListener callback =
              new HandleAdnEntryAccountSelectedCallback(applicationContext, handler, sc);

          DialogFragment dialogFragment =
              SelectPhoneAccountDialogFragment.newInstance(
                  SelectPhoneAccountDialogOptionsUtil.builderWithAccounts(
                          subscriptionAccountHandles)
                      .build(),
                  callback);
          dialogFragment.show(((Activity) context).getFragmentManager(), TAG_SELECT_ACCT_FRAGMENT);
          ///M: Fix ALPS04085590. remeber the dialogFragment.
          sc.accountDialog = dialogFragment;
        }

        return true;
      } catch (NumberFormatException ex) {
        // Ignore
      }
    }
    return false;
  }

  private static void handleAdnQuery(QueryHandler handler, SimContactQueryCookie cookie, Uri uri) {
    if (handler == null || cookie == null || uri == null) {
      LogUtil.w("SpecialCharSequenceMgr.handleAdnQuery", "queryAdn parameters incorrect");
      return;
    }

    // display the progress dialog
    cookie.progressDialog.show();

    // run the query.
    /// M: add projection ADN_ADDITIONAL_PHONE_NUMBER_COLUMN_NAME @ {
    handler.startQuery(
        ADN_QUERY_TOKEN,
        cookie,
        uri,
        new String[] {ADN_PHONE_NUMBER_COLUMN_NAME, ADN_ADDITIONAL_PHONE_NUMBER_COLUMN_NAME},
        null,
        null,
        null);
    /// @}

    if (previousAdnQueryHandler != null) {
      // It is harmless to call cancel() even after the handler's gone.
      previousAdnQueryHandler.cancel();
    }
    previousAdnQueryHandler = handler;
  }

  static boolean handlePinEntry(final Context context, final String input) {
    if ((input.startsWith("**04") || input.startsWith("**05")) && input.endsWith("#")) {
      List<PhoneAccountHandle> subscriptionAccountHandles =
          TelecomUtil.getSubscriptionPhoneAccounts(context);
      boolean hasUserSelectedDefault =
          subscriptionAccountHandles.contains(
              TelecomUtil.getDefaultOutgoingPhoneAccount(context, PhoneAccount.SCHEME_TEL));

      if (subscriptionAccountHandles.size() <= 1 || hasUserSelectedDefault) {
        // Don't bring up the dialog for single-SIM or if the default outgoing account is
        // a subscription account.
        return TelecomUtil.handleMmi(context, input, null);
      } else {
        SelectPhoneAccountListener listener = new HandleMmiAccountSelectedCallback(context, input);

        DialogFragment dialogFragment =
            SelectPhoneAccountDialogFragment.newInstance(
                SelectPhoneAccountDialogOptionsUtil.builderWithAccounts(subscriptionAccountHandles)
                    .build(),
                listener);
        dialogFragment.show(((Activity) context).getFragmentManager(), TAG_SELECT_ACCT_FRAGMENT);
      }
      return true;
    }
    return false;
  }

  // TODO: Use TelephonyCapabilities.getDeviceIdLabel() to get the device id label instead of a
  // hard-coded string.
  @SuppressLint("HardwareIds")
  static boolean handleDeviceIdDisplay(Context context, String input) {
    if (!PermissionsUtil.hasPermission(context, Manifest.permission.READ_PHONE_STATE)) {
      return false;
    }
    TelephonyManager telephonyManager =
        (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);

    if (telephonyManager != null && input.equals(MMI_IMEI_DISPLAY)) {
      int labelResId = R.string.imei;
      try {
        if (TelephonyManagerCompat.getPhoneCount(telephonyManager) > 1) {
          LogUtil.i(TAG,"getPhoneCount1 = "
                  + TelephonyManagerCompat.getPhoneCount(telephonyManager));
          for (int slot = 0; slot < telephonyManager.getPhoneCount(); slot++) {
            int phoneType = telephonyManager.getPhoneType(slot);
            LogUtil.i(TAG,"phoneType = " + phoneType);
            if (phoneType == TelephonyManager.PHONE_TYPE_CDMA &&
                telephonyManager.getLteOnCdmaMode() == PhoneConstants.LTE_ON_CDMA_TRUE) {
                LogUtil.i("SpecialCharSequenceMgr.handleRegulatoryInfoDisplay",
                                 "PHONE_TYPE_CDMA & LTE_ON_CDMA_TRUE");
                labelResId = R.string.imei_meid;
            }
          }
        }
      } catch (SecurityException e) {
        /// M: Catch the security exception to avoid dialer crash, such as user denied
        /// READ_PHONE_STATE permission in settings at N version. And display empty list.
        Toast.makeText(context, R.string.missing_required_permission, Toast.LENGTH_SHORT).show();
      }

      View customView = LayoutInflater.from(context).inflate(R.layout.dialog_deviceids, null);
      ViewGroup holder = customView.findViewById(R.id.deviceids_holder);

      if (TelephonyManagerCompat.getPhoneCount(telephonyManager) > 1) {
        if (!ExtensionManager.getDialPadExtension().handleDeviceIdDisplay(
            holder,
            context.getResources().getBoolean(R.bool.show_device_id_in_hex_and_decimal),
            false)) {
          for (int slot = 0; slot < telephonyManager.getPhoneCount(); slot++) {
            String deviceId = telephonyManager.getDeviceId(slot);
            if (!TextUtils.isEmpty(deviceId)) {
              addDeviceIdRow(
                  holder,
                  deviceId,
                  /* showDecimal */
                  context.getResources().getBoolean(R.bool.show_device_id_in_hex_and_decimal),
                  /* showBarcode */ false);
            }
          }
        }
      } else {
        addDeviceIdRow(
            holder,
            telephonyManager.getDeviceId(),
            /* showDecimal */
            context.getResources().getBoolean(R.bool.show_device_id_in_hex_and_decimal),
            /* showBarcode */
            context.getResources().getBoolean(R.bool.show_device_id_as_barcode));
      }

      new AlertDialog.Builder(context)
          .setTitle(labelResId)
          .setView(customView)
          .setPositiveButton(android.R.string.ok, null)
          .setCancelable(false)
          .show()
          .getWindow()
          .setLayout(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
      return true;
    }
    return false;
  }

  /// M: for plugin. @{
  /// Google code:
  /// private static void addDeviceIdRow(
  ///     ViewGroup holder, String deviceId, boolean showDecimal, boolean showBarcode) {
  public static void addDeviceIdRow(
      ViewGroup holder, String deviceId, boolean showDecimal, boolean showBarcode) {
  /// @}
    if (TextUtils.isEmpty(deviceId)) {
      return;
    }

    ViewGroup row =
        (ViewGroup)
            LayoutInflater.from(holder.getContext()).inflate(R.layout.row_deviceid, holder, false);
    holder.addView(row);

    // Remove the check digit, if exists. This digit is a checksum of the ID.
    // See https://en.wikipedia.org/wiki/International_Mobile_Equipment_Identity
    // and https://en.wikipedia.org/wiki/Mobile_equipment_identifier
    String hex = deviceId.length() == 15 ? deviceId.substring(0, 14) : deviceId;

    // If this is the valid length IMEI or MEID (14 digits), show it in all formats, otherwise fall
    // back to just showing the raw hex
    if (hex.length() == 14 && showDecimal) {
      ((TextView) row.findViewById(R.id.deviceid_hex)).setText(hex);
      ((TextView) row.findViewById(R.id.deviceid_dec)).setText(getDecimalFromHex(hex));
      row.findViewById(R.id.deviceid_dec_label).setVisibility(View.VISIBLE);
    } else {
      row.findViewById(R.id.deviceid_hex_label).setVisibility(View.GONE);
      ((TextView) row.findViewById(R.id.deviceid_hex)).setText(deviceId);
    }

    final ImageView barcode = row.findViewById(R.id.deviceid_barcode);
    if (showBarcode) {
      // Wait until the layout pass has completed so we the barcode is measured before drawing. We
      // do this by adding a layout listener and setting the bitmap after getting the callback.
      barcode
          .getViewTreeObserver()
          .addOnGlobalLayoutListener(
              new OnGlobalLayoutListener() {
                @Override
                public void onGlobalLayout() {
                  barcode.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                  Bitmap barcodeBitmap =
                      generateBarcode(hex, barcode.getWidth(), barcode.getHeight());
                  if (barcodeBitmap != null) {
                    barcode.setImageBitmap(barcodeBitmap);
                  }
                }
              });
    } else {
      barcode.setVisibility(View.GONE);
    }
  }

  private static String getDecimalFromHex(String hex) {
    final String part1 = hex.substring(0, 8);
    final String part2 = hex.substring(8);

    long dec1;
    try {
      dec1 = Long.parseLong(part1, 16);
    } catch (NumberFormatException e) {
      LogUtil.e("SpecialCharSequenceMgr.getDecimalFromHex", "unable to parse hex", e);
      return "";
    }

    final String manufacturerCode = String.format(Locale.US, "%010d", dec1);

    long dec2;
    try {
      dec2 = Long.parseLong(part2, 16);
    } catch (NumberFormatException e) {
      LogUtil.e("SpecialCharSequenceMgr.getDecimalFromHex", "unable to parse hex", e);
      return "";
    }

    final String serialNum = String.format(Locale.US, "%08d", dec2);

    StringBuilder builder = new StringBuilder(22);
    builder
        .append(manufacturerCode, 0, 5)
        .append(' ')
        .append(manufacturerCode, 5, manufacturerCode.length())
        .append(' ')
        .append(serialNum, 0, 4)
        .append(' ')
        .append(serialNum, 4, serialNum.length());
    return builder.toString();
  }

  /**
   * This method generates a 2d barcode using the zxing library. Each pixel of the bitmap is either
   * black or white painted vertically. We determine which color using the BitMatrix.get(x, y)
   * method.
   */
  private static Bitmap generateBarcode(String hex, int width, int height) {
    MultiFormatWriter writer = new MultiFormatWriter();
    String data = Uri.encode(hex);

    try {
      BitMatrix bitMatrix = writer.encode(data, BarcodeFormat.CODE_128, width, 1);
      Bitmap bitmap = Bitmap.createBitmap(bitMatrix.getWidth(), height, Config.RGB_565);

      for (int i = 0; i < bitMatrix.getWidth(); i++) {
        // Paint columns of width 1
        int[] column = new int[height];
        Arrays.fill(column, bitMatrix.get(i, 0) ? Color.BLACK : Color.WHITE);
        bitmap.setPixels(column, 0, 1, i, 0, 1, height);
      }
      return bitmap;
    } catch (WriterException e) {
      LogUtil.e("SpecialCharSequenceMgr.generateBarcode", "error generating barcode", e);
    }
    return null;
  }

  private static boolean handleRegulatoryInfoDisplay(Context context, String input) {
    if (input.equals(MMI_REGULATORY_INFO_DISPLAY)) {
      LogUtil.i(
          "SpecialCharSequenceMgr.handleRegulatoryInfoDisplay", "sending intent to settings app");
      Intent showRegInfoIntent = new Intent(Settings.ACTION_SHOW_REGULATORY_INFO);
      try {
        context.startActivity(showRegInfoIntent);
      } catch (ActivityNotFoundException e) {
        LogUtil.e(
            "SpecialCharSequenceMgr.handleRegulatoryInfoDisplay", "startActivity() failed: ", e);
      }
      return true;
    }
    return false;
  }

  public static class HandleAdnEntryAccountSelectedCallback extends SelectPhoneAccountListener {

    private final Context context;
    private final QueryHandler queryHandler;
    private final SimContactQueryCookie cookie;

    public HandleAdnEntryAccountSelectedCallback(
        Context context, QueryHandler queryHandler, SimContactQueryCookie cookie) {
      this.context = context;
      this.queryHandler = queryHandler;
      this.cookie = cookie;
    }

    @Override
    public void onPhoneAccountSelected(
        PhoneAccountHandle selectedAccountHandle, boolean setDefault, @Nullable String callId) {
      /**
       * original code:
       * Uri uri = TelecomUtil.getAdnUriForPhoneAccount(context, selectedAccountHandle);
       */
      /// M: to support CDMA ADN query, uri should change to PBR if CDMA sim @{
      final TelecomManager telecomManager =
              (TelecomManager) context.getSystemService(Context.TELECOM_SERVICE);
      int subId = TelephonyManager.getDefault().getSubIdForPhoneAccount(
              telecomManager.getPhoneAccount(selectedAccountHandle));
      Uri uri = SubInfoUtils.getIccProviderUri(subId);
      /// @}
      handleAdnQuery(queryHandler, cookie, uri);
      // TODO: Show error dialog if result isn't valid.
      /// M: clear accountDialog after Account selected to fix ALPS04085590 @{
      if (sSimContactQueryCookie != null && sSimContactQueryCookie.accountDialog != null) {
          sSimContactQueryCookie.accountDialog = null;
        }
      /// @}
    }

    /// M: clear accountDialog after Account select dialog dissed to fix ALPS04085590 @{
    @Override
    public void onDialogDismissed(@Nullable String callId) {
      if (sSimContactQueryCookie != null && sSimContactQueryCookie.accountDialog != null) {
        sSimContactQueryCookie.accountDialog = null;
      }
      LogUtil.v(TAG, "onDialogDismissed");
    }
    /// @}
  }

  public static class HandleMmiAccountSelectedCallback extends SelectPhoneAccountListener {

    private final Context context;
    private final String input;

    public HandleMmiAccountSelectedCallback(Context context, String input) {
      this.context = context.getApplicationContext();
      this.input = input;
    }

    @Override
    public void onPhoneAccountSelected(
        PhoneAccountHandle selectedAccountHandle, boolean setDefault, @Nullable String callId) {
      TelecomUtil.handleMmi(context, input, selectedAccountHandle);
    }
  }

  /**
   * Cookie object that contains everything we need to communicate to the handler's onQuery
   * Complete, as well as what we need in order to cancel the query (if requested).
   *
   * <p>Note, access to the textField field is going to be synchronized, because the user can
   * request a cancel at any time through the UI.
   */
  private static class SimContactQueryCookie implements DialogInterface.OnCancelListener {

    public ProgressDialog progressDialog;
    ///M: Fix ALPS04085590. Remember Acccout Selected Dialog.
    DialogFragment accountDialog;
    public int contactNum;

    // Used to identify the query request.
    private int token;
    private QueryHandler handler;

    // The text field we're going to update
    private EditText textField;

    public SimContactQueryCookie(int number, QueryHandler handler, int token) {
      contactNum = number;
      this.handler = handler;
      this.token = token;
    }

    /** Synchronized getter for the EditText. */
    public synchronized EditText getTextField() {
      return textField;
    }

    /** Synchronized setter for the EditText. */
    public synchronized void setTextField(EditText text) {
      textField = text;
    }

    /**
     * Cancel the ADN query by stopping the operation and signaling the cookie that a cancel request
     * is made.
     */
    @Override
    public synchronized void onCancel(DialogInterface dialog) {
      /** M: Fix CR ALPS01863413. Call QueryHandler.cancel(). @{ */
      /** original code:
      // close the progress dialog
      if (progressDialog != null) {
        progressDialog.dismiss();
      }

      // setting the textfield to null ensures that the UI does NOT get
      // updated.
      textField = null;

      // Cancel the operation if possible.
      handler.cancelOperation(token);
      */
      handler.cancel();
      /** @} */
    }
  }

  /**
   * Asynchronous query handler that services requests to look up ADNs
   *
   * <p>Queries originate from {@link #handleAdnEntry}.
   */
  private static class QueryHandler extends NoNullCursorAsyncQueryHandler {

    private boolean canceled;
    /// M: own activity
    private Activity mActivity;

    public QueryHandler(ContentResolver cr, Activity activity) {
      super(cr);
      mActivity = activity;
    }

    /** Override basic onQueryComplete to fill in the textfield when we're handed the ADN cursor. */
    @Override
    protected void onNotNullableQueryComplete(int token, Object cookie, Cursor c) {
      try {
        previousAdnQueryHandler = null;
        /// M: Fix CR ALPS01863413. Clear the ADN query cookie.
        sSimContactQueryCookie = null;
        /// M: add activity check to avoid JE error
        if (canceled  || mActivity == null
            || mActivity.isFinishing() || mActivity.isDestroyed()) {
          return;
        }

        SimContactQueryCookie sc = (SimContactQueryCookie) cookie;

        // close the progress dialog.
        sc.progressDialog.dismiss();

        // get the EditText to update or see if the request was cancelled.
        EditText text = sc.getTextField();

        // if the TextView is valid, and the cursor is valid and positionable on the
        // Nth number, then we update the text field and display a toast indicating the
        // caller name.
        /// M: [ALPS01764940]Add index to indicate the queried contacts @{
        String name = null;
        String number = null;
        String additionalNumber = null;
        if ((c != null) && (text != null)) {
          int adnIdIndex = c.getColumnIndex(ADN_ID_COLUMN_NAME);
          if (adnIdIndex == -1) {
            ///M:[portable] means can not find ADN_ID_COLUMN_NAME, use original logic.
            LogUtil.v(TAG, "onNotNullableQueryComplete, use original logic to get adn.");
            if (c.moveToPosition(sc.contactNum)) {
              name = c.getString(c.getColumnIndexOrThrow(ADN_NAME_COLUMN_NAME));
              number = c.getString(c.getColumnIndexOrThrow(ADN_PHONE_NUMBER_COLUMN_NAME));
            }
          } else {
            while (c.moveToNext()) {
              if (c.getInt(adnIdIndex) == sc.contactNum) {
                name = c.getString(c.getColumnIndexOrThrow(ADN_NAME_COLUMN_NAME));
                number = c.getString(c.getColumnIndexOrThrow(ADN_PHONE_NUMBER_COLUMN_NAME));
                additionalNumber = c.getString(c
                    .getColumnIndexOrThrow(ADN_ADDITIONAL_PHONE_NUMBER_COLUMN_NAME));
                break;
              }
            }
          }

          // fill the text in.
          if (!TextUtils.isEmpty(number)) {
            text.getText().replace(0, 0, number);
          } else if (!TextUtils.isEmpty(additionalNumber)) {
            text.getText().replace(0, 0, additionalNumber);
          }

          // display the name as a toast
          /// M: empty name will cause ANR when calling getTtsSpannedPhoneNumber()
          LogUtil.d(
              TAG,
              "onNotNullableQueryComplete, name : " + name + " number : "
                  + LogUtil.sanitizePii(number));
          if (!TextUtils.isEmpty(name)) {
            Context context = sc.progressDialog.getContext();
            CharSequence msg = ContactDisplayUtils.getTtsSpannedPhoneNumber(context.getResources(),
                R.string.menu_callNumber, name);
            Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
          }
        }
        /// @}
      } finally {
        MoreCloseables.closeQuietly(c);
      }
    }

    public void cancel() {
      canceled = true;
      // Ask AsyncQueryHandler to cancel the whole request. This will fail when the query is
      // already started.
      cancelOperation(ADN_QUERY_TOKEN);
        /// M: Fix CR ALPS01863413. Dismiss the progress and clear the ADN query cookie. @{
      if (sSimContactQueryCookie != null && sSimContactQueryCookie.progressDialog != null) {
        sSimContactQueryCookie.progressDialog.dismiss();
        sSimContactQueryCookie = null;
      }
        /// @}
    }
  }

  /** M: Fix CR ALPS01863413. Make the progress dismiss after the ADN query be cancelled.
   *  And make it support screen rotation while phone account pick dialog shown. @{ */
  private static SimContactQueryCookie sSimContactQueryCookie;

  /**
   * For ADN query with multiple phone accounts. If the the phone account pick
   * dialog shown, then rotate the screen and select one account to query ADN.
   * The ADN result would write into the old text view because the views
   * re-created but the class did not known. So, the dialpad fragment should
   * call this method to update the digits text filed view after it be
   * re-created.
   *
   * @param textFiled
   *            the digits text filed view
   */
  public static void updateTextFieldView(EditText textFiled) {
    if (sSimContactQueryCookie != null) {
      sSimContactQueryCookie.setTextField(textFiled);
    }
  }
  /** @} */

  /// M: for OP01 OM 6M project @{
  /**
   * handle IMEI display about MEID and IMEI.
   * @param List <String> list, the IMEI string list.
   * @return List<String>, the IMEI string list handled.
   */
  private static List<String> handleOpIMEIs(List<String> list) {
    int phoneCount = TelephonyManager.getDefault().getPhoneCount();
    String meid = "";
    list.clear();
    for (int i = 0; i < phoneCount; i++) {
      String imei = TelephonyManager.getDefault().getImei(i);
      LogUtil.d(TAG, "handleOpIMEIs, imei = " + LogUtil.sanitizePii(imei));
      list.add("IMEI:" + imei);
      if (TextUtils.isEmpty(meid)) {
          meid = TelephonyManager.getDefault().getMeid(i);
      }
    }
    meid = "MEID:" + meid;
    list.add(meid);
    return list;
  }
  /// @}

  /** M: Handle reboot meta secret code, if match,reboot the device and set the meta boot type
   * SystemProperties to usb or wifi mode according to the input.
   * @param context the context to use
   * @param input the secret code
   * @return true if secret code matched
   */
  private static boolean handleRebootMetaSecretCode(Context context, String input) {
    PowerManager powerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
    if (powerManager != null
        && (MMI_USB_REBOOT_META_SECRET_CODE.equals(input) || MMI_WIFI_REBOOT_META_SECRET_CODE
            .equals(input))) {
      if (MMI_USB_REBOOT_META_SECRET_CODE.equals(input)) {
        powerManager.reboot(PowerManagerCompat.REBOOT_META_USB);
      } else {
        powerManager.reboot(PowerManagerCompat.REBOOT_META_WIFI);
      }
      return true;
    }
    return false;
  }
  /// @}

  /**M:Read flag for ATM
   *@return flag
   */
  private static String readATMFlag() {
    String atm_flag = SystemProperties.get(ATM_FLAG_PROP);
    LogUtil.d(TAG,"readATMFlag value: "+atm_flag);
    return atm_flag;
  }
}
