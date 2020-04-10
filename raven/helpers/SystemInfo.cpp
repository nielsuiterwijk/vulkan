#include "SystemInfo.h"

#include <intrin.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#pragma warning( disable : 4191 ) // unsafe conversion from 'FARPROC'

namespace
{
	void CountPhysicalCores( uint32_t& nOutPhysical )
	{
		typedef BOOL( WINAPI * LPFN_GETLOGICALPROCESSORINFORMATION )( PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD );
		LPFN_GETLOGICALPROCESSORINFORMATION fnGetLogicalProcessorInformation = reinterpret_cast<LPFN_GETLOGICALPROCESSORINFORMATION>(
			GetProcAddress( GetModuleHandle( TEXT( "kernel32" ) ), "GetLogicalProcessorInformation" ) );

		if ( NULL != fnGetLogicalProcessorInformation )
		{
			DWORD nReturnLength = 0;

			if ( fnGetLogicalProcessorInformation( 0, &nReturnLength ) == FALSE ) //Should fail
			{
				if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
				{
					PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pBuffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc( nReturnLength );
					if ( fnGetLogicalProcessorInformation( pBuffer, &nReturnLength ) == TRUE )
					{
						nOutPhysical = 0;
						const uint32_t iMax = nReturnLength / sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION );
						for ( uint32_t i = 0; i < iMax; ++i )
						{
							if ( pBuffer[ i ].Relationship == RelationProcessorCore )
							{
								++nOutPhysical;
							}
						}
					}
					free( pBuffer );
				}
			}
		}
	}

	uint32_t ReadProcessorMHzFromRegistry()
	{
		HKEY hKey;
		long lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey );

		if ( lError != ERROR_SUCCESS )
		{
			//LOG_DEBUG( "Failed to read processor frequency." );
			return 0;
		}

		DWORD MHz = 0;
		DWORD DataSize = sizeof( DWORD );
		lError = RegGetValue( hKey, NULL, L"~Mhz", RRF_RT_DWORD, NULL, (PVOID)&MHz, &DataSize );

		if ( lError != ERROR_SUCCESS )
		{
			//LOG_DEBUG( "Failed to get value for processor frequency." );
			return 0;
		}

		return MHz;
	}

	std::string GetWindowsVersion()
	{
		// Attempt to load the RtlGetVersion function from ntdll directly so that we can bypass needing a manifest file
		// in our Windows 8 or Windows 10 application and still get the current version details.

		// We should probably "do it the right way" and add a manifest file to our application at some point as we
		// will likely need it for some other functions too.

#define NT_STATUS_SUCCESS ( 0x00000000 )

		typedef LONG NTSTATUS;
		typedef NTSTATUS( WINAPI * RtlGetVersionPtr )( PRTL_OSVERSIONINFOW );

		HMODULE hMod = ::GetModuleHandleW( L"ntdll.dll" );
		if ( hMod == nullptr )
		{
			//LOG_DEBUG( "Failed to get Windows Version (GetModuleHandle failed)" );
			return "Unknown";
		}

		RtlGetVersionPtr fxPtr = reinterpret_cast<RtlGetVersionPtr>( ::GetProcAddress( hMod, "RtlGetVersion" ) );

		if ( fxPtr == nullptr )
		{
			//LOG_DEBUG( "Failed to get Windows Version (GetProcAddress failed)" );
			return "Unknown";
		}

		RTL_OSVERSIONINFOW rovi = { 0 };
		rovi.dwOSVersionInfoSize = sizeof( rovi );

		if ( NT_STATUS_SUCCESS != fxPtr( &rovi ) )
		{
			//LOG_DEBUG( "Failed to get Windows Version (GetVersion failed)" );
			return "Unknown";
		}

		if ( rovi.dwMajorVersion == 10 )
		{
			return "Windows 10";
		}
		else if ( ( rovi.dwMajorVersion == 6 ) && ( rovi.dwMinorVersion == 3 ) )
		{
			return "Windows 8.1";
		}
		else if ( ( rovi.dwMajorVersion == 6 ) && ( rovi.dwMinorVersion == 2 ) )
		{
			return "Windows 8";
		}
		else if ( ( rovi.dwMajorVersion == 6 ) && ( rovi.dwMinorVersion == 1 ) )
		{
			return "Windows 7";
		}

		return "Unknown";
	}

	void FillOSInfo( std::string& OutName, std::string& OutLanguage )
	{
		OutName = GetWindowsVersion();
		OutName += " x86_64";

		// Language
		constexpr int32_t MaxLength = 10;
		TCHAR szName[ MaxLength ];
		if ( 0 != GetLocaleInfo( MAKELCID( GetSystemDefaultUILanguage(), SORT_DEFAULT ), LOCALE_SISO639LANGNAME, szName, MaxLength ) )
		{
			size_t outputSize = MaxLength + 1; // +1 for null terminator
			char* outputString = new char[ outputSize ];
			size_t charsConverted = 0;
			const wchar_t* inputW = &szName[ 0 ];
			wcstombs_s( &charsConverted, outputString, outputSize, inputW, MaxLength );

			OutLanguage = std::string( outputString );
			delete[] outputString;
		}
	}

	void FillCPUInfo( SystemInfo& OutInfo )
	{
		OutInfo._CPUSpeed = ReadProcessorMHzFromRegistry();

		SYSTEM_INFO SysInfo;
		memset( &SysInfo, 0, sizeof( SYSTEM_INFO ) );
		GetNativeSystemInfo( &SysInfo ); // Required for WOW64 mode (32-bit exe on 64-bit OS), but functions the same as GetSystemInfo on 64-bit CPUs

		OutInfo._PageAllocationSize = SysInfo.dwPageSize;

		OutInfo._CPULogicalCores = SysInfo.dwNumberOfProcessors;
		OutInfo._CPUPhysicalCores = OutInfo._CPULogicalCores; //Fallback

		CountPhysicalCores( OutInfo._CPUPhysicalCores );

		int CPUInfo[ 4 ] = { -1 };
		char CPUBrandString[ 0x40 ];
		__cpuid( CPUInfo, 0x80000000 );
		const int nExtIds = CPUInfo[ 0 ];

		memset( CPUBrandString, 0, sizeof( CPUBrandString ) );

		// Get the information associated with each extended ID.
		if ( nExtIds >= 0x80000004 )
		{
			__cpuid( CPUInfo, 0x80000002 );
			memcpy( CPUBrandString, CPUInfo, sizeof( CPUInfo ) );
			__cpuid( CPUInfo, 0x80000003 );
			memcpy( CPUBrandString + 16, CPUInfo, sizeof( CPUInfo ) );
			__cpuid( CPUInfo, 0x80000004 );
			memcpy( CPUBrandString + 32, CPUInfo, sizeof( CPUInfo ) );
			OutInfo._CPUModel = CPUBrandString;
		}

		std::string CPUBrandUpper = CPUBrandString;
		std::for_each( CPUBrandUpper.begin(), CPUBrandUpper.end(), []( char& c ) {
			c = ::toupper( c );
		} );

		if ( CPUBrandUpper.find( "INTEL" ) != -1 )
			OutInfo._CPUVendor = CPUVendor::Intel;
		else if ( CPUBrandUpper.find( "AMD" ) != -1 )
			OutInfo._CPUVendor = CPUVendor::AMD;
	}

	void FillRAMInfo( uint32_t& nOutTotal )
	{
		MEMORYSTATUSEX ms;
		ms.dwLength = sizeof( ms );
		if ( GlobalMemoryStatusEx( &ms ) != 0 )
		{
			nOutTotal = static_cast<uint32_t>( ms.ullTotalPhys / ( 1024 * 1024 ) );
		}
	}
}

void SystemInfo::FillInfoFromSystem( SystemInfo& OutInfo )
{
	FillOSInfo( OutInfo._OSName, OutInfo._OSLanguage );
	FillCPUInfo( OutInfo );
	FillRAMInfo( OutInfo._RAMTotal );
}
