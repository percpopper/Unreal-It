#include <UnrealEngine/UE.h>

template<typename T>
T FChunkedFixedUObjectArray::FindUObject(const char* NameToFind)
{
	for (int CurrentElement = 0; CurrentElement < NumElements; CurrentElement++)
	{
		FUObjectItem* ObjectItem = GetObjectPtr(CurrentElement);

		if (ObjectItem && ObjectItem->Object->GetFullName() == NameToFind) {
			printf("[SUCCESS] Object with the name \"%s\" was the %i object found.\n\n", NameToFind, CurrentElement);

			return static_cast<T>(ObjectItem->Object);
		}
	}

	printf("[FAIL] All %i objects were checked for the name %s with no result.\n\n", NumElements, NameToFind);

	return NULL;
}

void FChunkedFixedUObjectArray::Log()
{
	Logger LOG("\\FUObjectArray_ObjObjects_Log.txt");

	printf("[INFO] Logging FUObjectArray->ObjObjects...\n\n");

	for (int CurrentElement = 0; CurrentElement < NumElements; CurrentElement++)
	{
		FUObjectItem* ObjectItem = GetObjectPtr(CurrentElement);

		UObject* Object = ObjectItem->Object;

		LOG.LogToFile(Object->GetFullName());
	}

	printf("[SUCCESS] FUObjectArray->ObjObjects log has completed.\n\n");

	LOG.CloseLog();
}
