#ifndef __MNL_AT_INTERFACE_H__
#define __MNL_AT_INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/socket.h>

#include "nmea_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

//======================================================
// GPS AT -> MNLD
//======================================================
#define GPS_AT_COMMAND_SOCK    "/data/server"                 // For receive AT command

int mnl2at_send_data();

int at_test2mnl_gps_start(void);
int at_test2mnl_gps_stop(void);
void at_command_send_cno(char* cmdline);
int at_command_parse_test_num(char* cmdline);
void at_command_send_ack(char* ack, int len);
void at_command_send_test_result();
void at_cw_test_command_send_test_result();
void at_cw_test_command_test_result(int snr, float clk_drift);
void gps_at_command_test_proc(NmeaReader* const r);
void nmea_parser_at_cmd_pre(void);

void at_cmd2mnl_hdlr(int fd);

// -1 means failure
int create_at2mnl_fd();
//======================================================
// MNLD -> GPS AT CMD
//======================================================

#ifdef __cplusplus
}
#endif

#endif
