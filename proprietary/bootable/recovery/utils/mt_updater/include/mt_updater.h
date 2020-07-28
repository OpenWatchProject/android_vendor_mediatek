/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
#ifndef MT_INSTALL_H
#define MT_INSTALL_H

/* Common */
void mt_init_partition_type(void);

/* SetStageFn(), GetStageFn() */
Value* mt_SetStageFn(const char* name, State* state, char** filename, char** stagestr);
Value* mt_GetStageFn(const char* name, State* state, char** filename, char *buffer);

/* Register Functions */
void Register_librecovery_updater_mtk(void);

#endif
