#include "LevelEditor.h"
#include "../App.h"
#include "../Windows/PackageWindow.h"
#include "../Windows/ProgressWindow.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgUtil/SmoothingVisitor>
#include <osg/BlendFunc>
#include <osg/Depth>

#include <Tera/Cast.h>
#include <Tera/FPackage.h>
#include <Tera/UObject.h>
#include <Tera/UStaticMesh.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/UTexture.h>
#include <Tera/UMaterial.h>
#include <Tera/UPrefab.h>

static const osg::Vec3d YawAxis(0.0, 0.0, -1.0);
static const osg::Vec3d PitchAxis(0.0, -1.0, 0.0);
static const osg::Vec3d RollAxis(1.0, 0.0, 0.0);

LevelEditor::LevelEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  CreateRenderer();
  window->FixOSG();
}

void LevelEditor::OnTick()
{
  if (Renderer && Renderer->isRealized() && Renderer->checkNeedToDoFrame())
  {
    Renderer->frame();
    if (LevelLoaded && ShowEmptyMessage)
    {
      ShowEmptyMessage = false;
      wxMessageBox("Probably the level consists of lights, sounds, emitters, or other actors that can't be rendered.", "There are no actors to display!", wxICON_INFORMATION);
    }
  }
}

void LevelEditor::OnObjectLoaded()
{
  if (Loading || !Level)
  {
    LevelNodes.clear();
    Level = (ULevel*)Object;
    Root = new osg::Geode;
    Bind(wxEVT_IDLE, &LevelEditor::OnIdle, this);
  }
  GenericEditor::OnObjectLoaded();
}

void LevelEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  auto tool = toolbar->AddTool(eID_Level_Load, LevelLoaded ? "Loaded" : "Preview", wxBitmap("#127", wxBITMAP_TYPE_PNG_RESOURCE), "Load the level preview");
  tool->Enable(!LevelLoaded && Level);
  if (!Level)
  {
    tool->SetShortHelp("The level preview can't be loaded because the GMP file was not found!");
  }
  else if (LevelLoaded)
  {
    tool->SetShortHelp("The level was loaded!");
  }
}

void LevelEditor::OnToolBarEvent(wxCommandEvent& event)
{
  GenericEditor::OnToolBarEvent(event);
  if (event.GetSkipped())
  {
    event.Skip(false);
    return;
  }
  if (event.GetId() == eID_Level_Load)
  {
    LoadPersistentLevel();
    ShowEmptyMessage = !Root->getNumChildren();
    if (wxToolBarBase* item = (wxToolBarBase*)event.GetEventObject())
    {
      if (wxToolBarToolBase* sender = item->FindById(event.GetId()))
      {
        sender->Enable(false);
        if (LevelLoaded)
        {
          sender->SetLabel("Loaded");
          sender->SetShortHelp("The level was loaded!");
        }
      }
    }
  }
}

void LevelEditor::OnExportClicked(wxCommandEvent& e)
{
  LevelExportContext ctx = LevelExportContext::LoadFromAppConfig();
  LevelExportOptionsWindow optionsWin(this, ctx);
  if (optionsWin.ShowModal() != wxID_OK)
  {
    return;
  }
  ctx = optionsWin.GetExportContext();
  LevelExportContext::SaveToAppConfig(ctx);

  PrepareToExportLevel(ctx);
}

void LevelEditor::SetNeedsUpdate()
{
  if (Renderer)
  {
    Renderer->requestRedraw();
  }
}

void LevelEditor::CreateRenderer()
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
  Renderer->setThreadingModel(osgViewer::Viewer::AutomaticSelection);
}

void LevelEditor::LoadPersistentLevel()
{
  if (LevelLoaded)
  {
    wxMessageBox("Level is already loaded!", wxEmptyString, wxICON_INFORMATION);
    return;
  }
  if (!Level)
  {
    if (Object)
    {
      wxMessageBox(wxString::Format("Failed to load the level %s.gmp!", ((ULevelStreaming*)Object)->PackageName.UTF8().c_str()), wxT("Error!"), wxICON_ERROR);
    }
    else
    {
      wxMessageBox(wxT("Failed to load the level!"), wxT("Error!"), wxICON_ERROR);
    }
    return;
  }
  ProgressWindow progress(Window, "Please, wait...");
  progress.SetCurrentProgress(-1);
  progress.SetCanCancel(false);
  progress.SetActionText("Loading the level");
  progress.Layout();

  std::thread([&] {
    CreateLevel(Level, Root.get());
    UObject* world = Level->GetOuter();
    auto worldInner = world->GetInner();

    for (UObject* inner : worldInner)
    {
      if (ULevelStreaming* level = Cast<ULevelStreaming>(inner))
      {
        level->Load();
        if (level->Level)
        {
          osg::Geode* geode = new osg::Geode;
          geode->setName(level->Level->GetPackage()->GetPackageName().C_str());
          CreateLevel(level->Level, geode);
          Root->addChild(geode);
        }
      }
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH, true);
  }).detach();

  progress.ShowModal();

  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
  Renderer->setSceneData(Root.get());
  LevelLoaded = true;
}

void LevelEditor::CreateLevel(ULevel* level, osg::Geode* root)
{
  if (!level)
  {
    return;
  }

  auto actors = level->GetActors();

  if (actors.empty())
  {
    return;
  }

  std::vector<UActor*> skip;
  for (UActor* actor : actors)
  {
    if (!actor || actor->bHidden)
    {
      continue;
    }
    if (UStaticMeshActor* staticActor = Cast<UStaticMeshActor>(actor))
    {
      if (staticActor->StaticMeshComponent && staticActor->StaticMeshComponent->ReplacementPrimitive)
      {
        skip.push_back(actor);
      }
    }
    else if (UInterpActor* interpActor = Cast<UInterpActor>(actor))
    {
      if (interpActor->StaticMeshComponent && interpActor->StaticMeshComponent->ReplacementPrimitive)
      {
        skip.push_back(actor);
      }
    }
    else if (USkeletalMeshActor* skelActor = Cast<USkeletalMeshActor>(actor))
    {
      if (skelActor->SkeletalMeshComponent && skelActor->SkeletalMeshComponent->ReplacementPrimitive)
      {
        skip.push_back(actor);
      }
    }
  }

  for (UActor* actor : actors)
  {
    if (!actor || actor->bHidden)
    {
      continue;
    }
    if (std::find(skip.begin(), skip.end(), actor) != skip.end())
    {
      continue;
    }
    osg::MatrixTransform* componentTransform = nullptr;
    if (UStaticMeshActor* staticActor = Cast<UStaticMeshActor>(actor))
    {
      componentTransform = CreateStaticMeshComponent(staticActor->StaticMeshComponent);
    }
    else if (UInterpActor* interpActor = Cast<UInterpActor>(actor))
    {
      componentTransform = CreateStaticMeshComponent(interpActor->StaticMeshComponent);
    }
    else if (USkeletalMeshActor* skelActor = Cast<USkeletalMeshActor>(actor))
    {
      componentTransform = CreateSkelMeshComponent(skelActor->SkeletalMeshComponent);
    }
    else if (UPrefabInstance* prefab = Cast<UPrefabInstance>(actor))
    {
      componentTransform = CreatePrefabInstance(prefab);
    }

    if (!componentTransform)
    {
      continue;
    }

    osg::MatrixTransform* mt = new osg::MatrixTransform;
    mt->setName(actor->GetObjectName().UTF8().c_str());
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
    root->addChild(mt);
  }
}

void LevelEditor::OnIdle(wxIdleEvent& e)
{
  Unbind(wxEVT_IDLE, &LevelEditor::OnIdle, this);
}

osg::MatrixTransform* LevelEditor::CreateStaticMeshComponent(UStaticMeshComponent* component)
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
  for (auto & uvertex : uvertices)
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
    for (int32 faceIndex = 0; faceIndex < section.NumTriangles; ++faceIndex)
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
        tex->RenderTo(img.get(), 64);
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
  mt->setName(component->GetObjectName().UTF8().c_str());
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

osg::MatrixTransform* LevelEditor::CreateSkelMeshComponent(USkeletalMeshComponent* component)
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
    for (int32 faceIndex = 0; faceIndex < section->NumTriangles; ++faceIndex)
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
          tex->RenderTo(img.get());
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
  mt->setName(component->GetObjectName().UTF8().c_str());
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

osg::Geode* LevelEditor::CreateStreamingLevelVolumeActor(ULevelStreamingVolume* actor)
{
  if (!actor || actor->StreamingLevels.empty())
  {
    return nullptr;
  }

  std::vector<ULevelStreaming*> aliveLevels;
  for (ULevelStreaming* level : actor->StreamingLevels)
  {
    if (level && level->Level)
    {
      aliveLevels.push_back(level);
    }
  }

  if (aliveLevels.empty())
  {
    return nullptr;
  }

  osg::Geode* result = new osg::Geode;
  for (ULevelStreaming* level : aliveLevels)
  {
    osg::Geode* lgeode = new osg::Geode;
    lgeode->setName(level->PackageName.UTF8().c_str());
    result->addChild(lgeode);
    CreateLevel(level->Level, lgeode);
  }

  return result;
}

osg::MatrixTransform* LevelEditor::CreatePrefabInstance(UPrefabInstance* instance)
{
  osg::MatrixTransform* root = nullptr;

  for (UObject* archetype : instance->TemplatePrefab->PrefabArchetypes)
  {
    UActor* actor = Cast<UActor>(archetype);
    if (!actor)
    {
      continue;
    }
    osg::MatrixTransform* componentTransform = nullptr;
    if (UStaticMeshActor* actorSM = Cast<UStaticMeshActor>(actor))
    {
      componentTransform = CreateStaticMeshComponent(Cast<UStaticMeshComponent>(actorSM->StaticMeshComponent));
    }
    else if (UInterpActor* actorIp = Cast<UInterpActor>(actor))
    {
      componentTransform = CreateStaticMeshComponent(Cast<UStaticMeshComponent>(actorIp->StaticMeshComponent));
    }
    else if (USkeletalMeshActor* actorSkel = Cast<USkeletalMeshActor>(actor))
    {
      componentTransform = CreateSkelMeshComponent(Cast<USkeletalMeshComponent>(actorSkel->SkeletalMeshComponent));
    }
    if (!componentTransform)
    {
      continue;
    }

    osg::MatrixTransform* mt = new osg::MatrixTransform;
    mt->setName(actor->GetObjectName().UTF8().c_str());
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
    if (!root)
    {
      root = new osg::MatrixTransform;
    }
    root->addChild(mt);
  }
  return root;
}

void StreamingLevelEditor::OnObjectLoaded()
{
  bool needsResetLevel = (Loading || !Level);
  LevelEditor::OnObjectLoaded();
  if (needsResetLevel)
  {
    if (Level)
    {
      Level = ((ULevelStreaming*)Object)->Level;
    }
  }
}

void StreamingLevelEditor::PopulateToolBar(wxToolBar* toolbar)
{
  LevelEditor::PopulateToolBar(toolbar);
  auto tool = toolbar->AddTool(eID_StreamingLevel_Source, "Source", wxBitmap("#116", wxBITMAP_TYPE_PNG_RESOURCE), "Open the source package with this level.");
  tool->Enable(Level);
  if (!Level)
  {
    tool->SetShortHelp("Can't show the source level object because the GMP was not found!");
  }
}

void StreamingLevelEditor::OnToolBarEvent(wxCommandEvent& event)
{
  LevelEditor::OnToolBarEvent(event);
  if (event.GetId() == eID_StreamingLevel_Source && Level)
  {
    App::GetSharedApp()->OpenNamedPackage(Level->GetPackage()->GetPackageName().WString());
  }
}
