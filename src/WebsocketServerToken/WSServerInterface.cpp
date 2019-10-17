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
		is_ssl_ = is_ssl;
		if (is_ssl_) {
			server_ = new (std::nothrow) TokenSSLServer;
		}
		else {
			server_ = new (std::nothrow) TokenServer;
		}

		return server_ != nullptr;
	}

	void TokenServerInterface::Destory()
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<TokenSSLServer*>(server_)->stop();
		}
		else {
			reinterpret_cast<TokenServer*>(server_)->stop();
		}
		delete server_;
	}

	bool TokenServerInterface::Start(size_t request_threads)
	{
		if (!server_) return false;

		if (is_ssl_) {
			return reinterpret_cast<TokenSSLServer*>(server_)->start(request_threads);
		}
		else {
			return reinterpret_cast<TokenServer*>(server_)->start(request_threads);
		}
	}

	void TokenServerInterface::Stop()
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<TokenSSLServer*>(server_)->stop();
		}
		else {
			reinterpret_cast<TokenServer*>(server_)->stop();
		}
	}

	void TokenServerInterface::RegisterOnJoin(OnJoin onJoin, void* classObject /*= nullptr*/)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<TokenSSLServer*>(server_)->register_on_join(onJoin, classObject);
		}
		else {
			reinterpret_cast<TokenServer*>(server_)->register_on_join(onJoin, classObject);
		}
	}

	void TokenServerInterface::RegisterOnLeave(OnLeave onLeave, void* classObject /*= nullptr*/)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<TokenSSLServer*>(server_)->register_on_leave(onLeave, classObject);
		}
		else {
			reinterpret_cast<TokenServer*>(server_)->register_on_leave(onLeave, classObject);
		}
	}

	void TokenServerInterface::RegisterOnData(OnData onData, void* classObject /*= nullptr*/)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<TokenSSLServer*>(server_)->register_on_data(onData, classObject);
		}
		else {
			reinterpret_cast<TokenServer*>(server_)->register_on_data(onData, classObject);
		}
	}

	void TokenServerInterface::RegisterOnError(OnError onError, void* classObject /*= nullptr*/)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<TokenSSLServer*>(server_)->register_on_error(onError, classObject);
		}
		else {
			reinterpret_cast<TokenServer*>(server_)->register_on_error(onError, classObject);
		}
	}

	void TokenServerInterface::SetToken(const char* token)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<TokenSSLServer*>(server_)->set_token(token);
		}
		else {
			reinterpret_cast<TokenServer*>(server_)->set_token(token);
		}
	}

	void TokenServerInterface::SetListener(const char* address, unsigned short port)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<TokenSSLServer*>(server_)->set_listener(address, port);
		}
		else {
			reinterpret_cast<TokenServer*>(server_)->set_listener(address, port);
		}
	}

	void TokenServerInterface::SetCertificate(const char* certificateFile, const char* privateKeyFile)
	{

		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<TokenSSLServer*>(server_)->set_ssl_config(
				boost::asio::ssl::context_base::tls_server,
				certificateFile, privateKeyFile,
				boost::asio::ssl::context::file_format::pem);
		}
		else {
			reinterpret_cast<TokenServer*>(server_)->set_ssl_config(
				boost::asio::ssl::context_base::tls_server,
				certificateFile, privateKeyFile,
				boost::asio::ssl::context::file_format::pem);
		}			
	}
}