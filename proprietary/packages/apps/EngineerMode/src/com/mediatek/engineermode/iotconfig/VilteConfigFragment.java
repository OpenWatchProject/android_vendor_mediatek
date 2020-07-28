package com.mediatek.engineermode.iotconfig;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.regex.Pattern;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class VilteConfigFragment extends Fragment {

    private static final String TAG = "Iot/VilteConfigFragment";
    private static final int VILTE_ACTION_GET_DEFAULT_PROFILE = 0;
    private static final int VILTE_ACTION_WRITE_PROFILE = 1;
    private static final String VILTE_DEFAULT_TEMPLATE_FILE_NAME = "viLTE_media_profiles_opxx.xml";
    private static final String VILTE_DEFAULT_FILE_NAME = "viLTE_media_profiles.xml";
    public static final int VILTE_READ_DEFAULT_PROFILE_FAILED = 0;
    public static final int VILTE_READ_DEFAULT_PROFILE_OK = 1;
    public static final int VILTE_WRITE_PROFILE_FAILED = 2;
    public static final int VILTE_WRITE_PROFILE_OK = 3;
    public static final int UNKNOW = 4;
    private EditText operatorId;
    private ListView videoProfileListView;
    private Button btnProfileSave;
    private Button btnProfileCancel;
    private Button btnProfileReset;
    ArrayList<VideoProfileModel> profileModelList;
    public static ArrayList<VideoQualityModel> selectqualityList;
    public static ArrayList<VideoProfileModel> selectProfileList;
    private VideoProfileAdapter profileAdapter;
    private Button btnShowHelp;
    private HorizontalScrollView profileTitleScrollView;
    private LinkedHorizontalScrollView profileContentScrollView;
    private LinearLayout videoProfileContent;
    private HashMap<String, Integer> VideoQualityMapping = new HashMap<String, Integer>();
    private String curFileName = "";

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case VILTE_READ_DEFAULT_PROFILE_OK:
                showToast("Read profile template completed!");
                updateVideoProfileUI();
                break;
            case VILTE_READ_DEFAULT_PROFILE_FAILED:
                showToast("Read profile template failed!");
                break;
            case VILTE_WRITE_PROFILE_OK:
                showToast("Write profile completed!");
                operatorId.setText("");
                new XmlParserThread(VILTE_ACTION_GET_DEFAULT_PROFILE).start();
                break;
            case VILTE_WRITE_PROFILE_FAILED:
                showToast("Write profile failed!");
                break;
            default:
                break;
            }
        }
    };

    public static boolean isNumeric(String str) {
        Pattern pattern = Pattern.compile("[0-9]*");
        return pattern.matcher(str).matches();
    }

    public void showInfoDialog(Context context, String content) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        builder.setTitle("Warning");

        builder.setMessage(content).setCancelable(false)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        // TODO: handle the OK
                        dialog.cancel();
                    }
                });

        AlertDialog alertDialog = builder.create();
        alertDialog.show();
    }

    /**
     * Delete file on sdcard by fileName
     *
     * @param fileName
     *            , Deleted file name
     * @return Delete success return true, otherwise return false
     */
    public boolean deleteFileFromInnerSDCard(String fileName) {
        File file = new File(getInnerSDCardPath() + File.separator + fileName);
        if (file.isFile() && file.exists()) {
            file.delete();
            return true;
        } else {
            return false;
        }
    }

    public String getInnerSDCardPath() {
        return Environment.getExternalStorageDirectory().getAbsolutePath();
    }

    private OnClickListener btnOnClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            // TODO Auto-generated method stub
            switch (v.getId()) {
            case R.id.btn_profile_cancel:
                new XmlParserThread(VILTE_ACTION_GET_DEFAULT_PROFILE).start();
                break;
            case R.id.btn_profile_save:
                refreshSelectProfileList(profileAdapter.getList());
                refreshVideoProfileView(true);
                new XmlParserThread(VILTE_ACTION_WRITE_PROFILE).start();
                break;
            case R.id.btn_profile_reset:
                deleteFileFromInnerSDCard(VILTE_DEFAULT_FILE_NAME);
                new XmlParserThread(VILTE_ACTION_GET_DEFAULT_PROFILE).start();
                break;
            case R.id.btn_show_help:
                showInfoDialog(getActivity(), getActivity().getResources()
                        .getString(R.string.vilte_help_content));
                break;
            default:
                break;
            }
        }
    };

    public void showToast(String content) {
        Toast.makeText(getActivity(), content, Toast.LENGTH_SHORT).show();
    }

    public boolean checkSelectQualityList(ArrayList<VideoQualityModel> list) {
        if (selectqualityList.size() != 4) {
            return false;
        }
        for (String key : VideoQualityMapping.keySet()) {
            if (VideoQualityMapping.get(key) == null
                    || VideoQualityMapping.get(key) != 1) {
                return false;
            }
        }
        return true;
    }

    private void refreshSelectProfileList(ArrayList<VideoProfileModel> list) {
        selectProfileList.clear();
        selectqualityList.clear();
        resetVideoQualityMapping();
        for (VideoProfileModel profileModel : list) {
            if (profileModel.isSelected()) {
                selectProfileList.add(profileModel);
                if (!profileModel.getQuality().equals("")) {
                    selectqualityList.add(new VideoQualityModel(profileModel
                            .getQuality(), profileModel.getName(), profileModel
                            .getProfile(), profileModel.getLevel()));
                    Elog.d(TAG, "[refreshSelectProfileList]getQuality() key: "
                            + profileModel.getQuality()
                            + " value: "
                            + VideoQualityMapping.get(profileModel.getQuality())) ;
                    VideoQualityMapping
                            .put(profileModel.getQuality(), VideoQualityMapping
                                    .get(profileModel.getQuality()) + 1);
                }
            }
        }
        if (!checkSelectQualityList(selectqualityList)) {
            selectProfileList.clear();
            selectqualityList.clear();
            resetVideoQualityMapping();
            showInfoDialog(getActivity(), getActivity().getResources()
                    .getString(R.string.vilte_quality_size_error));
        }
    }

    public void updateVideoProfileUI() {
        if (profileAdapter == null) {
            profileAdapter = new VideoProfileAdapter(getActivity(),
                    profileModelList);
            videoProfileListView.setAdapter(profileAdapter);
        }
        profileAdapter.notifyDataSetChanged();
    }

    private void refreshVideoProfileView(boolean isShow) {
        // TODO Auto-generated method stub
        if (isShow) {
            updateVideoProfileUI();
            videoProfileContent.setVisibility(View.VISIBLE);
        } else {
            videoProfileContent.setVisibility(View.GONE);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        View view = inflater.inflate(R.layout.iot_vilte_config, container,
                false);

        initView(view);
        return view;
    }

    private void initView(View view) {
        // TODO Auto-generated method stub
        resetVideoQualityMapping(view);
        selectProfileList = new ArrayList<VideoProfileModel>();
        profileModelList = new ArrayList<VideoProfileModel>();
        selectqualityList = new ArrayList<VideoQualityModel>();
        videoProfileContent = (LinearLayout) view
                .findViewById(R.id.video_profile_content);
        btnShowHelp = (Button) view.findViewById(R.id.btn_show_help);
        btnShowHelp.setOnClickListener(btnOnClickListener);
        operatorId = (EditText) view.findViewById(R.id.operator_id_input);
        btnProfileSave = (Button) view.findViewById(R.id.btn_profile_save);
        btnProfileSave.setOnClickListener(btnOnClickListener);
        btnProfileCancel = (Button) view.findViewById(R.id.btn_profile_cancel);
        btnProfileCancel.setOnClickListener(btnOnClickListener);
        btnProfileReset = (Button) view.findViewById(R.id.btn_profile_reset);
        btnProfileReset.setOnClickListener(btnOnClickListener);
        videoProfileListView = (ListView) view
                .findViewById(R.id.video_profile_list);
        videoProfileListView.setAdapter(profileAdapter);
        profileTitleScrollView = (HorizontalScrollView) view
                .findViewById(R.id.video_profile_title);
        profileContentScrollView = (LinkedHorizontalScrollView) view
                .findViewById(R.id.video_profile_view);
        combination(videoProfileListView, profileTitleScrollView,
                profileContentScrollView);
        refreshVideoProfileView(true);

    }

    public void resetVideoQualityMapping(View view) {
        VideoQualityMapping.clear();
        VideoQualityMapping
                .put(view.getResources().getStringArray(
                        R.array.video_quality_mode)[1], 0);
        VideoQualityMapping
                .put(view.getResources().getStringArray(
                        R.array.video_quality_mode)[2], 0);
        VideoQualityMapping
                .put(view.getResources().getStringArray(
                        R.array.video_quality_mode)[3], 0);
        VideoQualityMapping
                .put(view.getResources().getStringArray(
                        R.array.video_quality_mode)[4], 0);
    }

    public void resetVideoQualityMapping() {
        for (String key : VideoQualityMapping.keySet()) {
            VideoQualityMapping.put(key, 0);
        }
    }

    public class XmlParserThread extends Thread {
        private int action;

        public XmlParserThread(int action) {
            this.action = action;
        }

        public void run() {
            Message msg = new Message();
            switch (this.action) {
            case VILTE_ACTION_GET_DEFAULT_PROFILE:
                curFileName = "";
                XmlPullParserFactory factory;
                try {
                    factory = XmlPullParserFactory.newInstance();
                    factory.setNamespaceAware(true);
                    XmlPullParser parser = factory.newPullParser();
                    boolean isSupported = false;
                    if (new File(getInnerSDCardPath() + File.separator
                            + VILTE_DEFAULT_FILE_NAME).exists()) {
                        curFileName = getInnerSDCardPath() + File.separator
                                + VILTE_DEFAULT_FILE_NAME;
                        isSupported = true;
                    } else if (new File(getInnerSDCardPath() + File.separator
                            + VILTE_DEFAULT_TEMPLATE_FILE_NAME).exists()) {
                        curFileName = getInnerSDCardPath() + File.separator
                                + VILTE_DEFAULT_TEMPLATE_FILE_NAME;
                    }
                    if (!curFileName.equals("")) {
                        FileInputStream fis = new FileInputStream(curFileName);
                        Elog.d(TAG, "Read from" + curFileName);
                        parser.setInput(new InputStreamReader(fis));
                    } else {
                        curFileName = "vilte_media_profiles_opxx.xml";
                        parser.setInput(new InputStreamReader(getActivity()
                                .getApplicationContext().getAssets()
                                .open(curFileName)));
                        Elog.d(TAG, "Read from "
                                + "R.xml.vilte_media_profiles_opxx");
                    }
                    if (getXMLContent(parser, isSupported)) {
                        msg.what = VILTE_READ_DEFAULT_PROFILE_OK;
                    } else {
                        msg.what = VILTE_READ_DEFAULT_PROFILE_FAILED;
                    }
                } catch (XmlPullParserException e) {
                    msg.what = VILTE_READ_DEFAULT_PROFILE_FAILED;
                    e.printStackTrace();
                } catch (FileNotFoundException e) {
                    msg.what = VILTE_READ_DEFAULT_PROFILE_FAILED;
                    e.printStackTrace();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    msg.what = VILTE_READ_DEFAULT_PROFILE_FAILED;
                    e.printStackTrace();
                }
                break;
            case VILTE_ACTION_WRITE_PROFILE:
                String opId = operatorId.getText().toString();
                if (selectProfileList.size() == 0
                        || selectqualityList.size() == 0) {
                    msg.what = VILTE_WRITE_PROFILE_FAILED;
                } else {
                    try {
                        if (opId != null && !opId.replace(" ", "").equals("")) {
                            if (isNumeric(opId)) {
                                opId = String.valueOf(Integer.parseInt(opId));
                            }
                        }
                        Elog.d(TAG, "Write  " +  curFileName);
                        BufferedReader br;
                        if (curFileName.equals("vilte_media_profiles_opxx.xml")) {
                            InputStreamReader tempInputStream = new InputStreamReader(
                                    getActivity().getApplicationContext()
                                            .getAssets().open(curFileName));
                            br = new BufferedReader(tempInputStream);
                        } else {
                            File tempFile = new File(curFileName);
                            br = new BufferedReader(new FileReader(tempFile));
                        }
                        ArrayList<String> lines = new ArrayList<String>();
                        String readline = "";
                        while ((readline = br.readLine()) != null) {
                            lines.add(readline);
                        }
                        boolean isProfileWrited = false;
                        boolean isQualityWrited = false;
                        StringBuffer sb = new StringBuffer();
                        String profileStart = "";
                        String qualityStart = "";
                        File newFile = new File(getInnerSDCardPath()
                                + File.separator + VILTE_DEFAULT_FILE_NAME);
                        BufferedWriter bw = new BufferedWriter(new FileWriter(
                                newFile, false));
                        for (String line : lines) {
                            if (line.contains("<VideoProfile name=")) {
                                profileStart = line.split("name")[0];
                            } else {
                                if (!isProfileWrited
                                        && !profileStart.equals("")) {
                                    for (VideoProfileModel model : selectProfileList) {
                                        bw.write(profileStart + "name=\""
                                                + model.getName() + "\""
                                                + " profile=\""
                                                + model.getProfile() + "\""
                                                + " level=\""
                                                + model.getLevel() + "\""
                                                + " width=\""
                                                + model.getWidth() + "\""
                                                + " height=\""
                                                + model.getHeight() + "\""
                                                + " framerate=\""
                                                + model.getFramerate() + "\""
                                                + " Iinterval=\""
                                                + model.getIinterval() + "\""
                                                + " minBitRate=\""
                                                + model.getMinBitRate() + "\""
                                                + " maxBitRate=\""
                                                + model.getMaxBitRate()
                                                + "\"/>");
                                        bw.write("\r\n");
                                        bw.flush();
                                    }
                                    isProfileWrited = true;
                                }
                                if (line.contains("<VideoQuality name=")) {
                                    qualityStart = line.split("name")[0];
                                } else {
                                    if (!isQualityWrited
                                            && !qualityStart.equals("")) {
                                        for (VideoQualityModel model : selectqualityList) {
                                            bw.write(qualityStart + "name=\""
                                                    + model.getName() + "\""
                                                    + " format=\""
                                                    + model.getFormat() + "\""
                                                    + " profile=\""
                                                    + model.getProfile() + "\""
                                                    + " level=\""
                                                    + model.getLevel() + "\"/>");
                                            bw.write("\r\n");
                                            bw.flush();
                                        }
                                        isQualityWrited = true;
                                    }
                                    bw.write(line);
                                    bw.write("\r\n");
                                    bw.flush();
                                }
                            }

                        }
                        msg.what = VILTE_WRITE_PROFILE_OK;
                        Elog.d(TAG, "Write success:" + curFileName);
                    } catch (Exception e) {
                        e.printStackTrace();
                        msg.what = VILTE_WRITE_PROFILE_FAILED;
                        Elog.d(TAG, "Read failed:" + curFileName);
                    }
                }
                break;
            default:
                msg.what = UNKNOW;
                break;
            }
            mHandler.sendMessage(msg);
        }
    }

    private void combination(final ListView lvDetail,
            final HorizontalScrollView title, LinkedHorizontalScrollView content) {

        content.setMyScrollChangeListener(
                new LinkedHorizontalScrollView.LinkScrollChangeListener() {
            @Override
            public void onscroll(LinkedHorizontalScrollView view, int x, int y,
                    int oldx, int oldy) {
                title.scrollTo(x, y);
            }
        });

    }

    private boolean getXMLContent(XmlPullParser parser, boolean supported) {

        try {
            profileModelList.clear();
            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                switch (eventType) {
                case XmlPullParser.START_TAG:
                    if ("VideoProfile".equals(parser.getName())) {
                        VideoProfileModel profileModel = new VideoProfileModel(
                                parser.getAttributeValue(null, "name"),
                                parser.getAttributeValue(null, "profile"),
                                parser.getAttributeValue(null, "level"),
                                parser.getAttributeValue(null, "width"),
                                parser.getAttributeValue(null, "height"),
                                parser.getAttributeValue(null, "framerate"),
                                parser.getAttributeValue(null, "Iinterval"),
                                parser.getAttributeValue(null, "minBitRate"),
                                parser.getAttributeValue(null, "maxBitRate"),
                                supported);
                        profileModelList.add(profileModel);
                    } else if ("VideoQuality".equals(parser.getName())) {
                        for (VideoProfileModel model : profileModelList) {
                            if (model.getName().equals(
                                    parser.getAttributeValue(null, "format"))
                                    && model.getProfile().equals(
                                            parser.getAttributeValue(null,
                                                    "profile"))
                                    && model.getLevel().equals(
                                            parser.getAttributeValue(null,
                                                    "level"))) {
                                model.setQuality(parser.getAttributeValue(null,
                                        "name"));
                            }
                        }
                    }
                    break;
                case XmlPullParser.END_TAG:
                    break;
                case XmlPullParser.TEXT:
                    break;
                default:
                    break;
                }
                eventType = parser.next();
            }
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } catch (XmlPullParserException e) {
            e.printStackTrace();
            return false;
        }
    }

    @Override
    public void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        new XmlParserThread(VILTE_ACTION_GET_DEFAULT_PROFILE).start();
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();
    }

}

class VideoProfileAdapter extends ArrayAdapter<VideoProfileModel> {

    private ArrayList<VideoProfileModel> list;
    private final Activity context;
    int listPosititon;

    public VideoProfileAdapter(Activity context,
            ArrayList<VideoProfileModel> list) {
        super(context, R.layout.iot_video_profile_row, list);
        this.context = context;
        this.list = list;
    }

    public void refresh(ArrayList<VideoProfileModel> list) {
        this.list = list;
        notifyDataSetChanged();
    }

    public ArrayList<VideoProfileModel> getList() {
        return this.list;
    }

    static class ViewHolder {
        protected CheckBox checkbox;
        protected TextView quality;
        protected TextView name;
        protected TextView profile;
        protected TextView level;
        protected TextView width;
        protected TextView height;
        protected TextView framerate;
        protected TextView Iinterval;
        protected TextView minBitRate;
        protected TextView maxBitRate;
    }

    public boolean checkNumeric(String str) {
        Pattern pattern = Pattern.compile("[0-9]*");
        if (!str.equals("") && pattern.matcher(str).matches()
                && Integer.parseInt(str) > 0) {
            return true;
        }
        return false;
    }

    public void showDialog(final View view) {
        // Toast.makeText(context, content, Toast.LENGTH_SHORT).show();
        LayoutInflater li = LayoutInflater.from(context);
        View dialogView = li.inflate(R.layout.iot_et_dialog, null);

        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                context);

        alertDialogBuilder.setView(dialogView);

        final EditText userInput = (EditText) dialogView
                .findViewById(R.id.et_dialog_input);
        userInput.setText(((TextView) view).getText());
        alertDialogBuilder
                .setCancelable(false)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        boolean isLegal = true;
                        String newValue = userInput.getText().toString().trim()
                                .replaceFirst("^0*", "");
                        if (checkNumeric(newValue)) {
                            switch (view.getId()) {
                            case R.id.video_profile_Iinterval:
                                list.get((Integer) view.getTag()).setIinterval(
                                        newValue);
                                break;
                            case R.id.video_profile_width:
                                list.get((Integer) view.getTag()).setWidth(
                                        newValue);
                                break;
                            case R.id.video_profile_height:
                                list.get((Integer) view.getTag()).setHeight(
                                        newValue);
                                break;
                            case R.id.video_profile_maxBitRate:
                                if (Integer.valueOf(newValue) < Integer
                                        .valueOf(list.get(
                                                (Integer) view.getTag())
                                                .getMinBitRate())) {
                                    isLegal = false;
                                } else {
                                    list.get((Integer) view.getTag())
                                            .setMaxBitRate(newValue);
                                }
                                break;
                            case R.id.video_profile_minBitRate:
                                if (Integer.valueOf(newValue) > Integer
                                        .valueOf(list.get(
                                                (Integer) view.getTag())
                                                .getMaxBitRate())) {
                                    isLegal = false;
                                } else {
                                    list.get((Integer) view.getTag())
                                            .setMinBitRate(newValue);
                                }
                                break;
                            case R.id.video_profile_framerate:
                                list.get((Integer) view.getTag()).setFramerate(
                                        newValue);
                                break;
                            default:
                                break;
                            }
                        } else {
                            isLegal = false;
                        }
                        if (isLegal) {
                            ((TextView) view).setText(newValue);
                        } else {
                            Toast.makeText(
                                    context,
                                    context.getResources().getString(
                                            R.string.vilte_input_illegal),
                                    Toast.LENGTH_SHORT).show();
                        }
                    }
                })
                .setNegativeButton("Cancel",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                dialog.cancel();
                            }
                        });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    public void showSpinnerDialog(final View view) {
        LayoutInflater li = LayoutInflater.from(context);
        View dialogView = li.inflate(R.layout.iot_spinner_dialog, null);

        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                context);

        alertDialogBuilder.setView(dialogView);

        final Spinner userInput = (Spinner) dialogView
                .findViewById(R.id.spinner_dialog_input);

        final CharSequence[] selectArray = context.getResources()
                .getStringArray(R.array.video_quality_mode);
        ArrayAdapter<CharSequence> arrayAdapter = new ArrayAdapter<CharSequence>(
                context, android.R.layout.simple_spinner_item, selectArray);
        arrayAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        userInput.setAdapter(arrayAdapter);
        alertDialogBuilder
                .setCancelable(false)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        if (userInput.getSelectedItem() != null) {
                            String newValue = userInput.getSelectedItem()
                                    .toString();
                            switch (view.getId()) {
                            case R.id.video_quality:
                                int mPosition = (Integer) view.getTag();
                                if (!list.get(mPosition).isSelected()
                                        && !newValue.equals("")) {
                                    Toast.makeText(
                                            context,
                                            context.getResources()
                                                    .getString(
                                                            R.string.vilte_quality_false),
                                            Toast.LENGTH_SHORT).show();
                                } else {
                                    ((TextView) view).setText(newValue);
                                    list.get(mPosition).setQuality(newValue);
                                }
                                break;
                            default:
                                break;
                            }
                        }

                    }
                })
                .setNegativeButton("Cancel",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                dialog.cancel();
                            }
                        });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        listPosititon = position;
        ViewHolder viewHolder = null;
        if (convertView == null) {
            OnClickListener mOnClickListener = new OnClickListener() {

                @Override
                public void onClick(View view) {
                    // TODO Auto-generated method stub
                    if (view.getId() == R.id.video_quality) {
                        showSpinnerDialog(view);
                    } else if (view.getId() == R.id.video_profile_name
                            || view.getId() == R.id.video_profile_profile
                            || view.getId() == R.id.video_profile_level) {
                        Toast.makeText(
                                context,
                                ((TextView) view).getText()
                                        + " can't be modified",
                                Toast.LENGTH_SHORT).show();
                    } else {
                        showDialog(view);
                    }
                }
            };
            LayoutInflater inflator = context.getLayoutInflater();
            convertView = inflator
                    .inflate(R.layout.iot_video_profile_row, null);
            viewHolder = new ViewHolder();
            viewHolder.checkbox = (CheckBox) convertView
                    .findViewById(R.id.video_profile_check);
            viewHolder.name = (TextView) convertView
                    .findViewById(R.id.video_profile_name);
            viewHolder.quality = (TextView) convertView
                    .findViewById(R.id.video_quality);
            viewHolder.profile = (TextView) convertView
                    .findViewById(R.id.video_profile_profile);
            viewHolder.level = (TextView) convertView
                    .findViewById(R.id.video_profile_level);
            viewHolder.Iinterval = (TextView) convertView
                    .findViewById(R.id.video_profile_Iinterval);
            viewHolder.width = (TextView) convertView
                    .findViewById(R.id.video_profile_width);
            viewHolder.height = (TextView) convertView
                    .findViewById(R.id.video_profile_height);
            viewHolder.maxBitRate = (TextView) convertView
                    .findViewById(R.id.video_profile_maxBitRate);
            viewHolder.minBitRate = (TextView) convertView
                    .findViewById(R.id.video_profile_minBitRate);
            viewHolder.framerate = (TextView) convertView
                    .findViewById(R.id.video_profile_framerate);
            convertView.setTag(viewHolder);
            convertView.setTag(R.id.video_profile_check, viewHolder.checkbox);
            convertView.setTag(R.id.video_quality, viewHolder.quality);
            convertView.setTag(R.id.video_profile_name, viewHolder.name);
            convertView.setTag(R.id.video_profile_profile, viewHolder.profile);
            convertView.setTag(R.id.video_profile_level, viewHolder.level);
            convertView.setTag(R.id.video_profile_framerate,
                    viewHolder.framerate);
            convertView.setTag(R.id.video_profile_width, viewHolder.profile);
            convertView.setTag(R.id.video_profile_height, viewHolder.height);
            convertView.setTag(R.id.video_profile_Iinterval,
                    viewHolder.Iinterval);
            convertView.setTag(R.id.video_profile_maxBitRate,
                    viewHolder.maxBitRate);
            convertView.setTag(R.id.video_profile_minBitRate,
                    viewHolder.minBitRate);
            viewHolder.checkbox
                    .setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

                        @Override
                        public void onCheckedChanged(CompoundButton buttonView,
                                boolean isChecked) {
                            int getPosition = (Integer) buttonView.getTag();
                            list.get(getPosition).setSelected(
                                    buttonView.isChecked());
                        }
                    });
            viewHolder.name.setOnClickListener(mOnClickListener);
            viewHolder.profile.setOnClickListener(mOnClickListener);
            viewHolder.level.setOnClickListener(mOnClickListener);
            viewHolder.quality.setOnClickListener(mOnClickListener);
            viewHolder.framerate.setOnClickListener(mOnClickListener);
            viewHolder.minBitRate.setOnClickListener(mOnClickListener);
            viewHolder.maxBitRate.setOnClickListener(mOnClickListener);
            viewHolder.height.setOnClickListener(mOnClickListener);
            viewHolder.width.setOnClickListener(mOnClickListener);
            viewHolder.Iinterval.setOnClickListener(mOnClickListener);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }
        viewHolder.checkbox.setTag(position); // This line is important.
        viewHolder.quality.setTag(position);
        viewHolder.framerate.setTag(position);
        viewHolder.minBitRate.setTag(position);
        viewHolder.maxBitRate.setTag(position);
        viewHolder.height.setTag(position);
        viewHolder.width.setTag(position);
        viewHolder.Iinterval.setTag(position);
        viewHolder.name.setTag(position);
        viewHolder.profile.setTag(position);
        viewHolder.level.setTag(position);

        viewHolder.name.setText(list.get(position).getName());
        viewHolder.checkbox.setChecked(list.get(position).isSelected());
        viewHolder.quality.setText(list.get(position).getQuality());
        viewHolder.profile.setText(list.get(position).getProfile());
        viewHolder.level.setText(list.get(position).getLevel());
        viewHolder.width.setText(list.get(position).getWidth());
        viewHolder.height.setText(list.get(position).getHeight());
        viewHolder.Iinterval.setText(list.get(position).getIinterval());
        viewHolder.maxBitRate.setText(list.get(position).getMaxBitRate());
        viewHolder.minBitRate.setText(list.get(position).getMinBitRate());
        viewHolder.framerate.setText(list.get(position).getFramerate());
        return convertView;
    }

}
