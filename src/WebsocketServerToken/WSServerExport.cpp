#include "WSServerExport.h"
#include "WSServerInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

	using TokenServerInterface = websocket::TokenServerInterface;

	WSSERVER_API void* __cdecl CreateServerInstance(int is_ssl)
	{
		auto ptr = new (std::nothrow) TokenServerInterface();
		if (!ptr) return nullptr;
		reinterpret_cast<TokenServerInterface*>(ptr)->Create(is_ssl);
		return ptr;
	}

	WSSERVER_API void __cdecl DestroyServerInstance(void* ptr)
	{
		if (!ptr) return;
		delete reinterpret_cast<TokenServerInterface*>(ptr);
	}

	WSSERVER_API void __cdecl RegisterOnConnected(void* ptr, OnConnected onConnected)
	{
		if (!ptr) return;
		reinterpret_cast<TokenServerInterface*>(ptr)->RegisterOnConnected(onConnected);
	}

	WSSERVER_API void __cdecl RegisterOnDisconnected(void* ptr, OnDisconnected onDisconnected)
	{
		if (!ptr) return;
		reinterpret_cast<TokenServerInterface*>(ptr)->RegisterOnDisconnected(onDisconnected);
	}

	WSSERVER_API void __cdecl RegisterOnData(void* ptr, OnData onData)
	{
		if (!ptr) return;
		reinterpret_cast<TokenServerInterface*>(ptr)->RegisterOnData(onData);
	}

	WSSERVER_API void __cdecl RegisterOnError(void* ptr, OnError onError)
	{
		if (!ptr) return;
		reinterpret_cast<TokenServerInterface*>(ptr)->RegisterOnError(onError);
	}

	WSSERVER_API void __cdecl SetListener(void* ptr, const char* address, unsigned short port)
	{
		if (!ptr) return;
		reinterpret_cast<TokenServerInterface*>(ptr)->SetListener(address, port);
	}

	WSSERVER_API void __cdecl SetCertificate(void* ptr, const char* certificate_file, const char* private_key_file)
	{
		if (!ptr) return;
		reinterpret_cast<TokenServerInterface*>(ptr)->SetCertificate(certificate_file, private_key_file);
	}

	WSSERVER_API bool __cdecl Start(void* ptr, unsigned short requestThreads)
	{
		if (!ptr) return false;
		return reinterpret_cast<TokenServerInterface*>(ptr)->Start(requestThreads);
	}

	WSSERVER_API void __cdecl Stop(void* ptr)
	{
		if (!ptr) return;
		reinterpret_cast<TokenServerInterface*>(ptr)->Stop();
	}

#ifdef __cplusplus
}
#endif
