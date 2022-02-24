#include "PrefabEditor.h"
#include "../Windows/PackageWindow.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgUtil/SmoothingVisitor>
#include <osg/BlendFunc>
#include <osg/Depth>

#include <Tera/Cast.h>
#include <Tera/UTexture.h>
#include <Tera/UActorComponent.h>
#include <Tera/UStaticMesh.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/UMaterial.h>

static const osg::Vec3d YawAxis(0.0, 0.0, -1.0);
static const osg::Vec3d PitchAxis(0.0, -1.0, 0.0);
static const osg::Vec3d RollAxis(1.0, 0.0, 0.0);

osg::MatrixTransform* CreateStaticMesh(UStaticMeshComponent* component);
osg::MatrixTransform* CreateSkelMesh(USkeletalMeshComponent* component);

PrefabEditor::PrefabEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  CreateRenderer();
  window->FixOSG();
}

PrefabEditor::~PrefabEditor()
{
  if (Renderer)
  {
    delete Renderer;
  }
}

void PrefabEditor::OnTick()
{
  if (Renderer && Renderer->isRealized() && Renderer->checkNeedToDoFrame())
  {
    Renderer->frame();
  }
}

void PrefabEditor::OnObjectLoaded()
{
  if (Loading || !Prefab)
  {
    Prefab = (UPrefab*)Object;
    CreateRenderModel();
  }
  GenericEditor::OnObjectLoaded();
}

void PrefabEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
}

void PrefabEditor::OnToolBarEvent(wxCommandEvent& e)
{
  GenericEditor::OnToolBarEvent(e);
}

void PrefabEditor::SetNeedsUpdate()
{
  if (Renderer)
  {
    Renderer->requestRedraw();
  }
}

void PrefabEditor::CreateRenderer()
{
  int attrs[] = { int(WX_GL_DOUBLEBUFFER), WX_GL_RGBA, WX_GL_DEPTH_SIZE, 8, WX_GL_STENCIL_SIZE, 8, 0 };

  Canvas = new OSGCanvas(Window, this, wxID_ANY, wxDefaultPosition, GetSize(), wxNO_BORDER, wxT("OSGCanvas"), attrs);

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(Canvas, 1, wxALL | wxEXPAND);
  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);

  OSGProxy = new OSGWindow(Canvas);
  Canvas->SetGraphicsWindow(OSGProxy);
  Renderer = new osgViewer::Viewer;
  Renderer->setRunFrameScheme(osgViewer::ViewerBase::FrameScheme::ON_DEMAND);
  Renderer->getCamera()->setClearColor({ .3, .3, .3, 1 });
  Renderer->getCamera()->setGraphicsContext(OSGProxy);
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
  Renderer->getCamera()->setProjectionMatrixAsPerspective(60, GetSize().x / GetSize().y, 0.1, 500);
  Renderer->getCamera()->setDrawBuffer(GL_BACK);
  Renderer->getCamera()->setReadBuffer(GL_BACK);

#if _DEBUG
  Renderer->addEventHandler(new osgViewer::StatsHandler);
#endif

  osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
  manipulator->setAllowThrow(false);
  manipulator->setVerticalAxisFixed(true);
  Renderer->setCameraManipulator(manipulator);
  Renderer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
}

void PrefabEditor::CreateRenderModel()
{
  if (!Prefab)
  {
    return;
  }
  Root = new osg::Geode;

  for (UObject* archetype : Prefab->PrefabArchetypes)
  {
    UActor* actor = Cast<UActor>(archetype);
    if (!actor)
    {
      continue;
    }
    osg::MatrixTransform* componentTransform = nullptr;
    if (UStaticMeshActor* actorSM = Cast<UStaticMeshActor>(actor))
    {
      componentTransform = CreateStaticMesh(Cast<UStaticMeshComponent>(actorSM->StaticMeshComponent));
    }
    else if (UInterpActor* actorIp = Cast<UInterpActor>(actor))
    {
      componentTransform = CreateStaticMesh(Cast<UStaticMeshComponent>(actorIp->StaticMeshComponent));
    }
    else if (USkeletalMeshActor* actorSkel = Cast<USkeletalMeshActor>(actor))
    {
      componentTransform = CreateSkelMesh(Cast<USkeletalMeshComponent>(actorSkel->SkeletalMeshComponent));
    }
    if (!componentTransform)
    {
      continue;
    }

    osg::MatrixTransform* mt = new osg::MatrixTransform;
    mt->setName(actor->GetObjectNameString().UTF8().c_str());
    osg::Matrix m;
    m.makeIdentity();

    FVector location = actor->GetLocation();
    FVector rotation = actor->Rotation.Normalized().Euler();
    FVector scale3D = actor->DrawScale3D * actor->DrawScale;

    m.preMultTranslate(osg::Vec3(location.X, -location.Y, location.Z));

    osg::Quat quat;
    quat.makeRotate(
      rotation.X * M_PI / 180., RollAxis,
      rotation.Y * M_PI / 180., PitchAxis,
      rotation.Z * M_PI / 180., YawAxis
    );
    m.preMultRotate(quat);

    m.preMultScale(osg::Vec3(scale3D.X, scale3D.Y, scale3D.Z));

    mt->setMatrix(m);
    mt->addChild(componentTransform);
    Root->addChild(mt);
  }
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
  Renderer->setSceneData(Root.get());
}

osg::MatrixTransform* CreateStaticMesh(UStaticMeshComponent* component)
{
  if (!component)
  {
    return nullptr;
  }

  UStaticMesh* mesh = component->StaticMesh;
  if (!mesh)
  {
    return nullptr;
  }

  const FStaticMeshRenderData* model = mesh->GetLod(0);

  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
  osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  osg::ref_ptr<osg::Vec2Array> uvs = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX);

  std::vector<FStaticVertex> uvertices = model->GetVertices();
  for (auto& uvertex : uvertices)
  {
    FVector normal = uvertex.TangentZ;
    normals->push_back(osg::Vec3(normal.X, -normal.Y, normal.Z));
    vertices->push_back(osg::Vec3(uvertex.Position.X, -uvertex.Position.Y, uvertex.Position.Z));
    uvs->push_back(osg::Vec2(uvertex.UVs[0].X, uvertex.UVs[0].Y));
  }

  osg::Geode* geode = new osg::Geode;
  std::vector<FStaticMeshElement> elements = model->GetElements();
  const FRawIndexBuffer& indexContainer = model->IndexBuffer;
  for (const FStaticMeshElement& section : elements)
  {
    if (!section.NumTriangles)
    {
      continue;
    }
    osg::Geometry* geo = new osg::Geometry;
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES);
    for (uint32 faceIndex = 0; faceIndex < section.NumTriangles; ++faceIndex)
    {
      indices->push_back(indexContainer.GetIndex(section.FirstIndex + (faceIndex * 3) + 0));
      indices->push_back(indexContainer.GetIndex(section.FirstIndex + (faceIndex * 3) + 1));
      indices->push_back(indexContainer.GetIndex(section.FirstIndex + (faceIndex * 3) + 2));
    }
    geo->addPrimitiveSet(indices.get());
    geo->setVertexArray(vertices.get());
    geo->setNormalArray(normals.get());
    geo->setTexCoordArray(0, uvs.get());

    if (UMaterialInterface* material = Cast<UMaterialInterface>(section.Material))
    {
      if (UTexture2D* tex = material->GetDiffuseTexture())
      {
        osg::ref_ptr<osg::Image> img = new osg::Image;
        UTextureBitmapInfo info(64);
        if (tex->GetBitmapData(info) && info.IsValid())
        {
          img->setImage(info.Width, info.Height, 0, info.InternalFormat, info.Format, info.Type, (unsigned char*)info.Allocation, osg::Image::AllocationMode::NO_DELETE);
        }
        osg::ref_ptr<osg::Texture2D> osgtex = new osg::Texture2D(img);
        osgtex->setWrap(osg::Texture::WrapParameter::WRAP_S, osg::Texture::WrapMode::REPEAT);
        osgtex->setWrap(osg::Texture::WrapParameter::WRAP_T, osg::Texture::WrapMode::REPEAT);
        geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, osgtex.get());
        if (material->GetBlendMode() == EBlendMode::BLEND_Masked)
        {
          geo->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
          geo->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }
      }
    }
    geode->addDrawable(geo);
  }

  FVector translation = component->Translation;
  FVector rotation = component->Rotation.Normalized().Euler();
  FVector scale3d = component->Scale3D * component->Scale;

  osg::MatrixTransform* mt = new osg::MatrixTransform;
  mt->setName(component->GetObjectNameString().UTF8().c_str());
  osg::Matrix m;
  m.makeIdentity();

  m.preMultTranslate(osg::Vec3(translation.X, -translation.Y, translation.Z));

  osg::Quat quat;
  quat.makeRotate(
    rotation.X * M_PI / 180., RollAxis,
    rotation.Y * M_PI / 180., PitchAxis,
    rotation.Z * M_PI / 180., YawAxis
  );
  m.preMultRotate(quat);

  m.preMultScale(osg::Vec3(scale3d.X, scale3d.Y, scale3d.Z));

  mt->setMatrix(m);
  mt->addChild(geode);

  return mt;
}

osg::MatrixTransform* CreateSkelMesh(USkeletalMeshComponent* component)
{
  if (!component)
  {
    return nullptr;
  }

  USkeletalMesh* mesh = component->SkeletalMesh;
  if (!mesh)
  {
    return nullptr;
  }

  const FStaticLODModel* model = mesh->GetLod(0);

  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
  osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  osg::ref_ptr<osg::Vec2Array> uvs = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX);

  std::vector<FSoftSkinVertex> uvertices = model->GetVertices();
  for (int32 idx = 0; idx < uvertices.size(); ++idx)
  {
    FVector normal = uvertices[idx].TangentZ;
    normals->push_back(osg::Vec3(normal.X, -normal.Y, normal.Z));
    vertices->push_back(osg::Vec3(uvertices[idx].Position.X, -uvertices[idx].Position.Y, uvertices[idx].Position.Z));
    uvs->push_back(osg::Vec2(uvertices[idx].UVs[0].X, uvertices[idx].UVs[0].Y));
  }

  osg::Geode* geode = new osg::Geode;
  std::vector<const FSkelMeshSection*> sections = model->GetSections();
  const FMultiSizeIndexContainer* indexContainer = model->GetIndexContainer();
  for (const FSkelMeshSection* section : sections)
  {
    osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES);
    for (uint32 faceIndex = 0; faceIndex < section->NumTriangles; ++faceIndex)
    {
      indices->push_back(indexContainer->GetIndex(section->BaseIndex + (faceIndex * 3) + 0));
      indices->push_back(indexContainer->GetIndex(section->BaseIndex + (faceIndex * 3) + 1));
      indices->push_back(indexContainer->GetIndex(section->BaseIndex + (faceIndex * 3) + 2));
    }
    geo->addPrimitiveSet(indices.get());
    geo->setVertexArray(vertices.get());
    geo->setNormalArray(normals.get());
    geo->setTexCoordArray(0, uvs.get());

    // TODO: Use MaterialMap to remap section materials to the global materials list
    if (mesh->GetMaterials().size() > section->MaterialIndex)
    {
      if (UMaterialInterface* material = Cast<UMaterialInterface>(mesh->GetMaterials()[section->MaterialIndex]))
      {
        if (UTexture2D* tex = material->GetDiffuseTexture())
        {
          osg::ref_ptr<osg::Image> img = new osg::Image;
          UTextureBitmapInfo info;
          if (tex->GetBitmapData(info) && info.IsValid())
          {
            img->setImage(info.Width, info.Height, 0, info.InternalFormat, info.Format, info.Type, (unsigned char*)info.Allocation, osg::Image::AllocationMode::NO_DELETE);
          }
          osg::ref_ptr<osg::Texture2D> osgtex = new osg::Texture2D(img);
          osgtex->setWrap(osg::Texture::WrapParameter::WRAP_S, osg::Texture::WrapMode::REPEAT);
          osgtex->setWrap(osg::Texture::WrapParameter::WRAP_T, osg::Texture::WrapMode::REPEAT);
          geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, osgtex.get());
          if (material->GetBlendMode() == EBlendMode::BLEND_Masked)
          {
            geo->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            geo->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
          }
        }
      }
    }
    geode->addDrawable(geo.get());
  }

  FVector translation = component->Translation;
  FVector rotation = component->Rotation.Normalized().Euler();
  FVector scale3d = component->Scale3D * component->Scale;

  osg::MatrixTransform* mt = new osg::MatrixTransform;
  mt->setName(component->GetObjectNameString().UTF8().c_str());
  osg::Matrix m;
  m.makeIdentity();

  m.preMultTranslate(osg::Vec3(translation.X, -translation.Y, translation.Z));

  osg::Quat quat;
  quat.makeRotate(
    rotation.X * M_PI / 180., RollAxis,
    rotation.Y * M_PI / 180., PitchAxis,
    rotation.Z * M_PI / 180., YawAxis
  );
  m.preMultRotate(quat);

  m.preMultScale(osg::Vec3(scale3d.X, scale3d.Y, scale3d.Z));

  mt->setMatrix(m);
  mt->addChild(geode);

  return mt;
}