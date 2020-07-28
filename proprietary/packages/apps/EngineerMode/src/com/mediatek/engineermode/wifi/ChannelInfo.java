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

package com.mediatek.engineermode.wifi;

import android.widget.ArrayAdapter;

import com.mediatek.engineermode.Elog;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * Class to hold channel information.
 *
 */
public class ChannelInfo {
    private static final String TAG = "WifiChannelInfo";
    public static final int CHANNEL_NUMBER_14 = 14;
    private static final int DEFAULT_CHANNEL_COUNT = 11;
    private static final int MAX_CHANNEL_COUNT = 75;
    static final int BW_20M = 1;
    static final int BW_40M = 2;
    static final int BW_80M = 4;
    static final int BW_160M = 8;
    static final int CHANNEL_2DOT4G = 10;
    static final int CHANNEL_5G = 11;

    /**
     * channel data class.
     * @author: mtk
     */
    private static class ChannelData {
        public int id;
        public String name;
        public int frequency;
        public int bandwidth;
        public int sequence;

        /**
         * Construct function.
         */
        public ChannelData() {}

        /**
         * COnstruct function.
         * @param id channel ID
         * @param name channel name
         * @param frequency channel frequency
         */
        public ChannelData(int id, String name, int frequency) {
            this(id, name, frequency, BW_20M);
        }
        /**
         * constructor.
         * @param id channel id
         * @param name channel name
         * @param frequency channel frequency
         * @param bandwidth channel bandwidth belong to
         */
        public ChannelData(int id, String name, int frequency, int bandwidth) {
            this.id = id;
            this.name = name;
            this.frequency = frequency;
            this.bandwidth = bandwidth;
        }
    }

    private String mChannelSelect = null;
    protected static long[] sChannels = null;
    protected static boolean sHas14Ch = false;
    protected static boolean sHasUpper14Ch = false;

    private static final int[] sBw40mUnsupported2dot4GChannels = {1, 2, 12, 13, 14};

    private static final ChannelData[] sWifiChannelDatas = {
        new ChannelData(1, "Channel 1 [2412MHz]", 2412000, BW_20M),
        new ChannelData(2, "Channel 2 [2417MHz]", 2417000, BW_20M),
        new ChannelData(3, "Channel 3 [2422MHz]", 2422000, BW_20M | BW_40M),
        new ChannelData(4, "Channel 4 [2427MHz]", 2427000, BW_20M | BW_40M),
        new ChannelData(5, "Channel 5 [2432MHz]", 2432000, BW_20M | BW_40M),
        new ChannelData(6, "Channel 6 [2437MHz]", 2437000, BW_20M | BW_40M),
        new ChannelData(7, "Channel 7 [2442MHz]", 2442000, BW_20M | BW_40M),
        new ChannelData(8, "Channel 8 [2447MHz]", 2447000, BW_20M | BW_40M),
        new ChannelData(9, "Channel 9 [2452MHz]", 2452000, BW_20M | BW_40M),
        new ChannelData(10, "Channel 10 [2457MHz]", 2457000, BW_20M | BW_40M),
        new ChannelData(11, "Channel 11 [2462MHz]", 2462000, BW_20M | BW_40M),
        new ChannelData(12, "Channel 12 [2467MHz]", 2467000, BW_20M),
        new ChannelData(13, "Channel 13 [2472MHz]", 2472000, BW_20M),
        new ChannelData(14, "Channel 14 [2484MHz]", 2484000, BW_20M),
        new ChannelData(18, "Channel 8 5G [5040MHz]", 5040000, BW_20M),     //8+10==18
        new ChannelData(20, "Channel 10 5G [5050MHz]", 5050000, BW_40M),    //10+10=20
        new ChannelData(22, "Channel 12 5G [5060MHz]", 5060000, BW_20M),    //12+10=22
        new ChannelData(26, "Channel 16 5G [5080MHz]", 5080000, BW_20M),    //16+10=26
        new ChannelData(36, "Channel 36 [5180MHz]", 5180000, BW_20M),
        new ChannelData(38, "Channel 38 [5190MHz]", 5190000, BW_40M),
        new ChannelData(40, "Channel 40 [5200MHz]", 5200000, BW_20M),
        new ChannelData(42, "Channel 42 [5210MHz]", 5210000, BW_80M),
        new ChannelData(44, "Channel 44 [5220MHz]", 5220000, BW_20M),
        new ChannelData(46, "Channel 46 [5230MHz]", 5230000, BW_40M),
        new ChannelData(48, "Channel 48 [5240MHz]", 5240000, BW_20M),
        new ChannelData(50, "Channel 50 [5250MHz]", 5250000, BW_160M),
        new ChannelData(52, "Channel 52 [5260MHz]", 5260000, BW_20M),
        new ChannelData(54, "Channel 54 [5270MHz]", 5270000, BW_40M),
        new ChannelData(56, "Channel 56 [5280MHz]", 5280000, BW_20M),
        new ChannelData(58, "Channel 58 [5290MHz]", 5290000, BW_80M),
        new ChannelData(60, "Channel 60 [5300MHz]", 5300000, BW_20M),
        new ChannelData(62, "Channel 62 [5310MHz]", 5310000, BW_40M),
        new ChannelData(64, "Channel 64 [5320MHz]", 5320000, BW_20M),
        new ChannelData(68, "Channel 68 [5340MHz]", 5340000, BW_20M),
        new ChannelData(70, "Channel 70 [5350MHz]", 5350000, BW_40M),
        new ChannelData(72, "Channel 72 [5360MHz]", 5360000, BW_20M),
        new ChannelData(74, "Channel 74 [5370MHz]", 5370000, BW_80M),
        new ChannelData(76, "Channel 76 [5380MHz]", 5380000, BW_20M),
        new ChannelData(78, "Channel 78 [5390MHz]", 5390000, BW_40M),
        new ChannelData(80, "Channel 80 [5400MHz]", 5400000, BW_20M),
        new ChannelData(82, "Channel 82 [5410MHz]", 5410000, BW_160M),
        new ChannelData(84, "Channel 84 [5420MHz]", 5420000, BW_20M),
        new ChannelData(86, "Channel 86 [5430MHz]", 5430000, BW_40M),
        new ChannelData(88, "Channel 88 [5440MHz]", 5440000, BW_20M),
        new ChannelData(90, "Channel 90 [5450MHz]", 5450000, BW_80M),
        new ChannelData(92, "Channel 92 [5460MHz]", 5460000, BW_20M),
        new ChannelData(94, "Channel 94 [5470MHz]", 5470000, BW_40M),
        new ChannelData(96, "Channel 96 [5480MHz]", 5480000, BW_20M),
        new ChannelData(100, "Channel 100 [5500MHz]", 5500000, BW_20M),
        new ChannelData(102, "Channel 102 [5510MHz]", 5510000, BW_40M),
        new ChannelData(104, "Channel 104 [5520MHz]", 5520000, BW_20M),
        new ChannelData(106, "Channel 106 [5530MHz]", 5530000, BW_80M),
        new ChannelData(108, "Channel 108 [5540MHz]", 5540000, BW_20M),
        new ChannelData(110, "Channel 110 [5550MHz]", 5550000, BW_40M),
        new ChannelData(112, "Channel 112 [5560MHz]", 5560000, BW_20M),
        new ChannelData(114, "Channel 114 [5570MHz]", 5570000, BW_160M),
        new ChannelData(116, "Channel 116 [5580MHz]", 5580000, BW_20M),
        new ChannelData(118, "Channel 118 [5590MHz]", 5590000, BW_40M),
        new ChannelData(120, "Channel 120 [5600MHz]", 5600000, BW_20M),
        new ChannelData(122, "Channel 122 [5610MHz]", 5610000, BW_80M),
        new ChannelData(124, "Channel 124 [5620MHz]", 5620000, BW_20M),
        new ChannelData(126, "Channel 126 [5630MHz]", 5630000, BW_40M),
        new ChannelData(128, "Channel 128 [5640MHz]", 5640000, BW_20M),
        new ChannelData(132, "Channel 132 [5660MHz]", 5660000, BW_20M),
        new ChannelData(134, "Channel 134 [5670MHz]", 5670000, BW_40M),
        new ChannelData(136, "Channel 136 [5680MHz]", 5680000, BW_20M),
        new ChannelData(138, "Channel 138 [5690MHz]", 5690000, BW_80M),
        new ChannelData(140, "Channel 140 [5700MHz]", 5700000, BW_20M),
        new ChannelData(142, "Channel 142 [5710MHz]", 5710000, BW_40M),
        new ChannelData(144, "Channel 144 [5720MHz]", 5720000, BW_20M),
        new ChannelData(149, "Channel 149 [5745MHz]", 5745000, BW_20M),
        new ChannelData(151, "Channel 151 [5755MHz]", 5755000, BW_40M),
        new ChannelData(153, "Channel 153 [5765MHz]", 5765000, BW_20M),
        new ChannelData(155, "Channel 155 [5775MHz]", 5775000, BW_80M),
        new ChannelData(157, "Channel 157 [5785MHz]", 5785000, BW_20M),
        new ChannelData(159, "Channel 159 [5795MHz]", 5795000, BW_40M),
        new ChannelData(161, "Channel 161 [5805MHz]", 5805000, BW_20M),
        new ChannelData(163, "Channel 163 [5815MHz]", 5815000, BW_160M),
        new ChannelData(165, "Channel 165 [5825MHz]", 5825000, BW_20M),
        new ChannelData(167, "Channel 167 [5835MHz]", 5835000, BW_40M),
        new ChannelData(169, "Channel 169 [5845MHz]", 5845000, BW_20M),
        new ChannelData(171, "Channel 171 [5855MHz]", 5855000, BW_80M),
        new ChannelData(173, "Channel 173 [5865MHz]", 5865000, BW_20M),
        new ChannelData(175, "Channel 175 [5875MHz]", 5875000, BW_40M),
        new ChannelData(177, "Channel 177 [5885MHz]", 5885000, BW_20M),
        new ChannelData(181, "Channel 181 [5905MHz]", 5905000, BW_20M),
        new ChannelData(184, "Channel 184 [4920MHz]", 4920000, BW_20M),
        new ChannelData(186, "Channel 186 [4930MHz]", 4930000, BW_40M),
        new ChannelData(188, "Channel 188 [4940MHz]", 4940000, BW_20M),
        new ChannelData(192, "Channel 192 [4960MHz]", 4960000, BW_20M),
        new ChannelData(194, "Channel 194 [4970MHz]", 4970000, BW_40M),
        new ChannelData(196, "Channel 196 [4980MHz]", 4980000, BW_20M),
};

    private static HashMap<Integer, ChannelData> sChannelDataDb = null;
    private static HashMap<Integer, ArrayList<ChannelData>> sChannelGroupMap = null;
    private static long[] sCachedSupportChannels = null;
    private static HashMap<Integer, int[]> sCachedSupportedChs = new HashMap<Integer, int[]>();
    private static boolean initChannelDataDatabase() {
        boolean result = true;
        if (sChannelDataDb == null) {
            sChannelDataDb = new HashMap<Integer, ChannelData>();
            if (sChannelGroupMap == null) {
                sChannelGroupMap = new HashMap<Integer, ArrayList<ChannelData>>();
            }
            for (int i = 0; i < sWifiChannelDatas.length; i++) {
                ChannelData channel = sWifiChannelDatas[i];
                channel.sequence = i + 1;
                String name = channel.name;
                int frequency = channel.frequency;
                String freqStr = String.valueOf(frequency / 1000);
                if (!name.contains(freqStr)) {
                    String msg = "UnMatch name & frequency at index:" + i + " name:" +
                       name + " frequency:" + frequency;
                    Elog.e(TAG, msg);
                    continue;
                }
                int id = channel.id;
                int nameId = extractChannelIdFromName(name);
                if (nameId != id) {
                    Elog.e(TAG, "UnMatch id :" + id + " and name:" + name);
                }
                sChannelDataDb.put(id, channel);
                if (isInBandwidth(BW_20M, channel)) {
                    addToChannelGroup(BW_20M, channel);
                }
                if (isInBandwidth(BW_40M, channel)) {
                    addToChannelGroup(BW_40M, channel);
                }
                if (isInBandwidth(BW_80M, channel)) {
                    addToChannelGroup(BW_80M, channel);
                }
               if (isInBandwidth(BW_160M, channel)) {
                    addToChannelGroup(BW_160M, channel);
                }
            }
        }
        return result;
    }

    static int[] getChannelGroupArray(int groupId) {
        ArrayList<ChannelData> groupList = getChannelGroup(groupId);
        if (groupList == null) {
            return null;
        }
        int length = groupList.size();
        int[] array = new int[length];
        for (int i = 0; i < length; i++) {
            ChannelData cd = groupList.get(i);
            array[i] = cd.id;
        }
        return array;
    }

    private static void addToChannelGroup(int groupId, ChannelData cd) {
        ArrayList<ChannelData> list = sChannelGroupMap.get(groupId);
        if (list == null) {
            list = new ArrayList<ChannelData>();
            sChannelGroupMap.put(groupId, list);
        }
        list.add(cd);
    }

    private static ArrayList<ChannelData> getChannelGroup(int groupId) {
        return sChannelGroupMap.get(groupId);
    }

    void removeBw40mUnsupported2dot4GChannels(ArrayAdapter<String> adapter) {
        for (int i = 0; i < sBw40mUnsupported2dot4GChannels.length; i++) {
            int channel = sBw40mUnsupported2dot4GChannels[i];
            String name = getChannelName(channel);
            if (name == null) {
                continue;
            }
            adapter.remove(name);
        }
    }

    int getSelectedChannelId() {
        return extractChannelIdFromName(mChannelSelect);
    }

    int getSelectedFrequency() {
        return getChannelFrequency(getSelectedChannelId());
    }
    void setSelectedChannel(String channelName) {
        mChannelSelect = channelName;
    }

    void insertBw40mUnsupported2dot4GChannels(ArrayAdapter<String> adapter) {
        for (int i = 0; i < sBw40mUnsupported2dot4GChannels.length; i++) {
            int channel = sBw40mUnsupported2dot4GChannels[i];
            String name = getChannelName(channel);
            if (name == null) {
                continue;
            }
            if (isSupported(channel)) {
                insertChannelIntoAdapterByOrder(adapter, name);
            }
        }
    }

    void resetSupportedChannels(ArrayAdapter<String> adapter) {
        adapter.clear();
        long[] supportedCh = getCachedSupportChannels();
        addChannelsIntoAdapter(supportedCh, adapter, false);
    }

    private static long[] getCachedSupportChannels() {
        if (sCachedSupportChannels == null) {

            int len = (int) sChannels[0];
            sCachedSupportChannels = new long[len];
            for (int i = 0; i < len; i++) {
                sCachedSupportChannels[i] = sChannels[i + 1];
            }
        }
        return sCachedSupportChannels;
    }

    private boolean isSupported(int channelId) {
        ChannelData cd = sChannelDataDb.get(channelId);
        if (cd == null) {
            return false;
        }

        if (isInBandwidth(BW_20M, cd)) {
            if (isChannelSupported(channelId, BW_20M)) {
                return true;
            }
        }
        if (isInBandwidth(BW_40M, cd)) {
            if (isChannelSupported(channelId, BW_40M)) {
                return true;
            }
        }
        if (isInBandwidth(BW_80M, cd)) {
            if (isChannelSupported(channelId, BW_80M)) {
                return true;
            }
        }
        return false;
    }

    void addChannelsIntoAdapter(long[] channels,
            ArrayAdapter<String> adapter, boolean byOrder, boolean checkSupported) {
        String name = null;
        if (channels == null) {
            return;
        }
        for (int i = 0; i < channels.length; i++) {
            int id = (int) channels[i];
            if (checkSupported) {
                if (!isSupported(id)) {
                    continue;
                }
            }
            name = getChannelName(id);
            if (name == null) {
                continue;
            }
            if (byOrder) {
                insertChannelIntoAdapterByOrder(adapter, name);
            } else {
                adapter.add(name);
            }
        }
    }

    void addChannelsIntoAdapter(long[] channels, ArrayAdapter<String> adapter, boolean byOrder) {
        addChannelsIntoAdapter(channels, adapter, byOrder, true);
    }

    void removeChannels(int[] channels, ArrayAdapter<String> adapter) {
        for (int i = 0; i < channels.length; i++) {
            int ch = channels[i];
            String name = getChannelName(ch);
            if (name != null) {
                adapter.remove(name);
            }
        }
    }

    void remove2dot4GChannels(ArrayAdapter<String> adapter) {
        for (int i = 1; i <= 14; i++) {
            String name = getChannelName(i);
            adapter.remove(name);
        }
    }

    void insertBw80MChannels(ArrayAdapter<String> adapter) {
        ArrayList<ChannelData> bw80mChannelList = getChannelGroup(BW_80M);
        if (bw80mChannelList == null) {
            Elog.e(TAG, "BW_80M channel group is null");
            return;
        }
        for (int i = 0; i < bw80mChannelList.size(); i++) {
            ChannelData cd = bw80mChannelList.get(i);
            int ch = cd.id;
            if (isChannelSupported(ch, BW_80M)) {
                String name = cd.name;
                insertChannelIntoAdapterByOrder(adapter, name);
            }
        }
    }

    void insertBw160MChannels(ArrayAdapter<String> adapter) {
        ArrayList<ChannelData> bw160mChannelList = getChannelGroup(BW_160M);
        if (bw160mChannelList == null) {
            Elog.e(TAG, "BW_160M channel group is null");
            return;
        }
        for (int i = 0; i < bw160mChannelList.size(); i++) {
            ChannelData cd = bw160mChannelList.get(i);
            int ch = cd.id;
            if (isChannelSupported(ch, BW_160M)) {
                String name = cd.name;
                insertChannelIntoAdapterByOrder(adapter, name);
            }
        }
    }

    void removeBw40MChannels(ArrayAdapter<String> adapter) {
        ArrayList<ChannelData> list = getChannelGroup(BW_40M);
        if (list == null) {
            Elog.e(TAG, "BW_40M channel group is null");
            return;
        }
        for (int i = 0; i < list.size(); i++) {
            ChannelData cd = list.get(i);
            if (cd.id > CHANNEL_NUMBER_14) {
                String name = cd.name;
                adapter.remove(name);
            }
        }
    }

    void remove5GChannels(ArrayAdapter<String> adapter) {
        for (int i = adapter.getCount() - 1; i >= 0; i--) {
            String name = adapter.getItem(i);
            int id = extractChannelIdFromName(name);
            if (id > CHANNEL_NUMBER_14) {
                adapter.remove(name);
            }
        }
    }

    void insert5GChannels(ArrayAdapter<String> adapter) {
        long[] channels = getCachedSupportChannels();
        for (int i = 0; i <= channels.length; i++) {
            int id = (int) channels[i];
            if (id > CHANNEL_NUMBER_14) {
                String tag = getChannelName(id);
                if (tag != null) {
                    insertChannelIntoAdapterByOrder(adapter, tag);
                } else {
                    Elog.d(TAG, "UNKnown channel:" + id);
                }
            }
        }
    }

    private int[] getSupported2dot4gChannels() {
        int[] cachedChs = sCachedSupportedChs.get(CHANNEL_2DOT4G);
        if (cachedChs != null) {
            return cachedChs;
        }
        long[] chsupported = getCachedSupportChannels();
        List<Long> list = new ArrayList<Long>();
        for (int i = 0; i < chsupported.length; i++) {
            long id = chsupported[i];
            if (id >= 1 && id <= 14) {
                list.add(id);
            }
        }
        if (list.size() > 0) {
            int[] target = new int[list.size()];
            for (int i = 0; i < target.length; i++) {
                target[i] = list.get(i).intValue();
            }
            sCachedSupportedChs.put(CHANNEL_2DOT4G, target);
            return target;
        } else {
            return null;
        }
    }

    void addSupported2dot4gChannels(ArrayAdapter<String> adapter, boolean byOrder) {
        int[] ch2dot4g = getSupported2dot4gChannels();
        long[] target = ints2longs(ch2dot4g);
        addChannelsIntoAdapter(target, adapter, byOrder);
    }

    void addSupported5gChannelsByBandwidth(ArrayAdapter<String> adapter,
            int bandwidth, boolean byOrder) {
        int[] channels = getSupported5gChannelsByBandwidth(bandwidth);
        if (channels == null) {
            return;
        }
        long[] target = ints2longs(channels);
        addChannelsIntoAdapter(target, adapter, byOrder, false);
    }

    private int[] getSupported5gChannelsByBandwidth(int bandwidth) {
        List<Integer> list = new ArrayList<Integer>();
        boolean isCached = true;
        if (bandwidth == BW_20M) {
            int[] cachedChs = sCachedSupportedChs.get(BW_20M);
            if (cachedChs != null) {
                return cachedChs;
            }
            int[] chsupported = longs2ints(getCachedSupportChannels());
            if (chsupported != null) {
                for (int i = 0; i < chsupported.length; i++) {
                    int id = chsupported[i];
                    if (isIn5gChannelBandwidth(id, bandwidth)) {
                        list.add(id);
                    }
                }
            }

        } else if (bandwidth == BW_40M) {
            int[] cachedChs = sCachedSupportedChs.get(BW_40M);
            if (cachedChs != null) {
                return cachedChs;
            }
            ArrayList<ChannelData> groupList = getChannelGroup(BW_40M);
            if (groupList == null) {
                Elog.e(TAG, "getSupported5gChannelsByBandwidth BW_40M channel group is null");
                return null;
            }
            for (int i = 0; i < groupList.size(); i++) {
                ChannelData cd = groupList.get(i);
                int id = cd.id;
                if (id > CHANNEL_NUMBER_14) {
                    if (isChannelSupported(id, BW_40M)) {
                        list.add(id);
                    }
                }
            }
        } else if (bandwidth == BW_80M) {
            int[] cachedChs = sCachedSupportedChs.get(BW_80M);
            if (cachedChs != null) {
                return cachedChs;
            }
            ArrayList<ChannelData> groupList = getChannelGroup(BW_80M);
            if (groupList == null) {
                Elog.e(TAG, "getSupported5gChannelsByBandwidth BW_80M channel group is null");
                return null;
            }
            for (int i = 0; i < groupList.size(); i++) {
                ChannelData cd = groupList.get(i);
                int id = cd.id;
                if (isChannelSupported(id, BW_80M)) {
                    list.add(id);
                }
            }
        } else {
            Elog.i(TAG, "getSupported5gChannelsByBandwidth invalid bandwidth:" + bandwidth);
            isCached = false;
        }
        if (list.size() > 0) {
            int[] target = new int[list.size()];
            for (int i = 0; i < target.length; i++) {
                target[i] = list.get(i);
            }
            if (isCached) {
                sCachedSupportedChs.put(bandwidth, target);
            }
            return target;
        } else {
            return null;
        }
    }

    private static int[] longs2ints(long[] array) {
        if (array == null || array.length == 0) {
            return null;
        }
        int[] ints = new int[array.length];
        for (int i = 0; i < array.length; i++) {
            ints[i] = (int) array[i];
        }
        return ints;
    }

    private static long[] ints2longs(int[] array) {
        if (array == null || array.length == 0) {
            return null;
        }
        long[] longs = new long[array.length];
        for (int i = 0; i < array.length; i++) {
            longs[i] = array[i];
        }
        return longs;
    }

    void insertBw20MChannels(ArrayAdapter<String> adapter) {
        ArrayList<ChannelData> groupList = getChannelGroup(BW_20M);
        if (groupList == null) {
            Elog.e(TAG, "BW_20M channel group is null");
            return;
        }
        for (int i = 0; i < groupList.size(); i++) {
            ChannelData cd = groupList.get(i);
            int id = cd.id;
            if (isChannelSupported(id, BW_20M)) {
                String name = cd.name;
                insertChannelIntoAdapterByOrder(adapter, name);
            }
        }
    }

    void insertBw40MChannels(ArrayAdapter<String> adapter) {
        ArrayList<ChannelData> groupList = getChannelGroup(BW_40M);
        if (groupList == null) {
            Elog.e(TAG, "BW_40M channel group is null");
            return;
        }
        for (int i = 0; i < groupList.size(); i++) {
            ChannelData cd = groupList.get(i);
            int id = cd.id;
            //if (id > CHANNEL_NUMBER_14) {
                if (isChannelSupported(id, BW_40M)) {
                    String name = cd.name;
                    insertChannelIntoAdapterByOrder(adapter, name);
                }
            //}
        }
    }

    private int computeInsertIndex(ArrayAdapter<String> adapter, int channel) {
        int targetIndex = -1;
        for (int i = 0; i < adapter.getCount(); i++) {
            String name = adapter.getItem(i);
            int id = extractChannelIdFromName(name);
            if (id > channel) {
                targetIndex = i;
                break;
            }
        }
        if (targetIndex == -1) {
            targetIndex = adapter.getCount();
        }
        return targetIndex;
    }

    static int parseChannelId(String fullName) {
        return extractChannelIdFromName(fullName);
    }

    private static int extractChannelIdFromName(String fullName) {
        int id = -1;
        String[] strs = fullName.split(" +");
        if (strs.length == 3) {
            try {
                id = Integer.valueOf(strs[1]);
            } catch (NumberFormatException e) {
                Elog.e(TAG, "NumberFormatException:" + e.getMessage());
            }
        } else if (strs.length == 4) {
            try {
                id = Integer.valueOf(strs[1])  + 10;
            } catch (NumberFormatException e) {
                Elog.e(TAG, "NumberFormatException:" + e.getMessage());
            }
        } else {
            Elog.w(TAG, "extractChannelIdFromName(): " + fullName + " invalid name format!");
        }
        return id;
    }

    void insertChannelIntoAdapterByOrder(ArrayAdapter<String> adapter, String channelName) {
        if (adapter.getPosition(channelName) == -1) {
            int id = extractChannelIdFromName(channelName);
            int targetIndex = computeInsertIndex(adapter, id);
            adapter.insert(channelName, targetIndex);
        }
    }

    private boolean isIn5gChannelBandwidth(int channel, int bandwidth) {
        if (channel >= 1 && channel <= 14) { // channel 1~14 is 2.4G
            return false;
        }

        ChannelData cd = sChannelDataDb.get(channel);
        if (cd == null) {
            return false;
        }
        if (channel > CHANNEL_NUMBER_14 && isInBandwidth(bandwidth, cd)) {
            return true;
        }
        return false;
    }

    private static boolean isInBandwidth(int bandwidth, ChannelData cd) {
        return (bandwidth & cd.bandwidth) > 0 ? true : false;
    }

    private boolean isChannelSupported(int channel, int bandwidth) {
        boolean supported = true;

        int[] testChannels = null;
        if (bandwidth == BW_20M) {
            testChannels = new int[]{channel};
        } else if (bandwidth == BW_40M) {
            testChannels = new int[]{channel - 2, channel + 2};
        } else if (bandwidth == BW_80M) {
            testChannels = new int[]{channel - 6, channel - 2, channel + 2, channel + 6};
        } else if (bandwidth == BW_160M) {
            testChannels = new int[]{channel - 14, channel - 10, channel - 6, channel - 2,
               channel + 2, channel + 6, channel + 10, channel + 14};
        } else {
            Elog.w(TAG, "Invalid bandwidth:" + bandwidth);
            return false;
        }

        for (int i = 0; i < testChannels.length; i++) {
            int ch = testChannels[i];
            if (!isContains(ch)) {
                supported = false;
                break;
            }
        }
        return supported;
    }

    static String getChannelName(int channelId) {
        ChannelData cd = sChannelDataDb.get(channelId);
        if (cd == null) {
            return null;
        }
        return cd.name;
    }

    static int getChannelFrequency(int channelId) {
        ChannelData cd = sChannelDataDb.get(channelId);
        if (cd == null) {
            return 0;
        }
        return cd.frequency;
    }

    /**
     * Check the channel is support or not in the phone.
     *
     * @param channel
     *            Need to check the channel's number
     * @return True if phone support the channel, or false
     */
    public boolean isContains(int channel) {
        for (int i = 1; i <= sChannels[0]; i++) {
            if (channel == sChannels[i]) {
                return true;
            }
        }
        return false;
    }

    /**
     * Get WiFi chip support channels.
     */
    public static void getSupportChannels() {
        sChannels = new long[MAX_CHANNEL_COUNT];
        if (EMWifi.sIsInitialed) {
            if (0 == EMWifi.getSupportChannelList(sChannels)) {
                Elog.i(TAG, "LENGTH channels[0] = " + sChannels[0]);
                for (int i = 1; i <= sChannels[0]; i++) {
                    if (CHANNEL_NUMBER_14 == sChannels[i]) {
                        sHas14Ch = true;
                    }
                    if (sChannels[i] > CHANNEL_NUMBER_14) {
                        sHasUpper14Ch = true;
                    }
                    Elog.i(TAG, "channels[" + (i) + "] = " + sChannels[i]);
                }
            } else {
                sChannels[0] = DEFAULT_CHANNEL_COUNT;
                for (int i = 0; i < DEFAULT_CHANNEL_COUNT; i++) {
                    sChannels[i + 1] = i + 1;
                }
            }
        } else {
            Elog.w(TAG, "Wifi is not initialed");
        }
    }

    /**
     * Constructor function.
     */
    public ChannelInfo() {
        mChannelSelect = sWifiChannelDatas[0].name;

    }

    static {
        initChannelDataDatabase();
    }
}
