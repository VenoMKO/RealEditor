#include <fbxsdk.h>

#include "FbxUtils.h"

char* FbxWideToUtf8(const wchar_t* in)
{
  char* unistr = nullptr;
  size_t unistrSize = 0;
  FbxWCToUTF8(in, unistr, &unistrSize);
  return unistr;
}

FbxAMatrix GetGlobalDefaultPosition(FbxNode* node)
{
  FbxAMatrix localPosition;
  FbxAMatrix globalPosition;
  FbxAMatrix parentGlobalPosition;

  localPosition.SetT(node->LclTranslation.Get());
  localPosition.SetR(node->LclRotation.Get());
  localPosition.SetS(node->LclScaling.Get());

  if (node->GetParent())
  {
    parentGlobalPosition = GetGlobalDefaultPosition(node->GetParent());
    globalPosition = parentGlobalPosition * localPosition;
  }
  else
  {
    globalPosition = localPosition;
  }

  return globalPosition;
}

void SetGlobalDefaultPosition(FbxNode* node, FbxAMatrix globalPosition)
{
  FbxAMatrix localPosition;
  FbxAMatrix parentGlobalPosition;

  if (node->GetParent())
  {
    parentGlobalPosition = GetGlobalDefaultPosition(node->GetParent());
    localPosition = parentGlobalPosition.Inverse() * globalPosition;
  }
  else
  {
    localPosition = globalPosition;
  }

  node->LclTranslation.Set(localPosition.GetT());
  node->LclRotation.Set(localPosition.GetR());
  node->LclScaling.Set(localPosition.GetS());
}

FbxNode* CreateSkeleton(USkeletalMesh* sourceMesh, FbxDynamicArray<FbxNode*>& boneNodes, FbxScene* scene)
{
  if (!sourceMesh)
  {
    return nullptr;
  }
  std::vector<FMeshBone> refSkeleton = sourceMesh->GetReferenceSkeleton();
  for (int32 idx = 0; idx < refSkeleton.size(); ++idx)
  {
    const FMeshBone& bone = refSkeleton[idx];
    FbxSkeleton* skeletonAttribute = FbxSkeleton::Create(scene, bone.Name.String().C_str());
    skeletonAttribute->SetSkeletonType((!idx && refSkeleton.size() > 1) ? FbxSkeleton::eRoot : FbxSkeleton::eLimbNode);
    FbxNode* boneNode = FbxNode::Create(scene, bone.Name.String().C_str());
    boneNode->SetNodeAttribute(skeletonAttribute);

    FbxVector4 lT = FbxVector4(bone.BonePos.Position.X, bone.BonePos.Position.Y * -1., bone.BonePos.Position.Z);
    FbxQuaternion lQ = FbxQuaternion(bone.BonePos.Orientation.X, bone.BonePos.Orientation.Y * -1., bone.BonePos.Orientation.Z, bone.BonePos.Orientation.W * 1.);
    lQ[3] *= -1.;
    FbxAMatrix lGM;
    lGM.SetT(lT);
    lGM.SetQ(lQ);

    SetGlobalDefaultPosition(boneNode, lGM);

    if (idx)
    {
      boneNodes[bone.ParentIndex]->AddChild(boneNode);
    }
    boneNodes.PushBack(boneNode);
  }
  return boneNodes[0];
}

void AddNodeRecursively(FbxArray<FbxNode*>& nodeArray, FbxNode* node)
{
  if (node)
  {
    AddNodeRecursively(nodeArray, node->GetParent());
    if (nodeArray.Find(node) == -1)
    {
      nodeArray.Add(node);
    }
  }
}

void CreateBindPose(FbxNode* meshRootNode, FbxScene* scene)
{
  FbxArray<FbxNode*> clusteredFbxNodes;

  if (meshRootNode && meshRootNode->GetNodeAttribute())
  {
    int32 skinCount = 0;
    int32 clusterCount = 0;
    switch (meshRootNode->GetNodeAttribute()->GetAttributeType())
    {
    case FbxNodeAttribute::eMesh:
    case FbxNodeAttribute::eNurbs:
    case FbxNodeAttribute::ePatch:
      skinCount = ((FbxGeometry*)meshRootNode->GetNodeAttribute())->GetDeformerCount(FbxDeformer::eSkin);
      for (int32 i = 0; i < skinCount; ++i)
      {
        FbxSkin* skin = (FbxSkin*)((FbxGeometry*)meshRootNode->GetNodeAttribute())->GetDeformer(i, FbxDeformer::eSkin);
        clusterCount += skin->GetClusterCount();
      }
      break;
    default:
      break;
    }
    if (clusterCount)
    {
      for (int32 i = 0; i < skinCount; ++i)
      {
        FbxSkin* skin = (FbxSkin*)((FbxGeometry*)meshRootNode->GetNodeAttribute())->GetDeformer(i, FbxDeformer::eSkin);
        clusterCount = skin->GetClusterCount();
        for (int32 j = 0; j < clusterCount; ++j)
        {
          FbxNode* clusterNode = skin->GetCluster(j)->GetLink();
          AddNodeRecursively(clusteredFbxNodes, clusterNode);
        }

      }
      clusteredFbxNodes.Add(meshRootNode);
    }
  }

  if (clusteredFbxNodes.GetCount())
  {
    FbxPose* pose = FbxPose::Create(scene, meshRootNode->GetName());
    pose->SetIsBindPose(true);

    for (int32 i = 0; i < clusteredFbxNodes.GetCount(); i++)
    {
      FbxNode* node = clusteredFbxNodes.GetAt(i);
      FbxMatrix bindMatrix = node->EvaluateGlobalTransform();
      pose->Add(node, bindMatrix);
    }
    scene->AddPose(pose);
  }
}

#define GetManager() ((FbxManager*)SdkManager)
#define GetScene() ((FbxScene*)Scene)

FbxUtils::FbxUtils()
{
  SdkManager = FbxManager::Create();
  if (!SdkManager)
  {
    return;
  }

  FbxIOSettings* ios = FbxIOSettings::Create(GetManager(), IOSROOT);
  GetManager()->SetIOSettings(ios);

  Scene = FbxScene::Create(GetManager(), "");
  if (!Scene)
  {
    return;
  }

  FbxAxisSystem::EFrontVector FrontVector = (FbxAxisSystem::EFrontVector) - FbxAxisSystem::eParityOdd;
  const FbxAxisSystem UnrealZUp(FbxAxisSystem::eZAxis, FrontVector, FbxAxisSystem::eRightHanded);
  GetScene()->GetGlobalSettings().SetAxisSystem(UnrealZUp);
  GetScene()->GetGlobalSettings().SetOriginalUpAxis(UnrealZUp);
  GetScene()->GetGlobalSettings().SetSystemUnit(FbxSystemUnit::cm);
}

FbxUtils::~FbxUtils()
{
  if (SdkManager)
  {
    GetManager()->Destroy();
  }
}

bool FbxUtils::ExportSkeletalMesh(USkeletalMesh* sourceMesh, FbxExportContext& ctx)
{
  FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(GetManager(), "SceneInfo");
  char* meshName = FbxWideToUtf8(sourceMesh->GetObjectName().WString().c_str());
  sceneInfo->mTitle = meshName;
  FbxFree(meshName);
  sceneInfo->mAuthor = "RealEditor (yupimods.tumblr.com)";
  GetScene()->SetSceneInfo(sceneInfo);

  FbxNode* meshNode = nullptr;
  if (!ExportSkeletalMesh(sourceMesh, ctx, (void**)&meshNode) || !meshNode)
  {
    return false;
  }

  GetScene()->GetRootNode()->AddChild(meshNode);
  if (!SaveScene(ctx.Path, ctx.EmbedMedia))
  {
    ctx.Error = "Failed to write data!";
    return false;
  }
  return true;
}

bool FbxUtils::ExportStaticMesh(UStaticMesh* sourceMesh, FbxExportContext& ctx)
{
  FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(GetManager(), "SceneInfo");
  char* meshName = FbxWideToUtf8(sourceMesh->GetObjectName().WString().c_str());
  sceneInfo->mTitle = meshName;
  FbxFree(meshName);
  sceneInfo->mAuthor = "RealEditor (yupimods.tumblr.com)";
  GetScene()->SetSceneInfo(sceneInfo);

  FbxNode* meshNode = nullptr;
  if (!ExportStaticMesh(sourceMesh, ctx, (void**)&meshNode) || !meshNode)
  {
    return false;
  }

  GetScene()->GetRootNode()->AddChild(meshNode);
  if (!SaveScene(ctx.Path, ctx.EmbedMedia))
  {
    ctx.Error = "Failed to write data!";
    return false;
  }
  return true;
}

bool FbxUtils::SaveScene(const std::wstring& path, bool embedMedia)
{
  FbxExporter* exporter = FbxExporter::Create(GetManager(), "");
  char* uniPath = nullptr;
  size_t uniPathSize = 0;
  FbxWCToUTF8(path.c_str(), uniPath, &uniPathSize);

  if (exporter->Initialize(uniPath, 0, GetManager()->GetIOSettings()) == false)
  {
    return false;
  }

  int lMajor, lMinor, lRevision;
  FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
  bool result = exporter->Export(GetScene());
  exporter->Destroy();
  FbxFree(uniPath);
  return result;
}

bool FbxUtils::ExportSkeletalMesh(USkeletalMesh* sourceMesh, FbxExportContext& ctx, void** outNode)
{
  const FStaticLODModel* lod = sourceMesh->GetLod(ctx.LodIndex);
  if (!lod)
  {
    ctx.Error = "Failed to get the lod model!";
    return false;
  }

  const std::vector<FSoftSkinVertex> verticies = lod->GetVertices();
  if (verticies.empty())
  {
    ctx.Error = "The model has no vertices!";
    return false;
  }

  FbxMesh* mesh = FbxMesh::Create(GetScene(), "geometry");
  mesh->InitControlPoints(verticies.size());

  for (uint32 idx = 0; idx < verticies.size(); ++idx)
  {
    const FSoftSkinVertex& v = verticies[idx];
    mesh->GetControlPoints()[idx] = FbxVector4(v.Position.X, -v.Position.Y, v.Position.Z);
  }

  FbxLayer* layer = mesh->GetLayer(0);
  if (!layer)
  {
    layer = mesh->GetLayer(mesh->CreateLayer());
  }

  const int32 numTexCoords = lod->GetNumTexCoords();
  FbxLayerElementUV* uvDiffuseLayer = FbxLayerElementUV::Create(mesh, "DiffuseUV");
  std::vector<FbxLayerElementUV*> customUVLayers;
  for (int32 idx = 1; idx < numTexCoords; ++idx)
  {
    std::string layerName = "Custom_" + std::to_string(idx);
    customUVLayers.push_back(FbxLayerElementUV::Create(mesh, layerName.c_str()));
  }

  FbxLayerElementNormal* layerElementNormal = FbxLayerElementNormal::Create(mesh, "");
  FbxLayerElementBinormal* layerElementBinormal = FbxLayerElementBinormal::Create(mesh, "");
  FbxLayerElementTangent* layerElementTangent = FbxLayerElementTangent::Create(mesh, "");

  uvDiffuseLayer->SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
  uvDiffuseLayer->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);
  layerElementNormal->SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
  layerElementNormal->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);
  layerElementBinormal->SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
  layerElementBinormal->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);
  layerElementTangent->SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
  layerElementTangent->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);

  for (uint32 idx = 0; idx < verticies.size(); ++idx)
  {
    const FSoftSkinVertex& v = verticies[idx];
    FVector tmp;
    tmp = v.TangentX;
    layerElementTangent->GetDirectArray().Add(FbxVector4(tmp.X, -tmp.Y, tmp.Z));
    tmp = v.TangentY;
    layerElementBinormal->GetDirectArray().Add(FbxVector4(tmp.X, -tmp.Y, tmp.Z));
    tmp = v.TangentZ;
    layerElementNormal->GetDirectArray().Add(FbxVector4(tmp.X, -tmp.Y, tmp.Z));

    uvDiffuseLayer->GetDirectArray().Add(FbxVector2(v.UVs[0].X, -v.UVs[0].Y + 1.f));
    for (int32 uvIdx = 0; uvIdx < customUVLayers.size(); ++uvIdx)
    {
      customUVLayers[uvIdx]->GetDirectArray().Add(FbxVector2(v.UVs[uvIdx + 1].X, -v.UVs[uvIdx + 1].Y + 1.f));
    }
  }

  layer->SetNormals(layerElementNormal);
  layer->SetBinormals(layerElementBinormal);
  layer->SetTangents(layerElementTangent);

  {
    int32 etype = (int32)FbxLayerElement::EType::eTextureDiffuse;
    layer->SetUVs(uvDiffuseLayer, (FbxLayerElement::EType)etype);
    for (int32 uvIdx = 0; uvIdx < customUVLayers.size(); ++uvIdx)
    {
      etype++;
      layer->SetUVs(customUVLayers[uvIdx], (FbxLayerElement::EType)etype);
    }
  }

  FbxLayerElementMaterial* matLayer = FbxLayerElementMaterial::Create(mesh, "");
  matLayer->SetMappingMode(FbxLayerElement::EMappingMode::eByPolygon);
  matLayer->SetReferenceMode(FbxLayerElement::EReferenceMode::eIndexToDirect);
  layer->SetMaterials(matLayer);

  const std::vector<const FSkelMeshSection*> sections = lod->GetSections();

  for (const FSkelMeshSection* section : sections)
  {
    const int32 material = section->MaterialIndex;
    const int32 numTriangles = section->NumTriangles;

    for (int32 triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex)
    {
      mesh->BeginPolygon(material);
      for (int32 pointIdx = 0; pointIdx < 3; ++pointIdx)
      {
        int32 vertIndex = lod->GetIndexContainer()->GetIndex(section->BaseIndex + (triangleIndex * 3) + pointIdx);
        mesh->AddPolygon(vertIndex);
      }
      mesh->EndPolygon();
    }
  }

  char* meshName = FbxWideToUtf8(sourceMesh->GetObjectName().WString().c_str());
  FbxNode* meshNode = FbxNode::Create(GetScene(), meshName);
  FbxFree(meshName);
  meshNode->SetNodeAttribute(mesh);
  *outNode = (void*)meshNode;

  std::vector<UObject*> materials = sourceMesh->GetMaterials();
  for (int32 idx = 0; idx < materials.size(); ++idx)
  {
    UObject* mat = materials[idx];
    std::string matName = mat ? mat->GetObjectName().String() : ("Material_" + std::to_string(idx + 1));
    FbxSurfaceMaterial* fbxMaterial = FbxSurfaceLambert::Create(GetScene(), matName.c_str());
    ((FbxSurfaceLambert*)fbxMaterial)->Diffuse.Set(FbxDouble3(0.72, 0.72, 0.72));
    meshNode->AddMaterial(fbxMaterial);
  }

  if (!ctx.ExportSkeleton)
  {
    return true;
  }

  FbxDynamicArray<FbxNode*> bonesArray;
  FbxNode* skelRootNode = CreateSkeleton(sourceMesh, bonesArray, GetScene());

  if (!skelRootNode)
  {
    ctx.Error = "Failed to build the skeleton!";
    return false;
  }

  GetScene()->GetRootNode()->AddChild(skelRootNode);

  FbxAMatrix meshMatrix = meshNode->EvaluateGlobalTransform();
  FbxGeometry* meshAttribute = (FbxGeometry*)mesh;
  FbxSkin* skin = FbxSkin::Create(GetScene(), "");

  for (int boneIndex = 0; boneIndex < bonesArray.Size(); boneIndex++)
  {
    FbxNode* boneNode = bonesArray[boneIndex];

    FbxCluster* currentCluster = FbxCluster::Create(GetScene(), "");
    currentCluster->SetLink(boneNode);
    currentCluster->SetLinkMode(FbxCluster::eTotalOne);

    int32 vertIndex = 0;
    for (const FSoftSkinVertex& v : verticies)
    {
      for (int influenceIndex = 0; influenceIndex < MAX_INFLUENCES; influenceIndex++)
      {

        uint16 influenceBone = v.BoneMap->at(v.InfluenceBones[influenceIndex]);
        float w = (float)v.InfluenceWeights[influenceIndex];
        float influenceWeight = w / 255.0f;

        if (influenceBone == boneIndex && influenceWeight > 0.f)
        {
          currentCluster->AddControlPointIndex(vertIndex, influenceWeight);
        }
      }
      vertIndex++;
    }
    currentCluster->SetTransformMatrix(meshMatrix);
    FbxAMatrix linkMatrix = boneNode->EvaluateGlobalTransform();
    currentCluster->SetTransformLinkMatrix(linkMatrix);
    skin->AddCluster(currentCluster);
  }

  meshAttribute->AddDeformer(skin);
  CreateBindPose(meshNode, GetScene());
  return true;
}

bool FbxUtils::ExportStaticMesh(UStaticMesh* sourceMesh, FbxExportContext& ctx, void** outNode)
{
  const FStaticMeshRenderData* lod = sourceMesh->GetLod(ctx.LodIndex);
  if (!lod)
  {
    ctx.Error = "Failed to get the lod model!";
    return false;
  }

  const std::vector<FStaticVertex> verticies = lod->GetVertices();
  if (verticies.empty())
  {
    ctx.Error = "The model has no vertices!";
    return false;
  }

  FbxMesh* mesh = FbxMesh::Create(GetScene(), "geometry");
  mesh->InitControlPoints(verticies.size());

  for (uint32 idx = 0; idx < verticies.size(); ++idx)
  {
    const FStaticVertex& v = verticies[idx];
    mesh->GetControlPoints()[idx] = FbxVector4(v.Position.X, -v.Position.Y, v.Position.Z);
  }

  FbxLayer* layer = mesh->GetLayer(0);
  if (!layer)
  {
    layer = mesh->GetLayer(mesh->CreateLayer());
  }

  const int32 numTexCoords = lod->VertexBuffer.NumTexCoords;
  FbxLayerElementUV* uvDiffuseLayer = FbxLayerElementUV::Create(mesh, "DiffuseUV");
  std::vector<FbxLayerElementUV*> customUVLayers;
  for (int32 idx = 1; idx < numTexCoords; ++idx)
  {
    std::string layerName = "Custom_" + std::to_string(idx);
    customUVLayers.push_back(FbxLayerElementUV::Create(mesh, layerName.c_str()));
  }

  FbxLayerElementNormal* layerElementNormal = FbxLayerElementNormal::Create(mesh, "");
  FbxLayerElementBinormal* layerElementBinormal = FbxLayerElementBinormal::Create(mesh, "");
  FbxLayerElementTangent* layerElementTangent = FbxLayerElementTangent::Create(mesh, "");

  uvDiffuseLayer->SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
  uvDiffuseLayer->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);
  layerElementNormal->SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
  layerElementNormal->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);
  layerElementBinormal->SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
  layerElementBinormal->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);
  layerElementTangent->SetMappingMode(FbxLayerElement::EMappingMode::eByControlPoint);
  layerElementTangent->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);

  for (uint32 idx = 0; idx < verticies.size(); ++idx)
  {
    const FStaticVertex& v = verticies[idx];
    FVector tmp;
    tmp = v.TangentX;
    layerElementTangent->GetDirectArray().Add(FbxVector4(tmp.X, -tmp.Y, tmp.Z));
    tmp = v.TangentY;
    layerElementBinormal->GetDirectArray().Add(FbxVector4(tmp.X, -tmp.Y, tmp.Z));
    tmp = v.TangentZ;
    layerElementNormal->GetDirectArray().Add(FbxVector4(tmp.X, -tmp.Y, tmp.Z));

    uvDiffuseLayer->GetDirectArray().Add(FbxVector2(v.UVs[0].X, -v.UVs[0].Y + 1.f));
    for (int32 uvIdx = 0; uvIdx < customUVLayers.size(); ++uvIdx)
    {
      customUVLayers[uvIdx]->GetDirectArray().Add(FbxVector2(v.UVs[uvIdx + 1].X, -v.UVs[uvIdx + 1].Y + 1.f));
    }
  }

  layer->SetNormals(layerElementNormal);
  layer->SetBinormals(layerElementBinormal);
  layer->SetTangents(layerElementTangent);

  {
    int32 etype = (int32)FbxLayerElement::EType::eTextureDiffuse;
    layer->SetUVs(uvDiffuseLayer, (FbxLayerElement::EType)etype);
    for (int32 uvIdx = 0; uvIdx < customUVLayers.size(); ++uvIdx)
    {
      etype++;
      layer->SetUVs(customUVLayers[uvIdx], (FbxLayerElement::EType)etype);
    }
  }

  FbxLayerElementMaterial* matLayer = FbxLayerElementMaterial::Create(mesh, "");
  matLayer->SetMappingMode(FbxLayerElement::EMappingMode::eByPolygon);
  matLayer->SetReferenceMode(FbxLayerElement::EReferenceMode::eIndexToDirect);
  layer->SetMaterials(matLayer);

  const std::vector<FStaticMeshElement> sections = lod->GetElements();
  std::vector<UObject*> umaterials = sourceMesh->GetMaterials();
  for (const FStaticMeshElement& section : sections)
  {
    const int32 numTriangles = section.NumTriangles;
    if (!numTriangles)
    {
      continue;
    }

    int32 materialIndex = -1;
    for (int32 idx = 0; idx < umaterials.size(); ++idx)
    {
      UObject* material = umaterials[idx];
      if (material == section.Material)
      {
        materialIndex = idx;
        break;
      }
    }
    if (materialIndex == -1)
    {
      materialIndex = (int32)umaterials.size();
      umaterials.push_back(section.Material);
    }

    for (int32 triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex)
    {
      mesh->BeginPolygon(materialIndex);
      for (int32 pointIdx = 0; pointIdx < 3; ++pointIdx)
      {
        mesh->AddPolygon(lod->IndexBuffer.GetIndex(section.FirstIndex + (triangleIndex * 3) + pointIdx));
      }
      mesh->EndPolygon();
    }
  }

  char* meshName = FbxWideToUtf8(sourceMesh->GetObjectName().WString().c_str());
  FbxNode* meshNode = FbxNode::Create(GetScene(), meshName);
  FbxFree(meshName);
  meshNode->SetNodeAttribute(mesh);
  *outNode = (void*)meshNode;

  for (int32 idx = 0; idx < umaterials.size(); ++idx)
  {
    UObject* mat = umaterials[idx];
    std::string matName = mat ? mat->GetObjectName().String() : ("Material_" + std::to_string(idx + 1));
    FbxSurfaceMaterial* fbxMaterial = FbxSurfaceLambert::Create(GetScene(), matName.c_str());
    ((FbxSurfaceLambert*)fbxMaterial)->Diffuse.Set(FbxDouble3(0.72, 0.72, 0.72));
    meshNode->AddMaterial(fbxMaterial);
  }
  return true;
}
