#pragma once
#include "CRingBuffer.h"
#include "CSerializationBuffer.h"
#include "CCrashDump.h"
#include "CLog.h"
#include <stack>
/*-------RAII--------
	  LockGuard
-------------------*/

class SRWLockGuard
{
public:
	SRWLockGuard(SRWLOCK* pLock) : _pLock(pLock) { AcquireSRWLockExclusive(_pLock); }
	~SRWLockGuard() { ReleaseSRWLockExclusive(_pLock); }

	SRWLOCK* _pLock;
};


namespace C_Utility
{
	// 생성자에 대한 정의가 필요하다.
	template <typename T, typename... Args>
	class ManagerPool
	{
	public:
		ManagerPool(uint maxElementCnt, Args... args) : _maxElementCnt(maxElementCnt), _curElementCnt(0)
		{
			InitializeSRWLock(&_indexListLock);

			_elementArr.reserve(maxElementCnt);

			for (int i = maxElementCnt - 1; i >= 0; i--)
			{
				_availableElementidxList.push(i);
				// TODO: POOL
				T* newElement = new T(args...);
				_elementArr.push_back(newElement);
			}
		}

		virtual ~ManagerPool() = 0;
		uint GetCurElementCount() const { return _curElementCnt; } // 현재 접속한 세션 수
		uint GetMaxElemenetCount() const { return _maxElementCnt;  }

		bool IsFull()
		{
			SRWLockGuard lockGuard(&_indexListLock);
			return _availableElementidxList.empty();
		}

	protected:
		// Accept에서는 차지 않았다가 차는 경우가 존재하지 않는다.
		uint GetAvailableIndex()
		{
			SRWLockGuard lockGuard(&_indexListLock);

			uint idx = _availableElementidxList.top();
			_availableElementidxList.pop();

			return idx;
		}

		std::vector<T*> _elementArr;
		std::stack<uint> _availableElementidxList;

		SRWLOCK _indexListLock;
		uint _maxElementCnt;
		volatile ULONG _curElementCnt;
	};
	template<typename T, typename ...Args>
	ManagerPool<T, Args...>::~ManagerPool()
	{
	}
}