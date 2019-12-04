#pragma once
#include "WSSession.h"

namespace websocket {
	class server_tcp_session : public tcp_session {
		using self_ptr = std::shared_ptr<tcp_session>;
	public:
		server_tcp_session() = delete;
		server_tcp_session& operator=(const server_tcp_session&) = delete;
		server_tcp_session(const server_tcp_session&) = delete;
		explicit server_tcp_session(
			boost::asio::ip::tcp::socket&& socket,
			boost::asio::ssl::context& ctx,
			boost::asio::io_context& ioc)
			: tcp_session(std::move(socket), ioc)
		{
		}

		void run(const OnConnectionCompleted<tcp_session>& handler) {
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
				handler = std::move(handler)]
			(boost::beast::error_code ec) {
				if (ec) {
					exception_log("session accept", ec);
					return;
				}

				do_timer_work(std::bind(
					&server_tcp_session::check_alive, this), 2);

				handler(self);
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
	
	class server_ssl_session : public ssl_session {
		using self_ptr = std::shared_ptr<ssl_session>;
	public:
		server_ssl_session() = delete;
		server_ssl_session& operator=(const server_ssl_session&) = delete;
		server_ssl_session(const server_ssl_session&) = delete;

		explicit server_ssl_session(
			boost::asio::ip::tcp::socket&& socket,
			boost::asio::ssl::context& ctx,
			boost::asio::io_context& ioc)
			: ssl_session(std::move(socket), ctx, ioc)
		{
		}

		virtual ~server_ssl_session() 
		{
		}

		void run(const OnConnectionCompleted<ssl_session>& handler) {
			// Set the timeout.
			boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

			// Perform the SSL handshake
			ws_.next_layer().async_handshake(
				boost::asio::ssl::stream_base::server,
				[this, self = shared_from_this(),
				h = std::move(handler)]
			(boost::beast::error_code ec) mutable {
				on_handshake(ec, std::move(h));
			});
		}
	
	private:
		void on_handshake(boost::beast::error_code ec, OnConnectionCompleted<ssl_session>&& handler) {
			if (ec)
				return exception_log("handshake", ec);

			// Turn off the timeout on the tcp_stream, because
			// the websocket stream has its own timeout system.
			boost::beast::get_lowest_layer(ws_).expires_never();

			// Set suggested timeout settings for the websocket
			ws_.set_option(
				boost::beast::websocket::stream_base::timeout::suggested(
					boost::beast::role_type::server));

			// Set a decorator to change the Server of the handshake
			ws_.set_option(boost::beast::websocket::stream_base::decorator(
				[](boost::beast::websocket::response_type& res) {
				res.set(boost::beast::http::field::server,
					std::string(BOOST_BEAST_VERSION_STRING) +
					" websocket-server-async-ssl");
			}));

			// Accept the websocket handshake
			ws_.async_accept(
				[this, self = shared_from_this(),
				h = std::move(handler)]
			(boost::beast::error_code ec) mutable {
				on_accept(ec, std::move(h));
			});
		}

		void on_accept(boost::beast::error_code ec, OnConnectionCompleted<ssl_session>&& handler) {
			if (ec) {
				exception_log("session accept", ec);
				return;
			}

			do_timer_work(std::bind(
				&server_ssl_session::check_alive, this), 5);

			handler(shared_from_this());
		}

		bool check_alive() {
			if (GetNowEpoch() - last_message_ >= 15) {
				shutdown("time out");
				return false;
			}
			return true;
		}
	};

	/*
	class ssl_context {
		boost::asio::ssl::context context_;
	public:
		ssl_context() = delete;
		explicit ssl_context(boost::asio::ssl::context_base::method method)
			: context_(method)
		{
		}

		self_ptr set_verity_mode(boost::asio::ssl::verify_mode& mode) {
			context_.set_verify_mode(mode);
		}

		
		/// Allow set multiple option into context like:
		///	ctx.set_options(
		///		boost::asio::ssl::context::default_workarounds |
		///		boost::asio::ssl::context::no_sslv2 |
		///		boost::asio::ssl::context::no_sslv3);		
		explicit self_ptr set_option(const boost::asio::ssl::context_base::options opntions) {
			ctx.set_options(opntions);
		}

		self_ptr set_private_key_file(const std::string& fileName, boost::asio::ssl::context::file_format format) {
			ctx.use_rsa_private_key_file(fileName, format);
		}

		self_ptr set_certificate_file(const std::string& fileName, boost::asio::ssl::context::file_format format) {
			ctx.use_certificate_file(fileName, format);
		}

		boost::asio::ssl::context& get_raw_context() {
			return context_;
		}
	};*/

}