#pragma once

#include "GenericEditor.h"
#include <Tera/UTexture.h>

#include "OSGWindow.h"

class TextureEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  ~TextureEditor() override;

  void OnObjectLoaded() override;

  void OnTick() override;

protected:
  void OnObjectSet() override
  {
    if (!Renderer)
    {
      CreateRenderer();
    }
  }
  // Create OpenGL context
  void CreateRenderer();

  // Create the texture and push it to the renderer;
  void CreateRenderTexture();

private:
  UTexture2D* Texture = nullptr;
  OSGCanvas* Canvas = nullptr;
  OSGWindow* OSGProxy = nullptr;
  osgViewer::Viewer* Renderer = nullptr;
};