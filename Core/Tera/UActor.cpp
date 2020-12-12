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

    FVector rot = Rotation.Normalized().Euler();
    
    rot.X = (rot.X / 360.) * 2. * M_PI;
    rot.Y = (rot.Y / 360.) * 2. * M_PI;
    rot.Z = (rot.Z / 360.) * 2. * M_PI;

    FMatrix mx(FPlane(1, 0, 0, 0),
      FPlane(0, cosf(rot.X), -sinf(rot.X), 0),
      FPlane(0, sinf(rot.X), cosf(rot.X), 0),
      FPlane(0, 0, 0, 1));

    FMatrix my(FPlane(cosf(rot.Y), 0, sinf(rot.Y), 0),
      FPlane(0, 1, 0, 0),
      FPlane(-sinf(rot.Y), 0, cosf(rot.Y), 0),
      FPlane(0, 0, 0, 1));

    FMatrix mz(FPlane(cosf(rot.Z), sinf(rot.Z), 0, 0),
      FPlane(-sinf(rot.Z), cosf(rot.Z), 0, 0),
      FPlane(0, 0, 1, 0),
      FPlane(0, 0, 0, 1));

    auto CleanupMatrix = [](FMatrix& m) {
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          if (abs(m.M[i][j]) < 0.01)
          {
            m.M[i][j] = 0;
          }
        }
      }
    };

    CleanupMatrix(mx);
    CleanupMatrix(my);
    CleanupMatrix(mz);

    mx *= my;
    mx *= mz;

    delta = mx.Rotate(delta);
    location -= delta;
  }
  return location;
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
