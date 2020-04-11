#pragma once


class Mutex
{
public:
	Mutex() { InitializeSRWLock( &_SrwLock ); }

	Mutex( const Mutex& ) = delete;
	Mutex& operator=( const Mutex& ) = delete;

	void Lock() { AcquireSRWLockExclusive( &_SrwLock ); }

	bool TryLock() { return TryAcquireSRWLockExclusive( &_SrwLock ); }
	void Unlock() { ReleaseSRWLockExclusive( &_SrwLock ); }

	//C++11 interface
public:
	void lock() { Lock(); }
	bool try_lock() { return TryLock(); }
	void unlock() { Unlock(); }

private:
	SRWLOCK _SrwLock;
};