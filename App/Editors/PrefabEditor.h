#pragma once
#include "GenericEditor.h"

#include <Tera/UPrefab.h>
#include <osg/MatrixTransform>

#include "../CustomViews/OSGView.h"


class PrefabEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  PrefabEditor(wxPanel* parent, PackageWindow* window);
  ~PrefabEditor() override;

  void OnTick() override;
  void OnObjectLoaded() override;

  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnToolBarEvent(wxCommandEvent& e) override;
  void SetNeedsUpdate() override;

protected:
  void CreateRenderer();
  void CreateRenderModel();

protected:
  UPrefab* Prefab = nullptr;
  osg::ref_ptr<osg::Geode> Root = nullptr;
  OSGCanvas* Canvas = nullptr;
  OSGWindow* OSGProxy = nullptr;
  osgViewer::Viewer* Renderer = nullptr;
};