#pragma once
#include "WSServerExport.h"
#include <WebSocketLibrary/WSListener.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace websocket {

	/*
			Session flow

	  Client			  Server
		|					|
		|	    Open		|
		|------------------>|
		|    Send Token		|
		|------------------>|
		|	Send Response	|
		|<------------------|
		|					|_______________
		| Start Send Data	|				|
		|------------------>|				|
		|	 Echo Data		|				|
		|<------------------|				|
		|					|  Transaction	|
		| Send Keep-alive	|				|
		|------------------>|				|
		|		...			|				|
		|		...			|				|
		|					|_______________|
		|	Close Session	|
		|<----------------->|
		|					|
	*/

	struct ssl_config {
		// method
		boost::asio::ssl::context_base::method method;

		// certificate
		std::string certificate_file_path;
		std::string private_key_file_path;
		boost::asio::ssl::context::file_format file_format;
	};

	template<typename server_session_type, typename base_session_type>
	class WSServerToken {
		std::shared_ptr<Listener<server_session_type, base_session_type>> listener_;
		std::unique_ptr<boost::asio::io_context> io_context_;
		boost::asio::ip::tcp::endpoint endpoint_;

		ThreadGroup thread_group_;
		
		std::unique_ptr<ssl_config> ssl_config_;
		std::shared_ptr<Channel> channel_;

		std::string token_;
		bool is_authenticated_;

		OnData on_data_;
		OnError on_error_;
		OnJoin on_join_;
		OnLeave on_leave_;
	public:
		explicit WSServerToken()
			: is_authenticated_(false)
			, on_data_(NULL)
			, on_error_(NULL)
			, on_join_(NULL)
			, on_leave_(NULL) {
		}
		virtual ~WSServerToken() {
			stop();
		}

		bool start(size_t thread_count) {
			thread_count = std::max<size_t>(1, thread_count);
			io_context_ = std::make_unique<boost::asio::io_context>(thread_count);
			if (!io_context_) {
				return false;
			}

			listener_ = std::make_shared<Listener<server_session_type, base_session_type>>(*io_context_, endpoint_);
			if (!listener_) {
				return false;
			}

			if (ssl_config_) {
				boost::asio::ssl::context ctx{ ssl_config_->method };
				ctx.use_certificate_file(ssl_config_->certificate_file_path, ssl_config_->file_format);
				ctx.use_rsa_private_key_file(ssl_config_->private_key_file_path, ssl_config_->file_format);
				listener_->set_ssl_context(std::move(ctx));
			}

			channel_ = std::make_shared<Channel>();
			if (channel_) {
				channel_->subscribe(std::bind(&WSServerToken::on_received_data, this, std::placeholders::_1));
				listener_->set_channel(channel_);
			}

			listener_->set_handshake_completed_handler(
				std::bind(&WSServerToken::on_client_join,
					this, std::placeholders::_1));

			// Start to listen
			if (!listener_->run()) return false;
			log("Server started!\tlisten port=%s!", endpoint_.port());
		
			// Enable work threads
			thread_group_.create_thread_count(
				[this]() { io_context_->run(); },
				thread_count);

			return true;
		}

		void stop() {
			if (io_context_) {
				io_context_->stop();
			}
			thread_group_.join_and_clear_all();
		}
		
		void set_listener(const char* address, unsigned short port) {
			endpoint_.address(boost::asio::ip::make_address(std::string(address)));
			endpoint_.port(port);
		}

		void set_token(const char* token) {
			token_ = token;
		}

		void set_ssl_config(int ssl_method, 
			const char* certificate_file_path,
			const char* private_key_file_path,
			int file_format) {
			ssl_config config{
				static_cast<boost::asio::ssl::context_base::method>(ssl_method),
				std::string(certificate_file_path),
				std::string(private_key_file_path),
				static_cast<boost::asio::ssl::context::file_format>(file_format)
			};

			ssl_config_ = std::make_unique<ssl_config>(std::move(config));
		}

		void register_on_data(OnData on_data) {
			on_data_ = on_data;
		}

		void register_on_error(OnError on_err) {
			on_error_ = on_err;
		}

		void register_on_join(OnJoin on_join) {
			on_join_ = on_join;
		}
		void register_on_leave(OnLeave on_leave) {
			on_leave_ = on_leave;
		}

	private:
		void on_client_join(std::shared_ptr<base_session_type> session) {
			session->receive(std::move(
				boost::beast::bind_front_handler(
					&WSServerToken::on_read_token, this)));
		}

		void do_write_response(unsigned short status_code, std::shared_ptr<base_session_type> session) {
			boost::property_tree::ptree tree;
			switch (status_code) {
			case 200:
				tree.add("status_code", "200");
				tree.add("message", "OK");
				break;
			case 401:
				tree.add("status_code", "401");
				tree.add("message", "Unauthorized");
				break;
			default:
				tree.add("status_code", "400");
				tree.add("message", "Bad Request");
			}

			std::stringstream ss;
			boost::property_tree::json_parser::write_json(ss, tree);

			session->send(std::move(ss.str()), 
				std::bind(&WSServerToken::on_write_response, this,
					std::placeholders::_1, std::placeholders::_2,
					std::placeholders::_3));
		}

		void on_read_token(
			boost::beast::error_code ec,
			std::size_t bytes_transferred, 
			std::string&& data,
			std::shared_ptr<base_session_type> session) {

			unsigned short status_code = 400;
			try {
				std::stringstream ss(data);
				boost::property_tree::ptree pTree;
				boost::property_tree::json_parser::read_json(ss, pTree);

				std::string token = pTree.get<std::string>("token");
				if (token != token_) {
					log("Invalid token");
					status_code = 401;
				}
				else {
					status_code = 200;
					is_authenticated_ = true;
				}
			}
			catch (boost::property_tree::json_parser::json_parser_error& ec) {
				exception_log("Invalid token format", ec);
			}
			catch (std::exception& ec) {
				log("Unknown exception in parse token JOSN, ec=%s", ec.what());
			}
			do_write_response(status_code, session);
		}

		void on_write_response(
			boost::beast::error_code ec,
			std::size_t bytes_transferred,
			std::shared_ptr<base_session_type> session) {
			if (is_authenticated_) {
				session->receive();
			}
		}

		void on_received_data(const std::string& data) {
			if (on_data_) {
				on_data_(data.c_str(), data.size());
			}
		}
	};

	using TokenServer = WSServerToken<websocket::server_tcp_session, websocket::tcp_session>;
	using TokenSSLServer = WSServerToken<websocket::server_ssl_session, websocket::ssl_session>;
}