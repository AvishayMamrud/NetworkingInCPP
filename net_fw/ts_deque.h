#pragma once
#include <deque>

template<typename T>
class ts_deque {
public:
	ts_deque() = default;
	ts_deque(const ts_deque&) = delete;
	~ts_deque() {
		clear();
	}

	T& front() {
		std::scoped_lock lock(locker);
		return q.front();
	}

	T& back() {
		std::scoped_lock lock(locker);
		return q.back();
	}
	
	void push_back(const T& item) {
		std::scoped_lock lock(locker);
		q.emplace_back(std::move(item));
	}
	
	void push_front(T& item) {
		std::scoped_lock lock(locker);
		q.emplace_front(std::move(item));
	}
	
	T pop_back() {
		std::scoped_lock lock(locker);
		T val = std::move(q.back());
		q.pop_back();
		return val;
	}
	
	T pop_front() {
		std::scoped_lock lock(locker);
		T val = std::move(q.front());
		q.pop_front();
		return val;
	}
	
	void clear() {
		std::scoped_lock lock(locker);
		q.clear();
	}
	
	bool empty() {
		std::scoped_lock lock(locker);
		return q.empty();
	}
	
	size_t size() {
		std::scoped_lock lock(locker);
		return q.size();
	}
	
private:
	std::mutex locker;
	std::deque<T> q;
};