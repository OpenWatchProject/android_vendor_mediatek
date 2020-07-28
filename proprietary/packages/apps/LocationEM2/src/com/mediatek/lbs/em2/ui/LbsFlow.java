package com.mediatek.lbs.em2.ui;

import java.io.File;
import java.io.IOException;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Locale;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v4.content.LocalBroadcastManager;
import android.text.Html;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.lbs.em2.utils.AgpsDebugInterface;
import com.mediatek.lbs.em2.utils.AgpsDebugListener;
import com.mediatek.lbs.em2.utils.AgpsInterface;

public class LbsFlow extends Activity {

    protected static Context context;
    protected AgpsInterface agpsInterface;
    protected AgpsDebugInterface debugInterface;
    protected StringList stringList;
    protected DelayShowHandler delayShowHandler;
    protected Toast toast;
    protected int toastCount;
    protected AlertDialog alert;
    protected boolean mAgpsdExisted = false;
    protected LocalBroadcastManager mLocalBroadcastManager;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mLocalBroadcastManager = LocalBroadcastManager.getInstance(this);

        try {
            initAgpsInterface();
            mAgpsdExisted = true;
        } catch (RuntimeException e) {
            log("Failed to check agps version mAgpsdExisted=" + mAgpsdExisted);
            return;
        }

        setContentView(R.layout.flow);
        init();
        initUI();
        initPopupMenu();
        addOnClickListener();
        setLayoutDependency();
        initDebugInterface();
        delayShowHandler = new DelayShowHandler(stringList, textViewMessage);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (debugInterface != null) {
            debugInterface.removeListener();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mAgpsdExisted) {
            setLayoutDependency();
        }
    }

    // PopupMenu
    protected PopupMenu popupEmulatorMode;
    protected PopupMenu popupTestCase;

    protected LinearLayout layoutDebug;
    protected Button buttonResetAgpsd;
    protected Button buttonEmulatorMode;
    protected Button buttonTestCase;
    protected Button buttonClearScreen;
    protected TextView textViewMessage;

    static class DelayShowHandler extends Handler {

        protected StringList stringList;
        protected TextView textViewMessage;

        public DelayShowHandler(StringList stringList, TextView textViewMessage) {
            super();
            this.stringList = stringList;
            this.textViewMessage = textViewMessage;
        }

        @Override
        public void handleMessage(Message msg) {
            textViewMessage.setText(Html.fromHtml(stringList.get()),
                    TextView.BufferType.SPANNABLE);
        }
    }

    protected void init() {
        context = getApplicationContext();
        stringList = new StringList(80);

        toast = Toast.makeText(context, "", Toast.LENGTH_LONG);
        //toast.setGravity(Gravity.CENTER, 0, 0);
        toastCount = 0;

        alert = new AlertDialog.Builder(this).setTitle("my title")
                .setMessage("my content").setCancelable(false)
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface dialog) {
                    }
                })
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialoginterface, int i) {
                    }
                }).create();
    }

    protected void initUI() {
        layoutDebug = (LinearLayout) findViewById(R.id.LinearLayout_Debug);
        buttonResetAgpsd = (Button) findViewById(R.id.Button_ResetAgpsd);
        buttonEmulatorMode = (Button) findViewById(R.id.Button_EmulatorMode);
        buttonTestCase = (Button) findViewById(R.id.Button_TestCase);
        buttonClearScreen = (Button) findViewById(R.id.Button_ClearScreen);
        textViewMessage = (TextView) findViewById(R.id.TextView_Message);
    }

    protected void initPopupMenu() {
        popupEmulatorMode = new PopupMenu(this, buttonEmulatorMode);
        popupEmulatorMode.getMenu().add(0, 0, Menu.NONE, "Off");
        popupEmulatorMode.getMenu().add(0, 1, Menu.NONE, "On");
        popupEmulatorMode
                .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        agpsInterface.startEmulatorMode(item.getItemId());
                        return false;
                    }
                });

        popupTestCase = new PopupMenu(this, buttonEmulatorMode);
        popupTestCase.getMenu().add(0, 0, Menu.NONE, "All TC Loop 1");
        popupTestCase.getMenu().add(0, 1, Menu.NONE, "All TC Loop 2");
        popupTestCase.getMenu().add(0, 2, Menu.NONE, "All TC Loop 1000");
        popupTestCase.getMenu().add(0, 3, Menu.NONE, "All Common TC");
        popupTestCase.getMenu().add(0, 4, Menu.NONE, "All UP TC");
        popupTestCase.getMenu().add(0, 5, Menu.NONE, "All CP TC");
        popupTestCase.getMenu().add(0, 6, Menu.NONE, "All EVDO TC");
        popupTestCase.getMenu().add(0, 7, Menu.NONE, "Partial TC 1");
        popupTestCase.getMenu().add(0, 8, Menu.NONE, "Partial TC 2");
        popupTestCase
                .setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    @Override
                    public boolean onMenuItemClick(MenuItem item) {
                        agpsInterface.testCase(item.getItemId());
                        return false;
                    }
                });

    }

    protected void addOnClickListener() {
        buttonResetAgpsd.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                agpsInterface.resetAgpsd();
            }
        });
        buttonEmulatorMode.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                popupEmulatorMode.show();
            }
        });
        buttonTestCase.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                popupTestCase.show();
            }
        });
        buttonClearScreen.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                stringList.clear();
                textViewMessage.setText(Html.fromHtml(stringList.get()),
                        TextView.BufferType.SPANNABLE);
            }
        });
    }

    protected void initAgpsInterface() {
        try {
            agpsInterface = new AgpsInterface();
        } catch (IOException e) {
            loge("agps interface connection failure");
            e.printStackTrace();
        }
    }

    protected AgpsDebugListener debugListener = new AgpsDebugListener() {
        @Override
        public void onToastMessageReceived(String message) {
            toastCount++;
            toast.setText("[" + toastCount + "] " + message);
            toast.show();
        }

        @Override
        public void onViewMessageReceived(int color, String message) {
            stringList.add("<font color='#"
                    + Integer.toHexString(color & 0x00ffffff) + "'>"
                    + getTimeString() + " " + message + "</font><br>");

            Message m = Message.obtain();
            m.what = 0;
            delayShowHandler.removeMessages(0);
            delayShowHandler.sendMessageDelayed(m, 200);
        }

        @Override
        public void onDialogMessageReceived(String title, String message) {
            alert.setTitle(title);
            alert.setMessage(message);
            alert.show();
            //showAlertDialog(LbsFlow.this, title, message);
        }

        @Override
        public void onVzwDebugMessageReceived(String message) {
            Intent intent = new Intent(LbsC2kDbg.EM_C2K_DEBUG_MSG);
            intent.setPackage("com.mediatek.lbs.em2.ui");
            intent.putExtra("vzw_dbg", message);
            mLocalBroadcastManager.sendBroadcast(intent);
        }
    };

    protected void initDebugInterface() {
        try {
            debugInterface = new AgpsDebugInterface(debugListener);
        } catch (IOException e) {
            loge("agps debug interface connection failure");
            e.printStackTrace();
        }
    }

    protected void setLayoutDependency() {
        boolean debugModeEnabled = isDebugModeEnabled();
        setVisibility(layoutDebug, debugModeEnabled);
    }

    protected void setVisibility(View layout, boolean visible) {
        if (visible) {
            layout.setVisibility(View.VISIBLE);
        } else {
            layout.setVisibility(View.GONE);
        }
    }

    public void showAlertDialog(Context context, CharSequence tilte,
            CharSequence content) {
        new AlertDialog.Builder(context).setTitle(tilte).setMessage(content)
                .setCancelable(false)
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface dialog) {
                    }
                })
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialoginterface, int i) {
                    }
                }).show();
    }

    protected static String getTimeString() {
        Calendar cal = new GregorianCalendar();
        cal.setTimeInMillis(System.currentTimeMillis());
        // String date = String.format(Locale.US,
        // "%04d-%02d-%02d %02d:%02d:%02d",
        // cal.get(Calendar.YEAR), cal.get(Calendar.MONDAY) + 1,
        // cal.get(Calendar.DAY_OF_MONTH), cal.get(Calendar.HOUR_OF_DAY),
        // cal.get(Calendar.MINUTE),
        // cal.get(Calendar.SECOND));

        String date = String.format(Locale.US, "%02d:%02d.%03d",
                cal.get(Calendar.MINUTE), cal.get(Calendar.SECOND),
                cal.get(Calendar.MILLISECOND));
        return date;
    }

    protected boolean isDebugModeEnabled() {
        File file = new File(getFilesDir() + "/../show");
        return file.exists();
    }

    protected static void log(Object msg) {
        Log.d("LocationEM", "" + msg);
    }

    protected static void logw(Object msg) {
        Log.d("LocationEM", "[agps] WARNING: " + msg);
    }

    protected static void loge(Object msg) {
        Log.d("LocationEM [agps] ERR: ", "" + msg);
        Toast t = Toast
                .makeText(context, "ERR: " + "" + msg, Toast.LENGTH_LONG);
        t.setGravity(Gravity.CENTER, 0, 0);
        t.show();
    }

}
