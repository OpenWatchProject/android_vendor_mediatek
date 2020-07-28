package com.mediatek.gallery3d.video;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.storage.StorageManager;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import com.android.gallery3d.R;
import com.mediatek.galleryportable.Log;
import com.mediatek.galleryportable.StorageManagerUtils;

import java.io.File;

public class ClearMotionTool extends Activity implements OnSeekBarChangeListener {
    public static final String ACTION_ClearMotionTool =
            "com.android.camera.action.ClearMotionTool";
    private static final String TAG = "VP/ClearMotionTool";
    private static String[] sExtPath = null;
    private RadioGroup mGroup;
    private final String BDR = "persist.clearMotion.fblevel.bdr";
    private final String BDR_NAME = "Fluency fine tune";
    private final String DEMOMODE = "persist.clearMotion.demoMode";
    private final static int DEFAULTVALUE = 125;
    private final static int DEFAULTVALUEOFDEMOMODE = 0;
    private final static short MAX_VALUE = 256;
    private int mRange;
    private String fblevel_nrm = null;
    private int[] mClearMotionParameters = new int[2];
    private int[] mOldClearMotionParameters = new int[2];

    private SeekBar mSeekBarSkinSat;
    private TextView mTextViewSkinSatProgress;
    private TextView mTextViewSkinSat;
    private TextView mTextViewSkinSatRange;

    private int mSkinSatRange;
    private String mStoragePath = null;
    private Context mContext = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_ACTION_BAR);
        requestWindowFeature(Window.FEATURE_ACTION_BAR_OVERLAY);
        mContext = this;
        sExtPath = StorageManagerUtils.getVolumnPaths(
                (StorageManager) this.getSystemService(Context.STORAGE_SERVICE));
        setContentView(R.layout.m_clear_motion_tool);
        getViewById();
        mRange = ClearMotionQualityJni.nativeGetFallbackRange();
        if (sExtPath != null) {
            int length = sExtPath.length;
            for (int i = 0; i < length; i++) {
                if (sExtPath[i] != null) {
                    File clearMotionCfg = new File(sExtPath[i], "SUPPORT_CLEARMOTION");
                    if (clearMotionCfg != null && clearMotionCfg.exists()) {
                        mStoragePath = sExtPath[i];
                        break;
                    }
                }
            }
        }
        setValue();
    }

    private void setValue() {

        mSeekBarSkinSat.setMax(mRange);
        mTextViewSkinSatRange.setText((mRange) + "");
        mSeekBarSkinSat.setOnSeekBarChangeListener(this);
        read();
        mOldClearMotionParameters[0] = mClearMotionParameters[0];
        mOldClearMotionParameters[1] = mClearMotionParameters[1];
        mOldBDRProgress = Integer.toString(mClearMotionParameters[0]);
        mOldDemoMode = Integer.toString(mClearMotionParameters[1]);
        Log.d(TAG, " <setValue> mOldBDRProgress==" + mOldBDRProgress + " mOldDemoMode="
                + mOldDemoMode);
        try {
            if (mOldBDRProgress != null && mOldDemoMode != null) {
                mBDRProgress = mOldBDRProgress;
                mSeekBarSkinSat.setProgress(Integer.parseInt(mOldBDRProgress));
                mDemoMode = mOldDemoMode;
                if (mOldDemoMode.equals(sDemooff)) {
                    RadioButton radioButton = (RadioButton) findViewById(R.id.demooff);
                    radioButton.setChecked(true);
                } else if (mOldDemoMode.equals(sVertical)) {
                    RadioButton radioButton = (RadioButton) findViewById(R.id.vertical);
                    radioButton.setChecked(true);
                } else if (mOldDemoMode.equals(sHorizontal)) {
                    RadioButton radioButton = (RadioButton) findViewById(R.id.horizontal);
                    radioButton.setChecked(true);
                }

            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
        }

        mTextViewSkinSatProgress.setText(BDR_NAME + " :" + mOldBDRProgress);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.m_clearmotion_actionbar, menu);
        return true;
    }

    private void recoverIndex() {
        if (mOldBDRProgress != null) {
            write(mOldClearMotionParameters);
            Log.d(TAG, "<recoverIndex>  mOldBDRProgress=" + mOldClearMotionParameters[0]
                    + "  mOldDemoMode = " + mOldClearMotionParameters[1]);
        }
    }

    private void onSaveClicked() {
        if (mBDRProgress != null) {
            write(mClearMotionParameters);
            Log.d(TAG, "<onSaveClicked>  mBDRProgress=" + mClearMotionParameters[0]
                    + " mDemoMode = " + mClearMotionParameters[1]);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                break;
            case R.id.cancel:
                recoverIndex();
                finish();
                break;
            case R.id.save:
                onSaveClicked();
                finish();
                break;
            default:
                break;
        }
        return true;
    }

    private void setVisible(View view, int visiable) {
        if (view != null) {
            view.setVisibility(visiable);
        }
    }

    private void getViewById() {
        mGroup = (RadioGroup) findViewById(R.id.radioGroup1);
        mGroup.setOnCheckedChangeListener(new OnCheckedChangeListener() {

            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if (R.id.demooff == checkedId) {
                    mDemoMode = sDemooff;
                    mClearMotionParameters[1] = sDemooffParameter;
                } else if (R.id.vertical == checkedId) {
                    mDemoMode = sVertical;
                    mClearMotionParameters[1] = sVerticalParameter;
                } else if (R.id.horizontal == checkedId) {
                    mDemoMode = sHorizontal;
                    mClearMotionParameters[1] = sHorizontalParameter;
                }
                Log.d(TAG, "<getViewById>SystemProperties.set = " + mClearMotionParameters[1]);
                write(mClearMotionParameters);
            }
        });

        mTextViewSkinSat = (TextView) findViewById(R.id.textView1_skinSat);
        mTextViewSkinSatRange = (TextView) findViewById(R.id.textView_skinSat);
        mTextViewSkinSatProgress = (TextView) findViewById(R.id.textView_skinSat_progress);
        mSeekBarSkinSat = (SeekBar) findViewById(R.id.seekBar_skinSat);
        mSeekBarSkinSat.setOnSeekBarChangeListener(this);

    }

    private String mBDRProgress = null;
    private String mDemoMode = null;

    private String mOldBDRProgress = null;
    private String mOldDemoMode = null;

    private static final int sDemooffParameter = 0;
    private static final int sVerticalParameter = 1;
    private static final int sHorizontalParameter = 2;

    private static final String sDemooff = Integer.toString(sDemooffParameter);
    private static final String sVertical = Integer.toString(sVerticalParameter);
    private static final String sHorizontal = Integer.toString(sHorizontalParameter);

    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (mSeekBarSkinSat == seekBar) {
            mTextViewSkinSatProgress.setText(BDR_NAME + ": " + progress);
            mClearMotionParameters[0] = progress;
        }
        Log.d(TAG, "<onProgressChanged>progress===" + progress
                + "  onProgressChanged  mClearMotionParameters:" + mClearMotionParameters[0]
                + "  " + mClearMotionParameters[1]);
    }

    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    public void onStopTrackingTouch(SeekBar seekBar) {
        Log.d(TAG, "<onProgressChanged>  mClearMotionParameters:" + mClearMotionParameters[0]
                + "  " + mClearMotionParameters[1]);
        write(mClearMotionParameters);
    }

    private void read() {
        mClearMotionParameters[0] = ClearMotionQualityJni.nativeGetFallbackIndex();
        mClearMotionParameters[1] = ClearMotionQualityJni.nativeGetDemoMode();
        Log.d(TAG, "<read> mClearMotionParameters[0]=" + mClearMotionParameters[0]
                + " mClearMotionParameters[1]=" + mClearMotionParameters[1]);
    }

    private void write(int[] mParameters) {
        ClearMotionQualityJni.nativeSetFallbackIndex(mParameters[0]);
        ClearMotionQualityJni.nativeSetDemoMode(mParameters[1]);
    }
}
