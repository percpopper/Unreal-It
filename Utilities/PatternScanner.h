#pragma once

/*	
	https://github.com/N-T33/PatternScanner
	Credits for the pattern scanner (tysm):
	lguilhermee - https://github.com/lguilhermee/Discord-DX11-Overlay-Hook/blob/master/Helper/Helper.cpp
	guttir14 - https://github.com/guttir14/CheatIt/blob/main/CheatIt/utils.cpp
*/

static std::vector<int> PatternToIntVector(const char* Pattern)
{
	std::vector<int> Bytes = std::vector<int>{};

	char* Start = const_cast<char*>(Pattern);
	char* End = const_cast<char*>(Pattern) + strlen(Pattern);

	for (char* Current = Start; Current < End; ++Current)
	{
		if (*Current == '?')
		{
			++Current;
			Bytes.push_back(-69);
		}
		else { 
			Bytes.push_back(strtoul(Current, &Current, 16)); 
		}
	}
	return Bytes;
}

inline static void* FindPointer(std::vector<int> Pattern, uint8* Address, int Addition = 0) {
	int i = 0;
	while (Pattern[i] != -69) i++;
	int Offset = *reinterpret_cast<int*>(Address + i);
	return reinterpret_cast<void*>(Address + i + 4 + Offset + Addition);
}

template<typename T>
static T PatternScan(const char* Pattern, uint64 Start, size_t Size, bool bFindPointer = false, int Addition = 0)
{
	std::vector<int> PatternVector = PatternToIntVector(Pattern);

	uint8* Search = reinterpret_cast<uint8*>(Start);

	const size_t SizeOfPattern = PatternVector.size();
	const int* PatternData = PatternVector.data();

	for (int i = 0; i < Size - SizeOfPattern; ++i)
	{
		bool FoundSignature = true;

		for (int j = 0; j < SizeOfPattern; ++j)
		{
			if (Search[i + j] != PatternData[j] && PatternData[j] != -69)
			{
				FoundSignature = false;
				break;
			}
		}

		if (FoundSignature) {

			if (bFindPointer) {
				return reinterpret_cast<T>(FindPointer(PatternVector, &Search[i], Addition));
			}

			return reinterpret_cast<T>(&Search[i]);

		}
	}

	return reinterpret_cast<T>(nullptr);
}

static int ScanVTable(std::string Pattern, void* Object, int SkipAmount = 0) {

	int Out_Index = 0;

	int FunctionCount = 0;

	void** VTable = *reinterpret_cast<void***>(Object);

	while (VTable[FunctionCount]) FunctionCount++;

	for (int Index = SkipAmount; Index <= FunctionCount; Index++) {

		void* Result = PatternScan<void*>(Pattern.c_str(), reinterpret_cast<uint64>(VTable[Index]), Pattern.size());
		if (!Result) continue;

		Out_Index = Index;

		break;
	}

	return Out_Index;
}