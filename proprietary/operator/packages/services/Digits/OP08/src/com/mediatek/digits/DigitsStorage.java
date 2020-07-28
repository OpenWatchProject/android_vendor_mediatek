package com.mediatek.digits;

import android.content.Context;
import android.util.Log;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;


import com.mediatek.digits.DigitsLine;
import com.mediatek.digits.iam.MsisdnAuth;
import com.mediatek.digits.iam.UserProfile;
import com.mediatek.digits.ses.SesManager;


public class DigitsStorage {
    static protected final String TAG = "DigitsStorage";

    // Use device ID as key
    class InnerDevice {

        private String mDeviceId;
        private String mDeviceName;
        private int mDeviceType;

        // <msisdn, friendly name>
        Map<String, String> mLines = new HashMap<String, String>();

        String getDeviceId() {
            return mDeviceId;
        }

        void setDeviceId(String deviceId) {
            mDeviceId = deviceId;
        }

        String getDeviceName() {
            return mDeviceName;
        }

        void setDeviceName(String deviceName) {
            mDeviceName = deviceName;
        }

        int getDeviceType() {
            return mDeviceType;
        }

        void setDeviceType(int deviceType) {
            mDeviceType = deviceType;
        }

        Map<String, String> getLines() {
            return mLines;
        }

        void setLines(Map<String, String> lines) {
            mLines = lines;
        }

        // For debugging purposes only.
        @Override
        public String toString() {
            return "InnerDevice {mDeviceId=" + mDeviceId
                    + ", mDeviceName=" + mDeviceName
                    + ", mDeviceType=" + mDeviceType
                    + ", mLines=" + mLines
                    + "}";
        }
    }

    // Use MSISDN as key
    class InnerLine {

        // Line attribute
        private String mMsisdn;
        private String mLineName;
        private String mLineType;
        private String mLineSit;
        private int mLineStatus;
        private int mLineColor;

        String getMsisdn() {
            return mMsisdn;
        }

        void setMsisdn(String msisdn) {
            mMsisdn = msisdn;
        }

        String getLineName() {
            return mLineName;
        }

        void setLineName(String lineName) {
            mLineName = lineName;
        }

        String getLineType() {
            return mLineType;
        }

        void setLineType(String lineType) {
            mLineType = lineType;
        }

        String getLineSit() {
            return mLineSit;
        }

        void setLineSit(String lineSit) {
            mLineSit = lineSit;
        }

        int getLineStatus() {
            return mLineStatus;
        }

        void setLineStatus(int status) {
            mLineStatus = status;
        }

        int getLineColor() {
            return mLineColor;
        }

        void setLineColor(int color) {
            mLineColor = color;
        }

        // For debugging purposes only.
        @Override
        public String toString() {
            return "InnerLine {mMsisdn=" + mMsisdn
                    + ", mLineName=" + mLineName
                    + ", mLineType=" + mLineType
                    + ", mLineSit=" + mLineSit
                    + ", mLineStatus=" + mLineStatus
                    + ", mLineColor=" + mLineColor
                    + "}";
        }
    }

    Context mContext;

    // Device info
    private InnerDevice mMyDevice;
    private HashMap<String, InnerDevice> mDeviceMap = new HashMap<String, InnerDevice>();

    // Line info
    private InnerLine mSimLine;
    private HashMap<String, InnerLine> mVirtualLineMap = new HashMap<String, InnerLine>();

    private UserProfile mConsumerProfile;

    // Key is transaction id
    private HashMap<String, MsisdnAuth> mMsisdnAuthMap = new HashMap<String, MsisdnAuth>();

    // Constructor
    public DigitsStorage(Context context) {

        mContext = context;

        mMyDevice = new InnerDevice();
        initMyDeviceId();

        mSimLine = new InnerLine();

        // Initialize as ACTIVATED
        mSimLine.setLineStatus(DigitsLine.LINE_STATUS_ACTIVATED);
    }

    public void initDeviceMap(HashMap<String, SesManager.DeviceInfo> deviceInfo) {
        Log.d(TAG, "initDeviceMap()");

        mDeviceMap.clear();

        for (Map.Entry<String, SesManager.DeviceInfo> entry : deviceInfo.entrySet()) {

            String id = entry.getKey();
            SesManager.DeviceInfo info = entry.getValue();

            boolean ifMyDevice = getIsMyDevice(id);

            InnerDevice device = ifMyDevice ? mMyDevice : new InnerDevice();

            device.setDeviceId(info.mDeviceId);
            device.setDeviceName(info.mDeviceName);
            device.setDeviceType(info.mDeviceType);

            Map<String, String> lines = new HashMap<String, String>();
            for (SesManager.ServiceInstance instance : info.mServiceInstances.values()) {
                lines.put(instance.mMsisdn, instance.mFriendlyName);
            }
            device.setLines(lines);

            // Add other device to device map
            if (!ifMyDevice) {
                mDeviceMap.put(id, device);
            }

            // Remove activated line if not in device line
            // Mark: try to activate the line to restore service-instance-id
            //if (ifMyDevice) {
            //    clarifyActivatedLine(lines.keySet());
            //}

            Log.d(TAG, "initDeviceMap(), device id:" + id + ", my:" + ifMyDevice +
                ", name:" + info.mDeviceName + ", type:" + info.mDeviceType + ", lines:" + lines);
        }
    }

    public DigitsDevice[] getRegisteredDevice() {

        ArrayList<DigitsDevice> devices = new ArrayList<DigitsDevice>();

        // My device
        Set<String> msisdns = mMyDevice.getLines().keySet();

        DigitsDevice myDevice = new DigitsDevice(
            mMyDevice.getDeviceId(), mMyDevice.getDeviceName(), mMyDevice.getDeviceType(), true,
            msisdns.toArray(new String[msisdns.size()]));

        devices.add(myDevice);

        // Other device
        for (InnerDevice d : mDeviceMap.values()) {

            Set<String> m = d.getLines().keySet();

            DigitsDevice device = new DigitsDevice(
                d.getDeviceId(), d.getDeviceName(), d.getDeviceType(), false,
                m.toArray(new String[m.size()]));

            devices.add(device);
        }
        Log.d(TAG, "getRegisteredDevice() devices:" + devices);

        return devices.toArray(new DigitsDevice[devices.size()]);
    }

    public void initLineMap(HashMap<String, SesManager.RegisteredMSISDN> msisdnInfo) {
        Log.d(TAG, "initLineMap()");

        mVirtualLineMap.clear();

        for (Map.Entry<String, SesManager.RegisteredMSISDN> entry : msisdnInfo.entrySet()) {

            String msisdn = entry.getKey();
            SesManager.RegisteredMSISDN info = entry.getValue();

            boolean ifNative = getIsNative(msisdn);

            String lineType = getTypefromMeta(info.mLineMetadata);

            // Apply the friendly name from reigsteredDevices response first
            // Mark the design due to device line name is often out of date
            String lineName = null; // getMyDeviceLineName(msisdn);

            if (ifNative) {

                mSimLine.setLineName(lineName == null ? info.mLineName : lineName);
                mSimLine.setLineType(lineType);

                mSimLine.setLineStatus(DigitsLine.LINE_STATUS_ACTIVATED);

            } else {

                InnerLine innerLine = new InnerLine();
                innerLine.setMsisdn(msisdn);
                innerLine.setLineName(lineName == null ? info.mLineName : lineName);
                innerLine.setLineType(lineType);

                int color = getUnusedColor();
                innerLine.setLineColor(color);
                innerLine.setLineStatus(DigitsLine.LINE_STATUS_APPROVED);

                mVirtualLineMap.put(msisdn, innerLine);
            }

            Log.d(TAG, "initLineMap(), msisdn:" + msisdn + ", native:" + ifNative +
                ", name:" + info.mLineName + ", type:" + lineType);
        }

        clarifyActivatedLine(mVirtualLineMap.keySet());
    }

    public void updateLineMap(HashMap<String, SesManager.RegisteredMSISDN> msisdnInfo) {
        Log.d(TAG, "updateLineMap()");

        // Remove line
        for (String m : new HashSet<String>(mVirtualLineMap.keySet())) {

            if (!msisdnInfo.keySet().contains(m)) {

                mVirtualLineMap.remove(m);
                Log.d(TAG, "updateLineMap(), remove msisdn:" + m);
            }
        }

        // Update: msisdn, line name, line type
        // No update: line sit, line status, line color
        for (Map.Entry<String, SesManager.RegisteredMSISDN> entry : msisdnInfo.entrySet()) {
            String msisdn = entry.getKey();
            SesManager.RegisteredMSISDN info = entry.getValue();

            InnerLine innerLine = getInnerLine(msisdn);

            // new line
            if (innerLine == null) {

                String lineType = getTypefromMeta(info.mLineMetadata);

                InnerLine newLine = new InnerLine();
                newLine.setMsisdn(msisdn);
                newLine.setLineName(info.mLineName);
                newLine.setLineType(lineType);

                int color = getUnusedColor();
                newLine.setLineColor(color);
                newLine.setLineStatus(DigitsLine.LINE_STATUS_APPROVED);

                mVirtualLineMap.put(msisdn, newLine);

                Log.d(TAG, "updateLineMap(), new msisdn:" + msisdn + ", name:" + info.mLineName +
                    ", type:" + lineType + ", color:" + color);

            } else {

                String lineType = getTypefromMeta(info.mLineMetadata);

                // Update line name and line type
                innerLine.setLineName(info.mLineName);
                innerLine.setLineType(lineType);

                Log.d(TAG, "updateLineMap(), old msisdn:" + msisdn + ", name:" + info.mLineName +
                    ", type:" + lineType);
            }
        }

        clarifyActivatedLine(mVirtualLineMap.keySet());
    }

    public void setApprovedAuthLine(String msisdn, boolean waitActivate) {
        Log.d(TAG, "setApprovedAuthLine(), msisdn: " + msisdn + ", waitActivate:" + waitActivate);

        if (waitActivate) {

            // Add an approve line which is waiting for activate
            if (!mVirtualLineMap.containsKey(msisdn)) {

                InnerLine innerLine = new InnerLine();
                innerLine.setMsisdn(msisdn);
                innerLine.setLineStatus(DigitsLine.LINE_STATUS_WAIT_FOR_ACTIVATE);

                int color = getUnusedColor();
                innerLine.setLineColor(color);

                mVirtualLineMap.put(msisdn, innerLine);
            }
        } else {

            // If fail to activate wait_for_activate line, change state as approved
            InnerLine innerLine = getInnerLine(msisdn);

            if (innerLine != null &&
                innerLine.getLineStatus() == DigitsLine.LINE_STATUS_WAIT_FOR_ACTIVATE) {

                innerLine.setLineStatus(DigitsLine.LINE_STATUS_APPROVED);
            }
        }

        Log.d(TAG, "setApprovedAuthLine(), virtual line map:" + mVirtualLineMap);
    }

    public DigitsLine[] getRegisteredLine(boolean simOnly) {

        ArrayList<DigitsLine> lines = new ArrayList<DigitsLine>();

        // Sim line
        DigitsLine simLine = new DigitsLine(
            mSimLine.getMsisdn(),
            mSimLine.getLineName(),
            mSimLine.getLineType(),
            false,
            mSimLine.getLineStatus(),
            mSimLine.getLineColor(),
            mSimLine.getLineSit());
        lines.add(simLine);

        if (!simOnly) {
            // Virtual line
            for (InnerLine line : mVirtualLineMap.values()) {

                // No include the line wait for activate
                int status = line.getLineStatus();
                if (status == DigitsLine.LINE_STATUS_WAIT_FOR_ACTIVATE) {
                    continue;
                }

                DigitsLine virtual = new DigitsLine(
                    line.getMsisdn(),
                    line.getLineName(),
                    line.getLineType(),
                    true,
                    line.getLineStatus(),
                    line.getLineColor(),
                    line.getLineSit());
                lines.add(virtual);
            }
        }
        Log.d(TAG, "getRegisteredLine(), simOnly:" + simOnly + " " + lines);

        return lines.toArray(new DigitsLine[lines.size()]);
    }

    public ArrayList<String> getWaitForActivateMsisdn() {

        ArrayList<String> msisdns = new ArrayList<String>();

        // Virtual line
        for (InnerLine line : mVirtualLineMap.values()) {

            if (line.getLineStatus() == DigitsLine.LINE_STATUS_WAIT_FOR_ACTIVATE) {
                msisdns.add(line.getMsisdn());
            }
        }

        return msisdns;
    }

    public ArrayList<String> getNativeAndSitMsisdn() {

        ArrayList<String> msisdns = new ArrayList<String>();

        // Sim line
        msisdns.add(mSimLine.getMsisdn());

        // Virtual line
        for (InnerLine line : mVirtualLineMap.values()) {

            if (line.getLineSit() != null) {
                msisdns.add(line.getMsisdn());
            }
        }

        Log.d(TAG, "getNativeAndSitMsisdn() msisdns:" + msisdns);
        return msisdns;
    }

    public void setSitLineActivated() {

        Set<String> msisdns = new HashSet<String>();

        for (InnerLine line : mVirtualLineMap.values()) {

            if (line.getLineSit() != null) {

                // Avoid to change status from registered to activated
                if (line.getLineStatus() == DigitsLine.LINE_STATUS_APPROVED ||
                    line.getLineStatus() == DigitsLine.LINE_STATUS_WAIT_FOR_ACTIVATE) {

                    line.setLineStatus(DigitsLine.LINE_STATUS_ACTIVATED);
                }

                msisdns.add(line.getMsisdn());
            } else {

                // Avoid to change status from wait_for_activate to approved
                if (line.getLineStatus() == DigitsLine.LINE_STATUS_REGISTERED||
                    line.getLineStatus() == DigitsLine.LINE_STATUS_ACTIVATED) {

                    line.setLineStatus(DigitsLine.LINE_STATUS_APPROVED);
                }
            }
        }
        Log.d(TAG, "setSitLineActivated(), msisdns:" + msisdns);

        // Save to shared preference
        DigitsSharedPreference pref = DigitsSharedPreference.getInstance(mContext);
        pref.saveActivatedVirtualLines(msisdns);


    }

    public void updateRegisteredStatus(String msisdn, boolean registered) {
        Log.d(TAG, "updateRegisteredStatus(), msisdn: " + msisdn + ", registered:" + registered);

        InnerLine innerLine = getInnerLine(msisdn);

        if (innerLine != null) {

            int status = innerLine.getLineStatus();
            if (status == DigitsLine.LINE_STATUS_ACTIVATED && registered) {

                innerLine.setLineStatus(DigitsLine.LINE_STATUS_REGISTERED);

            } else if (status == DigitsLine.LINE_STATUS_REGISTERED && !registered) {

                innerLine.setLineStatus(DigitsLine.LINE_STATUS_ACTIVATED);
            }

        } else {
            Log.e(TAG, "setRegisteredStatus(), invalid msisdn: " + msisdn);
        }
    }

    public DigitsProfile getConsumerProfile() {

        DigitsProfile profile = new DigitsProfile(
            mConsumerProfile.getData(UserProfile.USER_ORIG_TMOBILEID_KEY),
            mConsumerProfile.getData(UserProfile.USER_EMAIL_KEY),
            mConsumerProfile.getData(UserProfile.USER_FIRSTNAME_KEY),
            mConsumerProfile.getData(UserProfile.USER_LASTNAME_KEY));

        Log.d(TAG, "getConsumerProfile() profile:" + profile);
        return profile;

    }

    public void setConsumerProfile(HashMap<String, UserProfile> profile) {
        Log.d(TAG, "setConsumerProfile(), profile:" + profile);

        // Assume there is only one profile
        for (UserProfile p : profile.values()) {
            mConsumerProfile = p;
        }
    }

    public void initMsisdnAuthList(HashMap<String, MsisdnAuth> msisdnAuth) {

        mMsisdnAuthMap.clear();
        for (Map.Entry<String, MsisdnAuth> entry : msisdnAuth.entrySet()) {

            String tid = entry.getKey();
            MsisdnAuth auth = entry.getValue();

            // status: pending|approved|timeout|rejected|cancelled
            // No store can't cancel request (cancelled and approved)
            if (!auth.getStatus().equals("cancelled") && !auth.getStatus().equals("approved")) {
                mMsisdnAuthMap.put(tid, auth);
            }
        }
        Log.d(TAG, "initMsisdnAuthList(), list:" + mMsisdnAuthMap.values());
    }

    public MsisdnAuth getNextMsisdnAuth(String msisdn) {

        for (Map.Entry<String, MsisdnAuth> entry : mMsisdnAuthMap.entrySet()) {
            String tid = entry.getKey();
            MsisdnAuth auth = entry.getValue();

            if (auth.getMsisdn().equals(msisdn)) {
                Log.d(TAG, "getNextMsisdnAuth(), msisdn:" + msisdn + ", auth:" + auth);
                return auth;
            }
        }
        Log.d(TAG, "getNextMsisdnAuth(), msisdn:" + msisdn + ", auth: null");
        return null;
    }

    public void removeMsisdnAuth(String tid) {

        mMsisdnAuthMap.remove(tid);

        Log.d(TAG, "removeMsisdnAuth(), list:" + mMsisdnAuthMap.values());
    }

    public void setDeviceName(String deviceName) {
        Log.d(TAG, "setDeviceName(), name:" + deviceName);

        mMyDevice.setDeviceName(deviceName);

        // Save to shared preference
        DigitsSharedPreference pref = DigitsSharedPreference.getInstance(mContext);
        pref.saveDeviceName(deviceName);
    }

    public String getDeviceName() {
        //TODO: get other device name
        return mMyDevice.getDeviceName();
    }

    public void setLineName(String msisdn, String lineName) {
        Log.d(TAG, "setLineName(), msisdn:" + msisdn + ", name:" + lineName);
        InnerLine innerLine = getInnerLine(msisdn);

        if (innerLine != null) {
            innerLine.setLineName(lineName);
        } else {
            Log.d(TAG, "setLineName(), invalid msisdn: " + msisdn);
        }
    }

    public void setLineColor(String msisdn, int lineColor) {
        Log.d(TAG, "setLineColor(), msisdn:" + msisdn + ", color:" + lineColor);
        InnerLine innerLine = getInnerLine(msisdn);

        if (innerLine != null) {
            innerLine.setLineColor(lineColor);
        } else {
            Log.d(TAG, "setLineColor(), invalid msisdn: " + msisdn);
        }

        // TODO: save to shared preference
    }

    public void setLineSit(String msisdn, String lineSit) {
        Log.d(TAG, "setLineSit(), msisdn:" + msisdn + ", lineSit:" + lineSit);

        InnerLine innerLine = getInnerLine(msisdn);

        if (innerLine != null) {
            innerLine.setLineSit(lineSit);
        } else {
            Log.d(TAG, "setLineSit(), invalid msisdn: " + msisdn);
        }
    }

    public String getLineSit(String msisdn) {

        InnerLine innerLine = getInnerLine(msisdn);

        if (innerLine != null) {
            return innerLine.getLineSit();
        } else {
            Log.d(TAG, "getLineSit(), invalid msisdn: " + msisdn);
            return null;
        }
    }

    public boolean getIsNative(String msisdn) {

        if (msisdn.equals(mSimLine.getMsisdn())) {
            return true;
        } else {
            return false;
        }
    }

    public void setNativeMsisdn(String msisdn) {
        Log.d(TAG, "setNativeMsisdn(), msisdn:" + msisdn);

        mSimLine.setMsisdn(msisdn);
    }

    public String getNativeMsisdn() {
        return mSimLine.getMsisdn();
    }

    public boolean isRegisteredLineUpdated(DigitsLine[] a, DigitsLine[] b) {
        if (a == null && b == null) {
            return false;

        } if ((a == null && b != null) || (a != null && b == null)) {
            return true;

        } else if (a.length != b.length){
            return true;

        } else {
            for (DigitsLine aa : a) {
                boolean found = false;

                for (DigitsLine bb : b) {
                    if (aa.sameAs(bb)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return true;
                }
            }
            return false;
        }
    }

    public boolean getIsMyDevice(String deviceId) {

        if (deviceId.equals(mMyDevice.getDeviceId())) {
            return true;
        } else {
            return false;
        }
    }

    private void clarifyActivatedLine(Set<String> availableLine) {

        DigitsSharedPreference pref = DigitsSharedPreference.getInstance(mContext);
        Set<String> activated = pref.getActivatedVirtualLines();

        if (activated.isEmpty()) {
            return;
        }

        for (String m : new HashSet<String>(activated)) {

            if (!availableLine.contains(m)) {
                activated.remove(m);

                Log.d(TAG, "clarifyActivatedLine(), " + m + " is removed from shared preference");

                /*
                // The line shouldn't be activated line
                InnerLine innerLine = getInnerLine(m);
                if (innerLine != null) {

                    Log.d(TAG, "clarifyActivatedLine(), " + m + " is forced be approved");
                    innerLine.setLineStatus(DigitsLine.LINE_STATUS_APPROVED);
                    innerLine.setLineSit(null);
                }
                */
            }
        }

        // Save to shared preference
        pref.saveActivatedVirtualLines(activated);
    }

    private InnerLine getInnerLine(String msisdn) {
        if (msisdn.equals(mSimLine.getMsisdn())) {
            return mSimLine;
        } else {
            return mVirtualLineMap.get(msisdn);
        }
    }

    private void initMyDeviceId() {

        String imei = DigitsUtil.getInstance(mContext).getDeviceId();

        Log.d(TAG, "initMyDeviceId(), imei:" + imei);

        mMyDevice.setDeviceId(imei);
    }

    private String getMyDeviceLineName(String msisdn) {

        Map<String, String> lines = mMyDevice.getLines();

        String name = lines.get(msisdn);

        Log.d(TAG, "getMyDeviceLineName(), msisdn:" + msisdn + ", name:" + name);
        return name;

    }

    private String getTypefromMeta(String metaData) {

        // line-type="gsm"
        return metaData.substring(11, metaData.length() - 1);
    }

    private int getUnusedColor() {

        int[] colorArr = mContext.getResources().getIntArray(
            com.mediatek.digits.R.array.line_colors);

        for (int i = 0; i < colorArr.length; i++) {

            // Virtual line
            boolean sameColor = false;

            for (InnerLine line : mVirtualLineMap.values()) {

                if (line.getLineColor() == colorArr[i]) {

                    sameColor = true;
                    break;
                }
            }

            if (!sameColor) {
                return colorArr[i];
            }
        }

        return colorArr[mVirtualLineMap.size() % colorArr.length];
    }

}
