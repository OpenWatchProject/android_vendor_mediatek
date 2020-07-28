package com.mediatek.gallerybasic.dynamic;

import android.os.SystemClock;

import com.mediatek.gallerybasic.base.Player;
import com.mediatek.gallerybasic.util.DebugUtils;
import com.mediatek.gallerybasic.util.Log;

import java.util.ArrayList;
import java.util.Iterator;

class PlayThreads {
    private static final String TAG = "MtkGallery2/PlayThreads";

    private int mThreadNum;
    private boolean mThreadsStart = false;
    private ArrayList<WorkThread> mThreads;
    // Add "synchronized" when operate mWaitingCommands & mRunningCommands
    private ArrayList<Command> mWaitingCommands = new ArrayList<Command>();
    private ArrayList<Command> mRunningCommands = new ArrayList<Command>();

    public PlayThreads(int threadNum) {
        mThreadNum = threadNum;
    }

    public synchronized void start() {
        if (mThreadsStart) {
            return;
        }
        mThreads = new ArrayList<WorkThread>();
        for (int i = 0; i < mThreadNum; i++) {
            WorkThread t = new WorkThread(i);
            t.setName("WorkThread-" + i);
            mThreads.add(i, t);
            t.start();
        }
        mThreadsStart = true;
        Log.d(TAG, "<start>");
    }

    public synchronized void stop() {
        if (!mThreadsStart) {
            return;
        }
        for (int i = 0; i < mThreadNum; i++) {
            mThreads.get(i).interrupt();
            mThreads.set(i, null);
        }
        mThreads = null;
        mThreadsStart = false;
        Log.d(TAG, "<stop>");
    }

    // submit command sort by priority,
    // the earlier when submit, the higher the priority is.
    public synchronized void submit(PlayList.Entry entry,
            Player.State targetState) {
        // When there is command whose entry and targetState are same as input
        // parameters, not submit this command
        Iterator<Command> cmdItr = mWaitingCommands.iterator();
        while (cmdItr.hasNext()) {
            Command cmd = cmdItr.next();
            if (cmd.entry == entry && cmd.targetState == targetState) {
                Log.d(TAG,
                        "<submit> has same cmd in waiting cmds, not submit");
                return;
            }
        }
        cmdItr = mRunningCommands.iterator();
        while (cmdItr.hasNext()) {
            Command cmd = cmdItr.next();
            if (cmd.entry == entry && cmd.targetState == targetState) {
                Log.d(TAG,
                        "<submit> has same cmd in running cmds, not submit");
                return;
            }
        }

        Command newCmd = new Command(entry, targetState);
        mWaitingCommands.add(newCmd);
        Log.d(TAG, "<submit> Add new command = " + newCmd);

        // When targetState == RELEASED, check if there is !RELEASE command of
        // this entry is in running list, if yes, cancel it
        if (targetState == Player.State.RELEASED) {
            cmdItr = mRunningCommands.iterator();
            while (cmdItr.hasNext()) {
                Command cmd = cmdItr.next();
                if (cmd.entry == entry) {
                    if (cmd.targetState != Player.State.RELEASED) {
                        cmd.cancel();
                        Log.d(TAG, "<submit> Cancel command = " + cmd);
                    }
                    break;
                }
            }
        }
        if (mThreads == null) {
            start();
        }
        // if entry has specified work thread, notify this thread
        if (entry.threadIndex != PlayList.INVALIDE) {
            WorkThread thread = mThreads.get(entry.threadIndex);
            synchronized (thread) {
                thread.notifyAll();
            }
        } else {
            // else notify all threads
            Iterator<WorkThread> threadItr = mThreads.iterator();
            while (threadItr.hasNext()) {
                WorkThread thread = threadItr.next();
                synchronized (thread) {
                    thread.notifyAll();
                }
            }
        }
    }

    public synchronized void clearAllCmds() {
        // clear all commands in waiting list except release
        Iterator<Command> itr = mWaitingCommands.iterator();
        while (itr.hasNext()) {
            Command cmd = itr.next();
            if (cmd.targetState != Player.State.RELEASED) {
                itr.remove();
            }
        }
    }

    private synchronized Command getCmdsForThread(int threadIndex) {
        Iterator<Command> itr = mWaitingCommands.iterator();
        while (itr.hasNext()) {
            Command cmd = itr.next();
            if (cmd.entry.threadIndex == PlayList.INVALIDE) {
                itr.remove();
                cmd.entry.threadIndex = threadIndex;
                mRunningCommands.add(cmd);
                return cmd;
            } else if (cmd.entry.threadIndex == threadIndex) {
                itr.remove();
                mRunningCommands.add(cmd);
                return cmd;
            }
        }
        Log.d(TAG, "<getCmdsForThread> No cmd for thread " + threadIndex);
        return null;
    }

    class Command implements Player.TaskCanceller {
        public final PlayList.Entry entry;
        public final Player.State targetState;
        private boolean mIsCancelled = false;

        public Command(PlayList.Entry e, Player.State s) {
            entry = e;
            targetState = s;
        }

        public void cancel() {
            mIsCancelled = true;
            if (entry != null && entry.player != null) {
                entry.player.onCancel();
            }
        }

        public boolean isCancelled() {
            return mIsCancelled;
        }

        public String toString() {
            String res = "";
            res += "[filePath = ";
            res += entry.data.filePath;
            res += ", targetState = ";
            res += targetState;
            res += ", mIsCancelled = ";
            res += mIsCancelled;
            res += "]";
            return res;
        }
    }

    class WorkThread extends Thread {
        private int mIndex;
        private boolean mActive = true;

        public WorkThread(int index) {
            super();
            mIndex = index;
        }

        public synchronized void interrupt() {
            mActive = false;
            super.interrupt();
        }

        @Override
        public void run() {
            Log.d(TAG, "<WorkThread.run> begin, mIndex = " + mIndex);
            while (true) {
                Command cmd = null;
                cmd = getCmdsForThread(mIndex);
                synchronized (WorkThread.this) {
                    // If active is false and no command wait to run, Exit
                    if (!mActive && cmd == null) {
                        Log.d(TAG, "<WorkThread.run> exit, mIndex = " + mIndex);
                        break;
                    }
                    if (cmd == null && mActive) {
                        Log.d(TAG, "<WorkThread.run> wait, mIndex = "
                                + mIndex);
                        try {
                            WorkThread.this.wait();
                        } catch (InterruptedException e) {
                            Log.d(TAG,
                                    "<WorkThread.run> InterruptedException, mIndex = "
                                            + mIndex);
                        }
                    }
                }
                if (cmd == null) {
                    continue;
                }
                runCmd(cmd);
                synchronized (PlayThreads.this) {
                    mRunningCommands.remove(cmd);
                }
            }
        }

        private void runCmd(Command cmd) {
            if (cmd.isCancelled()) {
                Log.d(TAG, "<runCmd> cancelled, return");
                return;
            }
            long begin = SystemClock.uptimeMillis();
            cmd.entry.player.setTaskCanceller(cmd);
            switch (cmd.targetState) {
            case PREPARED:
                gotoStatePrepared(cmd);
                break;
            case PLAYING:
                gotoStatePlaying(cmd);
                break;
            case RELEASED:
                gotoStateReleased(cmd);
                break;
            default:
                Log.d(TAG, "<runCmd> error targetState = " + cmd.targetState);
                break;
            }
            cmd.entry.player.setTaskCanceller(null);
            long end = SystemClock.uptimeMillis();
            Log.d(TAG, "<runCmd> cost " + (end - begin) + " ms");
        }

        private void gotoStatePrepared(Command cmd) {
            Log.d(TAG, "<gotoStatePrepared> begin, filePath = "
                    + cmd.entry.data.filePath + ", player = "
                    + cmd.entry.player);
            boolean success;
            switch (cmd.entry.player.getState()) {
            case RELEASED:
                cmd.entry.player.prepare();
                break;
            case PREPARED:
                break;
            case PLAYING:
                success = cmd.entry.player.pause();
                if (!cmd.isCancelled() && success) {
                    cmd.entry.player.stop();
                }
                break;
            default:
                break;
            }
            Log.d(TAG, "<gotoStatePrepared> end, filePath = "
                    + cmd.entry.data.filePath + ", player = "
                    + cmd.entry.player + ", state = "
                    + cmd.entry.player.getState() + ", cancelled = "
                    + cmd.isCancelled());
        }

        private void gotoStatePlaying(Command cmd) {
            Log.d(TAG, "<gotoStatePlaying> begin, filePath = "
                    + cmd.entry.data.filePath + ", player = "
                    + cmd.entry.player);
            boolean success;
            switch (cmd.entry.player.getState()) {
            case RELEASED:
                success = cmd.entry.player.prepare();
                if (!cmd.isCancelled() && success) {
                    cmd.entry.player.start();
                }
                break;
            case PREPARED:
                cmd.entry.player.start();
                break;
            case PLAYING:
                break;
            default:
                break;
            }
            Log.d(TAG, "<gotoStatePlaying> end, filePath = "
                    + cmd.entry.data.filePath + ", player = "
                    + cmd.entry.player + ", state = "
                    + cmd.entry.player.getState() + ", cancelled = "
                    + cmd.isCancelled());
        }

        private void gotoStateReleased(Command cmd) {
            Log.d(TAG, "<gotoStateReleased> begin, filePath = "
                    + cmd.entry.data.filePath + ", player = "
                    + cmd.entry.player);
            boolean success;
            switch (cmd.entry.player.getState()) {
            case RELEASED:
                break;
            case PREPARED:
                cmd.entry.player.release();
                break;
            case PLAYING:
                success = cmd.entry.player.pause();
                if (!cmd.isCancelled() && success) {
                    cmd.entry.player.stop();
                }
                if (!cmd.isCancelled()) {
                    cmd.entry.player.release();
                }
                break;
            default:
                break;
            }
            Log.d(TAG, "<gotoStateReleased> end, filePath = "
                    + cmd.entry.data.filePath + ", player = "
                    + cmd.entry.player + ", state = "
                    + cmd.entry.player.getState() + ", cancelled = "
                    + cmd.isCancelled());
        }
    }

    public synchronized void logCmdsWaitToRun(String str) {
        if (!DebugUtils.DEBUG_PLAY_ENGINE) {
            return;
        }
        Iterator<Command> itr = mWaitingCommands.iterator();
        int i = 0;
        Log.d(TAG, str + " begin ----------------------------------------");
        while (itr.hasNext()) {
            Command cmd = itr.next();
            Log.d(TAG, str + " [" + i + "] " + cmd);
            i++;
        }
        Log.d(TAG, str + " end   ----------------------------------------");
    }
}