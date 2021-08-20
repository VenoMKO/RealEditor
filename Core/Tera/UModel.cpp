#include "UModel.h"
#include "UActor.h"
#include "UMaterial.h"

FStream& operator<<(FStream& s, FBspNode& n)
{
  s << n.Plane;
  s << n.ShadowMapScale;
  s << n.iVertPool;
  s << n.iSurf;
  s << n.iVertexIndex;
  s << n.ComponentIndex;
  s << n.ComponentNodeIndex;
  s << n.ComponentElementIndex;
  s << n.iChild[0];
  s << n.iChild[1];
  s << n.iChild[2];
  s << n.iCollisionBound;
  s << n.iZone[0];
  s << n.iZone[1];
  s << n.NumVertices;
  s << n.NodeFlags;
  s << n.iLeaf[0];
  s << n.iLeaf[1];
  return s;
}

FStream& operator<<(FStream& s, FBspSurf& surf)
{
  SERIALIZE_UREF(s, surf.Material);
  s << surf.PolyFlags;
  s << surf.pBase << surf.vNormal;
  s << surf.vTextureU << surf.vTextureV;
  s << surf.iBrushPoly;
  SERIALIZE_UREF(s, surf.Actor);
  s << surf.Plane;
  s << surf.ShadowMapScale;
  s << surf.LightingChannels;
  s << surf.LightmassIndex;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << surf.Unk1;
  }
  return s;
}

FStream& operator<<(FStream& s, FVert& v)
{
  return s << v.pVertex << v.pSide << v.ShadowTexCoord << v.BackfaceShadowTexCoord;
}

FStream& operator<<(FStream& s, FZoneProperties& z)
{
  SERIALIZE_UREF(s, z.ZoneActor);
  s << z.Connectivity;
  s << z.Visibility;
  s << z.LastRenderTime;
  return s;
}

FStream& operator<<(FStream& s, FModelVertex& v)
{
  s << v.Position;
  s << v.TangentX;
  s << v.TangentZ;
  s << v.TexCoord;
  s << v.ShadowTexCoord;
  return s;
}

FStream& operator<<(FStream& s, FLightmassPrimitiveSettings& l)
{
  s << l.bUseTwoSidedLighting;
  s << l.bShadowIndirectOnly;
  s << l.bUseEmissiveForStaticLighting;
  s << l.EmissiveLightFalloffExponent;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << l.EmissiveLightExplicitInfluenceRadius;
    s << l.Unk1;
  }
  s << l.EmissiveBoost;
  s << l.DiffuseBoost;
  s << l.SpecularBoost;
  return s;
}

FStream& operator<<(FStream& s, FPoly& p)
{
  s << p.Base;
  s << p.Normal;
  s << p.TextureU;
  s << p.TextureV;
  s << p.Vertices;
  s << p.PolyFlags;
  SERIALIZE_UREF(s, p.Actor);
  s << p.ItemName;
  SERIALIZE_UREF(s, p.Material);
  s << p.iLink;
  s << p.iBrushPoly;
  s << p.ShadowMapScale;
  s << p.LightingChannels;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << p.LightmassSettings;
    s << p.RulesetVariation;
  }
  return s;
}

void UModel::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Bounds;
  int32 tmp = (int32)sizeof(FVector);
  s << tmp;
  s << Vectors;
  s << tmp;
  s << Points;
  tmp = 64;
  s << tmp;
  s << Nodes;
  SERIALIZE_UREF(s, Owner);
  s << Surfs;
  tmp = (int32)sizeof(FVert);
  s << tmp;
  s << Verts;
  s << NumSharedSides;
  s << NumZones;
  for (int32 i = 0; i < NumZones; i++)
  {
    s << Zones[i];
  }
  SERIALIZE_UREF(s, Polys);
  tmp = (int32)sizeof(int32);
  s << tmp;
  s << LeafHulls;
  tmp = (int32)sizeof(int32);
  s << tmp;
  s << Leaves;
  s << RootOutside;
  s << Linked;
  tmp = (int32)sizeof(int32);
  s << tmp;
  s << PortalNodes;
  if (s.GetFV() < VER_TERA_MODERN)
  {
    tmp = sizeof(FMeshEdge);
    s << tmp;
    s << LegacyEdges;
  }
  s << NumUniqueVertices;
  tmp = sizeof(FModelVertex);
  s << tmp;
  s << VertexBuffer;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << LightmassGuid;
  }
}

void UPolys::Serialize(FStream& s)
{
  Super::Serialize(s);
  int32 count = Elements.size();
  int32 max = count;
  s << count;
  s << max;
  SERIALIZE_UREF(s, Owner);
  if (s.IsReading())
  {
    Elements.resize(count);
  }
  for (int32 idx = 0; idx < count; ++idx)
  {
    s << Elements[idx];
  }
}
