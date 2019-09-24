#pragma once
#include <functional>

namespace websocket {
	enum ErrorType {
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
	};

	template<typename T> using CallbackHandler = std::function<void(const T data)>;
}