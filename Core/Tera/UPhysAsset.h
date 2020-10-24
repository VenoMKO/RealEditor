#pragma once
#include "UObject.h"

struct FRigidBodyIndexPair {
  int32 Indices[2] = { 0, 0 };

  FRigidBodyIndexPair() = default;
  FRigidBodyIndexPair(int32 idx1, int32 idx2);

  bool operator<(const FRigidBodyIndexPair& b) const;

  friend FStream& operator<<(FStream& s, FRigidBodyIndexPair& p);
};

struct FKCachedConvexDataElement {
  std::vector<uint8> ConvexElementData;
  uint32 ConvexElementDataElementSize = 1;

  friend FStream& operator<<(FStream& s, FKCachedConvexDataElement& e);
};

struct FKCachedConvexData {
public:
  std::vector<FKCachedConvexDataElement>	CachedConvexElements;

  friend FStream& operator<<(FStream& s, FKCachedConvexData& e);
};

class URB_BodySetup : public UObject {
public:
  DECL_UOBJ(URB_BodySetup, UObject);

  void Serialize(FStream& s) override;

protected:
  std::vector<FKCachedConvexData> CachedPhysData;
};

class UPhysicsAssetInstance : public UObject {
public:
  DECL_UOBJ(UPhysicsAssetInstance, UObject);

  void Serialize(FStream& s) override;

protected:
  std::map<FRigidBodyIndexPair, bool> CollisionDisableMap;
};