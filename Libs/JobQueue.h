#pragma once

#include "Job.h"
namespace C_Utility
{
	class JobQueue : public std::enable_shared_from_this<JobQueue>
	{
	public:

		void DoAsync(CallbackFunc&& callback)
		{
			SharedJob job = std::make_shared<C_Utility::Job>(std::move(callback));

;			Push(job);
		}

		template <typename T, typename Ret, typename... Args>
		void DoAsync(Ret(T::* memFunc)(Args...), Args... args)
		{
			std::shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());

			SharedJob job = std::make_shared<C_Utility::Job>(owner, memFunc, std::forward<Args>(args)...);

			Push(job);
		}

		void Push(SharedJob job);
		void Execute();
		void ClearJob() { _jobQueue.Clear(); }
	protected:
		C_Utility::LockQueue<SharedJob> _jobQueue;
		std::atomic<uint> _jobCount;
	};

}