#pragma once
#include <UnrealEngine/UE.h>

FNamePool* NamePoolData = nullptr;
FUObjectArray* GUObjectArray = nullptr;
void* EngineVersionString = nullptr;

std::string GetEngineVersion() {
	if (EngineVersionString) {
		WIDECHAR Copy[64];
		memcpy(Copy, EngineVersionString, 64);
		std::wstring WideString(Copy, std::wcslen(Copy));
		return std::string(WideString.begin(), WideString.end());
	}
	else {
		return std::string("Unknown Engine Version");
	}
}

bool Initalize(HMODULE GameModule)
{
	const uint64 BaseAddress = reinterpret_cast<uint64>(GameModule);

	const IMAGE_DOS_HEADER* DOSHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(GameModule);
	const IMAGE_NT_HEADERS* NtHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<long long>(GameModule) + DOSHeader->e_lfanew);
	
	const DWORD SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;


	// Search for "++UE", you will likely end up with a wide string "++UEx+Release-x.xx"
	EngineVersionString = PatternScan<void*>("2B ? 2B ? 55 ? 45 ?", BaseAddress, SizeOfImage); 
	if (!EngineVersionString) {
		printf("[FAIL] Engine version was not found by string, you can proceed anyways.\n");
	}


	GUObjectArray = PatternScan<FUObjectArray*>("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 8D ? ? ? ? E8 ? ? ? ? 48 8D 8D", BaseAddress, SizeOfImage, true);
	if (!GUObjectArray) {
		printf("[FAIL] GUObjectArray not found.\n");
		return false;
	}


	NamePoolData = PatternScan<FNamePool*>("48 8D 0D ? ? ? ? E8 ? ? ? ? C6 05 ? ? ? ? 01 0F 10", BaseAddress, SizeOfImage, true);
	if (!NamePoolData) {
		printf("[FAIL] NamePoolData not found.\n");
		return false;
	}


	WIDECHAR GameFileDir[MAX_PATH];
	GetModuleFileNameW(NULL, GameFileDir, MAX_PATH);

	printf("[INFO] Game Directory -> %ws\n\n", GameFileDir);

	printf("[INFO] Game Module Base Address -> %llX \n\n", BaseAddress);

	printf("[INFO] Size Of Game Module -> 0x%X\n\n", SizeOfImage);

	printf("[INFO] Engine Version: %s\n\n", GetEngineVersion().c_str());

	printf("[INFO] FUObjectArray* GUObjectArray found -> Base Address + 0x%llX\n\n", reinterpret_cast<uint64>(GUObjectArray) - BaseAddress);

	printf("[INFO] FNamePool* NamePoolData found -> Base Address + 0x%llX\n\n", reinterpret_cast<uint64>(NamePoolData) - BaseAddress);

	return true;
}