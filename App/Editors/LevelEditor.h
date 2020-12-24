#pragma once
#include "GenericEditor.h"

#include <Tera/ULevel.h>
#include <Tera/ULevelStreaming.h>

#include "../Misc/OSGWindow.h"
#include "../Windows/LevelExportOptions.h"
#include <osg/MatrixTransform>

#include <unordered_map>
#include <filesystem>

class UActor;
class ULevel;
class UPrefabInstance;
class ULevelStreamingVolume;

class ProgressWindow;

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
  void PrepareToExportLevel(LevelExportContext& ctx);
  void ExportLevel(class T3DFile& file, ULevel* level, LevelExportContext& ctx, ProgressWindow* progress);
  bool ExportMaterialsAndTexture(LevelExportContext& ctx, ProgressWindow* progress);
  void OnIdle(wxIdleEvent& e);

  osg::MatrixTransform* CreateStaticMeshComponent(UStaticMeshComponent* actor);
  osg::MatrixTransform* CreateSkelMeshComponent(USkeletalMeshComponent* component);
  osg::MatrixTransform* CreatePrefabInstance(UPrefabInstance* instance);
  osg::Geode* CreateStreamingLevelVolumeActor(ULevelStreamingVolume* actor);

protected:
  ULevel* Level = nullptr;
  bool LevelLoaded = false;
  bool ShowEmptyMessage = false;
  std::unordered_map<UActor*, osg::Geode*> LevelNodes;
  osg::ref_ptr<osg::Geode> Root = nullptr;
  OSGCanvas* Canvas = nullptr;
  OSGWindow* OSGProxy = nullptr;
  osgViewer::Viewer* Renderer = nullptr;
};

class StreamingLevelEditor : public LevelEditor {
public:
  using LevelEditor::LevelEditor;
  void OnObjectLoaded() override;
  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnToolBarEvent(wxCommandEvent& event) override;
};