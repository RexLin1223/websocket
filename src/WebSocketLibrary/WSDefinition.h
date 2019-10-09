#pragma once
#include <functional>
#include <memory>
#include <string>

#include <boost/beast/core/error.hpp>
#include <boost/asio/ssl/context.hpp>

namespace websocket {
	template<typename T> using OnConnectionCompleted =
		std::function<void(std::shared_ptr<T>)>;

	template<typename T> using AsyncReadHandler = 
		std::function<void(boost::beast::error_code, 
			std::size_t, std::string&&, 
			std::shared_ptr<T>)>;

	template<typename T> using AsyncWriteHandler = 
		std::function<void(boost::beast::error_code,
		std::size_t, std::shared_ptr<T>)>;

	using SSLContext = std::shared_ptr<boost::asio::ssl::context>;

}