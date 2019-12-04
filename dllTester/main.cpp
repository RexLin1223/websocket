#include <Windows.h>
#include "../src/WebsocketServerKey/WSServerExport.h"
#include <iostream>
#include <fstream>

#include <rapidjson/document.h>

typedef HANDLE (_cdecl *CreateServer)(const char*, unsigned short);
typedef int (*DeleteServer)(HANDLE);
void OnDataFunc(const char* data, unsigned int dataLen, void* obj) {
	std::string s(data, dataLen);
	std::ofstream ofs;
	ofs.open("C:\\PTSLog\\output.txt", std::ios::binary | std::ios::app);
	if (ofs && ofs.is_open()) {
		ofs.write(data, dataLen);
		ofs.write("\r\n", 1);
	}
	ofs.close();
}

unsigned int OnValidateFunc(
	const char* data, 
	unsigned int dataLen, 
	char* error_buffer, 
	unsigned int error_buffer_length,  
	void* obj) {
	rapidjson::Document doc;
	doc.Parse(data);

	std::string error;
	if (doc.HasParseError()) {
		error = "Invalid JSON";
		memcpy(error_buffer, error.c_str(), error.size());
		return error.size();
	}
	
	if (!doc.HasMember("rm_detail")) {
		error = "Missing mandatory field: rm_detail";
		memcpy(error_buffer, error.c_str(), error.size());
		return error.size();
	}

	const auto& rm_details = doc["rm_detail"];
	for (auto itor = rm_details.Begin(); itor != rm_details.End(); ++itor) {
		const auto& rm_detail = *itor;
		if (!rm_detail.HasMember("sell_shopid")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: sell_shopid";
			break;
		}

		if (!rm_detail.HasMember("sell_posid")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: sell_posid";
			break;
		}

		if (!rm_detail.HasMember("sell_day")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: sell_day";
			break;
		}

		if (!rm_detail.HasMember("sell_no")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: sell_no";
			break;
		}

		if (!rm_detail.HasMember("tday")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: tday";
			break;
		}

		if (!rm_detail.HasMember("sell_time")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: sell_time";
			break;
		}

		if (!rm_detail.HasMember("sell_transtype")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: sell_transtype";
			break;
		}

		if (!rm_detail.HasMember("total_sales_amt")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: total_sales_amt";
			break;
		}

		if (!rm_detail.HasMember("total_qty")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: total_qty";
			break;
		}

		if (!rm_detail.HasMember("sell_detail")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: sell_detail";
			break;
		}

		if (!rm_detail.HasMember("tender_detail")) {
			if (!error.empty()) error.append("; ");
			error += "Missing mandatory field: tender_detail";
			break;
		}

		if (error.size() > 0 && error.size() <= error_buffer_length) {
			memcpy(error_buffer, error.c_str(), error.size());
		}
	}
	return error.size();
}

int main() {
	HINSTANCE h = LoadLibrary(".\\WebsocketKeyServer.dll");
	auto createServer = (fnCreateServerInstance)(GetProcAddress(h, "CreateServerInstance"));
	auto deleteServer = (fnDestroyServerInstance)(GetProcAddress(h, "DestroyServerInstance"));
	auto setListener = (fnSetListener)(GetProcAddress(h, "SetListener"));
	auto setKey = (fnSetKey)(GetProcAddress(h, "SetKey"));
	auto setCertificate = (fnSetCertificate)(GetProcAddress(h, "SetCertificate"));
	auto registerOnData = (fnRegisterOnData)(GetProcAddress(h, "RegisterOnData"));
	auto registerOnValidate = (fnRegisterOnValidate)(GetProcAddress(h, "RegisterOnValidate"));
	auto startServer = (fnStart)(GetProcAddress(h, "Start"));
	auto stopServer = (fnStop)(GetProcAddress(h, "Stop"));

	bool is_ssl = false;
	void* instance = createServer(is_ssl);
	setListener(instance, "0.0.0.0", 8080);
	setKey(instance, "C124654DE1458");
	setCertificate(instance,
		"C:\\Users\\Rex\\Documents\\Visual Studio 2017\\Projects\\web_socket_server\\dependencies\\cert\\host.crt",
		"C:\\Users\\Rex\\Documents\\Visual Studio 2017\\Projects\\web_socket_server\\dependencies\\cert\\host.key");
	registerOnData(instance, OnDataFunc, nullptr);
	registerOnValidate(instance, OnValidateFunc, nullptr);
	int ret = startServer(instance, 1);
	int i;
	std::cin >> i;
	stopServer(instance);
	deleteServer(instance);
	FreeLibrary(h);
}