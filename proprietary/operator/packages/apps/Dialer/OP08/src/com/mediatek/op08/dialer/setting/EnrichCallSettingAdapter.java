package com.mediatek.op08.dialer.setting;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.RecyclerView.ViewHolder;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.util.SparseBooleanArray;
import android.util.SparseArray;
import android.util.Log;

import com.mediatek.op08.dialer.R;

class EnrichCallSettingAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> implements
            SettingEntryViewHolder.SubSwitchChangeListener {
    private static final String TAG = "EnrichCallSettingAdapter";

    private static final int HEADER_VIEW_TYPE = 1;
    private static final int ENTRY_VIEW_TYPE = 3;
    private static final int ITEM_COUNT = 5;

    private SparseArray<String> mContent = new SparseArray<String>(ITEM_COUNT);
    private SparseBooleanArray mStatus = new SparseBooleanArray(ITEM_COUNT);
    private Context mContext;

    EnrichCallSettingAdapter(Context context) {
        Log.d(TAG, "construct");
        mContext = context;
  }

  @Override
  public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
    LayoutInflater inflater = LayoutInflater.from(parent.getContext());
    switch (viewType) {
      case HEADER_VIEW_TYPE:
        return new SettingHeaderViewHolder(
            inflater.inflate(R.layout.header_list_item_layout, parent, false), mContext);
      case ENTRY_VIEW_TYPE:
        return new SettingEntryViewHolder(
            inflater.inflate(R.layout.list_item_layout, parent, false), this, mContext);
      default:
        return null;
    }
  }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        Log.d(TAG, "onBindViewHolder position: " + position);
        if (position == 0) { // Header
            ((SettingHeaderViewHolder) holder).setData(
                    mStatus.valueAt(SettingConstant.TYPE_MASTER_SWITCH));
        } else {
            SettingEntryViewHolder viewHolder = (SettingEntryViewHolder) holder;
            viewHolder.setEntryData((position), mStatus.valueAt(position),
                    mContent.valueAt(position),
                    mStatus.valueAt(SettingConstant.TYPE_MASTER_SWITCH));
        }
    }

    @Override
    public int getItemViewType(int position) {
        if (position == 0) { // Header
            return HEADER_VIEW_TYPE;
        } else {
            return ENTRY_VIEW_TYPE;
        }
    }

    @Override
    public int getItemCount() {
        return ITEM_COUNT;
    }

    @Override
    public void onSubSwitchChanged(int tyle, boolean isChecked){
        Log.d(TAG, "onSubSwitchChanged tyle: " + tyle + " isChecked:" + isChecked);
        if (mStatus.valueAt(SettingConstant.TYPE_MASTER_SWITCH) != isChecked) {
            mStatus.put(tyle, isChecked);

            boolean masterSwitchStatus = mStatus.valueAt(SettingConstant.TYPE_MASTER_SWITCH);
            if (masterSwitchStatus != isChecked) {
                Log.d(TAG, "onSubSwitchChanged need update master status");
                if (isChecked) {
                    Log.d(TAG, "onSubSwitchChanged master if false, sub is ture, set master true");
                    SharedPreferencesHelper.setBooleanToPref(mContext,
                            SettingConstant.KEY_MASTER_SWITCH_STATUS, true);
                    refreshData();
                    return;
                } else {
                    Log.d(TAG, "onSubSwitchChanged master if true, sub is false, update master status");
                    boolean ret = false;
                    for (int index = 1; index < ITEM_COUNT; index++) {
                        if (mStatus.valueAt(index)) {
                            ret = true;
                            break;
                        }
                    }

                    if (!ret) {
                        SharedPreferencesHelper.setBooleanToPref(mContext,
                                SettingConstant.KEY_MASTER_SWITCH_STATUS, false);
                        refreshData();
                    }
                }
            }
        }
    }

    //callback when sharedPrefence data change.
    void refreshData() {
        Log.d(TAG, "refreshData");
        SharedPreferencesHelper.readStatusFromPref(mContext, mStatus);
        SharedPreferencesHelper.readContentsFromPref(mContext, mContent);
        notifyDataSetChanged();
    }
}
