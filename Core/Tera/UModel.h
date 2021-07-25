#pragma once
#include "UObject.h"
#include "FStructs.h"
#include "Cast.h"

struct FBspNode
{
  enum { MAX_NODE_VERTICES = 255 };
  enum { MAX_ZONES = 64 };

  FPlane Plane;
  int32 iVertPool = 0;
  float ShadowMapScale = 0.;
  int32 iSurf = 0;
  int32 iVertexIndex = 0;
  uint16 ComponentIndex = 0;
  uint16 ComponentNodeIndex = 0;
  int32 ComponentElementIndex = 0;

  union { int32 iBack; int32 iChild[1]; };
  int32 iFront = 0;
  int32 iPlane;

  int32 iCollisionBound;

  uint8 iZone[2] = { 0, 0 };
  uint8 NumVertices = 0;
  uint8 NodeFlags = 0;
  int32 iLeaf[2] = { 0, 0 };

  friend FStream& operator<<(FStream& s, FBspNode& n);
};

struct FBspSurf {
  DECL_UREF(class UMaterialInterface, Material);
  uint32 PolyFlags = 0;
  int32 pBase = 0;
  int32 vNormal = 0;
  int32 vTextureU = 0;
  int32 vTextureV = 0;
  int32 iBrushPoly = 0;
  DECL_UREF(class UActor, Actor);
  FPlane Plane;
  float ShadowMapScale = .0f;

  uint32 LightingChannels = 0;
  int32 LightmassIndex = 0;
  int32 Unk1 = 0;


  friend FStream& operator<<(FStream& s, FBspSurf& surf);
};

struct FVert {
  int32 pVertex = 0;
  int32 pSide = 0;
  FVector2D ShadowTexCoord;
  FVector2D BackfaceShadowTexCoord;

  friend FStream& operator<<(FStream& s, FVert& v);
};

struct FZoneProperties {
  DECL_UREF(UObject, ZoneActor);
  float LastRenderTime = 0;
  uint64 Connectivity = 0;
  uint64 Visibility = 0;

  friend FStream& operator<<(FStream& s, FZoneProperties& z);
};

struct FModelVertex {
  FVector Position;
  FPackedNormal TangentX;
  FPackedNormal TangentZ;
  FVector2D TexCoord;
  FVector2D ShadowTexCoord;

  friend FStream& operator<<(FStream& s, FModelVertex& v);
};

struct FLightmassPrimitiveSettings {
  bool bUseTwoSidedLighting = false;
  bool bShadowIndirectOnly = false;
  bool bUseEmissiveForStaticLighting = false;
  uint32 Unk1 = 0;
  float EmissiveLightFalloffExponent = 1.;
  float EmissiveLightExplicitInfluenceRadius = 0.;
  float EmissiveBoost = 1.;
  float DiffuseBoost = 1.;
  float SpecularBoost = 1.;

  friend FStream& operator<<(FStream& s, FLightmassPrimitiveSettings& l);
};

struct FPoly {
  FVector Base;
  FVector Normal;
  FVector TextureU;
  FVector TextureV;
  std::vector<FVector> Vertices;
  uint32 PolyFlags = 0;
  DECL_UREF(class UActor, Actor);
  DECL_UREF(class UMaterialInterface, Material);
  FName RulesetVariation;
  FName ItemName;
  int32 iLink = INDEX_NONE;
  int32 iBrushPoly = INDEX_NONE;
  uint32 SmoothingMask = 0;
  float ShadowMapScale = 32.;
  uint32 LightingChannels = 0;
  FLightmassPrimitiveSettings LightmassSettings;

  friend FStream& operator<<(FStream& s, FPoly& p);
};

class UPolys : public UObject {
public:
  DECL_UOBJ(UPolys, UObject);

  void Serialize(FStream& s) override;

  DECL_UREF(UObject, Owner);
  std::vector<FPoly> Elements;
};

class UModel : public UObject {
public:
  DECL_UOBJ(UModel, UObject);

  void Serialize(FStream& s) override;

  class UPolys* GetPolys() const
  {
    return Cast<class UPolys>(Polys);
  }

protected:
  FBoxSphereBounds Bounds;
  std::vector<FVector> Vectors;
  std::vector<FVector> Points;
  std::vector<FBspNode> Nodes;
  std::vector<FBspSurf> Surfs;
  std::vector<FVert> Verts;
  std::vector<int32> PortalNodes;
  int32 NumSharedSides = 0;
  int32 NumZones = 0;
  FZoneProperties Zones[FBspNode::MAX_ZONES];
  DECL_UREF(UObject, Owner);
  DECL_UREF(UObject, Polys);
  std::vector<int32> LeafHulls;
  std::vector<int32> Leaves;
  bool RootOutside = false;
  bool Linked = false;
  std::vector<FMeshEdge> LegacyEdges;
  uint32 NumUniqueVertices = 0;
  std::vector<FModelVertex> VertexBuffer;
  FGuid LightmassGuid;
  std::vector<FLightmassPrimitiveSettings> LightmassSettings;
};