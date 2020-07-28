#ifndef _COMMANDDISPATCH_H
#define _COMMANDDISPATCH_H
#include <pthread.h>
#include "CommandRespondor.h"
#include "FirewallController.h"

namespace android {
namespace netdagent {

//mLock guards all accesses to mCommandDispatchList
extern pthread_mutex_t gLock;

class CommandDispatch {

public:
    CommandDispatch(const char *cmdName) : mCmdName(cmdName) { };
    virtual ~CommandDispatch() { }
    virtual int runCommand(CommandRespondor *cr, int argc, char **argv) = 0;
    virtual int runCommand(int argc, char **argv) = 0;
    const char *getCommand() { return mCmdName; }

private:
    const char *mCmdName;
};

class FirewallCmd: public CommandDispatch {
public:
    FirewallCmd();
    virtual ~FirewallCmd() {}
    int runCommand(CommandRespondor *cr, int argc, char **argv);
    int runCommand(int argc, char **argv);
private:
    int sendGenericOkFail(CommandRespondor *cli, int cond);
    static FirewallRule parseRule(const char* arg);
    static ChildChain parseChildChain(const char* arg);
    static FirewallChinaRule parseChain(const char* arg);
};

class ThrottleCmd : public CommandDispatch {
public:
    ThrottleCmd();
    virtual ~ThrottleCmd() {}
    int runCommand(CommandRespondor *cr, int argc, char **argv);
    int runCommand(int argc, char **argv);
private:
};

class NetworkCmd : public CommandDispatch {
public:
    NetworkCmd();
    virtual ~NetworkCmd() {}
    int runCommand(CommandRespondor *cr, int argc, char **argv);
    int runCommand(int argc, char **argv);
private:
};

#if 0
class ThroughputCmd : public CommandDispatch {
public:
    ThroughputCmd();
    virtual ~ThroughputCmd() {}
    int runCommand(CommandRespondor *cr, int argc, char **argv);

private:
};

class PerfCmd : public CommandDispatch {
public:
    PerfCmd();
    virtual ~PerfCmd() {}
    int runCommand(CommandRespondor *cr, int argc, char **argv);

private:

};
#endif

}  // namespace netdagent
}  // namespace android

#endif
