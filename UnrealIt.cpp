#pragma once
#include <UnrealEngine/UE.h>

/** Credit to UnrealEngine's cute and sexy employees */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call != DLL_PROCESS_ATTACH) return FALSE;

	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	HMODULE GameModule = GetModuleHandleA(NULL);

	if (!GameModule) return FALSE;
		
	if (!Initalize(GameModule)) return FALSE;

	GUObjectArray->ObjObjects.Log();

	NamePoolData->Entries.Log();

    return TRUE;
}