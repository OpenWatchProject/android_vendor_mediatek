/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/**
 * Copyright (c) 2009, Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mms.ui;

import android.app.ActionBar;
import android.app.ListActivity;
import android.app.SearchManager;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Typeface;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.SearchRecentSuggestions;
import android.provider.Telephony;
import android.text.SpannableString;
import android.text.TextPaint;
import android.text.style.StyleSpan;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CursorAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.mms.MmsApp;
import com.android.mms.data.Contact;
import com.android.mms.R;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsLog;
import com.google.android.mms.pdu.EncodedStringValue;
import com.mediatek.android.mms.pdu.MtkPduPersister;

import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.ext.IOpSearchActivityExt;
import com.mediatek.mms.ipmessage.IIpSearchActivityExt;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.wappush.ui.WPMessageActivity;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/***
 * Presents a List of search results.  Each item in the list represents a thread which
 * matches.  The item contains the contact (or phone number) as the "title" and a
 * snippet of what matches, below.  The snippet is taken from the most recent part of
 * the conversation that has a match.  Each match within the visible portion of the
 * snippet is highlighted.
 */

public class SearchActivity extends ListActivity {
    private AsyncQueryHandler mQueryHandler;

    // Track which TextView's show which Contact objects so that we can update
    // appropriately when the Contact gets fully loaded.
    private HashMap<Contact, TextView> mContactMap = new HashMap<Contact, TextView>();


    /*
     * Subclass of TextView which displays a snippet of text which matches the full text and
     * highlights the matches within the snippet.
     */
    private static final String WP_TAG = "Mms/WapPush";
    private static final String TAG = "Mms/SearchActivity";
    private String searchString;

    private boolean mIsQueryComplete = true;
    private static boolean sNeedRequery = false;

    private IOpSearchActivityExt mOpSearchActivityExt = null;
    /// M: ADD For OP01: @{
    private IIpSearchActivityExt mIpSearchActivity;

    private TextView mTvEmpty;
    /// M: Add runnable for start query so that can remove call backs. @{
    private static final int TIME_WAIT_QUERY = 200;
    /// M: Mms draft save too slow, if draft does not save successful, show toast.
    private static boolean sWaitSaveDraft = false;
    /// M: Whether query from onContentChanged or not.
    private boolean mIsContentChanged = false;

    private Runnable mRunStartQuery = new Runnable() {
        public void run() {
            startQuery();
        }
    };
    private void startQuery() {
        MmsLog.d(TAG, "startQuery");
        // don't pass a projection since the search uri ignores it
        Uri uri = mIpSearchActivity.startQuery(searchString);
        if (uri == null) {
            uri = Telephony.MmsSms.SEARCH_URI.buildUpon()
                    .appendQueryParameter("pattern", searchString).build();
        }
        mTvEmpty.setText(getString(R.string.refreshing));
        mQueryHandler.startQuery(0, null, uri, null, null, null, null);
    }
    /// @}

    public static class TextViewSnippet extends TextView {
        private static String sEllipsis = "\u2026";

        private static int sTypefaceHighlight = Typeface.BOLD;

        private String mFullText;
        private String mTargetString;
        private Pattern mPattern;

        public TextViewSnippet(Context context, AttributeSet attrs) {
            super(context, attrs);
        }

        public TextViewSnippet(Context context) {
            super(context);
        }

        public TextViewSnippet(Context context, AttributeSet attrs, int defStyle) {
            super(context, attrs, defStyle);
        }

        /**
         * We have to know our width before we can compute the snippet string.  Do that
         * here and then defer to super for whatever work is normally done.
         */
        @Override
        protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
            String fullTextLower = mFullText.toLowerCase();
            String targetStringLower = mTargetString.toLowerCase();

            int startPos = 0;
            int searchStringLength = targetStringLower.length();
            int bodyLength = fullTextLower.length();

            Matcher m = mPattern.matcher(mFullText);
            if (m.find(0)) {
                startPos = m.start();
            }

            TextPaint tp = getPaint();

            float searchStringWidth = tp.measureText(mTargetString);
            float textFieldWidth = getWidth();
            MmsLog.d(TAG, "onLayout startPos = " + startPos
                    + " searchStringWidth = " + searchStringWidth
                    + " textFieldWidth = " + textFieldWidth);

            /// M: google jb.mr1 patch, Modify to take Ellipsis for avoiding JE
            /// assume we'll need one on both ends @{
            float ellipsisWidth = tp.measureText(sEllipsis);
            textFieldWidth -= (2F * ellipsisWidth);
            /// @}
            String snippetString = null;
            /// M: add "=".
            if (searchStringWidth >= textFieldWidth) {
                /// M: Code analyze 006, For fix bug ALPS00280615, The tips mms
                // has stopped show and JE happen after clicking the longer
                // search suggestion. @{
                try {
                     snippetString = mFullText.substring(startPos, startPos + searchStringLength);
                } catch (Exception e) {
                     MmsLog.e(TAG, " StringIndexOutOfBoundsException ");
                     e.printStackTrace();
                     /// M: for search je.
                     snippetString = mFullText;
                }
                /// @}
            } else {
                int offset = -1;
                int start = -1;
                int end = -1;
                /* TODO: this code could be made more efficient by only measuring the additional
                 * characters as we widen the string rather than measuring the whole new
                 * string each time.
                 */
                while (true) {
                    offset += 1;

                    int newstart = Math.max(0, startPos - offset);
                    int newend = Math.min(bodyLength, startPos + searchStringLength + offset);

                    if (newstart == start && newend == end) {
                        // if we couldn't expand out any further then we're done
                        break;
                    }
                    start = newstart;
                    end = newend;

                    // pull the candidate string out of the full text rather than body
                    // because body has been toLower()'ed
                    String candidate = mFullText.substring(start, end);
                    if (tp.measureText(candidate) > textFieldWidth) {
                        // if the newly computed width would exceed our bounds then we're done
                        // do not use this "candidate"
                        break;
                    }

                    snippetString = String.format(
                            "%s%s%s",
                            start == 0 ? "" : sEllipsis,
                            candidate,
                            end == bodyLength ? "" : sEllipsis);
                }
            }
            if (snippetString == null) {
               if (textFieldWidth >= mFullText.length()) {
                   snippetString = mFullText;
               } else {
                   snippetString = mFullText.substring(0, (int) textFieldWidth);
               }
            }
            SpannableString spannable = new SpannableString(snippetString);
            int start = 0;

            m = mPattern.matcher(snippetString);
            while (m.find(start)) {
                MmsLog.w(TAG, "onLayout(): start = " + start + ", m.end() = " + m.end());
                if (start == m.end()) {
                    break;
                }
                spannable.setSpan(new StyleSpan(sTypefaceHighlight), m.start(), m.end(), 0);
                start = m.end();
            }
            setText(spannable);
            // do this after the call to setText() above
            super.onLayout(changed, left, top, right, bottom);
        }

        public void setText(String fullText, String target) {
            // Use a regular expression to locate the target string
            // within the full text.  The target string must be
            // found as a word start so we use \b which matches
            // word boundaries.
            String patternString = Pattern.quote(target);
            mPattern = Pattern.compile(patternString, Pattern.CASE_INSENSITIVE);

            mFullText = fullText;
            mTargetString = target;
            requestLayout();
        }
    }

    private Handler mHandler = new Handler();
    Contact.UpdateListener mContactListener = new Contact.UpdateListener() {
        public void onUpdate(final Contact updated) {
            MmsLog.dpi(TAG, "SearchActivity#onUpdate, contact updated: " + updated);
            mHandler.post(new Runnable() {
                public void run() {
                    TextView tv = mContactMap.get(updated);
                    if (tv != null) {
                        tv.setText(updated.getNameAndNumber());
                    }
                }
            });
        }
    };

    @Override
    public void onStop() {
        super.onStop();
        MmsLog.d(TAG, "onStop");

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        Contact.removeListener(mContactListener);
    }

    private long getThreadId(long sourceId, long which) {
        Uri.Builder b = Uri.parse("content://mms-sms/messageIdToThread").buildUpon();
        b = b.appendQueryParameter("row_id", String.valueOf(sourceId));
        b = b.appendQueryParameter("table_to_use", String.valueOf(which));
        String s = b.build().toString();

        Cursor c = getContentResolver().query(
                Uri.parse(s),
                null,
                null,
                null,
                null);
        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    return c.getLong(c.getColumnIndex("thread_id"));
                }
            } finally {
                c.close();
            }
        }
        return -1;
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        MmsLog.d(TAG, "onCreate");

        /// M: Avoid runtime permission JE @{
        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            MmsLog.d(TAG, "onCreate request runtime permissions!!");
            return;
        }
        /// @}

        mOpSearchActivityExt = OpMessageUtils.getOpMessagePlugin().getOpSearchActivityExt();
        mIpSearchActivity = IpMessageUtils.getIpMessagePlugin(this).getIpSearchActivity();
        mOpSearchActivityExt.onCreate(this, getIntent());

        /// M: Code analyze 003, For fix bug ALPS00243326, Search for different
        // words, it still display the previous result. Query new string if
        // Search activity is re-launched. @{
        sNeedRequery = true;
        searchString = getSearchString();
        /// @}

        // If we're being launched with a source_id then just go to that particular thread.
        // Work around the fact that suggestions can only launch the search activity, not some
        // arbitrary activity (such as ComposeMessageActivity).
        final Uri u = getIntent().getData();
        if (u != null && u.getQueryParameter("source_id") != null) {
            /// M: Code analyze 003, For fix bug ALPS00243326, Search for different
            // words, it still display the previous result. Query new string if
            // Search activity is re-launched. @{
            gotoComposeMessageActivity(u);
            /// @}
            return;
        }

        setContentView(R.layout.search_activity);
        ContentResolver cr = getContentResolver();

        final ListView listView = getListView();
        listView.setItemsCanFocus(true);
        listView.setFocusable(true);
        listView.setClickable(true);
        mTvEmpty = (TextView) findViewById(android.R.id.empty);
        // tv.setText(getString(R.string.menu_search) + "...");
        // I considered something like "searching..." but typically it will
        // flash on the screen briefly which I found to be more distracting
        // than beneficial.
        // This gets updated when the query completes.
        setTitle("");

        // When the query completes cons up a new adapter and set our list adapter to that.
        mQueryHandler = new AsyncQueryHandler(cr) {
            protected void onQueryComplete(int token, Object cookie, Cursor c) {
                /// M: Code analyze 002, For fix bug ALPS00120575, Messages
                // does not support the storage by folder. Add the cmcc dir mode
                // code. @{
                if (mIsContentChanged) {
                    sWaitSaveDraft = false;
                    mIsContentChanged = false;
                }
                mIsQueryComplete = true;
                /// @}

                int cursorCount = (c == null) ? 0 : c.getCount();
                MmsLog.d(TAG, "cursorCount =: " + cursorCount);
                CharSequence title = mOpSearchActivityExt.onQueryComplete(c);
                if (title == null) {
                    title = getResources().getQuantityString(R.plurals.search_results_title,
                            cursorCount, cursorCount, searchString);
                }
                setTitle(title);
                if (c == null) {
                    return;
                }
                if (cursorCount == 0) {
                    mTvEmpty.setText(getString(R.string.search_empty));
                }
                // Note that we're telling the CursorAdapter not to do auto-requeries. If we
                // want to dynamically respond to changes in the search results,
                // we'll have have to add a setOnDataSetChangedListener().
                /// M: Do not new CursorAdapter every time to solve cursor leak. @{
                if (mAdapter == null) {
                    mAdapter = new SearchResultCursorAdapter(SearchActivity.this, c, false);
                    setListAdapter(mAdapter);
                } else {
                    mAdapter.changeCursor(c);
                }
                /// @}

                // ListView seems to want to reject the setFocusable until such time
                // as the list is not empty.  Set it here and request focus.  Without
                // this the arrow keys (and trackball) fail to move the selection.
                listView.setFocusable(true);
                listView.setFocusableInTouchMode(true);
                listView.requestFocus();

                // Remember the query if there are actual results
                if (cursorCount > 0) {
                    SearchRecentSuggestions recent =
                        ((MmsApp) getApplication()).getRecentSuggestions();
                    if (recent != null) {
                        recent.saveRecentQuery(searchString,
                                getString(R.string.search_history, cursorCount, searchString));
                    }
                }
            }
        };

        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        sWaitSaveDraft = false;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                // The user clicked on the Messaging icon in the action bar. Take them back from
                // wherever they came from
                finish();
                return true;
        }
        return false;
    }

    /// M: Code analyze 003, For fix bug ALPS00243326, Search for different
    // words, it still display the previous result. Query new string if
    // Search activity is re-launched. @{
    @Override
    protected void onNewIntent(Intent intent) {
        setIntent(intent);

        searchString = getSearchString();
        /** M: If we're being launched with a source_id then just go to that particular thread.
         * Work around the fact that suggestions can only launch the search activity, not some
         * arbitrary activity (such as ComposeMessageActivity).
         */
        final Uri u = getIntent().getData();
        if (u != null && u.getQueryParameter("source_id") != null) {
            gotoComposeMessageActivity(u);
            return;
        }
        // M: fix bug ALPS00351620
        sNeedRequery = true;
        super.onNewIntent(intent);
    }
    /// @}

    @Override
    public void onResume() {
        super.onResume();
        MmsLog.d(TAG, "onResume");
        // M: fix bug ALPS00351620
        if (mIsQueryComplete && sNeedRequery) {
            sNeedRequery = false;
            mIsQueryComplete = false;

            mQueryHandler.removeCallbacks(mRunStartQuery);
            if (!sWaitSaveDraft) {
                startQuery();
            }
        }

    }

    public static void setNeedRequery() {
         sNeedRequery = true;
    }

    /// M: Code analyze 003, For fix bug ALPS00243326, Search for different
    // words, it still display the previous result. Query new string if
    // Search activity is re-launched. @{
    private void gotoComposeMessageActivity(final Uri u) {
        Thread t = new Thread(new Runnable() {
            public void run() {
                try {
                    long sourceId = Long.parseLong(u.getQueryParameter("source_id"));
                    long whichTable = Long.parseLong(u.getQueryParameter("which_table"));
                    long threadId = getThreadId(sourceId, whichTable);

                    final Intent onClickIntent = new Intent(SearchActivity.this,
                            ComposeMessageActivity.class);
                    onClickIntent.putExtra(" searchString = ", searchString);
                    onClickIntent.putExtra("select_id", sourceId);
                    onClickIntent.putExtra("thread_id", threadId);
                    startActivity(onClickIntent);
                    finish();
                    return;
                } catch (NumberFormatException ex) {
                   MmsLog.e(TAG, "OK, we do not have a thread id so continue", ex);
                }
            }
        }, "Search thread");
        t.start();
    }

    private String getSearchString() {
        String searchStringParameter = mOpSearchActivityExt.getSearchString();
        if (searchStringParameter == null) {
            searchStringParameter = getIntent().getStringExtra(SearchManager.QUERY);
            if (searchStringParameter == null) {
                searchStringParameter = getIntent().getStringExtra("intent_extra_data_key"
                        /*SearchManager.SUGGEST_COLUMN_INTENT_EXTRA_DATA*/);
            }
        }
        return searchStringParameter !=
            null ? searchStringParameter.trim() : searchStringParameter;
    }
    /// @}

    @Override
    protected void onStart() {
        super.onStart();

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        /// M: fix ALPS01524752, update contact in another way, should not blocking UI thread.
        Contact.addListener(mContactListener);
        Contact.invalidateCache();
        mContactMap.clear();
        setNeedRequery();
    }

    /// M: Define class SearchResultCursorAdapter to solve cursor leak. @{
    private SearchResultCursorAdapter mAdapter;
    private class  SearchResultCursorAdapter extends CursorAdapter {

        public SearchResultCursorAdapter(Context context, Cursor c, boolean autoRequery) {
            super(context, c, autoRequery);
        }

        @Override
        public void bindView(View view, Context context, Cursor cursor) {
            final int threadIdPos = cursor.getColumnIndex("thread_id");
            final int addressPos  = cursor.getColumnIndex("address");
            final int bodyPos     = cursor.getColumnIndex("body");
            final int rowidPos    = cursor.getColumnIndex("_id");
            // M: fix bug ALPS00351620
            final int typeIndex   = cursor.getColumnIndex("index_text");
            // M: fix bug ALPS00378385
            final int charsetPos  = cursor.getColumnIndex("charset");
            /// M: fix bug ALPS00417470
            final int m_typePos = cursor.getColumnIndex("m_type");
            /// M: Code analyze 004, For fix bug ALPS00246438, the draft
            // shows abnormal in search results In folder mode, the draft
            // should be displayed in compose activity. @{
            final int msgTypePos;
            final int msgBoxPos;
            if (cursor.getColumnIndex("msg_type") > 0) {
                msgTypePos = cursor.getColumnIndex("msg_type");
            } else {
                msgTypePos = 0;
            }
            if (cursor.getColumnIndex("msg_box") > 0) {
                msgBoxPos = cursor.getColumnIndex("msg_box");
            } else {
                msgBoxPos = 0;
            }

            final TextView title = (TextView) (view.findViewById(R.id.title));
            final TextViewSnippet snippet = (TextViewSnippet) (view.findViewById(R.id.subtitle));

            /// M: fix bug ALPS378385, use given charset to avoid invaild number @{
            int addrCharset = cursor.getInt(charsetPos);
            String address = cursor.getString(addressPos);
            if (cursor.getInt(6) == 0) {
                EncodedStringValue v =
                    new EncodedStringValue(addrCharset, MtkPduPersister.getBytes(address));
                address = v.getString();
            }
            Contact contact = address != null ? Contact.get(address, false) : null;

            String titleString = contact != null ? contact.getNameAndNumber() : "";
            /// @}
            title.setText(titleString);
            if (contact != null) {
                mContactMap.put(contact, title);
            }
            /// M: Code analyze 005, For fix bug ALPS00278132,
            // Search the mms which hasn't subject or text. @{
            //if the type is mms, set the subject as title.
            String body = IpMessageUtils.formatIpMessage(
                cursor.getString(bodyPos), false, null).toString();
            if (cursor.getInt(6) == 0) {
                if (body == null || body.equals("")) {
                    body = context.getString(R.string.no_subject_view);
                } else {
                    try {
                        body = new String(body.getBytes("ISO-8859-1"), "UTF-8");
                    } catch (UnsupportedEncodingException e) {
                         MmsLog.w(TAG, "onQueryComplete UnsupportedEncodingException");
                         e.printStackTrace();
                    }
                }
            }

            snippet.setText(body, searchString);
            /// @}
            /// @}
            // if the user touches the item then launch the compose message
            // activity with some extra parameters to highlight the search
            // results and scroll to the latest part of the conversation
            // that has a match.
            final long threadId = cursor.getLong(threadIdPos);
            final long rowid = cursor.getLong(rowidPos);
            final int msgType = cursor.getInt(msgTypePos);
            final int msgBox = cursor.getInt(msgBoxPos);
            // M: fix bug ALPS00351620
            final int threadType = cursor.getInt(typeIndex);
            /// M: fix bug ALPS00417470
            final int m_type = cursor.getInt(m_typePos);
            mIpSearchActivity.onIpBindView(title, cursor);
            MmsLog.d(TAG, "onQueryComplete msgType = " + msgType + "rowid =" + rowid);

            view.setOnClickListener(new View.OnClickListener() {
                private void initializeClickIntent(Intent onClickIntent,
                        int msgType, int msgBox, long rowid) {
                    if (msgType == 1) {
                        onClickIntent.setClass(SearchActivity.this,
                                ComposeMessageActivity.class);
                    } else if (msgType == 2) {
                        if (msgBox == 3) { //draft
                              onClickIntent.setClass(SearchActivity.this,
                                      ComposeMessageActivity.class);
                          } else {
                              if (msgBox == 1 && m_type == 130) {
                                  Toast.makeText(SearchActivity.this,
                                          R.string.view_mms_notification,
                                          Toast.LENGTH_SHORT).show();
                                  return;
                              } else {
                                  final Uri MMS_URI = Uri.parse("content://mms/");
                                  onClickIntent.setClass(SearchActivity.this,
                                          MmsPlayerActivity.class);
                                  onClickIntent.setData(
                                          ContentUris.withAppendedId(MMS_URI, rowid));
                                  onClickIntent.putExtra("dirmode", true);
                              }
                          }
                    } else if (msgType == 4) {
   
                    }
                }

                public void onClick(View v) {
                    Intent onClickIntent = null;
                    if (FeatureOption.MTK_WAPPUSH_SUPPORT == true) {
                        if (threadType == 1) {
                            MmsLog.i(WP_TAG,
                                    "SearchActivity: " + "onClickIntent WPMessageActivity.");
                            onClickIntent = new Intent(SearchActivity.this,
                                    WPMessageActivity.class);
                        } else {
                            MmsLog.i(WP_TAG, "SearchActivity: "
                                    + "onClickIntent ComposeMessageActivity.");
                            onClickIntent = new Intent(SearchActivity.this,
                                    ComposeMessageActivity.class);
                        }
                    } else {
                        onClickIntent = new Intent(SearchActivity.this,
                                 ComposeMessageActivity.class);
                    }
                    if (msgBox == 3 && sWaitSaveDraft) {
                        Toast.makeText(SearchActivity.this, R.string.cannot_load_message,
                                Toast.LENGTH_SHORT).show();
                        return;
                    }
                    onClickIntent.putExtra("thread_id", threadId);
                    onClickIntent.putExtra("highlight", searchString);
                    onClickIntent.putExtra("select_id", rowid);
                    onClickIntent.putExtra("msg_type", getStringType(msgType));
                    startActivity(onClickIntent);
                }
            });
        }

        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {
            LayoutInflater inflater = LayoutInflater.from(context);
            View v = inflater.inflate(R.layout.search_item, parent, false);
            return v;
        }

        /*
         * M: Do not use onDraftChanged, because sometimes it could
         * not query out correct mms draft result when onDraftChanged
         * was called.
         */
        @Override
        protected void onContentChanged() {
            super.onContentChanged();
            MmsLog.d(TAG, "onContentChanged");
            /// M: if activity is not resumed, mIsContentChanged also should be set
            mIsContentChanged = true;
            if (!SearchActivity.this.isResumed()) {
                return;
            }
            mQueryHandler.removeCallbacks(mRunStartQuery);
            mQueryHandler.postDelayed(mRunStartQuery, TIME_WAIT_QUERY);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        if (mAdapter != null) {
            mAdapter.changeCursor(null);
        }
        if (mQueryHandler != null) {
            mQueryHandler.removeCallbacks(mRunStartQuery);
        }
    }

    public static void setWaitSaveDraft() {
        sWaitSaveDraft = true;
    }

    private String getStringType(int msgType) {
        if (mIpSearchActivity.getStringType(msgType) != null) {
            return mIpSearchActivity.getStringType(msgType);
        }
        switch (msgType) {
            case 1:
                return "sms";
            case 2:
                return "mms";
            case 3:
                return "wappush";
            case 4:
                return "cellbroadcast";
            default:
                return "";
        }
    }

}
