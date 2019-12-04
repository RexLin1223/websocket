#include "WSServerExport.h"
#include "WSServerInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

	using KeyServerInterface = websocket::KeyServerInterface;

	WSSERVER_API void* __cdecl CreateServerInstance(int is_ssl)
	{
		auto ptr = new (std::nothrow) KeyServerInterface();
		if (!ptr) return nullptr;
		reinterpret_cast<KeyServerInterface*>(ptr)->Create(is_ssl);
		return ptr;
	}

	WSSERVER_API void __cdecl DestroyServerInstance(void* ptr)
	{
		if (!ptr) return;
		delete reinterpret_cast<KeyServerInterface*>(ptr);
	}
	WSSERVER_API void __cdecl RegisterOnJoin(void* ptr, OnJoin onJoin, void* classObject /*= NULL*/)
	{
		if (!ptr) return;
		reinterpret_cast<KeyServerInterface*>(ptr)->RegisterOnJoin(onJoin, classObject);
	}

	WSSERVER_API void __cdecl RegisterOnLeave(void* ptr, OnLeave onLeave, void* classObject /*= NULL*/)
	{
		if (!ptr) return;
		reinterpret_cast<KeyServerInterface*>(ptr)->RegisterOnLeave(onLeave, classObject);
	}

	WSSERVER_API void __cdecl RegisterOnData(void* ptr, OnData onData, void* classObject /*= NULL*/)
	{
		if (!ptr) return;
		reinterpret_cast<KeyServerInterface*>(ptr)->RegisterOnData(onData, classObject);
	}

	WSSERVER_API void __cdecl RegisterOnError(void* ptr, OnError onError, void* classObject /*= NULL*/)
	{
		if (!ptr) return;
		reinterpret_cast<KeyServerInterface*>(ptr)->RegisterOnError(onError, classObject);
	}

	WSSERVER_API void __cdecl RegisterOnValidate(void* ptr, OnValidate onValidate, void* classObject /*= NULL*/)
	{
		if (!ptr) return;
		reinterpret_cast<KeyServerInterface*>(ptr)->RegisterOnValidate(onValidate, classObject);
	}

	WSSERVER_API void __cdecl SetListener(void* ptr, const char* address, unsigned short port)
	{
		if (!ptr) return;
		reinterpret_cast<KeyServerInterface*>(ptr)->SetListener(address, port);
	}

	WSSERVER_API void __cdecl SetCertificate(void* ptr, const char* certificate_file, const char* private_key_file)
	{
		if (!ptr) return;
		reinterpret_cast<KeyServerInterface*>(ptr)->SetCertificate(certificate_file, private_key_file);
	}

	WSSERVER_API void __cdecl SetKey(void* ptr, const char* key)
	{
		if (!ptr) return;
		reinterpret_cast<KeyServerInterface*>(ptr)->SetKey(key);
	}

	WSSERVER_API int __cdecl Start(void* ptr, unsigned short requestThreads)
	{
		if (!ptr) return false;
		return reinterpret_cast<KeyServerInterface*>(ptr)->Start(requestThreads);
	}

	WSSERVER_API void __cdecl Stop(void* ptr)
	{
		if (!ptr) return;
		reinterpret_cast<KeyServerInterface*>(ptr)->Stop();
	}

#ifdef __cplusplus
}
#endif
