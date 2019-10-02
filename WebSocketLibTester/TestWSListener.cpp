#include <WebSocketLib/WSListener.h>
#include <gtest/gtest.h>
/*
class WSListenerDaemon : public testing::Test {
	boost::asio::io_context io_context_;
	boost::asio::ip::tcp::endpoint endpoint_;
	std::shared_ptr<websocket::WSListener> listener_;

public:
	void set_endpoint(const std::string& address, unsigned short port) {
		endpoint_.address(boost::asio::ip::make_address(address.c_str()));
		endpoint_.port(port);
	}

	bool run() {
		listener_.reset(
			new websocket::WSListener(
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
		io_context_.run();

		return true;
	}

private:
	void on_data(const std::string& data) {
		printf("Received data %s\n", data.c_str());
	}
};

TEST_F(WSListenerDaemon, TestRun) {

	try {
		set_endpoint("0.0.0.0", 8080);
		run();
	}
	catch (std::exception& ex) {
	}
}
*/