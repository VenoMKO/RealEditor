#pragma once
#include "GenericEditor.h"
#include <Tera/USkeletalMesh.h>
#include <Tera/UTexture.h>

#include "../Misc/OSGWindow.h"

class SkelMeshEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  SkelMeshEditor(wxPanel* parent, PackageWindow* window);
  ~SkelMeshEditor() override;

  void OnTick() override;
  void OnObjectLoaded() override;

  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnToolBarEvent(wxCommandEvent& e) override;
  void OnExportMenuClicked(wxCommandEvent& e);
  void OnExportClicked(wxCommandEvent&) override;

protected:
  void CreateRenderer();
  void CreateRenderModel();
  void OnRefreshClicked();

protected:
  USkeletalMesh* Mesh = nullptr;
  osg::ref_ptr<osg::Geode> Root = nullptr;
  OSGCanvas* Canvas = nullptr;
  OSGWindow* OSGProxy = nullptr;
  osgViewer::Viewer* Renderer = nullptr;
};