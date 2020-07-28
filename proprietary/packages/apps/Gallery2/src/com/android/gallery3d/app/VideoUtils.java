/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
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
 */

// Modified example based on mp4parser google code open source project.
// http://code.google.com/p/mp4parser/source/browse/trunk/examples/src/main/java/com/googlecode/mp4parser/ShortenExample.java

package com.android.gallery3d.app;

import android.annotation.TargetApi;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaMetadataRetriever;
import android.media.MediaMuxer;
import android.os.Build;
import android.app.ProgressDialog;

import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.util.SaveVideoFileInfo;
import com.coremedia.iso.IsoFile;
import com.coremedia.iso.boxes.TimeToSampleBox;
import com.googlecode.mp4parser.authoring.Movie;
import com.googlecode.mp4parser.authoring.Track;
import com.googlecode.mp4parser.authoring.builder.DefaultMp4Builder;
import com.googlecode.mp4parser.authoring.container.mp4.MovieCreator;
import com.googlecode.mp4parser.authoring.tracks.CroppedTrack;
import com.mediatek.gallery3d.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

public class VideoUtils {
    private static final String LOGTAG = "Gallery2/VideoUtils";
    private static final int DEFAULT_BUFFER_SIZE = 1 * 1024 * 1024;

    /**
     * Remove the sound track.
     */
    public static boolean startMute(String filePath, SaveVideoFileInfo dstFileInfo,
            ProgressDialog mProgress)
            throws IOException {
        if (ApiHelper.HAS_MEDIA_MUXER) {
            return genVideoUsingMuxer(filePath, dstFileInfo.mFile.getPath(), -1, -1,
                           false, true, mProgress);
        } else {
            return startMuteUsingMp4Parser(filePath, dstFileInfo);
        }
    }

    /**
     * Shortens/Crops tracks
     */
     ///M: use for control main thread dialog display
    public static boolean startTrim(File src, File dst, int startMs, int endMs,
            TrimVideo trimVideo, ProgressDialog mProgress)
            throws IOException {
        if (ApiHelper.HAS_MEDIA_MUXER) {
            return genVideoUsingMuxer(src.getPath(), dst.getPath(), startMs, endMs,
                    true, true, mProgress);
        } else {
            return trimUsingMp4Parser(src, dst, startMs, endMs, trimVideo);
        }
    }

    private static boolean startMuteUsingMp4Parser(String filePath,
            SaveVideoFileInfo dstFileInfo) throws FileNotFoundException, IOException {
        File dst = dstFileInfo.mFile;
        File src = new File(filePath);
        RandomAccessFile randomAccessFile = new RandomAccessFile(src, "r");
        Movie movie = MovieCreator.build(randomAccessFile.getChannel());
        ///M: if this video can not be mute, show error@{
        if (movie == null) {
            Log.d(LOGTAG, "MovieCreator fail:" + filePath);
            return false;
        }
        /// @}

        // remove all tracks we will create new tracks from the old
        List<Track> tracks = movie.getTracks();
        movie.setTracks(new LinkedList<Track>());

        for (Track track : tracks) {
            if (track.getHandler().equals("vide")) {
                movie.addTrack(track);
            }
        }
        writeMovieIntoFile(dst, movie);
        randomAccessFile.close();
        return true;
    }

    private static void writeMovieIntoFile(File dst, Movie movie)
            throws IOException {
        if (!dst.exists()) {
            dst.createNewFile();
        }

        IsoFile out = new DefaultMp4Builder().build(movie);
        FileOutputStream fos = new FileOutputStream(dst);
        FileChannel fc = fos.getChannel();
        out.getBox(fc); // This one build up the memory.

        fc.close();
        fos.close();
    }

    /**
     * @param srcPath the path of source video file.
     * @param dstPath the path of destination video file.
     * @param startMs starting time in milliseconds for trimming. Set to
     *            negative if starting from beginning.
     * @param endMs end time for trimming in milliseconds. Set to negative if
     *            no trimming at the end.
     * @param useAudio true if keep the audio track from the source.
     * @param useVideo true if keep the video track from the source.
     * @throws IOException
     */
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
    private static boolean genVideoUsingMuxer(
            String srcPath, String dstPath, int startMs, int endMs,
            boolean useAudio, boolean useVideo,
            ProgressDialog mProgress) throws IOException {
        // Set up MediaExtractor to read from the source.
        MediaExtractor extractor = new MediaExtractor();
        // /M: log for setDataSource
        Log.d(LOGTAG, "setDataSource:" + srcPath);
        extractor.setDataSource(srcPath);

        int trackCount = extractor.getTrackCount();

        // Set up MediaMuxer for the destination.

        MediaMuxer muxer;
        muxer = new MediaMuxer(dstPath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);

        // Set up the tracks and retrieve the max buffer size for selected
        // tracks.
        HashMap<Integer, Integer> indexMap = new HashMap<Integer, Integer>(trackCount);
        int bufferSize = -1;
        int yuvBufferSize = -1;

        // /M:MediaMuxer only support 1 video track and 1 audio track, not
        // support 2 video track or 2 audio track
        int selectVideoTrackNum = 0;
        int selectAudioTrackNum = 0;
        int audioTrackIndex = -1;
        int maxInputSizeNum = 0;
        String mimes[] = new String[18];             // for check frame mime type recognize
        for (int i = 0; i < trackCount; i++) {
            MediaFormat format = extractor.getTrackFormat(i);
            String mime = format.getString(MediaFormat.KEY_MIME);

            // error handle for klocwork @{
            if (mime == null) {
                muxer.release(); // release would all stop
                mProgress.dismiss();
                Log.d(LOGTAG, "mime is null:" + i);
                return false;
            }
            if (i < 18) {
                mimes[i] = mime;
            }
            // @}
            // /M: Check trim capability
            // / Audio: MEDIA_MIMETYPE_AUDIO_AMR_NB,
            // MEDIA_MIMETYPE_AUDIO_AMR_WB, MEDIA_MIMETYPE_AUDIO_AAC
            // / Video: MEDIA_MIMETYPE_VIDEO_MPEG4, MEDIA_MIMETYPE_VIDEO_H263,
            // MEDIA_MIMETYPE_VIDEO_AVC @{
            Log.d(LOGTAG, "genVideoUsingMuxer mime:" + mime);
            boolean selectCurrentTrack = false;

            if (format.containsKey(MediaFormat.KEY_DURATION)) {
                long duration = format.getLong(MediaFormat.KEY_DURATION);
                if (duration / 1000 <= startMs) {
                    Log.d(LOGTAG, "durationMs: " + duration / 1000 + " < startMs:" + startMs);
                    continue;
                }
            }
            if ((mime.equals("audio/3gpp") || mime.equals("audio/amr-wb") || mime
                    .equals("audio/mp4a-latm")) && useAudio && selectAudioTrackNum == 0) {
                selectCurrentTrack = true;
                selectAudioTrackNum = 1;
                audioTrackIndex = i;
            } else if ((mime.equals("video/mp4v-es") || mime.equals("video/3gpp") ||
                    mime.equals("video/avc") ||
                    mime.equals("video/hevc")) && useVideo && selectVideoTrackNum == 0) {
                selectCurrentTrack = true;
                selectVideoTrackNum = 1;
            }
            // / @}

            if (selectCurrentTrack) {
                Log.d(LOGTAG, "Add Track mime:" + mime);
                extractor.selectTrack(i);

                int dstIndex = muxer.addTrack(format);
                indexMap.put(i, dstIndex);
                if (format.containsKey(MediaFormat.KEY_MAX_INPUT_SIZE)) {
                    int newSize = format.getInteger(MediaFormat.KEY_MAX_INPUT_SIZE);
                    // /M: log for max input size and KEY_MAX_INPUT_SIZE number
                    maxInputSizeNum++;
                    Log.d(LOGTAG, "KEY_MAX_INPUT_SIZE " + maxInputSizeNum + ":" + newSize);
                    bufferSize = newSize > bufferSize ? newSize : bufferSize;
                }
                if (format.containsKey(MediaFormat.KEY_WIDTH)
                    && format.containsKey(MediaFormat.KEY_HEIGHT)) {
                    int width = format.getInteger(MediaFormat.KEY_WIDTH);
                    int hight = format.getInteger(MediaFormat.KEY_HEIGHT);
                    Log.d(LOGTAG, "KEY_WIDTH : " +  width + " KEY_HEIGHT : " +  hight);
                    yuvBufferSize = (width * hight * 15) / 10;
                    // /M: log for buffersize when colorformat is YUV420, used when
                    // /M: KEY_MAX_INPUT_SIZE not set. Avoid JE when readsamplesize > buffersize.
                    Log.d(LOGTAG, "yuvBufferSize : " +  yuvBufferSize);
                }
            }
        }
        // /M: check timeUs back problem @{
        if (selectVideoTrackNum == 0 && selectAudioTrackNum == 0) {
            muxer.release(); // release would all stop
            mProgress.dismiss();
            Log.d(LOGTAG, "No Track support");
            return false;
        }
        // / @}

        // /M: if two track is selected, only one KEY_MAX_INPUT_SIZE would has
        // risk @{
        if (bufferSize < 0 || (maxInputSizeNum < selectVideoTrackNum + selectAudioTrackNum)) {
            bufferSize = DEFAULT_BUFFER_SIZE > yuvBufferSize ? DEFAULT_BUFFER_SIZE : yuvBufferSize;
            Log.d(LOGTAG, "use DEFAULT_BUFFER_SIZE or yuvbuffersize : " + bufferSize);
        }
        // / @}

        // Set up the orientation and starting time for extractor.
        MediaMetadataRetriever retrieverSrc = new MediaMetadataRetriever();
        retrieverSrc.setDataSource(srcPath);
        String degreesString = retrieverSrc
                .extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION);
        if (degreesString != null) {
            int degrees = Integer.parseInt(degreesString);
            if (degrees >= 0) {
                muxer.setOrientationHint(degrees);
            }
        }

        String fileMime = retrieverSrc
            .extractMetadata(MediaMetadataRetriever.METADATA_KEY_MIMETYPE);

        boolean shouldFixKeyFrame = false;
        if (fileMime.equals("video/mp2ts") || fileMime.equals("video/mp2p")) {
            Log.d(LOGTAG, "Fix key frame:" + fileMime);
            shouldFixKeyFrame = true;
        }
        if (startMs > 0) {
            // /M: startTime should correct to video sync frame, only for mpeg4
            // @{
            if (selectVideoTrackNum == 1 && selectAudioTrackNum == 1) {
                if (fileMime.equals("video/mp4") || fileMime.equals("video/3gpp")
                        || fileMime.equals("video/quicktime")) {
                    extractor.unselectTrack(audioTrackIndex);
                    startMs = correctSeekTime(extractor, startMs, bufferSize);
                    Log.d(LOGTAG, "correct new StartMs: " + startMs);
                    if (startMs == -10000000 || startMs > endMs) {
                        muxer.release(); // release would all stop
                        mProgress.dismiss();
                        Log.d(LOGTAG, "startMs can not be trimed");
                        return false;
                    }
                    extractor.selectTrack(audioTrackIndex);
                }
            }
            // /M: startMs overFlow
            extractor.seekTo((long) startMs * 1000, MediaExtractor.SEEK_TO_CLOSEST_SYNC);
            // / @}
        }

        // Copy the samples from MediaExtractor to MediaMuxer. We will loop
        // for copying each sample and stop when we get to the end of the source
        // file or exceed the end time of the trimming.
        int offset = 0;
        int trackIndex = -1;
        ByteBuffer dstBuf = ByteBuffer.allocate(bufferSize);
        BufferInfo bufferInfo = new BufferInfo();

        // /M: lastTimeUs is used to keep old timeUs
        long lastTimeUs = -1;
        try {
            muxer.start();
            while (true) {
                bufferInfo.offset = offset;
                bufferInfo.size = extractor.readSampleData(dstBuf, offset);
                if (bufferInfo.size < 0) {
                    Log.d(LOGTAG, "Saw input EOS.");
                    bufferInfo.size = 0;
                    break;
                } else {
                    bufferInfo.presentationTimeUs = extractor.getSampleTime();
                    // /M: check timeUs back problem @{
                    if (lastTimeUs != -1 && bufferInfo.presentationTimeUs < lastTimeUs) {
                        Log.d(LOGTAG, "timeUs back!");
                        muxer.release(); // release would call stop
                        mProgress.dismiss();
                        return false;
                    }
                    // / @}
                    // /M: endMs is int type, it would overflow and cause judge
                    // fail
                    if (endMs > 0 && bufferInfo.presentationTimeUs > ((long) endMs * 1000)) {
                        Log.d(LOGTAG, "The current sample is over the trim end time.");
                        Log.d(LOGTAG, "presentationTimeUs:" + bufferInfo.presentationTimeUs
                                + "endMs:" + endMs);
                        break;
                    } else {
                        bufferInfo.flags = extractor.getSampleFlags();
                        trackIndex = extractor.getSampleTrackIndex();

                        /// check key frame or not
                        if (shouldFixKeyFrame &&
                                checkKeyFrameIfPossible(dstBuf, mimes[trackIndex])) {
                            bufferInfo.flags |= 1;
                        }

                        muxer.writeSampleData(indexMap.get(trackIndex), dstBuf, bufferInfo);
                        extractor.advance();
                    }
                    // /M: set lastTimeUs
                    lastTimeUs = bufferInfo.presentationTimeUs;
                }
            }

            muxer.stop();
        } catch (IllegalStateException e) {
            mProgress.dismiss();
            Log.d(LOGTAG, "MediaMuxer.nativeStop failed");
            return false;
        } finally {
            try {
                muxer.release();
            } catch (IllegalStateException e) {
                Log.d(LOGTAG, "MediaMuxer release failed");
                return false;
            }
        }
        return true;
    }

    private static boolean trimUsingMp4Parser(File src, File dst, int startMs, int endMs,
            TrimVideo trimVideo) throws FileNotFoundException, IOException {

        if (src.exists() && dst.exists()) {
            Log.v(LOGTAG  , "startTrim() src is " + src.getAbsolutePath() +
                " and dst is " + dst.getAbsolutePath());
        }
        Log.v(LOGTAG  , "startTrim() startMs is " + startMs + " endMs is " + endMs);

        RandomAccessFile randomAccessFile = new RandomAccessFile(src, "r");
        Movie movie = MovieCreator.build(randomAccessFile.getChannel());
        ///M: if this video can be trimmed, show progress dialog @{
        if (movie == null) {
            return false;
        }
        /// @}

        // remove all tracks we will create new tracks from the old
        List<Track> tracks = movie.getTracks();
        movie.setTracks(new LinkedList<Track>());

        double startTime = startMs / 1000;
        double endTime = endMs / 1000;

        boolean timeCorrected = false;

        // Here we try to find a track that has sync samples. Since we can only
        // start decoding at such a sample we SHOULD make sure that the start of
        // the new fragment is exactly such a frame.
        for (Track track : tracks) {
            if (track.getSyncSamples() != null && track.getSyncSamples().length > 0) {
                if (timeCorrected) {
                    return false;
                }
                ///M:enhance correctTimeToSyncSample, correct only once{
                double[] newCut = new double[2];
                correctTimeToSyncSample2(track, startTime, endTime, newCut);
                startTime = newCut[0];
                endTime = newCut[1];
                ///@}
                timeCorrected = true;
            }
        }
        ///M:when the video cann't trim, show a toast to user.@{
        Log.v(LOGTAG  , "startTrim() startTime " + startTime + " endTime " + endTime);
        if (startTime == endTime) {
           return false;
        }
        ///@}
        for (Track track : tracks) {
            long currentSample = 0;
            double currentTime = 0;
            long startSample = -1;
            long endSample = -1;

            for (int i = 0; i < track.getDecodingTimeEntries().size(); i++) {
                TimeToSampleBox.Entry entry = track.getDecodingTimeEntries().get(i);
                for (int j = 0; j < entry.getCount(); j++) {
                    // entry.getDelta() is the amount of time the current sample
                    // covers.

                    if (currentTime <= startTime) {
                        // current sample is still before the new starttime
                        startSample = currentSample;
                    }
                    if (currentTime <= endTime) {
                        // current sample is after the new start time and still
                        // before the new endtime
                        endSample = currentSample;
                    } else {
                        // current sample is after the end of the cropped video
                        break;
                    }
                    currentTime += (double) entry.getDelta()
                            / (double) track.getTrackMetaData().getTimescale();
                    currentSample++;
                }
            }
            movie.addTrack(new CroppedTrack(track, startSample, endSample));
        }
        writeMovieIntoFile(dst, movie);
        randomAccessFile.close();
        return true;
    }

    private static double correctTimeToSyncSample(Track track, double cutHere,
            boolean next) {
        double[] timeOfSyncSamples = new double[track.getSyncSamples().length];
        long currentSample = 0;
        double currentTime = 0;
        for (int i = 0; i < track.getDecodingTimeEntries().size(); i++) {
            TimeToSampleBox.Entry entry = track.getDecodingTimeEntries().get(i);
            for (int j = 0; j < entry.getCount(); j++) {
                if (Arrays.binarySearch(track.getSyncSamples(), currentSample + 1) >= 0) {
                    // samples always start with 1 but we start with zero
                    // therefore +1
                    timeOfSyncSamples[Arrays.binarySearch(
                            track.getSyncSamples(), currentSample + 1)] = currentTime;
                }
                currentTime += (double) entry.getDelta()
                        / (double) track.getTrackMetaData().getTimescale();
                currentSample++;
            }
        }
        double previous = 0;
        for (double timeOfSyncSample : timeOfSyncSamples) {
            if (timeOfSyncSample > cutHere) {
                if (next) {
                    return timeOfSyncSample;
                } else {
                    return previous;
                }
            }
            previous = timeOfSyncSample;
        }
        return timeOfSyncSamples[timeOfSyncSamples.length - 1];
    }

    ///M:enhance correctTimeToSyncSample, correct only once{
    private static void correctTimeToSyncSample2(Track track, double cutStart,
            double cutEnd, double[] newCut) {
        double[] timeOfSyncSamples = new double[track.getSyncSamples().length];
        long currentSample = 0;
        double currentTime = 0;
        Log.v(LOGTAG  , "correctTimeToSyncSample()" + "SyncSample length:" +
                track.getSyncSamples().length + "DecodingTimeEntries: " +
                track.getDecodingTimeEntries().size());
        for (int i = 0; i < track.getDecodingTimeEntries().size(); i++) {
            TimeToSampleBox.Entry entry = track.getDecodingTimeEntries().get(i);
            for (int j = 0; j < entry.getCount(); j++) {
                int indexToSyncSample = Arrays.binarySearch(
                            track.getSyncSamples(), currentSample + 1);
                if (indexToSyncSample >= 0) {
                    // samples always start with 1 but we start with zero therefore +1
                    timeOfSyncSamples[indexToSyncSample] = currentTime;
                }
                currentTime += (double) entry.getDelta()
                    / (double) track.getTrackMetaData().getTimescale();
                currentSample++;
            }
        }

        double previous = 0;
        double newCutEnd = -1;
        double newCutStart = -1;
        for (double timeOfSyncSample : timeOfSyncSamples) {
            if (timeOfSyncSample > cutStart && newCutStart == -1) {
                newCutStart = previous;
                Log.v(LOGTAG  , "newCutStart " + newCutStart);
            }
            if (timeOfSyncSample > cutEnd && newCutEnd == -1) {
                newCutEnd = timeOfSyncSample;
                Log.v(LOGTAG  , "newCutEnd " + newCutEnd);
                break;
            }
            previous = timeOfSyncSample;
        }
        if (newCutStart == -1) {
            newCutStart = timeOfSyncSamples[timeOfSyncSamples.length - 1];
        }
        if (newCutEnd == -1) {
            newCutEnd = timeOfSyncSamples[timeOfSyncSamples.length - 1];
        }

        newCut[0] = newCutStart;
        newCut[1] = newCutEnd;
    }
    /// @}
    ///M: get first sample timeUs when video seek, startMs should use the time,
    ///or else, it would cause AV not syc{
    private static int correctSeekTime(MediaExtractor extractor, int startMs, int bufSize) {
        int offset = 0;

        extractor.seekTo((long) startMs * 1000, MediaExtractor.SEEK_TO_CLOSEST_SYNC);

        ByteBuffer dstBuf = ByteBuffer.allocate(bufSize);
        BufferInfo bufferInfo = new BufferInfo();
        bufferInfo.offset = offset;
        bufferInfo.size = extractor.readSampleData(dstBuf, offset);
        if (bufferInfo.size < 0) {
            Log.d(LOGTAG, "correctSeekTime again Saw input EOS.");
            return -10000000;
        }
        bufferInfo.presentationTimeUs = extractor.getSampleTime();
        return (int) (bufferInfo.presentationTimeUs / 1000);
    }
    /// @}

    ///M: CheckKeyFrameIfPossible{
    private static boolean checkKeyFrameIfPossible(ByteBuffer buf, String mime) {
        if (mime.equals("video/avc")) {
            int lastPos = buf.position();
            if (lastPos + 4 < buf.limit()) {
                buf.position(lastPos + 4);
                int type = buf.get();
                buf.position(lastPos);
                int nalUnitType = type & 0x1f;
                if (nalUnitType == 5 || nalUnitType == 7) {
                    Log.d(LOGTAG, "key Frame:" + nalUnitType);
                    return true;
                } else if (nalUnitType == 9) {
                    buf.position(lastPos + 10);
                    type = buf.get();
                    buf.position(lastPos);
                    nalUnitType = type & 0x1f;
                    if (nalUnitType == 5 || nalUnitType == 7) {
                        Log.d(LOGTAG, "key Frame:" + nalUnitType);
                        return true;
                    }
                }
            }
        } else if (mime.equals("video/mp4v-es")) {
            int lastPos = buf.position();
            if (lastPos + 4 < buf.limit()) {
                buf.position(lastPos + 3);
                int type = buf.get();
                buf.position(lastPos);
                if ((type & 0xff)  == 0xB3) {
                    Log.d(LOGTAG, "key Frame:" + type);
                    return true;
                }
            }
        }
        return false;
    }
    /// @}

}
