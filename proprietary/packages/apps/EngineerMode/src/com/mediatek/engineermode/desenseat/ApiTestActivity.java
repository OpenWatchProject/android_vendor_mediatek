package com.mediatek.engineermode.desenseat;


import android.app.Activity;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.text.Html;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.HashMap;
import java.util.List;

/**
 * Test item API test interface.
 *
 */
public class ApiTestActivity extends Activity {

    protected static final String TAG = "DesenseAT/APITest";
    private ListView mApiTestList;
    private ApiTestAdapter mAdapter;
    private HashMap<TestItem, Boolean> mTestResults;
    private Button mStartButton;
    private List<TestItem> mTestItems;
    private FunctionTask mTask;

    private static final CharSequence NOT_SUPPORT =
            Html.fromHtml("<font color='#FFFF00'>NOT_SUPPORT</font>");
    private static final CharSequence PASS = Html.fromHtml("<font color='#00FF00'>PASS</font>");
    private static final CharSequence FAIL = Html.fromHtml("<font color='#FF0000'>FAIL</font>");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.desense_at_api_test);

        mTestItems = TestItem.getTestItems();

        mAdapter = new ApiTestAdapter(this);
        mApiTestList = (ListView) findViewById(R.id.desense_at_api_test_list);
        mApiTestList.setAdapter(mAdapter);
        mTestResults = new HashMap<TestItem, Boolean>();
        updateListView();
        mStartButton = (Button) findViewById(R.id.api_test_start);
        mStartButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                mTestResults.clear();
                mAdapter.notifyDataSetChanged();
                // start test
                mTask = new FunctionTask();
                mTask.execute();
                mStartButton.setEnabled(false);
            }
        });
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        TestItem.updateContext(this);
    }


    private void updateListView() {
        mAdapter.clear();
        mAdapter.addAll(mTestItems);
        mAdapter.notifyDataSetChanged();
    }

    /**
     * Adapter for ListView.
     */
    private class ApiTestAdapter extends ArrayAdapter<TestItem> {
        /**
         * Default constructor.
         *
         * @param activity
         *              the context
         */
        ApiTestAdapter(Context activity) {
            super(activity, 0);
        }

        /**
         * ViewHolder.
         */
        private class ViewHolder {
            public TextView label;
            public TextView result;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            LayoutInflater inflater = ApiTestActivity.this.getLayoutInflater();
            if (convertView == null) {
                convertView = inflater.inflate(R.layout.desense_at_api_test_result_entry, null);
                holder = new ViewHolder();
                holder.label = (TextView) convertView.findViewById(R.id.column1);
                holder.result = (TextView) convertView.findViewById(R.id.column2);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            TestItem testItem = getItem(position);
            Boolean r = mTestResults.get(testItem);
            holder.label.setText(testItem.toString());

            if (r == null) {
                holder.result.setText("-");
            } else {
                holder.result.setText(r ? PASS : FAIL);
            }

            return convertView;
        }
    }

    /**
     * Task to run test items.
     */
    private class FunctionTask extends AsyncTask<Void, Void, Boolean> {
        private TestCondition mTestCondition = null;
        @Override
        protected Boolean doInBackground(Void... params) {
            mTestCondition.setCondition(ApiTestActivity.this);
            for (TestItem i : mTestItems) {
                // run the test item
                Elog.d(TAG, "run " + i.toString());
                boolean result = i.doApiTest();
                mTestResults.put(i, result);
                runOnUiThread(new Runnable() {
                    public void run() {
                        mAdapter.notifyDataSetChanged();
                    }
                });
            }
            mTestCondition.resetCondition(ApiTestActivity.this);
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            mStartButton.setEnabled(true);

        }

        @Override
        protected void onCancelled(Boolean result) {
            mStartButton.setEnabled(true);

        }

        @Override
        protected void onPreExecute() {
            // TODO Auto-generated method stub
            super.onPreExecute();
            mTestCondition = new TestCondition();
            mTestCondition.init(ApiTestActivity.this);
        }
    }

}
