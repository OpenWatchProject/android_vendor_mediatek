package com.mediatek.lbs.em2.ui;

import android.app.Activity;
import android.content.Intent;
import android.location.BatchedLocationCallback;
import android.location.Location;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import java.util.List;
import java.util.ArrayList;

public class LbsBatch extends Activity {
    public final static String TAG = "LocationEM_Batch";

    private final static int UPDATE_LOCATION_INFO = 1;

    TextView mTextViewSupportBatchSize;
    TextView mTextViewPeriod;
    Button   mButtonIncreasePeriod;
    Button   mButtonDecreasePeriod;
    CheckBox    mCheckBoxWakeupOnFifoFull;
    ToggleButton mToggleButtonStart;
    Button mButtonClearInfo;
    Button mButtonFlushLocations;
    TextView mTextViewLocationCount;
    TextView mTextViewLocationCallback;

    StringList mLocationInfo = new StringList(16);

    int mLocationCallbackCount;
    int mSupportedBatchSize;
    boolean mIsStarted = false;
    boolean mIsWakeOnFifoFull = true;
    long mPeriodSecond = 1;

    private LocationManager mLocationManager = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.batch);

        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        if (mLocationManager == null) {
            log("ERR: mLocationManager is null");
        }

        try {
            mSupportedBatchSize = mLocationManager.getGnssBatchSize();
        } catch (Exception e) {
            loge("Failed to run GnssBatching getGnssBatchSize.");
        }
        initUi();
        initUiListeners();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mLocationManager.unregisterGnssBatchedLocationCallback(mBatchingCallback);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    private void initUi() {
        mTextViewSupportBatchSize = (TextView) findViewById(R.id.supported_batch_size);
        mTextViewPeriod = (TextView) findViewById(R.id.period);
        mButtonIncreasePeriod= (Button) findViewById(R.id.button_IncreasePeriod);
        mButtonDecreasePeriod= (Button) findViewById(R.id.button_DecreasePeriod);
        mCheckBoxWakeupOnFifoFull = (CheckBox) findViewById(R.id.CheckBox_WakeupOnFifoFull);
        mToggleButtonStart = (ToggleButton) findViewById(R.id.batch_start);
        mButtonFlushLocations = (Button) findViewById(R.id.fused_flush_batched_locations);
        mButtonClearInfo = (Button) findViewById(R.id.fused_clear_info);
        mTextViewLocationCount = (TextView) findViewById(R.id.fused_count);
        mTextViewLocationCallback = (TextView) findViewById(R.id.fused_callback);

        mTextViewPeriod.setText("Location report Period (s) : " + mPeriodSecond);
        if (mIsWakeOnFifoFull) {
            mCheckBoxWakeupOnFifoFull.setChecked(true);
        }
        log("SupportedBatchSize = " + mSupportedBatchSize +", report period =" + mPeriodSecond);
        mTextViewSupportBatchSize.setText("" + mSupportedBatchSize);

    }

    private BatchedLocationCallback mBatchingCallback = new BatchedLocationCallback() {
        @Override
        public void onLocationBatch(List<Location> locations) {
            sendMessage(UPDATE_LOCATION_INFO, 0, 0, locations);
        }
    };

    private void initUiListeners() {
        mButtonIncreasePeriod.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPeriodSecond++;
                mTextViewPeriod.setText("Location report Period (s) : " + mPeriodSecond);
            }
        });
        mButtonDecreasePeriod.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mPeriodSecond--;
                mPeriodSecond = mPeriodSecond < 0 ? 0 : mPeriodSecond;
                mTextViewPeriod.setText("Location report Period (s) : " + mPeriodSecond);
            }
        });
        mCheckBoxWakeupOnFifoFull.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mCheckBoxWakeupOnFifoFull.isChecked()) {
                    mIsWakeOnFifoFull = true;
                } else {
                    mIsWakeOnFifoFull = false;
                }
            }
        });

        mToggleButtonStart.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                boolean enabled = mToggleButtonStart.isChecked();
                mIsStarted = enabled;
                try {
                    if (enabled) {
                        log("startBatching mIsWakeOnFifoFull=" + mIsWakeOnFifoFull +
                                ", BatchSize = " + mSupportedBatchSize);
                        mLocationCallbackCount = 0;
                        mLocationManager.registerGnssBatchedLocationCallback(
                                mPeriodSecond *1000000000,
                                mIsWakeOnFifoFull, mBatchingCallback, null);
                    } else {
                        log("stopBatching");
                        mLocationManager.unregisterGnssBatchedLocationCallback(mBatchingCallback);
                    }
                } catch (Exception e) {
                    loge("Failed to run GnssBatching registeration.");
                    Toast.makeText(getApplicationContext(),
                            "GnssBatching not support !!", Toast.LENGTH_SHORT).show();
                }
                enableOptions(!mIsStarted);
            }
        });
        mButtonClearInfo.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mLocationInfo.clear();
                mTextViewLocationCount.setText("");
                mTextViewLocationCallback.setText("");

                ///testing
                /*ArrayList<Location> mLocList = new ArrayList<Location>();
                Location loc = new Location(LocationManager.GPS_PROVIDER);
                loc.setTime(1343243233L);
                loc.setLatitude(22.22);
                loc.setLongitude(33.33);
                loc.setAccuracy((float)11.11);
                mLocList.add(loc);
                Location loc2 = new Location(LocationManager.GPS_PROVIDER);
                loc2.setTime(2343243233L);
                loc2.setLatitude(44.44);
                loc2.setLongitude(55.55);
                loc2.setAccuracy((float)66.66);
                mLocList.add(loc2);
                mBatchingCallback.onLocationBatch(mLocList);
                */
            }
        });
        mButtonFlushLocations.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                log("flushBatchedLocations");
                try {
                    mLocationManager.flushGnssBatch();
                } catch (Exception e) {
                    loge("Failed to run GnssBatching flushGnssBatch.");
                    Toast.makeText(getApplicationContext(),
                            "GnssBatching not support !!", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }

    private void enableOptions(boolean enabled) {
        mButtonIncreasePeriod.setEnabled(enabled);
        mButtonDecreasePeriod.setEnabled(enabled);
        mCheckBoxWakeupOnFifoFull.setEnabled(enabled);
    }

    private boolean isDoubleValid(EditText input) {
        try {
            Double.valueOf(input.getText().toString());
            return true;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    private boolean isIntValid(EditText input) {
        try {
            Integer.valueOf(input.getText().toString());
            return true;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    private boolean isFloatValid(EditText input) {
        try {
            Float.valueOf(input.getText().toString());
            return true;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    public void mapReportLocation(Location location) {
    }

    public static void log(Object msg) {
        Log.d(TAG, "" + msg);
    }

    public static void loge(Object msg) {
        Log.d(TAG, "ERR: " + msg);
    }

    private void sendMessage(int what, int arg1) {
        sendMessage(what, arg1, 0, null);
    }

    private void sendDelayedMessage(int what, long delayMillis) {
        mHandler.sendEmptyMessageDelayed(what, delayMillis);
    }

    private void sendMessage(int what, int arg1, int arg2, Object obj) {
        mHandler.obtainMessage(what, arg1, arg2, obj).sendToTarget();
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch(msg.what) {
            case UPDATE_LOCATION_INFO:
                if (msg.obj != null) {
                    Location l = null;
                    List<Location> locations = (List<Location>) msg.obj;
                    mLocationCallbackCount++;
                    for (int i = 0; i < locations.size(); i++) {
                        l = locations.get(i);
                        StringBuilder o = new StringBuilder();
                        o.append("i=" + i + " [" + l.getProvider() + "] " + " "
                                + LbsUtility.getTimeString(l.getTime()) + " "
                                + String.format("lat=%.06f, lng=%.06f ",
                                l.getLatitude(), l.getLongitude()));
                        if (l.hasAccuracy()) {
                            o.append("acc=" + l.getAccuracy() + " ");
                        }
                        if (l.hasAltitude()) {
                            o.append("atl=" + String.format("%.06f ",l.getAltitude()) + " ");
                        }
                        if (l.hasSpeed()) {
                            o.append("speed=" + l.getSpeed() + " ");
                        }
                        if (l.hasBearing()) {
                            o.append("bearing=" + l.getBearing() + " ");
                        }
                        o.append("\n");
                        mLocationInfo.add(o.toString());
                        log("location info updated index: " + i);
                    }
                    if (l != null) {
                        mapReportLocation(l);
                    }
                    String str = "Callback Count=" + mLocationCallbackCount
                            + ", Location size=" + locations.size();
                    mTextViewLocationCount.setText(str);
                    log(str);
                    mTextViewLocationCallback.setText(mLocationInfo.get());
                }
                break;
            default:
                log("WARNING: unknown handle event recv!!");
                break;
            }
        }
    };
}
