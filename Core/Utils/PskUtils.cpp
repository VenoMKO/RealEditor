#include "PskUtils.h"

#include <numeric>
#include <filesystem>

struct PskChunkHeader {
  static constexpr int32 DefaultVersion = 1999801;
  PskChunkHeader() = default;

  PskChunkHeader(const char* id, int32 size = 0, int32 count = 0)
  {
    SetChunkID(id);
    Size = size;
    Count = count;
  }

  void SetChunkID(const char* id)
  {
    memset(ChunkID, 0, sizeof(ChunkID));
    if (!id)
    {
      return;
    }
    size_t len = std::min<size_t>(strlen(id), sizeof(ChunkID) - 1);
    memcpy(ChunkID, id, len);
  }

  friend FStream& operator<<(FStream& s, PskChunkHeader& h);

  char ChunkID[20] = { 0 };
  int32 Flags = DefaultVersion;
  int32 Size = 0;
  int32 Count = 0;
};

FStream& operator<<(FStream& s, PskChunkHeader& h)
{
  s.SerializeBytes(h.ChunkID, sizeof(h.ChunkID));
  s << h.Flags;
  s << h.Size;
  s << h.Count;
  return s;
}

bool PskUtils::ExportSkeletalMesh(USkeletalMesh* sourceMesh, MeshExportContext& ctx)
{
  const FStaticLODModel* lod = sourceMesh->GetLod(0);
  if (!lod)
  {
    ctx.Error = "Failed to get the lod model!";
    return false;
  }
  std::vector<const FSkelMeshSection*> sections = lod->GetSections();
  const std::vector<FSoftSkinVertex> verticies = lod->GetVertices();
  if (verticies.empty())
  {
    ctx.Error = "The model has no vertices!";
    return false;
  }

  std::vector<UObject*> umaterials = sourceMesh->GetMaterials();
  std::map<int32, int32> materialMap;
  std::vector<VMaterial> materials;
  for (int32 idx = 0; idx < umaterials.size(); ++idx)
  {
    bool exists = false;
    UObject* mat = umaterials[idx];
    for (int32 tidx = 0; tidx < idx; ++tidx)
    {
      if (umaterials[tidx] == mat)
      {
        // Normalize materials
        exists = true;
        materialMap[idx] = tidx;
        break;
      }
    }
    if (exists)
    {
      continue;
    }
    materialMap[idx] = idx;
    std::string matName = mat ? mat->GetObjectName().String() : ("Material_" + std::to_string(idx + 1));
    VMaterial& vmat = materials.emplace_back();
    vmat.SetName(matName.c_str());
  }

  // Denormalize mesh by Hoppe wedge idiom
  std::vector<FVector> controlPoints;
  std::vector<FVertexInfluence> controlPointsWeights;
  std::vector<VTriangle> faces;
  std::vector<VVertex> wedges;
  std::map<size_t, size_t> wedgeToVertex;
  for (const FSkelMeshSection* section : sections)
  {
    const int32 material = materialMap[section->MaterialIndex];
    const int32 numTriangles = section->NumTriangles;
    for (int32 triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex)
    {
      VTriangle& face = faces.emplace_back();
      face.MatIndex = material;
      for (int32 pointIdx = 0; pointIdx < 3; ++pointIdx)
      {
        bool exists = false;
        int32 vertexIndex = lod->GetIndexContainer()->GetIndex(section->BaseIndex + (triangleIndex * 3) + pointIdx);
        const FSoftSkinVertex& vertex = verticies[vertexIndex];
        for (int32 wedgeIndex = 0; wedgeIndex < wedges.size(); ++wedgeIndex)
        {
          const VVertex& wedge = wedges[wedgeIndex];
          const FSoftSkinVertex& wedgeSrc = verticies[wedgeToVertex[wedgeIndex]];
          if (wedge.MatIndex != material)
          {
            continue;
          }
          if (!PointsEqual(controlPoints[wedge.VertexIndex], vertex.Position))
          {
            continue;
          }
          if (!NormalsEqual(wedge.TangentZ, vertex.TangentZ))
          {
            continue;
          }
          if (!UVsEqual(wedge.UVs[0], vertex.UVs[0]))
          {
            continue;
          }
          if (*(uint32*)vertex.InfluenceBones == *(uint32*)wedgeSrc.InfluenceBones ||
              *(uint32*)vertex.InfluenceWeights == *(uint32*)wedgeSrc.InfluenceWeights)
          {
            exists = true;
            face.WedgeIndex[pointIdx] = wedgeIndex;
            break;
          }
        }
        if (exists)
        {
          continue;
        }
        wedgeToVertex[wedges.size()] = vertexIndex;
        face.WedgeIndex[pointIdx] = (int32)wedges.size();
        VVertex& wedge = wedges.emplace_back();
        exists = false;
        for (int32 posIdx = 0; posIdx < controlPoints.size(); ++posIdx)
        {
          if (controlPoints[posIdx] != vertex.Position)
          {
            continue;
          }
          FVertexInfluence& influence = controlPointsWeights[posIdx];
          bool infMatch = true;
          for (int32 infIdx = 0; infIdx < MAX_INFLUENCES; ++infIdx)
          {
            if (influence.Bones.InfluenceBones[infIdx] != vertex.BoneMap->at(vertex.InfluenceBones[infIdx]) ||
                influence.Wieghts.InfluenceWeights[infIdx] != vertex.InfluenceWeights[infIdx])
            {
              infMatch = false;
              break;
            }
          }
          if (infMatch)
          {
            exists = true;
            wedge.VertexIndex = posIdx;
            break;
          }
        }
        if (!exists)
        {
          wedge.VertexIndex = (int32)controlPoints.size();
          controlPoints.emplace_back(vertex.Position);
          FVertexInfluence& influence = controlPointsWeights.emplace_back();
          for (int32 infIdx = 0; infIdx < MAX_INFLUENCES; ++infIdx)
          {
            influence.Bones.InfluenceBones[infIdx] = vertex.BoneMap->at(vertex.InfluenceBones[infIdx]);
            influence.Wieghts.InfluenceWeights[infIdx] = vertex.InfluenceWeights[infIdx];
          }
        }
        wedge.UVs[0] = vertex.UVs[0];
        wedge.MatIndex = material;
        wedge.TangentZ = vertex.TangentZ;
      }
      std::swap(face.WedgeIndex[0], face.WedgeIndex[1]);
    }
  }

  std::vector<FNamedBoneBinary> bones;
  std::vector<FVertInfluence> influences;
  if (ctx.ExportSkeleton)
  {
    std::vector<FMeshBone> refBones = sourceMesh->GetReferenceSkeleton();
    bones.resize(refBones.size());
    for (int32 idx = 0; idx < refBones.size(); ++idx)
    {
      const FMeshBone& refBone = refBones[idx];
      FNamedBoneBinary& bone = bones[idx];
      bone.SetName(refBone.Name.String().UTF8().c_str());
      bone.ParentIndex = refBone.ParentIndex;
      bone.NumChildren = refBone.NumChildren;
      bone.BonePos = refBone.BonePos;
      bone.BonePos.Position *= ctx.Scale3D;
      bone.BonePos.Position.Y *= -1.f;
      bone.BonePos.Orientation.Y *= -1.f;
    }
  }

  FWriteStream s(ctx.Path);
  PskChunkHeader mainHeader("ACTRHEAD");
  s << mainHeader;
  PskChunkHeader pointsHeader("PNTS0000", sizeof(FVector), (int32)controlPoints.size());
  s << pointsHeader;
  FVector tmpPos;
  for (const FVector& pos : controlPoints)
  {
    tmpPos = pos * ctx.Scale3D;
    tmpPos.Y = -tmpPos.Y;
    s << tmpPos;
  }
  PskChunkHeader wedgesHeader("VTXW0000", sizeof(VVertexExport), (int32)wedges.size());
  s << wedgesHeader;
  for (const VVertex& wedge : wedges)
  {
    VVertexExport tmp(wedge);
    s << tmp;
  }
  PskChunkHeader facesHeader("FACE0000", sizeof(VTriangleExport), (int32)faces.size());
  s << facesHeader;
  for (const VTriangle& face : faces)
  {
    VTriangleExport tmp(face);
    s << tmp;
  }
  PskChunkHeader matHeader("MATT0000", sizeof(VMaterial), (int32)materials.size());
  s << matHeader;
  for (VMaterial& mat : materials)
  {
    s << mat;
  }
  PskChunkHeader skelHeader("REFSKELT", sizeof(FNamedBoneBinary), bones.size());
  s << skelHeader;
  for (FNamedBoneBinary& bone : bones)
  {
    s << bone;
  }
  if (ctx.ExportSkeleton)
  {
    for (int32 cpIdx = 0; cpIdx < controlPointsWeights.size(); ++cpIdx)
    {
      const FVertexInfluence& inf = controlPointsWeights[cpIdx];
      for (int32 idx = 0; idx < MAX_INFLUENCES; ++idx)
      {
        if (inf.Wieghts.InfluenceWeights[idx])
        {
          float w = (float)inf.Wieghts.InfluenceWeights[idx];
          w = w / 255.0f;
          influences.emplace_back(w, cpIdx, inf.Bones.InfluenceBones[idx]);
        }
      }
    }
  }
  PskChunkHeader inflHeader("RAWWEIGHTS", sizeof(FVertInfluence), influences.size());
  s << inflHeader;
  for (FVertInfluence& inf : influences)
  {
    s << inf;
  }
  return true;
}

bool PskUtils::ExportStaticMesh(UStaticMesh* sourceMesh, MeshExportContext& ctx)
{
  const FStaticMeshRenderData* lod = sourceMesh->GetLod(0);
  if (!lod)
  {
    ctx.Error = "Failed to get the lod model!";
    return false;
  }
  std::vector<FStaticMeshElement> sections = lod->GetElements();
  const std::vector<FStaticVertex> verticies = lod->GetVertices();
  if (verticies.empty())
  {
    ctx.Error = "The model has no vertices!";
    return false;
  }

  std::vector<UObject*> umaterials = sourceMesh->GetMaterials();
  std::map<int32, int32> materialMap;
  std::vector<VMaterial> materials;
  for (int32 idx = 0; idx < umaterials.size(); ++idx)
  {
    bool exists = false;
    UObject* mat = umaterials[idx];
    for (int32 tidx = 0; tidx < idx; ++tidx)
    {
      if (umaterials[tidx] == mat)
      {
        // Normalize materials
        exists = true;
        materialMap[idx] = tidx;
        break;
      }
    }
    if (exists)
    {
      continue;
    }
    materialMap[idx] = idx;
    std::string matName = mat ? mat->GetObjectName().String() : ("Material_" + std::to_string(idx + 1));
    VMaterial& vmat = materials.emplace_back();
    vmat.SetName(matName.c_str());
  }

  // Denormalize mesh by Hoppe wedge idiom
  std::vector<FVector> controlPoints;
  std::vector<FVertexInfluence> controlPointsWeights;
  std::vector<VTriangle> faces;
  std::vector<VVertex> wedges;
  std::map<size_t, size_t> wedgeToVertex;
  for (const FStaticMeshElement& section : sections)
  {
    const int32 material = materialMap[section.MaterialIndex];
    const int32 numTriangles = section.NumTriangles;
    if (!numTriangles)
    {
      continue;
    }
    for (int32 triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex)
    {
      VTriangle& face = faces.emplace_back();
      face.MatIndex = material;
      for (int32 pointIdx = 0; pointIdx < 3; ++pointIdx)
      {
        bool exists = false;
        int32 vertexIndex = lod->IndexBuffer.GetIndex(section.FirstIndex + (triangleIndex * 3) + pointIdx);
        const FStaticVertex& vertex = verticies[vertexIndex];
        for (int32 wedgeIndex = 0; wedgeIndex < wedges.size(); ++wedgeIndex)
        {
          const VVertex& wedge = wedges[wedgeIndex];
          const FStaticVertex& wedgeSrc = verticies[wedgeToVertex[wedgeIndex]];
          if (wedge.MatIndex != material)
          {
            continue;
          }
          if (!PointsEqual(controlPoints[wedge.VertexIndex], vertex.Position))
          {
            continue;
          }
          if (!NormalsEqual(wedge.TangentZ, vertex.TangentZ))
          {
            continue;
          }
          if (!UVsEqual(wedge.UVs[0], vertex.UVs[0]))
          {
            continue;
          }
          exists = true;
          face.WedgeIndex[pointIdx] = wedgeIndex;
          break;
        }
        if (exists)
        {
          continue;
        }
        wedgeToVertex[wedges.size()] = vertexIndex;
        face.WedgeIndex[pointIdx] = (int32)wedges.size();
        VVertex& wedge = wedges.emplace_back();
        exists = false;
        for (int32 posIdx = 0; posIdx < controlPoints.size(); ++posIdx)
        {
          if (controlPoints[posIdx] != vertex.Position)
          {
            continue;
          }
          exists = true;
          wedge.VertexIndex = posIdx;
          break;
        }
        if (!exists)
        {
          wedge.VertexIndex = (int32)controlPoints.size();
          controlPoints.emplace_back(vertex.Position);
        }
        wedge.UVs[0] = vertex.UVs[0];
        wedge.MatIndex = material;
        wedge.TangentZ = vertex.TangentZ;
      }
      std::swap(face.WedgeIndex[0], face.WedgeIndex[1]);
    }
  }

  FWriteStream s(ctx.Path);
  PskChunkHeader mainHeader("ACTRHEAD");
  s << mainHeader;
  PskChunkHeader pointsHeader("PNTS0000", sizeof(FVector), (int32)controlPoints.size());
  s << pointsHeader;
  FVector tmpPos;
  for (const FVector& pos : controlPoints)
  {
    tmpPos = pos * ctx.Scale3D;
    tmpPos.Y = -tmpPos.Y;
    s << tmpPos;
  }
  PskChunkHeader wedgesHeader("VTXW0000", sizeof(VVertexExport), (int32)wedges.size());
  s << wedgesHeader;
  for (const VVertex& wedge : wedges)
  {
    VVertexExport tmp(wedge);
    s << tmp;
  }
  PskChunkHeader facesHeader("FACE0000", sizeof(VTriangleExport), (int32)faces.size());
  s << facesHeader;
  for (const VTriangle& face : faces)
  {
    VTriangleExport tmp(face);
    s << tmp;
  }
  PskChunkHeader matHeader("MATT0000", sizeof(VMaterial), (int32)materials.size());
  s << matHeader;
  for (VMaterial& mat : materials)
  {
    s << mat;
  }
  PskChunkHeader skelHeader("REFSKELT", sizeof(FNamedBoneBinary), 0);
  s << skelHeader;
  PskChunkHeader inflHeader("RAWWEIGHTS", sizeof(FVertInfluence), 0);
  s << inflHeader;
  return true;
}

bool PskUtils::ExportAnimationSequence(USkeletalMesh* sourceMesh, UAnimSequence* sequence, MeshExportContext& ctx)
{
  if (!sourceMesh)
  {
    ctx.Error = "Error!\n\nThe animation sequence is empty.";
    return false;
  }
  std::vector<FMeshBone> refBones = sourceMesh->GetReferenceSkeleton();

  FWriteStream s(ctx.Path);
  PskChunkHeader animHeader("ANIMHEAD", 0, 0);
  s << animHeader;

  PskChunkHeader bonesHeader("BONENAMES", sizeof(FNamedBoneBinary), refBones.size());
  s << bonesHeader;
  for (int32 idx = 0; idx < refBones.size(); ++idx)
  {
    const FMeshBone& refBone = refBones[idx];
    FNamedBoneBinary bone;
    bone.SetName(refBone.Name.String().UTF8().c_str());
    bone.ParentIndex = refBone.ParentIndex;
    if (refBone.ParentIndex == refBones[refBone.ParentIndex].ParentIndex)
    {
      bone.ParentIndex = -1;
    }
    bone.NumChildren = refBone.NumChildren;
    s << bone;
  }

  PskChunkHeader infoHeader("ANIMINFO", sizeof(AnimInfoBinary), 1);
  s << infoHeader;

  AnimInfoBinary animInfo;
  animInfo.SetName(sequence->SequenceName.String().C_str());
  animInfo.SetGroup(NAME_None);
  animInfo.TotalBones = refBones.size();
  animInfo.KeyQuotum = sequence->NumFrames * refBones.size();
  animInfo.AnimRate = sequence->GetFrameRate() * ctx.TrackRateScale;
  animInfo.TrackTime = sequence->SequenceLength * sequence->GetFrameRate();
  animInfo.FirstRawFrame = 0;
  animInfo.NumRawFrames = sequence->GetRawFramesCount();
  s << animInfo;

  return ExportSequence(sourceMesh, sequence, ctx, s);
}

bool PskUtils::ExportAnimationSet(USkeletalMesh* sourceMesh, UAnimSet* animSet, MeshExportContext& ctx)
{
  if (!sourceMesh)
  {
    ctx.Error = "Error!\n\nMissing skeletal mesh.";
    return false;
  }
  if (!animSet)
  {
    ctx.Error = "Error!\n\nAnimSet is NULL.";
    return false;
  }
  if (!ctx.SplitTakes)
  {
    ctx.Error = "Error!\n\nActorX PSA format does not support merged animations.\nTo fix the issue turn on \"Split takes\" checkbox in the export options.";
    return false;
  }
  sourceMesh->Load();
  animSet->Load();

  std::vector<UAnimSequence*> sequences = animSet->GetSequences();
  if (sequences.empty())
  {
    ctx.Error = "AnimSet has no sequences!\n\nNothing to export.";
    return false;
  }

  if (ctx.ExportMesh)
  {
    if (ctx.ProgressDescFunc)
    {
      ctx.ProgressDescFunc("Building ActorX model...");
    }
    std::wstring tmp = ctx.Path;
    ctx.Path = ((std::filesystem::path(tmp) / sourceMesh->GetObjectNameString().WString()).replace_extension("psk")).wstring();
    ExportSkeletalMesh(sourceMesh, ctx);
    ctx.Path = tmp;
  }
  if (ctx.ProgressDescFunc)
  {
    ctx.ProgressDescFunc("Exporting animation takes...");
  }
  int32 total = (int32)sequences.size();
  if (ctx.ProgressMaxFunc)
  {
    ctx.ProgressMaxFunc(total);
  }
  for (int32 idx = 0; idx < total; ++idx)
  {
    UAnimSequence* seq = sequences[idx];
    if (!seq)
    {
      continue;
    }
    
    if (ctx.ProgressFunc)
    {
      ctx.ProgressFunc(idx + 1);
    }
    std::wstring tmp = ctx.Path;
    ctx.Path = ((std::filesystem::path(tmp) / seq->SequenceName.String().WString()).replace_extension("psk")).wstring();
    ExportAnimationSequence(sourceMesh, seq, ctx);
    ctx.Path = tmp;
  }
  return true;
}

bool PskUtils::ExportSequence(USkeletalMesh* sourceMesh, UAnimSequence* sequence, MeshExportContext& ctx, FStream& s)
{
  if (!sourceMesh || !sequence)
  {
    return false;
  }
  std::vector<FMeshBone> refBones = sourceMesh->GetReferenceSkeleton();

  if (refBones.empty())
  {
    return false;
  }

  PskChunkHeader keysHeader("ANIMKEYS", sizeof(VQuatAnimKey), sequence->NumFrames * (int32)refBones.size());
  s << keysHeader;

  const int32 rawFrames = sequence->GetRawFramesCount();
  const int32 totalBones = (int32)refBones.size();
  for (int32 frame = 0; frame < rawFrames; ++frame)
  {
    VQuatAnimKey key;
    FVector position;
    FQuat orientation;
    for (int32 boneIndex = 0; boneIndex < totalBones; ++boneIndex)
    {
      if (!sequence->GetBoneTransform(sourceMesh, refBones[boneIndex].Name, frame, position, orientation))
      {
        position = refBones[boneIndex].BonePos.Position;
        orientation = refBones[boneIndex].BonePos.Orientation;
      }

      position *= ctx.Scale3D;
      position.Y *= -1.f;
      orientation.Y *= -1.f;
      // Looks like PSA requires -W by default.
      // Not a typo! If the flag is OFF - inverse!
      if (!ctx.InverseAnimQuatW)
      {
        orientation.W *= -1.f;
      }

      key.Orientation = orientation;
      key.Position = position;
      s << key;
    }
  }
  return true;
}
