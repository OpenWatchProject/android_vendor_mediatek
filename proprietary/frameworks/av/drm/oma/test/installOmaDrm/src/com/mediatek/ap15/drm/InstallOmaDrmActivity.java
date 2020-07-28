package com.mediatek.ap15.drm;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.content.Intent;
import android.drm.DrmManagerClient;
import android.drm.DrmRights;
import android.media.MediaPlayer;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.omadrm.OmaDrmUtils;

public class InstallOmaDrmActivity extends Activity {

    private static final String TAG = "InstallOmaDrmActivityLog";

    private final String CHOICE_FILEMANAGER_ACTION = "com.mediatek.filemanager.ADD_FILE";

    private final int REQUEST_CODE_INSTALL_DM = 1;
    private final int REQUEST_CODE_SAVE_RIGHTS = 2;
    private final int REQUEST_CODE_RECOVERY_DRM = 3;
    private final int REQUEST_CODE_TEST_PLAY = 10;

    private final String PREFIX_FILE = "file://";
    private final String SUFFIX_DM = ".dm";
    private final String SUFFIX_DR = "dr";
    private final String SUFFIX_DRC = "drc";

    private Button btnInstallDM = null;
    private Button btnSaveRights = null;
    private Button btnTestPlay = null;
    private Button btnRecoveryDrm = null;
    private EditText mDeviceIdEt = null;
    private String mDeviceId = "";

    private DrmManagerClient mClient = null;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        mClient = new DrmManagerClient(this);
        initializeUIControl();
    }

    @Override
    protected void onDestroy() {
        mClient.release();
        mClient = null;
        super.onDestroy();
    }

    private void initializeUIControl() {
        btnInstallDM = (Button) findViewById(R.id.btnInstallDM);
        btnInstallDM.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                selectFile(REQUEST_CODE_INSTALL_DM);
            }
        });
        btnSaveRights = (Button) findViewById(R.id.btnSaveRights);
        btnSaveRights.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                selectFile(REQUEST_CODE_SAVE_RIGHTS);
            }
        });
        btnTestPlay = (Button) findViewById(R.id.btnTestPlay);
        btnTestPlay.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                selectFile(REQUEST_CODE_TEST_PLAY);
            }
        });
        mDeviceIdEt = (EditText) findViewById(R.id.etDeviceId);
        btnRecoveryDrm = (Button) findViewById(R.id.btnRecovery);
        mDeviceIdEt.addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable s) {
                mDeviceId = s.toString();
                Log.d(TAG, "The deviced id you input is " + mDeviceId);
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

        });
        btnRecoveryDrm.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                selectFile(REQUEST_CODE_RECOVERY_DRM);
            }
        });

    }

    private void selectFile(int requestCode) {
        Intent intent = new Intent(CHOICE_FILEMANAGER_ACTION);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        startActivityForResult(intent, requestCode);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK) {
            Uri uri = data.getData();
            String path = uri.toString().replace(PREFIX_FILE, "");
            if (path == null || path.isEmpty()) {
                return;
            }
            Log.d(TAG, "onActivityResult(), uri: " + uri);
            Log.d(TAG, "onActivityResult(), path: " + path);
            switch (requestCode) {
            case REQUEST_CODE_INSTALL_DM:
                installDM(path);
                break;
            case REQUEST_CODE_SAVE_RIGHTS:
                saveRights(path);
                break;
            case REQUEST_CODE_TEST_PLAY:
                testPlay(path);
                break;
            case REQUEST_CODE_RECOVERY_DRM:
                recoveryDrmFile(path, mDeviceId);
                break;
            default:
                break;
            }
        }
    }

    private void installDM(String path) {
        Log.d(TAG, "installDM()");
        if (path.endsWith(SUFFIX_DM)) {
            showToast("start install dm file...");
            // install dm
            // change suffix from dm to dcf
            File oldDmFile = new File(path);
            path = path.replace(".dm", ".dcf");
            File newDmFile = new File(path);
            oldDmFile.renameTo(newDmFile);
            if (!newDmFile.exists()) {
                showToast("rename dm fail, dose not exist");
                return;
            }
            Log.d(TAG, "installDM(), change dm to dcf: " + path);
            boolean result = OmaDrmUtils.installDrmToDevice(mClient, path);
            Log.d(TAG, "install dm, result: " + result);
            if (result) {
                showToast("install dm success");
                Log.d(TAG, "installDM(), scan into MediaProvider, path: "
                        + path);
                String paths[] = { path };
                String mimeTypes[] = { "application/vnd.oma.drm.message" };
                MediaScannerConnection.scanFile(this, paths, mimeTypes, null);
            } else {
                showToast("install dm fail");
            }
        } else {
            Log.d(TAG, "not dm file, path: " + path);
            showToast("not dm file");
        }
    }

    private void saveRights(String path) {
        Log.d(TAG, "saveRights()");
        if (!path.endsWith(SUFFIX_DR) && !path.endsWith(SUFFIX_DRC)) {
            Log.d(TAG, "not rights file: " + path);
            showToast("not rights file");
            return;
        }
        showToast("start save rights...");
        // save rights
        String mime = "";
        if (path.endsWith(SUFFIX_DR)) {
            mime = "application/vnd.oma.drm.rights+xml";
        } else if (path.endsWith(SUFFIX_DRC)) {
            mime = "application/vnd.oma.drm.rights+wbxml";
        }
        try {
            DrmRights rights = new DrmRights(path, mime);
            int result = mClient.saveRights(rights, null, null);
            Log.d(TAG, "save rights, result: " + result);

            if (result == DrmManagerClient.ERROR_NONE) {
                showToast("save rights success");
                //result = DrmManagerClient.rescanDrmMediaFiles(this, rights, null);
                Log.d(TAG, "save rights, rescanDrmMediaFiles, result: " + result);
                // need scan dcf file to db
            } else if (result == DrmManagerClient.ERROR_UNKNOWN) {
                showToast("save rights fail");
            } else {
                showToast("unknow error");
            }
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private void testPlay(String path) {
        try {
            MediaPlayer mp = new MediaPlayer();
            mp.setDataSource(path);
            mp.prepare();
            mp.start();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (SecurityException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalStateException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private void recoveryDrmFile(String path, String deviceId) {

        //
        FileDescriptor outfFd = null;
        FileOutputStream outStream = null;
        boolean isDcfFile = path.endsWith(".dcf");
        if (isDcfFile == false) {
            Log.e(TAG, "Not a dcf file");
            return;
        }
        String mime = mClient.getOriginalMimeType(path);
        Log.d(TAG, "mime = " + mime);
        int start = mime.indexOf('/');
        String suffix = mime.substring(start);
        suffix = suffix.replace('/', '.') ;
        String outPath = path.replaceAll(".dcf", suffix);
        Log.d(TAG, "outPath = " + outPath);
        File outFile = new File(outPath);
        if (outFile.exists()) {
            Log.e
              (TAG, "The traget file exist, please copy it and delete the file fistly" + outPath);
            showToast("The traget file exist, please copy it and delete the file fistly" + outPath);
            return;
        } else {
            try {
                //create the file
                if (outFile.createNewFile() == false) {
                    Log.e(TAG, " create file fail");
                    showToast("Create file fail");
                    return;
                }
                outStream = new FileOutputStream(outFile);
                outfFd = outStream.getFD();
            } catch (FileNotFoundException fe) {
                outStream = null;
                outfFd = null;
                Log.e(TAG, "FileNotFoundException:" + fe.getMessage());
                return;
            } catch (IOException ie) {
                try {
                    outStream.close();
                } catch (IOException iee) {
                    Log.e(TAG, "IOException(close)" + iee.getMessage());
                    outStream = null;
                    return;
                }
            }
        }
        Log.d(TAG, "recoveryDrmFile outfFd = " + outfFd);
        int ret = RecoveryOmaDrmFile.recoveryOmaDrmFile(deviceId, path, outfFd);
        Log.d(TAG, "recoveryOmaDrmFile ret = " + ret);
        if (ret == 1) {
            showToast("recovery success");
        } else {
            showToast("recovery fail");
            outFile.delete();
        }
        try {
            if (outStream != null) {
                outStream.close();
            }
        } catch (IOException ie) {
            Log.e(TAG, "IOException" + ie.getMessage());
            outStream = null;
        }
    }

    private void showToast(String toast) {
        Toast.makeText(this, toast, Toast.LENGTH_SHORT).show();
    }
}
