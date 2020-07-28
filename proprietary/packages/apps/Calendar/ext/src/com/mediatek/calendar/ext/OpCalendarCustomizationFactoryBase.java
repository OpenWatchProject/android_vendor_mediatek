package com.mediatek.calendar.ext;

import android.content.Context;
import java.util.ArrayList;
import java.util.List;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

public class OpCalendarCustomizationFactoryBase {

    public ILunarExt makeLunarCalendar(Context context) {
        return new DefaultLunarExtension();
    }

    public IEditEventViewExt makeEditEventCalendar(Context context) {
        return new DefaultEditEventViewExt();
    }

    private static final List<OperatorFactoryInfo> sOpFactoryInfoList
                                                = new ArrayList<OperatorFactoryInfo>();
    static {
        sOpFactoryInfoList.add(
                new OperatorFactoryInfo("OP01Calendar.apk",
                         "com.mediatek.op01.calendar.Op01CalendarCustomizationFactory",
                         "com.mediatek.op01.calendar",
                         "OP01"
                        ));
        sOpFactoryInfoList.add(
                new OperatorFactoryInfo("OP02Calendar.apk",
                         "com.mediatek.op02.calendar.Op02CalendarCustomizationFactory",
                         "com.mediatek.op02.calendar",
                         "OP02"
                        ));
    }

    static OpCalendarCustomizationFactoryBase sFactory = null;
    public static synchronized OpCalendarCustomizationFactoryBase getOpFactory(Context context) {
        if (sFactory == null) {
            sFactory = (OpCalendarCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                           .loadFactory(context, sOpFactoryInfoList);
            if (sFactory == null) {
                sFactory = new OpCalendarCustomizationFactoryBase();
            }
        }
        return sFactory;
    }
}