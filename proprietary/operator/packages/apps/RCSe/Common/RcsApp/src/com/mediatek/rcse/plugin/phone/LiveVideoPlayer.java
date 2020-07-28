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

package com.mediatek.rcse.plugin.phone;

import java.io.IOException;
import java.util.Iterator;
import java.util.Vector;
import java.io.FileOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import com.gsma.services.rcs.sharing.video.VideoCodec;
import com.gsma.services.rcs.sharing.video.VideoPlayer;
import android.util.Log;
import java.nio.ByteBuffer;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.util.Log;
import android.hardware.Camera;
import android.os.RemoteException;
import android.os.SystemClock;

import com.mediatek.rcse.rtp.AndroidDatagramConnection;
import com.mediatek.rcse.rtp.MediaRegistry;
import com.mediatek.rcse.rtp.VideoRtpSender;
import com.mediatek.rcse.rtp.codec.video.h264.H264Config;
import com.mediatek.rcse.rtp.codec.video.h264.JavaPacketizer;
import com.mediatek.rcse.rtp.codec.video.h264.NalUnitHeader;
import com.mediatek.rcse.rtp.codec.video.h264.NalUnitType;
import com.mediatek.rcse.rtp.codec.video.h264.encoder.NativeH264EncoderParams;

import com.mediatek.rcse.rtp.format.video.CameraOptions;
import com.mediatek.rcse.rtp.format.video.Orientation;
import com.mediatek.rcse.rtp.format.video.VideoFormat;
import com.mediatek.rcse.rtp.format.video.VideoOrientation;
import com.mediatek.rcse.rtp.media.MediaException;
import com.mediatek.rcse.rtp.media.MediaInput;
import com.mediatek.rcse.rtp.media.VideoSample;
import com.mediatek.rcse.rtp.stream.RtpStreamListener;
import com.mediatek.rcse.rtp.DatagramConnection;
import com.mediatek.rcse.rtp.CodecsUtils;
import com.mediatek.rcse.rtp.FifoBuffer;
import com.mediatek.rcse.rtp.NetworkRessourceManager;
import com.mediatek.rcse.rtp.Logger;

/**
 * Live RTP video player. Only the H264 QCIF format is supported.
 */
public class LiveVideoPlayer extends VideoPlayer implements
		Camera.PreviewCallback, RtpStreamListener {

	/**
	 * List of supported video codecs
	 */
	private VideoCodec[] supportedMediaCodecs = null;

	/**
	 * Selected video codec
	 */
	private VideoCodec mSelectedVideoCodec = null;
	private static final String TAG = "ENCODER";

	private MediaCodec mEncoder = null;
	private int result = -1;
	/**
	 * Video format
	 */
	private VideoFormat videoFormat;

	/**
	 * AudioRenderer for RTP stream sharing
	 */
	//private LiveVideoRenderer videoRenderer = null;

	/**
	 * Local RTP port
	 */
	private int localRtpPort;

	/**
	 * RTP sender session
	 */
	private VideoRtpSender rtpSender = null;

	/**
	 * RTP media input
	 */
	private MediaRtpInput rtpInput = null;

	/**
	 * Is player opened
	 */
	private boolean opened = false;

	/**
	 * Is player started
	 */
	private boolean started = false;

	/**
	 * Video start time
	 */
	private long videoStartTime = 0L;

	/**
	 * Media event listeners
	 */
	//private Vector<VideoPlayerListener> listeners = new Vector<VideoPlayerListener>();

	/**
	 * Temporary connection to reserve the port
	 */
	private DatagramConnection temporaryConnection = null;

	private byte[] sps = null;
	private byte[] pps = null;

	/***
	 * Current time stamp
	 */
	private long timeStamp = 0;

	/**
	 * NAL initialization
	 */
	private boolean nalInit = false;

	/**
	 * Scaling factor for encoding
	 */
	private float scaleFactor = 1;

	/**
	 * Source Width - used for resizing
	 */
	private int srcWidth = 0;

	/**
	 * Source Height - used for resizing
	 */
	private int srcHeight = 0;

	/**
	 * Mirroring (horizontal and vertical) for encoding
	 */
	private boolean mirroring = false;

	/**
	 * Orientation header id.
	 */
	private int orientationHeaderId = -1;

	/**
	 * Camera ID
	 */
	private int cameraId = CameraOptions.FRONT.getValue();

	/**
	 * Video Orientation
	 */
	private Orientation mOrientation = Orientation.NONE;

	/**
	 * Frame process
	 */
	private FrameProcess frameProcess;

	/**
	 * Frame buffer
	 */
	private FrameBuffer frameBuffer = new FrameBuffer();

	/**
	 * The logger
	 */
	private Logger logger = Logger.getLogger(this.getClass().getName());

	/**
	 * Constructor
	 */
	public LiveVideoPlayer() {
		// Set the local RTP port
		localRtpPort = NetworkRessourceManager.generateLocalRtpPort();
		reservePort(localRtpPort);

		// Init codecs
		supportedMediaCodecs = CodecsUtils.getPlayerCodecList();

		// Set the default media codec
		if (supportedMediaCodecs.length > 0) {
			setVideoCodec(supportedMediaCodecs[0]);
		}

	}

	/**
	 * Constructor for sharing RTP stream with video renderer
	 * 
	 * @param vr
	 *            video renderer
	 *//*
	public LiveVideoPlayer(LiveVideoRenderer vr) {
		// Set the local RTP port
		localRtpPort = NetworkRessourceManager.generateLocalRtpPort();
		reservePort(localRtpPort);

		// Get and set locally the audio renderer reference
		videoRenderer = vr;

		// Init codecs
		supportedMediaCodecs = CodecsUtils.getPlayerCodecList();

		// Set the default media codec
		if (supportedMediaCodecs.length > 0) {
			setVideoCodec(supportedMediaCodecs[0]);
		}
	}*/

	/**
	 * Constructor with a list of video codecs
	 * 
	 * @param codecs
	 *            Ordered list of codecs (preferred codec in first)
	 */
	public LiveVideoPlayer(VideoCodec[] codecs) {
		// Set the local RTP port
		localRtpPort = NetworkRessourceManager.generateLocalRtpPort();
		reservePort(localRtpPort);

		// Init codecs
		supportedMediaCodecs = codecs;

		// Set the default media codec
		if (supportedMediaCodecs.length > 0) {
			setVideoCodec(supportedMediaCodecs[0]);
		}
	}

	/**
	 * Constructor with a list of video codecs and allowing to share RTP stream
	 * with video renderer
	 * 
	 * @param codecs
	 *            Ordered list of codecs (preferred codec in first)
	 * @param vr
	 *            video renderer
	 */
	/*public LiveVideoPlayer(VideoCodec[] codecs, LiveVideoRenderer vr) {
		// Set the local RTP port
		localRtpPort = NetworkRessourceManager.generateLocalRtpPort();
		reservePort(localRtpPort);

		// Get and set locally the audio renderer reference
		videoRenderer = vr;

		// Init codecs
		supportedMediaCodecs = codecs;

		// Set the default media codec
		if (supportedMediaCodecs.length > 0) {
			setVideoCodec(supportedMediaCodecs[0]);
		}
	}*/

	/**
	 * Returns the local RTP port
	 * 
	 * @return Port
	 */
	public int getLocalRtpPort() {
		return localRtpPort;
	}

	/**
	 * Reserve a port.
	 * 
	 * @param port
	 *            Port to reserve
	 */
	private void reservePort(int port) {
		if (temporaryConnection == null) {
			try {
				temporaryConnection = new AndroidDatagramConnection();
				temporaryConnection.open(port);
			} catch (IOException e) {
				temporaryConnection = null;
			}
		}
	}

	/**
	 * Release the reserved port.
	 */
	private void releasePort() {
		if (temporaryConnection != null) {
			try {
				temporaryConnection.close();
			} catch (IOException e) {
				temporaryConnection = null;
			}
		}
	}

	/**
	 * Return the video start time
	 * 
	 * @return Milliseconds
	 */
	public long getVideoStartTime() {
		return videoStartTime;
	}

	/**
	 * Is player opened
	 * 
	 * @return Boolean
	 */
	public boolean isOpened() {
		return opened;
	}

	/**
	 * Is player started
	 * 
	 * @return Boolean
	 */
	public boolean isStarted() {
		return started;
	}

	/**
	 * Open the player
	 * 
	 * @param remoteHost
	 *            Remote host
	 * @param remotePort
	 *            Remote port
	 */
	public void open(VideoCodec codec, String remoteHost, int remotePort) {
		if (opened) {
			// Already opened
			return;
		}

		// Check video codec
		if (codec == null) {
			notifyPlayerEventError("Video codec not selected");
			return;
		}

		mSelectedVideoCodec = codec;
		// Init video encoder
		try {
			NativeH264EncoderParams nativeH264EncoderParams = new NativeH264EncoderParams();

			// Codec dimensions
			nativeH264EncoderParams.setFrameWidth(mSelectedVideoCodec
					.getWidth());
			nativeH264EncoderParams.setFrameHeight(mSelectedVideoCodec
					.getHeight());
			nativeH264EncoderParams.setFrameRate(mSelectedVideoCodec
					.getFrameRate());
			nativeH264EncoderParams.setBitRate(mSelectedVideoCodec.getBitRate());

			// Codec profile and level
			nativeH264EncoderParams.setProfilesAndLevel(mSelectedVideoCodec
					.getParameters());

			// Codec settings optimization
			nativeH264EncoderParams
					.setEncMode(NativeH264EncoderParams.ENCODING_MODE_STREAMING);
			nativeH264EncoderParams.setSceneDetection(false);

			if (logger.isActivated()) {
				logger.info("Init H264Encoder "
						+ mSelectedVideoCodec.getParameters() + " "
						+ mSelectedVideoCodec.getWidth() + "x"
						+ mSelectedVideoCodec.getHeight() + " "
						+ mSelectedVideoCodec.getFrameRate() + " "
						+ mSelectedVideoCodec.getBitRate());
			}
			int result = InitEncoder(nativeH264EncoderParams);
			if (result != 0) {
				notifyPlayerEventError("Encoder init failed with error code "
						+ result);
				return;
			}
		} catch (UnsatisfiedLinkError e) {
			notifyPlayerEventError(e.getMessage());
			return;
		}

		// Init the RTP layer
		try {
			releasePort();
			rtpSender = new VideoRtpSender(videoFormat, localRtpPort);
			rtpInput = new MediaRtpInput();
			rtpInput.open();
			/*if (videoRenderer != null) {
				// The video renderer is supposed to be opened and so we used
				// its RTP stream
				if (logger.isActivated()) {
					logger.debug("Player shares the renderer RTP stream");
				}
				rtpSender.prepareSession(rtpInput, remoteHost, remotePort,
						videoRenderer.getRtpInputStream(), this);
			} else {*/
				// The video renderer doesn't exist and so we create a new RTP
				// stream
				rtpSender.prepareSession(rtpInput, remoteHost, remotePort, this);
			//}

		} catch (Exception e) {
			notifyPlayerEventError(e.getMessage());
			return;
		}

		// Player is opened
		opened = true;
		notifyPlayerEventOpened();
	}

	/**
	 * Close the player
	 */
	public synchronized void close() {
		if (!opened) {
			// Already closed
			return;
		}
		// Close the RTP layer
		rtpInput.close();
		rtpSender.stopSession();

		try {
			// Close the video encoder
			DeinitEncoder();
		} catch (UnsatisfiedLinkError e) {
			if (logger.isActivated()) {
				logger.error("Can't close correctly the encoder", e);
			}
		}

		// Player is closed
		opened = false;
		notifyPlayerEventClosed();
		//listeners.clear();
	}

	/**
	 * Start the player
	 */
	public synchronized void start() {
		if (!opened) {
			// Player not opened
			return;
		}

		if (started) {
			// Already started
			return;
		}

		nalInit = false;
		timeStamp = 0;

		// Start RTP layer
		rtpSender.startSession();

		// Player is started
		videoStartTime = SystemClock.uptimeMillis();
		started = true;
		frameProcess = new FrameProcess(mSelectedVideoCodec.getFrameRate());
		frameProcess.start();
		notifyPlayerEventStarted();
	}

	/**
	 * Stop the player
	 */
	public void stop() {
		if (!opened) {
			// Player not opened
			return;
		}

		if (!started) {
			// Already stopped
			return;
		}

		// Player is stopped
		videoStartTime = 0L;
		started = false;
		try {
			frameProcess.interrupt();
		} catch (Exception e) {
			// Nothing to do
		}
		notifyPlayerEventStopped();
	}

	/**
	 * Add a media event listener
	 * 
	 * @param listener
	 *            Media event listener
	 *//*
	public void addEventListener(VideoPlayerListener listener) {
		listeners.addElement(listener);
	}

	*//**
	 * Remove a media event listener
	 * 
	 * @param listener
	 *            Media event listener
	 *//*
	public void removeEventListener(VideoPlayerListener listener) {
		listeners.removeElement(listener);
	}

	*//**
	 * Remove all media event listeners
	 *//*
	public void removeAllListeners() {
		listeners.removeAllElements();
	}
*/
	/**
	 * Get supported video codecs
	 * 
	 * @return media Codecs list
	 */
	public VideoCodec[] getSupportedCodecs() {
		return supportedMediaCodecs;
	}

	/**
	 * Get video codec width
	 * 
	 * @return Width
	 */
	public int getVideoCodecWidth() {
		if (mSelectedVideoCodec == null) {
			return H264Config.VIDEO_WIDTH;
		} else {
			return mSelectedVideoCodec.getWidth();
		}
	}

	/**
	 * Get video codec height
	 * 
	 * @return Height
	 */
	public int getVideoCodecHeight() {
		if (mSelectedVideoCodec == null) {
			return H264Config.VIDEO_HEIGHT;
		} else {
			return mSelectedVideoCodec.getHeight();
		}
	}
	
	/**
     * Set the remote info
     * 
     * @param codec Video codec
     * @param remoteHost Remote RTP host
     * @param remotePort Remote RTP port
     * @param orientationHeaderId Orientation header extension ID. The extension ID is a value
     *            between 1 and 15 arbitrarily chosen by the sender, as defined in RFC5285
     */
    public void setRemoteInfo(VideoCodec codec, String remoteHost, int remotePort,
            int orientationHeaderId){
        // ToDO
    }

	/**
	 * Set video codec
	 * 
	 * @param mediaCodec
	 *            Video codec
	 */
	public void setVideoCodec(VideoCodec mediaCodec) {
		if (mediaCodec.compare(supportedMediaCodecs[0])) {
			if (mediaCodec.getHeight() == 0
					|| mediaCodec.getWidth() == 0) {
				mSelectedVideoCodec = new VideoCodec(mediaCodec.getEncoding(),
						mediaCodec.getPayloadType(), mediaCodec.getClockRate(),
						mediaCodec.getFrameRate(), mediaCodec.getBitRate(),
						H264Config.QCIF_WIDTH, H264Config.QCIF_HEIGHT,
						mediaCodec.getParameters());

			} else {
				mSelectedVideoCodec = mediaCodec;
			}
			videoFormat = (VideoFormat) MediaRegistry.generateFormat(mediaCodec
					.getEncoding());
		} else {
			notifyPlayerEventError("Codec not supported");
		}
	}

	/**
	 * Set extension header orientation id
	 * 
	 * @param headerId
	 *            extension header orientation id
	 */
	public void setOrientationHeaderId(int headerId) {
		this.orientationHeaderId = headerId;
	}

	/**
	 * Set camera ID
	 * 
	 * @param cameraId
	 *            Camera ID
	 */
	public void setCameraId(int cameraId) {
		this.cameraId = cameraId;
	}

	/**
	 * Set video orientation
	 * 
	 * @param orientation
	 */
	public void setOrientation(Orientation orientation) {
		mOrientation = orientation;
	}

	/**
	 * Set the scaling factor
	 * 
	 * @param scaleFactor
	 *            New scaling factor
	 */
	public void setScalingFactor(float scaleFactor) {
		this.scaleFactor = scaleFactor;
		this.srcWidth = 0;
		this.srcHeight = 0;
	}

	/**
	 * Set the source dimension for resizing
	 * 
	 * @param srcWidth
	 * @param srcHeight
	 */
	public void activateResizing(int srcWidth, int srcHeight) {
		this.srcWidth = srcWidth;
		this.srcHeight = srcHeight;
		this.scaleFactor = 1;
	}

	/**
	 * Set the mirroring value
	 * 
	 * @param mirroring
	 *            New mirroring value
	 */
	public void setMirroring(boolean mirroring) {
		this.mirroring = mirroring;
	}

	/**
	 * Notify RTP aborted
	 */
	public void rtpStreamAborted() {
		notifyPlayerEventError("RTP session aborted");
	}

	/**
	 * Notify player event started
	 */
	private void notifyPlayerEventStarted() {/*
		if (logger.isActivated()) {
			logger.debug("Player is started");
		}
		Iterator<VideoPlayerListener> ite = listeners.iterator();
		while (ite.hasNext()) {
			try {
				((VideoPlayerListener) ite.next()).onPlayerStarted();
			} catch (RemoteException e) {
				if (logger.isActivated()) {
					logger.error("Can't notify listener", e);
				}
			}
		}
	*/}

	/**
	 * Notify player event stopped
	 */
    private void notifyPlayerEventStopped() {/*
        if (logger.isActivated()) {
            logger.debug("Player is stopped");
        }
        Iterator<VideoPlayerListener> ite = listeners.iterator();
        while (ite.hasNext()) {
            try {
                ((VideoPlayerListener) ite.next()).onPlayerStopped();
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
    */}

	/**
	 * Notify player event opened
	 */
	private void notifyPlayerEventOpened() {/*
		if (logger.isActivated()) {
			logger.debug("Player is opened");
		}
		Iterator<VideoPlayerListener> ite = listeners.iterator();
		while (ite.hasNext()) {
			try {
				((VideoPlayerListener) ite.next()).onPlayerOpened();
			} catch (RemoteException e) {
				if (logger.isActivated()) {
					logger.error("Can't notify listener", e);
				}
			}
		}
	*/}

	/**
	 * Notify player event closed
	 */
	private void notifyPlayerEventClosed() {/*
		if (logger.isActivated()) {
			logger.debug("Player is closed");
		}
		Iterator<VideoPlayerListener> ite = listeners.iterator();
		while (ite.hasNext()) {
			try {
				((VideoPlayerListener) ite.next()).onPlayerClosed();
			} catch (RemoteException e) {
				if (logger.isActivated()) {
					logger.error("Can't notify listener", e);
				}
			}
		}
	*/}

	/**
	 * Notify player event error
	 */
	private void notifyPlayerEventError(String error) {/*
		if (logger.isActivated()) {
			logger.debug("Player error: " + error);
		}

		Iterator<VideoPlayerListener> ite = listeners.iterator();
		while (ite.hasNext()) {
			try {
				((VideoPlayerListener) ite.next()).onPlayerFailed();
			} catch (RemoteException e) {
				if (logger.isActivated()) {
					logger.error("Can't notify listener", e);
				}
			}
		}
	*/}

	/**
	 * Preview frame from the camera
	 * 
	 * @param data
	 *            Frame
	 * @param camera
	 *            Camera
	 */
	public void onPreviewFrame(byte[] data, Camera camera) {
		if (!started) {
			return;
		}
		frameBuffer.setData(data);
	};

	public int InitEncoder(NativeH264EncoderParams encoderParams) {

		 try{
		mEncoder = MediaCodec.createEncoderByType("video/avc");
		    }
		    catch(IOException ex){
		       
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

	
	/**
     * Converts (de-interleaves) NV21 to YUV420 planar.
     * Stride may be greater than width, slice height may be greater than height.
     */
    private byte[] NV21ToYUV420p(int width, int height,
            int stride, int sliceHeight, byte[] nv21) {
        byte[] yuv = new byte[width * height * 3 / 2];
        // Y plane we just copy.
        for (int i = 0; i < height; i++) {
            System.arraycopy(nv21, i * stride, yuv, i * width, width);
        }
        // U & V plane - de-interleave.
        int v_offset = width * height;
        int u_offset = v_offset + v_offset / 4;
        int nv_offset;
        for (int i = 0; i < height / 2; i++) {
            nv_offset = stride * (sliceHeight + i);
            for (int j = 0; j < width / 2; j++) {
                yuv[u_offset++] = nv21[nv_offset++];
                yuv[v_offset++] = nv21[nv_offset++];
            }
        }
        return yuv;
    }
	// Resize the frame and Encode
	public byte[] ResizeAndEncodeFrame(byte abyte0[], long l,
			boolean mirroring, int srcWidth, int srcHeight) {
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
					
					
				if (sps != null && pps != null) {
                    outputBuffer.position(4); //skip 0 0 0 1
					outputBuffer.get(encoded);
				} else {
                    byte[] outData = new byte[bufferInfo.size];
                    outputBuffer.get(outData);
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

	public void EncodeFrame(byte abyte0[], long l, boolean mirroring,
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
					outputBuffer.position(4);
					byte[] frameOut = new byte[bufferInfo.size -4];
					outputBuffer.get(frameOut);
					result = 0;

					// Set timestamp
					timeStamp = SystemClock.uptimeMillis() - videoStartTime;

					int encodeResult = getLastEncodeStatus();

					// see if sps pps are available
					if ((nalInit == false) && (getSps() != null)
							&& (getPps() != null)) {
						rtpInput.addFrame(getSps(), timeStamp);
						rtpInput.addFrame(getPps(), timeStamp);
						nalInit = true;
					}

					if ((encodeResult == 0) && (frameOut.length > 0)) {

						VideoOrientation videoOrientation = null;
						if (orientationHeaderId > 0) {
							videoOrientation = new VideoOrientation(
									orientationHeaderId,
									CameraOptions.convert(cameraId),
									mOrientation);
						}
						rtpInput.addFrame(frameOut, timeStamp, videoOrientation);
					}

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
		mEncoder = null;
		return 0;

	}

	public int getLastEncodeStatus() {
		return result;

	}

	/**
	 * encode a buffer and add in RTP input
	 * 
	 * @param data
	 */
	private void encode(byte[] data) {
		// Set timestamp
		timeStamp = SystemClock.uptimeMillis() - videoStartTime;

		
		EncodeFrame(NV21ToYUV420p(176, 144, 176, 144, data), timeStamp, mirroring, 1f);
		
	}

	/**
	 * Chech if the frame is IDR
	 * 
	 * @param encodedFrame
	 *            the encoded frame
	 * @return true if IDR
	 */
	private boolean isIdrFrame(byte[] encodedFrame) {
		if ((encodedFrame != null) && (encodedFrame.length > 0)) {
			NalUnitHeader header = NalUnitHeader.extract(encodedFrame);
			return header.getNalUnitType() == NalUnitType.CODE_SLICE_IDR_PICTURE;
		}
		return false;
	}

	/**
	 * Frame process
	 */
	private class FrameProcess extends Thread {

		/**
		 * Time between two frame
		 */
		private int interframe = 1000 / 15;

		/**
		 * Constructor
		 * 
		 * @param framerate
		 */
		public FrameProcess(int framerate) {
			super();
			interframe = 1000 / framerate;
		}

		@Override
		public void run() {
			byte[] frameData = null;
			while (started) {
				long time = System.currentTimeMillis();

				// Encode
				frameData = frameBuffer.getData();
				if (frameData != null) {
					encode(frameData);
				}

				// Sleep between frames if necessary
				long delta = System.currentTimeMillis() - time;
				if (delta < interframe) {
					try {
						Thread.sleep((interframe - delta)
								- (((interframe - delta) * 10) / 100));
					} catch (InterruptedException e) {
					}
				}
			}
		}
	}

	/**
	 * Frame buffer
	 */
	private class FrameBuffer {
		/**
		 * Data
		 */
		private byte[] data = null;

		/**
		 * Scaling factor for encoding
		 */
		public float dataScaleFactor = 1;

		/**
		 * Source Width - used for resizing
		 */
		public int dataSrcWidth = 0;

		/**
		 * Source Height - used for resizing
		 */
		public int dataSrcHeight = 0;

		/**
		 * Get the data
		 * 
		 * @return data
		 */
		public synchronized byte[] getData() {
			return data;
		}

		/**
		 * Set the data
		 * 
		 * @param data
		 */
		public synchronized void setData(byte[] data) {
			this.data = data;

			// Update resizing / scaling values
			this.dataScaleFactor = scaleFactor;
			this.dataSrcWidth = srcWidth;
			this.dataSrcHeight = srcHeight;
		}
	}

	/**
	 * Media RTP input
	 */
	private static class MediaRtpInput implements MediaInput {
		/**
		 * Received frames
		 */
		private FifoBuffer fifo = null;

		/**
		 * Constructor
		 */
		public MediaRtpInput() {
		}

		/**
		 * Add a new video frame
		 * 
		 * @param data
		 *            Data
		 * @param timestamp
		 *            Timestamp
		 * @param marker
		 *            Marker bit
		 */
		public void addFrame(byte[] data, long timestamp,
				VideoOrientation videoOrientation) {
			if (fifo != null) {
				VideoSample sample = new VideoSample(data, timestamp,
						videoOrientation);
				fifo.addObject(sample);
			}
		}

		/**
		 * Add a new video frame
		 * 
		 * @param data
		 *            Data
		 * @param timestamp
		 *            Timestamp
		 * @param marker
		 *            Marker bit
		 */
		public void addFrame(byte[] data, long timestamp) {
			addFrame(data, timestamp, null);
		}

		/**
		 * Open the player
		 */
		public void open() {
			fifo = new FifoBuffer();
		}

		/**
		 * Close the player
		 */
		public void close() {
			if (fifo != null) {
				fifo.close();
				fifo = null;
			}
		}

		/**
		 * Read a media sample (blocking method)
		 * 
		 * @return Media sample
		 * @throws MediaException
		 */
		public VideoSample readSample() throws MediaException {
			try {
				if (fifo != null) {
					return (VideoSample) fifo.getObject();
				} else {
					throw new MediaException("Media input not opened");
				}
			} catch (Exception e) {
				throw new MediaException("Can't read media sample");
			}
		}
	}

	@Override
	public VideoCodec getCodec() {
		if (mSelectedVideoCodec == null) {
			return null;
		} else {
			return mSelectedVideoCodec;
		}
	}
}
