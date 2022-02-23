#pragma once
#include "GenericEditor.h"
#include <Tera/USkeletalMesh.h>
#include <Tera/UTexture.h>

#include "../CustomViews/OSGView.h"

class SkelMeshEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  SkelMeshEditor(wxPanel* parent, PackageWindow* window);
  ~SkelMeshEditor() override;

  void OnTick() override;
  void OnObjectLoaded() override;

  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnToolBarEvent(wxCommandEvent& e) override;
  void OnExportClicked(wxCommandEvent&) override;
  void OnImportClicked(wxCommandEvent&) override;
  void SetNeedsUpdate() override;

protected:
  void CreateRenderer();
  void CreateRenderModel();
  void OnRefreshClicked();
  void OnMaterialsClicked();

protected:
  USkeletalMesh* Mesh = nullptr;
  wxToolBarToolBase* SkeletonTool = nullptr;
  osg::ref_ptr<osg::Geode> Root = nullptr;
  OSGCanvas* Canvas = nullptr;
  OSGWindow* OSGProxy = nullptr;
  osgViewer::Viewer* Renderer = nullptr;
  bool ShowSkeleton = true;
};