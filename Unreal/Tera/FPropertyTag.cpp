#include "FPropertyTag.h"
#include "FStructs.h"
#include "UObject.h"
#include "UClass.h"
#include "FString.h"
#include "FName.h"

FPropertyValue::~FPropertyValue()
{
	if (Data)
	{
		switch (Type)
		{
		case VID::Unk:
		case VID::Bool:
		case VID::Byte:
		case VID::Int:
		case VID::Float:
		case VID::Object:
			delete Data;
			break;
		case VID::None:
			break;
		case VID::Delegate:
			delete GetScriptDelegatePtr();
			break;
		case VID::Property:
			delete GetPropertyTagPtr();
			break;
		case VID::Name:
			delete GetNamePtr();
			break;
		case VID::String:
			delete GetStringPtr();
			break;
		case VID::Struct:
		case VID::Array:
			for (FPropertyValue* v : GetArray())
			{
				delete v;
			}
			delete GetArrayPtr();
			break;
		default:
			break;
		}
	}
}