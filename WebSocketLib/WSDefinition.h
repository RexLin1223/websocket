#pragma once
#include <functional>
#include <memory>
#include <string>
#include <boost/beast.hpp>
#include <boost/asio.hpp>

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

	/*enum ErrorType {
		NormalClose = 1000,
		EndPointClose = 1001,
		ProtocolError = 1002,
		InvalidContentFormat = 1003,
		Reserved = 1004,
		NoStatusCode = 1005,
		AbnormalClose = 1006,
		InvalidEncodeType = 1007,
		ViolatePolicy = 1008,
		ExceedLimit = 1009,
		HandshakeFailed = 1010,
		UnexpectedCondition = 1011,
		HandshakeTLSFailed = 1015,
		KeepaliveTimeout = 2001,
		Unknown
	};

	enum MessageType {
		Connect,
		Disconnect,
		Message
	};

	struct MessageStruct {

		const char* data;
		unsigned int dataLength;
		ErrorType error;
	};*/
}