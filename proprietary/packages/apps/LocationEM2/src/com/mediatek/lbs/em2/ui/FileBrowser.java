package com.mediatek.lbs.em2.ui;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog.Builder;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class FileBrowser extends ListActivity {
    private List<String> paths = null;
    private String        mCurrentPath = "/";
    private String        mRootPath = "/";
    private String        mSdcardPath = "/sdcard/nmea";
    private String        mLocationEmPath;
    private boolean     mExit = false;

    private TextView    mTextView_FilePath;
    private Button        mButtonSdCard;
    private Button        mButtonLocationEM;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.file_browser);
        mTextView_FilePath    = (TextView) findViewById(R.id.TextView_FilePath);
        mLocationEmPath = "/data/data/" + getPackageName() + "/nmea";

        File file = new File(mLocationEmPath);
        file.mkdir();
        file = new File(mSdcardPath);
        file.mkdir();

        getFileDir(mLocationEmPath);
        initWidget();
    }

    private void initWidget() {
        mButtonSdCard        = (Button) findViewById(R.id.button_Sdcard);
        mButtonLocationEM    = (Button) findViewById(R.id.button_LocationEm);

        mButtonSdCard.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                mExit = false;
                getFileDir(mSdcardPath);
            }
        });
        mButtonLocationEM.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                mExit = false;
                getFileDir(mLocationEmPath);
            }
        });
    }

    protected void onListItemClick(ListView l, View v, int position, long id) {
        final File file = new File(paths.get(position));
        mExit = false;
        if (file.canRead() == true) {
            if (file.isDirectory()) {
                getFileDir(paths.get(position));
            } else if (file.isFile()) {
                new Builder(FileBrowser.this)
                    .setTitle("Confirm")
                    .setMessage("Do you want to load " + file.getName() + "?")
                    .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            finish();
                        }
                    })
                    .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                        }
                    })
                    .show();
            } // end of file.isFile
        } else {
            Toast.makeText(FileBrowser.this, file.getAbsolutePath() + " cannot be read", Toast.LENGTH_SHORT).show();
        }
    }

    private void getFileDir(String filePath) {
        List<String> items = null;

        mCurrentPath = filePath;
        mTextView_FilePath.setText(filePath);
        items = new ArrayList<String>();
        paths = new ArrayList<String>();
        File f = new File(filePath);

        if (!f.canRead()) {
            Toast.makeText(FileBrowser.this,
                    "ERR: The folder " + filePath + " cannot open", Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        File[] files = f.listFiles();

        if (!filePath.equals(mRootPath)) {
            items.add("Back to " + mRootPath);
            paths.add(mRootPath);
            items.add("Back to ../");
            paths.add(f.getParent());
        }
        for (int i = 0; i < files.length; i++) {
            File file = files[i];
            items.add(file.getName());
            paths.add(file.getAbsolutePath());
        }
        setListAdapter(new MyAdapter(this, items, paths));
    }


    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (event.getKeyCode() == KeyEvent.KEYCODE_BACK) {
            File file = new File(paths.get(1));
            if (file.canRead() == true) {
                if (!mCurrentPath.equals(mRootPath) &&
                        !mCurrentPath.equals(mSdcardPath) &&
                        !mCurrentPath.equals(mLocationEmPath)) {
                    getFileDir(paths.get(1)); //open parent folder
                    mExit = false;
                    return true;
                }
            }

            if (mExit == false) {
                Toast.makeText(FileBrowser.this,
                        "click back key again to exit", Toast.LENGTH_SHORT).show();
                mExit = true;
                return true;
            }
        }
        return super.onKeyDown(keyCode, event);
    }

    protected void onStart() {
        super.onStart();
    }
    protected void onRestart() {
        super.onRestart();
    }
    protected void onResume() {
        super.onResume();
    }
    protected void onPause() {
        super.onPause();
    }
    protected void onStop() {
        super.onStop();
    }
    protected void onDestroy() {
        super.onDestroy();
    }

    @SuppressWarnings(value = {"unused" })
    private void log(String msg) {
        Log.d("LocationEM", msg);
    }

}