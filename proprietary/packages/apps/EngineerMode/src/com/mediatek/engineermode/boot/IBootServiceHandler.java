package com.mediatek.engineermode.boot;

/**
 * handler interface to handle start request.
 * @author mtk81238
 *
 */
public interface IBootServiceHandler {

    public static final int HANDLE_DONE = 0;
    public static final int HANDLE_ONGOING = 1;
    public static final int HANDLE_INVALID = 100;
    /**
     * handle start request.
     * @param service EmBootStartService
     * @return HANDLE_DONE, HANDLE_ONGOING, HANDLE_INVALID;
     *     if HANDLE_ONGOING was returned, must call EmBootStartService.stopStartService
     *     to stop the start service.
     */
    public int handleStartRequest(EmBootStartService service);
}
