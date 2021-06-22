#include "UObjectReferencer.h"
#include "FObjectResource.h"
#include "FPackage.h"

bool UObjectReferencer::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, ReferencedObjects))
  {
    ReferencedObjectsProperty = property;
    ReferencedObjects = property->Value->GetArrayPtr();
    return true;
  }
  return false;
}

void UObjectReferencer::AddObject(UObject* obj)
{
  if (!obj || !ReferencedObjects)
  {
    return;
  }
  for (FPropertyValue* v : *ReferencedObjects)
  {
    if (v->GetObjectValuePtr(false) == obj)
    {
      return;
    }
  }
  FPropertyValue* val = new FPropertyValue(ReferencedObjectsProperty, nullptr);
  val->Type = FPropertyValue::VID::Object;
  val->Data = new PACKAGE_INDEX;
  val->GetObjectIndex() = GetPackage()->GetObjectIndex(obj);
  ReferencedObjects->emplace_back(val);
  MarkDirty();
}

void UObjectReferencer::RemoveExport(FObjectExport* exp)
{
  if (!exp || !ReferencedObjects)
  {
    return;
  }
  for (int32 idx = 0; idx < ReferencedObjects->size(); ++idx)
  {
    if (ReferencedObjects->at(idx)->GetObjectIndex() == exp->ObjectIndex)
    {
      ReferencedObjects->erase(ReferencedObjects->begin() + idx);
    }
  }
}

std::vector<UObject*> UObjectReferencer::GetObject()
{
  std::vector<UObject*> result;
  for (FPropertyValue* v : *ReferencedObjects)
  {
    result.emplace_back(v->GetObjectValuePtr(false));
  }
  return result;
}
