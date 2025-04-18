#pragma once
#include "CRingBuffer.h"
#include "CSerializationBuffer.h"
#include "CCrashDump.h"
#include "CLog.h"
#include <stack>
#include <queue>
#include <math.h>
#include <algorithm>
/*-------RAII--------
	  LockGuard
-------------------*/

class SRWLockGuard
{
public:
	SRWLockGuard(SRWLOCK* lock) : _playerLock(lock) { AcquireSRWLockExclusive(_playerLock); }
	~SRWLockGuard() { ReleaseSRWLockExclusive(_playerLock); }

	SRWLOCK* _playerLock;
};

class SRWSharedLockGuard
{
public:
	SRWSharedLockGuard(SRWLOCK* lock) : _playerLock(lock) { AcquireSRWLockShared(_playerLock); }
	~SRWSharedLockGuard() { ReleaseSRWLockShared(_playerLock); }

private:
	SRWLOCK* _playerLock;

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
				// AA
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

		// 재사용되는 데이터는 사용하는 측에서 관리를 제대로 하도록 한다.
		void ObjectInitialize(uint deAllocateIndex)
		{			
			SRWLockGuard lockGuard(&_indexListLock);
			
			_availableElementidxList.push(deAllocateIndex);
		}

	protected:
		// Accept에서는 차지 않았다가 차는 경우가 존재하지 않는다. 다 찼을 경우 UINT_MAX 반환한다.
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
			InitializeSRWLock(&_playerLock);
		}
		void Push(T job)
		{
			SRWLockGuard lockGuard(&_playerLock);

			_queue.push(job);
		}

		T Pop()
		{
			SRWLockGuard lockGuard(&_playerLock);
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
			SRWLockGuard lockGuard(&_playerLock);

			while (_queue.size() > 0)
			{
				T ele = _queue.front();
				_queue.pop();

				vec.push_back(ele);
			}
		}

		void Clear()
		{
			SRWLockGuard lockGuard(&_playerLock);
			
			_queue = std::queue<T>();
			
		}
	private:
		SRWLOCK _playerLock;
		std::queue<T> _queue;
	};
}

void ExecuteProcess(const std::wstring& path);// , const std::wstring& args);

// Unity Vector System 이용
struct Vector3
{
	Vector3();
	Vector3(float x, float y, float z);
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

	Vector3& operator+= (const Vector3& other)
	{
		if (this != &other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
		}

		return *this;
	}

	Vector3 operator- (const Vector3& other)
	{
		return Vector3(x - other.x, y - other.y, z - other.z);
	}

	Vector3 operator- (const Vector3& other) const
	{
		return Vector3(x - other.x, y - other.y, z - other.z);
	}

	Vector3 operator*(float f)
	{
		return Vector3(x * f, y * f, z * f);
	}

	static Vector3 Zero()
	{
		static const Vector3 zero(0, 0, 0);
		return zero;
	}

	static Vector3 Left()
	{
		static const Vector3 left(-1.0f, 0, 0);
		return left;
	}
	static Vector3 Right()
	{
		static const Vector3 right(1.0f, 0, 0);
		return right;
	}
	static Vector3 Forward()
	{
		static const Vector3 forward(0, 0, 1.0f);
		return forward;
	}
	static Vector3 Back()
	{
		static const Vector3 back(0, 0, -1.0f);
		return back;
	}

	// Distance와 Magnitude는 루트계산 -> 느리다. 정확한 값아니면 사용x
	static float Distance(const Vector3& firstVec, const Vector3& secondVec);
	float Magnitude()
	{
		int powCount = 2;

		return sqrt(pow(x, powCount) + pow(y, powCount) + pow(z, powCount));
	}
	float sqrMagnitude() { return x * x + y * y + z * z; }

	static float Dot(const Vector3& from, const Vector3& to)
	{
		return from.x * to.x + from.y * to.y + from.z * to.z;
	}

	Vector3 Normalized()
	{
		float magnitude = Magnitude();

		if(magnitude > 1E-05f)
			return Vector3(x / magnitude, y / magnitude, z / magnitude);
		
		return Zero();
	}

	// 두 벡터 사이의 각도를 반환한다. 방향상관없이 리턴값은 0~180, 0~360을 위해선 SignedAngle을 사용하라.
	static float Angle(Vector3 from, Vector3 to)
	{
		float num = (float)sqrt(from.sqrMagnitude() * to.sqrMagnitude());
		if (num < 1E-15f)
		{
			return 0;
		}

		float num2 = std::clamp(Dot(from, to) / num, -1.0f, 1.0f);
		return (float)acos(num2) * 57.29578f;
	}


	bool operator== (const Vector3& other) const
	{
		return this->x == other.x && this->y == other.y && this->z == other.z;
	}

	bool operator!= (const Vector3& other) const
	{
		return !((*this) == other);
	}

	float x;
	float y;
	float z;
};

const float deg2Rad = 180.0f / 3.141592f;

double GetRandDouble(double min, double max, int roundPlaceValue = 0);
int GetRand(int min, int max);

bool CheckChance(int percentage);

float NormalizeAngle(float angle);