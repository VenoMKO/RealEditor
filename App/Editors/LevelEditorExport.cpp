#include "LevelEditor.h"
#include "../App.h"
#include "../Windows/ProgressWindow.h"

#include <Tera/Cast.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UObject.h>
#include <Tera/ULevelStreaming.h>
#include <Tera/UStaticMesh.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/USpeedTree.h>
#include <Tera/UMaterial.h>
#include <Tera/UTexture.h>
#include <Tera/ULight.h>
#include <Tera/UTerrain.h>
#include <Tera/UPrefab.h>

#include <Utils/ALog.h>
#include <Utils/T3DUtils.h>
#include <Utils/FbxUtils.h>
#include <Utils/TextureProcessor.h>

#include <functional>

typedef std::function<void(T3DFile&, LevelExportContext&, UActorComponent*)> ComponentDataFunc;

std::string GetLocalDir(UObject* obj, const char* sep = "\\");
void AddCommonPrimitiveComponentParameters(T3DFile& f, UPrimitiveComponent* component);
void SaveMaterialMap(UMeshComponent* component, LevelExportContext& ctx, const std::vector<UObject*>& materials);

void ExportActor(T3DFile& f, LevelExportContext& ctx, UActor* untypedActor);
void ExportTerrainActor(T3DFile& f, LevelExportContext& ctx, UTerrain* actor);

ComponentDataFunc ExportStaticMeshComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UStaticMeshComponent* component = Cast<UStaticMeshComponent>(acomp);
  if (!component || !component->StaticMesh)
  {
    return;
  }
  SaveMaterialMap(component, ctx, component->StaticMesh->GetMaterials());
  std::filesystem::path path = ctx.GetStaticMeshDir() / GetLocalDir(component->StaticMesh);
  std::error_code err;
  if (!std::filesystem::exists(path, err))
  {
    std::filesystem::create_directories(path, err);
  }
  if (std::filesystem::exists(path, err))
  {
    std::string fbxName = component->StaticMesh->GetObjectName().UTF8();
    path /= fbxName;
    path.replace_extension("fbx");
    if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
    {
      FbxUtils utils;
      FbxExportContext fbxCtx;
      fbxCtx.Path = path.wstring();
      fbxCtx.ExportLods = ctx.Config.ExportLods;
      fbxCtx.ExportCollisions = ctx.Config.ConvexCollisions;
      utils.ExportStaticMesh(component->StaticMesh, fbxCtx);
    }
    f.AddStaticMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->StaticMesh, "/") + fbxName).c_str());
  }
  AddCommonPrimitiveComponentParameters(f, component);
};

ComponentDataFunc ExportSkeletalMeshComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  USkeletalMeshComponent* component = Cast<USkeletalMeshComponent>(acomp);
  if (!component || !component->SkeletalMesh)
  {
    return;
  }
  SaveMaterialMap(component, ctx, component->SkeletalMesh->GetMaterials());
  std::filesystem::path path = ctx.GetSkeletalMeshDir() / GetLocalDir(component->SkeletalMesh);
  std::error_code err;
  if (!std::filesystem::exists(path, err))
  {
    std::filesystem::create_directories(path, err);
  }
  if (std::filesystem::exists(path, err))
  {
    std::string fbxName = component->SkeletalMesh->GetObjectName().UTF8();
    path /= fbxName;
    path.replace_extension("fbx");
    if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
    {
      FbxUtils utils;
      FbxExportContext fbxCtx;
      fbxCtx.Path = path.wstring();
      fbxCtx.ExportLods = ctx.Config.ExportLods;
      fbxCtx.ExportCollisions = ctx.Config.ConvexCollisions;
      utils.ExportSkeletalMesh(component->SkeletalMesh, fbxCtx);
    }
    f.AddSkeletalMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->SkeletalMesh, "/") + fbxName).c_str());
  }
  AddCommonPrimitiveComponentParameters(f, component);
};

ComponentDataFunc ExportSpeedTreeComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  USpeedTreeComponent* component = Cast<USpeedTreeComponent>(acomp);
  if (!component || !component->SpeedTree)
  {
    return;
  }
  std::filesystem::path path = ctx.GetSpeedTreeDir() / GetLocalDir(component->SpeedTree);
  std::error_code err;
  if (!std::filesystem::exists(path, err))
  {
    std::filesystem::create_directories(path, err);
  }
  if (std::filesystem::exists(path, err))
  {
    path /= component->SpeedTree->GetObjectName().UTF8();
    path.replace_extension("spt");
    if (!std::filesystem::exists(path, err))
    {
      void* sptData = nullptr;
      FILE_OFFSET sptDataSize = 0;
      component->SpeedTree->GetSptData(&sptData, &sptDataSize, false);
      std::ofstream s(path, std::ios::out | std::ios::trunc | std::ios::binary);
      s.write((const char*)sptData, sptDataSize);
      free(sptData);
    }
    f.AddStaticMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->SpeedTree, "/") + component->SpeedTree->GetObjectName().UTF8()).c_str());
  }
  AddCommonPrimitiveComponentParameters(f, component);
};

ComponentDataFunc ExportPointLightComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UPointLightComponent* component = Cast<UPointLightComponent>(acomp);
  if (!component)
  {
    return;
  }
  f.AddFloat("LightFalloffExponent", component->FalloffExponent);
  f.AddFloat("Intensity", component->Brightness* ctx.Config.PointLightMul);
  f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
  f.AddFloat("AttenuationRadius", component->Radius);
  f.AddCustom("LightmassSettings", wxString::Format("(ShadowExponent=%.06f)", component->ShadowFalloffExponent).c_str());
  f.AddColor("LightColor", component->LightColor);
  f.AddBool("CastShadows", component->CastShadows);
  f.AddBool("CastStaticShadows", component->CastStaticShadows);
  f.AddBool("CastDynamicShadows", component->CastDynamicShadows);
  if (!component->bEnabled)
  {
    f.AddBool("bAffectsWorld", false);
  }
  if (component->LightGuid.IsValid())
  {
    f.AddGuid("LightGuid", component->LightGuid);
  }
};

ComponentDataFunc ExportSpotLightComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  USpotLightComponent* component = Cast<USpotLightComponent>(acomp);
  if (!component)
  {
    return;
  }
  f.AddFloat("LightFalloffExponent", component->FalloffExponent);
  f.AddFloat("Intensity", component->Brightness* ctx.Config.SpotLightMul);
  f.AddFloat("InnerConeAngle", component->InnerConeAngle);
  f.AddFloat("OuterConeAngle", component->OuterConeAngle);
  f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
  f.AddFloat("AttenuationRadius", component->Radius);
  f.AddCustom("LightmassSettings", wxString::Format("(ShadowExponent=%.06f)", component->ShadowFalloffExponent).c_str());
  f.AddColor("LightColor", component->LightColor);
  f.AddBool("CastShadows", component->CastShadows);
  f.AddBool("CastStaticShadows", component->CastStaticShadows);
  f.AddBool("CastDynamicShadows", component->CastDynamicShadows);
  if (!component->bEnabled)
  {
    f.AddBool("bAffectsWorld", false);
  }
  if (component->LightGuid.IsValid())
  {
    f.AddGuid("LightGuid", component->LightGuid);
  }
};

ComponentDataFunc ExportDirectionalLightComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UDirectionalLightComponent* component = Cast<UDirectionalLightComponent>(acomp);
  if (!component)
  {
    return;
  }
  f.AddFloat("Intensity", component->Brightness * ctx.Config.SpotLightMul);
  f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
  f.AddColor("LightColor", component->LightColor);
  f.AddBool("CastShadows", component->CastShadows);
  f.AddBool("CastStaticShadows", component->CastStaticShadows);
  f.AddBool("CastDynamicShadows", component->CastDynamicShadows);
  if (!component->bEnabled)
  {
    f.AddBool("bAffectsWorld", false);
  }
  if (component->LightGuid.IsValid())
  {
    f.AddGuid("LightGuid", component->LightGuid);
  }
};

ComponentDataFunc ExportSkyLightComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  USkyLightComponent* component = Cast<USkyLightComponent>(acomp);
  if (!component)
  {
    return;
  }
  if (component->LowerBrightness)
  {
    FLinearColor lowColor = component->LowerColor;
    f.AddLinearColor("LowerHemisphereColor", lowColor * component->LowerBrightness);
  }
  f.AddFloat("Intensity", component->Brightness * ctx.Config.SpotLightMul);
  f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
  f.AddColor("LightColor", component->LightColor);
  f.AddBool("CastShadows", component->CastShadows);
  f.AddBool("CastStaticShadows", component->CastStaticShadows);
  f.AddBool("CastDynamicShadows", component->CastDynamicShadows);
  if (!component->bEnabled)
  {
    f.AddBool("bAffectsWorld", false);
  }
  if (component->LightGuid.IsValid())
  {
    f.AddGuid("LightGuid", component->LightGuid);
  }
};

ComponentDataFunc ExportHeightFogComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UHeightFogComponent* component = Cast<UHeightFogComponent>(acomp);
  if (!component)
  {
    return;
  }
  f.AddFloat("FogDensity", component->Density);
  f.AddFloat("StartDistance", component->StartDistance);
  f.AddFloat("FogCutoffDistance", component->ExtinctionDistance);
  f.AddLinearColor("FogInscatteringColor", component->LightColor);
};

struct T3DComponent {
  enum class ComponentMobility {
    Static = 0,
    Stationary,
    Movable
  };

  FString Name;
  FString Class;

  FVector Location;
  FRotator Rotation;
  FVector Scale3D = FVector::One;
  float Scale = 1.f;

  ComponentMobility Mobility = ComponentMobility::Static;
  bool IsInstance = false;

  ComponentDataFunc DataFunc;
  UActorComponent* ActorComponent = nullptr;
  T3DComponent* Parent = nullptr;

  T3DComponent() = default;

  T3DComponent(UActorComponent* comp)
    : Name(comp->GetObjectName())
    , Class(comp->GetClassName())
    , Location(comp->Translation)
    , Rotation(comp->Rotation)
    , Scale3D(comp->Scale3D)
    , Scale(comp->Scale)
    , ActorComponent(comp)
  {}

  T3DComponent(UActorComponent* comp, ComponentDataFunc& func)
    : Name(comp->GetObjectName())
    , Class(comp->GetClassName())
    , Location(comp->Translation)
    , Rotation(comp->Rotation)
    , Scale3D(comp->Scale3D)
    , Scale(comp->Scale)
    , DataFunc(func)
    , ActorComponent(comp)
  {}

  // RootTransform
  T3DComponent(UActor* actor)
    : Name("RootTransform")
    , Class("SceneComponent")
    , Location(actor->GetLocation())
    , Rotation(actor->Rotation)
    , Scale3D(actor->DrawScale3D)
    , Scale(actor->DrawScale)
  {}

  void Declare(T3DFile& f) const
  {
    f.Begin("Object", Class.UTF8().c_str(), Name.UTF8().c_str());
    f.End();
  }

  bool NeedsParent() const
  {
    return !Location.IsZero() || !Rotation.IsZero() || Scale3D != FVector::One || Scale != 1.f;
  }

  void TakeActorTransform(UActor* actor)
  {
    if (!actor)
    {
      return;
    }
    Location = actor->GetLocation();
    Rotation = actor->Rotation;
    Scale3D = actor->DrawScale3D;
    Scale = actor->DrawScale;
  }

  void TakeComponentTransform(UActorComponent* component)
  {
    if (!component)
    {
      return;
    }
    Location = component->Translation;
    Rotation = component->Rotation;
    Scale3D = component->Scale3D;
    Scale = component->Scale;
  }

  void Define(T3DFile& f, LevelExportContext& ctx) const
  {
    f.Begin("Object", nullptr, Name.UTF8().c_str());
    if (DataFunc && ActorComponent)
    {
      DataFunc(f, ctx, ActorComponent);
    }
    if (!Location.IsZero())
    {
      f.AddPosition(Location);
    }
    if (Scale3D.X < 0.f && Cast<USpotLightComponent>(ActorComponent))
    {
      FRotator inv = Rotation;
      inv.Pitch += 0x8000;
      f.AddRotation(inv);
    }
    else if (!Rotation.IsZero())
    {
      f.AddRotation(Rotation);
    }
    if (Scale3D != FVector::One || Scale != 1.f)
    {
      if (!Cast<USpotLightComponent>(ActorComponent))
      {
        f.AddScale(Scale3D, Scale);
      }
    }
    if (IsInstance)
    {
      f.AddCustom("CreationMethod", "Instance");
    }
    if (Parent)
    {
      f.AddCustom("AttachParent", FString::Sprintf("%s'\"%s\"'", Parent->Class.UTF8().c_str(), Parent->Name.UTF8().c_str()).UTF8().c_str());
    }
    else if (IsInstance)
    {
      // Don't show root instance
      f.AddBool("bVisualizeComponent", false);
    }
    switch (Mobility)
    {
    case ComponentMobility::Static:
      f.AddCustom("Mobility", "Static");
      break;
    case ComponentMobility::Stationary:
      f.AddCustom("Mobility", "Stationary");
      break;
    case ComponentMobility::Movable:
      f.AddCustom("Mobility", "Movable");
      break;
    }
    f.End();
  }

  ComponentDataFunc Data;
};

struct T3DActor {
  FString Name;
  FString Class;
  FString Type;

  std::list<T3DComponent> Components;
  T3DComponent* RootComponent = nullptr;
  std::map<FString, T3DComponent*> Properties;

  bool ConfigureStaticMeshActor(UStaticMeshActor* actor)
  {
    if (!actor || !actor->StaticMeshComponent || !actor->StaticMeshComponent->StaticMesh)
    {
      return false;
    }
    Name = actor->GetObjectName();
    T3DComponent& component = Components.emplace_back(actor->StaticMeshComponent, ExportStaticMeshComponentData);
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      component.Parent = RootComponent;
      RootComponent->IsInstance = true;
      component.IsInstance = true;
    }
    else
    {
      Class = "StaticMeshActor";
      RootComponent = &component;
      Properties["StaticMeshComponent"] = &component;
    }
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigureInterpActor(UInterpActor* actor)
  {
    if (!actor || !actor->StaticMeshComponent || !actor->StaticMeshComponent->StaticMesh)
    {
      return false;
    }
    Name = actor->GetObjectName();
    T3DComponent& component = Components.emplace_back(actor->StaticMeshComponent, ExportStaticMeshComponentData);
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      component.Parent = RootComponent;
      RootComponent->IsInstance = true;
      component.IsInstance = true;
    }
    else
    {
      Class = "StaticMeshActor";
      RootComponent = &component;
      Properties["StaticMeshComponent"] = &component;
    }
    component.Mobility = T3DComponent::ComponentMobility::Movable;
    RootComponent->Mobility = T3DComponent::ComponentMobility::Movable;
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigureSpeedTreeActor(USpeedTreeActor* actor)
  {
    if (!actor || !actor->SpeedTreeComponent || !actor->SpeedTreeComponent->SpeedTree)
    {
      return false;
    }
    Name = actor->GetObjectName();
    T3DComponent& component = Components.emplace_back(actor->SpeedTreeComponent, ExportSpeedTreeComponentData);
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      component.Parent = RootComponent;
      RootComponent->IsInstance = true;
      component.IsInstance = true;
    }
    else
    {
      Class = "StaticMeshActor";
      RootComponent = &component;
      Properties["StaticMeshComponent"] = &component;
    }
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigureSkelMeshActor(USkeletalMeshActor* actor)
  {
    if (!actor || !actor->SkeletalMeshComponent || !actor->SkeletalMeshComponent->SkeletalMesh)
    {
      return false;
    }
    Name = actor->GetObjectName();
    T3DComponent& component = Components.emplace_back(actor->SkeletalMeshComponent, ExportSkeletalMeshComponentData);
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      component.Parent = RootComponent;
      RootComponent->IsInstance = true;
      component.IsInstance = true;
    }
    else
    {
      Class = "SkeletalMeshActor";
      RootComponent = &component;
      Properties["SkeletalMeshComponent"] = &component;
    }
    component.Mobility = T3DComponent::ComponentMobility::Movable;
    RootComponent->Mobility = T3DComponent::ComponentMobility::Movable;
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigurePointLightActor(UPointLight* actor)
  {
    if (!actor || !actor->LightComponent)
    {
      return false;
    }
    Name = actor->GetObjectName();
    T3DComponent& component = Components.emplace_back(actor->LightComponent, ExportPointLightComponentData);
    component.Name = "LightComponent0";
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      component.Parent = RootComponent;
      RootComponent->IsInstance = true;
      component.IsInstance = true;
    }
    else
    {
      Class = "PointLight";
      RootComponent = &component;
      Properties["LightComponent"] = &component;
      Properties["PointLightComponent"] = &component;
    }
    if (actor->GetClassName() == UPointLightMovable::StaticClassName())
    {
      RootComponent->Mobility = T3DComponent::ComponentMobility::Movable;
      component.Mobility = T3DComponent::ComponentMobility::Movable;
    }
    else if (actor->GetClassName() == UPointLightToggleable::StaticClassName())
    {
      RootComponent->Mobility = T3DComponent::ComponentMobility::Stationary;
      component.Mobility = T3DComponent::ComponentMobility::Stationary;
    }
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigureSpotLightActor(USpotLight* actor)
  {
    if (!actor || !actor->LightComponent)
    {
      return false;
    }
    Name = actor->GetObjectName();
    T3DComponent& component = Components.emplace_back(actor->LightComponent, ExportSpotLightComponentData);
    component.Name = "LightComponent0";
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      component.Parent = RootComponent;
      RootComponent->IsInstance = true;
      component.IsInstance = true;
    }
    else
    {
      Class = "SpotLight";
      RootComponent = &component;
      Properties["LightComponent"] = &component;
      Properties["SpotLightComponent"] = &component;
    }
    if (actor->GetClassName() == USpotLightMovable::StaticClassName())
    {
      RootComponent->Mobility = T3DComponent::ComponentMobility::Movable;
      component.Mobility = T3DComponent::ComponentMobility::Movable;
    }
    else if (actor->GetClassName() == USpotLightToggleable::StaticClassName())
    {
      RootComponent->Mobility = T3DComponent::ComponentMobility::Stationary;
      component.Mobility = T3DComponent::ComponentMobility::Stationary;
    }
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigureDirectionalLightActor(UDirectionalLight* actor)
  {
    if (!actor || !actor->LightComponent)
    {
      return false;
    }
    Name = actor->GetObjectName();
    T3DComponent& component = Components.emplace_back(actor->LightComponent, ExportDirectionalLightComponentData);
    component.Name = "LightComponent0";
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      component.Parent = RootComponent;
      RootComponent->IsInstance = true;
      component.IsInstance = true;
    }
    else
    {
      Class = "DirectionalLight";
      RootComponent = &component;
      Properties["LightComponent"] = &component;
      Properties["DirectionalLightComponent"] = &component;
    }
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigureSkyLightActor(USkyLight* actor)
  {
    if (!actor || !actor->LightComponent)
    {
      return false;
    }
    Name = actor->GetObjectName();
    T3DComponent& component = Components.emplace_back(actor->LightComponent, ExportSkyLightComponentData);
    component.Name = "LightComponent0";
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      component.Parent = RootComponent;
      RootComponent->IsInstance = true;
      component.IsInstance = true;
    }
    else
    {
      Class = "SkyLight";
      RootComponent = &component;
      Properties["LightComponent"] = &component;
      Properties["SkyLightComponent"] = &component;
    }
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigureHeightFogActor(UHeightFog* actor)
  {
    if (!actor || !actor->Component)
    {
      return false;
    }
    Name = actor->GetObjectName();
    T3DComponent& component = Components.emplace_back(actor->Component, ExportHeightFogComponentData);
    component.Name = "ExponentialHeightFogComponent0";
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      component.Parent = RootComponent;
      RootComponent->IsInstance = true;
      component.IsInstance = true;
    }
    else
    {
      Class = "ExponentialHeightFog";
      RootComponent = &component;
      Properties["ExponentialHeightFogComponent"] = &component;
    }
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigureEmitterActor(UEmitter* actor)
  {
    if (!actor)
    {
      return false;
    }
    Name = actor->GetObjectName();
    Class = "Actor";
    T3DComponent& component = Components.emplace_back();
    component.Name = "EmitterComponent0";
    component.Class = "SceneComponent";
    RootComponent = &component;
    RootComponent->TakeActorTransform(actor);
    return true;
  }

  bool ConfigurePrefabInstance(UPrefabInstance* actor)
  {
    if (!actor)
    {
      return false;
    }
    Name = actor->GetObjectName();
    Class = "Actor";

    UPrefab* prefab = Cast<UPrefab>(actor->TemplatePrefab);
    if (!prefab)
    {
      return false;
    }
    prefab->Load();

    if (prefab->PrefabArchetypes.empty())
    {
      return false;
    }

    RootComponent = &Components.emplace_back(actor);
    RootComponent->IsInstance = true;

    for (UObject* archertype : prefab->PrefabArchetypes)
    {
      T3DActor tmp;
      if (UStaticMeshActor* typed = Cast<UStaticMeshActor>(archertype))
      {
        if (!typed->StaticMeshComponent || !typed->StaticMeshComponent->StaticMesh)
        {
          continue;
        }
        T3DComponent component(typed->StaticMeshComponent, ExportStaticMeshComponentData);
        component.IsInstance = true;
        if (component.NeedsParent())
        {
          T3DComponent& root = Components.emplace_back(typed);
          root.IsInstance = true;
          root.Parent = RootComponent;
          component.Parent = &root;
        }
        else
        {
          component.Name = typed->GetObjectName();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (USkeletalMeshActor* typed = Cast<USkeletalMeshActor>(archertype))
      {
        if (!typed->SkeletalMeshComponent || !typed->SkeletalMeshComponent->SkeletalMesh)
        {
          continue;
        }
        T3DComponent component(typed->SkeletalMeshComponent, ExportSkeletalMeshComponentData);
        component.IsInstance = true;
        if (component.NeedsParent())
        {
          T3DComponent& root = Components.emplace_back(typed);
          root.IsInstance = true;
          root.Parent = RootComponent;
          component.Parent = &root;
        }
        else
        {
          component.Name = typed->GetObjectName();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (UInterpActor* typed = Cast<UInterpActor>(archertype))
      {
        if (!typed->StaticMeshComponent || !typed->StaticMeshComponent->StaticMesh)
        {
          continue;
        }
        T3DComponent component(typed->StaticMeshComponent, ExportStaticMeshComponentData);
        component.IsInstance = true;
        if (component.NeedsParent())
        {
          T3DComponent& root = Components.emplace_back(typed);
          root.IsInstance = true;
          root.Parent = RootComponent;
          component.Parent = &root;
        }
        else
        {
          component.Name = typed->GetObjectName();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (USpeedTreeActor* typed = Cast<USpeedTreeActor>(archertype))
      {
        if (!typed->SpeedTreeComponent || !typed->SpeedTreeComponent->SpeedTree)
        {
          continue;
        }
        T3DComponent component(typed->SpeedTreeComponent, ExportSpeedTreeComponentData);
        component.IsInstance = true;
        if (component.NeedsParent())
        {
          T3DComponent& root = Components.emplace_back(typed);
          root.IsInstance = true;
          root.Parent = RootComponent;
          component.Parent = &root;
        }
        else
        {
          component.Name = typed->GetObjectName();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (UPointLight* typed = Cast<UPointLight>(archertype))
      {
        if (!typed->LightComponent)
        {
          continue;
        }
        T3DComponent component(typed->LightComponent, ExportPointLightComponentData);
        component.IsInstance = true;
        if (component.NeedsParent())
        {
          T3DComponent& root = Components.emplace_back(typed);
          root.IsInstance = true;
          root.Parent = RootComponent;
          component.Parent = &root;
        }
        else
        {
          component.Name = typed->GetObjectName();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (USpotLight* typed = Cast<USpotLight>(archertype))
      {
        if (!typed->LightComponent)
        {
          continue;
        }
        T3DComponent component(typed->LightComponent, ExportSpotLightComponentData);
        component.IsInstance = true;
        if (component.NeedsParent())
        {
          T3DComponent& root = Components.emplace_back(typed);
          root.IsInstance = true;
          root.Parent = RootComponent;
          component.Parent = &root;
        }
        else
        {
          component.Name = typed->GetObjectName();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (UDirectionalLight* typed = Cast<UDirectionalLight>(archertype))
      {
        if (!typed->LightComponent)
        {
          continue;
        }
        T3DComponent component(typed->LightComponent, ExportDirectionalLightComponentData);
        component.IsInstance = true;
        if (component.NeedsParent())
        {
          T3DComponent& root = Components.emplace_back(typed);
          root.IsInstance = true;
          root.Parent = RootComponent;
          component.Parent = &root;
        }
        else
        {
          component.Name = typed->GetObjectName();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (USkyLight* typed = Cast<USkyLight>(archertype))
      {
        if (!typed->LightComponent)
        {
          continue;
        }
        T3DComponent component(typed->LightComponent, ExportSkyLightComponentData);
        component.IsInstance = true;
        if (component.NeedsParent())
        {
          T3DComponent& root = Components.emplace_back(typed);
          root.IsInstance = true;
          root.Parent = RootComponent;
          component.Parent = &root;
        }
        else
        {
          component.Name = typed->GetObjectName();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (UHeightFog* typed = Cast<UHeightFog>(archertype))
      {
        if (!typed->Component)
        {
          continue;
        }
        T3DComponent component(typed->Component, ExportHeightFogComponentData);
        component.IsInstance = true;
        if (component.NeedsParent())
        {
          T3DComponent& root = Components.emplace_back(typed);
          root.IsInstance = true;
          root.Parent = RootComponent;
          component.Parent = &root;
        }
        else
        {
          component.Name = typed->GetObjectName();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (UEmitter* typed = Cast<UEmitter>(archertype))
      {
        T3DComponent& component = Components.emplace_back(typed);
        component.Name = typed->GetObjectName();
        component.Class = "SceneComponent";
        component.IsInstance = true;
        component.Parent = RootComponent;
      }
      else
      {
        LogW("Prefab archertype %s is not supported!", actor->GetClassName().UTF8().c_str());
        continue;
      }
    }

    return Components.size() > 1;
  }
};

void LevelEditor::PrepareToExportLevel(LevelExportContext& ctx)
{
  UObject* world = Level->GetOuter();
  auto worldInner = world->GetInner();
  std::vector<ULevel*> levels = { Level };
  int maxProgress = Level->GetActorsCount();
  ProgressWindow progress(this, "Please wait...");
  progress.SetActionText("Preparing...");

  std::thread([&] {

    for (UObject* inner : worldInner)
    {
      if (ULevelStreaming* streamedLevel = Cast<ULevelStreaming>(inner))
      {
        streamedLevel->Load();
        if (streamedLevel->Level && streamedLevel->Level != Level)
        {
          levels.push_back(streamedLevel->Level);
          maxProgress += streamedLevel->Level->GetActorsCount();
        }
      }
    }

    SendEvent(&progress, UPDATE_MAX_PROGRESS, maxProgress);

    T3DFile file;
    if (!ctx.Config.SplitT3D)
    {
      file.InitializeMap();
    }
    for (ULevel* level : levels)
    {
      ExportLevel(file, level, ctx, &progress);

      if (progress.IsCanceled())
      {
        SendEvent(&progress, UPDATE_PROGRESS_FINISH);
        return;
      }
    }
    if (!ctx.Config.SplitT3D)
    {
      file.FinalizeMap();
      std::filesystem::path dst = std::filesystem::path(ctx.Config.RootDir.WString()) / Level->GetPackage()->GetPackageName().WString();
      dst.replace_extension("t3d");
      file.Save(dst);
    }
    if (ctx.Config.Materials || ctx.Config.Textures)
    {
      if (!ExportMaterialsAndTexture(ctx, &progress))
      {
        // UPDATE_PROGRESS_FINISH was sent inside of the ExportMaterialsAndTexture
        return;
      }
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();
}

void LevelEditor::ExportLevel(T3DFile& f, ULevel* level, LevelExportContext& ctx, ProgressWindow* progress)
{
  SendEvent(progress, UPDATE_PROGRESS_DESC, wxString("Preparing: " + level->GetPackage()->GetPackageName().UTF8()));
  std::vector<UActor*> actors = level->GetActors();

  if (actors.empty())
  {
    return;
  }
  T3DFile lightF;
  size_t lightInitialSize = 0;
  size_t initialSize = 0;
  if (ctx.Config.SplitT3D)
  {
    f.Clear();
    f.InitializeMap();
    initialSize = f.GetBody().size();
  }
  else
  {
    lightF.InitializeMap();
    lightInitialSize = lightF.GetBody().size();
  }
  
  for (UActor* actor : actors)
  {
    if (progress)
    {
      if (progress->IsCanceled())
      {
        return;
      }
      ctx.CurrentProgress++;
      SendEvent(progress, UPDATE_PROGRESS, ctx.CurrentProgress);
      if (actor)
      {
        FString name = level->GetPackage()->GetPackageName() + "\\" + actor->GetObjectName();
        SendEvent(progress, UPDATE_PROGRESS_DESC, wxString(L"Exporting: " + name.WString()));
      }
    }
    if (!actor)
    {
      continue;
    }
    if (actor->GetClassName() == UTerrain::StaticClassName())
    {
      if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::StaticMeshes))
      {
        continue;
      }
      ExportTerrainActor(f, ctx, Cast<UTerrain>(actor));
    }
    if (!ctx.Config.SplitT3D && Cast<ULight>(actor))
    {
      ExportActor(lightF, ctx, actor);
    }
    else
    {
      ExportActor(f, ctx, actor);
    }
  }

  std::filesystem::path dst = std::filesystem::path(ctx.Config.RootDir.WString()) / level->GetPackage()->GetPackageName().WString();
  if (ctx.Config.SplitT3D && initialSize != f.GetBody().size())
  {
    f.FinalizeMap();
    dst.replace_extension("t3d");
    f.Save(dst);
  }
  if (!ctx.Config.SplitT3D && lightInitialSize != lightF.GetBody().size())
  {
    lightF.FinalizeMap();
    lightF.Save(dst + "_lights.t3d");
  }
}

bool LevelEditor::ExportMaterialsAndTexture(LevelExportContext& ctx, ProgressWindow* progress)
{
  if (ctx.Config.Materials)
  {
    SendEvent(progress, UPDATE_PROGRESS_DESC, wxString("Saving materials..."));
    SendEvent(progress, UPDATE_PROGRESS, 0);
    SendEvent(progress, UPDATE_MAX_PROGRESS, ctx.UsedMaterials.size());
  }
  else
  {
    SendEvent(progress, UPDATE_PROGRESS_DESC, wxString("Gathering textures..."));
    SendEvent(progress, UPDATE_PROGRESS, -1);
  }
  std::map<std::string, UTexture*> textures;
  int curProgress = 0;
  for (UObject* obj : ctx.UsedMaterials)
  {
    if (ctx.Config.Materials)
    {
      SendEvent(progress, UPDATE_PROGRESS, curProgress);
    }
    if (progress->IsCanceled())
    {
      SendEvent(progress, UPDATE_PROGRESS_FINISH);
      return false;
    }
    curProgress++;
    if (!obj)
    {
      continue;
    }
    std::filesystem::path path = ctx.GetMaterialDir() / GetLocalDir(obj);
    std::error_code err;
    if (!std::filesystem::exists(path, err))
    {
      if (!std::filesystem::create_directories(path, err))
      {
        LogW("Failed to save material %s", obj->GetObjectName().UTF8().c_str());
        continue;
      }
    }
    path /= obj->GetObjectName().UTF8();
    path.replace_extension("txt");
    if (!ctx.Config.OverrideData && std::filesystem::exists(path, err))
    {
      continue;
    }

    if (UMaterialInterface* mat = Cast<UMaterialInterface>(obj))
    {
      mat->Load();
      auto textureParams = mat->GetTextureParameters();

      if (ctx.Config.Materials)
      {
        std::ofstream s(path, std::ios::out);
        s << mat->GetObjectName().UTF8() << '(' << mat->GetClassName().UTF8() << ")\n";
        if (UObject* parent = mat->GetParent())
        {
          s << "Parent: " << GetLocalDir(parent) << parent->GetObjectName().UTF8() << '\n';
        }

        if (textureParams.size())
        {
          s << "\nTexture Parameters:\n";
          for (const auto& p : textureParams)
          {
            s << "  " << p.first.UTF8() << ": ";
            if (p.second)
            {
              s << GetLocalDir(p.second) << p.second->GetObjectName().UTF8();
            }
            else
            {
              s << "None";
            }
            s << '\n';
          }
        }

        auto scalarParameters = mat->GetScalarParameters();
        if (scalarParameters.size())
        {
          s << "\nScalar Parameters:\n";
          for (const auto& p : scalarParameters)
          {
            s << "  " << p.first.UTF8() << ": " << std::to_string(p.second) << '\n';
          }
        }

        auto vectorParameters = mat->GetVectorParameters();
        if (vectorParameters.size())
        {
          s << "\nVector Parameters:\n";
          for (const auto& p : vectorParameters)
          {
            s << "  " << p.first.UTF8() << ": ";
            s << "( R: " << std::to_string(p.second.R);
            s << ", G: " << std::to_string(p.second.G);
            s << ", B: " << std::to_string(p.second.B);
            s << ", A: " << std::to_string(p.second.A);
            s << " )\n";
          }
        }
      }

      if (ctx.Config.Textures)
      {
        if (UMaterial* parent = Cast<UMaterial>(mat->GetParent()))
        {
          auto parentTexParams = parent->GetTextureParameters();
          for (auto& p : parentTexParams)
          {
            if (p.second && !textureParams.count(p.first))
            {
              textureParams[p.first] = p.second;
            }
          }
        }
        for (auto& p : textureParams)
        {
          if (!p.second)
          {
            continue;
          }
          std::string tpath = GetLocalDir(p.second) + p.second->GetObjectName().UTF8();
          if (!textures.count(tpath))
          {
            textures[tpath] = p.second;
          }
        }
        auto constTextures = mat->GetTextureSamples();
        for (UTexture* tex : constTextures)
        {
          if (!tex)
          {
            continue;
          }
          std::string tpath = GetLocalDir(tex) + tex->GetObjectName().UTF8();
          if (!textures.count(tpath))
          {
            textures[tpath] = tex;
          }
        }
      }
    }
  }

  if (ctx.Config.Textures && textures.size())
  {
    SendEvent(progress, UPDATE_PROGRESS, 0);
    SendEvent(progress, UPDATE_MAX_PROGRESS, textures.size());
    curProgress = 0;

    TextureProcessor::TCFormat outputFormat = ctx.GetTextureFormat();
    std::string ext;
    switch (outputFormat)
    {
    case TextureProcessor::TCFormat::PNG:
      ext = "png";
      break;
    case TextureProcessor::TCFormat::TGA:
      ext = "tga";
      break;
    case TextureProcessor::TCFormat::DDS:
      ext = "dds";
      break;
    default:
      LogE("Invalid output format!");
      SendEvent(progress, UPDATE_PROGRESS_FINISH);
      break;
    }

    for (auto& p : textures)
    {
      if (progress->IsCanceled())
      {
        SendEvent(progress, UPDATE_PROGRESS_FINISH);
        return false;
      }
      SendEvent(progress, UPDATE_PROGRESS, curProgress);
      curProgress++;
      if (!p.second)
      {
        continue;
      }

      SendEvent(progress, UPDATE_PROGRESS_DESC, wxString("Exporing: ") + p.second->GetObjectName().UTF8());

      std::error_code err;
      std::filesystem::path path = ctx.GetTextureDir() / GetLocalDir(p.second);
      if (!std::filesystem::exists(path, err))
      {
        std::filesystem::create_directories(path, err);
      }
      path /= p.second->GetObjectName().UTF8();
      path.replace_extension(ext);

      if (UTexture2D* texture = Cast<UTexture2D>(p.second))
      {
        TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
        switch (texture->Format)
        {
        case PF_DXT1:
          inputFormat = TextureProcessor::TCFormat::DXT1;
          break;
        case PF_DXT3:
          inputFormat = TextureProcessor::TCFormat::DXT3;
          break;
        case PF_DXT5:
          inputFormat = TextureProcessor::TCFormat::DXT5;
          break;
        case PF_A8R8G8B8:
          inputFormat = TextureProcessor::TCFormat::ARGB8;
          break;
        case PF_G8:
          inputFormat = TextureProcessor::TCFormat::G8;
          break;
        default:
          LogE("Failed to export texture %s. Invalid format!", texture->GetObjectName().UTF8().c_str());
          continue;
        }

        if (!ctx.Config.OverrideData && std::filesystem::exists(path, err))
        {
          continue;
        }

        FTexture2DMipMap* mip = nullptr;
        for (FTexture2DMipMap* mipmap : texture->Mips)
        {
          if (mipmap->Data && mipmap->Data->GetAllocation() && mipmap->SizeX && mipmap->SizeY)
          {
            mip = mipmap;
            break;
          }
        }
        if (!mip)
        {
          LogE("Failed to export texture %s. No mipmaps!", texture->GetObjectName().UTF8().c_str());
          continue;
        }

        TextureProcessor processor(inputFormat, outputFormat);

        processor.SetInputData(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
        processor.SetOutputPath(W2A(path.wstring()));
        processor.SetInputDataDimensions(mip->SizeX, mip->SizeY);

        try
        {
          if (!processor.Process())
          {
            LogE("Failed to export %s: %s", texture->GetObjectName().UTF8().c_str(), processor.GetError().c_str());
            continue;
          }
        }
        catch (...)
        {
          LogE("Failed to export %s! Unknown texture processor error!", texture->GetObjectName().UTF8().c_str());
          continue;
        }
      }
      else if (UTextureCube* cube = Cast<UTextureCube>(p.second))
      {
        auto faces = cube->GetFaces();
        bool ok = true;
        TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
        for (UTexture2D* face : faces)
        {
          if (!face)
          {
            LogW("Failed to export a texture cube %s. Can't get one of the faces.", p.second->GetObjectPath().UTF8().c_str());
            ok = false;
            break;
          }
          TextureProcessor::TCFormat f = TextureProcessor::TCFormat::None;
          switch (face->Format)
          {
          case PF_DXT1:
            f = TextureProcessor::TCFormat::DXT1;
            break;
          case PF_DXT3:
            f = TextureProcessor::TCFormat::DXT3;
            break;
          case PF_DXT5:
            f = TextureProcessor::TCFormat::DXT5;
            break;
          case PF_A8R8G8B8:
            f = TextureProcessor::TCFormat::ARGB8;
            break;
          case PF_G8:
            f = TextureProcessor::TCFormat::G8;
            break;
          default:
            LogE("Failed to export texture cube %s.%s. Invalid face format!", p.second->GetObjectPath().UTF8().c_str(), face->GetObjectName().UTF8().c_str());
            ok = false;
            break;
          }
          if (inputFormat != TextureProcessor::TCFormat::None && inputFormat != f)
          {
            LogE("Failed to export texture cube %s.%s. Faces have different format!", p.second->GetObjectPath().UTF8().c_str(), face->GetObjectName().UTF8().c_str());
            ok = false;
            break;
          }
          inputFormat = f;
        }

        if (!ok)
        {
          continue;
        }

        // UE4 accepts texture cubes only in a DDS container with A8R8G8B8 format anf proper flags. Export cubes this way regardless of the user's output format.
        path.replace_extension("dds");
        TextureProcessor processor(inputFormat, TextureProcessor::TCFormat::DDS);
        for (int32 faceIdx = 0; faceIdx < faces.size(); ++faceIdx)
        {
          FTexture2DMipMap* mip = nullptr;
          for (FTexture2DMipMap* mipmap : faces[faceIdx]->Mips)
          {
            if (mipmap->Data && mipmap->Data->GetAllocation() && mipmap->SizeX && mipmap->SizeY)
            {
              mip = mipmap;
              break;
            }
          }
          if (!mip)
          {
            LogE("Failed to export texture cube face %s.%s. No mipmaps!", cube->GetObjectPath().UTF8().c_str(), faces[faceIdx]->GetObjectName().UTF8().c_str());
            ok = false;
            break;
          }

          processor.SetInputCubeFace(faceIdx, mip->Data->GetAllocation(), mip->Data->GetBulkDataSize(), mip->SizeX, mip->SizeY);
        }

        if (!ok)
        {
          continue;
        }

        processor.SetOutputPath(W2A(path.wstring()));

        try
        {
          if (!processor.Process())
          {
            LogE("Failed to export %s: %s", cube->GetObjectPath().UTF8().c_str(), processor.GetError().c_str());
            continue;
          }
        }
        catch (...)
        {
          LogE("Failed to export %s! Unknown texture processor error!", cube->GetObjectPath().UTF8().c_str());
          continue;
        }
      }
      else
      {
        LogW("Failed to export a texture object %s(%s). Class is not supported!", p.second->GetObjectPath().UTF8().c_str(), p.second->GetClassName().UTF8().c_str());
      }
    }
  }
  return true;
}

void AddCommonPrimitiveComponentParameters(T3DFile& f, UPrimitiveComponent* component)
{
  if (component->MinDrawDistance)
  {
    f.AddFloat("MinDrawDistance", component->MinDrawDistance);
  }
  if (component->MaxDrawDistance)
  {
    f.AddFloat("LDMaxDrawDistance", component->MaxDrawDistance);
  }
  if (component->CachedMaxDrawDistance)
  {
    f.AddFloat("CachedMaxDrawDistance", component->CachedMaxDrawDistance);
  }
  f.AddBool("CastShadow", component->CastShadow);
  f.AddBool("bCastDynamicShadow", component->bCastDynamicShadow);
  f.AddBool("bCastStaticShadow", component->bCastStaticShadow);
  if (!component->bAcceptsLights)
  {
    f.AddCustom("LightingChannels", "(bChannel0=False)");
  }
}

void SaveMaterialMap(UMeshComponent* component, LevelExportContext& ctx, const std::vector<UObject*>& materials)
{
  if (!component)
  {
    return;
  }
  UActor* actor = Cast<UActor>(component->GetOuter());
  std::vector<UObject*> materialsToSave = materials;
  for (int matIdx = 0; matIdx < component->Materials.size(); ++matIdx)
  {
    if (component->Materials[matIdx] && matIdx < materialsToSave.size())
    {
      materialsToSave[matIdx] = component->Materials[matIdx];
    }
  }
  ctx.UsedMaterials.insert(ctx.UsedMaterials.end(), materialsToSave.begin(), materialsToSave.end());
  if (ctx.Config.Materials)
  {
    if (materialsToSave.size())
    {
      std::error_code err;
      std::filesystem::path path = ctx.GetMaterialMapDir() / (actor ? (UObject*)actor : (UObject*)component)->GetPackage()->GetPackageName().UTF8();
      if (!std::filesystem::exists(path, err))
      {
        std::filesystem::create_directories(path, err);
      }
      if (std::filesystem::exists(path, err))
      {
        path /= (actor ? (UObject*)actor : (UObject*)component)->GetObjectName().UTF8();
        path.replace_extension("txt");

        if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
        {
          std::ofstream s(path, std::ios::out);
          for (int idx = 0; idx < materialsToSave.size(); ++idx)
          {
            s << std::to_string(idx + 1) << ". ";
            if (UObject* mat = materialsToSave[idx])
            {
              s << mat->GetObjectPath().UTF8();
            }
            else
            {
              s << "None";
            }
            s << '\n';
          }
        }
      }
    }
  }
}

void ExportActor(T3DFile& f, LevelExportContext& ctx, UActor* untypedActor)
{
  if (!untypedActor)
  {
    return;
  }
  T3DActor exportItem;
  if (UStaticMeshActor* actor = Cast<UStaticMeshActor>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::StaticMeshes))
    {
      return;
    }
    if (!exportItem.ConfigureStaticMeshActor(actor))
    {
      return;
    }
  }
  else if (USkeletalMeshActor* actor = Cast<USkeletalMeshActor>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::SkeletalMeshes))
    {
      return;
    }
    if (!exportItem.ConfigureSkelMeshActor(actor))
    {
      return;
    }
  }
  else if (UInterpActor* actor = Cast<UInterpActor>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::Interps))
    {
      return;
    }
    if (!exportItem.ConfigureInterpActor(actor))
    {
      return;
    }
  }
  else if (USpeedTreeActor* actor = Cast<USpeedTreeActor>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::SpeedTrees))
    {
      return;
    }
    if (!exportItem.ConfigureSpeedTreeActor(actor))
    {
      return;
    }
  }
  else if (UPointLight* actor = Cast<UPointLight>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::PointLights))
    {
      return;
    }
    if (!exportItem.ConfigurePointLightActor(actor))
    {
      return;
    }
  }
  else if (USpotLight* actor = Cast<USpotLight>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::SpotLights))
    {
      return;
    }
    if (!exportItem.ConfigureSpotLightActor(actor))
    {
      return;
    }
  }
  else if (UDirectionalLight* actor = Cast<UDirectionalLight>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::DirectionalLights))
    {
      return;
    }
    if (!exportItem.ConfigureDirectionalLightActor(actor))
    {
      return;
    }
  }
  else if (USkyLight* actor = Cast<USkyLight>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::SkyLights))
    {
      return;
    }
    if (!exportItem.ConfigureSkyLightActor(actor))
    {
      return;
    }
  }
  else if (UHeightFog* actor = Cast<UHeightFog>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::HeightFog))
    {
      return;
    }
    if (!exportItem.ConfigureHeightFogActor(actor))
    {
      return;
    }
  }
  else if (UEmitter* actor = Cast<UEmitter>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::Emitters))
    {
      return;
    }
    if (!exportItem.ConfigureEmitterActor(actor))
    {
      return;
    }
  }
  else if (UTerrain* actor = Cast<UTerrain>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::Terrains))
    {
      return;
    }
    ExportTerrainActor(f, ctx, actor);
    return;
  }
  else if (UPrefabInstance* actor = Cast<UPrefabInstance>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::Prefabs))
    {
      return;
    }
    if (!exportItem.ConfigurePrefabInstance(actor))
    {
      return;
    }
  }
  else
  {
    return;
  }

  f.Begin("Actor", exportItem.Class.UTF8().c_str(), exportItem.Name.UTF8().c_str());
  {
    for (const T3DComponent& component : exportItem.Components)
    {
      component.Declare(f);
    }
    for (const T3DComponent& component : exportItem.Components)
    {
      component.Define(f, ctx);
    }

    f.AddCustom("RootComponent", FString::Sprintf("%s'\"%s\"'", exportItem.RootComponent->Class.UTF8().c_str(), exportItem.RootComponent->Name.UTF8().c_str()).UTF8().c_str());
    for (const auto& p : exportItem.Properties)
    {
      f.AddCustom(p.first.UTF8().c_str(), FString::Sprintf("%s'\"%s\"'", p.second->Class.UTF8().c_str(), p.second->Name.UTF8().c_str()).UTF8().c_str());
    }

    {
      int32 inctanceIdx = 0;
      for (const T3DComponent& component : exportItem.Components)
      {
        if (component.IsInstance)
        {
          f.AddCustom(FString::Sprintf("InstanceComponents(%d)", inctanceIdx++).UTF8().c_str(), FString::Sprintf("%s'\"%s\"'", component.Class.UTF8().c_str(), component.Name.UTF8().c_str()).UTF8().c_str());
        }
      }
    }
    f.AddString("ActorLabel", (untypedActor->GetPackage()->GetPackageName() + "_" + untypedActor->GetObjectName()).UTF8().c_str());
    f.AddString("FolderPath", untypedActor->GetPackage()->GetPackageName().UTF8().c_str());
    auto layers = untypedActor->GetLayers();
    layers.push_back("RE_All");
    for (int32 idx = 0; idx < layers.size(); ++idx)
    {
      f.AddString(FString::Sprintf("Layers(%d)", idx).UTF8().c_str(), W2A(layers[idx].WString()).c_str());
    }
  }
  f.End();
}

void ExportTerrainActor(T3DFile& f, LevelExportContext& ctx, UTerrain* actor)
{
  int32 width = 0;
  int32 height = 0;

  std::filesystem::path dst = ctx.GetTerrainDir();
  dst /= actor->GetPackage()->GetPackageName().UTF8() + "_" + actor->GetObjectName().UTF8() + "_HeightMap";
  dst.replace_extension(HasAVX2() ? "png" : "dds");

  std::error_code err;
  if (ctx.Config.OverrideData || !std::filesystem::exists(dst, err))
  {
    if (!std::filesystem::exists(ctx.GetTerrainDir(), err))
    {
      std::filesystem::create_directories(ctx.GetTerrainDir(), err);
    }
    uint16* heights = nullptr;
    actor->GetHeightMap(heights, width, height, ctx.Config.ResampleTerrain);
    if (heights)
    {
      TextureProcessor processor(TextureProcessor::TCFormat::G16, HasAVX2() ? TextureProcessor::TCFormat::PNG : TextureProcessor::TCFormat::DDS);
      processor.SetOutputPath(W2A(dst.wstring()));
      processor.SetInputData(heights, width * height * sizeof(uint16));
      processor.SetInputDataDimensions(width, height);
      free(heights);

      if (!processor.Process())
      {
        LogW("Failed to export %s heights: %s", actor->GetObjectPath().UTF8().c_str(), processor.GetError().c_str());
      }
    }
  }

  dst = ctx.GetTerrainDir();
  dst /= actor->GetPackage()->GetPackageName().UTF8() + "_" + actor->GetObjectName().UTF8() + "_VisibilityMap";
  dst.replace_extension(HasAVX2() ? "png" : "dds");

  if (actor->HasVisibilityData() && (ctx.Config.OverrideData || !std::filesystem::exists(dst, err)))
  {
    if (!std::filesystem::exists(ctx.GetTerrainDir(), err))
    {
      std::filesystem::create_directories(ctx.GetTerrainDir(), err);
    }
    uint8* visibility = nullptr;
    width = 0;
    height = 0;
    actor->GetVisibilityMap(visibility, width, height, ctx.Config.ResampleTerrain);
    if (visibility)
    {
      TextureProcessor processor(TextureProcessor::TCFormat::G8, HasAVX2() ? TextureProcessor::TCFormat::PNG : TextureProcessor::TCFormat::DDS);
      processor.SetOutputPath(W2A(dst.wstring()));
      processor.SetInputData(visibility, width * height);
      processor.SetInputDataDimensions(width, height);
      free(visibility);

      if (!processor.Process())
      {
        LogW("Failed to export %s visibility: %s", actor->GetObjectPath().UTF8().c_str(), processor.GetError().c_str());
      }
    }
  }

  dst = ctx.GetTerrainDir();
  dst /= "WeightMaps";
  dst /= actor->GetPackage()->GetPackageName().UTF8();
  dst /= actor->GetObjectName().UTF8();

  if (!std::filesystem::exists(dst, err))
  {
    std::filesystem::create_directories(dst, err);
  }

  if (ctx.Config.SplitTerrainWeights)
  {
    for (const FTerrainLayer& layer : actor->Layers)
    {
      std::filesystem::path texPath = dst / layer.Name.UTF8();
      texPath.replace_extension(HasAVX2() ? "png" : "dds");
      if (ctx.Config.OverrideData || !std::filesystem::exists(texPath))
      {
        void* data = nullptr;
        int32 width = 0;
        int32 height = 0;
        if (!actor->GetWeightMapChannel(layer.AlphaMapIndex, data, width, height))
        {
          continue;
        }
        TextureProcessor processor(TextureProcessor::TCFormat::G8, HasAVX2() ? TextureProcessor::TCFormat::PNG : TextureProcessor::TCFormat::DDS);
        processor.SetOutputPath(W2A(texPath.wstring()));
        processor.SetInputData(data, width * height);
        processor.SetInputDataDimensions(width, height);
        free(data);

        if (!processor.Process())
        {
          LogW("Failed to export %s weightmap: %s", actor->GetObjectPath().UTF8().c_str(), processor.GetError().c_str());
        }
      }
    }
  }
  else
  {
    std::vector<UTerrainWeightMapTexture*> maps = actor->GetWeightMaps();
    for (UTerrainWeightMapTexture* map : maps)
    {
      if (!map)
      {
        continue;
      }

      std::filesystem::path texPath = dst / map->GetObjectName().UTF8();
      texPath.replace_extension(HasAVX2() ? "tga" : "dds");

      if (ctx.Config.OverrideData || !std::filesystem::exists(texPath))
      {
        map->Load();

        TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
        switch (map->Format)
        {
        case PF_A8R8G8B8:
          inputFormat = TextureProcessor::TCFormat::ARGB8;
          break;
        case PF_DXT1:
          inputFormat = TextureProcessor::TCFormat::DXT1;
          break;
        case PF_DXT3:
          inputFormat = TextureProcessor::TCFormat::DXT3;
          break;
        case PF_DXT5:
          inputFormat = TextureProcessor::TCFormat::DXT5;
          break;
        default:
          break;
        }

        if (inputFormat == TextureProcessor::TCFormat::None)
        {
          LogW("Failed to export %s weightmap %s: unsupported pixel format.", actor->GetObjectPath().UTF8().c_str(), map->GetObjectName().UTF8());
          continue;
        }

        FTexture2DMipMap* mip = nullptr;
        for (FTexture2DMipMap* mipmap : map->Mips)
        {
          if (!mipmap->Data)
          {
            continue;
          }
          mip = mipmap;
          break;
        }
        if (!mip)
        {
          LogW("Failed to export %s weightmap %s: mo mips.", actor->GetObjectPath().UTF8().c_str(), map->GetObjectName().UTF8());
          continue;
        }

        TextureProcessor processor(inputFormat, HasAVX2() ? TextureProcessor::TCFormat::TGA : TextureProcessor::TCFormat::DDS);
        processor.SetOutputPath(W2A(texPath.wstring()));
        processor.SetInputData(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
        processor.SetInputDataDimensions(mip->SizeX, mip->SizeY);

        if (!processor.Process())
        {
          LogW("Failed to export %s weightmap %s: %s", actor->GetObjectPath().UTF8().c_str(), map->GetObjectName().UTF8(), processor.GetError().c_str());
        }
      }
    }
  }

  dst = ctx.GetTerrainDir();
  dst /= actor->GetPackage()->GetPackageName().UTF8() + "_" + actor->GetObjectName().UTF8() + "_Setup";
  dst.replace_extension("txt");

  if (ctx.Config.OverrideData || !std::filesystem::exists(dst, err))
  {
    if (!std::filesystem::exists(ctx.GetTerrainDir(), err))
    {
      std::filesystem::create_directories(ctx.GetTerrainDir(), err);
    }
    std::ofstream s(dst);
    s << actor->GetObjectPath().UTF8() << "\n\n";
    FVector rot = actor->Rotation.Normalized().Euler();
    FVector scale = actor->DrawScale3D * actor->DrawScale;
    if (ctx.Config.ResampleTerrain)
    {
      scale.X *= actor->GetHeightMapRatioX();
      scale.Y *= actor->GetHeightMapRatioY();
    }
    s << "Location: (X=" << std::to_string(actor->Location.X) << ", Y=" << std::to_string(actor->Location.Y) << ", Z=" << std::to_string(actor->Location.Z) << ")\n";
    s << "Rotation: (X=" << std::to_string(rot.X) << ", Y=" << std::to_string(rot.Y) << ", Z=" << std::to_string(rot.Z) << ")\n";
    s << "Scale: (X=" << std::to_string(scale.X) << ", Y=" << std::to_string(scale.Y) << ", Z=" << std::to_string(scale.Z) << ")\n\n";
    s << "Layers:\n";

    const char* padding = "  ";
    for (int32 idx = 0; idx < actor->Layers.size(); ++idx)
    {
      const auto& layer = actor->Layers[idx];
      s << "Layer " << idx + 1 << ":\n";
      s << padding << "Name: " << layer.Name.UTF8() << '\n';
      s << padding << "Index: " << layer.AlphaMapIndex << '\n';
      s << padding << "Hidden:" << (layer.Hidden ? "True" : "False") << '\n';
      if (layer.Setup && layer.Setup->Materials.size())
      {
        const auto& layerMaterials = layer.Setup->Materials;
        s << padding << "Terrain Materials:" << '\n';
        for (int32 mIdx = 0; mIdx < layerMaterials.size(); ++mIdx)
        {
          UTerrainMaterial* tm = layerMaterials[mIdx].Material;
          s << padding << "TM " << mIdx << ":\n";
          s << padding << padding << "Material: ";
          if (!tm)
          {
            s << "NULL\n";
            continue;
          }
          if (!tm->Material)
          {
            s << "NULL\n";
          }
          else
          {
            s << GetLocalDir(tm->Material) + tm->Material->GetObjectName().UTF8() << '\n';
          }
          if (tm->DisplacementMap)
          {
            s << padding << padding << "Displacement Map: " << GetLocalDir(tm->DisplacementMap) + tm->DisplacementMap->GetObjectName().UTF8() << '\n';
            s << padding << padding << "Displacement Scale: " << std::to_string(tm->DisplacementScale) << '\n';
          }
          s << padding << padding << "Scale: " << std::to_string(tm->MappingScale ? 1.f / tm->MappingScale : 1.f) << '\n';
          if (tm->MappingRotation)
          {
            s << padding << padding << "Rotation: " << std::to_string(tm->MappingRotation) << '\n';
          }
          if (tm->MappingPanU)
          {
            s << padding << padding << "PanU: " << std::to_string(tm->MappingPanU) << '\n';
          }
          if (tm->MappingPanV)
          {
            s << padding << padding << "PanV: " << std::to_string(tm->MappingPanV) << '\n';
          }
        }
      }

    }
  }

  if (ctx.Config.Materials || ctx.Config.Textures)
  {
    for (const FTerrainLayer& layer : actor->Layers)
    {
      if (!layer.Setup)
      {
        continue;
      }

      for (const FTerrainFilteredMaterial& tmat : layer.Setup->Materials)
      {
        if (!tmat.Material || !tmat.Material->Material)
        {
          continue;
        }
        ctx.UsedMaterials.push_back(tmat.Material->Material);
      }
    }
  }
}

std::string GetLocalDir(UObject* obj, const char* sep)
{
  std::string path;
  FObjectExport* outer = obj->GetExportObject()->Outer;
  while (outer)
  {
    path = (outer->GetObjectName().UTF8() + sep + path);
    outer = outer->Outer;
  }
  if ((obj->GetExportFlags() & EF_ForcedExport) == 0)
  {
    path = obj->GetPackage()->GetPackageName().UTF8() + sep + path;
  }
  return path;
}