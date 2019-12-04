// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

EXTERN_C BOOL __cdecl DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#if defined(WIN32) || defined(WIN64)
		DisableThreadLibraryCalls(hModule);
#endif
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

