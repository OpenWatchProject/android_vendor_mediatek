package com.mediatek.custom;

import android.content.res.Resources;
import android.util.Log;

import java.lang.reflect.Field;
import java.text.ParseException;
import java.text.ParsePosition;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class CustomPropInterface {
    private static String getBrowserVersion() {
        String result = null;
        try {
            Class rClass = Class.forName("com.android.internal.R$string");
            Field webUa = rClass.getDeclaredField("web_user_agent");
            int resID = webUa.getInt(null);

            String defaultAgent = Resources.getSystem().getText(resID).toString();
            Pattern pattern = Pattern.compile("\\s+AppleWebKit\\/(\\d+\\.?\\d*)\\s+");
            Matcher matcher = pattern.matcher(defaultAgent);

            if (matcher.find()) {
                Log.i("CustomProp", "getBrowserVersion->matcher.find:true"
                        + " matcher.group(0):" + matcher.group(0)
                        + " matcher.group(1):" + matcher.group(1));
                result = matcher.group(1);
            }
            else {
                Log.i("CustomProp", "getBrowserVersion->matcher.find:false");
            }
        } catch (java.lang.ClassNotFoundException e1) {
            // nothing to do
        } catch (java.lang.NoSuchFieldException e2) {
            // nothing to do
        } catch (java.lang.IllegalAccessException e3) {
            // nothing to do
        } catch (RuntimeException e) {
            // nothing to do
        }

        Log.i("CustomProp", "getBrowserVersion->result:" + result);

        return result;
    }

    private static String getReleaseDate(String buildDate) {
        Log.i("CustomProp", "getReleaseDate->buildDate[" + buildDate + "]");
        SimpleDateFormat format = new SimpleDateFormat("EEE MMM dd HH:mm:ss zzz yyyy", new Locale("en","CN"));
        Date date;
        String result;

        try {
            date = format.parse(buildDate);
        } catch (ParseException e) {
            date = null;
        }

        if (date != null) {
            Log.i("CustomProp", "date: " + date);
            Calendar calendar = format.getCalendar();
            result = String.format("%02d.%02d.%d", calendar.get(Calendar.MONTH)+1, calendar.get(Calendar.DATE), calendar.get(Calendar.YEAR));
        }
        else
            result = null;

        return result;
    }
}

