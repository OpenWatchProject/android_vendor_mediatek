package com.mediatek.cellbroadcastreceiver;

import android.app.Activity;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.provider.Telephony;
import android.telephony.CellBroadcastMessage;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Iterator;

import com.mediatek.cmas.ext.ICmasMessageInitiationExt;
import com.mediatek.cmas.ext.DefaultCmasMessageInitiationExt.IAutoLinkClick;

public class CMASAlertFullWindow {

    private static CMASAlertFullWindow sInstance = null;
    public static ArrayList<ViewInfo> sShowingView = null;

    private static final String TAG = "[CMAS][CMASAlertFullWindow]";
    private static final int PRESIDENT_ALERT_ID = 4370;

    /** Intent extra for non-emergency alerts sent when user selects the notification. */
    public static final String FROM_NOTIFICATION_EXTRA = "from_notification";

    protected Context mContext;
    ArrayList<CellBroadcastMessage> mMessageList;

    /** Handler to add and remove screen on flags for emergency alerts. */
    private final ScreenOffHandler mScreenOffHandler = new ScreenOffHandler();

    //wheather this dialog is start autoly(true) or start from message list(false)
    boolean mStartAuto;

    private static WindowManager sWindowManager;

    private static final int CLEAR_SCREEN_FLAG = 1;

    private static final int ALERT_TIME_LENGTH = 10500; //10.5s

    private class ScreenOffHandler extends Handler {

        /** Package local constructor (called from outer class). */
        ScreenOffHandler() {}

        /** Clear the screen on window flags. */
        private void clearWindowFlags(ViewInfo viewInfo) {
            Log.i(TAG, "enter clearWindowFlags");
            WindowManager.LayoutParams lp = viewInfo.getLayoutParams();
            if (lp != null && lp != null && isMsgShowing(viewInfo.getMsgRowId())) {
                lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON));
                lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));

                try {
                    sWindowManager.updateViewLayout(viewInfo.getView(), lp);
                } catch (IllegalArgumentException e) {
                    // TODO: handle exception
                    e.printStackTrace();
                    Log.e(TAG, "updateViewLayout 1 exception, view not attached");
                }
            }
        }

        private void clearWindowFlags(View view, WindowManager.LayoutParams lp) {
            Log.i(TAG, "clearWindowFlags ++");
            if (lp != null && lp != null) {
                lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON));
                lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));

                try {
                    sWindowManager.updateViewLayout(view, lp);
                } catch (IllegalArgumentException e) {
                    // TODO: handle exception
                    e.printStackTrace();
                    Log.e(TAG, "updateViewLayout 2 exception, view not attached");
                }

            }
        }

        @Override
        public void handleMessage(Message msg) {
            int msgWhat = msg.what;
            Log.i(TAG, "HandleMessage msgWhat = " + msgWhat);
            switch (msgWhat) {
                case CLEAR_SCREEN_FLAG:
                    clearWindowFlags((ViewInfo) msg.obj);
                    break;

                default:
                    break;
            }
        }
    }

    public static synchronized CMASAlertFullWindow getInstance(Context ctx) {
        if (sInstance == null) {
            Log.i(TAG, "mInstance == null");
            sInstance = new CMASAlertFullWindow(ctx);
        } else {
            Log.i(TAG, "mInstance != null");
        }

        if (sShowingView == null) {
            sShowingView = new ArrayList<ViewInfo>();
        }

        return sInstance;
    }

    public CMASAlertFullWindow(Context ctx) {
        mContext = ctx;

        sWindowManager = (WindowManager) ctx.getSystemService(Activity.WINDOW_SERVICE);

        // Allow to accept Call on the CMAS Alert Screen Plugin feature.
        ICmasMessageInitiationExt optAllAnswerCallBeforeCMASDismiss = (ICmasMessageInitiationExt)
                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
        if (optAllAnswerCallBeforeCMASDismiss != null) {
            optAllAnswerCallBeforeCMASDismiss.registerBroadcastToCheckCallState(ctx);
        }
    }

    private CMASAlertLinearLayout initViewToShow(final CellBroadcastMessage message, final long msgRowId, final boolean bAutoStart) {

        LayoutInflater inflater = LayoutInflater.from(mContext);
        final CMASAlertLinearLayout view = (CMASAlertLinearLayout) inflater.inflate(getLayoutResId(), null);

        view.setOnKeyListener(new View.OnKeyListener() {

            public boolean onKey(View arg0, int arg1, KeyEvent arg2) {
                // TODO Auto-generated method stub
                Log.i(TAG, "onKey:: arg1 = " + arg1);
                return false;
            }
        });
        ((Button) view.findViewById(R.id.dismissButton)).setOnClickListener(new View.OnClickListener() {

            public void onClick(View arg0) {
                Log.i(TAG, "ok button is clicked, msgRowId = " + msgRowId);
                clearNotification(bAutoStart);
                //clearViewFlag(view);
                dismissAndMarkRead(view, message, msgRowId);
                ((Button) view.findViewById(R.id.dismissButton)).setOnClickListener(null);

                if (!sShowingView.isEmpty() && !CmasConfigManager.isTraProfile()
                    && !CmasConfigManager.isTwProfile()) {
                    Log.i(TAG, "after dismiss current dialog, there are still some other window showing");
                    //get the highest priority view and set top property
                    final ViewInfo viewInfo = getHighestPriorityShowingView();

                    if (viewInfo != null) {
                        Log.i(TAG, "get highest view to show");
                        sWindowManager.removeView(viewInfo.getView());
                        viewInfo.getLayoutParams().type =
                            WindowManager.LayoutParams.TYPE_SYSTEM_ERROR;
                        WindowManager.LayoutParams lp  =
                            initLayoutParams(WindowManager.LayoutParams.TYPE_SYSTEM_ERROR);

                        //update viewinfo's parameters stored in arraylist
                        int index = sShowingView.indexOf(viewInfo);
                        sShowingView.remove(viewInfo);
                        viewInfo.setLayoutParams(lp);
                        sShowingView.add(index, viewInfo);

                        sWindowManager.addView(viewInfo.getView(), lp);
                    }
                }
            }
        });

        updateAlertText(view, message, msgRowId);
        updateAlertIcon(view, message);

        return view;
    }

    public void showView(final CellBroadcastMessage message, final long msgRowId, final boolean bAutoStart) {
        Log.i(TAG, "showView ++, msgid = " + message.getServiceCategory() + " msgRowId = " + msgRowId);

        if (message == null) {
            Log.i(TAG, "showView: mMessageList == null, return");
            return;
        }

        //if the alert message already showing
        if (isMsgShowing(msgRowId)) {
            Log.i(TAG, "message already showing");
            return;
        }

        WindowManager.LayoutParams lp;
        if (!sShowingView.isEmpty() && !CmasConfigManager.isTraProfile()
             && !CmasConfigManager.isTwProfile()) {
            //there is some dialog already showing
            Log.i(TAG, "some other msgs already showing");
            ViewInfo topViewInfo = getTopShowedViewInfo(sShowingView);
            if (CMASUtils.getMsgPriority(
                     message.getServiceCategory()) <= topViewInfo.getPriority()) {
                Log.i(TAG, "new come message priority lower than top showed message");
                lp = initLayoutParams(WindowManager.LayoutParams.TYPE_PRIORITY_PHONE);
            } else {
                Log.i(TAG, "new come message priority higher than top showed message");
                lp = initLayoutParams(WindowManager.LayoutParams.TYPE_SYSTEM_ERROR);
            }
        } else {
            //there is no dialog showing
            lp = initLayoutParams(WindowManager.LayoutParams.TYPE_SYSTEM_ERROR);
        }

        // Make the View as Touchable, to accept the Call on the CMAS Alert Screen.
        ICmasMessageInitiationExt initMsg = (ICmasMessageInitiationExt)
                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
        if (initMsg != null) {
            Log.i(TAG, "Check for active Call on CMAS alert");
            lp = initMsg.updateViewLayoutParams(lp);
        }

        CMASAlertLinearLayout view = initViewToShow(message, msgRowId, bAutoStart);

        ViewInfo vInfo = new ViewInfo(msgRowId, view, CMASUtils.getMsgPriority(message.getServiceCategory()), lp);
        sShowingView.add(vInfo);

        if (sWindowManager == null) {
            sWindowManager = (WindowManager) mContext.getSystemService(Activity.WINDOW_SERVICE);
        }

        sWindowManager.addView(view, lp);

        Message msg = new Message();
        msg.what = CLEAR_SCREEN_FLAG;
        msg.obj = vInfo;
        mScreenOffHandler.sendMessageDelayed(msg, ALERT_TIME_LENGTH);
    }

    private WindowManager.LayoutParams initLayoutParams(int type) {
        Log.i(TAG, "initLayoutParams ++ new");
        WindowManager.LayoutParams lp = new WindowManager.LayoutParams();
        lp.type = type;

        lp.flags = WindowManager.LayoutParams.FLAG_FULLSCREEN
            | WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
            | WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON
            | WindowManager.LayoutParams.FLAG_SPLIT_TOUCH
            //|WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
            //|WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
            | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
            | WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD;

//        mLayoutParams.width = WindowManager.LayoutParams.MATCH_PARENT;
        lp.width = getMin(sWindowManager.getDefaultDisplay().getWidth(), sWindowManager.getDefaultDisplay()
                .getHeight()) - 64;
        //lp.height = WindowManager.LayoutParams.WRAP_CONTENT;
        lp.height = getMin(sWindowManager.getDefaultDisplay().getWidth(),
               sWindowManager.getDefaultDisplay().getHeight()) - 64;
        lp.gravity = Gravity.CENTER | Gravity.CENTER_HORIZONTAL | Gravity.CENTER_VERTICAL;
        lp.alpha = 1;
        lp.setTitle("title: " + SystemClock.uptimeMillis());

         return lp;
    }

    private void clearViewFlag(View view) {
        WindowManager.LayoutParams lp = new WindowManager.LayoutParams();
        lp.type = WindowManager.LayoutParams.TYPE_SYSTEM_ERROR;
        lp.flags = lp.flags & (~(WindowManager.LayoutParams.FLAG_FULLSCREEN))
            & (~(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED))
            & (~(WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON))
            & (~(WindowManager.LayoutParams.FLAG_SPLIT_TOUCH))
            & (~(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));

        try {
            sWindowManager.updateViewLayout(view, lp);
        } catch (IllegalArgumentException e) {
            // TODO: handle exception
            e.printStackTrace();
        }

    }

    public void updateShowingView(boolean bTouchable) {
        Log.i(TAG, "updateShowingView ++, bTouchable = " + bTouchable);
        if (sShowingView != null && !sShowingView.isEmpty()) {
            Iterator<ViewInfo> iterator = sShowingView.iterator();
            while (iterator.hasNext()) {
                CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
                WindowManager.LayoutParams lp = viewInfo.getLayoutParams();
                ICmasMessageInitiationExt initMsg = (ICmasMessageInitiationExt)
                    CellBroadcastPluginManager.getCellBroadcastPluginObject(
                    CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);

                    Log.i(TAG, "Check for active Call on CMAS alert");
                    lp = initMsg.updateViewLpToTouchable(lp, bTouchable);

                sWindowManager.updateViewLayout(viewInfo.getView(), lp);
            }
        }
    }

    private ViewInfo getTopShowedViewInfo(ArrayList<ViewInfo> viewInfos) {
        if (viewInfos == null || viewInfos.isEmpty()) {
            return null;
        }
        Iterator<ViewInfo> iterator = viewInfos.iterator();
        if (iterator.hasNext() && isAllPriorityTheSame()) {
            return iterator.next();
        } else {
            return getHighestPriorityShowingView();
        }
    }

    private void rmViewByRowId(long msgId) {
        if (sShowingView == null || sShowingView.isEmpty()) {
            Log.i(TAG, "rmViewContains::nothing to remove");
            return;
        }

        Iterator<ViewInfo> iterator = sShowingView.iterator();
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            if (viewInfo.getMsgRowId() == msgId) {
                Log.i(TAG, "rmViewByRowId::remove msgid = " + msgId);
                sShowingView.remove(viewInfo);
                return;
            }
        }
        return;
    }

    private ViewInfo getHighestPriorityShowingView() {
        if (sShowingView == null || sShowingView.isEmpty()) {
            Log.i(TAG, "getHighestPriorityShowingView return null");
            return null;
        }

        ViewInfo ret = null;
        int priority = -1;

        //precondition: not all view's priority is the same
        Iterator<ViewInfo> iterator = sShowingView.iterator();
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            if (viewInfo.getPriority() > priority) {
                priority = viewInfo.getPriority();
                ret = viewInfo;
            }
        }

        return ret;
    }

    private boolean isAllPriorityTheSame() {
        int priority = 0;
        if (sShowingView == null || sShowingView.isEmpty()) {
            return true;
        }

        Iterator<ViewInfo> iterator = sShowingView.iterator();
        if (iterator.hasNext()) {
            priority = iterator.next().getPriority();
        }
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            if (viewInfo.getPriority() != priority) {
                return false;
            }
        }

        return true;
    }

    public void dismissAll() {
        Log.i(TAG, "dismissAll ++, sShowingView.size() = " + sShowingView.size());
        mContext.stopService(new Intent(mContext, CellBroadcastAlertAudio.class));

        Iterator<ViewInfo> iterator = sShowingView.iterator();
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            try {
            sWindowManager.removeView(viewInfo.getView());
            } catch (IllegalArgumentException e) {
                // TODO: handle exception
                Log.e(TAG, "dismissAll exception");
                e.printStackTrace();
            }

        }

        sShowingView.clear();
    }

    /** Returns the resource ID for either the full screen or dialog layout. */
    protected int getLayoutResId() {
//        return R.layout.cell_broadcast_alert_fullscreen;
        return R.layout.cell_broadcast_alert;
    }

    private void updateAlertText(final View view, final CellBroadcastMessage message, final long msgId) {
        Log.i(TAG, "enter updateAlertText");

        int titleId = CellBroadcastResources.getDialogTitleResource(message);

        TextView titleTextView = (TextView) view.findViewById(R.id.alertTitle);
        ICmasMessageInitiationExt msgInitExt = (ICmasMessageInitiationExt)
                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
        Log.d(TAG, "hideUpdatedString() returned " + msgInitExt.hideUpdatedString());
        if (Comparer.getUpdateNumOfCb(message) > 0 &&
                !(CmasConfigManager.isTwProfile()) &&
                !(CmasConfigManager.isChileProfile()) &&
                !(CmasConfigManager.isTraProfile())
                && !msgInitExt.hideUpdatedString()
                ) {
            // >0 update message
            Log.i(TAG, "updateAlertText::this is update message");
            titleTextView.setText(mContext.getString(titleId) + " "
                    + mContext.getString(R.string.have_updated));
        } else {
            Log.i(TAG, "updateAlertIcon::this is normal message");
            titleTextView.setText(titleId);
        }

        TextView textViewMsgBody = (TextView) view.findViewById(R.id.message);
/*        if(getShowMsgId()) {
            //only TMO need to show MsgId, and did not support show hyper link. for TMO, getSetTextViewAutoLink return false
            content = "ID: " + CMASUtils.convertMsgId2Str(message.getServiceCategory()) + "\r\n"
                + CellBroadcastResources.getMessageDetails(mContext, message) + "\r\n"
                + message.getMessageBody();
        } else {
            content = CellBroadcastResources.getMessageDetails(mContext, message) + "\r\n"
            + message.getMessageBody();
        }*/
        String content = "";
        if (CmasConfigManager.isTwProfile()) {
            content = message.getMessageBody();
        } else {
            content = getShowMsgContent(CMASUtils.convertMsgId2Str(message.getServiceCategory()),
                CellBroadcastResources.getMessageDetails(mContext, message) + "\r\n"
                + message.getMessageBody());
            if (CmasConfigManager.getGeminiSupport()) {
                int subId = message.getSubId();
                SubscriptionInfo subInfo = SubscriptionManager.from(mContext).
                                   getActiveSubscriptionInfo(subId);
                if (subInfo != null) {
                    content = "From: " + subInfo.getDisplayName().toString() + "\r\n" + content;
                }
            }
        }

        //set typeface
        textViewMsgBody.setTypeface(Typeface.SANS_SERIF);

        //plugin set textview autolink
        Log.i(TAG, "before set textview content");
        IAutoLinkClick autoLinkClick = new IAutoLinkClick() {

            @Override
            public void onAutoLinkClicked() {
                // TODO Auto-generated method stub
                dismissAndMarkRead(view, message, msgId);
            }
        };

        setTextViewContent(textViewMsgBody, content, autoLinkClick, message.getServiceCategory());

    }

    private void updateAlertIcon(View view, CellBroadcastMessage message) {
        ImageView alertIcon = (ImageView) view.findViewById(R.id.icon);

        if (Comparer.getUpdateNumOfCb(message) > 0) {
            // >0 update message
            Log.i(TAG, "updateAlertIcon::this is update message");
            alertIcon.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.ic_dialog_update_alarm));
        } else {
            Log.i(TAG, "updateAlertIcon::this is normal message");
            alertIcon.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.ic_dialog_alarm));
        }

    }

//    private boolean setTextViewContent(TextView tv, String content,
//                             IAutoLinkClick autoLinkClick, int msgId) {
//        Log.i(TAG, "setTextViewContent ++");
//        ICmasMessageInitiationExt optSetAutoLink = (ICmasMessageInitiationExt)
//        CellBroadcastPluginManager.getCellBroadcastPluginObject(
//        CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
//        if (optSetAutoLink != null) {
//            return optSetAutoLink.setTextViewContent(tv, content, autoLinkClick, msgId);
//        }
//
//        return false;
//    }

    private boolean setTextViewContent(TextView tv, String content,
                      IAutoLinkClick autoLinkClick, int msgId) {
        Log.i(TAG, "setTextViewContent ++");
        SpannableString text = new SpannableString(content);
        CMASLinkify.addLinks(text, CMASLinkify.ALL, autoLinkClick);

        tv.setText(text);
        tv.setMovementMethod(LinkMovementMethod.getInstance());

        return true;
    }

    private String getShowMsgContent(String msgId, String content) {
        Log.i(TAG, "getShowMsgContent ++");
        ICmasMessageInitiationExt initMsg = (ICmasMessageInitiationExt)
        CellBroadcastPluginManager.getCellBroadcastPluginObject(
        CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
        if (initMsg != null) {
            return initMsg.getMsgContent(msgId, content);
        }

        return null;
    }


    /** Returns the currently displayed message. */
    CellBroadcastMessage getLatestMessage() {
        int index = mMessageList.size() - 1;
        if (index >= 0) {
            return mMessageList.get(index);
        } else {
            return null;
        }
    }

    /** Removes and returns the currently displayed message. */
    private CellBroadcastMessage removeLatestMessage() {
        int index = mMessageList.size() - 1;
        if (index >= 0) {
            return mMessageList.remove(index);
        } else {
            return null;
        }
    }

    private void dismissAndMarkRead(View view, CellBroadcastMessage msg, long msgRowId) {
        Log.i(TAG, "dismissAndMarkRead ++, msgRowId = " + msgRowId);
        mContext.stopService(new Intent(mContext, CellBroadcastAlertAudio.class));

        // Mark the alert as read.
        markRead(msg);
        Log.i(TAG, "dismissAndMarkRead::sShowingView.size() = " + sShowingView.size());
        /*if(isMsgShowing(msgRowId)) {
            Log.i(TAG, "dismissAndMarkRead:: before remove review");
            mWindowManager.removeView(view);
        }*/

        try {
            Log.i(TAG, "dismissAndMarkRead:: before remove review");
            sWindowManager.removeView(view);
            rmViewByRowId(msgRowId);
        } catch (IllegalArgumentException e) {
            // TODO: handle exception
            Log.i(TAG, "dismissAndMarkRead:: removeView Exception");
        }

        return;
    }

    private boolean isMsgShowing(long msgRowId) {
        if (sShowingView == null || sShowingView.isEmpty()) {
            return false;
        }

        Iterator<ViewInfo> iterator = sShowingView.iterator();
        while (iterator.hasNext()) {
            CMASAlertFullWindow.ViewInfo viewInfo = (CMASAlertFullWindow.ViewInfo) iterator.next();
            if (viewInfo.getMsgRowId() == msgRowId) {
                return true;
            }
        }

        return false;
    }

    private void markRead(final CellBroadcastMessage msg) {
        // Mark broadcast as read on a background thread.
        new CellBroadcastContentProvider.AsyncCellBroadcastTask(mContext.getContentResolver())
                .execute(new CellBroadcastContentProvider.CellBroadcastOperation() {
                    @Override
                    public boolean execute(CellBroadcastContentProvider provider) {
                        return provider.markBroadcastRead(
                                Telephony.CellBroadcasts.DELIVERY_TIME, msg.getDeliveryTime());
                    }
                });

        return;
    }

    private void clearNotification(boolean bClear) {
        Log.i(TAG, "enter clearNotification");
        if (bClear) {
            Log.i(TAG, "Dimissing notification");
            NotificationManager notificationManager = (NotificationManager) mContext
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager.cancel(CMASPresentationService.NOTIFICATION_ID);
        }
    }

    private int getMin(int width, int height) {
        if (width < height) {
            return width;
        }

        return height;
    }

    private class ViewInfo {
        private long mMsgRowId;
        private View mView;
        private int mPriority;
        private WindowManager.LayoutParams mLp;

        public ViewInfo(long msgId, View view, int priority, WindowManager.LayoutParams lp) {
            this.mMsgRowId = msgId;
            this.mView = view;
            this.mPriority = priority;
            this.mLp = lp;
        }

        public long getMsgRowId() {
            return this.mMsgRowId;
        }

        public View getView() {
            return this.mView;
        }

        public int getPriority() {
            return this.mPriority;
        }

        public WindowManager.LayoutParams getLayoutParams() {
            return this.mLp;
        }

        public void setLayoutParams(WindowManager.LayoutParams lp) {
            this.mLp = lp;
        }
    }

    private class ExtView extends View {

        public ExtView(Context context) {
            super(context);
            // TODO Auto-generated constructor stub
        }

        @Override
        public boolean onKeyDown(int keyCode, KeyEvent keyEvent) {
            Log.i(TAG, "onkeydown, keycode =" + keyCode);
            super.onKeyDown(keyCode, keyEvent);

            return true;
        }
    }

/*    public interface IAutoLinkClick {
        void onAutoLinkClicked();//dismiss the dialog
    }*/
}
