#include "WSServer.h"
#include "WSServerInterface.h"

namespace websocket {
	TokenServerInterface::TokenServerInterface()
	{
	}

	TokenServerInterface::~TokenServerInterface()
	{
		Destory();
	}

	bool TokenServerInterface::Create(bool is_ssl)
	{
		if (is_ssl) {
			server_ = new (std::nothrow) TokenServerSSL;
		}
		else {
			server_ = new (std::nothrow) TokenServer;
		}

		return server_ != nullptr;
	}

	void TokenServerInterface::Destory()
	{
		if (!server_) return;
		reinterpret_cast<TokenServer*>(server_)->stop();
		delete reinterpret_cast<TokenServer*>(server_);
	}

	bool TokenServerInterface::Start(size_t request_threads)
	{
		if (!server_) return false;
		return reinterpret_cast<TokenServer*>(server_)->start(request_threads);
	}

	void TokenServerInterface::Stop()
	{
		if (!server_) return;
		reinterpret_cast<TokenServer*>(server_)->stop();
	}

	void TokenServerInterface::RegisterOnConnected(OnConnected onConnected)
	{
		
	}

	void TokenServerInterface::RegisterOnDisconnected(OnDisconnected onDisconnected)
	{

	}

	void TokenServerInterface::RegisterOnData(OnData onData)
	{

	}

	void TokenServerInterface::RegisterOnError(OnError onError)
	{

	}

	void TokenServerInterface::SetListener(const char* address, unsigned short port)
	{
		if (!server_) return;
		reinterpret_cast<TokenServer*>(server_)->set_listener(address, port);
	}

	void TokenServerInterface::SetCertificate(const char* certificateFile, const char* privateKeyFile)
	{
		if (!server_) return;
		reinterpret_cast<TokenServer*>(server_)->set_ssl_config(
			boost::asio::ssl::context_base::tls_server,
			certificateFile, privateKeyFile,
			boost::asio::ssl::context::file_format::pem);
	}
}