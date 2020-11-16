#include "UPhysAsset.h"

FRigidBodyIndexPair::FRigidBodyIndexPair(int32 idx1, int32 idx2)
{
  Indices[0] = std::min(idx1, idx2);
  Indices[1] = std::max(idx1, idx2);
}

bool FRigidBodyIndexPair::operator<(const FRigidBodyIndexPair& b) const
{
  return std::tie(Indices[0], Indices[1]) < std::tie(b.Indices[0], b.Indices[1]);
}


FStream& operator<<(FStream& s, FRigidBodyIndexPair& p)
{
  return s << p.Indices[0] << p.Indices[1];
}

FStream& operator<<(FStream& s, FKCachedConvexDataElement& e)
{
  s << e.ConvexElementDataElementSize;
  if (e.ConvexElementDataElementSize != sizeof(uint8))
  {
    UThrow("Invalid ConvexElementDataElementSize %d", e.ConvexElementDataElementSize);
  }
  s << e.ConvexElementData;
  return s;
}

FStream& operator<<(FStream& s, FKCachedConvexData& e)
{
  return s << e.CachedConvexElements;
}

void URB_BodySetup::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << CachedPhysData;
}

void UPhysicsAssetInstance::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << CollisionDisableMap;
}