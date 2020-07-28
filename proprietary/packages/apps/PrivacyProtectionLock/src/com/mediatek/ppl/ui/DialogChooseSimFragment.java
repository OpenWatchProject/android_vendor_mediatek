package com.mediatek.ppl.ui;

import java.util.List;

import com.mediatek.ppl.PlatformManager;
import com.mediatek.ppl.PplSimInfo;

import com.mediatek.ppl.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

public class DialogChooseSimFragment extends DialogFragment  {

    private static final String ARG_KEY_ITEMS = "items";
    private static final String ARG_KEY_VALUES = "value";

    private List<PplSimInfo> mInfoList;
    private String[] mItemList;
    private int[] mSlots;
    private Activity mActivity;

    private boolean mSimpleMode = false;

    public static interface IChooseSim {
        void onSimSelected(int simId);
    }

    public static DialogChooseSimFragment newInstance(String[] itemList, int[] slots) {

        DialogChooseSimFragment frag = new DialogChooseSimFragment();

        Bundle args = new Bundle();
        args.putStringArray(ARG_KEY_ITEMS, itemList);
        args.putIntArray(ARG_KEY_VALUES, slots);
        frag.setArguments(args);
        return frag;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        mItemList = getArguments().getStringArray(ARG_KEY_ITEMS);
        mSlots = getArguments().getIntArray(ARG_KEY_VALUES);
        mActivity = getActivity();
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        mInfoList = PlatformManager.buildSimInfo(this.getActivity());

        if (null == mInfoList || mInfoList.size() != mItemList.length) {
            mSimpleMode = true;
            return new AlertDialog.Builder(mActivity)
                .setTitle(R.string.title_choose_sim)
                .setNegativeButton(android.R.string.cancel, null)
                .setItems(mItemList, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        ((IChooseSim) mActivity).onSimSelected(mSlots[which]);
                    }
                }).create();
        } else {
            for (int i = 0; i < mInfoList.size(); i++) {
                mInfoList.get(i).setSimId(mSlots[i]);
            }
        }
        return super.onCreateDialog(savedInstanceState);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        if (mSimpleMode) {
            return super.onCreateView(inflater, container, savedInstanceState);
        }
        getDialog().setTitle(R.string.title_choose_sim);
        //setTitle(R.string.title_choose_sim);
        View v = inflater.inflate(R.layout.choose_sim, container, false);
        ListView listView = (ListView) v.findViewById(R.id.lv_choose_sim);
        SimListAdapter adapter = new SimListAdapter(mActivity, mInfoList);
        listView.setAdapter(adapter);

        listView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view,
                    int position, long id) {
                ((IChooseSim) mActivity).onSimSelected(
                        mInfoList.get(position).getSimId());
            }
        });

        return v;
    }

    private class SimListAdapter extends BaseAdapter {

        private Context mContext;
        private List<PplSimInfo> mSimList;

        public SimListAdapter(Context context, List <PplSimInfo> data) {
            mContext = context;
            mSimList = data;
        }

        @Override
        public int getCount() {
            return mSimList.size();
        }
        @Override
        public PplSimInfo getItem(int position) {
            return mSimList.get(position);
        }
        @Override
        public long getItemId(int position) {
            return position;
        }
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            convertView = LayoutInflater.from(mContext).inflate(R.layout.choose_sim_item, null);
            ImageView image = (ImageView) convertView.findViewById(R.id.iv_sim_item);
            TextView  title = (TextView) convertView.findViewById(R.id.tv_sim_item);

            title.setText(getItem(position).getTitle());
			//image.setImageResource(getItem(position).getImage());
			image.setImageBitmap(getItem(position).getImage());
            image.setVisibility(View.VISIBLE);

            return convertView;
        }
    }
}
