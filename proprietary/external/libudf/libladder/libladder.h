#ifndef _LIBLADDER_H
#define _LIBLADDER_H

#ifdef __cplusplus
extern "C" {
#endif


extern "C" bool UnwindCurProcessBT(std::string *strBacktrace);
extern "C" bool UnwindCurThreadBT(std::string *strBacktrace);
extern "C" bool UnwindThreadBT(pid_t tid,std::string *strBacktrace);
extern "C" bool UnwindCurProcessBT_Vector(std::vector<std::string> *strBacktrace);

#ifdef __cplusplus
}
#endif

#endif
