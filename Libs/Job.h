#pragma once

#include <functional>

// ----------------------
//			Job
// ----------------------

namespace C_Utility
{
	using CallbackFunc = std::function<void()>;
	class Job
	{
	public:

		Job(CallbackFunc&& callback) : _callback(std::move(callback)) {}

		template <typename T, typename Ret, typename... Args>
		Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
		{
			_callback = [owner, memFunc, args...]()
			{
				(owner.get()->*memFunc)(args...);
			};
		}

		void Execute()
		{
			_callback();
		}
	private:
		CallbackFunc _callback;
	};
}