#pragma once
#include "Core.h"
#include "UClass.h"

extern const char* NAME_PropNone;
extern const char* NAME_PropBool;
extern const char* NAME_PropByte;
extern const char* NAME_PropInt;
extern const char* NAME_PropFloat;
extern const char* NAME_PropObj;
extern const char* NAME_PropName;
extern const char* NAME_PropString;
extern const char* NAME_PropStruct;
extern const char* NAME_PropArray;
extern const char* NAME_PropMap;
extern const char* NAME_PropInterface;
extern const char* NAME_PropDelegate;

extern const char* NAME_PropStructVector;
extern const char* NAME_PropStructVector4;
extern const char* NAME_PropStructColor;
extern const char* NAME_PropStructLinearColor;
extern const char* NAME_PropStructGuid;
extern const char* NAME_PropStructRotator;
extern const char* NAME_PropStructMatrix;

class UProperty : public UField {
public:
  DECL_UOBJ(UProperty, UField);

	void Serialize(FStream& s) override;

	virtual std::string GetID() = 0;

protected:
	int32 ArrayDim = 0;
	uint64 PropertyFlags = 0;
	uint16 RepOffset = 0;
	FName Category;
	UEnum* ArraySizeEnum = nullptr;
};

class UByteProperty : public UProperty {
public:
	DECL_UOBJ(UByteProperty, UProperty);

	void Serialize(FStream& s) override;

	std::string GetID() override
	{
		return NAME_PropByte;
	}

protected:
	UEnum* Enum = nullptr;
};

class UIntProperty : public UProperty {
public:
	DECL_UOBJ(UIntProperty, UProperty);

	std::string GetID() override
	{
		return NAME_PropInt;
	}
};

class UBoolProperty : public UProperty {
public:
	DECL_UOBJ(UBoolProperty, UProperty);

	std::string GetID() override
	{
		return NAME_PropBool;
	}

protected:
	uint32 BitMask = 1;
};

class UFloatProperty : public UProperty {
public:
	DECL_UOBJ(UFloatProperty, UProperty);

	std::string GetID() override
	{
		return NAME_PropFloat;
	}
};

class UObjectProperty : public UProperty {
public:
	DECL_UOBJ(UObjectProperty, UProperty);

	void Serialize(FStream& s) override;

	std::string GetID() override
	{
		return NAME_PropObj;
	}

protected:
	UClass* PropertyClass = nullptr;
};

class UComponentProperty : public UObjectProperty {
public:
	DECL_UOBJ(UComponentProperty, UObjectProperty);
};

class UClassProperty : public UObjectProperty {
public:
	DECL_UOBJ(UClassProperty, UObjectProperty);

	void Serialize(FStream& s) override;

protected:
	UClass* MetaClass = nullptr;
};

class UInterfaceProperty : public UProperty {
public:
	DECL_UOBJ(UInterfaceProperty, UProperty);

	void Serialize(FStream& s) override;

	std::string GetID() override
	{
		return NAME_PropInterface;
	}

protected:
	UClass* InterfaceClass = nullptr;
};

class UNameProperty : public UProperty {
public:
	DECL_UOBJ(UNameProperty, UProperty);

	std::string GetID() override
	{
		return NAME_PropName;
	}
};

class UStrProperty : public UProperty {
public:
	DECL_UOBJ(UStrProperty, UProperty);

	std::string GetID() override
	{
		return NAME_PropString;
	}
};

class UArrayProperty : public UProperty {
public:
	DECL_UOBJ(UArrayProperty, UProperty);

	void Serialize(FStream& s) override;

	std::string GetID() override
	{
		return NAME_PropArray;
	}

protected:
	UProperty* Inner = nullptr;
};

class UMapProperty : public UProperty {
public:
	DECL_UOBJ(UMapProperty, UProperty);

	void Serialize(FStream& s) override;

	std::string GetID() override
	{
		return NAME_PropMap;
	}

protected:
	UProperty* Key = nullptr;
	UProperty* Value = nullptr;
};

class UStructProperty : public UProperty {
public:
	DECL_UOBJ(UStructProperty, UProperty);

	std::string GetID() override
	{
		return NAME_PropStruct;
	}

	void Serialize(FStream& s) override;

protected:
	UObject* Struct = nullptr;
};

class UDelegateProperty : public UProperty {
public:
	DECL_UOBJ(UDelegateProperty, UProperty);

	std::string GetID() override
	{
		return NAME_PropDelegate;
	}
	void Serialize(FStream& s) override;

	UObject* Function;
	UObject* SourceDelegate;
};