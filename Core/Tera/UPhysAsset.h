#pragma once
#include "UObject.h"
#include "FStructs.h"

struct FKConvexElem {
  std::vector<FVector> VertexData;
  std::vector<int32> FaceTriData;
  void SetupFromPropertyValue(FPropertyValue* value);
};

struct FKAggregateGeom {
  // TODO: implement if necessary
  // std::vector<FKBoxElem> BoxElems;
  // std::vector<FKSphylElem> SphylElems;
  // std::vector<FKSphereElem> SphereElems;
  std::vector<FKConvexElem> ConvexElems;
};

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

  void PostLoad() override;

  FKAggregateGeom GetAggregateGeometry() const
  {
    return Geomtry;
  }

protected:
  void LoadAggGeom(const std::vector<FPropertyValue*>& root);

protected:
  std::vector<FKCachedConvexData> CachedPhysData;
  FKAggregateGeom Geomtry;
};

class UPhysicsAssetInstance : public UObject {
public:
  DECL_UOBJ(UPhysicsAssetInstance, UObject);

  void Serialize(FStream& s) override;

protected:
  std::map<FRigidBodyIndexPair, bool> CollisionDisableMap;
};