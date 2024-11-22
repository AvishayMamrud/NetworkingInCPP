#pragma once
#include "net_libs.h"

namespace net {

	template<typename T>
	struct message_header {
		T id{};
		uint32_t size = 0;
	};
	
	
	template<typename T>
	struct message {
		message_header<T> header;
		std::vector<uint8_t> body;

		size_t size() {
			return body.size();
		}

		friend std::ostream& operator<<(std::ostream& os, const message<T>& msg) {
			return os << "id - " << int(msg.header.id) << ".\nsize - " << msg.header.size;
		}

		template<typename DataType>
		friend message<T>& operator<<(message<T>& msg, const DataType& data) {
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex.");
			size_t curr_size = msg.header.size;
			msg.body.resize(curr_size + sizeof(DataType));
			memcpy(&msg.body + curr_size, &data, sizeof(DataType));
			msg.header.size += sizeof(DataType);
			return msg;
		}

		template<typename DataType>
		friend message<T>& operator>>(message<T>& msg, DataType& data) {
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex.");
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
		std::shared_ptr<connection<T>> con;
		message<T> msg;
		
	};
}