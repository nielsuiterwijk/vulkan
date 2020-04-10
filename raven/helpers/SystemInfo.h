#pragma once

enum class CPUVendor
{
	Unknown,
	Intel,
	AMD,
	Count
};

const char* GetCPUVendorString( CPUVendor CPUVendor );

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class SystemInfo
{
public:
	SystemInfo() = default;
	~SystemInfo() = default;

	const std::string& GetOSName() const;
	const std::string& GetOSLanguage() const;

	const std::string& GetCPUModel() const;

	CPUVendor GetCPUVendor() const;

	uint32_t GetCPUSpeed() const;
	uint32_t GetCPULogicalCores() const;
	uint32_t GetCPUPhysicalCores() const;

	uint32_t GetRAMTotal() const;

	static void FillInfoFromSystem( SystemInfo& OutInfo );

public:
	std::string _OSName;
	std::string _OSLanguage;
	std::string _CPUModel;

	uint32_t _CPUSpeed = -1;
	uint32_t _CPULogicalCores = -1;
	uint32_t _CPUPhysicalCores = -1;

	uint32_t _RAMTotal = -1;
	uint32_t _PageAllocationSize = -1;
	
	CPUVendor _CPUVendor = CPUVendor::Unknown;
};
