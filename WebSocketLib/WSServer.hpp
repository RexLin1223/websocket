#pragma once
#include "WSSession.h"

#include <boost/asio.hpp>

namespace websocket {
	class WSServer {
	public:
		WSServer() {
		}
		virtual ~WSServer() {
		}

		void Listen(unsigned int port) {
			// TODO listen port with web socket
		}

		void Send(CallbackHandler<MessageStruct> handler) {
			// TODO send
		}

	private:
		boost::asio::io_context io_context_;

	};
}