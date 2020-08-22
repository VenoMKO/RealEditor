#pragma once

#include "GenericEditor.h"
#include <Tera/UTexture.h>

#include "OSGWindow.h"

class TextureEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  TextureEditor(wxPanel* parent, PackageWindow* window);

  ~TextureEditor() override;

  void OnObjectLoaded() override;

  void OnTick() override;

  void PopulateToolBar(wxToolBar* toolbar) override;

  void OnToolBarEvent(wxCommandEvent& event) override;

  void OnImportClicked(wxCommandEvent& e) override;

protected:
  void OnAlphaMaskChange();

  // Create OpenGL context
  void CreateRenderer();

  // Create the texture and push it to the renderer;
  void CreateRenderTexture();

private:
  UTexture2D* Texture = nullptr;
  osg::ref_ptr<osg::Geode> Root = nullptr;
  osg::ref_ptr<osg::ColorMask> Mask = nullptr;
  OSGCanvas* Canvas = nullptr;
  OSGWindow* OSGProxy = nullptr;
  osgViewer::Viewer* Renderer = nullptr;
};