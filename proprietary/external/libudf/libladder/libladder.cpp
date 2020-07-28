#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
//#include <atomic>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

#include <android-base/stringprintf.h>
#include <unwindstack/Log.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsGetLocal.h>
#include <unwindstack/Unwinder.h>
#include <libladder.h>
#include <log/log.h>
#include <inttypes.h>
#include <time.h>



#define THREAD_SIGNAL (__SIGRTMIN+1)


namespace unwindstack {

///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

static std::atomic_bool g_finish;
static std::atomic_bool g_cond_wait;
static std::atomic_uintptr_t g_ucontext;
static pthread_mutex_t g_process_unwind_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_sigaction_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_wait_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* PATH_THREAD_NAME = "/proc/self/task/%d/comm";

#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
struct timespec t[8];
#endif

static std::string getThreadName(pid_t tid) {
	char path[PATH_MAX];
	char* procName = NULL;
	char procNameBuf[128];
	FILE* fp;
	snprintf(path, sizeof(path), PATH_THREAD_NAME, tid);
	if ((fp = fopen(path, "r"))) {
		procName = fgets(procNameBuf, sizeof(procNameBuf), fp);
		fclose(fp);
	} else {
		ALOGE("%s: Failed to open %s", __FUNCTION__, path);
        }
	if (procName == NULL) {
		// Reading /proc/self/task/%d/comm failed due to a race
		return android::base::StringPrintf("[err-unknown-tid-%d]", tid);
	}
	// Strip ending newline
	strtok(procName, "\n");
	return android::base::StringPrintf("%s", procName);
}

#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
bool ReadFileToString(const std::string& name, std::string* output) {
	char buffer[1024];
	FILE* file = fopen(name.c_str(), "r");
	if (file == NULL) return false;

	while (true) {
		size_t n = fread(buffer, 1, sizeof(buffer), file);
		if (n <= 0) break;
		output->append(buffer, n);
	}

	int error = ferror(file);
	if (fclose(file) != 0) return false;
	return error == 0;
}

void PrintThreadKernelStack(pid_t tid) {
	std::string kernel_stack_filename(android::base::StringPrintf("/proc/self/task/%d/stack", tid));
	std::string kernel_stack;
	if (ReadFileToString(kernel_stack_filename, &kernel_stack)) {
		ALOGE("%s\n %s\n", kernel_stack_filename.c_str(), kernel_stack.c_str());
	}
}
#endif

static void GetThreads(pid_t pid, std::vector<pid_t>* threads) {
	// Get the list of tasks.
	char task_path[128];
	snprintf(task_path, sizeof(task_path), "/proc/%d/task", pid);

	std::unique_ptr<DIR, decltype(&closedir)> tasks_dir(opendir(task_path), closedir);
	//ASSERT_TRUE(tasks_dir != nullptr);
	struct dirent* entry;
	while ((entry = readdir(tasks_dir.get())) != nullptr) {
		char* end;
		pid_t tid = strtoul(entry->d_name, &end, 10);
		if (*end == '\0') {
		threads->push_back(tid);
		}
 	}
}


static void SignalHandler(int, siginfo_t*, void* sigcontext) {
#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
	clock_gettime(CLOCK_MONOTONIC, &t[1]);
#endif
	while(!g_cond_wait.load()) {
		usleep(1000);
	}
#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
	clock_gettime(CLOCK_MONOTONIC, &t[2]);
#endif
	g_ucontext = reinterpret_cast<uintptr_t>(sigcontext);
	pthread_mutex_lock(&g_wait_mutex);
	pthread_cond_signal(&g_wait_cond);
	pthread_mutex_unlock(&g_wait_mutex);
	while (!g_finish.load()) {
		usleep(1000);
	}
#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
	clock_gettime(CLOCK_MONOTONIC, &t[3]);
#endif
}

static bool UnwindThread(pid_t tid, std::string *str) {
	struct sigaction act, oldact;
	pid_t pid= getpid();
	g_finish = false;
	g_cond_wait = false;
	g_ucontext = 0;
	pthread_mutex_lock(&g_sigaction_mutex);
	memset(&act, 0, sizeof(act));
	memset(&oldact, 0, sizeof(oldact));
	act.sa_sigaction = SignalHandler;
	act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
	if(sigaction(THREAD_SIGNAL, &act, &oldact) !=0) {
		ALOGE("sigaction failed: %s", strerror(errno));
		pthread_mutex_unlock(&g_sigaction_mutex);
		return false;
    	}
	ALOGE("unwind pid(%d),tid(%d)\n",pid,tid);

#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
	clock_gettime(CLOCK_MONOTONIC, &t[0]);
#endif

	// Portable tgkill method.
	if (tgkill(pid, tid, THREAD_SIGNAL) != 0) {
		// Do not emit an error message, this might be expected. Set the
		// error and let the caller decide.
		if (errno == ESRCH) {
			ALOGE("errno == ESRCH: %s,tid:%d", strerror(errno),tid);//BACKTRACE_UNWIND_ERROR_THREAD_DOESNT_EXIST;
		} else {
			ALOGE("errno != ESRCH: %s,tid:%d", strerror(errno),tid);
		}
	}else {
		// Wait for context data.
		pthread_mutex_lock(&g_wait_mutex);
		g_cond_wait = true;

		void* ucontext;
		struct timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
			ALOGE("Get clock time error");
		}
		ts.tv_sec += 5;
#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
		clock_gettime(CLOCK_MONOTONIC, &t[4]);
#endif
		int ret = pthread_cond_timedwait(&g_wait_cond, &g_wait_mutex, &ts);
		if (ret != 0) {
			ALOGE("pthread_cond_timedwait failed:ret %d, %s", ret,strerror(ret));
#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
			PrintThreadKernelStack(tid);
#endif
			g_ucontext = 0; //in order to exit in below "if(ucontext == nullptr)"
		}
		ucontext = reinterpret_cast<void*>(g_ucontext.load());
		pthread_mutex_unlock(&g_wait_mutex);

#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
		clock_gettime(CLOCK_MONOTONIC, &t[5]);
#endif
		if(ucontext == nullptr){
			ALOGE("context == nullptr, then break\n");
			sigaction(THREAD_SIGNAL, &oldact, nullptr);
			g_finish = true;
			pthread_mutex_unlock(&g_sigaction_mutex);
#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
			ALOGE("tid:%d, time:%lu.%ld-%lu.%ld-%lu.%ld-%lu.%ld-%lu.%ld", tid,
				t[0].tv_sec, t[0].tv_nsec, t[1].tv_sec, t[1].tv_nsec,
				t[2].tv_sec, t[2].tv_nsec, t[3].tv_sec, t[3].tv_nsec,
				t[4].tv_sec, t[4].tv_nsec, t[5].tv_sec, t[5].tv_nsec);
#endif
			return false;
		}
		LocalMaps maps;  ///   proc/pid/maps"
		if(maps.Parse()==false) {
			ALOGE("UnwindCurThreadBT,parse maps fail\n");
			sigaction(THREAD_SIGNAL, &oldact, nullptr);
			g_finish = true;
			pthread_mutex_unlock(&g_sigaction_mutex);
#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
			ALOGE("tid:%d, time:%lu.%ld-%lu.%ld-%lu.%ld-%lu.%ld-%lu.%ld", tid,
				t[0].tv_sec, t[0].tv_nsec, t[1].tv_sec, t[1].tv_nsec,
				t[2].tv_sec, t[2].tv_nsec, t[3].tv_sec, t[3].tv_nsec,
				t[4].tv_sec, t[4].tv_nsec, t[5].tv_sec, t[5].tv_nsec);
#endif
			return false;
		}

#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
		clock_gettime(CLOCK_MONOTONIC, &t[6]);
#endif
		std::unique_ptr<Regs> regs(Regs::CreateFromUcontext(Regs::CurrentArch(), ucontext));
		// VerifyUnwind(getpid(), &maps, regs.get(), kFunctionOrder);
		auto process_memory(Memory::CreateProcessMemory(getpid()));

		Unwinder unwinder(512, &maps, regs.get(), process_memory);
		unwinder.Unwind();

#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
		clock_gettime(CLOCK_MONOTONIC, &t[7]);
#endif
		// Print the frames.
		*str += android::base::StringPrintf(" pid(%d) tid(%d) ", pid,tid);
		*str += getThreadName(tid) + "\n";

		//ALOGV("unwind pid(%d),tid(%d)\n",pid,tid);
		for (size_t i = 0; i < unwinder.NumFrames(); i++) {
			*str += unwinder.FormatFrame(i) + "\n";
		}
	}
	sigaction(THREAD_SIGNAL, &oldact, nullptr);
	g_finish = true;
	pthread_mutex_unlock(&g_sigaction_mutex);
	//ALOGV("unwind tid(%d),done\n",tid);

#ifdef DEBUG_LIBLADDER_BACKTRACE_TIMEOUT
	ALOGE("tid:%d, time:%lu.%ld-%lu.%ld-%lu.%ld-%lu.%ld-%lu.%ld-%lu.%ld-%lu.%ld-%lu.%ld", tid,
		t[0].tv_sec, t[0].tv_nsec, t[1].tv_sec, t[1].tv_nsec,
		t[2].tv_sec, t[2].tv_nsec, t[3].tv_sec, t[3].tv_nsec,
		t[4].tv_sec, t[4].tv_nsec, t[5].tv_sec, t[5].tv_nsec,
		t[6].tv_sec, t[6].tv_nsec, t[7].tv_sec, t[7].tv_nsec);
#endif
	return true;
}

extern "C" bool UnwindCurProcessBT(std::string *strBacktrace);

bool UnwindCurProcessBT(std::string *strBacktrace) {
	bool ret=false;
	pid_t pid=getpid();
	pid_t tid=gettid();
	std::vector<pid_t> threads;
	GetThreads(pid, &threads);
	strBacktrace->clear();

	/*lock successfully when pthread_mutex_trylock return 0.*/
	if (pthread_mutex_trylock(&g_process_unwind_mutex))
		return false;
	for (std::vector<int>::const_iterator it = threads.begin(); it != threads.end(); ++it)
	{
		// Skip the current forked process, we only care about the threads.
		if (tid == *it) {
			//ALOGE("unwind the current thread tid(%x)\n",tid);

			LocalMaps maps;  ///   proc/pid/maps"
			if(maps.Parse()==false) {
				ALOGE("UnwindCurThreadBT,parse maps fail\n");
				pthread_mutex_unlock(&g_process_unwind_mutex);
				return false;
			}

			std::unique_ptr<Regs> regs(Regs::CreateFromLocal());
			RegsGetLocal(regs.get());

			auto process_memory(Memory::CreateProcessMemory(getpid()));
			Unwinder unwinder(512, &maps, regs.get(), process_memory);
			unwinder.Unwind();

			// Print the frames.
			*strBacktrace += android::base::StringPrintf(" pid(%d) tid(%d) ", pid,tid);
			*strBacktrace += getThreadName(tid) + "\n";
			for (size_t i = 0; i < unwinder.NumFrames(); i++) {
				*strBacktrace += unwinder.FormatFrame(i) + "\n";
			}
			continue;
		}
		ret=UnwindThread(*it,strBacktrace);
	}
	ALOGI("UnwindCurProcessBT +++\n");
	pthread_mutex_unlock(&g_process_unwind_mutex);
	return ret;
}

extern "C" bool UnwindCurThreadBT(std::string *strBacktrace);
bool UnwindCurThreadBT(std::string *strBacktrace) {
	LocalMaps maps;  ///   proc/pid/maps"
	if(maps.Parse()==false) {
		ALOGE("UnwindCurThreadBT,parse maps fail\n");
		return false;
	}

	std::unique_ptr<Regs> regs(Regs::CreateFromLocal());
	RegsGetLocal(regs.get());

	auto process_memory(Memory::CreateProcessMemory(getpid()));
	Unwinder unwinder(512, &maps, regs.get(), process_memory);
	unwinder.Unwind();

	// Print the frames.
	pid_t pid=getpid();
	pid_t tid=gettid();
	strBacktrace->clear();
	ALOGI("unwind the current thread tid(%d)\n",tid);
	*strBacktrace += android::base::StringPrintf(" pid(%d) tid(%d) ", pid,tid);
	*strBacktrace += getThreadName(tid) + "\n";
	for (size_t i = 0; i < unwinder.NumFrames(); i++) {
		*strBacktrace += unwinder.FormatFrame(i) + "\n";
	}
	return true;
}


extern "C" bool UnwindThreadBT(pid_t tid,std::string *strBacktrace);
bool UnwindThreadBT(pid_t tid,std::string *strBacktrace) {
	pid_t pid=getpid();
	pid_t Curtid=gettid();
	strBacktrace->clear();
	bool ret=false;
	if(tid == Curtid)
		ret = UnwindCurThreadBT(strBacktrace);
	else {
		std::vector<pid_t> threads;
		GetThreads(pid, &threads);
		for (std::vector<int>::const_iterator it = threads.begin(); it != threads.end(); ++it)
		{
			if (tid == *it) {
				/*lock successfully when pthread_mutex_trylock return 0.*/
				if (pthread_mutex_trylock(&g_process_unwind_mutex))
					return false;
				ret = UnwindThread(*it,strBacktrace);
				pthread_mutex_unlock(&g_process_unwind_mutex);
			}
		}
	}
	return ret;
}


extern "C" bool UnwindCurProcessBT_Vector(std::vector<std::string> *strBacktrace);

bool UnwindCurProcessBT_Vector(std::vector<std::string> *strBacktrace) {
	bool ret=false;
	pid_t pid=getpid();
	pid_t tid=gettid();
	std::vector<pid_t> threads;
	GetThreads(pid, &threads);
	strBacktrace->clear();

	/*lock successfully when pthread_mutex_trylock return 0.*/
	if (pthread_mutex_trylock(&g_process_unwind_mutex))
		return false;
	for (std::vector<int>::const_iterator it = threads.begin(); it != threads.end(); ++it)
	{
		// Skip the current forked process, we only care about the threads.
		if (tid == *it) {
			//ALOGE("unwind the current thread tid(%x)\n",tid);

			LocalMaps maps;  ///   proc/pid/maps"
			if(maps.Parse()==false) {
				ALOGE("UnwindCurProcessBT_Vector,parse maps fail\n");
				pthread_mutex_unlock(&g_process_unwind_mutex);
				return false;
			}

			std::unique_ptr<Regs> regs(Regs::CreateFromLocal());
			RegsGetLocal(regs.get());

			auto process_memory(Memory::CreateProcessMemory(getpid()));
			Unwinder unwinder(512, &maps, regs.get(), process_memory);
			unwinder.Unwind();

			// Print the frames.
			std::string strThreadBacktrace;
			strThreadBacktrace += android::base::StringPrintf(" pid(%d) tid(%d) ", pid,tid);
			strThreadBacktrace += getThreadName(tid) + "\n";
			for (size_t i = 0; i < unwinder.NumFrames(); i++) {
				strThreadBacktrace += unwinder.FormatFrame(i) + "\n";
			}
			(*strBacktrace).push_back(strThreadBacktrace);
			continue;
		}
		else
		{
			std::string strThreadBacktrace;
			if(UnwindThread(*it,&strThreadBacktrace))
				(*strBacktrace).push_back(strThreadBacktrace);
		}
	}
	ALOGI("UnwindCurProcessBT_Vector +++\n");
	pthread_mutex_unlock(&g_process_unwind_mutex);
	return ret;
}


}
