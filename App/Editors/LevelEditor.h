#pragma once
#include "GenericEditor.h"

#include <Tera/ULevel.h>

#include "../Misc/OSGWindow.h"
#include "../Windows/LevelExportOptions.h"
#include <osg/MatrixTransform>

#include <unordered_map>
#include <filesystem>

class UActor;
class ULevel;
class UStaticMeshActor;
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
  void ExportLevel(ULevel* level, LevelExportContext& ctx, ProgressWindow* progress);
  bool ExportMaterialsAndTexture(LevelExportContext& ctx, ProgressWindow* progress);
  void OnIdle(wxIdleEvent& e);

  osg::MatrixTransform* CreateStaticMeshComponent(UStaticMeshActor* actor);
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