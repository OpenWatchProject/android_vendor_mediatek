package com.mediatek.gallery3d.video;
/**
 * Movie list extension interface
 */
public interface IMovieList {
    /**
     * Add movie item to list.
     * @param item
     */
    void add(IMovieItem item);
    /**
     * Get the item index of list
     * @param item
     * @return
     */
    int index(IMovieItem item);
    /**
     *
     * @return list size
     */
    int size();
    /**
     *
     * @param item
     * @return next item of current item
     */
    IMovieItem getNext(IMovieItem item);
    /**
     *
     * @param item
     * @return previous item of current item
     */
    IMovieItem getPrevious(IMovieItem item);

}