package com.mediatek.rcse.service;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.concurrent.atomic.AtomicReference;

import org.w3c.dom.Text;

import com.mediatek.rcse.api.Logger;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.Matrix;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;

public class VideoCompress extends Activity{

   private static final int TIMEOUT_USEC = 10000;
    // parameters for the audio encoder
    private static final String OUTPUT_AUDIO_MIME_TYPE = "audio/mp4a-latm"; // Advanced
    public static final String TAG = "VideoCompressUtils";                                                                        // Audio
                                                                            // Coding
    private static final int OUTPUT_AUDIO_CHANNEL_COUNT = 1; // Must match the
                                                             // input stream.
    private static final int OUTPUT_AUDIO_BIT_RATE = 44 * 1024;
    private static final int OUTPUT_AUDIO_AAC_PROFILE = MediaCodecInfo.CodecProfileLevel.AACObjectHE;
    static ProgressBar progressbar = null;
    static TextView compresstext = null;
    // parameters for the video encoder
    private static final String OUTPUT_VIDEO_MIME_TYPE = "video/avc";
    private static final int OUTPUT_VIDEO_BIT_RATE = 600000;
    private static final int OUTPUT_VIDEO_FRAME_RATE = 30;
    private static final int OUTPUT_VIDEO_IFRAME_INTERVAL = 10;
    private static final int OUTPUT_VIDEO_COLOR_FORMAT = MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface;

    private InputSurface inputSurface = null;

    /** Width of the output frames. */
    private static int mWidth = 176;
    /** Height of the output frames. */
    private static int mHeight = 144;

    /** The destination file for the encoded output. */
    private static String mOutputFile;
    private static Handler  mainHandler = null;
    static boolean compressignDone = false;
    public static String inputFilePath = null;
    public static boolean mCancelCompression = false;
      
    public static boolean ismCancelCompression() {
        Logger.d(TAG, "[VideoCompress] ismCancelCompression entry" + mCancelCompression);
        return mCancelCompression;
    }
      
    @Override
        protected void onCreate(Bundle savedInstanceState) {
            // TODO Auto-generated method stub
            super.onCreate(savedInstanceState);
        }
   
    public boolean hasPermission(final String permission) {
        final Context context = MediatekFactory.getApplicationContext();
        final int permissionState = context.checkCallingOrSelfPermission(permission);        
        return permissionState == PackageManager.PERMISSION_GRANTED;
    }
    
    public static void VideoCompress () {
        //Empty Constructor
    }

    public static void cancelCompression() {
        Logger.d(TAG, "[VideoCompress] CancelCompression entry");
        mCancelCompression = true;
    }

    public static String compressVideo(String filePath) {
        try {
            //mainHandler = new Handler(MediatekFactory.getApplicationContext().getMainLooper());
            Logger.d(TAG, "compressVideo entry" + filePath);
            Utils.createRcseFolder();
            inputFilePath = filePath;
            mCancelCompression = false;
      /* try {
                new VideoCompressor().execute();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }*/
            setOutputFile();
            boolean compressionResult = false;            
            try {
                compressionResult = extractDecodeEncodeMux(inputFilePath ,mOutputFile,mWidth,mHeight,30);
            } catch (Exception e) {
                // TODO Auto-generated catch block
                Logger.d(TAG, "[VideoCompress] compressVideo exception 1 ");
                e.printStackTrace();
            }
            if(compressionResult) {
                Logger.d(TAG, "[VideoCompress] Compression Successful path=" + mOutputFile);
                return mOutputFile;
            }
            Logger.d(TAG, "Compression result " + compressionResult);
            File file = new File(mOutputFile);
            if (!file.exists()) {
                Logger.e(TAG, "[VideoCompress] Compression() file does not exist: "
                        + filePath);
                return filePath;
            }
            long fileSize = file.length();
            
            if(fileSize > 10240 && !mCancelCompression) {
                Logger.d(TAG, "[VideoCompress] Compression Successful size=" + fileSize);
                return mOutputFile;
            }
            
            return filePath;
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            Logger.d(TAG, "[VideoCompress] Compression exception ");
            return filePath;
        }
    }

   
    
    class VideoCompressor extends AsyncTask<Void, Void, Boolean>{
        
        void VideoCompressor () {
            
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();            
            setOutputFile();
            Logger.d(TAG, "[VideoCompress] onPreExecute entry");
            try {
                //progressbar.setVisibility(View.VISIBLE);               
            } catch (Exception e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            }
        }

        @Override
        protected Boolean doInBackground(Void... voids) {
            try {
                return true;
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return false;
            }
        }

        @Override
        protected void onPostExecute(Boolean compressed) {
            super.onPostExecute(compressed);
            Logger.d(TAG, "[VideoCompress] onPostExecute entry");
            //progressbar.setVisibility(View.GONE);
            compresstext.setText("Compression Done Result -" + mOutputFile.toString());
        }
    }

    private static void setOutputFile() {
        Logger.d(TAG, "[VideoCompress] setOutputFile entry");
        StringBuilder sb = new StringBuilder();
        sb.append(Utils.RCSE_COMPRESSED_FILE_DIR);
        String compressfile = "/VIDEO_" + new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(new Date());
        sb.append(compressfile);
        sb.append(".mp4");
        mOutputFile = sb.toString();
        Logger.d(TAG, "[VideoCompress] setOutputFile exit=" + mOutputFile);
    }

    private static Boolean extractDecodeEncodeMux(String inputFile, String outputFile, int scaledWidth, int scaledHeight, int outfps)
            throws Exception {
        Logger.d(TAG, "[VideoCompress] extractDecodeEncodeMux entry, inputFile=" + inputFile + "scaledWidth=" + scaledWidth + "scaledHeight=" + scaledHeight);
        // Exception that may be thrown during release.
        Exception exception = null;   
       
        // the output audio
        MediaCodecInfo audioCodecInfo = selectCodec("audio/mp4a-latm");

        MediaExtractor audioExtractor = null;
        MediaExtractor videoExtractor = null;
        MediaCodec audioDecoder = null;
        MediaCodec audioEncoder = null;
        MediaCodec videoEncoder = null;
        MediaCodec videoDecoder = null;
        MediaMuxer muxer = null;
        CodecOutputSurface outputSurface = null;

        try {
            // extractor
            audioExtractor = createExtractor(inputFile);
            videoExtractor = createExtractor(inputFile);

            // audio encoder decoder
            int audioInputTrack = getAndSelectAudioTrackIndex(audioExtractor);
int audioChannelsCount = audioExtractor.getTrackFormat(audioInputTrack).getInteger(MediaFormat.KEY_CHANNEL_COUNT);
            MediaFormat outputAudioFormat = MediaFormat.createAudioFormat("audio/mp4a-latm", 48000, audioChannelsCount);
            if(audioChannelsCount > 1) {
                outputAudioFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 12288);  
            }
            outputAudioFormat.setInteger(MediaFormat.KEY_BIT_RATE, 48000);           
            outputAudioFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectHE);
            audioEncoder = createAudioEncoder(audioCodecInfo, outputAudioFormat);
            audioDecoder = createAudioDecoder(audioExtractor.getTrackFormat(audioInputTrack));

            // video encoder decoder
            int videoInputTrack = getAndSelectVideoTrackIndex(videoExtractor);
            videoEncoder = MediaCodec.createEncoderByType("video/avc");
            MediaFormat mediaFormat = MediaFormat.createVideoFormat("video/avc", scaledWidth, scaledHeight);
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 60000);
            mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, outfps);
            mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_Format32bitARGB8888);
            mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 10);
            videoEncoder.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
           
            AtomicReference<Surface> inputSurfaceReference = new AtomicReference<Surface>();

            
            videoEncoder = createVideoEncoder(mediaFormat, inputSurfaceReference);
            
       /*     inputSurface = new InputSurface(inputSurfaceReference.get());
            inputSurface.makeCurrent();*/

            outputSurface = new CodecOutputSurface(scaledWidth, scaledHeight);
            videoDecoder = createVideoDecoder(videoExtractor.getTrackFormat(videoInputTrack), outputSurface);

            // Creates a muxer but do not start or add tracks just yet.
            muxer = createMuxer();
            doExtractDecodeEncodeMux(videoExtractor, audioExtractor, audioDecoder, audioEncoder, videoEncoder, videoDecoder,
                    muxer,outputSurface);
            Logger.d(TAG, "[VideoCompress] extractDecodeEncodeMux exit");
        } finally {
            // free resources
            try {
                if (outputSurface != null) {
                    outputSurface.release();
                    outputSurface = null;
                }
                if (audioExtractor != null) {
                    audioExtractor.release();
                }
            } catch (Exception e) {
                if (exception == null) {
                    exception = e;
                }
            }
            try {
                if (videoDecoder != null) {
                    videoDecoder.stop();
                    videoDecoder.release();
                }
            } catch (Exception e) {
                if (exception == null) {
                    exception = e;
                }
            }

            try {
                if (videoEncoder != null) {
                    videoEncoder.stop();
                    videoEncoder.release();
                }
            } catch (Exception e) {
                if (exception == null) {
                    exception = e;
                }
            }
            try {
                if (audioDecoder != null) {
                    audioDecoder.stop();
                    audioDecoder.release();
                }
           } catch (Exception e) {
                if (exception == null) {
                    exception = e;
                }
            }
            try {
                if (audioEncoder != null) {
                    audioEncoder.stop();
                    audioEncoder.release();
                }
            } catch (Exception e) {
                if (exception == null) {
                    exception = e;
                }
            }
            try {
                if (muxer != null) {
                    muxer.stop();
                    muxer.release();
                }
            } catch (Exception e) {
                if (exception == null) {
                    exception = e;
                }
            }

        }
        if (exception != null) {
            return false;
        }
        
        if(mCancelCompression) {
            Logger.d(TAG, "[VideoCompress] mCancelCompression true");
            return false;
        }
       return true;
    }

    /**
     * Does the actual work for extracting, decoding, encoding and muxing.
     */
    private static void doExtractDecodeEncodeMux(MediaExtractor videoExtractor, MediaExtractor audioExtractor,
            MediaCodec audioDecoder, MediaCodec audioEncoder, MediaCodec videoEncoder, MediaCodec videoDecoder,
            MediaMuxer muxer ,CodecOutputSurface outputSurface) {

        ByteBuffer[] audioDecoderInputBuffers = audioDecoder.getInputBuffers();
        ByteBuffer[] audioDecoderOutputBuffers = audioDecoder.getOutputBuffers();
       
        ByteBuffer[] audioEncoderInputBuffers = audioEncoder.getInputBuffers();
        ByteBuffer[] audioEncoderOutputBuffers = audioEncoder.getOutputBuffers();
        MediaCodec.BufferInfo audioDecoderOutputBufferInfo = new MediaCodec.BufferInfo();
        MediaCodec.BufferInfo audioEncoderOutputBufferInfo = new MediaCodec.BufferInfo();

        ByteBuffer[] videoDecoderInputBuffers = videoDecoder.getInputBuffers();
        ByteBuffer[] videoEncoderInputBuffers = videoEncoder.getInputBuffers();
        ByteBuffer[] videoDecoderOutputBuffers = videoDecoder.getOutputBuffers();
        ByteBuffer[] videoEncoderOutputBuffers = videoEncoder.getOutputBuffers();
        MediaCodec.BufferInfo videoDecoderOutputBufferInfo = new MediaCodec.BufferInfo();
        MediaCodec.BufferInfo videoEncoderOutputBufferInfo = new MediaCodec.BufferInfo();

        // We will get these from the decoders when notified of a format change.
        MediaFormat decoderOutputAudioFormat = null;
        MediaFormat decoderOutputVideoFormat = null;

        // We will get these from the encoders when notified of a format change.
        MediaFormat encoderOutputAudioFormat = null;
        MediaFormat encoderOutputVideoFormat = null;

        int outputAudioTrack = -1;
        int outputVideoTrack = -1;
        // Whether things are done
        boolean audioExtractorDone = false;
        boolean audioDecoderDone = false;
        boolean audioEncoderDone = false;
        boolean videoExtractorDone = false;
        boolean videoDecoderDone = false;
        boolean videoEncoderDone = false;
        int videoEncodedFrameCount = 0;
        // The audio decoder output buffer to process, -1 if none.
        int pendingAudioDecoderOutputBufferIndex = -1;
        boolean muxing = false;

        while ((!audioEncoderDone) || (!videoEncoderDone)) {
            
            // Process video
            while (!videoExtractorDone && (encoderOutputVideoFormat == null || muxing)) {
                int decoderInputBufferIndex = videoDecoder.dequeueInputBuffer(TIMEOUT_USEC);
                if (decoderInputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    break;
                }
                ByteBuffer decoderInputBuffer = videoDecoderInputBuffers[decoderInputBufferIndex];
                int size = videoExtractor.readSampleData(decoderInputBuffer, 0);
                long presentationTime = videoExtractor.getSampleTime();
                if (size >= 0) {
                    videoDecoder.queueInputBuffer(decoderInputBufferIndex, 0, size, presentationTime,
                            videoExtractor.getSampleFlags());
                }
                videoExtractorDone = !videoExtractor.advance();
                if (videoExtractorDone) {
                    videoDecoder.queueInputBuffer(decoderInputBufferIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                }
                break;
            }

            // Process audio
            while (!audioExtractorDone && (encoderOutputAudioFormat == null || muxing)) {
                int decoderInputBufferIndex = audioDecoder.dequeueInputBuffer(TIMEOUT_USEC);
                if (decoderInputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    break;
                }
                ByteBuffer decoderInputBuffer = audioDecoderInputBuffers[decoderInputBufferIndex];
                int size = audioExtractor.readSampleData(decoderInputBuffer, 0);
                long presentationTime = audioExtractor.getSampleTime();
                if (size >= 0) {
                    audioDecoder.queueInputBuffer(decoderInputBufferIndex, 0, size, presentationTime,
                            audioExtractor.getSampleFlags());
                }
                audioExtractorDone = !audioExtractor.advance();
                if (audioExtractorDone) {
                    audioDecoder.queueInputBuffer(decoderInputBufferIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                }
                break;
            }

            // Poll output frames from the video decoder and feed to Video
            // Encoder.
            while (!videoDecoderDone && (encoderOutputVideoFormat == null || muxing)) {
                int decoderOutputBufferIndex = videoDecoder.dequeueOutputBuffer(videoDecoderOutputBufferInfo, TIMEOUT_USEC);
                Logger.d(TAG, "[VideoCompress] video decoder to encoder , decoderOutputBufferIndex, " + decoderOutputBufferIndex);
                if (decoderOutputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    break;
                }               
                if (decoderOutputBufferIndex == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                    break;
                }
                if (decoderOutputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                    decoderOutputVideoFormat = videoDecoder.getOutputFormat();
                    break;
                }
                if ((videoDecoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                    videoDecoder.releaseOutputBuffer(decoderOutputBufferIndex, false);
                    break;
                }
                videoDecoder.releaseOutputBuffer(decoderOutputBufferIndex, true); 
                outputSurface.awaitNewImage();
                outputSurface.drawImage(true);
                //feed encoder the scaled frame
                int inputBufferIndex = videoEncoder.dequeueInputBuffer(TIMEOUT_USEC);

                if (inputBufferIndex >= 0) {
                    ByteBuffer inputBuffer = videoEncoderInputBuffers[inputBufferIndex];
                    inputBuffer.clear();
                    ByteBuffer decoderOutputBuffer = outputSurface.getFrame();

                    byte[] frame = new byte[(int) (mWidth * mHeight * 4)];
                    decoderOutputBuffer.get(frame);
                    decoderOutputBuffer.position(0);
                    try {
                        inputBuffer.put(frame);
                    } catch (Exception e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                    videoEncoder.queueInputBuffer(inputBufferIndex, 0, mWidth * mHeight * 4, 0, 0);
                }
                if ((videoDecoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    videoDecoderDone = true;
                    videoEncoder.signalEndOfInputStream();
                }
                if(mCancelCompression) {
                    Logger.d(TAG, "[VideoCompress] Compression has been cancelled in between"); 
                    return;
                } 
                break;
            }

            // Poll output frames from the audio decoder.
            // Do not poll if we already have a pending buffer to feed to the
            // encoder.
            while (!audioDecoderDone && pendingAudioDecoderOutputBufferIndex == -1
                    && (encoderOutputAudioFormat == null || muxing)) {
                int decoderOutputBufferIndex = audioDecoder.dequeueOutputBuffer(audioDecoderOutputBufferInfo, TIMEOUT_USEC);
                if (decoderOutputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    break;
                }
                if (decoderOutputBufferIndex == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                    audioDecoderOutputBuffers = audioDecoder.getOutputBuffers();
                    break;
                }
                if (decoderOutputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                    decoderOutputAudioFormat = audioDecoder.getOutputFormat();
                    break;
                }

                if ((audioDecoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                    audioDecoder.releaseOutputBuffer(decoderOutputBufferIndex, false);
                    break;
                }

                pendingAudioDecoderOutputBufferIndex = decoderOutputBufferIndex;
                break;
            }
            // Feed the pending decoded audio buffer to the audio encoder.
            while (pendingAudioDecoderOutputBufferIndex != -1) {
                int encoderInputBufferIndex = audioEncoder.dequeueInputBuffer(TIMEOUT_USEC);
                Logger.d(TAG, "[VideoCompress] audio decoder to encoder, encoderInputBufferIndex, " + encoderInputBufferIndex );
                if (encoderInputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    break;
                }
                if (encoderInputBufferIndex == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {                   
                    break;
                }
                if (encoderInputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {                    
                    break;
                }
                if(mCancelCompression) {
                    Logger.d(TAG, "[VideoCompress] Compression has been cancelled in between"); 
                    return;
                }                  
                ByteBuffer encoderInputBuffer = audioEncoderInputBuffers[encoderInputBufferIndex];
                int size = audioDecoderOutputBufferInfo.size;
                long presentationTime = audioDecoderOutputBufferInfo.presentationTimeUs;                  
                int numChannels = decoderOutputAudioFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT);
                if (size >= 0) {
                    /*if(numChannels > 1) {
                        size = size/numChannels;
                    }*/
                    ByteBuffer decoderOutputBuffer = audioDecoderOutputBuffers[pendingAudioDecoderOutputBufferIndex]
                            .duplicate();
                    Logger.d(TAG, "[VideoCompress] audio decoder to encoder,  size=, " + size + ",decoderOutputBuffer=" + 
                            decoderOutputBuffer + "&numChannels =" + numChannels );  
                    decoderOutputBuffer.position(audioDecoderOutputBufferInfo.offset);
                    decoderOutputBuffer.limit(audioDecoderOutputBufferInfo.offset + size);
                    encoderInputBuffer.position(0);
                    try {
                        
                        encoderInputBuffer.put(decoderOutputBuffer);
                    } catch (Exception e) {
                        e.printStackTrace();
                        break;
                        // TODO Auto-generated catch block
                        
                    }
                    audioEncoder.queueInputBuffer(encoderInputBufferIndex, 0, size, presentationTime,
                            audioDecoderOutputBufferInfo.flags);
                }
                audioDecoder.releaseOutputBuffer(pendingAudioDecoderOutputBufferIndex, false);
                pendingAudioDecoderOutputBufferIndex = -1;
                if ((audioDecoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    audioDecoderDone = true;
                }
                // We enqueued a pending frame, let's try something else next.
                break;
            }

            // Poll frames from the video encoder and send them to the muxer.

            while (!videoEncoderDone && (encoderOutputVideoFormat == null || muxing)) {
                int encoderOutputBufferIndex = videoEncoder.dequeueOutputBuffer(videoEncoderOutputBufferInfo, TIMEOUT_USEC);
                Logger.d(TAG, "[VideoCompress] video encoder to muxer, encoderInputBufferIndex, " + encoderOutputBufferIndex );
                if (encoderOutputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    break;
                }
                if (encoderOutputBufferIndex == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                    videoEncoderOutputBuffers = videoEncoder.getOutputBuffers();
                    break;
                }
                if (encoderOutputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                    encoderOutputVideoFormat = videoEncoder.getOutputFormat();
                    break;
                }
                if(mCancelCompression) {
                    Logger.d(TAG, "[VideoCompress] Compression has been cancelled in between"); 
                    return;
                }
                ByteBuffer encoderOutputBuffer = videoEncoderOutputBuffers[encoderOutputBufferIndex];
                if ((videoEncoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                    // Simply ignore codec config buffers.
                    videoEncoder.releaseOutputBuffer(encoderOutputBufferIndex, false);
                    break;
                }
                videoEncodedFrameCount++;
                Logger.d(TAG, "[VideoCompress] video encoder to muxer, videoEncodedFrameCount, " + videoEncodedFrameCount );
                if (videoEncoderOutputBufferInfo.size != 0) {
                    videoEncoderOutputBufferInfo.presentationTimeUs = ((1000000 / OUTPUT_VIDEO_FRAME_RATE) * videoEncodedFrameCount);
                    
                    muxer.writeSampleData(outputVideoTrack, encoderOutputBuffer, videoEncoderOutputBufferInfo);
                }
                if ((videoEncoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    videoEncoderDone = true;
                }
                videoEncoder.releaseOutputBuffer(encoderOutputBufferIndex, true);
                break;
            }

            // Poll frames from the audio encoder and send them to the muxer.
            while (!audioEncoderDone && (encoderOutputAudioFormat == null || muxing)) {
                int encoderOutputBufferIndex = audioEncoder.dequeueOutputBuffer(audioEncoderOutputBufferInfo, TIMEOUT_USEC);
                Logger.d(TAG, "[VideoCompress] audio encoder to muxer, encoderOutputBufferIndex, " + encoderOutputBufferIndex );
                if (encoderOutputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    break;
                }
                if (encoderOutputBufferIndex == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                    audioEncoderOutputBuffers = audioEncoder.getOutputBuffers();
                    break;
                }
                if (encoderOutputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {

                    encoderOutputAudioFormat = audioEncoder.getOutputFormat();
                    break;
                }
                if(mCancelCompression) {
                    Logger.d(TAG, "[VideoCompress] Compression has been cancelled in between"); 
                    return;
                } 
                ByteBuffer encoderOutputBuffer = audioEncoderOutputBuffers[encoderOutputBufferIndex];
                if ((audioEncoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                    // Simply ignore codec config buffers.
                    audioEncoder.releaseOutputBuffer(encoderOutputBufferIndex, false);
                    break;
                }
                if (audioEncoderOutputBufferInfo.size != 0) {
                    muxer.writeSampleData(outputAudioTrack, encoderOutputBuffer, audioEncoderOutputBufferInfo);
                }
                if ((audioEncoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    audioEncoderDone = true;
                }
                audioEncoder.releaseOutputBuffer(encoderOutputBufferIndex, false);
                // We enqueued an encoded frame, let's try something else next.
                break;
            }
            if (!muxing && (encoderOutputAudioFormat != null) && (encoderOutputVideoFormat != null)) {
                outputAudioTrack = muxer.addTrack(encoderOutputAudioFormat);
                outputVideoTrack = muxer.addTrack(encoderOutputVideoFormat);
                muxer.start();
                muxing = true;
            }            
           
            if(mCancelCompression) {
                Logger.d(TAG, "Compression has been cancelled in between"); 
                return;
            }
           
        }
        compressignDone = true;

    }

    private static boolean isAudioFormat(MediaFormat format) {
        return getMimeTypeFor(format).startsWith("audio/");
    }

    private static boolean isVideoFormat(MediaFormat format) {
        return getMimeTypeFor(format).startsWith("video/");
    }

    private static String getMimeTypeFor(MediaFormat format) {
        return format.getString(MediaFormat.KEY_MIME);
    }

    /**
     * Returns the first codec capable of encoding the specified MIME type, or
     * null if no match was found.
     */
    private static MediaCodecInfo selectCodec(String mimeType) {
        int numCodecs = MediaCodecList.getCodecCount();
        for (int i = 0; i < numCodecs; i++) {
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
            if (!codecInfo.isEncoder()) {
                continue;
            }
            String[] types = codecInfo.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase(mimeType)) {
                    return codecInfo;
                }
            }
        }
        return null;
    }

    private static MediaCodec createVideoEncoder(MediaFormat inputFormat, AtomicReference<Surface> surfaceReference ) {
        try {
            MediaCodec encoder = MediaCodec.createEncoderByType(getMimeTypeFor(inputFormat));
            encoder.configure(inputFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            //surfaceReference.set(encoder.createInputSurface());

            encoder.start();
            return encoder;
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        }
    }

    private static MediaCodec createVideoDecoder(MediaFormat inputFormat, CodecOutputSurface surface) {
        try {
            MediaCodec decoder = MediaCodec.createDecoderByType(getMimeTypeFor(inputFormat));
            decoder.configure(inputFormat, surface.getSurface(), null, 0);
            decoder.start();
            return decoder;
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        }
    }

    private static MediaExtractor createExtractor(String file)
            throws IOException {
        MediaExtractor extractor;
        Logger.d(TAG,"createExtractor file = " + file);     
        extractor = new MediaExtractor();
        /*
         * extractor.setDataSource(fd, sampleFD.getStartOffset(),
         * sampleFD.getLength());
         */
        extractor.setDataSource(file);
        return extractor;

    }

    private static MediaCodec createAudioDecoder(MediaFormat inputFormat) {
        try {
            MediaCodec decoder = null;

            decoder = MediaCodec.createDecoderByType(getMimeTypeFor(inputFormat));

            decoder.configure(inputFormat, null, null, 0);
            decoder.start();
            return decoder;
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        }
    }

    private static MediaCodec createAudioEncoder(MediaCodecInfo codecInfo, MediaFormat format) {
        try {
            MediaCodec encoder = null;
            encoder = MediaCodec.createByCodecName(codecInfo.getName());

            encoder.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            encoder.start();
            return encoder;
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        }
    }

    private static MediaMuxer createMuxer() throws IOException {
        return new MediaMuxer(mOutputFile, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
    }

    private static int getAndSelectAudioTrackIndex(MediaExtractor extractor) {
       
        for (int index = 0; index < extractor.getTrackCount(); ++index) {
            Logger.d(TAG,"[VideoCompress] getAndSelectAudioTrackIndex index = " + index);
            if (isAudioFormat(extractor.getTrackFormat(index))) {
                Logger.d(TAG,"getAndSelectAudioTrackIndex isAudioFormat = " + index);
                extractor.selectTrack(index);
                return index;
            }
        }
        Logger.d(TAG,"[VideoCompress] getAndSelectAudioTrackIndex return -1 ");
        return -1;
    }

    private static int getAndSelectVideoTrackIndex(MediaExtractor extractor) {
        for (int index = 0; index < extractor.getTrackCount(); ++index) {
            if (isVideoFormat(extractor.getTrackFormat(index))) {
                extractor.selectTrack(index);
                return index;
            }
        }
        return -1;
    }

    public static void writeBytesToFile( ByteBuffer pixelBuffer, String filename) throws IOException {
        pixelBuffer.rewind();
        GLES20.glReadPixels(0, 0, mWidth, mHeight, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, pixelBuffer);
        BufferedOutputStream bos = null;
        try {
            bos = new BufferedOutputStream(new FileOutputStream(filename,false));
            Bitmap bmp = Bitmap.createBitmap(mWidth, mWidth, Bitmap.Config.ARGB_8888);
            pixelBuffer.rewind();
            bmp.copyPixelsFromBuffer(pixelBuffer);
            bmp.compress(Bitmap.CompressFormat.PNG, 100, bos);
            bmp.recycle();
        } finally {
            if (bos != null)
                bos.close();
        }
    }

    /**
     * Holds state associated with a Surface used for MediaCodec decoder output.
     * <p>
     * The constructor for this class will prepare GL, create a SurfaceTexture,
     * and then create a Surface for that SurfaceTexture. The Surface can be
     * passed to MediaCodec.configure() to receive decoder output. When a frame
     * arrives, we latch the texture with updateTexImage(), then render the
     * texture with GL to a pbuffer.
     * <p>
     * By default, the Surface will be using a BufferQueue in asynchronous mode,
     * so we can potentially drop frames.
     */
    private static class CodecOutputSurface implements SurfaceTexture.OnFrameAvailableListener {
        private STextureRender mTextureRender;
        private SurfaceTexture mSurfaceTexture;
        private Surface mSurface;

        private EGLDisplay mEGLDisplay = EGL14.EGL_NO_DISPLAY;
        private EGLContext mEGLContext = EGL14.EGL_NO_CONTEXT;
        private EGLSurface mEGLSurface = EGL14.EGL_NO_SURFACE;
        int mWidth;
        int mHeight;

        private Object mFrameSyncObject = new Object(); // guards
                                                        // mFrameAvailable
        private boolean mFrameAvailable;

        private ByteBuffer mPixelBuf; // used by saveFrame()

        /**
         * Creates a CodecOutputSurface backed by a pbuffer with the specified
         * dimensions. The new EGL context and surface will be made current.
         * Creates a Surface that can be passed to MediaCodec.configure().
         */
        public CodecOutputSurface(int width, int height) {
            if (width <= 0 || height <= 0) {
                throw new IllegalArgumentException();
            }
            mWidth = width;
            mHeight = height;

            eglSetup();
            makeCurrent();
            setup();
        }

        /**
         * Creates interconnected instances of TextureRender, SurfaceTexture,
         * and Surface.
         */
        private void setup() {
            mTextureRender = new STextureRender();
            mTextureRender.surfaceCreated();

            mSurfaceTexture = new SurfaceTexture(mTextureRender.getTextureId());

            // This doesn't work if this object is created on the thread that
            // CTS started for
            // these test cases.
            //
            // The CTS-created thread has a Looper, and the SurfaceTexture
            // constructor will
            // create a Handler that uses it. The "frame available" message is
            // delivered
            // there, but since we're not a Looper-based thread we'll never see
            // it. For
            // this to do anything useful, CodecOutputSurface must be created on
            // a thread without
            // a Looper, so that SurfaceTexture uses the main application Looper
            // instead.
            //
            // Java language note: passing "this" out of a constructor is
            // generally unwise,
            // but we should be able to get away with it here.
            mSurfaceTexture.setOnFrameAvailableListener(this);

            mSurface = new Surface(mSurfaceTexture);

            mPixelBuf = ByteBuffer.allocateDirect(mWidth * mHeight * 4);
            mPixelBuf.order(ByteOrder.LITTLE_ENDIAN);
        }

        /**
         * Prepares EGL. We want a GLES 2.0 context and a surface that supports
         * pbuffer.
         */
        private void eglSetup() {
            mEGLDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
            if (mEGLDisplay == EGL14.EGL_NO_DISPLAY) {
                throw new RuntimeException("unable to get EGL14 display");
            }
            int[] version = new int[2];
            if (!EGL14.eglInitialize(mEGLDisplay, version, 0, version, 1)) {
                mEGLDisplay = null;
                throw new RuntimeException("unable to initialize EGL14");
            }

            // Configure EGL for pbuffer and OpenGL ES 2.0, 24-bit RGB.
            int[] attribList = { EGL14.EGL_RED_SIZE, 8, EGL14.EGL_GREEN_SIZE, 8, EGL14.EGL_BLUE_SIZE, 8,
                    EGL14.EGL_ALPHA_SIZE, 8, EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT, EGL14.EGL_SURFACE_TYPE,
                    EGL14.EGL_PBUFFER_BIT, EGL14.EGL_NONE };
            EGLConfig[] configs = new EGLConfig[1];
            int[] numConfigs = new int[1];
            if (!EGL14.eglChooseConfig(mEGLDisplay, attribList, 0, configs, 0, configs.length, numConfigs, 0)) {
                throw new RuntimeException("unable to find RGB888+recordable ES2 EGL config");
            }

            // Configure context for OpenGL ES 2.0.
            int[] attrib_list = { EGL14.EGL_CONTEXT_CLIENT_VERSION, 2, EGL14.EGL_NONE };
            mEGLContext = EGL14.eglCreateContext(mEGLDisplay, configs[0], EGL14.EGL_NO_CONTEXT, attrib_list, 0);
            checkEglError("eglCreateContext");
            if (mEGLContext == null) {
                throw new RuntimeException("null context");
            }

            // Create a pbuffer surface.
            int[] surfaceAttribs = { EGL14.EGL_WIDTH, mWidth, EGL14.EGL_HEIGHT, mHeight, EGL14.EGL_NONE };
            mEGLSurface = EGL14.eglCreatePbufferSurface(mEGLDisplay, configs[0], surfaceAttribs, 0);
            checkEglError("eglCreatePbufferSurface");
            if (mEGLSurface == null) {
                throw new RuntimeException("surface was null");
            }
        }

        /**
         * Discard all resources held by this class, notably the EGL context.
         */
        public void release() {
            if (mEGLDisplay != EGL14.EGL_NO_DISPLAY) {
                EGL14.eglDestroySurface(mEGLDisplay, mEGLSurface);
                EGL14.eglDestroyContext(mEGLDisplay, mEGLContext);
                EGL14.eglReleaseThread();
                EGL14.eglTerminate(mEGLDisplay);
            }
            mEGLDisplay = EGL14.EGL_NO_DISPLAY;
            mEGLContext = EGL14.EGL_NO_CONTEXT;
            mEGLSurface = EGL14.EGL_NO_SURFACE;

            mSurface.release();

            // this causes a bunch of warnings that appear harmless but might
            // confuse someone:
            // W BufferQueue: [unnamed-3997-2] cancelBuffer: BufferQueue has
            // been abandoned!
            // mSurfaceTexture.release();

            mTextureRender = null;
            mSurface = null;
            mSurfaceTexture = null;
        }

        /**
         * Makes our EGL context and surface current.
         */
        public void makeCurrent() {
            if (!EGL14.eglMakeCurrent(mEGLDisplay, mEGLSurface, mEGLSurface, mEGLContext)) {
                throw new RuntimeException("eglMakeCurrent failed");
            }
        }

        /**
         * Returns the Surface.
         */
        public Surface getSurface() {
            return mSurface;
        }

        /**
         * Latches the next buffer into the texture. Must be called from the
         * thread that created the CodecOutputSurface object. (More
         * specifically, it must be called on the thread with the EGLContext
         * that contains the GL texture object used by SurfaceTexture.)
         */
        public void awaitNewImage() {
            final int TIMEOUT_MS = 5000;

            synchronized (mFrameSyncObject) {
                while (!mFrameAvailable) {
                    try {
                        // Wait for onFrameAvailable() to signal us. Use a
                        // timeout to avoid
                        // stalling the test if it doesn't arrive.
                        mFrameSyncObject.wait(TIMEOUT_MS);
                        if (!mFrameAvailable) {
                            // TODO: if "spurious wakeup", continue while loop
                            throw new RuntimeException("frame time out");
                            
                        }
                    } catch (InterruptedException ie) {
                        // shouldn't happen
                        throw new RuntimeException(ie);
                    }
                }
                mFrameAvailable = false;
            }

            // Latch the data.
            mTextureRender.checkGlError("before updateTexImage");
            mSurfaceTexture.updateTexImage();
        }

        /**
         * Draws the data from SurfaceTexture onto the current EGL surface.
         * 
         * @param invert
         *            if set, render the image with Y inverted (0,0 in top left)
         */
        public void drawImage(boolean invert) {
            mTextureRender.drawFrame(mSurfaceTexture, invert);
        }

        // SurfaceTexture callback
        @Override
        public void onFrameAvailable(SurfaceTexture st) {
            synchronized (mFrameSyncObject) {
                if (mFrameAvailable) {
                    throw new RuntimeException("mFrameAvailable already set, frame could be dropped");
                }
                mFrameAvailable = true;
                mFrameSyncObject.notifyAll();
            }
        }

        public ByteBuffer getFrame()
        { 
            mPixelBuf.rewind();
            GLES20.glReadPixels(0, 0, mWidth, mHeight, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, mPixelBuf);
            return mPixelBuf;
        
        }
        /**
         * Saves the current frame to disk as a PNG image.
         */
        public void saveFrame(String filename) throws IOException {
            // glReadPixels gives us a ByteBuffer filled with what is
            // essentially big-endian RGBA
            // data (i.e. a byte of red, followed by a byte of green...). To use
            // the Bitmap
            // constructor that takes an int[] array with pixel data, we need an
            // int[] filled
            // with little-endian ARGB data.
            //
            // If we implement this as a series of buf.get() calls, we can spend
            // 2.5 seconds just
            // copying data around for a 720p frame. It's better to do a bulk
            // get() and then
            // rearrange the data in memory. (For comparison, the PNG compress
            // takes about 500ms
            // for a trivial frame.)
            //
            // So... we set the ByteBuffer to little-endian, which should turn
            // the bulk IntBuffer
            // get() into a straight memcpy on most Android devices. Our ints
            // will hold ABGR data.
            // Swapping B and R gives us ARGB. We need about 30ms for the bulk
            // get(), and another
            // 270ms for the color swap.
            //
            // We can avoid the costly B/R swap here if we do it in the fragment
            // shader (see
            // http://stackoverflow.com/questions/21634450/ ).
            //
            // Having said all that... it turns out that the
            // Bitmap#copyPixelsFromBuffer()
            // method wants RGBA pixels, not ARGB, so if we create an empty
            // bitmap and then
            // copy pixel data in we can avoid the swap issue entirely, and just
            // copy straight
            // into the Bitmap from the ByteBuffer.
            //
            // Making this even more interesting is the upside-down nature of
            // GL, which means
            // our output will look upside-down relative to what appears on
            // screen if the
            // typical GL conventions are used. (For ExtractMpegFrameTest, we
            // avoid the issue
            // by inverting the frame when we render it.)
            //
            // Allocating large buffers is expensive, so we really want
            // mPixelBuf to be
            // allocated ahead of time if possible. We still get some
            // allocations from the
            // Bitmap / PNG creation.

            mPixelBuf.rewind();
            GLES20.glReadPixels(0, 0, mWidth, mHeight, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, mPixelBuf);

            BufferedOutputStream bos = null;
            try {
                bos = new BufferedOutputStream(new FileOutputStream(filename,false));
                Bitmap bmp = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.ARGB_8888);
                mPixelBuf.rewind();
                bmp.copyPixelsFromBuffer(mPixelBuf);
                bmp.compress(Bitmap.CompressFormat.PNG, 100, bos);
                bmp.recycle();
            } finally {
                if (bos != null)
                    bos.close();
            }

        }

        /**
         * Checks for EGL errors.
         */
        private void checkEglError(String msg) {
            int error;
            if ((error = EGL14.eglGetError()) != EGL14.EGL_SUCCESS) {
                throw new RuntimeException(msg + ": EGL error: 0x" + Integer.toHexString(error));
            }
        }
    }

    /**
     * Code for rendering a texture onto a surface using OpenGL ES 2.0.
     */
    private static class STextureRender {
        private static final int FLOAT_SIZE_BYTES = 4;
        private static final int TRIANGLE_VERTICES_DATA_STRIDE_BYTES = 5 * FLOAT_SIZE_BYTES;
        private static final int TRIANGLE_VERTICES_DATA_POS_OFFSET = 0;
        private static final int TRIANGLE_VERTICES_DATA_UV_OFFSET = 3;
        private final float[] mTriangleVerticesData = {
                // X, Y, Z, U, V
                -1.0f, -1.0f, 0, 0.f, 0.f, 1.0f, -1.0f, 0, 1.f, 0.f, -1.0f, 1.0f, 0, 0.f, 1.f, 1.0f, 1.0f, 0, 1.f, 1.f, };

        private FloatBuffer mTriangleVertices;

        private static final String VERTEX_SHADER = "uniform mat4 uMVPMatrix;\n" + "uniform mat4 uSTMatrix;\n"
                + "attribute vec4 aPosition;\n" + "attribute vec4 aTextureCoord;\n" + "varying vec2 vTextureCoord;\n"
                + "void main() {\n" + "    gl_Position = uMVPMatrix * aPosition;\n"
                + "    vTextureCoord = (uSTMatrix * aTextureCoord).xy;\n" + "}\n";

        private static final String FRAGMENT_SHADER = "#extension GL_OES_EGL_image_external : require\n"
                + "precision mediump float;\n"
                + // highp here doesn't seem to matter
                "varying vec2 vTextureCoord;\n" + "uniform samplerExternalOES sTexture;\n" + "void main() {\n"
                + "    gl_FragColor = texture2D(sTexture, vTextureCoord);\n" + "}\n";

        private float[] mMVPMatrix = new float[16];
        private float[] mSTMatrix = new float[16];

        private int mProgram;
        private int mTextureID = -12345;
        private int muMVPMatrixHandle;
        private int muSTMatrixHandle;
        private int maPositionHandle;
        private int maTextureHandle;

        public STextureRender() {
            mTriangleVertices = ByteBuffer.allocateDirect(mTriangleVerticesData.length * FLOAT_SIZE_BYTES)
                    .order(ByteOrder.nativeOrder()).asFloatBuffer();
            mTriangleVertices.put(mTriangleVerticesData).position(0);

            Matrix.setIdentityM(mSTMatrix, 0);
        }

        public int getTextureId() {
            return mTextureID;
        }

        /**
         * Draws the external texture in SurfaceTexture onto the current EGL
         * surface.
         */
        public void drawFrame(SurfaceTexture st, boolean invert) {
            checkGlError("onDrawFrame start");
            st.getTransformMatrix(mSTMatrix);
            if (invert) {
                mSTMatrix[5] = -mSTMatrix[5];
                mSTMatrix[13] = 1.0f - mSTMatrix[13];
            }

            // (optional) clear to green so we can see if we're failing to set
            // pixels
            GLES20.glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

            GLES20.glUseProgram(mProgram);
            checkGlError("glUseProgram");

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureID);

            mTriangleVertices.position(TRIANGLE_VERTICES_DATA_POS_OFFSET);
            GLES20.glVertexAttribPointer(maPositionHandle, 3, GLES20.GL_FLOAT, false, TRIANGLE_VERTICES_DATA_STRIDE_BYTES,
                    mTriangleVertices);
            checkGlError("glVertexAttribPointer maPosition");
            GLES20.glEnableVertexAttribArray(maPositionHandle);
            checkGlError("glEnableVertexAttribArray maPositionHandle");

            mTriangleVertices.position(TRIANGLE_VERTICES_DATA_UV_OFFSET);
            GLES20.glVertexAttribPointer(maTextureHandle, 2, GLES20.GL_FLOAT, false, TRIANGLE_VERTICES_DATA_STRIDE_BYTES,
                    mTriangleVertices);
            checkGlError("glVertexAttribPointer maTextureHandle");
            GLES20.glEnableVertexAttribArray(maTextureHandle);
            checkGlError("glEnableVertexAttribArray maTextureHandle");

            Matrix.setIdentityM(mMVPMatrix, 0);
            GLES20.glUniformMatrix4fv(muMVPMatrixHandle, 1, false, mMVPMatrix, 0);
            GLES20.glUniformMatrix4fv(muSTMatrixHandle, 1, false, mSTMatrix, 0);

            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
            checkGlError("glDrawArrays");

            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);
        }

        /**
         * Initializes GL state. Call this after the EGL surface has been
         * created and made current.
         */
        public void surfaceCreated() {
            mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
            if (mProgram == 0) {
                throw new RuntimeException("failed creating program");
            }

            maPositionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
            checkLocation(maPositionHandle, "aPosition");
            maTextureHandle = GLES20.glGetAttribLocation(mProgram, "aTextureCoord");
            checkLocation(maTextureHandle, "aTextureCoord");

            muMVPMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uMVPMatrix");
            checkLocation(muMVPMatrixHandle, "uMVPMatrix");
            muSTMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uSTMatrix");
            checkLocation(muSTMatrixHandle, "uSTMatrix");

            int[] textures = new int[1];
            GLES20.glGenTextures(1, textures, 0);

            mTextureID = textures[0];
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureID);
            checkGlError("glBindTexture mTextureID");

            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
            checkGlError("glTexParameter");
        }

        /**
         * Replaces the fragment shader. Pass in null to reset to default.
         */
        public void changeFragmentShader(String fragmentShader) {
            if (fragmentShader == null) {
                fragmentShader = FRAGMENT_SHADER;
            }
            GLES20.glDeleteProgram(mProgram);
            mProgram = createProgram(VERTEX_SHADER, fragmentShader);
            if (mProgram == 0) {
                throw new RuntimeException("failed creating program");
            }
        }

        private int loadShader(int shaderType, String source) {
            int shader = GLES20.glCreateShader(shaderType);
            checkGlError("glCreateShader type=" + shaderType);
            GLES20.glShaderSource(shader, source);
            GLES20.glCompileShader(shader);
            int[] compiled = new int[1];
            GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
           if (compiled[0] == 0) {

                GLES20.glDeleteShader(shader);
                shader = 0;
            }
            return shader;
        }

        private int createProgram(String vertexSource, String fragmentSource) {
            int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
            if (vertexShader == 0) {
                return 0;
            }
            int pixelShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
            if (pixelShader == 0) {
                return 0;
            }

            int program = GLES20.glCreateProgram();

            GLES20.glAttachShader(program, vertexShader);
            checkGlError("glAttachShader");
            GLES20.glAttachShader(program, pixelShader);
            checkGlError("glAttachShader");
            GLES20.glLinkProgram(program);
            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
            if (linkStatus[0] != GLES20.GL_TRUE) {

                GLES20.glDeleteProgram(program);
                program = 0;
            }
            return program;
        }

        public void checkGlError(String op) {
            int error;
            while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
                throw new RuntimeException(op + ": glError " + error);
            }
        }

        public static void checkLocation(int location, String label) {
            if (location < 0) {
                throw new RuntimeException("Unable to locate '" + label + "' in program");
            }
        }
    }

}

