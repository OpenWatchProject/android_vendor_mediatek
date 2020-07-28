package com.mediatek.plugin.res;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;

import com.mediatek.plugin.utils.Log;

import java.io.IOException;
import java.io.InputStream;

/**
 * Parser Jar file resource.
 */
public class JarResource implements IResource {
    private static final String TAG = "PluginManager/JarResource";
    protected String mFilePath = null;
    protected String mPackageProcessName = null;
    private AssetManager mAssertManager;
    private Context mContext;

    /**
     * Construct.
     * @param context
     *            The package context.
     * @param filePath
     *            Jar file path.
     */
    public JarResource(Context context, String filePath) {
        mContext = context;
        mFilePath = filePath;
    }

    public AssetManager getAssertManager() {
        return mAssertManager;
    }

    @Override
    public String getString(String value) {
        Log.d(TAG, "<getString> value = " + value);
        return value;
    }

    @Override
    public Drawable getDrawable(String value) {
        Log.d(TAG, "<getDrawable> value = " + value);
        try {
            InputStream bitmapStream = mContext.getAssets().open(value);
            return new BitmapDrawable(bitmapStream);
        } catch (IOException e1) {
            Log.e(TAG, "IOException", e1);
        }
        return null;
    }

    @Override
    public String[] getString(String value, String separater) {
        if (value != null) {
            return value.split(separater);
        }
        return null;
    }

}
