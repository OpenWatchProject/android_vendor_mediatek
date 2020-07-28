/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mms.widget;

import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.widget.RemoteViews;

import com.android.mms.LogTag;
import com.android.mms.R;
/* P0 migration*/
//import com.android.mms.ui.BootActivity;
import com.android.mms.ui.ConversationList;
import com.android.mms.util.MmsLog;

public class MmsWidgetProvider extends AppWidgetProvider {
    public static final String ACTION_COMPOSE_NEW_MESSAGE =
            "com.android.mms.widget.ACTION_COMPOSE_NEW_MESSAGE";

    private static final String TAG = "MmsWidgetProvider";

    /**
     * Update all widgets in the list
     */
    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager, int[] appWidgetIds) {
        super.onUpdate(context, appWidgetManager, appWidgetIds);
        Log.i(TAG, "onUpdate is called: ");

        for (int i = 0; i < appWidgetIds.length; ++i) {
            updateWidget(context, appWidgetIds[i]);
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (Log.isLoggable(LogTag.WIDGET, Log.VERBOSE)) {
            MmsLog.vpi(TAG, "onReceive intent: " + intent);
        }
        String action = intent.getAction();
        Log.i(TAG, "onReceive action: " + action);

        // The base class AppWidgetProvider's onReceive handles the normal widget intents. Here
        // we're looking for an intent sent by the messaging app when it knows a message has
        // been sent or received (or a conversation has been read) and is telling the widget it
        // needs to update.
        super.onReceive(context, intent);
    }

    private static void updateAllWidget(Context context) {
        AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(context);
        int[] appWidgetIds = appWidgetManager.getAppWidgetIds(new ComponentName(context,
                MmsWidgetProvider.class));
        Log.i(TAG, "appWidgetIds length: " + appWidgetIds.length);
        if (appWidgetIds.length > 0) {
            // / M: fix bug ALPS00389743, all launcher mms widget should update,
            // not frist @{
            appWidgetManager.notifyAppWidgetViewDataChanged(appWidgetIds, R.id.conversation_list);
            // / @}
        }
    }

    /**
     * Update the widget appWidgetId
     */
    private static void updateWidget(Context context, int appWidgetId) {
        Log.i(TAG, "updateWidget appWidgetId: " + appWidgetId);
        RemoteViews remoteViews = new RemoteViews(context.getPackageName(), R.layout.widget);
        PendingIntent clickIntent;

        // Launch an intent to avoid ANRs
        final Intent intent = new Intent(context, MmsWidgetService.class);
        intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, appWidgetId);
        intent.setData(Uri.parse(intent.toUri(Intent.URI_INTENT_SCHEME)));
        remoteViews.setRemoteAdapter(appWidgetId, R.id.conversation_list, intent);

        // Open Mms's app conversation list when click on header
        /* P0 migration*/
        //final Intent convIntent = new Intent(context, BootActivity.class);
        final Intent convIntent = new Intent(context, ConversationList.class);
        clickIntent = PendingIntent.getActivity(
                context, 0, convIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        remoteViews.setOnClickPendingIntent(R.id.widget_header, clickIntent);

        final Intent composeIntent = new Intent(context, MmsWidgetProxyActivity.class);
        composeIntent.setAction(ACTION_COMPOSE_NEW_MESSAGE);
        clickIntent = PendingIntent.getActivity(
                context, 0, composeIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        remoteViews.setOnClickPendingIntent(R.id.widget_compose, clickIntent);

        Intent ic = new Intent(context, MmsWidgetProxyActivity.class);
        remoteViews.setPendingIntentTemplate(R.id.conversation_list,
                PendingIntent.getActivity(context, 0, ic, PendingIntent.FLAG_UPDATE_CURRENT));

        AppWidgetManager.getInstance(context).updateAppWidget(appWidgetId, remoteViews);
    }

    /*
     * notifyDatasetChanged call when the conversation list changes so the mms widget will
     * update and reflect the changes
     */
    public static void notifyDatasetChanged(Context context) {
        Log.i(TAG, "notifyDatasetChanged");
        updateAllWidget(context);
    }
}
