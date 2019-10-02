#pragma once
#include "WSListener.h"
#include <functional>

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
	class WSServer {
		std::shared_ptr<WSListener> listener_;
		std::unique_ptr<boost::asio::io_context> io_context_;
		boost::asio::ip::tcp::endpoint endpoint_;

		bool is_authenticated_;
		std::string token_;

		ThreadGroup thread_group_;
		std::unique_ptr<Channel> channel_;
	public:
		explicit WSServer(const std::string& address, unsigned short port)
			: is_authenticated_(false)
		{
			endpoint_.address(boost::asio::ip::make_address(address.c_str()));
			endpoint_.port(port);
		}
		virtual ~WSServer() {
			stop();
		}

		bool start(size_t thread_count) {
			thread_count = std::max<size_t>(1, thread_count);
			io_context_ = std::make_unique<boost::asio::io_context>(thread_count);
			if (!io_context_) {
				return false;
			}

			listener_ = std::make_shared<WSListener>(*io_context_, endpoint_);
			if (!listener_) {
				return false;
			}

			listener_->set_handshake_completed_handler(
				std::bind(&WSServer::on_client_join,
					this, std::placeholders::_1));

			// Start to listen
			listener_->run();
		
			// Enable work threads
			thread_group_.create_thread_count(
				[this]() { io_context_->run(); },
				thread_count);

			return true;
		}

		void stop() {
			io_context_->stop();
			thread_group_.join_all();
		}

		void set_token(const std::string& token) {
			token_ = token;
		}

	private:
		void on_client_join(std::shared_ptr<WSSession> session) {
			session->receive(std::move(
				boost::beast::bind_front_handler(
					&WSServer::on_read_token, this)));
		}

		void do_write_response(unsigned short status_code, std::shared_ptr<WSSession> session) {
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
				std::bind(&WSServer::on_write_response, this,
					std::placeholders::_1, std::placeholders::_2,
					std::placeholders::_3));
		}

		void on_read_token(
			boost::beast::error_code ec,
			std::size_t bytes_transferred, 
			std::string&& data,
			std::shared_ptr<WSSession> session) {

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
			std::shared_ptr<WSSession> session) {
			if (is_authenticated_) {
				session->receive();
			}
		}
	};
}