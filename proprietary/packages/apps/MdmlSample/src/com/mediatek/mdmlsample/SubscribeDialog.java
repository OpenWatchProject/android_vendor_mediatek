package com.mediatek.mdmlsample;

import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.ListView;
import android.widget.SearchView;

public class SubscribeDialog implements AdapterView.OnItemClickListener,
        CompoundButton.OnCheckedChangeListener, SearchView.OnQueryTextListener {
    private static final String TAG = "SubscribeDialog";
    private String[] mItemNames;
    private boolean[] mSelections;
    private String mTitle;
    private AlertDialog mInterface;
    private Context mContext;
    private DialogInterface.OnClickListener mListener;

    private SearchView mSearchBT;
    private CheckBox mSelectAll;
    private ListView mListView;

    public SubscribeDialog(Context context, String title, String[] items,
            boolean[] selections, DialogInterface.OnClickListener listener) {
        mTitle = title;
        mItemNames = items;
        mSelections = selections;
        mContext = context;
        mListener = listener;
    }

    public AlertDialog buildDialog() {
        Builder builder = new AlertDialog.Builder(mContext);
        builder.setTitle(mTitle);
        builder.setPositiveButton("OK", mListener);
        LayoutInflater inflater = LayoutInflater.from(mContext);
        final View view = inflater.inflate(R.layout.select_dialog, null);
        builder.setView(view);

        mListView = (ListView) view.findViewById(R.id.listView);
        mListView.setAdapter(new SubAdapter(inflater));
        mListView.setOnItemClickListener(this);
        mListView.setTextFilterEnabled(true);

        mSelectAll = (CheckBox) view.findViewById(R.id.selectAll);
        mSelectAll.setOnCheckedChangeListener(this);

        mSearchBT = (SearchView) view.findViewById(R.id.searchButton);
        mSearchBT.setOnQueryTextListener(this);
        mInterface = builder.create();
        return mInterface;
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        ListView listView = (ListView) parent;
        SubAdapter adapter = (SubAdapter) listView.getAdapter();
        mSelections[(int) id] = !mSelections[(int) id];
        adapter.notifyDataSetChanged();
        Log.d(TAG, "onItemClick, mSelections[" + id + "] changed to "
                + mSelections[(int) id]);
    }

    @Override
    public void onCheckedChanged(CompoundButton button, boolean checked) {
        for (int i = 0; i < mSelections.length; ++i) {
            mSelections[i] = checked;
        }
        ((SubAdapter) mListView.getAdapter()).notifyDataSetChanged();
    }

    @Override
    public boolean onQueryTextChange(String text) {
        if (text == null || text.isEmpty()) {
            mListView.clearTextFilter();
        } else {
            mListView.setFilterText(text.trim());
        }
        return false;
    }

    @Override
    public boolean onQueryTextSubmit(String arg0) {
        return false;
    }

    class SubAdapter extends BaseAdapter implements Filterable {
        private LayoutInflater mInflater;
        private String[] mShowItems;

        public SubAdapter(LayoutInflater inflater) {
            mInflater = inflater;
            mShowItems = mItemNames;
        }

        @Override
        public int getCount() {
            return mShowItems == null ? 0 : mShowItems.length;
        }

        @Override
        public String getItem(int position) {
            return mShowItems == null ? null : mShowItems[position];
        }

        @Override
        public long getItemId(int position) {
            String itemName = getItem(position);
            for (int i = 0; i < mItemNames.length; ++i) {
                if (mItemNames[i].equals(itemName)) {
                    return i;
                }
            }
            return -1;
        }

        @Override
        public View getView(int position, View view, ViewGroup viewGroup) {
            if (view == null) {
                view = mInflater.inflate(R.layout.select_item, null);
            }
            CheckBox checkbox = (CheckBox) view.findViewById(R.id.checkBox);
            checkbox.setText(getItem(position));
            checkbox.setChecked(mSelections[(int) getItemId(position)]);

            return view;
        }

        @Override
        public Filter getFilter() {
            return new Filter() {

                @Override
                protected FilterResults performFiltering(
                        CharSequence charSequence) {
                    Log.d(TAG, "performFiltering : " + charSequence);
                    FilterResults results = new FilterResults();
                    String[] visibleItems;
                    if (TextUtils.isEmpty(charSequence)) {
                        visibleItems = mItemNames;
                    } else {
                        List<String> items = new ArrayList<String>();
                        for (String item : mItemNames) {
                            if (item.contains(charSequence)) {
                                items.add(item);
                            }
                        }
                        visibleItems = new String[items.size()];
                        items.toArray(visibleItems);
                    }

                    results.values = visibleItems;
                    results.count = visibleItems.length;
                    return results;
                }

                @Override
                protected void publishResults(CharSequence charSequence,
                        FilterResults results) {
                    Log.d(TAG, "publishResults : " + results.count);
                    mShowItems = (String[]) results.values;
                    if (results.count > 0) {
                        notifyDataSetChanged();
                    } else {
                        notifyDataSetInvalidated();
                    }
                }

            };
        }
    }
}
