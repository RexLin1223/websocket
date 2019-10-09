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

#include "server_certificate.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>

#include <boost/date_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
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
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

static std::string GetNowDate() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
	return ss.str();
}

static std::string GetNow() {

	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string s(24, '\0');
	std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

	return s;
}

static uint64_t GetNowEpoch() {
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void write_log(const std::string& log) {
	std::string fileName = GetNowDate() + ".log";
	std::ofstream ofs;
	ofs.open(fileName.c_str(), std::ios::binary| std::ios::app, _SH_DENYWR);
	if (ofs) {
		ofs.write(log.c_str(), log.size());
		ofs.close();
	}
}

template<typename T>
void custom_fail(char const* what, T& err) {
	std::stringstream ss;
	ss << GetNow() << what << ": " << err.message() << "\n";

	std::cerr << ss.str();
	write_log(ss.str());
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
	std::stringstream ss;
	ss << GetNow() << content << std::endl;

	std::cerr << ss.str();
	write_log(ss.str());
}

void fail(char const* content) {
	std::stringstream ss;
	ss << GetNow();
	ss << "Error:"<< content;
	ss << "\n";

	std::cerr << ss.str();
	write_log(ss.str());
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
	websocket::stream<
		beast::ssl_stream<beast::tcp_stream>> ws_;
	//websocket::stream<beast::tcp_stream> ws_;
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
		session(tcp::socket&& socket, boost::asio::io_context& ioc, ssl::context& ctx)
		: ws_(std::move(socket), ctx)
		, io_cotext_(ioc)
		, timer_(ioc)
		, is_authenticated_(false)
		, lastAliveMsg_(GetNowEpoch())
	{
	}

	~session() {
		timer_.cancel();

		boost::system::error_code ec;
		ws_.close(boost::beast::websocket::close_reason("manual shutdown"), ec);
		if (ec) {
			custom_fail("shutdown", ec);
		}
		log("Connection closed");
	}

	// Start the asynchronous operation
	void run()
	{
		// Set the timeout.
		beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

		// Perform the SSL handshake
		ws_.next_layer().async_handshake(
			ssl::stream_base::server,
			beast::bind_front_handler(
				&session::on_handshake,
				shared_from_this()));
	}

	void on_handshake(beast::error_code ec) {
		if (ec)
			return beast_fail(ec, "handshake");

		// Turn off the timeout on the tcp_stream, because
		// the websocket stream has its own timeout system.
		beast::get_lowest_layer(ws_).expires_never();

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
				" websocket-server-async-ssl");
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
			return;
		}

		std::string received_data = beast::buffers_to_string(read_buffer_.data());
		log(received_data.c_str());

		// Clear the buffer
		read_buffer_.consume(read_buffer_.size());	
		
		// message 
		write_data_ = std::move(received_data);

		// Echo the message
		do_write();

		// Update last message time
		lastAliveMsg_ = GetNowEpoch();
	}

	void on_write(
			beast::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec) {
			beast_fail(ec, "write");
			return;
		}

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
		catch (boost::property_tree::ptree_error& ec) {
			fail("Can't get token");
			do_write_response(401);
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
	ssl::context& ctx_;
	tcp::acceptor acceptor_;
	tcp::endpoint endpoint_;

public:
	listener(
		net::io_context& ioc,
		ssl::context& ctx,
		tcp::endpoint endpoint)
		: ioc_(ioc)
		, ctx_(ctx)
		, acceptor_(ioc)
		, endpoint_(endpoint)
	{
		
	}

	bool init() {
		beast::error_code ec;

		// Open the acceptor
		acceptor_.open(endpoint_.protocol(), ec);
		if (ec)
		{
			beast_fail(ec, "open");
			return false;
		}

		// Allow address reuse
		acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec)
		{
			beast_fail(ec, "set_option");
			return false;
		}

		// Bind to the server address
		acceptor_.bind(endpoint_, ec);
		if (ec)
		{
			beast_fail(ec, "bind");
			return false;
		}

		// Start listening for connections
		acceptor_.listen(
			net::socket_base::max_listen_connections, ec);
		if (ec)
		{
			beast_fail(ec, "listen");
			return false;
		}
		return true;
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
			std::make_shared<session>(std::move(socket), ioc_, ctx_)->run();
		}

		// Accept another connection
		do_accept();
	}
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	// Check command line arguments.
	/*if (argc != 4)
	{
		std::cerr <<
			"Usage: websocket-server-async <address> <port> <threads>\n" <<
			"Example:\n" <<
			"    websocket-server-async 0.0.0.0 8080 1\n";
		return EXIT_FAILURE;
	}*/

	try {
		// Read ini file
		std::ifstream ifs;
		ifs.open("config.ini", std::ios::binary | std::ios::in);
		if (!ifs) {
			std::cerr <<
				"Please ensure config.ini was placed at same directory with WebSocketServer.exe\n";
			return EXIT_FAILURE;
		}
		ifs.close();

		boost::property_tree::ptree tree;
		boost::property_tree::ini_parser::read_ini("config.ini", tree);

		std::string addressStr = tree.get<std::string>("Listen.Address");
		std::string portStr = tree.get<std::string>("Listen.Port");
		std::string threadsStr = tree.get<std::string>("Listen.Threads");
		auto const address = net::ip::make_address(addressStr.c_str());
		auto const port = static_cast<unsigned int>(std::atoi(portStr.c_str()));
		auto const threads = std::max<int>(1, std::atoi(threadsStr.c_str()));
	
		if (port < 0 || port >= 65535) {
			fail("Invalid listen port");
			return EXIT_FAILURE;
		}

		// The io_context is required for all I/O
		net::io_context ioc{ threads };

		// The SSL context is required, and holds certificates
		ssl::context ctx{ ssl::context::tlsv12_server };

		// This holds the self-signed certificate used by the server
		load_server_certificate(ctx);
		//ctx.set_verify_mode(boost::asio::ssl::verify_none);

		// Create and launch a listening port
		auto ret = std::make_shared<listener>(ioc, ctx, tcp::endpoint{ address, static_cast<unsigned short>(port) })->run();
		if (!ret) {
			fail("Listen port was used by other process");
			return EXIT_FAILURE;
		}
		printf("WebSocket server started!!\nlisten address=%s, port=%s\n", addressStr.c_str(), portStr.c_str());

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