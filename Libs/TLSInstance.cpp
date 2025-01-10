#include "LibsPch.h"
#include "TLSInstance.h"
#include "JobQueue.h"

thread_local C_Utility::JobQueue* LThreadJobQueue = nullptr;
thread_local uint64 LExecuteTimeTick = 0;