#pragma once
#include <Windows.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "portable_executable.hpp"
#include "utils.hpp"
#include "nt.hpp"
#include "intel_driver.hpp"
#include "driver_image.hpp"

#define PAGE_SIZE 0x1000

namespace kdmapper
{
	HANDLE iqvw64e_device_handle;

	typedef bool (*mapCallback)(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize, ULONG64 mdlptr);

	//Note: if you set PassAllocationAddressAsFirstParam as true, param1 will be ignored
	uint64_t MapDriver(HANDLE iqvw64e_device_handle, BYTE* data, ULONG64 param1 = 0, ULONG64 param2 = 0, bool free = false, bool destroyHeader = true, bool mdlMode = false, bool PassAllocationAddressAsFirstParam = false, mapCallback callback = nullptr, NTSTATUS* exitCode = nullptr);
	void RelocateImageByDelta(portable_executable::vec_relocs relocs, const uint64_t delta);
	bool ResolveImports(HANDLE iqvw64e_device_handle, portable_executable::vec_imports imports);
	uint64_t AllocMdlMemory(HANDLE iqvw64e_device_handle, uint64_t size, uint64_t* mdlPtr);

	LONG WINAPI SimplestCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
	{
		if (iqvw64e_device_handle)
			intel_driver::Unload(iqvw64e_device_handle);

		return EXCEPTION_EXECUTE_HANDLER;
	}

	bool callbackExample(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize, ULONG64 mdlptr) {
		UNREFERENCED_PARAMETER(param1);
		UNREFERENCED_PARAMETER(param2);
		UNREFERENCED_PARAMETER(allocationPtr);
		UNREFERENCED_PARAMETER(allocationSize);
		UNREFERENCED_PARAMETER(mdlptr);
	
		return true;
	}

	
}