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

#include <Utils/ALog.h>
#include <Utils/T3DUtils.h>
#include <Utils/FbxUtils.h>
#include <Utils/TextureProcessor.h>

std::string GetLocalDir(UObject* obj, const char* sep = "\\")
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

void AddCommonActorParameters(T3DFile& f, UActor* actor);
void AddCommonActorComponentParameters(T3DFile& f, UActor* actor, UActorComponent* component, LevelExportContext& ctx);
void AddCommonPrimitiveComponentParameters(T3DFile& f, UPrimitiveComponent* component);
void AddCommonLightComponentParameters(T3DFile& f, ULightComponent* component);
void GetUniqueFbxName(UActor* actor, UActorComponent* comp, LevelExportContext& ctx, std::string& outObjectName);
void SaveMaterialMap(UActor* actor, UMeshComponent* component, LevelExportContext& ctx, const std::vector<UObject*>& materials);

void ExportStaticMeshActor(T3DFile& f, LevelExportContext& ctx, UStaticMeshActor* actor);
void ExportSkeletalMeshActor(T3DFile& f, LevelExportContext& ctx, USkeletalMeshActor* actor);
void ExportInterpActor(T3DFile& f, LevelExportContext& ctx, UInterpActor* actor);
void ExportSpeedTreeActor(T3DFile& f, LevelExportContext& ctx, USpeedTreeActor* actor);
void ExportPointLightActor(T3DFile& f, LevelExportContext& ctx, UPointLight* actor);
void ExportSpotLightActor(T3DFile& f, LevelExportContext& ctx, USpotLight* actor);
void ExportDirectionalLightActor(T3DFile& f, LevelExportContext& ctx, UDirectionalLight* actor);
void ExportSkyLightActor(T3DFile& f, LevelExportContext& ctx, USkyLight* actor);
void ExportHeightFogActor(T3DFile& f, LevelExportContext& ctx, UHeightFog* actor);
void ExportEmitterActor(T3DFile& f, LevelExportContext& ctx, UEmitter* actor);
void ExportTerrainActor(T3DFile& f, LevelExportContext& ctx, UTerrain* actor);

void LevelEditor::PrepareToExportLevel(LevelExportContext& ctx)
{
  UObject* world = Level->GetOuter();
  auto worldInner = world->GetInner();
  std::vector<ULevel*> levels = { Level };
  int maxProgress = Level->GetActorsCount();
  
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
  ProgressWindow progress(this, "Exporting");
  progress.SetMaxProgress(maxProgress);

  std::thread([&] {
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
  ctx.InterpActors.clear();
  SendEvent(progress, UPDATE_PROGRESS_DESC, wxString("Preparing: " + level->GetPackage()->GetPackageName().UTF8()));
  std::vector<UActor*> actors = level->GetActors();

  if (actors.empty())
  {
    return;
  }

  size_t initialSize = 0;
  if (ctx.Config.SplitT3D)
  {
    f.Clear();
    f.InitializeMap();
    initialSize = f.GetBody().size();
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
        FString name = level->GetPackage()->GetPackageName() + "/" + actor->GetObjectName();
        SendEvent(progress, UPDATE_PROGRESS_DESC, wxString(L"Exporting: " + name.WString()));
      }
    }
    if (!actor)
    {
      continue;
    }

    const FString className = actor->GetClassName();
    if (className == UStaticMeshActor::StaticClassName())
    {
      if (!ctx.Config.Statics)
      {
        continue;
      }
      ExportStaticMeshActor(f, ctx, Cast<UStaticMeshActor>(actor));
    }
    else if (className == USkeletalMeshActor::StaticClassName())
    {
      if (!ctx.Config.Skeletals)
      {
        continue;
      }
      ExportSkeletalMeshActor(f, ctx, Cast<USkeletalMeshActor>(actor));
    }
    else if (className == UInterpActor::StaticClassName())
    {
      if (!ctx.Config.Interps)
      {
        continue;
      }
      ExportInterpActor(f, ctx, Cast<UInterpActor>(actor));
    }
    else if (className == USpeedTreeActor::StaticClassName())
    {
      if (!ctx.Config.SpeedTrees)
      {
        continue;
      }
      ExportSpeedTreeActor(f, ctx, Cast<USpeedTreeActor>(actor));
    }
    else if (className == UPointLight::StaticClassName() || className == UPointLightMovable::StaticClassName() || className == UPointLightToggleable::StaticClassName())
    {
      if (!ctx.Config.PointLights)
      {
        continue;
      }
      ExportPointLightActor(f, ctx, Cast<UPointLight>(actor));
    }
    else if (className == USpotLight::StaticClassName() || className == USpotLightMovable::StaticClassName() || className == USpotLightToggleable::StaticClassName())
    {
      if (!ctx.Config.SpotLights)
      {
        continue;
      }
      ExportSpotLightActor(f, ctx, Cast<USpotLight>(actor));
    }
    else if (className == UDirectionalLight::StaticClassName() || className == UDirectionalLightToggleable::StaticClassName())
    {
      if (!ctx.Config.DirectionalLights)
      {
        continue;
      }
      ExportDirectionalLightActor(f, ctx, Cast<UDirectionalLight>(actor));
    }
    else if (className == USkyLight::StaticClassName() || className == USkyLightToggleable::StaticClassName())
    {
      if (!ctx.Config.SkyLights)
      {
        continue;
      }
      ExportSkyLightActor(f, ctx, Cast<USkyLight>(actor));
    }
    else if (className == UHeightFog::StaticClassName())
    {
      if (!ctx.Config.HeightFog)
      {
        continue;
      }
      ExportHeightFogActor(f, ctx, Cast<UHeightFog>(actor));
    }
    else if (className == UEmitter::StaticClassName())
    {
      if (!ctx.Config.Emitters)
      {
        continue;
      }
      ExportEmitterActor(f, ctx, Cast<UEmitter>(actor));
    }
    else if (className == UTerrain::StaticClassName())
    {
      if (!ctx.Config.Terrains)
      {
        continue;
      }
      ExportTerrainActor(f, ctx, Cast<UTerrain>(actor));
    }
  }

  std::filesystem::path dst = std::filesystem::path(ctx.Config.RootDir.WString()) / level->GetPackage()->GetPackageName().WString();
  if (ctx.Config.SplitT3D && initialSize != f.GetBody().size())
  {
    f.FinalizeMap();
    dst.replace_extension("t3d");
    f.Save(dst);
  }

  if (ctx.InterpActors.size())
  {
    dst = std::filesystem::path(ctx.Config.RootDir.WString()) / level->GetPackage()->GetPackageName().WString();
    dst += "_InterpActors.txt";
    std::ofstream s(dst);
    for (UInterpActor* actor : ctx.InterpActors)
    {
      s << actor->GetObjectPath().UTF8() << '\n';
    }
    ctx.InterpActors.clear();
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

void AddCommonActorParameters(T3DFile& f, UActor* actor)
{
  f.AddString("ActorLabel", actor->GetObjectName().UTF8().c_str());
  f.AddString("FolderPath", actor->GetPackage()->GetPackageName().UTF8().c_str());
  auto layers = actor->GetLayers();
  for (int32 idx = 0; idx < layers.size(); ++idx)
  {
    f.AddString(FString::Sprintf("Layers(%d)", idx).UTF8().c_str(), W2A(layers[idx].WString()).c_str());
  }
}

void AddCommonActorComponentParameters(T3DFile& f, UActor* actor, UActorComponent* component, LevelExportContext& ctx)
{
  FVector position = ctx.Config.BakeComponentTransform ? actor->Location : (actor->GetLocation() + component->Translation);
  FRotator rotation = ctx.Config.BakeComponentTransform ? actor->Rotation : (actor->Rotation + component->Rotation);
  FVector scale3D = ctx.Config.BakeComponentTransform ? actor->DrawScale3D : (actor->DrawScale3D * component->Scale3D);
  float scale = ctx.Config.BakeComponentTransform ? actor->DrawScale : (actor->DrawScale * component->Scale);
  f.AddPosition(position);
  f.AddRotation(rotation.Normalized());
  f.AddScale(scale3D, scale);
  if (actor->bHidden && !ctx.Config.IgnoreHidden)
  {
    f.AddBool("bVisible", false);
  }
  if (!actor->bCollideActors && ctx.Config.ConvexCollisions)
  {
    f.AddBool("bUseDefaultCollision", false);
    f.AddCustom("BodyInstance", "(CollisionEnabled=NoCollision,CollisionProfileName=\"NoCollision\")");
  }
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

void AddCommonLightComponentParameters(T3DFile& f, ULightComponent* component)
{
  f.AddGuid("LightGuid", component->LightGuid);
  f.AddColor("LightColor", component->LightColor);
  f.AddBool("CastShadows", component->CastShadows);
  f.AddBool("CastStaticShadows", component->CastStaticShadows);
  f.AddBool("CastDynamicShadows", component->CastDynamicShadows);
  if (!component->bEnabled)
  {
    f.AddBool("bAffectsWorld", false);
  }
}

void GetUniqueFbxName(UActor* actor, UActorComponent* comp, LevelExportContext& ctx, std::string& outObjectName)
{
  if (ctx.Config.BakeComponentTransform)
  {
    LevelExportContext::ComponentTransform t;
    t.PrePivot = actor->PrePivot;
    t.Translation = comp->Translation;
    t.Rotation = comp->Rotation;
    t.Scale3D = comp->Scale3D;
    t.Scale = comp->Scale;

    auto& transforms = ctx.FbxComponentTransformMap[outObjectName];
    for (size_t idx = 0; idx < transforms.size(); ++idx)
    {
      if (transforms[idx] == t)
      {
        // Found a name with the same offset. Use it.
        outObjectName += "_" + std::to_string(idx);
        return;
      }
    }
    // Add a new name and transform.
    outObjectName += "_" + std::to_string(transforms.size());
    transforms.push_back(t);
  }
}

void SaveMaterialMap(UActor* actor, UMeshComponent* component, LevelExportContext& ctx, const std::vector<UObject*>& materials)
{
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
      std::filesystem::path path = ctx.GetMaterialMapDir() / actor->GetPackage()->GetPackageName().UTF8();
      if (!std::filesystem::exists(path, err))
      {
        std::filesystem::create_directories(path, err);
      }
      if (std::filesystem::exists(path, err))
      {
        path /= actor->GetObjectName().UTF8();
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

void ExportStaticMeshActor(T3DFile& f, LevelExportContext& ctx, UStaticMeshActor* actor)
{
  if (!actor || !actor->StaticMeshComponent)
  {
    return;
  }
  actor->StaticMeshComponent->Load();
  if (!actor->StaticMeshComponent->StaticMesh)
  {
    return;
  }

  UStaticMeshComponent* component = actor->StaticMeshComponent;
  if (!component)
  {
    if (!(component = actor->StaticMeshComponent))
    {
      return;
    }
  }
  component->Load();

  f.Begin("Actor", "StaticMeshActor", FString::Sprintf("StaticMeshActor_%d", ctx.StaticMeshActorsCount++).UTF8().c_str());
  {
    f.Begin("Object", "StaticMeshComponent", "StaticMeshComponent0");
    f.End();
    f.Begin("Object", nullptr, "StaticMeshComponent0");
    {
      if (component->StaticMesh)
      {
        SaveMaterialMap(actor, component, ctx, component->StaticMesh->GetMaterials());

        std::filesystem::path path = ctx.GetStaticMeshDir() / GetLocalDir(component->StaticMesh);
        std::error_code err;
        if (!std::filesystem::exists(path, err))
        {
          std::filesystem::create_directories(path, err);
        }
        if (std::filesystem::exists(path, err))
        {
          std::string fbxName = component->StaticMesh->GetObjectName().UTF8();
          GetUniqueFbxName(actor, component, ctx, fbxName);
          path /= fbxName;
          path.replace_extension("fbx");
          if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
          {
            FbxUtils utils;
            FbxExportContext fbxCtx;
            fbxCtx.Path = path.wstring();
            fbxCtx.ApplyRootTransform = ctx.Config.BakeComponentTransform;
            fbxCtx.PrePivot = actor->PrePivot;
            fbxCtx.Translation = component->Translation;
            fbxCtx.Rotation = component->Rotation;
            fbxCtx.Scale3D = component->Scale3D * component->Scale;
            fbxCtx.ExportLods = ctx.Config.ExportLods;
            fbxCtx.ExportCollisions = ctx.Config.ConvexCollisions;
            utils.ExportStaticMesh(component->StaticMesh, fbxCtx);
          }
          f.AddStaticMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->StaticMesh, "/") + fbxName).c_str());
        }
      }
      AddCommonPrimitiveComponentParameters(f, component);
      AddCommonActorComponentParameters(f, actor, component, ctx);
    }
    f.End();
    f.AddString("StaticMeshComponent", "StaticMeshComponent0");
    f.AddString("RootComponent", "StaticMeshComponent0");
    AddCommonActorParameters(f, actor);
  }
  f.End();
}

void ExportSkeletalMeshActor(T3DFile& f, LevelExportContext& ctx, USkeletalMeshActor* actor)
{
  if (!actor || !actor->SkeletalMeshComponent)
  {
    return;
  }

  if (!actor->SkeletalMeshComponent->SkeletalMesh && !actor->SkeletalMeshComponent->ReplacementPrimitive)
  {
    return;
  }
  USkeletalMeshComponent* component = actor->SkeletalMeshComponent->ReplacementPrimitive ? Cast<USkeletalMeshComponent>(actor->SkeletalMeshComponent->ReplacementPrimitive) : actor->SkeletalMeshComponent;

  if (!component)
  {
    component = actor->SkeletalMeshComponent;
  }
  else
  {
    component->Load();
  }

  f.Begin("Actor", "SkeletalMeshActor", FString::Sprintf("SkeletalMeshActor_%d", ctx.SkeletalMeshActorsCount++).UTF8().c_str());
  {
    f.Begin("Object", "SkeletalMeshComponent", "SkeletalMeshComponent0");
    f.End();
    f.Begin("Object", nullptr, "SkeletalMeshComponent0");
    {
      if (component->SkeletalMesh)
      {
        std::vector<UObject*> materials = component->SkeletalMesh->GetMaterials();
        SaveMaterialMap(actor, component, ctx, materials);

        std::filesystem::path path = ctx.GetSkeletalMeshDir() / GetLocalDir(component->SkeletalMesh);
        std::error_code err;
        if (!std::filesystem::exists(path, err))
        {
          std::filesystem::create_directories(path, err);
        }
        if (std::filesystem::exists(path, err))
        {
          std::string fbxName = component->SkeletalMesh->GetObjectName().UTF8();
          GetUniqueFbxName(actor, component, ctx, fbxName);
          path /= fbxName;
          path.replace_extension("fbx");
          if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
          {
            FbxUtils utils;
            FbxExportContext fbxCtx;
            fbxCtx.Path = path.wstring();
            fbxCtx.ApplyRootTransform = ctx.Config.BakeComponentTransform;
            fbxCtx.PrePivot = actor->PrePivot;
            fbxCtx.Translation = component->Translation;
            fbxCtx.Rotation = component->Rotation;
            fbxCtx.Scale3D = component->Scale3D * component->Scale;
            fbxCtx.ExportLods = ctx.Config.ExportLods;
            utils.ExportSkeletalMesh(component->SkeletalMesh, fbxCtx);
          }
          f.AddSkeletalMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->SkeletalMesh, "/") + fbxName).c_str());
        }
      }
      f.AddCustom("ClothingSimulationFactory", "Class'\"/Script/ClothingSystemRuntimeNv.ClothingSimulationFactoryNv\"'");
      AddCommonPrimitiveComponentParameters(f, component);
      AddCommonActorComponentParameters(f, actor, component, ctx);
    }
    f.End();
    f.AddString("SkeletalMeshComponent", "SkeletalMeshComponent0");
    f.AddString("RootComponent", "SkeletalMeshComponent0");
    AddCommonActorParameters(f, actor);
  }
  f.End();
}

void ExportInterpActor(T3DFile& f, LevelExportContext& ctx, UInterpActor* actor)
{
  if (!actor || !actor->StaticMeshComponent)
  {
    return;
  }

  if (!actor->StaticMeshComponent->StaticMesh && !actor->StaticMeshComponent->ReplacementPrimitive)
  {
    return;
  }
  UStaticMeshComponent* component = actor->StaticMeshComponent->ReplacementPrimitive ? Cast<UStaticMeshComponent>(actor->StaticMeshComponent->ReplacementPrimitive) : actor->StaticMeshComponent;

  if (!component)
  {
    component = actor->StaticMeshComponent;
  }
  else
  {
    component->Load();
  }

  ctx.InterpActors.push_back(actor);

  f.Begin("Actor", "StaticMeshActor", FString::Sprintf("StaticMeshActor_%d", ctx.StaticMeshActorsCount++).UTF8().c_str());
  {
    f.Begin("Object", "StaticMeshComponent", "StaticMeshComponent0");
    f.End();
    f.Begin("Object", nullptr, "StaticMeshComponent0");
    {
      UStaticMesh* mesh = component->StaticMesh;
      bool replicated = false;
      if (actor->ReplicatedMesh)
      {
        mesh = actor->ReplicatedMesh;
        replicated = true;
      }
      if (mesh)
      {
        std::vector<UObject*> materials = mesh->GetMaterials();
        SaveMaterialMap(actor, component, ctx, materials);

        std::filesystem::path path = ctx.GetStaticMeshDir() / GetLocalDir(mesh);
        std::error_code err;
        if (!std::filesystem::exists(path, err))
        {
          std::filesystem::create_directories(path, err);
        }
        if (std::filesystem::exists(path, err))
        {
          std::string fbxName = component->StaticMesh->GetObjectName().UTF8();
          GetUniqueFbxName(actor, component, ctx, fbxName);
          path /= fbxName;
          path.replace_extension("fbx");
          if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
          {
            FbxUtils utils;
            FbxExportContext fbxCtx;
            fbxCtx.Path = path.wstring();
            fbxCtx.ApplyRootTransform = ctx.Config.BakeComponentTransform;
            fbxCtx.PrePivot = actor->PrePivot;
            fbxCtx.Translation = component->Translation;
            fbxCtx.Rotation = component->Rotation;
            fbxCtx.Scale3D = component->Scale3D * component->Scale;
            fbxCtx.ExportLods = ctx.Config.ExportLods;
            fbxCtx.ExportCollisions = ctx.Config.ConvexCollisions;
            utils.ExportStaticMesh(component->StaticMesh, fbxCtx);
          }
          f.AddStaticMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->StaticMesh, "/") + fbxName).c_str());
        }
      }
      AddCommonPrimitiveComponentParameters(f, component);
      if (replicated && !ctx.Config.BakeComponentTransform)
      {
        FVector vec = actor->Location;
        if (actor->ReplicatedMeshTranslationProperty)
        {
          vec = vec + actor->ReplicatedMeshTranslation;
        }
        else
        {
          vec = vec + component->Translation;
        }
        f.AddPosition(vec);

        vec = actor->DrawScale3D;
        if (actor->ReplicatedMeshScale3DProperty)
        {
          vec = vec * (actor->ReplicatedMeshScale3D / 100.);
        }
        else
        {
          vec = vec * actor->DrawScale3D;
        }
        f.AddScale(vec, actor->DrawScale * component->Scale);

        FRotator rot = actor->Rotation;
        if (actor->ReplicatedMeshRotationProperty)
        {
          rot = rot + actor->ReplicatedMeshRotation;
        }
        else
        {
          rot = rot + component->Rotation;
        }
        f.AddRotation(rot);
      }
      else
      {
        AddCommonActorComponentParameters(f, actor, component, ctx);
      }
      f.AddCustom("Mobility", "Movable");
    }
    f.End();
    f.AddString("StaticMeshComponent", "StaticMeshComponent0");
    f.AddString("RootComponent", "StaticMeshComponent0");
    AddCommonActorParameters(f, actor);
  }
  f.End();
}

void ExportSpeedTreeActor(T3DFile& f, LevelExportContext& ctx, USpeedTreeActor* actor)
{
  if (!actor || !actor->SpeedTreeComponent)
  {
    return;
  }

  if (!actor->SpeedTreeComponent->SpeedTree && !actor->SpeedTreeComponent->ReplacementPrimitive)
  {
    return;
  }
  USpeedTreeComponent* component = actor->SpeedTreeComponent->ReplacementPrimitive ? Cast<USpeedTreeComponent>(actor->SpeedTreeComponent->ReplacementPrimitive) : actor->SpeedTreeComponent;

  if (!component)
  {
    component = actor->SpeedTreeComponent;
  }
  else
  {
    component->Load();
  }

  f.Begin("Actor", "StaticMeshActor", FString::Sprintf("StaticMeshActor_%d", ctx.SpeedTreeActorsCount++).UTF8().c_str());
  {
    f.Begin("Object", "StaticMeshComponent", "StaticMeshComponent0");
    f.End();
    f.Begin("Object", nullptr, "StaticMeshComponent0");
    {
      if (component->SpeedTree)
      {
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
      }
      AddCommonPrimitiveComponentParameters(f, component);
      AddCommonActorComponentParameters(f, actor, component, ctx);
    }
    f.End();
    f.AddString("StaticMeshComponent", "StaticMeshComponent0");
    f.AddString("RootComponent", "StaticMeshComponent0");
    AddCommonActorParameters(f, actor);
  }
  f.End();
}

void ExportPointLightActor(T3DFile& f, LevelExportContext& ctx, UPointLight* actor)
{
  if (!actor || !actor->LightComponent)
  {
    return;
  }

  UPointLightComponent* component = Cast<UPointLightComponent>(actor->LightComponent);
  if (!component)
  {
    return;
  }

  f.Begin("Actor", "PointLight", FString::Sprintf("PointLight_%d", ctx.PointLightActorsCount++).UTF8().c_str());
  {
    f.Begin("Object", "PointLightComponent", "LightComponent0");
    f.End();
    f.Begin("Object", nullptr, "LightComponent0");
    {
      f.AddFloat("LightFalloffExponent", component->FalloffExponent);
      f.AddFloat("Intensity", component->Brightness * ctx.Config.PointLightMul);
      f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
      f.AddFloat("AttenuationRadius", component->Radius);
      f.AddCustom("LightmassSettings", wxString::Format("(ShadowExponent=%.06f)", component->ShadowFalloffExponent).c_str());
      AddCommonLightComponentParameters(f, component);
      AddCommonActorComponentParameters(f, actor, component, ctx);
      std::string mobility = "Static";
      const FString className = actor->GetClassName().String();
      if (className == UPointLightMovable::StaticClassName())
      {
        mobility = "Movable";
      }
      else if (className == UPointLightToggleable::StaticClassName())
      {
        mobility = "Stationary";
      }
      f.AddCustom("Mobility", mobility.c_str());
    }
    f.End();
    f.AddString("PointLightComponent", "LightComponent0");
    f.AddString("LightComponent", "LightComponent0");
    f.AddString("RootComponent", "LightComponent0");
    AddCommonActorParameters(f, actor);
  }
  f.End();
}

void ExportSpotLightActor(T3DFile& f, LevelExportContext& ctx, USpotLight* actor)
{
  if (!actor || !actor->LightComponent)
  {
    return;
  }

  USpotLightComponent* component = Cast<USpotLightComponent>(actor->LightComponent);
  if (!component)
  {
    return;
  }

  f.Begin("Actor", "SpotLight", FString::Sprintf("SpotLight_%d", ctx.SpotLightActorsCount++).UTF8().c_str());
  {
    f.Begin("Object", "SpotLightComponent", "LightComponent0");
    f.End();
    f.Begin("Object", "ArrowComponent", "ArrowComponent0");
    f.End();
    f.Begin("Object", nullptr, "LightComponent0");
    {
      f.AddFloat("LightFalloffExponent", component->FalloffExponent);
      f.AddFloat("Intensity", component->Brightness * ctx.Config.SpotLightMul);
      f.AddFloat("InnerConeAngle", component->InnerConeAngle);
      f.AddFloat("OuterConeAngle", component->OuterConeAngle);
      f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
      f.AddFloat("AttenuationRadius", component->Radius);
      f.AddCustom("LightmassSettings", wxString::Format("(ShadowExponent=%.06f)", component->ShadowFalloffExponent).c_str());
      AddCommonLightComponentParameters(f, component);
      f.AddPosition(actor->GetLocation() + component->Translation);
      FVector scale3D = actor->DrawScale3D;
      FRotator rotation = actor->Rotation;
      if (scale3D.X < 0 || scale3D.Y < 0 || scale3D.Z < 0)
      {
        if (scale3D.X < 0)
        {
          rotation.Roll += 0x8000;
          rotation.Yaw += 0x8000;
        }
        if (scale3D.Y < 0)
        {
          rotation.Pitch += 0x8000;
          rotation.Yaw += 0x8000;
        }
        if (scale3D.Z < 0)
        {
          rotation.Pitch += 0x8000;
          rotation.Roll += 0x8000;
        }
      }
      f.AddRotation(rotation + component->Rotation);
      std::string mobility = "Static";
      const FString className = actor->GetClassName();
      if (className == UPointLightMovable::StaticClassName())
      {
        mobility = "Movable";
      }
      else if (className == UPointLightToggleable::StaticClassName())
      {
        mobility = "Stationary";
      }
      f.AddCustom("Mobility", mobility.c_str());
    }
    f.End();
    f.AddString("SpotLightComponent", "LightComponent0");
    f.AddString("ArrowComponent", "ArrowComponent0");
    f.AddString("LightComponent", "LightComponent0");
    f.AddString("RootComponent", "LightComponent0");
    AddCommonActorParameters(f, actor);
  }
  f.End();
}

void ExportDirectionalLightActor(T3DFile& f, LevelExportContext& ctx, UDirectionalLight* actor)
{
  if (!actor || !actor->LightComponent)
  {
    return;
  }

  UDirectionalLightComponent* component = Cast<UDirectionalLightComponent>(actor->LightComponent);
  if (!component)
  {
    return;
  }

  f.Begin("Actor", "DirectionalLight", FString::Sprintf("DirectionalLight_%d", ctx.DirectionalLightActorsCount++).UTF8().c_str());
  {
    f.Begin("Object", "ArrowComponent", "ArrowComponent0");
    f.End();
    f.Begin("Object", "DirectionalLightComponent", "LightComponent0");
    f.End();
    f.Begin("Object", nullptr, "ArrowComponent0");
    {
      f.AddColor("ArrowColor", component->LightColor);
      f.AddString("AttachParent", "LightComponent0");
    }
    f.End();
    f.Begin("Object", nullptr, "LightComponent0");
    {
      f.AddFloat("Intensity", component->Brightness * ctx.Config.DirectionalLightMul);
      AddCommonLightComponentParameters(f, component);
      f.AddPosition(actor->GetLocation() + component->Translation);
      f.AddRotation(actor->Rotation + component->Rotation);
      f.AddCustom("Mobility", "Stationary");
    }
    f.End();
    f.AddString("ArrowComponent", "ArrowComponent0");
    f.AddString("DirectionalLightComponent", "LightComponent0");
    f.AddString("LightComponent", "LightComponent0");
    f.AddString("RootComponent", "LightComponent0");
    AddCommonActorParameters(f, actor);
  }
  f.End();
}

void ExportSkyLightActor(T3DFile& f, LevelExportContext& ctx, USkyLight* actor)
{
  if (!actor || !actor->LightComponent)
  {
    return;
  }

  USkyLightComponent* component = Cast<USkyLightComponent>(actor->LightComponent);
  if (!component)
  {
    return;
  }

  f.Begin("Actor", "SkyLight", FString::Sprintf("SkyLight_%d", ctx.SkyLightActorsCount++).UTF8().c_str());
  {
    f.Begin("Object", "SkyLightComponent", "SkyLightComponent0");
    f.End();
    f.Begin("Object", "BillboardComponent", "Sprite");
    f.End();
    f.Begin("Object", nullptr, "SkyLightComponent0");
    {
      if (component->LowerBrightness)
      {
        FLinearColor lowColor = component->LowerColor;
        f.AddLinearColor("LowerHemisphereColor", lowColor * component->LowerBrightness);
      }
      f.AddFloat("Intensity", component->Brightness * ctx.Config.SkyLightMul);
      AddCommonLightComponentParameters(f, component);
      f.AddPosition(actor->GetLocation() + component->Translation);
      f.AddCustom("Mobility", "Stationary");
    }
    f.End();
    f.Begin("Object", nullptr, "Sprite");
    {
      f.AddString("AttachParent", "SkyLightComponent0");
    }
    f.End();
    f.AddString("SpriteComponent", "Sprite");
    f.AddString("LightComponent", "SkyLightComponent0");
    f.AddString("RootComponent", "SkyLightComponent0");
    AddCommonActorParameters(f, actor);
  }
  f.End();
}

void ExportHeightFogActor(T3DFile& f, LevelExportContext& ctx, UHeightFog* actor)
{
  if (!actor)
  {
    return;
  }

  UHeightFogComponent* component = Cast<UHeightFogComponent>(actor->Component);
  if (!component)
  {
    return;
  }
  f.Begin("Actor", "ExponentialHeightFog", FString::Sprintf("ExponentialHeightFog_%d", ctx.HeightFogCount++).UTF8().c_str());
  {
    f.Begin("Object", "ExponentialHeightFogComponent", "HeightFogComponent0");
    f.End();
    f.Begin("Object", "BillboardComponent", "Sprite");
    f.End();
    f.Begin("Object", nullptr, "HeightFogComponent0");
    {
      f.AddFloat("FogDensity", component->Density);
      f.AddFloat("StartDistance", component->StartDistance);
      f.AddFloat("FogCutoffDistance", component->ExtinctionDistance);
      f.AddLinearColor("FogInscatteringColor", component->LightColor);
      f.AddPosition(actor->Location);
    }
    f.End();
    f.Begin("Object", nullptr, "Sprite");
    {
      f.AddString("AttachParent", "HeightFogComponent0");
    }
    f.End();
    f.AddCustom("Component", "HeightFogComponent0");
    f.AddCustom("SpriteComponent", "Sprite");
    f.AddCustom("RootComponent", "HeightFogComponent0");
    AddCommonActorParameters(f, actor);
  }
  f.End();
}

void ExportEmitterActor(T3DFile& f, LevelExportContext& ctx, UEmitter* actor)
{
  if (!actor)
  {
    return;
  }
  f.Begin("Actor", "Actor", FString::Sprintf("Actor_%d", ctx.UntypedActorsCount++).UTF8().c_str());
  {
    f.Begin("Object", "SceneComponent", "DefaultSceneRoot");
    f.End();
    f.Begin("Object", nullptr, "DefaultSceneRoot");
    {
      f.AddPosition(actor->Location);
      f.AddBool("bVisualizeComponent", true);
      f.AddCustom("CreationMethod", "Instance");
    }
    f.End();
    f.AddCustom("RootComponent", "SceneComponent'\"DefaultSceneRoot\"'");
    AddCommonActorParameters(f, actor);
    f.AddCustom("InstanceComponents(0)", "SceneComponent'\"DefaultSceneRoot\"'");
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