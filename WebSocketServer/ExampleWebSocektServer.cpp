//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket server, asynchronous
//
//------------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#define BOOST_SPIRIT_THREADSAFE

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <boost/date_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <sstream>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

static std::string GetNow() {

	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string s(24, '\0');
	std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

	return s;
}

static uint64_t GetNowEpoch() {
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

template<typename T>
void custom_fail(char const* what, T& err) {
	std::cerr << GetNow() << what << ": " << err.message() << "\n";
}

// Report a failure
void beast_fail(beast::error_code ec, char const* what)
{
	custom_fail(what, ec);
	//std::cerr << GetNow() << what << ": " << ec.message() << "\n";
}

void custom_log(const char* format) // base function
{
	std::cout << format;
}

template<typename T, typename... TArgs>
void custom_log(const char* fmt, T value, TArgs... args) {
	while (*fmt != '\0') {
		if (*fmt == '%') {
			std::cout << value;
			custom_log(fmt + 2, args...);
			return; 
		}
		std::cout << *fmt;
		++fmt;
	}
}

void log(char const* content) {
	std::cout << GetNow() << content << std::endl;
}

void fail(char const* content) {
	std::cout << GetNow();
	custom_log("Error: %s\n", content);
}



// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session>
{
	/*
	Transaction session flow
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
	websocket::stream<beast::tcp_stream> ws_;
	beast::flat_buffer read_buffer_;
	std::string write_data_;

	boost::asio::io_context& io_cotext_;
	boost::asio::steady_timer timer_;

	bool is_authenticated_;

	bool is_connected_;
	uint64_t lastAliveMsg_;

public:
	// Take ownership of the socket
	explicit
		session(tcp::socket&& socket, boost::asio::io_context& ioc)
		: ws_(std::move(socket))
		, io_cotext_(ioc)
		, timer_(ioc)
		, is_authenticated_(false)
		, lastAliveMsg_(GetNowEpoch())
	{
	}

	~session() {
		timer_.cancel();
		log("Connection closed");
	}

	// Start the asynchronous operation
	void run()
	{
		// Set suggested timeout settings for the websocket
		ws_.set_option(
			websocket::stream_base::timeout::suggested(
				beast::role_type::server));

		// Set a decorator to change the Server of the handshake
		ws_.set_option(websocket::stream_base::decorator(
			[](websocket::response_type& res)
		{
			res.set(http::field::server,
				std::string(BOOST_BEAST_VERSION_STRING) +
				" websocket-server-async");
		}));

		// Accept the websocket handshake
		ws_.async_accept(
			beast::bind_front_handler(
				&session::on_accept,
				shared_from_this()));

	}

	void on_accept(beast::error_code ec)
	{
		if (ec)
			return beast_fail(ec, "accept");

		alive_check();

		// Read first token message
		do_read_token();
	}

	void do_read()
	{
		// Read a message into our buffer
		ws_.async_read(
			read_buffer_,
			beast::bind_front_handler(
				&session::on_read,
				shared_from_this()));
	}

	void do_read_token() {
		ws_.async_read(
			read_buffer_,
			beast::bind_front_handler(
				&session::on_read_token,
				shared_from_this()));
	}

	void do_write() {
		ws_.text(true);
		ws_.async_write(
			boost::asio::buffer(write_data_),
			beast::bind_front_handler(
				&session::on_write,
				shared_from_this()));
	}

	void do_write_response(unsigned short status_code) {
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

		try {
			std::stringstream ss;
			boost::property_tree::json_parser::write_json(ss, tree);
			write_data_ = ss.str();

			ws_.text(true);
			ws_.async_write(
				boost::asio::buffer(write_data_),
				beast::bind_front_handler(
					&session::on_write_response,
					shared_from_this()));
		}
		catch (...) {
		}
	}

	void on_read(
			beast::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		// This indicates that the session was closed
		if (ec == websocket::error::closed)
			return;

		if (ec) {
			beast_fail(ec, "read");
		}

		std::string received_data = beast::buffers_to_string(read_buffer_.data());
		log(received_data.c_str());

		// Clear the buffer
		read_buffer_.consume(read_buffer_.size());

		std::stringstream ss(received_data);
		boost::property_tree::ptree tree;
		boost::property_tree::json_parser::read_json(ss, tree);
		try{
			// filter alive message
			uint64_t timestamp = tree.get<uint64_t>("timestamp");
			lastAliveMsg_ = timestamp;
		}
		catch (...) {
			// message 
			write_data_ = std::move(received_data);
		}

		// Echo the message
		do_write();
	}

	void on_write(
			beast::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
			return beast_fail(ec, "write");	

		// Do another read
		do_read();
	}

	void on_read_token(
			beast::error_code ec,
			std::size_t bytes_transferred) {
		try {
			// Check validation of token
			std::string data = beast::buffers_to_string(read_buffer_.data());

			std::stringstream ss(data);
			boost::property_tree::ptree pTree;
			boost::property_tree::json_parser::read_json(ss, pTree);

			std::string token = pTree.get<std::string>("token");
			if (0 != token.compare(R"(C124654DE1458)")) {
				fail("Invalid token");
				do_write_response(401);
			}
			else {
				do_write_response(200);
				is_authenticated_ = true;
			}
		}
		catch (boost::property_tree::json_parser::json_parser_error& ec) {
			custom_fail("Invalid token format", ec);
			do_write_response(400);
		}
		// Clear the buffer
		read_buffer_.consume(read_buffer_.size());
	}

	void on_write_response(
			beast::error_code ec,
			std::size_t bytes_transferred) {
		if (is_authenticated_) {
			do_read();
		}
	}

	void alive_check() {
		timer_.expires_from_now(std::chrono::seconds(10));
		timer_.async_wait([this](boost::system::error_code ec) {
			if (ec) {
				custom_fail("alive check", ec);
				return;
			}

			if (GetNowEpoch() - lastAliveMsg_ >= 15) {
				ws_.close(beast::websocket::close_reason("time out"), ec);
				if (ec) {
					custom_fail("close web socket", ec);
				}
				return;
			}

			alive_check();
		});
	}
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
	net::io_context& ioc_;
	tcp::acceptor acceptor_;

public:
	listener(
		net::io_context& ioc,
		tcp::endpoint endpoint)
		: ioc_(ioc)
		, acceptor_(ioc)
	{
		beast::error_code ec;

		// Open the acceptor
		acceptor_.open(endpoint.protocol(), ec);
		if (ec)
		{
			beast_fail(ec, "open");
			return;
		}

		// Allow address reuse
		acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec)
		{
			beast_fail(ec, "set_option");
			return;
		}

		// Bind to the server address
		acceptor_.bind(endpoint, ec);
		if (ec)
		{
			beast_fail(ec, "bind");
			return;
		}

		// Start listening for connections
		acceptor_.listen(
			net::socket_base::max_listen_connections, ec);
		if (ec)
		{
			beast_fail(ec, "listen");
			return;
		}
	}

	// Start accepting incoming connections
	void
		run()
	{
		do_accept();
	}

private:
	void
		do_accept()
	{
		// The new connection gets its own strand
		acceptor_.async_accept(
			net::make_strand(ioc_),
			beast::bind_front_handler(
				&listener::on_accept,
				shared_from_this()));
	}

	void
		on_accept(beast::error_code ec, tcp::socket socket)
	{
		if (ec)
		{
			beast_fail(ec, "accept");
		}
		else
		{
			log("Client connected!");
			// Create the session and run it
			std::make_shared<session>(std::move(socket), ioc_)->run();
		}

		// Accept another connection
		do_accept();
	}
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	// Check command line arguments.
	if (argc != 4)
	{
		std::cerr <<
			"Usage: websocket-server-async <address> <port> <threads>\n" <<
			"Example:\n" <<
			"    websocket-server-async 0.0.0.0 8080 1\n";
		return EXIT_FAILURE;
	}

	try {
		auto const address = net::ip::make_address(argv[1]);
		auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
		auto const threads = std::max<int>(1, std::atoi(argv[3]));
	
		printf("Server started!!\nlisten address=%s, port=%s\n", argv[1], argv[2]);

		// The io_context is required for all I/O
		net::io_context ioc{ threads };

		// Create and launch a listening port
		std::make_shared<listener>(ioc, tcp::endpoint{ address, port })->run();

		// Run the I/O service on the requested number of threads
		std::vector<std::thread> v;
		v.reserve(threads - 1);
		for (auto i = threads - 1; i > 0; --i)
			v.emplace_back(
				[&ioc]
		{
			ioc.run();
		});
		ioc.run();

		return EXIT_SUCCESS;
	}
	catch (...) {
		std::cerr <<
			"Usage: websocket-server-async <address> <port> <threads>\n" <<
			"Example:\n" <<
			"    websocket-server-async 0.0.0.0 8080 1\n";
		return EXIT_FAILURE;
	}
}