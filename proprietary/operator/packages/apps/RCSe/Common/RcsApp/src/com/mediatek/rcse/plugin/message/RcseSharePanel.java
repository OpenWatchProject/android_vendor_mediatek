package com.mediatek.rcse.plugin.message;

import android.content.Context;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IpMessageConsts.*;
import com.mediatek.rcs.R;

public class RcseSharePanel {
    private static String TAG = "RcseSharePanel";
    private Context mContext = null;
    /// M: IP message
    public static final int IPMSG_TAKE_PHOTO        = 100;
    public static final int IPMSG_RECORD_VIDEO      = 101;
    public static final int IPMSG_RECORD_AUDIO      = 102;
    public static final int IPMSG_CHOOSE_PHOTO      = 104;
    public static final int IPMSG_CHOOSE_VIDEO      = 105;
    public static final int IPMSG_CHOOSE_AUDIO      = 106;
    public static final int IPMSG_SHARE_CONTACT     = 108;
    public static final int IPMSG_SHARE_CALENDAR    = 109;
    public static final int IPMSG_SHARE_SLIDESHOW   = 110;
    public static final int IPMSG_SHARE_FILE        = 111;
    
    private static final int[] IP_MESSAGE_ACTIONS = {
        IPMSG_TAKE_PHOTO, IPMSG_RECORD_VIDEO, IPMSG_RECORD_AUDIO, IPMSG_SHARE_CONTACT,
        IPMSG_CHOOSE_PHOTO, IPMSG_CHOOSE_VIDEO, IPMSG_CHOOSE_AUDIO, IPMSG_SHARE_CALENDAR,
        IPMSG_SHARE_FILE};
    
    private static final int[] XMS_MESSAGE_ACTIONS = {
        IPMSG_TAKE_PHOTO, IPMSG_RECORD_VIDEO, IPMSG_RECORD_AUDIO, IPMSG_SHARE_CONTACT,
        IPMSG_CHOOSE_PHOTO, IPMSG_CHOOSE_VIDEO, IPMSG_CHOOSE_AUDIO, IPMSG_SHARE_CALENDAR,
        IPMSG_SHARE_SLIDESHOW};
    
	private static final int[] joynShareIconArr = { drawable.ipmsg_take_photo,
			drawable.ipmsg_record_video, drawable.ipmsg_record_audio,
			drawable.ipmsg_share_contact, drawable.ipmsg_choose_photo,
			drawable.ipmsg_choose_video, drawable.ipmsg_choose_audio,
			drawable.ipmsg_share_calendar, 
			drawable.ipmsg_choose_a_file };
    
	private static final int[] xmsShareIconArr = { drawable.ipmsg_take_photo,
        drawable.ipmsg_record_video, drawable.ipmsg_record_audio,
        drawable.ipmsg_share_contact, drawable.ipmsg_choose_photo,
        drawable.ipmsg_choose_video, drawable.ipmsg_choose_audio,
        drawable.ipmsg_share_calendar, 
        drawable.ipmsg_choose_a_file };
    
    private String[] mLableArray;

    public static final String SHARE_ACTION = "shareAction";

    public boolean onIpGridViewItemClick(int actionPosition, Bundle bundle) {
        Logger.d(TAG, "onIpGridViewItemClick actionPosition = " + actionPosition + "mCurrentChatMode" + RcseComposeActivity.mCurrentChatMode);
        if(RcseComposeActivity.mCurrentChatMode == IpMessageConsts.ChatMode.XMS)
        {
            Logger.d(TAG, "onIpGridViewItemClick XMS Mode ");
            return false;
        }
        if (actionPosition >= IP_MESSAGE_ACTIONS.length) {
            return false;
        }
        bundle.putInt(SHARE_ACTION, IP_MESSAGE_ACTIONS[actionPosition]);
        return true;
    }

    public String[] getIpLableArray(Context context) {
        Logger.d(TAG, "getLableArray context = " + context + "mCurrentChatMode:" + RcseComposeActivity.mCurrentChatMode);
        if(RcseComposeActivity.mCurrentChatMode == IpMessageConsts.ChatMode.XMS)
        {
            Logger.d(TAG, "getIpLableArray XMS Mode ");
            return null;
        }
        String[] source = IpMessageResourceMananger.getInstance(context).getStringArray(
                IpMessageConsts.array.ipmsg_share_string_array);
        mLableArray = source;
        return source;
    }

    public int[] getIpIconArray(Context context) {
        Logger.d(TAG, "getIpIconArray context = " + context + "mCurrentChatMode:" + RcseComposeActivity.mCurrentChatMode);
        mContext = context;
        if(RcseComposeActivity.mCurrentChatMode == IpMessageConsts.ChatMode.XMS)
        {
            Logger.d(TAG, "getIpIconArray XMS Mode ");
            return null;
        }
        int[]source = joynShareIconArr;
        return source;
    }

    public boolean getIpView(int position, TextView text, ImageView img) {
        Logger.d(TAG, "getIpView mLableArray is " + mLableArray +"mcurrentcvhatmode:" + RcseComposeActivity.mCurrentChatMode);
        if(RcseComposeActivity.mCurrentChatMode == IpMessageConsts.ChatMode.XMS)
        {
            mLableArray = IpMessageResourceMananger.getInstance(mContext).getStringArray(
                    IpMessageConsts.array.xms_share_string_array);
            Logger.d(TAG, "getIpView XMS Mode mLableArray" + mLableArray);
            if (position < mLableArray.length) {
                text.setText(mLableArray[position]);
                img.setImageDrawable(IpMessageResourceMananger.getInstance(mContext).getSingleDrawable(xmsShareIconArr[position]));
                return true;
            }
            return false;
        }
        if(RcseComposeActivity.mCurrentChatMode == IpMessageConsts.ChatMode.JOYN )
        {
            String[] source = IpMessageResourceMananger.getInstance(mContext).getStringArray(
                    IpMessageConsts.array.ipmsg_share_string_array);
            mLableArray = source;
            Logger.d(TAG, "getIpView Joyn Mode mLableArray" + mLableArray);
        if (position < mLableArray.length) {
            text.setText(mLableArray[position]);
            img.setImageDrawable(IpMessageResourceMananger.getInstance(mContext).getSingleDrawable(joynShareIconArr[position]));
            return true;
            } 
        }
        
            return false;
        }
       


}
