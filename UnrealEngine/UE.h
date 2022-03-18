#pragma once

#include <Windows.h>
#include <Utilities/Types.h>
#include <Utilities/Log.h>

// #define WITH_CASE_PRESERVING_NAME

#include <UnrealEngine/FNamePool/FNamePool.h>
#include <UnrealEngine/UObject/CoreUObject.h>
#include <UnrealEngine/FUObjectArray/FUObjectArray.h>
#include <Utilities/PatternScanner.h> 

extern FNamePool* NamePoolData;
extern FUObjectArray* GUObjectArray;
extern void* EngineVersionString;

bool Initalize(HMODULE GameModule);

