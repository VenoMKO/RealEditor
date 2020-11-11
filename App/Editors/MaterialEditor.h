#pragma once
#include "GenericEditor.h"
#include "../Windows/MaterialView.h"
#include <wx/scrolwin.h>

#include <Tera/UMaterial.h>
#include <Tera/UMaterialExpression.h>

class MaterialEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  MaterialEditor(wxPanel* parent, PackageWindow* window);

  void OnObjectLoaded() override;

protected:
  void BuildGraph();

protected:
  bool NeedsGraph = true;
  wxScrolledWindow* Canvas = nullptr;
  int32 CanvasOffsetX = 0;
  int32 CanvasOffsetY = 0;
};