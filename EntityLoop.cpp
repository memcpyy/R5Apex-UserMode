#include "Includes.hpp"

int EntityLoop()
{
	std::mutex thread_lock;

	thread_lock.lock();
	while (true)
	{
		

		if (ValidPointer(globals.LocalPlayer))
			continue;

		for (int i = 0; i < 100; i++) { 
			DWORD64 Entity = Aplayers.GetEntityById(i, globals.BaseAddress);
			if (Entity == 0) 
				continue;
			DWORD64 EntityHandle = rDriver.Read<DWORD64>(Entity + 0x500);

			std::string Identifier = rDriver.ReadString(EntityHandle);
			LPCSTR IdentifierC = Identifier.c_str();

			if (strcmp(IdentifierC, "player")) 
				Aplayers.HighlightEnable(Entity, 120.f, 0.f, 0.f);
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}
	thread_lock.unlock();

	return -1;
}