#pragma once
#include "GenericEditor.h"

#include <Tera/ULevel.h>

#include "../Misc/OSGWindow.h"

#include <unordered_map>

class UActor;
class ULevel;
class UStaticMeshActor;
class ULevelStreamingVolume;
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

  void SetNeedsUpdate() override;

protected:
  void CreateRenderer();
  void LoadPersistentLevel();
  void CreateLevel(ULevel* level, osg::Geode* root);
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