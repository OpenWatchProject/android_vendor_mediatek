/*
 * Copyright (C) 2017 The Android Open Source Project
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
 * limitations under the License
 */

package com.mediatek.dialer.search;


import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.support.annotation.IntDef;
import android.support.annotation.Nullable;
import android.support.v7.widget.RecyclerView.ViewHolder;
import android.text.TextUtils;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.QuickContactBadge;
import com.android.dialer.common.Assert;
import com.android.dialer.contactphoto.ContactPhotoManager;
import com.android.dialer.dialercontact.DialerContact;
import com.android.dialer.duo.DuoComponent;
import com.android.dialer.enrichedcall.EnrichedCallCapabilities;
import com.android.dialer.enrichedcall.EnrichedCallComponent;
import com.android.dialer.enrichedcall.EnrichedCallManager;
import com.android.dialer.lettertile.LetterTileDrawable;
import com.android.dialer.searchfragment.common.Projections;
import com.android.dialer.searchfragment.common.QueryBoldingUtil;
import com.android.dialer.searchfragment.common.R;
import com.android.dialer.searchfragment.common.RowClickListener;
import com.android.dialer.searchfragment.common.SearchCursor;
import com.android.dialer.widget.BidiTextView;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;


///M: @ {
import com.android.dialer.app.calllog.calllogcache.CallLogCache;
import com.android.dialer.calllogutils.PhoneNumberDisplayUtil;
import java.util.ArrayList;
import java.util.HashMap;
import android.text.BidiFormatter;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextDirectionHeuristics;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.text.style.StyleSpan;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.QuickContactBadge;
import android.widget.TextView;
import android.graphics.drawable.Drawable;
import com.mediatek.dialer.ontacts.common.format.TextHighlighter;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telephony.PhoneNumberUtils;
import android.content.res.TypedArray;
import com.android.dialer.telecom.TelecomUtil;

import com.android.dialer.contactphoto.ContactPhotoManager;
import com.android.dialer.contactphoto.ContactPhotoManager.DefaultImageRequest;
import com.android.dialer.phonenumbercache.ContactInfoHelper;
import com.android.dialer.calllogutils.PhoneAccountUtils;
import android.provider.CallLog.Calls;
import com.android.dialer.phonenumberutil.PhoneNumberHelper;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import com.mediatek.dialer.util.DialerSearchUtils;
import com.mediatek.dialer.search.DialerSearchHelper.DialerSearch;
import android.text.BidiFormatter;
import com.android.dialer.calllogutils.CallTypeIconsView;

import com.mediatek.dialer.compat.ContactsCompat.PhoneCompat;

//@}


/** ViewHolder for a contact row. */
public final class SmartSearchContactViewHolder extends ViewHolder implements OnClickListener {

  /// M: Mediatek start.
  /// M: [MTK Dialer Search] @{
  private final String TAG = "DialerPhoneNumberListAdapter";

  private final int VIEW_TYPE_UNKNOWN = -1;
  private final int VIEW_TYPE_CONTACT = 0;
  private final int VIEW_TYPE_CALL_LOG = 1;

  private final int NUMBER_TYPE_NORMAL = 0;
  private final int NUMBER_TYPE_UNKNOWN = 1;
  private final int NUMBER_TYPE_VOICEMAIL = 2;
  private final int NUMBER_TYPE_PRIVATE = 3;
  private final int NUMBER_TYPE_PAYPHONE = 4;
  private final int NUMBER_TYPE_EMERGENCY = 5;

  private final int DS_MATCHED_DATA_INIT_POS    = 3;
  private final int DS_MATCHED_DATA_DIVIDER     = 3;

  public final int NAME_LOOKUP_ID_INDEX        = 0;
  public final int CONTACT_ID_INDEX            = 1;
  public final int DATA_ID_INDEX               = 2;
  public final int CALL_LOG_DATE_INDEX         = 3;
  public final int CALL_LOG_ID_INDEX           = 4;
  public final int CALL_TYPE_INDEX             = 5;
  public final int CALL_GEOCODED_LOCATION_INDEX = 6;
  public final int PHONE_ACCOUNT_ID_INDEX                = 7;
  public final int PHONE_ACCOUNT_COMPONENT_NAME_INDEX     = 8;
  public final int PRESENTATION_INDEX          = 9;
  public final int INDICATE_PHONE_SIM_INDEX    = 10;
  public final int CONTACT_STARRED_INDEX       = 11;
  public final int PHOTO_ID_INDEX              = 12;
  public final int SEARCH_PHONE_TYPE_INDEX     = 13;
  public final int SEARCH_PHONE_LABEL_INDEX    = 14;
  public final int NAME_INDEX                  = 15;
  public final int SEARCH_PHONE_NUMBER_INDEX   = 16;
  public final int CONTACT_NAME_LOOKUP_INDEX   = 17;
  public final int IS_SDN_CONTACT              = 18;
  public final int DS_MATCHED_DATA_OFFSETS     = 19;
  public final int DS_MATCHED_NAME_OFFSETS     = 20;

  private ContactPhotoManager mContactPhotoManager;
  private final CallLogCache mPhoneNumberUtils;


  private PhoneNumberDisplayUtil mPhoneNumberHelper;

  private String mUnknownNumber;
  private String mPrivateNumber;
  private String mPayphoneNumber;
  private String mVoiceMail;
  private HashMap<Integer, Drawable> mCallTypeDrawables = new HashMap<Integer, Drawable>();

  private TextHighlighter mTextHighlighter;

  private final RowClickListener listener;

  private final Context context;

  private String number;

  private int position;

  private final BidiFormatter bidiFormatter = BidiFormatter.getInstance();

  
  public QuickContactBadge quickContactBadge;
  public TextView name;
  public TextView labelAndNumber;
  public View callInfo;
  public ImageView callType;
  public TextView address;
  public TextView date;
  public TextView accountLabel;

    
  public SmartSearchContactViewHolder(View view, RowClickListener listener) {
      super(view);
      this.listener = listener;

      context = view.getContext();
      TypedArray a = context.obtainStyledAttributes(null, R.styleable.ContactListItemView);
      
      view.setPadding(
          a.getDimensionPixelOffset(R.styleable.ContactListItemView_list_item_padding_left, 0),
          a.getDimensionPixelOffset(R.styleable.ContactListItemView_list_item_padding_top, 0),
          a.getDimensionPixelOffset(R.styleable.ContactListItemView_list_item_padding_right, 0),
          a.getDimensionPixelOffset(R.styleable.ContactListItemView_list_item_padding_bottom, 0));

      mPhoneNumberUtils = new CallLogCache(context);
      initResources(context);

      quickContactBadge = (QuickContactBadge) view.findViewById(R.id.quick_contact_photo);
      name = (TextView) view.findViewById(R.id.name);
      labelAndNumber = (TextView) view.findViewById(R.id.labelAndNumber);
      callInfo = (View) view.findViewById(R.id.call_info);
      callType = (ImageView) view.findViewById(R.id.callType);
      address = (TextView) view.findViewById(R.id.address);
      date = (TextView) view.findViewById(R.id.date);
      accountLabel = (TextView) view.findViewById(R.id.call_account_label);

      view.setOnClickListener(this);
    
    }

  public void bind(SearchCursor cursor, Context context) {
    if (getViewType(cursor) == VIEW_TYPE_CONTACT) {
        bindContactView(context,cursor);
    } else {
        bindCallLogView(context,cursor);
    }
  }


  /**
   * M: init UI resources
   * @param context
   */
   
  private void initResources(Context context) {
    mContactPhotoManager = ContactPhotoManager.getInstance(context);
    mPhoneNumberHelper = new PhoneNumberDisplayUtil();

    mVoiceMail = context.getResources().getString(R.string.voicemail);
    mPrivateNumber = context.getResources().getString(R.string.private_num_non_verizon);
    mPayphoneNumber = context.getResources().getString(R.string.payphone);
    mUnknownNumber = context.getResources().getString(R.string.unknown);

    // 1. incoming 2. outgoing 3. missed 4.voicemail
    // Align drawables of result items in dialer search to AOSP style.
    CallTypeIconsView.Resources resources = new CallTypeIconsView.Resources(context, false);
    mCallTypeDrawables.put(Calls.INCOMING_TYPE, resources.incoming);
    mCallTypeDrawables.put(Calls.OUTGOING_TYPE, resources.outgoing);
    mCallTypeDrawables.put(Calls.MISSED_TYPE, resources.missed);
    mCallTypeDrawables.put(Calls.VOICEMAIL_TYPE, resources.voicemail);
    /// M: Add reject icon
    mCallTypeDrawables.put(Calls.REJECTED_TYPE, resources.missed);

  }

  /**
   * M: calculate view's type from cursor
   * @param cursor
   * @return type number
   */
  private int getViewType(Cursor cursor) {
    int retval = VIEW_TYPE_UNKNOWN;
    final int contactId = cursor.getInt(CONTACT_ID_INDEX);
    final int callLogId = cursor.getInt(CALL_LOG_ID_INDEX);

    Log.d(TAG, "getViewType: contactId: " + contactId + " ,callLogId: " + callLogId);

    if (contactId > 0) {
      retval = VIEW_TYPE_CONTACT;
    } else if (callLogId > 0) {
      retval = VIEW_TYPE_CALL_LOG;
    }

    return retval;
  }

  /**
   * M: bind contact view from cursor data
   * @param view
   * @param context
   * @param cursor
   */
  private void bindContactView(Context context, Cursor cursor) {

    //final ViewHolder viewHolder = (ViewHolder) view.getTag();

    labelAndNumber.setVisibility(View.VISIBLE);
    callInfo.setVisibility(View.GONE);
    accountLabel.setVisibility(View.GONE);

    number = cursor.getString(SEARCH_PHONE_NUMBER_INDEX);
    String formatNumber = numberLeftToRight(number);
    if (formatNumber == null) {
      formatNumber = number;
    }


    position = cursor.getPosition();

    final int presentation = cursor.getInt(PRESENTATION_INDEX);
    //final PhoneAccountHandle accountHandle = PhoneAccountUtils.getAccount(
    //    cursor.getString(PHONE_ACCOUNT_COMPONENT_NAME_INDEX),
    //    cursor.getString(PHONE_ACCOUNT_ID_INDEX));

    final PhoneAccountHandle accountHandle = TelecomUtil.composePhoneAccountHandle(
        cursor.getString(PHONE_ACCOUNT_COMPONENT_NAME_INDEX),
        cursor.getString(PHONE_ACCOUNT_ID_INDEX));

    final int numberType = getNumberType(accountHandle, number, presentation);

    final int labelType = cursor.getInt(SEARCH_PHONE_TYPE_INDEX);
    CharSequence label = cursor.getString(SEARCH_PHONE_LABEL_INDEX);
    int subId = cursor.getInt(INDICATE_PHONE_SIM_INDEX);
    // Get type label only if it will not be "Custom" because of an empty label.
    // So IMS contacts search item don't show lable as "Custom".
    if (!(labelType == Phone.TYPE_CUSTOM && TextUtils.isEmpty(label))) {
      /// M: Using new API for AAS phone number label lookup
      label = PhoneCompat.getTypeLabel(context, labelType, label);
    }
    final CharSequence displayName = cursor.getString(NAME_INDEX);

    Uri contactUri = getContactUri(cursor);
    Log.d(TAG, "bindContactView, contactUri: " + contactUri);

    long photoId = cursor.getLong(PHOTO_ID_INDEX);

    if (numberType == NUMBER_TYPE_VOICEMAIL || numberType == NUMBER_TYPE_EMERGENCY) {
      photoId = 0;
      quickContactBadge.assignContactUri(null);
    } else {
      quickContactBadge.assignContactUri(contactUri);
    }
    quickContactBadge.setOverlay(null);

    if (photoId > 0) {
      mContactPhotoManager.loadThumbnail(quickContactBadge, photoId, false, true, null);
    } else {
      String identifier = cursor.getString(CONTACT_NAME_LOOKUP_INDEX);
      DefaultImageRequest request = new DefaultImageRequest((String) displayName, identifier, true);
      if (subId > 0) {
        request.subId = subId;
        request.photoId = cursor.getInt(IS_SDN_CONTACT);
      }
      mContactPhotoManager.loadThumbnail(quickContactBadge, photoId, false, true,
          request);
    }
   

    if (isSpecialNumber(numberType)) {
      if (numberType == NUMBER_TYPE_VOICEMAIL) {
        name.setText(mVoiceMail);

        labelAndNumber.setVisibility(View.VISIBLE);
        String highlight = getNumberHighlight(cursor);
        if (!TextUtils.isEmpty(highlight)) {
          SpannableStringBuilder style = highlightHyphen(highlight, formatNumber, number);
          labelAndNumber.setText(style);
        } else {
          labelAndNumber.setText(formatNumber);
        }
      } else {
        final String convert = specialNumberToString(numberType);
        name.setText(convert);
      }
    } else {
      // empty name ?
      if (!TextUtils.isEmpty(displayName)) {
        // highlight name
        String highlight = getNameHighlight(cursor);
        if (!TextUtils.isEmpty(highlight)) {
          SpannableStringBuilder style = highlightString(highlight, displayName);
          name.setText(style);
          //if (isRegularSearch(cursor)) {
          //  name.setText(highlightName(highlight, displayName));
          //}
        } else {
          name.setText(displayName);
        }
        // highlight number
        if (!TextUtils.isEmpty(formatNumber)) {
          highlight = getNumberHighlight(cursor);
          if (!TextUtils.isEmpty(highlight)) {
            SpannableStringBuilder style = highlightHyphen(highlight, formatNumber, number);
            setLabelAndNumber(labelAndNumber, label, style);
          } else {
            setLabelAndNumber(labelAndNumber, label, new SpannableStringBuilder(
                formatNumber));
          }
        } else {
          labelAndNumber.setVisibility(View.GONE);
        }
      } else {
        labelAndNumber.setVisibility(View.GONE);

        // highlight number and set number to name text view
        if (!TextUtils.isEmpty(formatNumber)) {
          final String highlight = getNumberHighlight(cursor);
          if (!TextUtils.isEmpty(highlight)) {
            SpannableStringBuilder style = highlightHyphen(highlight, formatNumber, number);
            name.setText(style);
          } else {
            name.setText(formatNumber);
          }
        } else {
          name.setVisibility(View.GONE);
        }
      }
    }
  }

  /**
   * M: Bind call log view by cursor data
   * @param view
   * @param context
   * @param cursor
   */
  private void bindCallLogView(Context context, Cursor cursor) {
    callInfo.setVisibility(View.VISIBLE);
    labelAndNumber.setVisibility(View.GONE);

    number = cursor.getString(SEARCH_PHONE_NUMBER_INDEX);
    String formattedNumber = numberLeftToRight(number);
    if (TextUtils.isEmpty(formattedNumber)) {
      formattedNumber = number;
    }

    final int presentation = cursor.getInt(PRESENTATION_INDEX);
    final PhoneAccountHandle accountHandle =
        TelecomUtil.composePhoneAccountHandle(cursor.getString(PHONE_ACCOUNT_COMPONENT_NAME_INDEX),
            cursor.getString(PHONE_ACCOUNT_ID_INDEX));

    final int numberType = getNumberType(accountHandle, number, presentation);

    final int type = cursor.getInt(CALL_TYPE_INDEX);
    final long callLogDate = cursor.getLong(CALL_LOG_DATE_INDEX);
    final int indicate = cursor.getInt(INDICATE_PHONE_SIM_INDEX);
    String geocode = cursor.getString(CALL_GEOCODED_LOCATION_INDEX);

    // create a temp contact uri for quick contact view.
    Uri contactUri = null;
    if (!TextUtils.isEmpty(number)) {
      contactUri = ContactInfoHelper.createTemporaryContactUri(number);
    }

    int contactType = ContactPhotoManager.TYPE_DEFAULT;
    if (numberType == NUMBER_TYPE_VOICEMAIL) {
      contactType = ContactPhotoManager.TYPE_VOICEMAIL;
      contactUri = null;
    }

    quickContactBadge.assignContactUri(contactUri);
    quickContactBadge.setOverlay(null);

    /// M: [ALPS01963857] keep call log and smart search's avatar in same color. @{
    boolean isVoiceNumber = mPhoneNumberUtils.isVoicemailNumber(accountHandle, number);
    String nameForDefaultImage = mPhoneNumberHelper.getDisplayNumber(context, number,
    /// M: [N Conflict Change]TODO:: POST_DIAL_DIGITS colum maybe need add in
    // DialerSearch table. here just set "" for build pass.
        presentation, number, "", isVoiceNumber).toString();
    /// @}

    String identifier = cursor.getString(CONTACT_NAME_LOOKUP_INDEX);
    DefaultImageRequest request = new DefaultImageRequest(nameForDefaultImage, identifier,
        contactType, true);
    mContactPhotoManager.loadThumbnail(quickContactBadge, 0, false, true, request);

    address.setText(geocode);

    if (isSpecialNumber(numberType)) {
      if (numberType == NUMBER_TYPE_VOICEMAIL) {
        name.setText(mVoiceMail);
        String highlight = getNumberHighlight(cursor);
        if (!TextUtils.isEmpty(highlight)) {
          SpannableStringBuilder style = highlightHyphen(highlight, formattedNumber, number);
          address.setText(style);
        } else {
          address.setText(formattedNumber);
        }
      } else {
        final String convert = specialNumberToString(numberType);
        name.setText(convert);
      }
    } else {
      if (!TextUtils.isEmpty(formattedNumber)) {
        String highlight = getNumberHighlight(cursor);
        if (!TextUtils.isEmpty(highlight)) {
          SpannableStringBuilder style = highlightHyphen(highlight, formattedNumber, number);
          name.setText(style);
        } else {
          name.setText(formattedNumber);
        }
      }
    }

    java.text.DateFormat dateFormat = DateFormat.getTimeFormat(context);
    String dateString = dateFormat.format(callLogDate);
    date.setText(dateString);

    callType.setImageDrawable(mCallTypeDrawables.get(type));

    final String accountLabelStr = PhoneAccountUtils.getAccountLabel(context, accountHandle);

    if (!TextUtils.isEmpty(accountLabelStr)) {
      accountLabel.setText(accountLabelStr);
      /// M: [ALPS02038899] set visible in case of gone
      accountLabel.setVisibility(View.VISIBLE);
      // Set text color for the corresponding account.
      int color = PhoneAccountUtils.getAccountColor(context, accountHandle);
      if (color == PhoneAccount.NO_HIGHLIGHT_COLOR) {
        int defaultColor = R.color.dialtacts_secondary_text_color;
        accountLabel.setTextColor(context.getResources().getColor(defaultColor));
      } else {
        accountLabel.setTextColor(color);
      }
    } else {
      accountLabel.setVisibility(View.GONE);
    }
  }

  private int getNumberType(PhoneAccountHandle accountHandle, CharSequence number,
          int presentation) {
    int type = NUMBER_TYPE_NORMAL;
    if (presentation == Calls.PRESENTATION_UNKNOWN) {
      type = NUMBER_TYPE_UNKNOWN;
    } else if (presentation == Calls.PRESENTATION_RESTRICTED) {
      type = NUMBER_TYPE_PRIVATE;
    } else if (presentation == Calls.PRESENTATION_PAYPHONE) {
      type = NUMBER_TYPE_PAYPHONE;
    } else if (mPhoneNumberUtils.isVoicemailNumber(accountHandle, number)) {
      type = NUMBER_TYPE_VOICEMAIL;
    }
    if (PhoneNumberHelper.isLegacyUnknownNumbers(number)) {
      type = NUMBER_TYPE_UNKNOWN;
    }
    return type;
  }

  private Uri getContactUri(Cursor cursor) {
    final String lookup = cursor.getString(CONTACT_NAME_LOOKUP_INDEX);
    final int contactId = cursor.getInt(CONTACT_ID_INDEX);
    return Contacts.getLookupUri(contactId, lookup);
  }

  private boolean isSpecialNumber(int type) {
    return type != NUMBER_TYPE_NORMAL;
  }

  /**
   * M: highlight search result string
   * @param highlight
   * @param target
   * @return
   */
  private SpannableStringBuilder highlightString(String highlight, CharSequence target) {
    SpannableStringBuilder style = new SpannableStringBuilder(target);
    int length = highlight.length();
    final int styleLength = style.length();
    int start = -1;
    int end = -1;
    for (int i = DS_MATCHED_DATA_INIT_POS; i + 1 < length; i += DS_MATCHED_DATA_DIVIDER) {
      start = (int) highlight.charAt(i);
      end = (int) highlight.charAt(i + 1) + 1;
      /// M: If highlight area is invalid, just skip it.
      if (start > styleLength || end > styleLength || start > end) {
        Log.d(TAG, "highlightString, start: " + start + " ,end: " + end + " ,styleLength: "
            + styleLength);
        break;
      }
      style.setSpan(new StyleSpan(Typeface.BOLD), start, end, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    return style;
  }

  /**
   * M: highlight searched result name
   * @param highlight
   * @param target
   * @return
   */
   /*
  private CharSequence highlightName(String highlight, CharSequence target) {
    String highlightedPrefix = getUpperCaseQueryString();
    if (highlightedPrefix != null) {
      mTextHighlighter = new TextHighlighter(Typeface.BOLD);
      target = mTextHighlighter.applyPrefixHighlight(target, highlightedPrefix);
    }
    return target;
  }
*/
  /**
   * M: highlight search result hyphen
   * @param highlight
   * @param target
   * @param origin
   * @return
   */
  private SpannableStringBuilder highlightHyphen(String highlight, String target, String origin) {
    if (target == null) {
      Log.w(TAG, "highlightHyphen target is null");
      return null;
    }
    SpannableStringBuilder style = new SpannableStringBuilder(target);
    ArrayList<Integer> numberHighlightOffset = DialerSearchUtils.adjustHighlitePositionForHyphen(
        target, highlight.substring(DS_MATCHED_DATA_INIT_POS), origin);
    if (numberHighlightOffset != null && numberHighlightOffset.size() > 1) {
      style.setSpan(new StyleSpan(Typeface.BOLD), numberHighlightOffset.get(0),
          numberHighlightOffset.get(1) + 1, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    return style;
  }

  private String getNameHighlight(Cursor cursor) {
    final int index = cursor.getColumnIndex(DialerSearch.MATCHED_NAME_OFFSET);
    return index != -1 ? cursor.getString(index) : null;
  }

  private boolean isRegularSearch(Cursor cursor) {
    final int index = cursor.getColumnIndex(DialerSearch.MATCHED_DATA_OFFSET);
    String regularSearch = (index != -1 ? cursor.getString(index) : "false");
    Log.d(TAG, "" + regularSearch);

    return Boolean.valueOf(regularSearch);
  }

  private String getNumberHighlight(Cursor cursor) {
    final int index = cursor.getColumnIndex(DialerSearch.MATCHED_DATA_OFFSET);
    return index != -1 ? cursor.getString(index) : null;
  }

  /**
   * M: set label and number to view
   * @param view
   * @param label
   * @param number
   */
  private void setLabelAndNumber(TextView view, CharSequence label,
          SpannableStringBuilder number) {
    if (PhoneNumberHelper.isUriNumber(number.toString())) {
      view.setText(number);
      return;
    }
    label = bidiFormatter.unicodeWrap(label,
                      TextDirectionHeuristics.FIRSTSTRONG_LTR);
    if (TextUtils.isEmpty(label)) {
      view.setText(number);
    } else if (TextUtils.isEmpty(number)) {
      view.setText(label);
    } else {
      number.insert(0, label + " ");
      view.setText(number);
    }
  }

  private String specialNumberToString(int type) {
    switch (type) {
      case NUMBER_TYPE_UNKNOWN:
        return mUnknownNumber;
      case NUMBER_TYPE_PRIVATE:
        return mPrivateNumber;
      case NUMBER_TYPE_PAYPHONE:
        return mPayphoneNumber;
      default:
        break;
    }
    return null;
  }


  /**
   * M: Fix ALPS01398152, Support RTL display for Arabic/Hebrew/Urdu
   * @param origin
   * @return
   */
  private String numberLeftToRight(String origin) {
    return TextUtils.isEmpty(origin) ? origin : '\u202D' + origin + '\u202C';
  }

  @Override
  public void onClick(View view) {
  /*
    if (view == callToActionView) {
      switch (currentAction) {
        case CallToAction.SHARE_AND_CALL:
          listener.openCallAndShare(dialerContact);
          break;
        case CallToAction.VIDEO_CALL:
          listener.placeVideoCall(number, position);
          break;
        case CallToAction.DUO_CALL:
          listener.placeDuoCall(number);
          break;
        case CallToAction.NONE:
        default:
          throw Assert.createIllegalStateFailException(
              "Invalid Call to action type: " + currentAction);
      }
    } else { */
      listener.placeVoiceCall(number, position);
    //}
  }
  /// @}
  
}
