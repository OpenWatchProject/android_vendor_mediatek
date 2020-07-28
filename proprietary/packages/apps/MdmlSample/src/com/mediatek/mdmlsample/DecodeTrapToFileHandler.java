package com.mediatek.mdmlsample;

import android.util.Log;

import com.mediatek.mdml.PlainDataDecoder;
import com.mediatek.mdml.TRAP_TYPE;
import com.mediatek.mdml.TrapHandlerInterface;

/**
 * Created by MTK03685 on 2015/10/2.
 */
public class DecodeTrapToFileHandler implements TrapHandlerInterface {
    private static final String TAG = "DecodeTrapToFileHandler";
    private PlainDataDecoder m_plainDataDecoder;  // trap decoder

    DecodeTrapToFileHandler(PlainDataDecoder trapDecoder){
        m_plainDataDecoder = trapDecoder;
    }

    public void ProcessTrap(long timestamp, TRAP_TYPE type, int len, byte[] data, int offset){
        Log.d(TAG, "frame is incoming: timestamp = [" + timestamp + "], type = [" + type.GetValue() + "], len = [" + len + "]");

        Log.d(TAG, "frame is incoming... done!");
    }

    private void DecodeTrapToFile(){

    }

    private void PushNewTrapArrival(){

    }
}
