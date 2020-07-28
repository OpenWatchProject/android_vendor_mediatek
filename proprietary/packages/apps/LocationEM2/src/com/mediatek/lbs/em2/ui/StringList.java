package com.mediatek.lbs.em2.ui;

import java.util.Vector;

public class StringList {

    private Vector<String>  mStrList = new Vector<String>();
    private int mNum = 0;

    public StringList() {
        this(16);
    }

    public StringList(int stringNum) {
        mNum = stringNum;
    }

    public void add(String str) {
        if (mStrList.size() > mNum)
            mStrList.remove(0);
        mStrList.add(str);
    }

    public String get() {
        String result = new String();
        for (String str : mStrList)
            result += str;
        return result;
    }

    public void clear() {
        mStrList.clear();
    }

    public String toString() {
        String result = new String();
        for (String str: mStrList)
            result += str;
        return result;
    }
}
