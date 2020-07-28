package com.mediatek.contacts.aas;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.contacts.R;

import com.mediatek.contacts.aassne.Anr;
import com.mediatek.contacts.aassne.SimAasSneUtils;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.internal.telephony.phb.AlphaTag;
import com.mediatek.provider.MtkContactsContract;

import java.util.ArrayList;
import java.util.List;

public class AasTagInfoAdapter extends BaseAdapter {
    private final static String TAG = "CustomAasAdapter";
    public final static int MODE_NORMAL = 0;
    public final static int MODE_EDIT = 1;
    private int mMode = MODE_NORMAL;

    private Context mContext = null;
    private LayoutInflater mInflater = null;
    //private int mSlotId = -1;
    private int mSubId = -1;
    private ToastHelper mToastHelper = null;

    private ArrayList<TagItemInfo> mTagItemInfos = new ArrayList<TagItemInfo>();

    public AasTagInfoAdapter(Context context, int subId) {
        mContext = context;
        mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mSubId = subId;
        mToastHelper = new ToastHelper(context);
    }

    public void updateAlphaTags() {
        mTagItemInfos.clear();
        SimAasSneUtils.refreshAASList(mSubId);
        List<AlphaTag> list = SimAasSneUtils.getAAS(mSubId);
        for (AlphaTag tag : list) {
            TagItemInfo tagItemInfo = new TagItemInfo(tag);
            mTagItemInfos.add(tagItemInfo);
            Log.d(TAG, "[updateAlphaTags] getPbrIndex: " + tag.getPbrIndex() +
                    ",getRecordIndex: " + tag.getRecordIndex() + ",getAlphaTag: "
                    + Log.anonymize(tag.getAlphaTag()));
        }
        notifyDataSetChanged();
    }

    public int getMode() {
        return mMode;
    }

    public void setMode(int mode) {
        Log.d(TAG, "[setMode] mode: " + mode);
        if (mMode != mode) {
            mMode = mode;
            if (isMode(MODE_NORMAL)) {
                for (TagItemInfo tagInfo : mTagItemInfos) {
                    tagInfo.mChecked = false;
                }
            }
            notifyDataSetChanged();
        }
    }

    public boolean isMode(int mode) {
        return mMode == mode;
    }

    @Override
    public int getCount() {
        return mTagItemInfos.size();
    }

    @Override
    public TagItemInfo getItem(int position) {
        return mTagItemInfos.get(position);
    }

    public void setChecked(int position, boolean checked) {
        TagItemInfo tagInfo = getItem(position);
        tagInfo.mChecked = checked;
        notifyDataSetChanged();
    }

    public void updateChecked(int position) {
        TagItemInfo tagInfo = getItem(position);
        tagInfo.mChecked = !tagInfo.mChecked;
        notifyDataSetChanged();
    }

    public void setAllChecked(boolean checked) {
        Log.d(TAG, "[setAllChecked] checked: " + checked);
        for (TagItemInfo tagInfo : mTagItemInfos) {
            tagInfo.mChecked = checked;
        }
        notifyDataSetChanged();
    }

    public void deleteCheckedAasTag() {
        for (TagItemInfo tagInfo : mTagItemInfos) {
            if (tagInfo.mChecked) {
                boolean success = SimAasSneUtils.removeUsimAasById(mSubId,
                        tagInfo.mAlphaTag.getRecordIndex(), tagInfo.mAlphaTag.getPbrIndex());
                if (!success) {
                    String msg = mContext.getResources().getString(R.string.aas_delete_fail,
                            tagInfo.mAlphaTag.getAlphaTag());
                    mToastHelper.showToast(msg);
                    Log.d(TAG, "[deleteCheckedAasTag] delete failed:"
                            + Log.anonymize(tagInfo.mAlphaTag.getAlphaTag()));
                } else {
                    ContentResolver resolver = mContext.getContentResolver();
                    String whereClause = Data.MIMETYPE + "='" + Phone.CONTENT_ITEM_TYPE + "'"
                            + " AND " + MtkContactsContract.DataColumns.IS_ADDITIONAL_NUMBER + "=1"
                            + " AND " + Data.DATA2 + "=" + Anr.TYPE_AAS
                            + " AND " + Data.DATA3 + "=?";
                    ContentValues values = new ContentValues();
                    values.putNull(Data.DATA3);
                    final String aasIndex = Integer.toString(mSubId) + "-"
                            + tagInfo.mAlphaTag.getRecordIndex();
                    Log.d(TAG, "[deleteCheckedAasTag] aasIndex=" + aasIndex);
                    Log.sensitive(TAG, "[deleteCheckedAasTag] whereClause=" + whereClause);
                    int updatedCount = resolver.update(Data.CONTENT_URI, values,
                            whereClause, new String[]{ aasIndex });
                    Log.d(TAG, "[deleteCheckedAasTag] updatedCount=" + updatedCount);
                }
            }
        }
        updateAlphaTags();
    }

    public int getCheckedItemCount() {
        int count = 0;
        if (isMode(MODE_EDIT)) {
            for (TagItemInfo tagInfo : mTagItemInfos) {
                if (tagInfo.mChecked) {
                    count++;
                }
            }
        }
        return count;
    }

    public int[] getCheckedIndexArray() {
        int[] checkedArray = new int[getCheckedItemCount()];
        int j = 0;
        for (int i = 0; i < mTagItemInfos.size(); i++) {
            if (mTagItemInfos.get(i).mChecked) {
                checkedArray[j++] = i;
            }
        }
        return checkedArray;
    }

    public void setCheckedByIndexArray(int[] checkedArray) {
        for (int i = 0; i < checkedArray.length; i++) {
            mTagItemInfos.get(checkedArray[i]).mChecked = true;
        }
        notifyDataSetChanged();
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    public Boolean isExist(String text) {
        for (int i = 0; i < mTagItemInfos.size(); i++) {
            if (mTagItemInfos.get(i).mAlphaTag.getAlphaTag().equals(text)) {
                return true;
            }
        }
        return false;
    }

    public boolean isFull() {
        final int maxCount = PhbInfoUtils.getUsimAasCount(mSubId);
        Log.d(TAG, "[isFull] getCount: " + getCount() + ",maxCount=" + maxCount +
                ",sub: " + mSubId);
        return getCount() >= maxCount;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        /* M: [ALPS03748938] reusing convertView meets an Google issue
         * that is the CheckBox unchecked animation will pending when the first
         * time clicks the back key and will show up until next time CheckBox visible.
         * So in order to work around this issue and considering sim card
         * only most support 5 AAS which has very small effect for performance,
         * here always inflate a new item view instead of reusing convertView.*/
        convertView = mInflater.inflate(R.layout.custom_aas_item, null);
        TextView tagView = (TextView) convertView.findViewById(R.id.aas_item_tag);
        ImageView editView = (ImageView) convertView.findViewById(R.id.aas_edit);
        CheckBox checkBox = (CheckBox) convertView.findViewById(R.id.aas_item_check);
        TagItemInfo tag = getItem(position);
        tagView.setText(tag.mAlphaTag.getAlphaTag());
        if (isMode(MODE_NORMAL)) {
            editView.setVisibility(View.VISIBLE);
            checkBox.setChecked(tag.mChecked);
            checkBox.setVisibility(View.GONE);
        } else {
            editView.setVisibility(View.GONE);
            checkBox.setVisibility(View.VISIBLE);
            checkBox.setChecked(tag.mChecked);
        }
        return convertView;
    }

    public static class TagItemInfo {
        AlphaTag mAlphaTag = null;
        boolean mChecked = false;

        public TagItemInfo(AlphaTag tag) {
            mAlphaTag = tag;
        }
    }
}
