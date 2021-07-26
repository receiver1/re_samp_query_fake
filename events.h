#pragma once

#include <mutex>
#include <functional>

template <typename R, typename... Args>
class c_event 
{
	using func_type = std::function<R(Args...)>;

	std::mutex event_lock;
	func_type m_func;

public:
	void set(const func_type &func) 
	{
		std::lock_guard<std::mutex> lock(event_lock);

		m_func = func;
	}

	R call(Args... params) {
		std::lock_guard<std::mutex> lock(event_lock);

		if (m_func) return m_func(std::forward<Args>(params)...);
	}
};
