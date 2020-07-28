package com.mediatek.engineermode.mdmcomponent;

import android.app.Activity;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class MdmLinearLayout extends LinearLayout {

    private static final String TAG = "EMInfo_MdmLinearLayout";
    private TextView  textView;
    private ListView  listView;
    private Activity mActivity;
    private View convertView;

    public MdmLinearLayout(Activity context) {
        super(context);
        mActivity = context;
        initView(mActivity);

        // TODO Auto-generated constructor stub
    }
    public MdmLinearLayout(Activity context, AttributeSet attrs) {
        super(context, attrs);
        // TODO Auto-generated constructor stub
        mActivity = context;
        initView(mActivity);
    }

    public void initView(Activity context) {
        LayoutInflater inflater = context.getLayoutInflater();
        convertView = inflater.inflate(R.layout.em_muti_info_entry, null);
        addView(convertView);
        textView=(TextView) convertView.findViewById(R.id.detail_title_muti_mdm);
        listView=(ListView) convertView.findViewById(R.id.detail_listview_muti_mdm);
    }

    public void setTextContent(String content) {
        textView.setText(content);
    }
    public void hideView() {
        convertView.setVisibility(View.GONE);
    }

    public void showView() {
        convertView.setVisibility(View.VISIBLE);
    }

    public void hideTitle() {
        textView.setVisibility(View.GONE);
    }

    public void showTitle() {
        textView.setVisibility(View.VISIBLE);
    }

    public void setAdapter(ListAdapter adapter) {
        listView.setAdapter(adapter);
    }

    View getListView() {
        return listView;
    }

    View getTextView() {
        return textView;
    }

    public void setBackgroudColor(int resid) {
        listView.setBackgroundResource(resid);
    }

    public void setTextBackgroudColor(int resid){
        textView.setBackgroundResource(resid);
    }

    public void setListViewHeightBasedOnChildren() {
        ListAdapter listAdapter = listView.getAdapter();
        if (listAdapter == null || listAdapter.getCount() == 0) {
            return;
        }

        int totalHeight = 0;
        int widthMeasureSpec = View.MeasureSpec.makeMeasureSpec(
                mActivity.getWindowManager().getDefaultDisplay().getWidth(),
                View.MeasureSpec.AT_MOST);
        int size = View.MeasureSpec.getSize(widthMeasureSpec);
        int heightMeasureSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
        ViewGroup.LayoutParams lp = new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);

        for (int i = 0; i < listAdapter.getCount(); i++) {
            View listItem = listAdapter.getView(i, null, listView);
            if (listItem instanceof ViewGroup) listItem.setLayoutParams(lp);
            listItem.measure(widthMeasureSpec, heightMeasureSpec);
            totalHeight += listItem.getMeasuredHeight();
        }

        totalHeight += listView.getPaddingTop() + listView.getPaddingBottom();
        totalHeight += (listView.getDividerHeight() * (listAdapter.getCount() - 1 ));
        ViewGroup.LayoutParams params = listView.getLayoutParams();
        params.height = totalHeight;

        listView.setLayoutParams(params);
        listView.requestLayout();
    }

}
