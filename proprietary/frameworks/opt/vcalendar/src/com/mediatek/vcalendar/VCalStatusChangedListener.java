package com.mediatek.vcalendar;

/**
 * Listener on status change when handling iCalendar components.
 *
 */
public interface VCalStatusChangedListener {
    /**
     * Will be called when compose/parse started.
     */
    void vCalOperationStarted(int totalCnt);

    /**
     * Will be called when the compose/parse operation finished
     *
     * @param successCnt
     *            the successful handled count
     * @param totalCnt
     *            total count
     */
    void vCalOperationFinished(int successCnt, int totalCnt, Object obj);

    /**
     * Will be called when the process status update
     *
     * @param currentCnt
     *            current handled count
     * @param totalCnt
     *            total count
     */
    void vCalProcessStatusUpdate(int currentCnt, int totalCnt);

    /**
     * Will be called when the cancel request has been finished.
     *
     * @param finishedCnt
     *            the count has been finished before the cancel operation
     * @param totalCnt
     *            total count
     */
    void vCalOperationCanceled(int finishedCnt, int totalCnt);

    /**
     * Will be called when exception occurred.
     *
     * @param finishedCnt
     *            the count has been finished before the exception occurred.
     * @param totalCnt
     *            total count
     * @param type
     *            the exception type.
     */
    void vCalOperationExceptionOccured(int finishedCnt, int totalCnt, int type);
}
