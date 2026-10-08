#include "ScopeTimer.hpp"

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")
struct ProcessMemoryInfo {
	SIZE_T resident; // 實體記憶體 (RSS / WorkingSetSize)
	SIZE_T virtualMem; // 虛擬記憶體 (VmSize / PagefileUsage)
};

ProcessMemoryInfo GetProcessMemory() {
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		return {pmc.WorkingSetSize, pmc.PagefileUsage};
	}
	return {0, 0};
}
#endif // _WIN32
#if defined(__linux__)
struct ProcessMemoryInfo {
	long long resident; // 實體記憶體 (RSS / WorkingSetSize)
	long long virtualMem; // 虛擬記憶體 (VmSize / PagefileUsage)
};

ProcessMemoryInfo GetProcessMemory() {
	long long rss = 0, vmsize = 0;
	auto read_mem_info = [](auto &line) {
		std::istringstream iss(line);
		std::string key;
		long long kb;
		iss >> key >> kb;
		return kb * 1024;
	};
	std::ifstream status("/proc/self/status");
	std::string line;
	while (std::getline(status, line)) {
		if (line.find("VmRSS:") == 0)
			rss = read_mem_info(line);
		else if (line.find("VmSize:") == 0)
			vmsize = read_mem_info(line);
	}
	return {rss, vmsize};
}
#endif

#if defined(__APPLE__)
#include <mach/mach.h>
struct ProcessMemoryInfo {
	long long resident; // 實體記憶體 (RSS / resident_size)
	long long virtualMem; // 虛擬記憶體 (VmSize / virtual_size)
};

ProcessMemoryInfo GetProcessMemory() {
	mach_task_basic_info_data_t info;
	mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
	auto result = task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
							reinterpret_cast<task_info_t>(&info), &count);
	if (result == KERN_SUCCESS) {
		return {static_cast<long long>(info.resident_size),
				static_cast<long long>(info.virtual_size)};
	}
	return {0, 0};
}
#endif // __APPLE__

double get_cpu_time() {
#ifdef _WIN32
	FILETIME a, b, kernel, user;
	if (GetProcessTimes(GetCurrentProcess(), &a, &b, &kernel, &user)) {
		ULARGE_INTEGER k, u;
		k.LowPart = kernel.dwLowDateTime;
		k.HighPart = kernel.dwHighDateTime;
		u.LowPart = user.dwLowDateTime;
		u.HighPart = user.dwHighDateTime;
		return (k.QuadPart + u.QuadPart) * 1e-7;
	}
	return 0;
#else
	struct timespec ts;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts); // 非 C++ 標準
	return ts.tv_sec + ts.tv_nsec * 1e-9;
#endif
}

namespace ScopeTimerPolicy {

GlobalStack &GlobalStack::instance() {
	static GlobalStack instance;
	return instance;
}

std::string GlobalStack::to_string() const {
	std::string ans = {};
	for (size_t i = 0; i < stk.size(); ++i) {
		if (i != 0)
			ans += ':';
		ans += '[' + stk[i] + ']';
	}
	return ans;
}

} // namespace ScopeTimerPolicy

struct ScopeTimer::Internal {
	struct Snapshot {
		std::chrono::steady_clock::time_point wall_time;
		double cpu_time;
		long long VmSize, VmRSS;
	};

	struct Increase {
		double wall_time, cpu_time;
		long long VmSize, VmRSS;
	};

	Snapshot start;

	static Snapshot get_snapshot();

	static Increase get_increase(Snapshot start, Snapshot end);

	static void log(const std::string &msg);

	explicit Internal(std::string &&name);
};

ScopeTimer::Internal::Internal(std::string &&name) : start(get_snapshot()) {
	ScopeTimerPolicy::GlobalStack::instance().push(std::move(name));
	log("ScopeTimer " + ScopeTimerPolicy::GlobalStack::instance().to_string() +
		" start");
}

inline ScopeTimer::Internal::Snapshot ScopeTimer::Internal::get_snapshot() {
	auto mem_info = GetProcessMemory();
	return {std::chrono::steady_clock::now(), get_cpu_time(),
			static_cast<long long>(mem_info.virtualMem),
			static_cast<long long>(mem_info.resident)};
}

inline ScopeTimer::Internal::Increase
ScopeTimer::Internal::get_increase(Snapshot start, Snapshot end) {
	return {
		std::chrono::duration<double>(end.wall_time - start.wall_time).count(),
		end.cpu_time - start.cpu_time, end.VmSize - start.VmSize,
		end.VmRSS - start.VmRSS};
}

inline void ScopeTimer::Internal::log(const std::string &msg) {
	std::cout << msg << std::endl;
}

ScopeTimer::ScopeTimer(std::string &&name)
	: internal(std::make_unique<Internal>(std::move(name))) {
}

ScopeTimer::~ScopeTimer() {
	auto end = Internal::get_snapshot();
	auto incr = Internal::get_increase(internal->start, end);
	auto sign = [](auto val) {
		return val < 0 ? '-' : (val > 0 ? '+' : '=');
	};
	auto to_Mb = [](auto val) {
		return val / 1024 / 1024;
	};
	std::string msg = std::format(
		"ScopeTimer {} end - Elapsed time {:.3f}({:.3f}c) sec, VmSize "
		"{}({}{}) "
		"Mb, VmRSS {}({}{}) Mb",
		ScopeTimerPolicy::GlobalStack::instance().to_string(), incr.wall_time,
		incr.cpu_time, to_Mb(end.VmSize), sign(incr.VmSize), to_Mb(incr.VmSize),
		to_Mb(end.VmRSS), sign(incr.VmRSS), to_Mb(incr.VmRSS));
	Internal::log(msg);
	ScopeTimerPolicy::GlobalStack::instance().pop();
}