#pragma once
#include "TextureEditor.h"
#include <Tera/ULandscape.h>

class LandscapeProxyEditor : public TextureEditor {
public:
  using TextureEditor::TextureEditor;
  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnToolBarEvent(wxCommandEvent& e) override;
  void OnExportClicked(wxCommandEvent&) override;
  virtual wxString OnExportWeightsClicked(wxCommandEvent&);

protected:
  void CreateRenderTexture() override;

protected:
  struct UTextureBitmapInfo HeightMapData;
};

class LandscapeEditor : public LandscapeProxyEditor {
public:
  using LandscapeProxyEditor::LandscapeProxyEditor;
  void OnObjectLoaded() override;
  wxString OnExportWeightsClicked(wxCommandEvent& e) override;

protected:
  ULandscape* Landscape = nullptr;
};

class LandscapeComponentEditor : public LandscapeProxyEditor {
public:
  using LandscapeProxyEditor::LandscapeProxyEditor;
  void OnObjectLoaded() override;

protected:
  ULandscapeComponent* Component = nullptr;
};