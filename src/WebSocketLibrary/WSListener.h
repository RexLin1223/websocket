#pragma once
#include "WSDefinition.h"
#include "WSServerSession.h"
#include "WSUtility.h"

namespace websocket {
	template<
		typename server_session_type,
		typename base_session_type>
	class Listener 
		: public std::enable_shared_from_this<Listener<server_session_type, base_session_type>>
	{
		boost::asio::io_context& ioc_;
		boost::asio::ip::tcp::endpoint endpoint_;
		std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;

		std::shared_ptr<Channel> channel_;

		OnConnectionCompleted<base_session_type> accepted_handler_;

		std::shared_ptr<boost::asio::ssl::context> ssl_context_;
	public:
		Listener(
			boost::asio::io_context& ioc,
			const boost::asio::ip::tcp::endpoint& endpoint)
			: ioc_(ioc)
			, endpoint_(endpoint)
			, acceptor_(new boost::asio::ip::tcp::acceptor(ioc))
			, ssl_context_(nullptr)
		{
		}

		~Listener() {
			stop();
			acceptor_.reset();
			channel_.reset();
		}

		// Start accepting incoming connections
		bool run()
		{
			if (!init()) {
				return false;
			}

			do_accept();			
			return true;
		}

		void stop() {
			if (acceptor_) {
				boost::system::error_code ec;
				acceptor_->cancel(ec);
				if (ec) {
					exception_log("acceptor close", ec);
					return;
				}
				acceptor_->close(ec);
				if (ec) {
					exception_log("acceptor close", ec);
					return;
				}
			}
		}

		void set_channel(std::shared_ptr<Channel> channel) {	
			channel_ = channel;
		}

		void set_handshake_completed_handler(OnConnectionCompleted<base_session_type>&& handler) {
			accepted_handler_ = std::move(handler);
		}

		void set_ssl_context(boost::asio::ssl::context&& context) {
			ssl_context_ = std::make_shared<boost::asio::ssl::context>(std::move(context));
		}

	private:
		bool init() {
			boost::beast::error_code ec;

			// Open the acceptor
			acceptor_->open(endpoint_.protocol(), ec);
			if (ec)
			{
				exception_log("open", ec);
				return false;
			}

			// Allow address reuse
			acceptor_->set_option(boost::asio::socket_base::reuse_address(true), ec);
			if (ec)
			{
				exception_log("set_option", ec);
				return false;
			}

			// Bind to the server address
			acceptor_->bind(endpoint_, ec);
			if (ec)
			{
				exception_log("bind", ec);
				return false;
			}

			// Start listening for connections
			acceptor_->listen(
				boost::asio::socket_base::max_listen_connections, ec);
			if (ec)
			{
				exception_log("listen", ec);
				return false;
			}
			return true;
		}
		
		void do_accept()
		{
			auto self = Listener<server_session_type, base_session_type>::shared_from_this();
			// The new connection gets its own strand
			acceptor_->async_accept(
				boost::asio::make_strand(ioc_),
				boost::beast::bind_front_handler(
					&Listener::on_accept,
					self));
		}

		void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
		{
			if (ec) {
				exception_log("on_accept", ec);
				return;
			}
			else {
				log_debug("Client connected!");
				// Create the session and run it
				auto session = std::make_shared<server_session_type>(std::move(socket), *ssl_context_, ioc_);
				if (!session) {
					return;
				}
				session->set_channel(channel_);
				session->run(accepted_handler_);
			}

			// Accept another connection
			do_accept();
		}

	};
		
	/*
	using ListenerTCP = Listener<server_tcp_session, tcp_session>;
	using ListenerSSL = Listener<server_ssl_session, ssl_session>;
	*/
}