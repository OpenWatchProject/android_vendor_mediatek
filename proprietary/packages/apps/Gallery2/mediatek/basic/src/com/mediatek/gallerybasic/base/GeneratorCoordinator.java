package com.mediatek.gallerybasic.base;

import com.mediatek.gallerybasic.util.Log;

import java.io.File;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public final class GeneratorCoordinator {
    private static final String TAG = "MtkGallery2/GeneratorCoordinator";

    private GeneratorCoordinator() {
        // do nothing
    }

    public interface OnGeneratedListener {
        void onGeneratedListen();
    }

    private static class MediaGenerator {
        public final MediaData media;
        public final Generator generator;

        public MediaGenerator(final Generator generator, final MediaData media) {
            this.media = media;
            this.generator = generator;
        }
    }

    private static class Secretary extends Thread {
        // TODO: use blocking stack is more friendly, to be modified
        private final BlockingQueue<MediaGenerator> mMediaGeneratorQueue;
        private MediaGenerator mCurrentMediaGenerator;

        public Secretary(String threadName) {
            super("GeneratorCoordinator - Secretary" + threadName);
            mMediaGeneratorQueue = new LinkedBlockingQueue<MediaGenerator>();
        }

        public void run() {
            try {
                MediaGenerator currentMediaGenerator;
                while (!Thread.currentThread().isInterrupted()) {
                    currentMediaGenerator = mMediaGeneratorQueue.take();
                    synchronized (Secretary.this) {
                        mCurrentMediaGenerator = currentMediaGenerator;
                    }

                    if (!currentMediaGenerator.generator.needGenerating(
                            currentMediaGenerator.media, Generator.VTYPE_THUMB)) {
                        continue;
                    }

                    Log.v(TAG, "begin handling transcoding request for "
                            + currentMediaGenerator.media.filePath);
                    int genRes = currentMediaGenerator.generator
                            .generateAndWait(currentMediaGenerator.media,
                                    Generator.VTYPE_THUMB);
                    Log.v(TAG, "end handling transcoding request for "
                            + currentMediaGenerator.media.filePath + " with result " + genRes);
                    if (genRes == Generator.GENERATE_OK) {
                        if (sOnGeneratedListener != null) {
                            sOnGeneratedListener.onGeneratedListen();
                        }
                    }
                }
            } catch (InterruptedException e) {
                Log.e(TAG, "Terminating " + getName());
                this.interrupt();
            }
        }

        private void submit(MediaGenerator mediaItem) {
            if (isAlive()) {
                Log.v(TAG, "submit transcoding request for " + mediaItem.media.filePath);
                mMediaGeneratorQueue.add(mediaItem);
            } else {
                Log.e(TAG, getName() + " should be started before submitting tasks.");
            }
        }

        private void cancelCurrentTranscode() {
            MediaGenerator currentMediaGenerator;
            synchronized (Secretary.this) {
                currentMediaGenerator = mCurrentMediaGenerator;
            }
            if (currentMediaGenerator != null) {
                currentMediaGenerator.generator.onCancelRequested(
                        currentMediaGenerator.media, Generator.VTYPE_THUMB);
            }
        }

        private void cancelTranscodingForLostFile() {
            MediaGenerator currentMediaGenerator;
            synchronized (Secretary.this) {
                currentMediaGenerator = mCurrentMediaGenerator;
            }
            if (currentMediaGenerator != null) {
                File f = new File(currentMediaGenerator.media.filePath);
                if (!f.exists()) {
                    Log.v(TAG, "cancelTranscodingForLostFile "
                            + currentMediaGenerator.media.filePath);
                    currentMediaGenerator.generator.onCancelRequested(
                            currentMediaGenerator.media, Generator.VTYPE_THUMB);
                }
            }
        }

        private void cancelPendingTranscode() {
            mMediaGeneratorQueue.clear();
        }

        private void cancelAllTranscode() {
            cancelCurrentTranscode();
            cancelPendingTranscode();
            // sSecretary.interrupt();
        }
    }

    private static volatile Secretary sSecretary = null;
    private static volatile OnGeneratedListener sOnGeneratedListener = null;

    public static void setOnGeneratedListener(final OnGeneratedListener listener) {
        sOnGeneratedListener = listener;
    }

    public static void requestThumbnail(Generator video, MediaData media) {
        Secretary secretary = sSecretary;
        if (secretary != null) {
            secretary.submit(new MediaGenerator(video, media));
        }
    }

    public static void pause() {
        if (sSecretary != null) {
            sSecretary.cancelAllTranscode();
            // sSecretary = null;
        }
    }

    public static void cancelTranscodingForLostFile() {
        if (sSecretary != null) {
            sSecretary.cancelTranscodingForLostFile();
        }
    }

    public static void cancelPendingTranscode() {
        if (sSecretary != null) {
            sSecretary.cancelPendingTranscode();
        }
    }

    public static void start() {
        if (sSecretary == null) {
            sSecretary = new Secretary("transcoding proxy");
            sSecretary.start();
        }
    }
}
