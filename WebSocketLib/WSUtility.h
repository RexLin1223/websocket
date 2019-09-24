#pragma once
#include <chrono>
#include <string>
#include <iostream>
#include <stdarg.h>

namespace websocket {
	namespace beast = boost::beast;

	std::string GetNow() {

		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::string s(24, '\0');
		std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

		return s;
	}

	// Report a failure
	void
		fail(beast::error_code ec, char const* what)
	{
		std::cerr << GetNow() << "  " << what << ": " << ec.message() << "\n";
	}

	void custom_fail(char const* what) {
		std::cerr << GetNow() << "  " << what << "\n";
	}

	void log(const std::string& content) {
		std::cout << GetNow() << content << std::endl;
	}

	template<typename T, typename... Targs>
	void tprintf(const char* format, T value, Targs... Fargs)
	{
		for (; *format != '\0'; format++) {
			if (*format == '%') {
				std::cout << value;
				tprintf(format + 1, Fargs...);
				return;
			}
			std::cout << *format;
		}
	}
}