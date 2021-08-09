#include "LevelEditor.h"
#include "../App.h"
#include "../Windows/ProgressWindow.h"
#include "../Windows/REDialogs.h"

#include <Tera/Cast.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UObject.h>
#include <Tera/ULevelStreaming.h>
#include <Tera/UStaticMesh.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/USpeedTree.h>
#include <Tera/USoundNode.h>
#include <Tera/UMaterial.h>
#include <Tera/UMaterialExpression.h>
#include <Tera/UTexture.h>
#include <Tera/ULight.h>
#include <Tera/UTerrain.h>
#include <Tera/UPrefab.h>
#include <Tera/UModel.h>

#include <Utils/ALog.h>
#include <Utils/T3DUtils.h>
#include <Utils/FbxUtils.h>
#include <Utils/TextureProcessor.h>

#include <functional>

const char* VSEP = "\t";

#define TEST_MAT_EXPS_EXPORT 0

typedef std::function<void(T3DFile&, LevelExportContext&, UActorComponent*)> ComponentDataFunc;

void AddCommonPrimitiveComponentParameters(T3DFile& f, LevelExportContext& ctx, UPrimitiveComponent* component);
std::vector<UObject*> SaveMaterialMap(UMeshComponent* component, LevelExportContext& ctx, const std::vector<UObject*>& materials);

void ExportActor(T3DFile& f, LevelExportContext& ctx, UActor* untypedActor);
void ExportTerrainActor(T3DFile& f, LevelExportContext& ctx, UTerrain* actor);

std::string GetActorName(UObject* actor)
{
  if (!actor)
  {
    return {};
  }
  return actor->GetPackage()->GetPackageName().UTF8() + "_" + actor->GetObjectNameString().UTF8();
}

void ExportMaterialExpressions(UMaterial* material, FString& output)
{
  std::vector<FExpressionInput> matInputs;
  for (FPropertyTag* tag : material->MaterialInputs)
  {
    FExpressionInput& in = matInputs.emplace_back(tag);
    in.Title = tag->Name.String();
  }
  output = "BlendMode:";
  output += VSEP;
  FString lightModel;
  FString softMask;
  bool addOpacityMask = false;
  switch (material->GetBlendMode())
  {
  case EBlendMode::BLEND_Additive:
    output += "Additive";
    break;
  case EBlendMode::BLEND_Masked:
    output += "Masked";
    addOpacityMask = true;
    break;
  case EBlendMode::BLEND_Opaque:
    output += "Opaque";
    break;
  case EBlendMode::BLEND_Translucent:
    output += "Translucent";
    lightModel += "Volumetric_NonDirectional";
    break;
  case EBlendMode::BLEND_DitheredTranslucent:
    output += "DitheredTranslucent";
    break;
  case EBlendMode::BLEND_SoftMasked:
    output += "Masked";
    softMask = VSEP;
    softMask += "DitherOpacityMask";
    softMask += "\n";
    addOpacityMask = true;
    break;
  case EBlendMode::BLEND_Modulate:
    output += "Modulate";
    break;
  case EBlendMode::BLEND_AlphaComposite:
    output += "AlphaComposite";
    break;
  default:
    output += "Unknown";
    break;
  }
  output += "\n";
  if (addOpacityMask)
  {
    output += "OpacityMaskClipValue:";
    output += VSEP;
    output += std::to_string(material->GetOpacityMaskClipValue());
    output += "\n";
  }
  if (softMask.Size())
  {
    output += softMask;
  }
  output += "TwoSided:";
  output += VSEP;
  output += material->IsTwoSided() ? "True" : "False";
  output += "\n";
  output += "ShadingModel:";
  output += VSEP;
  switch (material->GetLightingModel())
  {
  case MLM_Unlit:
    output += "Unlit";
    break;
  default:
    output += "DefaultLit";
    break;
  }
  output += "\n";
  if (lightModel.Size())
  {
    output += "LightingMode:";
    output += VSEP;
    output += lightModel;
    output += "\n";
  }

  output += "Inputs:\n";

  std::map<PACKAGE_INDEX, FString> orphans;
  for (const FExpressionInput& matIn : matInputs)
  {
    if (!matIn.Expression)
    {
      continue;
    }
    FString title = matIn.Title.UTF8();
    bool skipInput = true;
    if (title == "DiffuseColor")
    {
      skipInput = false;
      title = "BaseColor";
    }
    else if (title == "Normal")
    {
      skipInput = false;
    }
    else if (title == "SpecularColor")
    {
      title = "Specular";
      skipInput = false;
    }
    else if (title == "EmissiveColor")
    {
      skipInput = false;
    }
    else if (title == "OpacityMask")
    {
      skipInput = false;
    }

    if (skipInput)
    {
      if (matIn.Mask && (matIn.MaskA || matIn.MaskR || matIn.MaskG || matIn.MaskB))
      {
        output += VSEP;
        output += "Mask(";
        if (matIn.MaskA)
        {
          output += "A";
        }
        if (matIn.MaskR)
        {
          output += "R";
        }
        if (matIn.MaskG)
        {
          output += "G";
        }
        if (matIn.MaskB)
        {
          output += "B";
        }
        output += ")\n";
      }
      
      orphans[matIn.Expression->GetExportObject()->ObjectIndex] = FString("Material Input: ") + title;
      continue;
    }
    output += VSEP;
    output += title + VSEP;
    output += std::to_string(matIn.Expression->GetExportObject()->ObjectIndex);
    if (matIn.Mask && (matIn.MaskA || matIn.MaskR || matIn.MaskG || matIn.MaskB))
    {
      output += VSEP;
      output += "Mask(";
      if (matIn.MaskA)
      {
        output += "A";
      }
      if (matIn.MaskR)
      {
        output += "R";
      }
      if (matIn.MaskG)
      {
        output += "G";
      }
      if (matIn.MaskB)
      {
        output += "B";
      }
      output += ")";
    }
    output += "\n";
  }

  std::vector<UMaterialExpression*> expressions = material->GetExpressions();
  std::vector<AMaterialExpression> rwExpressions;
  PACKAGE_INDEX newItemsIndex = 1000000;
  for (UMaterialExpression* exp : expressions)
  {
    exp->ExportExpression(rwExpressions.emplace_back());
  }
  newItemsIndex++;
  std::vector<AMaterialExpression> newExpressions;
  for (AMaterialExpression& exp : rwExpressions)
  {
    if (exp.Class == UMaterialExpressionDepthBiasedAlpha::StaticClassName())
    {
      exp.Description = exp.Class;
      exp.Class == "MaterialExpressionDepthFade";
      float biasScale = 1.;
      if (AExpressionInput* input = exp.GetParameter("BiasScale"))
      {
        biasScale = input->FloatValue;
      }
      if (AExpressionInput* input = exp.GetParameter("Bias"))
      {
        if (input->Type == "expression" && biasScale != 1.)
        {
          if (PACKAGE_INDEX idx = input->Expression)
          {
            auto it = std::find_if(rwExpressions.begin(), rwExpressions.end(), [&](AMaterialExpression& i) {
              return i.Index == idx;
            });
            if (it != rwExpressions.end())
            {
              if (it->Class == UMaterialExpressionConstant::StaticClassName())
              {
                if (AExpressionInput* r = it->GetParameter("R"))
                {
                  r->FloatValue *= biasScale;
                }
              }
              else
              {
                AMaterialExpression& mul = newExpressions.emplace_back();
                mul.Index = newItemsIndex++;
                mul.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("A", it->Index, false));
                mul.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("B", biasScale));
                input->Expression = mul.Index;
              }
            }
          }
        }
      }
    }
    else if (exp.Class == UMaterialExpressionDestColor::StaticClassName())
    {
      exp.Description = exp.Class;
      exp.Class = "MaterialExpressionSceneColor";
    }
    else if (exp.Class == UMaterialExpressionDestDepth::StaticClassName())
    {
      exp.Description = exp.Class;
      exp.Class = "MaterialExpressionPixelDepth";
    }
    else if (exp.Class == UMaterialExpressionLensFlareIntensity::StaticClassName() ||
             exp.Class == UMaterialExpressionLensFlareOcclusion::StaticClassName() ||
             exp.Class == UMaterialExpressionLensFlareRadialDistance::StaticClassName() ||
             exp.Class == UMaterialExpressionLensFlareRayDistance::StaticClassName() ||
             exp.Class == UMaterialExpressionLensFlareSourceDistance::StaticClassName())
    {
      exp.Skip = true;
    }
    else if (exp.Class == UMaterialExpressionMeshEmitterVertexColor::StaticClassName())
    {
      exp.Description = exp.Class;
      exp.Class == "MaterialExpressionParticleColor";
    }
    else if (exp.Class == UMaterialExpressionMeshSubUV::StaticClassName())
    {
      exp.Description = exp.Class;
      exp.Class == "MaterialExpressionParticleSubUV";
    }
    else if (exp.Class == UMaterialExpressionFlipBookSample::StaticClassName())
    {
      AMaterialExpression& time = newExpressions.emplace_back();
      time.Index = newItemsIndex++;
      time.Class = "MaterialExpressionTime";
      time.Position.X = exp.Position.X - 320;
      time.Position.Y = exp.Position.Y - 16;

      AMaterialExpression& texture = newExpressions.emplace_back();
      texture.Index = newItemsIndex++;
      texture.Class = "MaterialExpressionTextureObject";
      texture.Position.X = exp.Position.X - 144;
      texture.Position.Y = exp.Position.Y - 96;
      texture.Parameters.emplace_back(*exp.GetParameter("Texture"));

      AMaterialExpression& cols = newExpressions.emplace_back();
      cols.Index = newItemsIndex++;
      cols.Class = "MaterialExpressionConstant";
      cols.Position.X = exp.Position.X - 208;
      cols.Position.Y = exp.Position.Y + 160;

      AMaterialExpression& rows = newExpressions.emplace_back();
      rows.Index = newItemsIndex++;
      rows.Class = "MaterialExpressionConstant";
      rows.Position.X = exp.Position.X - 208;
      rows.Position.Y = exp.Position.Y + 96;

      AMaterialExpression& mulFramesCount = newExpressions.emplace_back();
      mulFramesCount.Index = newItemsIndex++;
      mulFramesCount.Class = "MaterialExpressionMultiply";
      mulFramesCount.Position.X = exp.Position.X - 304;
      mulFramesCount.Position.Y = exp.Position.Y + 128;
      mulFramesCount.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("A", cols.Index, false));
      mulFramesCount.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("B", rows.Index, false));

      AMaterialExpression& frameTime = newExpressions.emplace_back();
      frameTime.Index = newItemsIndex++;
      frameTime.Class = "MaterialExpressionConstant";
      frameTime.Position.X = exp.Position.X - 304;
      frameTime.Position.Y = exp.Position.Y + 64;

      AMaterialExpression& durMul = newExpressions.emplace_back();
      durMul.Index = newItemsIndex++;
      durMul.Class = "MaterialExpressionMultiply";
      durMul.Position.X = exp.Position.X - 208;
      durMul.Position.Y = exp.Position.Y;
      durMul.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("A", frameTime.Index, false));
      durMul.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("B", mulFramesCount.Index, false));

      AMaterialExpression& timeDiv = newExpressions.emplace_back();
      timeDiv.Index = newItemsIndex++;
      timeDiv.Class = "MaterialExpressionDivide";
      timeDiv.Position.X = exp.Position.X - 96;
      timeDiv.Position.Y = exp.Position.Y;
      durMul.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("A", time.Index, false));
      durMul.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("B", durMul.Index, false));

      // TODO: UMaterialExpressionFlipBookSample might have custom UVs. Need to pass those to the UMaterialExpressionMaterialFunctionCall
      exp.Class = "UMaterialExpressionMaterialFunctionCall";
      exp.Function = "/Engine/Functions/Engine_MaterialFunctions02/Texturing/FlipBook.FlipBook";
      exp.Parameters.clear();
      exp.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Phase", timeDiv.Index, false));
      exp.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Columns", cols.Index, false));
      exp.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Rows", rows.Index, false));
      exp.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Texture", texture.Index, false));
    }
    if (orphans.count(exp.Index))
    {
      exp.Description = orphans[exp.Index];
    }
    for (AExpressionInput& param : exp.Parameters)
    {
      if (param.Type == "object")
      {
        if (param.ObjectValue)
        {
          if (UTexture2D* tex = Cast<UTexture2D>(param.ObjectValue))
          {
            tex->Load();
            if (tex->CompressionSettingsProperty && tex->CompressionSettings == TC_NormalmapAlpha)
            {
              AMaterialExpression& tex = newExpressions.emplace_back();
              tex.Index = newItemsIndex++;
              tex.Class = UMaterialExpressionTextureSampleParameter2D::StaticClassName();
              tex.Position.X = exp.Position.X;
              tex.Position.Y = exp.Position.Y + 250;
              tex.Parameters = exp.Parameters;
              if (AExpressionInput* in = tex.GetParameter("ParameterName"))
              {
                in->StringValue += "_Alpha";
              }
              tex.Description = "NormalMap Alpha channel";
              tex.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("RE_NormalAlpha", true));

              for (AMaterialExpression& tmpExp : rwExpressions)
              {
                for (AExpressionInput& tmpIn : tmpExp.Parameters)
                {
                  if (tmpIn.MaskA && tmpIn.Expression == exp.Index)
                  {
                    tmpIn.Expression = tex.Index;
                    tmpIn.MaskA = tmpIn.MaskB = tmpIn.MaskG = false;
                    tmpIn.MaskR = true;
                  }
                }
              }
              for (AMaterialExpression& tmpExp : newExpressions)
              {
                for (AExpressionInput& tmpIn : tmpExp.Parameters)
                {
                  if (tmpIn.MaskA && tmpIn.Expression == exp.Index)
                  {
                    tmpIn.Expression = tex.Index;
                    tmpIn.MaskA = tmpIn.MaskB = tmpIn.MaskG = false;
                    tmpIn.MaskR = true;
                  }
                }
              }
            }
          }
        }
        else
        {
          output += "NULL";
        }
      }
    }
  }
  rwExpressions.insert(rwExpressions.end(), newExpressions.begin(), newExpressions.end());
  output += "Expressions:\n";
  for (const auto& exp : rwExpressions)
  {
    if (exp.Skip)
    {
      continue;
    }
    output += "Expression";
    output += VSEP;
    output += std::to_string(exp.Index) + VSEP;
    output += exp.Class.UTF8() + "\n";
    output += VSEP;
    output += "EditorX";
    output += VSEP;
    output += std::to_string((int32)exp.Position.X) + "\n";
    output += VSEP;
    output += "EditorY";
    output += VSEP;
    output += std::to_string((int32)exp.Position.Y) + "\n";
    if (exp.Description.Size())
    {
      output += VSEP;
      output += "Desc";
      output += VSEP;
      output += exp.Description + "\n";
    }
    if (exp.Parameters.size())
    {
      output += VSEP;
      output += "Parameters:\n";
    }
    if (exp.Function.Size())
    {
      output += VSEP;
      output += "function";
      output += VSEP;
      output += exp.Function.UTF8();
      output += "\n";
    }
    for (const auto& param : exp.Parameters)
    {
      if (param.Name.Empty() || (param.Type == "expression" && param.Expression == 0) || param.Name == "RE_NormalAlpha")
      {
        continue;
      }
      output += VSEP;
      output += param.Name.UTF8() + VSEP;
      output += param.Type.UTF8() + VSEP;
      if (param.Type == "expression")
      {
        output += std::to_string(param.Expression);
      }
      else if (param.Type == "float")
      {
        output += std::to_string(param.FloatValue);
      }
      else if (param.Type == "bool")
      {
        output += param.BoolValue ? "True" : "False";
      }
      else if (param.Type == "int")
      {
        output += std::to_string(param.IntValue);
      }
      else if (param.Type == "object")
      {
        if (param.ObjectValue)
        {
          output += "Game/S1Game/";
          output += param.ObjectValue->GetLocalDir(true, "/");
          if (exp.GetParameter("RE_NormalAlpha"))
          {
            output += "_Alpha";
          }
        }
        else
        {
          output += "NULL";
        }
      }
      else if (param.Type == "color")
      {
        output += std::to_string(param.ColorValue.R) + ",";
        output += std::to_string(param.ColorValue.G) + ",";
        output += std::to_string(param.ColorValue.B) + ",";
        output += std::to_string(param.ColorValue.A) + ",";
        if (output.Back() == ',')
        {
          output.Resize(output.Size() - 1);
        }
      }
      else if (param.Type == "string")
      {
        output += param.StringValue.UTF8();
      }
      if (param.Mask && (param.MaskA || param.MaskR || param.MaskG || param.MaskB))
      {
        output += VSEP;
        output += "Mask(";
        if (param.MaskA)
        {
          output += "A";
        }
        if (param.MaskR)
        {
          output += "R";
        }
        if (param.MaskG)
        {
          output += "G";
        }
        if (param.MaskB)
        {
          output += "B";
        }
        output += ")";
      }
      output += "\n";
    }
  }
}

ComponentDataFunc ExportStaticMeshComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UStaticMeshComponent* component = Cast<UStaticMeshComponent>(acomp);
  if (!component || !component->StaticMesh)
  {
    if (acomp)
    {
      ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(acomp->GetOuter()));
    }
    return;
  }
  auto materials = component->StaticMesh->GetMaterials(ctx.Config.ExportLods ? -1 : 0);
  if (materials.size())
  {
    std::string key = "Game/";
    key += ctx.DataDirName;
    key += "/" + component->StaticMesh->GetLocalDir(true, "/").UTF8();

    if (!ctx.MeshDefaultMaterials.count(key))
    {
      for (UObject* material : materials)
      {
        if (!material)
        {
          ctx.MeshDefaultMaterials[key].push_back("None");
          continue;
        }
        std::string materialPath = "Game/";
        materialPath += ctx.DataDirName;
        materialPath += "/" + material->GetLocalDir(true, "/").UTF8();
        ctx.MeshDefaultMaterials[key].push_back(materialPath);
      }
    }
    
  }
  materials = SaveMaterialMap(component, ctx, materials);
  for (int32 idx = 0; idx < materials.size(); ++idx)
  {
    FString item = "None";
    if (materials[idx])
    {
      item = FString::Sprintf("%s'\"/Game/%s/%s%s\"'", materials[idx]->GetClassNameString().UTF8().c_str(), ctx.DataDirName, materials[idx]->GetLocalDir(false, "/").UTF8().c_str(), materials[idx]->GetObjectNameString().UTF8().c_str());
    }
    f.AddCustom(FString::Sprintf("OverrideMaterials(%d)", idx).UTF8().c_str(), item.UTF8().c_str());
  }
  std::filesystem::path path = ctx.GetStaticMeshDir() / component->StaticMesh->GetLocalDir().UTF8();
  std::error_code err;
  if (!std::filesystem::exists(path, err))
  {
    std::filesystem::create_directories(path, err);
  }
  std::string fbxName = component->StaticMesh->GetObjectNameString().UTF8();
  if (std::filesystem::exists(path, err))
  {
    path /= fbxName;
    path.replace_extension("fbx");
    if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
    {
      FbxUtils utils;
      FbxExportContext fbxCtx;
      fbxCtx.Path = path.wstring();
      fbxCtx.ExportLods = ctx.Config.ExportLods;
      fbxCtx.ExportCollisions = ctx.Config.ConvexCollisions;
      fbxCtx.ExportLightMapUVs = ctx.Config.ExportLightmapUVs;
      if (ctx.Config.GlobalScale != 1.f)
      {
        fbxCtx.Scale3D = FVector(ctx.Config.GlobalScale);
        fbxCtx.ApplyRootTransform = true;
      }
      if (!utils.ExportStaticMesh(component->StaticMesh, fbxCtx))
      {
        ctx.Errors.emplace_back("Error: Failed to save static mesh " + component->StaticMesh->GetLocalDir(true).UTF8() + " of " + GetActorName(component->GetOuter()));
      }
    }
    f.AddStaticMesh((std::string(ctx.DataDirName) + "/" + component->StaticMesh->GetLocalDir(false, "/").UTF8() + fbxName).c_str());
  }
  else
  {
    ctx.Errors.emplace_back("Error: Failed to create a folder to save static mesh " + component->StaticMesh->GetLocalDir(true).UTF8() + " of " + GetActorName(component->GetOuter()));
  }
  AddCommonPrimitiveComponentParameters(f, ctx, component);

  if (ctx.Config.ConvexCollisions && !component->StaticMesh->GetBodySetup())
  {
    if (!component->StaticMesh->UseSimpleBoxCollision || !component->StaticMesh->UseSimpleLineCollision)
    {
      const std::string item = component->StaticMesh->GetLocalDir().UTF8() + fbxName;
      if (std::find(ctx.ComplexCollisions.begin(), ctx.ComplexCollisions.end(), item) == ctx.ComplexCollisions.end())
      {
        ctx.ComplexCollisions.push_back(item);
      }
    }
  }

  if (component->ReplacementPrimitive && ctx.Config.ExportMLods)
  {
    const std::string itemPath = component->GetPackage()->GetPackageName().UTF8() + '_' + component->GetOuter()->GetObjectNameString().UTF8();
    const std::string mlodPath = component->GetPackage()->GetPackageName().UTF8() + '_' + component->ReplacementPrimitive->GetOuter()->GetObjectNameString().UTF8();
    auto& vec = ctx.MLODs[mlodPath];
    if (vec.empty() || std::find(vec.begin(), vec.end(), itemPath) == vec.end())
    {
      vec.push_back(itemPath);
    }
  }
};

ComponentDataFunc ExportSkeletalMeshComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  USkeletalMeshComponent* component = Cast<USkeletalMeshComponent>(acomp);
  if (!component || !component->SkeletalMesh)
  {
    if (acomp)
    {
      ctx.Errors.emplace_back("Error: Failed to export  " + GetActorName(acomp->GetOuter()));
    }
    return;
  }
  auto materials = component->SkeletalMesh->GetMaterials();
  if (materials.size())
  {
    std::string key = "Game/";
    key += ctx.DataDirName;
    key += "/" + component->SkeletalMesh->GetLocalDir(true, "/").UTF8();

    if (!ctx.MeshDefaultMaterials.count(key))
    {
      for (UObject* material : materials)
      {
        if (!material)
        {
          ctx.MeshDefaultMaterials[key].push_back("None");
          continue;
        }
        std::string materialPath = "Game/";
        materialPath += ctx.DataDirName;
        materialPath += "/" + material->GetLocalDir(true, "/").UTF8();
        ctx.MeshDefaultMaterials[key].push_back(materialPath);
      }
    }
  }
  materials = SaveMaterialMap(component, ctx, materials);
  for (int32 idx = 0; idx < materials.size(); ++idx)
  {
    FString item = "None";
    if (materials[idx])
    {
      item = FString::Sprintf("%s'\"/Game/%s/%s%s\"'", materials[idx]->GetClassNameString().UTF8().c_str(), ctx.DataDirName, materials[idx]->GetLocalDir(false, "/").UTF8().c_str(), materials[idx]->GetObjectNameString().UTF8().c_str());
    }
    f.AddCustom(FString::Sprintf("OverrideMaterials(%d)", idx).UTF8().c_str(), item.UTF8().c_str());
  }
  std::filesystem::path path = ctx.GetSkeletalMeshDir() / component->SkeletalMesh->GetLocalDir().UTF8();
  std::error_code err;
  if (!std::filesystem::exists(path, err))
  {
    std::filesystem::create_directories(path, err);
  }
  if (std::filesystem::exists(path, err))
  {
    std::string fbxName = component->SkeletalMesh->GetObjectNameString().UTF8();
    path /= fbxName;
    path.replace_extension("fbx");
    if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
    {
      FbxUtils utils;
      FbxExportContext fbxCtx;
      fbxCtx.Path = path.wstring();
      fbxCtx.ExportLods = ctx.Config.ExportLods;
      fbxCtx.ExportCollisions = ctx.Config.ConvexCollisions;
      if (ctx.Config.GlobalScale != 1.f)
      {
        fbxCtx.Scale3D = FVector(ctx.Config.GlobalScale);
        fbxCtx.ApplyRootTransform = true;
      }
      if (!utils.ExportSkeletalMesh(component->SkeletalMesh, fbxCtx))
      {
        ctx.Errors.emplace_back("Error: Failed to save skeletal mesh " + component->SkeletalMesh->GetLocalDir(true).UTF8() + " of " + GetActorName(component->GetOuter()));
      }
    }
    f.AddSkeletalMesh((std::string(ctx.DataDirName) + "/" + component->SkeletalMesh->GetLocalDir(false, "/").UTF8() + fbxName).c_str());
  }
  else
  {
    ctx.Errors.emplace_back("Error: Failed to create a folder to save skeletal mesh " + component->SkeletalMesh->GetLocalDir(true).UTF8() + " of " + GetActorName(component->GetOuter()));
  }
  AddCommonPrimitiveComponentParameters(f, ctx, component);
};

ComponentDataFunc ExportSpeedTreeComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  USpeedTreeComponent* component = Cast<USpeedTreeComponent>(acomp);
  if (!component || !component->SpeedTree)
  {
    ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(acomp->GetOuter()));
    return;
  }
  bool needsDefaults = false;
  {
    std::string key = "Game/";
    key += ctx.DataDirName;
    key += "/" + component->SpeedTree->GetLocalDir(true, "/").UTF8();
    needsDefaults = !ctx.MeshDefaultMaterials.count(key);
  }
  auto AddDefault = [&](UObject* material, bool isLeaf = false) {
    std::string key = "Game/";
    key += ctx.DataDirName;
    key += "/" + component->SpeedTree->GetLocalDir(true, "/").UTF8();
    std::string materialPath = "Game/";
    materialPath += ctx.DataDirName;
    materialPath += "/" + material->GetLocalDir(true, "/").UTF8();
    if (isLeaf)
    {
      materialPath += "_leafs";
    }
    ctx.MeshDefaultMaterials[key].push_back(materialPath);
  };

  std::map<std::string, UObject*> usedMaterials;
  if (component->SpeedTree->BranchMaterial)
  {
    ctx.UsedMaterials.push_back(component->SpeedTree->BranchMaterial);
    usedMaterials["branches"] = component->SpeedTree->BranchMaterial;
    if (needsDefaults)
    {
      AddDefault(component->SpeedTree->BranchMaterial);
    }
  }
  if (component->SpeedTree->FrondMaterial)
  {
    ctx.UsedMaterials.push_back(component->SpeedTree->FrondMaterial);
    usedMaterials["fronds"] = component->SpeedTree->FrondMaterial;
    if (needsDefaults)
    {
      AddDefault(component->SpeedTree->FrondMaterial);
    }
  }
  if (component->SpeedTree->LeafMaterial)
  {
    ctx.UsedMaterials.push_back(component->SpeedTree->LeafMaterial);
    usedMaterials["leafs"] = component->SpeedTree->LeafMaterial;
    if (needsDefaults)
    {
      AddDefault(component->SpeedTree->LeafMaterial, true);
    }

    ctx.SptLeafMaterials.push_back(component->SpeedTree->LeafMaterial);
    if (UMaterialInterface* m = Cast<UMaterialInterface>(component->SpeedTree->LeafMaterial))
    {
      UMaterialInterface* parent = Cast<UMaterialInterface>(m->GetParent());
      while (parent)
      {
        ctx.SptLeafMaterials.push_back(parent);
        parent = Cast<UMaterialInterface>(parent->GetParent());
      }
    }
  }

  for (const auto& p : usedMaterials)
  {
    if (UMaterialInterface* m = Cast<UMaterialInterface>(p.second))
    {
      UMaterialInterface* parent = Cast<UMaterialInterface>(m->GetParent());
      while (parent)
      {
        ctx.UsedMaterials.push_back(parent);
        parent = Cast<UMaterialInterface>(parent->GetParent());
      }
    }
  }

  if (usedMaterials.size())
  {
    std::string key = "Game/";
    key += ctx.DataDirName;
    key += "/" + component->SpeedTree->GetLocalDir(true, "/").UTF8();

    if (!ctx.MeshDefaultMaterials.count(key))
    {
      for (const auto& p : usedMaterials)
      {
        if (!p.second)
        {
          ctx.MeshDefaultMaterials[key].push_back("None");
          continue;
        }
        std::string materialPath = "Game/";
        materialPath += ctx.DataDirName;
        materialPath += "/" + p.second->GetLocalDir(true, "/").UTF8();
        ctx.MeshDefaultMaterials[key].push_back(materialPath);
      }
    }
  }

  if (component->BranchMaterial)
  {
    ctx.UsedMaterials.push_back(component->BranchMaterial);
    usedMaterials["branches"] = component->BranchMaterial;
  }
  if (component->FrondMaterial)
  {
    ctx.UsedMaterials.push_back(component->FrondMaterial);
    usedMaterials["fronds"] = component->FrondMaterial;
  }
  if (component->LeafMaterial)
  {
    ctx.UsedMaterials.push_back(component->LeafMaterial);
    usedMaterials["leafs"] = component->LeafMaterial;

    ctx.SptLeafMaterials.push_back(component->LeafMaterial);
    if (UMaterialInterface* m = Cast<UMaterialInterface>(component->LeafMaterial))
    {
      UMaterialInterface* parent = Cast<UMaterialInterface>(m->GetParent());
      while (parent)
      {
        ctx.SptLeafMaterials.push_back(parent);
        parent = Cast<UMaterialInterface>(parent->GetParent());
      }
    }
  }

  for (const auto& p : usedMaterials)
  {
    if (UMaterialInterface* m = Cast<UMaterialInterface>(p.second))
    {
      UMaterialInterface* parent = Cast<UMaterialInterface>(m->GetParent());
      while (parent)
      {
        ctx.UsedMaterials.push_back(parent);
        parent = Cast<UMaterialInterface>(parent->GetParent());
      }
    }
  }

  if (usedMaterials.size())
  {
    std::error_code err;
    UActor* actor = Cast<UActor>(component->GetOuter());
    
    std::filesystem::path path = ctx.GetMaterialMapDir();
    if (!std::filesystem::exists(path, err))
    {
      std::filesystem::create_directories(path, err);
    }
    if (std::filesystem::exists(path, err))
    {
      path /= component->GetPackage()->GetPackageName().UTF8() + "_" + (actor ? (UObject*)actor : (UObject*)component)->GetObjectNameString().UTF8();
      path.replace_extension("txt");

      if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
      {
        std::ofstream s(path, std::ios::out);
        for (const auto& p : usedMaterials)
        {
          s << p.first << ": " << p.second->GetLocalDir(true).UTF8() << '\n';
        }
      }
    }
    for (const auto& p : usedMaterials)
    {
      std::string gameDir = std::string("Game/") + ctx.DataDirName + '/';
      if (p.first == "leafs")
      {
        ctx.SpeedTreeMaterialOverrides[GetActorName(actor)][p.first] = p.second ? (gameDir + p.second->GetLocalDir(true, "/").UTF8() + "_leafs") : "None";
        ctx.SpeedTreeMaterialOverrides[GetActorName(actor)]["leafsmesh"] = p.second ? (gameDir + p.second->GetLocalDir(true, "/").UTF8()) : "None";
      }
      else
      {
        ctx.SpeedTreeMaterialOverrides[GetActorName(actor)][p.first] = p.second ? (gameDir + p.second->GetLocalDir(true, "/").UTF8()) : "None";
      }
    }
  }
  std::filesystem::path path = ctx.GetSpeedTreeDir() / component->SpeedTree->GetLocalDir().UTF8();
  std::error_code err;
  if (!std::filesystem::exists(path, err))
  {
    std::filesystem::create_directories(path, err);
  }
  if (std::filesystem::exists(path, err))
  {
    path /= component->SpeedTree->GetObjectNameString().UTF8();
    path.replace_extension("spt");
    if (!std::filesystem::exists(path, err))
    {
      void* sptData = nullptr;
      FILE_OFFSET sptDataSize = 0;
      component->SpeedTree->GetSptData(&sptData, &sptDataSize, true);
      std::ofstream s(path, std::ios::out | std::ios::trunc | std::ios::binary);
      s.write((const char*)sptData, sptDataSize);
      free(sptData);
    }
    f.AddStaticMesh((std::string(ctx.DataDirName) + "/" + component->SpeedTree->GetLocalDir(true, "/").UTF8()).c_str());
  }
  else
  {
    ctx.Errors.emplace_back("Error: Failed to create a folder to save SpeedTree " + component->SpeedTree->GetLocalDir(true).UTF8() + " of " + GetActorName(component->GetOuter()));
  }
  AddCommonPrimitiveComponentParameters(f, ctx, component);
};
#pragma mark ExportPointLightComponentData
ComponentDataFunc ExportPointLightComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UPointLightComponent* component = Cast<UPointLightComponent>(acomp);
  if (!component)
  {
    if (acomp)
    {
      ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(acomp));
    }
    return;
  }
  f.AddFloat("LightFalloffExponent", component->FalloffExponent);
  f.AddFloat("Intensity", component->Brightness* ctx.Config.PointLightMul);
  f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
  f.AddFloat("AttenuationRadius", component->Radius * ctx.Config.GlobalScale);
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
#pragma mark ExportSpotLightComponentData
ComponentDataFunc ExportSpotLightComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  USpotLightComponent* component = Cast<USpotLightComponent>(acomp);
  if (!component)
  {
    if (acomp)
    {
      ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(acomp));
    }
    return;
  }
  f.AddFloat("LightFalloffExponent", component->FalloffExponent);
  f.AddFloat("Intensity", component->Brightness* ctx.Config.SpotLightMul);
  f.AddFloat("InnerConeAngle", component->InnerConeAngle);
  f.AddFloat("OuterConeAngle", component->OuterConeAngle);
  f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
  f.AddFloat("AttenuationRadius", component->Radius * ctx.Config.GlobalScale);
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
#pragma mark ExportDirectionalLightComponentData
ComponentDataFunc ExportDirectionalLightComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UDirectionalLightComponent* component = Cast<UDirectionalLightComponent>(acomp);
  if (!component)
  {
    if (acomp)
    {
      ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(acomp));
    }
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
#pragma mark ExportSkyLightComponentData
ComponentDataFunc ExportSkyLightComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  USkyLightComponent* component = Cast<USkyLightComponent>(acomp);
  if (!component)
  {
    if (acomp)
    {
      ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(acomp));
    }
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
#pragma mark ExportHeightFogComponentData
ComponentDataFunc ExportHeightFogComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UHeightFogComponent* component = Cast<UHeightFogComponent>(acomp);
  if (!component)
  {
    if (acomp)
    {
      ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(acomp));
    }
    return;
  }
  f.AddFloat("FogDensity", component->Density);
  f.AddFloat("StartDistance", component->StartDistance);
  f.AddFloat("FogCutoffDistance", component->ExtinctionDistance);
  f.AddLinearColor("FogInscatteringColor", component->LightColor);
};
#pragma mark ExportVolumeComponentData
ComponentDataFunc ExportVolumeComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UBrushComponent* component = Cast<UBrushComponent>(acomp);
  if (!component)
  {
    if (acomp)
    {
      ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(acomp));
    }
    return;
  }
  
  f.Begin("Object", nullptr, "BodySetup_0");
  FKAggregateGeom geom = component->GetAggregateGeometry();
  FString convexElements = "(ConvexElems=(";
  for (int32 convexIdx = 0; convexIdx < geom.ConvexElems.size(); ++convexIdx)
  {
    FString vertexData = "(VertexData=(";
    for (int32 idx = 0; idx < geom.ConvexElems[convexIdx].VertexData.size(); ++idx)
    {
      FVector vertex = geom.ConvexElems[convexIdx].VertexData[idx] * ctx.Config.GlobalScale;
      vertexData += FString::Sprintf("(X=%.6f,Y=%.6f,Z=%.6f)", vertex.X, vertex.Y, vertex.Z);
      if (idx + 1 < geom.ConvexElems[convexIdx].VertexData.size())
      {
        vertexData += ",";
      }
    }
    vertexData += ")";
    convexElements += vertexData;
    FString indexData = "IndexData=(";
    for (int32 idx = 0; idx < geom.ConvexElems[convexIdx].FaceTriData.size(); ++idx)
    {
      indexData += std::to_string(geom.ConvexElems[convexIdx].FaceTriData[idx]);
      if (idx + 1 < geom.ConvexElems[convexIdx].FaceTriData.size())
      {
        indexData += ",";
      }
    }
    indexData += ")";
    if (geom.ConvexElems[convexIdx].FaceTriData.size())
    {
      convexElements += ",";
      convexElements += indexData;
    }
    if (geom.ConvexElems[convexIdx].BoxData.IsValid)
    {
      FVector vec = geom.ConvexElems[convexIdx].BoxData.Min * ctx.Config.GlobalScale;
      FString boxData = ",ElemBox=(";
      boxData += FString::Sprintf("Min=(X=%.6f,Y=%.6f,Z=%.6f),", vec.X, vec.Y, vec.Z);
      vec = geom.ConvexElems[convexIdx].BoxData.Max * ctx.Config.GlobalScale;
      boxData += FString::Sprintf("Max=(X=%.6f,Y=%.6f,Z=%.6f),", vec.X, vec.Y, vec.Z);
      boxData += "IsValid=1)";
      convexElements += boxData;
    }
    convexElements += ")";
    if (convexIdx + 1 < geom.ConvexElems.size())
    {
      convexElements += ",";
    }
  }
  convexElements += "))";
  f.AddCustom("AggGeom", convexElements.C_str());
  f.AddBool("bGenerateMirroredCollision", false);
  f.AddCustom("CollisionTraceFlag", "CTF_UseSimpleAsComplex");
  f.End();
  if (component->Brush)
  {
    f.AddCustom("Brush", FString::Sprintf("Model'\"%s\"'", component->Brush->GetObjectNameString().C_str()).UTF8().c_str());
    f.AddCustom("BrushBodySetup", "BodySetup'\"BodySetup_0\"'");
  }
};
#pragma mark ExportAudioComponentData
ComponentDataFunc ExportAudioComponentData = [](T3DFile& f, LevelExportContext& ctx, UActorComponent* acomp) {
  UAmbientSound* sound = acomp->GetTypedOuter<UAmbientSound>();
  if (!sound)
  {
    return;
  }
  USoundCue* cue = sound->AudioComponent->SoundCue;
  if (!cue)
  {
    return;
  }

  std::vector<USoundNodeWave*> waves;
  cue->GetWaves(waves);
  ctx.Waves.insert(ctx.Waves.end(), waves.begin(), waves.end());

  std::string asset = "Game/" + std::string(ctx.DataDirName) + '/';
  FString cueData = asset + cue->ExportCueToText(!Cast<UAmbientSoundNonLoop>(sound));
  asset += cue->GetLocalDir(true, "/").UTF8();
  ctx.CuesMap[cue] = cueData.UTF8();
  f.AddCustom("Sound", FString::Sprintf("SoundCue'\"/%s\"'", asset.c_str()).UTF8().c_str());

  float radiusMin = 100.f;
  float radiusMax = 1000.f;
  bool spatialize = true;
  FString distanceModel = "Linear";
  f.AddBool("bOverrideAttenuation", true);
  if (UAmbientSoundSimple* asound = Cast<UAmbientSoundSimple>(sound))
  {
    if (asound->AmbientProperties)
    {
      asound->AmbientProperties->Load();
      f.AddFloat("VolumeModulationMin", asound->AmbientProperties->GetVolumeMin());
      f.AddFloat("VolumeModulationMax", asound->AmbientProperties->GetVolumeMax());
      f.AddFloat("PitchModulationMin", asound->AmbientProperties->GetPitchMin());
      f.AddFloat("PitchModulationMax", asound->AmbientProperties->GetPitchMax());
      radiusMin = asound->AmbientProperties->GetRadiusMin() * ctx.Config.GlobalScale;
      radiusMax = asound->AmbientProperties->GetRadiusMax() * ctx.Config.GlobalScale;
      spatialize = asound->AmbientProperties->bSpatialize;
      switch (asound->AmbientProperties->DistanceModel)
      {
      case ATTENUATION_Logarithmic:
        distanceModel = "Logarithmic";
        break;
      case ATTENUATION_Inverse:
        distanceModel = "Inverse";
        break;
      case ATTENUATION_LogReverse:
        distanceModel = "LogReverse";
        break;
      case ATTENUATION_NaturalSound:
        distanceModel = "NaturalSound";
        break;
      }
    }
  }
  else
  {
    LogE("Can't export unknown sound class: %s", sound->GetClassNameString().UTF8().c_str());
    DBreak();
  }
  f.AddCustom("AttenuationOverrides", FString::Sprintf("(bSpatialize=%s,DistanceAlgorithm=%s,AttenuationShapeExtents=(X=%.06f,Y=0.000000,Z=0.000000),FalloffDistance=%.06f)", spatialize ? "True" : "False", distanceModel.UTF8().c_str(), radiusMin, radiusMax).UTF8().c_str());
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
  // Class, Name
  std::vector<std::pair<FString, FString>> ChildComponents;

  // This flag is set inside of the Define() and indicates that the component's actor must set bHidden flag
  mutable bool NeedsToBeHiddenInGame = false;

  ComponentMobility Mobility = ComponentMobility::Static;
  bool IsInstance = false;

  ComponentDataFunc DataFunc;
  UActorComponent* ActorComponent = nullptr;
  T3DComponent* Parent = nullptr;

  T3DComponent() = default;

  T3DComponent(UActorComponent* comp)
    : Name(comp->GetObjectNameString())
    , Class(comp->GetClassNameString())
    , Location(comp->Translation)
    , Rotation(comp->Rotation)
    , Scale3D(comp->Scale3D)
    , Scale(comp->Scale)
    , ActorComponent(comp)
  {}

  T3DComponent(UActorComponent* comp, ComponentDataFunc& func)
    : Name(comp->GetObjectNameString())
    , Class(comp->GetClassNameString())
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
    for (const auto& p : ChildComponents)
    {
      f.Begin("Object", p.first.UTF8().c_str(), p.second.UTF8().c_str());
      f.End();
    }
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
      f.AddPosition(Location * ctx.Config.GlobalScale);
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

    if (ActorComponent)
    {
      if (UActor* actor = Cast<UActor>(ActorComponent->GetOuter()))
      {
        if (ctx.Config.ConvexCollisions && !actor->bCollideActors)
        {
          f.AddBool("bUseDefaultCollision", false);
          f.AddCustom("BodyInstance", "(CollisionEnabled=NoCollision,CollisionProfileName=\"NoCollision\")");
        }
        if (!ctx.Config.IgnoreHidden && actor->bHidden)
        {
          f.AddBool("bVisible", false);
        }
      }
    }

    if (UPrimitiveComponent* prim = Cast<UPrimitiveComponent>(ActorComponent))
    {
      if (prim->HiddenGame)
      {
        if (Parent)
        {
          f.AddBool("bHiddenInGame", true);
        }
        else
        {
          // Handle visibility in the actor's body
          NeedsToBeHiddenInGame = true;
        }
      }
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
  std::vector<FString> CustomLines;
  struct AdditionalForward {
    AdditionalForward() = default;
    AdditionalForward(const FString& name, const FString& cls, const FString& tp)
      : Name(name)
      , Class(cls)
      , Type(tp)
    {}

    FString Name;
    FString Class;
    FString Type;
  };
  std::vector<AdditionalForward> AdditionalForwards;
  std::vector<FString> AdditionalLayers;

  bool ConfigureStaticMeshActor(UStaticMeshActor* actor)
  {
    if (!actor || !actor->StaticMeshComponent || !actor->StaticMeshComponent->StaticMesh)
    {
      return false;
    }
    Name = actor->GetObjectNameString();
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
      component.Name = "StaticMeshComponent0";
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
    Name = actor->GetObjectNameString();
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
      component.Name = "StaticMeshComponent0";
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
    Name = actor->GetObjectNameString();
    T3DComponent& component = Components.emplace_back(actor->SpeedTreeComponent, ExportSpeedTreeComponentData);
    component.Class = "StaticMeshComponent";
    component.Mobility = T3DComponent::ComponentMobility::Movable;
    if (component.NeedsParent())
    {
      Class = "Actor";
      RootComponent = &*Components.emplace(Components.begin(), T3DComponent(actor));
      RootComponent->Mobility = T3DComponent::ComponentMobility::Movable;
      RootComponent->IsInstance = true;
      component.Parent = RootComponent;
      component.IsInstance = true;
    }
    else
    {
      component.Name = "StaticMeshComponent0";
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
    Name = actor->GetObjectNameString();
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
      component.Name = "SkeletalMeshComponent0";
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
    Name = actor->GetObjectNameString();
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

  bool ConfigureVolumeActor(UVolume* actor, const char* className, const LevelExportContext& ctx)
  {
    if (!actor || !actor->Brush || !actor->Brush->GetPolys())
    {
      return false;
    }
    AdditionalForwards.emplace_back(AdditionalForward(actor->Brush->GetObjectNameString(), "", "Brush"));
    AdditionalForwards.emplace_back(AdditionalForward("Polys_0", "Polys", "Object"));

    Name = actor->GetObjectNameString();
    T3DComponent& component = Components.emplace_back(actor->BrushComponent, ExportVolumeComponentData);
    component.ChildComponents.emplace_back(std::make_pair("BodySetup", "BodySetup_0"));
    component.Name = "BrushComponent0";
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
      Class = className;
      RootComponent = &component;
      Properties["BrushComponent"] = &component;
    }
    RootComponent->Mobility = T3DComponent::ComponentMobility::Stationary;
    component.Mobility = T3DComponent::ComponentMobility::Stationary;
    RootComponent->TakeActorTransform(actor);

    if (UModel* model = actor->Brush)
    {
      model->Load();
      CustomLines.emplace_back("BrushType=Brush_Add");
      CustomLines.emplace_back(FString::Sprintf("Begin Brush Name=%s", model->GetObjectNameString().UTF8().c_str()));
      CustomLines.emplace_back("   Begin PolyList");
      UPolys* polys = model->GetPolys();
      polys->Load();
      for (const FPoly& polygon : polys->Elements)
      {
        CustomLines.emplace_back("      Begin Polygon");

        FVector origin = polygon.Base * ctx.Config.GlobalScale;
        CustomLines.emplace_back(FString::Sprintf("         Origin   %+013.6f,%+013.6f,%+013.6f", origin.X, origin.Y, origin.Z).C_str());
        CustomLines.emplace_back(FString::Sprintf("         Normal   %+013.6f,%+013.6f,%+013.6f", polygon.Normal.X, polygon.Normal.Y, polygon.Normal.Z).C_str());
        CustomLines.emplace_back(FString::Sprintf("         TextureU %+013.6f,%+013.6f,%+013.6f", polygon.TextureU.X, polygon.TextureU.Y, polygon.TextureU.Z).C_str());
        CustomLines.emplace_back(FString::Sprintf("         TextureV %+013.6f,%+013.6f,%+013.6f", polygon.TextureV.X, polygon.TextureV.Y, polygon.TextureV.Z).C_str());
        for (const FVector polyVec : polygon.Vertices)
        {
          FVector vec = polyVec * ctx.Config.GlobalScale;
          CustomLines.emplace_back(FString::Sprintf("         Vertex   %+013.6f,%+013.6f,%+013.6f", vec.X, vec.Y, vec.Z).C_str());
        }
        CustomLines.emplace_back("      End Polygon");
      }
      CustomLines.emplace_back("   End PolyList");
      CustomLines.emplace_back("End Brush");
      CustomLines.emplace_back(FString::Sprintf("Brush=Model'\"%s\"'", model->GetObjectNameString().UTF8().c_str()));
    }
    return true;
  }

  bool ConfigureSpotLightActor(USpotLight* actor)
  {
    if (!actor || !actor->LightComponent)
    {
      return false;
    }
    Name = actor->GetObjectNameString();
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
    Name = actor->GetObjectNameString();
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
    Name = actor->GetObjectNameString();
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
    Name = actor->GetObjectNameString();
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
    Name = actor->GetObjectNameString();
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
    Name = actor->GetObjectNameString();
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
          component.Name = typed->GetObjectNameString();
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
          component.Name = typed->GetObjectNameString();
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
          component.Name = typed->GetObjectNameString();
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
          component.Name = typed->GetObjectNameString();
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
          component.Name = typed->GetObjectNameString();
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
          component.Name = typed->GetObjectNameString();
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
          component.Name = typed->GetObjectNameString();
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
          component.Name = typed->GetObjectNameString();
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
          component.Name = typed->GetObjectNameString();
          component.TakeActorTransform(typed);
          component.Parent = RootComponent;
        }
        Components.push_back(component);
      }
      else if (UEmitter* typed = Cast<UEmitter>(archertype))
      {
        T3DComponent& component = Components.emplace_back(typed);
        component.Name = typed->GetObjectNameString();
        component.Class = "SceneComponent";
        component.IsInstance = true;
        component.Parent = RootComponent;
      }
      else
      {
        LogW("Prefab archertype %s is not supported!", actor->GetClassNameString().UTF8().c_str());
        continue;
      }
    }

    return Components.size() > 1;
  }

  bool ConfigureSoundActor(UAmbientSound* actor)
  {
    if (!actor)
    {
      return false;
    }
    AdditionalLayers.emplace_back("RE_Sounds");
    Name = actor->GetObjectNameString();
    Class = "AmbientSound";
    T3DComponent& component = Components.emplace_back(actor->AudioComponent, ExportAudioComponentData);
    component.Name = "AudioComponent0";
    component.Class = "AudioComponent";
    RootComponent = &component;
    RootComponent->TakeActorTransform(actor);
    return true;
  }
};

void LevelEditor::PrepareToExportLevel(LevelExportContext& ctx)
{
  if (!Level)
  {
    return;
  }
  {
    std::filesystem::path rootDir = ctx.Config.RootDir.WString();
    std::error_code err;
    if (!std::filesystem::exists(rootDir, err))
    {
      std::filesystem::create_directories(rootDir, err);
    }
  }
  UObject* world = Level->GetOuter();
  auto worldInner = world->GetInner();
  std::vector<ULevel*> levels = { Level };
  int maxProgress = Level->GetActorsCount();
  ProgressWindow progress(this, "Please wait...");
  progress.SetActionText("Preparing...");
  progress.SetCurrentProgress(-1);
  std::thread([&] {
    PERF_START(LevelExport);
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
        else if (!streamedLevel->Level)
        {
          ctx.Errors.emplace_back("Error! Failed to load streamed level: " + streamedLevel->PackageName.UTF8() + ".gmp!");
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
    if (ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::Terrains) && ctx.TerrainInfo.size())
    {
      std::ofstream s(std::filesystem::path(ctx.Config.RootDir.WString()) / "Terrains.txt");
      for (const std::string& item : ctx.TerrainInfo)
      {
        s << item;
      }
    }
    if (ctx.ComplexCollisions.size())
    {
      std::ofstream s(std::filesystem::path(ctx.Config.RootDir.WString()) / "ComplexCollisions.txt");
      for (const std::string& item : ctx.ComplexCollisions)
      {
        s << item << '\n';
      }
    }
    if (ctx.MLODs.size())
    {
      std::ofstream s(std::filesystem::path(ctx.Config.RootDir.WString()) / "MLODs.txt");
      for (const auto& p : ctx.MLODs)
      {
        s << p.first << '\n';
        for (const auto& i : p.second)
        {
          s << "  " << i << '\n';
        }
      }
    }
    if (ctx.Waves.size())
    {
      SendEvent(&progress, UPDATE_PROGRESS_DESC, wxString("Saving waves..."));
      std::vector<USoundNodeWave*> waves;
      for (UObject* obj : ctx.Waves)
      {
        if (USoundNodeWave* wave = Cast<USoundNodeWave>(obj))
        {
          wave->Load();
          if (std::find(waves.begin(), waves.end(), wave) == waves.end())
          {
            waves.emplace_back(wave);
          }
        }
      }
      maxProgress = waves.size();
      SendEvent(&progress, UPDATE_MAX_PROGRESS, maxProgress);
      int32 curProgress = 0;
      for (USoundNodeWave* wave : waves)
      {
        SendEvent(&progress, UPDATE_PROGRESS, wxString("Exporting: ") + wave->GetObjectNameString().UTF8(), curProgress);
        if (wave->GetResourceSize())
        {
          auto expPath = ctx.GetWaveDir();
          expPath /= wave->GetLocalDir().UTF8();
          std::error_code err;
          std::filesystem::create_directories(expPath, err);
          expPath += wave->GetObjectNameString() + ".ogg";
          if (!std::filesystem::exists(expPath) || ctx.Config.OverrideData)
          {
            std::ofstream s(expPath, std::ios::binary);
            s.write((const char*)wave->GetResourceData(), wave->GetResourceSize());
          }
        }
      }
    }
#if EXPERIMENTAL_SOUND_LEVEL_EXPORT
    {
      FString cuesList;
      std::error_code ec;
      for (const auto& p : ctx.CuesMap)
      {
        std::filesystem::path dirp = ctx.GetCueDir() / p.first->GetLocalDir().UTF8();
        std::filesystem::create_directories(dirp, ec);
        dirp /= (p.first->GetObjectNameString() + ".cue").UTF8();
        cuesList += dirp.wstring();
        cuesList += "\n";
        std::ofstream ofs(dirp);
        ofs << p.second;
      }
      {
        std::ofstream ofs(ctx.GetCuesInfoPath());
        ofs << cuesList.UTF8();
      }
    }
#endif
    if (ctx.Config.Materials || ctx.Config.Textures)
    {
      if (!ExportMaterialsAndTexture(ctx, &progress))
      {
        // UPDATE_PROGRESS_FINISH was sent inside of the ExportMaterialsAndTexture
        return;
      }
    }

    PERF_END(LevelExport);
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();

  if (!progress.IsCanceled())
  {
    if (ctx.Errors.empty())
    {
      REDialog::Info("Successfully exported the level.");
    }
    else
    {
      {
        std::ofstream s(ctx.Config.RootDir.WString() + L"/Errors.txt");
        for (const std::string& err : ctx.Errors)
        {
          s << err << '\n';
        }
      }
      REDialog::Warning("Successfully exported the level but some errors occurred during the process!\nSee the Errors.txt file in the destination folder for more details.");
    }
  }
}

void LevelEditor::ExportLevel(T3DFile& f, ULevel* level, LevelExportContext& ctx, ProgressWindow* progress)
{
  SendEvent(progress, UPDATE_PROGRESS_DESC, wxString("Preparing: " + level->GetPackage()->GetPackageName().UTF8()));
  std::vector<UActor*> actors = level->GetActors();

  if (actors.empty())
  {
    ctx.Errors.emplace_back("Warning: " + level->GetPackage()->GetPackageName().UTF8() + " has no actors!");
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

  std::vector<UActor*> skip;
  if (!ctx.Config.ExportMLods)
  {
    for (UActor* actor : actors)
    {
      UStaticMeshActor* smActor = Cast<UStaticMeshActor>(actor);
      if (!smActor || !smActor->StaticMeshComponent || !smActor->StaticMeshComponent->ReplacementPrimitive)
      {
        continue;
      }
      UActor* mlodActor = Cast<UActor>(smActor->StaticMeshComponent->ReplacementPrimitive->GetOuter());
      if (!mlodActor || std::find(skip.begin(), skip.end(), mlodActor) != skip.end())
      {
        continue;
      }
      skip.push_back(mlodActor);
    }
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
        FString name = level->GetPackage()->GetPackageName() + "\\" + actor->GetObjectNameString();
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
    if (skip.size() && std::find(skip.begin(), skip.end(), actor) != skip.end())
    {
      continue;
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
  if (ctx.UsedMaterials.empty())
  {
    return true;
  }

  if (ctx.Config.Materials)
  {
    SendEvent(progress, UPDATE_PROGRESS_DESC, wxString("Saving materials..."));
    SendEvent(progress, UPDATE_PROGRESS, 0);
    // Use 3x multiplier because collecting textures takes long. 2/3 proportion makes it feel a bit faster
    SendEvent(progress, UPDATE_MAX_PROGRESS, ctx.UsedMaterials.size() * (ctx.Config.Textures ? 3 : 1)); 
  }
  else
  {
    SendEvent(progress, UPDATE_PROGRESS_DESC, wxString("Preparing textures..."));
    SendEvent(progress, UPDATE_PROGRESS, -1);
  }

  int32 curProgress = 0;
  
  if (ctx.Config.Materials)
  {
    if (ctx.MeshDefaultMaterials.size())
    {
      const std::filesystem::path root = ctx.GetMeshDefaultMaterialsPath();
      std::ofstream s(root);
      for (const auto& p : ctx.MeshDefaultMaterials)
      {
        s << p.first << '\n';
        for (const auto& i : p.second)
        {
          s << "  " << i << '\n';
        }
      }
    }
    if (ctx.SpeedTreeMaterialOverrides.size())
    {
      std::filesystem::path root = ctx.GetSpeedTreeMaterialOverridesPath();
      std::ofstream s(root);
      for (const auto& p : ctx.SpeedTreeMaterialOverrides)
      {
        s << p.first << '\n';
        for (const auto& p2 : p.second)
        {
          s << "  " << p2.first << VSEP << p2.second << '\n';
        }
      }
    }
    
    struct MaterialParameters {
      std::map<FString, FTextureParameter> TextureParameters;
      std::map<FString, FLinearColor> VectorParameters;
      std::map<FString, float> ScalarParameters;
      std::map<FString, bool> BoolParameters;
      bool DoubleSided = false;
    };
    std::map<std::string, MaterialParameters> masterMaterials;
    std::map<std::string, MaterialParameters> materialInstances;
    std::vector<std::string> elements;

    for (UObject* obj : ctx.UsedMaterials)
    {
      SendEvent(progress, UPDATE_PROGRESS, curProgress);
      if (progress->IsCanceled())
      {
        SendEvent(progress, UPDATE_PROGRESS_FINISH);
        return false;
      }
      curProgress++;
      if (UMaterialInstance* mi = Cast<UMaterialInstance>(obj))
      {
        if (UObject* mat = mi->GetParent())
        {
          std::string e = obj->GetClassNameString().UTF8() + ' ';
          e += "Game/";
          e += ctx.DataDirName;
          e += '/' + mat->GetLocalDir(true, "/").UTF8() + ' ';
          e += "Game/";
          e += ctx.DataDirName;
          e += '/' + mi->GetLocalDir(true, "/").UTF8() + '\n';
          if (std::find(elements.begin(), elements.end(), e) == elements.end())
          {
            elements.push_back(e);
            MaterialParameters params;
            params.TextureParameters = mi->GetTextureParameters();
            params.VectorParameters = mi->GetVectorParameters();
            params.ScalarParameters = mi->GetScalarParameters();
            params.BoolParameters = mi->GetStaticBoolParameters();
            materialInstances[e] = params;
          }
        }
      }
      else if (UMaterial* mat = Cast<UMaterial>(obj))
      {
        std::string e = "Material Game/";
        e += ctx.DataDirName;
        e += '/' + mat->GetLocalDir(true, "/").UTF8() + '\n';
        if (std::find(elements.begin(), elements.end(), e) == elements.end())
        {
          elements.push_back(e);
          MaterialParameters params;
          params.DoubleSided = mat->TwoSided;
          params.TextureParameters = mat->GetTextureParameters();
          params.VectorParameters = mat->GetVectorParameters();
          params.ScalarParameters = mat->GetScalarParameters();
          params.BoolParameters = mat->GetStaticBoolParameters();
          masterMaterials[e] = params;
#if TEST_MAT_EXPS_EXPORT
          FString out;
          ExportMaterialExpressions(mat, out);
          auto expPath = ctx.GetMaterialExpressionsPath();
          expPath /= mat->GetLocalDir().UTF8();
          std::error_code err;
          std::filesystem::create_directories(expPath, err);
          expPath += mat->GetObjectNameString();
          {
            std::ofstream s(expPath);
            s << out.WString();
          }
#endif
        }
      }
    }

    // Add separate leaf card materials
    for (UObject* obj : ctx.SptLeafMaterials)
    {
      if (UMaterial* mat = Cast<UMaterial>(obj))
      {
        std::string e = "Material Game/";
        e += ctx.DataDirName;
        e += '/' + mat->GetLocalDir(true, "/").UTF8() + "_leafs" + '\n';
        if (std::find(elements.begin(), elements.end(), e) == elements.end())
        {
          elements.push_back(e);
          MaterialParameters params;
          params.DoubleSided = mat->TwoSided;
          params.TextureParameters = mat->GetTextureParameters();
          params.VectorParameters = mat->GetVectorParameters();
          params.ScalarParameters = mat->GetScalarParameters();
          params.BoolParameters = mat->GetStaticBoolParameters();
          masterMaterials[e] = params;
        }
      }
      else if (UMaterialInstance* mi = Cast<UMaterialInstance>(obj))
      {
        if (UMaterialInterface* parent = Cast<UMaterialInterface>(mi->GetParent()))
        {
          std::string e = obj->GetClassNameString().UTF8() + ' ';
          e += "Game/";
          e += ctx.DataDirName;
          e += '/' + parent->GetLocalDir(true, "/").UTF8() + "_leafs" + ' ';
          e += "Game/";
          e += ctx.DataDirName;
          e += '/' + mi->GetLocalDir(true, "/").UTF8() + "_leafs" + '\n';
          if (std::find(elements.begin(), elements.end(), e) == elements.end())
          {
            elements.push_back(e);
            MaterialParameters params;
            params.TextureParameters = mi->GetTextureParameters();
            params.VectorParameters = mi->GetVectorParameters();
            params.ScalarParameters = mi->GetScalarParameters();
            params.BoolParameters = mi->GetStaticBoolParameters();
            materialInstances[e] = params;
          }
        }
      }
    }

    if (elements.size())
    {
      const std::filesystem::path root = ctx.GetMaterialsListPath();
      std::ofstream s(root);
      for (const std::string& e : elements)
      {
        s << e;
      }
    }

    if (masterMaterials.size() || materialInstances.size())
    {
      auto DumpMaterial = [&](auto& s, const auto& mat) {
        s << mat.first;
        if (mat.second.DoubleSided)
        {
          s << "  TwoSided\n";
        }
        for (auto const& p : mat.second.TextureParameters)
        {
          if (UTexture2D* tmp = Cast<UTexture2D>(p.second.Texture))
          {
            if (p.second.AlphaChannelUsed && tmp->CompressionSettings == TC_NormalmapAlpha)
            {
              s << "  TextureA";
            }
            else
            {
              s << "  Texture";
            }
          }
          else
          {
            s << "  Texture";
          }
          s << VSEP << p.first.UTF8() << VSEP;
          if (p.second.Texture)
          {
            s << "Game/" << ctx.DataDirName << '/' << p.second.Texture->GetLocalDir(true, "/").UTF8() << '\n';
          }
          else
          {
            s << "None\n";
          }
        }
        for (auto const& p : mat.second.ScalarParameters)
        {
          s << "  Scalar" << VSEP << p.first.UTF8() << VSEP << p.second << '\n';
        }
        for (auto const& p : mat.second.VectorParameters)
        {
          s << "  Vector" << VSEP << p.first.UTF8() << VSEP << p.second.R << VSEP << p.second.G << VSEP << p.second.B << VSEP << p.second.A << '\n';
        }
        for (auto const& p : mat.second.BoolParameters)
        {
          s << "  Bool" << VSEP << p.first.UTF8() << VSEP << (p.second ? "True" : "False") << '\n';
        }
      };
      const std::filesystem::path root = ctx.GetMaterialsListPath();
      std::ofstream s(root);
      for (auto const& mat : masterMaterials)
      {
        DumpMaterial(s, mat);
      }
      for (auto const& mat : materialInstances)
      {
        DumpMaterial(s, mat);
      }
    }
  }
  
  if (ctx.Config.Textures)
  {
    std::map<std::string, UTexture*> textures;
    for (UObject* obj : ctx.UsedMaterials)
    {
      if (ctx.Config.Materials)
      {
        SendEvent(progress, UPDATE_PROGRESS, curProgress);
        curProgress+=2;
      }
      if (progress->IsCanceled())
      {
        SendEvent(progress, UPDATE_PROGRESS_FINISH);
        return false;
      }
      if (!obj)
      {
        continue;
      }

      if (UMaterialInterface* mat = Cast<UMaterialInterface>(obj))
      {
        auto textureParams = mat->GetTextureParameters();
        if (UMaterial* parent = Cast<UMaterial>(mat->GetParent()))
        {
          auto parentTexParams = parent->GetTextureParameters();
          for (auto& p : parentTexParams)
          {
            if (p.second.Texture && !textureParams.count(p.first))
            {
              textureParams[p.first].Texture = p.second.Texture;
            }
          }
        }
        for (auto& p : textureParams)
        {
          if (!p.second.Texture)
          {
            continue;
          }
          std::string tpath = p.second.Texture->GetLocalDir(true).UTF8();
          if (!textures.count(tpath))
          {
            textures[tpath] = p.second.Texture;
          }
        }
        auto constTextures = mat->GetTextureSamples();
        for (UTexture* tex : constTextures)
        {
          if (!tex)
          {
            continue;
          }
          std::string tpath = tex->GetLocalDir(true).UTF8();
          if (!textures.count(tpath))
          {
            textures[tpath] = tex;
          }
        }
      }
    }

    if (textures.size())
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

      std::vector<std::string> textureInfo;

      auto SaveTextureInfo = [&](UTexture2D* texture, const std::string& ext, bool alpha = false)
      {
        std::string result = TextureCompressionSettingsToString(alpha ? TC_Grayscale : texture->CompressionSettings).UTF8() + VSEP;
        result += (texture->SRGB && !alpha ? "True" : "False");
        result += VSEP;
        result += (texture->Format == PF_DXT1 || texture->Format == PF_DXT3 || texture->Format == PF_DXT5) ? "True" : "False";
        result += VSEP;
        result += "Game/";
        result += ctx.DataDirName;
        result += '/';
        result += texture->GetLocalDir(true, "/").UTF8();
        if (alpha)
        {
          result += "_Alpha";
        }
        result += VSEP;
        result += W2A((ctx.GetTextureDir() / texture->GetLocalDir().UTF8() / texture->GetObjectNameString().UTF8()).replace_extension().wstring());
        if (alpha)
        {
          result += "_Alpha";
        }
        result += '.' + ext;
        return result;
      };

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

        SendEvent(progress, UPDATE_PROGRESS_DESC, wxString("Exporting textures: ") + p.second->GetObjectNameString().UTF8());

        std::error_code err;
        std::filesystem::path path = ctx.GetTextureDir() / p.second->GetLocalDir().UTF8();
        if (!std::filesystem::exists(path, err))
        {
          std::filesystem::create_directories(path, err);
        }
        path /= p.second->GetObjectNameString().UTF8();
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
            LogE("Failed to export texture %s. Invalid format!", texture->GetObjectNameString().UTF8().c_str());
            continue;
          }

          textureInfo.emplace_back(SaveTextureInfo(texture, ext));

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
            LogE("Failed to export texture %s. No mipmaps!", texture->GetObjectNameString().UTF8().c_str());
            continue;
          }

          TextureProcessor processor(inputFormat, outputFormat);

          processor.SetInputData(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
          processor.SetOutputPath(W2A(path.wstring()));
          processor.SetInputDataDimensions(mip->SizeX, mip->SizeY);
          if (texture->CompressionSettings == TC_NormalmapAlpha)
          {
            processor.SetSplitAlpha(true);
            textureInfo.emplace_back(SaveTextureInfo(texture, ext, true));
          }
          try
          {
            if (!processor.Process())
            {
              LogE("Failed to export %s: %s", texture->GetObjectNameString().UTF8().c_str(), processor.GetError().c_str());
              continue;
            }
          }
          catch (...)
          {
            LogE("Failed to export %s! Unknown texture processor error!", texture->GetObjectNameString().UTF8().c_str());
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
              LogE("Failed to export texture cube %s.%s. Invalid face format!", p.second->GetObjectNameString().UTF8().c_str(), face->GetObjectNameString().UTF8().c_str());
              ok = false;
              break;
            }
            if (inputFormat != TextureProcessor::TCFormat::None && inputFormat != f)
            {
              LogE("Failed to export texture cube %s.%s. Faces have different format!", p.second->GetObjectNameString().UTF8().c_str(), face->GetObjectNameString().UTF8().c_str());
              ok = false;
              break;
            }
            inputFormat = f;
          }

          if (!ok)
          {
            continue;
          }

          // UE4 accepts texture cubes only in a DDS container with A8R8G8B8 format and proper flags. Export cubes this way regardless of the user's output format.
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
              LogE("Failed to export texture cube face %s.%s. No mipmaps!", cube->GetObjectPath().UTF8().c_str(), faces[faceIdx]->GetObjectNameString().UTF8().c_str());
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
          LogW("Failed to export a texture object %s(%s). Class is not supported!", p.second->GetObjectPath().UTF8().c_str(), p.second->GetClassNameString().UTF8().c_str());
        }
      }

      if (textureInfo.size())
      {
        std::ofstream s(ctx.GetTextureInfoPath());
        for (const std::string& i : textureInfo)
        {
          s << i << '\n';
        }
      }
    }
  }
  
  return true;
}

void AddCommonPrimitiveComponentParameters(T3DFile& f, LevelExportContext& ctx, UPrimitiveComponent* component)
{
  if (component->MinDrawDistance)
  {
    f.AddFloat("MinDrawDistance", component->MinDrawDistance * ctx.Config.GlobalScale);
  }
  if (component->MaxDrawDistance)
  {
    f.AddFloat("LDMaxDrawDistance", component->MaxDrawDistance * ctx.Config.GlobalScale);
  }
  if (component->CachedMaxDrawDistance)
  {
    f.AddFloat("CachedMaxDrawDistance", component->CachedMaxDrawDistance * ctx.Config.GlobalScale);
  }
  f.AddBool("CastShadow", component->CastShadow);
  f.AddBool("bCastDynamicShadow", ctx.Config.ForceDynamicShadows ? true : component->bCastDynamicShadow);
  f.AddBool("bCastStaticShadow", component->bCastStaticShadow);
  if (!component->bAcceptsLights)
  {
    f.AddCustom("LightingChannels", "(bChannel0=False)");
  }
}

std::vector<UObject*> SaveMaterialMap(UMeshComponent* component, LevelExportContext& ctx, const std::vector<UObject*>& materials)
{
  if (!component)
  {
    return {};
  }
  UActor* actor = Cast<UActor>(component->GetOuter());
  std::vector<UObject*> materialsToSave = materials;
  std::vector<UObject*> replacedMaterials;
  for (int matIdx = 0; matIdx < component->Materials.size(); ++matIdx)
  {
    if (component->Materials[matIdx] && matIdx < materialsToSave.size())
    {
      replacedMaterials.push_back(materialsToSave[matIdx]);
      materialsToSave[matIdx] = component->Materials[matIdx];
    }
  }

  for (UObject* object : replacedMaterials)
  {
    if (UMaterialInterface* m = Cast<UMaterialInterface>(object))
    {
      UMaterialInterface* parent = Cast<UMaterialInterface>(m->GetParent());
      while (parent)
      {
        ctx.UsedMaterials.push_back(parent);
        parent = Cast<UMaterialInterface>(parent->GetParent());
      }
    }
    ctx.UsedMaterials.push_back(object);
  }
  
  for (UObject* object : materialsToSave)
  {
    if (UMaterialInterface* m = Cast<UMaterialInterface>(object))
    {
      UMaterialInterface* parent = Cast<UMaterialInterface>(m->GetParent());
      while (parent)
      {
        ctx.UsedMaterials.push_back(parent);
        parent = Cast<UMaterialInterface>(parent->GetParent());
      }
    }
  }
  
  ctx.UsedMaterials.insert(ctx.UsedMaterials.end(), materialsToSave.begin(), materialsToSave.end());
  if (ctx.Config.Materials)
  {
    if (materialsToSave.size())
    {
      std::error_code err;
      std::filesystem::path path = ctx.GetMaterialMapDir();
      if (!std::filesystem::exists(path, err))
      {
        std::filesystem::create_directories(path, err);
      }
      if (std::filesystem::exists(path, err))
      {
        path /= component->GetPackage()->GetPackageName().UTF8() + "_" + (actor ? (UObject*)actor : (UObject*)component)->GetObjectNameString().UTF8();
        path.replace_extension("txt");

        if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
        {
          std::ofstream s(path, std::ios::out);
          for (int idx = 0; idx < materialsToSave.size(); ++idx)
          {
            s << std::to_string(idx + 1) << ". ";
            if (UObject* mat = materialsToSave[idx])
            {
              s << mat->GetLocalDir(true).UTF8();
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
  return materialsToSave;
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
    exportItem.RootComponent->Scale *= ctx.Config.GlobalScale;
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
  else if (UBlockingVolume* actor = Cast<UBlockingVolume>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::BlockVolumes))
    {
      return;
    }
    if (!exportItem.ConfigureVolumeActor(actor, "BlockingVolume", ctx))
    {
      return;
    }
  }
  else if (UAeroVolume* actor = Cast<UAeroVolume>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::AeroVolumes))
    {
      return;
    }
    if (!exportItem.ConfigureVolumeActor(actor, "TriggerVolume", ctx))
    {
      return;
    }
  }
  else if (UAeroInnerVolume* actor = Cast<UAeroInnerVolume>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::AeroVolumes))
    {
      return;
    }
    if (!exportItem.ConfigureVolumeActor(actor, "TriggerVolume", ctx))
    {
      return;
    }
  }
  else if (US1WaterVolume* actor = Cast<US1WaterVolume>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::WaterVolumes))
    {
      return;
    }
    if (!exportItem.ConfigureVolumeActor(actor, "TriggerVolume", ctx))
    {
      return;
    }
  }
  else if (US1MusicVolume* actor = Cast<US1MusicVolume>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::Sounds))
    {
      return;
    }
    if (!exportItem.ConfigureVolumeActor(actor, "TriggerVolume", ctx))
    {
      return;
    }
    exportItem.AdditionalLayers.emplace_back("RE_Sounds");
    std::vector<USoundNodeWave*> waves = actor->GetAllWaves();
    ctx.Waves.insert(ctx.Waves.end(), waves.begin(), waves.end());
#if EXPERIMENTAL_SOUND_LEVEL_EXPORT
    for (USoundCue* cue : actor->MusicList)
    {
      if (!cue)
      {
        continue;
      }
      std::string asset = "Game/";
      asset += ctx.DataDirName;
      asset += '/';
      FString cueData = cue->ExportCueToText();
      asset += cueData.UTF8();
      ctx.CuesMap[cue] = asset;
    }
#endif
  }
  else if (UAmbientSound* actor = Cast<UAmbientSound>(untypedActor))
  {
    if (!ctx.Config.GetClassEnabled(FMapExportConfig::ActorClass::Sounds))
    {
      return;
    }
    if (!exportItem.ConfigureSoundActor(actor))
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

    for (const T3DActor::AdditionalForward& fwd : exportItem.AdditionalForwards)
    {
      FString start = "Begin";
      FString end = "End";
      if (fwd.Type.Size())
      {
        start += " ";
        start += fwd.Type;
        end += " ";
        end += fwd.Type;
      }
      else
      {
        start += " Object";
        end += " Object";
      }
      if (fwd.Class.Size())
      {
        start += " ";
        start += "Class=/Script/Engine.";
        start += fwd.Class;
      }
      if (fwd.Name.Size())
      {
        start += " ";
        start += "Name=\"";
        start += fwd.Name;
        start += "\"";
      }
      f.AddCustomLine(start.UTF8().c_str());
      f.AddCustomLine(end.UTF8().c_str());
    }

    bool needsHiddenInGameFlag = false;
    for (const T3DComponent& component : exportItem.Components)
    {
      component.Define(f, ctx);
      if (component.NeedsToBeHiddenInGame)
      {
        needsHiddenInGameFlag = true;
      }
    }
    if (needsHiddenInGameFlag)
    {
      f.AddBool("bHidden", true);
    }

    for (const FString& customLine : exportItem.CustomLines)
    {
      f.AddCustomLine(customLine.C_str());
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
    f.AddString("ActorLabel", (untypedActor->GetPackage()->GetPackageName() + "_" + untypedActor->GetObjectNameString()).UTF8().c_str());
    f.AddString("FolderPath", untypedActor->GetPackage()->GetPackageName().UTF8().c_str());
    auto layers = untypedActor->GetLayers();
    layers.push_back("RE_All");
    if (untypedActor->bHidden)
    {
      layers.push_back("RE_Hidden");
    }
    layers.insert(layers.end(), exportItem.AdditionalLayers.begin(), exportItem.AdditionalLayers.end());
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

  std::error_code err;
  std::filesystem::path dst = ctx.GetTerrainDir();
  dst /= actor->GetPackage()->GetPackageName().UTF8();
  dst /= actor->GetObjectNameString().UTF8();
  if (!std::filesystem::exists(dst, err))
  {
    std::filesystem::create_directories(dst, err);
  }
  dst /= "HeightMap";
  dst.replace_extension(HasAVX2() ? "png" : "dds");

  
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
        ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(actor) + " heightmap: " + processor.GetError());
        LogW("Failed to export %s heights: %s", actor->GetObjectPath().UTF8().c_str(), processor.GetError().c_str());
      }
    }
  }

  dst = ctx.GetTerrainDir();
  dst /= actor->GetPackage()->GetPackageName().UTF8();
  dst /= actor->GetObjectNameString().UTF8();
  if (!std::filesystem::exists(dst, err))
  {
    std::filesystem::create_directories(dst, err);
  }
  dst /= "VisibilityMap";
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
        ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(actor) + " visibility mask: " + processor.GetError());
        LogW("Failed to export %s visibility: %s", actor->GetObjectPath().UTF8().c_str(), processor.GetError().c_str());
      }
    }
  }

  dst = ctx.GetTerrainDir();
  dst /= actor->GetPackage()->GetPackageName().UTF8();
  dst /= actor->GetObjectNameString().UTF8();
  dst /= "WeightMaps";

  if (!std::filesystem::exists(dst, err))
  {
    std::filesystem::create_directories(dst, err);
  }

  if (ctx.Config.SplitTerrainWeights)
  {
    for (int32 idx = 0; idx < actor->Layers.size(); ++idx)
    {
      const FTerrainLayer& layer = actor->Layers[idx];
      int32 layerMapIndex = layer.AlphaMapIndex;
      if (actor->GetAlphaMapsCount())
      {
        if (layerMapIndex == INDEX_NONE)
        {
          continue;
        }
      }
      else
      {
        layerMapIndex = idx;
      }
      std::filesystem::path texPath = dst / (std::to_string(idx + 1) + '_' + layer.Name.UTF8());
      texPath.replace_extension(HasAVX2() ? "png" : "dds");
      if (ctx.Config.OverrideData || !std::filesystem::exists(texPath))
      {
        void* data = nullptr;
        int32 width = 0;
        int32 height = 0;
        if (!actor->GetWeightMapChannel(layerMapIndex, data, width, height))
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
          ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(actor) + " weightmap: " + processor.GetError());
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

      std::filesystem::path texPath = dst / map->GetObjectNameString().UTF8();
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
          ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(actor) + " weightmap " + map->GetObjectNameString().UTF8() + ". Unsupported pixel format.");
          LogW("Failed to export %s weightmap %s: unsupported pixel format.", actor->GetObjectPath().UTF8().c_str(), map->GetObjectNameString().UTF8());
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
          ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(actor) + " weightmap " + map->GetObjectNameString().UTF8() + ". No mipmaps.");
          LogW("Failed to export %s weightmap %s: mo mips.", actor->GetObjectPath().UTF8().c_str(), map->GetObjectNameString().UTF8());
          continue;
        }

        TextureProcessor processor(inputFormat, HasAVX2() ? TextureProcessor::TCFormat::TGA : TextureProcessor::TCFormat::DDS);
        processor.SetOutputPath(W2A(texPath.wstring()));
        processor.SetInputData(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
        processor.SetInputDataDimensions(mip->SizeX, mip->SizeY);

        if (!processor.Process())
        {
          ctx.Errors.emplace_back("Error: Failed to export " + GetActorName(actor) + " weightmap " + map->GetObjectNameString().UTF8() + ". " + processor.GetError());
          LogW("Failed to export %s weightmap %s: %s", actor->GetObjectPath().UTF8().c_str(), map->GetObjectNameString().UTF8(), processor.GetError().c_str());
        }
      }
    }
  }

  dst = ctx.GetTerrainDir();
  dst /= actor->GetPackage()->GetPackageName().UTF8();
  dst /= actor->GetObjectNameString().UTF8();

  if (!std::filesystem::exists(dst, err))
  {
    std::filesystem::create_directories(dst, err);
  }

  dst /= "Setup";
  dst.replace_extension("txt");

  if (ctx.Config.OverrideData || !std::filesystem::exists(dst, err))
  {
    if (!std::filesystem::exists(ctx.GetTerrainDir(), err))
    {
      std::filesystem::create_directories(ctx.GetTerrainDir(), err);
    }
    std::ofstream s(dst);
    s << actor->GetObjectPath().UTF8() << "\n\n";
    FVector loc = actor->Location * ctx.Config.GlobalScale;
    FVector rot = actor->Rotation.Normalized().Euler();
    FVector scale = actor->DrawScale3D * actor->DrawScale * ctx.Config.GlobalScale;
    if (ctx.Config.ResampleTerrain)
    {
      scale.X *= actor->GetHeightMapRatioX();
      scale.Y *= actor->GetHeightMapRatioY();
    }
    ctx.TerrainInfo.emplace_back(actor->GetPackage()->GetPackageName().UTF8() + '_' + actor->GetObjectNameString().UTF8() + '\n');
    ctx.TerrainInfo.emplace_back(FString::Sprintf("\tLocation: (X=%06f,Y=%06f,Z=%06f)\n", loc.X, loc.Y, loc.Z).UTF8().c_str());
    ctx.TerrainInfo.emplace_back(FString::Sprintf("\tScale: (X=%06f,Y=%06f,Z=%06f)\n", scale.X, scale.Y, scale.Z).UTF8().c_str());
    if (!rot.IsZero())
    {
      ctx.TerrainInfo.emplace_back(FString::Sprintf("\tRotation: (Pitch=%06f,Yaw=%06f,Roll=%06f)\n", rot.Y, rot.Z, rot.X).UTF8().c_str());
    }
    s << "Location: (X=" << std::to_string(loc.X) << ",Y=" << std::to_string(loc.Y) << ",Z=" << std::to_string(loc.Z) << ")\n";
    if (!rot.IsZero())
    {
      s << "Rotation: (Pitch=" << std::to_string(rot.Y) << ",Yaw=" << std::to_string(rot.Z) << ",Roll=" << std::to_string(rot.X) << ")\n";
    }
    s << "Scale: (X=" << std::to_string(scale.X) << ",Y=" << std::to_string(scale.Y) << ",Z=" << std::to_string(scale.Z) << ")\n\n";
    s << "Layers(" << std::to_string(actor->Layers.size()) << "):\n";

    const char* padding = "  ";
    for (int32 idx = 0; idx < actor->Layers.size(); ++idx)
    {
      const auto& layer = actor->Layers[idx];
      s << "Layer " << idx + 1 << ":\n";
      s << padding << "Name: " << layer.Name.UTF8() << '\n';
      s << padding << "Hidden:" << (layer.Hidden ? "True" : "False") << '\n';
      if (layer.Setup && layer.Setup->Materials.size())
      {
        const auto& layerMaterials = layer.Setup->Materials;
        s << padding << "Terrain Materials:" << '\n';
        for (int32 mIdx = 0; mIdx < layerMaterials.size(); ++mIdx)
        {
          UTerrainMaterial* tm = layerMaterials[mIdx].Material;
          
          if (!tm || !tm->Material)
          {
            s << padding << padding << "None\n";
            continue;
          }

          s << padding << padding << tm->Material->GetLocalDir().UTF8() + tm->Material->GetObjectNameString().UTF8() << ":\n";
          auto textures = tm->Material->GetTextureParameters();
          for (const auto& p : textures)
          {
            if (p.second.Texture)
            {
              s << padding << padding << padding << p.first.UTF8() << ": " << p.second.Texture->GetLocalDir().UTF8() << p.second.Texture->GetObjectNameString().UTF8() << '\n';
            }
          }
          auto vectors = tm->Material->GetVectorParameters();
          for (const auto& p : vectors)
          {
            s << padding << padding << padding << p.first.UTF8();
            s << FString::Sprintf(": (R=%06f,G=%06f,B=%06f,A=%06f)\n", p.second.R, p.second.G, p.second.B, p.second.A).UTF8();
          }
          auto scalars = tm->Material->GetScalarParameters();
          for (const auto& p : scalars)
          {
            s << padding << padding << padding << p.first.UTF8() << ": " << p.second << '\n';
          }
          if (tm->DisplacementMap)
          {
            s << padding << padding << padding << "Displacement Map: " << tm->DisplacementMap->GetLocalDir().UTF8() + tm->DisplacementMap->GetObjectNameString().UTF8() << '\n';
            s << padding << padding << padding << "Displacement Scale: " << std::to_string(tm->DisplacementScale) << '\n';
          }
          s << padding << padding << padding << "Mapping Scale: " << std::to_string(tm->MappingScale ? 1.f / tm->MappingScale : 1.f) << '\n';
          if (tm->MappingRotation)
          {
            s << padding << padding << padding << "Rotation: " << std::to_string(tm->MappingRotation) << '\n';
          }
          if (tm->MappingPanU)
          {
            s << padding << padding << padding << "PanU: " << std::to_string(tm->MappingPanU) << '\n';
          }
          if (tm->MappingPanV)
          {
            s << padding << padding << padding << "PanV: " << std::to_string(tm->MappingPanV) << '\n';
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
        UMaterialInterface* parent = Cast<UMaterialInterface>(tmat.Material->Material->GetParent());
        while (parent)
        {
          ctx.UsedMaterials.push_back(parent);
          parent = Cast<UMaterialInterface>(parent->GetParent());
        }
        ctx.UsedMaterials.push_back(tmat.Material->Material);
      }
    }
  }
}