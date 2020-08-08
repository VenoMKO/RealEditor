#include "ALog.h"

#include "UClass.h"
#include "FStream.h"
#include "UProperty.h"
#include "UStaticMesh.h"

#include "Cast.h"

#include "FPackage.h"
#include "FObjectResource.h"
#include "FPropertyTag.h"

#define FUNC_Net 0x00000040

void UTextBuffer::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Pos << Top;
  s << Text;
}

void UField::Serialize(FStream& s)
{
  Super::Serialize(s);
  if (s.GetFV() < VER_TERA_MODERN)
  {
    s << Superfield;
  }
  s << Next;
}

UStruct::~UStruct()
{
  if (ScriptData)
  {
    free(ScriptData);
  }
  if (ScriptStorage)
  {
    free(ScriptStorage);
  }
}

void UStruct::Link()
{
  for (TFieldIterator<UStruct> it(this); it; ++it)
  {
    it->Link();
  }
  std::scoped_lock<std::mutex> lock(LinkMutex);
  if (!PropertyLink)
  {
    UProperty** propertyLinkPtr = &PropertyLink;
    for (TFieldIterator<UProperty> it(this); it; ++it)
    {
      UProperty* property = *it;
      *propertyLinkPtr = property;
      propertyLinkPtr = &(*propertyLinkPtr)->PropertyLinkNext;
    }
  }
}

void UStruct::Serialize(FStream& s)
{
  Super::Serialize(s);
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << SuperStruct;
  }
  else
  {
    SuperStruct = (UStruct*)Superfield;
  }

  s << ScriptText;
  s << Children;
  s << CppText;
  s << Line << TextPos;
  s << ScriptDataSize;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << ScriptStorageSize;
  }
  if (s.IsReading())
  {
    if (ScriptStorageSize)
    {
      ScriptStorage = malloc(ScriptStorageSize);
    }
    else if (ScriptDataSize)
    {
      ScriptData = malloc(ScriptDataSize);
    }
  }
  s.SerializeBytes(ScriptStorage, ScriptStorageSize);
  s.SerializeBytes(ScriptData, ScriptDataSize);
}

void UState::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ProbeMask;
  s << LabelTableOffset;
  s << StateFlags;
  s << FuncMap;
}

void UStruct::SerializeTaggedProperties(FStream& s, UObject* object, UStruct* defaultsStruct, void* defaults, int32 defaultsCount) const
{
  UClass* defaultsClass = (UClass*)defaultsStruct;

  if (s.IsReading())
  {
    bool advance = false;
    
    UProperty* property = PropertyLink;
    int32 remainingDim = property ? property->ArrayDim : 0;
#if _DEBUG
    UProperty* last = nullptr;
    static int32 GIteration = -1;
    int32 iterations = 0;
#endif
    while (1)
    {
#if _DEBUG
      GIteration++;
      iterations++;
      last = property;
#endif
      FPropertyTag tag;
      s << tag;

      std::string tagName = tag.Name.String().UTF8();

      if (tagName == NAME_None)
      {
        break;
      }

      std::string tagType = tag.Type.String().UTF8();

      if (advance && --remainingDim <= 0)
      {
        property = property->PropertyLinkNext;
        advance = false;
        remainingDim = property ? property->ArrayDim : 0;
      }

      if (!property || tag.Name.String() != property->GetObjectName())
      {
        UProperty* currentProperty = property;

        for (; property; property = property->PropertyLinkNext)
        {
          if (property->GetObjectName() == tagName)
          {
            break;
          }
        }
        if (!property)
        {
          for (property = PropertyLink; property && property != currentProperty; property = property->PropertyLinkNext)
          {
            if (property->GetObjectName() == tagName)
            {
              break;
            }
          }
          if (property == currentProperty)
          {
            property = nullptr;
          }
        }
        remainingDim = property ? property->ArrayDim : 0;
      }

      if (!property)
      {
        LogE("Property %s of %ls not found in %s", tagName.c_str(), object->GetObjectName().WString().c_str(), object->GetPackage()->GetPackageName().UTF8().c_str());
      }
      else if (tag.ArrayIndex >= property->ArrayDim || tag.ArrayIndex < 0)
      {
        LogE("Array bounds in %s of %s: %i/%i for package:  %s", tagName.c_str(), object->GetObjectName().WString().c_str(), tag.ArrayIndex, property->ArrayDim, object->GetPackage()->GetPackageName().UTF8().c_str());
        DBreak();
      }
      else if (tagType == NAME_StrProperty && Cast<UNameProperty>(property) != nullptr)
      {
        LogE("Property type mismatch in %s of %s", tagName.c_str(), object->GetObjectName().UTF8().c_str());
        DBreak();
      }
      else if (property->GetID() != tagType)
      {
        LogE("Property type mismatch in %s of %s", tagName.c_str(), object->GetObjectName().UTF8().c_str());
        DBreak();
      }
      else if (tagType == NAME_StructProperty && tag.StructName.String() != CastChecked<UStructProperty>(property)->Struct->GetObjectName())
      {
        LogE("Property %s of %s struct type mismatch %s/%s", tagName.c_str(), object->GetObjectName().UTF8(), tag.StructName.String().UTF8().c_str(), CastChecked<UStructProperty>(property)->Struct->GetObjectName().UTF8().c_str());
        DBreak();
      }
      else if (tagType == NAME_ByteProperty && 
              ((tag.EnumName == NAME_None && ExactCast<UByteProperty>(property)->Enum != nullptr) || 
                (tag.EnumName != NAME_None && ExactCast<UByteProperty>(property)->Enum == nullptr)) && s.GetFV() >= VER_TERA_CLASSIC)
      {
        LogE("Property coversion required in %s of %s", tagName.c_str(), object->GetObjectName().UTF8().c_str());
        DBreak();
      }
      else
      {
        tag.SerializeTaggedProperty(s, property, object, defaultsStruct);
        advance = true;
        continue;
      }
      s.SetPosition(s.GetPosition() + tag.Size);
      LogW("Skipping property %s of %ls in %s package", tagName.c_str(), object->GetObjectName().WString().c_str(), object->GetPackage()->GetPackageName().UTF8().c_str());
    }
  }
}

void UStruct::SerializeBin(FStream& s, FPropertyTag* tag, UObject* object) const
{
  for (UProperty* property = PropertyLink; property; property = property->PropertyLinkNext)
  {
    SerializeBinProperty(property, tag, s, object);
  }
}

void UStruct::SerializeBinEx(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaultStruct, void* defaults, int32 defaultsCount) const
{
  if (!defaults || !defaultsCount)
  {
    SerializeBin(s, tag, object);
  }
}

void UStruct::SerializeBinProperty(UProperty* property, FPropertyTag* tag, FStream& s, UObject* object) const
{
  for (int32 Idx = 0; Idx < property->ArrayDim; Idx++)
  {
    property->SerializeItem(s, tag, object);
  }
}

void UClass::CreateBuiltInClasses(FPackage* package)
{
#define MAKE_CLASS(NAME) exp = package->CreateVirtualExport(##NAME, NAME_Class); exp->Object = UObject::Object(exp); obj = (UClass*)exp->Object
  const auto pkgName = package->GetPackageName();
  VObjectExport* exp = nullptr;
  UClass* obj = nullptr;
  if (pkgName == "Core")
  {
    MAKE_CLASS(NAME_ArrayProperty);
    MAKE_CLASS(NAME_BoolProperty);
    MAKE_CLASS(NAME_ByteProperty);
    MAKE_CLASS(NAME_Class);
    MAKE_CLASS(NAME_ClassProperty);
    MAKE_CLASS(NAME_ComponentProperty);
    MAKE_CLASS(NAME_Const);
    MAKE_CLASS(NAME_DelegateProperty);
    MAKE_CLASS(NAME_Enum);
    MAKE_CLASS(NAME_FloatProperty);
    MAKE_CLASS(NAME_Function);
    MAKE_CLASS(NAME_InterfaceProperty);
    MAKE_CLASS(NAME_IntProperty);
    MAKE_CLASS(NAME_MapProperty);
    MAKE_CLASS(NAME_MetaData);
    MAKE_CLASS(NAME_NameProperty);
    MAKE_CLASS(NAME_ObjectProperty);
    MAKE_CLASS(NAME_Package);
    MAKE_CLASS(NAME_Property);
    MAKE_CLASS(NAME_State);
    MAKE_CLASS(NAME_ScriptStruct);
    MAKE_CLASS(NAME_StrProperty);
    MAKE_CLASS(NAME_StructProperty);
    MAKE_CLASS(NAME_TextBuffer);
    MAKE_CLASS(NAME_ObjectRedirector);
  }
  else if (pkgName == "Engine")
  {
    MAKE_CLASS(NAME_StaticMesh);
    UStaticMesh::ConfigureClassObject(obj);
  }
  else if (pkgName == "UnrealEd")
  {
    MAKE_CLASS(NAME_PersistentCookerData);
  }
}

void UClass::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ClassFlags;
  s << ClassWithin;
  s << ClassConfigName;
  s << ComponentNameToDefaultObjectMap;
  s << Interfaces;
  s << DontSortCategories;
  s << HideCategories;
  s << AutoExpandCategories;
  s << AutoCollapseCategories;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << bForceScriptOrder;
    s << ClassGroupNames;
  }
  s << ClassHeaderFilename;
  s << DLLBindName;
  s << ClassDefaultObject;
}

FStream& operator<<(FStream& s, FImplementedInterface& i)
{
  s << i.ObjectClass;
  s << i.PointerProperty;
  return s;
}

FStream& operator<<(FStream& s, FPushedState& f)
{
  s << f.State;
  s << f.Node;
  return s;
}

FStream& operator<<(FStream& s, FStateFrame& f)
{
  s << f.Node;
  s << f.StateNode;
  if (s.GetFV() < VER_TERA_MODERN)
  {
    s << f.ProbeMask64;
  }
  else
  {
    s << f.ProbeMask32;
  }
  s << f.LatentAction;
  s << f.StateStack;
  if (f.Node)
  {
    s << f.Offset;
  }
  return s;
}

void UEnum::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Names;
}

void UScriptStruct::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << StructFlags;
}

void UFunction::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << iNative;
  s << OperPrecedence;
  s << FunctionFlags;
  if (FunctionFlags & FUNC_Net)
  {
    s << RepOffset;
  }
  s << FriendlyName;
}

void UConst::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Value;
}
