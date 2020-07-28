package com.mediatek.engineermode.iotconfig;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.PagerTitleStrip;

import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

public class IotConfigurationActivity extends FragmentActivity {
    private static String TAG = "Iot/IotConfigurationActivity";
    private static int[] TAB_TITLE_IOT_WFC = { R.string.iot_apn, R.string.iot_xcap,
            R.string.iot_wfc, R.string.iot_vilte };
    private static int[] TAB_TITLE_IOT_NO_WFC = { R.string.iot_apn,
            R.string.iot_xcap, R.string.iot_vilte };
    private static int[] TAB_TITLE_IOT;
    private IotViewPager mViewPager;
    private PagerTitleStrip pagerTitleStrip;
    private List<Fragment> mFragments;
    private List<String> titleList;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.iot_config);

        mViewPager = (IotViewPager) findViewById(R.id.viewpager);
        pagerTitleStrip = (PagerTitleStrip) findViewById(R.id.pagertitle);
        pagerTitleStrip.setTextSpacing(100);
        titleList = new ArrayList<String>();
        TAB_TITLE_IOT = isSupportWFC() ? TAB_TITLE_IOT_WFC : TAB_TITLE_IOT_NO_WFC;
        for (int i = 0; i < TAB_TITLE_IOT.length; i++) {
            titleList.add(getString(TAB_TITLE_IOT[i]).toString());
        }
        mFragments = new ArrayList<Fragment>();
        mFragments.add(new ApnConfigFragment());
        mFragments.add(new XCAPConfigFragment());

        if (isSupportWFC()) {
            mFragments.add(new WfcConfigFragment());
        }
        mFragments.add(new VilteConfigFragment());
        FragPagerAdapter adapter = new FragPagerAdapter(
                getSupportFragmentManager(), mFragments, titleList);
        mViewPager.setAdapter(adapter);
    }

    public boolean isSupportWFC() {
        if (FeatureSupport.isSupportWfc() && !FeatureSupport.is93Modem()) {
            return true;
        }
        return false;
    }

    public class FragPagerAdapter extends FragmentPagerAdapter {

        private List<Fragment> mFragments;
        private List<String> mTitle;

        public FragPagerAdapter(FragmentManager fm, List<Fragment> mFragments,
                List<String> title) {
            super(fm);
            // TODO Auto-generated constructor stub
            this.mFragments = mFragments;
            this.mTitle = title;
        }

        @Override
        public Fragment getItem(int arg0) {
            // TODO Auto-generated method stub
            return mFragments.get(arg0);
        }

        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return mFragments.size();
        }

        @Override
        public CharSequence getPageTitle(int position) {
            // TODO Auto-generated method stub
            return getString(TAB_TITLE_IOT[position]).toString();
        }
    }

}
