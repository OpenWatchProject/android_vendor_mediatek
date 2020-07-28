
#ifndef __PERFSERVD_INT_H__
#define __PERFSERVD_INT_H__

#ifdef __cplusplus
extern "C" {
#endif

int powerd_main(int argc, char** argv, void (*listen_cb)(void));

#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __PERFSERVD_INT_H__ */

