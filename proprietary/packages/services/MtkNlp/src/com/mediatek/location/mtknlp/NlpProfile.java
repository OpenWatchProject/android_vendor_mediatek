package com.mediatek.location.mtknlp;

import android.util.Log;

public class NlpProfile {
    private String nlpName;
    private String packageName;

    public NlpProfile() {
        nlpName = "";
        packageName = "";
    }

    public String toString() {
        String ret = "";
        ret += "name=[" + nlpName + "] package=[" + packageName + "]";
        return ret;
    }

    public void setNlpName(String str) {
        nlpName = str;
    }

    public void setPackageName(String str) {
        packageName = str;
    }

    public String getNlpName() {
        return nlpName;
    }

    public String getPackageName() {
        return packageName;
    }
}
