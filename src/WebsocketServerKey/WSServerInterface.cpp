#include "WSServer.h"
#include "WSServerInterface.h"

namespace websocket {
	KeyServerInterface::KeyServerInterface()
	{
	}

	KeyServerInterface::~KeyServerInterface()
	{
		Destory();
	}

	bool KeyServerInterface::Create(bool is_ssl)
	{
		is_ssl_ = is_ssl;
		if (is_ssl_) {
			server_ = new (std::nothrow) KeySSLServer;
		}
		else {
			server_ = new (std::nothrow) KeyServer;
		}

		return server_ != nullptr;
	}

	void KeyServerInterface::Destory()
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->stop();
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->stop();
		}
		delete server_;
	}

	bool KeyServerInterface::Start(size_t request_threads)
	{
		if (!server_) return false;

		if (is_ssl_) {
			return reinterpret_cast<KeySSLServer*>(server_)->start(request_threads);
		}
		else {
			return reinterpret_cast<KeyServer*>(server_)->start(request_threads);
		}
	}

	void KeyServerInterface::Stop()
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->stop();
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->stop();
		}
	}

	void KeyServerInterface::RegisterOnJoin(OnJoin onJoin, void* classObject /*= nullptr*/)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->register_on_join(onJoin, classObject);
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->register_on_join(onJoin, classObject);
		}
	}

	void KeyServerInterface::RegisterOnLeave(OnLeave onLeave, void* classObject /*= nullptr*/)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->register_on_leave(onLeave, classObject);
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->register_on_leave(onLeave, classObject);
		}
	}

	void KeyServerInterface::RegisterOnData(OnData onData, void* classObject /*= nullptr*/)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->register_on_data(onData, classObject);
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->register_on_data(onData, classObject);
		}
	}

	void KeyServerInterface::RegisterOnError(OnError onError, void* classObject /*= nullptr*/)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->register_on_error(onError, classObject);
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->register_on_error(onError, classObject);
		}
	}

	void KeyServerInterface::RegisterOnValidate(OnValidate onValidate, void* classObject /*= nullptr*/)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->register_on_validate(onValidate, classObject);
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->register_on_validate(onValidate, classObject);
		}
	}

	void KeyServerInterface::SetKey(const char* key)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->set_key(key);
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->set_key(key);
		}
	}

	void KeyServerInterface::SetListener(const char* address, unsigned short port)
	{
		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->set_listener(address, port);
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->set_listener(address, port);
		}
	}

	void KeyServerInterface::SetCertificate(const char* certificateFile, const char* privateKeyFile)
	{

		if (!server_) return;

		if (is_ssl_) {
			reinterpret_cast<KeySSLServer*>(server_)->set_ssl_config(
				boost::asio::ssl::context_base::tls_server,
				certificateFile, privateKeyFile,
				boost::asio::ssl::context::file_format::pem);
		}
		else {
			reinterpret_cast<KeyServer*>(server_)->set_ssl_config(
				boost::asio::ssl::context_base::tls_server,
				certificateFile, privateKeyFile,
				boost::asio::ssl::context::file_format::pem);
		}			
	}
}