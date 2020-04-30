#pragma once

namespace StringUtils
{

	template <int32_t Size>
	struct StringFormat
	{
		template <typename... T>
		StringFormat( const char* pInput, const T&... Args )
		{
			Format( pInput, Args... );
		}

		char _Buffer[ Size ];

		operator const char*() { return _Buffer; }
		const char* c_str() const { return _Buffer; }

	private:
		void Format( const char* pInput, ... )
		{
			va_list Args;
			va_start( Args, pInput );
			Vsnprintf( _Buffer, Size, pInput, Args );
			va_end( Args );
		}
	};

	template <typename... T>
	int32_t Snprintf( char* pBuffer, int32_t Size, const char* pInput, const T&... Args )
	{
		int32_t CharsWritten = snprintf( pBuffer, Size, pInput, Args... );

		ASSERT( CharsWritten >= 0 && CharsWritten < Size );

		return CharsWritten;
	}

	template <typename... T>
	int32_t Vsnprintf( char* pBuffer, int32_t Size, const char* pInput, const T&... Args )
	{
		int32_t CharsWritten = vsnprintf( pBuffer, Size, pInput, Args... );

		ASSERT( CharsWritten >= 0 && CharsWritten < Size );

		return CharsWritten;
	}
}