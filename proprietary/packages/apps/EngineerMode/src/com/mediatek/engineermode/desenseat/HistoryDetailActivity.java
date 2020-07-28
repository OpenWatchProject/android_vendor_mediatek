package com.mediatek.engineermode.desenseat;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;


import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Activity for show detail history data.
 *
 */
public class HistoryDetailActivity extends Activity implements OnItemClickListener {

    protected static final String TAG = "DesenseAT/HistoryDetail";
    private static final int DIALOG_LOADING = 1;
    private static final int DIALOG_CURVE = 2;
    private static final int LOADING_DONE = 1;
    private String mStrPath;
    private String mStrSummary;
    private ListView mResultList;
    private MyAdapter mAdapter;
    private List<HistoryData> mDataItems = new ArrayList<HistoryData>();

    private static final CharSequence COR_X = "Time";
    private static final CharSequence COR_Y = "CNR";
    private static final int SV_GPS = 0;
    private static final int SV_GLONASS_L = 1;
    private static final int SV_GLONASS_M = 2;
    private static final int SV_GLONASS_H = 3;
    private static final int SV_BEIDOU = 4;
    private static final int SV_TYPE_NUMBER = 5;
    private TextView mInfo;
    /**
     * ViewHolder.
     */
    private class HistoryData {
        private String mTitle;
        private int mResult;
        private List<String> mValues = new ArrayList<String>();
        private String mSummary;
        private ArrayList<float[]> mData;

        HistoryData(String title, int i) {
            mTitle = title;
            mResult = i;
        }

        public void addValueString(String strValue) {
            mValues.add(strValue);
        }

        public void setSummary(String summary) {
            mSummary = summary;
        }

        public void setData(ArrayList<float[]> data) {
            mData = data;
        }

        public String getTitle() {
            return mTitle;
        }
        public int getResult() {
            return mResult;
        }
        public String getSummary() {
            return mSummary;
        }
        public List<String> getValues() {
            return mValues;
        }
        public ArrayList<float[]> getData() {
            return mData;
        }

        @Override
        public String toString() {
            // TODO Auto-generated method stub
            String str = mTitle + " " + mResult + " " + mSummary + "\r\n";
            for (String strValue:mValues) {
                str = str + " " + strValue;
            }
            str += "\r\n";
            for (float[] data:mData) {
                if (data == null) {
                    continue;
                }
                str += "Data :";
                for (float value: data) {
                    str = str + " " + value;
                }
                str += "\r\n";
            }
            return str;
        }

    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        String fileName = intent.getStringExtra(HistoryListActivity.FILE_NAME_KEY);
        setTitle(fileName);
        mStrPath = Util.getTestFilePath(this, Util.HISTORY_PATH) + "/"
                + fileName;
        Elog.d(TAG, "mStrPath = " + mStrPath);
        setContentView(R.layout.desense_at_history_data);
        mInfo = (TextView) findViewById(R.id.history_data_info);
        mAdapter = new MyAdapter(this);
        mResultList = (ListView) findViewById(R.id.desense_at_test_result);
        mResultList.setAdapter(mAdapter);
        mResultList.setOnItemClickListener(this);
        loadingData();
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        removeDialog(DIALOG_CURVE);
    }


    private void loadingData() {
        showDialog(DIALOG_LOADING);
        new LoadingTask().execute();

    }


    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        switch (id) {
        case DIALOG_LOADING:
            ProgressDialog dlgLoading = new ProgressDialog(this);
            dlgLoading.setMessage(getString(R.string.desense_at_history_loading));
            dlgLoading.setCancelable(false);
            dlgLoading.setIndeterminate(true);
            return dlgLoading;
        case DIALOG_CURVE:
            return createCurveDialog(bundle);
        default:
            return null;
        }
    }

    @SuppressLint("InflateParams")
    private Dialog createCurveDialog(Bundle bundle) {
        int position = bundle.getInt("position", 0);
        if ((mAdapter == null) || (mAdapter.getCount() <= position)) {
            return null;
        }
        View view = getLayoutInflater().inflate(R.layout.desense_at_curve, null);
        TextView title = (TextView) view.findViewById(R.id.desense_at_curve_title);
        TextView info = (TextView) view.findViewById(R.id.desense_at_curve_info);
        TextView x = (TextView) view.findViewById(R.id.desense_at_curve_x_label);
        TextView y = (TextView) view.findViewById(R.id.desense_at_curve_y_label);
        CurveView curve = (CurveView) view.findViewById(R.id.desense_at_curve);
        HistoryData data = mDataItems.get(position);

        title.setText(data.getTitle());
        info.setText(data.getSummary());
        x.setText(COR_X);
        y.setText(COR_Y);
        curve.setDataList(data.getData());
        AlertDialog dialog = new AlertDialog.Builder(this).setView(view).create();
        return dialog;
    }
    /**
     * Background operation for attach.
     *
     */
    private class LoadingTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub
            Elog.d(TAG, "LoadingTask start");
            mDataItems.clear();
            try {
                BufferedReader in = new BufferedReader(new FileReader(mStrPath));
                if (in != null) {
                    mStrSummary = in.readLine();
                }
                String strResult = new String();
                while ((strResult = in.readLine()) != null) {
                    String[] info = strResult.split("--");
                    HistoryData historyData = new HistoryData(info[0], Integer.parseInt(info[1]));
                    strResult = in.readLine();
                    if (strResult == null) {
                        break;
                    }
                    String[] values = strResult.split(" ");
                    for (String vl:values) {
                        historyData.addValueString(vl);
                    }
                    strResult = in.readLine();
                    if (strResult == null) {
                        break;
                    }
                    historyData.setSummary(strResult);
                    strResult = in.readLine();
                    if (strResult == null) {
                        break;
                    }
                    String[] dataGroup = strResult.split(";");
                    ArrayList<float[]> data = new ArrayList<float[]>();
                    for (String dataStr:dataGroup) {
                        if (Util.INVALID_DATA.equals(dataStr)) {
                            data.add(null);
                        } else {
                            String[] dataString = dataStr.split(" ");
                            int length = dataString.length;
                            float[] dataFloat = new float[length];
                            for (int i = 0; i < length; i++) {
                                dataFloat[i] = Float.parseFloat(dataString[i]);
                            }
                            data.add(dataFloat);
                        }

                    }
                    historyData.setData(data);
                    Elog.d(TAG, "historyData = " + historyData.toString());
                    mDataItems.add(historyData);
                }
                in.close();

            } catch (IOException ex) {
                ex.printStackTrace();
            } catch (NumberFormatException ex) {
                ex.printStackTrace();
            }
            runOnUiThread(new Runnable() {
                public void run() {
                    mInfo.setText(mStrSummary);
                    mAdapter.clear();
                    mAdapter.addAll(mDataItems);
                    mAdapter.notifyDataSetChanged();
                }
            });
            mHandler.sendEmptyMessage(LOADING_DONE);
            return true;
        }
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case LOADING_DONE:
                removeDialog(DIALOG_LOADING);
                break;

            default:
                break;
            }
        }
    };


    /**
     * Adapter for ListView.
     */
    private class MyAdapter extends ArrayAdapter<HistoryData> {
        /**
         * Default constructor.
         *
         * @param activity
         *              the context
         */
        MyAdapter(Context activity) {
            super(activity, 0);
        }

        /**
         * ViewHolder.
         */
        private class ViewHolder {
            public TextView label;
            public TextView result;
            public TextView[] values;
            @Override
            public String toString() {
                return "ViewHolder [label=" + label + ", result=" + result
                        + ", values=" + Arrays.toString(values) + "]";
            }

        }

        @SuppressLint("InflateParams")
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            LayoutInflater inflater = HistoryDetailActivity.this.getLayoutInflater();
            if (convertView == null) {
                convertView = inflater.inflate(R.layout.desense_at_result_entry, null);
                holder = new ViewHolder();
                holder.label = (TextView) convertView.findViewById(R.id.column1);
                holder.result = (TextView) convertView.findViewById(R.id.column2);
                holder.values = new TextView[SV_TYPE_NUMBER];
                holder.values[SV_GPS] = (TextView) convertView.findViewById(R.id.column3);
                holder.values[SV_GLONASS_L] = (TextView) convertView.findViewById(R.id.column4);
                holder.values[SV_GLONASS_M] = (TextView) convertView.findViewById(R.id.column5);
                holder.values[SV_GLONASS_H] = (TextView) convertView.findViewById(R.id.column6);
                holder.values[SV_BEIDOU] = (TextView) convertView.findViewById(R.id.column7);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            HistoryData data = mDataItems.get(position);

            holder.label.setText(data.getTitle());

            switch (data.getResult()) {
            case TestResult.NONE:
                holder.result.setText(Util.TESTING);
                break;
            case TestResult.PASS:
                holder.result.setText(Util.PASS);
                break;
            case TestResult.FAIL:
                holder.result.setText(Util.FAIL);
                break;
            case TestResult.CONNECTION_FAIL:
                holder.result.setText(Util.CONN_FAIL);
                break;
            case TestResult.CNR_FAIL:
                holder.result.setText(Util.CNR_FAIL);
                break;
            default:
                break;
            }
            // show desense values
            for (int i = 0; i < holder.values.length; i++) {
                holder.values[i].setText("-");
            }
            List<String> values = data.getValues();
            int size = values.size();
            for (int i = 0; i < size; i++) {
                String desense = values.get(i);
                if (!"null".equalsIgnoreCase(desense)) {
                    holder.values[i].setText(desense);
                }
            }
            return convertView;
        }
    }


    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        // TODO Auto-generated method stub
        Bundle bundle = new Bundle();
        bundle.putInt("position", position);
        removeDialog(DIALOG_CURVE);
        showDialog(DIALOG_CURVE, bundle);
    }
}
