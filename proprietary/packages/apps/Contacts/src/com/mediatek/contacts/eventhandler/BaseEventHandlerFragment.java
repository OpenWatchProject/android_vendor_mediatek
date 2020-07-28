package com.mediatek.contacts.eventhandler;

import android.app.Fragment;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.contacts.util.ContactsConstants;
import com.mediatek.contacts.util.Log;

/**
 * this class used to handle base event for Fragment.such as phb state change,
 * plug out/sd card.offer the default action for response base event.
 */
public class BaseEventHandlerFragment extends Fragment implements GeneralEventHandler.Listener {
    private static String TAG = "BaseEventHanleFragment";
    private static final int DEFAULT_NO_USE_SUBID = -1;

    @Override
    public void onCreate(Bundle savedState) {
        super.onCreate(savedState);
        Log.i(TAG, "[onCreate]");
        GeneralEventHandler.getInstance(getContext()).register(this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "[onDestroy]");
        GeneralEventHandler.getInstance(getContext()).unRegister(this);
    }

    @Override
    public void onReceiveEvent(String eventType, Intent extraData) {
        int subId = getSubId();
        int stateChangeSubId = extraData.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                ContactsConstants.ERROR_SUB_ID);
        Log.i(TAG,
                "[onReceiveEvent] eventType: " + eventType + ", extraData: " + extraData.toString()
                        + ",subId: " + subId + ",stateChangeSubId: " + stateChangeSubId);
        if (GeneralEventHandler.EventType.PHB_STATE_CHANGE_EVENT.equals(eventType)
                && SubscriptionManager.isValidSubscriptionId(subId)
                && SubscriptionManager.isValidSubscriptionId(stateChangeSubId)
                && (subId == stateChangeSubId)) {
            Log.i(TAG, "[onReceiveEvent] phb state change,default action: getActivity finish!");
            getActivity().finish();
        }
    }

    protected int getSubId() {
        return DEFAULT_NO_USE_SUBID;
    }
}
