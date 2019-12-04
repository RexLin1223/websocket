#pragma once
#include <memory>
#include "WSServerExport.h"

namespace websocket {
	class KeyServerInterface {
		void* server_;
		bool is_ssl_;
	public:
		KeyServerInterface();
		virtual ~KeyServerInterface();

		bool Create(bool is_ssl);
		void Destory();

		bool Start(size_t requestThreads);
		void Stop();

		void RegisterOnJoin(OnJoin onJoin, void* classObject = nullptr);
		void RegisterOnLeave(OnLeave onLeave, void* classObject = nullptr);
		void RegisterOnData(OnData onData, void* classObject = nullptr);
		void RegisterOnError(OnError onError, void* classObject = nullptr);
		void RegisterOnValidate(OnValidate onValidate, void* classObject = nullptr);

		void SetKey(const char* key);
		void SetListener(const char* address, unsigned short port);
		void SetCertificate(const char* certificateFile, const char* privateKeyFile);
	};
}