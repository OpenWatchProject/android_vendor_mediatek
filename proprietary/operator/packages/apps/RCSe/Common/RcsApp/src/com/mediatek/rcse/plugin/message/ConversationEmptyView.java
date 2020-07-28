package com.mediatek.rcse.plugin.message;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.rcse.plugin.message.IpMessageConsts;
import com.mediatek.rcse.plugin.message.IpMessageConsts.RemoteActivities;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcs.R;

public class ConversationEmptyView extends LinearLayout {

    private LinearLayout mBackground;
    private Context mContext;
    private View mConvertView;
    private TextView mContent;
    private LinearLayout mActivate;
    private RelativeLayout mSpam;
    private RelativeLayout mGroupChat;
    private Button mBtnActivate;

    public ConversationEmptyView(final Context context) {
        super(context);
        mContext = context;
        LayoutInflater inflater = LayoutInflater.from(MediatekFactory.getApplicationContext());        
        mConvertView = inflater.inflate(R.layout.conversation_empty, null);  
        mBackground = (LinearLayout) mConvertView.findViewById(R.id.background);
        mContent = (TextView) mConvertView.findViewById(R.id.tv_empty_content);
        mSpam = (RelativeLayout) mConvertView.findViewById(R.id.ll_empty_spam);
        mGroupChat = (RelativeLayout) mConvertView.findViewById(R.id.ll_empty_groupchat);
        mActivate = (LinearLayout) mConvertView.findViewById(R.id.ll_empty_activate);
        mBtnActivate = (Button) mConvertView.findViewById(R.id.btn_activate);
        Log.d("ConversationEmptyView", "mConvertView" + mConvertView);
        mBtnActivate.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                Intent intent = new Intent(RemoteActivities.ACTIVITION);
                // do to sim_id update.
                intent.putExtra(RemoteActivities.KEY_SIM_ID, 0); // need put int type SIM id
                IpMessageUtils.startRemoteActivity(context, intent);
            } });
    }

    public ConversationEmptyView(final Context context, AttributeSet attrs) {
        super(context, attrs);
       
    }

    public void setSpamEmpty(boolean isActivate) {
        mSpam.setVisibility(View.VISIBLE);
        mGroupChat.setVisibility(View.GONE);
        mBackground.setBackgroundResource(R.color.empty_background);
        mContent.setText(IpMessageResourceMananger.getInstance(mContext)
            .getSingleString(IpMessageConsts.string.ipmsg_spam_empty));
        setActivate(isActivate);

    }

    public void setGroupChatEmpty(boolean isActivate) {
        mSpam.setVisibility(View.GONE);
        mGroupChat.setVisibility(View.VISIBLE);
        mBackground.setBackgroundResource(R.color.empty_background);
        mContent.setText(IpMessageResourceMananger.getInstance(mContext)
            .getSingleString(IpMessageConsts.string.ipmsg_groupchat_empty));
        setActivate(isActivate);
    }

    public void setAllChatEmpty() {
        mBackground.setBackgroundResource(R.color.transparent);
       mContent.setText(IpMessageResourceMananger.getInstance(mContext)
            .getSingleString(IpMessageConsts.string.ipmsg_allchat_empty));
       mSpam.setVisibility(View.GONE);
       mGroupChat.setVisibility(View.GONE);
    }

    private void setActivate(boolean isActivate) {
        if (isActivate) {
            mActivate.setVisibility(View.GONE);
        } else {
            mActivate.setVisibility(View.VISIBLE);
        }
    }
}
