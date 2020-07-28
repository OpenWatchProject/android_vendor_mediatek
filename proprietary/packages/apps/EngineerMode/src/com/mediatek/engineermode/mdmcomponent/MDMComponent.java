/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.mdmcomponent;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Environment;
import android.os.SystemProperties;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ScrollView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;
import com.mediatek.mdml.Msg;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

abstract public class MDMComponent implements Comparable<MDMComponent> {
    protected static final String TAG = "EmInfo/MDMComponent";

    static private List<MDMComponent> mComponents = null;
    protected Activity mActivity;


    public MDMComponent(Activity context) {
        mActivity = context;
    }

    static List<MDMComponent> getComponents(Activity context) {

        if (mComponents == null)
            mComponents = new ArrayList<MDMComponent>();
        else
            return mComponents;
        mComponents.add(new RRCState(context));
        mComponents.add(new RRState(context));
        mComponents.add(new ERRCState(context));
        mComponents.add(new HighPriorityPLMNSearch(context));
        mComponents.add(new CarrierRSSIServing(context));
        mComponents.add(new FTNetworkInfo(context, 2));

        mComponents.add(new EUtranNeighborCellInfo(context));
        mComponents.add(new TddServingCellInfo(context));
        mComponents.add(new LaiRaiUmtsTdd(context));
        mComponents.add(new GeranNeighborCellInfoUmtsTdd(context));
        mComponents.add(new EUtranNeighborCellInfoUmtsTdd(context));
        mComponents.add(new LteServingCellInfo(context));
        mComponents.add(new LteSCellInfo(context));
        mComponents.add(new PrimaryCellDlBlockErrorRate(context));
        mComponents.add(new PrimaryCellDownlinkBandwidth(context));
        mComponents.add(new PrimaryCellUplinkBandwidth(context));
        mComponents.add(new PrimaryCellUplinkDownlinkConfiguration(context));
        mComponents.add(new PrimaryCellSpecialSubframeConfiguration(context));
        mComponents.add(new PrimaryCellTransmissionMode(context));
        mComponents.add(new SecondaryCellDlBlockErrorRate(context));
        mComponents.add(new SecondaryCellDownlinkBandwidth(context));
        mComponents.add(new SecondaryCellUplinkBandwidth(context));
        mComponents.add(new SecondaryCellUplinkDownlinkConfiguration(context));
        mComponents.add(new SecondaryCellSpecialSubframeConfiguration(context));
        mComponents.add(new SecondaryCellTransmissionMode(context));
        mComponents.add(new Tai(context));
        mComponents.add(new EmIrReport(context));
        mComponents.add(new UmtsRrcMeasurementControlForE3a(context));
        mComponents.add(new UmtsRrcMeasurementControlForE3b(context));
        mComponents.add(new UmtsRrcMeasurementControlForE3c(context));
        mComponents.add(new UmtsRrcMeasurementControlForE3d(context));
        mComponents.add(new UmtsRrcMeasurementReportForE3a(context));
        mComponents.add(new UmtsRrcMeasurementReportForE3b(context));
        mComponents.add(new UmtsRrcMeasurementReportForE3c(context));
        mComponents.add(new UmtsRrcMeasurementReportForE3d(context));
        mComponents.add(new UmtsNeighborCellInfoGsmTdd(context));
        mComponents.add(new LTENeighborCellInfoGSM(context));
        mComponents.add(new LteErrcMeasurementConfigForEventB1(context));
        mComponents.add(new LteErrcMeasurementConfigForEventB2(context));
        mComponents.add(new LteErrcMeasurementReportForEventB1(context));
        mComponents.add(new LteErrcMeasurementReportForEventB2(context));
        mComponents.add(new EDchTtiConfigured(context));
        mComponents.add(new HsdpaConfiguredUmtsFdd(context));
        mComponents.add(new DcHsdpaConfiguredUmtsFdd(context));
        mComponents.add(new HsDschModulationPrimaryCell(context));
        mComponents.add(new HsupaConfiguredUmtsFdd(context));
        mComponents.add(new WcdmaHsupaCapability(context));
        mComponents.add(new WcdmaHsdpaCapability(context));
        mComponents.add(new Fdd3gSpeechCodec(context));
        mComponents.add(new PlmnSearchStatusUmtsFdd(context));
        mComponents.add(new CellSupportPsInfo(context));
        mComponents.add(new DtxConfiguredUmtsFdd(context));
        mComponents.add(new DrxConfiguredUmtsFdd(context));
        mComponents.add(new FastDormancyConfiguration(context));
        mComponents.add(new IntraFrequencyMonitoredSetUmtsFdd(context));
        mComponents.add(new IntraFrequencyDetectedSetUmtsFdd(context));
        mComponents.add(new ActiveSetUmtsFdd(context));
        mComponents.add(new CsOverHspaUmtsFdd(context));
        mComponents.add(new ShareNetworkPlmnInfo(context));
        mComponents.add(new ServingCellInformationUmtsFdd(context));
        mComponents.add(new PrimaryCellCqi(context));
        mComponents.add(new PrimaryCellDlImcs(context));
        mComponents.add(new PrimaryCellDlResourceBlock(context));
        mComponents.add(new PrimaryCellAntennaPort(context));
        mComponents.add(new PrimaryCellDlThroughput(context));
        mComponents.add(new PrimaryCellDlBlockRate(context));
        mComponents.add(new SecondaryCellCqi(context));
        mComponents.add(new SecondaryCellDlImcs(context));
        mComponents.add(new SecondaryCellDlResourceBlock(context));
        mComponents.add(new SecondaryCellAntennaPort(context));
        mComponents.add(new SecondaryCellDlThroughput(context));
        mComponents.add(new SecondaryCellDlBlockRate(context));
        mComponents.add(new DownlinkDataThroughput(context));
        mComponents.add(new UplinkDataThroughput(context));
        mComponents.add(new HandoverIntraLte(context));
        mComponents.add(new MDFeatureDetection(context));
        mComponents.add(new RatAndServiceStatus(context));
        mComponents.add(new HsDschServingCellUmtsFdd(context));
        mComponents.add(new SecondHsDschServingCell(context));
        mComponents.add(new BasicInfoServingGsm(context));
        mComponents.add(new RsrpLteCandidateCellUmtsFdd(context));
        mComponents.add(new RsrqLteCandidateCellUmtsFdd(context));
        mComponents.add(new UmtsRscp(context));
        mComponents.add(new UmtsEcn0(context));
        mComponents.add(new IntraFrequencyNeighbouringCellInformationLteTdd(
                context));
        mComponents.add(new InterFrequencyNeighbouringCellInformationLteTdd(
                context));
        mComponents.add(new GeranNeighbouringCellInformation(context));
        mComponents.add(new UtraTddNeighbouringCellInformation(context));
        mComponents.add(new WcdmaTasInfo(context));
        mComponents.add(new WcdmaUTasInfo(context));
        mComponents.add(new GSMTasInfo(context));
        mComponents.add(new GSMUTasInfo(context));
        mComponents.add(new TddTasInfo(context));
        mComponents.add(new TddUTasInfo(context));
        mComponents.add(new PrimaryCellRsrpRx(context));
        mComponents.add(new PrimaryCellRsrp(context));
        mComponents.add(new PrimaryCellRsrq(context));
        mComponents.add(new PrimaryCellRssiRx(context));
        mComponents.add(new PrimaryCellSnrRx(context));
        mComponents.add(new PrimaryCellOsSnr(context));
        mComponents.add(new SecondaryCellRsrpRx(context));
        mComponents.add(new SecondaryCellRsrp(context));
        mComponents.add(new SecondaryCellRsrq(context));
        mComponents.add(new SecondaryCellRssiRx(context));
        mComponents.add(new SecondaryCellSnrRx(context));
        mComponents.add(new SecondaryCellOsSnr(context));
        mComponents.add(new EnhancedRRCState(context));
        mComponents.add(new LteUTasInfo(context));
        mComponents.add(new LteTasInfo(context));
        mComponents.add(new CellStrength(context));
        mComponents.add(new PCellSCellBasicInfo(context));
        mComponents.add(new ErrcFeatureDetection(context));
        mComponents.add(new El2FeatureDetection(context));
        mComponents.add(new LteErlcDlDrbConfiguration(context));
        mComponents.add(new LteErlcUlDrbConfiguration(context));
        mComponents.add(new SecondaryCell(context));
        mComponents.add(new EMMCallInformation(context));
        mComponents.add(new EmacInfo(context));
        mComponents.add(new EPSBearerThroughput(context));
        mComponents.add(new GSMRxdInfo(context));

        mComponents.add(new LteEmacRachFailure(context));
        mComponents.add(new LteErrcRlfEvent(context));
        mComponents.add(new LteErrcOosEvent(context));
        if (FeatureSupport.is93Modem()) {
            // C2k MDM support
            mComponents.add(new Cdma1xrttRadioInfo(context));
            mComponents.add(new Cdma1xrttRadioUTasInfo(context));
            mComponents.add(new EvdoServingInfo(context));
            mComponents.add(new Cdma1xrttInfo(context));
            mComponents.add(new Cdma1xSchInfo(context));
            mComponents.add(new Cdma1xStatisticsInfo(context));

            mComponents.add(new Cdma1xSeringNeihbrInfo(context));
            mComponents.add(new EvdoFlInfo(context));
            mComponents.add(new EvdoRlInfo(context));

            mComponents.add(new EvdoStatueInfo(context));
            mComponents.add(new EvdoSprintXRttInfo(context));
            mComponents.add(new EvdoSprintInfo(context));
            mComponents.add(new EvdoActiveSet(context));
            mComponents.add(new EvdoCandSet(context));
            mComponents.add(new EvdoNghdrSet(context));
        }
        mComponents.add(new SpeechCodecInfo(context));
        mComponents.add(new RFCalibrationStatusCheck(context));
        mComponents.add(new LastRegisteredNetwork(context));
        mComponents.add(new TMSIandPTMSI(context));
        mComponents.add(new PeriodicLocationUpdateValue(context));
        mComponents.add(new RejectCauseCode(context));
        mComponents.add(new ActivePDPContextInformationUmtsFDD(context));
        mComponents.add(new ActivePDPContextInformationLTE(context));

        //ICD
        mComponents.add(new EUTRAMeasurementReport(context));
        return mComponents;
    }

    @Override
    public int compareTo(MDMComponent another) {
        return getName().compareTo(another.getName());
    }

    abstract View getView();

    abstract void removeView();

    abstract String getName();

    abstract String getGroup();

    abstract void clearData();

    // abstract int[] getEmType();
    abstract boolean supportMultiSIM();

    abstract void update(String name, Object msg);


    abstract String[] getEmComponentName();

    boolean hasEmType(String type) {
        String[] types = getEmComponentName();
        if (types != null) {
            for (int i = 0; i < types.length; i++) {
                if (types[i].equals(type)) {
                    return true;
                }
            }
        } else {
            Elog.d(TAG, "type is null");
        }
        return false;
    }

    public int getFieldValue(Msg data, String msgName) {

        byte[] bData = data.getFieldValue(msgName);
        if (bData == null) {
            Elog.d(TAG, "[getFieldValue] returned null from :" + msgName);
        }
        int iData = (int) Utils.getIntFromByte(bData);
//        Elog.d(TAG, "[getFieldValue] msgName: " + msgName + ":" + iData);
        return iData;
    }

    public int getFieldValue(Msg data, String msgName, boolean sign) {
        if (!sign) {
            return getFieldValue(data, msgName);
        }
        byte[] bData = data.getFieldValue(msgName);

        if (bData == null) {
            Elog.d(TAG, "[getFieldValue] returned null from :" + msgName);
        }
        int iData = (int) Utils.getIntFromByte(bData, true);
//        Elog.d(TAG, "[getFieldValue] msgName: " + msgName + ":" + iData);
        return iData;
    }

    public int getFieldValueIcd(ByteBuffer icdPacket, int offset, int length, boolean sign) {
        int value = getFieldValueIcd(icdPacket, offset, length);
        if (!sign) {
            return value;
        }
        int endByte = (offset + length - 1) / 8 + 21;
        if ((icdPacket.get(endByte) & 0x80) != 0) {
            return (~value)  ;
        }
        return value;
    }
    public int getFieldValueIcdVersion(ByteBuffer icdPacket) {
        int version = icdPacket.get(20);
        return version;
    }

    public int getFieldValueIcd(ByteBuffer icdPacket, int offset, int length) {
        int value = 0;
        int beginBit = offset % 8;
        int beginByte = offset / 8 + 21;
        int endByte = (offset + length - 1) / 8 + 21;
        if (endByte > beginByte) {
            value = (icdPacket.get(beginByte) & 0xff) >> beginBit;
            int shiftBit = 8 - beginBit;
            for (int iByte = beginByte + 1; iByte < endByte; ++iByte) {
                value |= (icdPacket.get(iByte) & 0xff) << shiftBit;
                shiftBit += 8;
            }
            int remainBits = (offset + length) % 8;
            if (remainBits == 0) {
                remainBits = 8;
            }
            value |= (icdPacket.get(endByte) & ((1 << remainBits) - 1)) << shiftBit;
        } else {
            value = (icdPacket.get(beginByte) >> beginBit) & ((1 << length) - 1);
        }
        return value;
    }

    String[] addLablesAtPosition(String[] a, String[] b, int position) {
        String[] c = new String[a.length + b.length];
        if (position >= a.length) {
            System.arraycopy(a, 0, c, 0, a.length);
            System.arraycopy(b, 0, c, a.length, b.length);
        } else {
            for (int i = 0; i < a.length; i++) {
                if (i < position) {
                    c[i] = a[i];
                } else if (i == position) {
                    for (int j = 0; j < b.length; j++) {
                        c[i + j] = b[j];
                    }
                    c[i + b.length] = a[i];
                } else {
                    c[i + b.length] = a[i];
                }
            }
        }
        return c;
    }

    String[] removeLablesAtPosition(String[] a, int position, int num) {
        if (position >= a.length) {
            return a;
        }
        String[] c = new String[a.length - num];
        for (int i = 0; i < a.length - num; i++) {
            if (i < position) {
                c[i] = a[i];
            } else if (i == position) {
                if ((a.length - position) <= num) {
                    break;
                }
                c[i] = a[i + num];
            } else {
                c[i] = a[i + num];
            }
        }
        return c;
    }

    public <T> T[] concatAll(T[] first, T[]... rest) {
        int totalLength = first.length;
        for (T[] array : rest) {
            totalLength += array.length;
        }
        T[] result = Arrays.copyOf(first, totalLength);
        int offset = first.length;
        for (T[] array : rest) {
            System.arraycopy(array, 0, result, offset, array.length);
            offset += array.length;
        }
        return result;
    }

    public void saveToSDCard(String dirName, String fileName, String content, boolean append)
            throws IOException {
        String path = Environment.getExternalStorageDirectory().getPath() + dirName;
        File filedDir = new File(path);
        if (!filedDir.exists())
            filedDir.mkdir();
        File file = new File(path, fileName);
        FileOutputStream fos = new FileOutputStream(file, append);
        fos.write(content.getBytes());
        fos.write("\r\n".getBytes());
        fos.close();
    }

    public String getCurrectTime() {
        SimpleDateFormat mCurrectTime = null;
        mCurrectTime = new SimpleDateFormat("yyyy_MM_dd_HH_mm_ss_SS");
        String mTimes = mCurrectTime.format(new Date());
        return mTimes;
    }

    public boolean diff_time(String lastTime, String CurTime, int diff_s) {
        Date dlast = null;
        Date dcurrent = null;
        SimpleDateFormat mCurrectTime = null;
        mCurrectTime = new SimpleDateFormat("yyyy_MM_dd_HH_mm_ss_SS");

        try {
            dlast = mCurrectTime.parse(lastTime);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        try {
            dcurrent = mCurrectTime.parse(CurTime);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        long diff = dcurrent.getTime() - dlast.getTime();
        if (diff > diff_s) {
            return true;
        } else {
            return false;
        }
    }
}


class TableInfoAdapter extends ArrayAdapter<String[]> {
    private Activity mActivity;

    public TableInfoAdapter(Activity activity) {
        super(activity, 0);
        mActivity = activity;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;
        LayoutInflater inflater = mActivity.getLayoutInflater();
        if (convertView == null) {
            convertView = LayoutInflater.from(mActivity).inflate(R.layout.em_info_entry, parent,
                    false);
            holder = new ViewHolder();
            holder.texts = new MdmTextView[16];
            holder.seps = new View[15];
            holder.texts[0] = (MdmTextView) convertView.findViewById(R.id.info1);
            holder.texts[1] = (MdmTextView) convertView.findViewById(R.id.info2);
            holder.texts[2] = (MdmTextView) convertView.findViewById(R.id.info3);
            holder.texts[3] = (MdmTextView) convertView.findViewById(R.id.info4);
            holder.texts[4] = (MdmTextView) convertView.findViewById(R.id.info5);
            holder.texts[5] = (MdmTextView) convertView.findViewById(R.id.info6);
            holder.texts[6] = (MdmTextView) convertView.findViewById(R.id.info7);
            holder.texts[7] = (MdmTextView) convertView.findViewById(R.id.info8);
            holder.texts[8] = (MdmTextView) convertView.findViewById(R.id.info9);
            holder.texts[9] = (MdmTextView) convertView.findViewById(R.id.info10);
            holder.texts[10] = (MdmTextView) convertView.findViewById(R.id.info11);
            holder.texts[11] = (MdmTextView) convertView.findViewById(R.id.info12);
            holder.texts[12] = (MdmTextView) convertView.findViewById(R.id.info13);
            holder.texts[13] = (MdmTextView) convertView.findViewById(R.id.info14);
            holder.texts[14] = (MdmTextView) convertView.findViewById(R.id.info15);
            holder.texts[15] = (MdmTextView) convertView.findViewById(R.id.info16);
            for (int i = 0; i < holder.seps.length; i++) {
                holder.seps[i] = convertView.findViewWithTag(i + "");
            }
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        String[] text = getItem(position);
        for (int i = 0; i < text.length && i < holder.texts.length; i++) {
            holder.texts[i].setText(text[i]);
            holder.texts[i].setVisibility(View.VISIBLE);
            if (i < holder.seps.length && holder.seps[i] != null) {
                LinearLayout.LayoutParams st =
                        (LinearLayout.LayoutParams) holder.seps[i].getLayoutParams();
                st.width = 1;
                holder.seps[i].setLayoutParams(st);
            }
        }
        for (int i = text.length; i < holder.texts.length; i++) {
            holder.texts[i].setVisibility(View.GONE);
            if (i < holder.seps.length && holder.seps[i] != null) {
                LinearLayout.LayoutParams st =
                        (LinearLayout.LayoutParams) holder.seps[i].getLayoutParams();
                st.width = 0;
                holder.seps[i].setLayoutParams(st);
            }
        }
        return convertView;
    }

    private class ViewHolder {
        public View[] seps;
        public TextView[] texts;
    }
}

abstract class NormalTableComponent extends MDMComponent {
    ListView mListView;
    TableInfoAdapter mAdapter;
    String[] mLabels;

    public NormalTableComponent(Activity context) {
        super(context);
        if (mAdapter == null) {
            mAdapter = new TableInfoAdapter(mActivity);
        }
        if (mListView == null) {
            mListView = new ListView(mActivity);
        }
    }

    @Override
    View getView() {
        if (mLabels == null) {
            mLabels = getLabels();
        }
        if (mAdapter.getCount() == 0) {
            // Show placeholder to avoid showing empty list
            for (int i = 0; i < mLabels.length; i++) {
                mAdapter.add(new String[]{mLabels[i], ""});
            }
        }
        mListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
        return mListView;
    }

    @Override
    void removeView() {
        mListView.setAdapter(null);
    }

    void clearData() {
        mAdapter.clear();
    }

    void addData(Object... data) {
        for (int i = 0; i < data.length; i++) {
            addData(String.valueOf(data[i]));
        }
    }

    void addDataAtPostion(int pos, Object... data) {
        for (int i = 0; i < data.length; i++) {
            pos += i;
            addDataAtPostion(pos, String.valueOf(data[i]));
        }
    }

    void addData(String data) {
        mLabels = getLabels();
        int position = mAdapter.getCount();
        mAdapter.add(new String[]{mLabels[position % mLabels.length],
                data == null ? "" : data});
        mListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
    }

    void addDataAtPostion(int pos, String data) {
        if (mLabels == null) {
            mLabels = getLabels();
        }
        for (int i = mAdapter.getCount(); i <= pos; i++) {
            mAdapter.add(new String[]{mLabels[i], ""});
        }
        String[] item = mAdapter.getItem(pos);
        item[1] = data;
        mListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
    }

    void notifyDataSetChanged() {
        if (mLabels == null) {
            mLabels = getLabels();
        }
        if (mAdapter.getCount() < mLabels.length) {
            // Show placeholder to avoid showing incomplete list
            for (int i = mAdapter.getCount(); i < mLabels.length; i++) {
                mAdapter.add(new String[]{mLabels[i], ""});
            }
        }
        mAdapter.notifyDataSetChanged();
    }

    abstract String[] getLabels();
}

abstract class NormalTableTasComponent extends NormalTableComponent {
    private int infoValid = 0;

    public NormalTableTasComponent(Activity context) {
        super(context);
    }

    @Override
    View getView() {
        Elog.d(TAG, "getView");
        if (isInfoValid()) {
            clearData();
            mAdapter.add(new String[]{"Use " + getName().replace("TAS", "UTAS")});
        } else {
            if (mLabels == null) {
                mLabels = getLabels();
            }
            if (mAdapter.getCount() == 0) {
                // Show placeholder to avoid showing empty list
                for (int i = 0; i < mLabels.length; i++) {
                    mAdapter.add(new String[]{mLabels[i], ""});
                }
            }
        }
        mListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
        return mListView;
    }

    public boolean isInfoValid() {
        if (this.infoValid == 1)
            return true;
        return false;
    }

    public void setInfoValid(int infoValid) {
        this.infoValid = infoValid;
    }

    @Override
    void removeView() {
        mListView.setAdapter(null);
        setInfoValid(0);
    }
}

abstract class ArrayTableComponent extends MDMComponent {
    ListView mListView;
    TableInfoAdapter mAdapter;
    String[] mLabels;

    public ArrayTableComponent(Activity context) {
        super(context);
        if (mAdapter == null) {
            mAdapter = new TableInfoAdapter(mActivity);
        }
        if (mListView == null) {
            mListView = new ListView(mActivity);
        }
    }

    @Override
    View getView() {
        if (mLabels == null) {
            mLabels = getLabels();
        }
        if (mAdapter.getCount() == 0) {
            // Show placeholder to avoid showing empty list
            mAdapter.add(mLabels);
        }
        mListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
        return mListView;
    }

    @Override
    void removeView() {
        mListView.setAdapter(null);
    }

    void clearData() {
        if (mLabels == null) {
            mLabels = getLabels();
        }
        mAdapter.clear();
        mAdapter.add(mLabels);
        mAdapter.notifyDataSetChanged();
    }

    void addData(Object... data) {
        String[] strings = new String[data.length];
        for (int i = 0; i < data.length; i++) {
            strings[i] = String.valueOf(data[i]);
        }
        addData(strings);
    }

    void addData(String[] data) {
        mAdapter.add(data);
        mAdapter.notifyDataSetChanged();
        mListView.setAdapter(mAdapter);
    }

    abstract String[] getLabels();
}

abstract class CurveComponent extends MDMComponent {
    CurveView mCurveView;
    View mView;
    TextView mXLabel;
    TextView mYLabel;
    HashMap<Integer, float[]> mData = new HashMap<Integer, float[]>();
    Random rand = new Random();
    long mStartTime;
    boolean mStarted;

    public CurveComponent(Activity context) {
        super(context);
    }

    @Override
    View getView() {
        if (mView == null) {
            mView = mActivity.getLayoutInflater().inflate(
                    R.layout.mdm_em_info_curve, null);
            mXLabel = (TextView) mView.findViewById(R.id.em_info_curve_x_label);
            mYLabel = (TextView) mView.findViewById(R.id.em_info_curve_y_label);
            mCurveView = (CurveView) mView.findViewById(R.id.em_info_curve);
            if (!mStarted) {
                mStartTime = System.currentTimeMillis() / 10000 * 10;
                mStarted = true;
            }

            mXLabel.setText("Time");
            CurveView.AxisConfig xConfig = new CurveView.AxisConfig();
            xConfig.base = mStartTime;
            xConfig.min = 0;
            xConfig.max = 200;
            xConfig.step = 1;
            xConfig.configMin = true;
            xConfig.configMax = true;
            xConfig.configStep = true;
            xConfig.type = CurveView.AxisConfig.TYPE_TIME;
            CurveView.AxisConfig yConfig = configY();
            mCurveView.setAxisConfig(xConfig, yConfig);
        }
        return mView;
    }

    CurveView.AxisConfig configY() {
        mYLabel.setText("dBm");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF880000;
        configs[0].lineWidth = 2;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = 0;
        yConfig.max = 30;
        yConfig.step = 2;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    void removeView() {
        // mCurveView = null;
    }

    void clearData() {
        ;
    }

    void addData(int index, float data) {
        if (!mStarted) {
            mStartTime = System.currentTimeMillis() / 10000 * 10;
            mStarted = true;
        }

        long time = System.currentTimeMillis() / 1000;

        // TODO: unit test
        // data = ((long) data) % 30;
        // TODO: unit test end

        float[] d = mData.get(index);
        if (d == null) {
            float[] tmp = new float[2];
            tmp[0] = time - mStartTime;
            tmp[1] = data;
            mData.put(index, tmp);
        } else {
            // TODO: unit test
            // if (d.length > 100) return;
            // if ((long) data == (long) d[d.length - 1]) return;
            // TODO: unit test end

            float[] tmp = new float[d.length + 2];
            for (int i = 0; i < d.length; i++) {
                tmp[i] = d[i];
            }
            tmp[d.length] = time - mStartTime;
            tmp[d.length + 1] = data;
            mData.put(index, tmp);
        }
        if (mCurveView != null) {
            mCurveView.setData(index, mData.get(index));
        }
    }
}


abstract class CurveExComponent extends MDMComponent {
    CurveViewEx mCurveView;
    View mView;
    TextView mXLabel;
    TextView mYLabel;
    HashMap<Integer, float[]> mData = new HashMap<Integer, float[]>();

    public CurveExComponent(Activity context) {
        super(context);
    }

    @Override
    View getView() {
        if (mView == null) {
            mView = mActivity.getLayoutInflater().inflate(
                    R.layout.mdm_em_info_curveex, null);
            mXLabel = (TextView) mView.findViewById(R.id.em_info_curve_x_label);
            mYLabel = (TextView) mView.findViewById(R.id.em_info_curve_y_label);
            mCurveView = (CurveViewEx) mView.findViewById(R.id.em_info_curve);

            CurveViewEx.AxisConfig xConfig = configX();
            CurveViewEx.AxisConfig yConfig = configY();
            mCurveView.setAxisConfig(xConfig, yConfig);
            mCurveView.getRSRPSINRConfig();
        }
        return mView;
    }

    CurveViewEx.AxisConfig configX() {
        mXLabel.setText("RSRP");
        CurveViewEx.AxisConfig xConfig = new CurveViewEx.AxisConfig();
        xConfig.min = -140;
        xConfig.max = -30;
        xConfig.step = 10;
        xConfig.configMin = true;
        xConfig.configMax = true;
        xConfig.configStep = true;
        return xConfig;
    }

    CurveViewEx.AxisConfig configY() {
        mYLabel.setText("SNR");
        CurveViewEx.Config[] configs = new CurveViewEx.Config[5];
        configs[0] = new CurveViewEx.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveViewEx.Config.TYPE_CIRCLE;
        configs[0].name = "PCell";
        configs[1] = new CurveViewEx.Config();
        configs[1].color = 0xFFFF0000;
        configs[1].lineWidth = 3;
        configs[1].nodeType = CurveViewEx.Config.TYPE_CIRCLE;
        configs[1].name = "SCell";
        configs[2] = new CurveViewEx.Config();
        configs[2].color = Color.rgb(43, 101, 171);
        configs[2].lineWidth = 3;
        configs[2].nodeType = CurveViewEx.Config.TYPE_RHOMBUS;
        configs[2].name = "Strength";
        configs[3] = new CurveViewEx.Config();
        configs[3].color = Color.rgb(204, 153, 0);
        configs[3].lineWidth = 3;
        configs[3].nodeType = CurveViewEx.Config.TYPE_SQUARE;
        configs[3].name = "MediumWeak";
        configs[4] = new CurveViewEx.Config();
        configs[4].color = Color.rgb(152, 152, 186);
        configs[4].lineWidth = 3;
        configs[4].nodeType = CurveViewEx.Config.TYPE_TRIANGLE;
        configs[4].name = "Weak";
        mCurveView.setConfig(configs);
        CurveViewEx.AxisConfig yConfig = new CurveViewEx.AxisConfig();
        yConfig.min = -20;
        yConfig.max = 30;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    void removeView() {
        // mCurveView = null;
    }

    void addData(int index, float dataX, float dataY, float dataA, float dataB) {

        float[] d = mData.get(index);
        if (d == null) {
            float[] tmp = new float[4];
            tmp[0] = dataX;
            tmp[1] = dataY;
            tmp[2] = dataA;
            tmp[3] = dataB;
            mData.put(index, tmp);
        } else {

            float[] tmp = new float[d.length + 4];
            for (int i = 0; i < d.length; i++) {
                tmp[i] = d[i];
            }
            tmp[d.length] = dataX;
            tmp[d.length + 1] = dataY;
            tmp[d.length + 2] = dataA;
            tmp[d.length + 3] = dataB;
            mData.put(index, tmp);
        }
        if (mCurveView != null) {
            mCurveView.setData(index, mData.get(index));
        }
    }

    void clearData() {
        mData.clear();
    }
}


abstract class CombinationTableComponent extends MDMComponent {

    List<LinkedHashMap<String, LinkedHashMap>> hmapLabelsList =
            new ArrayList<LinkedHashMap<String, LinkedHashMap>>();
    List<HashMap<String, MdmLinearLayout>> hmapViewList =
            new ArrayList<HashMap<String, MdmLinearLayout>>();
    List<HashMap<String, TableInfoAdapter>> hmapAdapterList =
            new ArrayList<HashMap<String, TableInfoAdapter>>();
    List<String> tabTitleList = new ArrayList<String>();
    ScrollView scrollView;
    LinearLayout layout;
    LinearLayout tabTatileLayout;
    ArrayList<String> arrayTypeLabels;
    Activity mContext;
    private int tabCount = 0;
    private int curTab = 0;
    private int infoValid = 1;

    public CombinationTableComponent(Activity context) {
        super(context);
        mContext = context;
    }

    public boolean isInfoValid() {
        if (infoValid == 1)
            return true;
        return false;
    }

    public void setInfoValid(int utasInfoValid) {
        if (utasInfoValid != this.infoValid) {
            Elog.d(TAG, "Utas Info Valid -> Current :" + utasInfoValid + ", Last: " + this
                    .infoValid);
            if (utasInfoValid == 1) {
                setViewAsTable();
                setCurTab(this.curTab);
            } else {
                setViewAsPlank();
            }
        }
        this.infoValid = utasInfoValid;
    }

    public int getTabCount() {
        return tabCount;
    }

    public void setTabCount(int tabCount) {
        this.tabCount = tabCount;
    }

    public void initTableComponent(String[] tabTitles) {
        Elog.d(TAG, "[initTableComponent]" + Arrays.asList(tabTitles).toString());
        this.tabCount = tabTitles.length;
        this.tabTitleList.clear();
        arrayTypeLabels = getArrayTypeKey();
        for (int i = 0; i < getTabCount(); i++) {
            this.tabTitleList.add(i, tabTitles[i]);
            hmapLabelsList.add(i, new LinkedHashMap<String, LinkedHashMap>());
            hmapViewList.add(i, new HashMap<String, MdmLinearLayout>());
            hmapAdapterList.add(i, new HashMap<String, TableInfoAdapter>());
        }
        initHashMapList();
        if (scrollView == null) {
            scrollView = new ScrollView(mContext);
            scrollView.setLayoutParams(
                    new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
        }
        if (layout == null) {
            layout = new LinearLayout(mContext);
            layout.setOrientation(LinearLayout.VERTICAL);
            layout.setLayoutParams(
                    new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
        }
        if (tabTatileLayout == null) {
            tabTatileLayout = new LinearLayout(mContext);
            tabTatileLayout.setOrientation(LinearLayout.HORIZONTAL);
            tabTatileLayout.setLayoutParams(
                    new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
        }
    }

    void initHashMapList() {
        for (int i = 0; i < getTabCount(); i++) {
            if (hmapLabelsList.get(i) == null ||
                    hmapLabelsList.get(i).keySet().size() == 0) {
                hmapLabelsList.add(i, getHashMapLabels(i));
                hmapViewList.get(i).clear();
                hmapAdapterList.get(i).clear();
                for (String key : hmapLabelsList.get(i).keySet()) {
                    hmapAdapterList.get(i).put(key, new TableInfoAdapter(mContext));
                    hmapViewList.get(i).put(key, new MdmLinearLayout(mContext));
                }
            }
        }
    }

    public void setViewAsPlank() {
        resetView();
        TextView textView = new TextView(mContext);
        ListView.LayoutParams layoutParams = new ListView.LayoutParams(
                ListView.LayoutParams.MATCH_PARENT, ListView.LayoutParams.WRAP_CONTENT);
        textView.setLayoutParams(layoutParams);
        textView.setPadding(20, 0, 20, 0);
        textView.setText("Use " + getName().replace("UTAS", "TAS"));
        textView.setTextSize(16);
        scrollView.addView(textView);
    }

    public void setViewAsTable() {
        resetView();
        layout.addView(tabTatileLayout);
        for (int i = 0; i < tabCount; i++) {
            if (tabTatileLayout.getChildAt(i) == null) {
                Button codeBtn = new Button(mContext);
                setBtnAttribute(codeBtn, tabTitleList.get(i), i, Color.WHITE, 16);
                tabTatileLayout.addView(codeBtn);
            }
            if (hmapLabelsList.get(i) != null) {
                for (String label : hmapLabelsList.get(i).keySet()) {
                    if (hmapAdapterList.get(i).get(label).getCount() == 0) {
                        setHmapAdapterByLabel(label, i);
                    }
                    hmapViewList.get(i).get(label).setAdapter(hmapAdapterList.get(i).get(label));
                    hmapAdapterList.get(i).get(label).notifyDataSetChanged();
                    hmapViewList.get(i).get(label).setListViewHeightBasedOnChildren();
                    if (hmapViewList.get(i).get(label).getParent() != null) {
                        ((ViewGroup) hmapViewList.get(i).get(label).getParent()).removeView(
                                hmapViewList.get(i).get(label));
                    }
                    layout.addView(hmapViewList.get(i).get(label));
                }
            }
        }
        scrollView.removeAllViews();
        scrollView.addView(layout);
    }

    @Override
    View getView() {
        layout.removeAllViews();
        tabTatileLayout.removeAllViews();
        if (!isInfoValid()) {
            setViewAsPlank();
            return scrollView;
        }
        setViewAsTable();
        setCurTab(0);
        return scrollView;
    }

    @Override
    void clearData() {
        resetHashMapKeyValues();
        for (int i = 0; i < getTabCount(); i++) {
            for (String Label : hmapLabelsList.get(i).keySet()) {
                clearData(Label, i);
            }
        }

    }

    private void setBtnAttribute(Button codeBtn, String btnContent,
                                 int index, int textColor, int textSize) {
        if (null == codeBtn) {
            return;
        }
        codeBtn.setTextColor((textColor >= 0) ? textColor : Color.WHITE);
        codeBtn.setTextSize((textSize > 16) ? textSize : 16);
        codeBtn.setText(btnContent);
        codeBtn.setTag(index);
        Drawable drawable = mContext.getResources().getDrawable(R.drawable.selector_button);
        codeBtn.setBackground(drawable);
        codeBtn.setGravity(Gravity.CENTER);
        codeBtn.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                // btn click process
                setCurTab((Integer) v.getTag());
            }
        });
        LinearLayout.LayoutParams rlp = new LinearLayout.LayoutParams(
                LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT, 1.0f);
        codeBtn.setLayoutParams(rlp);
    }

    void inithmapLabelsList(int tabNum, String label) {
        if (hmapLabelsList.get(tabNum) == null ||
                hmapLabelsList.get(tabNum).keySet().size() == 0 ||
                hmapLabelsList.get(tabNum).get(label) == null ||
                hmapLabelsList.get(tabNum).get(label).keySet().size() == 0) {
            Elog.d(TAG, "[inithmapLabelsList] tabNum:" + tabNum + ",label:" + label);
            hmapLabelsList.add(tabNum, getHashMapLabels(tabNum));
            Elog.d(TAG, "[inithmapLabelsList] " + hmapLabelsList.get(tabNum));
        }
    }

    private int getCurTab() {
        return this.curTab;
    }

    private void setCurTab(int tabNum) {
        this.curTab = tabNum;
        for (int i = 0; i < this.tabCount; i++) {
            if (i == getCurTab()) {
                for (String key : hmapViewList.get(i).keySet()) {
                    hmapViewList.get(i).get(key).showView();
                    tabTatileLayout.findViewWithTag(i).setSelected(true);
                }
            } else {
                for (String key : hmapViewList.get(i).keySet()) {
                    hmapViewList.get(i).get(key).hideView();
                    tabTatileLayout.findViewWithTag(i).setSelected(false);
                }
            }
        }
    }

    @Override
    void removeView() {
        setInfoValid(1);
        resetView();
    }

    void resetView() {
        if (layout != null && layout.getChildCount() > 0) {
            layout.removeAllViews();
        }
        if (scrollView != null && scrollView.getChildCount() > 0) {
            scrollView.removeAllViews();
        }
        for (int i = 0; i < hmapViewList.size(); i++) {
            if (hmapViewList.get(i) != null) {
                for (String key : hmapViewList.get(i).keySet()) {
                    hmapAdapterList.get(i).get(key).clear();
                    hmapViewList.get(i).get(key).setAdapter(null);
                }
            }
        }
    }

    void clearData(String key, int tabNum) {
        if (isValidTabNum(tabNum)) {
            if (hmapAdapterList.get(tabNum).get(key) != null) {
                hmapAdapterList.get(tabNum).get(key).clear();
            }
        } else {
            for (int i = 0; i < hmapLabelsList.size(); i++) {
                if (hmapAdapterList.get(i).get(key) != null) {
                    hmapAdapterList.get(i).get(key).clear();
                }
            }
        }
    }

    void displayView(String label, int tabNum, boolean isShow) {
        if (isValidTabNum(tabNum)) {
            Elog.d(TAG, "[ShowView] label:" + label + "," + isShow);
            if (isShow && (!isValidTabNum(getCurTab()) || getCurTab() == tabNum)) {
                hmapViewList.get(tabNum).get(label).showView();
            } else {
                hmapViewList.get(tabNum).get(label).hideView();
            }
        } else {
            for (int i = 0; i < this.tabCount; i++) {
                if (isShow) {
                    hmapViewList.get(i).get(label).showView();
                } else {
                    hmapViewList.get(i).get(label).hideView();
                }
            }
        }
    }

    boolean isValidTabNum(int tabNum) {
        if (tabNum < this.tabCount && tabNum >= 0) {
            return true;
        }
        return false;
    }

    void addDataByArray(String label, int tabNum, String[] data) {
        if (isValidTabNum(tabNum)) {
            if (hmapLabelsList.get(tabNum) == null ||
                    hmapLabelsList.get(tabNum).keySet().size() == 0) {
                inithmapLabelsList(tabNum, label);
            }
            int position = hmapAdapterList.get(tabNum).get(label).getCount();
            hmapAdapterList.get(tabNum).get(label).add(data);
            hmapAdapterList.get(tabNum).get(label).notifyDataSetChanged();
            hmapViewList.get(tabNum).get(label).setListViewHeightBasedOnChildren();
        }
    }

    void addData(String label, int tabNum) {
        Elog.d(TAG, "[addData] label: " + label + ",tabNum: " + tabNum);
        inithmapLabelsList(tabNum, label);
        setHmapAdapterByLabel(label, tabNum);
        hmapViewList.get(tabNum).get(label).setAdapter(hmapAdapterList.get(tabNum).get(label));
        hmapAdapterList.get(tabNum).get(label).notifyDataSetChanged();
        hmapViewList.get(tabNum).get(label).setListViewHeightBasedOnChildren();
    }

    void notifyDataSetChanged(String label, int tabNum) {
        if (isValidTabNum(tabNum)) {
            if (hmapLabelsList.get(tabNum) == null ||
                    hmapLabelsList.get(tabNum).keySet().size() == 0) {
                inithmapLabelsList(tabNum, label);
            }
            if (hmapAdapterList.get(tabNum).get(label).getCount() <
                    hmapLabelsList.get(tabNum).get(label).size()) {
                hmapAdapterList.get(tabNum).clear();
                setHmapAdapterByLabel(label, tabNum);
            }
            hmapAdapterList.get(tabNum).get(label).notifyDataSetChanged();
            hmapViewList.get(tabNum).get(label).setListViewHeightBasedOnChildren();
        }
    }

    void setHmapAdapterByLabel(String label, int tabNum) {
        if (isValidTabNum(tabNum)) {
            Elog.d(TAG, "[setHmapAdapterByLabel] label: " + label + ",tabNum: " + tabNum);
            hmapAdapterList.get(tabNum).get(label).clear();
            String[] keys = new String[hmapLabelsList.get(tabNum).get(label).size()];
            hmapLabelsList.get(tabNum).get(label).keySet().toArray(keys);
            if (arrayTypeLabels.contains(label)) {
                List<String[]> cells = new ArrayList<String[]>();
                hmapAdapterList.get(tabNum).get(label).add(keys);
                int cellNum = 0;
                for (int i = 0; i < keys.length; i++) {
                    String[] values = (String[]) hmapLabelsList.get(tabNum).get(label).get(keys[i]);
                    if (values != null && values.length > 0) {
                        cellNum = values.length > cellNum ? values.length : cellNum;
                    }
                    cells.add(values);
                }
                for (int j = 0; j < cellNum; j++) {
                    String[] values = new String[keys.length];
                    Elog.d(TAG, "[setHmapAdapterByLabel] before row: "
                            + j + ":" + Arrays.toString(values));
                    for (int i = 0; i < keys.length; i++) {
                        values[i] = i < cells.size() && cells.get(i) != null &&
                                j < cells.get(i).length ? cells.get(i)[j] : "";
                    }
                    hmapAdapterList.get(tabNum).get(label).add(values);
                    Elog.d(TAG, "[setHmapAdapterByLabel] after row: " +
                            j + ":" + Arrays.toString(values));
                }
            } else {
                for (int i = 0; i < keys.length; i++) {
                    hmapAdapterList.get(tabNum).get(label).add(new String[]{
                            keys[i],
                            (String) (hmapLabelsList.get(tabNum).get(label).get(keys[i]) == null ?
                                    "" : hmapLabelsList.get(tabNum).get(label).get(keys[i])),
                    });
                }
            }
        }
    }

    LinkedHashMap<String, String> initHashMap(Object[] keys) {
        LinkedHashMap<String, String> hashMapObj = new LinkedHashMap<String, String>();
        for (int i = 0; i < keys.length; i++) {
            hashMapObj.put((String) keys[i], "");
        }
        return hashMapObj;
    }

    LinkedHashMap<String, String> setHashMap(Object[] keys, Object[] values) {
        LinkedHashMap<String, String> hashMapObj = new LinkedHashMap<String, String>();
        if (keys.length != values.length) return hashMapObj;
        for (int i = 0; i < keys.length; i++) {
            hashMapObj.put((String) keys[i], values[i].toString());
        }
        return hashMapObj;
    }

    LinkedHashMap<String, String[]> initArrayHashMap(Object[] keys) {
        LinkedHashMap<String, String[]> hashMapObj = new LinkedHashMap<String, String[]>();
        for (int i = 0; i < keys.length; i++) {
            hashMapObj.put((String) keys[i], null);
        }
        return hashMapObj;
    }

    void initHmapLabelsList() {
        if (hmapLabelsList == null) {
            hmapLabelsList = new ArrayList<LinkedHashMap<String, LinkedHashMap>>();
        }
        for (int i = 0; i < getTabCount(); i++) {
            if (hmapLabelsList.get(i) == null || hmapLabelsList.get(i).size() == 0) {
                this.hmapLabelsList.add(i, getHashMapLabels(i));
            }
        }
    }

    void resetHashMapKeyValues() {
        for (int i = 0; i < this.tabCount; i++) {
            for (String Label : hmapLabelsList.get(i).keySet()) {
                inithmapLabelsList(i, Label);
                LinkedHashMap map = new LinkedHashMap();
                if (isLabelArrayType(Label)) {
                    map = initArrayHashMap(hmapLabelsList.get(i).get(Label).keySet().toArray());
                } else {
                    map = initHashMap(hmapLabelsList.get(i).get(Label).keySet().toArray());
                }
                hmapLabelsList.get(i).get(Label).clear();
                hmapLabelsList.get(i).get(Label).putAll(map);
            }
        }
    }

    void setHashMapKeyValues(String label, int tabNum, String key, Object value) {
        inithmapLabelsList(tabNum, label);
        if (isLabelArrayType(label)) {
            if (value instanceof String[]) {
                hmapLabelsList.get(tabNum).get(label).put(key, (String[]) value);
                Elog.d(TAG, "[setHashMapKeyValues] key: " + key +
                        ",values:" + Arrays.toString((String[]) value));
            } else {
                if (value == null || value.equals("")) return;
                String[] indexs = new String[hmapLabelsList.get(tabNum).get(label).keySet().size()];
                hmapLabelsList.get(tabNum).get(label).keySet().toArray(indexs);
                String[] oldValues = (String[]) hmapLabelsList.get(tabNum).get(label).get(key);
                String[] values = Arrays.copyOf(oldValues, oldValues.length + 1);
                values[oldValues.length] = value.toString();
                hmapLabelsList.get(tabNum).get(label).put(key, values);
                Elog.d(TAG, "[setHashMapKeyValues] key: " + key +
                        ",values:" + Arrays.toString((String[]) values));
            }
        } else {
            hmapLabelsList.get(tabNum).get(label).put(key, String.valueOf(value));
            Elog.d(TAG, "[setHashMapKeyValues]Label: " + label +
                    ", tabNum: " + tabNum + ":" + hmapLabelsList.get(tabNum).get(label));
        }
    }

    private boolean isEmaptyStrArray(String[] value) {
        for (int i = 0; i < value.length; i++) {
            if (value[i] != null && !value[i].equals("")) {
                return false;
            }
        }
        return true;
    }

    void setHashMapKeyValues(String Label, int tabNum, LinkedHashMap<String, String> keyValues) {
        inithmapLabelsList(tabNum, Label);
        hmapLabelsList.get(tabNum).put(Label, keyValues);
    }

    void setHashMapKeyValues(String Label, int tabNum, String key, String[] values) {
        inithmapLabelsList(tabNum, Label);
        hmapLabelsList.get(tabNum).get(Label).put(key, values);
    }

    void setHashMapKeyValues(String label, int tabNum, String[] keys, Object[] values) {
        if (keys.length != values.length) {
            return;
        }
        inithmapLabelsList(tabNum, label);
        Elog.d(TAG, "[setHashMapKeyValues] tabNum: " + tabNum + ",label:" + label);
        Elog.d(TAG, "[setHashMapKeyValues] keys: " + Arrays.toString(keys) +
                ",values:" + Arrays.toString(values));
        if (isLabelArrayType(label)) {
            for (int i = 0; i < keys.length; i++) {
                if (hmapLabelsList.get(tabNum).get(label).containsKey(keys[i]) &&
                        hmapLabelsList.get(tabNum).get(label).get(keys[i]) != null) {
                    String[] oldValues =
                            (String[]) hmapLabelsList.get(tabNum).get(label).get(keys[i]);
                    String[] newValues = Arrays.copyOf(oldValues, oldValues.length + 1);
                    newValues[oldValues.length] = values[i].toString();
                    hmapLabelsList.get(tabNum).get(label).put(keys[i], newValues);
                } else {
                    hmapLabelsList.get(tabNum).get(label).put(keys[i],
                            new String[]{values[i].toString()});
                }
            }
            Elog.d(TAG, "[setHashMapKeyValues] " + hmapLabelsList.get(tabNum).get(label));
        } else {
            for (int i = 0; i < keys.length; i++) {
                hmapLabelsList.get(tabNum).get(label).put(keys[i],
                        values[i] == null ? "" : values[i].toString());
            }
        }
    }

    abstract LinkedHashMap<String, LinkedHashMap> getHashMapLabels(int index);

    abstract ArrayList<String> getArrayTypeKey();

    abstract boolean isLabelArrayType(String label);
}

class LteUTasInfo extends CombinationTableComponent {

    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};
    List<String[]> labelsList = new ArrayList<String[]>() {{
        add(new String[]{"TAS Enable", "Switch state",
                "TX Power", "RRC status"});
        add(new String[]{"Band", "TX Index"});
        add(new String[]{"ANT Index", "TX Pwr dBm",
                "PHR dB", "RSRP dBm"});
    }};
    String[] labelKeys = new String[]{"Tas Common", "Cell Info", "UL Info"};
    List<LinkedHashMap> valuesHashMap = new ArrayList<LinkedHashMap>() {{
        add(initHashMap(labelsList.get(0)));
        add(initHashMap(labelsList.get(1)));
        add(initArrayHashMap(labelsList.get(2)));
    }};
    HashMap<Integer, String> TasEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
        }
    };
    HashMap<Integer, String> RRCStateMapping = new HashMap<Integer, String>() {
        {
            put(0, "RRC Idle");
            put(1, "RRC Connected");
            put(2, "-");
        }
    };
    private String[] tabTitle = new String[]{"Common", "CC0", "CC1", "CC2"};

    public LteUTasInfo(Activity context) {
        super(context);
        initTableComponent(tabTitle);
    }

    @Override
    String getName() {
        return "LTE UTAS Info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String coULName = MDMContent.EL1_STATUS_IND_UL_INFO;
        String coCellName = MDMContent.EL1_STATUS_IND_CELL_INFO;
        int utas_info_valid = getFieldValue(data,
                coULName + "[0]." +
                        MDMContent.EL1_STATUS_IND_UTAS_INFO_VALID);
        setInfoValid(utas_info_valid);
        if (!isInfoValid()) {
            return;
        }
        int dl_cc_count = getFieldValue(data, MDMContent.DL_CC_COUNT);
        int ul_cc_count = getFieldValue(data, MDMContent.UL_CC_COUNT);
        if (dl_cc_count > 0) {
            int tasidx = getFieldValue(data, coULName + "[0]." +
                    MDMContent.EL1_STATUS_IND_TAS_STATUS);
            String tasEnable = (tasidx >= 0 && tasidx <= 1) ? TasEnableMapping.get(tasidx) :
                    TasEnableMapping.get(2) + "-(" + tasidx + ")";
            int switch_state = getFieldValue(data, coULName + "[0]." +
                    MDMContent.EL1_STATUS_IND_UTAS_SWITCH_STATE);
            String switchState = switch_state >= 0 && switch_state <= 31 ?
                    "" + switch_state : "-(" + switch_state + ")";
            int tx_power = getFieldValue(data, coULName + "[0]." +
                    MDMContent.EL1_STATUS_IND_UTAS_TX_POWER, true);
            String txPower = tx_power >= -50 && tx_power <= 33 ?
                    tx_power + "dBm" : "-(" + tx_power + ")";
            String rrcState = (dl_cc_count == 1 && ul_cc_count == 0) ? RRCStateMapping.get(0) :
                    (dl_cc_count >= 1 && ul_cc_count >= 1 ?
                            RRCStateMapping.get(1) : RRCStateMapping.get(2));
            setHashMapKeyValues(labelKeys[0], 0, labelsList.get(0),
                    new String[]{tasEnable, switchState, txPower, rrcState});
        } else {
            setHashMapKeyValues(labelKeys[0], 0, initHashMap(labelsList.get(0)));
        }
        addData(labelKeys[0], 0);

        int[] txActivated = new int[3];
        int[] rxActivated = new int[3];
        String[] rxBand = new String[3];
        String[] txIdx = new String[3];

        String[][] rxAntIndex = new String[3][5];
        String[][] txPwr = new String[3][5];
        String[][] txPHR = new String[3][5];
        String[][] rxRSRP = new String[3][5];
        txActivated[0] = (ul_cc_count >= 1) ? 1 : 0;
        rxActivated[0] = (dl_cc_count >= 1) ? 1 : 0;
        for (int i = 0; i < 3; i++) {
            int bandidx = getFieldValue(data,
                    coCellName + "[" + i + "]."
                            + MDMContent.EL1_STATUS_IND_BAND);
            int ul_cc_idx = getFieldValue(data,
                    coCellName + "[" + i + "]." +
                            MDMContent.EL1_STATUS_IND_UL_CC_IDX, true);
            int utas_cur_ant_idx = getFieldValue(data,
                    coULName + "[" + i + "]." +
                            MDMContent.EL1_STATUS_IND_UTAS_CUR_ANT_IDX);
            int utas_tx_activated = getFieldValue(data,
                    coULName + "[" + i + "]." +
                            MDMContent.EL1_STATUS_IND_UTAS_TX_ACTIVATED);
            if (i > 0) {
                txActivated[i] = ul_cc_idx != -1 && utas_tx_activated == 1 ? 1 : 0;
                rxActivated[i] = txActivated[i];
            }
            Elog.d(TAG, "tab: " + (i + 1) + ", " + txActivated[i] + ":" + rxActivated[i]);
            rxBand[i] = bandidx >= 0 && bandidx <= 255 ? "Band " + bandidx : "-(" + bandidx + ")";
            txIdx[i] = utas_cur_ant_idx >= 0 && utas_cur_ant_idx <= 10 ?
                    "" + utas_cur_ant_idx : "-(" + utas_cur_ant_idx + ")";
            for (int j = 0; j < 5; j++) {
                int utas_partial_blank = getFieldValue(data,
                        coULName + "[" + i + "]." +
                                MDMContent.EL1_STATUS_IND_UTAS_PARTIAL_BLANK + "[" + j + "]");
                int utas_ant_idx = getFieldValue(data,
                        coULName + "[" + i + "]." +
                                MDMContent.EL1_STATUS_IND_UTAS_ANT_IDX + "[" + j + "]");
                int utas_tx_pwr = getFieldValue(data,
                        coULName + "[" + i + "]." +
                                MDMContent.EL1_STATUS_IND_UTAS_TX_PWR + "[" + j + "]", true);
                int utas_phr = getFieldValue(data,
                        coULName + "[" + i + "]." +
                                MDMContent.EL1_STATUS_IND_UTAS_PHR + "[" + j + "]", true);
                int utas_rsrp = getFieldValue(data,
                        coULName + "[" + i + "]." +
                                MDMContent.EL1_STATUS_IND_UTAS_RSRP + "[" + j + "]", true);
                rxAntIndex[i][j] = utas_ant_idx >= 0 && utas_ant_idx <= 10 ?
                        (utas_ant_idx + "") : "-(" + utas_ant_idx + ")";
                if (utas_partial_blank != 1) {
                    txPwr[i][j] = utas_tx_pwr >= -50 && utas_tx_pwr <= 33 ?
                            utas_tx_pwr + "" : "-(" + utas_tx_pwr + ")";
                    txPHR[i][j] = utas_phr >= -50 && utas_phr <= 33 ?
                            utas_phr + "" : "-(" + utas_phr + ")";
                    rxRSRP[i][j] = utas_rsrp >= -140 && utas_rsrp <= 18 ?
                            utas_rsrp + "" : "-(" + utas_rsrp + ")";
                } else {
                    txPwr[i][j] = "";
                    txPHR[i][j] = "";
                    rxRSRP[i][j] = "";
                }
            }
        }
        if (txActivated[1] == 0 && txActivated[2] == 1) {
            txActivated[1] = rxActivated[1] = 1;
            txActivated[2] = rxActivated[2] = 0;
            rxBand[1] = rxBand[2];
            txIdx[1] = txIdx[2];
            rxAntIndex[1] = rxAntIndex[2];
            txPwr[1] = txPwr[2];
            txPHR[1] = txPHR[2];
            rxRSRP[1] = rxRSRP[2];
        }
        for (int i = 0; i < 3; i++) {
            setHashMapKeyValues(labelKeys[1], i + 1, labelsList.get(1)[0],
                    rxActivated[i] == 1 ? rxBand[i] : "");
            setHashMapKeyValues(labelKeys[2], i + 1, labelsList.get(2)[0],
                    rxActivated[i] == 1 ? rxAntIndex[i] : null);
            setHashMapKeyValues(labelKeys[2], i + 1, labelsList.get(2)[3],
                    rxActivated[i] == 1 ? rxRSRP[i] : null);
            setHashMapKeyValues(labelKeys[1], i + 1, labelsList.get(1)[1],
                    txActivated[i] == 1 ? txIdx[i] : "");
            setHashMapKeyValues(labelKeys[2], i + 1, labelsList.get(2)[1],
                    txActivated[i] == 1 ? txPwr[i] : null);
            setHashMapKeyValues(labelKeys[2], i + 1, labelsList.get(2)[2],
                    txActivated[i] == 1 ? txPHR[i] : null);
            addData(labelKeys[1], i + 1);
            addData(labelKeys[2], i + 1);
        }
    }

    public void addDataToList(ArrayList<int[]> list, int[] value) {
        for (int i = 0; i < value.length; i++) {
            if (list.get(i) == null) {
                list.add(i, new int[]{value[i]});
            } else {
                int[] newValue = Arrays.copyOf(list.get(i), list.get(i).length + 1);
                newValue[newValue.length - 1] = value[i];
                list.add(i, newValue);
            }
        }
    }

    @Override
    LinkedHashMap<String, LinkedHashMap> getHashMapLabels(int index) {
        LinkedHashMap<String, LinkedHashMap> hashMapkeyValues =
                new LinkedHashMap<String, LinkedHashMap>();
        switch (index) {
            case 0:
                hashMapkeyValues.put(labelKeys[0], valuesHashMap.get(0));
                break;
            case 1:
            case 2:
            case 3:
                hashMapkeyValues.put(labelKeys[1], valuesHashMap.get(1));
                hashMapkeyValues.put(labelKeys[2], valuesHashMap.get(2));
                break;
        }
        return hashMapkeyValues;
    }

    @Override
    ArrayList<String> getArrayTypeKey() {
        ArrayList<String> arrayTypeKeys = new ArrayList<String>();
        arrayTypeKeys.add("UL Info");
        return arrayTypeKeys;
    }

    @Override
    boolean isLabelArrayType(String label) {
        if (getArrayTypeKey().contains(label)) {
            return true;
        }
        return false;
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

}

class RRCState extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_URR_3G_GENERAL_STATUS_IND};
    private final HashMap<Integer, String> mStateMapping =
            new HashMap<Integer, String>() {
                {
                    put(0, "Idle");
                    put(1, "Cell FACH");
                    put(2, "Cell PCH");
                    put(3, "URA PCH");
                    put(4, "Cell DCH");
                    put(5, "");
                    put(6, "NO_CHANGE");
                    put(7, "Cell FACH");
                    put(8, "Cell PCH");
                    put(9, "Cell PCH");
                    put(10, "URA PCH");
                    put(11, "Cell FACH");
                    put(12, "Cell PCH");
                    put(13, "Cell PCH");

                }
            };

    public RRCState(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "RRC State";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RRC State"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String coName = MDMContent.TDD_EM_URR_3G_GENERAL_UMTS_UAS_3G_GENERAL_STATUS
                + ".";
        int state = getFieldValue(data, coName
                + MDMContent.TDD_EM_URR_3G_GENERAL_UMTS_RRC_STATE);
        if (state != 6) {
            String stateString = mStateMapping.get(state);
            clearData();
            addData(stateString);
        }
    }
}

class EPSBearerThroughput extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_UPCM_STATUS_IND
    };

    public EPSBearerThroughput(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EPS Bearer Throughput";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        String[] labels = new String[38];
        for (int i = 0; i <= 36; i += 2) {
            labels[i] = "EPSB[" + (5 + i / 2) + "] UL throughput";
            labels[i + 1] = "EPSB[" + (5 + i / 2) + "] DL throughput";
        }

        return labels;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String coName = MDMContent.EPS_BEARER_THROUGHPUT_UPCME + "." +
                MDMContent.EPS_BEARER_THROUGHPUT_EPSB + "[";
        for (int i = 5; i <= 23; i++) {
            int ul_bytes = getFieldValue(data, coName + i + "]." + "ul_bytes");
            int ul_bytes_pri = getFieldValue(data, coName + i + "]." + "ul_bytes_pri");
            addData(ul_bytes + ul_bytes_pri);
            int dl_bytes = getFieldValue(data, coName + i + "]." + "dl_bytes");
            addData(dl_bytes);
        }
    }
}

class RRState extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRM_MEASUREMENT_REPORT_INFO_IND};
    private static final String[] LABELS = new String[]{"RR State"};
    private final Map<Integer, String> mRrStateMapping =
            new HashMap<Integer, String>() {
                {
                    put(3, "Idle");
                    put(5, "Packet Transfer");
                    put(6, "Dedicated");
                }
            };

    public RRState(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "RR State";
    }

    @Override
    String getGroup() {
        return "2. GSM EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    String[] getLabels() {
        return LABELS;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String state = mRrStateMapping.get(getFieldValue(data,
                MDMContent.RR_EM_MEASUREMENT_REPORT_INFO + "."
                        + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_RR_STATE));
        clearData();
        addData(state);
    }
}

class TddServingCellInfo extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_CSCE_SERV_CELL_S_STATUS_IND,
            MDMContent.MSG_ID_TDD_EM_MEME_DCH_UMTS_CELL_INFO_IND,};

    public TddServingCellInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "TD-SCDMA Serving Cell Info (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"UARFCN", "CellParamId", "RSCP"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        if (name.equals(MDMContent.MSG_ID_TDD_EM_CSCE_SERV_CELL_S_STATUS_IND)) {
            String coName = MDMContent.EM_CSCE_SERV_CELL + ".";
            int uarfcn = getFieldValue(data, coName
                    + MDMContent.EM_CSCE_SERV_CELL_UARFCN_DL);
            int cellParaId = getFieldValue(data, coName
                    + MDMContent.EM_CSCE_SERV_CELL_PSC);
            int rscp = getFieldValue(data, coName
                    + MDMContent.EM_CSCE_SERV_CELL_RSCP, true);
            addData(uarfcn, cellParaId, (float) rscp / 4096);
        } else if (name
                .equals(MDMContent.MSG_ID_TDD_EM_MEME_DCH_UMTS_CELL_INFO_IND)) {
            String preStruct = MDMContent.EM_MEME_DCH_UMTS_CELL_LIST;
            for (int i = 0; i < 64; i++) {
                String coName = MDMContent.EM_MEME_DCH_UMTS_CELL_LIST + "[" + i
                        + "].";
                int uarfcn = getFieldValue(data, coName
                        + MDMContent.EM_MEME_DCH_UMTS_CELL_LIST_UARFCN);
                int cellParaId = getFieldValue(data, coName
                        + MDMContent.EM_MEME_DCH_UMTS_CELL_LIST_CELLPARAID);
                int rscp = getFieldValue(data, coName
                        + MDMContent.EM_MEME_DCH_UMTS_CELL_LIST_RSCP, true);
                int isServingCell = getFieldValue(data, coName
                        + MDMContent.EM_MEME_DCH_UMTS_CELL_LIST_IS_SERVING_CELL);

                if ((isServingCell != 0) && (rscp > -120)) {
                    addData(uarfcn, cellParaId, rscp);
                }
            }
        }
    }

}

class LaiRaiUmtsTdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_MM_INFO_IND};

    public LaiRaiUmtsTdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LAI and RAI (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"PLMN", "LAI", "RAI"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int rac = getFieldValue(data, MDMContent.EM_MM_RAC);
        String plmn = "";
        String loc = "";
        for (int i = 0; i < 3; i++) {
            plmn += getFieldValue(data, MDMContent.EM_MM_MCC + "[" + i + "]")
                    + " ";
        }
        for (int i = 0; i < 3; i++) {
            plmn += getFieldValue(data, MDMContent.EM_MM_MNC + "[" + i + "]")
                    + " ";
        }
        for (int i = 0; i < 2; i++) {
            loc += getFieldValue(data, MDMContent.EM_MM_LOC + "[" + i + "]")
                    + " ";
        }
        clearData();
        addData(plmn, loc, rac);
    }
}

class ERRCState extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_STATE_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(1, "-");
            put(2, "Idle");
            put(3, "Connected");
            put(4, "-");
            put(5, "Idle");
            put(6, "Connected");
        }
    };

    public ERRCState(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "ERRC State";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RRC State"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String state = mMapping.get(getFieldValue(data,
                MDMContent.EM_ERRC_STATE_ERRC_STS));
        clearData();
        addData(state);
    }
}

class HighPriorityPLMNSearch extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_NWSEL_PLMN_INFO_IND};

    public HighPriorityPLMNSearch(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "High Priority PLMN Search";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String[] getLabels() {
        return new String[]{"High Priority PLMN Search"};
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int value = getFieldValue(data, MDMContent.IS_HIGHER_PRI_PLMN_SRCH);
        clearData();
        addData(value);
    }
}

class CarrierRSSIServing extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRM_MEASUREMENT_REPORT_INFO_IND};

    public CarrierRSSIServing(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Cell measurement (Serving)";
    }

    @Override
    String getGroup() {
        return "2. GSM EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"BCCH RLA(Dedicated)", "RLA", "Reported",
                "RX level full", "RX quality sub", "RX quality full"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.RR_EM_MEASUREMENT_REPORT_INFO + ".";
        int rrState = getFieldValue(data, coName
                + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_RR_STATE);
        int rla = getFieldValue(
                data,
                coName
                        + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_RLA_IN_QUARTER_DBM,
                true);
        int reported = getFieldValue(
                data,
                coName
                        + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_RLA_REPORTED_VALUE);
        int bcchRlaValid = getFieldValue(data, coName
                + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_BCCH_RLA_VALID);
        int bcchRla = getFieldValue(
                data,
                coName + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_BCCH_RLA_IN_DEDI_STATE,
                true);
        int full = getFieldValue(
                data,
                coName + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_RLA_FULL_VALUE_IN_QUATER_DBM,
                true);
        int rxSub = getFieldValue(data, coName
                + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_RXQUAL_SUB);
        int rxFull = getFieldValue(data, coName
                + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_RXQUAL_FULL);

        clearData();
        if (rrState >= 3 && rrState <= 7) {
            if (bcchRlaValid > 0) {
                if (bcchRla == -1000) {
                    addData("-");
                } else {
                    float value = (float) bcchRla / 4;
                    addData(String.format("%.2f", value));
                }
            } else {
                addData("-");
            }

            if (rla == -1000) {
                addData("-");
            } else {
                float value = (float) rla / 4;
                addData(String.format("%.2f", value));
            }

            if (rla == -1000) {
                addData("-");
            } else {
                addData(reported);
            }

            if (rrState != 6) {
                addData("-");
            } else {
                float value = (float) full / 4;
                addData(String.format("%.2f", value));
            }

            if (rxSub == 0xFF) {
                addData("-");
            } else {
                addData(rxSub);
            }

            if (rxFull == 0xFF) {
                addData("-");
            } else {
                addData(rxFull);
            }
        }
        notifyDataSetChanged();
    }
}

class EUtranNeighborCellInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRM_IR_4G_NEIGHBOR_MEAS_STATUS_IND};
    HashMap<Integer, String> mCellTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "Normal");
            put(1, "CSG");
            put(2, "Hybrid");
        }
    };
    HashMap<Integer, String> mBandTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "FDD");
            put(2, "TDD");
        }
    };
    HashMap<Integer, String> mFailTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "None");
            put(1, "Never");
            put(2, "PLMN");
            put(3, "LA");
            put(4, "C_BAR");
            put(5, "TEMP");
            put(6, "CRI3");
            put(7, "TA");
            put(8, "FREQ");
            put(9, "RES_OP");
            put(10, "RES_OP_FRE");
            put(11, "CSG");
            put(12, "GEMINI");
            put(13, "NAS");
        }
    };

    public EUtranNeighborCellInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "E-UTRAN Neighbor Cell Info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Type", "EARFCN", "PCI", "Band", "RSRP", "RSRQ",
                "Report", "Non-Report", "Barred"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_STATUS + "[";
        clearData();
        for (int i = 0; i < 6; i++) {
            int isValid = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_IS_VALID);
            int cellType = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_CELL_TYPE);
            int earfcn = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_EARFCN);
            int pci = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_PCI);
            int bandType = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_BAND_TYPE);
            int rsrp = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_RSRP, true);
            int rsrq = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_RSRQ, true);
            int repValue = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_REP_VALUE);
            int nonRepValue = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_NON_REP_VALUE);
            int failType = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_FAIL_TYPE);
            if (isValid > 0) {
                String cell = mCellTypeMapping.get(cellType);
                String band = mBandTypeMapping.get(bandType);
                String fail = mFailTypeMapping.get(failType);
                addData(cell);
                addData(earfcn);
                addData(pci);
                addData(band);
                addData(String.format("%.2f", (float) rsrp / 4));
                addData(String.format("%.2f", (float) rsrq / 4));
                addData(repValue);
                addData(nonRepValue);
                addData(fail);
            }
        }
        notifyDataSetChanged();
    }
}

class LteServingCellInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND,
            MDMContent.MSG_ID_EM_ERRC_SERVING_INFO_IND,
            MDMContent.MSG_ID_EM_EMM_REG_COMMON_INFO_IND,
            MDMContent.MSG_ID_EM_ERRC_EL1_CONFIG_INFO_IND
    };

    HashMap<Integer, String> mBandMapping = new HashMap<Integer, String>() {
        {
            put(0, "6_RB");
            put(1, "15_RB");
            put(2, "25_RB");
            put(3, "50_RB");
            put(4, "75_RB");
            put(5, "100_RB");
            put(0xff, "Invalid(-1)");
        }
    };
    int earfcn;
    int pci;
    int rsrp;
    int rsrq;
    int sinr;
    int band;
    int dl_earfcn;
    int ul_earfcn;
    int powerClass;
    int cell_id;
    int dlBandwidth;
    int ulBandwidth;
    int highSpeed;
    String lac = "-";

    int servingValid = 0;

    public LteServingCellInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Serving Cell(Primary Cell) Info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"PCI", "EARFCN (Band)", "SINR", "RSRP", "RSRQ",
                "FreqBandInd", "DlBandwidth", "UlBandwidth", "dl_earfcn", "ul_earfcn",
                "powerclass", "HighSpeed",
                "Cell ID", "LAC"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_EM_ERRC_SERVING_INFO_IND)) {
            servingValid = getFieldValue(data, "is_serving_inf_valid");
            Elog.d(TAG, "is_serving_inf_valid = " + servingValid);
        }
        Elog.d(TAG, "servingValid = " + servingValid);
        if (servingValid == 0) {
            clearData();
            addData("-");
            addData("-");
            addData("-");
            addData("-");
            addData("-");
            addData("-");
            addData("-");
            addData("-");
            addData("-");
            addData("-");
            return;
        }

        if (servingValid == 1) {
            if (name.equals(MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND)) {
                String coName = MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SERVING_INFO + ".";

                rsrp = getFieldValue(data, coName
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_RSRP, true);
                rsrq = getFieldValue(data, coName
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_RSRQ, true);
                sinr = getFieldValue(data, coName
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_RS_SNR_IN_QDB, true);
                earfcn = getFieldValue(data, coName
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_EARFCN);
                pci = getFieldValue(data, coName
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_PCI);
                band = getFieldValue(data, coName
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SERV_LTE_BAND);

                dlBandwidth = getFieldValue(data, coName
                        + MDMContent.DL_BANDWIDTH, true);
                ulBandwidth = getFieldValue(data, coName
                        + MDMContent.UL_BANDWIDTH, true);
            } else if (name.equals(MDMContent.MSG_ID_EM_ERRC_SERVING_INFO_IND)) {
                highSpeed = getFieldValue(data,
                        MDMContent.EM_ERRC_HIGH_SPEED_FLG);
                cell_id = getFieldValue(data, "serv_inf.cell_id");
                dl_earfcn = getFieldValue(data, "serv_inf.dl_earfcn");
                ul_earfcn = getFieldValue(data, "serv_inf.ul_earfcn");
            } else if (name.equals(MDMContent.MSG_ID_EM_ERRC_EL1_CONFIG_INFO_IND)) {
                powerClass = getFieldValue(data, "powerclass");
            } else if (name.equals(MDMContent.MSG_ID_EM_EMM_REG_COMMON_INFO_IND)) {
                lac = "";
                for (int i = 0; i < 2; i++) {
                    int lac_value = getFieldValue(data, "lai.la_code[" + i + "]");
                    lac += Integer.toHexString(lac_value);
                }
            }
            clearData();
            addData(earfcn == 0xFFFFFFFF ? "" : pci);
            addData(earfcn == 0xFFFFFFFF ? "" : "EARFCN: " + earfcn + " (Band "
                    + band + ")");
            addData(sinr == 0xFFFFFFFF ? "" : (float) sinr / 4);
            addData(rsrp == 0xFFFFFFFF ? "" : (float) rsrp / 4);
            addData(rsrq == 0xFFFFFFFF ? "" : (float) rsrq / 4);
            addData(band == 0xFFFF ? "" : band);
            addData(mBandMapping.get(dlBandwidth) + "(" + dlBandwidth + ")");
            addData(mBandMapping.get(ulBandwidth) + "(" + ulBandwidth + ")");
            addData(dl_earfcn);
            addData(ul_earfcn);
            addData(powerClass);
            addData(highSpeed);
            addData(String.format("0x%X", cell_id));
            addData(lac);
        }
    }
}

class LteSCellInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND};

    public LteSCellInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell Info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"PCI", "EARFCN (Band)", "SINR", "RSRP", "RSRQ"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_LIST + "." +
                MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO + "[0].";
        long earfcn = getFieldValue(data,
                coName + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_EARFCN);
        int pci = getFieldValue(data,
                coName + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_PCI);
        int rsrp = getFieldValue(data,
                coName + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_RSRP, true);
        int rsrq = getFieldValue(data,
                coName + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_RSRQ, true);
        int sinr = getFieldValue(data,
                coName + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_RS_SNR_IN_QDB, true);
        int band = getFieldValue(data,
                coName + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_SERV_LTE_BAND);
        Elog.d(TAG, "band = " + band);
        clearData();
        addData(earfcn == 0xFFFFFFFF ? "" : pci);
        addData(earfcn == 0xFFFFFFFF ? "" : "EARFCN: " + earfcn + " (Band "
                + band + ")");
        addData(rsrp == 0xFFFFFFFF ? "" : (float) rsrp / 4);
        addData(rsrq == 0xFFFFFFFF ? "" : (float) rsrq / 4);
        addData(sinr == 0xFFFFFFFF ? "" : (float) sinr / 4);
    }
}

class PrimaryCellDlBlockErrorRate extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellDlBlockErrorRate(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell DL Block Error Rate";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("DL BLER");
        mYLabel.setText("D");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 2;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "DL BLER";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int bler = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_BLER, true);
        addData(0, bler);
    }
}

class PrimaryCellDownlinkBandwidth extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellDownlinkBandwidth(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell Downlink Bandwidth";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DL_BW"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[0].";
        int bw = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_DL_BW);
        clearData();
        addData(bw);
    }
}

class PrimaryCellUplinkBandwidth extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellUplinkBandwidth(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell Uplink Bandwidth";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"UL_BW"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[0].";
        int bw = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_UL_BW);
        clearData();
        addData(bw);
    }
}

class PrimaryCellUplinkDownlinkConfiguration extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellUplinkDownlinkConfiguration(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell Uplink-Downlink Configuration";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"UDC"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[0].";
        int cfg = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_TDD_CFG);
        clearData();
        addData(cfg);
    }
}

class PrimaryCellSpecialSubframeConfiguration extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellSpecialSubframeConfiguration(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell Special Subframe Configuration";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"SSC"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[0].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_SP_CFG);
        clearData();
        addData(value);
    }
}

class PrimaryCellTransmissionMode extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellTransmissionMode(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell Transmission Mode";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"TM"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[0].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_TM);
        clearData();
        addData(value);
    }
}

class SecondaryCellDlBlockErrorRate extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellDlBlockErrorRate(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell DL Block Error Rate";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("DL BLER");
        mYLabel.setText("D");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 2;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "UL BLER";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int bler = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_BLER, true);
        addData(0, bler);
    }
}

class SecondaryCellDownlinkBandwidth extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellDownlinkBandwidth(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell Downlink Bandwidth";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DL_BW"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[1].";
        int bw = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_DL_BW);
        clearData();
        addData(bw);
    }
}

class SecondaryCellUplinkBandwidth extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellUplinkBandwidth(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell Uplink Bandwidth";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"UL_BW"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[1].";
        int bw = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_UL_BW);
        clearData();
        addData(bw);
    }
}

class SecondaryCellUplinkDownlinkConfiguration extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellUplinkDownlinkConfiguration(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell Uplink-Downlink Configuration";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"UDC"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[1].";
        int cfg = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_TDD_CFG);
        clearData();
        addData(cfg);
    }
}

class SecondaryCellSpecialSubframeConfiguration extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellSpecialSubframeConfiguration(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell Special Subframe Configuration";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"SSC"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[1].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_SP_CFG);
        clearData();
        addData(value);
    }
}

class SecondaryCellTransmissionMode extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellTransmissionMode(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell Transmission Mode";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"TM"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[1].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_TM);
        clearData();
        addData(value);
    }
}

class Tai extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EMM_PLMNSEL_INFO_IND};

    public Tai(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "TAI";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"PLMN", "TAC"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String coName = MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN + ".";
        for (int i = 0; i < 1; i++) {
            int mcc1 = getFieldValue(data, coName
                    + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + 1);
            int mcc2 = getFieldValue(data, coName
                    + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + 2);
            int mcc3 = getFieldValue(data, coName
                    + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MCC + 3);
            int mnc1 = getFieldValue(data, coName
                    + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + 1);
            int mnc2 = getFieldValue(data, coName
                    + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + 2);
            int mnc3 = getFieldValue(data, coName
                    + MDMContent.EM_EMM_L4C_PLMNSEL_PARA_SELECTED_PLMN_MNC + 3);
            int tac = getFieldValue(data,
                    MDMContent.EM_EMM_L4C_PLMNSEL_PARA__MNC);
            if (mcc1 == 0xF && mcc2 == 0xF && mcc3 == 0xF && mnc1 == 0xF
                    && mnc2 == 0xF && mnc3 == 0xF) {
                addData("-");
            } else {
                addData("" + mcc1 + mcc2 + mcc3 + mnc1 + mnc2
                        + (mnc3 == 0xF ? "" : mnc3));
            }
            if (tac == 0xFFFE || tac == 0) {
                addData("-");
            } else {
                addData(String.format("0x%X", tac));
            }
        }
    }
}

class EmIrReport extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRM_SUCCESS_RATE_KPI_IND,
            MDMContent.MSG_ID_EM_RRCE_KPI_STATUS_IND,
            MDMContent.MSG_ID_EM_ERRC_SUCCESS_RATE_KPI_IND};

    private static final int EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_CR = 0;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G3_CR = 1;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G2_CR = 2;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G4_CR = 3;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_2G4_CR = 4;

    private static final int EM_ERRC_SUCCESS_RATE_KPI_INTRA_CELL_HO = 5;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_INTER_CELL_HO = 6;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G3_HO = 7;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G2_HO = 8;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G4_HO = 9;

    private static final int EM_ERRC_SUCCESS_RATE_KPI_LTE_FDD_TDD_CELL_HO = 10;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_LTE_TDD_FDD_CELL_HO = 11;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GTDD_3G_HO = 12;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GFDD_3G_HO = 13;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GTDD_2G_HO = 14;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GFDD_2G_HO = 15;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G_4GFDD_HO = 16;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G_4GTDD_HO = 17;

    private static final int EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_FDDTDD_CR = 18;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_TDDFDD_CR = 19;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_CR = 20;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_CR = 21;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_CR = 22;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_CR = 23;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_CR = 24;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_CR = 25;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_CR = 26;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_CR = 27;

    private static final int EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_FDDTDD_REDIRECT = 28;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_TDDFDD_REDIRECT = 29;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_R8_REDIRECT = 30;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_R9_REDIRECT = 31;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_R10_REDIRECT = 32;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_R8_REDIRECT = 33;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_R9_REDIRECT = 34;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_R10_REDIRECT = 35;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_R8_REDIRECT = 36;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_R9_REDIRECT = 37;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_R10_REDIRECT = 38;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_R8_REDIRECT = 39;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_R9_REDIRECT = 40;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_R10_REDIRECT = 41;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_REDIRECT = 42;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_REDIRECT = 43;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_REDIRECT = 44;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_REDIRECT = 45;

    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_AFR = 46;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_AFR = 47;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_AFR = 48;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_AFR = 49;

    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_BG_SRCH = 50;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_BG_SRCH = 51;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_BG_SRCH = 52;
    private static final int EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_BG_SRCH = 53;

    HashMap<Integer, String> mRrmDirectMapping = new HashMap<Integer, String>() {
        {
            put(0, "2G3");
            put(1, "2G3");
            put(2, "2G3");
            put(3, "2G3");
        }
    };
    HashMap<Integer, String> mRrmTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "Cell Reselection");
            put(1, "CCO");
            put(2, "Handover");
            put(3, "Redirection");
        }
    };
    HashMap<Integer, String> mRrceDirectMapping = new HashMap<Integer, String>() {
        {
            put(0, "3G2");
            put(1, "3G2");
            put(2, "3G2");
            put(3, "3G2");
            put(4, "3G2");
            put(5, "3G2");
        }
    };
    HashMap<Integer, String> mRrceTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "Handover");
            put(1, "Cell Reselection");
            put(2, "Cell Reselection");
            put(3, "CCO");
            put(4, "Redirection");
            put(5, "Redirection");
        }
    };
    HashMap<Integer, String> mErrcDirectMapping = new HashMap<Integer, String>() {
        {
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G3_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G2_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G4_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_2G4_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_CELL_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTER_CELL_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G3_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G2_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G4_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_LTE_FDD_TDD_CELL_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_LTE_TDD_FDD_CELL_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GTDD_3G_HO, "4GTDD to 3G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GFDD_3G_HO, "4GFDD to 3G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GTDD_2G_HO, "4GTDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GFDD_2G_HO, "4GFDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G_4GFDD_HO, "3G to 4GFDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G_4GTDD_HO, "3G to 4GTDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_FDDTDD_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_TDDFDD_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_CR, "4GFDD to 3G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_CR, "4GTDD to 3G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_CR, "4GFDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_CR, "4GTDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_CR, "3G to 4GFDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_CR, "3G to 4GTDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_CR, "2G to 4GFDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_CR, "2G to 4GTDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_FDDTDD_REDIRECT, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_TDDFDD_REDIRECT, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_R8_REDIRECT, "4GFDD to 3G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_R9_REDIRECT, "4GFDD to 3G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_R10_REDIRECT,
                    "4GFDD to 3G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_R8_REDIRECT, "4GTDD to 3G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_R9_REDIRECT, "4GTDD to 3G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_R10_REDIRECT,
                    "4GTDD to 3Gf");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_R8_REDIRECT, "4GFDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_R9_REDIRECT, "4GFDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_R10_REDIRECT,
                    "4GFDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_R8_REDIRECT, "4GTDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_R9_REDIRECT, "4GTDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_R10_REDIRECT,
                    "4GTDD to 2G");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_REDIRECT, "3G to 4GFDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_REDIRECT, "3G to 4GTDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_REDIRECT, "2G to 4GFDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_REDIRECT, "2G to 4GTDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_AFR, "3G to 4GFDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_AFR, "3G to 4GTDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_AFR, "2G to 4GFDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_AFR, "2G to 4GTDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_BG_SRCH, "3G to 4GFDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_BG_SRCH, "3G to 4GTDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_BG_SRCH, "2G to 4GFDD");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_BG_SRCH, "2G to 4GTDD");
        }
    };
    HashMap<Integer, String> mErrcTypeMapping = new HashMap<Integer, String>() {
        {
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G3_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G2_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G4_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_2G4_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_CELL_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTER_CELL_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G3_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4G2_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G4_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_LTE_FDD_TDD_CELL_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_LTE_TDD_FDD_CELL_HO, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GTDD_3G_HO, "Handover");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GFDD_3G_HO, "Handover");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GTDD_2G_HO, "Handover");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_4GFDD_2G_HO, "Handover");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G_4GFDD_HO, "Handover");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_3G_4GTDD_HO, "Handover");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_FDDTDD_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_TDDFDD_CR, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_CR, "Cell Reselection");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_CR, "Cell Reselection");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_CR, "Cell Reselection");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_CR, "Cell Reselection");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_CR, "Cell Reselection");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_CR, "Cell Reselection");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_CR, "Cell Reselection");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_CR, "Cell Reselection");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_FDDTDD_REDIRECT, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_INTRA_LTE_TDDFDD_REDIRECT, "");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_R8_REDIRECT,
                    "Rediretion Rel8");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_R9_REDIRECT,
                    "Rediretion Rel9");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G3_R10_REDIRECT,
                    "Rediretion Rel10");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_R8_REDIRECT,
                    "Rediretion Rel8");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_R9_REDIRECT,
                    "Rediretion Rel9");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G3_R10_REDIRECT,
                    "Rediretion Rel10");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_R8_REDIRECT,
                    "Rediretion Rel8");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_R9_REDIRECT,
                    "Rediretion Rel9");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD4G2_R10_REDIRECT,
                    "Rediretion Rel10");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_R8_REDIRECT,
                    "Rediretion Rel8");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_R9_REDIRECT,
                    "Rediretion Rel9");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD4G2_R10_REDIRECT,
                    "Rediretion Rel10");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_REDIRECT, "Rediretion");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_REDIRECT, "Rediretion");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_REDIRECT, "Rediretion");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_REDIRECT, "Rediretion");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_AFR, "Auto Fast Return");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_AFR, "Auto Fast Return");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_AFR, "Auto Fast Return");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_AFR, "Auto Fast Return");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD3G4_BG_SRCH,
                    "Background Search");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD3G4_BG_SRCH,
                    "Background Search");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_FDD2G4_BG_SRCH,
                    "Background Search");
            put(EM_ERRC_SUCCESS_RATE_KPI_IRAT_TDD2G4_BG_SRCH,
                    "Background Search");
        }
    };
    HashMap<Integer, String> mStatusMapping = new HashMap<Integer, String>() {
        {
            put(0, "Ongoing");
            put(1, "Success");
            put(2, "Failure");
        }
    };

    public EmIrReport(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EM IR Report";
    }

    @Override
    String getGroup() {
        return "6. Inter-RAT EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"IR Direction", "IR Type", "Status"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_EM_RRM_SUCCESS_RATE_KPI_IND)) {
            int procId = getFieldValue(data,
                    MDMContent.EM_RRM_SUCCESS_RATE_KPI_PROC_ID);
            int status = getFieldValue(data,
                    MDMContent.EM_RRM_SUCCESS_RATE_KPI_STATUS);
            String direction = mRrmDirectMapping.get(procId);
            String procType = mRrmTypeMapping.get(procId);
            clearData();
            addData(new String[]{direction, procType,
                    mStatusMapping.get(status)});
        } else if (name.equals(MDMContent.MSG_ID_EM_RRCE_KPI_STATUS_IND)) {
            int procId = getFieldValue(data, MDMContent.EM_RRCE_KPI_PROC_ID);
            int status = getFieldValue(data, MDMContent.EM_RRCE_KPI_STATUS);
            String direction = mRrceDirectMapping.get(procId);
            String procType = mRrceTypeMapping.get(procId);
            clearData();
            addData(new String[]{direction, procType,
                    mStatusMapping.get(status)});
        } else if (name.equals(MDMContent.MSG_ID_EM_ERRC_SUCCESS_RATE_KPI_IND)) {
            int procId = getFieldValue(data,
                    MDMContent.EM_ERRC_SUCCESS_RATE_KPI_PROC_ID);
            int status = getFieldValue(data,
                    MDMContent.EM_ERRC_SUCCESS_RATE_KPI_STATUS);
            String direction = mErrcDirectMapping.get(procId);
            String procType = mErrcTypeMapping.get(procId);
            clearData();
            addData(new String[]{direction, procType,
                    mStatusMapping.get(status)});
        }
    }
}

class UmtsRrcMeasurementControlForE3a extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_EVENT_TYPE_3_PARAMETER_INFO_IND};

    HashMap<Integer, String> mQtyMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "UMTS RSCP");
            put(2, "UMTS ECN0");
            put(3, "UMTS PATHLOSS");
        }
    };
    HashMap<Integer, String> mQtyOtherRatMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "GSM RSSI");
            put(2, "LTE RSRP");
            put(3, "LTE RSRQ");
        }
    };

    public UmtsRrcMeasurementControlForE3a(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS RRC Measurement control for e3a (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"e3a meas ID", "UMTS RAT meas quantity",
                "Other RAT meas quantity", "UMTS RAT threshold",
                "Other RAT threshold"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_MEME_EVENT_TYPE_3_EM_EVENT_INFO + "[";
        int num = getFieldValue(data,
                MDMContent.TDD_EM_MEME_EVENT_TYPE_3_NUM_EVENT_INFO);
        clearData();
        for (int i = 0; i < num && i < 4; i++) {
            int measId = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASID);
            int eventType = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_EVENT_TYPE);
            int measQty = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASQTY);
            int measQtyOtherRAT = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASQTYOTHERRAT);
            int thresholdOwnSystem = getFieldValue(data, coName + i + "]."
                            + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_THRESHOLDOWNSYSTEM,
                    true);
            int thresholdOtherSystem = getFieldValue(data, coName + i + "]."
                            + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_THRESHOLDOTHERSYSTEM,
                    true);
            if (eventType == 1) {
                if (measId == 0) {
                    addData("-");
                } else {
                    addData(measId);
                }
                addData(mQtyMapping.get(measQty));
                addData(mQtyOtherRatMapping.get(measQtyOtherRAT));
                if (thresholdOwnSystem == 0xFFFF) {
                    addData("-");
                } else {
                    addData((float) thresholdOwnSystem / 4);
                }
                if (thresholdOtherSystem == 0xFFFF) {
                    addData("-");
                } else {
                    addData((float) thresholdOtherSystem / 4);
                }
            }
        }
        notifyDataSetChanged();
    }
}

class UmtsRrcMeasurementControlForE3b extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_EVENT_TYPE_3_PARAMETER_INFO_IND};

    HashMap<Integer, String> mQtyMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "UMTS RSCP");
            put(2, "UMTS ECN0");
            put(3, "UMTS PATHLOSS");
        }
    };
    HashMap<Integer, String> mQtyOtherRatMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "GSM RSSI");
            put(2, "LTE RSRP");
            put(3, "LTE RSRQ");
        }
    };

    public UmtsRrcMeasurementControlForE3b(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS RRC Measurement control for e3b (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"e3b meas ID", "UMTS RAT meas quantity",
                "Other RAT meas quantity", "UMTS RAT threshold",
                "Other RAT threshold"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_MEME_EVENT_TYPE_3_EM_EVENT_INFO + "[";
        int num = getFieldValue(data,
                MDMContent.TDD_EM_MEME_EVENT_TYPE_3_NUM_EVENT_INFO);
        clearData();
        for (int i = 0; i < num && i < 4; i++) {
            int measId = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASID);
            int eventType = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_EVENT_TYPE);
            int measQty = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASQTY);
            int measQtyOtherRAT = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASQTYOTHERRAT);
            int thresholdOwnSystem = getFieldValue(data, coName + i + "]."
                            + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_THRESHOLDOWNSYSTEM,
                    true);
            int thresholdOtherSystem = getFieldValue(data, coName + i + "]."
                            + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_THRESHOLDOTHERSYSTEM,
                    true);
            if (eventType == 2) {
                if (measId == 0) {
                    addData("-");
                } else {
                    addData(measId);
                }
                addData(mQtyMapping.get(measQty));
                addData(mQtyOtherRatMapping.get(measQtyOtherRAT));
                if (thresholdOwnSystem == 0xFFFF) {
                    addData("-");
                } else {
                    addData((float) thresholdOwnSystem / 4);
                }
                if (thresholdOtherSystem == 0xFFFF) {
                    addData("-");
                } else {
                    addData((float) thresholdOtherSystem / 4);
                }
            }
        }
        notifyDataSetChanged();
    }
}

class UmtsRrcMeasurementControlForE3c extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_EVENT_TYPE_3_PARAMETER_INFO_IND};

    HashMap<Integer, String> mQtyMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "UMTS RSCP");
            put(2, "UMTS ECN0");
            put(3, "UMTS PATHLOSS");
        }
    };
    HashMap<Integer, String> mQtyOtherRatMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "GSM RSSI");
            put(2, "LTE RSRP");
            put(3, "LTE RSRQ");
        }
    };

    public UmtsRrcMeasurementControlForE3c(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS RRC Measurement control for e3c (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"e3c meas ID", "UMTS RAT meas quantity",
                "Other RAT meas quantity", "UMTS RAT threshold",
                "Other RAT threshold"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_MEME_EVENT_TYPE_3_EM_EVENT_INFO + "[";
        int num = getFieldValue(data,
                MDMContent.TDD_EM_MEME_EVENT_TYPE_3_NUM_EVENT_INFO);
        clearData();
        for (int i = 0; i < num && i < 4; i++) {
            int measId = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASID);
            int eventType = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_EVENT_TYPE);
            int measQty = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASQTY);
            int measQtyOtherRAT = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASQTYOTHERRAT);
            int thresholdOwnSystem = getFieldValue(data, coName + i + "]."
                            + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_THRESHOLDOWNSYSTEM,
                    true);
            int thresholdOtherSystem = getFieldValue(data, coName + i + "]."
                            + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_THRESHOLDOTHERSYSTEM,
                    true);
            if (eventType == 3) {
                if (measId == 0) {
                    addData("-");
                } else {
                    addData(measId);
                }
                addData(mQtyMapping.get(measQty));
                addData(mQtyOtherRatMapping.get(measQtyOtherRAT));
                if (thresholdOwnSystem == 0xFFFF) {
                    addData("-");
                } else {
                    addData((float) thresholdOwnSystem / 4);
                }
                if (thresholdOtherSystem == 0xFFFF) {
                    addData("-");
                } else {
                    addData((float) thresholdOtherSystem / 4);
                }
            }
        }
        notifyDataSetChanged();
    }
}

class UmtsRrcMeasurementControlForE3d extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_EVENT_TYPE_3_PARAMETER_INFO_IND};

    HashMap<Integer, String> mQtyMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "UMTS RSCP");
            put(2, "UMTS ECN0");
            put(3, "UMTS PATHLOSS");
        }
    };
    HashMap<Integer, String> mQtyOtherRatMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "GSM RSSI");
            put(2, "LTE RSRP");
            put(3, "LTE RSRQ");
        }
    };

    public UmtsRrcMeasurementControlForE3d(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS RRC Measurement control for e3d (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"e3d meas ID", "UMTS RAT meas quantity",
                "Other RAT meas quantity", "UMTS RAT threshold",
                "Other RAT threshold"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_MEME_EVENT_TYPE_3_EM_EVENT_INFO + "[";
        int num = getFieldValue(data,
                MDMContent.TDD_EM_MEME_EVENT_TYPE_3_NUM_EVENT_INFO);
        clearData();
        for (int i = 0; i < num && i < 4; i++) {
            int measId = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASID);
            int eventType = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_EVENT_TYPE);
            int measQty = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASQTY);
            int measQtyOtherRAT = getFieldValue(data, coName + i + "]."
                    + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_MEASQTYOTHERRAT);
            int thresholdOwnSystem = getFieldValue(data, coName + i + "]."
                            + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_THRESHOLDOWNSYSTEM,
                    true);
            int thresholdOtherSystem = getFieldValue(data, coName + i + "]."
                            + MDMContent.TDD_EM_MEME_EVENT_TYPE_3_THRESHOLDOTHERSYSTEM,
                    true);
            if (eventType == 4) {
                if (measId == 0) {
                    addData("-");
                } else {
                    addData(measId);
                }
                addData(mQtyMapping.get(measQty));
                addData(mQtyOtherRatMapping.get(measQtyOtherRAT));
                if (thresholdOwnSystem == 0xFFFF) {
                    addData("-");
                } else {
                    addData((float) thresholdOwnSystem / 4);
                }
                if (thresholdOtherSystem == 0xFFFF) {
                    addData("-");
                } else {
                    addData((float) thresholdOtherSystem / 4);
                }
            }
        }
        notifyDataSetChanged();
    }
}

class UmtsRrcMeasurementReportForE3a extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_REPORT_INFO_IND};

    public UmtsRrcMeasurementReportForE3a(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS RRC Measurement report for e3a (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"e3a meas ID"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_MEME_REPORT_EM_REPORT_INFO + ".";
        int measId = getFieldValue(data, coName
                + MDMContent.TDD_EM_MEME_REPORT_MEAS_ID);
        int eventType = getFieldValue(data, coName
                + MDMContent.TDD_EM_MEME_REPORT_EVENT_TYPE);
        clearData();
        if (eventType == 1) {
            if (measId == 0) {
                addData("-");
            } else {
                addData(measId);
            }
        }
        notifyDataSetChanged();
    }
}

class UmtsRrcMeasurementReportForE3b extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_REPORT_INFO_IND};

    public UmtsRrcMeasurementReportForE3b(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS RRC Measurement report for e3b (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"e3b meas ID"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_MEME_REPORT_EM_REPORT_INFO + ".";
        int measId = getFieldValue(data, coName
                + MDMContent.TDD_EM_MEME_REPORT_MEAS_ID);
        int eventType = getFieldValue(data, coName
                + MDMContent.TDD_EM_MEME_REPORT_EVENT_TYPE);
        clearData();
        if (eventType == 2) {
            if (measId == 0) {
                addData("-");
            } else {
                addData(measId);
            }
        }
        notifyDataSetChanged();
    }
}

class UmtsRrcMeasurementReportForE3c extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_REPORT_INFO_IND};

    public UmtsRrcMeasurementReportForE3c(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS RRC Measurement report for e3c (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"e3c meas ID"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_MEME_REPORT_EM_REPORT_INFO + ".";
        int measId = getFieldValue(data, coName
                + MDMContent.TDD_EM_MEME_REPORT_MEAS_ID);
        int eventType = getFieldValue(data, coName
                + MDMContent.TDD_EM_MEME_REPORT_EVENT_TYPE);
        clearData();
        if (eventType == 3) {
            if (measId == 0) {
                addData("-");
            } else {
                addData(measId);
            }
        }
        notifyDataSetChanged();
    }
}

class UmtsRrcMeasurementReportForE3d extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_REPORT_INFO_IND};

    public UmtsRrcMeasurementReportForE3d(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS RRC Measurement report for e3d (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"e3d meas ID"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_MEME_REPORT_EM_REPORT_INFO + ".";
        int measId = getFieldValue(data, coName
                + MDMContent.TDD_EM_MEME_REPORT_MEAS_ID);
        int eventType = getFieldValue(data, coName
                + MDMContent.TDD_EM_MEME_REPORT_EVENT_TYPE);
        clearData();
        if (eventType == 4) {
            if (measId == 0) {
                addData("-");
            } else {
                addData(measId);
            }
        }
        notifyDataSetChanged();
    }
}

class UmtsNeighborCellInfoGsmTdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRM_IR_3G_NEIGHBOR_MEAS_STATUS_IND};
    HashMap<Integer, String> mCellTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "NORMAL");
            put(1, "CSG");
            put(2, "HYBRID");
        }
    };
    HashMap<Integer, String> mFailTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "IR_INITIAL_VALUE");
            put(1, "NEVER_ALLOWED");
            put(2, "PLMN_ID_MISMATCHED");
            put(3, "LA_NOT_ALLOWED");
            put(4, "CELL_BARRED");
            put(5, "TEMP_FAILURE");
            put(6, "CRITERIA3_NOT_SATISFIED");
            put(7, "TA_NOT_ALLOWED");
            put(8, "CELL_BARRED_FREQ");
            put(9, "CELL_BARRED_RESV_OPERATOR");
            put(10, "CELL_BARRED_RESV_OPERATOR_FREQ");
            put(11, "CSG_NOT_ALLOWED");
        }
    };

    public UmtsNeighborCellInfoGsmTdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS Neighbor Cell Info (GSM TDD)";
    }

    @Override
    String getGroup() {
        return "2. GSM EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"cell_type", "uarfcn", "phy_id", "strength",
                "quailty", "rep_value", "non_rep_value", "fail_type"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS + "[";
        clearData();
        for (int i = 0; i < 6; i++) {
            int isValid = getFieldValue(data, coName + i + "]."
                    + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_IS_VALID);
            if (isValid > 0) {
                int cellType = getFieldValue(data, coName + i + "]."
                        + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_CELL_TYPE);
                int failType = getFieldValue(data, coName + i + "]."
                        + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_FAIL_TYPE);
                String cell = mCellTypeMapping.get(cellType);
                String fail = mFailTypeMapping.get(failType);
                addData(cell);
                addData(getFieldValue(data, coName + i + "]."
                        + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_UARFCN));
                addData(getFieldValue(data, coName + i + "]."
                        + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_PHY_ID));
                addData(getFieldValue(data, coName + i + "]."
                                + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_STRENGTH,
                        true));
                addData(getFieldValue(data, coName + i + "]."
                                + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_QUALITY,
                        true));
                addData(getFieldValue(data, coName + i + "]."
                        + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_REP_VALUE));
                addData(getFieldValue(
                        data,
                        coName
                                + i
                                + "]."
                                + MDMContent.RRM_IR_3G_NEIGHBOR_MEAS_STATUS_NON_REP_VALUE));
                addData(fail);
            }
        }
        notifyDataSetChanged();
    }
}

class LTENeighborCellInfoGSM extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRM_IR_4G_NEIGHBOR_MEAS_STATUS_IND};
    HashMap<Integer, String> mCellTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "Normal");
            put(1, "CSG");
            put(2, "Hybrid");
        }
    };
    HashMap<Integer, String> mBandTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "FDD");
            put(2, "TDD");
        }
    };
    HashMap<Integer, String> mFailTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "None");
            put(1, "Never");
            put(2, "PLMN");
            put(3, "LA");
            put(4, "C_BAR");
            put(5, "TEMP");
            put(6, "CRI3");
            put(7, "TA");
            put(8, "FREQ");
            put(9, "RES_OP");
            put(10, "RES_OP_FRE");
            put(11, "CSG");
            put(12, "GEMINI");
            put(13, "NAS");
        }
    };

    public LTENeighborCellInfoGSM(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE Neighbor Cell Info (GSM)";
    }

    @Override
    String getGroup() {
        return "2. GSM EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"cell_type", "earfcn", "pci", "band_type",
                "rsrp", "rsrq", "rep_value", "non_rep_value", "fail_type"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_STATUS + "[";
        clearData();
        for (int i = 0; i < 6; i++) {
            int isValid = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_IS_VALID);
            int cellType = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_CELL_TYPE);
            long earfcn = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_EARFCN);
            int pci = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_PCI);
            int bandType = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_BAND_TYPE);
            int rsrp = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_RSRP, true);
            int rsrq = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_RSRQ, true);
            int repValue = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_REP_VALUE);
            int nonRepValue = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_NON_REP_VALUE);
            int failType = getFieldValue(data, coName + i + "]."
                    + MDMContent.EM_RRM_IR_4G_NEIGHBOR_MEAS_FAIL_TYPE);
            if (isValid > 0) {
                String cell = mCellTypeMapping.get(cellType);
                String band = mBandTypeMapping.get(bandType);
                String fail = mFailTypeMapping.get(failType);
                addData(cell);
                addData(earfcn);
                addData(pci);
                addData(band);
                addData(rsrp);
                addData(rsrq);
                addData(repValue);
                addData(nonRepValue);
                addData(fail);
            }
        }
        notifyDataSetChanged();
    }
}

class LteErrcMeasurementConfigForEventB1 extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_CONFIG_INFO_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "GSM RSSI");
            put(2, "UMTS RSCP");
            put(3, "UMTS ECN0");
        }
    };

    public LteErrcMeasurementConfigForEventB1(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE ERRC Measurement config for event B1";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Event B1 meas ID", "Other RAT meas quantity",
                "Other RAT threshold"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO + "[";
        int num = getFieldValue(data,
                MDMContent.EM_ERRC_MOB_MEAS_CONFIG_NUM_EVENT_INFO);
        clearData();
        for (int i = 0; i < num && i < 2; i++) {
            int eventType = getFieldValue(
                    data,
                    coName
                            + i
                            + "]."
                            + MDMContent.EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_EVENT_TYPE);
            if (eventType == 1) {
                int measId = getFieldValue(
                        data,
                        coName
                                + i
                                + "]."
                                + MDMContent.EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_MEASID);
                int measQtyOtherRat = getFieldValue(
                        data,
                        coName
                                + i
                                + "]."
                                + MDMContent.EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_MEASQTYOTHERRAT);
                int threshold = getFieldValue(
                        data,
                        coName
                                + i
                                + "]."
                                + MDMContent
                                .EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_THRESHOLDOTHERSYSTEM,
                        true);
                addData(measId == 0 ? "-" : measId,
                        mMapping.get(measQtyOtherRat), threshold == 0 ? "-"
                                : (float) threshold / 4);
            }
        }
    }
}

class LteErrcMeasurementConfigForEventB2 extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_CONFIG_INFO_IND};
    HashMap<Integer, String> mMeasQtyMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "LTE RSRP");
            put(2, "LTE RSRQ");
        }
    };
    HashMap<Integer, String> mMeasQtyOtherMapping = new HashMap<Integer, String>() {
        {
            put(0, "-");
            put(1, "GSM RSSI");
            put(2, "UMTS RSCP");
            put(3, "UMTS ECN0");
        }
    };

    public LteErrcMeasurementConfigForEventB2(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE ERRC Measurement config for event B2";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Event B2 meas ID", "LTE RAT meas quantity",
                "Other RAT meas quantity", "LTE RAT threshold",
                "Other RAT threshold"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO + "[";
        int num = getFieldValue(data,
                MDMContent.EM_ERRC_MOB_MEAS_CONFIG_NUM_EVENT_INFO);
        clearData();
        for (int i = 0; i < num && i < 2; i++) {
            int eventType = getFieldValue(
                    data,
                    coName
                            + i
                            + "]."
                            + MDMContent.EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_EVENT_TYPE);
            if (eventType == 2) {
                int measId = getFieldValue(
                        data,
                        coName
                                + i
                                + "]."
                                + MDMContent.EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_MEASID);
                int measQty = getFieldValue(
                        data,
                        coName
                                + i
                                + "]."
                                + MDMContent.EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_MEASQTY);
                int measQtyOtherRat = getFieldValue(
                        data,
                        coName
                                + i
                                + "]."
                                + MDMContent.EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_MEASQTYOTHERRAT);
                int threshold = getFieldValue(
                        data,
                        coName
                                + i
                                + "]."
                                + MDMContent
                                .EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_THRESHOLDOWNSYSTEM,
                        true);
                int thresholdOtherSystem = getFieldValue(
                        data,
                        coName
                                + i
                                + "]."
                                + MDMContent
                                .EM_ERRC_MOB_MEAS_CONFIG_EM_EVENT_INFO_THRESHOLDOTHERSYSTEM,
                        true);
                addData(measId == 0 ? "-" : measId,
                        mMeasQtyMapping.get(measQty),
                        mMeasQtyOtherMapping.get(measQtyOtherRat),
                        threshold == 0 ? "-" : (float) threshold / 4,
                        thresholdOtherSystem == 0 ? "-"
                                : (float) thresholdOtherSystem / 4);
            }
        }
    }
}

class LteErrcMeasurementReportForEventB1 extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_REPORT_INFO_IND};

    public LteErrcMeasurementReportForEventB1(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE ERRC Measurement report for event B1";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Event B1 meas ID"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String coName = MDMContent.EM_ERRC_MOB_MEAS_REPORT_EM_REPORT_INFO + ".";
        int eventType = getFieldValue(data, coName
                + MDMContent.EM_ERRC_MOB_MEAS_REPORT_EM_EVENT_TYPE);
        if (eventType == 1) {
            int measId = getFieldValue(data, coName
                    + MDMContent.EM_ERRC_MOB_MEAS_REPORT_EM_EVENT_INFO_MEASID);
            addData(measId == 0 ? "-" : measId);
        }
    }
}

class LteErrcMeasurementReportForEventB2 extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_REPORT_INFO_IND};

    public LteErrcMeasurementReportForEventB2(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE ERRC Measurement report for event B2";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Event B2 meas ID"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String coName = MDMContent.EM_ERRC_MOB_MEAS_REPORT_EM_REPORT_INFO + ".";
        int eventType = getFieldValue(data, coName
                + MDMContent.EM_ERRC_MOB_MEAS_REPORT_EM_EVENT_TYPE);
        if (eventType == 2) {
            int measId = getFieldValue(data, coName
                    + MDMContent.EM_ERRC_MOB_MEAS_REPORT_EM_EVENT_INFO_MEASID);
            addData(measId == 0 ? "-" : measId);
        }
    }
}

class EDchTtiConfigured extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRCE_HSPA_CONFIG_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "2 ms");
            put(1, "10 ms");
            put(255, "N/A");
        }
    };

    public EDchTtiConfigured(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "E-DCH TTI Configured";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"E-DCH TTI"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String tti = mMapping.get(getFieldValue(data,
                MDMContent.EM_ERRC_HSPA_E_DCH_TTI));
        clearData();
        addData(tti == null ? "N/A" : tti);
    }
}

class HsdpaConfiguredUmtsFdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRCE_HSPA_CONFIG_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "FALSE");
            put(1, "TRUE");
        }
    };

    public HsdpaConfiguredUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "HSDPA configured (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"HSDPA configured"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String hsdpa = mMapping.get(getFieldValue(data,
                MDMContent.EM_ERRC_HSPA_HSDPA_CONFIG));
        clearData();
        addData(hsdpa == null ? "FALSE" : hsdpa);
    }
}

class DcHsdpaConfiguredUmtsFdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRCE_HSPA_CONFIG_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "FALSE");
            put(1, "TRUE");
        }
    };

    public DcHsdpaConfiguredUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "DC-HSDPA configured (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DC-HSDPA configured"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String dcHsdpa = mMapping.get(getFieldValue(data,
                MDMContent.EM_ERRC_HSPA_DC_HSDPA_CONFIG));
        clearData();
        addData(dcHsdpa == null ? "FALSE" : dcHsdpa);
    }
}

class HsDschModulationPrimaryCell extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_UL1_HSPA_INFO_GROUP_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "QPSK (0)");
            put(1, "16QAM (1)");
            put(2, "64QAM (2)");
        }
    };

    public HsDschModulationPrimaryCell(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "HS-DSCH Modulation (Primary Cell)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Modulation (Pri)"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.FDD_EM_UL1_HSPA_PRIMARY_HS_DSCH_BLER + ".";
        int mod = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_HSPA_DSCH_CURR_MOD);
        clearData();
        addData(mMapping.get(mod));
    }
}

class HsupaConfiguredUmtsFdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRCE_HSPA_CONFIG_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "FALSE");
            put(1, "TRUE");
        }
    };

    public HsupaConfiguredUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "HSUPA configured (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"HSUPA configured"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String hsupa = mMapping.get(getFieldValue(data,
                MDMContent.EM_ERRC_HSPA_HSUPA_CONFIG));
        clearData();
        addData(hsupa == null ? "FALSE" : hsupa);
    }
}

class WcdmaHsupaCapability extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_USIME_CAPABILITY_INFO_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "ON");
            put(1, "OFF");
        }
    };

    public WcdmaHsupaCapability(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "WCDMA HSUPA Capability";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"HSUPA Support"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        addData(mMapping.get(getFieldValue(data,
                MDMContent.FDD_EM_USIME_CAPABILITY_HSUPA_ENABLE)));
    }
}

class WcdmaHsdpaCapability extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_USIME_CAPABILITY_INFO_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "ON");
            put(1, "OFF");
        }
    };

    public WcdmaHsdpaCapability(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "WCDMA HSDPA Capability";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"HSDPA Support"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        addData(mMapping.get(getFieldValue(data,
                MDMContent.FDD_EM_USIME_CAPABILITY_HSDPA_ENABLE)));
    }
}

class WcdmaTasInfo extends NormalTableTasComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_UL1_TAS_INFO_IND,
            MDMContent.MSG_ID_FDD_EM_UL1_UTAS_INFO_IND};

    int TasVersion = 1;
    HashMap<Integer, String> AntennaMapping = new HashMap<Integer, String>() {
        {
            put(0, "LANT");
            put(1, "UANT");
            put(2, "LANT(')");
            put(3, "UANT");
            put(4, "-");
        }
    };
    HashMap<Integer, String> TasEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
            put(2, "-");
        }
    };

    public WcdmaTasInfo(Activity context) {
        super(context);
    }

    String antidxMapping(int antidx) {
        String antidx_s = "";
        if (antidx >= 0 && antidx <= 3)
            antidx_s = AntennaMapping.get(antidx);
        else
            antidx_s = AntennaMapping.get(4) + "(" + antidx + ")";
        return antidx_s;
    }

    String tasEableMapping(int tasidx) {
        String tasidx_s = "";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = TasEnableMapping.get(tasidx);
        } else
            tasidx_s = TasEnableMapping.get(2) + "(" + tasidx + ")";
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        String bandidx_s = "Band";
        bandidx_s = bandidx_s + " " + bandidx;
        return bandidx_s;
    }

    String[] conbineLablesByModem(String[] lables1, String[] lables2,
                                  int position) {
        if (FeatureSupport.is93Modem()) {
            if (position < 0) {
                position = Math.abs(position);
                return addLablesAtPosition(lables2, lables1, position);
            }
            return addLablesAtPosition(lables1, lables2, position);
        }
        return lables2;
    }

    @Override
    String getName() {
        return "WCDMA TAS Info";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        final String[] Lables_Verison_v1 = new String[]{"TX Antenna",
                "Antenna State", "RSCP_Diff", "RSCP_LANT", "RSCP_UANT",
                "TX Power", "DPCCH TX Power"};
        final String[] Lables_Verison_v2 = new String[]{"TX Antenna",
                "Antenna State", "RSCP_Diff", "RSCP_LANT", "RSCP_UANT",
                "RSCP_LANT(')", "TX Power", "DPCCH TX Power"};
        final String[] Lables_Tas = new String[]{"Tas Enable Info",
                "Serving Band", "Serving UARFCN"};
        final String[] Labels_Dat = new String[]{"DAT Index"};

        if (TasVersion == 2) {
            return concatAll(conbineLablesByModem(Lables_Tas, Lables_Verison_v2,
                    Lables_Tas.length), Labels_Dat);
        }
        return concatAll(conbineLablesByModem(Lables_Tas, Lables_Verison_v1,
                Lables_Tas.length), Labels_Dat);
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_FDD_EM_UL1_UTAS_INFO_IND)) {
            setInfoValid(1);
        } else {
            setInfoValid(0);
        }
        //@test
        //utas_info_valid = 1;
        if (isInfoValid()) {
            clearData();
            mAdapter.add(new String[]{"Use " + getName().replace("UTAS", "TAS")});
            return;
        }
        String coName = MDMContent.FDD_EM_UL1_TAS_EMUL1TAS + ".";
        int tasidx = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_ENABLE);
        int bandidx = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_BAND);
        int uarfcnidx = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_UARFCN);
        int antidx = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_MAIN_ANT_IDX);

        int rscp_diff = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_RSCP_DIFF, true);
        int dat_index = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_DAT_SCENARIO_INDEX, true);
        String rscp_diff_s;
        if (rscp_diff == -480)
            rscp_diff_s = " ";
        else
            rscp_diff_s = String.valueOf(rscp_diff / 4);

        int rscp_0 = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_RSCP0, true);
        String rscp_0_s;
        if (rscp_0 == -480)
            rscp_0_s = " ";
        else
            rscp_0_s = String.valueOf(rscp_0 / 4);

        int rscp_1 = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_RSCP1, true);
        String rscp_1_s;
        if (rscp_1 == -480)
            rscp_1_s = " ";
        else
            rscp_1_s = String.valueOf(rscp_1 / 4);

        int rscp_2 = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_RSCP2, true);
        String rscp_2_s;
        if (rscp_2 == -480)
            rscp_2_s = " ";
        else
            rscp_2_s = String.valueOf(rscp_2 / 4);

        int tx_pwr = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_TX_PWR, true);
        String tx_pwr_s;
        if (tx_pwr == -128)
            tx_pwr_s = " ";
        else
            tx_pwr_s = String.valueOf(tx_pwr);

        int dpcch_tx_pwr = getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_DPCCH_TX_PWR, true);
        String dpcch_tx_pwr_s;
        if (dpcch_tx_pwr == -128)
            dpcch_tx_pwr_s = " ";
        else
            dpcch_tx_pwr_s = String.valueOf(dpcch_tx_pwr);

        TasVersion = (getFieldValue(data, coName
                + MDMContent.FDD_EM_UL1_TAS_VERISION, true) == 2) ? 2 : 1;

        clearData();

        if (FeatureSupport.is93Modem()) {
            addData(tasEableMapping(tasidx), servingBandMapping(bandidx),
                    uarfcnidx);
        }
        if (TasVersion == 2) {
            addData(antidxMapping(antidx), antidx, rscp_diff_s, rscp_0_s,
                    rscp_1_s, rscp_2_s, tx_pwr_s, dpcch_tx_pwr_s);
        } else {
            addData(antidxMapping(antidx), antidx, rscp_diff_s, rscp_0_s,
                    rscp_1_s, tx_pwr_s, dpcch_tx_pwr_s);
        }
        addData(dat_index);
        notifyDataSetChanged();

    }
}

class WcdmaUTasInfo extends CombinationTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_UL1_TAS_INFO_IND,
            MDMContent.MSG_ID_FDD_EM_UL1_UTAS_INFO_IND};
    HashMap<Integer, String> RxSystemMapping = new HashMap<Integer, String>() {
        {
            put(1, "OneRX");
            put(2, "TwoRX");
        }
    };

    HashMap<Integer, String> TasEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
        }
    };

    HashMap<Integer, String> CcmStateMapping = new HashMap<Integer, String>() {
        {
            put(3, "PCH");
            put(4, "FACH");
            put(5, "DCH");
        }
    };
    List<String[]> labelsList = new ArrayList<String[]>() {{
        add(new String[]{"TAS Enable Info", "Serving Band", "Current Ant State",
                "Previous Ant State", "Rx System", "RRC State"});
        add(new String[]{"TX Index", "RX Index"});
        add(new String[]{"ANT Index", "TX Pwr dBm", "Pwr Hdr DB", "RSCP dBm"});
    }};
    String[] labelsKey = new String[]{"Tas Common", "TX Info", "ANT Info"};
    List<LinkedHashMap> valuesHashMap = new ArrayList<LinkedHashMap>() {{
        add(initHashMap(labelsList.get(0)));
        add(initHashMap(labelsList.get(1)));
        add(initArrayHashMap(labelsList.get(2)));
    }};
    private String[] tabTitle = new String[]{"Common", "Detail"};

    public WcdmaUTasInfo(Activity context) {
        super(context);
        initTableComponent(tabTitle);
    }

    String tasEableMapping(int tasidx) {
        String tasidx_s = "";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = TasEnableMapping.get(tasidx);
        } else
            tasidx_s = "-(" + tasidx + ")";
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        String bandidx_s = "Band";
        bandidx_s = bandidx_s + " " + bandidx;
        return bandidx_s;
    }

    @Override
    String getName() {
        return "WCDMA UTAS Info";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    ArrayList<String> getArrayTypeKey() {
        ArrayList<String> arrayTypeKeys = new ArrayList<String>();
        arrayTypeKeys.add(labelsKey[2]);
        return arrayTypeKeys;
    }

    @Override
    boolean isLabelArrayType(String label) {
        if (getArrayTypeKey().contains(label)) {
            return true;
        }
        return false;
    }

    @Override
    LinkedHashMap<String, LinkedHashMap> getHashMapLabels(int index) {
        LinkedHashMap<String, LinkedHashMap> hashMapkeyValues =
                new LinkedHashMap<String, LinkedHashMap>();
        switch (index) {
            case 0:
                hashMapkeyValues.put(labelsKey[0], valuesHashMap.get(0));
                break;
            case 1:
                hashMapkeyValues.put(labelsKey[1], valuesHashMap.get(1));
                hashMapkeyValues.put(labelsKey[2], valuesHashMap.get(2));
                break;
        }
        return hashMapkeyValues;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        if (name.equals(MDMContent.MSG_ID_FDD_EM_UL1_TAS_INFO_IND)) {
            setInfoValid(0);
        } else {
            setInfoValid(1);
        }
        if (!isInfoValid()) {
            resetView();
            TextView textView = new TextView(mContext);
            ListView.LayoutParams layoutParams = new ListView.LayoutParams(
                    ListView.LayoutParams.MATCH_PARENT, ListView.LayoutParams.WRAP_CONTENT);
            textView.setLayoutParams(layoutParams);
            textView.setPadding(20, 0, 20, 0);
            textView.setText("Use " + getName().replace("UTAS", "TAS"));
            textView.setTextSize(16);
            scrollView.addView(textView);
            return;
        }
        int tas_enable = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_ENABLE);
        int surving_band = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_BAND);
        int current_ant_state = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_CURRENT_ANT_STATE);
        int previous_ant_state = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_PREVIOUS_ANT_STATE);
        int rx_system = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_RX_SYSTEM);
        int ccm_state = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_CCMSTATE);
        setHashMapKeyValues(labelsKey[0], 0, labelsList.get(0), new String[]{
                tasEableMapping(tas_enable),
                servingBandMapping(surving_band),
                current_ant_state >= 0 && current_ant_state <= 23 ?
                        current_ant_state + "" : "-(" + current_ant_state + ")",
                previous_ant_state >= 0 && previous_ant_state <= 23 ?
                        previous_ant_state + "" : "-(" + previous_ant_state + ")",
                RxSystemMapping.containsKey(rx_system) ?
                        RxSystemMapping.get(rx_system) : "-(" + rx_system + ")",
                CcmStateMapping.containsKey(ccm_state) ?
                        CcmStateMapping.get(ccm_state) : "-(" + ccm_state + ")",
        });
        addData(labelsKey[0], 0);
        int tx_index = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_TX_INDEX);
        int rx_index = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_RX_INDEX);
        setHashMapKeyValues(labelsKey[1], 1, labelsList.get(1), new String[]{
                tx_index >= 0 && tx_index <= 7 ? tx_index + "" : "-(" + tx_index + ")",
                rx_index >= 0 && rx_index <= 7 ? rx_index + "" : "-(" + rx_index + ")"
        });
        addData(labelsKey[1], 1);
        int tx_index_txp = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_TX_INDEX_TXP, true);
        int rx_index_txp = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_RX_INDEX_TXP, true);
        int tx_index_pwrhdr = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_TX_INDEX_PWRHDR, true);
        int rx_index_pwrhdr = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_RX_INDEX_PWRHDR, true);
        int tx_index_rscp = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_TX_INDEX_RSCP, true);
        int rx_index_rscp = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_RX_INDEX_RSCP, true);

        setHashMapKeyValues(labelsKey[2], 1, labelsList.get(2), new String[]{
                tx_index >= 0 && tx_index <= 7 ? tx_index + "" : "-(" + tx_index + ")",
                tx_index_txp == -128 ? "-" : tx_index_txp + "",
                tx_index_pwrhdr == -128 ? "-" : tx_index_pwrhdr + "",
                tx_index_rscp == -480 ? "-" : String.valueOf(tx_index_rscp / 4)
        });
        setHashMapKeyValues(labelsKey[2], 1, labelsList.get(2), new String[]{
                rx_index >= 0 && rx_index <= 7 ? rx_index + "" : "-(" + rx_index + ")",
                rx_index_txp == -128 ? "-" : rx_index_txp + "",
                rx_index_pwrhdr == -128 ? "-" : rx_index_pwrhdr + "",
                rx_index_rscp == -480 ? "-" : String.valueOf(rx_index_rscp / 4)
        });
        addData(labelsKey[2], 1);
    }
}

class Fdd3gSpeechCodec extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_CSR_STATUS_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "AMR_4_75");
            put(1, "AMR_5_15");
            put(2, "AMR_5_90");
            put(3, "AMR_6_70");
            put(4, "AMR_7_40");
            put(5, "AMR_7_95");
            put(6, "AMR_10_20");
            put(7, "AMR_12_20");
            put(8, "AMR_SID");
            put(9, "GSM_EFR_SID");
            put(10, "TDMA_EFR_SID");
            put(11, "PDC_EFR_SID");
            put(12, "RESERVE_1");
            put(13, "RESERVE_2");
            put(14, "RESERVE_3");
            put(15, "AMR_NODATA");
            put(16, "AMRWB_6_60");
            put(17, "AMRWB_8_85");
            put(18, "AMRWB_12_65");
            put(19, "AMRWB_14_25");
            put(20, "AMRWB_15_85");
            put(21, "AMRWB_18_25");
            put(22, "AMRWB_19_85");
            put(23, "AMRWB_23_05");
            put(24, "AMRWB_23_85");
            put(25, "AMRWB_SID");
        }
    };

    public Fdd3gSpeechCodec(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "3G FDD Speech Codec";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"UL speech codec", "DL speech codec"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        addData(mMapping.get(getFieldValue(data, MDMContent.EM_CSR_ULAMRTYPE)));
        addData(mMapping.get(getFieldValue(data, MDMContent.EM_CSR_DLAMRTYPE)));
    }
}

class PlmnSearchStatusUmtsFdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_URR_3G_GENERAL_STATUS_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "");
            put(1, "Search any PLMN");
            put(2, "Search given PLMN");
            put(3, "Search any PLMN success");
            put(4, "Search any PLMN failure");
            put(5, "Search given PLMN success");
            put(6, "Search given PLMN failure");
            put(7, "Search PLMN abort");
        }
    };

    public PlmnSearchStatusUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "PLMN Search status (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"PLMN search status"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_URR_3G_GENERAL_UMTS_UAS_3G_GENERAL_STATUS
                + ".";
        clearData();
        addData(mMapping.get(getFieldValue(data, coName
                + MDMContent.FDD_EM_URR_3G_GENERAL_PLMN_SEARCH_STATUS)));
    }
}

class CellSupportPsInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_GMM_INFO_IND};

    public CellSupportPsInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Cell Support PS INFO";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"cell support PS"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        addData(getFieldValue(data, MDMContent.EM_GMM_CELL_SUPPORT_PS));
    }
}

class DtxConfiguredUmtsFdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRCE_HSPA_CONFIG_IND};

    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "FALSE");
            put(1, "TRUE");
        }
    };

    public DtxConfiguredUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "DTX configured (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DTX configured"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String dtxConfig = mMapping.get(getFieldValue(data,
                MDMContent.EM_ERRC_HSPA_DTX_CONFIG));
        clearData();
        addData(dtxConfig == null ? "FALSE" : dtxConfig);
    }
}

class DrxConfiguredUmtsFdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRCE_HSPA_CONFIG_IND};

    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "FALSE");
            put(1, "TRUE");
        }
    };

    public DrxConfiguredUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "DRX configured (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DRX configured"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String drxConfig = mMapping.get(getFieldValue(data,
                MDMContent.EM_ERRC_HSPA_DRX_CONFIG));
        clearData();
        addData(drxConfig == null ? "FALSE" : drxConfig);
    }
}

class FastDormancyConfiguration extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRCE_FD_CONFIGURATION_STATUS_IND};

    HashMap<Boolean, String> mMapping = new HashMap<Boolean, String>() {
        {
            put(false, "OFF");
            put(true, "ON");
        }
    };

    public FastDormancyConfiguration(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Fast Dormancy Configuration";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Fast Dormancy configured"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String value = mMapping.get(getFieldValue(data,
                MDMContent.EM_RRCE_FD_CONFIGURATION_FDCFGSTATUS));
        clearData();
        addData(value == null ? "OFF" : value);
    }
}

class PeriodicLocationUpdateValue extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_MM_INFO_IND};

    public PeriodicLocationUpdateValue(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Periodic Location Update Value";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String[] getLabels() {
        return new String[]{"Periodic Location Update Value"};
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int t3212_val = 0;
        clearData();
        if (name.equals(MDMContent.MSG_ID_EM_MM_INFO_IND)) {
            t3212_val = getFieldValue(data, MDMContent.EM_MM_T3212VAL);
        }
        addData(t3212_val);
    }
}

class RejectCauseCode extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_MM_INFO_IND,
            MDMContent.MSG_ID_EM_GMM_INFO_IND};
    int mm_cause;
    int attach_rej_cause;
    int rau_rej_cause;

    public RejectCauseCode(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Reject Cause Code";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String[] getLabels() {
        return new String[]{
                "MM Reject Cause Code",
                "GMM Attach Rejec Cause Code",
                "GMM RAU Reject Cause"
        };
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        if (name.equals(MDMContent.MSG_ID_EM_MM_INFO_IND)) {
            mm_cause = getFieldValue(data, MDMContent.EM_MM_CAUSE);
        } else if (name.equals(MDMContent.MSG_ID_EM_GMM_INFO_IND)) {
            attach_rej_cause = getFieldValue(data, MDMContent.EM_MM_ATTACH_REJ_CAUSE);
            rau_rej_cause = getFieldValue(data, MDMContent.EM_MM_RAU_REJ_CAUSE);
        }
        addData(mm_cause, attach_rej_cause, rau_rej_cause);
    }
}

class ActivePDPContextInformationLTE extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ESM_ESM_INFO_IND,
            MDMContent.MSG_ID_EM_DDM_IP_INFO_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0x21, "IPV4");
            put(0x57, "IPV6");
            put(0x8D, "IPV4V6");
            put(0x01, "PPP");
            put(0x02, "OSP_IMOSS");
            put(0x03, "NULL_PDP");
        }
    };

    String[] pdpapn_s = new String[11];
    String[] ipv4_s = new String[11];
    String[] ipv6_s = new String[11];
    int[] QoS = new int[11];
    int[] addr_type = new int[11];
    int[] linked_cid = new int[11];
    boolean[] status = new boolean[11];

    public ActivePDPContextInformationLTE(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Active PDP Context Information(LTE)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String[] getLabels() {
        return new String[]{"IPV4 address", "IPV6 address", "IP version", "QoS (QCI)", "APN"};
    }

    @Override
    void update(String name, Object msg) {
        clearData();
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_EM_ESM_ESM_INFO_IND)) {
            for (int j = 0; j < 11; j++) {
                String coName = MDMContent.MSG_ID_EM_ESM_ESM_EPSBC + "[" + j + "].";
                int is_active = getFieldValue(data, coName + "is_active");
                Elog.d(TAG, coName + "is_active: " + is_active);
                if (is_active == 0) {
                    status[j] = false;
                    continue;
                } else {
                    status[j] = true;
                }
                addr_type[j] = getFieldValue(data, coName + "ip_addr.ip_addr_type");
                QoS[j] = getFieldValue(data, coName + "qci");
                linked_cid[j] = getFieldValue(data, coName + "linked_cid");

                int apn_length = getFieldValue(data, coName + "apn.length");
                pdpapn_s[j] = "";
                for (int i = 0; i < apn_length; i++) {
                    int value = getFieldValue(data, coName + "apn.data" + "[" + i + "]");
                    if (i > 0 && value < 0x1f) {
                        pdpapn_s[j] += ".";
                    } else {
                        char pdpapn = (char) value;
                        pdpapn_s[j] += pdpapn;
                    }
                }
            }
        }

        if (name.equals(MDMContent.MSG_ID_EM_DDM_IP_INFO_IND)) {
            int p_cid = getFieldValue(data, "p_cid");
            int addr_len = getFieldValue(data, "addr.len");
            for (int index = 0; index < 11; index++) {
                if (p_cid == linked_cid[index]) {
                    ipv4_s[index] = "";
                    for (int i = 0; i < 4; i++) {
                        int ipv4 = getFieldValue(data, "addr.val[" + i + "]");
                        ipv4_s[index] += ipv4;
                        if (i != 3) {
                            ipv4_s[index] += ".";
                        }
                    }
                    int addr_ipv6_base = 4;
                    if (addr_len == 0x10) {  //ipv6 only
                        addr_ipv6_base = 0;
                    } else if (addr_len == 0x14) {//both ipv4 and ipv6
                        addr_ipv6_base = 4;
                    }
                    ipv6_s[index] = "";
                    for (int i = 0 + addr_ipv6_base; i < 16 + addr_ipv6_base; i++) {
                        int ipv6 = getFieldValue(data, "addr.val[" + i + "]");
                        ipv6_s[index] += String.format("%02x", ipv6);
                        if (((i + 1) % 2 == 0) && i != (15 + addr_ipv6_base)) {
                            ipv6_s[index] += ":";
                        }
                    }
                    break;
                }
            }
        }
        for (int i = 0; i < 11; i++) {
            if (status[i] == true) {
                addData(ipv4_s[i], ipv6_s[i], mMapping.get(addr_type[i]), QoS[i], pdpapn_s[i]);
            }
        }


    }
}

class ActivePDPContextInformationUmtsFDD extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_SM_NSAPI5_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI6_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI7_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI8_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI9_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI10_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI11_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI12_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI13_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI14_STATUS_IND,
            MDMContent.MSG_ID_EM_SM_NSAPI15_STATUS_IND
    };
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0x21, "IPV4");
            put(0x57, "IPV6");
            put(0x8D, "IPV4V6");
            put(0x01, "PPP");
            put(0x02, "OSP_IMOSS");
            put(0x03, "NULL_PDP");
        }
    };
    String[] pdpapn_s = new String[11];
    String[] ipv4_s = new String[11];
    String[] ipv6_s = new String[11];
    int[] QoS = new int[11];
    int[] addr_type = new int[11];
    boolean[] status = new boolean[11];

    public ActivePDPContextInformationUmtsFDD(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Active PDP Context Information(UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String[] getLabels() {
        return new String[]{"NSAPI index", "IP Type", "IPv4", "IPv6", "APN", "QoS (traffic_class)"};
    }

    int parsIndex(String name) {
        int index = 0;
        index = Integer.parseInt(name.substring(18).split("_")[0]);
        Elog.d(TAG, "name = " + index);
        return index - 5;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        int index = parsIndex(name);
        int pdp_context_status = getFieldValue(data, "pdp.pdp_context_status");
        Elog.d(TAG, "pdp_context_status = " + pdp_context_status);
        if (pdp_context_status != 0 && pdp_context_status != 2) {
            status[index] = true;
        } else {
            status[index] = false;
        }
        if (status[index] == true) {
            addr_type[index] = getFieldValue(data, "pdp.pdp_addr_type");
            ipv4_s[index] = "";
            for (int i = 0; i < 4; i++) {
                int ipv4 = getFieldValue(data, "pdp.ip[" + i + "]");
                ipv4_s[index] += ipv4;
                if (i != 3) {
                    ipv4_s[index] += ".";
                }
            }
            int addr_ipv6_base = 4;
            if (addr_type[index] == 0x57) {  //ipv6 only
                addr_ipv6_base = 0;
            } else if (addr_type[index] == 0x8D) {//both ipv4 and ipv6
                addr_ipv6_base = 4;
            }
            ipv6_s[index] = "";
            for (int i = 0 + addr_ipv6_base; i < 16 + addr_ipv6_base; i++) {
                int ipv6 = getFieldValue(data, "pdp.ip[" + i + "]");
                ipv6_s[index] += String.format("%02x", ipv6);
                if (((i + 1) % 2 == 0) && i != (15 + addr_ipv6_base)) {
                    ipv6_s[index] += ":";
                }
            }
            pdpapn_s[index] = "";
            for (int i = 0; i < 100; i++) {
                char pdpapn = (char) getFieldValue(data, "pdp.apn" + "[" + i + "]");
                pdpapn_s[index] += pdpapn;
            }
            if (addr_type[index] == 0x21) { //ipv4 only
                ipv6_s[index] = "-";
            } else if (addr_type[index] == 0x57) { //ipv6 only
                ipv4_s[index] = "-";
            }
            QoS[index] = getFieldValue(data, "pdp.em_negotiated_qos.human_readable_traffic_class");
        }

        for (int i = 0; i < 11; i++) {
            if (status[i] == true) {
                addData("SM_NSAP"
                                + (i + 5), mMapping.get(addr_type[i]), ipv4_s[i], ipv6_s[i],
                        pdpapn_s[i],
                        QoS[i]);
            }
        }

    }

}


class TMSIandPTMSI extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_MM_INFO_IND,
            MDMContent.MSG_ID_EM_GMM_INFO_IND};
    String tmsi = "-";
    String ptmsi = "-";

    public TMSIandPTMSI(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "TMSI and P-TMSI";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String[] getLabels() {
        return new String[]{"TMSI", "P-TMSI"};
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        if (name.equals(MDMContent.MSG_ID_EM_MM_INFO_IND)) {
            tmsi = "";
            for (int i = 0; i < 4; i++) {
                int value = getFieldValue(data, MDMContent.EM_MM_TMSI + "[" + i + "]");
                tmsi += Integer.toHexString(value) + " ";
            }
        } else if (name.equals(MDMContent.MSG_ID_EM_GMM_INFO_IND)) {
            ptmsi = "";
            for (int i = 0; i < 4; i++) {
                int value = getFieldValue(data, MDMContent.EM_MM_PTMSI + "[" + i + "]");
                ptmsi += Integer.toHexString(value) + " ";
            }
        }
        addData(tmsi, ptmsi);
    }
}

class LastRegisteredNetwork extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_NWSEL_PLMN_INFO_IND};

    public LastRegisteredNetwork(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Last Registered Network(s)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String[] getLabels() {
        return new String[]{"name", "value"};
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String rplmn = "";
        for (int i = 0; i < 6; i++) {
            int value = getFieldValue(data, MDMContent.EM_NWSEL_PLMN_MULTI_RPLMN + "[" + i + "]");
            rplmn += Integer.toHexString(value) + " ";
        }
        addData("rplmn", rplmn);
    }
}


class IntraFrequencyMonitoredSetUmtsFdd extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND};

    public IntraFrequencyMonitoredSetUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Monitored set Information(UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Monitored set Information: "};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int num_cells = getFieldValue(data, MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_CELLS);
        clearData();
        int num_monitored = 0;
        if (name.equals(MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND)) {
            String coName = MDMContent.FDD_EM_MEME_DCH_UMTS_UMTS_CELL_LIST + "[";
            for (int i = 0; i < num_cells; i++) {
                int cellType = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_MEME_DCH_UMTS_CELL_TYPE);
                Elog.d(TAG, "cellType = " + cellType);
                if (cellType == 1) {
                    int uarfcn = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_UARFCN);

                    int psc = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_PSC);

                    long rscp = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_RSCP, true);

                    long ecn0 = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_ECN0, true);

                    int lac = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_LAC);
                    int rac = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_RAC);
                    int cell_id = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_CELL_IDENTITY);
                    int num_plmn_id = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_PLMN_ID);

                    String[] plmn_a = new String[num_plmn_id];
                    for (int j = 0; j < num_plmn_id; j++) {
                        String secName = coName + i + "]."
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_PLMN_ID_LIST + "[" + j + "].";
                        int value = 0;
                        String plmn = "";
                        value = getFieldValue(data, secName
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_MCC);
                        plmn += value;
                        value = getFieldValue(data, secName
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_MNC);
                        plmn += String.format("%02d", value);
                        plmn_a[j] = plmn;
                    }
                    int num_ura_id = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_URA_ID);
                    int[] ura = new int[num_ura_id];
                    for (int j = 0; j < num_ura_id; j++) {
                        String secName = coName + i + "]."
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_URAIDENTITY + "[" + j + "].";
                        int stringData0 = getFieldValue(data, secName
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_STRINGDATA + "[0]");
                        int stringData1 = getFieldValue(data, secName
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_STRINGDATA + "[1]");
                        Elog.d(TAG, "stringData0 = " + stringData0);
                        Elog.d(TAG, "stringData1 = " + stringData1);
                        ura[j] = (stringData0 << 0x8) | stringData1;
                    }
                    addData("ActiveSetUmtsFdd(" + num_monitored + "): ");
                    num_monitored += 1;
                    addData("uarfcn", "psc", "plmn", "lac", "rac");
                    addData(uarfcn, psc, Arrays.toString(plmn_a), lac, rac);
                    addData("ura", "cell_id", "rscp", "ecn0", "-");
                    addData(Arrays.toString(ura), cell_id, rscp, ecn0, "-");
                }
            }
        }
    }
}

class IntraFrequencyDetectedSetUmtsFdd extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND};

    public IntraFrequencyDetectedSetUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Intra-frequency detected set (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"UARFCN", "PSC", "RSCP", "ECN0"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.FDD_EM_MEME_DCH_UMTS_UMTS_CELL_LIST + "[";
        int num = getFieldValue(data, MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_CELLS);
        clearData();
        for (int i = 0; i < num && i < 32; i++) {
            int cellType = getFieldValue(data, coName + i + "]."
                    + MDMContent.FDD_EM_MEME_DCH_UMTS_CELL_TYPE);
            if (cellType == 2) {
                int urafcn = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_MEME_DCH_UMTS_UARFCN);
                int psc = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_MEME_DCH_UMTS_PSC);
                long rscp = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_MEME_DCH_UMTS_RSCP, true);
                long ecn0 = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_MEME_DCH_UMTS_ECN0, true);
                addData(urafcn, psc, rscp, ecn0);
            }
        }
    }
}

class ActiveSetUmtsFdd extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND};


    public ActiveSetUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Active Set Information (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Active Set Information: "};
    }


    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int num_cells = getFieldValue(data, MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_CELLS);
        clearData();
        int num_active = 0;
        if (name.equals(MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND)) {
            String coName = MDMContent.FDD_EM_MEME_DCH_UMTS_UMTS_CELL_LIST + "[";
            for (int i = 0; i < num_cells; i++) {
                int cellType = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_MEME_DCH_UMTS_CELL_TYPE);
                Elog.d(TAG, "cellType = " + cellType);
                if (cellType == 0) {
                    int uarfcn = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_UARFCN);

                    int psc = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_PSC);

                    long rscp = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_RSCP, true);

                    long ecn0 = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_ECN0, true);

                    int lac = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_LAC);
                    int rac = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_RAC);
                    int cell_id = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_CELL_IDENTITY);
                    int num_plmn_id = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_PLMN_ID);

                    String[] plmn_a = new String[num_plmn_id];
                    for (int j = 0; j < num_plmn_id; j++) {
                        String secName = coName + i + "]."
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_PLMN_ID_LIST + "[" + j + "].";
                        int value = 0;
                        String plmn = "";
                        value = getFieldValue(data, secName
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_MCC);
                        plmn += value;
                        value = getFieldValue(data, secName
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_MNC);
                        plmn += String.format("%02d", value);
                        plmn_a[j] = plmn;
                    }
                    int num_ura_id = getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_URA_ID);
                    int[] ura = new int[num_ura_id];
                    for (int j = 0; j < num_ura_id; j++) {
                        String secName = coName + i + "]."
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_URAIDENTITY + "[" + j + "].";
                        int stringData0 = getFieldValue(data, secName
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_STRINGDATA + "[0]");
                        int stringData1 = getFieldValue(data, secName
                                + MDMContent.FDD_EM_MEME_DCH_UMTS_STRINGDATA + "[1]");
                        Elog.d(TAG, "stringData0 = " + stringData0);
                        Elog.d(TAG, "stringData1 = " + stringData1);
                        ura[j] = (stringData0 << 0x8) | stringData1;
                    }
                    addData("ActiveSetUmtsFdd(" + num_active + "): ");
                    num_active += 1;
                    addData("uarfcn", "psc", "plmn", "lac", "rac");
                    addData(uarfcn, psc, Arrays.toString(plmn_a), lac, rac);
                    addData("ura", "cell_id", "rscp", "ecn0", "-");
                    addData(Arrays.toString(ura), cell_id, rscp, ecn0, "-");
                }
            }
        }
    }
}


class CsOverHspaUmtsFdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRCE_CS_OVER_HSPA_STATUS_IND};

    HashMap<Boolean, String> mMapping = new HashMap<Boolean, String>() {
        {
            put(false, "OFF");
            put(true, "ON");
        }
    };

    public CsOverHspaUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "CS over HSPA (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"CS over HSPA"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String status = mMapping.get(getFieldValue(data,
                MDMContent.EM_RRCE_CS_OVER_HSPA_CS_OVER_HSPA_STATUS));
        clearData();
        addData(status == null ? "FALSE" : status);
    }
}

class ShareNetworkPlmnInfo extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_NWSEL_PLMN_INFO_IND};

    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "CELL_TYPE_SUITABLE");
            put(1, "CELL_TYPE_ACCEPTABLE");
            put(2, "CELL_TYPE_CAMPED_NOT_ALLOWED");
            put(3, "CELL_TYPE_NOT_APPLICABLE");
        }
    };

    public ShareNetworkPlmnInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Share Network PLMN Info";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"", "MCC1", "MCC2", "MCC3", "MNC1", "MNC2",
                "MNC3", "CELL TYPE"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int count = getFieldValue(data,
                MDMContent.EM_NWSEL_PLMN_MULTI_PLMN_COUNT);
        String coName = MDMContent.EM_NWSEL_PLMN_MULTI_PLMN_ID + "[";
        clearData();
        for (int i = 0; i < count && i < 6; i++) {
            addData("PLMN" + (i + 1),
                    String.format(
                            "%X",
                            getFieldValue(
                                    data,
                                    coName
                                            + i
                                            + "]."
                                            + MDMContent.EM_NWSEL_PLMN_MULTI_PLMN_ID_MCC
                                            + 1)),
                    String.format(
                            "%X",
                            getFieldValue(
                                    data,
                                    coName
                                            + i
                                            + "]."
                                            + MDMContent.EM_NWSEL_PLMN_MULTI_PLMN_ID_MCC
                                            + 2)),
                    String.format(
                            "%X",
                            getFieldValue(
                                    data,
                                    coName
                                            + i
                                            + "]."
                                            + MDMContent.EM_NWSEL_PLMN_MULTI_PLMN_ID_MCC
                                            + 3)),
                    String.format(
                            "%X",
                            getFieldValue(
                                    data,
                                    coName
                                            + i
                                            + "]."
                                            + MDMContent.EM_NWSEL_PLMN_MULTI_PLMN_ID_MNC
                                            + 1)),
                    String.format(
                            "%X",
                            getFieldValue(
                                    data,
                                    coName
                                            + i
                                            + "]."
                                            + MDMContent.EM_NWSEL_PLMN_MULTI_PLMN_ID_MNC
                                            + 2)),
                    String.format(
                            "%X",
                            getFieldValue(
                                    data,
                                    coName
                                            + i
                                            + "]."
                                            + MDMContent.EM_NWSEL_PLMN_MULTI_PLMN_ID_MNC
                                            + 3)),
                    mMapping.get(getFieldValue(data, coName + i + "]."
                            + MDMContent.EM_NWSEL_PLMN_MULTI_PLMN_ID_CELL_TYPE)));
        }
    }
}


class ServingCellInformationUmtsFdd extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_CSCE_SERV_CELL_S_STATUS_IND};

    public ServingCellInformationUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Serving Cell Information (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Item", "value"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.FDD_EM_CSCE_SERV_CELL + ".";
        clearData();
        int uarfcn = getFieldValue(data, coName + MDMContent.FDD_EM_CSCE_SERV_CELL_UARFCN_DL);
        int psc = getFieldValue(data, coName + MDMContent.FDD_EM_CSCE_SERV_CELL_PSC);

        int lac_valid = getFieldValue(data, coName + MDMContent.FDD_EM_CSCE_SERV_LAC_VALID);
        int lac = getFieldValue(data, coName + MDMContent.FDD_EM_CSCE_SERV_LAC);
        int rac_valid = getFieldValue(data, coName + MDMContent.FDD_EM_CSCE_SERV_RAC_VALID);
        int rac = getFieldValue(data, coName + MDMContent.FDD_EM_CSCE_SERV_RAC);
        int cell_id = getFieldValue(data, coName + MDMContent.FDD_EM_CSCE_SERV_CELL_IDENTITY);

        float rscp = (float) getFieldValue(data, coName
                + MDMContent.FDD_EM_CSCE_SERV_CELL_RSCP, true) / 4096;
        float ecn0 = (float) getFieldValue(data, coName
                + MDMContent.FDD_EM_CSCE_SERV_CELL_EC_N0, true) / 4096;

        addData("UARFCN", uarfcn);
        addData("PSC", psc);

        int value = 0;
        int count = getFieldValue(data, coName
                + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_COUNT);
        for (int i = 0; i < count; i++) {
            String secName = coName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID + "[" + i + "].";
            String plmn = "";
            value = getFieldValue(data, secName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MCC + 1);
            plmn += value;
            value = getFieldValue(data, secName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MCC + 2);
            plmn += value;
            value = getFieldValue(data, secName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MCC + 3);
            plmn += value;
            value = getFieldValue(data, secName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MNC + 1);
            plmn += value;
            value = getFieldValue(data, secName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MNC + 2);
            plmn += value;
            value = getFieldValue(data, secName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_MULTI_PLMN_ID_MNC + 3);
            plmn += (value == 0xF) ? "" : value;
            addData("PLMN[" + i + "]", plmn);
        }
        addData("LAC", (lac_valid == 0) ? "-" : lac);
        addData("RAC", (rac_valid == 0) ? "-" : rac);

        int num_ura_id = getFieldValue(data, coName + MDMContent.FDD_EM_CSCE_SERV_NUM_URA_ID);
        for (int i = 0; i < num_ura_id; i++) {
            String secName = coName + MDMContent.FDD_EM_CSCE_SERV_URAIDENTITY + "[" + i + "].";
            int stringData0 = getFieldValue(data, secName
                    + MDMContent.FDD_EM_CSCE_SERV_STRINGDATA + "[0]");
            int stringData1 = getFieldValue(data, secName
                    + MDMContent.FDD_EM_CSCE_SERV_STRINGDATA + "[1]");
            int stringData = (stringData0 << 0x8) | stringData1;
            addData("URA[" + i + "]", stringData);
        }

        addData("Cell ID", cell_id);
        addData("RSCP_CPICH Power Level", rscp);
        addData("Ec/Io", ecn0);
    }
}

class PrimaryCellCqi extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellCqi(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell CQI";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[2];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "CQI_CW0";

        configs[1] = new CurveView.Config();
        configs[1].color = 0xFF00FF00;
        configs[1].lineWidth = 3;
        configs[1].lineType = CurveView.Config.LINE_DASH;
        configs[1].nodeType = CurveView.Config.TYPE_CROSS;
        configs[1].name = "CQI_CW1";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int cw0 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_CQI_CW0, true);
        int cw1 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_CQI_CW1, true);
        addData(0, cw0);
        addData(0, cw1);
    }
}

class PrimaryCellDlImcs extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellDlImcs(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell DL Imcs";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DL_Imcs"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_IMCS, true);
        clearData();
        addData(value);
    }
}

class PrimaryCellDlResourceBlock extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellDlResourceBlock(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell DL Resource Block";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "DL Resource Block";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RB, true);
        addData(0, value);
    }
}

class PrimaryCellAntennaPort extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellAntennaPort(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell Antenna Port";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"ANT_PORT"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[0].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_ANT_PORT);
        clearData();
        addData(value);
    }
}

class PrimaryCellDlThroughput extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellDlThroughput(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell DL Throughput";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "DL Throughput (0.0)";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = 0;
        yConfig.max = 200;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        yConfig.type = CurveView.AxisConfig.TYPE_AUTO_SCALE;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_TPUT);
        Elog.d(TAG, "[MDMComponent ][PrimaryCellDlThroughput][update] name: "
                + name + " value : " + value);
        addData(0, value);
    }
}

class PrimaryCellDlBlockRate extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellDlBlockRate(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell DL Block Rate";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "DL Resource Block";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_BLOCK, true);
        addData(0, value);
    }
}

class PrimaryCellRsrpRx extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellRsrpRx(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell RSRP RX 0/1";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSRP[0]", "RSRP[1]"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int iRsrpRx0 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSRP + "[0]", true);
        int iRsrpRx1 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSRP + "[1]", true);
        clearData();
        addData(iRsrpRx0);
        addData(iRsrpRx1);
    }
}

class PrimaryCellRssiRx extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellRssiRx(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell RSSI RX 0/1";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSSI[0]", "RSSI[1]"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int iRssiRx0 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSSI + "[0]", true);
        int iRssiRx1 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSSI + "[1]", true);
        clearData();
        addData(iRssiRx0);
        addData(iRssiRx1);
    }
}

class PrimaryCellRsrqRx extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellRsrqRx(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell RSRQ RX 0/1";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSRQ[0]", "RSRQ[1]"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int iRsrqRx0 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSRQ + "[0]", true);
        int iRsrqRx1 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSRQ + "[1]", true);
        clearData();
        addData(iRsrqRx0);
        addData(iRsrqRx1);
    }
}

class PrimaryCellSinr extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellSinr(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell SINR";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"SINR"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int iSinr = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_SINR, true);
        clearData();
        addData(iSinr);
    }
}

class PrimaryCellRsrp extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellRsrp(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell RSRP";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSRP"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int iRsrp = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_RSRP, true);
        clearData();
        addData(iRsrp);
    }
}

class PrimaryCellRsrq extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellRsrq(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell RSRQ";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSRQ"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int iRsrq = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_RSRQ, true);
        clearData();
        addData(iRsrq);
    }
}

class PrimaryCellSnrRx extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellSnrRx(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell SNR RX 0/1";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"SINR[0]", "SINR[1]"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0]."
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_SINR + "[";
        int snr0 = getFieldValue(data, coName + 0 + "]", true);
        int snr1 = getFieldValue(data, coName + 1 + "]", true);
        clearData();
        addData(snr0);
        addData(snr1);
    }
}

class PrimaryCellOsSnr extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public PrimaryCellOsSnr(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Primary Cell OS SNR";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"SINR"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[0].";
        int osSnr = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_SINR, true);
        clearData();
        addData(osSnr);
    }
}

class SecondaryCellRsrpRx extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellRsrpRx(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell RSRP RX 0/1";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSRP[0]", "RSRP[1]"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int rsrp0 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSRP + "[0]", true);
        int rsrp1 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSRP + "[1]", true);
        clearData();
        addData(rsrp0);
        addData(rsrp1);
    }
}

class SecondaryCellRsrp extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellRsrp(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell RSRP";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSRP"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int iRsrp = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_RSRP, true);
        clearData();
        addData(iRsrp);
    }
}

class SecondaryCellRsrq extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellRsrq(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell RSRQ";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSRQ"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int iRsrq = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_RSRQ, true);
        clearData();
        addData(iRsrq);
    }
}

class SecondaryCellRssiRx extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellRssiRx(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell RSSI RX 0/1";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSSI[0]", "RSSI[1]"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int iRssiRx0 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSSI + "[0]", true);
        int iRssiRx1 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RSSI + "[1]", true);
        clearData();
        addData(iRssiRx0);
        addData(iRssiRx1);
    }
}

class SecondaryCellSnrRx extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellSnrRx(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell SNR RX 0/1";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"SINR[0]", "SINR[1]"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1]."
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_SINR + "[";
        int snr0 = getFieldValue(data, coName + 0 + "]", true);
        int snr1 = getFieldValue(data, coName + 1 + "]", true);
        clearData();
        addData(snr0);
        addData(snr1);
    }
}

class SecondaryCellOsSnr extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellOsSnr(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell OS SNR";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"OS SNR"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int osSnr = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_SINR, true);
        clearData();
        addData(osSnr);
    }
}

class SecondaryCellCqi extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellCqi(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell CQI";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[2];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "CQI_CW0";

        configs[1] = new CurveView.Config();
        configs[1].color = 0xFF00FF00;
        configs[1].lineWidth = 3;
        configs[1].lineType = CurveView.Config.LINE_DASH;
        configs[1].nodeType = CurveView.Config.TYPE_CROSS;
        configs[1].name = "CQI_CW1";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int cw0 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_CQI_CW0, true);
        int cw1 = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_CQI_CW1, true);
        addData(0, cw0);
        addData(0, cw1);
    }
}

class SecondaryCellDlImcs extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellDlImcs(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell DL Imcs";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DL_Imcs"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_IMCS, true);
        clearData();
        addData(value);
    }
}

class SecondaryCellDlResourceBlock extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellDlResourceBlock(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell DL Resource Block";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "DL Resource Block";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RB, true);
        addData(0, value);
    }
}

class SecondaryCellAntennaPort extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellAntennaPort(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell Antenna Port";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"ANT_PORT"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[1].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_CELL_INFO_ANT_PORT, true);
        clearData();
        addData(value);
    }
}

class SecondaryCellDlThroughput extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellDlThroughput(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell DL Throughput";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "DL Throughput (0.0)";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = 0;
        yConfig.max = 200;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        yConfig.type = CurveView.AxisConfig.TYPE_AUTO_SCALE;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_TPUT);
        addData(0, value);
    }
}

class SecondaryCellDlBlockRate extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public SecondaryCellDlBlockRate(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell DL Block Rate";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "DL Resource Block";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO + "[1].";
        int value = getFieldValue(data, coName
                + MDMContent.EM_EL1_STATUS_DL_INFO_DL_BLOCK, true);
        addData(0, value);
    }
}

class DownlinkDataThroughput extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_UPCM_PS_TPUT_INFO_IND};

    public DownlinkDataThroughput(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Downlink Data Throughput";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"IP Rate DL (bytes/s)"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        long value = getFieldValue(data,
                MDMContent.EM_UPCM_PS_TPUT_TOTAL_RX_BYTE_PER_SECOND);
        clearData();
        addData(value);
    }
}

class UplinkDataThroughput extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_UPCM_PS_TPUT_INFO_IND};

    public UplinkDataThroughput(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Uplink Data Throughput";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"IP Rate UL (bytes/s)"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        long value = getFieldValue(data,
                MDMContent.EM_UPCM_PS_TPUT_TOTAL_TX_BYTE_PER_SECOND);
        clearData();
        addData(value);
    }
}

class CellStrength extends CurveExComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    public CellStrength(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "CC0/CC1 RSRP and SINR";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveViewEx.AxisConfig configX() {
        mXLabel.setText("RSRP");

        CurveViewEx.AxisConfig xConfig = new CurveViewEx.AxisConfig();
        xConfig.min = -140;
        xConfig.max = -30;
        xConfig.step = 10;
        xConfig.configMin = true;
        xConfig.configMax = true;
        xConfig.configStep = true;
        return xConfig;
    }

    @Override
    CurveViewEx.AxisConfig configY() {
        mYLabel.setText("SNR");
        CurveViewEx.Config[] configs = new CurveViewEx.Config[5];
        configs[0] = new CurveViewEx.Config();
        configs[0].color = 0xFFFF0000;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveViewEx.Config.TYPE_CIRCLE;
        configs[0].name = "CC0";
        configs[1] = new CurveViewEx.Config();
        configs[1].color = 0xFF0000FF;
        configs[1].lineWidth = 3;
        configs[1].nodeType = CurveViewEx.Config.TYPE_TRIANGLE;
        configs[1].name = "CC1";
        configs[2] = new CurveViewEx.Config();
        configs[2].color = Color.rgb(43, 101, 171);
        configs[2].lineWidth = 3;
        configs[2].nodeType = CurveViewEx.Config.TYPE_NONE;
        configs[2].name = "Strong";
        configs[3] = new CurveViewEx.Config();
        configs[3].color = Color.rgb(204, 153, 0);
        configs[3].lineWidth = 3;
        configs[3].nodeType = CurveViewEx.Config.TYPE_NONE;
        configs[3].name = "MediumWeak";
        configs[4] = new CurveViewEx.Config();
        configs[4].color = Color.rgb(152, 152, 186);
        configs[4].lineWidth = 3;
        configs[4].nodeType = CurveViewEx.Config.TYPE_NONE;
        configs[4].name = "Weak";
        mCurveView.setConfig(configs);

        CurveViewEx.AxisConfig yConfig = new CurveViewEx.AxisConfig();
        yConfig.min = -20;
        yConfig.max = 30;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.EM_EL1_STATUS_DL_INFO;
        String coName1 = MDMContent.EM_EL1_STATUS_CELL_INFO;
        int pcellRsrp = getFieldValue(data, coName + "[0]."
                + MDMContent.EM_EL1_STATUS_DL_INFO_RSRP, true);
        int pcellSinr = getFieldValue(data, coName + "[0]."
                + MDMContent.EM_EL1_STATUS_DL_INFO_SINR, true);
        int pcellPci = getFieldValue(data, coName1 + "[0]." + MDMContent.PCI,
                true);
        long pcellEarfcn = getFieldValue(data, coName1 + "[0]."
                + MDMContent.EARFCN);
        int scellRsrp = getFieldValue(data, coName + "[1]."
                + MDMContent.EM_EL1_STATUS_DL_INFO_RSRP, true);
        int scellSinr = getFieldValue(data, coName + "[1]."
                + MDMContent.EM_EL1_STATUS_DL_INFO_SINR, true);
        int scellPci = getFieldValue(data, coName1 + "[1]." + MDMContent.PCI,
                true);
        long scellEarfcn = getFieldValue(data, coName1 + "[1]."
                + MDMContent.EARFCN);
        clearData();
        Elog.d(TAG, "pcellRsrp" + pcellRsrp + " pcellSinr" + pcellSinr
                + " pcellPci" + pcellPci + " pcellEarfcn" + pcellEarfcn
                + " scellRsrp" + scellRsrp + " scellSinr" + scellSinr
                + " scellPci" + scellPci + " scellEarfcn" + scellEarfcn);
        addData(0, pcellRsrp, pcellSinr, pcellPci, pcellEarfcn);
        addData(1, scellRsrp, scellSinr, scellPci, scellEarfcn);
    }
}

class HandoverIntraLte extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_SUCCESS_RATE_KPI_IND};
    String StartTime = null;

    boolean isFirstTimeRecord = true;
    String FileNamePS = "_Handover_Intra_LTE.txt";
    String title = "Time,success,procId,status";
    String FileNamePS1 = null;
    String FileNamePS2 = null;
    int[] success = new int[3];
    int[] procId = new int[3];
    int[] status = new int[3];
    int[] attempt = new int[3];
    String[] event = new String[3];

    public HandoverIntraLte(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Handover (Intra-LTE)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Attempt", "Success", "Fail"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    void LogRecord(int sim_idx) {
        String CurTime = null;

        String[] Content = new String[2];
        String path = "/Download";

        if (isFirstTimeRecord == true) {
            isFirstTimeRecord = false;
            StartTime = getCurrectTime();
            try {
                FileNamePS1 = StartTime + "_ps_1_"
                        + MDMComponentDetailActivity.mSimMccMnc[0] + FileNamePS;
                FileNamePS2 = StartTime + "_ps_2_"
                        + MDMComponentDetailActivity.mSimMccMnc[1] + FileNamePS;
                saveToSDCard(path, FileNamePS1, title, false);
                saveToSDCard(path, FileNamePS2, title, false);
            } catch (Exception e) {
                e.printStackTrace();
            }
            Elog.d(TAG, "isFirstTimeRecord = " + isFirstTimeRecord + "," + title);
        }
        CurTime = getCurrectTime();
        Content[sim_idx] = CurTime + "," + success[sim_idx] + "," + procId[sim_idx] +
                "," + status[sim_idx];

        try {
            if (sim_idx == 0) {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS1);
                saveToSDCard(path, FileNamePS1, Content[sim_idx], true);
            } else {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS2);
                saveToSDCard(path, FileNamePS2, Content[sim_idx], true);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int sim_idx = data.getSimIdx() - 1;

        if (MDMContent.MSG_ID_EM_ERRC_SUCCESS_RATE_KPI_IND.equals(name)) {
            attempt[sim_idx] = getFieldValue(data, MDMContent.EM_ERRC_SUCCESS_RATE_KPI_ATTEMPT);
            success[sim_idx] = getFieldValue(data, MDMContent.EM_ERRC_SUCCESS_RATE_KPI_SUCCESS);
            procId[sim_idx] = getFieldValue(data, MDMContent.EM_ERRC_SUCCESS_RATE_KPI_PROC_ID);
            status[sim_idx] = getFieldValue(data, MDMContent.EM_ERRC_SUCCESS_RATE_KPI_STATUS);
            Elog.d(TAG, "HandoverIntraLte,success = " + success[sim_idx]);
        }
        if (ComponentSelectActivity.mAutoRecordFlag.equals("1") && (sim_idx < 2)) {
            LogRecord(sim_idx);
        }
        if ((sim_idx + 1) != MDMComponentDetailActivity.mModemType) {
            return;
        }
        clearData();
        addData(attempt[sim_idx], success[sim_idx], attempt[sim_idx] -
                success[sim_idx]);
    }
}

class RatAndServiceStatus extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RAC_INFO_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "NO SERVICE");
            put(1, "LIMITED SERVICE");
            put(2, "GSM");
            put(3, "UMTS FDD");
            put(4, "UMTS TDD");
            put(5, "LTE  FDD");
            put(6, "LTE  TDD");
        }
    };

    public RatAndServiceStatus(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "RAT and Service Status";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[1];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "Service status";
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = 0;
        yConfig.max = 7;
        yConfig.step = 1;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        yConfig.customLabel = mMapping;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        addData(0, getFieldValue(data, MDMContent.EM_RAC_ACTIVE_RAT_INFO));
    }
}

class HsDschServingCellUmtsFdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_MEME_DCH_H_SERVING_CELL_INFO_IND};

    public HsDschServingCellUmtsFdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "HS-DSCH Serving cell (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"HS-DSCH Serving Cell"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int uarfcn = getFieldValue(data,
                MDMContent.FDD_EM_MEME_DCH_H_SERVING_HSDSCH_UARFCN);
        int psc = getFieldValue(data,
                MDMContent.FDD_EM_MEME_DCH_H_SERVING_HSDSCH_PSC);
        String value = "";
        value += (uarfcn == 0xFFFF ? "-" : uarfcn);
        value += " / " + (psc == 0xFFFF ? "-" : psc);
        clearData();
        addData(value);
    }
}

class SecondHsDschServingCell extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_UL1_HSPA_INFO_GROUP_IND};

    public SecondHsDschServingCell(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "2nd HS-DSCH Serving Cell";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Dual Cell", "Freq", "PSC", "64 QAM"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.FDD_EM_UL1_HSPA_PRIMARY_SEC_HS_DSCH_CONFIG_STATUS
                + ".";
        int dcOn = getFieldValue(
                data,
                coName
                        + MDMContent.FDD_EM_UL1_HSPA_PRIMARY_SEC_HS_DSCH_CONFIG_STATUS_DC_ON);
        int freq = getFieldValue(
                data,
                coName
                        + MDMContent.FDD_EM_UL1_HSPA_PRIMARY_SEC_HS_DSCH_CONFIG_STATUS_DL_FREQ);
        int psc = getFieldValue(
                data,
                coName
                        + MDMContent.FDD_EM_UL1_HSPA_PRIMARY_SEC_HS_DSCH_CONFIG_STATUS_PSC);
        int dlOn = getFieldValue(
                data,
                coName
                        + MDMContent.FDD_EM_UL1_HSPA_PRIMARY_SEC_HS_DSCH_CONFIG_STATUS_DL_64QAM_ON);
        clearData();
        addData((dcOn > 0) ? 1 : 0);
        addData(freq);
        addData(psc);
        addData((dlOn > 0) ? 1 : 0);
    }
}

class BasicInfoServingGsm extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_RRM_MEASUREMENT_REPORT_INFO_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "PGSM");
            put(1, "EGSM");
            put(2, "RGSM");
            put(3, "DCS1800");
            put(4, "PCS1900");
            put(5, "GSM450");
            put(6, "GSM480");
            put(7, "GSM850");
        }
    };
    HashMap<Integer, String> mGprsMapping = new HashMap<Integer, String>() {
        {
            put(0, "PGSM");
            put(1, "EGSM");
        }
    };
    HashMap<Integer, String> mPbcchMapping = new HashMap<Integer, String>() {
        {
            put(0, "PGSM");
            put(1, "EGSM");
        }
    };

    public BasicInfoServingGsm(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Basic Info (Serving) (GSM)";
    }

    @Override
    String getGroup() {
        return "2. GSM EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Band", "ARFCN", "BSIC", "GPRS supported",
                "PBCCH present"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.RR_EM_MEASUREMENT_REPORT_INFO + ".";
        int rrState = getFieldValue(data, coName
                + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_RR_STATE);
        clearData();
        if (rrState >= 3 && rrState <= 7) {
            int band = getFieldValue(
                    data,
                    coName
                            + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERVING_CURRENT_BAND);
            int arfcn = getFieldValue(data, coName
                    + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERVING_ARFCN);
            int bsic = getFieldValue(data, coName
                    + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERVING_BSIC);
            int gprs = getFieldValue(
                    data,
                    coName
                            + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_GPRS_SUPPORTED);
            int pbcch = getFieldValue(
                    data,
                    coName
                            + MDMContent.RR_EM_MEASUREMENT_REPORT_INFO_SERV_GPRS_PBCCH_PRESENT);
            addData(mMapping.get(band));
            addData(arfcn);
            addData(bsic);
            addData(mGprsMapping.get(gprs));
            addData(mPbcchMapping.get(pbcch));
        }
        notifyDataSetChanged();
    }
}

class RsrpLteCandidateCellUmtsFddArray extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND};

    public RsrpLteCandidateCellUmtsFddArray(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "RSRP (LTE candidate cell)(UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSRP (LTE)", "Earfcn", "PCI", "RSRP"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int rat = getFieldValue(data,
                MDMContent.FDD_EM_CSCE_NEIGH_CELL_RAT_TYPE);
        if (rat == 2) {
            int num = getFieldValue(data,
                    MDMContent.FDD_EM_CSCE_NEIGH_CELL_COUNT);
            String coName = MDMContent.FDD_EM_CSCE_NEIGH_CELL_CHOICE + "."
                    + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_NEIGH_CELL + "[";
            for (int i = 0; i < num && i < 16; i++) {
                int earfcn = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_EARFCN);
                int pci = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_PCI);
                float rsrp = (float) getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_RSRP, true) / 4096;
                addData(earfcn, pci, rsrp);
            }
        }
    }
}

class RsrpLteCandidateCellUmtsFddCurve extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND};

    public RsrpLteCandidateCellUmtsFddCurve(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "RSRP (LTE candidate cell)(UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        int[] colors = new int[]{0xFF0000FF, 0xFF00FF00, 0xFFFF0000,
                0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFF000088, 0xFF008800,
                0xFF880000, 0xFF008888, 0xFF880088, 0xFF888800, 0xFF000044,
                0xFF004400, 0xFF440000, 0xFF000000,};
        CurveView.Config[] configs = new CurveView.Config[16];
        for (int i = 0; i < 16; i++) {
            configs[i] = new CurveView.Config();
            configs[i].color = colors[i];
            configs[i].lineWidth = 3;
            configs[i].nodeType = CurveView.Config.TYPE_CIRCLE;
            configs[i].name = "";
        }
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int rat = getFieldValue(data,
                MDMContent.FDD_EM_CSCE_NEIGH_CELL_RAT_TYPE);
        if (rat == 2) {
            int num = getFieldValue(data,
                    MDMContent.FDD_EM_CSCE_NEIGH_CELL_COUNT);
            String coName = MDMContent.FDD_EM_CSCE_NEIGH_CELL_CHOICE + "."
                    + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_NEIGH_CELL + "[";
            for (int i = 0; i < num && i < 16; i++) {
                float rsrp = (float) getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_RSRP, true) / 4096;
                addData(i, rsrp);
            }
        }
    }
}

class RsrpLteCandidateCellUmtsFdd extends MDMComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND};
    private RsrpLteCandidateCellUmtsFddArray mArrayComponent;
    private RsrpLteCandidateCellUmtsFddCurve mCurveComponent;
    private LinearLayout mComponentView;

    public RsrpLteCandidateCellUmtsFdd(Activity context) {
        super(context);
        mArrayComponent = new RsrpLteCandidateCellUmtsFddArray(context);
        mCurveComponent = new RsrpLteCandidateCellUmtsFddCurve(context);
    }

    @Override
    View getView() {
        if (mComponentView == null) {
            mComponentView = new LinearLayout(mActivity);
            mComponentView.setOrientation(LinearLayout.VERTICAL);
        }
        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, 0, 1);
        LinearLayout.LayoutParams layoutParams2 = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, 0, 2);
        mComponentView.addView(mArrayComponent.getView(), layoutParams);
        mComponentView.addView(mCurveComponent.getView(), layoutParams2);
        return mComponentView;
    }

    @Override
    void removeView() {
        mArrayComponent.removeView();
        mCurveComponent.removeView();
        mComponentView.removeAllViews();
    }

    void clearData() {
        ;
    }

    @Override
    String getName() {
        return "RSRP (LTE candidate cell)(UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        mArrayComponent.update(name, msg);
        mCurveComponent.update(name, msg);
    }
}

class RsrqLteCandidateCellUmtsFddArray extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND};

    public RsrqLteCandidateCellUmtsFddArray(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "RSRQ (LTE candidate cell)(UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"RSRP (LTE)", "Earfcn", "PCI", "RSRQ"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int rat = getFieldValue(data,
                MDMContent.FDD_EM_CSCE_NEIGH_CELL_RAT_TYPE);
        String coName = MDMContent.FDD_EM_CSCE_NEIGH_CELL_CHOICE + "."
                + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_NEIGH_CELL + "[";
        if (rat == 2) {
            int num = getFieldValue(data,
                    MDMContent.FDD_EM_CSCE_NEIGH_CELL_COUNT);
            for (int i = 0; i < num && i < 16; i++) {
                int earfcn = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_EARFCN);
                int pci = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_PCI);
                float rsrq = (float) getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_RSRQ, true) / 4096;
                addData(earfcn, pci, rsrq);
            }
        }
    }
}

class RsrqLteCandidateCellUmtsFddCurve extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND};

    public RsrqLteCandidateCellUmtsFddCurve(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "RSRQ (LTE candidate cell)(UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    void clearData() {
        ;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        int[] colors = new int[]{0xFF0000FF, 0xFF00FF00, 0xFFFF0000,
                0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFF000088, 0xFF008800,
                0xFF880000, 0xFF008888, 0xFF880088, 0xFF888800, 0xFF000044,
                0xFF004400, 0xFF440000, 0xFF000000,};
        CurveView.Config[] configs = new CurveView.Config[16];
        for (int i = 0; i < 16; i++) {
            configs[i] = new CurveView.Config();
            configs[i].color = colors[i];
            configs[i].lineWidth = 3;
            configs[i].nodeType = CurveView.Config.TYPE_CIRCLE;
            configs[i].name = "";
        }
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int rat = getFieldValue(data,
                MDMContent.FDD_EM_CSCE_NEIGH_CELL_RAT_TYPE);
        if (rat == 2) {
            int num = getFieldValue(data,
                    MDMContent.FDD_EM_CSCE_NEIGH_CELL_COUNT);
            String coName = MDMContent.FDD_EM_CSCE_NEIGH_CELL_CHOICE + "."
                    + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_NEIGH_CELL + "[";
            for (int i = 0; i < num && i < 16; i++) {
                float rsrq = (float) getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_LTE_RSRQ, true) / 4096;
                addData(i, rsrq);
            }
        }
    }
}

class RsrqLteCandidateCellUmtsFdd extends MDMComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND};
    private RsrqLteCandidateCellUmtsFddArray mArrayComponent;
    private RsrqLteCandidateCellUmtsFddCurve mCurveComponent;
    private LinearLayout mComponentView;

    public RsrqLteCandidateCellUmtsFdd(Activity context) {
        super(context);
        mArrayComponent = new RsrqLteCandidateCellUmtsFddArray(context);
        mCurveComponent = new RsrqLteCandidateCellUmtsFddCurve(context);
    }

    @Override
    View getView() {
        if (mComponentView == null) {
            mComponentView = new LinearLayout(mActivity);
            mComponentView.setOrientation(LinearLayout.VERTICAL);
        }
        LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, 0, 1);
        LinearLayout.LayoutParams layoutParams2 = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, 0, 2);
        mComponentView.addView(mArrayComponent.getView(), layoutParams);
        mComponentView.addView(mCurveComponent.getView(), layoutParams2);
        return mComponentView;
    }

    @Override
    void removeView() {
        mArrayComponent.removeView();
        mCurveComponent.removeView();
        mComponentView.removeAllViews();
    }

    @Override
    String getName() {
        return "RSRQ (LTE candidate cell)(UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    void clearData() {
        ;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        mArrayComponent.update(name, msg);
        mCurveComponent.update(name, msg);
    }
}

class UmtsRscp extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_CSCE_SERV_CELL_S_STATUS_IND,
            MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND,
            MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND};

    public UmtsRscp(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS RSCP (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[3];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "RSCP(active)";
        configs[0].newLineThreadshold = 11;
        configs[1] = new CurveView.Config();
        configs[1].color = 0xFF0000FF;
        configs[1].lineWidth = 0;
        configs[1].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[1].name = "RSCP(monitored)";
        configs[1].newLineThreadshold = 11;
        configs[2] = new CurveView.Config();
        configs[2].color = 0xFF0000FF;
        configs[2].lineWidth = 1;
        configs[2].lineType = CurveView.Config.LINE_DASH;
        configs[2].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[2].name = "RSCP(detected)";
        configs[2].newLineThreadshold = 11;
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_FDD_EM_CSCE_SERV_CELL_S_STATUS_IND)) {
            String coName = MDMContent.FDD_EM_CSCE_SERV_CELL + ".";
            float rscp = (float) getFieldValue(data, coName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_RSCP, true) / 4096;
            float ecno = (float) getFieldValue(data, coName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_EC_N0, true) / 4096;
            if (rscp > -120 && ecno > -25) {
                addData(0, rscp);
            }
        } else if (name
                .equals(MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND)) {
            int operation = getFieldValue(data,
                    MDMContent.FDD_EM_CSCE_NEIGH_CELL_OPERATION);
            int rat = getFieldValue(data,
                    MDMContent.FDD_EM_CSCE_NEIGH_CELL_RAT_TYPE);
            if (operation == 1 && rat == 0) {
                int num = getFieldValue(data,
                        MDMContent.FDD_EM_CSCE_NEIGH_CELL_COUNT);
                String coName = MDMContent.FDD_EM_CSCE_NEIGH_CELL_CHOICE + "."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_NEIGH_CELL + "[";
                for (int i = 0; i < num && i < 16; i++) {
                    float rscp = (float) getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_CSCE_NEIGH_CELL_RSCP, true) / 4096;
                    float ecno = (float) getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_CSCE_NEIGH_CELL_EC_N0, true) / 4096;
                    if (rscp > -120 && ecno > -25) {
                        addData(1, rscp);
                    }
                }
            }
        } else if (name
                .equals(MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND)) {
            int num = getFieldValue(data,
                    MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_CELLS);
            String coName = MDMContent.FDD_EM_MEME_DCH_UMTS_UMTS_CELL_LIST
                    + "[";
            for (int i = 0; i < num && i < 32; i++) {
                int cellType = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_MEME_DCH_UMTS_CELL_TYPE);
                int rscp = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_MEME_DCH_UMTS_RSCP, true);
                if (cellType == 0) {
                    addData(0, rscp);
                } else if (cellType == 1 || cellType == 3) {
                    addData(1, rscp);
                } else if (cellType == 2) {
                    addData(2, rscp);
                }
            }
        }
    }
}

class UmtsEcn0 extends CurveComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_CSCE_SERV_CELL_S_STATUS_IND,
            MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND,
            MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND};

    public UmtsEcn0(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UMTS ECN0 (UMTS FDD)";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    CurveView.AxisConfig configY() {
        // mYLabel.setText("RSCP (dBm)");
        mYLabel.setText("R");

        CurveView.Config[] configs = new CurveView.Config[3];
        configs[0] = new CurveView.Config();
        configs[0].color = 0xFF0000FF;
        configs[0].lineWidth = 3;
        configs[0].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[0].name = "EcN0(active)";
        configs[0].newLineThreadshold = 11;
        configs[1] = new CurveView.Config();
        configs[1].color = 0xFF0000FF;
        configs[1].lineWidth = 0;
        configs[1].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[1].name = "EcN0(monitored)";
        configs[1].newLineThreadshold = 11;
        configs[2] = new CurveView.Config();
        configs[2].color = 0xFF0000FF;
        configs[2].lineWidth = 1;
        configs[2].lineType = CurveView.Config.LINE_DASH;
        configs[2].nodeType = CurveView.Config.TYPE_CIRCLE;
        configs[2].name = "EcN0(detected)";
        configs[2].newLineThreadshold = 11;
        mCurveView.setConfig(configs);

        CurveView.AxisConfig yConfig = new CurveView.AxisConfig();
        yConfig.min = -100;
        yConfig.max = 100;
        yConfig.step = 10;
        yConfig.configMin = true;
        yConfig.configMax = true;
        yConfig.configStep = true;
        return yConfig;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_FDD_EM_CSCE_SERV_CELL_S_STATUS_IND)) {
            String coName = MDMContent.FDD_EM_CSCE_SERV_CELL + ".";
            float rscp = (float) getFieldValue(data, coName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_RSCP, true) / 4096;
            float ecn0 = (float) getFieldValue(data, coName
                    + MDMContent.FDD_EM_CSCE_SERV_CELL_EC_N0, true) / 4096;
            if (rscp > -120 && ecn0 > -25) {
                addData(0, ecn0);
            }
        } else if (name
                .equals(MDMContent.MSG_ID_FDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND)) {
            int operation = getFieldValue(data,
                    MDMContent.FDD_EM_CSCE_NEIGH_CELL_OPERATION);
            int rat = getFieldValue(data,
                    MDMContent.FDD_EM_CSCE_NEIGH_CELL_RAT_TYPE);
            if (operation == 1 && rat == 0) {
                int num = getFieldValue(data,
                        MDMContent.FDD_EM_CSCE_NEIGH_CELL_COUNT);
                String coName = MDMContent.FDD_EM_CSCE_NEIGH_CELL_CHOICE + "."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_NEIGH_CELL + "[";
                for (int i = 0; i < num && i < 16; i++) {
                    float rscp = (float) getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_CSCE_NEIGH_CELL_RSCP, true) / 4096;
                    float ecn0 = (float) getFieldValue(data, coName + i + "]."
                            + MDMContent.FDD_EM_CSCE_NEIGH_CELL_EC_N0, true) / 4096;
                    if (rscp > -120 && ecn0 > -25) {
                        addData(1, ecn0);
                    }
                }
            }
        } else if (name
                .equals(MDMContent.MSG_ID_FDD_EM_MEME_DCH_UMTS_CELL_INFO_IND)) {
            int num = getFieldValue(data,
                    MDMContent.FDD_EM_MEME_DCH_UMTS_NUM_CELLS);
            String coName = MDMContent.FDD_EM_MEME_DCH_UMTS_UMTS_CELL_LIST
                    + "[";
            for (int i = 0; i < num && i < 32; i++) {
                int cellType = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_MEME_DCH_UMTS_CELL_TYPE);
                int ecn0 = getFieldValue(data, coName + i + "]."
                        + MDMContent.FDD_EM_CSCE_NEIGH_CELL_EC_N0, true);
                if (cellType == 0) {
                    addData(0, ecn0);
                } else if (cellType == 1 || cellType == 3) {
                    addData(1, ecn0);
                } else if (cellType == 2) {
                    addData(2, ecn0);
                }
            }
        }
    }
}

class GeranNeighborCellInfoUmtsTdd extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_DCH_GSM_CELL_INFO_IND,
            MDMContent.MSG_ID_TDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND};
    HashMap<Integer, String> mMapping = new HashMap<Integer, String>() {
        {
            put(0, "dcs1800");
            put(1, "pcs1900");
        }
    };

    public GeranNeighborCellInfoUmtsTdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "GERAN Neighbor Cell Info (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"BSIC", "Frequency band", "BCCH ARFCN"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_TDD_EM_MEME_DCH_GSM_CELL_INFO_IND)) {
            String coName = MDMContent.EM_MEME_DCH_GSM_CELL_LIST;
            int num = getFieldValue(data, MDMContent.EM_MEME_DCH_GSM_NCELL_NUM);
            clearData();
            for (int i = 0; i < num && i < 6; i++) {
                int band = getFieldValue(data, coName + "[" + i + "]."
                        + MDMContent.EM_MEME_DCH_GSM_CELL_LIST_FREQUENCY_BAND);
                int arfcn = getFieldValue(data, coName + "[" + i + "]."
                        + MDMContent.EM_MEME_DCH_GSM_CELL_LIST_ARFCN);
                int bsic = getFieldValue(data, coName + "[" + i + "]."
                        + MDMContent.EM_MEME_DCH_GSM_CELL_LIST_BSIC);
                int bcc = bsic & 0x7;
                int ncc = (bsic >> 3) & 0x7;
                String bandString = mMapping.get(band);
                addData("bcc: " + bcc + " ncc: " + ncc,
                        bandString == null ? "" : bandString,
                        String.valueOf(arfcn));
            }
        } else if (name
                .equals(MDMContent.MSG_ID_TDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND)) {
            String coName = MDMContent.EM_CSCE_NEIGH_CELL_CHOICE + "."
                    + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS;
            int num = getFieldValue(data, MDMContent.EM_CSCE_NEIGH_CELL_COUNT);
            clearData();
            for (int i = 0; i < num && i < 16; i++) {
                int arfcn = getFieldValue(
                        data,
                        coName
                                + "["
                                + i
                                + "]."
                                + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS_ARFCN);
                int bsic = getFieldValue(
                        data,
                        coName
                                + "["
                                + i
                                + "]."
                                + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS_BSIC);
                int band = getFieldValue(
                        data,
                        coName
                                + "["
                                + i
                                + "]."
                                + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_GSM_NEIGH_CELLS_FREQ_BAND);
                int bcc = bsic & 0x7;
                int ncc = (bsic >> 3) & 0x7;
                String bandString = mMapping.get(band);
                addData("bcc: " + bcc + " ncc: " + ncc,
                        bandString == null ? "" : bandString,
                        String.valueOf(arfcn));
            }
        }
    }
}

class GeranNeighbouringCellInformation extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_IND};
    HashMap<Integer, String> bandMapping = new HashMap<Integer, String>() {
        {
            put(0, "DCS1800");
            put(1, "PCS1900");
            put(2, "-");
        }
    };

    public GeranNeighbouringCellInformation(Activity context) {
        super(context);
    }

    String getBandMapping(int bandidx) {
        String bandidx_s = "";
        if (bandidx == 0 || bandidx == 1)
            bandidx_s = bandMapping.get(bandidx);
        else
            bandidx_s = bandMapping.get(2) + "(" + bandidx + ")";
        return bandidx_s;
    }

    @Override
    String getName() {
        return "GERAN neighbouring cell information";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"BAND INDICATOR", "ARFCN", "BSIC", "RSSI"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String coName = MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_GCELL;
        int cellNum = getFieldValue(data,
                MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_TOTAL_GCELL_NUM);
        for (int i = 0; i < cellNum && i < 6; i++) {
            int valid = getFieldValue(data, coName + "[" + i + "]."
                    + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_VALID);
            int band = getFieldValue(data, coName + "[" + i + "]."
                    + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_BAND_IND);
            int arfcn = getFieldValue(data, coName + "[" + i + "]."
                    + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_ARFCN);
            int bsic = getFieldValue(data, coName + "[" + i + "]."
                    + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_BSIC);
            int rssi = getFieldValue(data, coName + "[" + i + "]."
                    + MDMContent.EM_ERRC_MOB_MEAS_INTERRAT_GERAN_INFO_RSSI);
            addData(((valid > 0) ? getBandMapping(band) : ""), ((valid > 0) ? arfcn : ""),
                    ((valid > 0) ? bsic : ""), ((valid > 0) ? (float) rssi / 4
                            : ""));
        }
    }
}

class UtraTddNeighbouringCellInformation extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTERRAT_UTRAN_INFO_IND};

    public UtraTddNeighbouringCellInformation(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "UTRA-TDD neighbouring cell information";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"UARFCN", "PSC", "RSCP", "EcN0"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        int freqNum = getFieldValue(data,
                MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_FREQ_NUM);
        Elog.d(TAG, " freqNum: " + freqNum);
        String coName = MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_INTER_FREQ
                + "[";
        for (int i = 0; i < freqNum && i < 16; i++) {
            int valid = getFieldValue(data, coName + i + "]."
                    + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_VALID);
            int uarfcn = getFieldValue(data, coName + i + "]."
                    + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_EARFCN);
            int cellNum = getFieldValue(data, coName + i + "]."
                    + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_NUM);
            String secName = coName + i + "]."
                    + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL + "[";
            for (int j = 0; j < cellNum && j < 6; j++) {
                int valid2 = getFieldValue(data, secName + j + "]."
                        + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_VALID);
                int psc = getFieldValue(data, secName + j + "]."
                        + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_PSC);
                int rscp = getFieldValue(data, secName + j + "]."
                        + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_RSCP);
                int ecn0 = getFieldValue(data, secName + j + "]."
                        + MDMContent.ERRC_MOB_MEAS_INTERRAT_UTRAN_UCELL_EC_N0);
                addData(((valid > 0) ? uarfcn : ""), ((valid2 > 0) ? psc : ""),
                        ((valid2 > 0) && rscp != 0xFFFFFFFF ? (float) rscp / 4
                                : ""),
                        ((valid2 > 0) && ecn0 != 0xFFFFFFFF ? (float) ecn0 / 4
                                : ""));
            }
        }
    }
}

class EUtranNeighborCellInfoUmtsTdd extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_TDD_EM_MEME_DCH_LTE_CELL_INFO_IND,
            MDMContent.MSG_ID_TDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND};

    public EUtranNeighborCellInfoUmtsTdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "E-UTRAN Neighbor Cell Info (UMTS TDD)";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"EARFCN", "PCI", "RSRP", "RSRQ"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_TDD_EM_MEME_DCH_LTE_CELL_INFO_IND)) {
            String coName = MDMContent.EM_MEME_DCH_LTE_CELL_LIST;
            int num = getFieldValue(data, MDMContent.EM_MEME_DCH_LTE_NUM_CELLS);
            clearData();
            for (int i = 0; i < num && i < 32; i++) {
                int arfcn = getFieldValue(data, coName + "[" + i + "]."
                        + MDMContent.EM_MEME_DCH_LTE_CELL_EARFCN);
                int pci = getFieldValue(data, coName + "[" + i + "]."
                        + MDMContent.EM_MEME_DCH_LTE_CELL_PCI);
                int rsrp = getFieldValue(data, coName + "[" + i + "]."
                        + MDMContent.EM_MEME_DCH_LTE_CELL_RSRP, true);
                int rsrq = getFieldValue(data, coName + "[" + i + "]."
                        + MDMContent.EM_MEME_DCH_LTE_CELL_RSRQ, true);
                addData(arfcn, pci, rsrp, rsrq);
            }
        } else if (name
                .equals(MDMContent.MSG_ID_TDD_EM_CSCE_NEIGH_CELL_S_STATUS_IND)) {
            String coName = MDMContent.EM_CSCE_NEIGH_CELL_CHOICE + "."
                    + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS;
            int num = getFieldValue(data, MDMContent.EM_CSCE_NEIGH_CELL_COUNT);
            int rat = getFieldValue(data,
                    MDMContent.EM_CSCE_NEIGH_CELL_RAT_TYPE);
            if (rat == 2) {
                clearData();
                for (int i = 0; i < num && i < 16; i++) {
                    int arfcn = getFieldValue(
                            data,
                            coName
                                    + "["
                                    + i
                                    + "]."
                                    + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS_EARFCN);
                    int pci = getFieldValue(
                            data,
                            coName
                                    + "["
                                    + i
                                    + "]."
                                    + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS_PCI);
                    int rsrp = getFieldValue(
                            data,
                            coName
                                    + "["
                                    + i
                                    + "]."
                                    + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS_RSRP,
                            true);
                    int rsrq = getFieldValue(
                            data,
                            coName
                                    + "["
                                    + i
                                    + "]."
                                    + MDMContent.EM_CSCE_NEIGH_CELL_CHOICE_LTE_NEIGH_CELLS_RSRQ,
                            true);
                    addData(arfcn, pci, (float) rsrp / 4, (float) rsrq / 4);
                }
            }
        }
    }
}

class IntraFrequencyNeighbouringCellInformationLteTdd extends
        ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND};

    public IntraFrequencyNeighbouringCellInformationLteTdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Intra-frequency neighbouring cell information (LTE TDD)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"PCI", "RSCP", "ECN0"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String coName = MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO + ".";
        int cellNum = getFieldValue(data, coName
                + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_CELL_NUM);
        for (int i = 0; i < cellNum && i < 16; i++) {
            String coNameNew = coName
                    + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_INTRA_CELL
                    + "[" + i + "].";
            int valid = getFieldValue(data, coNameNew
                    + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_VALID);
            int pci = getFieldValue(data, coNameNew
                    + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_PCI);
            int rsrp = getFieldValue(data, coNameNew
                            + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_RSRP,
                    true);
            int rsrq = getFieldValue(data, coNameNew
                            + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTRA_INFO_RSRQ,
                    true);
            addData(((valid > 0) ? pci : ""), ((valid > 0)
                            && rsrp != 0xFFFFFFFF ? (float) rsrp / 4 : ""),
                    ((valid > 0) && rsrq != 0xFFFFFFFF ? (float) rsrq / 4 : ""));
        }
    }
}

class InterFrequencyNeighbouringCellInformationLteTdd extends
        ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND};

    public InterFrequencyNeighbouringCellInformationLteTdd(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Inter-frequency neighbouring cell information (LTE TDD)";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"EARFCN", "PCI", "RSCP", "ECN0"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        String coName = MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO + ".";
        int freqNum = getFieldValue(data, coName +
                MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_FREQ_NUM);
        for (int i = 0; i < freqNum && i < 4; i++) {
            String coNameNew = coName +
                    MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_FREQ + "[" + i + "].";

            int valid = getFieldValue(data,
                    coNameNew + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_VALID);
            int earfcn = getFieldValue(data,
                    coNameNew + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL_EARFCN);
            int cellNum = getFieldValue(data,
                    coNameNew + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_CELL_NUM);
            for (int j = 0; j < cellNum && j < 6; j++) {
                String interName = coNameNew
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL
                        + "[" + j + "].";
                int pci = getFieldValue(
                        data,
                        interName
                                + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL_PCI);
                int rsrp = getFieldValue(
                        data,
                        interName
                                + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL_RSRP,
                        true);
                int rsrq = getFieldValue(
                        data,
                        interName
                                + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_INTER_INFO_INTER_CELL_RSRQ,
                        true);
                addData(((valid > 0) ? earfcn : ""), ((valid > 0) ? pci : ""),
                        ((valid > 0) && rsrp != 0xFFFFFFFF ? (float) rsrp / 4
                                : ""),
                        ((valid > 0) && rsrq != 0xFFFFFFFF ? (float) rsrq / 4
                                : ""));
            }
        }
    }
}

class EnhancedRRCState extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_FDD_EM_URR_3G_GENERAL_STATUS_IND};
    private final HashMap<Integer, String> mStateMapping = new HashMap<Integer, String>() {
        {
            put(0, "N/A");
            put(1, "N/A");
            put(2, "N/A");
            put(3, "N/A");
            put(4, "N/A");
            put(5, "N/A");
            put(6, "");
            put(7, "HSDPA in Cell FACH");
            put(8, "HSDPA in Cell PCH:Receive Data");
            put(9, "HSDPA in Cell PCH:Receive Paging");
            put(10, "HSDPA in URA PCH:Receive Paging");
            put(11, "HSUPA in Cell FACH");
            put(12, "HSUPA in Cell PCH:Receive Data");
            put(13, "READY for HSPA in Cell PCH");
        }
    };

    public EnhancedRRCState(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Enhanced RRC State";
    }

    @Override
    String getGroup() {
        return "3. UMTS FDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    String[] getLabels() {
        return new String[]{"Enhanced RRC State"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String coName = MDMContent.TDD_EM_URR_3G_GENERAL_UMTS_UAS_3G_GENERAL_STATUS
                + ".";
        clearData();
        int state = getFieldValue(data, coName
                + MDMContent.TDD_EM_URR_3G_GENERAL_UMTS_RRC_STATE);
        if (state != 6) {
            String stateString = mStateMapping.get(state);
            // Elog.d(TAG, " Enhanced RRC State stateString: " + stateString);
            clearData();
            addData(stateString);
        }
    }
}


class GSMTasInfo extends NormalTableTasComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_GSM_TAS_INFO_IND,
            MDMContent.MSG_ID_EM_GSM_UTAS_INFO_IND};

    HashMap<Integer, String> AntennaMapping = new HashMap<Integer, String>() {
        {
            put(0, "LANT");
            put(1, "UANT");
            put(2, "LANT(')");
            put(3, "UANT");
            put(4, "-");
        }
    };
    HashMap<Integer, String> ServingBandMapping = new HashMap<Integer, String>() {
        {
            put(1, "Band 850");
            put(2, "Band 900");
            put(3, "Band 1800");
            put(4, "Band 1900");
            put(5, "-");
        }
    };
    HashMap<Integer, String> TasEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
            put(2, "-");
        }
    };

    public GSMTasInfo(Activity context) {
        super(context);
    }

    String antidxMapping(int antidx) {
        String antidx_s = "";
        if (antidx >= 0 && antidx <= 3)
            antidx_s = AntennaMapping.get(antidx);
        else
            antidx_s = AntennaMapping.get(4) + "(" + antidx + ")";
        return antidx_s;
    }

    String tasEableMapping(int tasidx) {
        String tasidx_s = "";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = TasEnableMapping.get(tasidx);
        } else
            tasidx_s = TasEnableMapping.get(2) + "(" + tasidx + ")";
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        String bandidx_s = " ";
        if (bandidx >= 1 && bandidx <= 4)
            bandidx_s = ServingBandMapping.get(bandidx);

        return bandidx_s;
    }

    String[] conbineLablesByModem(String[] lables1, String[] lables2,
                                  int position) {
        if (FeatureSupport.is93Modem()) {
            if (position < 0) {
                position = Math.abs(position);
                return addLablesAtPosition(lables2, lables1, position);
            }
            return addLablesAtPosition(lables1, lables2, position);
        }
        return lables2;
    }

    @Override
    String getName() {
        return "GSM TAS Info";
    }

    @Override
    String getGroup() {
        return "2. GSM EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        final String[] Lables_Common = {"Antenna Index",
                "Other Antenna Index", "Current Antenna Power",
                "Other Antenna Power", "Current Average SNR",
                "Other Average SNR", "Current Average SNR(dB)",
                "Other Average SNR(dB)"};
        final String[] Lables_Tas = {"TAS Enable Info", "Serving Band"};
        final String[] Labels_Dat = new String[]{"DAT Index"};

        return concatAll(conbineLablesByModem(Lables_Tas, Lables_Common,
                Lables_Tas.length), Labels_Dat);
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_EM_GSM_UTAS_INFO_IND)) {
            setInfoValid(1);
        } else {
            setInfoValid(0);
        }
        //@test
        //utas_info_valid = 1;
        if (isInfoValid()) {
            clearData();
            mAdapter.add(new String[]{"Use " + getName().replace("UTAS", "TAS")});
            return;
        }
        int tasEnableInfo = getFieldValue(data, MDMContent.GSM_TAS_ENABLE);
        int currentSurvingBand = getFieldValue(data,
                MDMContent.GSM_SERVING_BAND);
        int antennaIdx = getFieldValue(data, MDMContent.GSM_ANTENNA);

        int otherAntennaIdx = getFieldValue(data, MDMContent.GSM_OTHER_ANTENNA);

        int currentAntRxLevel = getFieldValue(data,
                MDMContent.GSM_CURRENT_ANTENNA_RXLEVEL, true);
        int otherAntRxLevel = getFieldValue(data,
                MDMContent.GSM_OTHER_ANTENNA_RXLEVEL, true);
        int currentAverageSNR = getFieldValue(data,
                MDMContent.GSM_CURRENT_AVERAGE_SNR, true);
        int otherAverageSNR = getFieldValue(data,
                MDMContent.GSM_OTHER_ANTENNA_SNR, true);
        int currentAverageSNRDb = (int) (10 * Math
                .log10((double) currentAverageSNR / 4));
        int otherAverageSNRDb = (int) (10 * Math
                .log10((double) otherAverageSNR / 4));
        int dat_index = getFieldValue(data, MDMContent.GSM_DAT_SCENARIO_INDEX, true);

        clearData();

        if (FeatureSupport.is93Modem()) {
            addData(tasEableMapping(tasEnableInfo));
            addData(servingBandMapping(currentSurvingBand));
        }
        addData(antidxMapping(antennaIdx));
        addData(antidxMapping(otherAntennaIdx));
        addData(currentAntRxLevel);
        addData(otherAntRxLevel);
        addData(currentAverageSNR);
        addData(otherAverageSNR);
        addData(currentAverageSNRDb);
        addData(otherAverageSNRDb);
        addData(dat_index);

        notifyDataSetChanged();
    }
}

class GSMUTasInfo extends CombinationTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_GSM_TAS_INFO_IND,
            MDMContent.MSG_ID_EM_GSM_UTAS_INFO_IND};
    HashMap<Integer, String> ServingBandMapping = new HashMap<Integer, String>() {
        {
            put(1, "Band 850");
            put(2, "Band 900");
            put(3, "Band 1800");
            put(4, "Band 1900");
        }
    };
    HashMap<Integer, String> TasEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
        }
    };
    List<String[]> labelsList = new ArrayList<String[]>() {{
        add(new String[]{"TAS Enable Info", "Serving Band", "Cur ant state",
                "Current Primary RxLev", "Current Diversity RxLev", "Current SNR",
                "Tx Power Detect"});
        add(new String[]{"TX Index"});
        add(new String[]{"ANT Index", "TX Pwr dBm", "SNR", "RSSI dBm"});

    }};
    String[] labelsKey = new String[]{"Tas Common", "TX Info", "ANT Info"};
    List<LinkedHashMap> valuesHashMap = new ArrayList<LinkedHashMap>() {{
        add(initHashMap(labelsList.get(0)));
        add(initHashMap(labelsList.get(1)));
        add(initArrayHashMap(labelsList.get(2)));
    }};
    private String[] tabTitle = new String[]{"Common", "Detail"};

    public GSMUTasInfo(Activity context) {
        super(context);
        initTableComponent(tabTitle);
    }

    String tasEableMapping(int tasidx) {
        String tasidx_s = "";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = TasEnableMapping.get(tasidx);
        } else
            tasidx_s = TasEnableMapping.get(2) + "(" + tasidx + ")";
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        String bandidx_s = " ";
        if (bandidx >= 1 && bandidx <= 4) {
            bandidx_s = ServingBandMapping.get(bandidx);
        } else {
            bandidx_s = "-(" + bandidx + ")";
        }

        return bandidx_s;
    }

    @Override
    String getName() {
        return "GSM UTAS Info";
    }

    @Override
    String getGroup() {
        return "2. GSM EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    ArrayList<String> getArrayTypeKey() {
        ArrayList<String> arrayTypeKeys = new ArrayList<String>();
        arrayTypeKeys.add(labelsKey[2]);
        return arrayTypeKeys;
    }

    @Override
    boolean isLabelArrayType(String label) {
        if (getArrayTypeKey().contains(label)) {
            return true;
        }
        return false;
    }

    @Override
    LinkedHashMap<String, LinkedHashMap> getHashMapLabels(int index) {
        LinkedHashMap<String, LinkedHashMap> hashMapkeyValues =
                new LinkedHashMap<String, LinkedHashMap>();
        switch (index) {
            case 0:
                hashMapkeyValues.put(labelsKey[0], valuesHashMap.get(0));
                break;
            case 1:
                hashMapkeyValues.put(labelsKey[1], valuesHashMap.get(1));
                hashMapkeyValues.put(labelsKey[2], valuesHashMap.get(2));
                break;
        }
        return hashMapkeyValues;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        if (name.equals(MDMContent.MSG_ID_EM_GSM_TAS_INFO_IND)) {
            setInfoValid(0);
        } else {
            setInfoValid(1);
        }
        if (!isInfoValid()) {
            resetView();
            TextView textView = new TextView(mContext);
            ListView.LayoutParams layoutParams = new ListView.LayoutParams(
                    ListView.LayoutParams.MATCH_PARENT, ListView.LayoutParams.WRAP_CONTENT);
            textView.setLayoutParams(layoutParams);
            textView.setPadding(20, 0, 20, 0);
            textView.setText("Use " + getName().replace("UTAS", "TAS"));
            textView.setTextSize(16);
            scrollView.addView(textView);
            return;
        }
        int tas_enable = getFieldValue(data, MDMContent.GSM_TAS_ENABLE);
        int surving_band = getFieldValue(data, MDMContent.GSM_SERVING_BAND);
        int tx_ant_rxLevel = getFieldValue(data, MDMContent.GSM_TX_ANTENNA_RXLEVEL, true);
        int div_ant_rxLevel = getFieldValue(data, MDMContent.GSM_DRX_ANTENNA_RXLEVEL, true);
        int cur_average_snr = getFieldValue(data, MDMContent.GSM_CURRENT_AVERAGE_SNR, true);
        int ant_state = getFieldValue(data, MDMContent.GSM_ANTENNA_STATE);
        int gsm_txPower_det = getFieldValue(data, MDMContent.GSM_TXPOWER_DET, true);
        setHashMapKeyValues(labelsKey[0], 0, labelsList.get(0), new String[]{
                tasEableMapping(tas_enable),
                servingBandMapping(surving_band),
                ant_state >= 0 && ant_state <= 23 ? ant_state + "" : "-(" + ant_state + ")",
                tx_ant_rxLevel + "dBm",
                div_ant_rxLevel == -1 ? "-(" + div_ant_rxLevel + ")" : div_ant_rxLevel + "dBm",
                cur_average_snr + "",
                gsm_txPower_det >= 0 ? gsm_txPower_det + "dBm" : ""
        });
        addData(labelsKey[0], 0);
        int tx_ant_index = getFieldValue(data, MDMContent.GSM_TX_ANT_INDEX);
        setHashMapKeyValues(labelsKey[1], 1, labelsList.get(1), new String[]{
                tx_ant_index >= 1 && tx_ant_index <= 14 ? tx_ant_index + "" : "-(" + tx_ant_index
                        + ")"
        });
        addData(labelsKey[1], 1);
        int available_ant_num = getFieldValue(data, MDMContent.GSM_AVAILABLE_ANT_NUM);
        for (int i = 0; i < available_ant_num && i <= 5; i++) {
            int available_ant = getFieldValue(data, MDMContent.GSM_AVAILABLE_ANT + "[" + i + "]");
            int gsm_antenna_rxlev = getFieldValue(data,
                    MDMContent.GSM_ANTENNA_RXLEVEL + "[" + i + "]", true);
            int gsm_snr = getFieldValue(data, MDMContent.GSM_SNR + "[" + i + "]", true);
            int gsm_txpower = getFieldValue(data, MDMContent.GSM_TXPOWER + "[" + i + "]", true);
            setHashMapKeyValues(labelsKey[2], 1, labelsList.get(2), new String[]{
                    available_ant >= 1 && available_ant <= 14 ?
                            available_ant + "" : "-(" + available_ant + ")",
                    gsm_txpower >= 0 ? gsm_txpower + "dBm" : "",
                    gsm_snr + "",
                    gsm_antenna_rxlev <= 0 ? gsm_antenna_rxlev + "dBm" : ""
            });
        }
        addData(labelsKey[2], 1);
    }
}

class GSMRxdInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_GSM_RXD_INFO_IND};
    HashMap<Integer, String> ServingBandMapping = new HashMap<Integer, String>() {
        {
            put(1, "Band 850");
            put(2, "Band 900");
            put(3, "Band 1800");
            put(4, "Band 1900");
            put(5, "-");
        }
    };
    HashMap<Integer, String> RxModeMapping = new HashMap<Integer, String>() {
        {
            put(-1, "-");
            put(0, "MODE_LEGACY");
            put(1, "MODE_1RX_DESENSE");
            put(3, "MODE_RXD");
            put(17, "MODE_1RX_DESENSE_CROSS");
            put(19, "RXD_CROSS");
        }
    };
    HashMap<Integer, String> RxdEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
            put(2, "-");
        }
    };

    public GSMRxdInfo(Activity context) {
        super(context);
    }

    String rxdModeMapping(int rxMode) {
        String rxMode_s = "";
        if (RxModeMapping.keySet().contains(rxMode))
            rxMode_s = RxModeMapping.get(rxMode);
        else
            rxMode_s = RxModeMapping.get(-1) + "(" + rxMode_s + ")";
        return rxMode_s;
    }

    String rxdEnableMapping(int tasidx) {
        String tasidx_s = "";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = RxdEnableMapping.get(tasidx);
        } else
            tasidx_s = RxdEnableMapping.get(2) + "(" + tasidx + ")";
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        String bandidx_s = " ";
        if (bandidx >= 1 && bandidx <= 4)
            bandidx_s = ServingBandMapping.get(bandidx);

        return bandidx_s;
    }

    @Override
    String getName() {
        return "GSM RXD Info";
    }

    @Override
    String getGroup() {
        return "2. GSM EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        final String[] Lables = {"RxD Enable",
                "Serving ARFCN", "Serving Band",
                "RxD Mode", "Current Primary RxLev",
                "Current Diversity RxLev"};
        return Lables;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int rxdEnable = getFieldValue(data, MDMContent.GSM_RXD_ENABLE);
        int servingArfcn = getFieldValue(data, MDMContent.GSM_SERVING_ARFCN, true);
        int currentSurvingBand = getFieldValue(data,
                MDMContent.GSM_SERVING_BAND);
        int rxdMode = getFieldValue(data, MDMContent.GSM_RXD_MODE);
        int curPriRxLev = getFieldValue(data, MDMContent.GSM_RXLEV_PRX, true);
        int curDivRxLev = getFieldValue(data, MDMContent.GSM_RXLEV_DRX, true);
        clearData();

        addData(rxdEnableMapping(rxdEnable));
        addData(servingArfcn);
        addData(servingBandMapping(currentSurvingBand));
        if (rxdEnable == 1) {
            addData(rxdModeMapping(rxdMode));
            addData(curPriRxLev + " dBm");
            addData(curDivRxLev + " dBm");
        }

        notifyDataSetChanged();
    }
}

class TddTasInfo extends NormalTableTasComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_TDD_TAS_INFO_IND};
    int TasVersion = 1;
    HashMap<Integer, String> AntennaMapping = new HashMap<Integer, String>() {
        {
            put(0, "LANT");
            put(1, "UANT");
            put(2, "LANT(')");
            put(3, "-");
        }
    };
    HashMap<Integer, String> TasEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
            put(2, "-");
        }
    };
    HashMap<Integer, String> ServingBandMapping = new HashMap<Integer, String>() {
        {
            put(0, "Band 34");
            put(1, "Band 39");
            put(2, "-");
        }
    };

    public TddTasInfo(Activity context) {
        super(context);
    }

    String antidxMapping(int antidx) {
        String antidx_s = "";
        if (antidx >= 0 && antidx <= 2)
            antidx_s = AntennaMapping.get(antidx);
        else
            antidx_s = AntennaMapping.get(3) + "(" + antidx + ")";
        return antidx_s;
    }

    String tasEableMapping(int tasidx) {
        String tasidx_s = "";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = TasEnableMapping.get(tasidx);
        } else
            tasidx_s = TasEnableMapping.get(2) + "(" + tasidx + ")";
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        String bandidx_s = "";
        if (bandidx >= 0 && bandidx <= 1) {
            bandidx_s = ServingBandMapping.get(bandidx);
        } else
            bandidx_s = ServingBandMapping.get(2) + "(" + bandidx + ")";
        return bandidx_s;
    }

    String[] conbineLablesByModem(String[] lables1, String[] lables2,
                                  int position) {
        if (FeatureSupport.is93Modem()) {
            if (position < 0) {
                position = Math.abs(position);
                return addLablesAtPosition(lables2, lables1, position);
            }
            return addLablesAtPosition(lables1, lables2, position);
        }
        return lables2;
    }

    @Override
    String getName() {
        return "TDD TAS Info";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        final String[] Lables_Verison_v1 = new String[]{"force tx en",
                "force ant state", "cur ant state", "RSCP_LANT", "RSSI_LANT",
                "SINR_LANT", "RSCP_UANT", "RSSI_UANT", "SINR_UANT", "tx pwr"};

        final String[] Lables_Verison_v2 = new String[]{"force tx en",
                "force ant state", "cur ant state", "RSCP_LANT", "RSSI_LANT",
                "SINR_LANT", "RSCP_UANT", "RSSI_UANT", "SINR_UANT",
                "RSCP_LANT(')", "RSSI_LANT(')", "SINR_LANT(')", "tx pwr"};
        final String[] Lables_Tas = new String[]{"Tas Enable Info",
                "Serving Band"};

        if (TasVersion == 2) {
            return conbineLablesByModem(Lables_Tas, Lables_Verison_v2,
                    Lables_Tas.length);
        }
        return conbineLablesByModem(Lables_Tas, Lables_Verison_v1,
                Lables_Tas.length);

    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int cur_ant_idx = getFieldValue(data, MDMContent.TDD_CUR_ANT_IDX);
        int utas_info_valid = ((cur_ant_idx >> 8)) == 3 ? 1 : 0;
        setInfoValid(utas_info_valid);
        if (isInfoValid()) {
            clearData();
            mAdapter.add(new String[]{"Use " + getName().replace("UTAS", "TAS")});
            return;
        }
        TasVersion = ((cur_ant_idx >> 8) & 0x3) == 2 ? 2 : 1;
        Elog.d(TAG, "TasVersion " + TasVersion);
        int tas_enable = getFieldValue(data, MDMContent.TDD_TAS_ENABLE);
        int serving_band = getFieldValue(data, MDMContent.TDD_SERVING_BAND);
        int force_tx_en = getFieldValue(data, MDMContent.TDD_FORCE_TX_EN);

        int force_ant_idx = getFieldValue(data, MDMContent.TDD_FORCE_ANT_IDX);

        int ant0_rscp = getFieldValue(data, MDMContent.TDD_ANT0_RSCP, true);
        int ant0_rssi = getFieldValue(data, MDMContent.TDD_ANT0_RSSI, true);
        int ant0_sinr = getFieldValue(data, MDMContent.TDD_ANT0_SINR, true);
        int ant1_rscp = getFieldValue(data, MDMContent.TDD_ANT1_RSCP, true);
        int ant1_rssi = getFieldValue(data, MDMContent.TDD_ANT1_RSSI, true);
        int ant1_sinr = getFieldValue(data, MDMContent.TDD_ANT1_SINR, true);
        int txpower = getFieldValue(data, MDMContent.TDD_TX_PWR, true);
        int ant2_rscp = 0;
        int ant2_rssi = 0;
        int ant2_sinr = 0;
        if (TasVersion == 2) {
            ant2_rscp = getFieldValue(data, MDMContent.TDD_ANT2_RSCP, true);
            ant2_rssi = getFieldValue(data, MDMContent.TDD_ANT2_RSSI, true);
            ant2_sinr = getFieldValue(data, MDMContent.TDD_ANT2_SINR, true);
        }

        clearData();

        if (FeatureSupport.is93Modem()) {
            addData(tasEableMapping(tas_enable),
                    servingBandMapping(serving_band));
        }
        if (TasVersion == 2) {
            addData(tasEableMapping(force_tx_en), antidxMapping(force_ant_idx),
                    (cur_ant_idx & 0xff), ant0_rscp, ant0_rssi, ant0_sinr,
                    ant1_rscp, ant1_rssi, ant1_sinr, ant2_rscp, ant2_rssi,
                    ant2_sinr, txpower);
        } else {
            addData(tasEableMapping(force_tx_en), antidxMapping(force_ant_idx),
                    (cur_ant_idx & 0xff), ant0_rscp, ant0_rssi, ant0_sinr,
                    ant1_rscp, ant1_rssi, ant1_sinr, txpower);
        }
        notifyDataSetChanged();
    }
}

class TddUTasInfo extends CombinationTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_TDD_TAS_INFO_IND};
    HashMap<Integer, String> ServingBandMapping =
            new HashMap<Integer, String>() {
                {
                    put(0, "B34");
                    put(1, "B39");
                }
            };
    HashMap<Integer, String> TasEnableMapping =
            new HashMap<Integer, String>() {
                {
                    put(0, "DISABLE");
                    put(1, "ENABLE");
                }
            };
    List<String[]> labelsList = new ArrayList<String[]>() {{
        add(new String[]{"TAS Enable Info", "Force TX Enable",
                "Force Ant State", "Current Ant State"});
        add(new String[]{"Current Band", "Main Index"});
        add(new String[]{"ANT Index", "TX Pwr dBm", "SINR dB", "RSCP dBm"});

    }};
    String[] labelsKey = new String[]{"Tas Common", "Main Info", "ANT Info"};
    List<LinkedHashMap> valuesHashMap = new ArrayList<LinkedHashMap>() {{
        add(initHashMap(labelsList.get(0)));
        add(initHashMap(labelsList.get(1)));
        add(initArrayHashMap(labelsList.get(2)));
    }};
    private String[] tabTitle = new String[]{"Common", "Detail"};

    public TddUTasInfo(Activity context) {
        super(context);
        initTableComponent(tabTitle);
    }

    String tasEableMapping(int tasidx) {
        String tasidx_s = " ";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = TasEnableMapping.get(tasidx);
        } else {
            tasidx_s = "-(" + tasidx + ")";
        }
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        String bandidx_s = " ";
        if (ServingBandMapping.containsKey(bandidx)) {
            bandidx_s = ServingBandMapping.get(bandidx);
        } else {
            bandidx_s = "-(" + bandidx + ")";
        }

        return bandidx_s;
    }

    @Override
    String getName() {
        return "TDD UTAS Info";
    }

    @Override
    String getGroup() {
        return "4. UMTS TDD EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    ArrayList<String> getArrayTypeKey() {
        ArrayList<String> arrayTypeKeys = new ArrayList<String>();
        arrayTypeKeys.add(labelsKey[2]);
        return arrayTypeKeys;
    }

    @Override
    boolean isLabelArrayType(String label) {
        if (getArrayTypeKey().contains(label)) {
            return true;
        }
        return false;
    }

    @Override
    LinkedHashMap<String, LinkedHashMap> getHashMapLabels(int index) {
        LinkedHashMap<String, LinkedHashMap> hashMapkeyValues =
                new LinkedHashMap<String, LinkedHashMap>();
        switch (index) {
            case 0:
                hashMapkeyValues.put(labelsKey[0], valuesHashMap.get(0));
                break;
            case 1:
                hashMapkeyValues.put(labelsKey[1], valuesHashMap.get(1));
                hashMapkeyValues.put(labelsKey[2], valuesHashMap.get(2));
                break;
        }
        return hashMapkeyValues;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        int cur_ant_idx = getFieldValue(data, MDMContent.TDD_CUR_ANT_IDX);
        int utas_info_valid = (cur_ant_idx >> 8) == 3 ? 1 : 0;
        setInfoValid(utas_info_valid);
        if (!isInfoValid()) {
            resetView();
            TextView textView = new TextView(mContext);
            ListView.LayoutParams layoutParams = new ListView.LayoutParams(
                    ListView.LayoutParams.MATCH_PARENT, ListView.LayoutParams.WRAP_CONTENT);
            textView.setLayoutParams(layoutParams);
            textView.setPadding(20, 0, 20, 0);
            textView.setText("Use " + getName().replace("UTAS", "TAS"));
            textView.setTextSize(16);
            scrollView.addView(textView);
            return;
        }
        int tas_enable = getFieldValue(data, MDMContent.TDD_TAS_ENABLE_INFO);
        int force_tx_en = getFieldValue(data, MDMContent.TDD_FORCE_TX_EN);
        int force_ant_state = getFieldValue(data, MDMContent.TDD_FORCE_ANT_STATE);
        int ant_state = getFieldValue(data, MDMContent.TDD_CUR_ANT_STATE);
        int cur_ant_state = (ant_state & 255);
        setHashMapKeyValues(labelsKey[0], 0, labelsList.get(0), new String[]{
                tasEableMapping(tas_enable),
                tasEableMapping(force_tx_en),
                force_ant_state >= 0 && force_ant_state <= 23 ?
                        force_ant_state + "" : "-(" + force_ant_state + ")",
                cur_ant_state >= 0 && cur_ant_state <= 23 ? cur_ant_state + "" : "-(" +
                        cur_ant_state + ")",
        });

        int serving_band = getFieldValue(data, MDMContent.TDD_CURRENT_SERVING_BAND);
        int cur_ant_index = getFieldValue(data, MDMContent.TDD_CUR_ANT_INDEX);
        int cur_ant_index_h = cur_ant_index >> 8;
        int cur_ant_index_l = cur_ant_index & 255;
        setHashMapKeyValues(labelsKey[1], 1, labelsList.get(1), new String[]{
                servingBandMapping(serving_band),
                cur_ant_index_l >= 0 && cur_ant_index_l <= 7 ?
                        cur_ant_index_l + "" : "-(" + cur_ant_index_l + ")"
        });
        String antNumCoName = (serving_band == 1 ?
                MDMContent.TDD_B39_AVAILABLE_ANT_NUM : MDMContent.TDD_B34_AVAILABLE_ANT_NUM);
        int cellNum = getFieldValue(data, antNumCoName);
        String coName = serving_band == 1 ?
                MDMContent.TDD_B39_AVAILABLE_ANT : MDMContent.TDD_B34_AVAILABLE_ANT;
        for (int i = 0; i < cellNum; i++) {
            int ant_index = getFieldValue(data, coName + "[" + i + "]");
            int tx_power = getFieldValue(data, MDMContent.TDD_TX_POWER + "[" + i + "]", true);
            int ant_sinr = getFieldValue(data, MDMContent.TDD_ANT_SINR + "[" + i + "]", true);
            int ant_rscp = getFieldValue(data, MDMContent.TDD_ANT_RSCP + "[" + i + "]", true);
            setHashMapKeyValues(labelsKey[2], 1, labelsList.get(2), new String[]{
                    ant_index <= 7 && ant_index >= 0 ? ant_index + "" : "-(" + ant_index + ")",
                    i == cur_ant_index_h ? (tx_power <= 32 && tx_power >= -50 ?
                            tx_power + "" : "-(" + tx_power + ")") : "",
                    ant_sinr <= 32 && ant_sinr >= -50 ? ant_sinr + "" : "-(" + ant_sinr + ")",
                    ant_rscp <= -40 && ant_rscp >= -140 ? ant_rscp + "" : "-(" + ant_rscp + ")"
            });
        }
        addData(labelsKey[0], 0);
        addData(labelsKey[1], 1);
        addData(labelsKey[2], 1);
    }
}


class LteTasInfo extends NormalTableTasComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    int TasVersion = 1;
    HashMap<Integer, String> AntennaMapping = new HashMap<Integer, String>() {
        {
            put(0, "LANT");
            put(1, "UANT");
            put(2, "LANT(')");
            put(3, "UANT");
            put(4, "-");
        }
    };
    HashMap<Integer, String> TasEnableMapping = new HashMap<Integer, String>() {
        {
            put(0, "DISABLE");
            put(1, "ENABLE");
            put(2, "-");
        }
    };

    public LteTasInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE TAS Info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] conbineLablesByModem(String[] lables1, String[] lables2,
                                  int position) {
        if (FeatureSupport.is93Modem()) {
            if (position < 0) {
                position = Math.abs(position);
                return addLablesAtPosition(lables2, lables1, position);
            }
            return addLablesAtPosition(lables1, lables2, position);
        }
        return lables2;
    }

    String[] getLabels() {
        final String[] Lables_Verison_v1_1 = new String[]{"TX Antenna",
                "RSRP_LANT", "RSRP_UANT", "TX Power"};

        final String[] Lables_Verison_v2_1 = new String[]{"TX Antenna",
                "RSRP_LANT", "RSRP_UANT", "RSRP_LANT(')", "TX Power"};

        final String[] Lables_Verison_v1_2 = new String[]{"CC0 TX Antenna",
                "CC0 RSRP_LANT", "CC0 RSRP_UANT", "CC1 TX Antenna",
                "CC1 RSRP_LANT", "CC1 RSRP_UANT", "TX Power"};

        final String[] Lables_Verison_v2_2 = new String[]{"CC0 TX Antenna",
                "CC0 RSRP_LANT", "CC0 RSRP_UANT", "CC0 RSRP_LANT(')",
                "CC1 TX Antenna", "CC1 RSRP_LANT", "CC1 RSRP_UANT",
                "CC1 RSRP_LANT(')", "TX Power"};

        final String[] Lables_Tas_1 = new String[]{"TAS Enable Info",
                "Serving Band"};
        final String[] Lables_Tas_2 = new String[]{"CC1 Serving Band"};
        final String[] Labels_Dat = new String[]{"DAT Index"};

        if (TasVersion == 1) {
            return concatAll(conbineLablesByModem(Lables_Tas_1, Lables_Verison_v1_1,
                    Lables_Tas_1.length), Labels_Dat);
        } else if (TasVersion == 2) {
            return concatAll(conbineLablesByModem(Lables_Tas_1, Lables_Verison_v2_1,
                    Lables_Tas_1.length), Labels_Dat);
        } else if (TasVersion == 3) {
            String[] result = conbineLablesByModem(Lables_Tas_1,
                    Lables_Verison_v1_2, Lables_Tas_1.length);
            return concatAll(conbineLablesByModem(Lables_Tas_2, result, -5), Labels_Dat);
        } else if (TasVersion == 4) {
            String[] result = conbineLablesByModem(Lables_Tas_1,
                    Lables_Verison_v2_2, Lables_Tas_1.length);
            return concatAll(conbineLablesByModem(Lables_Tas_2, result, -6), Labels_Dat);
        }
        return concatAll(conbineLablesByModem(Lables_Tas_1, Lables_Verison_v1_1,
                Lables_Tas_1.length), Labels_Dat);
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String rscpCheck(int value) {
        String value_s = String.valueOf(value);
        if (value == -255)
            value_s = " ";
        return value_s;
    }

    String antidxMapping(int antidx) {
        String antidx_s = " ";
        if (antidx >= 0 && antidx <= 3)
            antidx_s = AntennaMapping.get(antidx);
        else
            antidx_s = AntennaMapping.get(4) + "(" + antidx + ")";
        return antidx_s;
    }

    String tasEableMapping(int tasidx) {
        String tasidx_s = "";
        if (tasidx >= 0 && tasidx <= 1) {
            tasidx_s = TasEnableMapping.get(tasidx);
        } else
            tasidx_s = TasEnableMapping.get(2) + "(" + tasidx + ")";
        return tasidx_s;
    }

    String servingBandMapping(int bandidx) {
        if (bandidx == 0) {
            return "INVALID";
        }
        String bandidx_s = "Band";
        bandidx_s = bandidx_s + " " + bandidx;
        return bandidx_s;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int antidx = 0;
        int rsrp_l_ant = 0;
        int rsrp_u_ant = 0;
        int rsrp_l_ant_a = 0;
        int antidx_cc1 = 0;
        int rsrp_l_ant_cc1 = 0;
        int rsrp_u_ant_cc1 = 0;
        int rsrp_l_ant_a_cc1 = 0;
        int tx_pwr = 0;
        int tasidx = 0;
        int bandidx = 0;
        int bandidx_cc1 = 0;
        int dat_index = 0;

        int dl_cc_count = getFieldValue(data, MDMContent.DL_CC_COUNT);
        int ul_cc_count = getFieldValue(data, MDMContent.UL_CC_COUNT);

        int utas_info_valid = getFieldValue(data, MDMContent.EL1_STATUS_IND_UL_INFO + "[0]." +
                MDMContent.EL1_STATUS_IND_UTAS_INFO_VALID);
        setInfoValid(utas_info_valid);
        //@test
        //utas_info_valid = 1;
        if (isInfoValid()) {
            clearData();
            mAdapter.add(new String[]{"Use " + getName().replace("UTAS", "TAS")});
            return;
        }

        TasVersion = getFieldValue(data, MDMContent.FDD_EM_UL1_TAS_VERISION);
        TasVersion = (TasVersion == 0) ? 1 : 2;

        Elog.d(TAG, "dl_cc_count= " + dl_cc_count);
        Elog.d(TAG, "ul_cc_count= " + ul_cc_count);
        Elog.d(TAG, "TasVersion= " + TasVersion);

        antidx = getFieldValue(data, "ul_info[0].tx_ant_type", true);
        rsrp_l_ant = getFieldValue(data, "ul_info[0].rsrp_l_ant", true);
        rsrp_u_ant = getFieldValue(data, "ul_info[0].rsrp_u_ant", true);
        rsrp_l_ant_a = getFieldValue(data, "ul_info[0].rsrp_l_ant_a", true);
        tx_pwr = getFieldValue(data, "ul_info[0].tx_power", true);

        antidx_cc1 = getFieldValue(data, "ul_info[1].tx_ant_type", true);
        rsrp_l_ant_cc1 = getFieldValue(data, "ul_info[1].rsrp_l_ant", true);
        rsrp_u_ant_cc1 = getFieldValue(data, "ul_info[1].rsrp_u_ant", true);
        rsrp_l_ant_cc1 = getFieldValue(data, "ul_info[1].rsrp_l_ant_a", true);
        dat_index = getFieldValue(data, "ul_info[0].el1_dat_scenario_index", true);

        if (FeatureSupport.is93Modem()) {
            tasidx = getFieldValue(data, "ul_info[0].tas_status");
            bandidx = getFieldValue(data, "cell_info[0].band", true);
            bandidx_cc1 = getFieldValue(data, "cell_info[1].band", true);
        }

        clearData();
        if (dl_cc_count == 0) {
            TasVersion = 1;
            addData("", "", "", "", "", "");
            addData("");
            return;
        }

        if (dl_cc_count == 1 && ul_cc_count == 0 && TasVersion == 1) {
            addData("", "", "", rscpCheck(rsrp_l_ant), rscpCheck(rsrp_u_ant),
                    "");
            addData(dat_index);
            return;
        }

        if (dl_cc_count >= 1 && ul_cc_count == 1 && TasVersion == 1) {
            if (FeatureSupport.is93Modem()) {
                addData(tasEableMapping(tasidx), servingBandMapping(bandidx));
            }
            addData(antidxMapping(antidx), rscpCheck(rsrp_l_ant),
                    rscpCheck(rsrp_u_ant), tx_pwr);
            addData(dat_index);
            return;
        }

        if (dl_cc_count == 1 && ul_cc_count == 0 && TasVersion == 2) {
            addData("", "", "", rscpCheck(rsrp_l_ant), rscpCheck(rsrp_u_ant),
                    rscpCheck(rsrp_l_ant_a), "");
            addData(dat_index);
            return;
        }

        if (dl_cc_count >= 1 && ul_cc_count == 1 && TasVersion == 2) {
            if (FeatureSupport.is93Modem()) {
                addData(tasEableMapping(tasidx), servingBandMapping(bandidx));
            }
            addData(antidxMapping(antidx), rscpCheck(rsrp_l_ant),
                    rscpCheck(rsrp_u_ant), rscpCheck(rsrp_l_ant_a), tx_pwr);
            addData(dat_index);
            return;
        }

        if (dl_cc_count > 1 && ul_cc_count == 2 && TasVersion == 1) {
            TasVersion = 3;

            if (FeatureSupport.is93Modem()) {
                addData(tasEableMapping(tasidx), servingBandMapping(bandidx),
                        antidxMapping(antidx), rscpCheck(rsrp_l_ant),
                        rscpCheck(rsrp_u_ant), servingBandMapping(bandidx_cc1),
                        antidxMapping(antidx_cc1), rscpCheck(rsrp_l_ant_cc1),
                        rscpCheck(rsrp_u_ant_cc1), tx_pwr);
                addData(dat_index);
                return;
            }
            addData(antidxMapping(antidx), rscpCheck(rsrp_l_ant),
                    rscpCheck(rsrp_u_ant), antidxMapping(antidx_cc1),
                    rscpCheck(rsrp_l_ant_cc1), rscpCheck(rsrp_u_ant_cc1),
                    tx_pwr);
            addData(dat_index);
            return;
        }
        if (dl_cc_count > 1 && ul_cc_count == 2 && TasVersion == 2) {
            TasVersion = 4;

            if (FeatureSupport.is93Modem()) {
                addData(tasEableMapping(tasidx), servingBandMapping(bandidx),
                        antidxMapping(antidx), rscpCheck(rsrp_l_ant),
                        rscpCheck(rsrp_u_ant), rscpCheck(rsrp_l_ant_a),
                        servingBandMapping(bandidx_cc1),
                        antidxMapping(antidx_cc1), rscpCheck(rsrp_l_ant_cc1),
                        rscpCheck(rsrp_u_ant_cc1), rscpCheck(rsrp_l_ant_a_cc1),
                        tx_pwr);
                addData(dat_index);
                return;
            }
            addData(antidxMapping(antidx), rscpCheck(rsrp_l_ant),
                    rscpCheck(rsrp_u_ant), rscpCheck(rsrp_l_ant_a),
                    antidxMapping(antidx_cc1), rscpCheck(rsrp_l_ant_cc1),
                    rscpCheck(rsrp_u_ant_cc1), rscpCheck(rsrp_l_ant_a_cc1),
                    tx_pwr);
            addData(dat_index);
            return;
        }

    }
}

class ErrcFeatureDetection extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_FEATURE_DETECTION_IND};

    HashMap<Integer, String> mMappingFD = new HashMap<Integer, String>() {
        {
            put(0, "ERRC_FEAT_MFBI_PRIORITIZATION");
            put(1, "ERRC_FEAT_AC_BAR_SKIP_MMTEL_VOICE");
            put(2, "ERRC_FEAT_AC_BAR_SKIP_MMTEL_VIDEO");
            put(3, "ERRC_FEAT_AC_BAR_SKIP_SMS");
            put(4, "ERRC_FEAT_PLMN_SPECIFIC_AC_BAR");
            put(5, "ERRC_FEAT_PLMN_SPECIFIC_SSAC");
            put(6, "ERRC_FEAT_RRC_CONN_TEMP_FAIL_OFFSET");
            put(7, "ERRC_FEAT_INCMON_EUTRA");
            put(8, "ERRC_FEAT_INCMON_UTRA");
            put(9, "ERRC_FEAT_CELL_SPECIFIC_TTT");
            put(10, "ERRC_FEAT_FAST_RLF_REC_WITH_T312");
            put(11, "ERRC_FEAT_RSRQ_LOWER_VALUE_RANGE_EXT");
            put(12, "ERRC_FEAT_ENH_HARQ_TTI_BUND_FOR_FDD");
            put(13, "ERRC_FEAT_LOG_CH_SR_PROHIBIT_TIMER");
            put(14, "ERRC_FEAT_MOB_HIST_REPORTING");
            put(15, "ERRC_FEAT_SHORTER_MCH_SCHED_PERIOD");
            put(16, "ERRC_FEAT_IDC_ENH_FOR_UL_CA");
            put(17, "ERRC_FEAT_LOGGED_MDT");
            put(18, "ERRC_FEAT_IMMED_MDT");
            put(19, "ERRC_FEAT_EICIC_SF_PATTERN");
            put(20, "ERRC_FEAT_EICIC_DEDICATED_SIB1");
            put(21, "ERRC_FEAT_MBSFN_AREA_DETECTED");
            put(22, "ERRC_FEAT_UL_64QAM_DETECTED");
            put(23, "ERRC_FEAT_EAB_DETECTED");
            put(24, "ERRC_FEAT_DL_256QAM");
            put(25, "ERRC_FEAT_4G_BAND");
            put(26, "ERRC_FEAT_OOS_CAUSE");
            put(27, "ERRC_FEAT_LTE_INTER_FREQ_RESEL");
            put(28, "ERRC_FEAT_LTE_FDD_TDD_RESEL");
            put(29, "ERRC_FEAT_LTE_FDD_TDD_REDIRECT");
            put(30, "ERRC_FEAT_4G3_CSFB_REDIRECT");
            put(31, "ERRC_FEAT_4G2_CSFB_REDIRECT");
            put(32, "ERRC_FEAT_IMS_ECC_SUPPORT");
            put(33, "ERRC_FEAT_FOUR_LAYERS_MIMO_PCELL");
            put(34, "ERRC_FEAT_FOUR_LAYERS_MIMO_SCELL");
            put(35, "ERRC_FEAT_CONN_REL_TRIGGER_A2");
            put(36, "ERRC_FEAT_TM8_DETECTED");
            put(37, "ERRC_FEAT_TM9_DETECTED");
            put(38, "ERRC_FEAT_SRS_TX_ANT_SWITCH_DETECTED");
            put(39, "ERRC_FEAT_SRS_ENHANCEMENT_DETECTED");
            put(40, "ERRC_FEAT_DMRS_ENHANCEMENT_DETECTED");
            put(41, "ERRC_FEAT_EICIC_DETECTED");
            put(42, "ERRC_FEAT_FEICIC_DETECTED");
            put(43, "ERRC_FEAT_DMRS_ENHANCEMENT_DETECTED");
            put(44, "ERRC_FEAT_EICIC_DETECTED");
            put(45, "ERRC_FEAT_FEICIC_DETECTED");
            put(46, "ERRC_FEAT_TRANS_MODE");
            put(47, "ERRC_FEAT_DL_LAYERS");
        }
    };

    HashMap<Integer, String> mDisplay = new HashMap<Integer, String>();

    StringBuilder mDetectedFeature = new StringBuilder("");

    public ErrcFeatureDetection(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "ERRC Feature Detection";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Detected Feature"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (data == null) {
            clearData();
            mDetectedFeature.delete(0, mDetectedFeature.toString().length() - 1);
            mDisplay.clear();
            return;
        }
        int index = getFieldValue(data, MDMContent.DETECTED_FEATURE);
        if (mDisplay.get(index) == null) {
            clearData();
            mDisplay.put(index, mMappingFD.get(index));
            mDetectedFeature.append(mMappingFD.get(index));
            addData(mDetectedFeature.toString() + "\n");
        }

    }

}

class EmacInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL2_OV_STATUS_IND,
            MDMContent.MSG_ID_EM_EMAC_CONFIG_REPORT_IND,};

    int drx_long_cycle = 0;
    int drx_short_cycle = 0;
    int tti_bundling = 0;
    String tti_bundling_s = "";
    int dl_sps_configured = 0;
    int ul_sps_configured = 0;
    int sr_periodicity = 0;
    int earfcn = 0;
    int cellId = 0;
    HashMap<Integer, String> mMappingInfo = new HashMap<Integer, String>() {
        {
            put(0, "DISABLED");
            put(1, "NORMAL");
            put(2, "ENHANCED_FDD");
            put(3, " ");
        }
    };

    public EmacInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EMAC Info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DRX Long Cycle", "DRX Short Cycle",
                "TTI Bundling", "DL SPS Configured", "UL SPS Configured",
                "SR Periodicity", "EARFCN", "Cell ID"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        if (name.equals(MDMContent.MSG_ID_EM_EMAC_CONFIG_REPORT_IND)) {
            tti_bundling = getFieldValue(data, "tti_bundling");
            if (tti_bundling >= 0 && tti_bundling <= 2)
                tti_bundling_s = mMappingInfo.get(tti_bundling);
            else
                tti_bundling_s = mMappingInfo.get(3);
            dl_sps_configured = getFieldValue(data, "dl_sps_configured");
            ul_sps_configured = getFieldValue(data, "ul_sps_configured");
        } else if (name.equals(MDMContent.MSG_ID_EM_EL2_OV_STATUS_IND)) {
            drx_long_cycle = getFieldValue(data, "emac_stats.drx_long_cycle");
            drx_short_cycle = getFieldValue(data, "emac_stats.drx_short_cycle");
            sr_periodicity = getFieldValue(data, "emac_stats.sr_periodicity");
            earfcn = getFieldValue(data, "emac_stats.earfcn");
            cellId = getFieldValue(data, "emac_stats.phys_cell_id");
        }

        clearData();
        addData(drx_long_cycle, drx_short_cycle, tti_bundling_s,
                dl_sps_configured, ul_sps_configured, sr_periodicity, earfcn, cellId);
    }

}

class El2FeatureDetection extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL2_FEATURE_DETECTION_IND};

    HashMap<Integer, String> mMappingFD = new HashMap<Integer, String>() {
        {
            put(0, "ERRC_FEAT_MFBI_PRIORITIZATION");
        }
    };

    HashMap<Integer, String> mDisplay = new HashMap<Integer, String>();

    StringBuilder mDetectedFeature = new StringBuilder("");

    public El2FeatureDetection(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EL2 Feature Detection";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Detected Feature"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int index = getFieldValue(data, MDMContent.DETECTED_FEATURE);
        if (data == null) {
            clearData();
            mDetectedFeature
                    .delete(0, mDetectedFeature.toString().length() - 1);
            mDisplay.clear();
            return;
        }
        if (mDisplay.get(index) == null) {
            clearData();
            mDisplay.put(index, mMappingFD.get(index));
            mDetectedFeature.append(mMappingFD.get(index));
            addData(mDetectedFeature.toString() + "\n");
        }
    }

}

class LteErlcUlDrbConfiguration extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL2_PUB_STATUS_IND};

    public LteErlcUlDrbConfiguration(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE eRLC UL DRB Congiguration";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DRB ID", "RB ID", "EPS bearer ID",
                "Logical channel ID", "RLC mode", "LI length 15 bits"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String preName = MDMContent.ERLC_STATS + "." + MDMContent.UL_RB_CFG;
        clearData();
        for (int i = 0; i < 8; i++) {
            addData(i,
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.RB_ID),
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.EPSB_ID),
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.LGCH_ID),
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.RLC_MODE),
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.LI_LEN_15_BIT));
        }
    }

}

class LteErlcDlDrbConfiguration extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EL2_PUB_STATUS_IND};

    public LteErlcDlDrbConfiguration(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE eRLC DL DRB Congiguration";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"DRB ID", "RB ID", "EPS bearer ID",
                "Logical channel ID", "RLC mode", "LI length 15 bits"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        String preName = MDMContent.ERLC_STATS + "." + MDMContent.DL_RB_CFG;
        clearData();
        for (int i = 0; i < 8; i++) {
            addData(i,
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.RB_ID),
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.EPSB_ID),
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.LGCH_ID),
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.RLC_MODE),
                    getFieldValue(data, preName + "[" + (i + 2) + "]."
                            + MDMContent.LI_LEN_15_BIT));
        }
    }

}

class SecondaryCell extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_CONN_INFO_IND};

    public SecondaryCell(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Secondary Cell";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"EARFCN", "PCI", "Band", "Bandwidth",
                "Belongs to STAG"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int earfcn, pci, band, bandwidth, stag;
        for (int i = 0; i < 3; i++) {
            earfcn = getFieldValue(data, MDMContent.SCELL_EARFCN + "[" + i + "]");
            pci = getFieldValue(data, MDMContent.SCELL_PCI + "[" + i + "]");
            band = getFieldValue(data, MDMContent.SCELL_BAND + "[" + i + "]");
            bandwidth = getFieldValue(data, MDMContent.SCELL_BW + "[" + i + "]");
            stag = getFieldValue(data, MDMContent.SCELL_BELONGS_TO_STAG + "[" + i + "]");
            addData(
                    (earfcn == 0 ? "-" : earfcn),
                    (pci == 0 ? "" : pci),
                    (band == 0 ? "" : (band + "(LTE-U)")),
                    (bandwidth == 0 ? "" : (float) bandwidth / 10),
                    (stag == 0 ? "" : stag)
            );
        }
    }

}

class EMMCallInformation extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EMM_CALL_INFO_IND};

    public EMMCallInformation(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "EMM Call information";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Bearer active", "PS instance", "CS instance",
                "SMS establishment trigger", "Keep SMS establishment trigger",
                "Paging trigger", "Re-establishment request",
                "Call type(SR/EXSR)", "Establishment type(R11)",
                "RRC Establishment cause(R12)", "RRC Establishment type(R12)",
                "# of ESM msg's", "# of SMS msg's", "CS MO Trigger",
                "CS MT Trigger"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int bearerActive = getFieldValue(data, MDMContent.IS_BEARER_ACTIVE);
        int psInstance = getFieldValue(data, MDMContent.IS_PS_MO_MT_INSTANCE);
        int csInstance = getFieldValue(data, MDMContent.IS_CS_MO_MT_INSTANCE);
        int smsEstTrigger = getFieldValue(data, MDMContent.IS_SMS_EST_TRIGGER);
        int keepSmsEstTrigger = getFieldValue(data, MDMContent.IS_KEEP_SMS_EST);
        int pageTrigger = getFieldValue(data, MDMContent.PAGE_INDE_FLG);
        int reEstReq = getFieldValue(data, MDMContent.REEST_REQ_FLG);
        int callType = getFieldValue(data, MDMContent.CALL_TYPE);
        int EstType = getFieldValue(data, MDMContent.EST_CAUSE);
        int rrcEstCause = getFieldValue(data, MDMContent.ESTABLISHMENT_CAUSE);
        int rrcEstType = getFieldValue(data, MDMContent.ESTABLISHMENT_TYPE);
        int emsMsg = getFieldValue(data, MDMContent.WAIT_SND_ESM_MSG_NUM);
        int smsMsg = getFieldValue(data, MDMContent.WAIT_SND_SMS_MSG_NUM);
        int csMoTrigger = getFieldValue(data, MDMContent.CS_MO_TRIGGER);
        int csMtTrigger = getFieldValue(data, MDMContent.CS_MT_TRIGGER);
        addData(bearerActive, psInstance, csInstance, smsEstTrigger,
                keepSmsEstTrigger, pageTrigger, reEstReq, callType, EstType,
                rrcEstCause, rrcEstType, emsMsg, smsMsg, csMoTrigger,
                csMtTrigger);
    }

}

class PCellSCellBasicInfo extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND,
            MDMContent.MSG_ID_EM_EL1_STATUS_IND};

    HashMap<Integer, String> mMappingBW = new HashMap<Integer, String>() {
        {
            put(14, "1.4");
            put(30, "3");
            put(50, "5");
            put(100, "10");
            put(150, "15");
            put(200, "20");
            put(0xFF, "");
        }
    };
    HashMap<Integer, String> mMappingQam = new HashMap<Integer, String>() {
        {
            put(1, "QPSK");
            put(2, "16QAM");
            put(3, "64QAM");
            put(4, "256QAM");
        }
    };

    long[] earfcnPcell = new long[3];
    long[][] earfcnScellArr = new long[3][3];
    int[] bandPcell = new int[3];
    int[][] bandScellArr = new int[3][3];
    int[] dlBandwidthPcell = new int[3];
    int[][] dlBandwidthScellArr = new int[3][3];
    int[] ulBandwidthPcell = new int[3];
    int[][] ulBandwidthScellArr = new int[3][3];
    int[] tmPcell = new int[3];
    int[][] tmScellArr = new int[3][3];
    int[] pciPcell = new int[3];
    int[][] pciScellArr = new int[3][3];
    int[] snrPcell = new int[3];
    int[][] snrScellArr = new int[3][3];
    int[] dlFreqPcell = new int[3];
    int[][] dlFreqScellArr = new int[3][3];
    int[] ulFreqPcell = new int[3];
    int[][] ulFreqScellArr = new int[3][3];
    int[] rsrpPcell = new int[3];
    int[][] rsrpScellArr = new int[3][3];
    int[] rsrqPcell = new int[3];
    int[][] rsrqScellArr = new int[3][3];
    int[] dlMod0Pcell = new int[3];
    int[] oldDlMod0Pcell = new int[3];
    int[][] dlMod0ScellArr = new int[3][3];
    int[][] oldDlMod0ScellArr = new int[3][3];
    int[] dlMod1Pcell = new int[3];
    int[] oldDlMod1Pcell = new int[3];
    int[][] dlMod1ScellArr = new int[3][3];
    int[][] oldDlMod1ScellArr = new int[3][3];
    int[] ulPcell = new int[3];
    int[] oldUlPcell = new int[3];
    int[][] ulScellArr = new int[3][3];
    int[][] oldUlScellArr = new int[3][3];


    int[][] DlCellAntPort = new int[3][4];
    int[][] DlCellTput = new int[3][4];
    int[][] UlCellTput = new int[3][4];
    int[][] DlCellImcs = new int[3][4];
    int[][] UlCellImcs = new int[3][4];
    int[][] DlCellBler = new int[3][4];
    int[][] UlCellBler = new int[3][4];
    int[][] DlCellRb = new int[3][4];
    int[][] UlCellRb = new int[3][4];
    int[][] DlCellCqiCw0 = new int[3][4];
    int[][] DlCellCqiCw1 = new int[3][4];
    int[][] DlCellri = new int[3][4];

    String StartTime = null;
    String[] lastTime = new String[2];
    String FileNamePS = "_Pcell_and_Scell_basic_info.txt";
    String title = "Time,Pcell_Band, Pcell_DL BW, Pcell_UL BW, Pcell_TM, Pcell_PCI, " +
            "Pcell_EARFCN, Pcell_SNR, Pcell_DL_Freq, Pcell_UL_Freq, Pcell_RSRP, Pcell_RSRQ," +
            "Pcell_ant_port,Pcell_DL_Tput,Pcell_UL_Tput,Pcell_DL_Imcs,Pcell_UL_Imcs," +
            "Pcell_DL_bler,Pcell_UL_bler,Pcell_DL_rb,Pcell_UL_rb,Pcell_cqi_cw0," +
            "Pcell_cqi_cw1,Pcell_ri," +
            "Scell0_Band, Scell0_DL BW, Scell0_UL BW, Scell0_TM, Scell0_PCI, Scell0_EARFCN," +
            "Scell0_SNR, Scell0_DL_Freq, Scell0_UL_Freq, Scell0_RSRP, Scell0_RSRQ, " +
            "Scell0_ant_port,Scell0_DL_Tput,Scell0_UL_Tput,Scell0_DL_Imcs,Scell0_UL_Imcs," +
            "Scell0_DL_bler,Scell0_UL_bler,Scell0_DL_rb,Scell0_UL_rb,Scell0_cqi_cw0," +
            "Scell0_cqi_cw1,Scell0_ri," +
            "Scell1_Band,Scell1_DL BW, Scell1_UL BW, Scell1_TM, Scell1_PCI, Scell1_EARFCN, " +
            "Scell1_SNR,Scell1_DL_Freq, Scell1_UL_Freq, Scell1_RSRP, Scell1_RSRQ, " +
            "Scell1_ant_port,Scell1_DL_Tput,Scell1_UL_Tput,Scell1_DL_Imcs,Scell1_UL_Imcs," +
            "Scell1_DL_bler,Scell1_UL_bler,Scell1_DL_rb,Scell1_UL_rb,Scell1_cqi_cw0," +
            "Scell1_cqi_cw1,Scell1_ri," +
            "Scell2_Band,Scell2_DL BW, Scell2_UL BW, Scell2_TM, Scell2_PCI, Scell2_EARFCN, " +
            "Scell2_SNR,Scell2_DL_Freq, Scell2_UL_Freq,Scell2_RSRP, Scell2_RSRQ," +
            "Scell2_ant_port,Scell2_DL_Tput,Scell2_UL_Tput,Scell2_DL_Imcs,Scell2_UL_Imcs," +
            "Scell2_DL_bler,Scell2_UL_bler,Scell2_DL_rb,Scell2_UL_rb,Scell2_cqi_cw0," +
            "Scell2_cqi_cw1,Scell2_ri";
    String FileNamePS1 = null;
    String FileNamePS2 = null;
    boolean isFirstTimeRecord = true;

    public PCellSCellBasicInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Pcell and Scell basic info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    String[] getLabels() {
        return new String[]{"", "Pcell", "Scell0", "Scell1", "Scell2"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }


    void LogRecord(int sim_idx) {
        String Pcell_log[] = new String[2];
        String[][] Scell_log = new String[2][3];
        String[] CurTime = new String[2];

        String[] Content = new String[2];
        String path = "/Download";
        if (ComponentSelectActivity.mAutoRecordFlag.equals("0")) {
            Elog.d(TAG, "Do not save info");
            return;
        }
        if (isFirstTimeRecord == true) {
            isFirstTimeRecord = false;
            StartTime = getCurrectTime();
            try {
                FileNamePS1 = StartTime + "_ps_1_"
                        + MDMComponentDetailActivity.mSimMccMnc[0] + FileNamePS;
                FileNamePS2 = StartTime + "_ps_2_"
                        + MDMComponentDetailActivity.mSimMccMnc[1] + FileNamePS;
                saveToSDCard(path, FileNamePS1, title, false);
                saveToSDCard(path, FileNamePS2, title, false);
                lastTime[0] = StartTime;
                lastTime[1] = StartTime;
            } catch (Exception e) {
                e.printStackTrace();
            }
            Elog.d(TAG, "isFirstTimeRecord = " + isFirstTimeRecord + "," + title);
        }
        CurTime[sim_idx] = getCurrectTime();
        Pcell_log[sim_idx] = CurTime[sim_idx] + " ,"
                + bandPcell[sim_idx] + ","
                + mMappingBW.get(dlBandwidthPcell[sim_idx]) + ","
                + mMappingBW.get(ulBandwidthPcell[sim_idx]) + ","
                + tmPcell[sim_idx] + ","
                + pciPcell[sim_idx] + ","
                + earfcnPcell[sim_idx] + ","
                + (float) snrPcell[sim_idx] / 4 + ","
                + dlFreqPcell[sim_idx] + ","
                + ulFreqPcell[sim_idx] + ","
                + (float) rsrpPcell[sim_idx] / 4 + ","
                + (float) rsrqPcell[sim_idx] / 4 + ","
                + DlCellAntPort[sim_idx][0] + ","
                + DlCellTput[sim_idx][0] + ","
                + UlCellTput[sim_idx][0] + ","
                + DlCellImcs[sim_idx][0] + ","
                + UlCellImcs[sim_idx][0] + ","
                + DlCellBler[sim_idx][0] + ","
                + UlCellBler[sim_idx][0] + ","
                + DlCellRb[sim_idx][0] + ","
                + UlCellRb[sim_idx][0] + ","
                + DlCellCqiCw0[sim_idx][0] + ","
                + DlCellCqiCw1[sim_idx][0] + ","
                + DlCellri[sim_idx][0];

        Scell_log[sim_idx][0] = bandScellArr[sim_idx][0] + ","
                + mMappingBW.get(dlBandwidthScellArr[sim_idx][0]) + ","
                + mMappingBW.get(ulBandwidthScellArr[sim_idx][0]) + ","
                + tmScellArr[sim_idx][0] + ","
                + pciScellArr[sim_idx][0] + ","
                + earfcnScellArr[sim_idx][0] + ","
                + (float) snrScellArr[sim_idx][0] / 4 + ","
                + dlFreqScellArr[sim_idx][0] + ","
                + ulFreqScellArr[sim_idx][0] + ","
                + (float) rsrpScellArr[sim_idx][0] / 4 + ","
                + (float) rsrqScellArr[sim_idx][0] / 4 + ","
                + DlCellAntPort[sim_idx][1] + ","
                + DlCellTput[sim_idx][1] + ","
                + UlCellTput[sim_idx][1] + ","
                + DlCellImcs[sim_idx][1] + ","
                + UlCellImcs[sim_idx][1] + ","
                + DlCellBler[sim_idx][1] + ","
                + UlCellBler[sim_idx][1] + ","
                + DlCellRb[sim_idx][1] + ","
                + UlCellRb[sim_idx][1] + ","
                + DlCellCqiCw0[sim_idx][1] + ","
                + DlCellCqiCw1[sim_idx][1] + ","
                + DlCellri[sim_idx][1];

        Scell_log[sim_idx][1] = bandScellArr[sim_idx][1] + ","
                + mMappingBW.get(dlBandwidthScellArr[sim_idx][1]) + ","
                + mMappingBW.get(ulBandwidthScellArr[sim_idx][1]) + ","
                + tmScellArr[sim_idx][1] + ","
                + pciScellArr[sim_idx][1] + ","
                + earfcnScellArr[sim_idx][1] + ","
                + (float) snrScellArr[sim_idx][1] / 4 + ","
                + dlFreqScellArr[sim_idx][1] + ","
                + ulFreqScellArr[sim_idx][1] + ","
                + (float) rsrpScellArr[sim_idx][1] / 4 + ","
                + (float) rsrqScellArr[sim_idx][1] / 4 + ","
                + DlCellAntPort[sim_idx][2] + ","
                + DlCellTput[sim_idx][2] + ","
                + UlCellTput[sim_idx][2] + ","
                + DlCellImcs[sim_idx][2] + ","
                + UlCellImcs[sim_idx][2] + ","
                + DlCellBler[sim_idx][2] + ","
                + UlCellBler[sim_idx][2] + ","
                + DlCellRb[sim_idx][2] + ","
                + UlCellRb[sim_idx][2] + ","
                + DlCellCqiCw0[sim_idx][2] + ","
                + DlCellCqiCw1[sim_idx][2] + ","
                + DlCellri[sim_idx][2];

        Scell_log[sim_idx][2] = bandScellArr[sim_idx][2] + ","
                + mMappingBW.get(dlBandwidthScellArr[sim_idx][2]) + ","
                + mMappingBW.get(ulBandwidthScellArr[sim_idx][2]) + ","
                + tmScellArr[sim_idx][2] + ","
                + pciScellArr[sim_idx][2] + ","
                + earfcnScellArr[sim_idx][2] + ","
                + (float) snrScellArr[sim_idx][2] / 4 + ","
                + dlFreqScellArr[sim_idx][2] + ","
                + ulFreqScellArr[sim_idx][2] + ","
                + (float) rsrpScellArr[sim_idx][2] / 4 + ","
                + (float) rsrqScellArr[sim_idx][2] / 4 + ","
                + DlCellAntPort[sim_idx][3] + ","
                + DlCellTput[sim_idx][3] + ","
                + UlCellTput[sim_idx][3] + ","
                + DlCellImcs[sim_idx][3] + ","
                + UlCellImcs[sim_idx][3] + ","
                + DlCellBler[sim_idx][3] + ","
                + UlCellBler[sim_idx][3] + ","
                + DlCellRb[sim_idx][3] + ","
                + UlCellRb[sim_idx][3] + ","
                + DlCellCqiCw0[sim_idx][3] + ","
                + DlCellCqiCw1[sim_idx][3] + ","
                + DlCellri[sim_idx][3];

        Content[sim_idx] = Pcell_log[sim_idx] + "," + Scell_log[sim_idx][0] + "," +
                Scell_log[sim_idx][1] + "," + Scell_log[sim_idx][2];

        try {
            String sFlag = SystemProperties.get("persist.radio.record.time", "5");
            int recordTimer = Integer.parseInt(sFlag);

            if (diff_time(lastTime[sim_idx], CurTime[sim_idx],
                    recordTimer * 1000) == true) {
                lastTime[sim_idx] = CurTime[sim_idx];
                Elog.d(TAG, "recordTimer " + recordTimer);

                if (sim_idx == 0) {
                    Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS1);
                    saveToSDCard(path, FileNamePS1, Content[sim_idx], true);
                } else if (sim_idx == 1) {
                    Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS2);
                    saveToSDCard(path, FileNamePS2, Content[sim_idx], true);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int sim_idx = data.getSimIdx() - 1;
        if (name.equals(MDMContent.MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND)) {
            String coName = MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SERVING_INFO + ".";
            String coName1 = MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_LIST
                    + "." + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO + "[";
            earfcnPcell[sim_idx] = getFieldValue(data, coName
                    + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_EARFCN);
            pciPcell[sim_idx] = getFieldValue(data, coName
                    + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_PCI);
            rsrpPcell[sim_idx] = getFieldValue(data, coName
                    + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_RSRP, true);
            rsrqPcell[sim_idx] = getFieldValue(data, coName
                    + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_RSRQ, true);
            snrPcell[sim_idx] = getFieldValue(data, coName
                    + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_RS_SNR_IN_QDB, true);
            bandPcell[sim_idx] = getFieldValue(data, coName
                    + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SERV_LTE_BAND);
            for (int i = 0; i < 3; i++) {
                earfcnScellArr[sim_idx][i] = getFieldValue(data, coName1 + i + "]."
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_EARFCN);
                pciScellArr[sim_idx][i] = getFieldValue(data, coName1 + i + "]."
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_PCI);
                rsrpScellArr[sim_idx][i] = getFieldValue(data, coName1 + i + "]."
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_RSRP, true);
                rsrqScellArr[sim_idx][i] = getFieldValue(data, coName1 + i + "]."
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_RSRQ, true);
                snrScellArr[sim_idx][i] = getFieldValue(data, coName1 + i + "]."
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_RS_SNR_IN_QDB, true);
                bandScellArr[sim_idx][i] = getFieldValue(data, coName1 + i + "]."
                        + MDMContent.EM_ERRC_MOB_MEAS_INTRARAT_SCELL_INFO_SERV_LTE_BAND);
            }
        } else if (name.equals(MDMContent.MSG_ID_EM_EL1_STATUS_IND)) {
            String coName = MDMContent.EM_EL1_STATUS_CELL_INFO + "[0].";
            String coName1 = MDMContent.EM_EL1_STATUS_CELL_INFO + "[";
            dlBandwidthPcell[sim_idx] = getFieldValue(data, coName
                    + MDMContent.EM_EL1_STATUS_CELL_INFO_DL_BW);
            ulBandwidthPcell[sim_idx] = getFieldValue(data, coName
                    + MDMContent.EM_EL1_STATUS_CELL_INFO_UL_BW);
            tmPcell[sim_idx] = getFieldValue(data, coName
                    + MDMContent.EM_EL1_STATUS_CELL_INFO_TM);
            dlFreqPcell[sim_idx] = getFieldValue(data, coName + "dlFreq");
            ulFreqPcell[sim_idx] = getFieldValue(data, coName + "ulFreq");
            dlMod0Pcell[sim_idx] = getFieldValue(data, "dl_info[0]." + "DL_Mod0");
            oldDlMod0Pcell[sim_idx] = (dlMod0Pcell[sim_idx] == 0xFF) ?
                    oldDlMod0Pcell[sim_idx] : dlMod0Pcell[sim_idx];

            dlMod1Pcell[sim_idx] = getFieldValue(data, "dl_info[0]." + "DL_Mod1");
            oldDlMod1Pcell[sim_idx] = (dlMod1Pcell[sim_idx] == 0xFF) ?
                    oldDlMod1Pcell[sim_idx] : dlMod1Pcell[sim_idx];

            ulPcell[sim_idx] = getFieldValue(data, "ul_info[0]." + "UL_Mod");
            oldUlPcell[sim_idx] = (ulPcell[sim_idx] == 0xFF) ?
                    oldUlPcell[sim_idx] : ulPcell[sim_idx];

            for (int i = 0; i < 3; i++) {
                dlBandwidthScellArr[sim_idx][i] = getFieldValue(data, coName1 + (i + 1)
                        + "]." + MDMContent.EM_EL1_STATUS_CELL_INFO_DL_BW);
                ulBandwidthScellArr[sim_idx][i] = getFieldValue(data, coName1 + (i + 1)
                        + "]." + MDMContent.EM_EL1_STATUS_CELL_INFO_UL_BW);
                tmScellArr[sim_idx][i] = getFieldValue(data, coName1 + (i + 1) + "]."
                        + MDMContent.EM_EL1_STATUS_CELL_INFO_TM);
                dlFreqScellArr[sim_idx][i] = getFieldValue(data, coName1 + (i + 1) + "].dlFreq");
                ulFreqScellArr[sim_idx][i] = getFieldValue(data, coName1 + (i + 1) + "].ulFreq");
                dlMod0ScellArr[sim_idx][i] = getFieldValue(data, "dl_info[" + (i + 1)
                        + "].DL_Mod0");
                oldDlMod0ScellArr[sim_idx][i] = (dlMod0ScellArr[sim_idx][i] == 0xFF) ?
                        oldDlMod0ScellArr[sim_idx][i] : dlMod0ScellArr[sim_idx][i];
                dlMod1ScellArr[sim_idx][i] = getFieldValue(data, "dl_info[" + (i + 1)
                        + "].DL_Mod1");
                oldDlMod1ScellArr[sim_idx][i] = (dlMod1ScellArr[sim_idx][i] == 0xFF) ?
                        oldDlMod1ScellArr[sim_idx][i] : dlMod1ScellArr[sim_idx][i];
                ulScellArr[sim_idx][i] = getFieldValue(data, "ul_info[" + (i + 1)
                        + "].UL_Mod");
                oldUlScellArr[sim_idx][i] = (ulScellArr[sim_idx][i] == 0xFF) ?
                        oldUlScellArr[sim_idx][i] : ulScellArr[sim_idx][i];
                if (bandScellArr[sim_idx][i] == 0) {
                    oldDlMod0ScellArr[sim_idx][i] = 0;
                    oldDlMod1ScellArr[sim_idx][i] = 0;
                    oldUlScellArr[sim_idx][i] = 0;
                }
            }

            for (int i = 0; i < 4; i++) {
                String coNamedl = MDMContent.EM_EL1_STATUS_DL_INFO + "[" + i + "].";
                String coNameul = MDMContent.EM_EL1_STATUS_UL_INFO + "[" + i + "].";
                String coNamecell = MDMContent.EM_EL1_STATUS_CELL_INFO + "[" + i + "].";

                DlCellAntPort[sim_idx][i] = getFieldValue(data, coNamecell
                        + MDMContent.EM_EL1_STATUS_CELL_INFO_ANT_PORT, true);
                DlCellTput[sim_idx][i] = getFieldValue(data, coNamedl
                        + MDMContent.EM_EL1_STATUS_DL_INFO_DL_TPUT);
                UlCellTput[sim_idx][i] = getFieldValue(data, coNameul
                        + MDMContent.EM_EL1_STATUS_UL_INFO_UL_TPUT);
                DlCellImcs[sim_idx][i] = getFieldValue(data, coNamedl
                        + MDMContent.EM_EL1_STATUS_DL_INFO_DL_IMCS, true);
                UlCellImcs[sim_idx][i] = getFieldValue(data, coNameul
                        + MDMContent.EM_EL1_STATUS_UL_INFO_UL_IMCS, true);
                DlCellBler[sim_idx][i] = getFieldValue(data, coNamedl
                        + MDMContent.EM_EL1_STATUS_DL_INFO_DL_BLER, true);
                UlCellBler[sim_idx][i] = getFieldValue(data, coNameul
                        + MDMContent.EM_EL1_STATUS_UL_INFO_UL_BLER, true);
                DlCellRb[sim_idx][i] = getFieldValue(data, coNamedl
                        + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RB, true);
                UlCellRb[sim_idx][i] = getFieldValue(data, coNameul
                        + MDMContent.EM_EL1_STATUS_UL_INFO_UL_RB, true);
                DlCellCqiCw0[sim_idx][i] = getFieldValue(data, coNamedl
                        + MDMContent.EM_EL1_STATUS_DL_INFO_CQI_CW0, true);
                DlCellCqiCw1[sim_idx][i] = getFieldValue(data, coNamedl
                        + MDMContent.EM_EL1_STATUS_DL_INFO_CQI_CW1, true);
                DlCellri[sim_idx][i] = getFieldValue(data, coNamedl
                        + MDMContent.EM_EL1_STATUS_DL_INFO_DL_RI);
            }
        } else {
            Elog.d(TAG, "PCellSCellBasicInfo update invalid name");
            return;
        }

        if (ComponentSelectActivity.mAutoRecordFlag.equals("1") && (sim_idx < 2)) {
            LogRecord(sim_idx);
        }
        if ((sim_idx + 1) != MDMComponentDetailActivity.mModemType) {
            return;
        }
        clearData();
        // Band
        addData("Band", bandPcell[sim_idx],
                bandScellArr[sim_idx][0],
                bandScellArr[sim_idx][1],
                bandScellArr[sim_idx][2]);

        // DL BW
        addData("DL BW(MHz)", mMappingBW.get(dlBandwidthPcell[sim_idx]),
                mMappingBW.get(dlBandwidthScellArr[sim_idx][0]),
                mMappingBW.get(dlBandwidthScellArr[sim_idx][1]),
                mMappingBW.get(dlBandwidthScellArr[sim_idx][2]));
        // UL BW
        addData("UL BW(MHz)", mMappingBW.get(ulBandwidthPcell[sim_idx]),
                mMappingBW.get(ulBandwidthScellArr[sim_idx][0]),
                mMappingBW.get(ulBandwidthScellArr[sim_idx][1]),
                mMappingBW.get(ulBandwidthScellArr[sim_idx][2]));
        // TM
        addData("TM", tmPcell[sim_idx], tmScellArr[sim_idx][0],
                tmScellArr[sim_idx][1], tmScellArr[sim_idx][2]);
        // PCI
        addData("PCI", pciPcell[sim_idx],
                pciScellArr[sim_idx][0],
                pciScellArr[sim_idx][1],
                pciScellArr[sim_idx][2]);

        // EARFCN
        addData("EARFCN", earfcnPcell[sim_idx],
                earfcnScellArr[sim_idx][0],
                earfcnScellArr[sim_idx][1],
                earfcnScellArr[sim_idx][2]);

        // RS_SNR
        addData("RS_SNR",
                (float) snrPcell[sim_idx] / 4,
                (float) snrScellArr[sim_idx][0] / 4,
                (float) snrScellArr[sim_idx][1] / 4,
                (float) snrScellArr[sim_idx][2] / 4);

        // DL Freq
        addData("DL Freq", dlFreqPcell[sim_idx],
                dlFreqScellArr[sim_idx][0],
                dlFreqScellArr[sim_idx][1],
                dlFreqScellArr[sim_idx][2]);
        // UL Freq
        addData("UL Freq", ulFreqPcell[sim_idx],
                ulFreqScellArr[sim_idx][0],
                ulFreqScellArr[sim_idx][1],
                ulFreqScellArr[sim_idx][2]);

        // RSRP
        addData("RSRP", (float) rsrpPcell[sim_idx] / 4,
                (float) rsrpScellArr[sim_idx][0] / 4,
                (float) rsrpScellArr[sim_idx][1] / 4,
                (float) rsrpScellArr[sim_idx][2] / 4);

        // RSRQ
        addData("RSRQ", (float) rsrqPcell[sim_idx] / 4,
                (float) rsrqScellArr[sim_idx][0] / 4,
                (float) rsrqScellArr[sim_idx][1] / 4,
                (float) rsrqScellArr[sim_idx][2] / 4);

        // DL Modulation TB1
        addData("DL Mod TB1", mMappingQam.get(oldDlMod0Pcell[sim_idx]),
                mMappingQam.get(oldDlMod0ScellArr[sim_idx][0]),
                mMappingQam.get(oldDlMod0ScellArr[sim_idx][1]),
                mMappingQam.get(oldDlMod0ScellArr[sim_idx][2]));

        // DL Modulation TB2
        addData("DL Mod TB2", mMappingQam.get(oldDlMod1Pcell[sim_idx]),
                mMappingQam.get(oldDlMod1ScellArr[sim_idx][0]),
                mMappingQam.get(oldDlMod1ScellArr[sim_idx][1]),
                mMappingQam.get(oldDlMod1ScellArr[sim_idx][2]));

        // UL Modulation
        addData("UL Mod", mMappingQam.get(oldUlPcell[sim_idx]),
                mMappingQam.get(oldUlScellArr[sim_idx][0]),
                mMappingQam.get(oldUlScellArr[sim_idx][1]),
                mMappingQam.get(oldUlScellArr[sim_idx][2]));
    }
}


class SpeechCodecInfo extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]
            {MDMContent.MSG_ID_EM_SPEECH_INFO_SPH_CODEC_IND};
    HashMap<Integer, String> mMappingSpeechCodec = new HashMap<Integer, String>() {
        {
            put(0, "SPH_CODEC_FR");
            put(1, "SPH_CODEC_HR");
            put(2, "SPH_CODEC_EFR");
            put(3, "SPH_CODEC_AMR_12_20");
            put(4, "SPH_CODEC_AMR_10_20");
            put(5, "SPH_CODEC_AMR_7_95");
            put(6, "SPH_CODEC_AMR_7_40");
            put(7, "SPH_CODEC_AMR_6_70");
            put(8, "SPH_CODEC_AMR_5_90");
            put(9, "SPH_CODEC_AMR_5_15");
            put(10, "SPH_CODEC_AMR_4_75");
            put(11, "SPH_CODEC_AMR_SID");
            put(12, "SPH_CODEC_AMR_NODATA");
            put(32, "SPH_CODEC_AMRWB_6_60");
            put(33, "SPH_CODEC_AMRWB_8_85");
            put(34, "SPH_CODEC_AMRWB_12_65");
            put(35, "SPH_CODEC_AMRWB_14_25");
            put(36, "SPH_CODEC_AMRWB_15_85");
            put(37, "SPH_CODEC_AMRWB_18_25");
            put(38, "SPH_CODEC_AMRWB_19_85");
            put(39, "SPH_CODEC_AMRWB_23_05");
            put(40, "SPH_CODEC_AMRWB_23_85");
            put(41, "SPH_CODEC_AMRWB_SID");
            put(42, "SPH_CODEC_LOST_FRAME");
            put(48, "SPH_CODEC_C2K_SO1");
            put(49, "SPH_CODEC_C2K_SO3");
            put(50, "SPH_CODEC_C2K_SO17");
            put(51, "SPH_CODEC_C2K_SO68");
            put(52, "SPH_CODEC_C2K_SO73");
            put(53, "SPH_CODEC_C2K_SO73WB");
            put(96, "SPH_CODEC_G711");
            put(97, "SPH_CODEC_G722");
            put(98, "SPH_CODEC_G723_1");
            put(99, "SPH_CODEC_G726");
            put(100, "SPH_CODEC_G729");
            put(128, "SPH_CODEC_EVS_08K_005_9/SPH_CODEC_EVS_08K_002_8");
            put(129, "SPH_CODEC_EVS_08K_007_2");
            put(130, "SPH_CODEC_EVS_08K_008_0");
            put(131, "SPH_CODEC_EVS_08K_009_6");
            put(132, "SPH_CODEC_EVS_08K_013_2");
            put(133, "SPH_CODEC_EVS_08K_016_4");
            put(134, "SPH_CODEC_EVS_08K_024_4");
            put(135, "SPH_CODEC_EVS_08K_032_0");
            put(136, "SPH_CODEC_EVS_08K_048_0");
            put(137, "SPH_CODEC_EVS_08K_064_0");
            put(138, "SPH_CODEC_EVS_08K_096_0");
            put(139, "SPH_CODEC_EVS_08K_128_0");
            put(140, "SPH_CODEC_EVS_08K_002_4_SID");
            put(141, "SPH_CODEC_EVS_08K_000_0_REV");
            put(142, "SPH_CODEC_EVS_08K_000_0_LOST");
            put(143, "SPH_CODEC_EVS_08K_000_0_NODATA");
            put(144, "SPH_CODEC_EVS_16K_005_9/SPH_CODEC_EVS_16K_002_8");
            put(145, "SPH_CODEC_EVS_16K_007_2");
            put(146, "SPH_CODEC_EVS_16K_008_0");
            put(147, "SPH_CODEC_EVS_16K_009_6");
            put(148, "SPH_CODEC_EVS_16K_013_2");
            put(149, "SPH_CODEC_EVS_16K_016_4");
            put(150, "SPH_CODEC_EVS_16K_024_4");
            put(151, "SPH_CODEC_EVS_16K_032_0");
            put(152, "SPH_CODEC_EVS_16K_048_0");
            put(153, "SPH_CODEC_EVS_16K_064_0");
            put(154, "SPH_CODEC_EVS_16K_096_0");
            put(155, "SPH_CODEC_EVS_16K_128_0");
            put(156, "SPH_CODEC_EVS_16K_002_4_SID");
            put(157, "SPH_CODEC_EVS_16K_000_0_REV");
            put(158, "SPH_CODEC_EVS_16K_000_0_LOST");
            put(159, "SPH_CODEC_EVS_16K_000_0_NODATA");
            put(160, "SPH_CODEC_EVS_32K_005_9/SPH_CODEC_EVS_32K_002_8");
            put(161, "SPH_CODEC_EVS_32K_007_2");
            put(162, "SPH_CODEC_EVS_32K_008_0");
            put(163, "SPH_CODEC_EVS_32K_009_6");
            put(164, "SPH_CODEC_EVS_32K_013_2");
            put(165, "SPH_CODEC_EVS_32K_016_4");
            put(166, "SPH_CODEC_EVS_32K_024_4");
            put(167, "SPH_CODEC_EVS_32K_032_0");
            put(168, "SPH_CODEC_EVS_32K_048_0");
            put(169, "SPH_CODEC_EVS_32K_064_0");
            put(170, "SPH_CODEC_EVS_32K_096_0");
            put(171, "SPH_CODEC_EVS_32K_128_0");
            put(172, "SPH_CODEC_EVS_32K_002_4_SID");
            put(173, "SPH_CODEC_EVS_32K_000_0_REV");
            put(174, "SPH_CODEC_EVS_32K_000_0_LOST");
            put(175, "SPH_CODEC_EVS_32K_000_0_NODATA");
            put(176, "SPH_CODEC_EVS_48K_005_9/SPH_CODEC_EVS_48K_002_8");
            put(177, "SPH_CODEC_EVS_48K_007_2");
            put(178, "SPH_CODEC_EVS_48K_008_0");
            put(179, "SPH_CODEC_EVS_48K_009_6");
            put(180, "SPH_CODEC_EVS_48K_013_2");
            put(181, "SPH_CODEC_EVS_48K_016_4");
            put(182, "SPH_CODEC_EVS_48K_024_4");
            put(183, "SPH_CODEC_EVS_48K_032_0");
            put(184, "SPH_CODEC_EVS_48K_048_0");
            put(185, "SPH_CODEC_EVS_48K_064_0");
            put(186, "SPH_CODEC_EVS_48K_096_0");
            put(187, "SPH_CODEC_EVS_48K_128_0");
            put(188, "SPH_CODEC_EVS_48K_002_4_SID");
            put(189, "SPH_CODEC_EVS_48K_000_0_REV");
            put(190, "SPH_CODEC_EVS_48K_000_0_LOST");
            put(191, "SPH_CODEC_EVS_48K_000_0_NODATA");
            put(208, "SPH_CODEC_EVS_AWB_06_60");
            put(209, "SPH_CODEC_EVS_AWB_08_85");
            put(210, "SPH_CODEC_EVS_AWB_12_65");
            put(211, "SPH_CODEC_EVS_AWB_14_25");
            put(212, "SPH_CODEC_EVS_AWB_15_85");
            put(213, "SPH_CODEC_EVS_AWB_18_25");
            put(214, "SPH_CODEC_EVS_AWB_19_85");
            put(215, "SPH_CODEC_EVS_AWB_23_05");
            put(216, "SPH_CODEC_EVS_AWB_23_85");
            put(217, "SPH_CODEC_EVS_AWB_02_00_SID");
            put(218, "SPH_CODEC_EVS_AWB_00_00_REV0");
            put(219, "SPH_CODEC_EVS_AWB_00_00_REV1");
            put(220, "SPH_CODEC_EVS_AWB_00_00_REV2");
            put(221, "SPH_CODEC_EVS_AWB_00_00_REV3");
            put(222, "SPH_CODEC_EVS_AWB_00_00_LOST");
            put(223, "SPH_CODEC_EVS_AWB_00_00_NODATA");
            put(255, "SPH_CODEC_CODEC_NONE");
        }
    };

    HashMap<Integer, String> mMappingSpeechNetwork = new HashMap<Integer, String>() {
        {
            put(0, "SPH_NETWORK_IDLE");
            put(1, "SPH_NETWORK_2G_SPEECH_ON");
            put(2, "SPH_NETWORK_3G_SPEECH_ON");
            put(3, "SPH_NETWORK_3G324M_SPEECH_ON");
            put(4, "SPH_NETWORK_3G_SPEECH_CLOSING");
            put(5, "SPH_NETWORK_4G_SPEECH_ON");
            put(6, "SPH_NETWORK_4G_SPEECH_CLOSING");
        }
    };

    public SpeechCodecInfo(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "Speech Codec Info";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"UL speech codec", "DL speech codec", "speech network"};
    }

    @Override
    boolean supportMultiSIM() {
        return false;
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        clearData();
        int ulSphCodec = getFieldValue(data, MDMContent.EM_SPEECH_INFO_SPH_CODEC_ULSPH_CODEC);
        int dlSphCodec = getFieldValue(data, MDMContent.EM_SPEECH_INFO_SPH_CODEC_DLSPH_CODEC);
        int sphNetwork = getFieldValue(data, MDMContent.EM_SPEECH_INFO_SPH_CODEC_SPH_NETWORK);
        Elog.d(TAG, "ulSphCodec = " + ulSphCodec);
        Elog.d(TAG, "dlSphCodec = " + dlSphCodec);
        Elog.d(TAG, "sphNetwork = " + sphNetwork);
        addData(mMappingSpeechCodec.get(ulSphCodec));
        addData(mMappingSpeechCodec.get(dlSphCodec));
        addData(mMappingSpeechNetwork.get(sphNetwork));
    }
}

class RFCalibrationStatusCheck extends ArrayTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_MMRF_CDDC_INFO_IND,
    };

    public RFCalibrationStatusCheck(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "RF Calibration Status Check";
    }

    @Override
    String getGroup() {
        return "1. General EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    String[] getLabels() {
        return new String[]{"The Result:"};
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        char[] cddc_info = new char[512];
        for (int i = 0; i < 512; i++) {
            cddc_info[i] = (char) getFieldValue(data, "cddc_info[" + i + "]");
        }
        String cddc_info_s = new String(cddc_info);

        clearData();
        addData(new String[]{cddc_info_s.replace("\\n", "\n")});
    }
}


class LteErrcOosEvent extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_OOS_EVENT_IND};
    String StartTime = null;

    boolean isFirstTimeRecord = true;
    String FileNamePS = "_LTE_ERRC_OOS_Event_Info.txt";
    String FileNamePS1 = null;
    String FileNamePS2 = null;
    int[] oos_counter = new int[3];
    String title = "Time,oos_counter";

    public LteErrcOosEvent(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE ERRC OOS Event Info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"oos_counter"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    void LogRecord(int sim_idx) {
        String CurTime = null;

        String[] Content = new String[2];
        String path = "/Download";

        if (isFirstTimeRecord == true) {
            isFirstTimeRecord = false;
            StartTime = getCurrectTime();
            try {
                FileNamePS1 = StartTime + "_ps_1_"
                        + MDMComponentDetailActivity.mSimMccMnc[0] + FileNamePS;
                FileNamePS2 = StartTime + "_ps_2_"
                        + MDMComponentDetailActivity.mSimMccMnc[1] + FileNamePS;
                saveToSDCard(path, FileNamePS1, title, false);
                saveToSDCard(path, FileNamePS2, title, false);
            } catch (Exception e) {
                e.printStackTrace();
            }
            Elog.d(TAG, "isFirstTimeRecord = " + isFirstTimeRecord + "," + title);
        }
        CurTime = getCurrectTime();
        Content[sim_idx] = CurTime + "," + oos_counter[sim_idx];

        try {
            if (sim_idx == 0) {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS1);
                saveToSDCard(path, FileNamePS1, Content[sim_idx], true);
            } else {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS2);
                saveToSDCard(path, FileNamePS2, Content[sim_idx], true);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int sim_idx = data.getSimIdx() - 1;
        oos_counter[sim_idx] = getFieldValue(data, "oos_counter");
        Elog.d(TAG, "oos_counter = " + oos_counter[sim_idx]);
        if (ComponentSelectActivity.mAutoRecordFlag.equals("1") && (sim_idx < 2)) {
            LogRecord(sim_idx);
        }
        if ((sim_idx + 1) != MDMComponentDetailActivity.mModemType) {
            return;
        }
        clearData();
        addData(oos_counter[sim_idx]);
    }
}

class LteErrcRlfEvent extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_ERRC_RLF_EVENT_IND};
    String StartTime = null;
    boolean isFirstTimeRecord = true;
    String FileNamePS = "_LTE_ERRC_RLF_Event_Info.txt";
    String title = "Time,rlf_counter,rlf_cause,rlf_causeother";
    String FileNamePS1 = null;
    String FileNamePS2 = null;
    int[] rlf_counter = new int[3];
    int[] rlf_cause = new int[3];
    int[] rlf_causeother = new int[3];

    public LteErrcRlfEvent(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE ERRC ELF Event Info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"rlf_counter", "rlf_cause", "rlf_causeother"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    void LogRecord(int sim_idx) {
        String CurTime = null;

        String[] Content = new String[2];
        String path = "/Download";

        if (isFirstTimeRecord == true) {
            isFirstTimeRecord = false;
            StartTime = getCurrectTime();
            try {
                FileNamePS1 = StartTime + "_ps_1_"
                        + MDMComponentDetailActivity.mSimMccMnc[0] + FileNamePS;
                FileNamePS2 = StartTime + "_ps_2_"
                        + MDMComponentDetailActivity.mSimMccMnc[1] + FileNamePS;
                saveToSDCard(path, FileNamePS1, title, false);
                saveToSDCard(path, FileNamePS2, title, false);
            } catch (Exception e) {
                e.printStackTrace();
            }
            Elog.d(TAG, "isFirstTimeRecord = " + isFirstTimeRecord + "," + title);
        }
        CurTime = getCurrectTime();
        Content[sim_idx] = CurTime + "," + rlf_counter[sim_idx] + "," + rlf_cause[sim_idx] +
                "," + rlf_causeother[sim_idx];

        try {
            if (sim_idx == 0) {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS1);
                saveToSDCard(path, FileNamePS1, Content[sim_idx], true);
            } else {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS2);
                saveToSDCard(path, FileNamePS2, Content[sim_idx], true);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int sim_idx = data.getSimIdx() - 1;
        rlf_counter[sim_idx] = getFieldValue(data, "rlf_counter");
        rlf_cause[sim_idx] = getFieldValue(data, "rlf_cause");
        rlf_causeother[sim_idx] = getFieldValue(data, "rlf_causeother");
        Elog.d(TAG, "rlf_counter = " + rlf_counter[sim_idx]);
        Elog.d(TAG, "rlf_cause = " + rlf_cause[sim_idx]);
        Elog.d(TAG, "rlf_causeother = " + rlf_causeother[sim_idx]);

        if (ComponentSelectActivity.mAutoRecordFlag.equals("1") && (sim_idx < 2)) {
            LogRecord(sim_idx);
        }
        if ((sim_idx + 1) != MDMComponentDetailActivity.mModemType) {
            return;
        }
        clearData();
        addData(rlf_counter[sim_idx]);
        addData(rlf_cause[sim_idx]);
        addData(rlf_causeother[sim_idx]);
    }
}

class LteEmacRachFailure extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_EMAC_RACH_FAILURE_IND};
    String StartTime = null;
    boolean isFirstTimeRecord = true;
    String FileNamePS = "_LTE_EMAC_Rach_Failure_Info.txt";
    String title = "Time,num_of_rach_try";
    String FileNamePS1 = null;
    String FileNamePS2 = null;
    int[] num_of_rach_try = new int[3];

    public LteEmacRachFailure(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "LTE EMAC Rach Failure Info";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"num_of_rach_try"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    void LogRecord(int sim_idx) {
        String CurTime = null;

        String[] Content = new String[2];
        String path = "/Download";

        if (isFirstTimeRecord == true) {
            isFirstTimeRecord = false;
            StartTime = getCurrectTime();
            try {
                FileNamePS1 = StartTime + "_ps_1_"
                        + MDMComponentDetailActivity.mSimMccMnc[0] + FileNamePS;
                FileNamePS2 = StartTime + "_ps_2_"
                        + MDMComponentDetailActivity.mSimMccMnc[1] + FileNamePS;
                saveToSDCard(path, FileNamePS1, title, false);
                saveToSDCard(path, FileNamePS2, title, false);
            } catch (Exception e) {
                e.printStackTrace();
            }
            Elog.d(TAG, "isFirstTimeRecord = " + isFirstTimeRecord + "," + title);
        }
        CurTime = getCurrectTime();
        Content[sim_idx] = CurTime + "," + num_of_rach_try[sim_idx];

        try {
            if (sim_idx == 0) {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS1);
                saveToSDCard(path, FileNamePS1, Content[sim_idx], true);
            } else {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS2);
                saveToSDCard(path, FileNamePS2, Content[sim_idx], true);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int sim_idx = data.getSimIdx() - 1;
        num_of_rach_try[sim_idx] = getFieldValue(data, "num_of_rach_try");
        Elog.d(TAG, "num_of_rach_try = " + num_of_rach_try[sim_idx]);
        if (ComponentSelectActivity.mAutoRecordFlag.equals("1") && (sim_idx < 2)) {
            LogRecord(sim_idx);
        }
        if ((sim_idx + 1) != MDMComponentDetailActivity.mModemType) {
            return;
        }
        clearData();
        addData(num_of_rach_try[sim_idx]);
    }
}

class MDFeatureDetection extends NormalTableComponent {
    private static final String[] EM_TYPES = new String[]{
            MDMContent.MSG_ID_EM_L4C_MD_EVENT_IND};
    String StartTime = null;

    boolean isFirstTimeRecord = true;
    String FileNamePS = "_md_feature_detection.txt";
    String title = "Time,event";
    String FileNamePS1 = null;
    String FileNamePS2 = null;

    String[] event = new String[3];

    public MDFeatureDetection(Activity context) {
        super(context);
    }

    @Override
    String getName() {
        return "MD Feature Detection";
    }

    @Override
    String getGroup() {
        return "5. LTE EM Component";
    }

    @Override
    String[] getEmComponentName() {
        return EM_TYPES;
    }

    String[] getLabels() {
        return new String[]{"Event"};
    }

    @Override
    boolean supportMultiSIM() {
        return true;
    }

    void LogRecord(int sim_idx) {
        String CurTime = null;

        String[] Content = new String[2];
        String path = "/Download";

        if (isFirstTimeRecord == true) {
            isFirstTimeRecord = false;
            StartTime = getCurrectTime();
            try {
                FileNamePS1 = StartTime + "_ps_1_"
                        + MDMComponentDetailActivity.mSimMccMnc[0] + FileNamePS;
                FileNamePS2 = StartTime + "_ps_2_"
                        + MDMComponentDetailActivity.mSimMccMnc[1] + FileNamePS;
                saveToSDCard(path, FileNamePS1, title, false);
                saveToSDCard(path, FileNamePS2, title, false);
            } catch (Exception e) {
                e.printStackTrace();
            }
            Elog.d(TAG, "isFirstTimeRecord = " + isFirstTimeRecord + "," + title);
        }
        CurTime = getCurrectTime();
        Content[sim_idx] = CurTime + "," + event[sim_idx].trim();

        try {
            if (sim_idx == 0) {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS1);
                saveToSDCard(path, FileNamePS1, Content[sim_idx], true);
            } else {
                Elog.d(TAG, "save " + Content[sim_idx] + " to " + FileNamePS2);
                saveToSDCard(path, FileNamePS2, Content[sim_idx], true);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    void update(String name, Object msg) {
        Msg data = (Msg) msg;
        int sim_idx = data.getSimIdx() - 1;

        event[sim_idx] = "";
        if (MDMContent.MSG_ID_EM_L4C_MD_EVENT_IND.equals(name)) {
            for (int i = 0; i < 50; i++) {
                char info = (char) getFieldValue(data, "event[" + i + "]");
                event[sim_idx] += info;
            }
            Elog.d(TAG, "MD Event Lte,event = " + event[sim_idx].trim());
        }

        if (ComponentSelectActivity.mAutoRecordFlag.equals("1") && (sim_idx < 2)) {
            LogRecord(sim_idx);
        }

        if ((sim_idx + 1) != MDMComponentDetailActivity.mModemType) {
            return;
        }

        clearData();
        addData(event[sim_idx].trim());
    }
}
