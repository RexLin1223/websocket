#pragma once

#ifdef EXPORTFUNC
#define WSSERVER_API  __declspec(dllexport)
#else
#define WSSERVER_API  __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	WSSERVER_API void* __cdecl CreateServerInstance(int is_ssl);
	WSSERVER_API void __cdecl DestroyServerInstance(void* ptr);

	typedef void(__cdecl *OnConnected)();
	WSSERVER_API void __cdecl RegisterOnConnected(void* ptr, OnConnected onConnected);
	typedef void(__cdecl *OnDisconnected)();
	WSSERVER_API void __cdecl RegisterOnDisconnected(void* ptr, OnDisconnected onDisconnected);
	typedef void(__cdecl *OnData)(const char*, unsigned int);
	WSSERVER_API void __cdecl RegisterOnData(void* ptr, OnData onData);
	typedef void(__cdecl *OnError)(int);
	WSSERVER_API void __cdecl RegisterOnError(void* ptr, OnError onError);

	WSSERVER_API void __cdecl SetListener(
		void* ptr, const char* address, unsigned short port);
	WSSERVER_API void __cdecl SetCertificate(
		void* ptr, const char* certificate_file, const char* private_key_file);

	WSSERVER_API bool __cdecl Start(void* ptr, unsigned short requestThreads);
	WSSERVER_API void __cdecl Stop(void* ptr);

	typedef void*(__cdecl *fnCreateServerInstance)();
	typedef void(__cdecl *fnDestroyServerInstance)(void*);
	typedef void(__cdecl *fnRegisterOnConnected)(void*, OnConnected);
	typedef void(__cdecl *fnRegisterOnDisconnected)(void*, OnDisconnected);
	typedef void(__cdecl *fnRegisterOnData)(void*, OnData);
	typedef void(__cdecl *fnRegisterOnError)(void*, OnError);
	typedef void(__cdecl *fnSetListener)(void*, const char*, unsigned short);
	typedef void(__cdecl *fnSetCertificate)(void*, const char*, const char*);
	typedef bool(__cdecl *fnStart)(void*, unsigned short);
	typedef void(__cdecl *fnStop)(void*);

#ifdef __cplusplus
}
#endif

