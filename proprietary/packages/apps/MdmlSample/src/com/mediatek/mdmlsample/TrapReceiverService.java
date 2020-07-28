package com.mediatek.mdmlsample;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.mdml.MonitorTrapReceiver;
import com.mediatek.mdml.Msg;
import com.mediatek.mdml.PlainDataDecoder;
import com.mediatek.mdml.TRAP_TYPE;
import com.mediatek.mdml.TrapHandlerInterface;


import java.lang.ref.WeakReference;
import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.nio.ByteBuffer;


/**
 * Service that wrapped MonitorTrapReceiver and is responsible for handling trap.
 */
public class TrapReceiverService extends Service implements TrapHandlerInterface, TrapReceiverInitListener{
    private static final String TAG = "TrapReceiverService";
    MonitorTrapReceiver m_trapReceiver;//should be initialized after getting session ID
    private final IBinder mBinder = new LocalBinder();
    // Trap decoder objects
    private PlainDataDecoder m_plainDataDecoder;
    // Output file related objects
    OutDbgStr outputFile;
    int trapCount = 0;
    private int[] mOtaTraceId;

    @Override
    public void onCreate(){
        Log.d(TAG, "onCreate()");
        // Get MDML MonitorTrapReceiver objects and connection state from app object
        AppApplication app = (AppApplication) getApplicationContext();
        // Get trap decoder object from app object
        m_plainDataDecoder = app.GetPlainDataDecoder();
        // create output file (use OutDbgStr)
        String filePath = getCacheDir().getAbsolutePath() + "/MDML_output.txt";
        outputFile = new OutDbgStr(filePath);
    }

    @Override
    public int onStartCommand(Intent intenr, int flags, int startID){
        Toast.makeText(this, "TrapReceiverService Start", Toast.LENGTH_SHORT).show();
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()");
        return mBinder;
    }

    @Override
    public void onDestroy(){
        Toast.makeText(this, "TrapReceiverService Done", Toast.LENGTH_SHORT).show();
        Log.d(TAG, "onDestroy()");
        // close output file (use OutDbgStr)
        outputFile.close();
    }

    /**
     * Class used for the client Binder.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with IPC.
     */
    public class LocalBinder extends Binder {
        TrapReceiverService getService() {
            // Return this instance of LocalService so clients can call public methods
            return TrapReceiverService.this;
        }
    }

    // Public methods for clients
    WeakReference<NewTrapListener> m_trapListeners;
    public void registerLogUpdater(NewTrapListener listener){
        Log.d(TAG, "registerLogUpdater()");
        m_trapListeners = new WeakReference<NewTrapListener>(listener);
    }

    private String byteArrayToHex(byte[] a) {
        StringBuilder sb = new StringBuilder(a.length * 2);
        for(byte b: a) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public void ProcessTrap(long timeInMillisecond, TRAP_TYPE type, int len, byte[] data, int offset) {
        Log.d(TAG, "frame is incoming: timestamp = [" + timeInMillisecond + "], type = [" + type.GetValue() + "], len = [" + len + "]");
        // Dump trap number
        trapCount++;
        outputFile.writeLine("#*" + trapCount + "*#  ");
        outputFile.writeLine(String.valueOf(timeInMillisecond));
        // Message object
        Msg msg = null;

        // Decode trap by trap type
        switch (type) {
            case TRAP_TYPE_OTA: {
                msg = m_plainDataDecoder.otaInfo_getMsg(data, offset);
                if (msg != null) {
                    Log.d(TAG, "[OTA] " + msg.getMsgName() + " trace ID = " +  m_plainDataDecoder.otaInfo_getMsgID(msg.getMsgName()) +  " SIM idx= " + msg.getSimIdx());
                    int traceID = m_plainDataDecoder.otaInfo_getMsgID(msg.getMsgName());
                    if (mOtaTraceId != null) {
                        for (int id : mOtaTraceId) {
                            if (traceID == id) {
                                outputFile.writeLine("[OTA] " + msg.getMsgName() + " trace ID = " +  traceID  +  " SIM idx= " + msg.getSimIdx());
                                DumpOTAMessageFields(
                                        msg.getMsgName(), msg);
                            }
                        }
                    }
                }
                else{
                    Log.d(TAG, "[OTA] msg is NULL" );
                }
                break;
            }
            case TRAP_TYPE_EM: {
                msg = m_plainDataDecoder.msgInfo_getMsg(data, offset);
                if (msg != null) {
                    Log.d(TAG, "[EM] " + msg.getMsgName() + " msg ID = " + m_plainDataDecoder.msgInfo_getMsgID( msg.getMsgName())  + " SIM idx= " + msg.getSimIdx());
                    outputFile.writeLine("[EM] " + msg.getMsgName() + " msg ID = " + m_plainDataDecoder.msgInfo_getMsgID( msg.getMsgName())  + " SIM idx= " + msg.getSimIdx());
                    DumpGeneralMessageFields(msg);
                }else{
                    Log.d(TAG, "[EM] msg is NULL" );
                }
                break;
            }
            case TRAP_TYPE_ICD_RECORD: {
                byte [] trap_data = Arrays.copyOfRange(data, offset, data.length);
                // hardcode decode
                ByteBuffer icdPacket = ByteBuffer.wrap(trap_data);
                // parse ICD packet header info
                int icd_data = icdPacket.getInt(); // default ByteBuffer is BIG_ENDIAN, below method is based on BIG_ENDIAN.
                int icd_type = (icd_data & 0xF0000000) >> 28;
                int icd_version = (icd_data & 0x0F000000) >> 24;
                int icd_packet_size = ((icd_data & 0x00FF0000) >> 16) + (icd_data & 0x0000FF00) + ((icd_data & 0x0000FF) << 16);
                icd_data = icdPacket.getInt();
                int icd_protocol_id = (icd_data & 0xF0000000) >> 28;
                int icd_record_code = ((icd_data & 0x0000FF00) >> 8) + ((icd_data & 0x0000FF) << 8);
                Log.d(TAG, "[ICD type " +  icd_type + " v" + icd_version + "] Record code : " + icd_record_code + " from protocol id : " + icd_protocol_id + " packet size : " +  icd_packet_size);
                outputFile.writeLine("[ICD type" +  icd_type + " v" + icd_version + "] Record code : " + icd_record_code + " from protocol id : " + icd_protocol_id);
                break;
            }
            case TRAP_TYPE_ICD_EVENT: {
                byte [] trap_data = Arrays.copyOfRange(data, offset, data.length);
                // hardcode decode
                ByteBuffer icdPacket = ByteBuffer.wrap(trap_data);
                // parse ICD packet header info
                int icd_data = icdPacket.getInt();  // default ByteBuffer is BIG_ENDIAN, below method is based on BIG_ENDIAN.
                int icd_type = (icd_data & 0xF0000000) >> 28;
                int icd_version = (icd_data & 0x0F000000) >> 24;
                int icd_packet_size = ((icd_data & 0x00FF0000) >> 16) + (icd_data & 0x0000FF00) + ((icd_data & 0x0000FF) << 16);
                icd_data = icdPacket.getInt();
                int icd_protocol_id = (icd_data & 0xF0000000) >> 28;
                int icd_event_code = ((icd_data & 0x0000FF00) >> 8) + ((icd_data & 0x0000FF) << 8);
                Log.d(TAG, "[ICD type " +  icd_type + " v" + icd_version + "] Event code : " + icd_event_code + " from protocol id : " + icd_protocol_id+ " packet size : " +  icd_packet_size);
                outputFile.writeLine("[ICD type" +  icd_type + " v" + icd_version + "] Event code : " + icd_event_code + " from protocol id : " + icd_protocol_id);
                break;
            }
            case TRAP_TYPE_DISCARDINFO:
                outputFile.writeLine("[Discard ... ]");
                break;
            default:
                Log.e(TAG, "[Unknown Type, Error ...]");
                outputFile.writeLine("[Unknown Type, Error ...]");
                break;
        }

        // End token for one trap
        outputFile.writeLine("*#*#");

        Log.d(TAG, "Trigger Activity listener !");
        if (m_trapListeners != null && m_trapListeners.get() != null) {
            m_trapListeners.get().NewTrapArrival();
        }
        Log.d(TAG, "frame is incoming... done!");
    }

    private void DumpGeneralMessageFields(Msg msg){
        if (msg != null) {
            Log.d(TAG, "Print General fields ! peer buffer size = " + msg.getPeerBufferLength());
            String[] fieldList = msg.getFieldList();
            if (fieldList != null) {
                // dump local field
                for (String fieldName : fieldList) {
                    byte[] fieldValueBytes = msg.getFieldValue(fieldName);
                    if (fieldValueBytes == null) {
                       outputFile.writeLine("     " + fieldName + " : " +  "null");
                    } else if (fieldValueBytes.length <= 4) { // single value
                        outputFile.write("     " + fieldName + " : ");
                        long fieldValue = Utils.GetIntFromByte(fieldValueBytes, 0, fieldValueBytes.length);
                        outputFile.write(" " + fieldValue);
                        outputFile.newLine();
                    }
                }
                // dump peer buffer
                outputFile.write("     Peer Buffer : ");
                int peerSize = msg.getPeerBufferLength();
                if(peerSize > 0) {
                    byte [] peerBufBytes = msg.getPeerBufferValue();
                    outputFile.writeLine("length = " + peerSize);
                    for (int i = 0 ; i < peerSize ; i++) {
                        outputFile.write(String.format("0x%02x", peerBufBytes[i]) + "(" + Byte.toString(peerBufBytes[i]) + ") ");
                    }
                    outputFile.newLine();
                }
                else {
                    outputFile.writeLine("null");
                }
            }
        }
    }

    private void DumpOTAMessageFields(String otaTraceId, Msg msg) {
        if (msg != null) {
            Log.d(TAG, "Print OTA fields !");
            if (otaTraceId != null) {
                outputFile.writeLine("     Trace ID : " + otaTraceId);
            }
            // get OTA global ID
            byte[] otaOffset = msg.getOtaMsgFieldValue();
            String otaGlobalID = msg.getOtaMsgGlobalID(otaOffset);
            if (otaGlobalID != null) {
                outputFile.writeLine("     Global ID : " + otaGlobalID);
            }
            // get OTA raw data
            int raw_offset = msg.getOtaMsgAirMsgRawDataBufOffset(otaOffset);
            int raw_length = msg.getOtaMsgAirMsgRawDataBufLength(otaOffset);
            Log.d(TAG, "raw length = " + raw_length + ",  otaOffset.length - raw_offset - 2 = " + (otaOffset.length - raw_offset - 2));
            if(raw_length > 0){
                outputFile.write("     OTA raw data : ");
                for (int i=0; i<raw_length; i++) {
                    outputFile.write(String.format("0x%02x", otaOffset[raw_offset+i]) + "(" + Byte.toString(otaOffset[raw_offset+i]) + ") ");
                }
                outputFile.newLine();
            }
        }
    }

    public void setOtaTraceIdEnable(int[] otaTraceId) {
        mOtaTraceId = otaTraceId;
    }
    /**
     * In order to make MonitorTrapReceiver and Session to be one-to-one correspondence,
     * MonitorTrapReceiver should be initialized after getting session ID.
     * 
     * @param szServerName [IN] The host name of the receiver.
     */
    private void InitMDMLTrapReceiver(long sessionId, String serverName) {
        if (0 >= sessionId) {
            throw new Error("Invalid session id! MonitorTrapReceiver"
                    + "should be constructed after getting a valid session ID");
        }
        m_trapReceiver = new MonitorTrapReceiver(sessionId, serverName);
        // setup trap handler to trap receiver
        m_trapReceiver.SetTrapHandler(this);
    }

    @Override
    public void onTrapReceiverInit(long sessionId, String serverName) {
        InitMDMLTrapReceiver(sessionId, serverName);
    }
}
