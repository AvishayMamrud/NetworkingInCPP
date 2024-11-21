#pragma once
#include "net.h"
#include "ts_deque.h"

namespace net {
	template<typename T>
	class connection : public std::enable_shared_from_this<connection<T>> {
	public:
		enum class owner {
			server,
			client
		};

		connection(owner ownerType, asio::io_context& context, asio::ip::tcp::socket sock, ts_deque<message<T>>& qin)
			: asioContext(context), asioSock(sock), q_inMessages(qin), ownerType(ownerType){}
		~connection(){}

		void connectToServer(asio::ip::tcp::resolver::results_type& endpoints) {
			if (ownerType == owner::client) {
				asio::async_connect(asioSock, endpoints,
					[](const asio::error_code& ec, const asio::ip::tcp::endpoint& next) {
						if (!ec) {
							ReadHeader();
						}
						else {
							std::cout << "[" << id << "] Connection to server failed." << std::endl;
						}
					}
				);
			}
		}

		void connectToClient(uint32_t id = 0) {
			if (ownerType == owner::server) {
				id_ = id;
				readHeader();
			}
		}

		void disconnect() {
			if (asioSock.is_open())
				asio::post(asioContext, [this]() { asioSock.close(); });
		}

		bool isConnected() {
			return asioSock.is_open()
		}

		void send(message<T>& msg) {
			asio::post(asioContext,
				[this, msg]() {
					if (!ec) {
						bool noPending = q_outMessages.empty();
						q_outMessages.push_back(msg);
						if (noPending) {
							WriteHeader();
						}
					}
					else {
						std::cout << "[" << std::to_string(ownerType) << "] - Sending failed." << std::endl;
					}
				}
				);
		}

	private:
		void readHeader() {
			asio::async_read(asioSock, asio::buffer(&tmpmsg.header, sizeof(message_header)),
				[this](const asio::error_code& ec, size_t bytesTransferred) {
					if (!ec) {
						if (tmpmsg.header.size > 0) {
							tmpmsg.body.resize(tmpmsg.header.size);
							readBody();
						}
						else {
							q_inMessages.push_back(std::move(tmpmsg));
							readHeader();
						}
					}
					else {
						std::cout << "[] Reading header failed." << std::endl;
					}
				}
			);
		}

		void readBody() {
			asio::async_read(asioSock, asio::buffer(tmpmsg.body.data(), tmpmsg.body.size()),
				[this](const asio::error_code& ec, size_t bytesTransferred) {
					if (!ec) {
						q_inMessages.push_back(std::move(tmpmsg));
						readHeader();
					}
					else {
						std::cout << "[] Reading body failed." << std::endl;
					}
				}
			);
		}

		void writeHeader() {
			asio::async_write(asioSock, asio::buffer(&q_outMessages.front().header, sizeof(message_header)),
				[this](const asio::error_code& ec, size_t bytesTransferred) {
					if (!ec) {
						if (q_outMessages.front().header.size > 0) {
							writeBody();
						}
						else {
							q_outMessages.pop_front();
							writeHeader();
						}
					}
					else {
						std::cout << "[] Writing header failed." << std::endl;
					}
				}
			);
		}

		void writeBody() {
			asio::async_write(asioSock, 
				asio::buffer(q_outMessages.front().body.data(), q_outMessages.front().body.size()),
				[this](const asio::error_code& ec, size_t bytesTransferred) {
					if (!ec) {
						q_outMessages.pop_front();
						if(!q_outMessages.empty()) {
							writeHeader();
						}
					}
					else {
						std::cout << "[] Writing header failed." << std::endl;
					}
				}

			);
		}


		asio::io_context& asioContext;
		asio::ip::tcp::socket asioSock;

		ts_deque<message<T>>& q_inMessages;
		ts_deque<message<T>> q_outMessages;
		message<T> tmpmsg;

		owner ownerType;
		uint32_t id_ = 0;
	};

}