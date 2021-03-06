#pragma once
#include "Core.h"
#include "FStream.h"
#include "FPropertyTag.h"

// Common UObject subclass declarations
#define DECL_UOBJ(TClass, TSuper)\
public:\
  inline friend FStream& operator<<(FStream& s, TClass*& obj) { return s << *(UObject**)&obj; }\
  typedef TClass ThisClass;\
  typedef TSuper Super;\
  static const char* StaticClassName() { return ((char*)#TClass) + 1; }\
  const char* GetStaticClassName() const override { return ThisClass::StaticClassName(); }\
  std::string GetStaticClassChain() const override { return Super::GetStaticClassChain() + "." + ThisClass::StaticClassName(); }\
  using TSuper::TSuper

// Common property helpers for faster property definition and registration

// Private macro for naming
#define __GLUE_PROP(TName, Suffix) TName##Suffix

// Check if the PropertyTag matches TName
#define PROP_IS(prop, TName) (prop->Name == P_##TName)

// Basic property declaration
#define UPROP(TType, TName, TDefault)\
TType TName = TDefault;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr

// Basic property declaration with a default value
#define UPROP_NOINIT(TType, TName)\
TType TName;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr

// Property with a custom method to create it
#define UPROP_CREATABLE(TType, TName, TDefault)\
TType TName = TDefault;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr;\
FPropertyTag* CreateProperty##TName(const TType& value)\
{\
  if (FPropertyTag* tag = CreateProperty(P_##TName))\
  {\
    tag->GetTypedValue<TType>() = value;\
    AddProperty(tag);\
    RegisterProperty(tag);\
    return tag;\
  }\
  return nullptr;\
}\
//

// Enum property with a custom method to create it
#define UPROP_CREATABLE_ENUM(TType, TName, TDefault)\
TType TName = TDefault;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr;\
FPropertyTag* CreateProperty##TName(const TType& value)\
{\
  if (FPropertyTag* tag = CreateProperty(P_##TName))\
  {\
    tag->Value->GetByte() = (uint8)value;\
    AddProperty(tag);\
    tag->Value->RegisterEnumNames();\
    RegisterProperty(tag);\
    return tag;\
  }\
  return nullptr;\
}\
//

// Static array property with a custom method to create it
#define UPROP_CREATABLE_STATIC_ARR(TType, TCount, TName, ...)\
TType TName [ TCount ] = { __VA_ARGS__ };\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) [ TCount ] = { nullptr };\
FPropertyTag* CreateProperty##TName(const TType& value, int32 arrayIdx)\
{\
  if (FPropertyTag* tag = CreateProperty(P_##TName))\
  {\
    tag->Value->GetTypedValue<TType>() = value;\
    if (arrayIdx &&  __GLUE_PROP(TName, Property)[arrayIdx - 1])\
    {\
      __GLUE_PROP(TName, Property)[arrayIdx - 1]->StaticArrayNext = tag;\
    }\
    else if (arrayIdx + 1 < TCount && __GLUE_PROP(TName, Property)[arrayIdx + 1])\
    {\
      tag->StaticArrayNext = __GLUE_PROP(TName, Property)[arrayIdx - 1];\
    }\
    tag->ArrayIndex = arrayIdx;\
    AddProperty(tag);\
    RegisterProperty(tag);\
    return tag;\
  }\
  return nullptr;\
}\
//

// Array property with a custom method to create it
#define UPROP_CREATABLE_ARR_PTR(TName)\
std::vector<FPropertyValue*>* TName = nullptr;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr;\
FPropertyTag* CreateProperty##TName()\
{\
  if (FPropertyTag* tag = CreateProperty(P_##TName))\
  {\
    tag->Value->GetArray() = *(new std::vector<FPropertyValue*>);\
    AddProperty(tag);\
    RegisterProperty(tag);\
    return tag;\
  }\
  return nullptr;\
}
//

// Private registration macros

#define __REGISTER_PROP(TName, TType)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  TName = property->__GLUE_PROP(Get,TType)();\
  return true;\
}\
//

#define __REGISTER_TYPED_PROP(TName, TCast, TType)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  TName = (TCast)property->__GLUE_PROP(Get,TType)();\
  return true;\
}\
//

#define REGISTER_ENUM_STR_PROP(TName)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  if (property->Value->Enum)\
  {\
    TName = property->Value->Enum->GetEnum(property->GetByte()).String().String();\
  }\
  else\
  {\
    TName = FString::Sprintf("%d", int(property->GetByte()));\
  }\
  return true;\
}\
//

// Public registration macros

#define REGISTER_FLOAT_PROP(TName) __REGISTER_PROP(TName, Float)

#define REGISTER_INT_PROP(TName) __REGISTER_PROP(TName, Int)

#define REGISTER_BOOL_PROP(TName) __REGISTER_PROP(TName, Bool)

#define REGISTER_BYTE_PROP(TName) __REGISTER_PROP(TName, Byte)

#define REGISTER_STR_PROP(TName) __REGISTER_PROP(TName, String)

#define REGISTER_OBJ_PROP(TName) __REGISTER_PROP(TName, ObjectValuePtr)

#define REGISTER_TOBJ_PROP(TName, TType) __REGISTER_TYPED_PROP(TName, TType, ObjectValuePtr)

#define REGISTER_NAME_PROP(TName) __REGISTER_PROP(TName, Name)

#define REGISTER_VEC_PROP(TName)\
if (PROP_IS(property, TName))\
{\
__GLUE_PROP(TName, Property) = property; \
return property->GetVector(TName); \
}\
//

#define REGISTER_ROT_PROP(TName)\
if (PROP_IS(property, TName))\
{\
__GLUE_PROP(TName, Property) = property; \
return property->GetRotator(TName); \
}\
//

#define REGISTER_COL_PROP(TName)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  property->GetColor(TName);\
  return true;\
}\
//

#define REGISTER_LCOL_PROP(TName)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  property->GetLinearColor(TName);\
  return true;\
}\
//

#define SUPER_REGISTER_PROP()\
if (Super::RegisterProperty(property))\
{\
  return true;\
}\
//

class UObject {
public:
  // =========================================================================
  // Specialized methods used by DECL_UOBJ, ClassCast, etc. Don't override!!!
  // =========================================================================

  enum { StaticClassCastFlags = CASTCLASS_None };
  virtual uint32 GetStaticClassCastFlags() const
  {
    return StaticClassCastFlags;
  }

  // Cpp class name. Use ONLY with a class (e.g., UObject::StaticClass())
  static const char* StaticClassName()
  {
    return "Object";
  }

  // Get cpp class name from an instance
  virtual const char* GetStaticClassName() const
  {
    return StaticClassName();
  }

  // Internal method used for safe casting
  virtual std::string GetStaticClassChain() const
  {
    return StaticClassName();
  }

  // =========================================================================
  // Normal UObject methods
  // =========================================================================

  // Helper for loading nonull objects
  static void LoadObject(UObject* object)
  {
    if (object)
    {
      object->Load();
    }
  }

  // Object factory
  static UObject* Object(FObjectExport* exp);

  UObject() = delete;
  UObject(const UObject&) = delete;
  UObject(FObjectExport* exp);

  virtual ~UObject();

  // Load the object from its package. Creates a copy of package's stream
  virtual void Load();

  // Load the object from the provided stream. Will set correct stream offset.
  virtual void Load(FStream& s);

  // Serialize object by an index
  friend FStream& operator<<(FStream& s, UObject*& obj);

  // Used to collect properties during serialization.
  virtual bool RegisterProperty(FPropertyTag* property);

  FString GetObjectPath() const;

  FName GetObjectName() const;

  FString GetObjectNameString() const;

  FName GetClassName() const;

  FString GetClassNameString() const;

  uint32 GetExportFlags() const;

  uint64 GetObjectFlags() const;

  FString GetFullObjectName() const;

  inline FObjectExport* GetExportObject() const
  {
    return Export;
  }

  inline NET_INDEX GetNetIndex() const
  {
    return NetIndex;
  }

  FPackage* GetPackage() const;

  // Is UComponent?
  virtual bool IsComponent() const
  {
    return false;
  }

  inline bool IsTransacting() const
  {
    return HasAnyFlags(RF_IsTransacting);
  }

  void SetTransacting(bool flag);

  inline bool IsNewTrans() const
  {
    return HasAnyFlags(RF_TransNew);
  }

  void SetIsNewTrans(bool flag);

  // Check if UObject has any object flags
  bool HasAnyFlags(uint64 flags) const;

  UObject* GetOuter() const
  {
    return Outer;
  }

  std::vector<UObject*> GetInner() const
  {
    return Inner;
  }

  void MarkDirty(bool dirty = true);

  inline bool IsDirty() const
  {
    return HasAnyFlags(RF_Marked);
  }

  FILE_OFFSET GetSerialOffset() const;
  
  FILE_OFFSET GetSerialSize() const;

  FILE_OFFSET GetPropertiesSize() const;

  FILE_OFFSET GetDataSize() const;

  bool IsTemplate(uint64 templateTypes = (RF_ArchetypeObject | RF_ClassDefaultObject)) const
  {
    for (const UObject* outer = this; outer; outer = outer->GetOuter())
    {
      if (outer->HasAnyFlags((EObjectFlags)templateTypes))
      {
        return true;
      }
    }
    return false;
  }

  template<typename T>
  T* GetTypedOuter() const
  {
    T* result = NULL;
    for (UObject* nextOuter = Outer; result == NULL && nextOuter != NULL; nextOuter = nextOuter->GetOuter())
    {
      if (nextOuter->IsA(T::StaticClassName()))
      {
        result = (T*)nextOuter;
      }
    }
    return result;
  }

  bool IsA(const char* base) const;

  inline bool IsLoaded() const
  {
    return Loaded;
  }

  inline void SetLoaded(bool loaded)
  {
    Loaded = loaded;
  }

  // Get object's class object
  inline UClass* GetClass() const
  {
    return Class;
  }

  inline void SetClass(UClass* cls)
  {
    Class = cls;
  }

  // Get object's root properties
  std::vector<FPropertyTag*> GetProperties() const
  {
    return Properties;
  }

  // Create a new property. Allocates properties value. Doesn't add the property to objects Properties!
  virtual FPropertyTag* CreateProperty(const FString& name);

  // Add root property to the object. Don't call this with a struct/array element property!
  void AddProperty(FPropertyTag* property);

  // Remove root property
  void RemoveProperty(FPropertyTag* tag);
  
  // Object initialization. Wont modify package's object tree!
  void SetOuter(UObject* outer)
  {
    Outer = outer;
  }

  // Object initialization. Wont modify package's object tree!
  void AddInner(UObject* inner)
  {
    Inner.push_back(inner);
  }

  // Get objects raw binary data. The data is unmodified unless SetRawData was called.
  void* GetRawData();

  // Set raw binary data
  void SetRawData(void* data, FILE_OFFSET size);

  // Serialize the object from a stream. Should not be called outside of the Load
  virtual void Serialize(FStream& s);

private:
  virtual void SerializeScriptProperties(FStream& s);

protected:
  // Handle object initialization for derived classes
  virtual void PostLoad();

  // Special serialization for class default objects
  void SerializeDefaultObject(FStream& s);

  // Helper to serialize object data leftovers(e.g., incomplete implementation). Should be called in the Serialize method of a derived class
  void SerializeTrailingData(FStream& s);

protected:
  bool Loaded = false;
  FObjectExport* Export = nullptr;
  FStateFrame* StateFrame = nullptr;
  NET_INDEX NetIndex = INDEX_NONE;

  UClass* Class = nullptr;
  UObject* DefaultObject = nullptr;

  UObject* Outer = nullptr;
  std::vector<UObject*> Inner;

  std::vector<FPropertyTag*> Properties;

  FILE_OFFSET RawDataOffset = 0;
  FILE_OFFSET RawDataSize = 0;
  char* RawData = nullptr;

  void* TrailingData = nullptr;
  FILE_OFFSET TrailingDataSize = 0;
#ifdef _DEBUG
  std::string Description;
#endif
private:
  bool Loading = false;
};

class UPackage : public UObject {
public:
  DECL_UOBJ(UPackage, UObject);
};