#pragma once
#include <source_location>
#include <string>
#include <fmt/format.h>

struct FormatAndLocation {
	std::string_view fmt;
	std::source_location location;
public:
	template<size_t N>
	FormatAndLocation(const char(&arr)[N], std::source_location location = std::source_location::current())
	: fmt(arr), location(location) {

	}
	FormatAndLocation(const std::string_view &fmt, std::source_location location = std::source_location::current())
	: fmt(fmt), location(location) {

	}
};

class Exception : public std::exception {
public:
	Exception(const std::string &message, const std::source_location &sourceLocation);
	auto what() const noexcept -> const char* override;
	auto GetLine() const noexcept -> int;
	auto GetFile() const noexcept -> const char *;
	auto GetFunc() const noexcept -> const char *;
	auto GetMessage() const noexcept -> const std::string &;

	template<typename ...Args>
	static void Throw(const FormatAndLocation &fmtAndLocation, Args&&...args) {
		std::string message;
		if constexpr (sizeof...(Args)) {
			message = fmt::vformat(fmtAndLocation.fmt, fmt::make_format_args(args...));
		} else {
			message = fmtAndLocation.fmt.data();
		}
		ThrowException(Exception(std::move(message), fmtAndLocation.location));
	}
	template<typename ...Args>
	static void Throw(bool cond, const FormatAndLocation &fmtAndLocation, Args&&...args) {
		if (!cond) {
			Exception::Throw(fmtAndLocation, std::forward<Args>(args)...);
		}
	}
private:
	static void ThrowException(const Exception &exception) noexcept(false);
protected:
	int				_line;
	const char *	_file;
	const char *	_func;
	std::string		_message;
	std::string		_whatBuffer;
};

class NotImplementedException : public std::exception {
public:
	explicit NotImplementedException(const char *func);
	auto what() const noexcept -> const char *override;
	static void Throw(const std::source_location &sourceLocation = std::source_location::current()) {
		throw NotImplementedException(sourceLocation.function_name());
	}
private:
	const char		   *_func;
	mutable std::string _whatBuffer;
};
