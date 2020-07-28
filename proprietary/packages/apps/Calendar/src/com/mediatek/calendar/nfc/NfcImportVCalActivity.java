package com.mediatek.calendar.nfc;

import android.Manifest;
import android.content.pm.PackageManager;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.provider.CalendarContract.Events;
import android.util.Log;
import android.widget.Toast;

import com.android.calendar.R;

import com.mediatek.calendar.features.Features;
import com.mediatek.vcalendar.VCalParser;
import com.mediatek.vcalendar.VCalStatusChangeOperator;

public class NfcImportVCalActivity extends Activity implements VCalStatusChangeOperator {
    private static final boolean DEBUG = false;
    private static final String TAG = "NfcImportVCalActivity";
    protected static final String BUNDLE_KEY_EVENT_ID = "key_event_id";
    protected static final String BUNDLE_KEY_START_MILLIS = "key_start_millis";
    protected static final String BUNDLE_KEY_END_MILLIS = "key_end_millis";
    private static final String HANDLER_NAME = "importVCalendar";
    private NdefRecord mRecord;
    private Handler mImportHandler = null;
    private static final String[] CALENDAR_PERMISSION = {Manifest.permission.READ_CALENDAR,
                                                    Manifest.permission.WRITE_CALENDAR};

    private Handler getsaveContactHandler() {
        if (null == mImportHandler) {
            HandlerThread controllerThread = new HandlerThread(HANDLER_NAME);
            controllerThread.start();
            mImportHandler = new Handler(controllerThread.getLooper());
        }
        return mImportHandler;
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Log.i(TAG, "NfcImportVCalActivity, onCreate.");
        //if (!Features.isBeamPlusEnabled()) {
            //Log.w(TAG, "MTK_NFC_SUPPORT is not enabled!");
            //return;
        //}

        Intent intent = getIntent();
        if (!NfcAdapter.ACTION_NDEF_DISCOVERED.equals(intent.getAction())) {
            Log.w(TAG, "Unknowon intent " + intent);
            finish();
            return;
        }
        if (!hasRequiredPermission(CALENDAR_PERMISSION)) {
            Toast.makeText(getApplicationContext(),
                    getResources().getString(R.string.denied_required_permission),
                    Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        String type = intent.getType();
        if (type == null
                || (!"text/x-vcalendar".equalsIgnoreCase(type) && !"text/calendar"
                        .equalsIgnoreCase(type))) {
            Log.w(TAG, "Not a vcalendar!");
            finish();
            return;
        }
        if (intent.getParcelableArrayExtra(NfcAdapter.EXTRA_NDEF_MESSAGES) != null) {
        NdefMessage msg = (NdefMessage) intent
                .getParcelableArrayExtra(NfcAdapter.EXTRA_NDEF_MESSAGES)[0];
        mRecord = msg.getRecords()[0];
      }

        doImportAction(this);
    }

     protected boolean hasRequiredPermission(String[] permissions) {
         for (String permission : permissions) {
            if (checkSelfPermission(permission)
                    != PackageManager.PERMISSION_GRANTED) {
                 return false;
            }
        }
        return true;
       }

    private void doImportAction(VCalStatusChangeOperator listener) {
        Log.i(TAG, "In doImportAction ");
        byte[] content = mRecord.getPayload();
        String vcsContent = new String(content);

        if (DEBUG) {
            Log.v(TAG, "doImportAction, vcsContent=" + vcsContent);
        }
        Handler handler = getsaveContactHandler();
        if (handler != null) {
            handler.post(new ImporterThread(getApplicationContext(), vcsContent, listener));
        }
    }

    private static class ImporterThread extends Thread {
        private Context mContext;
        private VCalStatusChangeOperator mListener;
        private VCalParser mParser;
        private String mVcsContent;

        public ImporterThread(Context context, String vcsContent, VCalStatusChangeOperator listener) {
            mContext = context;
            mVcsContent = vcsContent;
            mListener = listener;
        }

        @Override
        public void run() {
            if (DEBUG) {
                Log.d(TAG, "startParseVcsContent... ... mVcsContent=" + mVcsContent);
            }
            mParser = new VCalParser(mVcsContent, mContext, mListener);
            mParser.startParseVcsContent();
        }
    }

    @Override
    public void vCalOperationCanceled(int finishedCnt, int totalCnt) {
        Log.v(TAG, "vCalOperationCanceled");
    }

    @Override
    public void vCalOperationExceptionOccured(int finishedCnt, int totalCnt, int type) {
        Log.v(TAG, "vCalOperationExceptionOccured");
    }

    @Override
    public void vCalOperationFinished(int successCnt, int totalCnt, Object obj) {
        Log.v(TAG, "vCalOperationFinished, obj=" + obj);
        Uri eventUri = null;
        long startMillis = System.currentTimeMillis();
        long endMills = startMillis;
        if (obj != null) {
            Bundle out = (Bundle) obj;
            long eventId = out.getLong(BUNDLE_KEY_EVENT_ID);
            Log.d(TAG, "vCalOperationFinished, eventId=" + eventId);
            startMillis = out.getLong(BUNDLE_KEY_START_MILLIS);
            endMills = out.getLong(BUNDLE_KEY_END_MILLIS);
            eventUri = Uri.withAppendedPath(Events.CONTENT_URI, String.valueOf(eventId));
        }
        Log.v(TAG, "vCalOperationFinished, timeMillis=" + startMillis);
        Log.v(TAG, "vCalOperationFinished, endMills=" + endMills);
        Intent intent = createViewEventIntent(eventUri, startMillis, endMills);
        startActivity(intent);
        finish();
    }

    @Override
    public void vCalOperationStarted(int totalCnt) {
        Log.v(TAG, "vCalOperationStarted");
    }

    @Override
    public void vCalProcessStatusUpdate(int currentCnt, int totalCnt) {
        Log.v(TAG, "vCalProcessStatusUpdate");
    }

    public Intent createViewEventIntent(Uri uri, long startMills, long endMills) {
        Intent intent = new Intent(Intent.ACTION_VIEW);
        //intent.setClass(this, EventInfoActivity.class);
        intent.setDataAndType(uri, "vnd.android.cursor.item/event");
        intent.putExtra(android.provider.CalendarContract.EXTRA_EVENT_BEGIN_TIME, startMills);
        intent.putExtra(android.provider.CalendarContract.EXTRA_EVENT_END_TIME, endMills);
        //intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }
}
