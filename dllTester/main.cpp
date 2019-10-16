#include <Windows.h>
#include "../src/WebsocketServerToken/WSServerExport.h"
#include <iostream>

typedef HANDLE (_cdecl *CreateServer)(const char*, unsigned short);
typedef int (*DeleteServer)(HANDLE);
void OnDataFunc(const char* data, unsigned int dataLen) {
	std::string s(data, dataLen);
	printf("!!!!Received data %s\n", s.c_str());
}

int main() {
	HINSTANCE h = LoadLibrary(".\\WebsocketTokenServer.dll");
	auto createServer = (fnCreateServerInstance)(GetProcAddress(h, "CreateServerInstance"));
	auto deleteServer = (fnDestroyServerInstance)(GetProcAddress(h, "DestroyServerInstance"));
	auto setListener = (fnSetListener)(GetProcAddress(h, "SetListener"));
	auto setToken = (fnSetToken)(GetProcAddress(h, "SetToken"));
	auto setCertificate = (fnSetCertificate)(GetProcAddress(h, "SetCertificate"));
	auto registerOnData = (fnRegisterOnData)(GetProcAddress(h, "RegisterOnData"));
	auto startServer = (fnStart)(GetProcAddress(h, "Start"));
	auto stopServer = (fnStop)(GetProcAddress(h, "Stop"));

	bool is_ssl = false;
	void* instance = createServer(is_ssl);
	setListener(instance, "0.0.0.0", 8080);
	setToken(instance, "C124654DE1458");
	setCertificate(instance,
		"C:\\Users\\Rex\\Documents\\Visual Studio 2017\\Projects\\web_socket_server\\dependencies\\cert\\host.crt",
		"C:\\Users\\Rex\\Documents\\Visual Studio 2017\\Projects\\web_socket_server\\dependencies\\cert\\host.key");
	registerOnData(instance, OnDataFunc);
	bool ret = startServer(instance, 1);
	int i;
	std::cin >> i;
	stopServer(instance);
	deleteServer(instance);
	FreeLibrary(h);
}