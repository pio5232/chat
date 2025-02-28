#include "LibsPch.h"
#include "GlobalQueue.h"

GlobalQueue::GlobalQueue()
{
}

GlobalQueue::~GlobalQueue()
{
}

void GlobalQueue::Push(JobQueuePtr sharedJobQueue)
{
	_queue.Push(sharedJobQueue);
}

JobQueuePtr GlobalQueue::Pop()
{
	return _queue.Pop();
}
