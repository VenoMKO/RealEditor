#include <fbxsdk.h>

#include "FbxUtils.h"

#include <Utils/ALog.h>
#include <Tera/UPhysAsset.h>

#include <string>
#include <vector>
#include <utility>

#define MERGE_COVEX_HULLS 0

struct VTriangle {
  uint32 WedgeIndex[3] = { 0 };
  uint8 MatIndex = 0;
  FVector TangentX[3];
  FVector TangentY[3];
  FVector TangentZ[3];
};

struct VVertex {
  int32 VertexIndex = 0;
  FVector2D UVs[MAX_TEXCOORDS];
  uint16 MatIndex = 0;
};

struct FVertInfluence {
  float Weight = 0.;
  uint32 VertIndex = 0;
  uint16 BoneIndex = 0;
};

struct VBone {
  FString Name;
  VJointPos Transform;
  int32 ParentIndex = 0;
  int32 NumChildren = 0;
  int32 Depth = 0;
};

struct VMaterial {
  int32 MaterialIndex = 0;
  std::string MaterialImportName;
};

char* FbxWideToUtf8(const wchar_t* in)
{
  char* unistr = nullptr;
  size_t unistrSize = 0;
  FbxWCToUTF8(in, unistr, &unistrSize);
  return unistr;
}

bool IsOddNegativeScale(FbxAMatrix& totalMatrix)
{
  FbxVector4 scale = totalMatrix.GetS();
  int32 negativeCount = 0;

  if (scale[0] < 0) negativeCount++;
  if (scale[1] < 0) negativeCount++;
  if (scale[2] < 0) negativeCount++;

  return negativeCount == 1 || negativeCount == 3;
}

FbxAMatrix ComputeTotalMatrix(FbxNode* node, FbxScene* scene)
{
  FbxAMatrix geometry;
  FbxVector4 translation, rotation, scaling;
  translation = node->GetGeometricTranslation(FbxNode::eSourcePivot);
  rotation = node->GetGeometricRotation(FbxNode::eSourcePivot);
  scaling = node->GetGeometricScaling(FbxNode::eSourcePivot);
  geometry.SetT(translation);
  geometry.SetR(rotation);
  geometry.SetS(scaling);
  return scene->GetAnimationEvaluator()->GetNodeGlobalTransform(node) * geometry;
}

bool IsUnrealBone(FbxNode* link)
{
  if (FbxNodeAttribute* attr = link->GetNodeAttribute())
  {
    FbxNodeAttribute::EType type = attr->GetAttributeType();
    if (type == FbxNodeAttribute::eSkeleton ||
        type == FbxNodeAttribute::eMesh ||
        type == FbxNodeAttribute::eNull)
    {
      return true;
    }
  }
  return false;
}

void RecursiveBuildLinks(FbxNode* link, std::vector<FbxNode*>& outLinks)
{
  if (IsUnrealBone(link))
  {
    outLinks.push_back(link);
    for (int32 childIndex = 0; childIndex < link->GetChildCount(); childIndex++)
    {
      RecursiveBuildLinks(link->GetChild(childIndex), outLinks);
    }
  }
}

FbxNode* GetRootBone(FbxNode* link, FbxScene* scene)
{
  FbxNode* rootBone = link;
  while (rootBone->GetParent() && rootBone->GetParent()->GetSkeleton())
  {
    rootBone = rootBone->GetParent();
  }

  while (rootBone->GetParent())
  {
    FbxNodeAttribute* attr = rootBone->GetParent()->GetNodeAttribute();
    if (attr && (attr->GetAttributeType() == FbxNodeAttribute::eMesh || attr->GetAttributeType() == FbxNodeAttribute::eNull) &&
        rootBone->GetParent() != scene->GetRootNode())
    {
      if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
      {
        FbxMesh* mesh = (FbxMesh*)attr;
        if (mesh->GetDeformerCount(FbxDeformer::eSkin) > 0)
        {
          break;
        }
      }

      rootBone = rootBone->GetParent();
    }
    else
    {
      break;
    }
  }
  return rootBone;
}

void BuildSortedLinks(std::vector<FbxCluster*>& clusters, std::vector<FbxNode*>& outLinks, FbxScene* scene)
{
  int32 stop = 1;
  if (clusters.size() > 1)
  {
    stop = (int32)clusters.size() - 1;
  }

  FbxNode* link = nullptr;
  int32 clusterIndex = 0;
  std::vector<FbxNode*> rootLinks;
  for (; clusterIndex < stop; clusterIndex++)
  {
    link = clusters[clusterIndex]->GetLink();
    link = GetRootBone(link, scene);
    int32 linkIndex = 0;
    for (; linkIndex < rootLinks.size(); ++linkIndex)
    {
      if (link == rootLinks[linkIndex])
      {
        break;
      }
    }
    if (linkIndex == rootLinks.size())
    {
      rootLinks.push_back(link);
    }
  }

  for (auto& rootLink : rootLinks)
  {
    RecursiveBuildLinks(rootLink, outLinks);
  }
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

FbxNode* CreateSkeleton(USkeletalMesh* sourceMesh, FbxDynamicArray<FbxNode*>& boneNodes, FbxScene* scene, const FbxVector4& scale)
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

    FbxVector4 lT = FbxVector4(bone.BonePos.Position.X * scale[0], bone.BonePos.Position.Y * -scale[1], bone.BonePos.Position.Z * scale[2]);
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

  FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(GetManager(), "SceneInfo");
  sceneInfo->mTitle = "Real Editor Fbx Exporter";
  sceneInfo->mSubject = "Export FBX meshes from Tera";
  sceneInfo->Original_ApplicationVendor.Set("Yupi");
  sceneInfo->Original_ApplicationName.Set("Real Editor");
  sceneInfo->Original_ApplicationVersion.Set(FString::Sprintf("%.2f %s", APP_VER, BUILD_SUFFIX).UTF8().c_str());
  sceneInfo->LastSaved_ApplicationVendor.Set("Yupi");
  sceneInfo->LastSaved_ApplicationName.Set("Real Editor");
  sceneInfo->LastSaved_ApplicationVersion.Set(FString::Sprintf("%.2f %s", APP_VER, BUILD_SUFFIX).UTF8().c_str());
  GetScene()->SetSceneInfo(sceneInfo);
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
  FbxNode* firstLod = nullptr;
  if (ctx.ExportLods && sourceMesh->GetLodCount() > 1)
  {
    int32 lodCount = sourceMesh->GetLodCount();
    std::vector<FbxNode*> lods;
    lods.reserve(lodCount);
    for (int32 idx = 0; idx < lodCount; ++idx)
    {
      FbxNode* meshNode = nullptr;
      if (!ExportStaticMesh(sourceMesh, idx, ctx, (void**)&meshNode) || !meshNode)
      {
        continue;
      }
      if (ctx.ApplyRootTransform)
      {
        ApplyRootTransform(meshNode, ctx);
      }
      lods.push_back(meshNode);
    }
    if (lods.empty())
    {
      ctx.Error = "Failed to export the mesh. No lods found!";
      return false;
    }

    FbxNode* lodGroup = FbxNode::Create(GetScene(), sourceMesh->GetObjectName().UTF8().c_str());
    lodGroup->AddNodeAttribute(FbxLODGroup::Create(GetScene(), (sourceMesh->GetObjectName() + "_LodGroup").UTF8().c_str()));
    for (FbxNode* node : lods)
    {
      lodGroup->AddChild(node);
    }
    GetScene()->GetRootNode()->AddChild(lodGroup);
    firstLod = lods.front();
  }
  else
  {
    if (!ExportStaticMesh(sourceMesh, 0, ctx, (void**)&firstLod) || !firstLod)
    {
      return false;
    }

    if (ctx.ApplyRootTransform)
    {
      ApplyRootTransform(firstLod, ctx);
    }
    GetScene()->GetRootNode()->AddChild(firstLod);
  }

  if (ctx.ExportCollisions && firstLod)
  {
    std::vector<void*> colNodes;
    if (ExportCollision(sourceMesh, ctx, firstLod->GetNameOnly(), colNodes))
    {
      if (ctx.ApplyRootTransform)
      {
        for (void* node : colNodes)
        {
          ApplyRootTransform(node, ctx);
        }
      }
      for (void* node : colNodes)
      {
        GetScene()->GetRootNode()->AddChild((FbxNode*)node);
      }
    }
  }
  
  if (!SaveScene(ctx.Path, ctx.EmbedMedia))
  {
    ctx.Error = "Failed to write data!";
    return false;
  }
  return true;
}

bool FbxUtils::ImportSkeletalMesh(FbxImportContext& ctx)
{
  struct ScopedImporter {
    ScopedImporter(FbxImporter* imp)
      : Importer(imp)
    {}

    ~ScopedImporter()
    {
      if (Importer)
      {
        Importer->Destroy(true);
      }
    }
    FbxImporter* Importer = nullptr;
  };
  ScopedImporter holder(FbxImporter::Create(GetManager(), ""));

  char* uniPath = nullptr;
  size_t uniPathSize = 0;
  FbxWCToUTF8(ctx.Path.c_str(), uniPath, &uniPathSize);

  if (!holder.Importer->Initialize(uniPath, 0, GetManager()->GetIOSettings()))
  {
    ctx.Error = "Failed to open the FBX file! It may be corrupted or its version may be incompatible.";
    FbxFree(uniPath);
    return false;
  }
  FbxFree(uniPath);

  if (!holder.Importer->Import(GetScene()))
  {
    ctx.Error = "Failed to load a scene from the FBX file. File version may be incompatible.";
    return false;
  }

  {
    FbxAxisSystem::EFrontVector frontVector = (FbxAxisSystem::EFrontVector)-FbxAxisSystem::eParityOdd;
    const FbxAxisSystem unrealZUp(FbxAxisSystem::eZAxis, frontVector, FbxAxisSystem::eRightHanded);
    unrealZUp.ConvertScene(GetScene());
  }
  
  FbxSkeleton* skel = nullptr;
  FbxNode* meshNode = nullptr;
  FbxMesh* mesh = nullptr;

  for (int nodeIndex = 0; nodeIndex < GetScene()->GetNodeCount(); nodeIndex++)
  {
    FbxNode* node = GetScene()->GetNode(nodeIndex);
    if (node->GetSkeleton())
    {
      skel = node->GetSkeleton();
      if (mesh && meshNode)
      {
        break;
      }
    }
    if (node->GetMesh() && node->GetMesh()->GetDeformerCount(FbxDeformer::eSkin))
    {
      meshNode = node;
      mesh = meshNode->GetMesh();
      if (skel)
      {
        break;
      }
    }
  }

  if (!skel)
  {
    ctx.Error = "Your model has no skeleton/rig. Skeletal mesh must be rigged to a skeleton, exported by Real Editor.";
    return false;
  }
  if (!mesh)
  {
    ctx.Error = "The imported FBX scene has no geometry rigged to the skeleton!";
    return false;
  }

  if (!mesh->IsTriangleMesh())
  {
    ctx.Error = "The imported 3D model contains polygons with more than 3 vertices. Tera supports only triangular polygons.";
    return false;
  }

  std::vector<FbxCluster*> clusters;
  for (int deformerIndex = 0; deformerIndex < mesh->GetDeformerCount(FbxDeformer::eSkin); deformerIndex++)
  {
    FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(deformerIndex);
    if (skin->GetSkinningType() != fbxsdk::FbxSkin::eLinear && skin->GetSkinningType() != fbxsdk::FbxSkin::eRigid)
    {
      ctx.Error = "The imported 3D model uses unsupported skin type! Tera supports only classic linear skin with up to 4 weights/bones per vertex.";
      return false;
    }

    clusters.reserve(skin->GetClusterCount());
    for (int32 clusterIndex = 0; clusterIndex < skin->GetClusterCount(); ++clusterIndex)
    {
      clusters.emplace_back(skin->GetCluster(clusterIndex));
    }
  }

  std::vector<FString> mats;
  std::vector<VMaterial> materials;
  std::vector<FbxSurfaceMaterial*> fbxMaterials;
  for (int32 materialIndex = 0; materialIndex < meshNode->GetMaterialCount(); ++materialIndex)
  {
    FbxSurfaceMaterial* mat = meshNode->GetMaterial(materialIndex);
    if (std::find(fbxMaterials.begin(), fbxMaterials.end(), mat) == fbxMaterials.end())
    {
      fbxMaterials.emplace_back(mat);
      VMaterial vmat;
      vmat.MaterialImportName = mat->GetName();
      vmat.MaterialIndex = (int)fbxMaterials.size() - 1;
      mats.emplace_back(mat->GetName());
      materials.emplace_back(vmat);
    }
  }

  if (!clusters.size())
  {
    ctx.Error = "The imported 3D model has no skin. Make sure you've rigged it correctly.";
    return false;
  }

  if (!materials.size())
  {
    ctx.Error = "The imported 3D model has no materials.";
    return false;
  }

  std::vector<FbxNode*>sortedLinks;
  BuildSortedLinks(clusters, sortedLinks, GetScene());

  if (sortedLinks.size() > 128)
  {
    ctx.Error = "The imported skeleton has " + std::to_string(sortedLinks.size()) + " bones. Tera supports only up to 128 bones. You should rig your model to a skeleton exported by Real Editor.";
    return false;
  }

  std::vector<FbxAMatrix>globalsPerLink;
  globalsPerLink.resize(sortedLinks.size());
  globalsPerLink[0].SetIdentity();

  bool globalLinkFoundFlag = false;
  bool nonIdentityScaleFound = false;
  FbxVector4 localLinkT;
  FbxQuaternion localLinkQ;
  FbxNode* link = nullptr;
  std::vector<VBone> refBones;
  refBones.resize(sortedLinks.size());
  for (int32 linkIndex = 0; linkIndex < sortedLinks.size(); ++linkIndex)
  {
    link = sortedLinks[linkIndex];
    int32 parentIndex = 0;
    FbxNode* parent = link->GetParent();
    if (linkIndex)
    {
      for (int32 parentLinkIndex = 0; parentLinkIndex < linkIndex; ++parentLinkIndex)
      {
        FbxNode* otherLink = sortedLinks[parentLinkIndex];
        if (otherLink == parent)
        {
          parentIndex = parentLinkIndex;
          break;
        }
      }
    }

    globalLinkFoundFlag = false;
    for (FbxCluster* cluster : clusters)
    {
      if (link == cluster->GetLink())
      {
        cluster->GetTransformLinkMatrix(globalsPerLink[linkIndex]);
        globalLinkFoundFlag = true;
        break;
      }
    }
    if (!globalLinkFoundFlag)
    {
      // if root bone is not in bind pose and cluster, it is correct to use the local matrix as global matrix
      FbxAMatrix localMatrix;
      localMatrix.SetR(link->LclRotation.Get());
      FbxAMatrix postRotationMatrix, preRotationMatrix;
      FbxVector4 postRotation, preRotation;
      preRotation = link->GetPreRotation(FbxNode::eSourcePivot);
      postRotation = link->GetPostRotation(FbxNode::eSourcePivot);
      preRotationMatrix.SetR(preRotation);
      postRotationMatrix.SetR(postRotation);

      localMatrix = preRotationMatrix * localMatrix * postRotationMatrix;

      localLinkT = link->LclTranslation.Get();
      // bake the rotate pivot to translation
      FbxVector4 rotatePivot = link->GetRotationPivot(FbxNode::eSourcePivot);
      localLinkT[0] += rotatePivot[0];
      localLinkT[1] += rotatePivot[1];
      localLinkT[2] += rotatePivot[2];
      localLinkQ = localMatrix.GetQ();

      // if this skeleton has no cluster, its children may have cluster, so still need to set the Globals matrix
      localMatrix.SetT(localLinkT);
      localMatrix.SetS(link->LclScaling.Get());
      globalsPerLink[linkIndex] = globalsPerLink[parentIndex] * localMatrix;
    }
    if (linkIndex)
    {
      FbxAMatrix	matrix;
      matrix = globalsPerLink[parentIndex].Inverse() * globalsPerLink[linkIndex];
      localLinkT = matrix.GetT();
      localLinkQ = matrix.GetQ();
    }
    else	// skeleton root
    {
      // for root, this is global coordinate
      localLinkT = globalsPerLink[linkIndex].GetT();
      localLinkQ = globalsPerLink[linkIndex].GetQ();
    }
    {
      static float SCALE_TOLERANCE = .1f;
      FbxVector4 gs = globalsPerLink[linkIndex].GetS();
      if ((gs[0] > 1.0 + SCALE_TOLERANCE || gs[1] < 1.0 - SCALE_TOLERANCE) ||
          (gs[0] > 1.0 + SCALE_TOLERANCE || gs[1] < 1.0 - SCALE_TOLERANCE) ||
          (gs[0] > 1.0 + SCALE_TOLERANCE || gs[1] < 1.0 - SCALE_TOLERANCE))
      {
        nonIdentityScaleFound = true;
      }
    }
    

    VBone& bone = refBones[linkIndex];
    bone.Name = FString(link->GetName()).Split(':').back();
    bone.ParentIndex = parentIndex;
    bone.NumChildren = 0;

    for (int32 childIndex = 0; childIndex < link->GetChildCount(); ++childIndex)
    {
      FbxNode* child = link->GetChild(childIndex);
      if (IsUnrealBone(child))
      {
        bone.NumChildren++;
      }
    }

    bone.Transform.Position.X = static_cast<float>(localLinkT.mData[0]);
    bone.Transform.Position.Y = static_cast<float>(-localLinkT.mData[1]);
    bone.Transform.Position.Z = static_cast<float>(localLinkT.mData[2]);
    bone.Transform.Orientation.X = static_cast<float>(linkIndex ? -localLinkQ.mData[0] : localLinkQ.mData[0]);
    bone.Transform.Orientation.Y = static_cast<float>(linkIndex ? -localLinkQ.mData[1] : localLinkQ.mData[1]);
    bone.Transform.Orientation.Z = static_cast<float>(linkIndex ? -localLinkQ.mData[2] : localLinkQ.mData[2]);
    bone.Transform.Orientation.W = static_cast<float>(localLinkQ.mData[3]);
  }

  

  std::vector<FString> uvSets;
  if (mesh->GetLayerCount() > 0)
  {
    for (int32 layerIndex = 0; layerIndex < mesh->GetLayerCount(); layerIndex++)
    {
      FbxLayer* layer = mesh->GetLayer(layerIndex);
      FbxArray<FbxLayerElementUV const*> elementUVs = layer->GetUVSets();
      for (int uvSetIndex = 0; uvSetIndex < layer->GetUVSetCount(); uvSetIndex++)
      {
        FbxLayerElementUV const* element = elementUVs[uvSetIndex];
        if (element)
        {
          FbxString localuv = FbxString(element->GetName());
          if (std::find(uvSets.begin(), uvSets.end(), element->GetName()) == uvSets.end())
          {
            uvSets.emplace_back(element->GetName());
          }
        }
      }
    }
  }

  int32 controlPointsCount = mesh->GetControlPointsCount();
  int32 uniqueUVCount = static_cast<int32>(uvSets.size());
  FbxLayerElementUV** LayerElementUV = nullptr;
  FbxLayerElement::EReferenceMode* UVReferenceMode = nullptr;
  FbxLayerElement::EMappingMode* UVMappingMode = nullptr;
  if (uniqueUVCount > 0)
  {
    LayerElementUV = new FbxLayerElementUV * [uniqueUVCount];
    UVReferenceMode = new FbxLayerElement::EReferenceMode[uniqueUVCount];
    UVMappingMode = new FbxLayerElement::EMappingMode[uniqueUVCount];
  }

  for (uint32 uniqueUVIndex = 0; uniqueUVIndex < uniqueUVCount; uniqueUVIndex++)
  {
    LayerElementUV[uniqueUVIndex] = nullptr;
    for (int32 layerIndex = 0; layerIndex < mesh->GetLayerCount(); ++layerIndex)
    {
      FbxLayer* layer = mesh->GetLayer(layerIndex);
      FbxArray<const FbxLayerElementUV*> elements = layer->GetUVSets();
      for (int32 uvSetIndex = 0; uvSetIndex < layer->GetUVSetCount(); ++uvSetIndex)
      {
        const FbxLayerElementUV* element = elements[uvSetIndex];
        if (!element)
        {
          continue;
        }

        if (uvSets[uniqueUVIndex] == element->GetName())
        {
          LayerElementUV[uniqueUVIndex] = const_cast<FbxLayerElementUV*>(element);
          UVReferenceMode[uniqueUVIndex] = LayerElementUV[uvSetIndex]->GetReferenceMode();
          UVMappingMode[uniqueUVIndex] = LayerElementUV[uvSetIndex]->GetMappingMode();
          break;
        }
      }
    }
  }

  FbxLayer* baseLayer = mesh->GetLayer(0);
  FbxLayerElementMaterial* LayerElementMaterial = baseLayer->GetMaterials();
  FbxLayerElement::EMappingMode MaterialMappingMode = LayerElementMaterial ? LayerElementMaterial->GetMappingMode() : FbxLayerElement::eByPolygon;

  uniqueUVCount = std::min(uniqueUVCount, 4);

  FbxAMatrix totalMatrix;
  FbxAMatrix totalMatrixForNormal;
  totalMatrix = ComputeTotalMatrix(meshNode, GetScene());
  totalMatrixForNormal = totalMatrix.Inverse();
  totalMatrixForNormal = totalMatrixForNormal.Transpose();

  FbxLayerElementNormal* layerElementNormal = baseLayer->GetNormals();
  FbxLayerElementTangent* layerElementTangent = baseLayer->GetTangents();
  FbxLayerElementBinormal* layerElementBinormal = baseLayer->GetBinormals();
  FbxLayerElement::EReferenceMode normalReferenceMode(FbxLayerElement::eDirect);
  FbxLayerElement::EMappingMode normalMappingMode(FbxLayerElement::eByControlPoint);
  FbxLayerElement::EReferenceMode tangentReferenceMode(FbxLayerElement::eDirect);
  FbxLayerElement::EMappingMode tangentMappingMode(FbxLayerElement::eByControlPoint);

  if (layerElementNormal)
  {
    normalReferenceMode = layerElementNormal->GetReferenceMode();
    normalMappingMode = layerElementNormal->GetMappingMode();
  }

  if (layerElementTangent)
  {
    tangentReferenceMode = layerElementTangent->GetReferenceMode();
    tangentMappingMode = layerElementTangent->GetMappingMode();
  }

  bool bHasNormalInformation = layerElementNormal != nullptr;
  bool bHasTangentInformation = layerElementTangent != nullptr && layerElementBinormal != nullptr;
  if (!bHasNormalInformation)
  {
    ctx.MissingNormals = true;
  }
  std::vector<FbxVector4> points;
  points.reserve(controlPointsCount);
  for (int32 controlPointIndex = 0; controlPointIndex < controlPointsCount; ++controlPointIndex)
  {
    FbxVector4 pos = mesh->GetControlPoints()[controlPointIndex];
    pos = totalMatrix.MultT(pos);
    pos.mData[1] = static_cast<float>(pos.mData[1]) * -1.f;
    points.emplace_back(pos);
  }

  bool oddNegativeScale = IsOddNegativeScale(totalMatrix);

  int32 maxMaterialIndex = 0;
  int32 triangleCount = mesh->GetPolygonCount();
  std::vector<VTriangle> faces;
  faces.resize(triangleCount);
  FbxDynamicArray<VVertex> wedges = FbxDynamicArray<VVertex>();
  VVertex tmpWedges[3];
  for (int triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
  {
    VTriangle& triangle = faces[triangleIndex];

    for (int32 vertexIndex = 0; vertexIndex < 3; vertexIndex++)
    {
      int32 unrealVertexIndex = oddNegativeScale ? 2 - vertexIndex : vertexIndex;
      int32 controlPointIndex = mesh->GetPolygonVertex(triangleIndex, vertexIndex);

      if (bHasNormalInformation)
      {
        int32 tmpIndex = triangleIndex * 3 + vertexIndex;

        int32 normalMapIndex = (normalMappingMode == FbxLayerElement::eByControlPoint) ? controlPointIndex : tmpIndex;
        int32 normalValueIndex = (normalReferenceMode == FbxLayerElement::eDirect) ? normalMapIndex : layerElementNormal->GetIndexArray().GetAt(normalMapIndex);
        int32 tangentMapIndex = tmpIndex;

        // Normal

        FbxVector4 tempValue;
        tempValue = layerElementNormal->GetDirectArray().GetAt(normalValueIndex);
        tempValue = totalMatrixForNormal.MultT(tempValue);

        triangle.TangentZ[unrealVertexIndex].X = static_cast<float>(tempValue.mData[0]);
        triangle.TangentZ[unrealVertexIndex].Y = static_cast<float>(-tempValue.mData[1]);
        triangle.TangentZ[unrealVertexIndex].Z = static_cast<float>(tempValue.mData[2]);
        triangle.TangentZ[unrealVertexIndex].Normalize();

        if (bHasTangentInformation && ctx.ImportTangents)
        {
          tempValue = layerElementTangent->GetDirectArray().GetAt(tangentMapIndex);
          tempValue = totalMatrixForNormal.MultT(tempValue);

          triangle.TangentX[unrealVertexIndex].X = static_cast<float>(tempValue.mData[0]);
          triangle.TangentX[unrealVertexIndex].Y = static_cast<float>(-tempValue.mData[1]);
          triangle.TangentX[unrealVertexIndex].Z = static_cast<float>(tempValue.mData[2]);

          triangle.TangentY[unrealVertexIndex] = triangle.TangentX[unrealVertexIndex] ^ triangle.TangentZ[unrealVertexIndex];
          triangle.TangentY[unrealVertexIndex] = -triangle.TangentY[unrealVertexIndex].Z;
          triangle.TangentY[unrealVertexIndex].Normalize();
        }
      }
      else
      {
        triangle.TangentZ[unrealVertexIndex] = FVector(0);
        triangle.TangentY[unrealVertexIndex] = FVector(0);
        triangle.TangentX[unrealVertexIndex] = FVector(0);
      }
    }

    triangle.MatIndex = 0;
    for (int32 vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
    {
      if (LayerElementMaterial)
      {
        switch (MaterialMappingMode)
        {
        case FbxLayerElement::eAllSame:
          triangle.MatIndex = materials[LayerElementMaterial->GetIndexArray().GetAt(0)].MaterialIndex;
          break;
        case FbxLayerElement::eByPolygon:
          triangle.MatIndex = materials[LayerElementMaterial->GetIndexArray().GetAt(triangleIndex)].MaterialIndex;
          break;
        default:
          break;
        }
      }
    }
    maxMaterialIndex = std::max<int32>(maxMaterialIndex, triangle.MatIndex);

    for (int32 vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
    {
      int32 unrealVertexIndex = oddNegativeScale ? 2 - vertexIndex : vertexIndex;
      tmpWedges[unrealVertexIndex].MatIndex = triangle.MatIndex;
      tmpWedges[unrealVertexIndex].VertexIndex = mesh->GetPolygonVertex(triangleIndex, vertexIndex);
    }

    bool hasUVs = false;
    for (uint32 layerIndex = 0; layerIndex < uniqueUVCount; ++layerIndex)
    {
      if (LayerElementUV[layerIndex] != nullptr)
      {
        // Get each UV from the layer
        for (int vertexIndex = 0; vertexIndex < 3; +vertexIndex)
        {
          // If there are odd number negative scale, invert the vertex order for triangles
          int32 unrealVertexIndex = oddNegativeScale ? 2 - vertexIndex : vertexIndex;
          int32 controlPointIndex = mesh->GetPolygonVertex(triangleIndex, vertexIndex);
          int32 mapIndex = (UVMappingMode[layerIndex] == FbxLayerElement::eByControlPoint) ? controlPointIndex : triangleIndex * 3 + vertexIndex;
          int32 index = (UVReferenceMode[layerIndex] == FbxLayerElement::eDirect) ? mapIndex : LayerElementUV[layerIndex]->GetIndexArray().GetAt(mapIndex);

          FbxVector2 uv = LayerElementUV[layerIndex]->GetDirectArray().GetAt(index);
          tmpWedges[unrealVertexIndex].UVs[layerIndex].X = static_cast<float>(uv.mData[0]);
          tmpWedges[unrealVertexIndex].UVs[layerIndex].Y = 1.f - static_cast<float>(uv.mData[1]);
          if (!hasUVs && (uv.mData[0] || uv.mData[1]))
          {
            hasUVs = true;
          }
        }
      }
      else if (layerIndex == 0)
      {
        // Set all UV's to zero.  If we are here the mesh had no UV sets so we only need to do this for the
        // first UV set which always exists.
        for (auto & tmpWedge : tmpWedges)
        {
          tmpWedge.UVs[layerIndex].X = 0.0f;
          tmpWedge.UVs[layerIndex].Y = 0.0f;
        }
      }
    }

    if (!hasUVs)
    {
      return false;
    }

    for (int32 vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
    {
      VVertex wedge;
      wedge.VertexIndex = tmpWedges[vertexIndex].VertexIndex;
      wedge.MatIndex = tmpWedges[vertexIndex].MatIndex;
      wedge.UVs[0] = tmpWedges[vertexIndex].UVs[0];
      wedge.UVs[1] = tmpWedges[vertexIndex].UVs[1];
      wedge.UVs[2] = tmpWedges[vertexIndex].UVs[2];
      wedge.UVs[3] = tmpWedges[vertexIndex].UVs[3];
      wedges.PushBack(wedge);
      triangle.WedgeIndex[vertexIndex] = (int)wedges.Size() - 1;
    }
  }
  // weights
  std::vector<FVertInfluence> influences;
  if (FbxSkin* skin = (FbxSkin*)static_cast<FbxGeometry*>(mesh)->GetDeformer(0, FbxDeformer::eSkin))
  {
    for (int32 clusterIndex = 0; clusterIndex < skin->GetClusterCount(); ++clusterIndex)
    {
      FbxCluster* cluster = skin->GetCluster(clusterIndex);

      // When Maya plug-in exports rigid binding, it will generate "CompensationCluster" for each ancestor links.
      // FBX writes these "CompensationCluster" out. The CompensationCluster also has weight 1 for vertices.
      // Unreal importer should skip these clusters.
      if (strcmp(cluster->GetUserDataID(), "Maya_ClusterHint") == 0 || strcmp(cluster->GetUserDataID(), "CompensationCluster") == 0)
      {
        continue;
      }

      FbxNode* link = cluster->GetLink();
      int32 boneIndex = -1;
      for (int32 linkIndex = 0; linkIndex < sortedLinks.size(); ++linkIndex)
      {
        if (link == sortedLinks[linkIndex])
        {
          boneIndex = linkIndex;
          break;
        }
      }
      int32 controlPointIndicesCount = cluster->GetControlPointIndicesCount();
      int* controlPointIndices = cluster->GetControlPointIndices();
      double* weights = cluster->GetControlPointWeights();

      for (int32 controlPointIndex = 0; controlPointIndex < controlPointIndicesCount; ++controlPointIndex)
      {
        FVertInfluence influence;
        influence.BoneIndex = boneIndex;
        influence.Weight = static_cast<float>(weights[controlPointIndex]);
        influence.VertIndex = controlPointIndices[controlPointIndex];
        influences.emplace_back(influence);
      }
    }
  }
  else
  {
    // Rigid mesh
    int32 boneIndex = -1;
    for (int32 linkIndex = 0; linkIndex < sortedLinks.size(); ++linkIndex)
    {
      if (meshNode == sortedLinks[linkIndex])
      {
        boneIndex = linkIndex;
        break;
      }
    }
    for (int32 controlPointIndex = 0; controlPointIndex < controlPointsCount; ++controlPointIndex)
    {
      FVertInfluence influence;
      influence.BoneIndex = boneIndex;
      influence.Weight = 1.0;
      influence.VertIndex = controlPointIndex;
      influences.emplace_back(influence);
    }

  }

  ctx.ImportData.Points.clear();
  ctx.ImportData.Points.reserve(points.size());
  for (const auto& p : points)
  {
    ctx.ImportData.Points.emplace_back(p.mData[0], p.mData[1], p.mData[2]);
  }
  ctx.ImportData.Wedges.resize(wedges.Size());
  for (int i = 0; i < (int)wedges.Size(); i++)
  {
    ctx.ImportData.Wedges[i].pointIndex = wedges[i].VertexIndex;
    ctx.ImportData.Wedges[i].materialIndex = wedges[i].MatIndex;
    for (int j = 0; j < 4; j++)
    {
      ctx.ImportData.Wedges[i].UV[j] = wedges[i].UVs[j];
    }
  }

  ctx.ImportData.Faces.resize(triangleCount);
  for (int32 faceIndex = 0; faceIndex < triangleCount; ++faceIndex)
  {
    ctx.ImportData.Faces[faceIndex].materialIndex = faces[faceIndex].MatIndex;
    for (int32 wedgeIndex = 0; wedgeIndex < 3; ++wedgeIndex)
    {
      ctx.ImportData.Faces[faceIndex].wedgeIndices[wedgeIndex] = faces[faceIndex].WedgeIndex[wedgeIndex];
      ctx.ImportData.Faces[faceIndex].tangentX[wedgeIndex] = faces[faceIndex].TangentX[wedgeIndex];
      ctx.ImportData.Faces[faceIndex].tangentY[wedgeIndex] = faces[faceIndex].TangentY[wedgeIndex];
      ctx.ImportData.Faces[faceIndex].tangentZ[wedgeIndex] = faces[faceIndex].TangentZ[wedgeIndex];
    }
  }

  std::vector<FVector> tan1;
  std::vector<FVector> tan2;
  tan1.resize(points.size());
  tan2.resize(points.size());

  if (!ctx.ImportTangents || !bHasTangentInformation)
  {
    for (int32 faceIndex = 0; faceIndex < triangleCount; ++faceIndex)
    {
      unsigned int i1 = ctx.ImportData.Wedges[ctx.ImportData.Faces[faceIndex].wedgeIndices[0]].pointIndex;
      unsigned int i2 = ctx.ImportData.Wedges[ctx.ImportData.Faces[faceIndex].wedgeIndices[1]].pointIndex;
      unsigned int i3 = ctx.ImportData.Wedges[ctx.ImportData.Faces[faceIndex].wedgeIndices[2]].pointIndex;

      FVector v1 = ctx.ImportData.Points[i1];
      FVector v2 = ctx.ImportData.Points[i2];
      FVector v3 = ctx.ImportData.Points[i3];

      FVector2D w1 = ctx.ImportData.Wedges[ctx.ImportData.Faces[faceIndex].wedgeIndices[0]].UV[0];
      FVector2D w2 = ctx.ImportData.Wedges[ctx.ImportData.Faces[faceIndex].wedgeIndices[1]].UV[0];
      FVector2D w3 = ctx.ImportData.Wedges[ctx.ImportData.Faces[faceIndex].wedgeIndices[2]].UV[0];

      float x1 = v2.X - v1.X;
      float x2 = v3.X - v1.X;
      float y1 = v2.Y - v1.Y;
      float y2 = v3.Y - v1.Y;
      float z1 = v2.Z - v1.Z;
      float z2 = v3.Z - v1.Z;

      float s1 = w2.X - w1.X;
      float s2 = w3.X - w1.X;
      float t1 = w2.Y - w1.Y;
      float t2 = w3.Y - w1.Y;

      float div = s1 * t2 - s2 * t1;
      float r = div == 0.0f ? 0.0f : 1.0f / div;

      FVector sdir = FVector((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
      FVector tdir = FVector((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

      tan1[i1] += sdir;
      tan1[i2] += sdir;
      tan1[i3] += sdir;

      tan2[i1] += tdir;
      tan2[i2] += tdir;
      tan2[i3] += tdir;
    }
    for (int32 faceIndex = 0; faceIndex < triangleCount; ++faceIndex)
    {
      for (int32 wedgeIndex = 0; wedgeIndex < 3; ++wedgeIndex)
      {
        FVector normal = ctx.ImportData.Faces[faceIndex].tangentZ[wedgeIndex];
        FVector tangent = tan1[ctx.ImportData.Wedges[ctx.ImportData.Faces[faceIndex].wedgeIndices[wedgeIndex]].pointIndex];

        // Gram-Schmidt orthogonalize
        tangent = (tangent - (normal * (normal | tangent))).Normalize();
        ctx.ImportData.Faces[faceIndex].tangentX[wedgeIndex] = tangent;
        ctx.ImportData.Faces[faceIndex].basis[wedgeIndex] = ((normal ^ tangent) | tan2[ctx.ImportData.Wedges[ctx.ImportData.Faces[faceIndex].wedgeIndices[wedgeIndex]].pointIndex]) < 0 ? -1.f : 1.f;
        ctx.ImportData.Faces[faceIndex].tangentY[wedgeIndex] = (tangent ^ normal);
      }
    }
  }
  ctx.ImportData.FlipTangents = !ctx.ImportTangents || !bHasTangentInformation;
  ctx.ImportData.Influences.resize(influences.size());
  for (int32 influenceIndex = 0; influenceIndex < (int)influences.size(); ++influenceIndex)
  {
    ctx.ImportData.Influences[influenceIndex].weight = (float)influences[influenceIndex].Weight;
    ctx.ImportData.Influences[influenceIndex].boneIndex = (int)influences[influenceIndex].BoneIndex;
    ctx.ImportData.Influences[influenceIndex].vertexIndex = (int)influences[influenceIndex].VertIndex;
  }

  ctx.ImportData.Materials = mats;
  ctx.ImportData.Bones.resize(refBones.size());
  for (int32 boneIndex = 0; boneIndex < refBones.size(); ++boneIndex)
  {
    RawBone& bone = ctx.ImportData.Bones[boneIndex];
    const VBone& refBone = refBones[boneIndex];
    bone.boneName = refBone.Name;
    bone.parentIndex = refBone.ParentIndex;
    bone.orientation = refBone.Transform.Orientation;
    bone.position = refBone.Transform.Position;
  }
  ctx.ImportData.UVSetCount = uniqueUVCount;
  if (uniqueUVCount > 0)
  {
    delete[] LayerElementUV;
    delete[] UVReferenceMode;
    delete[] UVMappingMode;
  }
  return true;
}

bool FbxUtils::ImportStaticMesh(FbxImportContext& ctx)
{
  return false;
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
  const FStaticLODModel* lod = sourceMesh->GetLod(0);
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
    mesh->GetControlPoints()[idx] = FbxVector4(v.Position.X * ctx.Scale3D.X, -v.Position.Y * ctx.Scale3D.Y, v.Position.Z * ctx.Scale3D.Z);
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
  FbxNode* skelRootNode = CreateSkeleton(sourceMesh, bonesArray, GetScene(), FbxVector4(ctx.Scale3D.X, ctx.Scale3D.Y, ctx.Scale3D.Z));

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

bool FbxUtils::ExportStaticMesh(UStaticMesh* sourceMesh, int32 lodIdx, FbxExportContext& ctx, void** outNode)
{
  const FStaticMeshRenderData* lod = sourceMesh->GetLod(lodIdx);
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

  const int32 numTexCoords = ctx.ExportLightMapUVs ? lod->VertexBuffer.NumTexCoords : 1;
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
  FString objectName = sourceMesh->GetObjectName();
  if (ctx.ExportLods && sourceMesh->GetLodCount() > 1)
  {
    objectName += FString::Sprintf("_LOD%d", lodIdx);
  }
  char* meshName = FbxWideToUtf8(objectName.WString().c_str());
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

bool FbxUtils::ExportCollision(UStaticMesh* sourceMesh, FbxExportContext& ctx, const char* meshName, std::vector<void*>& outNodes)
{
  URB_BodySetup* bodySetup = sourceMesh->GetBodySetup();
  if (!bodySetup)
  {
    return false;
  }

  FKAggregateGeom aggGeom = bodySetup->GetAggregateGeometry();

  if (MERGE_COVEX_HULLS)
  {
    int32 totalVertCount = 0;
    int32 totalIndexCount = 0;
    for (FKConvexElem& element : aggGeom.ConvexElems)
    {
      totalVertCount += (int32)element.VertexData.size();
      totalIndexCount += (int32)element.FaceTriData.size();
    }
    if (!totalVertCount || !totalIndexCount)
    {
      return false;
    }

    FbxNode* colNode = FbxNode::Create(GetScene(), ("UCX_" + FString(meshName)).UTF8().c_str());
    FbxMesh* mesh = FbxMesh::Create(GetScene(), ("UCX_" + FString(meshName) + "_AggGeom").UTF8().c_str());
    colNode->SetNodeAttribute(mesh);
    outNodes.push_back(colNode);

    FbxLayer* layer = mesh->GetLayer(0);
    if (!layer)
    {
      layer = mesh->GetLayer(mesh->CreateLayer());
    }

    FbxLayerElementNormal* layerElementNormal = FbxLayerElementNormal::Create(mesh, "");
    layerElementNormal->SetMappingMode(FbxLayerElement::EMappingMode::eByPolygonVertex);
    layerElementNormal->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);

    FbxSurfaceMaterial* fbxMat = nullptr;
    const int32 matIdx = colNode->AddMaterial(fbxMat);

    mesh->InitControlPoints(totalVertCount);
    std::vector<int32> indicies;
    std::vector<FVector> points;
    {
      int32 cpIdx = 0;
      int32 wgOffset = 0;
      for (FKConvexElem& element : aggGeom.ConvexElems)
      {
        for (FVector& v : element.VertexData)
        {
          mesh->GetControlPoints()[cpIdx] = FbxVector4(v.X, -v.Y, v.Z);
          points.push_back(v);
          cpIdx++;
        }
        for (int32 idx : element.FaceTriData)
        {
          indicies.push_back(idx + wgOffset);
        }
        wgOffset += (int32)element.VertexData.size();
      }
    }

    for (int32 pointIdx = 0; pointIdx < totalIndexCount; pointIdx += 3)
    {
      mesh->BeginPolygon(matIdx);
      mesh->AddPolygon(indicies[pointIdx + 0]);
      mesh->AddPolygon(indicies[pointIdx + 1]);
      mesh->AddPolygon(indicies[pointIdx + 2]);
      mesh->EndPolygon();

      FVector a = points[pointIdx + 0];
      FVector b = points[pointIdx + 1];
      FVector c = points[pointIdx + 2];
      FVector normal = (a - b) ^ (b - c);
      normal.Normalize();
      layerElementNormal->GetDirectArray().Add(FbxVector4(-normal.X, normal.Y, -normal.Z));
      layerElementNormal->GetDirectArray().Add(FbxVector4(-normal.X, normal.Y, -normal.Z));
      layerElementNormal->GetDirectArray().Add(FbxVector4(-normal.X, normal.Y, -normal.Z));
    }
    layer->SetNormals(layerElementNormal);
  }
  else
  {
    for (int32 elementIndex = 0; elementIndex < aggGeom.ConvexElems.size(); ++elementIndex)
    {
      const FKConvexElem& element = aggGeom.ConvexElems[elementIndex];
      FbxNode* colNode = FbxNode::Create(GetScene(), FString::Sprintf("UCX_%s_%d", meshName, elementIndex).UTF8().c_str());
      FbxMesh* mesh = FbxMesh::Create(GetScene(), FString::Sprintf("UCX_%s_%d_AggGeom", meshName, elementIndex).UTF8().c_str());
      colNode->SetNodeAttribute(mesh);
      outNodes.push_back(colNode);

      FbxLayer* layer = mesh->GetLayer(0);
      if (!layer)
      {
        layer = mesh->GetLayer(mesh->CreateLayer());
      }

      FbxLayerElementNormal* layerElementNormal = FbxLayerElementNormal::Create(mesh, "");
      layerElementNormal->SetMappingMode(FbxLayerElement::EMappingMode::eByPolygonVertex);
      layerElementNormal->SetReferenceMode(FbxLayerElement::EReferenceMode::eDirect);

      FbxSurfaceMaterial* fbxMat = nullptr;
      const int32 matIdx = colNode->AddMaterial(fbxMat);
      mesh->InitControlPoints((int)element.VertexData.size());
      for (int32 cpIdx = 0; cpIdx < element.VertexData.size(); ++cpIdx)
      {
        const FVector& v = element.VertexData[cpIdx];
        mesh->GetControlPoints()[cpIdx] = FbxVector4(v.X, -v.Y, v.Z);
      }

      for (int32 pointIdx = 0; pointIdx < element.FaceTriData.size(); pointIdx += 3)
      {
        mesh->BeginPolygon(matIdx);
        mesh->AddPolygon(element.FaceTriData[pointIdx + 0]);
        mesh->AddPolygon(element.FaceTriData[pointIdx + 1]);
        mesh->AddPolygon(element.FaceTriData[pointIdx + 2]);
        mesh->EndPolygon();

        FVector a = element.VertexData[element.FaceTriData[pointIdx + 0]];
        FVector b = element.VertexData[element.FaceTriData[pointIdx + 1]];
        FVector c = element.VertexData[element.FaceTriData[pointIdx + 2]];
        FVector normal = (a - b) ^ (b - c);
        normal.Normalize();
        layerElementNormal->GetDirectArray().Add(FbxVector4(-normal.X, normal.Y, -normal.Z));
        layerElementNormal->GetDirectArray().Add(FbxVector4(-normal.X, normal.Y, -normal.Z));
        layerElementNormal->GetDirectArray().Add(FbxVector4(-normal.X, normal.Y, -normal.Z));
      }
      layer->SetNormals(layerElementNormal);
    }
  }

  
  return true;
}

void FbxUtils::ApplyRootTransform(void* node, FbxExportContext& ctx)
{
  FbxNode* meshNode = (FbxNode*)node;
  FbxDouble3 translation = meshNode->LclTranslation.Get();
  if (!ctx.PrePivot.IsZero())
  {
    translation[0] -= ctx.PrePivot.X;
    translation[1] -= -ctx.PrePivot.Y;
    translation[2] -= ctx.PrePivot.Z;
  }
  if (!ctx.Translation.IsZero())
  {
    translation[0] += ctx.Translation.X;
    translation[1] += -ctx.Translation.Y;
    translation[2] += ctx.Translation.Z;
  }
  meshNode->LclTranslation.Set(translation);

  meshNode->LclScaling.Set(FbxDouble3(ctx.Scale3D.X, ctx.Scale3D.Y, ctx.Scale3D.Z));

  if (!ctx.Rotation.IsZero())
  {
    FVector rot = ctx.Rotation.Euler();
    meshNode->LclRotation.Set(FbxDouble3(rot.X, -rot.Y, -rot.Z));
  }
}
