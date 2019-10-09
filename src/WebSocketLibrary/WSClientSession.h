#pragma once
#include "WSSession.h"
#include <boost/lexical_cast.hpp>

namespace websocket {
	class WSClientSession : public WSSession {
		boost::asio::ip::tcp::resolver resolver_;
		std::string host_;
		std::string port_;

		OnConnectionCompleted<WSSession> connected_handler_;
	public: 
		explicit WSClientSession(
			boost::asio::io_context& ioc,
			const std::string& host,
			const std::string& port)
			: WSSession(ioc)
			, resolver_(boost::asio::make_strand(ioc))
			, host_(host)
			, port_(port)
		{

		}

		void run(const OnConnectionCompleted<WSSession>& handler) {
			connected_handler_ = std::move(handler);

			// Look up the domain name
			resolver_.async_resolve(host_, port_,
				std::bind(&WSClientSession::on_resolve, 
					this, std::placeholders::_1, std::placeholders::_2));
		}
	private:

		void on_resolve(boost::beast::error_code ec,
				boost::asio::ip::tcp::resolver::results_type results)
		{
			if (ec) {
				return exception_log("resolve", ec);
			}


			// Set the timeout for the operation
			boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

			// Make the connection on the IP address we get from a lookup
			boost::beast::get_lowest_layer(ws_).async_connect(
				results, std::bind(&WSClientSession::on_connect, this,
					std::placeholders::_1, std::placeholders::_2));
		}

		void on_connect(boost::beast::error_code ec, 
			boost::asio::ip::tcp::resolver::results_type::endpoint_type)
		{
			if (ec)
				return exception_log("connect", ec);

			// Turn off the timeout on the tcp_stream, because
			// the websocket stream has its own timeout system.
			boost::beast::get_lowest_layer(ws_).expires_never();

			// Set suggested timeout settings for the websocket
			ws_.set_option(
				boost::beast::websocket::stream_base::timeout::suggested(
					boost::beast::role_type::client));

			// Set a decorator to change the User-Agent of the handshake
			ws_.set_option(boost::beast::websocket::stream_base::decorator(
				[](boost::beast::websocket::request_type& req)
			{
				req.set(boost::beast::http::field::user_agent,
					std::string(BOOST_BEAST_VERSION_STRING) +
					" websocket-client-async");
			}));

			// Perform the websocket handshake
			ws_.async_handshake(host_, "/",
				std::bind(&WSClientSession::on_handshake, this,
					std::placeholders::_1));
		}

		void on_handshake(boost::beast::error_code ec)
		{
			if (ec)
				return exception_log("handshake", ec);

			do_send_alive();

			// Send the message
			if (connected_handler_) {
				connected_handler_(shared_from_this());
			}
		}

		void do_send_alive() {
			timer_.expires_from_now(std::chrono::seconds(5));
			timer_.async_wait([this](boost::system::error_code ec) {
				if (ec) {
					return;
				}

				send(boost::lexical_cast<std::string>(GetNowEpoch()));
				do_send_alive();
			});
		}
	};
}