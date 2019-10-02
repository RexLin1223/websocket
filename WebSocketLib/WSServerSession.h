#pragma once
#include "WSSession.h"

namespace websocket {
	class WSServerSession : public WSSession {

	public:
		explicit WSServerSession(
			boost::asio::ip::tcp::socket&& socket, 
			boost::asio::io_context& ioc)
			: WSSession(std::move(socket), ioc)
		{
		}

		template<typename T>
		void run(const OnHandshakeCompleted<T>& handler) {
			// Set suggested timeout settings for the websocket
			ws_.set_option(
				boost::beast::websocket::stream_base::timeout::suggested(
					boost::beast::role_type::server));

			// Set a decorator to change the Server of the handshake
			ws_.set_option(boost::beast::websocket::stream_base::decorator(
				[](boost::beast::websocket::response_type& res) {
				res.set(boost::beast::http::field::server,
					std::string(BOOST_BEAST_VERSION_STRING) +
					" websocket-server-async");
			}));

			// Accept the websocket handshake
			ws_.async_accept(
				[this, self = shared_from_this(),
				done_handler = std::move(handler)]
			(boost::beast::error_code ec) {
				if (ec) {
					exception_log("session accept", ec);
					return;
				}

				do_timer_work(std::bind(
					&WSServerSession::check_alive, this));

				done_handler(self);
			});
		}

	private:

		bool check_alive() {
			if (GetNowEpoch() - last_message_ >= 15) {
				shutdown("time out");
				return false;
			}
			return true;
		}
	};
}