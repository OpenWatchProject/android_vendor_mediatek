#ifndef NETDAGENT_UTILS_H
#define NETDAGENT_UTILS_H

#include <pthread.h>
#include <string>
#include <chrono>

namespace android {
namespace netdagent {

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define CMD_ARG_SIZE 1024
#define CMD_ARG_COUNT 26

extern const char * const IPTABLES_PATH;
extern const char * const IP6TABLES_PATH;
extern const char * const IPTABLES_RESTORE_PATH;
extern const char * const IP6TABLES_RESTORE_PATH;
extern const char * const TC_PATH;
extern const char * const IP_PATH;
extern const char * const NDC_PATH;

bool isIfaceName(const char *name);

//interface ioctl
int ifc_is_up(const char *name, unsigned *isup);
int ifc_init(void);
void ifc_close(void);
int ifc_up(const char *name);
int ifc_down(const char *name);

//file handling
#if !defined(_WIN32) && !defined(O_BINARY)
#define O_BINARY 0
#endif
bool ReadFdToString(int fd, std::string* content);
bool ReadFileToString(const std::string& path, std::string* content);
bool WriteStringToFd(const std::string& content, int fd);
bool WriteStringToFile(const std::string& content, const std::string& path,
                       mode_t mode, uid_t owner, gid_t group);
bool WriteStringToFile(const std::string& content, const std::string& path);

//time handling
class Stopwatch {
public:
    Stopwatch() : mStart(clock::now()) {}

    virtual ~Stopwatch() {};

    float timeTaken() const {
        return getElapsed(clock::now());
    }

    float getTimeAndReset() {
        const auto& now = clock::now();
        float elapsed = getElapsed(now);
        mStart = now;
        return elapsed;
    }

private:
    typedef std::chrono::steady_clock clock;
    typedef std::chrono::time_point<clock> time_point;
    time_point mStart;

    float getElapsed(const time_point& now) const {
        using ms = std::chrono::duration<float, std::ratio<1, 1000>>;
        return (std::chrono::duration_cast<ms>(now - mStart)).count();
    }
};

class MutexLock {
public:
    MutexLock() { pthread_mutex_init(&mMutex, NULL); }
    ~MutexLock() { pthread_mutex_destroy(&mMutex); }
    void lock() { pthread_mutex_lock(&mMutex); }
    void unlock() { pthread_mutex_unlock(&mMutex); }
private:
    pthread_mutex_t mMutex;
};
  
class AutoMutexLock {
public:
    AutoMutexLock(MutexLock& mutex) : mAutolock(mutex) {
        mAutolock.lock();
    }

    ~AutoMutexLock() {
        mAutolock.unlock();
    }

private:
    MutexLock& mAutolock;
};

}  // namespace netdagent
}  // namespace android

#endif

