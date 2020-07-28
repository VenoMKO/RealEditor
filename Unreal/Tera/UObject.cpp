#include "UObject.h"
#include "FStream.h"
#include "FPackage.h"
#include "FObjectResource.h"
#include "UClass.h"
#include "UProperty.h"

UObject* UObject::Object(FObjectExport* exp)
{
  if (exp->Object)
  {
    return exp->Object;
  }
  UObject* result = nullptr;
  const std::string c = exp->GetClassName();
  if (c == "Class")
  {
    result = new UClass(exp);
  }
  else if (c == "Field")
  {
    result = new UField(exp);
  }
  else if (c == "Struct")
  {
    result = new UStruct(exp);
  }
  else if (c == "State")
  {
    result = new UState(exp);
  }
  else if (c == "Enum")
  {
    result = new UEnum(exp);
  }
  else if (c == "TextBuffer")
  {
    result = new UTextBuffer(exp);
  }
  else if (c == NAME_PropInt)
  {
    result = new UIntProperty(exp);
  }
  else if (c == NAME_PropBool)
  {
    result = new UBoolProperty(exp);
  }
  else if (c == NAME_PropByte)
  {
    result = new UByteProperty(exp);
  }
  else if (c == NAME_PropFloat)
  {
    result = new UFloatProperty(exp);
  }
  else if (c == NAME_PropObj)
  {
    result = new UObjectProperty(exp);
  }
  else if (c == NAME_PropName)
  {
    result = new UNameProperty(exp);
  }
  else if (c == NAME_PropString)
  {
    result = new UStrProperty(exp);
  }
  else if (c == NAME_PropStruct)
  {
    result = new UStructProperty(exp);
  }
  else if (c == NAME_PropArray)
  {
    result = new UArrayProperty(exp);
  }
  else if (c == NAME_PropMap)
  {
    result = new UMapProperty(exp);
  }
  else if (c == NAME_PropInterface)
  {
    result = new UInterfaceProperty(exp);
  }
  else if (c == NAME_PropDelegate)
  {
    result = new UDelegateProperty(exp);
  }
  else if (c == "SomeCustomClass")
  {
    // result = new SomeCustomClass(exp);
  }
  else
  {
    result = new UObject(exp);
  }
  exp->Object = result;
  return result;
}

UObject::UObject(FObjectExport* exp)
  : Export(exp)
{
#ifdef _DEBUG
  Description = Export->GetObjectName() + "(" + Export->GetClassName() + ")";
#endif
}

uint32 UObject::GetExportFlags() const
{
  return Export->ExportFlags;
}

uint64 UObject::GetObjectFlags() const
{
  return Export->ObjectFlags;
}

FPackage* UObject::GetPackage() const
{
  return Export->Package;
}

std::string UObject::GetObjectName() const
{
  return Export->GetObjectName();
}

std::string UObject::GetClassName() const
{
  return Export->GetClassName();
}

UObject::~UObject()
{
  if (StateFrame)
  {
    delete StateFrame;
  }
}

void UObject::Serialize(FStream& s)
{
  if (s.IsReading())
  {
    s.SetPosition(Export->SerialOffset);
  }
  if (GetObjectFlags() & RF_HasStack)
  {
    if (s.IsReading())
    {
      StateFrame = new FStateFrame();
    }
    if (StateFrame)
    {
      s << *StateFrame;
    }
  }

  s << NetIndex;
  if (s.IsReading() && NetIndex != NET_INDEX_NONE)
  {
    GetPackage()->AddNetObject(this);
  }

  // TODO: Load class component

  if (GetObjectFlags() & RF_Native)
  {
    return;
  }

  
}

void UObject::Load()
{
  if (Loaded)
  {
    return;
  }

  // Load object's class and a default object
  if (!Class && GetClassName() != "Class")
  {
    bool isNative = GetObjectFlags() & RF_Native;
    PACKAGE_INDEX outerIndex = Export->OuterIndex;
    while (outerIndex && !isNative)
    {
      FObjectExport* outer = GetPackage()->GetExportObject(outerIndex);
      isNative = outer->ObjectFlags & RF_Native;
      outerIndex = outer->OuterIndex;
    }
    if (!isNative)
    {
      Class = GetPackage()->LoadClass(GetClassName());
    }
    if (!(GetObjectFlags() & RF_ClassDefaultObject))
    {
      // TODO: Get default object
    }
  }

  FStream& s = GetPackage()->GetStream();
  Serialize(s);
  Loaded = true;
}

FStream& operator<<(FStream& s, UObject*& obj)
{
  PACKAGE_INDEX idx = 0;
  if (s.IsReading())
  {
    s << idx;
    obj = s.GetPackage()->GetObject(idx);
  }
  else
  {
    idx = s.GetPackage()->GetObjectIndex(obj);
    s << idx;
  }
  return s;
}
