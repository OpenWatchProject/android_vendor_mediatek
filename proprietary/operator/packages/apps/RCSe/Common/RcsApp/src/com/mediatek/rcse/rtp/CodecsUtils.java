/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.mediatek.rcse.rtp;

import java.util.List;
import java.util.Vector;

import com.gsma.services.rcs.sharing.video.VideoCodec;


import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.os.Build;

import com.gsma.services.rcs.sharing.video.VideoCodec;

import com.mediatek.rcse.plugin.phone.Utils;
import com.mediatek.rcse.rtp.codec.audio.amr.AMRWBConfig;
import com.mediatek.rcse.rtp.codec.video.h264.H264Config;
import com.mediatek.rcse.rtp.codec.video.h264.JavaPacketizer;
import com.mediatek.rcse.rtp.codec.video.h264.profiles.H264Profile1_2;
import com.mediatek.rcse.rtp.codec.video.h264.profiles.H264Profile1_3;
import com.mediatek.rcse.rtp.codec.video.h264.profiles.H264Profile1b;
import com.mediatek.rcse.rtp.format.audio.AmrWbAudioFormat;
import com.mediatek.rcse.rtp.format.video.H264VideoFormat;
import com.mediatek.rcse.service.MediatekFactory;

/**
 * Codecs utility functions
 *
 * @author hlxn7157
 */
public class CodecsUtils {

    /**
     * Get list of supported video codecs according to current network
     *
     * @return Codecs list
     */
    public static VideoCodec[] getRendererCodecList() {
        return getSupportedCodecList(true, true);
    }

    /**
     * Get list of supported video codecs according to current network
     *
     * @return Codecs list
     */
    public static VideoCodec[] getPlayerCodecList() {
        return getSupportedCodecList(false, false);
    }

    


    /**
     * Test if size is in list.
     * Can't use List.contains because it doesn't work with some devices.
     *
     * @param list
     * @param width
     * @param height
     * @return boolean
     */
    private static boolean sizeContains(List<Camera.Size> list, int width, int height) {
        for (int i = 0; i < list.size(); i++) {
            if (list.get(i).width == width && list.get(i).height == height) {
                return true;
            }
        }
        return false;
    }

    // Codec parameters
	static String param_1_3 = H264Config.CODEC_PARAM_PROFILEID + "=" + H264Profile1_3.BASELINE_PROFILE_ID + ";" +
		H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=" + JavaPacketizer.H264_ENABLED_PACKETIZATION_MODE;
	static String param_1_2 = H264Config.CODEC_PARAM_PROFILEID + "=" + H264Profile1_2.BASELINE_PROFILE_ID + ";" +
		H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=" + JavaPacketizer.H264_ENABLED_PACKETIZATION_MODE;
	static String param_1_b = H264Config.CODEC_PARAM_PROFILEID + "=" + H264Profile1b.BASELINE_PROFILE_ID + ";" +
		H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=" + JavaPacketizer.H264_ENABLED_PACKETIZATION_MODE;
    
    /**
     * Get list of supported video codecs according to current network
     *
     * @param cif true if available
     * @param qvga true if available
     * @return Codecs list
     */
    private static VideoCodec[] getSupportedCodecList(boolean cif, boolean qvga) {
        int networkLevel = Utils.getNetworkType(MediatekFactory.getApplicationContext());
        int payload_count = H264VideoFormat.PAYLOAD - 1;
        Vector<VideoCodec> list = new Vector<VideoCodec>();

        // Add codecs settings (ordered list)
        /*
         * 3G/3g+ -> level 1.B: profile-level-id=42900b, frame_rate=15, frame_size=QCIF, bit_rate=96k
         *
         * WIFI   -> level 1.2: profile-level-id=42800c, frame_rate=15, frame_size=QVGA, bit_rate=384k
         * WIFI   -> level 1.2: profile-level-id=42800c, frame_rate=15, frame_size=CIF, bit_rate=384k
         * WIFI   -> level 1.3: profile-level-id=42800d, frame_rate=15, frame_size=CIF, bit_rate=384k
         */

  /*  	if (networkLevel == NetworkUtils.NETWORK_ACCESS_WIFI || networkLevel == NetworkUtils.NETWORK_ACCESS_4G) {
            if (cif) {
                list.add(new VideoCodec(H264Config.CODEC_NAME,
                        ++payload_count,
                        H264Config.CLOCK_RATE,
                        15,
                        256000,
                        H264Config.CIF_WIDTH, 
                        H264Config.CIF_HEIGHT,
                        param_1_3));
                
                list.add(new VideoCodec(H264Config.CODEC_NAME,
                        ++payload_count,
                        H264Config.CLOCK_RATE,
                        15,
                        176000,
                        H264Config.CIF_WIDTH, 
                        H264Config.CIF_HEIGHT,
                        param_1_2));
            }
            if (qvga) {
                list.add(new VideoCodec(H264Config.CODEC_NAME,
                        ++payload_count,
                        H264Config.CLOCK_RATE,
                        15,
                        176000,
                        H264Config.QVGA_WIDTH, 
                        H264Config.QVGA_HEIGHT,
                        param_1_2));
            }
        }*/
        list.add(new VideoCodec(H264Config.CODEC_NAME,
                ++payload_count,
                H264Config.CLOCK_RATE,
                15,
                96000,
                H264Config.QCIF_WIDTH, 
                H264Config.QCIF_HEIGHT,
                param_1_b));

        return (VideoCodec[]) list.toArray(new VideoCodec[list.size()]);
    }
    
 
}
