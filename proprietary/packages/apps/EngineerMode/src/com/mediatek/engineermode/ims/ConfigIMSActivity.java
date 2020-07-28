package com.mediatek.engineermode.ims;

import android.app.Activity;
import android.content.Intent;
import android.content.res.XmlResourceParser;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 *
 */
public class ConfigIMSActivity extends Activity implements OnItemClickListener {
    private static final String TAG = "Ims/ImsConfig";

    private String mCategory = null;
    private ArrayList<Setting> mSettings = new ArrayList<Setting>();
    private ListView mList;
    private Toast mToast;
    private String mSettingRule =
            "Setting Rule:<digit of list num><list num><mnc_len><MNC><mcc_len><MCC>...";
    private List<String> mlabelName = new ArrayList<String>();

    private ArrayList<Setting> mSettingDisplay;
    private int mSimType;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ims_config);
        mCategory = getIntent().getStringExtra("category");
        mSimType = getIntent().getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        setTitle(mCategory);
        mList = (ListView) findViewById(R.id.ims_item_list);

        getXMLContent(getResources().getXml(R.xml.ims_config));
        String[] tmp = mlabelName.toArray(new String[mlabelName.size()]);
        Arrays.sort(tmp);
        mlabelName = Arrays.asList(tmp);
        initializeViews();
    }

    private void setListViewItemsHeight(ListView listview) {
        if (listview == null) {
            return;
        }
        ListAdapter adapter = listview.getAdapter();
        int totalHeight = 0;
        for (int i = 0; i < adapter.getCount(); i++) {
            View itemView = adapter.getView(i, null, listview);
            itemView.measure(0, 0);
            totalHeight += itemView.getMeasuredHeight();
        }
        totalHeight += (adapter.getCount() - 1) * listview.getDividerHeight();
        ViewGroup.LayoutParams params = listview.getLayoutParams();
        params.height = totalHeight;
        listview.setLayoutParams(params);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    private int parseInt(String s) {
        try {
            return Integer.parseInt(s);
        } catch (NumberFormatException e) {
            showToast("Wrong integer format: " + s);
            return -1;
        }
    }

    private void getXMLContent(XmlResourceParser parser) {
        Setting setting = new Setting();
        String text = "";
        String category = "";
        try {
            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                switch (eventType) {
                    case XmlPullParser.START_TAG:
                        if ("setting".equals(parser.getName())) {
                            setting = new Setting();
                            category = "";
                        } else if ("option".equals(parser.getName())) {
                            setting.getEntries().add(parser.getAttributeValue(null, "name"));
                            setting.getValues().add(parseInt(parser.getAttributeValue(null,
                                    "value")));
                        }
                        text = "";
                        break;
                    case XmlPullParser.END_TAG:
                        String name = parser.getName();
                        if ("label".equals(name)) {
                            setting.setLabel(text);
                        } else if ("suffix".equals(name)) {
                            setting.setSuffix(text);
                        } else if ("category".equals(name)) {
                            category = text;
                        } else if ("type".equals(name)) {
                            setting.setType(parseInt(text));
                        } else if ("default".equals(name)) {
                            setting.setDefaultValue(text);
                        } else if ("setting".equals(name)) {
                            if (mCategory.equals(category)) {
                                mSettings.add(setting);
                                if (!(setting.label.equals("white list")) &&
                                        !(setting.label.equals(mSettingRule)) &&
                                        !(setting.label.equals("operator_code_textview"))) {
                                    mlabelName.add(setting.label);
                                }
                            }
                        }
                        break;
                    case XmlPullParser.TEXT:
                        text = parser.getText();
                        break;
                    default:
                        break;
                }
                eventType = parser.next();
            }
        } catch (IOException e) {
            Elog.e(TAG, "");
        } catch (XmlPullParserException e) {
            Elog.e(TAG, "");
        }
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        Intent intent = new Intent(this, imsSettingPageActivity.class);
        TextView textView = (TextView) arg1;
        mSettingDisplay = new ArrayList<Setting>();
        mSettingDisplay.add(getSetting(textView.getText().toString()));
        if (textView.getText().equals("mncmcc check")) {
            mSettingDisplay.add(getSetting("white list"));
            mSettingDisplay.add(getSetting(mSettingRule));
        } else if (textView.getText().equals("force_user_account_by_manual")) {
            mSettingDisplay.add(getSetting("manual_impi"));
            mSettingDisplay.add(getSetting("manual_impu"));
            mSettingDisplay.add(getSetting("manual_domain_name"));
        } else if (textView.getText().equals("operator_code")) {
            mSettingDisplay.add(getSetting("operator_code_textview"));
        }
        intent.putExtra("mSettingDisplay", mSettingDisplay);
        intent.putExtra("mSimType", mSimType);
        startActivity(intent);
    }

    private void initializeViews() {
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_list_item_1, mlabelName);
        mList.setAdapter(adapter);
        mList.setOnItemClickListener(this);
        setListViewItemsHeight(mList);
    }

    private Setting getSetting(String label) {
        Elog.d(TAG, "the label is" + label);
        for (Setting setting : mSettings) {
            Elog.d(TAG, "the label in setting is" + setting.getLabel());
            if (setting.getLabel().equals(label)) {
                return setting;
            }
        }
        return null;
    }
}
