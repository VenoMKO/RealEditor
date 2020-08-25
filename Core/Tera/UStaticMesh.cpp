#include "UStaticMesh.h"
#include "UClass.h"
#include "UProperty.h"

#include "FPackage.h"
#include "FObjectResource.h"

UProperty* CreateProperty(const char* name, const  char* className, UStruct* parent)
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

void UStaticMesh::ConfigureClassObject(UClass* object)
{
  CreateProperty("UseSimpleLineCollision", UBoolProperty::StaticClassName(), object);
  CreateProperty("UseSimpleBoxCollision", UBoolProperty::StaticClassName(), object);
  CreateProperty("UseSimpleRigidBodyCollision", UBoolProperty::StaticClassName(), object);
  CreateProperty("UseFullPrecisionUVs", UBoolProperty::StaticClassName(), object);
  CreateProperty("bUsedForInstancing", UBoolProperty::StaticClassName(), object);
  CreateProperty("bUseMaximumStreamingTexelRatio", UBoolProperty::StaticClassName(), object);
  CreateProperty("bCanBecomeDynamic", UBoolProperty::StaticClassName(), object);

  CreateProperty("LightMapResolution", UIntProperty::StaticClassName(), object);
  CreateProperty("LightMapCoordinateIndex", UIntProperty::StaticClassName(), object);
  CreateProperty("LODDistanceRatio", UFloatProperty::StaticClassName(), object);
  CreateProperty("LODMaxRange", UFloatProperty::StaticClassName(), object);
  CreateProperty("StreamingDistanceMultiplier", UFloatProperty::StaticClassName(), object);

  CreateProperty("BodySetup", UObjectProperty::StaticClassName(), object);

  object->Link();
}