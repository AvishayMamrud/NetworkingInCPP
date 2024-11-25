#pragma once
#include "net.h"
#include "net_client.h"
#include "ts_deque.h"

enum class msgType : uint32_t {
	s_accept,
	c_hello_everyone,
	c_hello_server,
	s_hey_muy_caliente,
	goodbye
};

using namespace net;

class caliente : public client<msgType> {
public:
	void waitForInput(msgType cmd) {
		int a = 0, b = 234;
		switch (cmd) {
			case msgType::c_hello_server:
			{
				std::cout << "xdfcgvghkj" << std::endl;
				message<msgType> msg;
				msg.header.id = msgType::c_hello_server;
				send(msg);
			}
			break;
			case msgType::c_hello_everyone:
			{
				message<msgType> msg;
				msg.header.id = msgType::c_hello_everyone;
				send(msg);
			}
			break;
			default:
			{
				std::cout << "[CLIENT] wrong command." << std::endl;
			}
		}
	}
};

int main() {
	caliente c;
	c.connect("127.0.0.1", 60000);

	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };

	bool quit = false;

	while (!quit) {

		if (GetForegroundWindow() == GetConsoleWindow()) {
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}

		if (key[0] && !old_key[0]) quit = true;
		if (key[1] && !old_key[1]) c.waitForInput(msgType::c_hello_server);
		if (key[2] && !old_key[2]) c.waitForInput(msgType::c_hello_everyone);

		for (int i = 0; i < 3; i++)
			old_key[i] = key[i];

		if (c.isConnected()) {
			if (!c.incomingMessages().empty()) {
				message<msgType> msg = c.incomingMessages().pop_front().msg;

				switch (msg.header.id) {
				case msgType::s_accept:
				{
					std::cout << "amazing" << std::endl;
				}
				break;
				case msgType::s_hey_muy_caliente:
				{
					std::cout << "received reply." << std::endl;
				}
				break;
				case msgType::goodbye:
				{
					std::cout << "server says goodbye..." << std::endl;
					quit = true;
				}
				break;
				}
			}
		}
		else {
			quit = true;
			std::cout << "connection lost." << std::endl;
		}
		
	}
	return 0;
}