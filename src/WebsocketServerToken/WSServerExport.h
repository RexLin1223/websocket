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

	typedef void(__cdecl *OnJoin)(void*);
	WSSERVER_API void __cdecl RegisterOnJoin(void* ptr, OnJoin onJoin, void* classObject = 0);
	typedef void(__cdecl *OnLeave)(void*);
	WSSERVER_API void __cdecl RegisterOnLeave(void* ptr, OnLeave onLeave, void* classObject = 0);
	typedef void(__cdecl *OnData)(const char*, unsigned int, void*);
	WSSERVER_API void __cdecl RegisterOnData(void* ptr, OnData onData, void* classObject = 0);
	typedef void(__cdecl *OnError)(int, void*);
	WSSERVER_API void __cdecl RegisterOnError(void* ptr, OnError onError, void* classObject = 0);

	WSSERVER_API void __cdecl SetListener(
		void* ptr, const char* address, unsigned short port);
	WSSERVER_API void __cdecl SetCertificate(
		void* ptr, const char* certificate_file, const char* private_key_file);
	WSSERVER_API void __cdecl SetToken(
		void* ptr, const char* token);

	WSSERVER_API int __cdecl Start(void* ptr, unsigned short requestThreads);
	WSSERVER_API void __cdecl Stop(void* ptr);

	typedef void*(__cdecl *fnCreateServerInstance)(int);
	typedef void(__cdecl *fnDestroyServerInstance)(void*);
	typedef void(__cdecl *fnRegisterOnJoin)(void*, OnJoin, void*);
	typedef void(__cdecl *fnRegisterOnLeave)(void*, OnLeave, void*);
	typedef void(__cdecl *fnRegisterOnData)(void*, OnData, void*);
	typedef void(__cdecl *fnRegisterOnError)(void*, OnError, void*);
	typedef void(__cdecl *fnSetListener)(void*, const char*, unsigned short);
	typedef void(__cdecl *fnSetCertificate)(void*, const char*, const char*);
	typedef void(__cdecl *fnSetToken)(void*, const char*);
	typedef int(__cdecl *fnStart)(void*, unsigned short);
	typedef void(__cdecl *fnStop)(void*);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <functional>
// Functor declaration
typedef std::function<void* __cdecl(int)> CreateServerFunc;
typedef std::function<void __cdecl(void*)> DestoryServerFunc;
typedef std::function<void __cdecl(void*, OnJoin, void*)> RegisterOnJoinFunc;
typedef std::function<void __cdecl(void*, OnLeave, void*)> RegisterOnLeaveFunc;
typedef std::function<void __cdecl(void*, OnData, void*)> RegisterOnDataFunc;
typedef std::function<void __cdecl(void*, OnError, void*)> RegisterOnErrorFunc;
typedef std::function<void __cdecl(void*, const char*, unsigned short)> SetListenerFunc;
typedef std::function<void __cdecl(void*, const char*, const char*)> SetCertificateFunc;
typedef std::function<void __cdecl(void*, const char*)> SetTokenFunc;
typedef std::function<int __cdecl(void*, unsigned short)> StartFunc;
typedef std::function<void __cdecl(void*)> StopFunc;
#endif
