#pragma once

//using namespace Offsets;

class ApexPlayers
{
public:

	

	DWORD64 GetEntityById(int Ent, DWORD64 Base)
	{

		DWORD64 EntityList = Base + 0x1b2e708; //Testing
		DWORD64 BaseEntity = rDriver.Read<DWORD64>(EntityList); 
		if (!BaseEntity) 
			return NULL;

		return rDriver.Read<DWORD64>(EntityList + (Ent << 5));
	}
	void HighlightEnable(DWORD64 Entity, float r, float g, float b) {

		

		rDriver.Write<bool>(Entity + Offsets::main.glow_enable, true);
		rDriver.Write<int>(Entity + Offsets::main.glow_type + 4, 1);
		rDriver.Write<float>(Entity + Offsets::main.glow_color, r);
		rDriver.Write<float>(Entity + Offsets::main.glow_color + 4, g);
		rDriver.Write<float>(Entity + Offsets::main.glow_color + 8, b); 

		for (int offset = 0x2B0; offset <= 0x2C8; offset += 0x4)
			rDriver.Write<float>(Entity + offset, FLT_MAX);
		
		rDriver.Write<float>(Entity + 0x2DC, FLT_MAX);
	}

	
}Aplayers;