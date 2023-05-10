#include "ThreadGuard.h"
#include "Exception.h"

std::thread::id ThreadGuard::sMainThreadId = std::this_thread::get_id();

bool ThreadGuard::IsMainThread() {
	return std::this_thread::get_id() == sMainThreadId;
}

void ThreadGuard::EnsureMainThread(std::string_view message, std::source_location sl) {
	if (message.empty()) {
	    Exception::Throw(IsMainThread(), 
			"{}({},{}) EnsureMainThread failed!", 
			sl.file_name(), 
			sl.line(), 
			sl.column()
		);
	} else {
	    Exception::Throw(IsMainThread(), message);
	}
}

auto ThreadGuard::GetMainThreadId() -> std::thread::id {
	return sMainThreadId;
}
