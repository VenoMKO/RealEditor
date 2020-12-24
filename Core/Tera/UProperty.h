#pragma once
#include "Core.h"
#include "UClass.h"

struct FPropertyTag;
struct FPropertyValue;
class UProperty : public UField {
public:
  DECL_UOBJ(UProperty, UField);
  DECL_CLASS_CAST(UProperty);

  void Serialize(FStream& s) override;

  virtual FString GetID() = 0;

  virtual void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const = 0;

  int32 ArrayDim = 1;
  uint64 PropertyFlags = 0;
  uint16 RepOffset = 0;
  FName Category;
  DECL_UREF(UEnum, ArraySizeEnum);
  UProperty* PropertyLinkNext = nullptr;

  FString DisplayName;
};

class UByteProperty : public UProperty {
public:
  DECL_UOBJ(UByteProperty, UProperty);
  DECL_CLASS_CAST(UByteProperty);

  void Serialize(FStream& s) override;

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_ByteProperty;
  }

public:
  DECL_UREF(UEnum, Enum);
};

class UIntProperty : public UProperty {
public:
  DECL_UOBJ(UIntProperty, UProperty);
  DECL_CLASS_CAST(UIntProperty);

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_IntProperty;
  }
};

class UBoolProperty : public UProperty {
public:
  DECL_UOBJ(UBoolProperty, UProperty);
  DECL_CLASS_CAST(UBoolProperty);

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

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
  DECL_CLASS_CAST(UFloatProperty);

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_FloatProperty;
  }
};

class UObjectProperty : public UProperty {
public:
  DECL_UOBJ(UObjectProperty, UProperty);
  DECL_CLASS_CAST(UObjectProperty);

  void Serialize(FStream& s) override;

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_ObjectProperty;
  }

public:
  DECL_UREF(UClass, PropertyClass);
};

class UComponentProperty : public UObjectProperty {
public:
  DECL_UOBJ(UComponentProperty, UObjectProperty);
  DECL_CLASS_CAST(UComponentProperty);
};

class UClassProperty : public UObjectProperty {
public:
  DECL_UOBJ(UClassProperty, UObjectProperty);
  DECL_CLASS_CAST(UClassProperty);

  void Serialize(FStream& s) override;

public:
  DECL_UREF(UClass, MetaClass);
};

class UInterfaceProperty : public UProperty {
public:
  DECL_UOBJ(UInterfaceProperty, UProperty);
  DECL_CLASS_CAST(UInterfaceProperty);

  void Serialize(FStream& s) override;

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_InterfaceProperty;
  }

public:
  DECL_UREF(UClass, InterfaceClass);
};

class UNameProperty : public UProperty {
public:
  DECL_UOBJ(UNameProperty, UProperty);
  DECL_CLASS_CAST(UNameProperty);

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_NameProperty;
  }
};

class UStrProperty : public UProperty {
public:
  DECL_UOBJ(UStrProperty, UProperty);
  DECL_CLASS_CAST(UStrProperty);

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_StrProperty;
  }
};

class UArrayProperty : public UProperty {
public:
  DECL_UOBJ(UArrayProperty, UProperty);
  DECL_CLASS_CAST(UArrayProperty);

  void Serialize(FStream& s) override;

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_ArrayProperty;
  }

public:
  DECL_UREF(UProperty, Inner);
};

class UMapProperty : public UProperty {
public:
  DECL_UOBJ(UMapProperty, UProperty);
  DECL_CLASS_CAST(UMapProperty);

  void Serialize(FStream& s) override;

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_MapProperty;
  }

public:
  DECL_UREF(UProperty, Key);
  DECL_UREF(UProperty, Value);
};

class UStructProperty : public UProperty {
public:
  DECL_UOBJ(UStructProperty, UProperty);
  DECL_CLASS_CAST(UStructProperty);

  FString GetID() override
  {
    return NAME_StructProperty;
  }

  void Serialize(FStream& s) override;
  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

public:
  DECL_UREF(UScriptStruct, Struct);
};

class UDelegateProperty : public UProperty {
public:
  DECL_UOBJ(UDelegateProperty, UProperty);
  DECL_CLASS_CAST(UDelegateProperty);

  void Serialize(FStream& s) override;

  void SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults = nullptr) const override;

  FString GetID() override
  {
    return NAME_DelegateProperty;
  }

public:
  DECL_UREF(UObject, Function);
  DECL_UREF(UObject, SourceDelegate);
};