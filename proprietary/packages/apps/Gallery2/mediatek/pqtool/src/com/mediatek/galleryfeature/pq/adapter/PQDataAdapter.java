package com.mediatek.galleryfeature.pq.adapter;

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import com.mediatek.gallerybasic.util.Log;
import com.mediatek.galleryfeature.pq.PictureQualityActivity;
import com.mediatek.galleryfeature.pq.R;
import com.mediatek.galleryfeature.pq.Representation;
import com.mediatek.galleryfeature.pq.dcfilter.DCFilter;
import com.mediatek.galleryfeature.pq.filter.Filter;
import com.mediatek.galleryfeature.pq.filter.FilterInterface;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * Create PQ menu by Data and set menu item height.
 */
public class PQDataAdapter extends BaseAdapter {
    private static final String TAG = "MtkGallery2/PQDataAdapter";
    private ArrayList<FilterInterface> mData;
    private LayoutInflater mInflater;
    private HashMap<ViewHolder, Representation> mAllPresentation =
            new HashMap<ViewHolder, Representation>();
    private Context mContext;
    private ListView mListView;
    private String mUri;
    private FilterInterface mFilter;
    private PictureQualityActivity mActivity;

    /**
     * PQ view holder.
     */
    public final class ViewHolder {
        public RelativeLayout layout;
        public TextView left;
        public SeekBar seekbar;
        public TextView blow;
        public TextView right;
    }

    /**
     * Constructor.
     * @param context
     *            getWindowManager by the context.
     * @param uri
     *            the image uri.If uri == null means this is an video.
     */
    public PQDataAdapter(PictureQualityActivity activity, Context context, String uri) {
        mActivity = activity;
        mUri = uri;
        this.mInflater = LayoutInflater.from(context);
        if (uri != null) {
            mFilter = new Filter();
        } else {
            mFilter = new DCFilter();
        }

        mData = mFilter.getFilterList();
        mContext = context;
    }

    public int getCount() {
        return mData.size();
    }

    public void setListView(ListView listView) {
        mListView = listView;
    }

    @Override
    public Object getItem(int position) {
        return null;
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder = null;
        RelativeLayout layout = null;
        if (convertView == null) {
            holder = new ViewHolder();
            convertView = mInflater.inflate(R.layout.m_pq_seekbar, null);
            holder.left = (TextView) convertView.findViewById(R.id.m_textViewMinValue);
            holder.seekbar = (SeekBar) convertView.findViewById(R.id.m_seekbar);
            holder.blow = (TextView) convertView.findViewById(R.id.m_textViewCurrentIndex);
            holder.right = (TextView) convertView.findViewById(R.id.m_textViewMaxValue);
            holder.layout = (RelativeLayout) convertView.findViewById(R.id.m_listitem);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        Representation presentaion = mAllPresentation.get(holder);
        if (presentaion == null) {
            presentaion = new Representation(mUri);
            mAllPresentation.put(holder, presentaion);
        }
        presentaion.init(holder, mData.get(position));
        setItemHeight(holder, mData.size());
        return convertView;
    }

    private void setItemHeight(ViewHolder holder, int count) {
        Log.d(TAG, "<setItemHeight> setItemHeight");
        int sceenHeigh = mListView.getHeight();
        if (sceenHeigh == 0) {
            sceenHeigh =
                    ((Activity) mContext).getWindowManager().getDefaultDisplay().getHeight()
                            - mActivity.getActionBarHeight();
        }
        int defaultItemH = mActivity.getDefaultItemHeight();
        int itemHeight;
        if (count * defaultItemH < sceenHeigh) {
            itemHeight = (sceenHeigh - mActivity.getActionBarHeight()) / count;
        } else {
            itemHeight = defaultItemH - mActivity.getActionBarHeight() / count;
        }
        holder.layout.setMinimumHeight(itemHeight);
    }

    /**
     * Set all Filter as default value.
     */
    public void restoreIndex() {
        int size = mData.size();
        for (int i = 0; i < size; i++) {
            FilterInterface data = mData.get(i);
            if (data != null) {
                data.setIndex(data.getDefaultIndex());
            }
        }
    }

    /**
     * Reset state while onResume activity.
     */
    public void onResume() {
        mFilter.onResume();
    }

    /**
     * Reset state while onDestroy activity.
     */
    public void onDestroy() {
        mFilter.onDestroy();
    }
}
