#include "UActor.h"
#include "FPackage.h"

#include "UClass.h"
#include "FPropertyTag.h"
#include "Cast.h"

#include "UActorComponent.h"
#include "UStaticMesh.h"
#include "USkeletalMesh.h"
#include "USpeedTree.h"

FVector UActor::GetLocation()
{
  FVector location = Location;
  if (PrePivotProperty)
  {
    FVector delta = PrePivot;
    delta *= DrawScale3D;
    delta *= DrawScale;

    const FRotationMatrix Matrix = FRotationMatrix(Rotation);
    delta = Matrix.TransformFVector(delta);

    location -= delta;
  }
  return location;
}

std::vector<FString> UActor::GetLayers() const
{
  std::vector<FString> result;
  if (LayerProperty)
  {
    FString layerStr = Layer.String();
    if (layerStr.Size())
    {
      size_t pos = 0;
      size_t last = 0;
      size_t next = 0;
      while ((next = layerStr.Find(",", last)) != std::string::npos)
      {
        result.push_back(layerStr.Substr(last, next - last));
        last = next + 1;
      }
      result.push_back(layerStr.Substr(last));
    }
  }
  return result;
}

bool UActor::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, Components))
  {
    ComponentsProperty = property;
    auto& tarray = property->Value->GetArray();
    for (FPropertyValue* v : tarray)
    {
      Components.push_back((UActorComponent*)GetPackage()->GetObject(v->GetObjectIndex(), false));
    }
    return true;
  }
  REGISTER_VEC_PROP(PrePivot);
  REGISTER_VEC_PROP(Location);
  REGISTER_ROT_PROP(Rotation);
  REGISTER_VEC_PROP(DrawScale3D);
  REGISTER_FLOAT_PROP(DrawScale);
  REGISTER_BOOL_PROP(bHidden);
  REGISTER_BOOL_PROP(bCollideActors);
  REGISTER_NAME_PROP(Layer);
  return false;
}

void UActor::PostLoad()
{
  Super::PostLoad();
  for (UObject* obj : Components)
  {
    LoadObject(obj);
  }
}

bool UStaticMeshActor::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, StaticMeshComponent))
  {
    StaticMeshComponentProperty = property;
    StaticMeshComponent = Cast<UStaticMeshComponent>(GetPackage()->GetObject(property->Value->GetObjectIndex(), false));
    return true;
  }
  return false;
}

void UStaticMeshActor::PostLoad()
{
  LoadObject(StaticMeshComponent);
  Super::PostLoad();
}

USkeletalMeshActor::USkeletalMeshActor(FObjectExport* exp)
  : UActor(exp)
{
  bCollideActors = false;
}

bool USkeletalMeshActor::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(SkeletalMeshComponent, USkeletalMeshComponent*);
  return false;
}

void USkeletalMeshActor::PostLoad()
{
  LoadObject(SkeletalMeshComponent);
  Super::PostLoad();
}

bool USpeedTreeActor::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(SpeedTreeComponent, USpeedTreeComponent*);
  return false;
}

void USpeedTreeActor::PostLoad()
{
  LoadObject(SpeedTreeComponent);
  Super::PostLoad();
}

bool UDynamicSMActor::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(StaticMeshComponent, UStaticMeshComponent*);
  REGISTER_TOBJ_PROP(ReplicatedMesh, UStaticMesh*);
  REGISTER_VEC_PROP(ReplicatedMeshTranslation);
  REGISTER_ROT_PROP(ReplicatedMeshRotation);
  REGISTER_VEC_PROP(ReplicatedMeshScale3D);
  return false;
}

void UDynamicSMActor::PostLoad()
{
  LoadObject(StaticMeshComponent);
  Super::PostLoad();
}

UDynamicSMActor::UDynamicSMActor(FObjectExport* exp)
  : UActor(exp)
{
  bCollideActors = false;
}
