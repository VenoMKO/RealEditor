#pragma once
#include "Core.h"
#include "FName.h"
#include "FStream.h"
#include "UProperty.h"

struct FPropertyTag
{
	// Variables.
	FName Type;
	uint8 BoolVal = 0;
	FName Name;
	FName StructName;
	FName EnumName;
	int32 Size = 0;
	int32 ArrayIndex = 0;
	int32 SizeOffset = 0;

	FPropertyTag()
	{}

	friend FStream& operator<<(FStream& s, FPropertyTag& tag)
	{
		s << tag.Name;
		if (tag.Name == NAME_None)
		{
			return s;
		}

		s << tag.Type;
		if (!s.IsReading())
		{
			tag.SizeOffset = s.GetPosition();
		}
		s << tag.Size << tag.ArrayIndex;

		if (tag.Type == NAME_StructProperty)
		{
			s << tag.StructName;
		}
		else if (tag.Type == NAME_BoolProperty)
		{
			if (s.GetFV() < VER_TERA_MODERN)
			{
				bool Value = 0;
				s << Value;
				tag.BoolVal = uint8(Value);
			}
			else
			{
				s << tag.BoolVal;
			}
		}
		else if (tag.Type == NAME_ByteProperty && s.GetFV() > VER_TERA_CLASSIC)
		{
			s << tag.EnumName;
		}
		return s;
	}

	void SerializeTaggedProperty(FStream& s, UProperty* property, UObject* Value, UStruct* Defaults)
	{
		if (property->GetStaticClassName() != UBoolProperty::StaticClassName())
		{
			property->SerializeItem(s, this, Value, Defaults);
		}
	}
};