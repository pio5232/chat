#include "LibsPch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"

void C_Utility::JobQueue::Push(JobPtr job)
{
	const uint16 prev = _jobCount.fetch_add(1);

	_jobQueue.Push(job);

	// 처음 잡을 넣은 스레드라면 실행
	if (prev == 0)
	{
		if (LThreadJobQueue == nullptr)
		{
			Execute();
		}
		else
		{
			GGlobalQueue->Push(shared_from_this());
		}
	}
}

void C_Utility::JobQueue::Execute()
{
	LThreadJobQueue = this;

	while (true)
	{
		std::vector<JobPtr> jobVec;
		_jobQueue.PopAll(OUT jobVec);

		const uint16 jobCount = jobVec.size();
		for (int i = 0; i < jobCount; i++)
			jobVec[i]->Execute();

		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			LThreadJobQueue = nullptr;
			return;
		}

		const ULONGLONG now = GetTickCount64();
		if (now >= ::LExecuteTimeTick)
		{
			LThreadJobQueue = nullptr;

			GGlobalQueue->Push(shared_from_this());
			break;
		}
	}
}
