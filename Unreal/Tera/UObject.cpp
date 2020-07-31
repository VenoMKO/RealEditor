#include "UObject.h"
#include "FStream.h"
#include "FPackage.h"
#include "FObjectResource.h"
#include "UClass.h"
#include "UComponent.h"

#include "ALog.h"

#include <filesystem>

UObject::UObject(FObjectExport* exp)
  : Export(exp)
{
#ifdef _DEBUG
  Description = Export->GetFullObjectName();
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

UObject* UObject::GetOuter() const
{
  return GetPackage()->GetObject((FObjectExport*)Export->GetOuter());
}

void UObject::SerializeScriptProperties(FStream& s) const
{
  if (Class)
  {
    Class->SerializeTaggedProperties(s, (UObject*)this, (GetObjectFlags() & RF_ClassDefaultObject) ? Class->GetSuperClass() : Class, nullptr);
    return;
  }
  // TODO: serialize props without a Class obj
  // I belive the object has only a None property if there is no Class
  FName nonePropertyName;
  s << nonePropertyName;
  DBreakIf(nonePropertyName.String() != "None");
}

std::string UObject::GetObjectPath() const
{
  std::string path = GetPackage()->GetPackageName();
  FObjectResource* outer = Export->GetOuter();
  while (outer)
  {
    path += "." + outer->GetObjectName();
    outer = outer->GetOuter();
  }
  path += "." + GetObjectName();
  return path;
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
  if (RawData)
  {
    delete[] RawData;
  }
}

void UObject::Serialize(FStream& s)
{
  if (s.IsReading())
  {
    s.SetPosition(Export->SerialOffset);
#if defined(_DEBUG) && defined(DUMP_PATH)
    if (s.IsReading())
    {
      void* data = malloc(Export->SerialSize);
      s.SerializeBytes(data, Export->SerialSize);
      s.SetPosition(Export->SerialOffset);
      std::filesystem::path path = std::filesystem::path(DUMP_PATH) / GetPackage()->GetPackageName() / "Objects";
      std::filesystem::create_directories(path);
      path /= (Export->GetFullObjectName() + ".bin");
      std::ofstream os(path.wstring(), std::ios::out | std::ios::binary);
      os.write((const char*)data, Export->SerialSize);
      free(data);
    }
#endif
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

  if (IsComponent())
  {
    ((UComponent*)this)->PreSerialize(s);
  }

  s << NetIndex;

  if (s.IsReading() && NetIndex != INDEX_NONE)
  {
    GetPackage()->AddNetObject(this);
  }

  if (GetStaticClassName() != UClass::StaticClassName())
  {
    SerializeScriptProperties(s);
  }

  if (s.IsReading())
  {
    RawDataOffset = s.GetPosition();
  }
  // Serialize RawData for unimplemented classes
  if (!strcmp(GetStaticClassName(), UObject::StaticClassName()))
  {
    if (s.IsReading())
    {
      RawDataSize = Export->SerialOffset + Export->SerialSize - RawDataOffset;
      if (RawDataSize)
      {
        RawData = new char[RawDataSize];
      }
    }
    s.SerializeBytes(RawData, RawDataSize);
  }
}

void UObject::Load()
{
  if (Loaded || Loading)
  {
    return;
  }
  Loading = true;

  // Load object's class and a default object
  if (GetClassName() != UClass::StaticClassName())
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
      if (Class && !(GetObjectFlags() & RF_ClassDefaultObject))
      {
        DefaultObject = Class->GetClassDefaultObject();
      }
    }
    else
    {
      LogI("Skip class load for %s", Export->GetFullObjectName().c_str());
    }
  }

  FStream& s = GetPackage()->GetStream();
  Serialize(s);
  Loaded = true;
  Loading = false;

  if (s.IsReading())
  {
    PostLoad();
    //DBreakIf(s.GetPosition() != Export->SerialOffset + Export->SerialSize);
  }
}

void UObject::PostLoad()
{
}

FStream& operator<<(FStream& s, UObject*& obj)
{
  PACKAGE_INDEX idx = 0;
  if (s.IsReading())
  {
    s << idx;
    // GetObject will try to load the object, so we need to save our current stream position
    // Loading objects here is actually a bit risky. May end up with a stack overflow
    FILE_OFFSET tmpPos = s.GetPosition();
    obj = s.GetPackage()->GetObject(idx);
    s.SetPosition(tmpPos);
  }
  else
  {
    idx = s.GetPackage()->GetObjectIndex(obj);
    s << idx;
  }
  return s;
}
