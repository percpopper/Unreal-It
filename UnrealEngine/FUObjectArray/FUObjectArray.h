#pragma once

enum EInternalObjectFlags
{
	None = 0,
	ReachableInCluster = 1 << 23,
	ClusterRoot = 1 << 24,
	Native = 1 << 25,
	Async = 1 << 26,
	AsyncLoading = 1 << 27,
	Unreachable = 1 << 28,
	PendingKill = 1 << 29,
	RootSet = 1 << 30,
	GarbageCollectionKeepFlags = Native | Async | AsyncLoading,
	AllFlags = ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet,
};


struct FUObjectItem
{
	// Pointer to the allocated object
	class UObject* Object;

	// Internal flags
	int32 Flags;

	// UObject Owner Cluster Index
	int32 ClusterRootIndex;

	// Weak Object Pointer Serial number associated with the object
	int32 SerialNumber;
};


class FChunkedFixedUObjectArray
{
	enum
	{
		NumElementsPerChunk = 64 * 1024,
	};

	/** Master table to chunks of pointers **/
	FUObjectItem** Objects;

	/** If requested, a contiguous memory where all objects are allocated **/
	FUObjectItem* PreAllocatedObjects;

	/** Maximum number of elements **/
	int32 MaxElements;

	/** Number of elements we currently have **/
	int32 NumElements;

	/** Maximum number of chunks **/
	int32 MaxChunks;

	/** Number of chunks we currently have **/
	int32 NumChunks;

public:

	FORCEINLINE int32 Num() const
	{
		return NumElements;
	}

	FORCEINLINE int32 Capacity() const
	{
		return MaxElements;
	}

	FORCEINLINE bool IsValidIndex(int32 Index) const
	{
		return Index < Num() && Index >= 0;
	}

	FORCEINLINE FUObjectItem const* GetObjectPtr(int32 Index) const
	{
		const int32 ChunkIndex = Index / NumElementsPerChunk;
		const int32 WithinChunkIndex = Index % NumElementsPerChunk;

		if (!IsValidIndex(Index)) {
			printf("[FAIL] IsValidIndex Returned False For Index -> (%d)\n", Index);
			return NULL;
		}

		if (ChunkIndex >= NumChunks) {
			printf("[FAIL] ChunkIndex (%d) >= NumChunks (%d)\n", ChunkIndex, NumChunks);
			return NULL;
		}

		if (Index > MaxElements) {
			printf("[FAIL] Index (%d) > MaxElements (%d)\n", Index, MaxElements);
			return NULL;
		}

		FUObjectItem* Chunk = Objects[ChunkIndex];
		if (!Chunk) return NULL;

		return Chunk + WithinChunkIndex;
	}

	FORCEINLINE FUObjectItem* GetObjectPtr(int32 Index)
	{
		const int32 ChunkIndex = Index / NumElementsPerChunk;
		const int32 WithinChunkIndex = Index % NumElementsPerChunk;
		if (!IsValidIndex(Index)) {
			printf("[FAIL] IsValidIndex Returned False For Index -> (%d)\n", Index);
			return NULL;
		}

		if (ChunkIndex > NumChunks) {
			printf("[FAIL] ChunkIndex (%d) > NumChunks (%d)\n", ChunkIndex, NumChunks);
			return NULL;
		}

		if (Index > MaxElements) {
			printf("[FAIL] Index (%d) > MaxElements (%d)\n", Index, MaxElements);
			return NULL;
		}

		FUObjectItem* Chunk = Objects[ChunkIndex];
		if (!Chunk) return NULL;

		return Chunk + WithinChunkIndex;
	}

	FORCEINLINE FUObjectItem const& operator[](int32 Index) const
	{
		FUObjectItem const* ItemPtr = GetObjectPtr(Index);
		if (ItemPtr) return *ItemPtr;
	}

	FORCEINLINE FUObjectItem& operator[](int32 Index)
	{
		FUObjectItem* ItemPtr = GetObjectPtr(Index);
		if (ItemPtr) return *ItemPtr;
	}

	int64 GetAllocatedSize() const
	{
		return MaxChunks * sizeof(FUObjectItem*) + NumChunks * NumElementsPerChunk * sizeof(FUObjectItem);
	}

	template<typename T> T FindUObject(const char* NameToFind);

	void Log();
};


class FUObjectArray
{
public:
	typedef FChunkedFixedUObjectArray TUObjectArray;

	/** First index into objects array taken into account for GC.							*/
	int32 ObjFirstGCIndex;

	/** Index pointing to last object created in range disregarded for GC.					*/
	int32 ObjLastNonGCIndex;

	/** Maximum number of objects in the disregard for GC Pool */
	int32 MaxObjectsNotConsideredByGC;

	/** If true this is the intial load and we should load objects int the disregarded for GC range.	*/
	bool OpenForDisregardForGC;

	/** Array of all live objects.											*/
	TUObjectArray ObjObjects;

	FORCEINLINE FUObjectItem* IndexToObject(int32 Index)
	{
		if (Index < ObjObjects.Num())
		{
			return const_cast<FUObjectItem*>(&ObjObjects[Index]);
		}
		return nullptr;
	}

	/*
	* Not interested in other members.
	* Complete from here /Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectArray.h
	*/
};
