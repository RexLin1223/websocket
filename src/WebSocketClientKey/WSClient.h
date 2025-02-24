#pragma once
#include <WebSocketLibrary/WSClientSession.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

/*
			Session flow

	  Client			  Server
		|					|
		|	    Open		|
		|------------------>|
		|    Send Key		|
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

namespace websocket {
	class WSClient : public std::enable_shared_from_this<WSClient> {
		boost::asio::io_context io_context_;

		bool is_connected_;
		std::string Key_;

		std::shared_ptr<client_tcp_session> session_;

		std::string host_;
		unsigned short port_;

	public:
		explicit WSClient(const std::string& host, unsigned short port)
			: is_connected_(false)
			, host_(host)
			, port_(port)
		{
		}

		virtual ~WSClient() {
			disconnect();
		}

		void connect() {
			session_ =  std::make_shared<client_tcp_session>(
				io_context_, host_, boost::lexical_cast<std::string>(port_));

			if (!session_) {
				return;
			}
			session_->run(std::bind(
				&WSClient::on_handshake_completed,
				this, std::placeholders::_1));
		}

		void disconnect() {
			io_context_.stop();

			session_->shutdown();
			session_.reset();
		}

		void set_Key(const std::string& Key) {
			Key_ = Key;
		}
	private:
		void on_handshake_completed(std::shared_ptr<tcp_session> session) {
			session->send(get_Key_message(),
				std::bind(&WSClient::on_send_Key, this,
					std::placeholders::_1, 
					std::placeholders::_2,
					std::placeholders::_3));
		}

		void on_send_Key(
			boost::beast::error_code ec, 
			std::size_t bytes_transferred,
			std::shared_ptr<tcp_session> session) {
			if (ec) {
				exception_log("send Key", ec);
				disconnect();
				return;
			}
		}

		void on_Key_response(boost::beast::error_code ec,
			std::size_t bytes_transferred,
			std::string&& received_data,
			std::shared_ptr<tcp_session> session) {
			// Parse send Key result
			boost::property_tree::ptree tree;
			std::stringstream ss(received_data);
			boost::property_tree::json_parser::read_json(ss, tree);

			try {
				size_t status_code = tree.get<size_t>("status_code");
				std::string message = tree.get<std::string>("message");

				is_connected_ = status_code == 200 ? true : false;
				if (!is_connected_) {
					disconnect();
				}
			}
			catch (...) {
				log("parse Key response fail");
			}
		}

		std::string get_Key_message() {
			boost::property_tree::ptree tree;
			tree.add("Key", Key_);

			std::stringstream ss;
			boost::property_tree::json_parser::write_json(ss, tree);

			return ss.str();
		}
		
	};
}