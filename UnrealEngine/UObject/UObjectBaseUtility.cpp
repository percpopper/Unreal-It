#include <UnrealEngine/UE.h>

std::string UObjectBaseUtility::GetName()
{
	return NamePrivate.GetName();
}

/** Credit to KN4CK3R's UnrealEngineSDKGenerator for this format of GetFullName. */
std::string UObjectBaseUtility::GetFullName()
{

	std::string Name;

	if (ClassPrivate)
	{
		std::string Temp;
		for (UObject* CurrentOuter = OuterPrivate; CurrentOuter; CurrentOuter = CurrentOuter->OuterPrivate)
		{
			Temp = CurrentOuter->GetName() + "." + Temp;
		}

		Name = ClassPrivate->GetName();
		Name += " ";
		Name += Temp;
		Name += GetName();
	}

	return Name;
}
