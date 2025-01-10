#pragma once
class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();

	void Push(SharedJobQueue sharedJobQueue);
	SharedJobQueue Pop();
private:
	C_Utility::LockQueue<SharedJobQueue> _queue;
};

