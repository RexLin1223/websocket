#pragma once
#include "WSDefinition.h"

namespace websocket {
	class WebSocektSession {
	public:
		WebSocektSession() {

		}

		~WebSocektSession() {

		}

		virtual void run() {

		}

		virtual void send() {
		}

	private:
		void do_read() {
		}

		void do_write() {
		}

		void on_read(beast::error_code ec,
			std::size_t bytes_transferred) {
		}

		void on_write(beast::error_code ec,
			std::size_t bytes_transferred) {
		}
	};
}