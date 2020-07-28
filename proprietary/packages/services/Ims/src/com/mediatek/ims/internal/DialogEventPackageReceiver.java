
package com.mediatek.ims.internal;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.content.LocalBroadcastManager;
import android.telephony.Rlog;
import android.telephony.ims.ImsExternalCallState;

import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.ims.ImsConstants;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import org.xmlpull.v1.XmlPullParserException;

/**
 * Responsible for receiving broadcast of Dialog Event Package.
 *
 */
public class DialogEventPackageReceiver {
    private static final String TAG = "DialogEventPackageReceiver";
    private DialogEventPackageParser mParser;
    private Listener mListener;

    /**
     * Listener interface for notifying Dialog Event Package updates from IMS UA.
     */
    public interface Listener {
        /**
         * Notifies for Dialog Event Package updates.
         * @param list the instances of IMS external call state
         */
        public void onStateChanged(List<ImsExternalCallState> list);
    }

    /**
     * Implements for the {@link Listener}, which is responsible for notifying Dialog Event Package
     * updates from IMS UA.
     */
    public abstract static class ListenerBase implements Listener {

        /**
         * Notifies for Dialog Event Package updates.
         * @param list the instances of IMS external call state
         */
        public void onStateChanged(List<ImsExternalCallState> list) {
            // no-op
        }
    }

    /**
     * Creates Dialog Event Package object.
     * @param context application context
     * @param listener  to receive Dialog Event Package updates
     */
    public DialogEventPackageReceiver(Context context, Listener listener) {
        registerForBroadcast(context);
        mParser = new DepXmlPullParser();
        mListener = listener;
        Rlog.d(TAG, "DialogEventPackageReceiver");
    }

    private void registerForBroadcast(Context context) {
        final IntentFilter filter = new IntentFilter();
        filter.addAction(ImsConstants.ACTION_IMS_DIALOG_EVENT_PACKAGE);
        LocalBroadcastManager.getInstance(context).registerReceiver(mReceiver, filter);
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(ImsConstants.ACTION_IMS_DIALOG_EVENT_PACKAGE)) {
                /// M: keep old logic for 92gen and before
                if (ImsCommonUtil.supportMdAutoSetupIms() == false) {
                    processDepIntent(intent);
                } else {
                    ArrayList<ImsExternalCallState> dialogList = intent
                            .getParcelableArrayListExtra(ImsConstants.EXTRA_DEP_CONTENT);
                    for(ImsExternalCallState dialog: dialogList) {
                        Rlog.d(TAG, "ACTION_IMS_DIALOG_EVENT_PACKAGE content:" + dialog.toString());
                    }
                    mListener.onStateChanged(dialogList);
                }
            }
        }
    };

    private void processDepIntent(Intent intent) {
        // TODO: check phone id for multiple ims
        String content = intent.getStringExtra(ImsConstants.EXTRA_DEP_CONTENT);
        Rlog.d(TAG, "ACTION_IMS_DIALOG_EVENT_PACKAGE " + content);
        InputStream in =
                new ByteArrayInputStream(content.getBytes(StandardCharsets.UTF_8));
        try {
            DialogInfo dialogInfo = mParser.parse(in);
            List<ImsExternalCallState> dialogList = ExternalCallStateFactory.getInstance()
                    .makeExternalCallStates(dialogInfo);
            mListener.onStateChanged(dialogList);
        } catch (XmlPullParserException e) {
            Rlog.d(TAG, "processDepIntent has XmlPullParserException " + e);
        } catch (IOException e) {
            Rlog.d(TAG, "processDepIntent has IOException " + e);
        }
    }
}
