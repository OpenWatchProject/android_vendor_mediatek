package com.mediatek.lbs.em2.ui;

import android.location.GnssStatus;
import android.content.Context;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Date;

public class LbsUtility {

    //return true: success
    static public boolean write2File(Context context, String folder, String subFolder,
            String fileName, String data, boolean isAppendMode) {
        DataOutputStream dos;
        String fullFileName;

        File sdFile;
        File dirFile;
        fullFileName = "/sdcard" + File.separator + folder + File.separator;
        // Log.d("LocationEM", "Dump file path: " + fullFileName);

        dirFile = new File(fullFileName);
        if(!dirFile.exists()){
            dirFile.mkdirs();
        }

        if (subFolder != null) {
            fullFileName = fullFileName + "/" + subFolder;
            dirFile = new File(fullFileName);
            if(!dirFile.exists()){
                dirFile.mkdirs();
            }
        }

        fullFileName += "/" + fileName;
        dirFile = new File(fullFileName);
        try {
            if (isAppendMode)
                dos = new DataOutputStream(new FileOutputStream(fullFileName, true));
            else
                dos = new DataOutputStream(new FileOutputStream(dirFile));
            dos.writeBytes(data);
            dos.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    static public String getTimeString(long milliseconds) {
            Date date = new Date(milliseconds);
            String str = (date.getYear() + 1900) + "/" + (date.getMonth() + 1) + "/" + date.getDate() +
                    " " + date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds();
                    //+ "." + String.format("%03d", milliseconds%1000);
            return str;
        }

    static public String getTimeString2(long milliseconds) {
        Date date = new Date(milliseconds);
        String str = String.format("%04d%02d%02d_%02d%02d%02d",
                (date.getYear() + 1900),
                (date.getMonth() + 1),
                date.getDate(),
                date.getHours(),
                date.getMinutes(),
                date.getSeconds());
        return str;
    }

    static public String getTimeString3(long milliseconds) {
        Date date = new Date(milliseconds);
        String str = String.format("%02d/%02d/%04d %02d:%02d:%02d",
                (date.getMonth() + 1),
                date.getDate(),
                (date.getYear() + 1900),
                date.getHours(),
                date.getMinutes(),
                date.getSeconds());
        return str;
    }

    static public String[] split(String str) {
        String[] result = null;
        try {
            String delims = "[,]";
            result = str.split(delims);
        } catch (Exception e) {
            Log.d("nmeaParser", "split:" + e);
        }
        return result;
    }


    static public int parseInt(String str) {
        int d = 0;
        if (str.equals("")) {
            return d;
        }
        String mStr = removeFirstZero(str);
        try
        {
            d = Integer.valueOf(mStr);
        }
        catch (Exception e) {
            Log.d("nmeaParser", "parseDouble:" + e);
        }
        return d;
    }

    static private String removeFirstZero(String record) {
        String result = record;
        int ind = 0;
        while (record.charAt(ind++) == '0') {
            if (ind == record.length())
                break;
        }
        if (ind != 0) {
            result = record.substring(--ind);
        }
        return result;
    }

    /**
     * The band information is as of 2018, per http://www.navipedia.net/index.php/GNSS_signal
     * Bands are combined for simplicity as the constellation is also tracked.
     *
     * @param frequencyHz Frequency in Hz
     * @return GnssBand where the frequency lies.
     */
    static private enum GnssBand {
        GNSS_L1,
        GNSS_L2,
        GNSS_L5,
        GNSS_E6
    }

    static private GnssBand frequencyToGnssBand(float frequencyHz) {
        float frequencyMhz = frequencyHz/1e6F;
        if (frequencyMhz >= 1151 && frequencyMhz <= 1214) {
            return GnssBand.GNSS_L5;
        }
        if (frequencyMhz > 1214 && frequencyMhz <= 1255) {
            return GnssBand.GNSS_L2;
        }
        if (frequencyMhz > 1255 && frequencyMhz <= 1300) {
            return GnssBand.GNSS_E6;
        }
        return GnssBand.GNSS_L1; // default to L1 band
    }

    static public String getFrequencyBand(float frequencyHz) {
        GnssBand band = frequencyToGnssBand(frequencyHz);
        String str = "UN"; // unknown
        switch(band) {
            case GNSS_L1:
                str = "L1";
                break;
            case GNSS_L2:
                str = "L2";
                break;
            case GNSS_L5:
                str = "L5";
                break;
            case GNSS_E6:
                str = "E6";
                break;
        }
        return str;
    }

    static String map2SvTypes(int type) {
        String outType = "";
        if (type == GnssStatus.CONSTELLATION_GPS) {
            outType = "G";
        } else if (type == GnssStatus.CONSTELLATION_SBAS) {
            outType = "S";
        } else if (type == GnssStatus.CONSTELLATION_GLONASS) {
            outType = "R";
        } else if (type == GnssStatus.CONSTELLATION_QZSS) {
            outType = "Q";
        } else if (type == GnssStatus.CONSTELLATION_BEIDOU) {
            outType = "B";
        } else if (type == GnssStatus.CONSTELLATION_GALILEO) {
            outType = "E";
        } else if (type == GnssStatus.CONSTELLATION_IRNSS) {
            outType = "I";
        }
        return outType;
    }
}
