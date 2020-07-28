package com.mediatek.engineermode.desenseat;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;


import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Band item selection.
 *
 */
public class RfBandConfigActivity extends Activity {

    private static final String TAG = "DesenseAT/RfBandConfig";
    public static final String GSM_ITEM = "gsm_item";
    public static final String FDD_ITEM = "fdd_item";
    public static final String TDD_ITEM = "tdd_item";
    public static final String LTE_ITEM = "lte_item";
    public static final String CDMA_ITEM = "cdma_item";

    private BandAdapter mGsmAdapter;
    private BandAdapter mFddAdapter;
    private BandAdapter mTddAdapter;
    private BandAdapter mLteAdapter;
    private BandAdapter mCdmaAdapter;

    private ListView mGsmList;
    private ListView mFddList;
    private ListView mTddList;
    private ListView mLteList;
    private ListView mCdmaList;
    private static BandItem sItemClicked;

    private Intent mIntentBandDetail;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.desense_at_rf_config);
        mIntentBandDetail = new Intent(this, BandDetailActivity.class);

        updateListView();

    }
    private void updateEditItem() {

        if (sItemClicked == null) {
            return;
        }

        BandItem.BandType type = sItemClicked.getType();
        if (type == BandItem.BandType.BAND_GSM) {
            Elog.d(TAG, "edit gsm");
            mGsmAdapter.notifyDataSetChanged();
        } else if (type == BandItem.BandType.BAND_WCDMA) {
            Elog.d(TAG, "edit wcdma");
            mFddAdapter.notifyDataSetChanged();
        } else if (type == BandItem.BandType.BAND_TD) {
            Elog.d(TAG, "edit td");
            mTddAdapter.notifyDataSetChanged();
        }  else if ((type == BandItem.BandType.BAND_LTE_FDD)
                || (type == BandItem.BandType.BAND_LTE_TDD)) {
            Elog.d(TAG, "edit lte");
            mLteAdapter.notifyDataSetChanged();
        } else if (type == BandItem.BandType.BAND_CDMA) {
            Elog.d(TAG, "edit cdma");
            mCdmaAdapter.notifyDataSetChanged();
        }
    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();
        Elog.d(TAG, "onStart");
        updateEditItem();
    }

    private void setListViewHeightBasedOnChildren(ListView listView) {
        ListAdapter listAdapter = listView.getAdapter();
        if (listAdapter == null) {
            return;
        }

        int totalHeight = 0;
        // get the ListView count
        int count = listAdapter.getCount();
        for (int i = 0; i < count; i++) {
            View listItem = listAdapter.getView(i, null, listView);
            // measure the child view
            listItem.measure(0, 0);
            // calculate the total height of items
            totalHeight += listItem.getMeasuredHeight();
        }

        ViewGroup.LayoutParams params = listView.getLayoutParams();
        // get divider height for all items and add the total height
        params.height = totalHeight + (listView.getDividerHeight()
                * (listAdapter.getCount() - 1));
        listView.setLayoutParams(params);
    }

    private void updateListView() {
        mGsmAdapter = new BandAdapter(this);
        mGsmList = (ListView) findViewById(R.id.desense_at_gsm_list);
        mGsmList.setAdapter(mGsmAdapter);
        mGsmAdapter.setListView(mGsmList);
        mGsmAdapter.clear();
        mGsmAdapter.addAll(DesenseAtActivity.sGsmItems);
        setListViewHeightBasedOnChildren(mGsmList);
        mGsmList.setOnItemClickListener(mCommonListener);

//        for (BandItem item:DesenseAtActivity.sGsmItems) {
//            Elog.d(TAG, "item content: " + item.getBandName() + " " + item.getSummary());
//        }
        mGsmAdapter.notifyDataSetChanged();

        if (DesenseAtActivity.sFddItems != null) {
            mFddAdapter = new BandAdapter(this);
            mFddList = (ListView) findViewById(R.id.desense_at_fdd_list);
            mFddList.setAdapter(mFddAdapter);
            mFddAdapter.setListView(mFddList);
            mFddAdapter.clear();
            mFddAdapter.addAll(DesenseAtActivity.sFddItems);
            setListViewHeightBasedOnChildren(mFddList);
            mFddList.setOnItemClickListener(mCommonListener);
            mFddAdapter.notifyDataSetChanged();

        } else {
            View view = findViewById(R.id.fddCategory);
            if (view != null) {
                view.setVisibility(View.GONE);
            }
        }

        if (DesenseAtActivity.sTddItems != null) {
            mTddAdapter = new BandAdapter(this);
            mTddList = (ListView) findViewById(R.id.desense_at_tdd_list);
            mTddList.setAdapter(mTddAdapter);
            mTddAdapter.setListView(mTddList);
            mTddAdapter.clear();
            mTddAdapter.addAll(DesenseAtActivity.sTddItems);
            setListViewHeightBasedOnChildren(mTddList);
            mTddList.setOnItemClickListener(mCommonListener);
            mTddAdapter.notifyDataSetChanged();
        } else {
            View view = findViewById(R.id.tddCategory);
            if (view != null) {
                view.setVisibility(View.GONE);
            }
        }

        if (DesenseAtActivity.sLteItems != null) {
            mLteAdapter = new BandAdapter(this);
            mLteList = (ListView) findViewById(R.id.desense_at_lte_list);
            mLteList.setAdapter(mLteAdapter);
            mLteAdapter.setListView(mLteList);
            mLteAdapter.clear();
            mLteAdapter.addAll(DesenseAtActivity.sLteItems);
            setListViewHeightBasedOnChildren(mLteList);
            mLteList.setOnItemClickListener(mCommonListener);
            mLteAdapter.notifyDataSetChanged();
        } else {
            View view = findViewById(R.id.lteCategory);
            if (view != null) {
                view.setVisibility(View.GONE);
            }
        }

        if (DesenseAtActivity.sCdmaItems != null) {
            mCdmaAdapter = new BandAdapter(this);
            mCdmaList = (ListView) findViewById(R.id.desense_at_cdma_list);
            mCdmaList.setAdapter(mCdmaAdapter);
            mCdmaAdapter.setListView(mCdmaList);
            mCdmaAdapter.clear();
            mCdmaAdapter.addAll(DesenseAtActivity.sCdmaItems);
            setListViewHeightBasedOnChildren(mCdmaList);
            mCdmaList.setOnItemClickListener(mCommonListener);
            mCdmaAdapter.notifyDataSetChanged();
        } else {
            View view = findViewById(R.id.cdmaCategory);
            if (view != null) {
                view.setVisibility(View.GONE);
            }
        }

    }

    /**
     * Adapter for listview.
     *
     */
    private class BandAdapter extends ArrayAdapter<BandItem> {

        private ListView mListView;
        /**
         * Default constructor.
         *
         * @param activity
         *              the context
         */
        BandAdapter(Context activity) {
            super(activity, 0);
        }

        public void setListView(ListView listview) {
            mListView = listview;
        }

        /**
         * ViewHolder.
         */
        private class ViewHolder {
            public CheckBox checkbox;
            public TextView title;
            public TextView summary;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            int visiblePosition = mListView.getFirstVisiblePosition();
//            Elog.d(TAG, "visiblePosition: " + visiblePosition + " " + position + " position");
            if (position - visiblePosition < 0) {
                return null;
            }
            ViewHolder holder;
            BandItem testItem = getItem(position);
//            if (testItem.isSelected()) {
//                Elog.d(TAG, "testItem: " + position + " " + testItem.getBandName() + " "
//                        + testItem.getSummary() + " " + testItem.isSelected());
//            }
            LayoutInflater inflater = RfBandConfigActivity.this.getLayoutInflater();
            if (convertView == null) {
                convertView = inflater.inflate(R.layout.desense_at_band_entry, null);
                holder = new ViewHolder();
                holder.title = (TextView) convertView.findViewById(R.id.title);
                holder.summary = (TextView) convertView.findViewById(R.id.summary);
                holder.checkbox = (CheckBox) convertView.findViewById(R.id.band_checkbox);
                final int pos = position;
                holder.checkbox.setOnCheckedChangeListener(new CheckBox.OnCheckedChangeListener() {
                    @Override
                    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                        getItem(pos).setSelected(isChecked);
//                        Elog.d(TAG, "getItem: position =  " +
//                                    pos + " item:" + getItem(pos).toString());
                    }
                    });
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }


            holder.title.setText(testItem.getBandName());
            holder.summary.setText(testItem.getSummary());
            holder.checkbox.setChecked(testItem.isSelected());

            return convertView;
        }
    }


    private AdapterView.OnItemClickListener mCommonListener =
            new AdapterView.OnItemClickListener() {
        public void onItemClick(AdapterView parent, View view, int position, long id) {
            sItemClicked = (BandItem) parent.getItemAtPosition(position);
            Elog.d(TAG, "clicked item is : " + sItemClicked.toString());
            startActivity(mIntentBandDetail);
        }
    };

    /**
     * @return get item clicked
     */
    public static BandItem getClickedItem() {
        Elog.d(TAG, "return item is : " + sItemClicked.toString());
        return sItemClicked;
    }

}
