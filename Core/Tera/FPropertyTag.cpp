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
		case VID::Field:
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

bool FPropertyTag::GetVector(FVector& output) const
{
	if (!Value || Value->Type != FPropertyValue::VID::Struct)
	{
		return false;
	}
	std::vector<FPropertyValue*> arr = Value->GetArray();
	for (FPropertyValue* v : arr)
	{
		if (v->Field)
		{
      if (v->Field->GetObjectName() == "X")
      {
        output.X = v->GetArray()[0]->GetFloat();
      }
      else if (v->Field->GetObjectName() == "Y")
      {
        output.Y = v->GetArray()[0]->GetFloat();
      }
      else if (v->Field->GetObjectName() == "Z")
      {
        output.Z = v->GetArray()[0]->GetFloat();
      }
			else
			{
				DBreak();
			}
		}
	}
	return true;
}

bool FPropertyTag::GetRotator(FRotator& output) const
{
  if (!Value || Value->Type != FPropertyValue::VID::Struct)
  {
    return false;
  }
  std::vector<FPropertyValue*> arr = Value->GetArray();
  for (FPropertyValue* v : arr)
  {
    if (v->Field)
    {
      if (v->Field->GetObjectName() == "Pitch")
      {
        output.Pitch = v->GetArray()[0]->GetInt();
      }
      else if (v->Field->GetObjectName() == "Yaw")
      {
        output.Yaw = v->GetArray()[0]->GetInt();
      }
      else if (v->Field->GetObjectName() == "Roll")
      {
        output.Roll = v->GetArray()[0]->GetInt();
      }
      else
      {
        DBreak();
      }
    }
  }
  return true;
}
