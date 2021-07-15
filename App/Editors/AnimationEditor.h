#pragma once
#include "GenericEditor.h"
#include "../Misc/OSGWindow.h"

#include <osgAnimation/Animation>
#include <osgAnimation/BasicAnimationManager>

#include <Tera/UAnimation.h>
#include <Tera/USkeletalMesh.h>

class GenericAnimEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  ~GenericAnimEditor();

  void OnTick() override;
  void OnObjectLoaded() override;
  void ClearToolbar() override;

protected:
  virtual USkeletalMesh* GetMesh() = 0;
  virtual UAnimSequence* GetActiveSequence() = 0;
  void CreateRenderer(wxPanel* parent);
  void CreateRenderModel();

protected:
  USkeletalMesh* Mesh = nullptr;
  osg::ref_ptr<osg::Geode> Root = nullptr;
  OSGCanvas* Canvas = nullptr;
  OSGWindow* OSGProxy = nullptr;
  osgViewer::Viewer* Renderer = nullptr;
  osg::ref_ptr<osgAnimation::BasicAnimationManager> Manager = nullptr;
  osg::ref_ptr<osgAnimation::Animation> ActiveAnimation = nullptr;
};

class AnimSetEditor : public GenericAnimEditor {
public:
  using GenericAnimEditor::GenericAnimEditor;
  AnimSetEditor(wxPanel* parent, PackageWindow* window);

  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnExportClicked(wxCommandEvent& e) override;
  void OnObjectLoaded() override;

protected:
  USkeletalMesh* GetMesh() override;
  UAnimSequence* GetActiveSequence() override;
  void ShowMissingMesh(bool show);
  void OnTakeChanged(wxCommandEvent&);
  void OnMeshClicked(wxCommandEvent&);

private:
  wxChoice* TakePicker = nullptr;
  wxButton* MeshButton = nullptr;
  wxStaticText* ErrorLabel = nullptr;
  wxPanel* ContainerPanel = nullptr;
};

class AnimSequenceEditor : public GenericAnimEditor {
public:
  using GenericAnimEditor::GenericAnimEditor;
  AnimSequenceEditor(wxPanel* parent, PackageWindow* window);

  void OnExportClicked(wxCommandEvent& e) override;

protected:
  USkeletalMesh* GetMesh() override;
  UAnimSequence* GetActiveSequence() override;
};