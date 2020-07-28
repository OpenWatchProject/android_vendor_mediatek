#ifndef RIL_MAL_H
#define RIL_MAL_H 1

// Declare in mtk_ril_data.c
extern void *g_rds_conn_ptr;

// API
extern void registerMalRestartCallbacks();
extern void rilMalRestartCallback(void *arg);
extern void initRdsConnection(void *param);
extern void *getDlopenHandler(const char *libPath);
extern void *getAPIbyDlopenHandler(void *dlHandler, const char *apiName);
extern void initMalApi();

#endif /* RIL_MAL_H */
