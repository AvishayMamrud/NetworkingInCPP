#pragma once
#include "net.h"
#include "ts_deque.h"

namespace net {
	template<typename T>
	class client {
	public:
		client() {

		}

		virtual ~client() {

		}

		void connect();
		void disconnect();

	private:
		std::shared_ptr<connection<T>> con;

	};
}
