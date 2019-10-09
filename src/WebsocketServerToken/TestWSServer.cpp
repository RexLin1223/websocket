#include "WSServer.h"

int main() {
	{
		websocket::WSServerToken<websocket::server_ssl_session, websocket::ssl_session> server("0.0.0.0", 8080);
		server.set_token("C124654DE1458");
		server.start(1);

		// Blocking at here
		int a;
		std::cin >> a;
	}
	printf("done");
}