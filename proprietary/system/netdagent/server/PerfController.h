/*
 * Copyright (C) 2008 The Android Open Source Project
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
 */

#ifndef _PERF_SERVICE_H
#define _PERF_SERVICE_H


typedef int  (*user_reg_scn)(void);
typedef void (*user_reg_scn_config)(int, int, int, int, int, int);
typedef void (*user_unreg_scn)(int);
typedef void (*user_enable)(int);
typedef void (*user_disable)(int);

typedef void (*user_reset_all)(void);
typedef void (*user_disable_all)(void);
typedef void (*dump_all)(void);
typedef void (*set_favor_pid)(int);
typedef void (*notify_user_status)(int, int);


class PerfController {
public:
        PerfController();
        ~PerfController();

        static int enable_perf_rps(const char* intIface, const char* extIface);
        static int disable_perf_rps(const char* intIface, const char* extIface);
        static int enter_little_cpu();
        static int exit_little_cpu();
        static int set_rps(const char* iface,const char* rps);
        static int recover_rps(const char* iface);
        static int is_eng(void);
        static int is_testsim(void);
        static int is_op01(void);
        static int get_tether_perfhandle() {return tether_perfHandle;}
        static int get_lowpower_perfhandle() {return lowpower_perfHandle;}
        static int load_PerfService();
        static int set_ack_reduction(const char *);
        static int restore_ack_reduction();
        static int get_load();
        static void dump_cpuinfo(int type);
private:
        static int tether_perfHandle;
        static int lowpower_perfHandle;
        static user_reg_scn perfUserRegScn;
        static user_reg_scn_config perfUserRegScnConfig;
        static user_unreg_scn perfUserUnregScn;
        static user_enable perfUserScnEnable;
        static user_disable perfUserScnDisable;
        static user_reset_all perfUserScnResetAll;
        static user_disable_all perfUserScnDisableAll;
        static dump_all perfDumpAll;
        static set_favor_pid perfSetFavorPid;
        static notify_user_status perfNotifyUserStatus;
        static void dump_cpuinfo_jade();
        static void dump_cpuinfo_everest();
        static void dump_cpuinfo_olympus();
        static int parse_perf_prop(char *, char *, int parse_value[][2]);
    };

#endif
