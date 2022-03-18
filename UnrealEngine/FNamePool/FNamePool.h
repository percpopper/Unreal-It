#pragma once

template <typename T>
FORCEINLINE constexpr T Align(T Val, uint64 Alignment)
{
	return (T)(((uint64)Val + Alignment - 1) & ~(Alignment - 1));
}


static constexpr uint32 FNameMaxBlockBits = 13;
static constexpr uint32 FNameBlockOffsetBits = 16;
static constexpr uint32 FNameMaxBlocks = 1 << FNameMaxBlockBits;
static constexpr uint32 FNameBlockOffsets = 1 << FNameBlockOffsetBits;


/** Implementation detail exposed for debug visualizers */
struct FNameEntryHeader
{
	uint16 bIsWide : 1;
#ifdef WITH_CASE_PRESERVING_NAME
	uint16 Len : 15;
#else
	static constexpr uint32 ProbeHashBits = 5;
	uint16 LowercaseProbeHash : ProbeHashBits;
	uint16 Len : 10;
#endif
};


/** A global deduplicated name stored in the global name table. */
struct FNameEntry {
	enum { NAME_SIZE = 1024 };
public:

	FORCEINLINE bool IsWide() const
	{
		return Header.bIsWide;
	}

	FORCEINLINE int32 GetNameLength() const
	{
		return Header.Len;
	}

	FORCEINLINE FNameEntryHeader GetEntryHeader() const
	{
		return Header;
	}

	FORCEINLINE void CopyUnterminatedName(ANSICHAR* Out) const
	{
		memcpy(Out, AnsiName, sizeof(ANSICHAR) * Header.Len);
	}

	FORCEINLINE void CopyUnterminatedName(WIDECHAR* Out) const
	{
		memcpy(Out, WideName, sizeof(WIDECHAR) * Header.Len);
	}

	FORCEINLINE static int32 GetDataOffset()
	{
		return offsetof(FNameEntry, AnsiName);
	}

	static int32 GetSize(int32 Length, bool bIsPureAnsi);

	void GetAnsiName(ANSICHAR(&Out)[NAME_SIZE]) const;

	void GetWideName(WIDECHAR(&Out)[NAME_SIZE]) const;

	std::string String();

private:

#ifdef WITH_CASE_PRESERVING_NAME
	FNameEntryId ComparisonId;
#endif

	FNameEntryHeader Header;

	union
	{
		ANSICHAR	AnsiName[NAME_SIZE];
		WIDECHAR	WideName[NAME_SIZE];
	};

};


/** Opaque id to a deduplicated name */
struct FNameEntryId
{
	FNameEntryId() : Value(0) {}

	FNameEntryId(uint32 Id) : Value(Id) {}

	explicit operator bool() const { return Value != 0; }

	FORCEINLINE static FNameEntryId FromUnstableInt(uint32 UnstableInt)
	{
		FNameEntryId Id;
		Id.Value = UnstableInt;
		return Id;
	}

	FORCEINLINE uint32 ToUnstableInt() const {
		return Value;
	}

	uint32 Value;
};


/** An unpacked FNameEntryId */
struct FNameEntryHandle
{
	uint32 Block = 0;
	uint32 Offset = 0;

	FNameEntryHandle(uint32 InBlock, uint32 InOffset)
		: Block(InBlock)
		, Offset(InOffset)
	{}

	FNameEntryHandle(FNameEntryId Id)
		: Block(Id.ToUnstableInt() >> FNameBlockOffsetBits)
		, Offset(Id.ToUnstableInt()& (FNameBlockOffsets - 1))
	{}

	operator FNameEntryId() const
	{
		return FNameEntryId::FromUnstableInt(Block << FNameBlockOffsetBits | Offset);
	}

	static uint32 GetTypeHash(FNameEntryHandle Handle)
	{
		return (Handle.Block << (32 - FNameMaxBlockBits)) + Handle.Block
			+ (Handle.Offset << FNameBlockOffsetBits) + Handle.Offset
			+ (Handle.Offset >> 4);
	}

	uint32 GetTypeHash(FNameEntryId Id)
	{
		return GetTypeHash(FNameEntryHandle(Id));
	}

	explicit operator bool() const { return Block | Offset; }
};


/** Paged FNameEntry allocator */
class FNameEntryAllocator
{
public:
	enum { Stride = alignof(FNameEntry) };
	enum { BlockSizeBytes = Stride * FNameBlockOffsets };

	void* FRWLock;
	uint32 CurrentBlock = 0;
	uint32 CurrentByteCursor = 0;
	uint8* Blocks[FNameMaxBlocks] = {};

	FORCEINLINE uint32 NumBlocks() const
	{
		return CurrentBlock + 1;
	}

	FORCEINLINE FNameEntry& Resolve(FNameEntryHandle Handle) const
	{
		if (Handle.Offset < 0 && Handle.Block > NumBlocks() && Handle.Offset * Stride < FNameBlockOffsets) {
			printf("[FAIL] Invalid FNameEntryHandle Passed To FNameEntryAllocator::Resolve.\n");
			return *reinterpret_cast<FNameEntry*>(Blocks[0] + Stride * 0);
		}

		return *reinterpret_cast<FNameEntry*>(Blocks[Handle.Block] + Stride * Handle.Offset);
	}

	void LogBlock(uint8* It, uint32 BlockSize, Logger& LOG);

	void Log();
};


class FNamePool
{
public:
	FNameEntryAllocator Entries;

	/*
	* Not interested in other members.
	* Complete from /Engine/Source/Runtime/Core/Private/UObject/UnrealNames.cpp if neccesary.
	*/
};


/**
 * Public name, available to the world.  Names are stored as a combination of
 * an index into a table of unique strings and an instance number.
 * Names are case-insensitive, but case-preserving (when WITH_CASE_PRESERVING_NAME is 1)
 */
struct FName {

	/** Index into the Names array (used to find String portion of the string/number pair used for comparison) */
	FNameEntryId	ComparisonIndex;

#ifdef WITH_CASE_PRESERVING_NAME
	/** Index into the Names array (used to find String portion of the string/number pair used for display) */
	FNameEntryId	DisplayIndex;
#endif

	/** Number portion of the string/number pair (stored internally as 1 more than actual, so zero'd memory will be the default, no-instance case) */
	uint32			Number;

	FORCEINLINE FNameEntryId GetComparisonIndex() const
	{
		return ComparisonIndex;
	}

	FORCEINLINE FNameEntryId GetDisplayIndex() const
	{
#ifdef WITH_CASE_PRESERVING_NAME
		return DisplayIndex;
#else
		return ComparisonIndex;
#endif
	}

	FORCEINLINE int32 GetNumber() const
	{
		return Number;
	}

#ifndef WITH_CASE_PRESERVING_NAME
	FORCEINLINE uint64 ToComparableInt() const
	{
		static_assert(sizeof(*this) == sizeof(uint64), "");
		alignas(uint64) FName AlignedCopy = *this;
		return reinterpret_cast<uint64&>(AlignedCopy);
	}
#endif

	FORCEINLINE bool operator==(FName Other) const
	{
#ifndef WITH_CASE_PRESERVING_NAME
		return ToComparableInt() == Other.ToComparableInt();
#else
		return (ComparisonIndex == Other.ComparisonIndex) & (GetNumber() == Other.GetNumber());
#endif
	}

	FORCEINLINE bool operator!=(FName Other) const
	{
		return !(*this == Other);
	}

	FName() :
		ComparisonIndex(FNameEntryId()),
		Number(0)
	{ }

	FName(int32 i, int32 n = 0) :
		ComparisonIndex(FNameEntryId(i)),
		Number(n)
	{ }
	
	// To do: Find FName from name

	const FNameEntry* GetDisplayNameEntry() const;

	std::string GetName();
};