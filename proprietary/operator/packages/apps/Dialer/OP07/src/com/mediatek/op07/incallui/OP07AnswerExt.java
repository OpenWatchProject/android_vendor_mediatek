package com.mediatek.op07.incallui;

import android.content.Context;
import android.content.res.Resources;
import android.util.Log;
import android.view.View;

import com.android.dialer.telecom.TelecomUtil;
import com.android.incallui.InCallPresenter;
import com.android.incallui.answer.impl.AffordanceHolderLayout;
import com.android.incallui.answer.impl.AnswerFragment;
import com.android.incallui.answer.impl.AnswerFragment.SecondaryBehavior;
import com.android.incallui.answer.impl.SmsBottomSheetFragment;
import com.android.incallui.answer.impl.affordance.SwipeButtonView;
import com.android.incallui.call.CallList;

import com.mediatek.incallui.ext.DefaultAnswerExt;
import com.mediatek.op07.dialer.R;

import java.util.ArrayList;
import java.util.List;
/**
 * Plugin implementation for IAnswerExt.
 */
public class OP07AnswerExt extends DefaultAnswerExt {
    private static final String TAG = "OP07AnswerExt";
    private Context mContext;

    private static final String ID = "id";
    private static final String STRING_ID = "string";

    private ArrayList<CharSequence> additionActionNames = new ArrayList<CharSequence>();

    private static final int RIGHT_ICON_ANSWER_AS_VOICE = 1;
    private static final int RIGHT_ICON_REJECT_BY_SMS = 2;
    private static final int RIGHT_ICON_ANSWER_AND_RELEASE = 3;
    private static final int RIGHT_ICON_ANSWER_AS_ONE_WAY = 4;

    private static final String ID_VALUE_STRING_ANSWER_ONEWAY_VIDEO =
            "menu_oneway_videocall";
    private static final String ID_VALUE_STRING_REJECT_VIDEO_BY_SMS =
            "a11y_incoming_call_reject_with_sms";
    private static final String ID_VALUE_STRING_ANSWER_RELEASE_VIDEO =
            "a11y_incoming_call_answer_and_release";
    private static final String ID_VALUE_INCOMING_SECONDARY_BUTTON =
            "incoming_secondary_button2";
    private static final String ID_VALUE_INCOMING_CONTAINER =
            "incoming_container";

    private SwipeButtonView answerAndReleaseButton;
    private AffordanceHolderLayout affordanceHolderLayout;
    private SmsBottomSheetFragment additionResponseFragment;

    public OP07AnswerExt(Context context) {
        super();
        Log.d(TAG, "OP07AnswerExt");
        mContext = context;
    }

    /**
     * Called when view created in answecallfragment
     * @param context  the context of fragment
     * @param rootView the rootView in fragment
     */
    @Override
    public void onViewCreated(Context context, View view) {
        if (context == null) {
            Log.d(TAG, "onViewCreated, context is null.");
            return;
        }

        answerAndReleaseButton = (SwipeButtonView) getViewInstanceById(
                context, ID_VALUE_INCOMING_SECONDARY_BUTTON, view);
        if (answerAndReleaseButton == null) {
            Log.d(TAG, "onViewCreated, answerAndReleaseButton is null");
            return;
        }

        affordanceHolderLayout = (AffordanceHolderLayout) getViewInstanceById(
                context, ID_VALUE_INCOMING_CONTAINER, view);
        if (affordanceHolderLayout == null) {
            Log.d(TAG, "onViewCreated, affordanceHolderLayout is null");
            return;
        }
    }

    private String getStringValueById(Context cnx, String idValue) {
        if (cnx == null) {
            Log.d(TAG, "onViewCreated, context is null.");
            return "";
        }

        Resources resource = cnx.getResources();
        String packageName = cnx.getPackageName();
        int resId = resource.getIdentifier(idValue, STRING_ID, packageName);
        if (resId != 0) {
            return resource.getString(resId);
        }
        return "";
    }

    private View getViewInstanceById(Context cnx, String idValue, View view) {
        if (cnx == null) {
            Log.d(TAG, "getViewInstanceById, context is null.");
            return null;
        }

        Resources resource = cnx.getResources();
        String packageName = cnx.getPackageName();
        return view.findViewById(resource.getIdentifier(idValue, ID, packageName));
    }

    /**
     * Update the right bottom icon in answerfragment
     * @param obj the Object of fragment
     * @return true if updated in plugin side
     */
    @Override
    public boolean updateRightIconAction(Object obj) {
        Log.d(TAG, "updateRightIconAction");
        if (obj == null) {
            return false;
        }

        AnswerFragment answerFragment = (AnswerFragment) obj;

        additionActionNames.clear();
        Context cnx = answerFragment.getContext();
        if (cnx == null) {
            Log.d(TAG, "updateRightIconAction, context is null");
            return false;
        }
        boolean showOneWayVideoEntry = false;
        if (answerFragment.isVideoCall()) {
            additionActionNames.add(getStringValueById(cnx, ID_VALUE_STRING_ANSWER_ONEWAY_VIDEO));
            showOneWayVideoEntry = true;
        }

        boolean showRejectVideoBySms = false;
        if (answerFragment.isVideoCall()
                && answerFragment.supportsRejectVideoCallBySms()) {
            additionActionNames.add(getStringValueById(cnx, ID_VALUE_STRING_REJECT_VIDEO_BY_SMS));
            showRejectVideoBySms = true;
        }

        if (answerFragment.allowAnswerAndRelease()
                && CallList.getInstance().getActiveCall() != null) {
            additionActionNames.add(getStringValueById(cnx, ID_VALUE_STRING_ANSWER_RELEASE_VIDEO));
        }

        if (additionActionNames.size() > 1) {
            applyRightIconAction(answerFragment, SecondaryBehavior.ADDITION_ACTION_ITEMS);
            return true;
        } else if (additionActionNames.size() == 1) {
            if (showOneWayVideoEntry) {
                applyRightIconAction(answerFragment, SecondaryBehavior.ANSWER_VIDEO_AS_ONEWAY);
            } else if (showRejectVideoBySms) {
                applyRightIconAction(answerFragment, SecondaryBehavior.REJECT_WITH_SMS);
            }
            return true;
        } else {
            additionActionNames.clear();
            return false;
        }
    }

    private void applyRightIconAction(AnswerFragment answerFragment,
          SecondaryBehavior desBehavior) {
        if (answerAndReleaseButton == null) {
            Log.d(TAG, "applyRightIconAction, button is null at this time");
            return;
        }

        answerFragment.answerAndReleaseBehavior = desBehavior;
        answerFragment.answerAndReleaseBehavior.applyToView(answerAndReleaseButton);
    }

    /**
     * Show menu popup when slid right button in the bottom
     * @param obj the Object of fragment
     */
    @Override
    public void showAdditionMenu(Object obj) {
        if (obj == null) {
            return;
        }

        AnswerFragment answerFragment = (AnswerFragment) obj;
        if (answerFragment.getContext() == null
                || answerFragment.isDetached()
                || answerFragment.getChildFragmentManager().isDestroyed()) {
            return;
        }

        if (additionActionNames == null || additionActionNames.size() <= 1) {
            Log.d(TAG, "showAdditionMenu, not enough action");
            return;
        }

        additionResponseFragment = SmsBottomSheetFragment.newInstance(additionActionNames);
        additionResponseFragment.show(answerFragment.getChildFragmentManager(), null);
        answerAndReleaseButton
            .animate()
            .alpha(0)
            .withEndAction(
                new Runnable() {
                    @Override
                    public void run() {
                        affordanceHolderLayout.reset(false);
                        answerAndReleaseButton.animate().alpha(1);
                    }
                });
        TelecomUtil.silenceRinger(answerFragment.getContext());
    }


    private int findIndexByString(Context cnx, CharSequence text) {
        int index = -1;
        if (text == null || cnx == null) {
            return index;
        }

        if (text.equals(getStringValueById(cnx, ID_VALUE_STRING_REJECT_VIDEO_BY_SMS))) {
            index = RIGHT_ICON_REJECT_BY_SMS;
        } else if (text.equals(getStringValueById(cnx, ID_VALUE_STRING_ANSWER_RELEASE_VIDEO))) {
            index = RIGHT_ICON_ANSWER_AND_RELEASE;
        } else if (text.equals(getStringValueById(cnx, ID_VALUE_STRING_ANSWER_ONEWAY_VIDEO))) {
            index = RIGHT_ICON_ANSWER_AS_ONE_WAY;
        }
        return index;
    }

    /**
     * Called when the item in addition menu clicked.
     * @param obj the Object of fragment
     * @param text the CharSequence string
     * @return true if handle selection in plugin side
     */
    @Override
    public boolean handleAdditionSelected(Object obj, CharSequence text) {
        if (obj == null) {
            return false;
        }
        boolean handled = false;
        AnswerFragment answerFragment = (AnswerFragment) obj;
        int index = findIndexByString(answerFragment.getContext(), text);
        Log.d(TAG, "handleAdditionSelected, text = " + text + ", index = " + index);

        switch(index) {
            case RIGHT_ICON_REJECT_BY_SMS:
                handled = true;
                answerFragment.showMessageMenu();
                break;
            case RIGHT_ICON_ANSWER_AND_RELEASE:
                handled = true;
                answerFragment.performAnswerAndRelease();
                break;
            case RIGHT_ICON_ANSWER_AS_ONE_WAY:
                handled = true;
                answerFragment.acceptCallAsOneWayVideo();
            default:
            break;
        }
        return handled;
    }

    /**
     * Check if should show customization view.
     * @param items  List<CharSequence>, the menu items
     * @return true if should show customization view
     */
    @Override
    public boolean shouldShowCustomizationView(List<CharSequence> items) {
        return (items != null) ? items.size() > 3: true;
    }
}
