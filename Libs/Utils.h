#pragma once
#include "CRingBuffer.h"
#include "CSerializationBuffer.h"
#include "CCrashDump.h"
#include "CLog.h"
#include <stack>
#include <queue>
/*-------RAII--------
	  LockGuard
-------------------*/

class SRWLockGuard
{
public:
	SRWLockGuard(SRWLOCK* lock) : _lock(lock) { AcquireSRWLockExclusive(_lock); }
	~SRWLockGuard() { ReleaseSRWLockExclusive(_lock); }

	SRWLOCK* _lock;
};

class SRWSharedLockGuard
{
public:
	SRWSharedLockGuard(SRWLOCK* lock) : _lock(lock) { AcquireSRWLockShared(_lock); }
	~SRWSharedLockGuard() { ReleaseSRWLockShared(_lock); }

private:
	SRWLOCK* _lock;

};


namespace C_Utility
{
	// �����ڿ� ���� ���ǰ� �ʿ��ϴ�.
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
				// AA
				T* newElement = new T(args...);
				_elementArr.push_back(newElement);
			}
		}

		virtual ~ManagerPool() = 0;
		uint GetCurElementCount() const { return _curElementCnt; } // ���� ������ ���� ��
		uint GetMaxElemenetCount() const { return _maxElementCnt;  }

		bool IsFull()
		{
			SRWLockGuard lockGuard(&_indexListLock);
			return _availableElementidxList.empty();
		}

		// ����Ǵ� �����ʹ� ����ϴ� ������ ������ ����� �ϵ��� �Ѵ�.
		void ObjectInitialize(uint deAllocateIndex)
		{			
			SRWLockGuard lockGuard(&_indexListLock);
			
			_availableElementidxList.push(deAllocateIndex);
		}

	protected:
		// Accept������ ���� �ʾҴٰ� ���� ��찡 �������� �ʴ´�. �� á�� ��� UINT_MAX ��ȯ�Ѵ�.
		uint GetAvailableIndex()
		{
			SRWLockGuard lockGuard(&_indexListLock);

			if (_availableElementidxList.size() == 0)
				return UINT_MAX;

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

	template <typename T>
	class LockQueue {
	public:
		LockQueue()
		{
			InitializeSRWLock(&_lock);
		}
		void Push(T job)
		{
			SRWLockGuard lockGuard(&_lock);

			_queue.push(job);
		}

		T Pop()
		{
			SRWLockGuard lockGuard(&_lock);
			if (_queue.size() > 0)
			{
				T ret = _queue.front();
				_queue.pop();
				
				return ret;
			}
			
			return nullptr;
		}

		void PopAll(OUT std::vector<T>& vec)
		{
			SRWLockGuard lockGuard(&_lock);

			while (_queue.size() > 0)
			{
				T ele = _queue.front();
				_queue.pop();

				vec.push_back(ele);
			}
		}

		void Clear()
		{
			SRWLockGuard lockGuard(&_lock);
			
			_queue = std::queue<T>();
			
		}
	private:
		SRWLOCK _lock;
		std::queue<T> _queue;
	};
}

void ExecuteProcess(const std::wstring& path, const std::wstring& args);

struct Vector3
{
	Vector3();
	Vector3(float x, float y, float z);

	static Vector3 Zero()
	{
		return Vector3();
	}
	Vector3(const Vector3& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}

	Vector3& operator= (const Vector3& other)
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
		}

		return *this;
	}

	float x;
	float y;
	float z;
};