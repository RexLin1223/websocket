#pragma once
#include <memory>
#include "WSServerExport.h"

namespace websocket {
	class TokenServerInterface {
		void* server_;

	public:
		TokenServerInterface();
		virtual ~TokenServerInterface();

		bool Create(bool is_ssl);
		void Destory();

		bool Start(size_t requestThreads);
		void Stop();

		void RegisterOnConnected(OnConnected onConnected);
		void RegisterOnDisconnected(OnDisconnected onDisconnected);
		void RegisterOnData(OnData onData);
		void RegisterOnError(OnError onError);

		void SetListener(const char* address, unsigned short port);
		void SetCertificate(const char* certificateFile, const char* privateKeyFile);
	};
}