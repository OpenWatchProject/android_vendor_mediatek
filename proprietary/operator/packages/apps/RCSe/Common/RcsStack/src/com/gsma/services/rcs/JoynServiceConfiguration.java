/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/
package com.gsma.services.rcs;

//import com.orangelabs.rcs.platform.AndroidFactory;

import com.gsma.services.rcs.chat.ChatLog;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.SystemProperties;
import android.util.Log;

/**
 * joyn Service configuration
 *
 * @author Jean-Marc AUFFRET
 */
public class JoynServiceConfiguration {
    /**
     * Returns True if the joyn service is activated, else returns False. The
     * service may be activated or deactivated by the end user via the joyn
     * settings application.
     *
     * @param ctx
     *            Context
     * @return Boolean
     */
    /**
     * Boolean value "true"
     */
    public static final String TRUE = Boolean.toString(true);

    /**
     * Boolean value "false"
     */
    public static final String FALSE = Boolean.toString(false);

    public static boolean isServiceActivated() {
        /*
         * if
         * (AndroidFactory.getApplicationContext().checkCallingOrSelfPermission
         * (Permissions.READ_RCS_STATE) != PackageManager.PERMISSION_GRANTED) {
         * throw new SecurityException(" Required permission READ_RCS_STATE"); }
         */
        /*
         * boolean result = false; Uri databaseUri =
         * Uri.parse("content://com.orangelabs.rcs.settings/settings");
         * ContentResolver cr =
         * AndroidFactory.getApplicationContext().getContentResolver(); Cursor c
         * = cr.query(databaseUri, null, "key" + "='" + "ServiceActivated" +
         * "'", null, null); if (c != null) { if ((c.getCount() > 0) &&
         * c.moveToFirst()) { String value = c.getString(2); result =
         * Boolean.parseBoolean(value); } c.close(); } return result;
         */
        return false;
    }

    /**
     * Returns True if the joyn service is activated, else returns False. The
     * service may be activated or deactivated by the end user via the joyn
     * settings application.
     *
     * @param ctx
     * @return Boolean
     */
    public static boolean isServiceActivated(Context ctx) {
        boolean result = false;
        Uri databaseUri = Uri
                .parse("content://com.orangelabs.rcs.settings/settings");
        ContentResolver cr = ctx.getContentResolver();
        Cursor c = cr.query(databaseUri, null, "key" + "='"
                + "ServiceActivated" + "'", null, null);
        if (c != null) {
            if ((c.getCount() > 0) && c.moveToFirst()) {
                String value = c.getString(2);
                result = Boolean.parseBoolean(value);
            }
            c.close();
        }
        return result;
    }

    /**
     * Returns the display name associated to the joyn user account. The display
     * name may be updated by the end user via the joyn settings application.
     *
     * @param ctx
     *            Context
     * @return Display name
     */
    public static String getUserDisplayName() {
        // TODO: to be changed
        /*
         * String result = null; Uri databaseUri =
         * Uri.parse("content://com.orangelabs.rcs.settings/settings");
         * ContentResolver cr =
         * AndroidFactory.getApplicationContext().getContentResolver(); Cursor c
         * = cr.query(databaseUri, null, "key" + "='" + "ImsDisplayName" + "'",
         * null, null); if (c != null) { if ((c.getCount() > 0) &&
         * c.moveToFirst()) { result = c.getString(2); } c.close(); } return
         * result;
         */
        return null;
    }

    /**
     * Get the alias name
     *
     * @param contact no
     * @result alias name for contact
     */
    public static String getAliasName(Context ctx, String Contact) {
         Log.d("getAliasName ", Contact);
         Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.chat/message");
         ContentResolver cr = ctx.getContentResolver();
        String aliasName = "";
        Cursor cursor = cr.query(CONTENT_URI,
                new String[] {
                ChatLog.Message.DISPLAY_NAME,
                },
                "(" + ChatLog.Message.CONTACT + "='" + Contact + "' "+
                "AND " +ChatLog.Message.DISPLAY_NAME + " <> '' "+")",
                null,
                ChatLog.GroupChat.TIMESTAMP + " DESC");
        if (cursor.moveToFirst()) {
            String status = cursor.getString(0);
            aliasName = status;
        }
        cursor.close();
        return aliasName;
    }

    /**
     * Message Store Address
     *
      * @param ctx
      * @return String address
     */
     public String getMessageStoreAddress(Context ctx) {
         String result = null;
         Uri databaseUri = Uri
                 .parse("content://com.orangelabs.rcs.settings/settings");
         ContentResolver cr = ctx.getContentResolver();
         Cursor c = cr.query(databaseUri, null, "key" + "='" + "msgStoreAddress"
                 + "'", null, null);
         if (c != null) {
             if ((c.getCount() > 0) && c.moveToFirst()) {
                 result = c.getString(2);
             }
             c.close();
         }
         return result;
     }


    /**
     * Message Store Username
     *
      * @param ctx
      * @return String port
     */
     public String getMessageStoreUser(Context ctx) {
         String result = null;
         Uri databaseUri = Uri
                 .parse("content://com.orangelabs.rcs.settings/settings");
         ContentResolver cr = ctx.getContentResolver();
         Cursor c = cr.query(databaseUri, null, "key" + "='"
                 + "msgStoreUser" + "'", null, null);
         if (c != null) {
             if ((c.getCount() > 0) && c.moveToFirst()) {
                 result = c.getString(2);
             }
             c.close();
         }
         return result;
     }

    /**
     * Public Uri set by stack
     *
     * @param ctx
     * @return String
     */
     public String getPublicUri(Context ctx) {
         String result = "";
         Uri databaseUri = Uri.parse("content://com.orangelabs.rcs.settings/settings");
         ContentResolver cr = ctx.getContentResolver();
         Cursor c = cr.query(databaseUri, null,
                 "key" + "='" + "publicUri" + "'", null, null);
         if (c != null) {
             if ((c.getCount() > 0) && c.moveToFirst()) {
                 result = c.getString(2);
             }
             c.close();
         }

         return result;
     }

    /**
     * Public Uri set by stack
     *
     * @param ctx
     * @return String
     */
     public String getSecondaryUserIdentity(Context ctx) {
         String result = "";
         Uri databaseUri = Uri.parse("content://com.orangelabs.rcs.settings/settings");
         ContentResolver cr = ctx.getContentResolver();
         Cursor c = cr.query(databaseUri, null,
                 "key" + "='" + "publicUserIdentityPC" + "'", null, null);
         if (c != null) {
             if ((c.getCount() > 0) && c.moveToFirst()) {
                 result = c.getString(2);
             }
             c.close();
         }

         return result;
     }

     /**
      * Configuration
      *
     * @param ctx
      * @return Configuration State
      */
      public boolean getConfigurationState(Context ctx) {
          boolean result = false;
        Uri databaseUri = Uri
                .parse("content://com.orangelabs.rcs.settings/settings");
          ContentResolver cr = ctx.getContentResolver();
        Cursor c = cr.query(databaseUri, null, "key" + "='"
                + "configurationState" + "'", null, null);
          if (c != null) {
              if ((c.getCount() > 0) && c.moveToFirst()) {
                  String value = c.getString(2);
                  result = Boolean.parseBoolean(value);
              }
              c.close();
          }
          return result;
      }



      /**
       * Service Activation State
       *
     * @param ctx
     * @return String Service State
       */
       public static boolean getServiceState(Context ctx) {
           boolean result = false;
        Uri databaseUri = Uri
                .parse("content://com.orangelabs.rcs.settings/settings");
           ContentResolver cr = ctx.getContentResolver();
        Cursor c = cr.query(databaseUri, null, "key" + "='"
                + "ServiceActivated" + "'", null, null);
           if (c != null) {
               if ((c.getCount() > 0) && c.moveToFirst()) {
                   String value = c.getString(2);
                   result = Boolean.parseBoolean(value);
               }
               c.close();
           }
           return result;
       }

       /**
        * Set the root directory for files
        *
        *  @param path Directory path
        */
       public void setFileRootDirectory(String path, Context ctx) {
           Uri databaseUri = Uri
           .parse("content://com.orangelabs.rcs.settings/settings");
           if (ctx != null) {
               ContentResolver cr = ctx.getContentResolver();
               ContentValues values = new ContentValues();
               values.put("value", path);
               String where = "key" + "='" + "DirectoryPathFiles" + "'";
              // long startTime = System.currentTimeMillis();
               cr.update(databaseUri, values, where, null);
           }
       }

       /** Set the root directory for Photo
       *
       *  @param path Directory path
       */
      public void setPhotoRootDirectory(String path, Context ctx) {
          Uri databaseUri = Uri
          .parse("content://com.orangelabs.rcs.settings/settings");
          if (ctx != null) {
              ContentResolver cr = ctx.getContentResolver();
              ContentValues values = new ContentValues();
              values.put("value", path);
              String where = "key" + "='" + "DirectoryPathPhotos" + "'";
             // long startTime = System.currentTimeMillis();
              cr.update(databaseUri, values, where, null);
          }
      }

      /** Set the root directory for Videos
       *
       *  @param path Directory path
       */
      public void setVideoRootDirectory(String path, Context ctx) {
          Uri databaseUri = Uri
          .parse("content://com.orangelabs.rcs.settings/settings");
          if (ctx != null) {
              ContentResolver cr = ctx.getContentResolver();
              ContentValues values = new ContentValues();
              values.put("value", path);
              String where = "key" + "='" + "DirectoryPathVideos" + "'";
             // long startTime = System.currentTimeMillis();
              cr.update(databaseUri, values, where, null);
          }
      }

      /** Set the root directory for Photo
      *
      *  @param path Directory path
      */
      public static void setServicePermissionState(boolean state, Context ctx) {
          String stringState = "";
          if(state) {
              stringState = TRUE;
          } else {
              stringState = FALSE;
          }
          Uri databaseUri = Uri
          .parse("content://com.orangelabs.rcs.settings/settings");
          if (ctx != null) {
          ContentResolver cr = ctx.getContentResolver();
          ContentValues values = new ContentValues();
              values.put("value", stringState);
              String where = "key" + "='" + "servicePermitted" + "'";
              // long startTime = System.currentTimeMillis();
              try {
                  cr.update(databaseUri, values, where, null);
              } catch  (android.database.sqlite.SQLiteException ex) {
                  // after over-night tests, sometimes can't open database
                  ex.printStackTrace();
              }
          }
      }

      /**
       * Returns True if the App Permission is granted .
       *
       * @param ctx
       * @return Boolean
       */
      public static boolean isServicePermission(Context ctx) {
          boolean result = false;
          Uri databaseUri = Uri
                  .parse("content://com.orangelabs.rcs.settings/settings");
          ContentResolver cr = ctx.getContentResolver();
          Cursor c = cr.query(databaseUri, null, "key" + "='"
                  + "servicePermitted" + "'", null, null);
          if (c != null) {
              if ((c.getCount() > 0) && c.moveToFirst()) {
                  String value = c.getString(2);
                  result = Boolean.parseBoolean(value);
              }
              c.close();
          }
          return result;
      }


     /**
      * Public Account Auth
      *
      * @param ctx
      * @return Boolean Auth Type
      */
      public static boolean isClosedGroupSupported(Context ctx) {
          boolean result = false;
          /*Uri databaseUri = Uri
                 .parse("content://com.orangelabs.rcs.settings/settings");
          ContentResolver cr = ctx.getContentResolver();
          Cursor c = cr.query(databaseUri, null, "key" + "='"
                 + "closedGroupChat" + "'", null, null);
          if (c != null) {
              if ((c.getCount() > 0) && c.moveToFirst()) {
                  String value = c.getString(2);
                  result = Boolean.parseBoolean(value);
              }
              c.close();
          }*/

          String optr = SystemProperties.get("persist.vendor.operator.optr");
          if(optr.equalsIgnoreCase("op08") || optr.equalsIgnoreCase("op07")) {
              result = true;
          }
          return result;
      }

     /**
     * Public Account Auth
     *
     * @param ctx
     * @return Boolean Auth Type
     */
     public static boolean getIR94VideoCapabilityAuth(Context ctx) {
         boolean result = false;
         Uri databaseUri = Uri
                .parse("content://com.orangelabs.rcs.settings/settings");
         ContentResolver cr = ctx.getContentResolver();
         Cursor c = cr.query(databaseUri, null, "key" + "='"
                + "ir94VideoSupported" + "'", null, null);
         if (c != null) {
             if ((c.getCount() > 0) && c.moveToFirst()) {
                 String value = c.getString(2);
                 result = Boolean.parseBoolean(value);
             }
             c.close();
         }
         return result;
      }

     /**
      * Public Account Auth
      *
      * @param ctx
      * @return Boolean Auth Type
      */
      public static int getEnableRcsSwitch(Context ctx) {
          int result = 1;
          Uri databaseUri = Uri
                 .parse("content://com.orangelabs.rcs.settings/settings");
          ContentResolver cr = ctx.getContentResolver();
          Cursor c = cr.query(databaseUri, null, "key" + "='"
                 + "enableRcsSwitch" + "'", null, null);
          if (c != null) {
              if ((c.getCount() > 0) && c.moveToFirst()) {
                  String value = c.getString(2);
                  try {
                      result = Integer.parseInt(value);
                  } catch(Exception e) {}
              }
              c.close();
          }
          return result;
     }

     /**
      * Presence support or not
      *
      * @param ctx
      * @return Boolean support or not
      */

     public static boolean isPresenceDiscoverySupported(Context ctx){
        return (SystemProperties.getInt("persist.vendor.mtk_uce_support", 0) == 1) ? true : false;
    }
}
