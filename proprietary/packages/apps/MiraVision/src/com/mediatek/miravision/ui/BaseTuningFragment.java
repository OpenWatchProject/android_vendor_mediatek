package com.mediatek.miravision.ui;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class BaseTuningFragment extends Fragment implements OnClickListener,
        OnSeekBarChangeListener {

    int mTuningTitle;
    int index;
    ImageView mImageOrignal;
    ImageView mImageBasicAfter;
    ImageView mImageAdvAfter;
    View mLineView;
    TextView mOrignalText;
    TextView mAfterText;
    TextView mSeekBarText;
    SeekBar mSeekBar;
    ImageButton mLastButton;
    ImageButton mNextButton;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {

        View rootView = inflater.inflate(R.layout.color_tuning_view, container, false);

        mImageOrignal = (ImageView) rootView.findViewById(R.id.image_view_original);
        mImageBasicAfter = (ImageView) rootView.findViewById(R.id.image_view_basic_after);
        mImageAdvAfter = (ImageView) rootView.findViewById(R.id.image_view_adv_after);
        mLineView = rootView.findViewById(R.id.line_view);
        mOrignalText = (TextView) rootView.findViewById(R.id.text_original);
        mAfterText = (TextView) rootView.findViewById(R.id.text_after);
        mSeekBarText = (TextView) rootView.findViewById(R.id.tuning_set_text);
        mSeekBar = (SeekBar) rootView.findViewById(R.id.tuning_set_seekbar);
        mLastButton = (ImageButton) rootView.findViewById(R.id.last_button);
        mNextButton = (ImageButton) rootView.findViewById(R.id.next_button);
        mSeekBar.setOnSeekBarChangeListener(this);
        mLastButton.setOnClickListener(this);
        mNextButton.setOnClickListener(this);

        return rootView;
    }

    @Override
    public void onClick(View v) {
        if (v == mLastButton) {
            index--;
        } else if (v == mNextButton) {
            index++;
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
    }
}
