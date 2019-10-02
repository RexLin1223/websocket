#pragma once
#include "WSUtility.h"
#include "WSDefinition.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace websocket {
	class WSSession : public std::enable_shared_from_this<WSSession> {
	protected:
		boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;

		boost::beast::flat_buffer read_buffer_;
		LockQueue<std::string> write_queue_;

		// Channel using for broadcast read message
		std::shared_ptr<Channel> channel_;
		boost::asio::io_context& io_context_;
		boost::asio::steady_timer timer_;
	
		uint64_t last_message_;

	public:
		// Assign a accepted socket for listen
		explicit WSSession(boost::asio::ip::tcp::socket&& socket,
			boost::asio::io_context& ioc)
			: ws_(std::move(socket))
			, io_context_(ioc)
			, timer_(ioc)
			, last_message_(GetNowEpoch())
		{
		}

		// Create a socket for connect
		explicit WSSession(boost::asio::io_context& ioc)
			: ws_(boost::asio::make_strand(ioc))
			, io_context_(ioc)
			, timer_(ioc)
			, last_message_(GetNowEpoch())
		{
		}

		~WSSession() {
			timer_.cancel();
			log("Connection closed");
		}

		void shutdown() {
			io_context_.post([this] {
				boost::system::error_code ec;
				ws_.close(boost::beast::websocket::close_reason("manual shutdown"), ec);
				if (ec) {
					exception_log("shutdown", ec);
				}
			});
		}

		void shutdown(std::string&& reason) {
			io_context_.post([this, reason = std::move(reason)] {
				boost::system::error_code ec;
				ws_.close(boost::beast::websocket::close_reason(reason.c_str()), ec);
				if (ec) {
					exception_log("shutdown", ec);
				}
			});
		}

		void send(const std::string& data) {
			bool is_writing = !write_queue_.empty();
			write_queue_.push(data);

			if (!is_writing) {
				do_write();
			}
		}

		void send(std::string&& data) {
			bool is_writing = !write_queue_.empty();	
			write_queue_.push(std::move(data));
			
			if (!is_writing) {
				do_write();
			}
		}

		void receive() {
			do_read();
		}
		
		void set_channel(std::shared_ptr<Channel> channel) {
			channel_ = channel;
		}

		void send(std::string&& data, AsyncWriteHandler<WSSession>&& handler) {
			bool is_writing = !write_queue_.empty();
			write_queue_.push(std::move(data));

			if (!is_writing) {
				auto h = std::move(handler);
				do_write(std::move(h));
			}
		}

		void receive(AsyncReadHandler<WSSession>&& handler) {
			do_read(std::move(handler));
		}

	protected:
		void do_read() {
			ws_.async_read(
				read_buffer_,
				[self = shared_from_this()](boost::beast::error_code ec,
					std::size_t bytes_transferred) {
				self->on_read(ec, bytes_transferred);
			});
		}		

		void do_write() {
			ws_.text(true);
			ws_.async_write(
				boost::asio::buffer(write_queue_.peek()),
				[self = shared_from_this()](boost::beast::error_code ec,
					std::size_t bytes_transferred) {
				self->on_write(ec, bytes_transferred);
			});
		}

		void on_read(
			boost::beast::error_code ec,
			std::size_t bytes_transferred)
		{
			boost::ignore_unused(bytes_transferred);

			// This indicates that the session was closed
			if (ec == boost::beast::websocket::error::closed) {
				return;
			}

			if (ec) {
				exception_log("read", ec);
				return;
			}

			std::string received_data = boost::beast::buffers_to_string(read_buffer_.data());
			log(received_data.c_str());

			if (channel_) {
				channel_->broadcast(std::move(received_data));
			}

			// Clear the buffer
			read_buffer_.consume(read_buffer_.size());

			// Update message time
			last_message_ = GetNowEpoch();

			// read next
			auto self(shared_from_this());
			io_context_.post([self]{
				self->do_read();
			});
		}
		
		void on_write(
			boost::beast::error_code ec,
			std::size_t bytes_transferred)
		{
			boost::ignore_unused(bytes_transferred);

			if (ec) {
				exception_log("write", ec);
				return;
			}

			write_queue_.consume_one();

			auto self(shared_from_this());
			if (!write_queue_.empty()) {
				io_context_.post([self] {
					self->do_write();
				});
			}
		}

		//////////////////////////////////////////////////////////////////////////

		void do_read(AsyncReadHandler<WSSession>&& handler) {
			ws_.async_read(
				read_buffer_, 
				[self = shared_from_this(), read_handler = std::move(handler)]
				(boost::beast::error_code ec, 
					std::size_t bytes_transferred) mutable {
				self->on_read(ec, bytes_transferred, std::move(read_handler));
			});
		}

		void do_write(AsyncWriteHandler<WSSession>&& handler) {
			ws_.text(true);
			ws_.async_write(
				boost::asio::buffer(write_queue_.peek()),
				[self = shared_from_this(), write_handler = std::move(handler)]
				(boost::beast::error_code ec,
					std::size_t bytes_transferred) mutable {
					self->on_write(ec, bytes_transferred, std::move(write_handler));
			});
		}

		void on_read(
			boost::beast::error_code ec,
			std::size_t bytes_transferred, 
			AsyncReadHandler<WSSession>&& read_handler)
		{
			boost::ignore_unused(bytes_transferred);

			// This indicates that the session was closed
			if (ec == boost::beast::websocket::error::closed) {
				return;
			}

			if (ec) {
				exception_log("read", ec);
				return;
			}

			std::string received_data = boost::beast::buffers_to_string(read_buffer_.data());
			log(received_data.c_str());

			// Clear the buffer
			read_buffer_.consume(read_buffer_.size());

			if (read_handler) {
				read_handler(ec, bytes_transferred, std::move(received_data), shared_from_this());
			}
		}

		void on_write(
			boost::beast::error_code ec,
			std::size_t bytes_transferred,
			AsyncWriteHandler<WSSession>&& write_handler) {

			boost::ignore_unused(bytes_transferred);

			if (ec) {
				exception_log("write", ec);
				return;
			}

			write_queue_.consume_one();

			if (write_handler) {
				write_handler(ec, bytes_transferred, shared_from_this());
			}
		}

		template<typename T>
		void do_timer_work(T&& handler) {
			timer_.expires_from_now(std::chrono::seconds(5));
			timer_.async_wait(
				[this, next_handler = std::move(handler)]
				(boost::system::error_code ec) {
					if (ec) {
						return;
					}

					if (next_handler()) {
						do_timer_work(std::move(next_handler));
					}
				});
		}

	};

	
}