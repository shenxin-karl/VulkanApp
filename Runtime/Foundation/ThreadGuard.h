#pragma once
#include <source_location>
#include <string_view>
#include <thread>

class ThreadGuard {
public:
	static bool IsMainThread();
	static void EnsureMainThread(std::string_view message = {}, std::source_location sl = std::source_location::current());
	static auto GetMainThreadId() -> std::thread::id;
private:
	static std::thread::id sMainThreadId;
};