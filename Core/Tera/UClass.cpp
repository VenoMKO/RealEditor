#include "Utils/ALog.h"

#include "UClass.h"
#include "FStream.h"
#include "UProperty.h"

#include "UStaticMesh.h"
#include "ULevel.h"
#include "UTexture.h"

#include "Cast.h"

#include "FPackage.h"
#include "FObjectResource.h"
#include "FPropertyTag.h"

#define FUNC_Net 0x00000040

UProperty* CreateClassProperty(const char* name, const  char* className, UStruct* parent)
{
  VObjectExport* exp = parent->GetPackage()->CreateVirtualExport(name, className);
  parent->GetExportObject()->Inner.push_back(exp);
  UProperty* prop = (UProperty*)UObject::Object(exp);
  exp->SetObject(prop);
  if (UField* field = parent->GetChildren())
  {
    for (; field->GetNext(); field = field->GetNext());
    field->SetNext(prop);
  }
  else
  {
    parent->SetChildren(prop);
  }
  return prop;
}

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
    SERIALIZE_UREF(s, Superfield);
  }
  SERIALIZE_UREF(s, Next);
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

UProperty* UStruct::GetProperty(const FString& name) const
{
  UProperty* propertyLinkPtr = PropertyLink;
  for (TFieldIterator<UProperty> it(this); it; ++it)
  {
    UProperty* property = *it;
    if (property->GetObjectName() == name)
    {
      return property;
    }
  }
  return nullptr;
}

void UStruct::Link()
{
  for (TFieldIterator<UStruct> it(this); it; ++it)
  {
    it->Link();
  }
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
    SERIALIZE_UREF(s, SuperStruct);
  }
  else
  {
    SuperStruct = (UStruct*)Superfield;
    SuperStructRefIndex = SuperfieldRefIndex;
  }
  
  SERIALIZE_UREF(s, ScriptText);
  SERIALIZE_UREF(s, Children);
  SERIALIZE_UREF(s, CppText);

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
  if (s.GetFV() < VER_TERA_MODERN)
  {
    int32 unk = 0;
    s << unk;
  }
  s << ProbeMask;
  if (s.GetFV() < VER_TERA_MODERN)
  {
    s << IgnoreMask;
  }
  s << LabelTableOffset;
  s << StateFlags;
  s << FuncMap;
}

void UStruct::SerializeTaggedProperties(FStream& s, UObject* object, FPropertyValue* value, UStruct* defaultsStruct, void* defaults) const
{
  if (s.IsReading())
  {
    bool advance = false;

    if (value && !value->Data)
    {
      value->Data = new std::vector<FPropertyValue*>;
    }
    
    UProperty* property = PropertyLink;
    int32 remainingDim = property ? property->ArrayDim : 0;
    FPropertyTag* tagPtr = nullptr;
    FPropertyTag* prevTagPtr = nullptr;
    while (1)
    {
      FPropertyValue* newValue = nullptr;
      if (value)
      {
        newValue = new FPropertyValue(value->Property);
        newValue->Type = FPropertyValue::VID::Property;
        newValue->Data = new FPropertyTag(object);
        tagPtr = newValue->GetPropertyTagPtr();
        value->GetArray().push_back(newValue);
      }
      else
      {
        tagPtr = new FPropertyTag(object);
        object->AddProperty(tagPtr);
      }

      FPropertyTag& tag = *tagPtr;
      s << tag;

      if (tag.Name == NAME_None)
      {
        break;
      }

      DBreakIf(object->GetClassName() == NAME_Package);

      if (advance && --remainingDim <= 0)
      {
        property = property->PropertyLinkNext;
        advance = false;
        remainingDim = property ? property->ArrayDim : 0;
      }
      else if (prevTagPtr)
      {
        prevTagPtr->StaticArrayNext = tagPtr;
      }

      if (newValue)
      {
        newValue->Field = property;
      }

      if (!property || tag.Name != property->GetObjectName())
      {
        UProperty* currentProperty = property;

        for (; property; property = property->PropertyLinkNext)
        {
          if (property->GetObjectName() == tag.Name)
          {
            break;
          }
        }

        if (!property)
        {
          for (property = PropertyLink; property && property != currentProperty; property = property->PropertyLinkNext)
          {
            if (tag.Name == property->GetObjectName())
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
        LogE("Property %s of %s not found in %s", tag.Name.GetString().UTF8().c_str(), object->GetObjectNameString().String().c_str(), object->GetPackage()->GetPackageName().UTF8().c_str());
      }
      else if (tag.ArrayIndex >= property->ArrayDim || tag.ArrayIndex < 0)
      {
        LogE("Array bounds in %s of %s: %i/%i for package:  %s", tag.Name.GetString().UTF8().c_str(), object->GetObjectNameString().String().c_str(), tag.ArrayIndex, property->ArrayDim, object->GetPackage()->GetPackageName().UTF8().c_str());
        DBreak();
      }
      else if (tag.Type == NAME_StrProperty && Cast<UNameProperty>(property) != nullptr)
      {
        LogE("Property type mismatch in %s of %s", tag.Name.GetString().UTF8().c_str(), object->GetObjectNameString().UTF8().c_str());
        DBreak();
      }
      else if (tag.Type != property->GetID())
      {
        LogE("Property type mismatch in %s of %s", tag.Name.GetString().UTF8().c_str(), object->GetObjectNameString().UTF8().c_str());
        DBreak();
      }
      else if (tag.Type == NAME_StructProperty && tag.StructName.String() != CastChecked<UStructProperty>(property)->Struct->GetObjectNameString())
      {
        LogE("Property %s of %s struct type mismatch %s/%s", tag.Name.GetString().UTF8().c_str(), object->GetObjectNameString().UTF8(), tag.StructName.String().UTF8().c_str(), CastChecked<UStructProperty>(property)->Struct->GetObjectNameString().UTF8().c_str());
        DBreak();
      }
      else if (tag.Type == NAME_ByteProperty && ((tag.EnumName == NAME_None && ExactCast<UByteProperty>(property)->Enum != nullptr) || (tag.EnumName != NAME_None && ExactCast<UByteProperty>(property)->Enum == nullptr)) && s.GetFV() >= VER_TERA_CLASSIC)
      {
        LogE("Property coversion required in %s of %s", tag.Name.GetString().UTF8().c_str(), object->GetObjectNameString().UTF8().c_str());
        // DBreak();
      }
      else
      {
        tag.ClassProperty = property;
        tag.ArrayDim = property->ArrayDim;
        if (property->GetStaticClassName() == UBoolProperty::StaticClassName())
        {
          tag.Value->Data = new bool;
          tag.Value->Type = FPropertyValue::VID::Bool;
          tag.Value->GetBool() = tag.BoolVal;
        }
        else
        {
          property->SerializeItem(s, tag.Value, object, defaultsStruct);
        }

        object->RegisterProperty(tagPtr);
        advance = true;
        prevTagPtr = tagPtr;
        continue;
      }
      

      tag.Value->Data = new uint8[tag.Size];
      tag.Value->Type = FPropertyValue::VID::Unk;
      prevTagPtr = tagPtr;
      s.SerializeBytes(tag.GetValueData(), tag.Size);
      LogW("Skipping property %s of %s in %s package", tag.Name.GetString().UTF8().c_str(), object->GetObjectNameString().String().c_str(), object->GetPackage()->GetPackageName().UTF8().c_str());
    }
  }
  else
  {
    int32 idx = 0;
    FPropertyTag* tagPtr = nullptr;
    std::vector<FPropertyTag*> properties = object->GetProperties();
    while (1)
    {
      if (!value)
      {
        if (idx >= properties.size())
        {
          break;
        }
        tagPtr = properties[idx];
      }
      else
      {
        auto& array = value->GetArray();
        if (idx >= array.size())
        {
          break;
        }
        tagPtr = array[idx]->GetPropertyTagPtr();
      }
      s << *tagPtr;
      FILE_OFFSET size = s.GetPosition();
      if (tagPtr->ClassProperty)
      {
        if (tagPtr->ClassProperty->GetStaticClassName() != UBoolProperty::StaticClassName())
        {
          tagPtr->ClassProperty->SerializeItem(s, tagPtr->Value, object, defaultsStruct);
        }
      }
      else
      {
        s.SerializeBytes(tagPtr->GetValueData(), tagPtr->Size);
      }
      FILE_OFFSET tmpPos = s.GetPosition();
      size = tmpPos - size;
      if (size != tagPtr->Size)
      {
        tagPtr->Size = size;
        s.SetPosition(tagPtr->SizeOffset);
        s << tagPtr->Size;
        s.SetPosition(tmpPos);
      }
      idx++;
    }
    if (!tagPtr)
    {
      FName none(s.GetPackage(), NAME_None);
      s << none;
    }
  }
}

void UStruct::SerializeBin(FStream& s, FPropertyValue* value, UObject* object) const
{
  int32 idx = 0;
  for (UProperty* property = PropertyLink; property; property = property->PropertyLinkNext, ++idx)
  {
    if (s.IsReading())
    {
      value->GetArray().push_back(new FPropertyValue(value->Property, property));
      value->GetArray()[idx]->Field = property;
    }
    SerializeBinProperty(property, value->GetArray()[idx], s, object);
  }
}

void UStruct::SerializeBinProperty(UProperty* property, FPropertyValue* value, FStream& s, UObject* object) const
{
  if (s.IsReading())
  {
    value->Data = new std::vector<FPropertyValue*>(property->ArrayDim);
    value->Type = FPropertyValue::VID::Field;
  }
  for (int32 idx = 0; idx < property->ArrayDim; idx++)
  {
    if (s.IsReading())
    {
      value->GetArray()[idx] = new FPropertyValue(value->Property);
      value->GetArray()[idx]->Field = property;
    }
    property->SerializeItem(s, value->GetArray()[idx], object);
  }
}

void UClass::CreateBuiltInClasses(FPackage* package)
{
#define MAKE_CLASS(NAME) exp = package->CreateVirtualExport(##NAME, NAME_Class); exp->SetObject(new UClass(exp, true)); obj = (UClass*)exp->GetObject(); obj->Loaded = true; FPackage::RegisterClass((UClass*)obj)
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
    MAKE_CLASS(NAME_Level);
    ULevel::ConfigureClassObject(obj);
    MAKE_CLASS(NAME_LightMapTexture2D);
    ULightMapTexture2D::ConfigureClassObject(obj);
  }
  else if (pkgName == "UnrealEd")
  {
    MAKE_CLASS(NAME_PersistentCookerData);
  }
}

FString UClass::GetDLLBindName() const
{
    return DLLBindName.String();
}

void UClass::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ClassFlags;
  SERIALIZE_UREF(s, ClassWithin);
  s << ClassConfigName;
  if (s.GetFV() == VER_TERA_CLASSIC)
  {
    s << HideCategories;
    s << ComponentNameToDefaultObjectMap;
    s << Interfaces;
    s << AutoExpandCategories;
  }
  else
  {
    s << ComponentNameToDefaultObjectMap;
    s << Interfaces;
    s << DontSortCategories;
    s << HideCategories;
    s << AutoExpandCategories;
    s << AutoCollapseCategories;
    s << bForceScriptOrder;
    s << ClassGroupNames;
    s << ClassHeaderFilename;
    s << DLLBindName;
  }
  SERIALIZE_UREF(s, ClassDefaultObject);
}

FStream& operator<<(FStream& s, FImplementedInterface& i)
{
  SERIALIZE_UREF(s, i.ObjectClass);
  SERIALIZE_UREF(s, i.PointerProperty);
  return s;
}

FStream& operator<<(FStream& s, FPushedState& f)
{
  SERIALIZE_UREF(s, f.State);
  SERIALIZE_UREF(s, f.Node);
  return s;
}

FStream& operator<<(FStream& s, FStateFrame& f)
{
  SERIALIZE_UREF(s, f.Node);
  SERIALIZE_UREF(s, f.StateNode);
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
