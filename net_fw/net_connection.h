#pragma once
#include "net.h"
#include "ts_deque.h"
#include "net_server.h"


namespace net {
	template<typename T>
	class server;

	template<typename T>
	class connection : public std::enable_shared_from_this<connection<T>> {
	public:
		enum class owner {
			server,
			client
		};

		connection(owner ownerType, asio::io_context& context, asio::ip::tcp::socket sock, ts_deque<owned_message<T>>& qin)
			: ownerType(ownerType), asioContext(context), asioSock(std::move(sock)), q_inMessages(qin) {}

		~connection() {
			disconnect();
		}

		void connectToServer(asio::ip::tcp::resolver::results_type& endpoints) {
			if (ownerType == owner::client) {
				asio::async_connect(asioSock, endpoints,
					[this](const asio::error_code& ec, const asio::ip::tcp::endpoint& next) {
						if (!ec) {
							readValidation();
						}
						else {
							std::cout << "[" << id_ << "] Connection to server failed." << std::endl;
						}
					}
				);
			}
		}

		void connectToClient(net::server<T>* server, uint32_t id = 0) {
			if (ownerType == owner::server) {
				id_ = id;
				validation_out = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
				validation_check = scramble(validation_out);
				writeValidation();
				readValidation(server);
			}
		}

		void disconnect() {
			if (asioSock.is_open())
				asio::post(asioContext, [this]() { asioSock.close(); });
		}

		bool isConnected() {
			return asioSock.is_open();
		}

		void send(const message<T>& msg) {
			asio::post(asioContext,
				[this, msg]() {
					bool noPending = q_outMessages.empty();
					q_outMessages.push_back(msg);
					if (noPending) {
						writeHeader();
					}
				}
				);
		}

	private:

		void readValidation(net::server<T>* server = nullptr) {
			asio::async_read(asioSock, asio::buffer(&validation_in, sizeof(uint64_t)),
				[this, server](const asio::error_code& ec, size_t bytesTransferred) {
					if (!ec) {
						if (ownerType == owner::server) {
							if (validation_in == validation_check) {
								server->onClientValidation(this->shared_from_this());
								readHeader();
							}
							else {
								std::cout << "[] Validation failed." << std::endl;
								asioSock.close();
							}
						}
						else if (ownerType == owner::client) {
							validation_out = scramble(validation_in);
							writeValidation();
						}
					}
					else {
						std::cout << "[] Reading header failed." << std::endl;
						asioSock.close();
					}
				}
			);
		}

		void writeValidation() {
			asio::async_write(asioSock, asio::buffer(&validation_out, sizeof(uint64_t)),
				[this](const asio::error_code& ec, size_t bytesTransferred) {
					if (!ec) {
						if (ownerType == owner::client) {
							readHeader();
						}
					}
					else {
						std::cout << "[] Writing header failed." << std::endl;
					}
				}
			);
		}

		uint64_t scramble(uint64_t input) {
			input = input ^ 0x943278ACFE924781;
			input = (input << 32) | (input >> 32);
			input = input ^ 0x5a5a5a5a5a5a5a5a;
			return input;
		}

		void readHeader() {
			asio::async_read(asioSock, asio::buffer(&tmpmsg.header, sizeof(message_header<T>)),
				[this](const asio::error_code& ec, size_t bytesTransferred) {
					if (!ec) {
						if (tmpmsg.header.size > 0) {
							tmpmsg.body.resize(tmpmsg.header.size);
							readBody();
						}
						else {
							addIncomingMessage();
						}
					}
					else {
						std::cout << "[] Reading header failed." << std::endl;
						asioSock.close();
					}
				}
			);
		}

		void readBody() {
			asio::async_read(asioSock, asio::buffer(tmpmsg.body.data(), tmpmsg.body.size()),
				[this](const asio::error_code& ec, size_t bytesTransferred) {
					if (!ec) {
						addIncomingMessage();
					}
					else {
						std::cout << "[] Reading body failed." << std::endl;
						asioSock.close();
					}
				}
			);
		}

		void writeHeader() {
			asio::async_write(asioSock, asio::buffer(&q_outMessages.front().header, sizeof(message_header<T>)),
				[this](const asio::error_code& ec, size_t bytesTransferred) {
					if (!ec) {
						if (q_outMessages.front().header.size > 0) {
							writeBody();
						}
						else {
							q_outMessages.pop_front();
							if(!q_outMessages.empty())
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

		void addIncomingMessage() {
			if(ownerType == connection<T>::owner::server)
				q_inMessages.push_back({ this->shared_from_this(), tmpmsg });
			else q_inMessages.push_back({ nullptr, tmpmsg });
			readHeader();
		}

		asio::io_context& asioContext;
		asio::ip::tcp::socket asioSock;

		ts_deque<owned_message<T>>& q_inMessages;
		ts_deque<message<T>> q_outMessages;
		message<T> tmpmsg;

		owner ownerType;
		uint32_t id_ = 0;

		uint64_t validation_in = 0;
		uint64_t validation_out = 0;
		uint64_t validation_check = 0;
	};

}