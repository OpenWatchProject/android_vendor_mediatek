package com.mediatek.engineermode.vilte;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;


/**
 * ViLTE Configuration.
 */
public class VilteMenuCommon extends Activity implements View.OnClickListener {
    private final String TAG = "Vilte/MenuCommon";
    private final String PROP_VILTE_VIDEO_FPS = "persist.vendor.vt.vfps";
    private final String PROP_VILTE_SOURCE_BITSTREAM = "persist.vendor.vt.dump_source";
    private final String PROP_VILTE_SINK_BITSTREAM = "persist.vendor.vt.dump_sink";
    private final String PROP_VILTE_VIDEO_LEVEL = "persist.vendor.vt.vlevel";
    private final String PROP_VILTE_VIDEO_PROFILE = "persist.vendor.vt.vprofile";
    private final String PROP_VILTE_DEBUG_INFO_DISPLAY = "persist.vendor.vt.RTPInfo";
    private final String PROP_VILTE_DOWN_GRADE = "persist.vendor.vt.downgrade";

    private final String PROP_VILTE_VIDEO_LEVEL_BIT_RATE = "persist.vendor.vt.vbitrate";
    private final String PROP_VILTE_VIDEO_VENC_BITRATE_RATIO = "persist.vendor.vt.bitrate_ratio";

    private final String PROP_VILTE_VIDEO_IDR_PERIOD = "persist.vendor.vt.viperiod";
    private final String PROP_VILTE_CAMERA = "persist.vendor.vt.camera";
    private final String PROP_VILTE_VFORMAT = "persist.vendor.vt.vformat";
    private final String PROP_VILTE_VWIDTH = "persist.vendor.vt.vwidth";
    private final String PROP_VILTE_VHEIGHT = "persist.vendor.vt.vheight";

    private final String PROPERTY_VIDEOCALL_AUDIO_OUTPUT = "persist.radio.call.audio.output";

    private final String[] PROFILE_VALUES = {"0", "1", "4", "16"};
    private final String[] PROFILE_LABLE = {"Default", "Baseline 1", "", "", "Main 4",
            "", "", "", "", "", "", "", "", "", "", "", "High 16"};
    private TextView mVilteVideoFps;
    private TextView mVilteSourceBitstream;
    private TextView mVilteSinkBitstream;
    private TextView mTextviewLevel;
    private TextView mTextviewProfile;
    private TextView mTextviewBitrate;
    private TextView mTextviewBitrateRatio;
    private TextView mTextviewIPeriod;
    private TextView mTextviewDebugInfoDisplay;
    private TextView mTextviewDownGrade;
    private TextView mTextviewCamera;

    private TextView mTextviewVformat;
    private TextView mTextviewVwidth;
    private TextView mTextviewVheight;
    private TextView mVilteAudioOutput;

    private Button mButtonSetFps;
    private Button mButtonEnableSourceBitstream;
    private Button mButtonDisableSourceBitstream;
    private Button mButtonEnableSinkBitstream;
    private Button mButtonDisableSinkBitstream;
    private Button mButtonSetLevel;
    private Button mButtonSetProfile;
    private Button mButtonSetBitRate;
    private Button mButtonSetBitrateRatio;
    private Button mButtonSetIperiod;
    private Spinner mSpinner;
    private Spinner mSpinnerProfile;
    private Spinner mSpinnerCamera;
    private Spinner mSpinnerLevel;
    private Spinner mSpinnerVformat;
    private EditText mEdittextBitRate;
    private EditText mEdittextBitrateRatio;
    private EditText mEdittextIperiod;
    private EditText mEdittextVwidth;
    private EditText mEdittextVheight;
    private Button mButtonEnableDebugInfoDisplay;
    private Button mButtonDisableDebugInfoDisplay;
    private Button mButtonEnableDownGrade;
    private Button mButtonDisableDownGrade;
    private Button mButtonEnableCamera;
    private Button mButtonVformat;
    private Button mButtonVwidth;
    private Button mButtonVheight;
    private Button mButtonEnabletAudioOutput;
    private Button mButtonDisabletAudioOutput;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.vilte_menu_common);
        Elog.d(TAG, "onCreate()");
        mVilteVideoFps = (TextView) findViewById(R.id.vilte_video_fps_status);
        mVilteSourceBitstream = (TextView) findViewById(R.id.vilte_source_bitstream_status);
        mVilteSinkBitstream = (TextView) findViewById(R.id.vilte_sink_bitstream_status);
        mTextviewLevel = (TextView) findViewById(R.id.vilte_video_level_status);
        mTextviewProfile = (TextView) findViewById(R.id.vilte_video_profile_status);
        mTextviewBitrate = (TextView) findViewById(R.id.vilte_video_bitrate_status);
        mTextviewBitrateRatio = (TextView) findViewById(R.id.vilte_video_bitrate_ratio_status);
        mTextviewIPeriod = (TextView) findViewById(R.id.vilte_video_iperiod_status);
        mTextviewDebugInfoDisplay = (TextView) findViewById(R.id.vilte_debug_info_display_status);
        mTextviewDownGrade = (TextView) findViewById(R.id.vilte_down_grade_status);
        mTextviewCamera = (TextView) findViewById(R.id.vilte_camera_status);

        mTextviewVformat = (TextView) findViewById(R.id.vilte_video_format);
        mTextviewVwidth = (TextView) findViewById(R.id.vilte_video_width);
        mTextviewVheight = (TextView) findViewById(R.id.vilte_video_height);

        mSpinner = (Spinner) findViewById(R.id.vilte_video_fps_values);
        mSpinnerLevel = (Spinner) findViewById(R.id.vilte_video_level_values);
        mSpinnerProfile = (Spinner) findViewById(R.id.vilte_video_profile_values);
        mSpinnerCamera = (Spinner) findViewById(R.id.vilte_camera_values);
        mSpinnerVformat = (Spinner) findViewById(R.id.vilte_video_format_values);

        mEdittextBitRate = (EditText) findViewById(R.id.vilte_video_level_bit_rate_values);
        mEdittextBitrateRatio = (EditText) findViewById(R.id.vilte_video_bitrate_ratio_values);
        mEdittextIperiod = (EditText) findViewById(R.id.vilte_video_idr_period_values);
        mEdittextVwidth = (EditText) findViewById(R.id.vilte_video_width_values);
        mEdittextVheight = (EditText) findViewById(R.id.vilte_video_height_values);
        mVilteAudioOutput = (TextView) findViewById(R.id.vilte_audio_output_status);


        mButtonSetFps = (Button) findViewById(R.id.vilte_video_fps_set);
        mButtonSetLevel = (Button) findViewById(R.id.vilte_video_level_set);
        mButtonSetProfile = (Button) findViewById(R.id.vilte_video_profile_set);
        mButtonSetBitRate = (Button) findViewById(R.id.vilte_video_level_bit_rate_set);
        mButtonSetBitrateRatio = (Button) findViewById(R.id.vilte_video_venc_bitrate_ratio_set);
        mButtonSetIperiod = (Button) findViewById(R.id.vilte_video_idr_period_set);
        mButtonEnableSourceBitstream = (Button) findViewById(R.id.vilte_source_bitstream_enable);
        mButtonDisableSourceBitstream = (Button) findViewById(R.id.vilte_source_bitstream_disable);
        mButtonEnableSinkBitstream = (Button) findViewById(R.id.vilte_sink_bitstream_enable);
        mButtonDisableSinkBitstream = (Button) findViewById(R.id.vilte_sink_bitstream_disable);
        mButtonEnableDebugInfoDisplay = (Button) findViewById(R.id.vilte_debug_info_dispaly_enable);
        mButtonDisableDebugInfoDisplay = (Button) findViewById(R.id
                .vilte_debug_info_dispaly_disable);
        mButtonEnableDownGrade = (Button) findViewById(R.id.vilte_downGrade_enable);
        mButtonEnableCamera = (Button) findViewById(R.id.vilte_camera_enable);
        mButtonDisableDownGrade = (Button) findViewById(R.id.vilte_downGrade_disable);
        mButtonVformat = (Button) findViewById(R.id.vilte_video_format_set);
        mButtonVwidth = (Button) findViewById(R.id.vilte_video_width_set);
        mButtonVheight = (Button) findViewById(R.id.vilte_video_height_set);
        mButtonEnabletAudioOutput = (Button) findViewById(R.id.vilte_audio_output_enable);
        mButtonDisabletAudioOutput = (Button) findViewById(R.id.vilte_audio_output_disable);

        mButtonSetFps.setOnClickListener(this);
        mButtonEnableSourceBitstream.setOnClickListener(this);
        mButtonDisableSourceBitstream.setOnClickListener(this);
        mButtonEnableSinkBitstream.setOnClickListener(this);
        mButtonDisableSinkBitstream.setOnClickListener(this);
        mButtonSetLevel.setOnClickListener(this);
        mButtonSetProfile.setOnClickListener(this);
        mButtonSetBitRate.setOnClickListener(this);
        mButtonSetBitrateRatio.setOnClickListener(this);
        mButtonSetIperiod.setOnClickListener(this);
        mButtonEnableDebugInfoDisplay.setOnClickListener(this);
        mButtonDisableDebugInfoDisplay.setOnClickListener(this);
        mButtonEnableDownGrade.setOnClickListener(this);
        mButtonDisableDownGrade.setOnClickListener(this);
        mButtonEnableCamera.setOnClickListener(this);
        mButtonVformat.setOnClickListener(this);
        mButtonVwidth.setOnClickListener(this);
        mButtonVheight.setOnClickListener(this);
        mButtonEnabletAudioOutput.setOnClickListener(this);
        mButtonDisabletAudioOutput.setOnClickListener(this);

    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.d(TAG, "onResume()");
        queryCurrentValue();
    }


    public void onClick(View v) {
        if (v == mButtonSetFps) {
            String fps = mSpinner.getSelectedItem().toString();
            Elog.d(TAG, "Set " + PROP_VILTE_VIDEO_FPS + " = " + fps);
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VIDEO_FPS, fps);
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonEnableSourceBitstream) {
            Elog.d(TAG, "Set " + PROP_VILTE_SOURCE_BITSTREAM + " = 1");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_SOURCE_BITSTREAM, "1");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonDisableSourceBitstream) {
            Elog.d(TAG, "Set " + PROP_VILTE_SOURCE_BITSTREAM + " = 0");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_SOURCE_BITSTREAM, "0");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonEnableSinkBitstream) {
            Elog.d(TAG, "Set " + PROP_VILTE_SINK_BITSTREAM + " = 1");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_SINK_BITSTREAM, "1");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonDisableSinkBitstream) {
            Elog.d(TAG, "Set " + PROP_VILTE_SINK_BITSTREAM + " = 0");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_SINK_BITSTREAM, "0");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonSetLevel) {
            String level = String.valueOf(mSpinnerLevel.getSelectedItemPosition());
            Elog.d(TAG, "Set " + PROP_VILTE_VIDEO_LEVEL + " = " + level);
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VIDEO_LEVEL, level);
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonSetBitRate) {
            Elog.d(TAG, "Set " + PROP_VILTE_VIDEO_LEVEL_BIT_RATE +
                    " = " + mEdittextBitRate.getText().toString());
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VIDEO_LEVEL_BIT_RATE,
                    mEdittextBitRate.getText().toString());
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonSetBitrateRatio) {
            Elog.d(TAG, "Set " + PROP_VILTE_VIDEO_VENC_BITRATE_RATIO +
                    " = " + mEdittextBitrateRatio.getText().toString());
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VIDEO_VENC_BITRATE_RATIO,
                    mEdittextBitrateRatio.getText().toString());
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonSetIperiod) {
            Elog.d(TAG, "Set " + PROP_VILTE_VIDEO_IDR_PERIOD +
                    " = " + mEdittextIperiod.getText().toString());
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VIDEO_IDR_PERIOD,
                    mEdittextIperiod.getText().toString());
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonSetProfile) {
            String values = PROFILE_VALUES[mSpinnerProfile.getSelectedItemPosition()];
            Elog.d(TAG, "Set " + PROP_VILTE_VIDEO_PROFILE + " = " + values);
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VIDEO_PROFILE, values);
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonEnableDebugInfoDisplay) {
            Elog.d(TAG, "Set " + PROP_VILTE_DEBUG_INFO_DISPLAY + " = 1");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_DEBUG_INFO_DISPLAY, "1");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonDisableDebugInfoDisplay) {
            Elog.d(TAG, "Set " + PROP_VILTE_DEBUG_INFO_DISPLAY + " = 0");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_DEBUG_INFO_DISPLAY, "0");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonEnableDownGrade) {
            Elog.d(TAG, "Set " + PROP_VILTE_DOWN_GRADE + " = 1");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_DOWN_GRADE, "1");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonDisableDownGrade) {
            Elog.d(TAG, "Set " + PROP_VILTE_DOWN_GRADE + " = 0");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_DOWN_GRADE, "0");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonEnableCamera) {
            Elog.d(TAG, "Set " + PROP_VILTE_CAMERA +
                    mSpinnerCamera.getSelectedItem().toString());
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_CAMERA,
                    mSpinnerCamera.getSelectedItem().toString());
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonVformat) {
            String format = String.valueOf(mSpinnerVformat.getSelectedItemPosition());
            Elog.d(TAG, "Set " + PROP_VILTE_VFORMAT + format);
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VFORMAT, format);
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonVwidth) {
            Elog.d(TAG, "Set " + PROP_VILTE_VWIDTH +
                    mEdittextVwidth.getText().toString());
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VWIDTH,
                    mEdittextVwidth.getText().toString());
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonVheight) {
            Elog.d(TAG, "Set " + PROP_VILTE_VHEIGHT +
                    mEdittextVheight.getText().toString());
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VHEIGHT,
                    mEdittextVheight.getText().toString());
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mButtonEnabletAudioOutput) {
            Elog.d(TAG, "Set " + PROPERTY_VIDEOCALL_AUDIO_OUTPUT + " = 0");
            EmUtils.systemPropertySet(PROPERTY_VIDEOCALL_AUDIO_OUTPUT, "0");
        } else if (v == mButtonDisabletAudioOutput) {
            Elog.d(TAG, "Set " + PROPERTY_VIDEOCALL_AUDIO_OUTPUT + " = 1");
            EmUtils.systemPropertySet(PROPERTY_VIDEOCALL_AUDIO_OUTPUT, "1");
        }
        queryCurrentValue();
    }

    private void queryCurrentValue() {

        String fps = EmUtils.systemPropertyGet(PROP_VILTE_VIDEO_FPS, "0");
        String level = EmUtils.systemPropertyGet(PROP_VILTE_VIDEO_LEVEL, "0");
        String profile = EmUtils.systemPropertyGet(PROP_VILTE_VIDEO_PROFILE, "0");
        String bitrate = EmUtils.systemPropertyGet(PROP_VILTE_VIDEO_LEVEL_BIT_RATE, "");
        String bitrateRatio = EmUtils.systemPropertyGet(PROP_VILTE_VIDEO_VENC_BITRATE_RATIO, "");
        String iPeriod = EmUtils.systemPropertyGet(PROP_VILTE_VIDEO_IDR_PERIOD, "");
        String source = EmUtils.systemPropertyGet(PROP_VILTE_SOURCE_BITSTREAM, "");
        String sink = EmUtils.systemPropertyGet(PROP_VILTE_SINK_BITSTREAM, "");

        String debugInfoDisplay = EmUtils.systemPropertyGet(PROP_VILTE_DEBUG_INFO_DISPLAY, "");
        String downGrade = EmUtils.systemPropertyGet(PROP_VILTE_DOWN_GRADE, "");
        String camera = EmUtils.systemPropertyGet(PROP_VILTE_CAMERA, "");

        String format = EmUtils.systemPropertyGet(PROP_VILTE_VFORMAT, "0");
        String width = EmUtils.systemPropertyGet(PROP_VILTE_VWIDTH, "");
        String height = EmUtils.systemPropertyGet(PROP_VILTE_VHEIGHT, "");
        String audioOutput = EmUtils.systemPropertyGet(PROPERTY_VIDEOCALL_AUDIO_OUTPUT, "0");


        mVilteVideoFps.setText(PROP_VILTE_VIDEO_FPS + " = " + fps);
        mVilteSourceBitstream.setText(PROP_VILTE_SOURCE_BITSTREAM + " = " + source);
        mVilteSinkBitstream.setText(PROP_VILTE_SINK_BITSTREAM + " = " + sink);

        mTextviewLevel.setText(PROP_VILTE_VIDEO_LEVEL + " = " + level);
        mTextviewProfile.setText(PROP_VILTE_VIDEO_PROFILE + " = " + profile);
        mTextviewBitrate.setText(PROP_VILTE_VIDEO_LEVEL_BIT_RATE + " = " + bitrate);
        mTextviewBitrateRatio.setText(PROP_VILTE_VIDEO_VENC_BITRATE_RATIO + " = " + bitrateRatio);
        mTextviewIPeriod.setText(PROP_VILTE_VIDEO_IDR_PERIOD + " = " + iPeriod);

        for (int i = 0; i < mSpinner.getCount(); i++) {
            if (mSpinner.getItemAtPosition(i).toString().equals(fps)) {
                mSpinner.setSelection(i);
                break;
            }
        }
        String profile_label = PROFILE_LABLE[Integer.valueOf(profile)];
        for (int i = 0; i < mSpinnerProfile.getCount(); i++) {
            if (mSpinnerProfile.getItemAtPosition(i).toString().equals(profile_label)) {
                mSpinnerProfile.setSelection(i);
                break;
            }
        }

        mSpinnerLevel.setSelection(Integer.parseInt(level));
        mSpinnerVformat.setSelection(Integer.parseInt(format));

        for (int i = 0; i < mSpinnerCamera.getCount(); i++) {
            if (mSpinnerCamera.getItemAtPosition(i).toString().equals(camera)) {
                mSpinnerCamera.setSelection(i);
                break;
            }
        }

        mEdittextBitRate.setText(bitrate);
        mEdittextBitrateRatio.setText(bitrateRatio);
        mEdittextIperiod.setText(iPeriod);
        mEdittextVwidth.setText(width);
        mEdittextVheight.setText(height);

        mTextviewDebugInfoDisplay.setText(PROP_VILTE_DEBUG_INFO_DISPLAY + " = " + debugInfoDisplay);
        mTextviewDownGrade.setText(PROP_VILTE_DOWN_GRADE + " = " + downGrade);
        mTextviewCamera.setText(PROP_VILTE_CAMERA + " = " + camera);
        mTextviewVformat.setText(PROP_VILTE_VFORMAT + " = " + format);
        mTextviewVwidth.setText(PROP_VILTE_VWIDTH + " = " + width);
        mTextviewVheight.setText(PROP_VILTE_VHEIGHT + " = " + height);
        mVilteAudioOutput.setText(PROPERTY_VIDEOCALL_AUDIO_OUTPUT + " = " + audioOutput);
    }

}
