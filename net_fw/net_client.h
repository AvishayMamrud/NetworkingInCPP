#pragma once
#include "net.h"
#include "ts_deque.h"
#include "net_connection.h"

namespace net {
	template<typename T>
	class client {
	public:
		client() {

		}

		virtual ~client() {
			disconnect();
		}

		void connect(const std::string& host, uint16_t port) {
			try {
				asio::ip::tcp::resolver resolver(context);
				auto endpoints = resolver.resolve(asio::ip::tcp::v4(), host, std::to_string(port));

				con = std::make_shared<connection<T>>(
					connection<T>::owner::client,
					context,
					asio::ip::tcp::socket(context),
					q_messagesIn
				);

				con->connectToServer(endpoints);

				thr = std::thread([this]() { context.run(); });
			}
			catch (std::exception& e) {
				std::cout << "[CLIENT] Error occured while connecting to server - " << e.what() << std::endl;
			}
		}

		void disconnect() {
			con->disconnect();
			context.stop();
			if (thr.joinable())
				thr.join();
		}

		bool isConnected() {
			return con->isConnected();
		}

		void send(message<T>& msg) {
			if(con->isConnected())
				con->send(msg);
		}

		ts_deque<owned_message<T>>& incomingMessages() {
			return q_messagesIn;
		}

	private:
		asio::io_context context;
		std::thread thr;
		std::shared_ptr<connection<T>> con;
		ts_deque<owned_message<T>> q_messagesIn;
	};
}
