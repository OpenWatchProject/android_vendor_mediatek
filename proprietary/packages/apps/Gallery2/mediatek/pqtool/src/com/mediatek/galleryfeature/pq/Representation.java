package com.mediatek.galleryfeature.pq;

import android.widget.SeekBar;
import android.widget.TextView;

import com.mediatek.gallerybasic.util.Log;
import com.mediatek.galleryfeature.pq.adapter.PQDataAdapter.ViewHolder;
import com.mediatek.galleryfeature.pq.filter.FilterInterface;

/**
 * Init PQ list view and start load task while tag PQ bar.
 */
public class Representation implements SeekBar.OnSeekBarChangeListener {

    private static final String TAG = "MtkGallery2/Representation";
    TextView mMinValue;
    TextView mMaxValue;
    TextView mCurrentValue;
    SeekBar mController;
    private ViewHolder mHolder;
    private FilterInterface mFilter;
    private String mUri;

    /**
     * Constructor.
     * @param uri the current bitmap uri.
     */
    public Representation(String uri) {
        mUri = uri;
    }

    /**
     * Init ViewHolder.
     * @param holder contain
     * @param filter the current PQ parameter.
     */
    public void init(ViewHolder holder, FilterInterface filter) {
        mHolder = holder;
        mFilter = filter;
        holder.left.setText(mFilter.getMinValue());
        holder.right.setText(mFilter.getMaxValue());
        holder.blow.setText(mFilter.getCurrentValue());
        holder.seekbar.setMax(mFilter.getRange() - 1);
        holder.seekbar.setProgress(Integer.parseInt(mFilter
                .getSeekbarProgressValue()));
        Log.d(TAG,
                "<init>: mFilter.getCurrentValue() = "
                        + mFilter.getCurrentValue()
                        + " mFilter.getSeekbarProgressValue() = "
                        + Integer.parseInt(mFilter.getSeekbarProgressValue())
                        + "  holder.seekbar Max=" + holder.seekbar.getMax());
        holder.seekbar.setOnSeekBarChangeListener(this);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress,
            boolean fromUser) {
        if (fromUser) {
            mFilter.setCurrentIndex(progress);
            mHolder.blow.setText(mFilter.getCurrentValue());
            if (mUri != null) {
                PresentImage.getPresentImage().loadBitmap(mUri);
            }
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        mFilter.setIndex(mFilter.getCurrentIndex());
        mHolder.blow.setText(mFilter.getCurrentValue());
    }

}
