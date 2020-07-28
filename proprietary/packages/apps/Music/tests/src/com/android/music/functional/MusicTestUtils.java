package com.android.music.tests.functional;

import java.io.File;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import junit.framework.Assert;

import android.app.Activity;
import android.app.ListActivity;
import android.app.LocalActivityManager;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Environment;
import android.os.RemoteException;
import android.os.storage.StorageManager;
import android.provider.MediaStore;
import android.util.Log;
import android.view.View;

import com.android.music.*;

public class MusicTestUtils {
    private static final String TAG = "MusicTestUtils";
    private static final int WAIT_FOR_RESPOND_TIME = 500;
    private static final String TEST_PLAYLIST_NAME = "functiontest playlist";
    private static final String SONGS_LESS_THAN_TRHEE = "There are less than three songs or the" +
     " system performance is bad, please push more or reboot phone to make music pass test";
    private static final int MINIMUM_SONGS_COUNT_IN_MUSIC = 3;
    public static final int WAIT_TEAR_DONW_FINISH_TIME = 500;

    /**
     * Get long value by string which is declared in activity.
     *
     * @param activity
     *            activity
     * @param long
     *            The member declared in activity
     * @return The long value of member declared in activity
     */
    public static long getDeclaredLongValue(Activity activity, String string) {
        try {
            Field field = activity.getClass().getDeclaredField(string.toString());
            field.setAccessible(true);
            String fieldString = field.get(activity).toString();
            return Long.parseLong(fieldString);
        } catch (NoSuchFieldException ex) {
            Log.e(TAG, "No such field in runTestOnUiThread:", ex);
            return -1;
        } catch (IllegalAccessException ex) {
            Log.e(TAG, "Illegal access exception in runTestOnUiThread:", ex);
            return -1;
        } catch (IllegalArgumentException ex) {
            Log.e(TAG, "Illegal argument exception in runTestOnUiThread:", ex);
            return -1;
        }
    }

    /**
     * Get string value by string which is declared in activity.
     *
     * @param activity
     *            activity
     * @param string
     *            The member declared in activity
     * @return The string value of member declared in activity
     */
    public static String getDeclaredStringValue(Activity activity, String string) {
        try {
            Field field = activity.getClass().getDeclaredField(string.toString());
            field.setAccessible(true);
            String fieldString = field.get(activity).toString();
            return fieldString;
        } catch (NoSuchFieldException ex) {
            Log.e(TAG, "No such field in runTestOnUiThread:", ex);
            return null;
        } catch (IllegalAccessException ex) {
            Log.e(TAG, "Illegal access exception in runTestOnUiThread:", ex);
            return null;
        } catch (IllegalArgumentException ex) {
            Log.e(TAG, "Illegal argument exception in runTestOnUiThread:", ex);
            return null;
        }
    }

    public static void invokeDeclaredMethod(
    Activity activity, String methodName, Class[] classes, Object[] para) {
        try {
            Method method = activity.getClass().getDeclaredMethod(methodName, classes);
            method.setAccessible(true);
            method.invoke(activity, para);
            Log.d(TAG, "invokeDeclaredMethod: methodName = " +
             methodName + ", para length = " + para.length);
        } catch (NoSuchMethodException ex) {
            Log.e(TAG, "NoSuchMethodException in invokeDeclaredMethod:", ex);
        } catch (InvocationTargetException ex) {
            Log.e(TAG, "InvocationTargetException in relect method:", ex);
        } catch (IllegalAccessException ex) {
            Log.e(TAG, "IllegalAccessException in relect method:", ex);
        }
    }
    /**
     * Get local activity manager by string which is declared in MusicBrowserActivity.
     *
     * @param activity
     *            activity
     * @param long
     *            The member declared in activity
     * @return The local activity manager declared in MusicBrowserActivity
     */
    public static LocalActivityManager getDeclaredLocalActivityManager
     (Activity activity, String string) {
        try {
            Field field = activity.getClass().getDeclaredField(string.toString());
            field.setAccessible(true);
            return ((LocalActivityManager) field.get(activity));
        } catch (NoSuchFieldException ex) {
            Log.e(TAG, "No such field in runTestOnUiThread:", ex);
            return null;
        } catch (IllegalAccessException ex) {
            Log.e(TAG, "Illegal access exception in runTestOnUiThread:", ex);
            return null;
        }
    }

    /**
     * Check whether music is playing in 5000 ms.
     *
     * @return If music is playing,return true,else return false
     */
    public static boolean isPlaying(IMediaPlaybackService service) {
        long startTime = System.currentTimeMillis();
        long waitTime = 5000;
        boolean isTimeOut = false;
        try {
            while (!service.isPlaying()) {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
                if (System.currentTimeMillis() - startTime > waitTime) {
                    isTimeOut = true;
                    break;
                }
            }
        }
        catch (RemoteException e) {
            Log.e(TAG, "Fail to remote to service " + e);
        }
        catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException " + e);
        }

        return !isTimeOut;
    }

    /**
     * Check whether music stop playing in 5000 ms.
     *
     * @return If music stop playing,return true,else return false
     */
    public static boolean isStopping(IMediaPlaybackService service) {
        long startTime = System.currentTimeMillis();
        long waitTime = 5000;
        boolean isTimeOut = false;
        try {
            while (service.isPlaying()) {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
                if (System.currentTimeMillis() - startTime > waitTime) {
                    isTimeOut = true;
                    break;
                }
            }
        }
        catch (RemoteException e) {
            Log.e(TAG, "Fail to remote to service " + e);
        }
        catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException " + e);
        }

        return !isTimeOut;
    }

    /**
     * Check the error view state whether same as given state in 10000 ms.
     *
     * @return If same as given state, return true,else return false
     */
    public static boolean checkErrorViewState(View errorView, int expectState) {
        long startTime = System.currentTimeMillis();
        long waitTime = 10000;
        boolean isTimeOut = false;
        try {
            while (expectState != errorView.getVisibility()) {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
                if (System.currentTimeMillis() - startTime > waitTime) {
                    isTimeOut = true;
                    break;
                }
            }
        }
        catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException " + e);
        }
        Log.e(TAG, "checkSdcardState: expectState = " + expectState + ", actualState = "
         + errorView.getVisibility());
        return !isTimeOut;
    }

    /**
     * Makesure there are three songs in music list.
     *
     * @return If more than three song,return true,else return false
     */
    public static boolean isMoreThanThreeSongs(Context context) {
        int count = -1;
        count = MusicUtils.getAllSongs(context).length;
        // Requery the songs number again for 6 times to
        // makesure there are really no songs in Music.
        // If after 6 times requery there are no songs but
        // we sure there are some songs in sdcard, maybe
        // the phone performance is too bad.
        for (int reQueryTimes = 6, i = 1; count <= MINIMUM_SONGS_COUNT_IN_MUSIC; i++) {
            try {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
            }
            catch (InterruptedException ex) {
                Log.e(TAG, "InterruptedException in sleep", ex);
            }
            count = MusicUtils.getAllSongs(context).length;
            Log.d(TAG, "isMoreThanThreeSongs: requery times = " + i + ", song count = " + count);
            if (i > reQueryTimes) {
                break;
            }
        }
        return count > MINIMUM_SONGS_COUNT_IN_MUSIC;
    }

    /**
     * Create a test playlist.
     *
     */
    public static void createTestPlaylist(Activity activity) {
        deleteAllPlaylist(activity);
        // Create a new playlist with name of TEST_PLAYLIST_NAME
        ContentValues cv = new ContentValues();
        cv.put(MediaStore.Audio.Playlists.NAME, TEST_PLAYLIST_NAME);
        Uri newPlaylistUri = activity.getContentResolver().insert(
                MediaStore.Audio.Playlists.getContentUri("external"), cv);
        Assert.assertNotNull(newPlaylistUri);
        // Add all songs to test playlist
        String idString = newPlaylistUri.getLastPathSegment();
        long newPlaylistId = Long.parseLong(idString);
        Context context = activity.getApplicationContext();
        MusicUtils.addToPlaylist(context, MusicUtils.getAllSongs(context), newPlaylistId);

    }

    /**
     * Delete a test playlist.
     *
     */
    public static void deleteAllPlaylist(Activity activity) {
        Cursor c = activity.getContentResolver().query(
                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists._ID, MediaStore.Audio.Playlists.NAME},
                null,
                null,
                MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);
        try {
            if (c != null && c.moveToFirst()) {
                do {
                    Uri deleteUri = ContentUris.withAppendedId
                    (MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI, c.getInt(0));
                    Log.d(TAG, "deleteAllPlaylist: deleteUri = " + deleteUri +
                     ", playlist = " + c.getString(1)
                            + "(" + c.getInt(0) + ")" + ", count = " + c.getCount());
                    activity.getContentResolver().delete(deleteUri, null, null);
                } while (c.moveToNext());
            }
        } finally {
        if(c != null){
            c.close();
            c = null;}
        }
    }

    /**
     * Get the track counts of playlist with given name.
     * @param playlistName
     *            The playlist name
     * @return The track counts of the playlist
     */
    public static int getPlaylistTrackCounts(Activity activity, String playlistName) {
        Cursor c = activity.getContentResolver().query(
                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists._ID},
                MediaStore.Audio.Playlists.NAME + "=" + "'" + playlistName + "'",
                null,
                MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);
        int playlistID = -1;
        try {
            if (c != null && c.moveToFirst()) {
                playlistID = c.getInt(0);
            }
        } catch (Exception e) {
            Log.e(TAG, "cannot get valid playlist ID from Media DB!");
        } finally {
        if(c != null){
            c.close();
            c = null;
        }
        }
        c = activity.getContentResolver().query(
                MediaStore.Audio.Playlists.Members.getContentUri("external", playlistID),
                new String[] { MediaStore.Audio.Playlists.Members.AUDIO_ID },
                null,
                null,
                MediaStore.Audio.Playlists.Members.DEFAULT_SORT_ORDER);
        if (c != null) {
            try {
                return c.getCount();
            } catch (Exception e) {
                Log.e(TAG, "cannot get valid audio ID from Media DB!");
            } finally {
                c.close();
                c = null;
            }
        }
        return -1;
    }

    /**
     * Get the current listview count(such as using to get the search counts),we shoule requery
     * the songs number again for 6 times to makesure there are really no songs in Music.
     * If after 6 times requery there are no songs but we sure there are some songs in sdcard,
     * maybe the phone performance is too bad.
     *
     * @return The count of search result
     */
    public static int getListViewCount(ListActivity activity) {
        int count = activity.getListView().getCount();
        // Requery the songs number again for 6 times to makesure
        // there are really no songs in Music.
        // If after 6 times requery there are no songs but we sure
        // there are some songs in sdcard, maybe
        // the phone performance is too bad.
        for (int reQueryTimes = 6, i = 1; count <= 0; i++) {
            try {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
            }
            catch (InterruptedException e) {
                // TODO: handle exception
            }
            count = activity.getListView().getCount();
            Log.d(TAG, "getListViewCount: query times = " + i + ", song count = " + count);
            if (i > reQueryTimes) {
                break;
            }
        }
        return count;
    }

    /**
     * M: check the give view visibility status
     *
     * @param v view
     * @param expectedStatus expected visibility status
     * @return if the status is the same as expected, return true, otherwise false.
     */
    public static boolean CheckViewStatus(View v, int expectedStatus) {
        int actualStatus = -1;
        /// M: check the visibility status for 6 times.
        for (int checkTimes = 6, i = 0; i < checkTimes; i++) {
            try {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
            }
            catch (InterruptedException e) {
                // TODO: handle exception
            }
            actualStatus = v.getVisibility();
            Log.d(TAG, "CheckViewStatus: actualStatus = " + actualStatus + ", expectedStatus = "
                    + expectedStatus + ", checkTimes = " + i);
            if (expectedStatus == actualStatus) {
                return true;
            }
        }
        return false;
    }

    /**
     * Delete all unknown artist and album tracks.
     *
     */
    public static void deleteAllUnknownTracks(Activity activity) {
        Cursor c = activity.getContentResolver().query(
                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                new String[] {MediaStore.Audio.Media._ID, MediaStore.Audio.Media.DATA},
                MediaStore.Audio.Artists.ARTIST + "=?" + " OR " +
                 MediaStore.Audio.Albums.ALBUM + "=?",
                new String[] {MediaStore.UNKNOWN_STRING, MediaStore.UNKNOWN_STRING},
                MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);
        if (c != null) {
            try {
                if (!c.moveToFirst()) {
                    return;
                }
                do {
                    Uri deleteUri = ContentUris.withAppendedId
                    (MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, c.getInt(0));
                    activity.getContentResolver().delete(deleteUri, null, null);
                    try {
                        String filePath = c.getString(1);
                        File file = new File(filePath);
                        file.delete();
                    }
                    catch (SecurityException e) {
                        Log.e(TAG, "SecurityException in delete unknown tracks " + e);
                    }
                } while (c.moveToNext());
            }
            finally {
                c.close();
                c = null;
            }
        }
    }

    /**
     * M: Whether the device has a mounded sdcard or not.
     *
     * @param context
     *            a context.
     * @return If one of the sdcard is mounted, return true, otherwise return false.
     */
    public static boolean hasMountedSDcard(Context context) {
        StorageManager storageManager =
        (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        boolean hasMountedSDcard = false;
        if (storageManager != null) {
            String[] volumePath = storageManager.getVolumePaths();
            if (volumePath != null) {
                String status = null;
                int length = volumePath.length;
                for (int i = 0; i < length; i++) {
                    status = storageManager.getVolumeState(volumePath[i]);
                    MusicLogUtils.d(TAG, "hasMountedSDcard: path = "
                     + volumePath[i] + ",status = " + status);
                    if (Environment.MEDIA_MOUNTED.equals(status)) {
                        hasMountedSDcard = true;
                    }
                }
            }
        }
        MusicLogUtils.d(TAG, "hasMountedSDcard = " + hasMountedSDcard);
        return hasMountedSDcard;
    }

    /**
     * Get a private/protected field from a declared class
     * @param clazz The class where you need to get private/protected field
     * @param filedName The name of this field
     * @return The field required
     * @throws NoSuchFieldException
     */
    public static Field getPrivateField(Class clazz, String filedName) throws NoSuchFieldException {
        Field field = clazz.getDeclaredField(filedName);
        field.setAccessible(true);
        return field;
    }

    /**
     * Get a private/protected method from a declared class
     * @param clazz The class where you need to get private/protected method
     * @param methodName The name of this method
     * @return The field method
     * @throws NoSuchMethodException
     */
    public static Method getPrivateMethod
    (Class clazz, String methodName, Class<?>... parameterTypes)
            throws NoSuchMethodException {
        Method method = clazz.getDeclaredMethod(methodName, parameterTypes);
        method.setAccessible(true);
        return method;
    }

    /**
     * Check the actual value whether same as given value in 5000 ms.
     *
     * @return If same as given value, return true,else return false
     */
    public static boolean checkValueStatus(int actualValue, int expectValue) {
        long startTime = System.currentTimeMillis();
        long waitTime = 5000;
        boolean isTimeOut = false;
        try {
            while (actualValue != expectValue) {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
                if (System.currentTimeMillis() - startTime > waitTime) {
                    isTimeOut = true;
                    break;
                }
            }
        }
        catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException " + e);
        }
        Log.e(TAG, "checkValueStatus: expectValue = " + expectValue +
         ", actualValue = " + actualValue);
        return !isTimeOut;
    }

    public static boolean waitForScanFinishAfterDisableUsbMassStorage(Context context) {
        long startTime = System.currentTimeMillis();
        long waitTime = 60000;
        boolean isTimeOut = false;
        try {
            while (MusicUtils.isMediaScannerScanning(context) == true) {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
                if (System.currentTimeMillis() - startTime > waitTime) {
                    isTimeOut = true;
                    break;
                }
            }
        }
        catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException " + e);
        }
        Log.e(TAG, "waitForScanFinishAfterDisableUsbMassStorage:isMediaScannerScanning = "
         + MusicUtils.isMediaScannerScanning(context));
        return isTimeOut;
    }


}