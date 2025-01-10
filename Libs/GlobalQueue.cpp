#include "LibsPch.h"
#include "GlobalQueue.h"

GlobalQueue::GlobalQueue()
{
}

GlobalQueue::~GlobalQueue()
{
}

void GlobalQueue::Push(SharedJobQueue sharedJobQueue)
{
	_queue.Push(sharedJobQueue);
}

SharedJobQueue GlobalQueue::Pop()
{
	return _queue.Pop();
}
