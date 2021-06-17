#include "UObject.h"
#include "FStream.h"
#include "FPackage.h"
#include "FObjectResource.h"
#include "UClass.h"
#include "UComponent.h"
#include "UProperty.h"
#include "Cast.h"

#include "Utils/ALog.h"

#if DUMP_OBJECTS
#include <filesystem>
#endif

UObject::UObject(FObjectExport* exp)
  : Export(exp)
{
#ifdef _DEBUG
  Description = Export->GetObjectNameString();
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

FString UObject::GetFullObjectName() const
{
  return Export->GetFullObjectName();
}

FPackage* UObject::GetPackage() const
{
  return Export->Package;
}

void UObject::SetTransacting(bool flag)
{
  flag ? Export->ObjectFlags |= RF_IsTransacting : Export->ObjectFlags &= ~RF_IsTransacting;
}

bool UObject::HasAnyFlags(uint64 flags) const
{
  return (GetObjectFlags() & flags) != 0 || flags == RF_AllFlags;
}

void UObject::MarkDirty(bool dirty)
{
  if (dirty)
  {
    Export->ObjectFlags |= RF_Marked;
    GetPackage()->MarkDirty();
  }
  else
  {
    Export->ObjectFlags &= ~RF_Marked;
  }
}

FILE_OFFSET UObject::GetSerialOffset() const
{
  return Export ? Export->SerialOffset : -1;
}

FILE_OFFSET UObject::GetSerialSize() const
{
  return Export ? Export->SerialSize : -1;
}

FILE_OFFSET UObject::GetPropertiesSize() const
{
  return RawDataOffset ? RawDataOffset - GetSerialOffset() : -1;
}

FILE_OFFSET UObject::GetDataSize() const
{
  return RawDataOffset ? (GetSerialOffset() + GetSerialSize() - RawDataOffset) : -1;
}

FPropertyTag* UObject::CreateProperty(const FString& name)
{
  if (!Class)
  {
    LogE("Failed to create a property %s. Object %s(%s) has no class.", name.UTF8().c_str(), GetObjectNameString().UTF8().c_str(), GetClassNameString().UTF8().c_str());
    return nullptr;
  }
  if (UProperty* classProperty = Class->GetProperty(name))
  {
    return classProperty->CreatePropertyTag(this);
  }
  return nullptr;
}

void UObject::AddProperty(FPropertyTag* property)
{
  if (!property)
  {
    return;
  }
  const FName propName = property->Name;
  if (property->ClassProperty && Properties.size() && propName != NAME_None)
  {
    // Try to insert the property at the correct index
    // Object properties must be already sorted for this to work correctly 
    int32 lastIndex = -1;
    UClass* objClass = Class;
    std::vector<FName> classPropNames;

    while (objClass)
    {
      UField* field = objClass->GetPropertyLink();
      while (field)
      {
        if (!Cast<UProperty>(field))
        {
          field = field->GetNext();
          continue;
        }
        FName fieldName = field->GetObjectName();
        if (fieldName == propName)
        {
          lastIndex = classPropNames.size();
        }
        classPropNames.emplace_back(fieldName);
        field = field->GetNext();
      }
      objClass = objClass->GetSuperClass();
    }

    std::vector<size_t> ordered;
    for (FPropertyTag* tag : Properties)
    {
      auto it = std::find(classPropNames.begin(), classPropNames.end(), tag->Name);
      if (it != classPropNames.end())
      {
        ordered.emplace_back(it - classPropNames.begin());
      }
    }
    ordered.emplace_back(lastIndex);
    std::sort(ordered.begin(), ordered.end());
    auto it = std::find(ordered.rbegin(), ordered.rend(), lastIndex);
    if (it != ordered.rend())
    {
      int32 idx = std::distance(it, ordered.rend()) - 1;
      if (idx && idx < Properties.size() && Properties[idx]->Name == propName)
      {
        // The prop is a static array element. Add it to the end.
        idx++;
      }
      Properties.insert(Properties.begin() + idx, property);
    }
    else
    {
      Properties.emplace_back(property);
    }
    return;
  }

  if (property->Name != NAME_None && Properties.size() && Properties.back()->Name == NAME_None)
  {
    Properties.insert(Properties.end() - 1, property);
  }
  else
  {
    Properties.emplace_back(property);
  }
}

void UObject::RemoveProperty(FPropertyTag* tag)
{
  if (!tag)
  {
    return;
  }
  // Store offset to the field and null it too
  Properties.erase(std::remove(Properties.begin(), Properties.end(), tag), Properties.end());
  delete tag;
}

void* UObject::GetRawData()
{
  if (RawData)
  {
    return RawData;
  }
  FReadStream s(GetPackage()->GetDataPath());
  if (!RawDataOffset)
  {
    try
    {
      Load(s);
    }
    catch (const std::exception& e)
    {
      LogE("Failed to load the object %s", GetObjectNameString().C_str());
      LogE(e.what());
    }
    return nullptr;
  }
  if (!RawDataOffset || GetDataSize() <= 0)
  {
    return nullptr;
  }
  void* result = malloc(GetDataSize());
  s.SetPosition(RawDataOffset);
  s.SerializeBytes(result, GetDataSize());
  RawData = (char*)result;
  return result;
}

void UObject::SetRawData(void* data, FILE_OFFSET size)
{
  if (RawData)
  {
    free(RawData);
    RawData = nullptr;
    MarkDirty();
  }
  if (size)
  {
    RawData = (char*)malloc(size);
    memcpy(RawData, data, size);
    Export->SerialSize = RawDataOffset - GetSerialOffset() + size;
    MarkDirty();
  }
  
}

void UObject::SerializeScriptProperties(FStream& s)
{
  if (Class && s.GetFV() == FPackage::GetCoreVersion())
  {
    Class->SerializeTaggedProperties(s, (UObject*)this, nullptr, HasAnyFlags(RF_ClassDefaultObject) ? Class->GetSuperClass() : Class, nullptr);
    if (!s.IsReading() && Properties.size() && Properties.back()->Name != NAME_None)
    {
      FPropertyTag* none = new FPropertyTag(this, NAME_None, NAME_None);
      Properties.push_back(none);
      s << *none;
      LogW("Object %s doesn't have a terminating property! RE has terminated the property list manually.", GetObjectNameString().UTF8().c_str());
    }
  }
  else
  {
    if (s.IsReading())
    {
      // TODO: This won't work for complex structs
      // We will break the loop on a first 'None'
      // sub-property leaving rest of the props unread
      // Need to use legacy serialization
      bool warned = false;
      while (1)
      {
        FPropertyTag* tag = new FPropertyTag((UObject*)this);
        s << *tag;
        AddProperty(tag);
        if (tag->Name == NAME_None)
        {
          break;
        }
        else if (tag->Size)
        {
          tag->Value->Data = new uint8[tag->Size];
          tag->Value->Type = FPropertyValue::VID::Unk;
          s.SerializeBytes(tag->GetValueData(), tag->Size);
        }
        if (!warned)
        {
          LogW("Failed to use class serialization for %s.", GetObjectPath().C_str());
          warned = true;
        }
      }
    }
    else
    {
      for (FPropertyTag* tag : Properties)
      {
        s << *tag;
        if (tag->Size)
        {
          s.SerializeBytes(tag->GetValueData(), tag->Size);
        }
      }
    }
  }
}

bool UObject::RegisterProperty(FPropertyTag* property)
{
  return false;
}

FString UObject::GetObjectPath() const
{
  if (Export)
  {
    return Export->GetObjectPath();
  }
  return FString();
}

FName UObject::GetObjectName() const
{
  return Export->ObjectName;
}

FString UObject::GetObjectNameString() const
{
  return Export->GetObjectName().String();
}

FName UObject::GetClassName() const
{
  return Export->GetClassName();
}

FString UObject::GetClassNameString() const
{
  return Export->GetClassNameString();
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
  for (FPropertyTag* tag : Properties)
  {
    delete tag;
  }
  Properties.clear();
  if (TrailingData)
  {
    free(TrailingData);
  }
}

void UObject::Serialize(FStream& s)
{
  if (HasAnyFlags(RF_HasStack))
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

  if (IsA(UComponent::StaticClassName()))
  {
    ((UComponent*)this)->PreSerialize(s);
  }

  if (!IsTransacting())
  {
    s << NetIndex;

    if (s.IsReading() && NetIndex != INDEX_NONE)
    {
      GetPackage()->AddNetObject(this);
    }
  }
  

  if (GetStaticClassName() != UClass::StaticClassName())
  {
    SerializeScriptProperties(s);
  }

#if _DEBUG
  if (!IsTransacting())
  {
    FILE_OFFSET curPos = s.GetPosition();
    FILE_OFFSET fileEnd = Export->SerialOffset + Export->SerialSize;
    DBreakIf(s.IsReading() && fileEnd < curPos);
  }
  
#endif

  if (s.IsReading())
  {
    RawDataOffset = s.GetPosition();
  }
  // Serialize RawData for unimplemented classes
  if (!strcmp(GetStaticClassName(), UObject::StaticClassName()))
  {
    if (s.IsReading() && Export->SerialOffset + Export->SerialSize >= RawDataOffset)
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
  if (!GetPackage()->GetStream().GetLoadSerializedObjects() || Loaded || Loading)
  {
    return;
  }
  // Create a new stream here. This allows safe multithreading
  FReadStream s = FReadStream(A2W(GetPackage()->GetDataPath()));
  s.SetPackage(GetPackage());
  s.SetLoadSerializedObjects(GetPackage()->GetStream().GetLoadSerializedObjects());

  Load(s);
}

void UObject::Load(FStream& s)
{
  if (Loaded || Loading)
  {
    return;
  }
  Loading = true;

  // Load object's class and a default object
  if (GetClassName() != UClass::StaticClassName())
  {
    Class = GetPackage()->LoadClass(Export->ClassIndex);
    bool isNative = HasAnyFlags(RF_Native);
    PACKAGE_INDEX outerIndex = Export->OuterIndex;
    while (outerIndex && !isNative)
    {
      FObjectExport* outer = GetPackage()->GetExportObject(outerIndex);
      isNative = outer->ObjectFlags & RF_Native;
      outerIndex = outer->OuterIndex;
    }
    if (!isNative)
    {
      if (Class && !HasAnyFlags(RF_ClassDefaultObject))
      {
        DefaultObject = Class->GetClassDefaultObject();
      }
    }
  }

  if (s.IsReading() && !IsTransacting())
  {
    s.SetPosition(Export->SerialOffset);
#if DUMP_OBJECTS
    void* data = malloc(Export->SerialSize);
    s.SerializeBytes(data, Export->SerialSize);
    s.SetPosition(Export->SerialOffset);
    std::filesystem::path path = std::filesystem::path(DUMP_PATH) / GetPackage()->GetPackageName().String() / "Objects";
    std::filesystem::create_directories(path);
    path /= (Export->GetFullObjectName().String() + ".bin");
    std::ofstream os(path.wstring(), std::ios::out | std::ios::binary);
    os.write((const char*)data, Export->SerialSize);
    free(data);
#endif
  }
  
#if SERIALIZE_PROPERTIES
  if (HasAnyFlags(RF_ClassDefaultObject))
  {
    SerializeDefaultObject(s);
  }
  else
  {
    Serialize(s);
  }
#endif

  Loaded = true;
  Loading = false;

  if (s.IsReading())
  {
    //DBreakIf(s.GetPosition() != Export->SerialOffset + Export->SerialSize);
    PostLoad();
  }
}

void UObject::PostLoad()
{
}

void UObject::SerializeDefaultObject(FStream& s)
{
  s << NetIndex;
  if (s.IsReading() && NetIndex != INDEX_NONE)
  {
    GetPackage()->AddNetObject(this);
  }
  if (GetStaticClassName() != UClass::StaticClassName())
  {
    SerializeScriptProperties(s);
  }
}

void UObject::SerializeTrailingData(FStream& s)
{
  if (s.IsReading())
  {
    if (TrailingData)
    {
      // Should be called once per instance
      DBreak();
      free(TrailingData);
      TrailingData = nullptr;
    }
    TrailingDataSize = std::max(GetExportObject()->SerialOffset + GetExportObject()->SerialSize - s.GetPosition(), 0);
    if (TrailingDataSize)
    {
      TrailingData = malloc(TrailingDataSize);
    }
  }
  s.SerializeBytes(TrailingData, TrailingDataSize);
}

bool UObject::IsA(const char* base) const
{
  const auto thisClassName = GetStaticClassName();
  if (base == thisClassName)
  {
    return true;
  }
  for (UClass* tmp = Class; tmp; tmp = static_cast<UClass*>(tmp->GetSuperStruct()))
  {
    if (base == tmp->GetStaticClassName())
    {
      return true;
    }
  }
  std::string chain = GetStaticClassChain();
  size_t pos = chain.size();
  size_t lastPos = pos;
  while ((pos = chain.find_last_of('.', pos)) != std::string::npos)
  {
    if (std::string_view(chain.c_str() + pos + 1, lastPos - pos - 1) == base)
    {
      return true;
    }
    lastPos = pos;
    pos--;
  }
  return false;
}

FStream& operator<<(FStream& s, UObject*& obj)
{
  PACKAGE_INDEX idx = 0;
  if (s.IsReading())
  {
    s << idx;
    FILE_OFFSET tmpPos = s.GetPosition();
    obj = s.GetPackage()->GetObject(idx, s.GetLoadSerializedObjects());
    s.SetPosition(tmpPos);
  }
  else
  {
    idx = s.GetPackage()->GetObjectIndex(obj);
    s << idx;
  }
  return s;
}
