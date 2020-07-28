package com.mediatek.gallery3d.video;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.provider.MediaStore;

import com.mediatek.gallery3d.util.Log;
import com.mediatek.galleryportable.VideoConstantUtils;

public class DefaultMovieItem implements IMovieItem {
    private static final String TAG = "VP_DefaultMovieItem";

    private static final int SCHEME_NONE = 0;
    //mapping scheme with "file" type
    private static final int SCHEME_FILE = 1;
    //mapping scheme with "content" type
    private static final int SCHEME_CONTENT = 2;

    private static final int INVALID = -1;

    private Context mContext;
    private Uri mUri;
    private String mMimeType;
    private String mTitle;
    private String mVideoPath;
    private boolean mIsDrm;
    private int mCurId;
    private long mBuckedId;
    private String mDisplayName;
    private int mUriScheme = SCHEME_NONE;
    private int mVideoType;

    /**
     * Constructor.
     * Instantiate to record video information.
     * @param context
     * @param uri
     * @param mimeType
     * @param title
     */
    public DefaultMovieItem(Context context, Uri uri, String mimeType,
            String title) {
        Log.d(TAG, "DefaultMovieItem() construct");
        mContext = context;
        initValueByUri(uri);
        mMimeType = mimeType;
        judgeVideoType(mUri,mMimeType);
        mTitle = title;
    }

    /**
     * Constructor.
     * Instantiate to record video information.
     * @param context
     * @param uri
     * @param mimeType
     * @param title
     */
    public DefaultMovieItem(Context context, String uri, String mimeType,
            String title) {
        this(context, Uri.parse(uri), mimeType, title);
    }

    @Override
    public Uri getUri() {
        Log.d(TAG, "getUri()");
        return mUri;
    }

    @Override
    public String getMimeType() {
        Log.d(TAG, "getMimeType() mMimeType= " + mMimeType);
        return mMimeType;
    }

    @Override
    public String getTitle() {
        Log.d(TAG, "getTitle() mTitle= " + mTitle);
        return mTitle;
    }

    public void setTitle(String title) {
        Log.d(TAG, "setTitle() title= " + title);
        mTitle = title;
    }

    @Override
    public void setUri(Uri uri) {
        Log.d(TAG, "setUri() uri= " + uri);
        initValueByUri(uri);
        judgeVideoType(mUri, mMimeType);
    }

    @Override
    public void setMimeType(String mimeType) {
        Log.d(TAG, "setMimeType() mimeType= " + mimeType);
        mMimeType = mimeType;
        judgeVideoType(mUri, mMimeType);
    }

    @Override
    public String toString() {
        return new StringBuilder().append("MovieItem(uri=").append(mUri)
                .append(", mCurId=").append(mCurId)
                .append(", mBuckedId=").append(mBuckedId)
                .append(", mime=").append(mMimeType)
                .append(", title=").append(mTitle)
                .append(", isDrm=").append(mIsDrm)
                .append(", VideoPath=").append(mVideoPath)
                .append(", mDisplayName=").append(mDisplayName)
                .append(")").toString();
    }

    @Override
    public boolean isDrm() {
        Log.d(TAG, "isDrm() mIsDrm= " + mIsDrm);
        return mIsDrm;
    }

    @Override
    public boolean canBeRetrieved() {
        return (mCurId != INVALID);
    }

    @Override
    public String getVideoPath() {
        Log.d(TAG, "getVideoPath() mVideoPath= " + mVideoPath);
        return mVideoPath;
    }

    @Override
    public boolean canShare() {
        // Add for transfer URI with "file" scheme type between packages is
        // forbidden on Android N, so do not allow share it.
        return (mUri.getScheme() == null
                || !mUri.getScheme().equals(ContentResolver.SCHEME_FILE));
    }

    @Override
    public int getCurId() {
        Log.d(TAG, "getCurId() mCurId= " + mCurId);
        return mCurId;
    }

    @Override
    public long getBuckedId() {
        Log.d(TAG, "getBuckedId() mBuckedId= " + mBuckedId);
        return mBuckedId;
    }

    @Override
    public String getDisplayName() {
        Log.d(TAG, "getDisplayName() mDisplayName= " + mDisplayName);
        return mDisplayName;
    }

    @Override
    public int getVideoType() {
        Log.d(TAG, "getVideoType() mVideoType= " + mVideoType);
        return mVideoType;
    }

    @Override
    public void setVideoType(int videoType) {
        Log.d(TAG, "getVideoType() videoType= " + videoType);
        mVideoType = videoType;
    }

    private void judgeVideoType(Uri uri, String mimeType) {
        mVideoType = MovieUtils.judgeVideoType(uri, mimeType);
        Log.d(TAG, "judgeVideoType() mVideoType= " + mVideoType);
    }

    private void setDefaultValue(Uri uri) {
        Log.d(TAG, "setDefaultValue()");
        mUri = uri;
        mVideoPath = null;
        mIsDrm = false;
        mCurId = INVALID;
        mBuckedId = INVALID;
        mDisplayName = null;
    }

    private void initValueByUri(Uri uri) {
        Log.d(TAG, "initValueByUri() uri= " + uri);
        setDefaultValue(uri);
        getInfoFromDB(uri);
    }

    private int getCurId(Cursor cursor) {
        int index = cursor.getColumnIndex(MediaStore.Video.Media._ID);
        Log.d(TAG, "getCurId() index= " + index);
        if (index != INVALID) {
            mCurId = cursor.getInt(index);
        }
        return mCurId;
    }

    private Long getBucketId(Cursor cursor) {
        int index = cursor.getColumnIndex(MediaStore.Video.Media.BUCKET_ID);
        Log.d(TAG, "getBucketId() index= " + index);
        if (index != INVALID) {
            mBuckedId = cursor.getLong(index);
        }
        return mBuckedId;
    }

    private String getVideoPath(Cursor cursor) {
        int index = cursor.getColumnIndex(MediaStore.Video.Media.DATA);
        Log.d(TAG, "getVideoPath() index= " + index);
        if (index != INVALID) {
            mVideoPath = cursor.getString(index);
        }
        return mVideoPath;
    }

    private boolean isDrm(Cursor cursor) {
        int index = cursor.getColumnIndex(VideoConstantUtils.IS_DRM);
        Log.d(TAG, "isDrm() index= " + index);
        if (index != INVALID) {
            mIsDrm = (1 == cursor.getInt(index));
        }
        return mIsDrm;
    }

    private String getDisplayName(Cursor cursor) {
        int index = cursor.getColumnIndex(MediaStore.Video.Media.DISPLAY_NAME);
        Log.d(TAG, "getDisplayName() index= " + index);
        if (index != INVALID) {
            mDisplayName = cursor.getString(index);
        }
        return mDisplayName;
    }

    private boolean retrieveInfoFromDb(Uri dbUri, String[] projection, String where) {
        Cursor cursor = null;
        try {
            cursor = mContext.getContentResolver().query(dbUri,
                    projection, where, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                mCurId = getCurId(cursor);
                mBuckedId = getBucketId(cursor);
                mVideoPath = getVideoPath(cursor);
                mDisplayName = getDisplayName(cursor);
                mIsDrm = isDrm(cursor);
            }
        } catch (SQLiteException ex) {
            Log.e(TAG, "retrieveInfoFromDb encountered SQLiteException, " + ex);
            return false;
        } catch (IllegalArgumentException ex) {
            Log.e(TAG, "retrieveInfoFromDb encountered IllegalArgumentException, " + ex);
            return false;
        } catch (SecurityException ex) {
            Log.e(TAG, "retrieveInfoFromDb encountered SecurityException, " + ex);
            return false;
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        return true;
    }

    private void doRetrieve(Uri dbUri, String where) {
        Log.d(TAG, "doRetrieve() where= " + where);
        String[] projection;
        projection = new String[] {
                  MediaStore.Video.Media._ID,
                  MediaStore.Video.Media.BUCKET_ID,
                  MediaStore.Video.Media.DATA,
                  MediaStore.Video.Media.DISPLAY_NAME,
                  VideoConstantUtils.IS_DRM
        };
        if(!retrieveInfoFromDb(dbUri, projection, where)) {
            retrieveWithoutDrm(dbUri, where);
        }
        createUri(dbUri);
        Log.d(TAG, toString());
    }

    private void retrieveWithoutDrm(Uri dbUri, String where){
        String[] projection = new String[] {
                MediaStore.Video.Media._ID,
                MediaStore.Video.Media.BUCKET_ID,
                MediaStore.Video.Media.DATA,
                MediaStore.Video.Media.DISPLAY_NAME
        };
        retrieveInfoFromDb(dbUri, projection, where);
    }

    private void createUri(Uri dbUri) {
        if (mCurId != INVALID && mUriScheme == SCHEME_FILE) {
            mUri = ContentUris.withAppendedId(dbUri, mCurId);
        }
    }

    private void retrieveByContentUri(Uri uri) {
        doRetrieve(uri, null);
    }

    private void retrieveByFileUri(Uri uri) {
        String videoPath = uri.getPath();
        Log.d(TAG, "retrieveFileUri, videoPath " + videoPath);
        if (videoPath == null) {
            return;
        }
        videoPath = videoPath.replaceAll("'", "''");
        String where = MediaStore.Video.Media.DATA + " = '" + videoPath + "'";
        doRetrieve(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, where);
    }

    private int getUriScheme(Uri uri) {
        if (uri.getScheme() == null) {
            mUriScheme = SCHEME_NONE;
        } else if (uri.getScheme().equals(ContentResolver.SCHEME_FILE)) {
            mUriScheme = SCHEME_FILE;
        } else if(uri.getScheme().equals(ContentResolver.SCHEME_CONTENT)) {
            mUriScheme = SCHEME_CONTENT;
        }
        Log.d(TAG, "getUriScheme() mUriScheme= " + mUriScheme);
        return mUriScheme;
    }

    private void getInfoFromDB(Uri uri) {
        switch(getUriScheme(uri)) {
        case SCHEME_CONTENT:
            retrieveByContentUri(uri);
            break;
        case SCHEME_FILE:
            retrieveByFileUri(uri);
            break;
        default:
            //do nothing
        }
    }
}
