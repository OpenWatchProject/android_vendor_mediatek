package com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.encoder;

import java.nio.ByteBuffer;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.util.Log;
import java.io.IOException;
public class NativeH264Encoder {
    private static final String TAG = "ENCODER";
    private byte[] sps = null;
    private byte[] pps = null;
    private MediaCodec mEncoder = null;
    private int result = -1;

    public int InitEncoder(NativeH264EncoderParams encoderParams) {
try{
        mEncoder = MediaCodec.createEncoderByType("video/avc");
} catch (IOException e) {
            
            }

        MediaFormat mediaFormat = MediaFormat.createVideoFormat("video/avc",
                encoderParams.getFrameWidth(), encoderParams.getFrameHeight());
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE,
                encoderParams.getBitRate());
        mediaFormat.setFloat(MediaFormat.KEY_FRAME_RATE,
                encoderParams.getFrameRate());
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT,
                MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar);
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL,
                encoderParams.getIFrameInterval());
        mEncoder.configure(mediaFormat, null, null,
                MediaCodec.CONFIGURE_FLAG_ENCODE);
        mEncoder.start();

        return 0;

    }

    // Resize the frame and Encode
    public byte[] ResizeAndEncodeFrame(byte abyte0[], long l,
            boolean mirroring, int srcWidth, int srcHeight) {
        ByteBuffer frameBuffer = null;
        byte[] encoded = null;

        try {
            ByteBuffer[] inputBuffers = mEncoder.getInputBuffers();
            ByteBuffer[] outputBuffers = mEncoder.getOutputBuffers();

            int inputBufferIndex = mEncoder.dequeueInputBuffer(-1);
            if (inputBufferIndex >= 0) {
                ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
                inputBuffer.clear();
                inputBuffer.put(abyte0);
                mEncoder.queueInputBuffer(inputBufferIndex, 0, abyte0.length,
                        0, 0);
            }

            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
            int outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 0);
            while (outputBufferIndex >= 0) {
                ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
                byte[] outData = new byte[bufferInfo.size];
                outputBuffer.get(outData);
                if (sps != null && pps != null) {
                    frameBuffer = ByteBuffer.wrap(outData);
                    frameBuffer.putInt(bufferInfo.size - 4);
                    frameBuffer.get(encoded);
                } else {
                    ByteBuffer spsPpsBuffer = ByteBuffer.wrap(outData);
                    if (spsPpsBuffer.getInt() == 0x00000001) {
                        System.out.println("parsing sps/pps");
                    } else {
                        System.out.println("something is amiss?");
                    }
                    int ppsIndex = 0;
                    while (!(spsPpsBuffer.get() == 0x00
                            && spsPpsBuffer.get() == 0x00
                            && spsPpsBuffer.get() == 0x00 && spsPpsBuffer.get() == 0x01)) {

                    }
                    ppsIndex = spsPpsBuffer.position();
                    sps = new byte[ppsIndex - 8];
                    System.arraycopy(outData, 4, sps, 0, sps.length);
                    pps = new byte[outData.length - ppsIndex];
                    System.arraycopy(outData, ppsIndex, pps, 0, pps.length);

                }
                mEncoder.releaseOutputBuffer(outputBufferIndex, false);
                outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 0);
            }
        } catch (Throwable t) {
            t.printStackTrace();
        }

        return encoded;

    }

    public byte[] EncodeFrame(byte abyte0[], long l, boolean mirroring,
            float scalingFactor) {

        // TODO how to fit in timestamp
        ByteBuffer[] inputBuffers = mEncoder.getInputBuffers();
        ByteBuffer[] outputBuffers = mEncoder.getOutputBuffers();

        int inputBufferIndex = mEncoder.dequeueInputBuffer(-1);
        if (inputBufferIndex >= 0) {
            ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
            inputBuffer.clear();

            inputBuffer.put(abyte0);
            mEncoder.queueInputBuffer(inputBufferIndex, 0, abyte0.length, 0, 0);
        }
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        // TODO see how to scale
        mEncoder.setVideoScalingMode(MediaCodec.VIDEO_SCALING_MODE_SCALE_TO_FIT);

        int outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 0);
        // TODO make sure sps and pps can be generated more than once . Check on
        // the basis of NAL header.
        if (outputBufferIndex >= 0) {
            while (outputBufferIndex >= 0) {
                ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
                // Slices
                if (sps != null && pps != null) {
                    byte[] frameOut = new byte[bufferInfo.size];
                    outputBuffer.get(frameOut);
                    result = 0;
                    return frameOut;
                }
                // SPS PPS
                else {
                    byte[] outData = new byte[bufferInfo.size];
                    outputBuffer.get(outData);
                    ByteBuffer spsPpsBuffer = ByteBuffer.wrap(outData);

                    if (spsPpsBuffer.getInt() == 0x00000001) {
                        System.out.println("parsing sps/pps");
                    } else {
                        System.out.println("something wrong");
                    }
                    int ppsIndex = 0;
                    // second 0001
                    while (!(spsPpsBuffer.get() == 0x00
                            && spsPpsBuffer.get() == 0x00
                            && spsPpsBuffer.get() == 0x00 && spsPpsBuffer.get() == 0x01)) {

                    }
                    ppsIndex = spsPpsBuffer.position();
                    sps = new byte[ppsIndex - 8];
                    System.arraycopy(outData, 4, sps, 0, sps.length);


                    pps = new byte[outData.length - ppsIndex];
                    System.arraycopy(outData, ppsIndex, pps, 0, pps.length);

                }

                mEncoder.releaseOutputBuffer(outputBufferIndex, false);
                outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 0);
            }

        } else if (outputBufferIndex >= MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
            outputBuffers = mEncoder.getOutputBuffers();
        }
        return null;
    }

    public byte[] getPps() {
        return pps;
    }

    public byte[] getSps() {
        return sps;
    }

    public int DeinitEncoder() {
        mEncoder.stop();
        mEncoder.release();
        return 0;

    }

    public int getLastEncodeStatus() {
        return result;

    }

}
