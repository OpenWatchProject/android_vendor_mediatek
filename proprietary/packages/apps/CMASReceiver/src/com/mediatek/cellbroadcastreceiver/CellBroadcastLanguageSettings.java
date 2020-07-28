package com.mediatek.cellbroadcastreceiver;

import java.util.ArrayList;
import java.util.HashMap;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.widget.ListView;
import android.widget.Toast;

import mediatek.telephony.MtkSmsManager;

public class CellBroadcastLanguageSettings {
    private static final String TAG = "[CMAS][CellBroadcastLanguageSettings]";
    protected Context mContext;
    private static CellBroadcastLanguageSettings sInstance = null;
    private int mSubId;
    private static final int LANGUAGE_NUM = 22; // the number of language,include "All languages"
    private ArrayList<CellBroadcastLanguage> mLanguageList =
        new ArrayList<CellBroadcastLanguage>();
    private HashMap<String, CellBroadcastLanguage> mLanguageMap;

    public CellBroadcastLanguageSettings(Context ctx, int slotId) {
        mContext = ctx;
        final int[] subIds = SubscriptionManager.getSubId(slotId);
        if (subIds != null && subIds.length > 0) {
            mSubId = subIds[0];
            Log.d(TAG, "valid mSubId = " + mSubId);
        }
    }

    public class CellBroadcastLanguage {
        private int mLanguageId;
        private String mLanguageName;
        private boolean mLanguageState;
        public CellBroadcastLanguage(int id, String name, boolean state) {
            mLanguageId = id;
            mLanguageName = name;
            mLanguageState = state;
        }

        public int getLanguageId() {
            return mLanguageId;
        }

        public String getLanguageName() {
            return mLanguageName;
        }

        public boolean getLanguageState() {
            return mLanguageState;
        }

        public void setLanguageState(boolean state) {
            mLanguageState = state;
        }
    }

    public void showLanguageSelectDialog(int slotId) {
        final boolean[] temp = new boolean[LANGUAGE_NUM];
        final boolean[] temp2 = new boolean[LANGUAGE_NUM];
        boolean allLanguagesFlag = true;
        for (int i = 1; i < temp.length; i++) {
            CellBroadcastLanguage tLanguage = mLanguageList.get(i);
            if (tLanguage != null) {
                Log.d(TAG, "language status " + tLanguage.getLanguageState());
                temp[i] = tLanguage.getLanguageState();
                temp2[i] = tLanguage.getLanguageState();
            } else {
                Log.w(TAG,
                        "showLanguageSelectDialog() init the language list failed when i=" + i);
            }
            if (!temp[i]) {
                allLanguagesFlag = false; // not select all languages
            }
        }
        // init "All Languages" selection
        Log.d(TAG, "All language status " + allLanguagesFlag);
        mLanguageList.get(0).setLanguageState(allLanguagesFlag);
        temp[0] = allLanguagesFlag;
        temp2[0] = allLanguagesFlag;
        final AlertDialog.Builder dlgBuilder = new AlertDialog.Builder(mContext);
        dlgBuilder.setTitle(mContext.getString(R.string.cb_dialog_title_language_choice));
        dlgBuilder.setPositiveButton(R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        int tLength = temp.length;
                        //if select "all languages"
                        if (temp[0]) {
                            for (int i = 0; i < tLength; i++) {
                                temp[i] = true;
                            }
                        }
                        // select a language at least
                        boolean flag = false;
                        for (int i = 0; i < tLength; i++) {
                            mLanguageList.get(i).setLanguageState(temp[i]);
                            if (temp[i]) {
                                flag = true;
                            }
                        }

                        if (flag) {
                            // need to update languages to the SMSManager & handle the response
                            //SmsBroadcastConfigInfo[] langList = makeLanguageConfigArray();
                            boolean isSetConfigSuccess = setLangauge();
                            Log.d(TAG, "isSetConfigSuccess = " + isSetConfigSuccess);
                            updateLanguagesFromModemConfig();
                        } else {
                            displayMessage(R.string.cb_error_language_select);
                            for (int i = 0; i < tLength; i++) {
                                mLanguageList.get(i).setLanguageState(temp2[i]);
                            }
                        }
                    }
                });
        dlgBuilder.setNegativeButton(R.string.cancel, null);
        DialogInterface.OnMultiChoiceClickListener multiChoiceListener
                = new DialogInterface.OnMultiChoiceClickListener() {
            public void onClick(DialogInterface dialog, int whichButton, boolean isChecked) {
                    temp[whichButton] = isChecked;
                    AlertDialog languageDialog = null;
                    if (dialog instanceof AlertDialog) {
                        languageDialog = (AlertDialog) dialog;
                    }
                    if (whichButton == 0) {
                        if (languageDialog != null) {
                            for (int i = 1; i < temp.length; ++i) {
                                ListView items = languageDialog.getListView();
                                items.setItemChecked(i, isChecked);
                                temp[i] = isChecked;
                            }
                        }
                    } else {
                        if ((!isChecked) && (languageDialog != null)) {
                            ListView items = languageDialog.getListView();
                            items.setItemChecked(0, isChecked);
                            temp[0] = false;
                        } else if (isChecked && (languageDialog != null)) {
                            /// M: ALPS00641361 @{
                            // if select all language, the first item should be checked
                            //
                            // MTK add
                            setCheckedAlllanguageItem(temp, isChecked, languageDialog);
                            /// @}
                        }
                    }
                }
            };
        dlgBuilder.setMultiChoiceItems(R.array.language_list_values, temp, multiChoiceListener);
        AlertDialog languageDialog = dlgBuilder.create();
        if (languageDialog != null) {
            languageDialog.show();
        }
    }

    private void setCheckedAlllanguageItem(
            final boolean[] temp, boolean isChecked, AlertDialog languageDialog) {
        boolean alllanguage = true;
        for (int i = 1; i < temp.length; ++i) {
            if (!temp[i]) {
                alllanguage = false;
                break;
            }
        }
        Log.d(TAG, "All language alllanguage " + alllanguage);
        if (alllanguage) {
            ListView items = languageDialog.getListView();
            items.setItemChecked(0, isChecked);
            temp[0] = true;
        }
    }

    private boolean setLangauge() {
        StringBuilder langList = new StringBuilder();
        boolean first = true;
        // ignore the select all as we need to send id for each individual language
        for (int i = 1; i < mLanguageList.size(); i++) {
            CellBroadcastLanguage language = (CellBroadcastLanguage) mLanguageList.get(i);
            CellBroadcastLanguage lang
                    = getLanguageObjectFromKey(String.valueOf(language.getLanguageId()));
            if (lang != null && lang.getLanguageState()) {
                if (first) {
                    langList.append(String.valueOf(language.getLanguageId()));
                    first = false;
                } else {
                    langList.append(",");
                    langList.append(String.valueOf(language.getLanguageId()));
                }
            }
        }
        Log.d(TAG, "setLangauge lang_list:" + langList.toString());
        return MtkSmsManager.getSmsManagerForSubscriptionId(mSubId)
                    .setCellBroadcastLang(langList.toString());
    }

    public void updateLanguagesFromModemConfig() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "updateLanguagesList start");
                updateLanguagesList();
                Log.d(TAG, "updateLanguagesList end");
            }
        }).start();
    }

    private void updateLanguagesList() {
        String langList = MtkSmsManager.
            getSmsManagerForSubscriptionId(mSubId).getCellBroadcastLang();
        Log.d(TAG, "updateLanguagesList langList:" + langList);
        // set all the languages to off first
        for (int i = 0; i < mLanguageList.size(); i++) {
            CellBroadcastLanguage language = (CellBroadcastLanguage) mLanguageList.get(i);
            CellBroadcastLanguage lang
                    = getLanguageObjectFromKey(String.valueOf(language.getLanguageId()));
            if (lang != null) {
                lang.setLanguageState(false);
            }
        }
        // update the languages returned from SMSManager as selected
        // all language will be set to true while displaying the list so no need to set here
        try {
            for (String lang : langList.split(",")) {
                CellBroadcastLanguage language = getLanguageObjectFromKey(lang);
                if (language != null) {
                    language.setLanguageState(true);
                }

            }
        } catch (NumberFormatException e) {
            Log.d(TAG, "NumberFormatException ", e);
        }
    }

    private CellBroadcastLanguage getLanguageObjectFromKey(String key) {
        return mLanguageMap.get(key);
    }

    public void initLanguage() {
        initLanguageList();
        initLanguageMap(); // Map(LanguageId,CellBroadcastLanguage)
    }

    private void initLanguageMap() {
        mLanguageMap = new HashMap<String, CellBroadcastLanguage>();
        for (int i = 0; i < LANGUAGE_NUM; i++) {
            CellBroadcastLanguage language = mLanguageList.get(i);
            if (language != null) {
                int id = language.getLanguageId();
                mLanguageMap.put(String.valueOf(id), language);
            }
        }
    }

    private void initLanguageList() {
        boolean[] languageEnable = new boolean[LANGUAGE_NUM];
        String[] languageId = new String[LANGUAGE_NUM];
        String[] languageName = new String[LANGUAGE_NUM];
        languageName = mContext.getResources().getStringArray(R.array.language_list_values);
        languageId = mContext.getResources().getStringArray(R.array.language_list_id);
        for (int i = 0; i < LANGUAGE_NUM; i++) {
            int id = Integer.valueOf(languageId[i]).intValue();
            String name = languageName[i];
            boolean enable = languageEnable[i];
            CellBroadcastLanguage language = new CellBroadcastLanguage(id, name, enable);
            mLanguageList.add(language);
        }
    }

    private void displayMessage(int strId) {
        Toast.makeText(mContext, mContext.getString(strId), Toast.LENGTH_SHORT).show();
    }
}
