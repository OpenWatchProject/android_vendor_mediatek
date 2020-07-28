package com.mediatek.rcse.plugin.message;

import java.util.ArrayList;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.mms.ipmessage.DefaultIpContactExt;
import com.mediatek.mms.callback.IContactCallback;
import com.mediatek.rcse.plugin.message.IpMessageConsts;

public class RcseContact extends DefaultIpContactExt {
    private static String TAG = "RcseContact";
    /// M: add for ip message
    private long mThreadId = 0;
    private BitmapDrawable mIpMessageAvatar;
    private boolean mIpMessageAvatarFetched = false;
    public static final int IPMSG_AVATAR_FETCH_TIME_OUT = 200;
    private Bitmap mIpMessageAvatarBitmap;

    private BitmapDrawable mAvatar;

    private Context mContext;
    public IContactCallback mCallback;
    
    @Override
    public void onIpInit(Context context, IContactCallback callback) {
        mContext = context;
        mCallback = callback;
    }

    public String onIpUpdateContact(String number, String name) {
        if (mThreadId > 0) {
            Log.d("avatar", "Contact.updateContact(): mThreadId > 0, name = " + name);
            if (TextUtils.isEmpty(name) || number.equals(name)) {
                name = IpMessageContactManager.getInstance(mContext).getNameByThreadId(mThreadId, number);
                Log.d("avatar", "Contact.updateContact(): group name = " + name);
            }
        }
        return name;
    }

    @Override
    public Drawable onIpGetAvatar(Drawable defaultValue, final long threadId, String number) {
        try {
        /// M: use ip message avatar if exist
//        if (IpMessageUtils.getIpMessagePlugin(context).isActualPlugin()) {
            if (!mIpMessageAvatarFetched) {
                if (threadId > 0 && IpMessageContactManager.getInstance(mContext).isIpMessageNumber(number)) {
                    final Object lock = new Object();
                    mIpMessageAvatarBitmap = null;
                    new Thread(new Runnable() {
                        public void run() {
                            mIpMessageAvatarBitmap = IpMessageContactManager.getInstance(mContext).getAvatarByThreadId(threadId);
                            synchronized (lock) {
                                lock.notifyAll();
                            }
                        }
                    }).start();
                    synchronized (lock) {
                        try {
                            lock.wait(IPMSG_AVATAR_FETCH_TIME_OUT);
                        } catch (InterruptedException ex) {
                            // do nothing
                        }
                    }

                    if (null != mIpMessageAvatarBitmap) {
                        mIpMessageAvatar = new BitmapDrawable(mIpMessageAvatarBitmap);
                    }
                }
                mIpMessageAvatarFetched = true;
            }

            if (mIpMessageAvatar != null) {
                return mIpMessageAvatar;
            } else {
                return defaultValue;
            }
//        }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return defaultValue;
        }
    }

    @Override
    public String onIpGetNumber(String number) {
        // add for joyn converged inbox mode
        if (number.startsWith(IpMessageConsts.JOYN_START)) {
            number = number.substring(4);
        }
        return number;
    }

    @Override
    public boolean onIpIsGroup(String number) {
        if (IpMmsConfig.isServiceEnabled(mContext)
                && number != null && number.startsWith(IpMessageConsts.GROUP_START)) {
            return true;
        }
        return false;
    }

    public void setThreadId(long threadId) {
        mThreadId = threadId;
    }

    /// M: add for ipmessage,fix bug ALPS01608034 @{
    public synchronized Drawable getGroupAvatar(final Handler mUiHandler) {
       // mAvatar = mCallback.getAvatar();
        Log.d(TAG, "Contact.getGroupAvatar(): avatar is null ?= "
                + (mAvatar == null));
        if (mAvatar == null) {
            final Object lock = new Object();
            mIpMessageAvatarBitmap = null;
            //new Thread(new Runnable() {
               // public void run() {
                    mIpMessageAvatarBitmap = IpMessageContactManager.getInstance(mContext).getAvatarByThreadId(mThreadId);
                    if (null != mIpMessageAvatarBitmap) {
                        mAvatar = new BitmapDrawable(mIpMessageAvatarBitmap);
                    }
                    if (null != mUiHandler) {
                        mUiHandler.sendEmptyMessage((int) mThreadId);
                    }
              //  }
           // }).start();

            Log.d(TAG,
                    "Contact.getGroupAvatar(): mIpMessageAvatarBitmap is null ?= "
                            + (mIpMessageAvatarBitmap == null) + "avatar =" + mAvatar);
        }
        mCallback.setAvatar(mAvatar);
        return mAvatar;
    }

    public Drawable getGroupAvatar() {
        return mCallback.getAvatar();
    }
    /// @}

    public synchronized void clearAvatar() {
        mCallback.setAvatar(null);
    }

    @Override
    public void invalidateGroupContact(String number) {
      mIpMessageAvatarFetched = false;
      if (number != null && number.startsWith(IpMessageConsts.GROUP_START)) {
          clearAvatar();
      }
      return;
    }

}
