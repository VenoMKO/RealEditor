#pragma once
#include "Core.h"
#include "UClass.h"
#include "FScriptArray.h"

union UPropertyValue
{
	uint8 ByteValue;
	int32 IntValue;
	bool BoolValue;
	float FloatValue;

	UObject* ObjectValue;
	UComponent* ComponentValue;
	UClass* ClassValue;

	FName* NameValue;
	FString* StringValue;
	FScriptArray* ArrayValue;
};

struct FPropertyTag;
class UProperty : public UField {
public:
  DECL_UOBJ(UProperty, UField);
	DECL_CLASS_INERIT(UProperty);

	void Serialize(FStream& s) override;

	virtual FString GetID() = 0;

	virtual void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const = 0;

	int32 ArrayDim = 1;
	uint64 PropertyFlags = 0;
	uint16 RepOffset = 0;
	FName Category;
	UEnum* ArraySizeEnum = nullptr;
	UProperty* PropertyLinkNext = nullptr;
};

class UByteProperty : public UProperty {
public:
	DECL_UOBJ(UByteProperty, UProperty);
	DECL_CLASS_INERIT(UByteProperty);

	void Serialize(FStream& s) override;

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_ByteProperty;
	}

public:
	UEnum* Enum = nullptr;
};

class UIntProperty : public UProperty {
public:
	DECL_UOBJ(UIntProperty, UProperty);
	DECL_CLASS_INERIT(UIntProperty);

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_IntProperty;
	}
};

class UBoolProperty : public UProperty {
public:
	DECL_UOBJ(UBoolProperty, UProperty);
	DECL_CLASS_INERIT(UBoolProperty);

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_BoolProperty;
	}

public:
	uint32 BitMask = 1;
};

class UFloatProperty : public UProperty {
public:
	DECL_UOBJ(UFloatProperty, UProperty);
	DECL_CLASS_INERIT(UFloatProperty);

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_FloatProperty;
	}
};

class UObjectProperty : public UProperty {
public:
	DECL_UOBJ(UObjectProperty, UProperty);
	DECL_CLASS_INERIT(UObjectProperty);

	void Serialize(FStream& s) override;

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_ObjectProperty;
	}

public:
	UClass* PropertyClass = nullptr;
};

class UComponentProperty : public UObjectProperty {
public:
	DECL_UOBJ(UComponentProperty, UObjectProperty);
	DECL_CLASS_INERIT(UComponentProperty);
};

class UClassProperty : public UObjectProperty {
public:
	DECL_UOBJ(UClassProperty, UObjectProperty);
	DECL_CLASS_INERIT(UClassProperty);

	void Serialize(FStream& s) override;

public:
	UClass* MetaClass = nullptr;
};

class UInterfaceProperty : public UProperty {
public:
	DECL_UOBJ(UInterfaceProperty, UProperty);
	DECL_CLASS_INERIT(UInterfaceProperty);

	void Serialize(FStream& s) override;

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_InterfaceProperty;
	}

public:
	UClass* InterfaceClass = nullptr;
};

class UNameProperty : public UProperty {
public:
	DECL_UOBJ(UNameProperty, UProperty);
	DECL_CLASS_INERIT(UNameProperty);

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_NameProperty;
	}
};

class UStrProperty : public UProperty {
public:
	DECL_UOBJ(UStrProperty, UProperty);
	DECL_CLASS_INERIT(UStrProperty);

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_StrProperty;
	}
};

class UArrayProperty : public UProperty {
public:
	DECL_UOBJ(UArrayProperty, UProperty);
	DECL_CLASS_INERIT(UArrayProperty);

	void Serialize(FStream& s) override;

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_ArrayProperty;
	}

public:
	UProperty* Inner = nullptr;
};

class UMapProperty : public UProperty {
public:
	DECL_UOBJ(UMapProperty, UProperty);
	DECL_CLASS_INERIT(UMapProperty);

	void Serialize(FStream& s) override;

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_MapProperty;
	}

public:
	UProperty* Key = nullptr;
	UProperty* Value = nullptr;
};

class UStructProperty : public UProperty {
public:
	DECL_UOBJ(UStructProperty, UProperty);
	DECL_CLASS_INERIT(UStructProperty);

	FString GetID() override
	{
		return NAME_StructProperty;
	}

	void Serialize(FStream& s) override;
	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

public:
	UScriptStruct* Struct = nullptr;
};

class UDelegateProperty : public UProperty {
public:
	DECL_UOBJ(UDelegateProperty, UProperty);
	DECL_CLASS_INERIT(UDelegateProperty);

	void Serialize(FStream& s) override;

	void SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults = nullptr) const override;

	FString GetID() override
	{
		return NAME_DelegateProperty;
	}

public:
	UObject* Function;
	UObject* SourceDelegate;
};