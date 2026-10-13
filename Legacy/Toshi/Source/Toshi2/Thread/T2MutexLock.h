#pragma once
#include "T2Mutex.h"

TOSHI_NAMESPACE_START

class T2MutexLock
{
public:
	T2MutexLock(T2Mutex& mutex)
	    : m_Mutex(mutex) { m_Mutex.Lock(); }
	~T2MutexLock() { m_Mutex.Unlock(); }

private:
	T2Mutex& m_Mutex;
};

TOSHI_NAMESPACE_END
