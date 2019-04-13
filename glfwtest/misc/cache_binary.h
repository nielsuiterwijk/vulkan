#pragma once

#include "helpers/measure/Measure.h"
#include <set>

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <vector>

namespace SearchTest
{

struct Data
{
	int32_t Key;
	int64_t Value;
};

struct LessThen
{
	bool operator()( const Data& left, const Data& right )
	{
		return left.Key < right.Key;
	}
};

template <typename T>
class vector_wrapper_linear
{
	typedef std::vector<T> vector;

public:
	typedef typename vector::value_type value_type;
	typedef typename vector::reference reference;
	typedef typename vector::const_reference const_reference;
	typedef typename vector::const_iterator iterator;
	typedef typename vector::const_iterator const_iterator;
	typedef typename vector::difference_type difference_type;
	typedef typename vector::size_type size_type;

	vector_wrapper_linear() {}
	vector_wrapper_linear( const vector_wrapper_linear& x ) :
		impl( x.impl ) {}
	vector_wrapper_linear& operator=( const vector_wrapper_linear& x )
	{
		impl = x.impl;
		return *this;
	}

	template <typename InputIterator>
	vector_wrapper_linear( InputIterator first, InputIterator last )
	{
		vector aux( first, last );
		std::sort( aux.begin(), aux.end(), LessThen() );
		impl = aux;
	}

	const_iterator begin() const { return impl.begin(); }
	const_iterator end() const { return impl.end(); }
	const_iterator cbegin() const { return impl.cbegin(); }
	const_iterator cend() const { return impl.cend(); }
	friend bool operator==( const vector_wrapper_linear& x, const vector_wrapper_linear& y )
	{
		return x.impl == y.impl;
	}
	friend bool operator!=( const vector_wrapper_linear& x, const vector_wrapper_linear& y )
	{
		return x.impl != y.impl;
	}
	void swap( vector_wrapper_linear& x ) { impl.swap( x.impl ); }
	friend void swap( vector_wrapper_linear& x, vector_wrapper_linear& y ) { x.swap( y ); }
	size_type size() const { return impl.size(); }
	size_type max_size() const { return impl.max_size(); }
	bool empty() const { return impl.empty(); }

	const_iterator lower_bound( const T& x ) const
	{
		for ( const_iterator it = impl.begin(); it != impl.end(); ++it )
		{
			if ( ( *it ).Key == x.Key )
				return it;
		}

		return impl.end();
		//return std::lower_bound(impl.begin(), impl.end(), x);
	}

private:
	vector impl;
};

template <typename T>
class vector_wrapper_binary
{
	typedef std::vector<T> vector;

public:
	typedef typename vector::value_type value_type;
	typedef typename vector::reference reference;
	typedef typename vector::const_reference const_reference;
	typedef typename vector::const_iterator iterator;
	typedef typename vector::const_iterator const_iterator;
	typedef typename vector::difference_type difference_type;
	typedef typename vector::size_type size_type;

	vector_wrapper_binary() {}
	vector_wrapper_binary( const vector_wrapper_binary& x ) :
		impl( x.impl ) {}
	vector_wrapper_binary& operator=( const vector_wrapper_binary& x )
	{
		impl = x.impl;
		return *this;
	}

	template <typename InputIterator>
	vector_wrapper_binary( InputIterator first, InputIterator last )
	{
		vector aux( first, last );
		std::sort( aux.begin(), aux.end(), LessThen() );
		impl = aux;
	}

	const_iterator begin() const { return impl.begin(); }
	const_iterator end() const { return impl.end(); }
	const_iterator cbegin() const { return impl.cbegin(); }
	const_iterator cend() const { return impl.cend(); }
	friend bool operator==( const vector_wrapper_binary& x, const vector_wrapper_binary& y )
	{
		return x.impl == y.impl;
	}
	friend bool operator!=( const vector_wrapper_binary& x, const vector_wrapper_binary& y )
	{
		return x.impl != y.impl;
	}
	void swap( vector_wrapper_binary& x ) { impl.swap( x.impl ); }
	friend void swap( vector_wrapper_binary& x, vector_wrapper_binary& y ) { x.swap( y ); }
	size_type size() const { return impl.size(); }
	size_type max_size() const { return impl.max_size(); }
	bool empty() const { return impl.empty(); }

	const_iterator lower_bound( const T& x ) const
	{
		return std::lower_bound( impl.begin(), impl.end(), x, LessThen() );
	}

private:
	vector impl;
};

template <typename T>
class levelorder_vector
{
	typedef std::vector<T> vector;

public:
	typedef typename vector::value_type value_type;
	typedef typename vector::reference reference;
	typedef typename vector::const_reference const_reference;
	typedef typename vector::const_iterator iterator;
	typedef typename vector::const_iterator const_iterator;
	typedef typename vector::difference_type difference_type;
	typedef typename vector::size_type size_type;

	levelorder_vector() {}
	levelorder_vector( const levelorder_vector& x ) :
		impl( x.impl ) {}
	levelorder_vector& operator=( const levelorder_vector& x )
	{
		impl = x.impl;
		return *this;
	}

	template <typename InputIterator>
	levelorder_vector( InputIterator first, InputIterator last )
	{
		vector aux( first, last );
		std::sort( aux.begin(), aux.end(), LessThen() );
		impl = aux;
		insert( 0, aux.size(), aux.begin() );
	}

	const_iterator begin() const { return impl.begin(); }
	const_iterator end() const { return impl.end(); }
	const_iterator cbegin() const { return impl.cbegin(); }
	const_iterator cend() const { return impl.cend(); }
	friend bool operator==( const levelorder_vector& x, const levelorder_vector& y )
	{
		return x.impl == y.impl;
	}
	friend bool operator!=( const levelorder_vector& x, const levelorder_vector& y )
	{
		return x.impl != y.impl;
	}
	void swap( levelorder_vector& x ) { impl.swap( x.impl ); }
	friend void swap( levelorder_vector& x, levelorder_vector& y ) { x.swap( y ); }
	size_type size() const { return impl.size(); }
	size_type max_size() const { return impl.max_size(); }
	bool empty() const { return impl.empty(); }

	const_iterator lower_bound( const T& x ) const
	{
		size_type n = impl.size(), i = n, j = 0;
		while ( j < n )
		{
			if ( impl[ j ].Key < x.Key )
			{
				j = 2 * j + 2;
			}
			else
			{
				i = j;
				j = 2 * j + 1;
			}
		}
		return begin() + i;
	}

private:
	void insert( size_type i, size_type n, const_iterator first )
	{
		if ( n )
		{
			size_type h = root( n );

			impl[ i ] = *( first + h );

			insert( 2 * i + 1, h, first );

			insert( 2 * i + 2, n - h - 1, first + h + 1 );
		}
	}

	size_type root( size_type n )
	{
		if ( n <= 1 )
			return 0;

		size_type i = 2;
		while ( i <= n )
			i *= 2;

		return std::min( i / 2 - 1, n - i / 4 );
	}

	vector impl;
};

struct rand_seq
{
	rand_seq( unsigned int n ) :
		dist( 0, n - 1 ),
		gen( 34862 ) {}
	int32_t operator()() { return (int32_t)dist( gen ); }

private:
	std::uniform_int_distribution<unsigned int> dist;
	std::mt19937 gen;
};

template <typename Container>
Container create( int32_t n )
{
	std::vector<Data> v;

	for ( int32_t m = 0; m < n; ++m )
		v.push_back( { m, m } );

	return Container( v.begin(), v.end() );
}

template <typename Container>
struct binary_search
{
	typedef unsigned int result_type;

	unsigned int operator()( const Container& c ) const
	{
		unsigned int res = 0;
		unsigned int n = (unsigned int)c.size();
		rand_seq rnd( n );
		auto end_ = c.end();

		while ( n-- )
		{
			if ( c.lower_bound( { rnd(), 0 } ) != end_ )
				++res;
		}

		return res;
	}
};

template <
	template <typename> class Tester,
	typename Container1, typename Container2, typename Container3>
void test(
	const char* title,
	const char* name1, const char* name2, const char* name3 )
{
	unsigned int n0 = 16, n1 = 8192, dn = 8;
	double fdn = 1.5;

	std::cout << title << " in micro seconds per search operation:" << std::endl;
	std::cout << name1 << ";      " << name2 << ";   " << name3 << ";" << std::endl;

	for ( unsigned int n = n0; n <= n1; n += dn, dn = (unsigned int)( dn * fdn ) )
	{
		double t1, t2, t3;

		std::cout << std::setprecision( 5 );
		std::cout << n << ": ";

		std::cout << std::fixed;
		std::cout << std::setprecision( 5 );

		{
			auto c = create<Container1>( n );
			t1 = measure( std::bind( Tester<Container1>(), std::cref( c ) ) );
			std::cout << ( t1 / n ) * 10E6;
		}
		{
			auto c = create<Container2>( n );
			t2 = measure( std::bind( Tester<Container2>(), std::cref( c ) ) );
			std::cout << "; " << ( t2 / n ) * 10E6;
		}
		{
			auto c = create<Container3>( n );
			t3 = measure( std::bind( Tester<Container3>(), std::cref( c ) ) );
			std::cout << "; " << ( t3 / n ) * 10E6;
		}

		std::cout << std::fixed;
		std::cout << std::setprecision( 2 );

		double percentageT1 = ( t1 / t1 ) * 100.0;
		double percentageT2 = ( t2 / t1 ) * 100.0;
		double percentageT3 = ( t3 / t1 ) * 100.0;

		std::cout << "; " << percentageT1 << "% " << percentageT2 << "% " << percentageT3 << "% " << std::endl;
	}
}

void Run()
{
	typedef vector_wrapper_linear<Data> container_t1;
	typedef vector_wrapper_binary<Data> container_t2;
	typedef levelorder_vector<Data> container_t3;

	test<binary_search, container_t1, container_t2, container_t3>( "Binary search", "linear", "binary", "cache friendly" );
}
};
