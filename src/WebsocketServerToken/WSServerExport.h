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

	typedef void(__cdecl *OnJoin)();
	WSSERVER_API void __cdecl RegisterOnJoin(void* ptr, OnJoin onJoin);
	typedef void(__cdecl *OnLeave)();
	WSSERVER_API void __cdecl RegisterOnLeave(void* ptr, OnLeave onLeave);
	typedef void(__cdecl *OnData)(const char*, unsigned int);
	WSSERVER_API void __cdecl RegisterOnData(void* ptr, OnData onData);
	typedef void(__cdecl *OnError)(int);
	WSSERVER_API void __cdecl RegisterOnError(void* ptr, OnError onError);

	WSSERVER_API void __cdecl SetListener(
		void* ptr, const char* address, unsigned short port);
	WSSERVER_API void __cdecl SetCertificate(
		void* ptr, const char* certificate_file, const char* private_key_file);
	WSSERVER_API void __cdecl SetToken(
		void* ptr, const char* token);

	WSSERVER_API bool __cdecl Start(void* ptr, unsigned short requestThreads);
	WSSERVER_API void __cdecl Stop(void* ptr);

	typedef void*(__cdecl *fnCreateServerInstance)(int);
	typedef void(__cdecl *fnDestroyServerInstance)(void*);
	typedef void(__cdecl *fnRegisterOnJoin)(void*, OnJoin);
	typedef void(__cdecl *fnRegisterOnLeave)(void*, OnLeave);
	typedef void(__cdecl *fnRegisterOnData)(void*, OnData);
	typedef void(__cdecl *fnRegisterOnError)(void*, OnError);
	typedef void(__cdecl *fnSetListener)(void*, const char*, unsigned short);
	typedef void(__cdecl *fnSetCertificate)(void*, const char*, const char*);
	typedef void(__cdecl *fnSetToken)(void*, const char*);
	typedef bool(__cdecl *fnStart)(void*, unsigned short);
	typedef void(__cdecl *fnStop)(void*);

#ifdef __cplusplus
}
#endif

