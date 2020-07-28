/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.hdmi;

/**
 * @hide
 */
public class HdmiDef {

    /**
     * HDMI resolution define AUTO mode offset value
     * Ex. 104, UI show AUTO, output resolution is 720p_60Hz
     *
     * @internal
     */
    public static final int AUTO = 100;

    /**
     * HDMI resolution definition
     */
    public static final int HDMI_VIDEO_720x480i_60Hz = 0;
    public static final int HDMI_VIDEO_720x576i_50Hz = 1;
    public static final int RESOLUTION_720X480P_60HZ = 2;
    public static final int RESOLUTION_720X576P_50HZ = 3;
    public static final int RESOLUTION_1280X720P_60HZ = 4;
    public static final int RESOLUTION_1280X720P_50HZ = 5;
    public static final int RESOLUTION_1920X1080I_60HZ = 6;
    public static final int RESOLUTION_1920X1080I_50HZ = 7;
    public static final int RESOLUTION_1920X1080P_30HZ = 8;
    public static final int RESOLUTION_1920X1080P_25HZ = 9;
    public static final int RESOLUTION_1920X1080P_24HZ = 10;
    public static final int RESOLUTION_1920X1080P_23HZ = 11;
    public static final int RESOLUTION_1920X1080P_29HZ = 12;
    public static final int RESOLUTION_1920X1080P_60HZ = 13;
    public static final int RESOLUTION_1920X1080P_50HZ = 14;
    public static final int RESOLUTION_1280X720P3D_60HZ = 15;
    public static final int RESOLUTION_1280X720P3D_50HZ = 16;
    public static final int RESOLUTION_1920X1080I3D_60HZ = 17;
    public static final int RESOLUTION_1920X1080I3D_50HZ = 18;
    public static final int RESOLUTION_1920X1080P3D_24HZ = 19;
    public static final int RESOLUTION_1920X1080P3D_23HZ = 20;

    public static final int RESOLUTION_3840X2160P23_976HZ = 21;
    public static final int RESOLUTION_3840X2160P_24HZ = 22;
    public static final int RESOLUTION_3840X2160P_25HZ = 23;
    public static final int RESOLUTION_3840X2160P19_97HZ = 24;
    public static final int RESOLUTION_3840X2160P_30HZ = 25;
    public static final int RESOLUTION_4096X2161P_24HZ = 26;

    /**
     * HDMI resolution 4k
     */

    public static final int RESOLUTION_3840X2160P_60HZ = 27;
    public static final int RESOLUTION_3840X2160P_50HZ = 28;
    public static final int RESOLUTION_4096X2161P_60HZ = 29;
    public static final int RESOLUTION_4096X2161P_50HZ = 30;

    /**
     * HDMI resolution EDID mask
     */
    public static final int SINK_480P = (1 << 0);
    public static final int SINK_720P60 = (1 << 1);
    public static final int SINK_1080I60 = (1 << 2);
    public static final int SINK_1080P60 = (1 << 3);
    public static final int SINK_480P_1440 = (1 << 4);
    public static final int SINK_480P_2880 = (1 << 5);
    public static final int SINK_480I = (1 << 6);
    public static final int SINK_480I_1440 = (1 << 7);
    public static final int SINK_480I_2880 = (1 << 8);
    public static final int SINK_1080P30 = (1 << 9);
    public static final int SINK_576P = (1 << 10);
    public static final int SINK_720P50 = (1 << 11);
    public static final int SINK_1080I50 = (1 << 12);
    public static final int SINK_1080P50 = (1 << 13);
    public static final int SINK_576P_1440 = (1 << 14);
    public static final int SINK_576P_2880 = (1 << 15);
    public static final int SINK_576I = (1 << 16);
    public static final int SINK_576I_1440 = (1 << 17);
    public static final int SINK_576I_2880 = (1 << 18);
    public static final int SINK_1080P25 = (1 << 19);
    public static final int SINK_1080P24 = (1 << 20);
    public static final int SINK_1080P23976 = (1 << 21);
    public static final int SINK_1080P2997 = (1 << 22);

    //add for 4K2K resolution
    public static final int SINK_2160P_23_976HZ  = (1 << 0);
    public static final int SINK_2160P_24HZ = (1 << 1);
    public static final int SINK_2160P_25HZ = (1 << 2);
    public static final int SINK_2160P_29_97HZ  = (1 << 3);
    public static final int SINK_2160P_30HZ = (1 << 4);
    public static final int SINK_2161P_24HZ = (1 << 5);
    public static final int SINK_2161P_25HZ = (1 << 6);
    public static final int SINK_2161P_30HZ = (1 << 7);
    public static final int SINK_2160P_50HZ = (1 << 8);
    public static final int SINK_2160P_60HZ  = (1 << 9);
    public static final int SINK_2161P_50HZ = (1 << 10);
    public static final int SINK_2161P_60HZ = (1 << 11);


    public static int[] sResolutionMask = new int[] {0,0, SINK_480P, SINK_576P,
            SINK_720P60, SINK_720P50, SINK_1080I60, SINK_1080I50, SINK_1080P30,
            SINK_1080P25, SINK_1080P24, SINK_1080P23976, SINK_1080P2997,
            SINK_1080P60, SINK_1080P50 };

    public static int[] sResolutionMask_4k2k = new int[] {SINK_2160P_23_976HZ, SINK_2160P_24HZ,
            SINK_2160P_25HZ, SINK_2160P_29_97HZ, SINK_2160P_30HZ, SINK_2161P_24HZ, SINK_2160P_60HZ,
            SINK_2160P_50HZ, SINK_2161P_60HZ, SINK_2161P_50HZ};

    public static int[] getAllResolutions() {
        int[] resolutions = new int[] {
                RESOLUTION_4096X2161P_24HZ, RESOLUTION_3840X2160P_30HZ,
                RESOLUTION_3840X2160P_25HZ, RESOLUTION_3840X2160P_24HZ,
                RESOLUTION_3840X2160P23_976HZ,RESOLUTION_1920X1080P_60HZ,
                RESOLUTION_1920X1080P_50HZ, RESOLUTION_1920X1080P_30HZ,
                RESOLUTION_1920X1080P_25HZ, RESOLUTION_1920X1080P_24HZ,
                RESOLUTION_1920X1080P_23HZ, RESOLUTION_1920X1080I_60HZ,
                RESOLUTION_1920X1080I_50HZ, RESOLUTION_1280X720P_60HZ,
                RESOLUTION_1280X720P_50HZ, RESOLUTION_720X480P_60HZ,
                RESOLUTION_720X576P_50HZ };
        return resolutions;
    }

    public static int[] getTabletAllResolutions() {
        int[] resolutions = new int[] { RESOLUTION_1920X1080P_60HZ,
                RESOLUTION_1920X1080P_50HZ, RESOLUTION_1920X1080P_30HZ,
                RESOLUTION_1920X1080P_25HZ, RESOLUTION_1920X1080P_24HZ,
                RESOLUTION_1920X1080P_23HZ, RESOLUTION_1920X1080I_60HZ,
                RESOLUTION_1920X1080I_50HZ, RESOLUTION_1280X720P_60HZ,
                RESOLUTION_1280X720P_50HZ, RESOLUTION_720X480P_60HZ,
                RESOLUTION_720X576P_50HZ };
        return resolutions;
    }

    public static int[] getAllResolutionsBox() {
        int[] resolutions = new int[] { RESOLUTION_3840X2160P_60HZ,
                RESOLUTION_3840X2160P_30HZ,RESOLUTION_1920X1080P_60HZ,
                RESOLUTION_1280X720P_60HZ,RESOLUTION_720X480P_60HZ };
        return resolutions;
    }

    public static int[] getDefaultResolutions(int index) {
        int[] resolutions;
        if (0 == index) {
            resolutions = new int[] { RESOLUTION_1280X720P_60HZ,
                    RESOLUTION_1280X720P_50HZ, RESOLUTION_1920X1080P_24HZ,
                    RESOLUTION_1920X1080P_23HZ };
        } else if (1 == index) {
            resolutions = getAllResolutions();
        } else if (2 == index){
            resolutions = new int[] { RESOLUTION_1920X1080P_30HZ,
                    RESOLUTION_1280X720P_60HZ, RESOLUTION_720X480P_60HZ };
        } else if (4 == index) {
            resolutions = getTabletAllResolutions();
        } else {
            resolutions = new int[] { RESOLUTION_1920X1080P_30HZ,
                    RESOLUTION_1920X1080P_60HZ,
                    RESOLUTION_1280X720P_60HZ, RESOLUTION_720X480P_60HZ };
        }
        return resolutions;
    }

    public static int[] getPreferedResolutions(int index) {
        int[] prefered = null;
        if (0 == index) {
            prefered = new int[] { AUTO + RESOLUTION_1280X720P_60HZ,
                    AUTO + RESOLUTION_1280X720P_50HZ,
                    AUTO + RESOLUTION_720X480P_60HZ,
                    AUTO + RESOLUTION_720X576P_50HZ };
        } else if (1 == index) {
            prefered = new int[] { AUTO + RESOLUTION_4096X2161P_24HZ,
                    AUTO + RESOLUTION_3840X2160P_30HZ,
                    AUTO + RESOLUTION_3840X2160P19_97HZ,
                    AUTO + RESOLUTION_3840X2160P_25HZ,
                    AUTO + RESOLUTION_3840X2160P_24HZ,
                    AUTO + RESOLUTION_3840X2160P23_976HZ,
                    AUTO + RESOLUTION_1920X1080P_60HZ,
                    AUTO + RESOLUTION_1920X1080P_50HZ,
                    AUTO + RESOLUTION_1920X1080P_30HZ,
                    AUTO + RESOLUTION_1920X1080P_25HZ,
                    AUTO + RESOLUTION_1920X1080P_24HZ,
                    AUTO + RESOLUTION_1920X1080P_23HZ,
                    AUTO + RESOLUTION_1920X1080I_60HZ,
                    AUTO + RESOLUTION_1920X1080I_50HZ,
                    AUTO + RESOLUTION_1280X720P_60HZ,
                    AUTO + RESOLUTION_1280X720P_50HZ,
                    AUTO + RESOLUTION_720X480P_60HZ,
                    AUTO + RESOLUTION_720X576P_50HZ };
        } else if (2 == index) {
            prefered = new int[] { AUTO + RESOLUTION_1280X720P_60HZ,
                    AUTO + RESOLUTION_1920X1080P_30HZ,
                    AUTO + RESOLUTION_720X480P_60HZ };
        } else {
            prefered = new int[] { AUTO + RESOLUTION_1280X720P_60HZ,
                    AUTO + RESOLUTION_1920X1080P_30HZ,
                    AUTO + RESOLUTION_1920X1080P_60HZ,
                    AUTO + RESOLUTION_720X480P_60HZ };
        }
        return prefered;
    }

    /**
     * HDMI display type definition: HDMI
     *
     * @internal
     */
    public static final int DISPLAY_TYPE_HDMI = 0;

    /**
     * HDMI display type definition: smartbook
     *
     * @internal
     */
    public static final int DISPLAY_TYPE_SMB = 1;

    /**
     * HDMI display type definition: MHL
     *
     * @internal
     */
    public static final int DISPLAY_TYPE_MHL = 2;

    /**
     * HDMI display type definition: Slim Port
     */
    public static final int DISPLAY_TYPE_SLIMPORT = 3;

    /**
     * HDMI capability definition: adjust scale setting
     *
     * @internal
     */
    public static final int CAPABILITY_SCALE_ADJUST = 0x01;

    /**
     * HDMI capability definition: HDMI and main display mutex
     *
     * @internal
     */
    public static final int CAPABILITY_RDMA_LIMIT = 0x02;

    /**
     * HDMI capability definition: HDMI and call mutex
     *
     * @internal
     */
    public static final int CAPABILITY_MUTEX_CALL = 0x04;

    public static final int HDMI_MAX_CHANNEL = 0x78; // 1111000
    public static final int HDMI_MAX_SAMPLERATE= 0x380; // 1110000000
    public static final int HDMI_MAX_BITWIDTH = 0xc00; // 110000000000
    public static final int HDMI_MAX_CHANNEL_OFFSETS = 3;
    public static final int HDMI_MAX_SAMPLERATE_OFFSETS = 7;
    public static final int HDMI_MAX_BITWIDTH_OFFSETS = 10;

    public static final int AUDIO_OUTPUT_STEREO = 2;
    public static final int AUDIO_OUTPUT_MULTICHANNEL= 6;

}
