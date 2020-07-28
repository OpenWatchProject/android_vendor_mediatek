package com.mediatek.mdmlsample;

import android.os.Environment;

import com.mediatek.mdml.Msg;
import com.mediatek.mdml.PlainDataDecoder;

public class DecoderSample {
    private OutDbgStr outputFile;
    public void Run() {
        String sdcardPath = Environment.getDataDirectory().getPath();
        String infoDatPath = sdcardPath + "/Download/info.dat";
        String outputTxtPath = sdcardPath + "/Download/output.txt";
        outputFile = new OutDbgStr(outputTxtPath);
        String s = "";
        try {
            PlainDataDecoder decoder = PlainDataDecoder.getInstance(infoDatPath);

            outputFile.writeLine("test buffer: 0 1 2 ... 97 98 99");
            otaInfo_test(decoder);
            msgInfo_test(decoder);
            globalId_test(decoder);
        } catch (Exception e) {
            s += e.toString();
        }

        /*TextView v = (TextView) findViewById(R.id.textViewItem);
        v.setText(s);*/
        outputFile.close();
    }

    private void otaInfo_test(PlainDataDecoder decoder) {
        outputFile.writeLine("============================================================");
        String[] msgList = decoder.otaInfo_getMsgList();
        outputFile.writeLine("Decoder.otaInfo_getMsgList()");
        outputFile.writeLine("------------------------------------------------------------");
        if (msgList == null) {
            outputFile.writeLine("null");
            return;
        }

        for (String i : msgList) {
            outputFile.writeLine(i);
        }

        otaInfo_testMsg(decoder, "SIBE_PEER_MSG_RRC_SI", 225);
        otaInfo_testMsg(decoder, "RR_NW_TO_MS_SI_MSG", 226);
        otaInfo_testMsg(decoder, "ERRC_SYS_SI_SIB_PEER", 227);
    }

    private void otaInfo_testMsg(PlainDataDecoder decoder, String msgName, int simIdx) {
        byte[] testBuf = getTestbuf();
        outputFile.writeLine("------------------------------------------------------------");
        outputFile.writeLine("test msg: " + msgName + ", sim index: " + Integer.toString(simIdx));
        int msgID = decoder.otaInfo_getMsgID(msgName);
        outputFile.writeLine("Decoder.otaInfo_getMsgID(" + msgName + ") = " + Integer.toString(msgID));
        outputFile.writeLine("Decoder.otaInfo_getMsgName(" + Integer.toString(msgID) + ") = " + decoder.otaInfo_getMsgName(msgID));
        writeBuf(testBuf, 0, 4, msgID);
        writeBuf(testBuf, 6, 2, simIdx);
        Msg msg = decoder.otaInfo_getMsg(testBuf, 0);
        outputFile.writeLine("Decoder.otaInfo_getMsg() --> Msg");
        msgApiTest(msg);
    }

    private void msgInfo_test(PlainDataDecoder decoder) {
        outputFile.writeLine("============================================================");
        String[] msgList = decoder.msgInfo_getMsgList();
        outputFile.writeLine("Decoder.msgInfo_getMsgList()");
        outputFile.writeLine("------------------------------------------------------------");
        if (msgList == null) {
            outputFile.writeLine("null");
            return;
        }

        for (String i : msgList) {
            outputFile.writeLine(i);
        }

        msgInfo_testMsg(decoder, "MSG_ID_EM_ERRC_MOB_MEAS_INTERRAT_UTRAN_INFO_IND", 225);
        msgInfo_testMsg(decoder, "MSG_ID_EM_MEME_DCH_LTE_CELL_INFO_IND", 226);
        msgInfo_testMsg(decoder, "MSG_ID_EM_MEME_DCH_UMTS_CELL_INFO_IND", 227);
    }

    private void msgInfo_testMsg(PlainDataDecoder decoder, String msgName, int simIdx) {
        byte[] testBuf = getTestbuf();
        outputFile.writeLine("------------------------------------------------------------");
        outputFile.writeLine("test msg: " + msgName + ", sim index: " + Integer.toString(simIdx));
        int msgID = decoder.msgInfo_getMsgID(msgName);
        outputFile.writeLine("Decoder.msgInfo_getMsgID(" + msgName + ") = " + Integer.toString(msgID));
        outputFile.writeLine("Decoder.msgInfo_getMsgName(" + Integer.toString(msgID) + ") = " + decoder.msgInfo_getMsgName(msgID));
        writeBuf(testBuf, 6, 2, msgID);
        writeBuf(testBuf, 0, 2, simIdx);
        Msg msg = decoder.msgInfo_getMsg(testBuf, 0);
        outputFile.writeLine("Decoder.msgInfo_getMsg() --> Msg");
        msgApiTest(msg);
    }

    private void globalId_test(PlainDataDecoder decoder) {
        outputFile.writeLine("============================================================");
        String[] msgList = decoder.globalId_getList();
        outputFile.writeLine("Decoder.globalId_getList()");
        outputFile.writeLine("------------------------------------------------------------");
        if (msgList == null) {
            outputFile.writeLine("null");
            return;
        }

        for (String i : msgList) {
            outputFile.writeLine(i + "(" + Integer.toString(decoder.globalId_getValue(i)) + ")");
        }
    }

    private void msgApiTest(Msg msg) {
        if (msg == null) {
            outputFile.writeLine("null");
            return;
        }


        outputFile.writeLine("Msg.getSimIdx() == " + msg.getSimIdx());
        outputFile.writeLine("Msg.getMsgName() = " + msg.getMsgName());

        outputFile.write("Msg.getOtaMsgGlobalID() = ");
        byte [] otaOffset = msg.getOtaMsgFieldValue();

        String otaMsgGlobalID = msg.getOtaMsgGlobalID(otaOffset);
        if (otaMsgGlobalID == null) {
            outputFile.writeLine("null");
        } else {
            outputFile.writeLine(otaMsgGlobalID);
        }

        String[] fieldList = msg.getFieldList();
        outputFile.writeLine("----------Msg.getFieldList() : Msg.getFieldValue()----------");
        if (fieldList == null) {
            outputFile.writeLine("null");
        } else {
            for (String fieldName : fieldList) {
                writeField(fieldName, msg.getFieldValue(fieldName));
            }
        }

        outputFile.writeLine("------------------------------------------------------------");
        outputFile.writeLine("Msg.getOtaMsgFieldValue()");
        writeField("OTA_MSG_OFFSET", otaOffset);
    }

    private void writeField(String fieldName, byte[] fieldValue) {
        outputFile.write(fieldName + " :");
        if (fieldValue == null) {
            outputFile.write(" null");
        } else {
            for (byte val : fieldValue) {
                outputFile.write(" " + Byte.toString(val));
            }
        }

        outputFile.newLine();
    }

    private byte[] getTestbuf() {
        byte[] testBuf = new byte[100];
        for (byte i = 0; i < 100; ++i) {
            testBuf[i] = i;
        }

        return testBuf;
    }

    private void writeBuf(byte[] buffer, int offset, int size, int data) {
        if (buffer.length < offset + size) {
            return;
        }

        for (int i = 0; i < size; ++i) {
            buffer[i + offset] = (byte) (0xFF & (data >> (8 * i)));
        }
    }
}
