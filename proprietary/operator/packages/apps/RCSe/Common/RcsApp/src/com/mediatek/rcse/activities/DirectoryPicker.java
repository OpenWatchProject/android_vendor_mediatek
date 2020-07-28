package com.mediatek.rcse.activities;

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import com.mediatek.rcs.R;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;

/**
 * @author MTK33197 .
 */
public class DirectoryPicker extends ListActivity {

    public static final String START_DIR = "startDir";
    public static final String ONLY_DIRS = "onlyDirs";
    public static final String SHOW_HIDDEN = "showHidden";
    public static final String CHOSEN_DIRECTORY = "chosenDir";
    public static final int PICK_DIRECTORY = 43522432;
    private File mDir;
    private boolean mShowHidden = false;
    private boolean mOnlyDirs = true;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle extras = getIntent().getExtras();
        mDir = Environment.getExternalStorageDirectory();
        if (extras != null) {
            String preferredStartDir = extras.getString(START_DIR);
            mShowHidden = extras.getBoolean(SHOW_HIDDEN, false);
            mOnlyDirs = extras.getBoolean(ONLY_DIRS, true);
            if (preferredStartDir != null) {
                File startDir = new File(preferredStartDir);
                if (startDir.isDirectory()) {
                    mDir = startDir;
                }
            }
        }

        setContentView(R.layout.chooser_list);
        setTitle(mDir.getAbsolutePath());
        Button btnChoose = (Button) findViewById(R.id.btnChoose);
        String name = mDir.getName();
        if (name.length() == 0) {
            name = "/";
        }
        btnChoose.setText("Choose " + "'" + name + "'");
        btnChoose.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                returnDir(mDir.getAbsolutePath());
            }
        });

        ListView lv = getListView();
        lv.setTextFilterEnabled(true);

        if (!mDir.canRead()) {
            Context context = getApplicationContext();
            String msg = "Could not read folder contents.";
            Toast toast = Toast.makeText(context, msg, Toast.LENGTH_LONG);
            toast.show();
            return;
        }

        final ArrayList<File> files = filter(mDir.listFiles(), mOnlyDirs,
                mShowHidden);
        String[] names = names(files);
        setListAdapter(new ArrayAdapter<String>(this, R.layout.list_item, names));

        lv.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view,
                    int position, long id) {
                if (!files.get(position).isDirectory()) {
                    return;
                }
                String path = files.get(position).getAbsolutePath();
                Intent intent = new Intent(DirectoryPicker.this,
                        DirectoryPicker.class);
                intent.putExtra(DirectoryPicker.START_DIR, path);
                intent.putExtra(DirectoryPicker.SHOW_HIDDEN, mShowHidden);
                intent.putExtra(DirectoryPicker.ONLY_DIRS, mOnlyDirs);
                startActivityForResult(intent, PICK_DIRECTORY);
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == PICK_DIRECTORY && resultCode == RESULT_OK) {
            Bundle extras = data.getExtras();
            String path = (String) extras.get(DirectoryPicker.CHOSEN_DIRECTORY);
            returnDir(path);
        }
    }

    private void returnDir(String path) {
        Intent result = new Intent();
        result.putExtra(CHOSEN_DIRECTORY, path);
        setResult(RESULT_OK, result);
        finish();
    }

    /**
     * @param fileList .
     * @param onlyDirs .
     * @param showHidden .
     * @return .
     */
    public ArrayList<File> filter(File[] fileList, boolean onlyDirs,
            boolean showHidden) {
        ArrayList<File> files = new ArrayList<File>();
        for (File file : fileList) {
            if (onlyDirs && !file.isDirectory()) {
                continue;
            }
            if (!showHidden && file.isHidden()) {
                continue;
            }
            files.add(file);
        }
        Collections.sort(files);
        return files;
    }

    /**
     * @param files .
     * @return .
     */
    public String[] names(ArrayList<File> files) {
        String[] names = new String[files.size()];
        int i = 0;
        for (File file : files) {
            names[i] = file.getName();
            i++;
        }
        return names;
    }
}
