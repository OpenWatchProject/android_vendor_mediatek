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

package com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.decoder;

import java.nio.ByteBuffer;

import java.io.IOException;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.decoder.VideoSurfaceView;;

public class NativeH264Decoder {

    VideoSurfaceView mSurface = null;
    int  rotation = 90; 

    public NativeH264Decoder() {
    }

    private MediaCodec mDecoder = null;
    private MediaFormat mediaFormatRenderer = null;

    public int InitDecoder(VideoSurfaceView surface) {
    try {
        mDecoder = MediaCodec.createDecoderByType("video/avc");
} catch (IOException e) {
            
            }

        mediaFormatRenderer = MediaFormat.createVideoFormat("video/avc", 176,
                144);
        mediaFormatRenderer.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
        mediaFormatRenderer.setInteger(MediaFormat.KEY_FRAME_RATE, 15);
        mediaFormatRenderer.setInteger(MediaFormat.KEY_COLOR_FORMAT,
                MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        mediaFormatRenderer.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 5);
        mediaFormatRenderer.setInteger("rotation-degrees", rotation);
        mSurface = surface;

        return 0;
    }

    public int InitDecoder(VideoSurfaceView surface ,int width , int height,int bitRate , int frameRate,int iFrameInterval) {
    try {
        mDecoder = MediaCodec.createDecoderByType("video/avc");
} catch (IOException e) {
            
            }

        mediaFormatRenderer = MediaFormat.createVideoFormat("video/avc", width,
                height);
        mediaFormatRenderer.setInteger(MediaFormat.KEY_BIT_RATE, bitRate);
        mediaFormatRenderer.setInteger(MediaFormat.KEY_FRAME_RATE, frameRate);
        mediaFormatRenderer.setInteger(MediaFormat.KEY_COLOR_FORMAT,
                MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        mediaFormatRenderer.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, iFrameInterval);
        mediaFormatRenderer.setInteger("rotation-degrees", rotation);

        mSurface = surface;

        return 0;
    }

    public int configureDecoder(byte[] sps, byte[] pps) {
        if((sps == null) || (pps == null)) return -1;
        mediaFormatRenderer.setByteBuffer("csd-0", ByteBuffer.wrap(sps));
        mediaFormatRenderer.setByteBuffer("csd-1", ByteBuffer.wrap(pps));

        mDecoder.configure(mediaFormatRenderer, mSurface.getHolder().getSurface(), null, 0);
        mDecoder.start();
        return 0;
    }

    public int DecodeAndConvert(byte abyte0[], int rotateOrientation,
            int[] dimensions) {

        if(rotateOrientation != rotation){
        rotation = rotateOrientation;
        mediaFormatRenderer.setInteger("rotation-degrees", rotation);
        mDecoder.stop();
        mDecoder.configure(mediaFormatRenderer, mSurface.getHolder().getSurface(), null, 0);
        mDecoder.start();
         }

        ByteBuffer[] inputBuffersRenderer = mDecoder.getInputBuffers();
        mDecoder.getOutputBuffers();

        int inputBufferIndexRenderer = mDecoder.dequeueInputBuffer(-1);

        if (inputBufferIndexRenderer >= 0) {
            ByteBuffer inputBufferRenderer = inputBuffersRenderer[inputBufferIndexRenderer];
            inputBufferRenderer.put(abyte0);
            mDecoder.queueInputBuffer(inputBufferIndexRenderer, 0,abyte0.length, 0, 0);
        }

        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        int outputBufferIndex = mDecoder.dequeueOutputBuffer(bufferInfo, 0);
        while (outputBufferIndex >= 0) {
            mDecoder.releaseOutputBuffer(outputBufferIndex, true);
            outputBufferIndex = mDecoder.dequeueOutputBuffer(bufferInfo, 0);
        }
        return 0;

    }

    public int DeinitDecoder() {
        mDecoder.stop();
        mDecoder.release();
        mDecoder = null;
        return 0;
    }

    public int getLastDecodeStatus() {
        return 0;
    }

}
