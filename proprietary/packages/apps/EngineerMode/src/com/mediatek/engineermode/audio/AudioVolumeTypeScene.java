package com.mediatek.engineermode.audio;


import android.app.Activity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Activity for showing volume items.
 *
 */
public class AudioVolumeTypeScene {

    public static final String TAG = "Audio/VolumnTypeScene";
    private static final String CATEGORY_SPEECHVOL = "SpeechVol";
    private static final String TYPE_SCENE = "Scene";
    private static final String PARAM2 = "Scene,%1$s,%2$s";
    private Spinner mSceneSpinner;
    private boolean mIsSupportScene;
    private String mCurrentScene;
    private Listener mListener;


    public interface Listener {
        void onSceneChanged();
    }

    AudioVolumeTypeScene(Listener listener){
        mListener = listener;
    }

    public boolean initSceneSpinner(Activity activity, String category) {

        mSceneSpinner = (Spinner) activity.findViewById(R.id.audio_volume_scene_spinner);
        String strSpinnerScene = AudioTuningJni.getCategory(category, TYPE_SCENE);
        Elog.d(TAG, "strSpinnerScene:" + strSpinnerScene);
        if (strSpinnerScene == null) {
            mIsSupportScene = false;
            return false;
        }
        String[] value = strSpinnerScene.split(",");

        int length = value.length / 2;

        if (length <= 0) {
            mIsSupportScene = false;
            return false;
        }

        final String[] arraySpinner = new String[length];
        final String[] mArrayValue = new String[length];
        for (int k = 0; k < length; k++) {
            mArrayValue[k] = value[k * 2];
            arraySpinner[k] = value[k * 2 + 1];
        }

        ArrayAdapter<String> adatper = new ArrayAdapter<String>(activity,
                android.R.layout.simple_spinner_item, arraySpinner);
        adatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSceneSpinner.setAdapter(adatper);
        mCurrentScene = mArrayValue[0];
        mSceneSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                if (mCurrentScene != mArrayValue[arg2]) {
                    mCurrentScene = mArrayValue[arg2];
                    Elog.d(TAG, "onSceneChanged");
                    mListener.onSceneChanged();
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                // TODO Auto-generated method stub

            }
        });

        mSceneSpinner.setVisibility(View.VISIBLE);
        mIsSupportScene = true;
        return true;
    }

    public String getPara2String(String para2) {
        if(mIsSupportScene == true) {
            return String.format(PARAM2, mCurrentScene, para2);
        } else {
            return para2;
        }
    }
}
