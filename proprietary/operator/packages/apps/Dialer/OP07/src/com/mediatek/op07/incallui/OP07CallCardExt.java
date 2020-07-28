package com.mediatek.op07.incallui;

import android.content.Context;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.incallui.answer.impl.AnswerFragment;
import com.android.incallui.incall.impl.InCallFragment;
import com.android.incallui.video.impl.VideoCallFragment;

import com.android.incallui.call.DialerCall;
import com.android.incallui.call.state.DialerCallState;
import com.mediatek.incallui.ext.DefaultCallCardExt;

import com.mediatek.op07.dialer.R;

/**
 * Plugin implementation for InCallExt.
 */
public class OP07CallCardExt extends DefaultCallCardExt {
    private static final String TAG = "OP07CallCardExt";
    private Context mContext;

    private static final String ID = "id";
    private static final String NAME_ID = "contactgrid_contact_name";

    private static final int ID_CALLER_STATUS = 0x20ffffff;

    private static final String KEY_EXTRA_VERSTAT        = "verstat";
    private static final String VERSTAT_VALID_STATUS = "Valid Number";
    private static final String VERSTAT_INVALID_STATUS = "Invalid Number";

    private static final int DEFAULT_VAULE_EXTRA_VERSTAT = -1;

    public OP07CallCardExt(Context context) {
        super();
        Log.d(TAG, "OP07CallCardExt");
        mContext = context;
    }

    /**
      * Interface to modify the hd icon of OP08
      *
      * @param context the incallactivity context
      * @param view the callcard view
      */
    @Override
    public void onViewCreated(Context context, View view) {
        if (context == null) {
            Log.d(TAG, "onViewCreated, context is null.");
            return;
        }

        Resources resource = context.getResources();
        String packageName = context.getPackageName();

        View nameView =
                view.findViewById(
                        resource.getIdentifier(NAME_ID, ID, packageName));
        if (nameView == null) {
            Log.d(TAG, "onViewCreated, nameView is null");
            return;
        }

        ViewGroup parentView = (ViewGroup) nameView.getParent();
        if (parentView == null) {
            Log.d(TAG, "onViewCreated, parentView is null");
            return;
        }

        int destIndex = parentView.indexOfChild(nameView) + 1;
        int color = ((TextView) nameView).getCurrentTextColor();

        TextView textView = new TextView(mContext);
        textView.setTextAppearance(mContext, R.style.InCall_TextAppearance);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        lp.gravity = Gravity.CENTER_HORIZONTAL;
        textView.setLayoutParams(lp);
        textView.setVisibility(View.INVISIBLE);
        textView.setText(VERSTAT_INVALID_STATUS);
        textView.setTextColor(color);
        textView.setId(ID_CALLER_STATUS);
        parentView.addView(textView, destIndex);
    }

    @Override
    public void setPrimary(Context context, Object obj, Object fragmentObj) {
        Log.d(TAG, "setPrimary");
        if (fragmentObj == null) {
            return;
        }

        View view = null;
        if (fragmentObj instanceof InCallFragment) {
            InCallFragment incallFragment = (InCallFragment) fragmentObj;
            view = incallFragment.getView();
        } else if (fragmentObj instanceof AnswerFragment) {
            AnswerFragment answerFragment = (AnswerFragment) fragmentObj;
            view = answerFragment.getView();
        } else if (fragmentObj instanceof VideoCallFragment) {
            VideoCallFragment videoCallFragment = (VideoCallFragment) fragmentObj;
            view = videoCallFragment.getView();
        }

        if (view == null) {
            Log.d(TAG, "setPrimary, view is null");
            return;
        }

        TextView textView = (TextView) view.findViewById(ID_CALLER_STATUS);
        if (textView == null) {
            Log.d(TAG, "setPrimary, textView is null");
            return;
        }

        if (obj == null || !(obj instanceof DialerCall)) {
            textView.setText("");
            textView.setVisibility(View.GONE);
            return;
        }

        DialerCall call = (DialerCall) obj;
        if (call.getState() == DialerCallState.INCOMING
                || call.getState() == DialerCallState.CALL_WAITING) {
            Bundle bundle = call.getExtras();
            if (bundle != null) {
                int value = bundle.getInt(KEY_EXTRA_VERSTAT, DEFAULT_VAULE_EXTRA_VERSTAT);
                if (value == -1) {
                    textView.setText("");
                    textView.setVisibility(View.GONE);
                } else if (value == 0) {
                    textView.setText(VERSTAT_INVALID_STATUS);
                    textView.setVisibility(View.VISIBLE);
                } else if (value == 1) {
                    textView.setText(VERSTAT_VALID_STATUS);
                    textView.setVisibility(View.VISIBLE);
                }
            }
        } else {
            textView.setVisibility(View.GONE);
        }
    }
}