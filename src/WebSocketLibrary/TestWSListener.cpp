#include "WSListener.h"
using TCPListener = websocket::Listener<websocket::server_tcp_session, websocket::tcp_session>;
using SSLListener = websocket::Listener<websocket::server_ssl_session, websocket::ssl_session>;

class WSListenerDaemon {
	boost::asio::io_context io_context_;
	boost::asio::ip::tcp::endpoint endpoint_;
	std::shared_ptr<TCPListener> listener_;

public:
	void set_endpoint(const std::string& address, unsigned short port) {
		endpoint_.address(boost::asio::ip::make_address(address.c_str()));
		endpoint_.port(port);
	}

	bool run() {
		listener_.reset(
			new TCPListener(
				io_context_, endpoint_));

		if (!listener_) {
			return false;
		}

		auto channel = std::make_shared<websocket::Channel>();
		channel->subscribe(
			std::bind(&WSListenerDaemon::on_data, 
				this, std::placeholders::_1));

		listener_->set_channel(channel);
		listener_->run();
		websocket::log("Server started!\tlisten port=%d!", endpoint_.port());

		io_context_.run();

		return true;
	}

private:
	void on_data(const std::string& data) {
		printf("Received data %s\n", data.c_str());
	}
};

int main() {
	WSListenerDaemon d;

}
