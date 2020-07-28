package com.mediatek.protect.exchange;

import android.content.Context;
import android.database.Cursor;
import android.util.Log;

import com.mediatek.protect.emailcommon.provider.SmartPushContent;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * The SmartPushCalculator responsible for the core algorithm process of the
 * smart push calculation
 */
public class SmartPushCalculator {
    private static final String TAG = "SmartPushService";
    // The habit data objects list for the calculation this time
    private ArrayList<HabitData> mHabitDataList = new ArrayList<HabitData>();
    // The SmartPushCalculator singleton
    private static SmartPushCalculator sCalculator = new SmartPushCalculator();

    private static final int HOUR = 60 * 60 * 1000;
    private static final int DAY = 24 * HOUR;

    public static SmartPushCalculator getCalculator() {
        // add for JACK compile changed.
        com.mediatek.common.jpe.a aa = new com.mediatek.common.jpe.a();
        aa.a();

        return sCalculator;
    }

    /**
     * Start the smart push calculation
     *
     * @param context
     * @param accountMap
     *            the calculatable accounts and the days of their habit data
     * @param cursors
     *            all the habit data of each account
     */
    public void startCalculate(Context context, HashMap<Long, Integer> accountMap, Cursor[] cursors) {
        if (context == null) {
            return;
        }
        // Clear the stale habit data at first
        mHabitDataList.clear();

        Set<Map.Entry<Long, Integer>> entrySet = accountMap.entrySet();
        int i = 0;
        for (Map.Entry<Long, Integer> entry : entrySet) {
            long accountId = (Long) entry.getKey();
            int dayCount = (Integer) entry.getValue();
            Log.v(TAG, "accountId:" + accountId + " ,dayCount:" + dayCount);
            // Create a HabitData object for each account
            HabitData habitData = new HabitData(context, accountId, dayCount);
            mHabitDataList.add(habitData);
            // Start to calculate for one account
            habitData.startCalculate(cursors[i++]);
        }
    }

    /**
     * Get the calculation result of the account
     *
     * @param accountId
     *            the ID of the account whose result would be fetched
     * @return the calculation result that indicates the sync interval of each
     *         time scale (1 scale = 2 hours, e.g. 2:00am-4:00am), null if no
     *         result data found for the account
     */
    public int[] getResult(long accountId) {
        for (HabitData data : mHabitDataList) {
            if (data.mAccountId == accountId) {
                return data.mResults;
            }
        }
        return null;
    }

    public static int getScaleNum() {
        return Table.SCALE_NUM;
    }

    /**
     * A HabitData object stands for an account's habit data, as well as the
     * calculating logic with those data
     */
    private static class HabitData {
        // The ID of the account whose habit data is represented by this
        // HabitData object
        long mAccountId;

        // A table stands for one-day habit data statistical table of the
        // account, so
        // this is the all statistical tables
        Table[] mTables;
        Context mContext;

        // All the habit data of this account, input these data to the
        // corresponding Table object
        // to do statistics
        ArrayList<ArrayList<TableData>> mTableData = new ArrayList<ArrayList<TableData>>();

        // The sync interval results for each time scale, which are come from
        // the calculation
        int[] mResults = new int[Table.SCALE_NUM];

        // The data structure represents a habit data record
        private class TableData {
            private int mEventType; //NOPMD
            private long mTime; //NOPMD
            private long mValue; //NOPMD

            TableData(int eventType, long time, long value) {
                mEventType = eventType;
                mTime = time;
                mValue = value;
            }
        }

        private static final int EVENTTYPE_COLUMN = 0;
        private static final int TIMESTAMP_COLUMN = 1;
        private static final int VALUE_COLUMN = 2;

        HabitData(Context context, long accountId, int dayCount) {
            mContext = context;
            mAccountId = accountId;
            mTables = new Table[dayCount];
            for (int i = 0; i < dayCount; i++) {
                mTableData.add(new ArrayList<TableData>());
            }
        }

        /**
         *
         * @param c
         *            the account habit data
         */
        private void startCalculate(Cursor c) {
            long current = System.currentTimeMillis();
            // "MOD" operation can exclude today's time
            long days = current / DAY;
            // Get the start time of today (GMT)
            long todayStartTime = days * DAY;
            if (c != null) {
                while (c.moveToNext()) {
                    long timestamp = c.getLong(TIMESTAMP_COLUMN);
                    // The day account between today start time and the habit
                    // data recording time,
                    // ignore the fragmentary day (For example, today is
                    // Sept.7th, and the recording
                    // time is Sept.4th 3:00 pm, the day count should be 2)
                    long timeSpan = todayStartTime - timestamp;
                    int day;
                    if (timeSpan <= 0) {
                        day = -1;
                    } else {
                        day = Long.valueOf(timeSpan / DAY).intValue();
                    }
                    // We only concern about the data of full day(from Sept.5th
                    // 00:00:00 GMT
                    // to Sept.7yh 00:00:00 GMT),
                    // so ignore the habit data belong to the fragmentary day
                    if (day < 0 || day >= mTables.length) {
                        continue;
                    }
                    // The start time of the day which the habit data belongs to
                    long timeStart = todayStartTime - (day + 1) * DAY;
                    // The relative time of the habit data record
                    long relativeTime = timestamp - timeStart;
                    // Add a refined habit data to the corresponding table data
                    // list
                    mTableData.get(day).add(
                            new TableData(c.getInt(EVENTTYPE_COLUMN), relativeTime, c
                                    .getLong(VALUE_COLUMN)));
                }
            }

            // So far, all the habit data of the account have been refined and
            // stored in mTableData.
            // Now input these data to the corresponding Tables to do the
            // calculation in turn
            float[] fieldSum = new float[Table.SCALE_NUM]; // The sum of the
                                                           // "chance" for each
                                                           // time scale
            for (int i = 0; i < mTables.length; i++) {
                mTables[i] = new Table();
                mTables[i].inputData(mTableData.get(i));
                Log.v(TAG, "Table[" + i + "] startCalculate...");
                mTables[i].startCalculate();
                for (int j = 0; j < Table.SCALE_NUM; j++) {
                    fieldSum[j] += mTables[i].mResults[j];
                }
            }

            // Log out the chances
            StringBuilder s = new StringBuilder("[");
            for (float f : fieldSum) {
                s.append(f + ", ");
            }
            s.append("]");
            Log.v(TAG, "chances: " + s.toString());

            // The time scales with the Top4 chances would get high syncing
            // interval,
            // the Middle4 get middle interval and the Last4 get low interval
            float[] sortSum = fieldSum.clone();
            Arrays.sort(sortSum); // sort chances ascendingly
            int n = 0;
            // Some values in sortSum may be the same, so need to avoid set a
            // result
            // element duplicately and miss another one
            Set<Integer> checked = new HashSet<Integer>();
            for (int m = 0; m < 3; m++) {
                for (; n < Table.SCALE_NUM / 3 * (m + 1); n++) {
                    for (int k = 0; k < Table.SCALE_NUM; k++) {
                        if (fieldSum[k] == sortSum[n] && !checked.contains(k)) {
                            mResults[k] = m; // m value is related to the sync
                                             // level value
                            checked.add(k);
                            Log.v(TAG, "final mResults[" + k + "]:" + m);
                            break;
                        }
                    }
                }
            }
        }
    }

    /**
     * One table stands for one day record statistics of one account
     */
    private static class Table {
        // One day has 12 time scales (24hours / 2)
        private static final int SCALE_NUM = 12;
        // The summarized habit data for each time scale
        // The 3 columns are "Time", "Mail", "Duration" respectively
        // as demanded by the calculation algorithm design
        private int[][] mSummaries = new int[SCALE_NUM][3];
        // The "chance" value of each time scale
        private float[] mResults = new float[SCALE_NUM];

        private static final int TIME_COLUMN = 0;
        private static final int MAIL_COLUMN = 1;
        private static final int DURATION_COLUMN = 2;

        private void inputData(ArrayList<HabitData.TableData> dataList) {
            // Summarize the input data by time scale
            for (HabitData.TableData data : dataList) {
                int scale = Long.valueOf(data.mTime / (HOUR * 2)).intValue();
                // In a kind of extreme case, the scale value may be 12, take
                // it as 11 in this case
                if (scale == SCALE_NUM) {
                    scale = SCALE_NUM - 1;
                }
                int eventType = data.mEventType;

                switch (eventType) {
                case SmartPushContent.TYPE_DURATION:
                    mSummaries[scale][DURATION_COLUMN] += data.mValue;
                    break;
                case SmartPushContent.TYPE_MAIL:
                    mSummaries[scale][MAIL_COLUMN] += data.mValue;
                    break;
                case SmartPushContent.TYPE_OPEN:
                    mSummaries[scale][TIME_COLUMN] += data.mValue;
                    break;
                default:
                    break;
                }
            }
        }

        /**
         * Calculate in terms of the algorithm design
         */
        private void startCalculate() {
            float j;
            float k;
            float timeF;
            float mailF;
            float durationF;
            int sumTime = 0;
            int sumMail = 0;
            int sumDuration = 0;

            for (int z = 0; z < SCALE_NUM; z++) {
                sumTime += mSummaries[z][TIME_COLUMN];
                sumMail += mSummaries[z][MAIL_COLUMN];
                sumDuration += mSummaries[z][DURATION_COLUMN];
            }

            // Set all the results as 0 and return at once if this day has no
            // habit data
            // (user did not use email this day)
            if (sumTime == 0 && sumMail == 0 && sumDuration == 0) {
                for (int z = 0; z < SCALE_NUM; z++) {
                    mResults[z] = 0;
                }
                return;
            }

            for (int z = 0; z < SCALE_NUM; z++) {
                timeF = (sumTime == 0 ? 0.0f : (float) mSummaries[z][TIME_COLUMN] / sumTime);
                mailF = (sumMail == 0 ? 0.0f : (float) mSummaries[z][MAIL_COLUMN] / sumMail);
                durationF = (sumDuration == 0 ? 0.0f : (float) mSummaries[z][DURATION_COLUMN]
                        / sumDuration);

                int temp = mSummaries[z][TIME_COLUMN] + mSummaries[z][MAIL_COLUMN];
                float bc = 0.0f;
                if (temp != 0) {
                    bc = (float) mSummaries[z][TIME_COLUMN] / temp;
                }

                j = mailF * bc;
                k = durationF * bc;
                mResults[z] = timeF * 0.6f + j * 0.1f + k * 0.3f;
            }

            // Just for debugging
            printResult();
        }

        /**
         * Log out the calculation result for the table
         */
        private void printResult() {
            Log.v(TAG, "------------------------------------\n");
            for (int i = 0; i < SCALE_NUM; i++) {
                Log.v(TAG, mSummaries[i][0] + " " + mSummaries[i][1] + " " + mSummaries[i][2] + " "
                        + mResults[i]);
                Log.v(TAG, "\n");
            }
        }
    }
}
