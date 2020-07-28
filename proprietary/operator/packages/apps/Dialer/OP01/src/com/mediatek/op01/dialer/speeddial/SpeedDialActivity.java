package com.mediatek.op01.dialer.speeddial;

import static android.Manifest.permission.READ_CONTACTS;
import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.graphics.Rect;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.PhoneLookup;
import android.telephony.PhoneNumberUtils;
import android.text.method.DialerKeyListener;
import android.text.TextUtils;
import android.util.Log;
import android.view.MenuItem;
import android.view.TouchDelegate;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.Toast;

//import com.android.contacts.common.ContactPhotoManager;
//import com.mediatek.contacts.util.SimContactPhotoUtils;
import com.mediatek.op01.dialer.R;
import com.mediatek.provider.MtkContactsContract;
import com.mediatek.provider.MtkContactsContract.CommonDataKinds.ImsCall;

import java.util.ArrayList;

import com.mediatek.dialer.common.LogUtils;


public class SpeedDialActivity extends ListActivity
        implements View.OnClickListener, DialogInterface.OnClickListener,
        DialogInterface.OnShowListener, DialogInterface.OnDismissListener {

    public static final String PREF_NAME = "speed_dial";
    private static final int QUERY_TOKEN = 47;
    private static final int VIEW_EDGE = 30;
    private static final int SPEED_DIAL_DIALOG_ADD = 1;

    private String TAG = "SpeedDialActivity";

    private SharedPreferences       mPref;
    private int                     mIndex;
    private int                     mQueryTimes;
    private ListView                mListView;
    private SpeedDialQueryHandler   mQueryHandler;
    private MatrixCursor            mMatrixCursor;
    private SimpleCursorAdapter     mSimpleCursorAdapter;
    private AlertDialog             mRemoveConfirmDialog = null;

    //private ContactPhotoManager     mContactPhotoManager;
    private boolean                 mIsWaitingActivityResult;
    public Uri mUri = Uri.parse("content://speed_dial/numbers");

    private int mAddPosition = -1;
    private int mRemovePosition = -1;

    static final int SPEED_DIAL_MIN = 2;
    static final int SPEED_DIAL_MAX = 9;
    private static final int LIST_CAPACITY = 9;

    private ProgressDialog mProgressDialog = null;
    private static final int WAIT_CURSOR_START = 1000;
    private static final long WAIT_CURSOR_DELAY_TIME = 500;

    private boolean mActivityDestroyed = false;

    //private SimReceiver mSimReceiver;
    //public static final String ACTION_PHB_LOAD_FINISHED = "com.android.contacts.ACTION_PHB_LOAD_FINISHED";

    // For SharePreference
    private String[] mPrefNumState = {
        "", "", "", "", "", "", "", "", "", ""
    };

    //For adapter
    public static final String[] DATA_FROM = {
        PhoneLookup._ID,
        PhoneLookup.DISPLAY_NAME,
        PhoneLookup.TYPE,
        PhoneLookup.NUMBER,
        PhoneLookup.PHOTO_ID,
        MtkContactsContract.PhoneLookup.INDICATE_PHONE_SIM
    };

    public static final int[] ID_TO = {
        R.id.sd_index,
        R.id.sd_name,
        R.id.sd_label,
        R.id.sd_number,
        R.id.sd_photo,
        R.id.sd_remove,
    };

    //For query
    static final String[] QUERY_PROJECTION = {
            PhoneLookup._ID, // 0
            PhoneLookup.DISPLAY_NAME, // 1
            PhoneLookup.TYPE, // 2
            PhoneLookup.NUMBER, // 3
            MtkContactsContract.PhoneLookup.INDICATE_PHONE_SIM, // 4
            PhoneLookup.PHOTO_ID, // 5
            PhoneLookup.LABEL, // 6
    };

    private static final int QUERY_DISPLAY_NAME_INDEX = 1;
    private static final int QUERY_LABEL_INDEX = 2;
    private static final int QUERY_NUMBER_INDEX = 3;
    private static final int QUERY_INDICATE_PHONE_SIM_INDEX = 4;
    private static final int QUERY_PHOTO_ID_INDEX = 5;
    private static final int QUERY_CUSTOM_LABEL_INDEX = 6;

    private static final int BIND_ID_INDEX = 0;
    private static final int BIND_DISPLAY_NAME_INDEX = 1;
    private static final int BIND_LABEL_INDEX = 2;
    private static final int BIND_NUMBER_INDEX = 3;
    private static final int BIND_PHOTO_ID_INDEX = 4;
    private static final int BIND_INDICATE_PHONE_SIM_INDEX = 5;

    public static final int TYPE_NUMBER_NORMAL = 7;
    public static final int TYPE_NUMBER_IMS    = 9;

    private static int REQUEST_CODE_PICK_CONTACT = 1;

    private ArrayList<QueueItem> mToastQueue = new ArrayList<QueueItem>();

    private static final boolean DEBUG = ("eng".equals(Build.TYPE) ||
                                        "userdebug".equals(Build.TYPE));

    public void onListItemClick(ListView l, View v, int position, long id) {
        if (position == 0 || !TextUtils.isEmpty(mPrefNumState[position + 1])) {
            return;
        }

        mAddPosition = position;
        showDialog(SPEED_DIAL_DIALOG_ADD);
        return;
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(TAG, "onActivityResult() mAddPosition:" + mAddPosition);
        mIsWaitingActivityResult = false;
        if (requestCode != REQUEST_CODE_PICK_CONTACT || RESULT_OK != resultCode || data == null) {
            return;
        }

        String index = data.getData().getLastPathSegment();
        String number = "";
        int type = TYPE_NUMBER_NORMAL;
        String numberType;
        Cursor cursor = this.getContentResolver().query(Data.CONTENT_URI, new String[] {
                Data._ID, Data.DATA1, Data.MIMETYPE
            }, "Data._ID" + " = " + index, null, null);

        Log.i(TAG, "onActivityResult: index = " + index);
        if (cursor != null && cursor.getCount() > 0) {
            cursor.moveToFirst();
            number = cursor.getString(1);
            numberType = cursor.getString(2);
            Log.d(TAG, "onActivityResult: numberType = " + numberType);
            if (ImsCall.CONTENT_ITEM_TYPE.equals(numberType)) {
                type = TYPE_NUMBER_IMS;
            }
        } else {
            if (cursor != null) {
                cursor.close();
            }
            return;
        }
        cursor.close();

        if (DEBUG) {
            LogUtils.printSensitiveInfo(TAG, "onActivityResult: number :  " + number
                                   + ", type = " + type);
        }
        if (PhoneNumberUtils.isUriNumber(number)) {
            mMatrixCursor.moveToPosition(-1);
            Toast.makeText(this, getString(R.string.invalid_number), Toast.LENGTH_LONG).show();
            return;
        } else if (findKeyByNumber(number) > -1) {
            mMatrixCursor.moveToPosition(-1);
            Toast.makeText(this, getString(R.string.reselect_number), Toast.LENGTH_LONG).show();
            return;
        } else {
            getPrefStatus();
            mPrefNumState[mAddPosition + 1] = number;
            SharedPreferences.Editor editor = mPref.edit();
            editor.putString(String.valueOf(mAddPosition + 1), mPrefNumState[mAddPosition + 1]);
            editor.apply();

            updateSpeedDial(mAddPosition + 1, number, type);

            enQueueItem(mAddPosition);
        }
    }

    private int findKeyByNumber(String number) {
        for (int i = SPEED_DIAL_MIN; i < SPEED_DIAL_MAX + 1; i++) {
            if (shouldCollapse(this, number, mPrefNumState[i])) {
                return i;
            }
        }
        return -1;
    }

    public static final boolean shouldCollapse(Context context, CharSequence data1, CharSequence data2) {
        if (data1 == data2) {
            return true;
        }

        if (data1 == null || data2 == null) {
            return false;
        }

        if (TextUtils.equals(data1, data2)) {
            return true;
        }

        String[] dataParts1 = data1.toString().split(String.valueOf(PhoneNumberUtils.WAIT));
        String[] dataParts2 = data2.toString().split(String.valueOf(PhoneNumberUtils.WAIT));
        if (dataParts1.length != dataParts2.length) {
            return false;
        }

        for (int i = 0; i < dataParts1.length; i++) {
            if (!PhoneNumberUtils.compare(context, dataParts1[i], dataParts2[i])) {
                return false;
            }
        }

        return true;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate() , begin");

        //mContactPhotoManager = ContactPhotoManager.getInstance(this);

        mListView = getListView();
        mSimpleCursorAdapter = new SimpleCursorAdapter(this, R.layout.mtk_speed_dial_list_item, null,
                DATA_FROM, ID_TO);
        mSimpleCursorAdapter.setViewBinder(new SimpleCursorAdapter.ViewBinder() {
            public boolean setViewValue(final View view, Cursor cursor, int columnIndex) {
                boolean isEmpty = TextUtils.isEmpty(cursor.getString(BIND_NUMBER_INDEX));
                int viewId = view.getId();
                if (viewId == R.id.sd_photo) {
                    view.setVisibility(View.GONE);
                    view.setClickable(false);
                    if (!isEmpty) {
                        //view.setBackgroundDrawable(null);
                        //mContactPhotoManager.loadThumbnail((ImageView) view, Long.valueOf(
                                //cursor.getString(BIND_PHOTO_ID_INDEX)).longValue(), true);
                    }
                    return true;
                } else if (viewId == R.id.sd_label) {
                    view.setVisibility(isEmpty ? View.GONE : View.VISIBLE);
                } else if (viewId == R.id.sd_number) {
                    view.setVisibility(isEmpty ? View.GONE : View.VISIBLE);
                } else if (viewId == R.id.sd_name) {
                    view.setEnabled(!isEmpty
                            || TextUtils.equals(cursor.getString(BIND_ID_INDEX), "1"));

                } else if (viewId == R.id.sd_remove) {
                    view.setVisibility(isEmpty ? View.GONE : View.VISIBLE);
                    view.setOnClickListener(SpeedDialActivity.this);
                    if (!isEmpty) {
                       final View parent = (View) view.getParent();
                       parent.post(new Runnable() {
                           public void run() {
                               final Rect r = new Rect();
                               view.getHitRect(r);
                               r.top -= VIEW_EDGE;
                               r.bottom += VIEW_EDGE;
                               r.left -= VIEW_EDGE;
                               r.right += VIEW_EDGE;
                               parent.setTouchDelegate(new TouchDelegate(r, view));
                           }
                       });
                    } else {
                        final View parent = (View) view.getParent();
                        parent.post(new Runnable() {
                            public void run() {
                                parent.setTouchDelegate(null);
                            }
                        });
                    }
                    return true;
                }
                return false;
            }
        });

        mQueryHandler = new SpeedDialQueryHandler(this);
        mListView.setAdapter(mSimpleCursorAdapter);

        //mSimReceiver = new SimReceiver(this);
        //mSimReceiver.register();

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
        Log.d(TAG, "onCreate() , end");
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart end");
    }

    @Override
    protected void onResume() {
        super.onResume();
        getPrefStatus();

        startQuery();
        Log.d(TAG, "onResume end");
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();

        dismissProgressIndication();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();

        //if (mSimReceiver != null) {
        //    mSimReceiver.unregister();
        //}

        if (mMatrixCursor != null) {
            mMatrixCursor.close();
        }
        mActivityDestroyed = true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            finish();
            return true;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (SPEED_DIAL_DIALOG_ADD ==  id) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.call_speed_dial);
            builder.setPositiveButton(R.string.sd_add, this);
            builder.setNegativeButton(android.R.string.cancel, this);
            builder.setView(View.inflate(this, R.layout.mtk_speed_dial_input_dialog, null));
            Dialog dialog = builder.create();
            dialog.setOnShowListener(this);
            dialog.setOnDismissListener(this);
            return dialog;
        }
        return null;
    }

    private void getPrefStatus() {
        Log.i(TAG, "getPrefStatus()");
        mPref = getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
        for (int i = SPEED_DIAL_MIN; i < SPEED_DIAL_MAX + 1; ++i) {
            mPrefNumState[i] = mPref.getString(String.valueOf(i), "");
        }
    }

    private void initMatrixCursor() {
        // if (mMatrixCursor != null) mMatrixCursor.close();
        mMatrixCursor = new MatrixCursor(DATA_FROM, LIST_CAPACITY);
        mMatrixCursor.addRow(new String[] {
                "1", getResources().getString(R.string.voicemail), "", "", "", ""
        });
        mQueryTimes = SPEED_DIAL_MIN;
    }

    private void startQuery() {
        Log.i(TAG, "startQuery");
        mDialogHandler.sendMessageDelayed(mDialogHandler.obtainMessage(WAIT_CURSOR_START),
                WAIT_CURSOR_DELAY_TIME);

        initMatrixCursor();
        goOnQuery();
    }

    private void goOnQuery() {
        Log.i(TAG, "goOnQuery");

        int end;
        for (end = mQueryTimes;  end < SPEED_DIAL_MAX + 1 && TextUtils.isEmpty(mPrefNumState[end]); ++end) {
            Log.v(TAG, "log for empry block, index = " + end);
            populateMatrixCursorEmpty(this, mMatrixCursor, end, "");
        }

        Log.i(TAG, "goOnQuery, end = " + end);
        if (end > SPEED_DIAL_MAX) {
            mSimpleCursorAdapter.changeCursor(mMatrixCursor);
            mSimpleCursorAdapter.notifyDataSetChanged();

            processQueue();
            updatePreferences();

            mDialogHandler.removeMessages(WAIT_CURSOR_START);
            dismissProgressIndication();
        } else {

            QueryInfo info = new QueryInfo();
            mQueryTimes = end;
            info.mQueryIndex = mQueryTimes;

            Log.i(TAG, "goOnQuery(), startQuery at mQueryTimes = " + mQueryTimes);
            if (DEBUG) {
                LogUtils.printSensitiveInfo(TAG, "goOnQuery(), number : "
                                                      + mPrefNumState[mQueryTimes]);
            }
            Uri uri = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, Uri
                    .encode(mPrefNumState[mQueryTimes]));
            mQueryHandler.startQuery(QUERY_TOKEN, info, uri, QUERY_PROJECTION, null, null, null);
        }

    }

    private void populateMatrixCursorEmpty(Context cnx, MatrixCursor cursor, int id, String number) {
        Log.i(TAG, "populateMatrixCursorEmpty, id = " + id);
        if (TextUtils.isEmpty(number)) {
            cursor.addRow(new String[]{
                String.valueOf(id),
                cnx.getResources().getString(R.string.add_speed_dial), "", "", "0", "-1"});
        } else {
            cursor.addRow(new String[]{
                String.valueOf(id),
                number, "", number, "0", "-1"});
        }
    }

    //Need update lable && simcard icon
    private void populateMatrixCursorRow(int row, Cursor cursor) {
        cursor.moveToFirst();
        String name = cursor.getString(QUERY_DISPLAY_NAME_INDEX);
        int type = cursor.getInt(QUERY_LABEL_INDEX);
        String label = "";
        if (type == 0) {
            label = cursor.getString(QUERY_CUSTOM_LABEL_INDEX);
        } else {
            label = (String) CommonDataKinds.Phone.getTypeLabel(getResources(), type, null);
        }
        String number = cursor.getString(QUERY_NUMBER_INDEX);
        long photoId = cursor.getLong(QUERY_PHOTO_ID_INDEX);
        int simId = -1;
        if (!cursor.isNull(QUERY_INDICATE_PHONE_SIM_INDEX)) {
            simId = cursor.getInt(QUERY_INDICATE_PHONE_SIM_INDEX);
        }

        if (DEBUG) {
            Log.i(TAG, "populateMatrixCursorRow(), label = " + label
                               + " photoId:" + photoId + "simId: " + simId);
            LogUtils.printSensitiveInfo(TAG, " number : " + number);
        }

        if (simId > 0) {
            //photoId = SimContactPhotoUtils.getSimContactPhotoId(simId, false);
        }

        if (TextUtils.isEmpty(number)) {
            populateMatrixCursorEmpty(this, mMatrixCursor, row, "");
            mPrefNumState[row] = mPref.getString(String.valueOf(row), "");
            updateSpeedDial(row, "", TYPE_NUMBER_NORMAL);
            return;
        }
        mMatrixCursor.addRow(new String[] {
                String.valueOf(row), name, label,
                number, String.valueOf(photoId), String.valueOf(simId)});
    }


    public void onClick(View v) {
        if (v.getId() == R.id.sd_remove) {
            for(int i = 0;  i < mListView.getCount(); i++) {
                if (mListView.getChildAt(i) == v.getParent()) {
                    confirmRemovePosition(i + mListView.getFirstVisiblePosition());
                    return;
                }
            }
        } else if (v.getId() == R.id.contacts) {
            if (this.checkSelfPermission(READ_CONTACTS) != PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "onClick NO READ_CONTACTS requestPermission");
                requestPermissions(new String[] {READ_CONTACTS}, 0);
                return;
            }

            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);

            ComponentName component = new ComponentName("com.android.contacts",
                                                "com.android.contacts.activities.ContactSelectionActivity");
            intent.setComponent(component);
            intent.setType(Phone.CONTENT_ITEM_TYPE);
            intent.putExtra("isCallableUri", true);
            startActivityForResult(intent, REQUEST_CODE_PICK_CONTACT);

            dismissDialog(SPEED_DIAL_DIALOG_ADD);
            Log.d(TAG, "[startActivityForResult], mAddPosition = " + mAddPosition);
        }
    }

    public void onClick(DialogInterface dialog, int which) {
        if (which == DialogInterface.BUTTON_POSITIVE) {
            EditText editText = (EditText) ((AlertDialog) dialog).findViewById(R.id.number);
            final String number = editText.getText().toString();
            if (TextUtils.isEmpty(number)) {
                return;
            }

            if (-1 == findKeyByNumber(number)) {
                mPrefNumState[mAddPosition + 1] = number;

                SharedPreferences.Editor editor = mPref.edit();
                editor.putString(String.valueOf(mAddPosition + 1), number);
                editor.commit();

                updateSpeedDial(mAddPosition + 1, number, TYPE_NUMBER_NORMAL);

                startQuery();

                enQueueItem(mAddPosition);
                Log.i(TAG, "[onClick], mAddPosition =" + mAddPosition);
            } else {
                Toast.makeText(this, getString(R.string.reselect_number), Toast.LENGTH_LONG).show();
            }
        }
    }

    public void onShow(DialogInterface dialog) {
        EditText editText = (EditText) ((AlertDialog) dialog).findViewById(R.id.number);
        if (!TextUtils.isEmpty(mPrefNumState[mAddPosition + 1])) {
            editText.setText(mPrefNumState[mAddPosition + 1]);
            editText.setSelection(mPrefNumState[mAddPosition + 1].length());
        }

        editText.setKeyListener(InputKeyListener.getInstance());
        ImageView imageView = (ImageView) ((AlertDialog) dialog).findViewById(R.id.contacts);
        imageView.setOnClickListener(this);
    }

    public void onDismiss(DialogInterface dialog) {
        Log.i(TAG, "[onDismiss]");
        if (dialog == null) {
            Log.i(TAG, "[onDismiss] dialog is null, just return");
            return;
        }

        EditText editText = (EditText) ((AlertDialog) dialog).findViewById(R.id.number);
        if (editText!= null) {
            editText.setText("");
        }
    }

    public void confirmRemovePosition(int position) {
        if (position < 1 || position > 9) {
            Log.i(TAG, "position out of bound, do nothing");
            return;
        }

        Cursor c = (Cursor) mSimpleCursorAdapter.getItem(position);
        if (c == null) {
            Log.d(TAG, "[confirmRemovePosition] the cursor for the position is null");
            return;
        }

        mRemovePosition =  position;
        String name = c.getString(BIND_DISPLAY_NAME_INDEX);
        String label = c.getString(BIND_LABEL_INDEX);
        String message;
        if (TextUtils.isEmpty(label)) {
            message = getString(R.string.remove_sd_confirm_2, name, String.valueOf(position + 1));
        } else {
            message = getString(R.string.remove_sd_confirm_1, name, label, String
                    .valueOf(position + 1));
        }
        Log.d(TAG, "confirmRemovePosition(), message= " + message);
        if (mRemoveConfirmDialog == null) {
            mRemoveConfirmDialog = new AlertDialog.Builder(this).setCancelable(true)
                    .setOnCancelListener(new DialogInterface.OnCancelListener() {
                        public void onCancel(DialogInterface arg0) {
                            // TODO Auto-generated method stub
                            mRemovePosition = -1;
                            mRemoveConfirmDialog = null;
                        }
                    }).setTitle(R.string.remove_speed_dial).setIcon(
                            android.R.drawable.ic_dialog_alert).setMessage(message)
                    .setPositiveButton(R.string.remove_speed_dial,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    actuallyRemove();
                                    mRemovePosition = -1;
                                    mRemoveConfirmDialog = null;
                                }
                            }).setNegativeButton(android.R.string.cancel,
                            new DialogInterface.OnClickListener() {

                                public void onClick(DialogInterface dialog, int which) {
                                    mRemovePosition = -1;
                                    mRemoveConfirmDialog = null;
                                }
                            }).create();
        }
        mRemoveConfirmDialog.show();
    }

    private void updatePreferences() {
        SharedPreferences.Editor editor = mPref.edit();
        for (int i = SPEED_DIAL_MIN; i < SPEED_DIAL_MAX + 1; ++i) {
            editor.putString(String.valueOf(i), mPrefNumState[i]);
        }
        editor.apply();
    }

    private void actuallyRemove() {
        mPrefNumState[mRemovePosition + 1] = "";
        SharedPreferences.Editor editor = mPref.edit();
        editor.putString(String.valueOf(mRemovePosition + 1), mPrefNumState[mRemovePosition + 1]);
        editor.apply();

        startQuery();

        updateSpeedDial(mRemovePosition + 1, "", TYPE_NUMBER_NORMAL);
    }

    @Override
    protected void onRestoreInstanceState(Bundle state) {
        Log.i(TAG, "onRestoreInstanceState");
        super.onRestoreInstanceState(state);
        mAddPosition = state.getInt("add_position", -1);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        Log.i(TAG, "onSaveInstanceState");
        if (mAddPosition != -1) {
            outState.putInt("add_position", mAddPosition);
        }
        super.onSaveInstanceState(outState);
    }

    private void enQueueItem(int index) {
        Log.i(TAG, "enQueueItem(), index = " + index);
        mToastQueue.add(new QueueItem(index));
    }

    private void processQueue() {
        if (mToastQueue != null) {
            for (QueueItem item : mToastQueue) {
                Log.i(TAG, "processQueue, item index = " + item.index);
                item.run();
            }
            mToastQueue.clear();
        }
    }

    private Handler mDialogHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.i(TAG, "handleMessage msg==== " + msg.what);

            switch (msg.what) {

                case WAIT_CURSOR_START:
                    Log.i(TAG, "start WAIT_CURSOR_START showProgressIndication.");
                    showProgressIndication();
                    break;

                default:
                    break;
            }
        }
    };

    private void showProgressIndication() {
        Log.i(TAG, "loading contacts... ");
        if (mActivityDestroyed) {
            Log.i(TAG, "showProgressIndication(),the master Activity is destroyed!");
            return;
        }

        dismissProgressIndication(); // Clean up any prior progress indication

        mProgressDialog = new ProgressDialog(this);
        mProgressDialog.setMessage(this.getResources().getString(R.string.contact_list_loading));
        mProgressDialog.setIndeterminate(true);
        mProgressDialog.setCancelable(false);
        mProgressDialog.show();
    }

    private void dismissProgressIndication() {
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            try {
                mProgressDialog.dismiss(); // safe even if already dismissed
            } catch (Exception e) {
                Log.i(TAG, "dismiss exception: " + e);
            }
            mProgressDialog = null;
        }
    }


    private class SpeedDialQueryHandler extends AsyncQueryHandler{
        SpeedDialQueryHandler(Context context) {
            super(context.getContentResolver());
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            Log.d(TAG, "onQueryComplete(), cursor = " + cursor);

            if (cookie instanceof QueryInfo) {
                int index = ((QueryInfo) cookie).mQueryIndex;
                Log.d(TAG, "onQueryComplete(), index = " + index + ", mQueryTimes = " + mQueryTimes);
                if (index != mQueryTimes) {
                    if (cursor != null) {
                        cursor.close();
                    }
                    return;
                }
            }


            if (mQueryTimes < SPEED_DIAL_MAX + 1 && cursor != null && cursor.getCount() > 0) {
                populateMatrixCursorRow(mQueryTimes, cursor);
            } else if (mQueryTimes < SPEED_DIAL_MAX + 1){
                populateMatrixCursorEmpty(SpeedDialActivity.this, mMatrixCursor, mQueryTimes, mPrefNumState[mQueryTimes]);
            }

            if (cursor != null) {
                cursor.close();
            }

            if (mQueryTimes < SPEED_DIAL_MAX) {
                mQueryTimes++;
                goOnQuery();
            } else {
                mSimpleCursorAdapter.changeCursor(mMatrixCursor);
                mSimpleCursorAdapter.notifyDataSetChanged();

                processQueue();
                updatePreferences();

                mDialogHandler.removeMessages(WAIT_CURSOR_START);
                dismissProgressIndication();
            }
        }
    }

    private void updateSpeedDial(int id, String number, int type) {
        SpeedDialArgs args = new SpeedDialArgs(SpeedDialActivity.this, id, number, type, mUri);
        new UpdateSpeedDialTask().execute(args);
    }

    private class SpeedDialArgs {
        public SpeedDialArgs(Context context, int id, String number, int type, Uri uri) {
            this.mContext = context;
            this.mIndex = id;
            this.mNumber = number;
            this.mUri = uri;
            this.mNumberType = type;
        }
        public Uri mUri;
        public Context mContext;
        public int mIndex;
        public String mNumber;
        public int mNumberType;
    }

    private class UpdateSpeedDialTask extends AsyncTask<SpeedDialArgs, Void, Void> {
        @Override
        protected Void doInBackground(SpeedDialArgs... argList) {
            int count = argList.length;
            for (int i = 0; i < count; i ++) {
                SpeedDialArgs arg = argList[i];
                Context context = arg.mContext;
                ContentValues value = new ContentValues();
                value.put("number", arg.mNumber);
                value.put("type", arg.mNumberType);
                context.getContentResolver().update(arg.mUri, value, "_id" + " = " + arg.mIndex, null);
                Log.i(TAG, "UpdateSpeedDialTask(), doInBackground");
            }
            return null;
        }
    }

    private static class InputKeyListener extends DialerKeyListener {
        private static InputKeyListener sKeyListener;
        public static final char[] CHARACTERS = new char[] { '0', '1', '2',
            '3', '4', '5', '6', '7', '8', '9', '+', '*', '#',',',';'};

        @Override
        protected char[] getAcceptedChars() {
            return CHARACTERS;
        }
        public static InputKeyListener getInstance() {
            if (sKeyListener == null) {
                sKeyListener = new InputKeyListener();
            }
            return sKeyListener;
        }
    }

    private class QueueItem {
        private int index;
        private Runnable runnable;
        QueueItem(int id) {
            this.index = id;
            this.initialize();
        }

        private void initialize() {
            this.runnable = new Runnable() {
                @Override
                public void run() {

                    mMatrixCursor.moveToPosition(QueueItem.this.index);
                    if (QueueItem.this.index < SPEED_DIAL_MIN - 1) {
                        Log.i(TAG, "Toast index is invalid, just return");
                        return;
                    }

                    CharSequence name = mMatrixCursor.getString(BIND_DISPLAY_NAME_INDEX);
                    CharSequence label = mMatrixCursor.getString(BIND_LABEL_INDEX);

                    CharSequence fullInfo;
                    if (TextUtils.isEmpty(label)) {
                        fullInfo = getString(R.string.speed_dial_added2, name, String.valueOf(QueueItem.this.index + 1));
                    } else {
                        fullInfo = getString(R.string.speed_dial_added, name, label, String.valueOf(QueueItem.this.index + 1));
                    }
                    Toast.makeText(SpeedDialActivity.this, fullInfo, Toast.LENGTH_LONG).show();
                }
            };
        }

        public void run() {
            this.runnable.run();
        }
    }

    private class QueryInfo {
        int mQueryIndex;
    }
/*
    private class SimReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            Log.i(TAG, "[SimReceiver.onReceive]action is " + action);
            refreshUi();
        }

        private void refreshUi() {
            mRefreshHandler.sendEmptyMessage(0);
        }

        public void register() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(ACTION_PHB_LOAD_FINISHED);
            mContext.registerReceiver(this, filter);
        }

        public void unregister() {
            mContext.unregisterReceiver(this);
        }

        public SimReceiver(Context context) {
            mContext = context;
        }

        private Context mContext;
        private Handler mRefreshHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                Log.d(TAG, "[SimReceiver.handleMessage] update UI due to SIMService finished");
                initMatrixCursor();
                goOnQuery();
            }
        };
    }
*/
}
