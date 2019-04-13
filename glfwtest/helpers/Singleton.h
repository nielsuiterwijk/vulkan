/*
 * Usage:
 *
 * class MyClass : public Singleton<MyClass>
 * {
 * }
 *
 * MyClass::getInstance().doSomething();
 *
 * if you want to make sure your derived class can't be
 * instanciated or new'ed you have to add your class'
 * constructor to the protected part of the class and add
 * a friend reference to the Singleton<MyClass> class.
 *
 */

#ifndef SINGLETON_H
#define SINGLETON_H

template <class T>
class Singleton
{
public:
	static T& GetInstance()
	{
		static T _instance;
		return _instance;
	}

	virtual void destroyInstance() {};

	// hide normal/copy/assignment constructors and destructor
protected:
	Singleton() {};
	virtual ~Singleton() {};

private:
	Singleton( Singleton const& );
	Singleton& operator=( Singleton const& );
};

#endif
