#include <Windows.h>
#include "../src/WebsocketServerToken/WSServerExport.h"
#include <iostream>

typedef HANDLE (_cdecl *CreateServer)(const char*, unsigned short);
typedef int (*DeleteServer)(HANDLE);
int main() {
	HINSTANCE h = LoadLibrary(".\\WebsocketTokenServer.dll");
	auto createServer = (fnCreateServerInstance)(GetProcAddress(h, "CreateServerInstance"));
	auto deleteServer = (fnDestroyServerInstance)(GetProcAddress(h, "DestroyServerInstance"));
	auto setlistener = (fnSetListener)(GetProcAddress(h, "SetListener"));
	auto startServer = (fnStart)(GetProcAddress(h, "Start"));

	void* instance = createServer();
	setlistener(instance, "0.0.0.0", 8080);
	bool ret = startServer(instance, 1);
	int i;
	std::cin >> i;
	deleteServer(instance);
	FreeLibrary(h);
}