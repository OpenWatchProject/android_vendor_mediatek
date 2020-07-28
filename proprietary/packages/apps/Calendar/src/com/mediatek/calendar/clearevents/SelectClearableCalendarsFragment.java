/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.calendar.clearevents;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Fragment;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.os.Bundle;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Calendars;
import android.provider.CalendarContract.Events;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import com.android.calendar.AllInOneActivity;
import com.android.calendar.AsyncQueryService;
import com.android.calendar.R;
import com.android.calendar.Utils;
import com.android.calendar.selectcalendars.SelectCalendarsSimpleAdapter;
import com.mediatek.calendar.LogUtil;

import java.util.ArrayList;

///M:#ClearAllEvents#
public class SelectClearableCalendarsFragment extends Fragment
        implements AdapterView.OnItemClickListener/*, CalendarController.EventHandler*/ {

    private static final String TAG = "Calendar";
    private static final String IS_PRIMARY = "\"primary\"";
    private static final String SELECTION = Calendars.SYNC_EVENTS + "=?";
    private static final String[] SELECTION_ARGS = new String[] {"1"};

    private static final String[] PROJECTION = new String[] {
        Calendars._ID,
        Calendars.ACCOUNT_NAME,
        Calendars.ACCOUNT_TYPE,
        Calendars.OWNER_ACCOUNT,
        Calendars.CALENDAR_DISPLAY_NAME,
        Calendars.CALENDAR_COLOR,
        Calendars.VISIBLE,
        Calendars.SYNC_EVENTS,
        "(" + Calendars.ACCOUNT_NAME + "=" + Calendars.OWNER_ACCOUNT + ") AS " + IS_PRIMARY,
    };
    private static int sDeleteToken;
    private /*static*/ int sQueryToken;
    private static int mCalendarItemLayout = R.layout.mini_calendar_item;

    private View mView;
    private ListView mList;
    private SelectCalendarsSimpleAdapter mAdapter;
    private Activity mContext;
    private AsyncQueryService mService;
    private Cursor mCursor;
    private Toast mToast;

    private Button mBtnDelete;
    private AlertDialog mAlertDialog;
    private ArrayList<Long> mCalendarIds = new ArrayList<Long>();

    ///M:The flag that set the account check status diable.
    private static final int FLAG_ACCOUNT_CHECK_DISABLE = 0;
    private static final int FLAG_ACCOUNT_CHECK_ENABLE = 1;
    private static final String KEY_CALENDAR_IDS = "key_calendar_ids";

    public SelectClearableCalendarsFragment() {
    }

    public SelectClearableCalendarsFragment(int itemLayout) {
        mCalendarItemLayout = itemLayout;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        mContext = activity;
        mService = new AsyncQueryService(activity) {
            @Override
            protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
                /*
                 * M: The ContentResolver.query maybe return null in some
                 * special cases, for example the case the provider process was
                 * dead.
                 */
                if (cursor == null) {
                    LogUtil.i(TAG,
                            "cursor is null, the provider process may be dead.");
                    return;
                }
                /// M:update the account check status, depend on data stored in mCalendarIds.
                Cursor newCursor = updateAccountCheckStatus(cursor);
                mAdapter.changeCursor(newCursor);
                mCursor = newCursor;
            }

            @Override
            protected void onDeleteComplete(int token, Object cookie, int result) {
                LogUtil.i(TAG, "Clear all events,onDeleteComplete.  result(delete number)=" + result);
                if (mProgressDialog != null && mProgressDialog.isShowing()) {
                    mProgressDialog.cancel();
                    LogUtil.i(TAG, "Cancel Progress dialog.");
                }

                if (mToast == null) {
                    mToast = Toast.makeText(mContext, R.string.delete_completed, Toast.LENGTH_SHORT);
                }
                mToast.show();
                /// M: need to set flag true to make sure "event_change" is sent to update UI about
                // events in month view @{
                AllInOneActivity.setClearEventsCompletedStatus(true);
                /// @}

                super.onDeleteComplete(token, cookie, result);
            }
        };
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        if (mCalendarItemLayout == R.layout.mini_calendar_item) {
            //set the item layout here before new adapter in onActivityCreated.
            LogUtil.i(TAG, " set mCalendarItemLayout to be calendar_sync_item");
            mCalendarItemLayout = R.layout.calendar_sync_item;
        }
        mView = inflater.inflate(R.layout.select_calendars_to_clear_fragment, null);
        mList = (ListView) mView.findViewById(R.id.list);

        // Hide the Calendars to Sync button on tablets for now.
        // Long terms stick it in the list of calendars
        if (Utils.getConfigBool(getActivity(), R.bool.multiple_pane_config)) {
            // Don't show dividers on tablets
            mList.setDivider(null);
            View v = mView.findViewById(R.id.manage_sync_set);
            if (v != null) {
                v.setVisibility(View.GONE);
            }
        }
        mBtnDelete = (Button) mView.findViewById(R.id.btn_ok);
        if (mBtnDelete != null) {
            mBtnDelete.setOnClickListener(mClickListener);
            mBtnDelete.setEnabled(mCalendarIds.size() > 0 ? true : false);
        }
        Button cancel = (Button) mView.findViewById(R.id.btn_cancel);
        if (cancel != null) {
            cancel.setOnClickListener(mClickListener);
        }
        return mView;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mAdapter = new SelectCalendarsSimpleAdapter(mContext, mCalendarItemLayout, null, getFragmentManager());
        mList.setAdapter(mAdapter);
        mList.setOnItemClickListener(this);
    }


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mProgressDialog = createProgressDialog();
        mCalendarIds.clear();
        if (savedInstanceState != null) {
            long[] ids = savedInstanceState.getLongArray(KEY_CALENDAR_IDS);
            if (ids != null) {
                for (int i = 0; i < ids.length; i++) {
                    mCalendarIds.add(ids[i]);
                }
                LogUtil.i(TAG, "restored calendar ids: " + mCalendarIds);
            }
        }
        sQueryToken = mService.getNextToken();
        mService.startQuery(sQueryToken, null, Calendars.CONTENT_URI, PROJECTION, SELECTION,
                SELECTION_ARGS, Calendars.ACCOUNT_NAME);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mCalendarIds != null && !mCalendarIds.isEmpty()) {
            mCalendarIds.clear();
        }
        dismissAlertDialog();

        if (mProgressDialog != null) {
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        if (mCursor != null) {
            mAdapter.changeCursor(null);
            mCursor.close();
            mCursor = null;
        }
    }

    private OnClickListener mClickListener = new OnClickListener() {
        public void onClick(View view) {
            switch (view.getId()) {
            case R.id.btn_ok:
                LogUtil.d(TAG, "Clear all events, ok");
                AlertDialog dialog = new AlertDialog.Builder(mContext)
                .setTitle(R.string.delete_label)
                .setMessage(R.string.clear_all_selected_events_title)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setNegativeButton(android.R.string.cancel, null).create();

                dialog.setButton(DialogInterface.BUTTON_POSITIVE,
                        mContext.getText(R.string.delete_certain),
                        mClearEventsDialogListener);
                dialog.show();
                mAlertDialog = dialog;
                break;
            case R.id.btn_cancel:
                LogUtil.d(TAG, "Clear all events, cancel");
                mContext.finish();
                break;
            default:
                LogUtil.e(TAG, "Unexpected view called: " + view);
                break;
            }
        }
    };

    private DialogInterface.OnClickListener mClearEventsDialogListener =
            new DialogInterface.OnClickListener() {
        public void onClick(DialogInterface dialog, int button) {
            LogUtil.d(TAG, "Clear all events, to delete.");
            dismissAlertDialog();
            sDeleteToken = mService.getNextToken();
            //mContext.finish();
            if (mProgressDialog != null) {
                mProgressDialog.show();
            }

            ///M: delete all events whose id > 0 && belong to selected Accounts
            String selection = Events._ID + ">0";
            selection = getSelection(selection);
            LogUtil.i(TAG, "Clear all events, start delete, selection=" + selection);
            mService.startDelete(sDeleteToken, null, CalendarContract.Events.CONTENT_URI, selection, null, 0);
        }
    };

    public void onItemClick(AdapterView<?> parent, View view, int position, long id)  {
        if (mAdapter == null || mAdapter.getCount() <= position) {
            return;
        }
        saveCalendarId(position);
    }

    public void saveCalendarId(int position) {
        Log.d(TAG, "Toggling calendar at " + position);
        long calId = mAdapter.getItemId(position);
        int selected = mAdapter.getVisible(position) ^ 1;

        mAdapter.setVisible(position, selected);
        if (selected != 0) {
            mCalendarIds.add(calId);
        } else {
            if (mCalendarIds.contains(calId)) {
                mCalendarIds.remove(calId);
            }
        }

        if (!mCalendarIds.isEmpty()) {
            mBtnDelete.setEnabled(true);
        } else {
            mBtnDelete.setEnabled(false);
        }
    }

    private String getSelection(String selection) {
        String tmpSelection = "";
        for (Long calId : mCalendarIds) {
            tmpSelection += " OR " + Events.CALENDAR_ID + "=" + String.valueOf(calId);
        }
        if (!TextUtils.isEmpty(tmpSelection)) {
            tmpSelection = tmpSelection.replaceFirst(" OR ", "");
            return selection + " AND (" + tmpSelection + ")";
        } else {
            return selection;
        }
    }

    private void dismissAlertDialog() {
        if (mAlertDialog != null) {
            mAlertDialog.dismiss();
        }
    }

    ProgressDialog mProgressDialog = null ;
    private ProgressDialog createProgressDialog() {
        ProgressDialog dialog = new ProgressDialog(mContext);
        dialog.setMessage(getString(R.string.wait_deleting_tip));
        dialog.setIndeterminate(true);
        dialog.setCancelable(false);
        return dialog;
    }

    public boolean isProgressDialogShowing() {
        return mProgressDialog != null ? mProgressDialog.isShowing() : false;
    }

    /**
     * M:update the account check status
     * @param cursor
     * @return
     */
     public Cursor updateAccountCheckStatus(Cursor cursor) {
         MatrixCursor newCursor = new MatrixCursor(cursor.getColumnNames());
         int numColumns = cursor.getColumnCount();
         String data[] = new String[numColumns];
         while (cursor.moveToNext()) {
             for (int i = 0; i < numColumns; i++) {
                 data[i] = cursor.getString(i);
             }

             int calIdIndex = cursor.getColumnIndex(Calendars._ID);
             int index = cursor.getColumnIndex(Calendars.VISIBLE);
             if (mCalendarIds.contains(cursor.getLong(calIdIndex))) {
                 data[index] = String.valueOf(FLAG_ACCOUNT_CHECK_ENABLE);
             } else {
                data[index] = String.valueOf(FLAG_ACCOUNT_CHECK_DISABLE);
             }
             newCursor.addRow(data);
         }
         cursor.close();
         return newCursor;
     }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        LogUtil.i(TAG, "save calendar ids:" + mCalendarIds);
        int size = mCalendarIds.size();
        long[] calendarIds = new long[size];
        for (int i = 0; i < size; i++) {
            calendarIds[i] = mCalendarIds.get(i).longValue();
        }
        outState.putLongArray(KEY_CALENDAR_IDS, calendarIds);
        super.onSaveInstanceState(outState);
    }
}
