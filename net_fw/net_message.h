#pragma once
#include "net_libs.h"

namespace net {

	template<typename T>
	struct message_header {
		T id;
		uint32_t size = 0;
	};
	
	
	template<typename T>
	struct message {
		message_header<T> header;
		vector<uint8_t> body;

		size_t size() {
			return body.size();
		}

		friend std::ostream& operator<<(std::ostream& os, message<T>& msg) {
			return os << "id - " << msg.header.id << ".\nsize - " << msg.header.size;
		}

		template<typename DataType>
		message<T>& operator<<(message<T>& msg, DataType& data) {
			static_assert(std::is_standard_layout<DataType>);
			size_t curr_size = msg.header.size;
			msg.body.resize(curr_size + sizeof(DataType));
			memcpy(&msg.body + curr_size, data, sizeof(DataType));
			msg.header.size += sizeof(DataType);
			return msg;
		}

		template<typename DataType>
		message<T>& operator>>(message<T>& msg, DataType& data) {
			static_assert(std::is_standard_layout<DataType>);
			size_t new_size = msg.body.size() - sizeof(DataType);
			memcpy(&data, msg.body.data() + new_size, sizeof(DataType));
			msg.body.resize(new_size);
			msg.header.size = msg.body.size();
			return msg;
		}
	};

	template<typename T>
	class connection;

	template<typename T>
	struct owned_message {
		message<T> msg;
		std::shared_ptr<connection<T>> con;
	};
}