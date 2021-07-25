#include "UPhysAsset.h"
#include "FPackage.h"
#include "UClass.h"

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

void URB_BodySetup::PostLoad()
{
  Super::PostLoad();
  for (FPropertyTag* tag : Properties)
  {
    if (tag->Name == "AggGeom")
    {
      LoadAggGeom(tag->GetArray(), GetPackage());
      break;
    }
  }
}

void AggGeomOwner::LoadAggGeom(const std::vector<FPropertyValue*>& root, FPackage* package)
{
  for (FPropertyValue* rootVal : root)
  {
    FPropertyTag* tag = rootVal->GetPropertyTagPtr();
    const FString name = tag->Name.String();
    if (name == "ConvexElems")
    {
      Geomtry.ConvexElems.resize(tag->GetArray().size());
      for (int32 idx = 0; idx < tag->GetArray().size(); ++idx)
      {
        Geomtry.ConvexElems[idx].SetupFromPropertyValue(tag->GetArray()[idx]);
      }
    }
    else if (!package->GetPackageFlag(PKG_ContainsScript) && (name == "BoxElems" || name == "SphylElems" || name == "SphereElems"))
    {
      // TODO: implement if necessary
    }
  }
}

void UPhysicsAssetInstance::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << CollisionDisableMap;
}

void FKConvexElem::SetupFromPropertyValue(FPropertyValue* value)
{
  for (FPropertyValue* v : value->GetArray())
  {
    if (v->GetPropertyTagPtr()->Name == "VertexData")
    {
      for (FPropertyValue* vector : v->GetPropertyTagPtr()->GetArray())
      {
        FVector& vertex = VertexData.emplace_back();
        vector->GetVector(vertex);
      }
    }
    else if (v->GetPropertyTagPtr()->Name == "FaceTriData")
    {
      for (FPropertyValue* index : v->GetPropertyTagPtr()->GetArray())
      {
        FaceTriData.push_back(index->GetInt());
      }
    }
    else if (v->GetPropertyTagPtr()->Name == "ElemBox")
    {
      for (FPropertyValue* tmp : v->GetPropertyTagPtr()->GetArray())
      {
        if (tmp->Field && tmp->Field->GetObjectName() == "Min")
        {
          tmp->GetArray()[0]->GetVector(BoxData.Min);
        }
        else if (tmp->Field && tmp->Field->GetObjectName() == "Max")
        {
          tmp->GetArray()[0]->GetVector(BoxData.Max);
        }
        else if (tmp->Field && tmp->Field->GetObjectName() == "IsValid")
        {
          BoxData.IsValid = tmp->GetArray()[0]->GetBool();
        }
      }
    }
  }
}
