package com.mediatek.lbs.em2.utils;

import static com.mediatek.lbs.em2.utils.DataCoder2.getBinary;
import static com.mediatek.lbs.em2.utils.DataCoder2.getBoolean;
import static com.mediatek.lbs.em2.utils.DataCoder2.getByte;
import static com.mediatek.lbs.em2.utils.DataCoder2.getDouble;
import static com.mediatek.lbs.em2.utils.DataCoder2.getFloat;
import static com.mediatek.lbs.em2.utils.DataCoder2.getInt;
import static com.mediatek.lbs.em2.utils.DataCoder2.getLong;
import static com.mediatek.lbs.em2.utils.DataCoder2.getShort;
import static com.mediatek.lbs.em2.utils.DataCoder2.getString;
import static com.mediatek.lbs.em2.utils.DataCoder2.putBinary;
import static com.mediatek.lbs.em2.utils.DataCoder2.putBoolean;
import static com.mediatek.lbs.em2.utils.DataCoder2.putByte;
import static com.mediatek.lbs.em2.utils.DataCoder2.putDouble;
import static com.mediatek.lbs.em2.utils.DataCoder2.putFloat;
import static com.mediatek.lbs.em2.utils.DataCoder2.putInt;
import static com.mediatek.lbs.em2.utils.DataCoder2.putLong;
import static com.mediatek.lbs.em2.utils.DataCoder2.putShort;
import static com.mediatek.lbs.em2.utils.DataCoder2.putString;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.Calendar;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.util.Log;

import vendor.mediatek.hardware.lbs.V1_0.ILbs;
import vendor.mediatek.hardware.lbs.V1_0.ILbsCallback;
import android.os.RemoteException;
import android.os.HwBinder;
import java.util.ArrayList;
import java.util.Arrays;
import com.mediatek.lbs.em2.utils.DataCoder2.DataCoderBuffer;

public class AgpsInterface {

    public final static short VERSION_MAJOR = 1;
    public final static short VERSION_MINOR = 1;

    public final static int EMULATOR_MODE_OFF = 0;
    public final static int EMULATOR_MODE_GEMINI = 1;
    public final static int EMULATOR_MODE_DT = 2;
    public final static int EMULATOR_MODE_COMPORT = 3;

    public final static String SOCKET_ADDRESS = "agpsd2";

    // Command Enum APP -> AGPSD
    protected final static int APP_MGR_CMD_CODER_TEST = 0;
    protected final static int APP_MGR_CMD_VERSION = 1;

    protected final static int APP_MGR_CMD_GET_CONFIG = 100;
    protected final static int APP_MGR_CMD_GET_OMA_CP_SUPL_PROFILE = 101;
    protected final static int APP_MGR_CMD_GET_SYSTEM_PROPERTY = 102;
    protected final static int APP_MGR_CMD_GET_AGPS_PROFILING = 103;
    protected final static int APP_MGR_CMD_GET_EMULATOR_MODE = 104;
    protected final static int APP_MGR_CMD_GET_CONFIG_V2 = 105;
    protected final static int APP_MGR_CMD_GET_CONFIG_V3 = 106;
    protected final static int APP_MGR_CMD_GET_CONFIG_V4 = 107;
    protected final static int APP_MGR_CMD_GET_CONFIG_V5 = 108;
    protected final static int APP_MGR_CMD_GET_CONFIG_V6 = 109;
    protected final static int APP_MGR_CMD_GET_CONFIG_V7 = 110;
    protected final static int APP_MGR_CMD_GET_CONFIG_V8 = 111;
    protected final static int APP_MGR_CMD_GET_CONFIG_V9 = 112;
    protected final static int APP_MGR_CMD_GET_CONFIG_V10 = 113;
    protected final static int APP_MGR_CMD_GET_CONFIG_V11 = 114;
    protected final static int APP_MGR_CMD_GET_CONFIG_V12 = 115;
    protected final static int APP_MGR_CMD_GET_CONFIG_V13 = 116;
    protected final static int APP_MGR_CMD_GET_CONFIG_V14 = 117;
    protected final static int APP_MGR_CMD_GET_CONFIG_V15 = 118;
    protected final static int APP_MGR_CMD_GET_CONFIG_V16 = 119;
    protected final static int APP_MGR_CMD_GET_CONFIG_V17 = 120;
    protected final static int APP_MGR_CMD_GET_CONFIG_V18 = 121;
    protected final static int APP_MGR_CMD_GET_CONFIG_V19 = 122;
    protected final static int APP_MGR_CMD_GET_CONFIG_V20 = 123;
    protected final static int APP_MGR_CMD_GET_CONFIG_V21 = 124;
    protected final static int APP_MGR_CMD_GET_CONFIG_V22 = 125;
    protected final static int APP_MGR_CMD_GET_CONFIG_V23 = 126;
    protected final static int APP_MGR_CMD_GET_CONFIG_V24 = 127;

    protected final static int APP_MGR_CMD_SET_AGPS_ENABLE = 200;
    protected final static int APP_MGR_CMD_SET_PROTOCOL = 201;
    protected final static int APP_MGR_CMD_SET_CDMA_PREF = 202;
    protected final static int APP_MGR_CMD_SET_UP_PREF_METHOD = 203;
    protected final static int APP_MGR_CMD_SET_POS_TECHNOLOGY_MSA = 204;
    protected final static int APP_MGR_CMD_SET_POS_TECHNOLOGY_MSB = 205;
    protected final static int APP_MGR_CMD_SET_POS_TECHNOLOGY_ECID = 206;
    protected final static int APP_MGR_CMD_SET_POS_TECHNOLOGY_OTDOA = 207;
    protected final static int APP_MGR_CMD_SET_SUPL_VERSION = 208;
    protected final static int APP_MGR_CMD_SET_SUPL_PROFILE = 209;
    protected final static int APP_MGR_CMD_SET_QOP = 210;
    protected final static int APP_MGR_CMD_SET_MOLR_POS_METHDO = 211;
    protected final static int APP_MGR_CMD_SET_EXTERNAL_ADDR = 212;
    protected final static int APP_MGR_CMD_SET_MLC_NUMBER = 213;
    protected final static int APP_MGR_CMD_SET_CP_AUTO_RESET = 214;
    protected final static int APP_MGR_CMD_SET_ALLOW_NI = 215;
    protected final static int APP_MGR_CMD_SET_ALLOW_ROAMING = 216;
    protected final static int APP_MGR_CMD_SET_SUPL_2_FILE = 217;
    protected final static int APP_MGR_CMD_SET_RESET_TO_DEFAULT = 218;
    protected final static int APP_MGR_CMD_SET_OMA_CP_SUPL_PROFILE = 219;
    protected final static int APP_MGR_CMD_SET_NI_REQ = 220;
    protected final static int APP_MGR_CMD_SET_EPC_MOLR_PDU_ENABLE = 221;
    protected final static int APP_MGR_CMD_SET_EPC_MOLR_PDU = 222;
    protected final static int APP_MGR_CMD_SET_TLS_VERSION = 223;
    protected final static int APP_MGR_CMD_SET_CA_ENABLE = 224;
    protected final static int APP_MGR_CMD_SET_UDP_ENABLE = 225;
    protected final static int APP_MGR_CMD_SET_LPP_ENABLE = 226;
    protected final static int APP_MGR_CMD_SET_CERT_FROM_SDCARD_ENABLE = 227;
    protected final static int APP_MGR_CMD_SET_AUTO_PROFILE_ENABLE = 228;
    protected final static int APP_MGR_CMD_SET_UT2 = 229;
    protected final static int APP_MGR_CMD_SET_UT3 = 230;
    protected final static int APP_MGR_CMD_SET_SUPL_APN_ENABLE = 231;
    protected final static int APP_MGR_CMD_SET_SYNC_TO_SLP = 232;
    protected final static int APP_MGR_CMD_SET_UDP_ENABLE_V2 = 233;
    protected final static int APP_MGR_CMD_SET_AUTONOMOUS_ENABLE = 234;
    protected final static int APP_MGR_CMD_SET_AFLT_ENABLE = 235;
    protected final static int APP_MGR_CMD_SET_IMSI_ENABLE = 236;
    protected final static int APP_MGR_CMD_SET_SIB8_16_ENABLE = 237;
    protected final static int APP_MGR_CMD_SET_GPS_ENABLE = 238;
    protected final static int APP_MGR_CMD_SET_GLONASS_ENABLE = 239;
    protected final static int APP_MGR_CMD_SET_BEIDOU_ENABLE = 240;
    protected final static int APP_MGR_CMD_SET_GALILEO_ENABLE = 241;
    protected final static int APP_MGR_CMD_SET_SUPL_SHA_VERSION = 242;
    protected final static int APP_MGR_CMD_SET_SUPL_TLS_VERSION = 243;
    protected final static int APP_MGR_CMD_SET_SUPL_VER_MINOR = 244;
    protected final static int APP_MGR_CMD_SET_SUPL_VER_SER_IND = 245;
    protected final static int APP_MGR_CMD_SET_A_GLONASS_ENABLE = 246;
    protected final static int APP_MGR_CMD_SET_PDE_PROFILE = 247;
    protected final static int APP_MGR_CMD_SET_E911_GPS_ICON_ENABLE = 248;
    protected final static int APP_MGR_CMD_SET_E911_OPEN_GPS_ENABLE = 249;
    protected final static int APP_MGR_CMD_SET_A_GPS_ENABLE = 250;
    protected final static int APP_MGR_CMD_SET_A_BEIDOU_ENABLE = 251;
    protected final static int APP_MGR_CMD_SET_A_GALILEO_ENABLE = 252;
    protected final static int APP_MGR_CMD_SET_PREF_2G3G_CELL_AGE = 253;
    protected final static int APP_MGR_CMD_SET_UT1 = 254;
    protected final static int APP_MGR_CMD_SET_NO_SENSITIVE_LOG = 255;
    protected final static int APP_MGR_CMD_SET_TLS_REUSE_ENABLE = 256;
    protected final static int APP_MGR_CMD_SET_IMSI_CACHE_ENABLE = 257;
    protected final static int APP_MGR_CMD_SET_SUPL_RAW_DATA_ENABLE = 258;
    protected final static int APP_MGR_CMD_SET_TC10_ENABLE = 259;
    protected final static int APP_MGR_CMD_SET_TC10_USE_APN = 260;
    protected final static int APP_MGR_CMD_SET_TC10_USE_FW_DNS = 261;
    protected final static int APP_MGR_CMD_SET_ALLOW_NI_FOR_GPS_OFF = 262;
    protected final static int APP_MGR_CMD_SET_FORCE_OTDOA_ASSIST_REQ = 263;
    protected final static int APP_MGR_CMD_SET_REJECT_NON911_NILR_ENABLE = 264;
    protected final static int APP_MGR_CMD_SET_CP_2G_DISABLE = 265;
    protected final static int APP_MGR_CMD_SET_CP_3G_DISABLE = 266;
    protected final static int APP_MGR_CMD_SET_CP_4G_DISABLE = 267;
    protected final static int APP_MGR_CMD_SET_TC10_IGNORE_FW_CONFIG = 268;
    protected final static int APP_MGR_CMD_SET_LPPE_HIDE_WIFI_BT_STATUS = 269;
    protected final static int APP_MGR_CMD_SET_LPPE_NETWORK_LOCATION_DISABLE = 270;
    protected final static int APP_MGR_CMD_SET_LPPE_CP_ENABLE = 271;
    protected final static int APP_MGR_CMD_SET_LPPE_UP_ENABLE = 272;
    protected final static int APP_MGR_CMD_SET_VZW_DEBUG_SCREEN_ENABLE = 273;
    protected final static int APP_MGR_CMD_SET_AOSP_PROFILE_ENABLE = 274;
    protected final static int APP_MGR_CMD_SET_BIND_NLP_SETTING_TO_SUPL = 275;
    protected final static int APP_MGR_CMD_SET_ESUPL_APN_MODE = 276;
    protected final static int APP_MGR_CMD_SET_TCP_KEEPALIVE = 277;
    protected final static int APP_MGR_CMD_SET_AGPS_NVRAM_ENABLE = 278;
    protected final static int APP_MGR_CMD_SET_LBS_LOG_ENABLE = 279;
    protected final static int APP_MGR_CMD_SET_LPPE_CROWD_SOURCE_CONFIDENT = 280;
    protected final static int APP_MGR_CMD_SET_IGNORE_SI_FOR_E911 = 281;
    protected final static int APP_MGR_CMD_SET_LPPE_CP_WLAN_ENABLE = 282;
    protected final static int APP_MGR_CMD_SET_LPPE_CP_SRN_ENABLE = 283;
    protected final static int APP_MGR_CMD_SET_LPPE_CP_SENSOR_ENABLE = 284;
    protected final static int APP_MGR_CMD_SET_LPPE_CP_DBH_ENABLE = 285;
    protected final static int APP_MGR_CMD_SET_LPPE_UP_WLAN_ENABLE = 286;
    protected final static int APP_MGR_CMD_SET_LPPE_UP_SRN_ENABLE = 287;
    protected final static int APP_MGR_CMD_SET_LPPE_UP_SENSOR_ENABLE = 288;
    protected final static int APP_MGR_CMD_SET_LPPE_UP_DBH_ENABLE = 289;
    protected final static int APP_MGR_CMD_SET_IP_VERSION_PREFER = 290;
    protected final static int APP_MGR_CMD_SET_UP_LPP_IN_2G3G_DISABLE = 291;
    protected final static int APP_MGR_CMD_SET_UP_RRLP_IN_4G_DISABLE = 292;
    protected final static int APP_MGR_CMD_UP_SI_DISABLE = 293;
    protected final static int APP_MGR_CMD_SET_USE_NI_SLP = 294;
    protected final static int APP_MGR_CMD_SET_USE_TC10_CONFIG = 295;
    protected final static int APP_MGR_CMD_SET_LPPE_DEF_NLP_ENABLE = 296;
    protected final static int APP_MGR_CMD_SET_AOSP_POS_MODE_ENABLE = 297;
    protected final static int APP_MGR_CMD_SET_PRIVACY_OVERRIDE_MODE = 298;
    protected final static int APP_MGR_CMD_SET_EMERGENCY_EXT_SECS = 299;


    protected final static int APP_MGR_CMD_START_PERIODIC = 300;
    protected final static int APP_MGR_CMD_ABORT_PERIODIC = 301;
    protected final static int APP_MGR_CMD_START_AREA_EVENT = 302;
    protected final static int APP_MGR_CMD_ABORT_AREA_EVENT = 303;

    protected final static int APP_MGR_CMD_START_TEST_CASE = 400;
    protected final static int APP_MGR_CMD_START_TEST_BUTTON = 401;
    protected final static int APP_MGR_CMD_START_RESET_AGPSD = 402;
    protected final static int APP_MGR_CMD_START_EMULATOR_MODE = 403;

    //protected LocalSocket client;
    protected DataCoderBuffer out = new DataCoderBuffer(16 * 1024);
    protected DataCoderBuffer in = new DataCoderBuffer(16 * 1024);

    //HIDL start
    public static ArrayList<Byte> convertByteArrayToArrayList(byte[] data, int size) {
        if (data == null) {
            return null;
        }
        int max = (size < data.length)? size : data.length;
        ArrayList<Byte> ret = new ArrayList<Byte>();
        for (int i = 0; i < max; i++) {
            ret.add(data[i]);
        }
        return ret;
    }

    public static void covertArrayListToByteArray(ArrayList<Byte> in, byte[] out) {
        for(int i = 0; i < in.size() && i < out.length; i++) {
            out[i] = in.get(i);
        }
    }

    ILbs mLbsHidlClient;
    ArrayList<Byte> mData = new ArrayList<Byte>();

    LbsHidlCallback mLbsHidlCallback = new LbsHidlCallback();
    class LbsHidlCallback extends ILbsCallback.Stub {
        public boolean callbackToClient(ArrayList<Byte> data) {
            mData = data;
            return true;
        }
    };

    LbsHidlDeathRecipient mLLbsHidlDeathRecipient = new LbsHidlDeathRecipient();
    class LbsHidlDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            log("serviceDied");
            mLbsHidlClient = null;
        }
    }

    private void doHidl(String name, byte[] data, int size) {
        try {
            if(mLbsHidlClient == null) {
                mLbsHidlClient = ILbs.getService(name);
                mLbsHidlClient.linkToDeath(mLLbsHidlDeathRecipient, 0);
            }
            //it is a sychronization call and mData will be updated by callbackToClient()
            if(mLbsHidlClient.sendToServerWithCallback(mLbsHidlCallback,
                convertByteArrayToArrayList(data, size))) {
                covertArrayListToByteArray(mData, in.mBuff);
            } else {
                //error case (ex: AGPS reboot or HIDL error)
                Arrays.fill(in.mBuff, (byte) 0);
            }
        } catch (RemoteException | RuntimeException e) {
            e.printStackTrace();
        }
    }

    private void writeToHidl() {
        doHidl("AgpsInterface", out.mBuff, out.mOffset);
        out.flush();
        in.clear();
    }
    
    //HILD end

    public AgpsInterface() throws IOException {
        checkVersion();
    }

    public void coderTest() throws IOException {
        try {
            connect();

            // write
            putInt(out, APP_MGR_CMD_CODER_TEST);
            putByte(out, (byte) 0x12);
            putShort(out, (short) 0x1234);
            putInt(out, 0x12345678);
            putLong(out, 0x1234567890abcdefL);
            putFloat(out, 1f);
            putDouble(out, 2.0);
            putString(out, "string");
            putBinary(out, new byte[] { 0, 1, 2, 3, 4, 5, 6, 7, 8 });
            writeToHidl();

            // read + verification
            long ret;
            ret = getByte(in);
            if (ret != 0x12) {
                throw new IOException("getByte failed expected="
                        + Long.toHexString(0x12) + " actually="
                        + Long.toHexString(ret));
            }
            ret = getShort(in);
            if (ret != 0x1234) {
                throw new IOException("getShort failed expected="
                        + Long.toHexString(0x1234) + " actually="
                        + Long.toHexString(ret));
            }
            ret = getInt(in);
            if (ret != 0x12345678) {
                throw new IOException("getInt failed expected="
                        + Long.toHexString(0x12345678) + " actually="
                        + Long.toHexString(ret));
            }
            ret = getLong(in);
            if (ret != 0x1234567890abcdefL) {
                throw new IOException("getLong failed expected="
                        + Long.toHexString(0x1234567890abcdefL) + " actually="
                        + Long.toHexString(ret));
            }
            float f = getFloat(in);
            if (f != 1f) {
                throw new IOException("getFloat failed expected=" + 1f
                        + " actually=" + f);
            }
            double d = getDouble(in);
            if (d != 2.0) {
                throw new IOException("getDouble failed expected=" + 2
                        + " actually=" + d);
            }
            String string = getString(in);
            if (!string.equals("string")) {
                throw new IOException("getString failed expected=" + string
                        + " actually=" + string);
            }
            byte[] binary = getBinary(in);
            if (binary == null) {
                throw new IOException("getBinary failed, get null");
            }
            if (binary.length != 9) {
                throw new IOException("getBinary failed, length expected=" + 9
                        + " actually=" + binary.length);
            }
            for (int i = 0; i < 9; i++) {
                if (binary[i] != i) {
                    throw new IOException("getBinary failed, data[" + i
                            + "] expected=" + i + " actually=" + binary[i]);
                }
            }

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new IOException(e);
        } finally {
            close();
        }
    }

    public void checkVersion() {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_VERSION);
            putShort(out, VERSION_MAJOR);
            putShort(out, VERSION_MINOR);

            writeToHidl();

            short majorVersion = getShort(in);
            short minorVersion = getShort(in);

            if (majorVersion != VERSION_MAJOR) {
                throw new IOException("app maj ver=" + VERSION_MAJOR
                        + " is not equal to AGPSD's maj ver=" + majorVersion);
            }
            if (minorVersion < VERSION_MINOR) {
                throw new IOException("app min ver=" + VERSION_MINOR
                        + " is greater than AGPSD's min ver=" + minorVersion);
            }

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public AgpsConfig getAgpsConfig() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV2() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V2;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV3() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V3;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV4() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V4;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV5() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V5;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV6() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V6;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV7() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V7;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV8() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V8;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV9() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V9;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV10() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V10;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV11() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V11;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV12() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V12;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV13() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V13;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV15() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V15;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV16() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V16;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV17() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V17;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV19() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V19;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }
    
    public AgpsConfig getAgpsConfigV20() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V20;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV21() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V21;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV22() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V22;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV23() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V23;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }

    public AgpsConfig getAgpsConfigV24() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V24;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config;
    }
    public CdmaProfile getCdmaProrfile() {
        AgpsConfig config = new AgpsConfig();
        try {
            int cmd = APP_MGR_CMD_GET_CONFIG_V14;
            connect();
            // write
            putInt(out, cmd);
            writeToHidl();

            // read
            getAgpsConfigInt(cmd, config);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return config.getCdmaProfile();
    }

    private void getAgpsConfigInt(int cmd, AgpsConfig config) throws IOException {
        // read
        AgpsSetting agpsSetting = config.getAgpsSetting();
        agpsSetting.agpsEnable = getBoolean(in);
        agpsSetting.agpsProtocol = getInt(in);
        agpsSetting.gpevt = getBoolean(in);

        CpSetting cpSetting = config.getCpSetting();
        cpSetting.molrPosMethod = getInt(in);
        cpSetting.externalAddrEnable = getBoolean(in);
        cpSetting.externalAddr = getString(in);
        cpSetting.mlcNumberEnable = getBoolean(in);
        cpSetting.mlcNumber = getString(in);
        cpSetting.cpAutoReset = getBoolean(in);
        cpSetting.epcMolrLppPayloadEnable = getBoolean(in);
        cpSetting.epcMolrLppPayload = getBinary(in);

        UpSetting upSetting = config.getUpSetting();
        GnssSetting gnssSetting = config.getGnssSetting();
        upSetting.caEnable = getBoolean(in);
        upSetting.niRequest = getBoolean(in);
        upSetting.roaming = getBoolean(in);
        upSetting.cdmaPreferred = getInt(in);
        upSetting.prefMethod = getInt(in);
        upSetting.suplVersion = getInt(in);
        upSetting.tlsVersion = getInt(in);
        upSetting.suplLog = getBoolean(in);
        upSetting.msaEnable = getBoolean(in);
        upSetting.msbEnable = getBoolean(in);
        upSetting.ecidEnable = getBoolean(in);
        upSetting.otdoaEnable = getBoolean(in);
        upSetting.qopHacc = getInt(in);
        upSetting.qopVacc = getInt(in);
        upSetting.qopLocAge = getInt(in);
        upSetting.qopDelay = getInt(in);
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V2) {
            upSetting.lppEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V3) {
            upSetting.certFromSdcard = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V4) {
            upSetting.autoProfileEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V5) {
            upSetting.ut2 = getByte(in);
            upSetting.ut3 = getByte(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V6) {
            upSetting.apnEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V7) {
            upSetting.syncToslp = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V8) {
            upSetting.udpEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V9) {
            upSetting.autonomousEnable = getBoolean(in);
            upSetting.afltEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V10) {
            upSetting.imsiEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V11) {
            gnssSetting.sib8sib16Enable = getBoolean(in);
            gnssSetting.gpsSatelliteEnable = getBoolean(in);
            gnssSetting.glonassSatelliteEnable = getBoolean(in);
            gnssSetting.beidouSatelliteEnable = getBoolean(in);
            gnssSetting.galileoSatelliteEnable = getBoolean(in);
            gnssSetting.gpsSatelliteSupport = getBoolean(in);
            gnssSetting.glonassSatelliteSupport = getBoolean(in);
            gnssSetting.beidousSatelliteSupport = getBoolean(in);
            gnssSetting.galileoSatelliteSupport = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V12) {
            upSetting.suplVerMinor  = getByte(in);
            upSetting.suplVerSerInd = getByte(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V13) {
            gnssSetting.aGlonassSatelliteEnable = getBoolean(in);
        }

        SuplProfile suplProfile = config.getCurSuplProfile();
        suplProfile.name = getString(in);
        suplProfile.addr = getString(in);
        suplProfile.port = getInt(in);
        suplProfile.tls = getBoolean(in);
        suplProfile.mccMnc = getString(in);
        suplProfile.appId = getString(in);
        suplProfile.providerId = getString(in);
        suplProfile.defaultApn = getString(in);
        suplProfile.optionalApn = getString(in);
        suplProfile.optionalApn2 = getString(in);
        suplProfile.addressType = getString(in);

        if (cmd >= APP_MGR_CMD_GET_CONFIG_V14) {
            CdmaProfile cdmaProfile = config.getCdmaProfile();
            cdmaProfile.name = getString(in);
            cdmaProfile.mcpEnable = getBoolean(in);
            cdmaProfile.mcpAddr = getString(in);
            cdmaProfile.mcpPort = getInt(in);
            cdmaProfile.pdeAddrValid = getBoolean(in);
            cdmaProfile.pdeIpType = getInt(in);
            cdmaProfile.pdeAddr = getString(in);
            cdmaProfile.pdePort = getInt(in);
            cdmaProfile.pdeUrlValid = getBoolean(in);
            cdmaProfile.pdeUrlAddr = getString(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V15) {
            agpsSetting.e911GpsIconEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V16) {
            agpsSetting.e911OpenGpsEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V17) {
            gnssSetting.aGpsSatelliteEnable = getBoolean(in);
            gnssSetting.aBeidouSatelliteEnable = getBoolean(in);
            gnssSetting.aGalileoSatelliteEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V18) {
            upSetting.shaVersion = getInt(in);
            upSetting.preferred2g3gCellAge = getInt(in);
            upSetting.ut1 = getByte(in);
            upSetting.noSensitiveLog = getBoolean(in);
            upSetting.tlsReuseEnable = getBoolean(in);
            upSetting.imsiCacheEnable = getBoolean(in);
            upSetting.suplRawDataEnable = getBoolean(in);
            upSetting.tc10Enable = getBoolean(in);
            upSetting.tc10UseApn = getBoolean(in);
            upSetting.tc10UseFwDns = getBoolean(in);
            upSetting.allowNiForGpsOff = getBoolean(in);
            upSetting.forceOtdoaAssistReq = getBoolean(in);
            cpSetting.rejectNon911NilrEnable = getBoolean(in);
            cpSetting.cp2gDisable = getBoolean(in);
            cpSetting.cp3gDisable = getBoolean(in);
            cpSetting.cp4gDisable = getBoolean(in);
            agpsSetting.tc10IgnoreFwConfig = getBoolean(in);
            agpsSetting.lppeHideWifiBtStatus = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V19) {
            agpsSetting.lppeNetworkLocationDisable = getBoolean(in);
            cpSetting.cpLppeEnable = getBoolean(in);
            upSetting.upLppeEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V20) {
            cpSetting.cpLppeSupport = getBoolean(in);
            gnssSetting.lppeSupport = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V21) {
            agpsSetting.agpsNvramEnable = getBoolean(in);
            agpsSetting.lbsLogEnable = getBoolean(in);
            agpsSetting.lppeCrowdSourceConfident = getInt(in);

            upSetting.esuplApnMode = getInt(in);
            upSetting.tcpKeepAlive = getInt(in);
            upSetting.aospProfileEnable = getBoolean(in);
            upSetting.bindNlpSettingToSupl = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V22) {
            agpsSetting.ignoreSiForE911 = getBoolean(in);
            cpSetting.cpLppeWlanEnable = getBoolean(in);
            cpSetting.cpLppeSrnEnable = getBoolean(in);
            cpSetting.cpLppeSensorEnable = getBoolean(in);
            cpSetting.cpLppeDbhEnable = getBoolean(in);

            upSetting.upLppeWlanEnable = getBoolean(in);
            upSetting.upLppeSrnEnable = getBoolean(in);
            upSetting.upLppeSensorEnable = getBoolean(in);
            upSetting.upLppeDbhEnable = getBoolean(in);
            upSetting.ipVersionPrefer = getInt(in);
            upSetting.upLppIn2g3gDisable = getBoolean(in);
            upSetting.upRrlpIn4gDisable = getBoolean(in);
            upSetting.upSiDisable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V23) {
            upSetting.useNiSlp = getBoolean(in);
            agpsSetting.useTc10Config = getBoolean(in);
            agpsSetting.defaultNlpEnable = getBoolean(in);
        }
        if (cmd >= APP_MGR_CMD_GET_CONFIG_V24) {
            agpsSetting.emergencyExtSecs = getInt(in);
            upSetting.aospPosModeEnable = getBoolean(in);
            upSetting.privacyOverrideMode = getInt(in); /// value 3 is to override GNSS visibility control
        }
    }

    public SuplProfile getSuplProfile() {
        SuplProfile profile = new SuplProfile();
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_GET_OMA_CP_SUPL_PROFILE);
            writeToHidl();

            // read
            profile.name = getString(in);
            profile.addr = getString(in);
            profile.port = getInt(in);
            profile.tls = getBoolean(in);
            profile.mccMnc = getString(in);
            profile.appId = getString(in);
            profile.providerId = getString(in);
            profile.defaultApn = getString(in);
            profile.optionalApn = getString(in);
            profile.optionalApn2 = getString(in);
            profile.addressType = getString(in);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return profile;
    }

    public String getSystemProperty(String key, String defaultValue) {
        String ret = null;
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_GET_SYSTEM_PROPERTY);
            putString(out, key);
            putString(out, defaultValue);
            writeToHidl();

            // read
            ret = getString(in);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return ret;
    }

    public AgpsProfilingInfo getAgpsProfilingInfo() {
        AgpsProfilingInfo info = new AgpsProfilingInfo();
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_GET_AGPS_PROFILING);
            writeToHidl();

            // read
            int size = getInt(in);
            for (int i = 0; i < size; i++) {
                int type = getInt(in);
                long timestamp = getLong(in);
                String message = getString(in);
                info.addElement(type, timestamp, message);
            }

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return info;
    }

    /**
     * AgpsInterface.EMULATOR_MODE_OFF = 0<br>
     * AgpsInterface.EMULATOR_MODE_GEMINI = 1<br>
     * AgpsInterface.EMULATOR_MODE_DT = 2<br>
     * AgpsInterface.EMULATOR_MODE_COMPORT = 3<br>
     * */
    public int getEmulatorMode() {
        int ret = 0;
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_GET_EMULATOR_MODE);
            writeToHidl();

            // read ACK
            ret = getInt(in);

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
        return ret;
    }

    public void setAgpsEnabled(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_AGPS_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    /**
     * AgpsSetting.PROTOCOL_UP = 0 <br>
     * AgpsSetting.PROTOCOL_CP = 1 <br>
     * */
    public void setProtocol(int protocol) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_PROTOCOL);
            putByte(out, (byte) protocol);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    /**
     * UpSetting.CDMA_PREF_WCDMA = 0 <br>
     * UpSetting.CDMA_PREF_CDMA = 1 <br>
     * UpSetting.CDMA_PREF_CDMA_FORCE = 2 <br>
     * */
    public void setCdmaPref(int cdmaPref) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_CDMA_PREF);
            putByte(out, (byte) cdmaPref);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    /**
     * UpSetting.PREF_METHOD_MSA = 0 <br>
     * UpSetting.PREF_METHOD_MSB = 1 <br>
     * UpSetting.PREF_METHOD_NO_PREFER = 2 <br>
     * */
    public void setUpPrefMethod(int prefMethod) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_UP_PREF_METHOD);
            putByte(out, (byte) prefMethod);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setPosTechnologyMSA(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_POS_TECHNOLOGY_MSA);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setPosTechnologyMSB(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_POS_TECHNOLOGY_MSB);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setPosTechnologyECID(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_POS_TECHNOLOGY_ECID);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setPosTechnologyOTDOA(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_POS_TECHNOLOGY_OTDOA);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }


    public void setPosTechnologyAutonomous(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_AUTONOMOUS_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setPosTechnologyAFLT(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_AFLT_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setImsiEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_IMSI_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSib8Sib16Enable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SIB8_16_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setGpsSatelliteEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_GPS_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAgpsSatelliteEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_A_GPS_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setGlonassSatelliteEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_GLONASS_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAglonassSatelliteEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_A_GLONASS_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setBeidouSatelliteEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_BEIDOU_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAbeidouSatelliteEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_A_BEIDOU_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setGalileoSatelliteEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_GALILEO_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAgalileoSatelliteEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_A_GALILEO_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    /**
     * 0 = SHA1 for SUPL1.0 and SHA256 for SUPL2.0 (default) <br>
     * 1 = SHA1 for SUPL1.0 and SUPL2.0 <br>
     * 2 = SHA256 for SUPL1.0 and SUPL2.0 <br>
     * */
    public void setSuplShaVersion(int version) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SUPL_SHA_VERSION);
            putInt(out, version);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    /**
     * 0 = TLS1.0 (default) <br>
     * 1 = TLS1.1 <br>
     * 2 = TLS1.2 <br>
     * */
    public void setSuplTlsVersion(int version) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SUPL_TLS_VERSION);
            putInt(out, version);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplVersion(int suplVersion) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SUPL_VERSION);
            putByte(out, (byte) suplVersion);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setTlsVersion(int tlsVersion) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_TLS_VERSION);
            putByte(out, (byte) tlsVersion);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setCertVerify(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_CA_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setUdpEnable() {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_UDP_ENABLE);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPP_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setCertFromSdcard(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_CERT_FROM_SDCARD_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAutoProfileEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_AUTO_PROFILE_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplUt1(int timeoutInSecond) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_UT1);
            putByte(out, (byte)timeoutInSecond);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplUt2(int timeoutInSecond) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_UT2);
            putByte(out, (byte)timeoutInSecond);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplUt3(int timeoutInSecond) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_UT3);
            putByte(out, (byte)timeoutInSecond);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplVerMinor(int suplVerMinor) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SUPL_VER_MINOR);
            putByte(out, (byte)suplVerMinor);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplVerSerInd(int suplVerSerInd) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SUPL_VER_SER_IND);
            putByte(out, (byte)suplVerSerInd);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplDedicatedApnEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SUPL_APN_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSyncToSlpEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SYNC_TO_SLP);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setUdpEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_UDP_ENABLE_V2);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplProfile(String addr, int port, boolean tlsEnabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SUPL_PROFILE);
            putString(out, addr);
            putInt(out, port);
            putBoolean(out, tlsEnabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setQop(int hacc, int vacc, int locAge, int delay) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_QOP);
            putInt(out, hacc);
            putInt(out, vacc);
            putInt(out, locAge);
            putInt(out, delay);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    /**
     * CpSetting.MOLR_POS_METHOD_LOC_EST = 0 <br>
     * CpSetting.MOLR_POS_METHOD_ASSIST_DATA = 1 <br>
     * */
    public void setMolrPosMethod(int molrPosMethod) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_MOLR_POS_METHDO);
            putByte(out, (byte) molrPosMethod);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setExternalAddr(boolean enabled, String string) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_EXTERNAL_ADDR);
            putBoolean(out, enabled);
            putString(out, string);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setMlcNumber(boolean enabled, String string) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_MLC_NUMBER);
            putBoolean(out, enabled);
            putString(out, string);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setCpAutoReset(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_CP_AUTO_RESET);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAllowNI(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_ALLOW_NI);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAllowRoaming(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_ALLOW_ROAMING);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSupl2file(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SUPL_2_FILE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setResetToDefault() {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_RESET_TO_DEFAULT);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplProfile(SuplProfile profile) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_OMA_CP_SUPL_PROFILE);
            putString(out, profile.name);
            putString(out, profile.addr);
            putInt(out, profile.port);
            putBoolean(out, profile.tls);
            putString(out, profile.mccMnc);
            putString(out, profile.appId);
            putString(out, profile.providerId);
            putString(out, profile.defaultApn);
            putString(out, profile.optionalApn);
            putString(out, profile.optionalApn2);
            putString(out, profile.addressType);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setNiRequest(int type, byte[] data) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_NI_REQ);
            putInt(out, type);
            putBinary(out, data);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setEpcMolrPduEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_EPC_MOLR_PDU_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setEpcMolrPdu(byte[] data) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_EPC_MOLR_PDU);
            putBinary(out, data);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setPdeProfile(String addr, int port) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_PDE_PROFILE);
            putString(out, addr);
            putInt(out, port);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setE911GpsIconEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_E911_GPS_ICON_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setE911OpenGpsEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_E911_OPEN_GPS_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setPreferred2g3gCellAge(int preferred2g3gCellAge) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_PREF_2G3G_CELL_AGE);
            putInt(out, preferred2g3gCellAge);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setNoSensitiveLog(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_NO_SENSITIVE_LOG);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setTlsReuseEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_TLS_REUSE_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setImsiCacheEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_IMSI_CACHE_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setSuplRawDataEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_SUPL_RAW_DATA_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setTc10Enable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_TC10_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setTc10UseApn(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_TC10_USE_APN);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setTc10UseFwDns(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_TC10_USE_FW_DNS);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAllowNiForGpsOff(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_ALLOW_NI_FOR_GPS_OFF);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setForceOtdoaAssistReq(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_FORCE_OTDOA_ASSIST_REQ);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setRejectNon911NilrEnable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_REJECT_NON911_NILR_ENABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setCp2gDisable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_CP_2G_DISABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setCp3gDisable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_CP_3G_DISABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setCp4gDisable(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_CP_4G_DISABLE);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setTc10IgnoreFwConfig(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_TC10_IGNORE_FW_CONFIG);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeHideWifiBtStatus(boolean enabled) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_HIDE_WIFI_BT_STATUS);
            putBoolean(out, enabled);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeNetworkLocationDisable(boolean disable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_NETWORK_LOCATION_DISABLE);
            putBoolean(out, disable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeCpEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_CP_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeUpEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_UP_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setVzwDebugScreenEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_VZW_DEBUG_SCREEN_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAospProfileEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_AOSP_PROFILE_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setBindNlpSettingToSupl(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_BIND_NLP_SETTING_TO_SUPL);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setEsuplApnMode(int apnMode) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_ESUPL_APN_MODE);
            putInt(out, apnMode);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setTcpKeepAlive(int aliveTime) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_TCP_KEEPALIVE);
            putInt(out, aliveTime);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAgpsNvramEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_AGPS_NVRAM_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLbsLogEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LBS_LOG_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeCrowdSourceConfident(int confident) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_CROWD_SOURCE_CONFIDENT);
            putInt(out, confident);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setIngoreSiForE911(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_IGNORE_SI_FOR_E911);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeCpWlanEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_CP_WLAN_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeCpSrnEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_CP_SRN_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeCpSensorEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_CP_SENSOR_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeCpDbhEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_CP_DBH_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeUpWlanEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_UP_WLAN_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeUpSrnEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_UP_SRN_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeUpSensorEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_UP_SENSOR_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setLppeUpDbhEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_UP_DBH_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setIpVersionPrefer(int ipVersionPrefer) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_IP_VERSION_PREFER);
            putInt(out, ipVersionPrefer);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setUpLppIn2g3gDisable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_UP_LPP_IN_2G3G_DISABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setUpRrlpIn4gDisable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_UP_RRLP_IN_4G_DISABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setUpSiDisable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_UP_SI_DISABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setDefaultNlpEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_LPPE_DEF_NLP_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setAospPosModeEnable(boolean enable) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_AOSP_POS_MODE_ENABLE);
            putBoolean(out, enable);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void setPrivacyOverrideMode(int mode) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_SET_PRIVACY_OVERRIDE_MODE);
            putInt(out, mode);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void startPeriodic() {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_START_PERIODIC);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void stopPeriodic() {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_ABORT_PERIODIC);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    /**
     * UpSetting.AREA_EVENT_TYPE_ENTERING = 0 <br>
     * UpSetting.AREA_EVENT_TYPE_INSIDE = 1 <br>
     * UpSetting.AREA_EVENT_TYPE_OUTSIDE = 2 <br>
     * UpSetting.AREA_EVENT_TYPE_LEAVING = 3 <br>
     * */
    public void startAreaEvent(int type) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_START_AREA_EVENT);
            putInt(out, type);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void stopAreaEvent() {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_ABORT_AREA_EVENT);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void testCase(int i) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_START_TEST_CASE);
            putInt(out, i);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void testButton(int i) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_START_TEST_BUTTON);
            putInt(out, i);
            writeToHidl();

            // read ACK
            getByte(in);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void resetAgpsd() {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_START_RESET_AGPSD);
            writeToHidl();

            // no need to wait the ACK for resetAgpsd

            log("resetAgpsd() before sleep 1 sec");
            msleep(1000);
            log("resetAgpsd() after sleep 1 sec");
            /*
            if (!waitDisconnected()) {
                loge("reset Agpsd failure");
                return;
            }
            
            retryConnection();
            */

        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    public void startEmulatorMode(int mode) {
        try {
            connect();
            // write
            putInt(out, APP_MGR_CMD_START_EMULATOR_MODE);
            putInt(out, mode);
            writeToHidl();

            // no need to wait the ACK for resetAgpsd

            log("startEmulatorMode() before sleep 1 sec");
            msleep(1000);
            log("startEmulatorMode() after sleep 1 sec");
            /*
            if (!waitDisconnected()) {
                loge("reset Agpsd failure");
                return;
            }

            retryConnection();
            */
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            close();
        }
    }

    protected void retryConnection() {
        //TODO need to move to LBS HIDL Service
        /*
        // wait 20 seconds
        for (int i = 0; i < 100; i++) {
            msleep(200);
            try {
                log("i=" + i + " connecting..");
                connect();
                log("i=" + i + " connecting success");
                break;
            } catch (IOException e1) {
                log("i=" + i + " connecting failure");
                continue;
            }
        }
        */
    }

    protected void connect() throws IOException {
        //do nothing
        /*
        if (client != null) {
            client.close();
        }
        client = new LocalSocket();
        client.connect(new LocalSocketAddress(SOCKET_ADDRESS,
                LocalSocketAddress.Namespace.RESERVED));

        // set read timeout
        client.setSoTimeout(3000);

        out = new BufferedOutputStream(client.getOutputStream());
        in = new DataInputStream(client.getInputStream());
        */
    }

    protected void close() {
        in.clear();
        //TODO need to reset the readBuff (ex: in)
        
        /*
        try {
            if (client != null) {
                client.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        */
    }

    protected boolean waitDisconnected() {
        //TODO this part need to move to LBS HIDL Service
        /*
        byte[] tmp = new byte[2048];
        int readlen;
        try {
            readlen = in.read(tmp);
            if (readlen <= 0) {
                return true;
            }
        } catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException(e);
        }
        */
        return true;
    }

    protected static void dump(byte[] a) {
        log("len=" + a.length);
        for (int i = 0; i < a.length; i++) {
            log("i=" + i + " value=" + Integer.toHexString(a[i]));
        }
    }

    // ============================================================
    protected static void msleep(int milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    protected static void log(String msg) {
        Log.d("AgpsInterface [agps]", msg);
        // System.out.println(getTimeString() + msg);
    }

    protected static void loge(Object msg) {
        Log.d("AgpsInterface [agps] ERR:", msg.toString());
    }

    protected static String getTimeString() {
        String str = "";
        Calendar c = Calendar.getInstance();
        str += String.format("%d/%02d/%02d [%02d:%02d:%02d.%03d] ",
                c.get(Calendar.YEAR), (c.get(Calendar.MONTH) + 1),
                c.get(Calendar.DAY_OF_MONTH), c.get(Calendar.HOUR_OF_DAY),
                c.get(Calendar.MINUTE), c.get(Calendar.SECOND),
                c.get(Calendar.MILLISECOND));
        return str;
    }

}
