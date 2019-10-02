#pragma once
#include "WSUtility.h"
#include "WSDefinition.h"
#include "WSServerSession.h"

namespace websocket {
	class WSListener : public std::enable_shared_from_this<WSListener>
	{
		boost::asio::io_context& ioc_;
		boost::asio::ip::tcp::endpoint endpoint_;
		std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
		std::shared_ptr<Channel> channel_;

		OnConnectionCompleted<WSSession> accepted_handler_;
	public:
		WSListener(
			boost::asio::io_context& ioc,
			boost::asio::ip::tcp::endpoint endpoint)
			: ioc_(ioc)
			, endpoint_(endpoint)
			, acceptor_(new boost::asio::ip::tcp::acceptor(ioc))
		{
		}

		virtual ~WSListener() {
			if (acceptor_) {
				boost::system::error_code ec;
				acceptor_->close(ec);
				if (ec) {
					exception_log("acceptor close", ec);
				}
			}

			acceptor_.reset();
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

		void set_channel(std::shared_ptr<Channel> channel) {
			channel_ = channel;
		}

		void set_handshake_completed_handler(OnConnectionCompleted<WSSession>&& handler) {
			accepted_handler_ = std::move(handler);
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
			// The new connection gets its own strand
			acceptor_->async_accept(
				boost::asio::make_strand(ioc_),
				boost::beast::bind_front_handler(
					&WSListener::on_accept,
					shared_from_this()));
		}

		void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
		{
			if (ec) {
				exception_log("on_accept", ec);
			}
			else {
				log("Client connected!");
				// Create the session and run it
				auto session = std::make_shared<WSServerSession>(std::move(socket), ioc_);
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
}