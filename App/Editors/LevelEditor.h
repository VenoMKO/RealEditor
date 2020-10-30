#pragma once
#include "GenericEditor.h"

#include <Tera/ULevel.h>

#include "../Misc/OSGWindow.h"

#include <unordered_map>

class UActor;
class ULevel;
class UStaticMeshActor;
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

  void SetNeedsUpdate() override;

protected:
  void CreateRenderer();
  void OnIdle(wxIdleEvent& e);

  osg::Geode* CreateStaticActor(UStaticMeshActor* actor, FVector& translation, FVector& scale3d, FRotator& rotation, float& scale);

protected:
  ULevel* Level = nullptr;
  std::vector<UActor*> LevelActors;
  std::unordered_map<UActor*, osg::Geode*> LevelNodes;
  osg::ref_ptr<osg::Geode> Root = nullptr;
  OSGCanvas* Canvas = nullptr;
  OSGWindow* OSGProxy = nullptr;
  osgViewer::Viewer* Renderer = nullptr;
};