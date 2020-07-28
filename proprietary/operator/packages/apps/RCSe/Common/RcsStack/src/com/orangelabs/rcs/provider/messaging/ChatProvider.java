/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2014 Sony Mobile Communications Inc.
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
 *
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.orangelabs.rcs.provider.messaging;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import com.gsma.services.rcs.chat.ChatLog;
import com.orangelabs.rcs.provider.CursorUtil;
import com.orangelabs.rcs.provider.history.HistoryMemberBaseIdCreator;
import com.orangelabs.rcs.service.api.ServerApiPersistentStorageException;
import com.orangelabs.rcs.utils.DatabaseUtils;
import com.orangelabs.rcs.utils.PhoneUtils;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.os.Environment;
import android.text.TextUtils;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

/**
 * Chat provider
 * 
 * @author Jean-Marc AUFFRET
 */
public class ChatProvider extends ContentProvider {

    private static final int INVALID_ROW_ID = -1;

    private static final int ADD_CHAT_CONVERSATION_ID_COLUMN = 9;
    private static final int ADD_MSG_CONVERSATION_ID_COLUMN = 10;

    //group member
    private static final int GROUP_MEMBER = 11;

    //multi message

    private static final int MULTI_PARTICIPANT_CHATS = 12;
    private static final int MULTI_PARTICIPANT_CHAT_ID = 13;

    private static final String SELECTION_WITH_CHAT_ID_ONLY = ChatData.KEY_CHAT_ID
            .concat("=?");

    private static final String SELECTION_WITH_MSG_ID_ONLY = MessageData.KEY_MESSAGE_ID
            .concat("=?");

    private static final UriMatcher sUriMatcher = new UriMatcher(UriMatcher.NO_MATCH);

    static {
        sUriMatcher.addURI(ChatData.CONTENT_URI.getAuthority(), ChatData.CONTENT_URI
                .getPath().substring(1), UriType.InternalChat.CHAT);
        sUriMatcher.addURI(ChatData.CONTENT_URI.getAuthority(), ChatData.CONTENT_URI
                .getPath().substring(1).concat("/*"), UriType.InternalChat.CHAT_WITH_ID);
        sUriMatcher.addURI(ChatLog.GroupChat.CONTENT_URI.getAuthority(),
                ChatLog.GroupChat.CONTENT_URI.getPath().substring(1), UriType.Chat.CHAT);
        sUriMatcher.addURI(ChatLog.GroupChat.CONTENT_URI.getAuthority(),
                ChatLog.GroupChat.CONTENT_URI.getPath().substring(1).concat("/*"),
                UriType.Chat.CHAT_WITH_ID);
        sUriMatcher.addURI(MessageData.CONTENT_URI.getAuthority(), MessageData.CONTENT_URI
                .getPath().substring(1), UriType.InternalMessage.MESSAGE);
        sUriMatcher.addURI(MessageData.CONTENT_URI.getAuthority(), MessageData.CONTENT_URI
                .getPath().substring(1).concat("/*"), UriType.InternalMessage.MESSAGE_WITH_ID);
        sUriMatcher.addURI(ChatLog.Message.CONTENT_URI.getAuthority(), ChatLog.Message.CONTENT_URI
                .getPath().substring(1), UriType.Message.MESSAGE);
        sUriMatcher.addURI(ChatLog.Message.CONTENT_URI.getAuthority(), ChatLog.Message.CONTENT_URI
                .getPath().substring(1).concat("/*"), UriType.Message.MESSAGE_WITH_ID);

        sUriMatcher.addURI(ChatData.CONTENT_URI.getAuthority(), "add_chat_conv_id", ADD_CHAT_CONVERSATION_ID_COLUMN);
        sUriMatcher.addURI(MessageData.CONTENT_URI.getAuthority(), "add_msg_conv_id", ADD_MSG_CONVERSATION_ID_COLUMN);
        sUriMatcher.addURI(GroupMemberData.CONTENT_URI.getAuthority(), "groupmember", GROUP_MEMBER);
        // sUriMatcher.addURI(ChatLog.GroupChatMember.CONTENT_URI.getAuthority(), "groupmember", GROUP_MEMBER);
        sUriMatcher.addURI(MultiMessageData.CONTENT_URI.getAuthority(), "multimessage", MULTI_PARTICIPANT_CHATS);
        sUriMatcher.addURI(ChatLog.MultiMessage.CONTENT_URI.getAuthority(), "multimessage", MULTI_PARTICIPANT_CHATS);
        sUriMatcher.addURI(MultiMessageData.CONTENT_URI.getAuthority(), "multimessage/#", MULTI_PARTICIPANT_CHAT_ID);
        sUriMatcher.addURI(ChatLog.MultiMessage.CONTENT_URI.getAuthority(), "multimessage/#", MULTI_PARTICIPANT_CHAT_ID);

    }

    /**
     * Database tables
     */
    /**
     * Messages table name
     */
    public static final String TABLE_MESSAGE = "message";

    /**
     * Group chats table name
     */
    public static final String TABLE_GROUP_CHAT = "groupchat";

    private static final String TABLE_GROUPMEMBER = "GroupMember";
    private static final String TABLE_MULTIMESSAGE = "multimessage";

    /**
     * Database name
     */
    public static final String DATABASE_NAME = "chat.db";

    /**
     * String to allow projection for exposed group chat URI to a set of columns.
     */
    private static final String[] GROUP_CHAT_COLUMNS_ALLOWED_FOR_EXTERNAL_ACCESS = new String[]{
            ChatData.KEY_BASECOLUMN_ID, ChatData.KEY_CHAT_ID, ChatData.KEY_CONTACT,
            ChatData.KEY_STATE, ChatData.KEY_SUBJECT, ChatData.KEY_DIRECTION,
            ChatData.KEY_TIMESTAMP, ChatData.KEY_REASON_CODE,
            ChatData.KEY_PARTICIPANTS
    };

    private static final Set<String> GROUP_CHAT_COLUMNS_SET_ALLOWED_FOR_EXTERNAL_ACCESS = new HashSet<>(
            Arrays.asList(GROUP_CHAT_COLUMNS_ALLOWED_FOR_EXTERNAL_ACCESS));

    /**
     * String to allow projection for exposed message URI to a set of columns.
     */
    private static final String[] MESSAGE_COLUMNS_ALLOWED_FOR_EXTERNAL_ACCESS = new String[]{
            MessageData.KEY_BASECOLUMN_ID, MessageData.KEY_CHAT_ID, MessageData.KEY_CONTACT,
            MessageData.KEY_CONTENT, MessageData.KEY_DIRECTION, MessageData.KEY_EXPIRED_DELIVERY,
            MessageData.KEY_MESSAGE_ID, MessageData.KEY_MIME_TYPE, MessageData.KEY_READ_STATUS,
            MessageData.KEY_REASON_CODE, MessageData.KEY_STATUS, MessageData.KEY_TIMESTAMP,
            MessageData.KEY_TIMESTAMP_DELIVERED, MessageData.KEY_TIMESTAMP_DISPLAYED,
            MessageData.KEY_TIMESTAMP_SENT, MessageData.KEY_TYPE
    };

    private static final Set<String> MESSAGE_COLUMNS_SET_ALLOWED_FOR_EXTERNAL_ACCESS = new HashSet<>(
            Arrays.asList(MESSAGE_COLUMNS_ALLOWED_FOR_EXTERNAL_ACCESS));

    private static final class UriType {

        private static final class Chat {

            private static final int CHAT = 1;

            private static final int CHAT_WITH_ID = 2;
        }

        private static final class Message {

            private static final int MESSAGE = 3;

            private static final int MESSAGE_WITH_ID = 4;
        }

        private static final class InternalChat {

            private static final int CHAT = 5;

            private static final int CHAT_WITH_ID = 6;
        }

        private static final class InternalMessage {

            private static final int MESSAGE = 7;

            private static final int MESSAGE_WITH_ID = 8;
        }

    }

    private static final class CursorType {

        private static final class Chat {

            private static final String TYPE_DIRECTORY = "vnd.android.cursor.dir/groupchat";

            private static final String TYPE_ITEM = "vnd.android.cursor.item/groupchat";
        }

        private static final class Message {

            private static final String TYPE_DIRECTORY = "vnd.android.cursor.dir/chatmessage";

            private static final String TYPE_ITEM = "vnd.android.cursor.item/chatmessage";
        }
    }
    // Create the constants used to differentiate between the different URI requests


    private static class DatabaseHelper extends SQLiteOpenHelper {
        private static final int DATABASE_VERSION = 17;

        public DatabaseHelper(Context ctx) {
            super(ctx, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            // @formatter:off
            db.execSQL("CREATE TABLE IF NOT EXISTS " + TABLE_GROUP_CHAT + '('
                    + ChatData.KEY_CHAT_ID + " TEXT NOT NULL PRIMARY KEY,"
                    + ChatData.KEY_BASECOLUMN_ID + " INTEGER NOT NULL,"
                    + ChatData.KEY_REJOIN_ID + " TEXT,"
                    + ChatData.KEY_SUBJECT + " TEXT,"
                    + ChatData.KEY_PARTICIPANTS + " TEXT NOT NULL,"
                    + ChatData.KEY_STATE + " INTEGER NOT NULL,"
                    + ChatData.KEY_REASON_CODE + " INTEGER NOT NULL,"
                    + ChatData.KEY_DIRECTION + " INTEGER NOT NULL,"
                    + ChatData.KEY_TIMESTAMP + " INTEGER NOT NULL,"
                    + ChatData.KEY_USER_ABORTION + " INTEGER NOT NULL,"
                    + ChatData.KEY_CHAIRMAN + " TEXT,"
                    + ChatData.KEY_NICKNAME + " TEXT,"
                    + ChatData.KEY_ISBLOCKED + " integer DEFAULT 0,"
                    + ChatData.KEY_CONVERSATION_ID + " TEXT DEFAULT '',"
                    + ChatData.KEY_CONTACT + " TEXT)");
            // @formatter:on
            db.execSQL("CREATE INDEX " + TABLE_GROUP_CHAT + '_' + ChatData.KEY_BASECOLUMN_ID
                    + "_idx" + " ON " + TABLE_GROUP_CHAT + '(' + ChatData.KEY_BASECOLUMN_ID
                    + ')');
            db.execSQL("CREATE INDEX " + TABLE_GROUP_CHAT + '_' + ChatData.KEY_TIMESTAMP
                    + "_idx" + " ON " + TABLE_GROUP_CHAT + '(' + ChatData.KEY_TIMESTAMP + ')');
            // @formatter:off
            db.execSQL("CREATE TABLE IF NOT EXISTS " + TABLE_MESSAGE + '('
                    + MessageData.KEY_BASECOLUMN_ID + " INTEGER NOT NULL,"
                    + MessageData.KEY_CHAT_ID + " TEXT NOT NULL,"
                    + MessageData.KEY_CONTACT + " TEXT,"
                    + MessageData.KEY_DISPLAY_NAME + " TEXT,"
                    + MessageData.KEY_MESSAGE_ID + " TEXT NOT NULL PRIMARY KEY,"
                    + MessageData.KEY_CONTENT + " TEXT,"
                    + MessageData.KEY_MIME_TYPE + " TEXT NOT NULL,"
                    + MessageData.KEY_DIRECTION + " INTEGER NOT NULL,"
                    + MessageData.KEY_STATUS + " INTEGER NOT NULL,"
                    + MessageData.KEY_REASON_CODE + " INTEGER,"
                    + MessageData.KEY_READ_STATUS + " INTEGER NOT NULL,"
                    + MessageData.KEY_TIMESTAMP + " INTEGER NOT NULL,"
                    + MessageData.KEY_TIMESTAMP_SENT + " INTEGER NOT NULL,"
                    + MessageData.KEY_TIMESTAMP_DELIVERED + " INTEGER NOT NULL,"
                    + MessageData.KEY_TIMESTAMP_DISPLAYED + " INTEGER NOT NULL,"
                    + MessageData.KEY_DELIVERY_EXPIRATION + " INTEGER ,"          /*NOT NULL*/
                    + MessageData.KEY_CONVERSATION_ID + " TEXT DEFAULT '',"
                    + MessageData.KEY_TYPE + " INTEGER,"
                    + MessageData.KEY_EXPIRED_DELIVERY + " INTEGER )");  /*NOT NULL*/
            // @formatter:on
            db.execSQL("CREATE INDEX " + TABLE_MESSAGE + '_' + MessageData.KEY_BASECOLUMN_ID
                    + "_idx" + " ON " + TABLE_MESSAGE + '(' + MessageData.KEY_BASECOLUMN_ID + ')');
            db.execSQL("CREATE INDEX " + TABLE_MESSAGE + '_' + MessageData.KEY_CHAT_ID + "_idx"
                    + " ON " + TABLE_MESSAGE + '(' + MessageData.KEY_CHAT_ID + ')');
            db.execSQL("CREATE INDEX " + MessageData.KEY_TIMESTAMP + "_idx" + " ON "
                    + TABLE_MESSAGE + '(' + MessageData.KEY_TIMESTAMP + ')');
            db.execSQL("CREATE INDEX " + MessageData.KEY_TIMESTAMP_SENT + "_idx" + " ON "
                    + TABLE_MESSAGE + '(' + MessageData.KEY_TIMESTAMP_SENT + ')');


            db.execSQL("CREATE TABLE " + TABLE_MULTIMESSAGE + " ("
                    + MultiMessageData.KEY_ID + " integer primary key autoincrement,"
                    + MultiMessageData.KEY_MESSAGE_ID + " TEXT,"
                    + MultiMessageData.KEY_CHAT_ID + " TEXT,"
                    + MultiMessageData.KEY_SUBJECT + " TEXT,"
                    + MultiMessageData.KEY_PARTICIPANTS + " TEXT,"
                    + MultiMessageData.KEY_STATUS + " integer,"
                    + MultiMessageData.KEY_DIRECTION + " integer,"
                    + MultiMessageData.KEY_TIMESTAMP + " long);");

            db.execSQL("CREATE TABLE " + TABLE_GROUPMEMBER + " ("
                    + GroupMemberData.KEY_ID + " integer primary key autoincrement,"
                    + GroupMemberData.KEY_CHAT_ID + " TEXT ,"
                    + GroupMemberData.KEY_CONTACT_NUMBER + " TEXT,"
                    + GroupMemberData.KEY_CONTACT_TYPE + " TEXT,"
                    + GroupMemberData.KEY_MEMBER_NAME + " TEXT,"
                    + GroupMemberData.KEY_PORTRAIT + " TEXT);");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int currentVersion) {
            db.execSQL("DROP TABLE IF EXISTS ".concat(TABLE_GROUP_CHAT));
            db.execSQL("DROP TABLE IF EXISTS ".concat(TABLE_MESSAGE));
            onCreate(db);
        }
    }

    private SQLiteOpenHelper mOpenHelper;

    private String getSelectionWithChatId(String selection) {
        if (TextUtils.isEmpty(selection)) {
            return SELECTION_WITH_CHAT_ID_ONLY;
        }
        return "(" + SELECTION_WITH_CHAT_ID_ONLY + ") AND (" + selection + ')';
    }

    private String[] getSelectionArgsWithChatId(String[] selectionArgs, String chatId) {
        return DatabaseUtils.appendIdWithSelectionArgs(chatId, selectionArgs);
    }

    private String getSelectionWithMessageId(String selection) {
        if (TextUtils.isEmpty(selection)) {
            return SELECTION_WITH_MSG_ID_ONLY;
        }
        return "(" + SELECTION_WITH_MSG_ID_ONLY + ") AND (" + selection + ')';
    }

    private String[] getSelectionArgsWithMessageId(String[] selectionArgs, String messageId) {
        return DatabaseUtils.appendIdWithSelectionArgs(messageId, selectionArgs);
    }

    private String[] restrictGroupChatProjectionToExternallyDefinedColumns(String[] projection)
            throws UnsupportedOperationException {
        if (projection == null || projection.length == 0) {
            return GROUP_CHAT_COLUMNS_ALLOWED_FOR_EXTERNAL_ACCESS;
        }
        for (String projectedColumn : projection) {
            if (!GROUP_CHAT_COLUMNS_SET_ALLOWED_FOR_EXTERNAL_ACCESS.contains(projectedColumn)) {
                throw new UnsupportedOperationException("No visibility to the accessed column "
                        + projectedColumn + "!");
            }
        }
        return projection;
    }

    private String[] restrictMessageProjectionToExternallyDefinedColumns(String[] projection)
            throws UnsupportedOperationException {
        if (projection == null || projection.length == 0) {
            return MESSAGE_COLUMNS_ALLOWED_FOR_EXTERNAL_ACCESS;
        }
        for (String projectedColumn : projection) {
            if (!MESSAGE_COLUMNS_SET_ALLOWED_FOR_EXTERNAL_ACCESS.contains(projectedColumn)) {
                throw new UnsupportedOperationException("No visibility to the accessed column "
                        + projectedColumn + "!");
            }
        }
        return projection;
    }

    @Override
    public boolean onCreate() {
        mOpenHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public String getType(Uri uri) {
        switch (sUriMatcher.match(uri)) {
            case UriType.InternalChat.CHAT:
                /* Intentional fall through */
            case UriType.Chat.CHAT:
                return CursorType.Chat.TYPE_DIRECTORY;

            case UriType.InternalChat.CHAT_WITH_ID:
                /* Intentional fall through */
            case UriType.Chat.CHAT_WITH_ID:
                return CursorType.Chat.TYPE_ITEM;

            case UriType.InternalMessage.MESSAGE:
                /* Intentional fall through */
            case UriType.Message.MESSAGE:
                return CursorType.Message.TYPE_DIRECTORY;

            case UriType.InternalMessage.MESSAGE_WITH_ID:
                /* Intentional fall through */
            case UriType.Message.MESSAGE_WITH_ID:
                return CursorType.Message.TYPE_ITEM;
            case ADD_CHAT_CONVERSATION_ID_COLUMN:
                return CursorType.Chat.TYPE_ITEM;
            case ADD_MSG_CONVERSATION_ID_COLUMN:
                return "vnd.android.cursor.item/message";
            case MULTI_PARTICIPANT_CHATS:
                return CursorType.Message.TYPE_ITEM;

            default:
                throw new IllegalArgumentException("Unsupported URI " + uri + "!");
        }
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        boolean isAlterTableQuery = false;

        // Generate the body of the query
        Cursor cursor = null;
        int match = sUriMatcher.match(uri);
        switch (match) {
            case UriType.InternalChat.CHAT_WITH_ID:
                String chatId = uri.getLastPathSegment();
                selection = getSelectionWithChatId(selection);
                selectionArgs = getSelectionArgsWithChatId(selectionArgs, chatId);
                SQLiteDatabase db = mOpenHelper.getReadableDatabase();
                cursor = db.query(TABLE_GROUP_CHAT, projection, selection, selectionArgs, null,
                        null, sort);
                CursorUtil.assertCursorIsNotNull(cursor, uri);
                cursor.setNotificationUri(getContext().getContentResolver(),
                        Uri.withAppendedPath(ChatLog.GroupChat.CONTENT_URI, chatId));
                return cursor;

            case UriType.InternalChat.CHAT:
                db = mOpenHelper.getReadableDatabase();
                cursor = db.query(TABLE_GROUP_CHAT, projection, selection, selectionArgs, null,
                        null, sort);
                CursorUtil.assertCursorIsNotNull(cursor, uri);
                cursor.setNotificationUri(getContext().getContentResolver(),
                        ChatLog.GroupChat.CONTENT_URI);
                return cursor;

            case UriType.Chat.CHAT_WITH_ID:
                chatId = uri.getLastPathSegment();
                selection = getSelectionWithChatId(selection);
                selectionArgs = getSelectionArgsWithChatId(selectionArgs, chatId);
                /* Intentional fall through */
                //$FALL-THROUGH$
            case UriType.Chat.CHAT:
                db = mOpenHelper.getReadableDatabase();
                cursor = db.query(TABLE_GROUP_CHAT,
                        restrictGroupChatProjectionToExternallyDefinedColumns(projection),
                        selection, selectionArgs, null, null, sort);
                CursorUtil.assertCursorIsNotNull(cursor, uri);
                cursor.setNotificationUri(getContext().getContentResolver(), uri);
                return cursor;

            case UriType.InternalMessage.MESSAGE_WITH_ID:
                String msgId = uri.getLastPathSegment();
                selection = getSelectionWithMessageId(selection);
                selectionArgs = getSelectionArgsWithMessageId(selectionArgs, msgId);
                db = mOpenHelper.getReadableDatabase();
                cursor = db.query(TABLE_MESSAGE, projection, selection, selectionArgs, null,
                        null, sort);
                CursorUtil.assertCursorIsNotNull(cursor, uri);
                cursor.setNotificationUri(getContext().getContentResolver(),
                        Uri.withAppendedPath(ChatLog.Message.CONTENT_URI, msgId));
                return cursor;

            case UriType.InternalMessage.MESSAGE:
                /* Intentional fall through */
                db = mOpenHelper.getReadableDatabase();
                cursor = db.query(TABLE_MESSAGE, projection, selection, selectionArgs, null,
                        null, sort);
                CursorUtil.assertCursorIsNotNull(cursor, uri);
                cursor.setNotificationUri(getContext().getContentResolver(),
                        ChatLog.Message.CONTENT_URI);
                return cursor;

            case UriType.Message.MESSAGE_WITH_ID:
                msgId = uri.getLastPathSegment();
                selection = getSelectionWithMessageId(selection);
                selectionArgs = getSelectionArgsWithMessageId(selectionArgs, msgId);
                /* Intentional fall through */
                //$FALL-THROUGH$
            case UriType.Message.MESSAGE:
                db = mOpenHelper.getReadableDatabase();
                cursor = db.query(TABLE_MESSAGE,
                        restrictMessageProjectionToExternallyDefinedColumns(projection),
                        selection, selectionArgs, null, null, sort);
                CursorUtil.assertCursorIsNotNull(cursor, uri);
                cursor.setNotificationUri(getContext().getContentResolver(), uri);
                return cursor;
            case GROUP_MEMBER:
                qb.setTables(TABLE_GROUPMEMBER);
                // qb.appendWhere(GroupMemberData.KEY_CHAT_ID + "=");
                // qb.appendWhere(uri.getPathSegments().get(1));
                break;
            case MULTI_PARTICIPANT_CHATS:
                qb.setTables(TABLE_MULTIMESSAGE);
                break;
            case MULTI_PARTICIPANT_CHAT_ID:
                qb.setTables(TABLE_MULTIMESSAGE);
                qb.appendWhere(MultiMessageData.KEY_CHAT_ID + "=");
                qb.appendWhere(uri.getPathSegments().get(1));
                break;
            default:
                throw new IllegalArgumentException("Unsupported URI " + uri + "!");
        }

        SQLiteDatabase db = mOpenHelper.getReadableDatabase();

        Cursor c = qb.query(db, projection, selection, selectionArgs, null, null, sort);

        // Register the contexts ContentResolver to be notified if the cursor result set changes
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(), uri);
        }
        return c;

    }

    @Override
    public int update(Uri uri, ContentValues values, String selection,
                      String[] selectionArgs) {
        int count = 0;
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();

        int match = sUriMatcher.match(uri);
        switch (match) {
            case UriType.InternalChat.CHAT_WITH_ID:
                String chatId = uri.getLastPathSegment();
                selection = getSelectionWithChatId(selection);
                selectionArgs = getSelectionArgsWithChatId(selectionArgs, chatId);
                // SQLiteDatabase db = mOpenHelper.getWritableDatabase();
                count = db.update(TABLE_GROUP_CHAT, values, selection, selectionArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(
                            Uri.withAppendedPath(ChatLog.GroupChat.CONTENT_URI, chatId), null);
                }
                return count;

            case UriType.InternalChat.CHAT:
                //db = mOpenHelper.getWritableDatabase();
                count = db.update(TABLE_GROUP_CHAT, values, selection, selectionArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(ChatLog.GroupChat.CONTENT_URI,
                            null);
                }
                return count;

            case UriType.InternalMessage.MESSAGE_WITH_ID:
                String msgId = uri.getLastPathSegment();
                selection = getSelectionWithMessageId(selection);
                selectionArgs = getSelectionArgsWithMessageId(selectionArgs, msgId);
                //db = mOpenHelper.getWritableDatabase();
                count = db.update(TABLE_MESSAGE, values, selection, selectionArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(
                            Uri.withAppendedPath(ChatLog.Message.CONTENT_URI, msgId), null);
                }
                return count;

            case UriType.InternalMessage.MESSAGE:
                // db = mOpenHelper.getWritableDatabase();
                count = db.update(TABLE_MESSAGE, values, selection, selectionArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(ChatLog.Message.CONTENT_URI,
                            null);
                }
                return count;
            case GROUP_MEMBER:
                count = db.update(TABLE_GROUPMEMBER, values, selection, null);
                break;
            case MULTI_PARTICIPANT_CHATS:
                count = db.update(TABLE_MULTIMESSAGE, values, selection, null);
                break;
            case MULTI_PARTICIPANT_CHAT_ID:
                count = db.update(TABLE_MULTIMESSAGE, values,
                        MultiMessageData.KEY_CHAT_ID + "=" + Integer.parseInt(uri.getPathSegments().get(1)), null);
                break;
            case UriType.Chat.CHAT_WITH_ID:
                /* Intentional fall through */
            case UriType.Chat.CHAT:
                /* Intentional fall through */
            case UriType.Message.MESSAGE_WITH_ID:
                /* Intentional fall through */
            case UriType.Message.MESSAGE:
                throw new UnsupportedOperationException("This provider (URI=" + uri
                        + ") supports read only access!");

            default:
                throw new IllegalArgumentException("Unsupported URI " + uri + "!");
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        switch (sUriMatcher.match(uri)) {
            case UriType.InternalChat.CHAT:
                /* Intentional fall through */
            case UriType.InternalChat.CHAT_WITH_ID:
                //SQLiteDatabase db = mOpenHelper.getWritableDatabase();
                String chatId = initialValues.getAsString(ChatData.KEY_CHAT_ID);
                initialValues.put(ChatData.KEY_BASECOLUMN_ID, HistoryMemberBaseIdCreator
                        .createUniqueId(getContext(), ChatLog.GroupChat.HISTORYLOG_MEMBER_ID));

                if (db.insertOrThrow(TABLE_GROUP_CHAT, null, initialValues) == INVALID_ROW_ID) {
                    throw new ServerApiPersistentStorageException("Unable to insert row for URI "
                            + uri + '!');
                }
                Uri notificationUri = Uri.withAppendedPath(ChatLog.GroupChat.CONTENT_URI, chatId);
                getContext().getContentResolver().notifyChange(notificationUri, null);
                return notificationUri;

            case UriType.InternalMessage.MESSAGE:
                /* Intentional fall through */
            case UriType.InternalMessage.MESSAGE_WITH_ID:
                //db = mOpenHelper.getWritableDatabase();
                String messageId = initialValues.getAsString(MessageData.KEY_MESSAGE_ID);
                initialValues.put(MessageData.KEY_BASECOLUMN_ID, HistoryMemberBaseIdCreator
                        .createUniqueId(getContext(), MessageData.HISTORYLOG_MEMBER_ID));

                if (db.insertOrThrow(TABLE_MESSAGE, null, initialValues) == INVALID_ROW_ID) {
                    throw new ServerApiPersistentStorageException("Unable to insert row for URI "
                            + uri + '!');
                }
                notificationUri = Uri.withAppendedPath(ChatLog.Message.CONTENT_URI, messageId);
                getContext().getContentResolver().notifyChange(notificationUri, null);
                return notificationUri;
            case GROUP_MEMBER:
                long grpMember = db.insertOrThrow(TABLE_GROUPMEMBER, null, initialValues);
                uri = ContentUris.withAppendedId(GroupMemberData.CONTENT_URI, grpMember);
                break;
            case MULTI_PARTICIPANT_CHAT_ID:
            case MULTI_PARTICIPANT_CHATS:
                long multi_chatRowId = db.insertOrThrow(TABLE_MULTIMESSAGE, null, initialValues);
                uri = ContentUris.withAppendedId(MultiMessageData.CONTENT_URI, multi_chatRowId);
                break;

            case UriType.Chat.CHAT:
                /* Intentional fall through */
            case UriType.Chat.CHAT_WITH_ID:
                /* Intentional fall through */
            case UriType.Message.MESSAGE:
                /* Intentional fall through */
            case UriType.Message.MESSAGE_WITH_ID:
                throw new UnsupportedOperationException("This provider (URI=" + uri
                        + ") supports read only access!");

            default:
                throw new IllegalArgumentException("Unsupported URI " + uri + "!");
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return uri;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        int count = 0;
        switch (sUriMatcher.match(uri)) {
            case UriType.InternalChat.CHAT_WITH_ID:
                String chatId = uri.getLastPathSegment();
                selection = getSelectionWithChatId(selection);
                selectionArgs = getSelectionArgsWithChatId(selectionArgs, chatId);
                //SQLiteDatabase db = mOpenHelper.getWritableDatabase();
                count = db.delete(TABLE_GROUP_CHAT, selection, selectionArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(
                            Uri.withAppendedPath(ChatLog.GroupChat.CONTENT_URI, chatId), null);
                }
                return count;

            case UriType.InternalChat.CHAT:
                //db = mOpenHelper.getWritableDatabase();
                count = db.delete(TABLE_GROUP_CHAT, selection, selectionArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(ChatLog.GroupChat.CONTENT_URI,
                            null);
                }
                return count;

            case UriType.InternalMessage.MESSAGE_WITH_ID:
                String msgId = uri.getLastPathSegment();
                selection = getSelectionWithMessageId(selection);
                selectionArgs = getSelectionArgsWithMessageId(selectionArgs, msgId);
                //db = mOpenHelper.getWritableDatabase();
                count = db.delete(TABLE_MESSAGE, selection, selectionArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(
                            Uri.withAppendedPath(ChatLog.Message.CONTENT_URI, msgId), null);
                }
                return count;

            case UriType.InternalMessage.MESSAGE:
                //db = mOpenHelper.getWritableDatabase();
                count = db.delete(TABLE_MESSAGE, selection, selectionArgs);
                if (count > 0) {
                    getContext().getContentResolver().notifyChange(ChatLog.Message.CONTENT_URI,
                            null);
                }
                return count;
            case GROUP_MEMBER:
                count = db.delete(TABLE_GROUPMEMBER, selection, selectionArgs);
                break;
            case MULTI_PARTICIPANT_CHATS:
                count = db.delete(TABLE_MULTIMESSAGE, selection, selectionArgs);
                break;
            case MULTI_PARTICIPANT_CHAT_ID:
                count = db.delete(TABLE_MULTIMESSAGE, MultiMessageData.KEY_ID + "="
                                + uri.getPathSegments().get(1)
                                + (!TextUtils.isEmpty(selection) ? " AND (" + selection + ')' : ""),
                        selectionArgs);
                break;
            case UriType.Chat.CHAT_WITH_ID:
                /* Intentional fall through */
            case UriType.Chat.CHAT:
                /* Intentional fall through */
            case UriType.Message.MESSAGE_WITH_ID:
                /* Intentional fall through */
            case UriType.Message.MESSAGE:
                throw new UnsupportedOperationException("This provider (URI=" + uri
                        + ") supports read only access!");

            default:
                throw new IllegalArgumentException("Unsupported URI " + uri + "!");
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    static final String DATABASE_LOCATION = "/data/" + "com.orangelabs.rcs" + "/databases/";
    public static void backupChatDatabase(String account) {
        try {
            String dbFile = Environment.getDataDirectory() + DATABASE_LOCATION + ChatProvider.DATABASE_NAME;
            File file = new File(dbFile);
            if (file.exists()) {
                File backupFileRoot = new File(Environment.getDataDirectory() + DATABASE_LOCATION);
                backupFileRoot.mkdirs();
                File backupFile = new File(backupFileRoot, ChatProvider.DATABASE_NAME + "_" + account + ".db");

                OutputStream outStream = new FileOutputStream(backupFile, false);
                InputStream inStream = new FileInputStream(dbFile);
                byte[] buffer = new byte[1024];
                int length;
                while ((length = inStream.read(buffer)) > 0) {
                    outStream.write(buffer, 0, length);
                }
                outStream.flush();
                outStream.close();
                inStream.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void restoreChatDatabase(String account) {
        try {
            String dbFile = Environment.getDataDirectory() + DATABASE_LOCATION + ChatProvider.DATABASE_NAME;
            File backupFileRoot = new File(Environment.getDataDirectory() + DATABASE_LOCATION);
            File file = new File(backupFileRoot, ChatProvider.DATABASE_NAME + "_" + account + ".db");
            if (!file.exists()) {
                return;
            }

            //delete the original file
            File dbOriginalFile = new File(dbFile);
            if (!dbOriginalFile.exists()) {
                dbOriginalFile.createNewFile();
            }

            new FileOutputStream(dbFile, false).close();
            // dbOriginalFile.createNewFile();
            OutputStream outStream = new FileOutputStream(dbFile, false);
            InputStream inStream = new FileInputStream(file);
            byte[] buffer = new byte[1024];
            int length;
            while ((length = inStream.read(buffer)) > 0) {
                outStream.write(buffer, 0, length);
            }
            outStream.flush();
            outStream.close();
            inStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
