#include "LibsPch.h"
#include "GlobalInstance.h"
#include "GlobalQueue.h"

class GlobalQueue* GGlobalQueue;

class GlobalManager
{
public:

	GlobalManager()
	{
		GGlobalQueue = new GlobalQueue();
	}

	~GlobalManager()
	{
		delete GGlobalQueue;
	}
};

GlobalManager GGlobalManager;