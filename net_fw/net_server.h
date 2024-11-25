#pragma once
#include "net.h"
#include "net_connection.h"
#include "ts_deque.h"

namespace net {
	template<typename T>
	class server {
	public:
		server(uint16_t port) : acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

		virtual ~server() {
			stop();
		}

		bool start() {
			try {
				waitForClients();
				thr = std::thread([this]() { context.run(); });
				std::cout << "[SERVER] Started successfully." << std::endl;
			}
			catch (std::exception& e) {
				std::cout << "[SERVER] Starting failed." << std::endl;
				return false;
			}
			return true;
		}

		void stop() {
			context.stop();
			if (thr.joinable())
				thr.join();
		}

		void waitForClients() {
			acceptor.async_accept(
				[this](const asio::error_code& ec, asio::ip::tcp::socket clientSock) {
					if (!ec) {
						std::shared_ptr<connection<T>> new_conn = std::make_shared<connection<T>>(
							connection<T>::owner::server,
							context,
							std::move(clientSock),
							q_messagesIn
						);
						if (onClientConnect(new_conn)) {
							clients.push_back(new_conn);
							clients.back()->connectToClient(this, idCounter++);
							std::cout << "[SERVER] Client accepted." << std::endl;
						}
						else {
							std::cout << "[SERVER] Client regected." << std::endl;
						}
					}
					else {
						std::cout << "[SERVER] Client acceptance failed." << std::endl;
					}
					waitForClients();
				}
				);
		}

		void messageClient(std::shared_ptr<connection<T>> con, message<T>& msg) {
			if(con->isConnected())
				con->send(msg);
		}

		void messageAllClients(message<T>& msg, std::shared_ptr<connection<T>> clientToIgnore = nullptr) {
			for (auto client : clients) {
				if(client != clientToIgnore)
					client->send(msg);
			}
		}

		void blocking_update(uint32_t max_msg_count = -1) { // the default maximun is the max_value of an unsigned int
			uint32_t readCount = 0;
			while (readCount < max_msg_count) {
				owned_message<T> om = std::move(q_messagesIn.blocking_pop_front());
				onMessage(om.con, om.msg);
				readCount++;
			}
		}

		void update(uint32_t max_msg_count = -1) { // the default maximun is the max_value of an unsigned int
			uint32_t readCount = 0;
			while (readCount < max_msg_count && !q_messagesIn.empty()) {
				owned_message<T> om = std::move(q_messagesIn.pop_front());
				onMessage(om.con, om.msg);
				readCount++;
			}
		}

		virtual void onClientValidation(std::shared_ptr<connection<T>> client){}

		virtual bool onClientConnect(std::shared_ptr<connection<T>> client) { return false; }

		virtual void onMessage(std::shared_ptr<connection<T>> client_conn, message<T>& msg) {}

		virtual void onClientDisconnect(std::shared_ptr<connection<T>> client) {}

	private:
		asio::io_context context;
		std::thread thr;
		asio::ip::tcp::acceptor acceptor;
		std::vector<std::shared_ptr<connection<T>>> clients;
		ts_deque<owned_message<T>> q_messagesIn;
		uint32_t idCounter = 10000;
	};
}
