#include "net.h"
#include "net_server.h"
#include "net_connection.h"

enum class msgType : uint32_t {
	s_accept,
	c_hello_everyone,
	c_hello_server,
	s_hey_mui_cliente,
	goodbye
};

using namespace net;

class cerberous : public server<msgType> {
public:
	cerberous(uint16_t port) : server(port){}
	bool onClientConnect(std::shared_ptr<connection<msgType>> client) {
		return true;
	}

	void onMessage(std::shared_ptr<connection<msgType>> client_conn, message<msgType>& msg) {
		std::cout << "msg - " << msg << std::endl;
		switch (msg.header.id) {
			case msgType::c_hello_server:
			{
				message<msgType> new_msg;
				new_msg.header.id = msgType::s_hey_mui_cliente;
				messageClient(client_conn, new_msg);
			}
			break;
			case msgType::c_hello_everyone:
			{
				message<msgType> new_msg;
				new_msg.header.id = msgType::s_accept;
				messageAllClients(new_msg, client_conn);
				//messageAllClients(new_msg, client_conn);
			}
			break;
		}
	}
};

int main() {
	cerberous servo(60000);
	servo.start();

	while (true) {
		servo.blocking_update();
	}
	return 0;
}