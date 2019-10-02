#include <WebSocketLib/WSClient.hpp>

int main() {
	{
		websocket::WSClient server("0.0.0.0", 8080);
		server.set_token("C124654DE1458");
		server.start(1);

		// Blocking at here
		int a;
		std::cin >> a;
	}
	printf("done");
}