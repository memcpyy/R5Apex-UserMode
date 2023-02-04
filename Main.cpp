#include "Includes.hpp"

bool DriverInit()
{
    for (int i = 0; i < 2; i++)
    {
        SetUnhandledExceptionFilter(kdmapper::SimplestCrashHandler);

        kdmapper::iqvw64e_device_handle = intel_driver::Load();

        if (kdmapper::iqvw64e_device_handle == INVALID_HANDLE_VALUE) return -1;

        if (!kdmapper::MapDriver(kdmapper::iqvw64e_device_handle, raw_image.data(), 0, 0, false, true, false, false, kdmapper::callbackExample, 0))
        {
            intel_driver::Unload(kdmapper::iqvw64e_device_handle);
            return -1;
        }
        intel_driver::Unload(kdmapper::iqvw64e_device_handle);
        return 1;
    }
}

int main()
{
    std::mutex thread_lock;
    thread_lock.lock();
   
    if (!FindWindow(NULL, L"Apex Legends") && DriverInit())
    {
        Client::Connect();
        Sleep(15000); //Sleeping for 15seconds

    }
    else
    {
        printf("[!] APEX FOUND OPEN WHILE MAPPING DRIVER CLOSE APEX BEFORE\n");
        Sleep(2500);

    }

    globals.PID = rDriver.GetProcessID(L"r5apex.exe");
    globals.BaseAddress = rDriver.GetBaseAddress("r5apex.exe");
    globals.LocalPlayer = rDriver.Read<uintptr_t>(globals.BaseAddress + Offsets::main.local_player);

    
     printf("[+] PID FOUND -> %d", globals.PID);
     printf("[+] PID FOUND -> %p", globals.BaseAddress);
     printf("[+] PID FOUND -> %p", globals.BaseAddress);
    
   

    if (globals.BaseAddress)
    {
        EntityLoop();
    }
   
    
    thread_lock.unlock();
    Client::Disconnect();
    return -1;
}


