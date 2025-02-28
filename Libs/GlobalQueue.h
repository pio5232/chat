#pragma once
class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();

	void Push(JobQueuePtr sharedJobQueue);
	JobQueuePtr Pop();
private:
	C_Utility::LockQueue<JobQueuePtr> _queue;
};

