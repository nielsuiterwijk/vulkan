#pragma once

//Implementation from: http://szelei.me/constexpr-murmurhash/
namespace Murmur3
{
	namespace Impl
	{
		constexpr uint32_t GetBlock( const char* pInput, uint32_t Length, uint32_t Index )
		{
			uint32_t i = ( ( Length / 4 ) + Index ) * 4;
			uint32_t b0 = pInput[ i ];
			uint32_t b1 = pInput[ i + 1 ];
			uint32_t b2 = pInput[ i + 2 ];
			uint32_t b3 = pInput[ i + 3 ];
			return ( b3 << 24 ) | ( b2 << 16 ) | ( b1 << 8 ) | b0;
		}

		constexpr char Tail( const char* pInput, uint32_t Length, const int Offset )
		{
			int32_t TailSize = Length % 4;
			return pInput[ Length - TailSize + Offset ];
		}
	}
	constexpr uint32_t Hash( const char* pInput, uint32_t Length, uint32_t Seed = 0 )
	{
		uint32_t h1 = Seed;

		const uint32_t c1 = 0xcc9e2d51;
		const uint32_t c2 = 0x1b873593;

		const int32_t nblocks = Length / 4;
		for ( int32_t i = -nblocks; i; i++ )
		{
			uint32_t k1 = Impl::GetBlock( pInput, Length, i );

			k1 *= c1;
			k1 = ( k1 << 15 ) | ( k1 >> ( 32 - 15 ) );
			k1 *= c2;

			h1 ^= k1;
			h1 = ( h1 << 13 ) | ( h1 >> ( 32 - 13 ) );
			h1 = h1 * 5 + 0xe6546b64;
		}

		uint32_t k1 = 0;
		char t = Impl::Tail( pInput, Length, 0 );

		switch ( Length & 3 )
		{
			case 3:
				k1 ^= Impl::Tail( pInput, Length, 2 ) << 16;
			case 2:
				k1 ^= Impl::Tail( pInput, Length, 1 ) << 8;
			case 1:
				k1 ^= Impl::Tail( pInput, Length, 0 );
				k1 *= c1;
				k1 = ( k1 << 15 ) | ( k1 >> ( 32 - 15 ) );
				k1 *= c2;
				h1 ^= k1;
		};

		h1 ^= Length;

		h1 ^= h1 >> 16;
		h1 *= 0x85ebca6b;
		h1 ^= h1 >> 13;
		h1 *= 0xc2b2ae35;
		h1 ^= h1 >> 16;

		return h1;
	}
	   
	constexpr uint32_t Hash( std::string_view Input, uint32_t Seed = 0 ) { return Hash( Input.data(), static_cast<uint32_t>( Input.size() ), Seed ); }

	static_assert( Hash( "Hello World" ) == 427197390 );
}