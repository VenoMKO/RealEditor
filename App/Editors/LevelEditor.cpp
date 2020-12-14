#include "LevelEditor.h"
#include "../App.h"
#include "../Windows/PackageWindow.h"
#include "../Windows/ProgressWindow.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgUtil/SmoothingVisitor>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/PositionAttitudeTransform>

#include <Tera/Cast.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UObject.h>
#include <Tera/ULevelStreaming.h>
#include <Tera/UStaticMesh.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/USpeedTree.h>
#include <Tera/UMaterial.h>
#include <Tera/UTexture.h>
#include <Tera/ULight.h>

#include <Utils/T3DUtils.h>
#include <Utils/FbxUtils.h>

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
  toolbar->AddSeparator();
  toolbar->AddTool(eID_Level_Load, "Load", wxBitmap("#112", wxBITMAP_TYPE_PNG_RESOURCE), "Load the level preview");
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
  UObject* world = Level->GetOuter();
  auto worldInner = world->GetInner();
  std::vector<ULevel*> levels = { Level };
  int maxProgress = Level->GetActorsCount();
  for (UObject* inner : worldInner)
  {
    if (ULevelStreaming* streamedLevel = Cast<ULevelStreaming>(inner))
    {
      streamedLevel->Load();
      if (streamedLevel->Level && streamedLevel->Level != Level)
      {
        levels.push_back(streamedLevel->Level);
        maxProgress += streamedLevel->Level->GetActorsCount();
      }
    }
  }
  ProgressWindow progress(this, "Exporting");
  progress.SetMaxProgress(maxProgress);

  std::thread([&] {
    for (ULevel* level : levels)
    {
      ExportLevel(level, ctx, &progress);
      if (progress.IsCanceled())
      {
        SendEvent(&progress, UPDATE_PROGRESS_FINISH);
        return;
      }
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();
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

  static const osg::Vec3d yawAxis(0.0, 0.0, -1.0);
  static const osg::Vec3d pitchAxis(-1.0, 0.0, 0.0);
  static const osg::Vec3d rollAxis(0.0, -1.0, 0.0);

  for (UActor* actor : actors)
  {
    if (!actor)
    {
      continue;
    }

    osg::Geode* geode = nullptr;
    FVector compTranslation;
    FRotator compRotation;
    FVector compScale3D;
    float compScale = 1.;
    if (UStaticMeshActor* a = Cast<UStaticMeshActor>(actor))
    {
      geode = CreateStaticActor(a, compTranslation, compScale3D, compRotation, compScale);
    }

    if (geode)
    {
      FVector location = actor->GetLocation();
      geode->setName(actor->GetObjectName().UTF8().c_str());
      osg::PositionAttitudeTransform* transform = new osg::PositionAttitudeTransform;
      transform->addChild(geode);
      transform->setPosition(osg::Vec3(location.X + compTranslation.X,
        -(location.Y + compTranslation.Y),
        location.Z + compTranslation.Z));
      transform->setScale(osg::Vec3(actor->DrawScale3D.X * actor->DrawScale * compScale3D.X * compScale,
        actor->DrawScale3D.Y * actor->DrawScale * compScale3D.Y * compScale,
        actor->DrawScale3D.Z * actor->DrawScale * compScale3D.Z * compScale));

      osg::Quat quat;
      FRotator rot = actor->Rotation + compRotation;
      FVector euler = rot.Normalized().Euler(); // {X: Roll, Y: Pitch, Z: Yaw}
      quat.makeRotate(
        euler.X * M_PI / 180., pitchAxis,
        euler.Y * M_PI / 180., rollAxis,
        euler.Z * M_PI / 180., yawAxis
      );
      transform->setAttitude(quat);
      root->addChild(transform);
    }
  }
}

void LevelEditor::ExportLevel(ULevel* level, LevelExportContext& ctx, ProgressWindow* progress)
{
  auto AddCommonActorParameters = [](T3DFile& f, UActor* actor) {
    f.AddString("ActorLabel", actor->GetObjectName().UTF8().c_str());
    f.AddString("FolderPath", actor->GetPackage()->GetPackageName().UTF8().c_str());
  };

  auto AddCommonActorComponentParameters = [&ctx](T3DFile& f, UActor* actor, UActorComponent* component, bool bakeTransform = false) {
    FVector position = bakeTransform ? actor->Location : (actor->GetLocation() + component->Translation);
    FRotator rotation = bakeTransform ? actor->Rotation : (actor->Rotation + component->Rotation);
    FVector scale3D = bakeTransform ? actor->DrawScale3D : (actor->DrawScale3D * component->Scale3D);
    float scale = bakeTransform ? actor->DrawScale : (actor->DrawScale * component->Scale);
    f.AddPosition(position);
    f.AddRotation(rotation);
    f.AddScale(scale3D, scale);
    if (actor->bHidden && !ctx.Config.IgnoreHidden)
    {
      f.AddBool("bVisible", false);
    }
  };

  auto AddCommonPrimitiveComponentParameters = [](T3DFile& f, UPrimitiveComponent* component) {
    if (component->MinDrawDistance)
    {
      f.AddFloat("MinDrawDistance", component->MinDrawDistance);
    }
    if (component->MaxDrawDistance)
    {
      f.AddFloat("LDMaxDrawDistance", component->MaxDrawDistance);
    }
    if (component->CachedMaxDrawDistance)
    {
      f.AddFloat("CachedMaxDrawDistance", component->CachedMaxDrawDistance);
    }
    f.AddBool("CastShadow", component->CastShadow);
    f.AddBool("bCastDynamicShadow", component->bCastDynamicShadow);
    f.AddBool("bCastStaticShadow", component->bCastStaticShadow);
    if (!component->bAcceptsLights)
    {
      f.AddCustom("LightingChannels","(bChannel0=False)");
    }
  };

  auto AddCommonLightComponentParameters = [](T3DFile& f, ULightComponent* component) {
    f.AddGuid("LightGuid", component->LightGuid);
    f.AddColor("LightColor", component->LightColor);
    f.AddBool("CastShadows", component->CastShadows);
    f.AddBool("CastStaticShadows", component->CastStaticShadows);
    f.AddBool("CastDynamicShadows", component->CastDynamicShadows);
  };

  auto GetLocalDir = [](UObject* obj, const char* sep = "\\") {
    std::string path;
    FObjectExport* outer = obj->GetExportObject()->Outer;
    while (outer)
    {
      path = (outer->GetObjectName().UTF8() + sep + path);
      outer = outer->Outer;
    }
    if ((obj->GetExportFlags() & EF_ForcedExport) == 0)
    {
      path = obj->GetPackage()->GetPackageName().UTF8() + sep + path;
    }
    return path;
  };

  auto GetUniqueFbxName = [&ctx](UActor* actor, UActorComponent* comp, std::string& outObjectName) {
    if (ctx.Config.BakeComponentTransform)
    {
      LevelExportContext::ComponentTransform t;
      t.PrePivot = actor->PrePivot;
      t.Translation = comp->Translation;
      t.Rotation = comp->Rotation;
      t.Scale3D = comp->Scale3D;
      t.Scale = comp->Scale;

      auto& transforms = ctx.FbxComponentTransformMap[outObjectName];
      for (size_t idx = 0; idx < transforms.size(); ++idx)
      {
        if (transforms[idx] == t)
        {
          // Found a name with the same offset. Use it.
          outObjectName += "_" + std::to_string(idx);
          return;
        }
      }
      // Add a new name and offset.
      outObjectName += "_" + std::to_string(transforms.size());
      transforms.push_back(t);
      return;
    }
  };

  std::vector<UActor*> actors = level->GetActors();

  if (actors.empty())
  {
    return;
  }

  std::vector<UInterpActor*> interpActors;
  T3DFile f;
  f.InitializeMap();
  size_t initialSize = f.GetBody().size();
  for (UActor* actor : actors)
  {
    if (progress)
    {
      if (progress->IsCanceled())
      {
        return;
      }
      ctx.CurrentProgress++;
      SendEvent(progress, UPDATE_PROGRESS, ctx.CurrentProgress);
      if (actor)
      {
        FString name = level->GetPackage()->GetPackageName() + "/" + actor->GetObjectName();
        SendEvent(progress, UPDATE_PROGRESS_DESC, wxString(L"Exporting: " + name.WString()));
      }
    }
    if (!actor)
    {
      continue;
    }

    const FString className = actor->GetClassName();
    if (className == UStaticMeshActor::StaticClassName())
    {
      if (!ctx.Config.Statics)
      {
        continue;
      }
      UStaticMeshActor* staticMeshActor = Cast<UStaticMeshActor>(actor);
      if (!staticMeshActor || !staticMeshActor->StaticMeshComponent)
      {
        continue;
      }
      staticMeshActor->StaticMeshComponent->Load();
      if (!staticMeshActor->StaticMeshComponent->StaticMesh && !staticMeshActor->StaticMeshComponent->ReplacementPrimitive)
      {
        continue;
      }
      UStaticMeshComponent* component = staticMeshActor->StaticMeshComponent->ReplacementPrimitive ? Cast<UStaticMeshComponent>(staticMeshActor->StaticMeshComponent->ReplacementPrimitive) : staticMeshActor->StaticMeshComponent;

      if (!component)
      {
        component = staticMeshActor->StaticMeshComponent;
      }
      else
      {
        component->Load();
      }

      f.Begin("Actor", "StaticMeshActor", FString::Sprintf("StaticMeshActor_%d", ctx.StaticMeshActorsCount++).UTF8().c_str());
      {
        f.Begin("Object", "StaticMeshComponent", "StaticMeshComponent0");
        f.End();
        f.Begin("Object", nullptr, "StaticMeshComponent0");
        {
          if (component->StaticMesh)
          {
            std::filesystem::path path = ctx.GetStaticMeshDir() / GetLocalDir(component->StaticMesh);
            std::error_code err;
            if (!std::filesystem::exists(path, err))
            {
              std::filesystem::create_directories(path, err);
            }
            if (std::filesystem::exists(path, err))
            {
              std::string fbxName = component->StaticMesh->GetObjectName().UTF8();
              GetUniqueFbxName(actor, component, fbxName);
              path /= fbxName;
              path.replace_extension("fbx");
              if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
              {
                FbxUtils utils;
                FbxExportContext fbxCtx;
                fbxCtx.Path = path.wstring();
                fbxCtx.ApplyRootTransform = ctx.Config.BakeComponentTransform;
                fbxCtx.PrePivot = actor->PrePivot;
                fbxCtx.Translation = component->Translation;
                fbxCtx.Rotation = component->Rotation;
                fbxCtx.Scale3D = component->Scale3D * component->Scale;
                fbxCtx.ExportLods = ctx.Config.ExportLods;
                fbxCtx.ExportCollisions = ctx.Config.ConvexCollisions;
                utils.ExportStaticMesh(component->StaticMesh, fbxCtx);
              }
              f.AddStaticMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->StaticMesh, "/") + fbxName).c_str());
            }
          }
          AddCommonPrimitiveComponentParameters(f, component);
          AddCommonActorComponentParameters(f, actor, component, ctx.Config.BakeComponentTransform);
        }
        f.End();
        f.AddString("StaticMeshComponent", "StaticMeshComponent0");
        f.AddString("RootComponent", "StaticMeshComponent0");
        AddCommonActorParameters(f, staticMeshActor);
      }
      f.End();
      continue;
    }
    if (className == USkeletalMeshActor::StaticClassName())
    {
      if (!ctx.Config.Skeletals)
      {
        continue;
      }
      USkeletalMeshActor* skeletalMeshActor = Cast<USkeletalMeshActor>(actor);
      if (!skeletalMeshActor || !skeletalMeshActor->SkeletalMeshComponent)
      {
        continue;
      }

      if (!skeletalMeshActor->SkeletalMeshComponent->SkeletalMesh && !skeletalMeshActor->SkeletalMeshComponent->ReplacementPrimitive)
      {
        continue;
      }
      USkeletalMeshComponent* component = skeletalMeshActor->SkeletalMeshComponent->ReplacementPrimitive ? Cast<USkeletalMeshComponent>(skeletalMeshActor->SkeletalMeshComponent->ReplacementPrimitive) : skeletalMeshActor->SkeletalMeshComponent;

      if (!component)
      {
        component = skeletalMeshActor->SkeletalMeshComponent;
      }
      else
      {
        component->Load();
      }

      f.Begin("Actor", "SkeletalMeshActor", FString::Sprintf("SkeletalMeshActor_%d", ctx.SkeletalMeshActorsCount++).UTF8().c_str());
      {
        f.Begin("Object", "SkeletalMeshComponent", "SkeletalMeshComponent0");
        f.End();
        f.Begin("Object", nullptr, "SkeletalMeshComponent0");
        {
          if (component->SkeletalMesh)
          {
            std::filesystem::path path = ctx.GetSkeletalMeshDir() / GetLocalDir(component->SkeletalMesh);
            std::error_code err;
            if (!std::filesystem::exists(path, err))
            {
              std::filesystem::create_directories(path, err);
            }
            if (std::filesystem::exists(path, err))
            {
              std::string fbxName = component->SkeletalMesh->GetObjectName().UTF8();
              GetUniqueFbxName(actor, component, fbxName);
              path /= fbxName;
              path.replace_extension("fbx");
              if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
              {
                FbxUtils utils;
                FbxExportContext fbxCtx;
                fbxCtx.Path = path.wstring();
                fbxCtx.ApplyRootTransform = ctx.Config.BakeComponentTransform;
                fbxCtx.PrePivot = actor->PrePivot;
                fbxCtx.Translation = component->Translation;
                fbxCtx.Rotation = component->Rotation;
                fbxCtx.Scale3D = component->Scale3D * component->Scale;
                fbxCtx.ExportLods = ctx.Config.ExportLods;
                utils.ExportSkeletalMesh(component->SkeletalMesh, fbxCtx);
              }
              f.AddSkeletalMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->SkeletalMesh, "/") + fbxName).c_str());
            }
          }
          f.AddCustom("ClothingSimulationFactory", "Class'\"/Script/ClothingSystemRuntimeNv.ClothingSimulationFactoryNv\"'");
          AddCommonPrimitiveComponentParameters(f, component);
          AddCommonActorComponentParameters(f, actor, component);
        }
        f.End();
        f.AddString("SkeletalMeshComponent", "SkeletalMeshComponent0");
        f.AddString("RootComponent", "SkeletalMeshComponent0");
        AddCommonActorParameters(f, skeletalMeshActor);
      }
      f.End();

      continue;
    }
    if (className == UInterpActor::StaticClassName())
    {
      if (!ctx.Config.Interps)
      {
        continue;
      }
      UInterpActor* interpActor = Cast<UInterpActor>(actor);
      if (!interpActor || !interpActor->StaticMeshComponent)
      {
        continue;
      }

      if (!interpActor->StaticMeshComponent->StaticMesh && !interpActor->StaticMeshComponent->ReplacementPrimitive)
      {
        continue;
      }
      UStaticMeshComponent* component = interpActor->StaticMeshComponent->ReplacementPrimitive ? Cast<UStaticMeshComponent>(interpActor->StaticMeshComponent->ReplacementPrimitive) : interpActor->StaticMeshComponent;

      if (!component)
      {
        component = interpActor->StaticMeshComponent;
      }
      else
      {
        component->Load();
      }

      interpActors.push_back(interpActor);

      f.Begin("Actor", "StaticMeshActor", FString::Sprintf("StaticMeshActor_%d", ctx.StaticMeshActorsCount++).UTF8().c_str());
      {
        f.Begin("Object", "StaticMeshComponent", "StaticMeshComponent0");
        f.End();
        f.Begin("Object", nullptr, "StaticMeshComponent0");
        {
          UStaticMesh* mesh = component->StaticMesh;
          bool replicated = false;
          if (interpActor->ReplicatedMesh)
          {
            mesh = interpActor->ReplicatedMesh;
            replicated = true;
          }
          if (mesh)
          {
            std::filesystem::path path = ctx.GetStaticMeshDir() / GetLocalDir(mesh);
            std::error_code err;
            if (!std::filesystem::exists(path, err))
            {
              std::filesystem::create_directories(path, err);
            }
            if (std::filesystem::exists(path, err))
            {
              std::string fbxName = component->StaticMesh->GetObjectName().UTF8();
              GetUniqueFbxName(actor, component, fbxName);
              path /= fbxName;
              path.replace_extension("fbx");
              if (ctx.Config.OverrideData || !std::filesystem::exists(path, err))
              {
                FbxUtils utils;
                FbxExportContext fbxCtx;
                fbxCtx.Path = path.wstring();
                fbxCtx.ApplyRootTransform = ctx.Config.BakeComponentTransform;
                fbxCtx.PrePivot = actor->PrePivot;
                fbxCtx.Translation = component->Translation;
                fbxCtx.Rotation = component->Rotation;
                fbxCtx.Scale3D = component->Scale3D * component->Scale;
                fbxCtx.ExportLods = ctx.Config.ExportLods;
                fbxCtx.ExportCollisions = ctx.Config.ConvexCollisions;
                utils.ExportStaticMesh(component->StaticMesh, fbxCtx);
              }
              f.AddStaticMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->StaticMesh, "/") + fbxName).c_str());
            }
          }
          AddCommonPrimitiveComponentParameters(f, component);
          if (replicated && !ctx.Config.BakeComponentTransform)
          {
            FVector vec = actor->Location;
            if (interpActor->ReplicatedMeshTranslationProperty)
            {
              vec = vec + interpActor->ReplicatedMeshTranslation;
            }
            else
            {
              vec = vec + component->Translation;
            }
            f.AddPosition(vec);

            vec = actor->DrawScale3D;
            if (interpActor->ReplicatedMeshScale3DProperty)
            {
              vec = vec * (interpActor->ReplicatedMeshScale3D / 100.);
            }
            else
            {
              vec = vec * interpActor->DrawScale3D;
            }
            f.AddScale(vec, actor->DrawScale* component->Scale);

            FRotator rot = actor->Rotation;
            if (interpActor->ReplicatedMeshRotationProperty)
            {
              rot = rot + interpActor->ReplicatedMeshRotation;
            }
            else
            {
              rot = rot + component->Rotation;
            }
            f.AddRotation(rot);
          }
          else
          {
            AddCommonActorComponentParameters(f, actor, component);
          }
          f.AddCustom("Mobility", "Movable");
        }
        f.End();
        f.AddString("StaticMeshComponent", "StaticMeshComponent0");
        f.AddString("RootComponent", "StaticMeshComponent0");
        AddCommonActorParameters(f, interpActor);
      }
      f.End();
      continue;
    }
    if (className == USpeedTreeActor::StaticClassName())
    {
      if (!ctx.Config.SpeedTrees)
      {
        continue;
      }
      USpeedTreeActor* speedTreeActor = Cast<USpeedTreeActor>(actor);
      if (!speedTreeActor || !speedTreeActor->SpeedTreeComponent)
      {
        continue;
      }

      if (!speedTreeActor->SpeedTreeComponent->SpeedTree && !speedTreeActor->SpeedTreeComponent->ReplacementPrimitive)
      {
        continue;
      }
      USpeedTreeComponent* component = speedTreeActor->SpeedTreeComponent->ReplacementPrimitive ? Cast<USpeedTreeComponent>(speedTreeActor->SpeedTreeComponent->ReplacementPrimitive) : speedTreeActor->SpeedTreeComponent;

      if (!component)
      {
        component = speedTreeActor->SpeedTreeComponent;
      }
      else
      {
        component->Load();
      }

      f.Begin("Actor", "StaticMeshActor", FString::Sprintf("StaticMeshActor_%d", ctx.SpeedTreeActorsCount++).UTF8().c_str());
      {
        f.Begin("Object", "StaticMeshComponent", "StaticMeshComponent0");
        f.End();
        f.Begin("Object", nullptr, "StaticMeshComponent0");
        {
          if (component->SpeedTree)
          {
            std::filesystem::path path = ctx.GetSpeedTreeDir() / GetLocalDir(component->SpeedTree);
            std::error_code err;
            if (!std::filesystem::exists(path, err))
            {
              std::filesystem::create_directories(path, err);
            }
            if (std::filesystem::exists(path, err))
            {
              path /= component->SpeedTree->GetObjectName().UTF8();
              path.replace_extension("spt");
              if (!std::filesystem::exists(path, err))
              {
                void* sptData = nullptr;
                FILE_OFFSET sptDataSize = 0;
                component->SpeedTree->GetSptData(&sptData, &sptDataSize, false);
                std::ofstream s(path, std::ios::out | std::ios::trunc | std::ios::binary);
                s.write((const char*)sptData, sptDataSize);
                free(sptData);
              }
              f.AddStaticMesh((std::string(ctx.DataDirName) + "/" + GetLocalDir(component->SpeedTree, "/") + component->SpeedTree->GetObjectName().UTF8()).c_str());
            }
          }
          AddCommonPrimitiveComponentParameters(f, component);
          AddCommonActorComponentParameters(f, actor, component);
        }
        f.End();
        f.AddString("StaticMeshComponent", "StaticMeshComponent0");
        f.AddString("RootComponent", "StaticMeshComponent0");
        AddCommonActorParameters(f, speedTreeActor);
      }
      f.End();
      continue;
    }
    if (className == UPointLight::StaticClassName())
    {
      if (!ctx.Config.PointLights)
      {
        continue;
      }
      UPointLight* pointLightActor = Cast<UPointLight>(actor);
      if (!pointLightActor || !pointLightActor->LightComponent)
      {
        continue;
      }

      UPointLightComponent* component = Cast<UPointLightComponent>(pointLightActor->LightComponent);
      if (!component)
      {
        continue;
      }

      f.Begin("Actor", "PointLight", FString::Sprintf("PointLight_%d", ctx.PointLightActorsCount++).UTF8().c_str());
      {
        f.Begin("Object", "PointLightComponent", "LightComponent0");
        f.End();
        f.Begin("Object", nullptr, "LightComponent0");
        {
          f.AddFloat("LightFalloffExponent", component->FalloffExponent);
          f.AddFloat("Intensity", component->Brightness * ctx.Config.PointLightMul);
          f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
          f.AddFloat("AttenuationRadius", component->Radius);
          f.AddCustom("LightmassSettings", wxString::Format("(ShadowExponent=%.06f)", component->ShadowFalloffExponent).c_str());
          AddCommonLightComponentParameters(f, component);
          AddCommonActorComponentParameters(f, actor, component);
          f.AddCustom("Mobility", "Static");
        }
        f.End();
        f.AddString("PointLightComponent", "LightComponent0");
        f.AddString("LightComponent", "LightComponent0");
        f.AddString("RootComponent", "LightComponent0");
        AddCommonActorParameters(f, actor);
      }
      f.End();
      continue;
    }
    if (className == USpotLight::StaticClassName())
    {
      if (!ctx.Config.SpotLights)
      {
        continue;
      }
      USpotLight* spotLightActor = Cast<USpotLight>(actor);
      if (!spotLightActor || !spotLightActor->LightComponent)
      {
        continue;
      }

      USpotLightComponent* component = Cast<USpotLightComponent>(spotLightActor->LightComponent);
      if (!component)
      {
        continue;
      }

      f.Begin("Actor", "SpotLight", FString::Sprintf("SpotLight_%d", ctx.SpotLightActorsCount++).UTF8().c_str());
      {
        f.Begin("Object", "SpotLightComponent", "LightComponent0");
        f.End();
        f.Begin("Object", "ArrowComponent", "ArrowComponent0");
        f.End();
        f.Begin("Object", nullptr, "LightComponent0");
        {
          f.AddFloat("LightFalloffExponent", component->FalloffExponent);
          f.AddFloat("Intensity", component->Brightness * ctx.Config.SpotLightMul);
          f.AddFloat("InnerConeAngle", component->InnerConeAngle);
          f.AddFloat("OuterConeAngle", component->OuterConeAngle);
          f.AddBool("bUseInverseSquaredFalloff", ctx.Config.InvSqrtFalloff);
          f.AddFloat("AttenuationRadius", component->Radius);
          f.AddCustom("LightmassSettings", wxString::Format("(ShadowExponent=%.06f)", component->ShadowFalloffExponent).c_str());
          AddCommonLightComponentParameters(f, component);
          f.AddPosition(actor->GetLocation() + component->Translation);
          FVector scale3D = actor->DrawScale3D * component->Scale3D;
          float scale = actor->DrawScale * component->Scale;
          FRotator rotation = actor->Rotation;
          if (scale3D.X < 0 || scale3D.Y < 0 || scale3D.Z < 0)
          {
            if (scale3D.X < 0)
            {
              rotation.Roll += 0x8000;
              rotation.Yaw += 0x8000;
              scale3D.X *= -1.;
            }
            if (scale3D.Y < 0)
            {
              rotation.Pitch += 0x8000;
              rotation.Yaw += 0x8000;
              scale3D.Y *= -1.;
            }
            if (scale3D.Z < 0)
            {
              rotation.Pitch += 0x8000;
              rotation.Roll += 0x8000;
              scale3D.Z *= -1.;
            }
          }
          f.AddRotation(rotation + component->Rotation);
          f.AddScale(scale3D, scale);
          f.AddCustom("Mobility", "Static");
        }
        f.End();
        f.AddString("SpotLightComponent", "LightComponent0");
        f.AddString("ArrowComponent", "ArrowComponent0");
        f.AddString("LightComponent", "LightComponent0");
        f.AddString("RootComponent", "LightComponent0");
        AddCommonActorParameters(f, actor);
      }
      f.End();
      continue;
    }
    if (className == UEmitter::StaticClassName())
    {
      if (!ctx.Config.Emitters)
      {
        continue;
      }
      UEmitter* emitter = Cast<UEmitter>(actor);
      if (!emitter)
      {
        continue;
      }
      f.Begin("Actor", "Actor", FString::Sprintf("Actor_%d", ctx.UntypedActorsCount++).UTF8().c_str());
      {
        f.Begin("Object", "SceneComponent", "DefaultSceneRoot");
        f.End();
        f.Begin("Object", nullptr, "DefaultSceneRoot");
        {
          f.AddPosition(emitter->Location);
          f.AddBool("bVisualizeComponent", true);
          f.AddCustom("CreationMethod", "Instance");
        }
        f.End();
        f.AddCustom("RootComponent", "SceneComponent'\"DefaultSceneRoot\"'");
        AddCommonActorParameters(f, emitter);
        f.AddCustom("InstanceComponents(0)", "SceneComponent'\"DefaultSceneRoot\"'");
      }
      f.End();
      continue;
    }
  }

  if (initialSize == f.GetBody().size())
  {
    // Don't save empty T3D
    return;
  }

  f.FinalizeMap();
  std::filesystem::path dst = std::filesystem::path(ctx.Config.RootDir.WString()) / level->GetPackage()->GetPackageName().WString();
  dst.replace_extension("t3d");
  f.Save(dst);
}

void LevelEditor::OnIdle(wxIdleEvent& e)
{
  Unbind(wxEVT_IDLE, &LevelEditor::OnIdle, this);
}

osg::Geode* LevelEditor::CreateStaticActor(UStaticMeshActor* actor, FVector& translation, FVector& scale3d, FRotator& rotation, float& scale)
{
  if (!actor->StaticMeshComponent)
  {
    return nullptr;
  }
  UStaticMeshComponent* component = actor->StaticMeshComponent;
  UStaticMesh* mesh = nullptr;

  // TODO: not sure what should be replaced. The component or its model?
  if (component->ReplacementPrimitive)
  {
    UStaticMeshComponent* replacement = Cast<UStaticMeshComponent>(component->ReplacementPrimitive);
    if (replacement && replacement->StaticMesh)
    {
      mesh = replacement->StaticMesh;
    }
    else
    {
      mesh = component->StaticMesh;
    }
  }
  else
  {
    mesh = component->StaticMesh;
  }

  if (!mesh)
  {
    return nullptr;
  }

  translation = component->Translation;
  scale3d = component->Scale3D;
  rotation = component->Rotation;
  scale = component->Scale;

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

  osg::Geode* result = new osg::Geode;
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
    result->addDrawable(geo);
  }
  return result;
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
