package com.mediatek.gallery3d.video;


import com.mediatek.gallery3d.ext.DefaultActivityHooker;

public class MovieHooker extends DefaultActivityHooker {
    private IMoviePlayer mPlayer;

    @Override
    public void setParameter(final String key, final Object value) {
        super.setParameter(key, value);
        if (value instanceof IMoviePlayer) {
            mPlayer = (IMoviePlayer) value;
            onMoviePlayerChanged(mPlayer);
        }
    }

    public IMoviePlayer getPlayer() {
        return mPlayer;
    }

    public void onMoviePlayerChanged(final IMoviePlayer player){}
}
