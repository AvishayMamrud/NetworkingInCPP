#pragma once

#include "net_libs.h"
#include <deque>

namespace net
{
	template<typename T>
	class ts_deque
	{
	public:
		ts_deque() = default;
		ts_deque(const ts_deque<T>&) = delete;
		virtual ~ts_deque() { clear(); }

	public:
		// Returns and maintains item at front of Queue
		const T& front()
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			return deq.front();
		}

		// Returns and maintains item at back of Queue
		const T& back()
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			return deq.back();
		}

		// Removes and returns item from front of Queue
		T pop_front()
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			auto t = std::move(deq.front());
			deq.pop_front();
			return t;
		}

		// Removes and returns item from back of Queue
		T pop_back()
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			auto t = std::move(deq.back());
			deq.pop_back();
			return t;
		}
		
		// Removes and returns item from front of Queue - wait if empty
		T blocking_pop_front()
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			while (deq.empty())
				cvBlocking.wait(ul);
			auto t = std::move(deq.front());
			deq.pop_front();
			return t;
		}

		// Removes and returns item from back of Queue - wait if empty
		T blocking_pop_back()
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			while (deq.empty())
				cvBlocking.wait(ul);
			auto t = std::move(deq.back());
			deq.pop_back();
			return t;
		}

		// Adds an item to back of Queue
		void push_back(const T& item)
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			deq.emplace_back(std::move(item));

			cvBlocking.notify_one();
		}

		// Adds an item to front of Queue
		void push_front(const T& item)
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			deq.emplace_front(std::move(item));

			cvBlocking.notify_one();
		}

		// Returns true if Queue has no items
		bool empty()
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			return deq.empty();
		}

		// Returns number of items in Queue
		size_t count()
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			return deq.size();
		}

		// Clears Queue
		void clear()
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			deq.clear();
		}

		void wait()
		{
			while (empty())
			{
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.wait(ul);
			}
		}

	protected:
		std::deque<T> deq;
		std::condition_variable cvBlocking;
		std::mutex muxBlocking;
	};
}