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
	void log(const char* format, T value, Targs... Fargs)
	{
		//  Call stack:
		// 	log("% world% %\n", "Hello", '!', 123);
		//  Output -> "Hello"
		// 	log(" world% %\n", '!', 123);
		//	Output -> "Hello world" -> "Hello world!"
		// 	log(" %\n", 123);
		//	Output -> "Hello world! " -> "Hello world! 123"
		// 	log(" \n");
		//	Output -> "Hello world! " -> "Hello world! 123 \n"
		for (; *format != '\0'; format++) {
			if (*format == '%') {
				std::cout << value;
				log(format + 1, Fargs...);
				return;
			}
			std::cout << *format;
		}
	}
}