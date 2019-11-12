#pragma once

#include "glm.hpp"
#include <immintrin.h>
#include <iostream>

namespace AVXTest
{
	void Test()
	{

		float a[ 8 ] = { 1, 2, 3, 4, 5, 6, 7, 8 };
		float b[ 8 ] = { 2, 3, 4, 5, 6, 7, 8, 9 };
		float c[ 8 ];

		auto AVX = [ a, b, &c ]() {
			__m256 ymm0, ymm1; //define the registers used

			ymm0 = _mm256_loadu_ps( a ); //load the 8 floats in a into ymm0
			ymm1 = _mm256_loadu_ps( b ); //load the 8 floats in b into ymm1

			//multiply ymm0 and ymm1, store the result in ymm0
			ymm0 = _mm256_mul_ps( ymm0, ymm1 );
			_mm256_storeu_ps( c, ymm0 ); //copy the 8 floats in ymm0 to c

			return c[ 0 ];
		};

		auto Scalar = [ a, b, &c ]() {
			for ( int i = 0; i < 8; i++ )
			{
				c[ i ] = a[ i ] * b[ i ];
			}

			return c[ 0 ];
		};

		//auto AvxResult = Measures::MeasureLong( AVX );
		//NumberOfRuns = 1945588
		//	auto ScalarResult = Measures::MeasureLong( Scalar );
		//NumberOfRuns = 1910432

		return;
	}

	glm::mat4x4 MultiplyScalar( const glm::mat4x4& M1, const glm::mat4x4& M2 )
	{
		glm::mat4x4 mResult;
		// Cache the invariants in registers
		float x = M1[ 0 ][ 0 ];
		float y = M1[ 0 ][ 1 ];
		float z = M1[ 0 ][ 2 ];
		float w = M1[ 0 ][ 3 ];
		// Perform the operation on the first row
		mResult[ 0 ][ 0 ] = ( M2[ 0 ][ 0 ] * x ) + ( M2[ 1 ][ 0 ] * y ) + ( M2[ 2 ][ 0 ] * z ) + ( M2[ 3 ][ 0 ] * w );
		mResult[ 0 ][ 1 ] = ( M2[ 0 ][ 1 ] * x ) + ( M2[ 1 ][ 1 ] * y ) + ( M2[ 2 ][ 1 ] * z ) + ( M2[ 3 ][ 1 ] * w );
		mResult[ 0 ][ 2 ] = ( M2[ 0 ][ 2 ] * x ) + ( M2[ 1 ][ 2 ] * y ) + ( M2[ 2 ][ 2 ] * z ) + ( M2[ 3 ][ 2 ] * w );
		mResult[ 0 ][ 3 ] = ( M2[ 0 ][ 3 ] * x ) + ( M2[ 1 ][ 3 ] * y ) + ( M2[ 2 ][ 3 ] * z ) + ( M2[ 3 ][ 3 ] * w );
		// Repeat for all the other rows

		x = M1[ 1 ][ 0 ];
		y = M1[ 1 ][ 1 ];
		z = M1[ 1 ][ 2 ];
		w = M1[ 1 ][ 3 ];
		mResult[ 1 ][ 0 ] = ( M2[ 0 ][ 0 ] * x ) + ( M2[ 1 ][ 0 ] * y ) + ( M2[ 2 ][ 0 ] * z ) + ( M2[ 3 ][ 0 ] * w );
		mResult[ 1 ][ 1 ] = ( M2[ 0 ][ 1 ] * x ) + ( M2[ 1 ][ 1 ] * y ) + ( M2[ 2 ][ 1 ] * z ) + ( M2[ 3 ][ 1 ] * w );
		mResult[ 1 ][ 2 ] = ( M2[ 0 ][ 2 ] * x ) + ( M2[ 1 ][ 2 ] * y ) + ( M2[ 2 ][ 2 ] * z ) + ( M2[ 3 ][ 2 ] * w );
		mResult[ 1 ][ 3 ] = ( M2[ 0 ][ 3 ] * x ) + ( M2[ 1 ][ 3 ] * y ) + ( M2[ 2 ][ 3 ] * z ) + ( M2[ 3 ][ 3 ] * w );

		x = M1[ 2 ][ 0 ];
		y = M1[ 2 ][ 1 ];
		z = M1[ 2 ][ 2 ];
		w = M1[ 2 ][ 3 ];
		mResult[ 2 ][ 0 ] = ( M2[ 0 ][ 0 ] * x ) + ( M2[ 1 ][ 0 ] * y ) + ( M2[ 2 ][ 0 ] * z ) + ( M2[ 3 ][ 0 ] * w );
		mResult[ 2 ][ 1 ] = ( M2[ 0 ][ 1 ] * x ) + ( M2[ 1 ][ 1 ] * y ) + ( M2[ 2 ][ 1 ] * z ) + ( M2[ 3 ][ 1 ] * w );
		mResult[ 2 ][ 2 ] = ( M2[ 0 ][ 2 ] * x ) + ( M2[ 1 ][ 2 ] * y ) + ( M2[ 2 ][ 2 ] * z ) + ( M2[ 3 ][ 2 ] * w );
		mResult[ 2 ][ 3 ] = ( M2[ 0 ][ 3 ] * x ) + ( M2[ 1 ][ 3 ] * y ) + ( M2[ 2 ][ 3 ] * z ) + ( M2[ 3 ][ 3 ] * w );

		x = M1[ 3 ][ 0 ];
		y = M1[ 3 ][ 1 ];
		z = M1[ 3 ][ 2 ];
		w = M1[ 3 ][ 3 ];
		mResult[ 3 ][ 0 ] = ( M2[ 0 ][ 0 ] * x ) + ( M2[ 1 ][ 0 ] * y ) + ( M2[ 2 ][ 0 ] * z ) + ( M2[ 3 ][ 0 ] * w );
		mResult[ 3 ][ 1 ] = ( M2[ 0 ][ 1 ] * x ) + ( M2[ 1 ][ 1 ] * y ) + ( M2[ 2 ][ 1 ] * z ) + ( M2[ 3 ][ 1 ] * w );
		mResult[ 3 ][ 2 ] = ( M2[ 0 ][ 2 ] * x ) + ( M2[ 1 ][ 2 ] * y ) + ( M2[ 2 ][ 2 ] * z ) + ( M2[ 3 ][ 2 ] * w );
		mResult[ 3 ][ 3 ] = ( M2[ 0 ][ 3 ] * x ) + ( M2[ 1 ][ 3 ] * y ) + ( M2[ 2 ][ 3 ] * z ) + ( M2[ 3 ][ 3 ] * w );

		return mResult;
	}

	glm::mat4x4 MyMultiplyAVX( const glm::mat4x4& M1, const glm::mat4x4& M2 )
	{
		glm::mat4x4 mResult;

		__m256 a0, a1, b0, b1;
		__m256 c0, c1, c2, c3, c4, c5, c6, c7;
		__m256 t0, t1, u0, u1;

		t0 = _mm256_loadu_ps( &M1[ 0 ][ 0 ] ); // t0 = a00, a01, a02, a03, a10, a11, a12, a13
		t1 = _mm256_loadu_ps( &M1[ 2 ][ 0 ] ); // t1 = a20, a21, a22, a23, a30, a31, a32, a33
		u0 = _mm256_loadu_ps( &M2[ 0 ][ 0 ] ); // u0 = b00, b01, b02, b03, b10, b11, b12, b13
		u1 = _mm256_loadu_ps( &M2[ 2 ][ 0 ] ); // u1 = b20, b21, b22, b23, b30, b31, b32, b33

		a0 = _mm256_shuffle_ps( t0, t0, _MM_SHUFFLE( 0, 0, 0, 0 ) ); // a0 = a00, a00, a00, a00, a10, a10, a10, a10
		a1 = _mm256_shuffle_ps( t1, t1, _MM_SHUFFLE( 0, 0, 0, 0 ) ); // a1 = a20, a20, a20, a20, a30, a30, a30, a30
		b0 = _mm256_permute2f128_ps( u0, u0, 0x00 ); // b0 = b00, b01, b02, b03, b00, b01, b02, b03
		c0 = _mm256_mul_ps( a0, b0 ); // c0 = a00*b00  a00*b01  a00*b02  a00*b03  a10*b00  a10*b01  a10*b02  a10*b03
		c1 = _mm256_mul_ps( a1, b0 ); // c1 = a20*b00  a20*b01  a20*b02  a20*b03  a30*b00  a30*b01  a30*b02  a30*b03

		a0 = _mm256_shuffle_ps( t0, t0, _MM_SHUFFLE( 1, 1, 1, 1 ) ); // a0 = a01, a01, a01, a01, a11, a11, a11, a11
		a1 = _mm256_shuffle_ps( t1, t1, _MM_SHUFFLE( 1, 1, 1, 1 ) ); // a1 = a21, a21, a21, a21, a31, a31, a31, a31
		b0 = _mm256_permute2f128_ps( u0, u0, 0x11 ); // b0 = b10, b11, b12, b13, b10, b11, b12, b13
		c2 = _mm256_mul_ps( a0, b0 ); // c2 = a01*b10  a01*b11  a01*b12  a01*b13  a11*b10  a11*b11  a11*b12  a11*b13
		c3 = _mm256_mul_ps( a1, b0 ); // c3 = a21*b10  a21*b11  a21*b12  a21*b13  a31*b10  a31*b11  a31*b12  a31*b13

		a0 = _mm256_shuffle_ps( t0, t0, _MM_SHUFFLE( 2, 2, 2, 2 ) ); // a0 = a02, a02, a02, a02, a12, a12, a12, a12
		a1 = _mm256_shuffle_ps( t1, t1, _MM_SHUFFLE( 2, 2, 2, 2 ) ); // a1 = a22, a22, a22, a22, a32, a32, a32, a32
		b1 = _mm256_permute2f128_ps( u1, u1, 0x00 ); // b0 = b20, b21, b22, b23, b20, b21, b22, b23
		c4 = _mm256_mul_ps( a0, b1 ); // c4 = a02*b20  a02*b21  a02*b22  a02*b23  a12*b20  a12*b21  a12*b22  a12*b23
		c5 = _mm256_mul_ps( a1, b1 ); // c5 = a22*b20  a22*b21  a22*b22  a22*b23  a32*b20  a32*b21  a32*b22  a32*b23

		a0 = _mm256_shuffle_ps( t0, t0, _MM_SHUFFLE( 3, 3, 3, 3 ) ); // a0 = a03, a03, a03, a03, a13, a13, a13, a13
		a1 = _mm256_shuffle_ps( t1, t1, _MM_SHUFFLE( 3, 3, 3, 3 ) ); // a1 = a23, a23, a23, a23, a33, a33, a33, a33
		b1 = _mm256_permute2f128_ps( u1, u1, 0x11 ); // b0 = b30, b31, b32, b33, b30, b31, b32, b33
		c6 = _mm256_mul_ps( a0, b1 ); // c6 = a03*b30  a03*b31  a03*b32  a03*b33  a13*b30  a13*b31  a13*b32  a13*b33
		c7 = _mm256_mul_ps( a1, b1 ); // c7 = a23*b30  a23*b31  a23*b32  a23*b33  a33*b30  a33*b31  a33*b32  a33*b33

		c0 = _mm256_add_ps( c0, c2 ); // c0 = c0 + c2 (two terms, first two rows)
		c4 = _mm256_add_ps( c4, c6 ); // c4 = c4 + c6 (the other two terms, first two rows)
		c1 = _mm256_add_ps( c1, c3 ); // c1 = c1 + c3 (two terms, second two rows)
		c5 = _mm256_add_ps( c5, c7 ); // c5 = c5 + c7 (the other two terms, second two rose)

		// Finally complete addition of all four terms and return the results
		__m256 r01 = _mm256_add_ps(
			c0,
			c4 ); // n0 = a00*b00+a01*b10+a02*b20+a03*b30  a00*b01+a01*b11+a02*b21+a03*b31  a00*b02+a01*b12+a02*b22+a03*b32  a00*b03+a01*b13+a02*b23+a03*b33
			//      a10*b00+a11*b10+a12*b20+a13*b30  a10*b01+a11*b11+a12*b21+a13*b31  a10*b02+a11*b12+a12*b22+a13*b32  a10*b03+a11*b13+a12*b23+a13*b33
		__m256 r23 = _mm256_add_ps(
			c1,
			c5 ); // n1 = a20*b00+a21*b10+a22*b20+a23*b30  a20*b01+a21*b11+a22*b21+a23*b31  a20*b02+a21*b12+a22*b22+a23*b32  a20*b03+a21*b13+a22*b23+a23*b33

		_mm256_storeu_ps( &mResult[ 0 ][ 0 ], r01 );
		_mm256_storeu_ps( &mResult[ 2 ][ 0 ], r23 );

		return mResult;
	}

	__declspec( align( 16 ) ) struct XMMATRIX
	{
		__m128 r[ 4 ];
	};

	glm::mat4x4 DxMultiplyAVX( const glm::mat4x4 M1, const glm::mat4x4 M2 )
	{
		XMMATRIX _M1;
		XMMATRIX _M2;
		XMMATRIX _Result;

		_M1.r[ 0 ] = _mm_loadu_ps( &M1[ 0 ][ 0 ] );
		_M1.r[ 1 ] = _mm_loadu_ps( &M1[ 1 ][ 0 ] );
		_M1.r[ 2 ] = _mm_loadu_ps( &M1[ 2 ][ 0 ] );
		_M1.r[ 3 ] = _mm_loadu_ps( &M1[ 3 ][ 0 ] );

		_M2.r[ 0 ] = _mm_loadu_ps( &M2[ 0 ][ 0 ] );
		_M2.r[ 1 ] = _mm_loadu_ps( &M2[ 1 ][ 0 ] );
		_M2.r[ 2 ] = _mm_loadu_ps( &M2[ 2 ][ 0 ] );
		_M2.r[ 3 ] = _mm_loadu_ps( &M2[ 3 ][ 0 ] );

		// Splat the component X,Y,Z then W
		__m128 vX = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 0 ] ) + 0 );
		__m128 vY = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 0 ] ) + 1 );
		__m128 vZ = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 0 ] ) + 2 );
		__m128 vW = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 0 ] ) + 3 );
		// Perform the operation on the first row
		vX = _mm_mul_ps( vX, _M2.r[ 0 ] );
		vY = _mm_mul_ps( vY, _M2.r[ 1 ] );
		vZ = _mm_mul_ps( vZ, _M2.r[ 2 ] );
		vW = _mm_mul_ps( vW, _M2.r[ 3 ] );
		// Perform a binary add to reduce cumulative errors
		vX = _mm_add_ps( vX, vZ );
		vY = _mm_add_ps( vY, vW );
		vX = _mm_add_ps( vX, vY );
		_Result.r[ 0 ] = vX;
		// Repeat for the other 3 rows

		vX = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 1 ] ) + 0 );
		vY = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 1 ] ) + 1 );
		vZ = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 1 ] ) + 2 );
		vW = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 1 ] ) + 3 );
		vX = _mm_mul_ps( vX, _M2.r[ 0 ] );
		vY = _mm_mul_ps( vY, _M2.r[ 1 ] );
		vZ = _mm_mul_ps( vZ, _M2.r[ 2 ] );
		vW = _mm_mul_ps( vW, _M2.r[ 3 ] );
		vX = _mm_add_ps( vX, vZ );
		vY = _mm_add_ps( vY, vW );
		vX = _mm_add_ps( vX, vY );
		_Result.r[ 1 ] = vX;

		vX = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 2 ] ) + 0 );
		vY = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 2 ] ) + 1 );
		vZ = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 2 ] ) + 2 );
		vW = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 2 ] ) + 3 );

		vX = _mm_mul_ps( vX, _M2.r[ 0 ] );
		vY = _mm_mul_ps( vY, _M2.r[ 1 ] );
		vZ = _mm_mul_ps( vZ, _M2.r[ 2 ] );
		vW = _mm_mul_ps( vW, _M2.r[ 3 ] );
		vX = _mm_add_ps( vX, vZ );
		vY = _mm_add_ps( vY, vW );
		vX = _mm_add_ps( vX, vY );
		_Result.r[ 2 ] = vX;

		vX = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 3 ] ) + 0 );
		vY = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 3 ] ) + 1 );
		vZ = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 3 ] ) + 2 );
		vW = _mm_broadcast_ss( reinterpret_cast<const float*>( &_M1.r[ 3 ] ) + 3 );

		vX = _mm_mul_ps( vX, _M2.r[ 0 ] );
		vY = _mm_mul_ps( vY, _M2.r[ 1 ] );
		vZ = _mm_mul_ps( vZ, _M2.r[ 2 ] );
		vW = _mm_mul_ps( vW, _M2.r[ 3 ] );
		vX = _mm_add_ps( vX, vZ );
		vY = _mm_add_ps( vY, vW );
		vX = _mm_add_ps( vX, vY );
		_Result.r[ 3 ] = vX;

		glm::mat4x4 mResult;
		_mm_storeu_ps( &mResult[ 0 ][ 0 ], _Result.r[ 0 ] );
		_mm_storeu_ps( &mResult[ 1 ][ 0 ], _Result.r[ 1 ] );
		_mm_storeu_ps( &mResult[ 2 ][ 0 ], _Result.r[ 2 ] );
		_mm_storeu_ps( &mResult[ 3 ][ 0 ], _Result.r[ 3 ] );

		return mResult;
	}

	glm::mat4x4 DxMultiplySSE2( const glm::mat4x4 M1, const glm::mat4x4 M2 )
	{
		XMMATRIX _M1;
		XMMATRIX _M2;
		XMMATRIX _Result;

		_M1.r[ 0 ] = _mm_loadu_ps( &M1[ 0 ][ 0 ] );
		_M1.r[ 1 ] = _mm_loadu_ps( &M1[ 1 ][ 0 ] );
		_M1.r[ 2 ] = _mm_loadu_ps( &M1[ 2 ][ 0 ] );
		_M1.r[ 3 ] = _mm_loadu_ps( &M1[ 3 ][ 0 ] );

		_M2.r[ 0 ] = _mm_loadu_ps( &M2[ 0 ][ 0 ] );
		_M2.r[ 1 ] = _mm_loadu_ps( &M2[ 1 ][ 0 ] );
		_M2.r[ 2 ] = _mm_loadu_ps( &M2[ 2 ][ 0 ] );
		_M2.r[ 3 ] = _mm_loadu_ps( &M2[ 3 ][ 0 ] );

		//float a[] = { 0.0f, 1.0f, 2.0f, 3.0f };
		//
		//_M1.r[0] = _mm_loadu_ps(a);
		//_M1.r[1] = _mm_loadu_ps(a);
		//_M1.r[2] = _mm_loadu_ps(a);
		//_M1.r[3] = _mm_loadu_ps(a);

		//_MM_SHUFFLE works from right to left -> right most parameter is in left most slot

		// Cache the invariants in registers
		__m128 x = _mm_shuffle_ps( _M1.r[ 0 ], _M1.r[ 0 ], _MM_SHUFFLE( 0, 0, 0, 0 ) );
		__m128 y = _mm_shuffle_ps( _M1.r[ 0 ], _M1.r[ 0 ], _MM_SHUFFLE( 1, 1, 1, 1 ) );
		__m128 z = _mm_shuffle_ps( _M1.r[ 0 ], _M1.r[ 0 ], _MM_SHUFFLE( 2, 2, 2, 2 ) );
		__m128 w = _mm_shuffle_ps( _M1.r[ 0 ], _M1.r[ 0 ], _MM_SHUFFLE( 3, 3, 3, 3 ) );

		//Note: can't do this as there are only 16 registers available
		/*__m128 ResultX = _mm_mul_ps(x, _M2.r[0]);
	__m128 ResultY = _mm_mul_ps(y, _M2.r[1]);
	__m128 ResultZ = _mm_mul_ps(z, _M2.r[2]);
	__m128 ResultW = _mm_mul_ps(w, _M2.r[3]);*/
		x = _mm_mul_ps( x, _M2.r[ 0 ] );
		y = _mm_mul_ps( y, _M2.r[ 1 ] );
		z = _mm_mul_ps( z, _M2.r[ 2 ] );
		w = _mm_mul_ps( w, _M2.r[ 3 ] );
		// Perform a binary add to reduce cumulative errors
		x = _mm_add_ps( x, y );
		z = _mm_add_ps( z, w );
		x = _mm_add_ps( x, z );
		_Result.r[ 0 ] = x;

		x = _mm_shuffle_ps( _M1.r[ 1 ], _M1.r[ 1 ], _MM_SHUFFLE( 0, 0, 0, 0 ) );
		y = _mm_shuffle_ps( _M1.r[ 1 ], _M1.r[ 1 ], _MM_SHUFFLE( 1, 1, 1, 1 ) );
		z = _mm_shuffle_ps( _M1.r[ 1 ], _M1.r[ 1 ], _MM_SHUFFLE( 2, 2, 2, 2 ) );
		w = _mm_shuffle_ps( _M1.r[ 1 ], _M1.r[ 1 ], _MM_SHUFFLE( 3, 3, 3, 3 ) );

		x = _mm_mul_ps( x, _M2.r[ 0 ] );
		y = _mm_mul_ps( y, _M2.r[ 1 ] );
		z = _mm_mul_ps( z, _M2.r[ 2 ] );
		w = _mm_mul_ps( w, _M2.r[ 3 ] );
		// Perform a binary add to reduce cumulative errors
		x = _mm_add_ps( x, y );
		z = _mm_add_ps( z, w );
		x = _mm_add_ps( x, z );
		_Result.r[ 1 ] = x;

		x = _mm_shuffle_ps( _M1.r[ 2 ], _M1.r[ 2 ], _MM_SHUFFLE( 0, 0, 0, 0 ) );
		y = _mm_shuffle_ps( _M1.r[ 2 ], _M1.r[ 2 ], _MM_SHUFFLE( 1, 1, 1, 1 ) );
		z = _mm_shuffle_ps( _M1.r[ 2 ], _M1.r[ 2 ], _MM_SHUFFLE( 2, 2, 2, 2 ) );
		w = _mm_shuffle_ps( _M1.r[ 2 ], _M1.r[ 2 ], _MM_SHUFFLE( 3, 3, 3, 3 ) );

		x = _mm_mul_ps( x, _M2.r[ 0 ] );
		y = _mm_mul_ps( y, _M2.r[ 1 ] );
		z = _mm_mul_ps( z, _M2.r[ 2 ] );
		w = _mm_mul_ps( w, _M2.r[ 3 ] );
		// Perform a binary add to reduce cumulative errors
		x = _mm_add_ps( x, y );
		z = _mm_add_ps( z, w );
		x = _mm_add_ps( x, z );
		_Result.r[ 2 ] = x;

		x = _mm_shuffle_ps( _M1.r[ 3 ], _M1.r[ 3 ], _MM_SHUFFLE( 0, 0, 0, 0 ) );
		y = _mm_shuffle_ps( _M1.r[ 3 ], _M1.r[ 3 ], _MM_SHUFFLE( 1, 1, 1, 1 ) );
		z = _mm_shuffle_ps( _M1.r[ 3 ], _M1.r[ 3 ], _MM_SHUFFLE( 2, 2, 2, 2 ) );
		w = _mm_shuffle_ps( _M1.r[ 3 ], _M1.r[ 3 ], _MM_SHUFFLE( 3, 3, 3, 3 ) );

		x = _mm_mul_ps( x, _M2.r[ 0 ] );
		y = _mm_mul_ps( y, _M2.r[ 1 ] );
		z = _mm_mul_ps( z, _M2.r[ 2 ] );
		w = _mm_mul_ps( w, _M2.r[ 3 ] );
		// Perform a binary add to reduce cumulative errors
		x = _mm_add_ps( x, y );
		z = _mm_add_ps( z, w );
		x = _mm_add_ps( x, z );
		_Result.r[ 3 ] = x;

		/*	
	// Cache the invariants in registers
	float x = M1[ 0 ][ 0 ];
	float y = M1[ 0 ][ 1 ];
	float z = M1[ 0 ][ 2 ];
	float w = M1[ 0 ][ 3 ];
	// Perform the operation on the first row
	mResult[ 0 ][ 0 ] = ( M2[ 0 ][ 0 ] * x ) + ( M2[ 1 ][ 0 ] * y ) + ( M2[ 2 ][ 0 ] * z ) + ( M2[ 3 ][ 0 ] * w );
	mResult[ 0 ][ 1 ] = ( M2[ 0 ][ 1 ] * x ) + ( M2[ 1 ][ 1 ] * y ) + ( M2[ 2 ][ 1 ] * z ) + ( M2[ 3 ][ 1 ] * w );
	mResult[ 0 ][ 2 ] = ( M2[ 0 ][ 2 ] * x ) + ( M2[ 1 ][ 2 ] * y ) + ( M2[ 2 ][ 2 ] * z ) + ( M2[ 3 ][ 2 ] * w );
	mResult[ 0 ][ 3 ] = ( M2[ 0 ][ 3 ] * x ) + ( M2[ 1 ][ 3 ] * y ) + ( M2[ 2 ][ 3 ] * z ) + ( M2[ 3 ][ 3 ] * w ); */

		glm::mat4x4 mResult;
		_mm_storeu_ps( &mResult[ 0 ][ 0 ], _Result.r[ 0 ] );
		_mm_storeu_ps( &mResult[ 1 ][ 0 ], _Result.r[ 1 ] );
		_mm_storeu_ps( &mResult[ 2 ][ 0 ], _Result.r[ 2 ] );
		_mm_storeu_ps( &mResult[ 3 ][ 0 ], _Result.r[ 3 ] );
		return mResult;
	}

	void gemm4x4_SSE( const float* A, const float* B, float* C )
	{
		__m128 row[ 4 ], sum[ 4 ];

		for ( int i = 0; i < 4; i++ )
			row[ i ] = _mm_load_ps( &B[ i * 4 ] );

		for ( int i = 0; i < 4; i++ )
		{
			sum[ i ] = _mm_setzero_ps();

			for ( int j = 0; j < 4; j++ )
			{
				sum[ i ] = _mm_add_ps( _mm_mul_ps( _mm_set1_ps( A[ i * 4 + j ] ), row[ j ] ), sum[ i ] );
			}
		}

		for ( int i = 0; i < 4; i++ )
			_mm_store_ps( &C[ i * 4 ], sum[ i ] );
	}

//#define _XM_NO_INTRINSICS_
//#define _XM_MY_SSE_INTRINSICS_
#define _XM_SSE_INTRINSICS_

#define drand48() ( (float)( rand() / ( RAND_MAX + 1.0 ) ) )

	void SetRandom( glm::mat4x4& dest )
	{
		dest = {};

		srand( (unsigned int)time( NULL ) );

		/* random position */
		dest[ 3 ][ 0 ] = drand48();
		dest[ 3 ][ 1 ] = drand48();
		dest[ 3 ][ 2 ] = drand48();

		/* random rotatation around random axis with random angle */
		dest = glm::rotate( dest, drand48(), { drand48(), drand48(), drand48() } );

		/* random scale */
		//dest = glm::scale( dest, { drand48(), drand48(), drand48() } );
	}

	void TestMatrix()
	{
		glm::mat4x4 M1 = {};
		glm::mat4x4 M2 = {};

		M1[ 0 ] = glm::vec4( 16, 15, 14, 13 );
		M1[ 1 ] = glm::vec4( 12, 11, 10, 9 );
		M1[ 2 ] = glm::vec4( 8, 7, 6, 5 );
		M1[ 3 ] = glm::vec4( 4, 3, 2, 1 );

		M2[ 0 ] = glm::vec4( 1, 2, 3, 4 );
		M2[ 1 ] = glm::vec4( 5, 6, 7, 8 );
		M2[ 2 ] = glm::vec4( 9, 10, 11, 12 );
		M2[ 3 ] = glm::vec4( 13, 14, 15, 16 );

		glm::mat4x4 Result2 = MultiplyScalar( M1, M2 );
#if defined( _XM_MY_SSE_INTRINSICS_ )
		glm::mat4x4 Result = MyMultiplyAVX( M1, M2 );
#elif defined( _XM_SSE_INTRINSICS_ )
		glm::mat4x4 Result = DxMultiplyAVX( M1, M2 );
		glm::mat4x4 Result3 = DxMultiplySSE2( M1, M2 );

		glm::mat4x4 Results4;
		gemm4x4_SSE( &M1[ 0 ][ 0 ], &M2[ 0 ][ 0 ], &Results4[ 0 ][ 0 ] );

		glm::mat4x4 Results5;
		gemm4x4_SSE( &M2[ 0 ][ 0 ], &M1[ 0 ][ 0 ], &Results5[ 0 ][ 0 ] );

#endif

		//assert(Result2 == M1);
		assert( Result == Result2 );
		assert( Result == Result3 );
		assert( Result == Results4 );
		//assert(Result == M1);

		SetRandom( M1 );
		SetRandom( M2 );

		auto Noop = Measures::MeasureShort( [ M1, M2 ]() {
			return M1[ 0 ][ 0 ];
		} );

		auto AvxResult = Measures::MeasureShort( [ M1, M2 ]() {
			glm::mat4x4 Result = DxMultiplyAVX( M1, M2 );

			return Result[ 0 ][ 0 ];
		} );
		//NumberOfRuns = 481114
		auto ScalarResult = Measures::MeasureShort( [ M1, M2 ]() {
			glm::mat4x4 Result = MultiplyScalar( M1, M2 );

			return Result[ 0 ][ 0 ];
		} );

		auto GLMResult = Measures::MeasureShort( [ M1, M2 ]() {
			glm::mat4x4 Result = M1 * M2;

			return Result[ 0 ][ 0 ];
		} );

		auto SSEResult = Measures::MeasureShort( [ M1, M2 ]() {
			glm::mat4x4 Result = DxMultiplySSE2( M1, M2 );

			return Result[ 0 ][ 0 ];
		} );

		auto FloatResult = Measures::MeasureShort( [ M1, M2 ]() {
			glm::mat4x4 Result;
			gemm4x4_SSE( &M1[ 0 ][ 0 ], &M2[ 0 ][ 0 ], &Result[ 0 ][ 0 ] );

			return Result[ 0 ][ 0 ];
		} );

		Measures::Print( Noop, "NOOP  " );
		Measures::Print( AvxResult, "AVX   " );
		Measures::Print( GLMResult, "GLM   " );
		Measures::Print( SSEResult, "SSE2  " );
		Measures::Print( ScalarResult, "Scalar" );
		Measures::Print( FloatResult, "Float " );
		//NumberOfRuns = 105193
	}

}