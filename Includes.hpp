#pragma once
#define WIN32_LEAN_AND_MEAN

/*-------Windows-------*/
#include <windows.h>
#include <winternl.h>
#include <iostream>
#include <TlHelp32.h>
#include <thread>
#include <vector>
#include <array>
#include <mutex>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <codecvt>
#include <algorithm>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <urlmon.h>
#include <mmsystem.h>

#pragma comment(lib, "urlmon.lib")

inline auto ValidPointer(uintptr_t pointer) -> bool
{
    return (pointer && pointer > 0xFFFFFF && pointer < 0x7FFFFFFFFFFF);
}


/*-----Driver-----*/

/*-------KdMapper-------*/
#include "kdmapper.hpp"

/*-----Misc-----*/
#include "Driver.hpp"
#include "Offsets.hpp"
#include "Global.hpp"
#include "Players.hpp"














