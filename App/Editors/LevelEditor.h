#pragma once
#include "GenericEditor.h"

#include <Tera/ULevel.h>

#include "../Misc/OSGWindow.h"

#include <unordered_map>
#include <filesystem>

class UActor;
class ULevel;
class UStaticMeshActor;
class ULevelStreamingVolume;

class ProgressWindow;

struct LevelExportContext {
  const char* DataDirName = "S1Game";
  const char* StaticMeshStorage = "StaticMeshes";
  const char* SkeletalMeshStorage = "SkeletalMeshes";
  const char* SpeedTreeStorage = "SpeedTrees";
  const char* TextureStorage = "Textures";
  const char* MaterialStorage = "Materials";
  std::filesystem::path Root;

  inline std::filesystem::path GetStaticMeshDir() const
  {
    return Root / StaticMeshStorage / DataDirName;
  }

  inline std::filesystem::path GetSkeletalMeshDir() const
  {
    return Root / SkeletalMeshStorage / DataDirName;
  }

  inline std::filesystem::path GetSpeedTreeDir() const
  {
    return Root / SpeedTreeStorage / DataDirName;
  }

  inline std::filesystem::path GetTextureDir() const
  {
    return Root / TextureStorage / DataDirName;
  }

  inline std::filesystem::path GetMaterialDir() const
  {
    return Root / MaterialStorage / DataDirName;
  }

  bool Terrains = false;

  bool StaticMeshes = false;
  
  bool SkeletalMeshes = false;

  bool SpotLights = false;
  
  bool PointLights = false;

  bool InterpActors = false;
  
  bool SpeedTrees = false;

  bool Emitters = false;

  bool Sounds = false;
  
  bool ResampleTerrain = false;
  bool InvSqrtLightFalloff = false;
  float SpotLightMultiplier = 1.;
  float PointLightMultiplier = 1.;
  bool Materials = false;
  bool Textures = false;

  int CurrentProgress = 0;
  int StaticMeshActorsCount = 0;
  int SkeletalMeshActorsCount = 0;
  int SpeedTreeActorsCount = 0;
  int PointLightActorsCount = 0;
  int SpotLightActorsCount = 0;
  int UntypedActorsCount = 0;
};

class LevelEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  LevelEditor(wxPanel* parent, PackageWindow* window);

  ~LevelEditor() override
  {
    delete Renderer;
  }

  void OnTick() override;
  void OnObjectLoaded() override;

  void PopulateToolBar(wxToolBar* toolbar) override;

  void OnToolBarEvent(wxCommandEvent& event) override;

  void OnExportClicked(wxCommandEvent& e) override;

  void SetNeedsUpdate() override;

protected:
  void CreateRenderer();
  void LoadPersistentLevel();
  void CreateLevel(ULevel* level, osg::Geode* root);
  void ExportLevel(ULevel* level, LevelExportContext& ctx, ProgressWindow* progress);
  void OnIdle(wxIdleEvent& e);

  osg::Geode* CreateStaticActor(UStaticMeshActor* actor, FVector& translation, FVector& scale3d, FRotator& rotation, float& scale);
  osg::Geode* CreateStreamingLevelVolumeActor(ULevelStreamingVolume* actor);

protected:
  ULevel* Level = nullptr;
  bool LevelLoaded = false;
  std::unordered_map<UActor*, osg::Geode*> LevelNodes;
  osg::ref_ptr<osg::Geode> Root = nullptr;
  OSGCanvas* Canvas = nullptr;
  OSGWindow* OSGProxy = nullptr;
  osgViewer::Viewer* Renderer = nullptr;
};