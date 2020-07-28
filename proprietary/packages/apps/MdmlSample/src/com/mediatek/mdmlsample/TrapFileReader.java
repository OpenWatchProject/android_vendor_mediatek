package com.mediatek.mdmlsample;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

/**
 * Created by MTK03685 on 2015/10/7.
 */
public class TrapFileReader {
    private static final String TAG = "TrapFileReader";
    private BufferedReader m_fileReader;
    private int m_trapCount = 0;

    public TrapFileReader(Context context){
        String trapFilePath = context.getCacheDir().getAbsolutePath()
                + "/MDML_output.txt";
        // open read file
        try {
            m_fileReader = new BufferedReader(new FileReader(trapFilePath));
        }
        catch (Exception e){
            Log.e(TAG, "Read file open failed. path = " + trapFilePath);
        }
    }

    public StringBuilder GetLastTrapContext(){
        if(m_fileReader != null){
            m_trapCount++;
            try {
                StringBuilder text = new StringBuilder();
                String line;

                // move to the first line after "#*" syntax
                while(true){
                    line = m_fileReader.readLine();
                    if(line == null){
                        Log.d(TAG, "Check prefix is #* , line = null happen, so return empty");
                        return text;
                    }
                    Log.d(TAG, "Check prefix is #* , line = " + line);
                    if(line.startsWith("#*")) {
                        // record trap order here !!
                        // TODO: 2015/10/7
                        break;
                    }
                }

                // append line string into string builder until meat the line start with "*#*#" syntax
                while(true) {
                    line = m_fileReader.readLine();
                    if(line == null){
                         Log.d(TAG, "Check postfix is *#*# , line = null happen, so return empty");
                         text.setLength(0);
                         return text;
                    }
                    if(line.startsWith("*#*#")) {
                        // end of this trap, break
                        Log.d(TAG, "Trap end");
                        break;
                    }
                    text.append(line);
                    text.append('\n');
                }
                if(text.length() > 200){
                    text.setLength(200);
                }
                return text;
            }
            catch(IOException ioe)
            {
                Log.e(TAG, "File Read error");
                return new StringBuilder("Read File is failed!");
            }
        }else{
            return new StringBuilder("Read File is not open!");
        }
    }

    public int GetCurrentTrapOrder(){
        return m_trapCount;
    }


}
