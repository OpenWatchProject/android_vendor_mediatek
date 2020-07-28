package com.mediatek.gallery3d.video;

import android.net.Uri;
import android.nfc.NfcAdapter;
import android.nfc.NfcAdapter.CreateBeamUrisCallback;
import android.nfc.NfcAdapter.OnNdefPushCompleteCallback;
import android.nfc.NfcEvent;
import android.os.Bundle;
import android.os.Handler;

import com.android.gallery3d.app.MoviePlayer;

import com.mediatek.gallery3d.util.Log;

public class NfcHooker extends MovieHooker implements CreateBeamUrisCallback {

    private static final String TAG = "VP_NfcHooker";
    private IMovieItem mMovieItem;
    private MoviePlayer mPlayer;
    private boolean mBeamVideoIsPlaying = false;
    private NfcAdapter mNfcAdapter;
    private final Handler mHandler = new Handler();

    private final Runnable mPlayVideoRunnable = new Runnable() {
        @Override
        public void run() {
            if (mPlayer != null && mBeamVideoIsPlaying) {
                Log.v(TAG, "NFC call play video");
                mPlayer.onPlayPause();
            }
        }
    };

    private final Runnable mPauseVideoRunnable = new Runnable() {
        @Override
        public void run() {
            if (mPlayer != null && mPlayer.isPlaying()) {
                Log.v(TAG, "NFC call pause video");
                mBeamVideoIsPlaying = true;
                mPlayer.onPlayPause();
            } else {
                mBeamVideoIsPlaying = false;
            }
        }
    };

    public void setParameter(final String key, final Object value) {
        super.setParameter(key, value);
        if (value instanceof IMovieItem) {
            mMovieItem = (IMovieItem) value;
        }
        if (value instanceof MoviePlayer) {
            mPlayer = (MoviePlayer) value;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        init();
    }

    private void init() {
        // / M: Get Nfc adapter and set callback available. @{
        mNfcAdapter = NfcAdapter.getDefaultAdapter(getContext()
                .getApplicationContext());
        if (mNfcAdapter == null) {
            Log.w(TAG, "NFC not available!");
            return;
        }
        mNfcAdapter.setBeamPushUrisCallback(this, getContext());
        OnNdefPushCompleteCallback completeCallBack = new OnNdefPushCompleteCallback() {
            @Override
            public void onNdefPushComplete(NfcEvent event) {
                mHandler.removeCallbacks(mPlayVideoRunnable);
                mHandler.post(mPlayVideoRunnable);
            }
        };
        mNfcAdapter.setOnNdefPushCompleteCallback(completeCallBack,
                getContext(), getContext());
        // / @}
    }

    /**
     * M: Add NFC callback to provide the uri.
     */
    @Override
    public Uri[] createBeamUris(NfcEvent event) {
        if (mMovieItem == null) {
            Log.v(TAG, "createBeamUris, mMovieItem == null, return");
            return null;
        }
        mHandler.removeCallbacks(mPauseVideoRunnable);
        // / M: ALPS02096911 post delay to avoid timing issue
        mHandler.postDelayed(mPauseVideoRunnable, 500);
        Uri beamUri = mMovieItem.getUri();
        Log.v(TAG, "NFC call for uri " + beamUri);
        return new Uri[] { beamUri };
    }

}
