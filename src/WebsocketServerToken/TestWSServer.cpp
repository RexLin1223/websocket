#include "WSServer.h"

int main() {
	{
		websocket::WSServerToken<websocket::server_ssl_session, websocket::ssl_session> server("0.0.0.0", 8080);
		server.set_token("C124654DE1458");

		websocket::ssl_config config = {
			boost::asio::ssl::context_base::tls_server,
			"C:\\Users\\Rex\\Documents\\Visual Studio 2017\\Projects\\web_socket_server\\dependencies\\cert\\host.crt",
			"C:\\Users\\Rex\\Documents\\Visual Studio 2017\\Projects\\web_socket_server\\dependencies\\cert\\host.key",
			boost::asio::ssl::context::file_format::pem
		};
		server.set_ssl_config(std::move(config));
		server.start(1);

		// Blocking at here
		int a;
		std::cin >> a;
	}
	printf("done");
}